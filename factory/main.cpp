#include <iostream>

#include "optimizer.h"

void optimizers_ex( ) {
    auto sgd = OptimizerFactory::create_optimizer(
        "sgd", {{"learning_rate", 0.01f}, {"momentum", 0.9f}});
    auto adam =
        OptimizerFactory::create_optimizer("adam", {{"learning_rate", 0.001f}});
    auto rmsprop = OptimizerFactory::create_optimizer(
        "rmsprop", {{"learning_rate", 0.005f}});

    std::cout << "Created optimizers:\n";
    std::cout << "- " << sgd->get_name( ) << "\n";
    std::cout << "- " << adam->get_name( ) << "\n";
    std::cout << "- " << rmsprop->get_name( ) << "\n";

    std::vector<float> params    = {1.0f, 2.0f, 3.0f};
    std::vector<float> gradients = {0.1f, 0.2f, 0.3f};

    std::cout << "\nUpdating parameters with " << sgd->get_name( ) << ":\n";
    std::cout << "Before: ";
    for (auto p : params) std::cout << p << " ";
    std::cout << "\n";

    sgd->update(params, gradients);

    std::cout << "After: ";
    for (auto p : params) std::cout << p << " ";
    std::cout << "\n";

    // example of prototype pattern integration
    // register a custom SGD configuration as a prototype
    auto custom_sgd = std::make_unique<SGDOptimizer>(0.1f, 0.95f);
    OptimizerFactory::register_optimizer("fast_sgd", std::move(custom_sgd));

    // create optimizer from prototype
    auto fast_sgd = OptimizerFactory::create_from_prototype("fast_sgd");
    std::cout << "\nCreated optimizer from prototype: " << fast_sgd->get_name( )
              << "\n";

    // create a custom optimizer class without modifying the factory
    class AdaGradOptimizer : public Optimizer {
    private:
        float              _learning_rate = 0.01f;
        float              _epsilon       = 1e-8f;
        std::vector<float> _cache;

    public:
        AdaGradOptimizer(float learning_rate = 0.01f, float epsilon = 1e-8f)
            : _learning_rate(learning_rate), _epsilon(epsilon) {}

        void update(std::vector<float>&       params,
                    const std::vector<float>& gradients) override {
            if (_cache.empty( )) {
                _cache.resize(params.size( ), 0.0f);
            }

            for (size_t i = 0; i < params.size( ); ++i) {
                _cache[i] += gradients[i] * gradients[i];
                params[i] -= _learning_rate * gradients[i] /
                             (std::sqrt(_cache[i]) + _epsilon);
            }
        }

        void configure(
            const std::unordered_map<std::string, float>& config) override {
            if (config.count("learning_rate"))
                _learning_rate = config.at("learning_rate");
            if (config.count("epsilon")) _epsilon = config.at("epsilon");
        }

        std::unique_ptr<Optimizer> clone( ) const override {
            return std::make_unique<AdaGradOptimizer>(_learning_rate, _epsilon);
        }

        std::string get_name( ) const override { return "AdaGrad"; }
    };

    // register our new optimizer type
    OptimizerFactory::register_optimizer("adagrad",
                                         std::make_unique<AdaGradOptimizer>( ));

    // now we can create instances of our new type without modifying
    // OptimizerFactory
    auto adagrad = OptimizerFactory::create_from_prototype(
        "adagrad", {{"learning_rate", 0.02f}});
    std::cout << "Created custom optimizer: " << adagrad->get_name( ) << "\n";
}

int main( ) {
    std::cout << "Creating various optimizers." << std::endl;
    optimizers_ex( );
    return 0;
}
