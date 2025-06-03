### XLang Virtual Machine

#### High-Level Overview
 - Stack based virtual architecture with stack(s).
    - Special registers:
        - **R_IP** 2 bytes
        - **R_ERR** 1 byte: (see error codes)
    - A N-peekable stack of operands
    - A "heap" for _aggregate_ objects i.e strings, tuples, and arrays
    - A call frame stack & instruction store
 - Contains a periodic, pausing GC

#### Instructions
 - **HALT**
 - **NOOP**
 - **PUSH** value-id
 - **POP** pop-n
 - **PEEK** base-offset
 - **LOAD_CONST** const-id
 - **MAKE_ARRAY** descending-temps-n
 - **MAKE_TUPLE** descending-temps-n
 - **ACCESS_FIELD** object-id, key-id
 - **NEGATE**
 - **ADD**
 - **SUB**
 - **MUL**
 - **DIV**
 - **CMP_EQ**
 - **CMP_NOT_EQ**
 - **CMP_GT**
 - **CMP_LT**
 - **LOG_AND**
 - **LOG_OR**
 - **JUMP** offset-id
 - **JUMP_IF** offset-id
 - **JUMP_NOT_IF** offset-id
 - **ENTER**
    - Places a NULL placeholder on the stack similar to an EBP marker on x86.
 - **RET** result-location / result-id
    - Removes the stack frame's items on the stack until a placeholder NULL is reached (like the base pointer in ASM). Then replaces NULL with the result.
 - **CALL** func-id
 - **CALL_NATIVE** unit-id, func-id

#### Error Codes
 - **0** normal
 - **1** arithmetic error
 - **2** access error
 - **3** value stack error
 - **4** call stack error
 - **5** heap error
 - **6** memory exceeded error
 - **7** general error

### Bytecode Format:
 - Instruction:
   1. Arity prefix: 0, 1, 2, 3 (how many `Locator` arguments are present after the opcode)
   2. Opcode: 1 byte for a `VM::Opcode`.
   3. Arguments: An arity-prefix sized sequence of `Locator` arguments.
