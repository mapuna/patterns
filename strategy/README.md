Strategy Pattern
---

### Intent

Define a family of algorithms (strategies), encapsulate each, and make them interchangeable via a common interface.

### Structure

```
+------------------+                     +----------------------+
|      Context     |<>──────────────────>|     Strategy         |
|       (Tree)     |                     | (TraversalStrategy)  |
+------------------+                     +----------------------+
| set_strategy(...)|                     | Algo - traverse(...) |
+------------------+                     +----------------------+
       ▲      ▲
       |      |──────────────|
       |                     |
+--------------------+       +-----------------------+
|  InOrderTraversal  | ...   |  LevelOrderTraversal  |
+--------------------+       +-----------------------+
```
