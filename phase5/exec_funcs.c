#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "structs.h"
#include "avm.h"
#include "exec_funcs.h"


extern unsigned pc;
extern unsigned codeSize;
extern unsigned top,topsp;
extern unsigned currLine;
extern unsigned char executionFinished;
extern avm_memcell ax,bx,cx;
extern instruction* code;
extern avm_memcell retval;
extern avm_memcell stack[AVM_STACKSIZE];

extern unsigned int constArrayFilled;
extern unsigned int userFunctionsFilled;
extern unsigned int libFunctionsFilled;
extern unsigned int instructionsFilled;

extern const_s	*constArray;
extern Symbol 	**userFunctions;
extern Symbol 	**libFunctions;
extern instruction *VmargsTable;

#define EXPAND_ST_SIZE 1024
#define CURR_ST_SIZE (length)*sizeof(char)
#define NEW_ST_SIZE (EXPAND_ST_SIZE* sizeof(char)+CURR_ST_SIZE)

unsigned totalActuals = 0;


void execute_ret(instruction *inst){}
void execute_getretval(instruction *instr) {}

void execute_call(instruction *instr){
	avm_memcell* func = avm_translate_operand(instr -> result, &ax);
	assert(func);
	avm_callsaveenviroment();

	switch(func -> type){
		case userfunc_m: {
			pc = func -> data.funcVal;//TODO:-1;
			assert(pc < AVM_ENDING_PC);
			assert(code[pc].opcode == funcenter_v);
			break;
		}
		
		case string_m: avm_calllibfunc(func -> data.strVal);
			break;
		
		case libfunc_m:
			//printf("DEBUG:CALL:%s\n",func -> data.libfuncVal); 
			avm_calllibfunc(func -> data.libfuncVal);
			break;
			
		default: {
			char *s = all_tostring(func);
			avm_error("call: cannot bind to function!");
			free(s);
			//executionFinished = 1;
		}
	}
}



 
void avm_callsaveenviroment(void){
	avm_push_envvalue(totalActuals);
	avm_push_envvalue(pc + 1);
	avm_push_envvalue(top + totalActuals + 2);
	avm_push_envvalue(topsp);
}


void avm_push_envvalue(unsigned int val){
	stack[top].type = number_m;
	stack[top].data.numVal = val;
	avm_dec_top();
}

void avm_dec_top(void){
	if(!top){
		avm_error("stack overflow");
		//executionFinished = 1;
	}
	else
		--top;
}

void avm_calllibfunc(char* id){
	Symbol * f = avm_getlibraryfunc(id);
	//printf("DEBUG:FNAME %s\n",f->name);
	if(!f){
		avm_error("unsupported lib func called!");
		//executionFinished = 1;
	}
	else{
		topsp = top;
		totalActuals = 0;
		if (!strcmp(f->name,"print")){
			libfunc_print();
		}
		else if (!strcmp(f->name,"typeof")){
			libfunc_typeof();
		}

		//TODO: Na siblirothoun oi ipoloipes sinartiseis....
		if(!executionFinished)
			execute_funcexit((instruction*)0);
	}
}

Symbol * avm_getlibraryfunc(char *id){
	int i;
	if((i = FindLibFunction(id))!=-1)
		return (libFunctions[i]);
	return (Symbol *)0;
}


void execute_funcexit(instruction *unused){
	unsigned int oldTop = top;
	top = avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);
	pc = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);
	topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	//const_s tmpconst_s = constArray[topsp];
	int tmp_s = stack[topsp].data.numVal;
	while(oldTop <= top){
		avm_memcellclear(&stack[oldTop]);
		++oldTop;
	}
	avm_memcellclear(&retval);
		retval.type = constArray[tmp_s].type;
		if((retval.type==string_m))
			retval.data.strVal = strdup(constArray[tmp_s].value->strtype);
		//if(retval.type==number_m)
		//	retval.data.intVal = tmpconst_s->value->inttype;
		if(retval.type==number_m){
			//printf("DEBUG:%Lf",tmpconst_s.value->doubletype);		
			retval.data.numVal = (double)constArray[tmp_s].value->doubletype;
		}
}

void execute_funcenter(instruction *instr){
	avm_memcell *func = avm_translate_operand(instr -> result, &ax);
	assert(func);
	assert(pc == func -> data.funcVal);
	
	totalActuals = 0;
	Symbol *funcInfo = avm_getfuncinfo(pc);
	topsp = top;
	top = top - funcInfo -> line; //Local Vars Here - reusing fields. :P 
}

Symbol *avm_getfuncinfo(unsigned int address){
	int i;
	for(i=0 ; i<userFunctionsFilled ; i++){
		if(userFunctions[i]->taddress == address)
			return userFunctions[i];
	}
	return 0;
}

unsigned int avm_get_envvalue(unsigned int i){
	assert(stack[i].type == number_m);
	double val = (double)stack[i].data.numVal;
	//assert(stack[i].data.intVal == ((int)val));
	return val;
}

unsigned avm_totalactuals(){
	return avm_get_envvalue(topsp+AVM_NUMACTUALS_OFFSET);
}

avm_memcell *avm_getactual(unsigned i){
	assert(i < avm_totalactuals());
	return &stack[topsp + AVM_STACKENV_SIZE + 1 + i];
}

