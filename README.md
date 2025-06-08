## README

### Summary
Xplice is my first ever stack-bytecode-based language. The goal of this language is to become a familiar-looking, simple, and procedural utility language. See the below points for its design philosophy.

### General:
 - Familiar-ish syntax to TS and C++.
 - Static, strong type system.
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
 - Fix bug where local variables passed to calls evaluate in a wrong order.
   - Redo codegen to track state "push/pop scoring" so that local slots are precisely tracked in compilation... Fix VM as needed!
 - Add semantic analysis for:
   - Name-defined checks
   - Simple type checks for expressions
 - Add native function support!
 - Add while loop support.
 - Add support for imports of native functions!
 - Add array and tuple parsing support.
 - Add array and tuple codegen support.
 - Finally add string codegen support!
