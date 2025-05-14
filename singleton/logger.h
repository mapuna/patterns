#ifndef LOGGER_H
#define LOGGER_H

#include <concurrentqueue/concurrentqueue.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <ratio>
#include <sstream>
#include <unordered_map>

class Logger {
public:
    enum class LogLevel { TRACE, DEBUG, INFO, WARNING, ERROR, CRITICAL, OFF };

    // returns the created instance
    static Logger& get_instance( ) {
        static Logger instance;
        return instance;
    }

    void set_level(LogLevel level) {
        _current_level.store(level, std::memory_order_relaxed);
    }

    void set_file_output(const std::string& filename) {
        std::lock_guard<std::mutex> lock(_config_mutex);
        _log_file.open(filename, std::ios::app);
        _log_to_file = _log_file.is_open( );
    }

    void set_console_output(bool enabled) {
        std::lock_guard<std::mutex> lock(_config_mutex);
        _log_to_console = enabled;
    }

    // logging methods
    template <typename... Args>
    void trace(const char* format, Args&&... args) {
        log(LogLevel::TRACE, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(const char* format, Args&&... args) {
        log(LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(const char* format, Args&&... args) {
        log(LogLevel::INFO, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warning(const char* format, Args&&... args) {
        log(LogLevel::WARNING, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(const char* format, Args&&... args) {
        log(LogLevel::ERROR, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(const char* format, Args&&... args) {
        log(LogLevel::CRITICAL, format, std::forward<Args>(args)...);
    }

    // performance metrics
    void count_logged_messages( ) {
        _total_logged.fetch_add(1, std::memory_order_relaxed);
    }

    size_t get_pending_logs( ) const { return _log_queue.size_approx( ); }

    size_t get_total_logs_processed( ) const {
        return _logs_processed.load(std::memory_order_relaxed);
    }

    size_t get_total_logged( ) const {
        return _total_logged.load(std::memory_order_relaxed);
    }

    size_t get_filtered_logs( ) const {
        return _filtered_logs.load(std::memory_order_relaxed);
    }

    double get_avg_processing_time_ms( ) const {
        if (_logs_processed.load( ) == 0) return 0.0;
        return _total_processing_time_ms.load( ) /
               static_cast<double>(_logs_processed.load( ));
    }

    void shutdown( ) {
        _running = false;
        _condition.notify_all( );
        if (_worker_thread.joinable( )) {
            _worker_thread.join( );
        }

        // Final dump of queue state for debugging
        std::cout << "\nLogger shutdown. Queue state: "
                  << "Total logged: " << _total_logged
                  << ", Processed: " << _logs_processed
                  << ", Filtered: " << _filtered_logs
                  << ", Pending: " << get_pending_logs( ) << std::endl;
    }

private:
    struct LogEntry {
        LogLevel                              level;
        std::string                           message;
        std::chrono::system_clock::time_point timestamp;

        LogEntry(LogLevel lvl, const std::string& msg,
                 std::chrono::system_clock::time_point ts)
            : level(lvl), message(msg), timestamp(ts) {}
    };

    std::atomic<LogLevel> _current_level;
    std::atomic<bool>     _running;
    std::atomic<size_t>   _logs_processed;
    std::atomic<size_t>   _total_logged;
    std::atomic<size_t>   _filtered_logs;
    std::atomic<double>   _total_processing_time_ms;

    bool          _log_to_console;
    bool          _log_to_file;
    std::ofstream _log_file;

    moodycamel::ConcurrentQueue<std::unique_ptr<LogEntry>> _log_queue;
    std::thread                                            _worker_thread;
    std::condition_variable                                _condition;
    std::mutex                                             _mutex;
    std::mutex                                             _config_mutex;

    // our logger is a singleton -- so a hidden  constructor
    Logger( )
        : _current_level(LogLevel::INFO),
          _log_to_console(true),
          _log_to_file(false),
          _running(true),
          _logs_processed(0),
          _total_processing_time_ms(0.0) {
        _worker_thread = std::thread(&Logger::process_log_queue, this);
    }

    ~Logger( ) { shutdown( ); }

    // copy and move constructors must be disabled
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(Logger&&)      = delete;

    template <typename... Args>
    std::string format_string(const char* format, Args&&... args) {
        std::ostringstream oss;
        oss << format;
        (oss << ... << std::forward<Args>(args));
        return oss.str( );
    }

    template <typename... Args>
    void log(LogLevel level, const char* format, Args&&... args) {
        _total_logged.fetch_add(1, std::memory_order_relaxed);

        // filter by level
        if (level < _current_level.load(std::memory_order_relaxed)) {
            _filtered_logs.fetch_add(1, std::memory_order_relaxed);
            return;
        }

        std::string formatted_message =
            format_string(format, std::forward<Args>(args)...);

        auto now = std::chrono::system_clock::now( );
        auto log_entry =
            std::make_unique<LogEntry>(level, formatted_message, now);

        _log_queue.enqueue(std::move(log_entry));
        _condition.notify_one( );
    }

    void process_log_queue( ) {
        const size_t BATCH_SIZE = 128;

        while (_running || _log_queue.size_approx( ) > 0) {
            bool have_logs = false;

            // wait for remaining logs or shutdown
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _condition.wait(lock, [this] {
                    return !_running || _log_queue.size_approx( ) > 0;
                });

                have_logs = _log_queue.size_approx( ) > 0;
            }

            if (have_logs) {
                size_t                    count = 0;
                std::unique_ptr<LogEntry> entry;

                auto start_time = std::chrono::high_resolution_clock::now( );

                while (count < BATCH_SIZE && _log_queue.try_dequeue(entry)) {
                    if (entry) {
                        write_log_entry(*entry);
                        _logs_processed.fetch_add(1, std::memory_order_relaxed);
                        count++;
                    }
                }

                if (count > 0) {
                    auto end_time = std::chrono::high_resolution_clock::now( );
                    auto duration = std::chrono::duration<double, std::milli>(
                                        end_time - start_time)
                                        .count( );
                    _total_processing_time_ms += duration;
                }
            }
        }
    }

    static const char* level_to_string(LogLevel level) {
        static const std::unordered_map<LogLevel, const char*> level_strings = {
            {LogLevel::TRACE, "TRACE"}, {LogLevel::DEBUG, "DEBUG"},
            {LogLevel::INFO, "INFO"},   {LogLevel::WARNING, "WARNING"},
            {LogLevel::ERROR, "ERROR"}, {LogLevel::CRITICAL, "CRITICAL"}};

        auto it = level_strings.find(level);
        return (it != level_strings.end( )) ? it->second : "UNKNOWN";
    }

    static std::string format_timestamp(
        std::chrono::system_clock::time_point time) {
        auto in_time_t = std::chrono::system_clock::to_time_t(time);
        auto ms        = std::chrono::duration_cast<std::chrono::milliseconds>(
            time.time_since_epoch( ) % std::chrono::seconds(1));

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count( );

        return ss.str( );
    }

    void write_log_entry(const LogEntry& entry) {
        std::ostringstream log_stream;
        log_stream << "[" << format_timestamp(entry.timestamp) << "] " << "["
                   << level_to_string(entry.level) << "] " << entry.message
                   << std::endl;
        std::string formatted_log = log_stream.str( );

        std::lock_guard<std::mutex> lock(_config_mutex);

        if (_log_to_console) std::cout << formatted_log;

        if (_log_to_file && _log_file.is_open( )) {
            _log_file << formatted_log;
            _log_file.flush( );
        }
    }
};

#define LOG_TRACE(...)    Logger::get_instance( ).trace(__VA_ARGS__)
#define LOG_DEBUG(...)    Logger::get_instance( ).debug(__VA_ARGS__)
#define LOG_INFO(...)     Logger::get_instance( ).info(__VA_ARGS__)
#define LOG_WARNING(...)  Logger::get_instance( ).warning(__VA_ARGS__)
#define LOG_ERROR(...)    Logger::get_instance( ).error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::get_instance( ).critical(__VA_ARGS__)

#endif  // LOGGER_H