void execute_pusharg(instruction* instr){
	avm_memcell *arg = avm_translate_operand(instr -> result, &ax);
	assert(arg);
	//printf("DEBUG:TOP:%d\n",top);
	assert(top<AVM_STACKSIZE); //TODO: check name for avmstacksize
	avm_assign(&stack[top], arg);
	++totalActuals;
	avm_dec_top();
}




//****LIBRARY FUNCTIONS****

//atroul
void libfunc_print(){

	unsigned i = 0;
	unsigned n = 0;	
	n = avm_totalactuals();
	//printf("DEBUG: TOTALACTUALS:%d\n",n);
	for(i=0;i<n;++i){  
		char *s =all_tostring(avm_getactual(i)); //avm_tostring(avm_getactual(i));
		//printf("%s", s);
		
			puts(strdup(s));
		if(*s){
			free(s);
		}
	}
	
}


void libfunc_typeof(){
	unsigned n = avm_totalactuals();
	if(n != 1){
		avm_error("One argument (not many) expected in 'typeof'.");
	}
	else{
		avm_memcellclear(&retval);
		retval.type = string_m;
		retval.data.strVal = strdup(constArray[avm_getactual(0)->type].value->strtype);
	}

}

void libfunc_totalarguments(){
	unsigned int tmp_topsp = avm_get_envvalue( topsp + AVM_SAVEDTOPSP_OFFSET );
	avm_memcellclear( &retval );

	if( !tmp_topsp ){
		avm_warning("Totalarguments() (library function) called outside of function!");
		retval.type = nil_m;
	}
	else{
		retval.type = number_m;
		retval.data.numVal = avm_get_envvalue( tmp_topsp + AVM_NUMACTUALS_OFFSET);
	}
}

void libfunc_argument(){

	unsigned tmp , tmp_topsp = 0;
	unsigned k = avm_totalactuals();

	avm_memcell *function = malloc( sizeof( avm_memcell ) );
	if( function == NULL ){
		avm_error("No space for malloc!!");
	}

	if( k < 1 ){
		avm_error("Argument() (library function) expects one argument");
	}
	else{
		tmp_topsp = avm_get_envvalue( topsp + AVM_SAVEDTOPSP_OFFSET );
		avm_memcellclear( &retval );//checkpoint!!!!
		if( tmp_topsp == AVM_STACKSIZE - constArrayFilled - 1 ){
			avm_warning("Argument() (library function) called outside of a function");
			retval.type = nil_m;
		}
		else{
			function = avm_getactual( 0 );
			if( function->type != number_m ){
				avm_error( "Argument() (library function) expects number for argument!");
			}
			else{
				tmp = ( int )function->data.numVal;
			}
			if( ( tmp >= stack[ tmp_topsp + AVM_NUMACTUALS_OFFSET ].data.numVal ) || ( tmp < 0 ) ){
				avm_warning( "Argument() (library function) expects different type of argument");
				retval.type = undef_m;
				return;
			}

			avm_assign( &retval , &stack[ tmp_topsp + AVM_NUMACTUALS_OFFSET + tmp + 1 ]);
		}
	}
}

void libfunc_input(){
	char *tmp_string = malloc( sizeof( 1024 ) );
	scanf( "%s" , tmp_string );
	int i = 0;
	int noDigits = 0;
	int noDots = 0;
	int noAlphas = 0;

	while( i < strlen( tmp_string ) ){
		if( isdigit( tmp_string[ i ] ) ){
			noDigits++;
		}
		else if( tmp_string[i] == '.' ){
			noDots++;
		}
		else if( isalpha( tmp_string[ i ] ) ){
			noAlphas++;
		}
		i++;
	}

	if( (noDigits == strlen( tmp_string )) && (noDots == 0 ) && ( noAlphas == 0) ){ //AKERAIOI
		retval.type = number_m;
		retval.data.numVal = atoi( tmp_string );
	}
	else if( (noDigits == strlen( tmp_string )) && (noDots > 0 ) && ( noAlphas == 0) ){ //DOUBLES
		retval.type = number_m;
		retval.data.numVal = atof( tmp_string );
	}
	else if( noAlphas > 0 ){ //STRINGS
		retval.type = string_m;
		retval.data.strVal = tmp_string;
	}
	else if( !(strcmp( tmp_string , "nil" )) ){
		retval.type = nil_m;
	}
	else if( !(strcmp( tmp_string , "true" )) || !(strcmp( tmp_string , "false") ) ){
		retval.type = bool_m;
		if( !(strcmp( tmp_string , "true" ) ) ){
			retval.data.boolVal = 1;
		}
		else{
			retval.data.boolVal = 0;
		}
	}
}

//atroul

char * all_tostring(avm_memcell *memcell){
	
	switch(memcell->type){
		case number_m: {
			char *tmp = malloc(16*sizeof(char));
			sprintf(tmp,"%'.2f", memcell->data.numVal);
			return strdup(tmp);
		}
		case string_m: {
			return strdup(memcell->data.strVal);
		}
		case userfunc_m: {
			unsigned int tmp = memcell->data.funcVal;
			char * tmp1 = malloc(64*sizeof(char));
			sprintf(tmp1,"User Function <%s, %d>",userFunctions[tmp]->name,userFunctions[tmp]->taddress);
			return strdup(tmp1);
		}
		case libfunc_m: {
			return strdup(memcell->data.libfuncVal);
		}
		case nil_m: {
			return "NIL";
		}
		default:
			return "";
		
	}
}




