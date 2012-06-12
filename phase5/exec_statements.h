#ifndef EXEC_STATEMENTS_H_
#define EXEC_STATEMENTS_H_

typedef int (*statements_func_t)(double , double );
typedef unsigned char (* tobool_func_t) (avm_memcell *);



int jle_stmt(double , double );
int jge_stmt(double , double );
int jlt_stmt(double , double );
int jgt_stmt(double , double );


void execute_statement (instruction* instr);
void execute_jeq(instruction* instr);
void execute_jne(instruction* instr);
void execute_jlt(instruction* instr);
void execute_jgt(instruction* instr);

void execute_jle(instruction* instr);
void execute_jge(instruction* instr);
void execute_jeq(instruction* instr);
void execute_jne(instruction* instr);

void execute_jump(instruction* instr);

unsigned char number_tobool (avm_memcell* m);

unsigned char string_tobool (avm_memcell* m);

unsigned char bool_tobool (avm_memcell* m);

unsigned char table_tobool (avm_memcell* m);

unsigned char userfunc_tobool (avm_memcell* m);

unsigned char libfunc_tobool (avm_memcell* m);

unsigned char nil_tobool (avm_memcell* m);
	
unsigned char undef_tobool (avm_memcell* m);

unsigned char avm_tobool(avm_memcell* m);

#endif
