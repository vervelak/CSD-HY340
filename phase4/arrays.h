
#ifndef ARRAYS_H_
#define ARRAYS_H_

#include "target.h"

#define EXPAND_CONST_SIZE 1024
#define EXPAND_SYM_SIZE 1024

#define CURR_CONST_SIZE (constArraySize*sizeof(const_s))
#define CURR_SYM_SIZE (functionUserSize*sizeof(Symbol))
#define NEW_CONST_SIZE  (EXPAND_CONST_SIZE * sizeof(const_s)+CURR_CONST_SIZE)
#define NEW_SYM_SIZE  (EXPAND_SYM_SIZE * sizeof(Symbol)+CURR_SYM_SIZE)

typedef union Const_t{
	unsigned int	booltype;
	long int	inttype;
	char		*strtype;
	long double	doubletype;
}const_t;

typedef struct Const_s{
	const_t *value;
	vmarg_t type;
}const_s;


void ExpandConstArray(void);

void ExpandFunctionUser(void);

void ExpandLibFunctions(void);

int AddConstArrayElem(const_s *);

int AddFunctionUserElem(Symbol *newelem);

int AddLibFunctions(Symbol *newelem);

int FindFunctionUser(char *name,int scope);

int FindLibFunction(char *fname);

const_s *IntToConst(long int);

const_s *DoubleToConst(long double);

const_s *StringToConst(char *);

const_s *BoolToConst(unsigned int);

int isLibFunction(char * fname);

void InitLibFuncs();

void patchLocals(unsigned int, unsigned int);




#endif
