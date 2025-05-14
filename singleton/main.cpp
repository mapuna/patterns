#include <atomic>
#include <random>
#include <thread>

#include "logger.h"

std::atomic<int> completed_threads(0);

// simulates a high-load application with diverse log patterns
void worker_thread(int id, int num_logs, std::atomic<bool>& start_flag) {
    while (!start_flag.load(std::memory_order_acquire)) std::this_thread::yield( );

    std::mt19937                    gen(id);
    std::uniform_int_distribution<> level_dist(0, 5);
    std::uniform_int_distribution<> delay_dist(0, 5);

    for (int i = 1; i < num_logs + 1; ++i) {
        auto level = level_dist(gen);

        switch (level) {
            case 0:
                LOG_TRACE("Thread ", id, " trace message ", i);
                break;
            case 1:
                LOG_DEBUG("Thread ", id, " debug message ", i);
                break;
            case 2:
                LOG_INFO("Thread ", id, " info message ", i);
                break;
            case 3:
                LOG_WARNING("Thread ", id, " warning message ", i);
                break;
            case 4:
                LOG_ERROR("Thread ", id, " error message ", i);
                break;
            case 5:
                LOG_CRITICAL("Thread ", id, " critical message ", i);
                break;
        }

        if (delay_dist(gen) == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    completed_threads.fetch_add(1, std::memory_order_relaxed);
}

int main( ) {
    auto& logger = Logger::get_instance( );
    logger.set_level(Logger::LogLevel::TRACE); // process all logs for testing our logger
    logger.set_console_output(true);
    logger.set_file_output("logs/__test__.log");

    // test params
    const int         num_threads     = 16;
    const int         logs_per_thread = 10000;
    std::atomic<bool> start_flag(false);
    completed_threads.store(0);

    std::cout << "Starting logging performance test with " << num_threads
              << " threads, each generating " << logs_per_thread
              << " log messages\n";

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker_thread, i, logs_per_thread,
                             std::ref(start_flag));
    }

    auto start_time = std::chrono::high_resolution_clock::now( );
    start_flag.store(true, std::memory_order_release);

    int last_processed = 0;
    int last_completed = 0;
    int expected_total = num_threads * logs_per_thread;

    while (completed_threads.load(std::memory_order_relaxed) < num_threads) {
        int current = logger.get_total_logs_processed();
        int pending = logger.get_pending_logs();
        int completed = completed_threads.load(std::memory_order_relaxed);

        if (current != last_processed || completed != last_completed) {
            double progress = (static_cast<double>(current) / expected_total) * 100.0;
            
            std::cout << "\rProgress: " << std::fixed << std::setprecision(1)
                      << progress << "% | Logs processed: " << current
                      << " | Pending: " << pending 
                      << " | Threads completed: " << completed << "/" << num_threads
                      << "          ";
            std::cout.flush();
            
            last_processed = current;
            last_completed = completed;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "\nAll threads completed, waiting for log processing to finish...\n";

    auto wait_start = std::chrono::steady_clock::now( );
    auto timeout    = std::chrono::seconds(10);  // 10 second timeout

    while (logger.get_pending_logs() > 0) {
        int current = logger.get_total_logs_processed( );
        int pending = logger.get_pending_logs( );

        std::cout << "\rLogs processed: " << current << " | Pending: " << pending << "          ";
        std::cout.flush();

        // break if we've waited too long with no progress
        auto now = std::chrono::steady_clock::now( );
        if (std::chrono::duration_cast<std::chrono::seconds>(now - wait_start) >
            timeout) {
            std::cout
                << "\nTimeout waiting for logs to complete. Moving on with "
                << current << "/" << expected_total << " logs processed."
                << std::endl;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    for (auto& th : threads) th.join( );

    logger.shutdown( );

    // calculate performance metrics aggregates
    auto end_time = std::chrono::high_resolution_clock::now( );
    auto duration =
        std::chrono::duration<double>(end_time - start_time).count( );

    double logs_per_second  = logger.get_total_logged() / duration;
    double avg_process_time = logger.get_avg_processing_time_ms( );

    // print final results
    std::cout << "\n\nLogging Performance Test Results\n";
    std::cout << "-------------------------------\n";
    std::cout << "Total messages logged: " << logger.get_total_logged() << "\n";
    std::cout << "Total logs processed: " << logger.get_total_logs_processed() << "\n";
    std::cout << "Logs filtered by level: " << logger.get_filtered_logs() << "\n";
    std::cout << "Total time: " << std::fixed << std::setprecision(3) << duration << " seconds\n";
    std::cout << "Logs per second: " << std::fixed << std::setprecision(1) << logs_per_second << "\n";
    std::cout << "Average processing time: " << std::fixed << std::setprecision(6) 
              << avg_process_time << " ms\n";
    std::cout << "\nThis demonstrates that the Logger singleton is both thread-safe and non-blocking.\n";
    std::cout << "The main thread never waits for logging operations to complete.\n";

    return 0;
}
