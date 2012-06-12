#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "functions.h"
#include "target.h"
#include "arrays.h"
#include "symbolt.h"

unsigned int constArraySize = 0;
unsigned int functionUserSize = 0;
unsigned int libFunctionsSize = 0;
unsigned int constArrayFilled = 0;
unsigned int functionUserFilled = 0;
unsigned int libFunctionsFilled = 0;


const_s	*constArray = (const_s *) 0;
Symbol 	**functionUser = (Symbol **) 0;
Symbol 	**libFunctions = (Symbol **) 0;

void ExpandConstArray(void){
	assert(constArrayFilled == constArraySize);
	const_s* newConstArray = (const_s*) malloc(NEW_CONST_SIZE);
	if (constArray){
		memcpy(newConstArray,constArray , CURR_CONST_SIZE);
		free(constArray);
	}
	constArray = newConstArray;
	constArraySize += EXPAND_CONST_SIZE;
}

void ExpandFunctionUser(void){
	assert(functionUserFilled == functionUserSize);
	Symbol **newFunctionUser = (Symbol **) malloc(NEW_SYM_SIZE);
	if (functionUser){
		memcpy(newFunctionUser,functionUser , CURR_SYM_SIZE);
		
		free(functionUser);
	}
	functionUser = newFunctionUser;
	functionUserSize += EXPAND_SYM_SIZE;
}

int FindFunctionUser(char *fname,int fscope){
	Symbol ** iter;
	int i =0;
	while (i<functionUserFilled){
		iter = functionUser+i;		
		if ((!strcmp((*iter)->name,fname)) && ((*iter)->scope<=fscope)){
			return (*iter)->taddress;
		}
		i++;
	}
	return -1;


}

int FindLibFunction(char *fname){
	Symbol ** iter;
	int i =0;
	while (i<libFunctionsFilled){
		iter = libFunctions+i;		
		if ((!strcmp((*iter)->name,fname))){
			return i;//(*iter)->taddress;
		}
		i++;
	}
	return -1;


}

void ExpandLibFunctions(void){
	assert(libFunctionsFilled == libFunctionsSize);
	Symbol **newLibFunctions = (Symbol **) malloc(NEW_SYM_SIZE);
	if (libFunctions){
		memcpy(newLibFunctions,libFunctions , CURR_SYM_SIZE);
		
		free(libFunctions);
	}
	libFunctions = newLibFunctions;
	libFunctionsSize += EXPAND_SYM_SIZE;
}

//TODO:
/*
if((constArray+constArrayFilled) == NULL){
		return;
	}
	Na metatrapei se oles se assertion.
*/
//fuctionUser[i]->


int AddConstArrayElem(const_s * newelem){
	const_s *constTmp;	
	if(constArrayFilled == constArraySize){
		ExpandConstArray();
	}
	
	constTmp = constArray + constArrayFilled;
	constTmp->value = newelem->value;
	constTmp->type = newelem->type;
	
	//free(newelem);
	constArrayFilled++;
	return constArrayFilled - 1;
}

int AddFunctionUserElem(Symbol *newelem){
	Symbol **funcTmp;	
	if(functionUserFilled == functionUserSize){
		ExpandFunctionUser();
	}
	if((functionUser + functionUserFilled) == NULL){
		return;
	}
	
	funcTmp = functionUser + functionUserFilled;
	*funcTmp = newelem;
	functionUserFilled++;
	return functionUserFilled - 1;
}

int AddLibFunctions(Symbol *newelem){
	Symbol **funclibTmp;
	if(libFunctionsFilled == libFunctionsSize){
		ExpandLibFunctions();
	}
	if((libFunctions + libFunctionsFilled) == NULL){
		return;
	}
	
	funclibTmp = libFunctions + libFunctionsFilled;
	*funclibTmp = newelem;
	libFunctionsFilled++;
	return libFunctionsFilled - 1;
}

int isLibFunction(char * fname){
	int iter = 0;	
	char *libs[]=	{"print","input","objectmemberkeys","objecttotalmembers","objectcopy","totalarguments","argument","typeof","sqrtonum","cos","sin"};

	while (iter<11){
		if(!strcmp(fname,*(libs+iter++))){
			return 1;
		}
	}
	return 0;
	


}
//Voithitiki Sinartisi gia arxikopoiisi-isws na min xreiastei.
void InitLibFuncs(){

	instruction *t =(instruction *) InitInstruction();
	expr * e = malloc(sizeof(expr));
	e->type = libraryfunc_e;
	e->sym =createS( "print", 0, 0, "print", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());
	make_operand(e,t->arg1);
	e->sym = createS( "input", 0, 0, "input", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());
	make_operand(e,t->arg1);
	e->sym = createS( "objectmemberkeys", 0, 0, "objectmemberkeys", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());
	make_operand(e,t->arg1);
	e->sym = createS( "objecttotalmembers", 0, 0, "objecttotalmembers", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());
	make_operand(e,t->arg1);
	e->sym = createS( "objectcopy", 0, 0, "objectcopy", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());
	make_operand(e,t->arg1);
	e->sym = createS( "totalarguments", 0, 0, "totalarguments", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());	
	make_operand(e,t->arg1);
	e->sym = createS( "argument", 0, 0, "argument", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());
	make_operand(e,t->arg1);
	e->sym = createS( "typeof", 0, 0, "typeof", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());	
	make_operand(e,t->arg1);
	e->sym = createS( "strtonum", 0, 0, "strtonum", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());	
	make_operand(e,t->arg1);
	e->sym = createS( "sqrt", 0, 0, "sqrt", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());	
	make_operand(e,t->arg1);
	e->sym = createS( "cos", 0, 0, "cos", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());	
	make_operand(e,t->arg1);
	e->sym = createS( "sin", 0, 0, "sin", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());
	make_operand(e,t->arg1);
}

const_s *IntToConst(long int integer){
	const_s *tmp = malloc(sizeof(const_s));	
	tmp->value = malloc (sizeof(const_t));
	
	tmp->value->inttype = integer;
	tmp->type = integer_a;
	return tmp;
}

const_s *DoubleToConst(long double doublenum){
	const_s *tmp = malloc(sizeof(const_s));	
	tmp->value = malloc (sizeof(const_t));
	tmp->value->doubletype = doublenum;
	tmp->type = double_a;
	return tmp;
}

const_s *StringToConst(char * str){
	const_s *tmp = malloc(sizeof(const_s));
	tmp->value = malloc (sizeof(const_t));	
	tmp->value->strtype = strdup(str);
	tmp->type =string_a;
	return tmp;
}

const_s *BoolToConst(unsigned int boolean){
	const_s *tmp = malloc(sizeof(const_s));	
	tmp->value = malloc (sizeof(const_t));
	tmp->value->booltype = boolean;
	tmp->type = bool_a;
	return tmp;
}

void patchLocals(unsigned int Faddress, unsigned int locals){
	Symbol ** iter;
	int i =0;
	while (i<functionUserFilled){
		iter = functionUser+i;		
		if ((*iter)->taddress == Faddress){
			(*iter)->line = locals;
			return;
		}
		i++;
	}
}



