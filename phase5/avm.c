#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "structs.h"
#include "avm.h"
#include "exec_arithmetic.h"
#include "exec_assign.h"
#include "exec_funcs.h"
#include "exec_statements.h"

typedef void (*memclear_func_t)(avm_memcell *);

unsigned int constArrayFilled = 0;
unsigned int userFunctionsFilled = 0;
unsigned int libFunctionsFilled = 0;
unsigned int instructionsFilled = 0;


const_s	*constArray = (const_s *)0;
Symbol 	**userFunctions ;
Symbol 	**libFunctions ;
instruction *VmargsTable =(instruction *) 0;
avm_memcell stack[AVM_STACKSIZE];



char* typeStrings[8] = {
	"number_m",
	"string_m",
	"bool_mavm_memcell stack[AVM_STACKSIZE];",
	"table_m",
	"userfunc_m",
	"libfunc_m",
	"nil_m","undef_m"
};



execute_func_t executeFuncs[] = {
   
   execute_assign,
   execute_add,
   execute_sub,
   execute_mul,
   execute_div,
   execute_mod,
   /*execute_uminus,
   execute_and,
   execute_or,
   execute_not,*/
0,
0,
0,
0,
   execute_jeq,
   execute_jne,
   execute_jle,
   execute_jge,
   execute_jlt,
   execute_jgt,
   execute_call,
   execute_pusharg,
   execute_ret,
   execute_getretval,
   execute_funcenter,
   execute_funcexit,
   execute_jump,
0,
0,
0,
   //execute_newtable,
   //execute_tablegetelem,
   //execute_tablesetelem,
   execute_nop
};

unsigned char executionFinished = 0;
unsigned int pc = 0;
//unsigned int firstfunc = 0 
unsigned int currLine = 0;
unsigned int codeSize = 0;
unsigned int global_num = 0;
instruction* code = (instruction*) 0;

avm_memcell stack[AVM_STACKSIZE];
extern unsigned int currInstruction;
extern unsigned int programVarOffset;
#define AVM_STACKENV_SIZE	4

memclear_func_t memclearFuncs[] = {
	0, /*number*/
	memclear_string,
	0,/*bool*/
	memclear_table,
	0,/*userfunc*/
	0,/*livfunc*/
	0,/*nil*/
	0/*undef*/
};


double 	 consts_getnumber (unsigned int index);
//char 	*consts_getstring (
void avm_memcellclear (avm_memcell* m)
{
	if(m->type != undef_m)
	{
		memclear_func_t f = memclearFuncs[m->type];
		if(f)
		{
			(*f)(m);
		}
		m->type = undef_m;
	}
}


void execute_cycle(void){
   if(executionFinished)
      return;
   else
      if(pc == AVM_ENDING_PC){
	//printf("DEBUG:%d\n",pc);
         executionFinished = 1;
         return;
      }
   else {
      //assert(pc < AVM_ENDING_PC);
      instruction* instr = code + pc;
      //printf("DEBUG:%d\n",instr -> opcode);
      assert(instr -> opcode >= 0 && instr -> opcode <= nop_v);
      if(instr -> srcLine)
         currLine = instr -> srcLine;
      unsigned int oldPC = pc; 
      (*executeFuncs[instr -> opcode])(instr);
      if(pc == oldPC)
         ++pc;
   }
}

 
avm_memcell* avm_translate_operand( vmarg* arg, avm_memcell* reg){

	switch (arg->type){
      case global_a :
	  	//printf("ok global\n");
	 	return(&stack[AVM_STACKSIZE-1-arg->val]);
      case local_a :
	 	return(&stack[topsp-arg->val]);
      case formal_a :
	 	return(&stack[topsp+AVM_STACKENV_SIZE+avm_totalactuals()-arg->val+1]);
      case retval_a :
	 	return(&retval);
      case integer_a:
      case number_a :
	 	reg->type = number_m;
	 	reg->data.numVal = constArray[arg->val].value->doubletype;
	 	return(reg);	 
      case string_a :
	 	reg->type = string_m;
	 	reg->data.strVal=strdup(constArray[arg->val].value->strtype);
	 	return(reg);
      case bool_a :
	 	reg->type = bool_m;
	 	reg->data.boolVal = arg->val;
	 	return(reg);
      case nil_a :
	 	reg->type = nil_m;
	 	return(reg);
      case userfunc_a :
	 	reg->type = userfunc_m;
	 	reg->data.funcVal = userFunctions[FindFunctionUserByAddr(arg->val)]->taddress;
		assert(reg->data.funcVal);
	 	return(reg);
      case libfunc_a :
	 	reg->type = libfunc_m;
	 	reg->data.libfuncVal=strdup(libFunctions[arg->val]->name);
	 
	 	return(reg);
      default :
		//printf("DEBUG:argtype:%d\n",arg->type);
	 	assert(0);
   }
}





