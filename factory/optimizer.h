// base abstract class
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class Optimizer {
public:
    virtual ~Optimizer( ) = default;

    // workhorse of the gradient descent procedures
    virtual void update(std::vector<float>&       params,
                        const std::vector<float>& gradients) = 0;

    // hyperparameters of the chosen Optimizer
    virtual void configure(
        const std::unordered_map<std::string, float>& config) = 0;

    // we allow cloning the optimizer -- for prototype pattern to work
    virtual std::unique_ptr<Optimizer> clone( ) const = 0;

    virtual std::string get_name( ) const = 0;
};

class SGDOptimizer : public Optimizer {
private:
    float              _learning_rate = 0.01f;
    float              _momentum      = 0.0f;
    std::vector<float> _velocity;

public:
    SGDOptimizer( ) = default;
    SGDOptimizer(float learning_rate, float momentum = 0.0f)
        : _learning_rate(learning_rate), _momentum(momentum) {}

    void update(std::vector<float>&       params,
                const std::vector<float>& gradients) override {
        // initialize velocity vector for the first update
        if (_velocity.empty( )) _velocity.resize(params.size( ), 0.0f);

        // SGD update with momentum
        for (size_t i = 0; i < params.size( ); ++i) {
            _velocity[i] =
                _momentum * _velocity[i] - _learning_rate * gradients[i];
            params[i] += _velocity[i];
        }
    }

    void configure(
        const std::unordered_map<std::string, float>& config) override {
        if (config.count("learning_rate"))
            _learning_rate = config.at("learning_rate");
        if (config.count("momentum")) _momentum = config.at("momentum");
    }

    std::unique_ptr<Optimizer> clone( ) const override {
        return std::make_unique<SGDOptimizer>(_learning_rate, _momentum);
    }

    std::string get_name( ) const override { return "SGD"; }
};

class AdamOptimizer : public Optimizer {
private:
    float              _learning_rate = 0.001f;
    float              _beta1         = 0.9f;
    float              _beta2         = 0.999f;
    float              _epsilon       = 1e-8f;
    std::vector<float> _m;      // first moment
    std::vector<float> _v;      // second moment
    int                _t = 0;  // timestep

public:
    AdamOptimizer( ) = default;

    AdamOptimizer(float learning_rate, float beta1 = 0.9f, float beta2 = 0.999f,
                  float epsilon = 1e-8f)
        : _learning_rate(learning_rate),
          _beta1(beta1),
          _beta2(beta2),
          _epsilon(epsilon) {}

    void update(std::vector<float>&       params,
                const std::vector<float>& gradients) override {
        if (_m.empty( )) {
            _m.resize(params.size( ), 0.0f);
            _v.resize(params.size( ), 0.0f);
        }

        _t++;

        for (size_t i = 0; i < params.size( ); ++i) {
            _m[i] = _beta1 * _m[i] + (1 - _beta1) * gradients[i];
            _v[i] = _beta2 * _v[i] + (1 - _beta2) * gradients[i] * gradients[i];

            // bias-corrected moment estimates
            float m_hat = _m[i] / (1 - std::pow(_beta1, _t));
            float v_hat = _v[i] / (1 - std::pow(_beta2, _t));

            params[i] -= _learning_rate * m_hat / (std::sqrt(v_hat) + _epsilon);
        }
    }

    void configure(
        const std::unordered_map<std::string, float>& config) override {
        if (config.count("learning_rate"))
            _learning_rate = config.at("learning_rate");
        if (config.count("beta1")) _beta1 = config.at("beta1");
        if (config.count("beta2")) _beta2 = config.at("beta2");
        if (config.count("epsilon")) _epsilon = config.at("epsilon");
    }

    std::string get_name( ) const override { return "Adam"; }

    std::unique_ptr<Optimizer> clone( ) const override {
        return std::make_unique<AdamOptimizer>(_learning_rate, _beta1, _beta2,
                                               _epsilon);
    }
};

class RMSPropOptimizer : public Optimizer {
private:
    float              _learning_rate = 0.01f;
    float              _decay_rate    = 0.99f;
    float              _epsilon       = 1e-8f;
    std::vector<float> _square_avg;

public:
    RMSPropOptimizer( ) = default;
    RMSPropOptimizer(float learning_rate, float decay_rate = 0.99f,
                     float epsilon = 1e-8f)
        : _learning_rate(learning_rate),
          _decay_rate(decay_rate),
          _epsilon(epsilon) {}

    void update(std::vector<float>&       params,
                const std::vector<float>& gradients) override {
        if (_square_avg.empty( )) _square_avg.resize(params.size( ), 0.0f);

        for (size_t i = 0; i < params.size( ); ++i) {
            _square_avg[i] = _decay_rate * _square_avg[i] +
                             (1 - _decay_rate) * gradients[i] * gradients[i];
            params[i] -= _learning_rate * gradients[i] /
                         (std::sqrt(_square_avg[i]) + _epsilon);
        }
    }

    void configure(
        const std::unordered_map<std::string, float>& config) override {
        if (config.count("learning_rate"))
            _learning_rate = config.at("learning_rate");
        if (config.count("decay_rate")) _decay_rate = config.at("decay_rate");
        if (config.count("epsilon")) _epsilon = config.at("epsilon");
    }

    std::string get_name( ) const override { return "RMSProp"; }

    std::unique_ptr<Optimizer> clone( ) const override {
        return std::make_unique<RMSPropOptimizer>(_learning_rate, _decay_rate,
                                                  _epsilon);
    }
};

// the factory
class OptimizerFactory {
public:
    static std::unique_ptr<Optimizer> create_optimizer(
        const std::string&                            type,
        const std::unordered_map<std::string, float>& config = { }) {
        std::unique_ptr<Optimizer> optimizer;

        if (type == "sgd")
            optimizer = std::make_unique<SGDOptimizer>( );
        else if (type == "adam")
            optimizer = std::make_unique<AdamOptimizer>( );
        else if (type == "rmsprop")
            optimizer = std::make_unique<RMSPropOptimizer>( );
        else
            throw std::invalid_argument("Unknown optimizer type: " + type);

        optimizer->configure(config);
        return optimizer;
    }

    // here on we extend the factory to use Prototype pattern to clone existing
    // templates
    static std::unordered_map<std::string, std::unique_ptr<Optimizer>>
        _prototypes;

    static void register_optimizer(const std::string&         name,
                                   std::unique_ptr<Optimizer> prototype) {
        _prototypes[name] = std::move(prototype);
    }

    static std::unique_ptr<Optimizer> create_from_prototype(
        const std::string&                            name,
        const std::unordered_map<std::string, float>& config = { }) {
        // map `contains` works only in C++20 or later
        if (!_prototypes.contains(name)) {
            throw std::invalid_argument(
                "No optimizer prototype registered with name: " + name);
        }
        auto it        = _prototypes.find(name);
        auto optimizer = it->second->clone( );
        optimizer->configure(config);

        return optimizer;
    }
};

std::unordered_map<std::string, std::unique_ptr<Optimizer>>
    OptimizerFactory::_prototypes = { };
