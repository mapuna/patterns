PIMPL Idiom
---

### Intent

Hide implementation details in `.cpp` file so that changes to implementation do not force recompilation 
of clients if the API is fixed.

### Structure

```
+---------------+         +------------------+
|   HTTPClient  |<>──────>| HTTPClient::Impl |
|    (public)   |         |     (private)    |
+---------------+         +------------------+
| get(...)      |         | ...              |
| post(...)     |         |                  |
+---------------+         +------------------+

```
