### XLang Virtual Machine

#### High-Level Overview
 - Register based virtual architecture with stack(s).
    - 16 general purpose registers
    - 4 special registers:
        - **R_IP** 2 bytes
        - **R_ERR** 1 byte: (see error codes)
        - **R_FLAG** 1 byte: 0 or 1 for F/T
        - **R_BASE** 2 bytes: call-frame-id
    - A temporary stack of objects.
    - A "heap" for complex objects.
    - A call frame stack & instruction storage.
 - Contains a periodic, pausing GC.

#### Instructions
 - **HALT**
 - **NOOP**
 - **PUSH_TEMP** src-id / src-reg
 - **POP_TEMP** pops-n
 - **PUT_CONST** dest-id / dest-reg, const-literal / const-id
 - **PUT_VALUE** dest-id / dest-reg, const-literal / value-id
 - **MAKE_ARRAY** dest-id / dest-reg, item-type-id, item-n
 - **MAKE_TUPLE** dest-id / dest-reg, tuple-type-id
 - **GET_SLOT** dest-id / dest-reg, obj-id, field-id
 - **SET_SLOT** dest-id / dest-reg, field-id, src-id / src-reg
 - **GET_ITEM** dest-id / dest-reg, src-tuple-id, tuple-index
 - **ADD_INT** dest-id / dest-reg, arg-0
 - **ADD_FLOAT** dest-id / dest-reg, arg-0
 - **SUB_INT** dest-id / dest-reg, arg-0
 - **SUB_FLOAT** dest-id / dest-reg, arg-0
 - **MUL_INT** dest-id / dest-reg, arg-0
 - **MUL_FLOAT** dest-id / dest-reg, arg-0
 - **DIV_INT** dest-id / dest-reg, arg-0
 - **DIV_FLOAT** dest-id / dest-reg, arg-0
 - **CMP_EQ** dest-id / dest-reg, dest-id / dest-reg
 - **CMP_NOT_EQ** dest-id / dest-reg, dest-id / dest-reg
 - **CMP_GT** dest-id / dest-reg, dest-id / dest-reg
 - **CMP_LT** dest-id / dest-reg, dest-id / dest-reg
 - **JUMP** offset-id
 - **JUMP_IF** offset-id
 - **CREATE_FRAME** func-id, slot-n
 - **LEAVE_FRAME** func-id, slot-n
 - **CALL** func-id
 - **CALL_NATIVE** unit-id, func-id

#### Error Codes
 - **0** normal
 - **1** arithmetic error
 - **2** access error
 - **3** temporary stack error
 - **4** call stack error
 - **5** heap error
 - **6** memory exceeded error
 - **7** general error
