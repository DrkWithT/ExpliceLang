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
 - **RET** result-location / result-id
    - Removes the stack frame's items on the stack until the temporary callee ref., and it then replaces it with the result. Then sets IP to the return address from the call frame.
 - **CALL** func-id, args-n
    - Places function ref. on the stack and creates a call frame with the return address (location in caller) and an arg-list of all pushed param. values.
 - **CALL_NATIVE** unit-id, func-id, args-n

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

### Sample Bytecode Dump (Test 3b - Fibonacci)
```
Function Chunk (main):

0: load_const consts:0 
6: call routines:0 none:1 
17: push stack:0 
23: load_const consts:1 
29: cmp_ne 
30: jump_not_if none:48 
36: load_const consts:2 
42: ret consts:2 
48: jump none:54 
54: noop 
55: load_const consts:3 
61: ret consts:3 

Function Chunk 0:

0: push frame_slot:0 
6: load_const consts:0 
12: cmp_eq 
13: push frame_slot:0 
19: load_const consts:1 
25: cmp_eq 
26: log_or 
27: jump_not_if none:45 
33: load_const consts:1 
39: ret consts:1 
45: jump none:51 
51: noop 
52: load_const consts:1 
58: push frame_slot:0 
64: sub 
65: call routines:0 none:1 
76: load_const consts:2 
82: push frame_slot:0 
88: sub 
89: call routines:0 none:1 
100: add 
101: ret none:-1
```
