## README

### Summary
Xplice is my first ever register-bytecode-based language. The goal of this language is to become a familiar-looking, simple, and procedural utility language. See the below points for its design philosophy.

### General:
 - Familiar-ish syntax to TS and C++.
 - Static, checked type system.
 - Module-based? (TODO)
 - Native function support? (TODO)

### Philosophy:
 - Explicit behavior is better for understanding.
 - Concise and clear code helps maintainability.
 - There is only one proper way to do something.
 - It just works.

### More Docs:
 - [Grammar](./docs/grammar.md)
 - [VM](./docs/vm.md)

### Roadmap:
 - Complete emit pass for graph.
   - **TODO**: fix emit pass by building a queue of pending backpatches during traversal, and then apply all of them after traversal finishes.
 - Create VM and runtime wrappers...
   - **NOTE**: Unpack function arguments from position `top - argc` to the `top`.
 - Add support for imports of native functions!
 - Add while loop support.
 - Add array and tuple parsing support.
 - Add array and tuple codegen support.
 - Finally add string codegen support!
