## README

### Summary
Xplice is my first ever stack-bytecode-based language. The goal of this language is to become a familiar-looking, simple, and procedural utility language. See the below points for its design philosophy.

### Philosophy:
 - Explicit behavior is better for understanding.
 - Concise and clear code helps maintainability.
 - It just works.

### More Docs:
 - [Grammar](./docs/grammar.md)
 - [VM](./docs/vm.md)

### Roadmap:
 - Add native function support!
 - Add while loop support.
 - Add optimization passes for:
   - Dead branches in control flow.
   - Peephole optimizing by replacing "useless" instructions.
 - Add support for imports of native functions!
 - Add array and tuple parsing support.
 - Add array and tuple codegen support.
 - Finally add string codegen support!

### Changes:
 - **0.2.0** Added some semantic checking to the bytecode compiler.
