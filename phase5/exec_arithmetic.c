#include <stdio.h>
#include <assert.h>

#include "avm.h"
#include "exec_arithmetic.h"
#include "structs.h"
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

arithmetic_func_t arithmeticFuncs[] ={
	add_impl,
	sub_impl,
	mul_impl,
	div_impl,
	mod_impl
};

void execute_arithmetic (instruction* instr){
	avm_memcell* lv = avm_translate_operand(instr->result,(avm_memcell*)0);
	avm_memcell* rv1= avm_translate_operand(instr->arg1,&ax);
	avm_memcell* rv2= avm_translate_operand(instr->arg2,&bx);

	assert(lv && ( (&stack[AVM_STACKSIZE]>= lv && &stack[top]<lv )|| lv==&retval ));
	assert(rv1 && rv2);

	if (rv1->type !=number_m || rv2->type !=number_m ){
		avm_error("not a number arithmetic!\n");
		executionFinished=1;
	}
	else{
		arithmetic_func_t op=arithmeticFuncs[instr->opcode - add_v];
		avm_memcellclear(lv);
		lv->type = number_m;
		lv->data.numVal = (*op)(rv1->data.numVal,rv2->data.numVal);
	}
	//printf("DEBUG VALUE:%d\n",(int)lv->data.numVal);
}

double add_impl( double x, double y ){
	return x+y;
}

double sub_impl( double x, double y ){
	return x-y;
}

double mul_impl( double x, double y ){
	return x*y;
}

double div_impl( double x, double y ){
	if( y == 0){avm_error("Error:Division with zero attempted.\n");}
	return x/y;
}

double mod_impl( double x, double y ){
	if( y == 0){avm_error("Error:Division with zero attempted.\n");}
	return (( unsigned ) x ) % (( unsigned ) y );
}

//exec logic?


