#include "symbolt.h"
 
#ifndef	FUNCTIONS_H_
#define FUNCTIONS_H_


typedef enum opcode iopcode; 
typedef enum Expr_t expr_t; 
typedef struct expression expr;

enum opcode{
	assign, 	add,		sub,
	mul,		divide,		mod,
	uminus,		and,		or,
	not,		if_eq,		if_not_eq,
	if_lesseq,	if_greatereq,	if_less,
	if_greater,	call,		param,
	ret,		getretval,	funcstart,
	funcend,	jump,		tablecreate,	
	tablegetelem,	tablesetelem
};



enum Expr_t{
	var_e,
	tableitem_e,
	programfunc_e,
	libraryfunc_e,
	arithexpr_e,
	boolexpr_e,
	assignexpr_e,
	newtable_e,
	constint_e,
	constdouble_e,
	constbool_e,
	conststring_e,
	nil_e
};


struct expression{
	expr_t 	type;
	Symbol 	*sym;
	expr    *index;
	int 	intConst;
	double	numConst;
	char 	*strConst;
	unsigned char boolConst;
	expr	*next;
};


typedef struct Quad{
	iopcode	op;	
	expr *	arg1;
	expr *	arg2;
	expr *	result;
	int label;
	unsigned int line;
}quad;

typedef struct Node{   
	unsigned int info;
	struct Node *next;
}node;

typedef struct Break{
	unsigned int start;
	unsigned int jump;
	struct Break *next;
}specialJumpStruct;

typedef struct Offset{
	scopespace_t scopespace;
	struct Offset *next;
}offsetStruct;

void expand(void); //Kanei expand ton pinaka me ta quads.
void emit(iopcode,expr *, expr *, expr *,int, unsigned int); 
expr * emitrelop(iopcode op, expr * arg1, expr * arg2, unsigned int line);
expr * emitlogic(iopcode op, expr * arg1, expr * arg2, unsigned int line);
char *newtempname(void);// Dimiourgei kainourio onoma prosorinis metavlitis
void resettemp(void);	//Reset ton counter gia ta onomata
Symbol *newtemp(void);	//dimiourgei kainourio entry ston symbol table
expr *numConst_expr(int );
expr *strConst_expr(char *);
expr *numStrConst_expr(int , char * );

scopespace_t currscopespace( void );	//epistrefei to current scope space
unsigned int currscopeoffset( void );	//epistrefei to offset sto current scope space
void inccurrscopeoffset( void );	//afksanei to offset sto current scope space analoga me to ti eidous metavliti mpenei sto symbol table
void enterscopespace( void );		//afksanei to scopespace
void exitscopespace( void );		//meionei to scopespace
unsigned int nextquadlabel( void );    	//epistrefei to label tou epomenou quad
void resetformalargoffset( void );     	//midenizei to offset twn formal args
void resetfunctionlocaloffset( void );	//midenizei to offset twn function locals
expr * lvalue_expr(Symbol *);          	//dimiourgei ena lvalue expression kai to epistrefei ( se expr * )
void restorecurrscopeoffset( unsigned int ); //epanafora toy arxikou scope offset
expr* emit_iftableitem( expr* e,unsigned int line);     //i zitoumeni emit_iftableitem
expr *emit_table(expr *arg1,expr *arg2,int line);         //voithitiki table emit sinartisi gia stoixeio toy table-kalei tin emit_iftableitem
expr *func_call(expr *lval, expr *args, unsigned int line); //sinartisi i opoia diaxeirizetai ta emits gia to function call.
expr *createMember(expr* value, char* name, int line); //dimiourgei ena table member.
void backpatch(unsigned quadNumber,unsigned int label); //Sinartisi gia backpatching.


expr * constInt_expr(int num);
expr * constDouble_expr(double num);
expr * constString_expr(char * str);
expr * constBool_expr( unsigned char boole);
expr *newexpr_t(expr_t type);
expr *arith_emit(iopcode op, expr *arg1 , expr *arg2,int line);
int isFunction(Symbol * sym, char * type1, char * type2);


static char *print_expr(expr * expression);
void printquads();


/*Stacks/Lists:*/


//void emitwhile(int arg1, int arg2, node * break_list, node * continue_list, int line);

node * initStack();  //Arxikopoiisi stoivas
void pushStack(node *stack, unsigned int number); //Push sti stiva
node * popStack(node *stack); //Pop sti stoiva-den eleftherwnei ti mnimi!!
node * topStack(node *stack); //Pop sti stoiva-den eleftherwnei ti mnimi!!
int isEmptyStack(node *stack); //Elegxos an i stoiva einai adeia

node * initList(); //Arxikopoiisi listas
void insertList(node *lista, unsigned int number); //eisagwgi sti lista
node *	mergeLists(node *lista, node *lista2); //Sinenwsi 2 listwn
int isEmptyList(node *lista); //Elegxos an mia lista einai keni

specialJumpStruct * initSpecialJumpStack();
void pushSpecialJumpStack(specialJumpStruct *stack, unsigned int start, unsigned int jump);
specialJumpStruct * topSpecialJumpStack(specialJumpStruct *stack);
specialJumpStruct * popSpecialJumpStack(specialJumpStruct *stack);
int isEmptySpecialJumpStack(specialJumpStruct *stack);

offsetStruct * initOffsetStack();
void pushOffsetStack(offsetStruct *stack, scopespace_t space);
scopespace_t topOffsetStack(offsetStruct *strack);
scopespace_t popOffsetStack(offsetStruct *strack);
int isEmptyOffsetStack(offsetStruct *strack);

#endif 


