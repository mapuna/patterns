Factory Pattern
---

### Intent
Provides an interface for creating objects without specifying their concrete classes. It centralizes object creation logic in a separate component, allowing client code to focus on using objects rather than creating them.

We should use this pattern when:

- We don't know ahead of time which class you need to instantiate
- We want to localize/centralize creation logic
- Classes require complex setup or configuration
- We need to create different objects based on environment or parameters
- We want to hide initialization details from client code
- The creation of an object requires making design decisions

### Structure

The following structure is specific to the example (implementing various optimizers for gradient descent), but 
should give the core idea:

```
┌──────────────────┐                       ┌──────────────────┐
│                  │                       │                  │
│ OptimizerFactory │<─────creates────      │    Optimizer     │ (Interface)
│                  │                       │                  │
└─────────┬────────┘                       └─────────┬────────┘
          │                                          │
          │                                          │
          │                                          │ implements
          │                             ┌────────────┴───────────┐
          │                             │                        │
          │      ┌─────────────────────>│     SGDOptimizer       │
          │      │                      │                        │
          │      │                      └────────────────────────┘
          │      │
          │      │                      ┌────────────────────────┐
          │      │                      │                        │
creates   │      └─────────────────────>│     AdamOptimizer      │
          │      │                      │                        │
          │      │                      └────────────────────────┘
          │      │
          │      │                      ┌────────────────────────┐
          │      │                      │                        │
          │      └─────────────────────>│   RMSPropOptimizer     │
          │                             │                        │
          │                             └────────────────────────┘
          │
          │                             ┌────────────────────────┐
          │                             │                        │
          └────────────────────────────>│  Custom Optimizers     │
                                        │  (added via registry)  │
                                        └────────────────────────┘
```

- `Optimizer` defines the interface for all optimizer types.
- `SGDOptimizer`, `AdamOptimizer`, and `RMSPropOptimizer` implement the Optimizer interface.
- **Factory**: `OptimizerFactory` provides the static factory method `create_optimizer()` that creates and returns instances of different optimizer types.
- Extended Factory Pattern: This implementation includes a registry mechanism that combines the Factory Method with the **Prototype pattern**, allowing new optimizer types to be registered dynamically without modifying the factory code.
