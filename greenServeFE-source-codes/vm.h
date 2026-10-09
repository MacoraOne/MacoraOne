#ifndef GSFE_VM_H
#define GSFE_VM_H
#include "bytecode.h"
typedef struct GSVM GSVM;GSVM*vm_new(void);void vm_free(GSVM*);int vm_run(GSVM*,GSCode*);void vm_set_trace(GSVM*,int);void vm_set_source(GSVM*,const char*,const char*);int vm_runtime_errors(const GSVM*);
#endif
