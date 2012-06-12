#ifndef STRUCTS_H_
#define STRUCTS_H_

#define execute_jle execute_statement
#define execute_jge execute_statement
#define execute_jlt execute_statement
#define execute_jgt execute_statement


typedef enum Vmopcode vmopcode;
typedef struct Instruction instruction;
typedef enum Vmarg_t vmarg_t;
typedef struct Vmarg_p vmarg;



enum Vmopcode{
	assign_v = 0 ,
	add_v = 1,
	sub_v = 2,
	mul_v = 3,
	div_v = 4,
	mod_v = 5,
	uminus_v = 6 ,
	and_v = 7 ,
	or_v = 8 ,
	not_v = 9 ,
	jeq_v = 10,
	jne_v = 11,
	jle_v = 12,
	jge_v = 13,
	jlt_v =14,
        jgt_v = 15,
	call_v = 16,
	pusharg_v = 17,
	ret_v = 18,
	getretval_v = 19,
	funcenter_v = 20,
	funcexit_v = 21,
	jump_v = 22,
	tablecreate_v = 23,
	tablegetelem_v = 24,
	tablesetelem_v = 25,
	nop_v = 26
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
        number_a = 5,
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

typedef struct  symT{
        char    *name; //onoma tis metavlitis
//        int     scope;  //scope tis metavlitis
        int     line;   //grammi tis metavlitis-stis sinartiseis xrisimeyei gia na metraei tis localvars
//        char     *nameF; //onoma tis function pou vrisketai i metavliti
//        int     scopeF;  //scope tis function pou vrisketai i metavliti
//        int     lineF;   //line tis function pou vrisketai i metavliti
//        char     *type;  //tipos tou node (an einai func , lib_func , var, formarg )
//        int     active;  //0 an einai hide
        unsigned int taddress; //dieythinsi mnimis- gia functions.
 //       scopespace_t spscope;
  //      unsigned int offset;
//        struct symT *next;
}Symbol;

typedef void (*execute_func_t)(instruction *);


typedef union Const_t{
	long int	inttype;
	char		*strtype;
	long double	doubletype;
}const_t;

typedef struct Const_s{
	const_t *value;
	vmarg_t type;
}const_s;

int FindLibFunction(char *fname);
int FindFunctionUserByAddr(unsigned int addr);


#endif