//typedef void (*memclear_func_t)(avm_memcell*);
extern void memclear_string (avm_memcell* m){
	assert(m->data.strVal);
	free(m->data.strVal);
}

extern void memclear_table (avm_memcell* m)
{
	assert(m->data.tableVal);
	//avm_tabledecrefcounter(m->data.tableVal);
}




void avm_error(char* msg)
{
	executionFinished = 1;
	printf("\nRuntime Error: %s \n", msg);
}

void avm_warning(char* msg)
{
	printf("\nRuntime Warning: %s \n ",msg);
}





void readBin(void){

	FILE *fp;

	if(!(fp = fopen("../phase4/alpha.bin","rb+"))){

		fprintf(stderr, "Cannot open file: alpha.bin\n");
		assert(0);

		return;

	}

	unsigned int magicNumber;

	unsigned int i;

	fread(&magicNumber,sizeof(unsigned int),1,fp);

	if (magicNumber != 4){

		fprintf(stderr, "Wrong Magic Number exiting!!!\n");
		assert(0);

	}
 
	fread(&constArrayFilled,sizeof(unsigned int),1,fp);

	if (constArrayFilled != 0){

		//printf("DEBUG %d\n",constArrayFilled);

		constArray = /*(const_s)*/ malloc(constArrayFilled * sizeof(const_s));

		unsigned int thesis;

		unsigned int j=0;

		fread(&j,sizeof(unsigned int),1,fp);

		if(j!=0){

			for(i=0; i<j; i++){

				long int inttmp;

				long double doubletmp;

				const_s *tmp = malloc(sizeof(const_s));

				tmp->value = malloc(sizeof(const_t));

				fread(&thesis,sizeof(unsigned int),1,fp);

				fread(&inttmp,sizeof(long int),1,fp);

				doubletmp = (long double)inttmp;

				tmp->value->doubletype = doubletmp;

				tmp->type = number_a;

				const_s *constTmp = constArray + thesis;

				constTmp->value = tmp->value;

				constTmp->type = tmp->type;

				//constArray[thesis]=tmp;	

			}

		}

		fread(&j,sizeof(unsigned int),1,fp);

		if(j!=0){

			for(i=0; i<j; i++){

				long double doubletmp;

				const_s *tmp = malloc(sizeof(const_s));

				tmp->value = malloc(sizeof(const_t));

				fread(&thesis,sizeof(unsigned int),1,fp);

				fread(&doubletmp, sizeof(long double),1,fp);

				tmp->value->doubletype = doubletmp;

				tmp->type = number_a;

				const_s *constTmp = constArray + thesis;

				constTmp->value = tmp->value;

				constTmp->type = tmp->type;

				//constArray[thesis] = tmp;

			}

		}

		fread(&j,sizeof(unsigned int),1,fp);

		unsigned int length;

		if(j!=0){

			for(i=0; i<j; i++){

				const_s *tmp = malloc(sizeof(const_s));

				tmp->value = malloc(sizeof(const_t));

				fread(&thesis,sizeof(unsigned int),1,fp);

				fread(&length,sizeof(unsigned int),1,fp);

			//	printf("DEBUG %d\n",thesis);

				char * stringtmp = malloc(length*sizeof(char));

				fread(stringtmp,(length*sizeof(char)),1,fp);

			//	printf("DEBUG %s \n",stringtmp);

				tmp->value->strtype = stringtmp;

				tmp->type = string_a;

				const_s *constTmp = constArray + thesis;

				constTmp->value = tmp->value;

				constTmp->type = string_a/*tmp->type*/;

				//constArray[thesis] = tmp;

			//	printf("DEBUG %s \n",constTmp->value->strtype);

			//	printf("DEBUG %d \n",constTmp->type);

			}

		}

	}

	fread(&userFunctionsFilled,sizeof(unsigned int),1,fp);

	if(userFunctionsFilled != 0 ){

		userFunctions = malloc(userFunctionsFilled*sizeof(Symbol));

		unsigned int taddressTmp=0;

		unsigned int lineTmp = 0;

		unsigned int length = 0;

		for(i=0;i<userFunctionsFilled;i++){

			fread(&length,sizeof(unsigned int),1,fp);

			//printf("DEBUG %d length \n",length*sizeof(char));

			char *nameTmp = malloc((length * sizeof(char))+1);

			fread(nameTmp,length*sizeof(char),1,fp);

			fread(&taddressTmp,sizeof(unsigned int),1,fp);

			fread(&lineTmp,sizeof(unsigned int),1,fp);

			//printf("DEBUG %s\n",nameTmp);

			Symbol* tmpFunc = malloc(sizeof(Symbol));

			tmpFunc->name = nameTmp;

			tmpFunc->taddress = taddressTmp;

			tmpFunc->line = lineTmp;

			userFunctions[i] = tmpFunc;

		}

	}

	fread(&libFunctionsFilled,sizeof(unsigned int),1,fp);

	if (libFunctionsFilled != 0){

		unsigned int length = 0;

		libFunctions = malloc(libFunctionsFilled*sizeof(Symbol));

		for(i=0;i<libFunctionsFilled;i++){

			fread(&length,sizeof(unsigned int),1,fp);

			char *nameTmp = malloc(length*sizeof(char));

			fread(nameTmp,length*sizeof(char),1,fp);

			Symbol *tmpFunc = malloc(sizeof(Symbol));

			tmpFunc->name = nameTmp;

			libFunctions[i] = tmpFunc;

		}

	}

	fread(&instructionsFilled,sizeof(unsigned int),1,fp);

	if (instructionsFilled != 0){

		int type = 0;

		VmargsTable = malloc(instructionsFilled*sizeof(instruction));

		for(i=0;i<instructionsFilled;i++){

			fread(&type,sizeof(int),1,fp);

			instruction *tmp = malloc(sizeof(instruction));

			if(type == 5){

		//		instruction *tmp = malloc(sizeof(instruction));

				tmp->result = malloc(sizeof(vmarg));

				tmp->arg1 = malloc(sizeof(vmarg));

				tmp->arg2 = malloc(sizeof(vmarg));

				unsigned int uitmp;

				int itmp;

				fread(&itmp,sizeof(int),1,fp);

				tmp->opcode = (vmopcode)itmp;

				fread(&itmp,sizeof(int),1,fp);

				tmp->result->type = (vmarg_t)itmp;

				fread(&uitmp,sizeof(unsigned int),1,fp);

				tmp->result->val = uitmp;

				fread(&itmp,sizeof(int),1,fp);

				tmp->arg1->type = (vmarg_t)itmp;

				fread(&uitmp,sizeof(unsigned int),1,fp);

				tmp->arg1->val = (vmarg_t)uitmp;

				fread(&itmp,sizeof(int),1,fp);

				tmp->arg2->type = (vmarg_t)itmp;

				fread(&uitmp,sizeof(unsigned int),1,fp);

				tmp->arg2->val = uitmp;

				fread(&uitmp,sizeof(unsigned int),1,fp);

				tmp->srcLine = uitmp;

				instruction *instructionTmp = VmargsTable +i;

				instructionTmp->opcode = tmp->opcode;

				instructionTmp->result = tmp->result;

				instructionTmp->arg1 = tmp->arg1;

				instructionTmp->arg2 = tmp->arg2;

				instructionTmp->srcLine = tmp->srcLine;

			}

			else if(type == 4){

		//		instruction *tmp = malloc(sizeof(instruction));

				tmp->result = malloc(sizeof(vmarg));

				tmp->arg1 = malloc(sizeof(vmarg));

				unsigned int uitmp;

				int itmp;

				fread(&itmp,sizeof(int),1,fp);

				tmp->opcode = (vmopcode)itmp;

				fread(&itmp,sizeof(int),1,fp);

				tmp->result->type = (vmarg_t)itmp;

				fread(&uitmp,sizeof(unsigned int),1,fp);

				tmp->result->val = uitmp;

				fread(&itmp,sizeof(int),1,fp);

				tmp->arg1->type = (vmarg_t)itmp;

				fread(&uitmp,sizeof(unsigned int),1,fp);

				tmp->arg1->val = (vmarg_t)uitmp;

				fread(&uitmp,sizeof(unsigned int),1,fp);

				tmp->srcLine = uitmp;

				instruction *instructionTmp = VmargsTable +i;

				instructionTmp->opcode = tmp->opcode;

				instructionTmp->result = tmp->result;

				instructionTmp->arg1 = tmp->arg1;

				instructionTmp->srcLine = tmp->srcLine;

			}

			else if(type == 3){

		//		instruction *tmp = malloc(sizeof(instruction));

				tmp->result = malloc(sizeof(vmarg));

				unsigned int uitmp;

				int itmp;

				fread(&itmp,sizeof(int),1,fp);

				tmp->opcode = (vmopcode)itmp;

				fread(&itmp,sizeof(int),1,fp);

				tmp->result->type = (vmarg_t)itmp;

				fread(&uitmp,sizeof(unsigned int),1,fp);

				tmp->result->val = uitmp;

				fread(&uitmp,sizeof(unsigned int),1,fp);

				tmp->srcLine = uitmp;

				instruction *instructionTmp = VmargsTable +i;

				instructionTmp->opcode = tmp->opcode;

				instructionTmp->result = tmp->result;

				instructionTmp->srcLine = tmp->srcLine;

			}

			else{

				fprintf(stderr,"Error in reading file.\n");

				assert(0);

			}

                        //instruction *instructionTmp = VmargsTable +i;

			//instructionTmp = tmp;

		}

	}

	fclose(fp);

}

void execute_nop(instruction* instr){

printf("Warning!NOP instruction executed!.\n");

}

static void avm_StackInit(void){
	unsigned int i = 0;
	while (i<AVM_STACKSIZE){
		AVM_WIPEOUT(stack[i]);
		stack[i].type = undef_m;		
		++i;
	}

}

void main(){
	int i = 0;
	readBin();
	int ik = 0;
	printf("instructions : ");
	for(ik=0;ik<instructionsFilled;ik++){
		printf("%d: %d,res %d, \n",ik,VmargsTable[ik].opcode,VmargsTable[ik].result->val);
	}
	printf("\n");
	code = VmargsTable;
	avm_StackInit();
	codeSize = instructionsFilled;
	top = topsp = AVM_STACKSIZE- constArrayFilled -1;
	
	while (executionFinished ==0){
		
		//printf("DEBUG:Ektellestike : %d\n",executionFinished);
		execute_cycle();
	}
	//printf("%d, %d, %d, %d\n",constArrayFilled, userFunctionsFilled, libFunctionsFilled, instructionsFilled);

}
