## README

### Summary
Xplice is my first ever stack-bytecode-based language. The goal of this language is to become a familiar-looking, simple, and procedural utility language. See the below points for its design philosophy.

### Philosophy:
 - Explicit behavior is better for understanding.
 - Feature creep is bad for traction.
 - Concise and clear code helps its maintainability.

### More Docs:
 - [Grammar](./docs/grammar.md)
 - [VM](./docs/vm.md)

### Roadmap of Changes:
 - **0.1.0** Created the initial bytecode interpreter. Runs Fibonacci.
 - **0.2.0** Added some semantic checking to the bytecode compiler.
 - **0.3.0** Some support for native functions added.
 - **0.4.0?** While loops will be added after some IR overhauls.
 - **0.5.0?** Two basic optimization passes will be added.
 - **0.6.0?** Sequential types (array, tuple) will be added with a mark-and-sweep GC.
 - **0.7.0?** Add actual strings.
 - More versions will be planned...
