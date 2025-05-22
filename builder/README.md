Builder Pattern
---

### Intent
Separate the construction of a complex object from its representation so that the same construction process can create different representations. The Builder pattern allows us to produce different types and representations of an object using the same construction code.

We should use this pattern when we face the following situation:

- Objects with many optional parameters (like HTTP requests)
- Complex construction that requires validation
- When you need different representations of the same object
- APIs where parameter order doesn't matter

We should NOT use this pattern when:

- Simple objects with few parameters
- Objects that change frequently after construction
- When performance is absolutely critical (has slight overhead)

### Structure
```
    ┌─────────────┐    creates    ┌─────────────────┐
    │   Client    │──────────────▶│ ConcreteBuilder │
    └─────────────┘               └─────────────────┘
           │                               │
           │ uses                          │ builds
           ▼                               ▼
    ┌─────────────┐    constructs   ┌─────────────┐
    │  Director   │────────────────▶│   Product   │
    │ (optional)  │                 │(HttpRequest)│
    └─────────────┘                 └─────────────┘
           │                                ▲
           │ uses builder                   │
           ▼                                │ returns
    ┌─────────────┐                         │
    │   Builder   │─────────────────────────┘
    │ (interface) │
    └─────────────┘
           ▲
           │ implements
           │
    ┌───────────────────┐
    │HttpRequestBuilder │ ◄─── ConcreteBuilder
    │ - add_header()    │
    │ - set_body()      │
    │ - build()         │
    └───────────────────┘
```

- `Client` creates `ConcreteBuilder` and calls construction methods
- `Director` (optional) knows which construction steps to execute
- `ConcreteBuilder` implements the construction interface
- Product is a "complex" object being constructed (immutable result)

This example demonstrates the Builder pattern through a practical HTTP request construction system. 
The pattern is essential when dealing with complex objects that have many optional parameters and 
configuration options.

**Benefits**:

1. Separates construction logic from representation
2. Allows step-by-step construction with validation
3. Supports both required and optional parameters elegantly  
4. Enables fluent interface for better readability
5. Provides flexibility in construction strategies

### Important Side Note
1. **Director Pattern Integration**: The `HttpRequestDirector` class shows how to build common request types
2. **Builder Safety Mechanisms**: Once `build()` is called, the builder becomes unusable; Prevents accidental modification of already-constructed objects; Clear error messages guide proper usage
3. **Immutable Product**: The final HttpRequest object cannot be modified; Thread-safe once constructed; Follows RAII principles for resource management
