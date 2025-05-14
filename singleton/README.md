Singleton Pattern
---

### Intent
Ensure that a class has only one instance and provides a global point of access to it. This pattern is useful when exactly one object is needed to coordinate actions across a system, such as a configuration manager, connection pool, or logger.

This implementation uses Scott Meyer's Singleton approach (local `static` variable) which is guaranteed to be thread-safe since C++11.

### Structure
```
  ┌───────────────────────────────────┐
  │            Logger                 │
  ├───────────────────────────────────┤
  │ -Logger()                         │◄── Private constructor 
  │ -_log_queue: concurrentqueue      │
  │ -_worker_thread: std::thread      │
  │ -_current_level: std::atomic      │
  │ ...                               │
  ├───────────────────────────────────┤
  │ +get_instance(): Logger&          │◄── Thread-safe static method
  │ +trace/debug/info/warning/error/  │    returns singleton
  │  critical(): void                 │
  │ +set_level(LogLevel): void        │
  │ +set_file_output(filename): void  │
  │ +set_console_output(bool): void   │
  │ +shutdown(): void                 │
  └───────────────────────────────────┘
                    │
                    │ uses
                    ▼
  ┌───────────────────────────────────┐
  │         LogEntry                  │
  ├───────────────────────────────────┤
  │ +level: LogLevel                  │
  │ +message: std::string             │
  │ +timestamp: time_point            │
  └───────────────────────────────────┘
```

### Note 
I have built a cutom logger that is not as high-performance as `spdlog`, but has minimal locking and less than a day's work for me... It is meant just as an example to show the `Singleton` idea to the freshers and new joinees in my team.

- Used `moodycamel::concurrentqueue` -- since it is a non-blocking, lockfree queue implementation.
