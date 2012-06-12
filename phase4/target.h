#ifndef TARGET_H_
#define TARGET_H_

#include <stdio.h>

#define CURR_VMARG_SIZE (totalVmargs*sizeof(instruction))
#define NEW_VMARG_SIZE (EXPAND_SIZE * sizeof(instruction)+CURR_VMARG_SIZE)

typedef void (*generator_func_t)(quad* );
typedef enum Vmopcode vmopcode;
typedef struct Instruction instruction;
typedef enum Vmarg_t vmarg_t;
typedef struct Vmarg_p vmarg;
//XXX Vervelak 
extern quad *quads;
//XXX ./Vervelak


enum Vmopcode{
	assign_v = 0 , 		add_v = 1,		sub_v = 2,
	mul_v = 3,		div_v = 4,		mod_v = 5,
	uminus_v = 6 ,		and_v = 7 ,		or_v = 8 ,
	not_v = 9 ,		jeq_v = 10,		jne_v = 11,
	jle_v = 12,		jge_v = 13,		jlt_v =14,
	jgt_v = 15,		call_v = 16,		pusharg_v = 17,
	ret_v = 18,		getretval_v = 19,	funcenter_v = 20,
	funcexit_v = 21,	jump_v = 22,		tablecreate_v = 23,	
	tablegetelem_v = 24,	tablesetelem_v = 25, 	nop_v = 26
};


struct Instruction{
	vmopcode opcode;
	vmarg *result;
	vmarg *arg1;
	vmarg *arg2;
	unsigned int srcLine;

};

enum Vmarg_t{
   	label_a = 0,
   	global_a = 1,
   	formal_a = 2,
   	local_a = 3,
   	integer_a = 4,
   	double_a = 5,
   	string_a = 6,
   	bool_a = 7,
   	nil_a = 8,
   	userfunc_a = 9,
   	libfunc_a = 10,
   	retval_a = 11
};


struct Vmarg_p{
   vmarg_t type;
   unsigned int val;
};

struct incomplete_jump{
	unsigned int instrNo;	
	unsigned int iaddress;		
	struct incomplete_jump *next;		
};
//XXX Vervelak


typedef struct incJump{
unsigned instrAddress;
unsigned instrNo;
struct incJump *next;
}incJump; 

typedef struct Rlist{
	unsigned int rvalue;
	struct Rlist *next;
}rlist;

typedef struct Funcs{
	Symbol *sym;
	rlist *nodeR;
	unsigned line;
	struct Funcs *next;
}funcs;

extern quad *quads;
extern generator_func_t generators[];

//XXX /Vervelak
void expandVmargsTable();
void emitVmarg (instruction * );
void make_operand( expr * , vmarg * );
int nextInstructionLabel();
void generate( vmopcode ,quad * );

//XXX Vervelak
unsigned int currProcessedQuad();

instruction *InitInstructtion( void );
void addIncJump( unsigned instrAddress, unsigned instrNo );
void patchIncJump();
void generate_relational(vmopcode op,quad *quad);
static void patchReturnList(rlist *Rlist,unsigned int label);
void pushSym( Symbol *func );
funcs *topSym();
funcs *popSym();
rlist *appendR( rlist *rHead, unsigned int retLabel );

extern void generate_ADD ( quad* );
extern void generate_SUB ( quad* );
extern void generate_MUL ( quad* );
extern void generate_DIV ( quad* );
extern void generate_MOD ( quad* );
extern void generate_TABLECREATE ( quad* );
extern void generate_TABLEGETELEM ( quad* );
extern void generate_TABLESETELEM ( quad* );
extern void generate_ASSIGN ( quad* );
extern void generate_NOP ();
extern void generate_JUMP ( quad* );
extern void generate_IF_EQ ( quad* );
extern void generate_IF_NOTEQ ( quad* );
extern void generate_IF_GREATER ( quad* );
extern void generate_IF_GREATEREQ ( quad* );
extern void generate_IF_LESS ( quad* );
extern void generate_IF_LESSEQ ( quad* );
extern void generate_UMINUS ( quad* );
extern void generate_NOT ( quad* );
extern void generate_AND ( quad* );
extern void generate_OR ( quad* );
extern void generate_PARAM ( quad* );
extern void generate_CALL ( quad* );
extern void generate_GETRETVAL ( quad* );

extern void generate_FUNCSTART ( quad* );
extern void generate_RETURN ( quad* );
extern void generate_FUNCEND ( quad* );

void make_booloperand( vmarg* , unsigned int  );
void make_retvaloperand( vmarg*  );

unsigned int consts_newstring( char *s );
unsigned int consts_newnumber( double n );
unsigned int libfuncs_newused( char *s );
unsigned int userfuncs_newfunc( Symbol *sym );

int nextInstructionLabel();


instruction *InitInstruction();
void printConsts( FILE *fp );
void printLibFs( FILE *fp );
void printUserFs( FILE *fp );
void vmType(vmarg *arg,FILE *fp);
void printBin();
void printFun();
void generation(void);


#endif

