## README

### Summary
Xplice is my first ever stack-bytecode-based language. The goal of this language is to become a familiar-looking, simple, and procedural utility language. See the below points for its design philosophy.

### Philosophy:
 - Explicit behavior is better for understanding.
    - The code clearly expresses what it does.
 - Feature creep is bad for traction.

### More Docs:
 - [Grammar](./docs/grammar.md)
 - [VM](./docs/vm.md)

### Roadmap of Changes:
 - **0.1.0** Created the initial bytecode interpreter. Runs Fibonacci.
 - **0.2.0** Added some semantic checking to the bytecode compiler.
 - **0.3.0** Some support for native functions added.
 - **0.4.0** While loops somewhat added.
 - **0.5.0** Add string types.
 - **0.5.1?** Add array and tuple types.
 - **0.6.0?** Add a stop-the-world GC.
 - **0.7.0?** Add module support.
 - **0.7.1?** Improve parser error messages.
 - **0.7.2?** Add `const` checking of variables.
 - **0.7.3?** Add an optimization pass for pruning dead branches and useless expressions.
 - More versions will be planned...
