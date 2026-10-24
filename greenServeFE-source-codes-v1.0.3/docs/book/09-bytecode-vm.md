# 9. Bytecode and the VM

A bytecode instruction is:

~~~c
typedef struct {
    uint8_t op;
    int32_t arg;
    int line;
} GSInstr;
~~~

A GSCode contains instructions, constants, names, child function code objects, and parameter metadata.

A VM frame contains a code object, instruction pointer, operand-stack base, and lexical environment.

Inspect execution with:

~~~bash
greenServeFE --dump test.gsve
greenServeFE --trace test.gsve
~~~

The native API contains a VM context and callback:

~~~c
void *context;
int (*call)(void *, GSValue, GSValue *, size_t, GSValue *);
~~~

This callback bridge permits the server module to invoke a GSVE route function while the VM remains responsible for execution.
