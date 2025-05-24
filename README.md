## README

### Summary
X Lang is my first ever register-bytecode-based language. The goal of this language is to become a great improvement in syntax and ergonomics over some feature creeping languages. See the below points for its design philosophy.

### General:
 - Familiar-ish syntax to TS and C++.
 - Strong, static type system.
 - Module-based. (TODO)
 - Support of metaprogramming. (TODO)

### Philosophy:
 - Explicit behavior is better for understanding.
 - Concise and clear code helps maintainability.
 - There is only one proper way to do something.
 - It just works.

### More Docs:
 - [Grammar](./docs/grammar.md)
 - [VM](./docs/vm.md)

### Roadmap:
 - Complete control-flow graph pass for AST.
   - Only consider "primitive" types i.e bool, int, float
 - Complete emit pass for graph.
 - Create VM and runtime wrappers...
   - **NOTE**: Unpack function arguments from position `top - argc` to the `top`.
 - Add while loop support.
 - Add array and tuple parsing support.
 - Add array and tuple codegen support.
 - Finally add string codegen support!
