#include <stdio.h>
#include <assert.h>


#include "avm.h"
#include "structs.h"
#include "exec_statements.h"
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

statements_func_t statementsFuncs[] ={
	jle_stmt,
	jge_stmt,
	jlt_stmt,
	jgt_stmt
};

tobool_func_t toboolFuncs[] ={
	number_tobool,
	string_tobool,
	bool_tobool,
	table_tobool,
	userfunc_tobool,
	libfunc_tobool,
	nil_tobool,
	undef_tobool
};

void execute_statement (instruction* instr){
	
	unsigned result = 0;
        avm_memcell* rv1= avm_translate_operand(instr->arg1,&ax);
        avm_memcell* rv2= avm_translate_operand(instr->arg2,&bx);

        assert(rv1 && rv2);

        if (rv1->type !=number_m || rv2->type !=number_m )   
        {
                avm_error("Error:Not an arithmetic constant!\n");
                //executionFinished=1;
        }
        else
        {
		switch (instr->opcode){
			case (jle_v):
				result = statementsFuncs[0] (rv1->data.numVal,rv2->data.numVal);
				break;
			case (jge_v):
				result = statementsFuncs[1] (rv1->data.numVal,rv2->data.numVal);
				break;
			case (jlt_v):
				result = statementsFuncs[2] (rv1->data.numVal,rv2->data.numVal);
				break;
			case (jgt_v):
				result = statementsFuncs[3] (rv1->data.numVal,rv2->data.numVal);
				break;
		}

		if( (result) && (!executionFinished) )
		{
			pc = instr->result->val;
		}
			
        }
}


void execute_jeq(instruction* instr){
	unsigned char result = 0;

	assert(instr->result->type == label_a);

	avm_memcell* rv1 = avm_translate_operand(instr->arg1, &ax);
	avm_memcell* rv2 = avm_translate_operand(instr->arg2, &bx);


	if (rv1->type == undef_m || rv2->type == undef_m)
	{
		avm_error("'Undef' involved in equality!");
	}
	else if(rv1->type==nil_m || rv2->type ==nil_m)
	{
		result = (rv1->type == nil_m) && (rv2->type == nil_m);
	}
	else if(rv1->type == bool_m || rv2->type == bool_m)
	{
		result=rv1->data.boolVal == rv2->data.boolVal;
	}
	else if(rv1->type !=rv2->type)
	{
		avm_error("String is illegal!"); //,typeStrings[rv1->type]);
	}

	else
	{
		switch (rv1->type)
		{
			case number_m:
			{
				result = rv1->data.numVal == rv2->data.numVal;	
				break;
			}
			case string_m:
			{
				result = !(strcmp(rv1->data.strVal,rv2->data.strVal));		
				break;
			}
			case table_m:
			{
				result = rv1->data.tableVal == rv2->data.tableVal ; 	
				break;	
			}
			case userfunc_m:
			{
				result = rv1->data.funcVal == rv2->data.funcVal;			
				break;
			}
			case libfunc_m:
			{
				result = !(strcmp(rv1->data.libfuncVal,rv2->data.libfuncVal));		
				break;
			}

			default: assert(0);

		}
	}
	
	if(!executionFinished && (result == 1))
	{
		pc=instr->result->val;
	}
}

void execute_jne(instruction* instr){
	unsigned char result = 0;

	assert(instr->result->type == label_a);

	avm_memcell* rv1 = avm_translate_operand(instr->arg1, &ax);
	avm_memcell* rv2 = avm_translate_operand(instr->arg2, &bx);

	
	if (rv1->type == undef_m || rv2->type == undef_m)
	{
		avm_error("'Undef' involved in equality!");
	}
	else if(rv1->type==nil_m || rv2->type ==nil_m)
	{
		result = rv1->type == nil_m && rv2->type == nil_m;
	}

	else if(rv1->type == bool_m || rv2->type == bool_m)
	{
		result=(rv1->data.boolVal == rv2->data.boolVal);
	}
	else if(rv1->type !=rv2->type)
	{
		avm_error("String is illegal!"); //,typeStrings[rv1->type]);
	}
	else
	{
		switch (rv1->type)
		{
			case number_m:
			{
				result = rv1->data.numVal == rv2->data.numVal;	
				break;
			}
			case string_m:
			{
				result = !(strcmp(rv1->data.strVal,rv2->data.strVal));		
				break;
			}
			case table_m:
			{
				result = rv1->data.tableVal == rv2->data.tableVal;	
				break;	
			}
			case userfunc_m:
			{
				result = rv1->data.funcVal == rv2->data.funcVal;			
				break;
			}
			case libfunc_m:
			{
				result = !(strcmp(rv1->data.libfuncVal,rv2->data.libfuncVal));		
				break;
			}

			default: assert(0);

		}
	}

	if( (!executionFinished) && (!result) )
	{
		pc=instr->result->val;
	}
}


void execute_jump(instruction* instr){
	//assert(instr->result->type == label_a);

	pc=instr->result->val;
	
}

int jge_stmt(double x,double y)
{
	return x >= y;
}

int jgt_stmt(double x,double y)
{
    	return x > y;
}

int jle_stmt(double x,double y)
{
      	return x<=y;
}

int jlt_stmt(double x,double y)
{
	return x < y;
}


unsigned char number_tobool (avm_memcell* m) {
	return m->data.numVal !=0; 
}

unsigned char string_tobool (avm_memcell* m) {	
	return m->data.strVal[0] !=0; 
}

unsigned char bool_tobool (avm_memcell* m) {
	return m->data.boolVal !=0; 
}

unsigned char table_tobool (avm_memcell* m) {	
	return 1; 
}

unsigned char userfunc_tobool (avm_memcell* m) {
	return 1; 
}

unsigned char libfunc_tobool (avm_memcell* m) {
	return 1; 
}

unsigned char nil_tobool (avm_memcell* m) {	
	return 0; 
}
	
unsigned char undef_tobool (avm_memcell* m) {
	return 0; 
}

unsigned char avm_tobool(avm_memcell* m){
	assert(m->type >=0 && m->type <undef_m);
	return (*toboolFuncs[m->type])(m);
}
