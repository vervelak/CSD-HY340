#ifndef AVM_H_
#define AVM_H_

#include "structs.h"

#define AVM_STACKENV_SIZE	4
#define AVM_STACKSIZE 4096
#define AVM_ENDING_PC codeSize
#define AVM_WIPEOUT(m) memset(&(m),0,sizeof(m))

typedef enum avm_memcell_T{
	number_m	=0,
	string_m	=1,
	bool_m		=2,
	table_m		=3,
	userfunc_m	=4,
	libfunc_m	=5,
	nil_m		=6,
	undef_m		=7
}avm_memcell_t;

typedef struct avm_Memcell {
   avm_memcell_t type ;
   union {
      double numVal;
      int intVal;
      char *strVal;
      unsigned char boolVal;
      struct avm_table *tableVal;
      unsigned int funcVal;
      char *libfuncVal;   
   }data;
}avm_memcell;

void execute_nop(instruction* instr);




avm_memcell ax, bx, cx;
avm_memcell retval;
unsigned int top, topsp;


void memclear_table(avm_memcell *);
void memclear_string(avm_memcell *);

double 	 consts_getnumber (unsigned int index);
char 	*consts_getstring (unsigned int index);
char 	*libfuncs_getused (unsigned int index);

avm_memcell*  avm_translate_operand( vmarg* arg, avm_memcell* reg);
void avm_memcellclear (avm_memcell* m);
void avm_error(char* msg);
void avm_warning(char* msg);
void readBin(void);
static void avm_StackInit(void);


#endif
