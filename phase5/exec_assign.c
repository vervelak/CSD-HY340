#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "avm.h"
#include "exec_assign.h"
//TODO:Fixed includes.

extern unsigned pc;
extern unsigned codeSize;
extern unsigned top,topsp;
extern unsigned currLine;
extern unsigned char executionFinished;
extern avm_memcell ax,bx,cx;
extern instruction* code;
extern avm_memcell retval;
extern avm_memcell stack[AVM_STACKSIZE];


void avm_assign(avm_memcell* lv, avm_memcell* rv){
	if(lv == rv)
		return;
	
	
	if(rv -> type == undef_m ||rv -> type ==  nil_m)
		avm_warning("assigning from 'undef' content!");
	//printf("DEBUG:VALUE:%s\n",rv->data.numVal);
	
	avm_memcellclear(lv);
	
	memcpy(lv, rv, sizeof(avm_memcell));
	
	if(lv -> type == string_m){
		lv -> data.strVal = strdup(rv -> data.strVal);
	}
	else if(lv -> type == number_m){
		lv -> data.numVal = rv -> data.numVal;
	}
	else if(lv -> type = bool_m){
		lv -> data.boolVal = rv -> data.boolVal;
	}
	else if(lv -> type == userfunc_m){
		lv -> data.funcVal = rv -> data.funcVal;
	}
	else if(lv -> type == table_m){
		lv -> data.tableVal = rv -> data.tableVal;
	}
	else if(lv -> type == libfunc_m){
		lv -> data.libfuncVal = rv -> data.libfuncVal;
	}
}

void execute_assign (instruction *instr){

unsigned int totalActuals =0;

	avm_memcell *lv = avm_translate_operand(instr->result,(avm_memcell *) 0);
	avm_memcell *rv = avm_translate_operand(instr->arg1,&ax);

	//assert(lv && (&stack[AVM_STACKSIZE-1] >= lv && lv > &stack[top] || lv==&retval ));
	assert(rv);	
	
	avm_assign(lv,rv);

}
