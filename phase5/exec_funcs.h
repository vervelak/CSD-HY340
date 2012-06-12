#ifndef EXEC_FUNCS_H_
#define EXEC_FUNCS_H_

#include "structs.h"

#define AVM_NUMACTUALS_OFFSET 4
#define AVM_SAVEDPC_OFFSET 3
#define AVM_SAVEDTOP_OFFSET 2
#define AVM_SAVEDTOPSP_OFFSET 1


void execute_call(instruction *instr);
void avm_callsaveenviroment(void);
void avm_push_envvalue(unsigned int val);
void avm_dec_top(void);
void avm_calllibfunc(char* id);
Symbol *avm_getlibraryfunc(char *id);
void execute_funcexit(instruction *unused);
void execute_funcenter(instruction *instr);
Symbol *avm_getfuncinfo(unsigned int address);
unsigned int avm_get_envvalue(unsigned int i);
unsigned avm_totalactuals();
avm_memcell *avm_getactual(unsigned i);
void execute_pusharg(instruction* instr);
char* all_tostring( avm_memcell* m );

void execute_ret(instruction *inst);
void execute_getretval(instruction *instr);


typedef char* (*tostring_func_t)(avm_memcell* );


//****LIBRARY FUNCTIONS****



void libfunc_print();
void libfunc_typeof();
void libfunc_totalarguments();
void libfunc_argument();
void libfunc_input();




#endif
