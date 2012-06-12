#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symbolt.h"
#include "functions.h"
#include "target.h"
#include "arrays.h"


generator_func_t generators[] = {
	generate_ASSIGN ,
	generate_ADD ,
       	generate_SUB ,
       	generate_MUL ,
       	generate_DIV ,
       	generate_MOD ,
	generate_UMINUS ,
	generate_AND,
	generate_OR ,
	generate_NOT ,
	generate_IF_EQ ,
	generate_IF_NOTEQ ,	
	generate_IF_LESSEQ ,
	generate_IF_GREATEREQ ,
	generate_IF_LESS ,
	generate_IF_GREATER ,
	generate_CALL ,
	generate_PARAM ,
	generate_RETURN ,
	generate_GETRETVAL ,
	generate_FUNCSTART ,
	generate_FUNCEND ,
	generate_JUMP ,
	generate_TABLECREATE ,
       	generate_TABLEGETELEM ,
       	generate_TABLESETELEM ,
       	generate_NOP
};


//TODO: ADD TO MAKEFILE and the header file too.

extern unsigned int constArraySize;
extern unsigned int functionUserSize;
extern unsigned int libFunctionsSize;
extern unsigned int constArrayFilled;
extern unsigned int functionUserFilled;
extern unsigned int libFunctionsFilled;
extern const_s	*constArray ;
extern Symbol 	**functionUser ;
extern Symbol 	**libFunctions ;
extern int currQuad;
extern int deepestFunctionScope;

unsigned int currInstruction = 0;
unsigned int totalVmargs = 0;
instruction *VmargsTable =(instruction *) 0;
//XXX Vervelak
funcs *fHead = NULL;
//instruction *VmargsTable = NULL;
incJump *incJumpH = NULL;
unsigned int processedQuad = 0;
//XXX /Vervelak

void expandVmargsTable(){
	assert(totalVmargs==currInstruction);
	instruction* t = (instruction *) malloc(NEW_VMARG_SIZE);
	if (VmargsTable){
		memcpy(t,VmargsTable, CURR_VMARG_SIZE);
		free(VmargsTable);
	}
	VmargsTable = t;
	totalVmargs += EXPAND_SIZE;

}

void emitVmarg (instruction * t){
	instruction *tmp = malloc (sizeof(instruction));
	if (currInstruction== totalVmargs){
		expandVmargsTable();
	}
	assert(totalVmargs);
	
	tmp = VmargsTable + currInstruction++;
	tmp->opcode = t->opcode;
	tmp->arg1 = t->arg1;
	tmp->arg2 = t->arg2;
	tmp->result = t->result;
	tmp->srcLine = t-> srcLine;

}
vmarg *make_vmarg(){
	vmarg *vmArg = malloc(sizeof(vmarg));
	return( vmArg );
}



void make_operand( expr *e , vmarg *arg ){
	//assert(e && arg);
	//printf("TYYYYYPOOOOOS:%d\n",e->sym->offset);
	switch( e->type ){
		/*
		* All those below use a variable for storage
		*/
		case var_e:
		case tableitem_e:
		case arithexpr_e:
		case boolexpr_e:
		case assignexpr_e:
		case newtable_e: {
			assert( e->sym );
			arg->val = e->sym->offset;
			///print///printf("VAR NAME: %s,OFFSET: %d \n", e->sym->name,arg->val);
			switch( e->sym->spscope ){
				case globalvar: 	arg->type = global_a; 	break;
				case functionlocal: 	arg->type = local_a; 	break;
				case formalarg:		arg->type = formal_a;	break;
				default: assert( 0 );
			}
			break; /* from case newtable_e */
		}

		/*
		*	Constants
		*/

		case constbool_e:{
			arg->val = e->boolConst; //AddConstArrayElem(BoolToConst(e->boolConst));
			arg->type = bool_a;		
			break;
		}
		case conststring_e:{
			arg->val = AddConstArrayElem(StringToConst(e->strConst));
			arg->type = string_a;		
			break;
		}
		case constdouble_e:{
			arg->val = AddConstArrayElem(DoubleToConst(e->numConst));
			
			arg->type = double_a;		
			break;
		}
		case constint_e:{
			arg->val = AddConstArrayElem(IntToConst(e->intConst));
			//printf("AASFADFDSFSDFDFAA: %d \n",constArrayFilled);
			arg->type = integer_a;		
			break;
		}
		case nil_e: arg->type = nil_a; break;

		/*
		*	Functions
		*/

		case programfunc_e:{
			///print///printf("AAAAAAAAdsfsfsdfsdf:%d\n\n",e->sym->taddress);
			arg->type = userfunc_a; //TODO: Na diorthothei.
			arg->val = e->sym->taddress;

			break;
		}
		case libraryfunc_e:{
			arg->type = libfunc_a;
			//printf("TEST HEREEEE!!");
			arg->val = AddLibFunctions( e->sym );
			break;
		}

		//default: return;//assert( 0 ); //TODO: Thelei fix edw.

	}

}

vmarg *reset_operand(){
   return( (vmarg *) NULL);
}

int nextInstructionLabel(){
	return currInstruction;
}

vmarg *make_integeroperand(int val){
   vmarg *arg = (vmarg *)malloc(sizeof(vmarg));
   arg->val = AddConstArrayElem(IntToConst(val));
   arg->type = integer_a;
   return(arg);
}


vmarg *make_doubleoperand(float val){
   vmarg *arg = (vmarg *)malloc(sizeof(vmarg));
   arg->val = AddConstArrayElem(DoubleToConst(val));
   arg->type = double_a;
   return(arg);
}



void make_booloperand( vmarg* arg, unsigned int val ){
	arg->val = val;
	arg->type = bool_a;
}

void make_retvaloperand( vmarg* arg ){
	 arg->type = retval_a; 
}



void generate(vmopcode op ,quad * quad ){
	instruction *t = malloc(sizeof(instruction));
	t->arg1 = malloc(sizeof(vmarg));
	t->arg2 = malloc(sizeof(vmarg));
	t->result = malloc(sizeof(vmarg));
	t->opcode = op;
	if (quad->arg1 && t->arg1){
		make_operand( quad->arg1, t->arg1 );
	}
	if (quad->arg2 && t->arg2){
		make_operand( quad->arg2, t->arg2 );
	}
	if (quad->result && t->result){
		make_operand( quad->result, t->result );
	}
		//printf("VAR SCOPE NAME2:%d\n",t->result->val);
	t->srcLine = quad->line ;
	quad->taddress = nextInstructionLabel();
	emitVmarg( t );
}
//XXX Vervelak
unsigned int currProcessedQuad(){
	return(processedQuad);
}


void addIncJump( unsigned instrAddress, unsigned instrNo ){
    incJump *tmp = malloc( sizeof( incJump ) );
    tmp->instrAddress = instrAddress;
    tmp->instrNo = instrNo;

    if( incJumpH == NULL ){
        tmp->next = NULL;
    }else{
        tmp->next = incJumpH;
    }

    incJumpH = tmp;

    return;
}

void patchIncJump(){
    incJump *tmp = malloc( sizeof( incJump ) );
    tmp = incJumpH;

    while( tmp != NULL ){
        if( tmp->instrAddress == nextquadlabel() ){
            VmargsTable[ tmp->instrNo ].result->val = nextInstructionLabel();
        }
        else{
            VmargsTable[ tmp->instrNo ].result->val = quads[ tmp->instrAddress].taddress;
        }
        tmp = tmp->next;
    }

    return;
} 

void generate_relational(vmopcode op,quad *quad){
	instruction *t = (instruction *)malloc(sizeof(instruction));
	t->arg1 = malloc(sizeof(vmarg));
	t->arg2 = malloc(sizeof(vmarg));
	vmarg *result = make_vmarg();
   
	t->opcode = op;
	if (quad->arg1)
		make_operand(quad->arg1,t->arg1);
	if (quad->arg2)
		make_operand(quad->arg2,t->arg2);
	result->type = label_a;
   
	t->srcLine = quad->line;
	if(quad->label < currProcessedQuad()){	
		result->val = (quads+quad->label)->taddress;
	}
	else{
		addIncJump(quad->label,nextInstructionLabel());
	}
	t->result = result;
	quad->taddress = nextInstructionLabel();
	t->srcLine = quad->line;
	emitVmarg(t);
}
//XXX /Vervelak

void generate_ADD(quad *quad ){
	generate( add_v, quad );
}

void generate_SUB(quad *quad ){
	generate( sub_v, quad );
}

void generate_MUL(quad *quad){
	generate( mul_v, quad );
}

void generate_DIV( quad *quad ){
	generate( div_v, quad );
}

void generate_MOD( quad *quad ){
	generate( mod_v, quad );
}

void generate_TABLECREATE( quad *quad ){
	generate( tablecreate_v, quad );
}

void generate_TABLEGETELEM(quad *quad){
	generate( tablegetelem_v, quad );
}

//XXX Vervelak
void generate_TABLESETELEM( quad *quad){
	generate(tablesetelem_v, quad);
}
//XXX /Vervelak

void generate_ASSIGN( quad *quad ){
	generate( assign_v, quad );
}

void generate_NOP(){
	instruction *t = malloc (sizeof(instruction));
	t->opcode = nop_v;
	emitVmarg( t );
}

void generate_JUMP( quad *quad ){
	generate_relational( jump_v, quad );
}

void generate_IF_EQ( quad *quad ){
	generate_relational( jeq_v, quad );
}

void generate_IF_NOTEQ( quad *quad ){
	generate_relational( jne_v, quad );
}

void generate_IF_GREATER( quad *quad ){
	generate_relational( jgt_v, quad );
}

void generate_IF_GREATEREQ( quad *quad ){
	generate_relational( jge_v, quad );
}

void generate_IF_LESS( quad *quad ){
	generate_relational( jlt_v, quad );
}

void generate_IF_LESSEQ( quad *quad ){
	generate_relational( jle_v, quad );
}

//XXX Vervelak

static void patchReturnList(rlist *Rlist,unsigned int label){
   rlist *tmp;
	tmp = Rlist;

   while(tmp != NULL){
      assert(tmp->rvalue < currInstruction);
      (VmargsTable[tmp->rvalue].result)->val = label;
      tmp = tmp->next;
   }
   return;
}

void generate_CALL( quad* quad ){
	quad->taddress = nextInstructionLabel();
	///print///printf("QUAD FUNC NUM: %d",quad->line );
	instruction *t = malloc( sizeof( instruction ) );
	t->result = malloc(sizeof(vmarg));
	t->opcode = call_v;
	make_operand( quad->result, t->result );
	reset_operand(t->arg1);
	reset_operand(t->arg2);
	t->srcLine = quad->line;
	
	t->result->val = FindFunctionUser((quad->result->sym)->name,deepestFunctionScope);
	if (t->result->val == -1){
		if(isLibFunction((quad->result->sym)->name)){
			int check = FindLibFunction((quad->result->sym)->name);
			if (check == -1){
				instruction *t1 =(instruction *) InitInstruction();
				expr * e = malloc(sizeof(expr));
				e->type = libraryfunc_e;
				e->sym =createS( (quad->result->sym)->name, 0, 0, (quad->result->sym)->name, 0, 0, "lib_func", 1,currscopespace(),currscopeoffset());
				make_operand(e,t1->arg1);
			}
			t->result->val = FindLibFunction((quad->result->sym)->name);
			t->result->type = libfunc_a;
			emitVmarg( t );
		}
	}
	else {
		emitVmarg( t );
	}
	
	
	

	return;
}

void generate_GETRETVAL( quad* quad ){
	quad->taddress = nextInstructionLabel();
	instruction *t = malloc( sizeof( instruction ) );
	t->result = malloc(sizeof(vmarg));

	t->opcode = getretval_v;
	make_operand( quad->result ,t->result);
	reset_operand(t->arg1);/////////Vervelak XXX
	reset_operand(t->arg2);
	t->srcLine = quad->line;

	emitVmarg( t );

	return;
}

void pushSym( Symbol *func ){
	funcs *fun = malloc( sizeof( funcs ) );

	fun->sym = func;
	fun->line = nextInstructionLabel()-1;
	fun->nodeR = NULL;

	if( fHead == NULL ){ 
		fun->next = NULL;
	}
	else{
		fun->next = fHead;
	}
	fHead = fun;

	return;
}

funcs *topSym(){
	return( fHead );
}

funcs *popSym(){
	funcs *tmp = malloc( sizeof( funcs ) );
	tmp = fHead;
	fHead = tmp->next;

	return( tmp );
}

rlist *appendR( rlist *rHead, unsigned int retLabel ){
	rlist *tmp = malloc( sizeof( rlist ) );
	tmp->rvalue = retLabel;

	if( rHead == NULL ){
		tmp->next = NULL;
	}
	else{
		tmp->next = rHead;
	}
	rHead = tmp;

	return( rHead );
}


instruction *InitInstruction(){
	instruction *t = (instruction *)malloc(sizeof(instruction));
	t->arg1 = malloc(sizeof(vmarg));
	t->arg2 = malloc(sizeof(vmarg));
	t->result = malloc(sizeof(vmarg));
	return t;
}

void generate_AND( quad *quad ){

	
	instruction *t = InitInstruction();
	instruction *t1 = InitInstruction();
	instruction *t2 = InitInstruction();
	instruction *t3 = InitInstruction();
	instruction *t4 = InitInstruction();

	quad->taddress = nextInstructionLabel();
	t->opcode = jeq_v;
	make_operand( quad->arg1, t->arg1 );
	make_booloperand( t->arg2, 0);
	t->result->type = label_a;
	t->result->val = nextInstructionLabel()+4;
	emitVmarg( t );

	t1->opcode = jeq_v;
   	make_operand(quad->arg2,t1->arg1);
   	make_booloperand(t1->arg2,0);
   	t1->result = make_vmarg();
   	(t1->result)->val = nextInstructionLabel()+3;
   	(t1->result)->type = label_a;
   	emitVmarg( t1 );	
	
	
	t2->opcode = assign_v;
   	make_booloperand(t2->arg1,1);
   	t2->arg2 = reset_operand();
   	make_operand(quad->result,t2->result);
   	t2->srcLine = quad->line;
   	emitVmarg(t2);	
	
	t3->opcode = jump_v;
  	t3->arg1 = reset_operand();
   	t3->arg2 = reset_operand();
   	t3->result = make_vmarg();
   	(t3->result)->type = label_a;
   	(t3->result)->val = nextInstructionLabel()+2;
   	t3->srcLine = quad->line;
   	emitVmarg(t3);

	t4->opcode = assign_v;
	   make_booloperand(t4->arg1,0);
	   t4->arg2 = reset_operand();
	   make_operand(quad->result,t4->result);
	   t4->srcLine = quad->line;
	   emitVmarg(t4);
	   return;
	
}

void generate_UMINUS(quad *quad){
	quad->taddress = nextInstructionLabel();
	instruction *t = (instruction *)malloc(sizeof(instruction));
	t->arg1 = malloc(sizeof(vmarg));
	t->result = malloc(sizeof(vmarg));

	t->opcode = mul_v;
	make_operand(quad->arg1, t->arg1);
	t->arg2 = make_integeroperand(-1);
	make_operand(quad->result, t->result);
	t->srcLine = quad->line;
	emitVmarg(t);
}

void generate_PARAM(quad *quad){
	quad->taddress = nextInstructionLabel();
	instruction *t = (instruction *)malloc(sizeof(instruction));
	t->result = malloc(sizeof(vmarg));

	t->opcode = pusharg_v;
	make_operand(quad->result, t->result);
	reset_operand(t->arg1);
	reset_operand(t->arg2);
	t->srcLine = quad->line;
	emitVmarg(t);
}

void generate_RETURN(quad *quad){
	quad->taddress = nextInstructionLabel();
	instruction *t = (instruction *)malloc(sizeof(instruction));
	instruction *t1 = (instruction *)malloc(sizeof(instruction)); 
	t->arg1 = malloc(sizeof(vmarg));  
	t->result = malloc(sizeof(vmarg));
	t1->result = malloc(sizeof(vmarg));
	funcs *fun;

	if(quad->result!=NULL){      
		t->opcode = assign_v;
		make_retvaloperand(t->result);
		make_operand(quad->result, t->arg1);
		reset_operand(t->arg2);
		t->srcLine = quad->line;
		emitVmarg(t);
	}

	fun = topSym();
	fun->nodeR = appendR(fun->nodeR,nextInstructionLabel());
	t1->opcode = jump_v;
	reset_operand(t1->arg1);
	reset_operand(t1->arg2);
	t->arg1 = malloc(sizeof(vmarg));
	make_retvaloperand(t1->result);
	(t1->result)->type = label_a;
	t1->srcLine = quad->line;
	emitVmarg(t1);
}

void generate_FUNCEND(quad *quad){
	funcs *fun;
	instruction *t = (instruction *)malloc(sizeof(instruction));
   	t->result = (vmarg *)malloc(sizeof(vmarg));
	fun = popSym();
	patchReturnList(fun->nodeR,nextInstructionLabel());
	(VmargsTable[fun->line].result)->val = nextInstructionLabel()+1;
    	
	quad->taddress = nextInstructionLabel();
	t->opcode = funcexit_v;
	make_operand(quad->result, t->result);
	reset_operand(t->arg1);
	reset_operand(t->arg2);
	t->srcLine = quad->line;
	t->result->val = quad->line;
	//fun->sym->line = quad->result->lvar; --
	//quad->faddress =  fun->sym->taddress;
	t->result->val = fun->sym->taddress;   //  --
	patchLocals(fun->sym->taddress, quad->result->lvar);
	//AddFunctionUserElem(fun->sym); 
	emitVmarg(t);
}



void generate_FUNCSTART( quad* quad ){
	Symbol *funSymbol = malloc( sizeof( Symbol ) );

	funSymbol = createS( quad->result->sym->name, 0, 0, quad->result->sym->name, 0, 0, "func", 0, 0, 0);
	instruction *t = malloc( sizeof( instruction ) );

	t->opcode = jump_v;
   	reset_operand(t->arg1);
   	reset_operand(t->arg2);
   	t->result = make_vmarg();
   	(t->result)->type = label_a;
   	(t->result)->val = nextInstructionLabel();
   	t->srcLine = quad->line;
	emitVmarg( t );

   	funSymbol = (quad->result)->sym;
	funSymbol->taddress = nextInstructionLabel();
	quad->taddress = nextInstructionLabel();
	funSymbol->line = 0;
	AddFunctionUserElem(funSymbol); 
	pushSym(funSymbol);
	
	instruction *t1 = (instruction *)malloc(sizeof(instruction));
	t1->result = (vmarg *)malloc(sizeof(vmarg));
	t1->opcode = funcenter_v;
	make_operand(quad->result, t1->result);
	reset_operand(t1->arg1);
	reset_operand(t1->arg2);
	t1->srcLine = quad->line;
	emitVmarg(t1);
}


void generate_OR( quad* quad ){

	quad->taddress = nextInstructionLabel();
	instruction *t = (instruction *)malloc(sizeof(instruction));
	t->arg1 = malloc(sizeof(vmarg));
	t->arg2 = malloc(sizeof(vmarg));
	t->result = malloc(sizeof(vmarg));

	instruction *t1 = (instruction *)malloc(sizeof(instruction));
	t1->arg1 = malloc(sizeof(vmarg));
	t1->arg2 = malloc(sizeof(vmarg));
	t1->result = malloc(sizeof(vmarg));
	
	instruction *t2 = (instruction *)malloc(sizeof(instruction));
	t2->arg1 = malloc(sizeof(vmarg));
	t2->arg2 = malloc(sizeof(vmarg));
	t2->result = malloc(sizeof(vmarg));

	instruction *t3 = (instruction *)malloc(sizeof(instruction));
	t3->arg1 = malloc(sizeof(vmarg));
	t3->arg2 = malloc(sizeof(vmarg));
	t3->result = malloc(sizeof(vmarg));
	
	instruction *t4 = (instruction *)malloc(sizeof(instruction));
	t4->arg1 = malloc(sizeof(vmarg));
	t4->arg2 = malloc(sizeof(vmarg));
	t4->result = malloc(sizeof(vmarg));		
	
	t->opcode = jeq_v;
	make_operand( quad->arg1, t->arg1 );
	make_booloperand( t->arg2, 1);
	t->result->type = label_a;
	t->result->val = nextInstructionLabel()+4;
	emitVmarg( t );

	t1->opcode = jeq_v;
   	make_operand(quad->arg2,t1->arg1);
   	make_booloperand(t1->arg2,1);
   	t1->result = make_vmarg();
   	(t1->result)->val = nextInstructionLabel()+3;
   	(t1->result)->type = label_a;
   	emitVmarg( t1 );	
	
	
	t2->opcode = assign_v;
   	make_booloperand(t2->arg1,0);
   	t2->arg2 = reset_operand();
   	make_operand(quad->result,t2->result);
   	t2->srcLine = quad->line;
   	emitVmarg(t2);	
	
	t3->opcode = jump_v;
  	t3->arg1 = reset_operand();
   	t3->arg2 = reset_operand();
   	t3->result = make_vmarg();
   	(t3->result)->type = label_a;
   	(t3->result)->val = nextInstructionLabel()+2;
   	t3->srcLine = quad->line;
   	emitVmarg(t3);

	t4->opcode = assign_v;
	   make_booloperand(t4->arg1,1);
	   t4->arg2 = reset_operand();
	   make_operand(quad->result,t4->result);
	   t4->srcLine = quad->line;
	   emitVmarg(t4);
	   return;
}

void generate_NOT( quad *quad ){
	quad->taddress = nextInstructionLabel();
	instruction *t = InitInstruction();
	instruction *t1 = InitInstruction();
	instruction *t2 = InitInstruction();
	instruction *t3 = InitInstruction();

	t->opcode = jeq_v;
	make_operand( quad->arg1, t->arg1 );
	make_booloperand( t->arg2, 1 );
	t->result->type = label_a;
	t->result->val = nextInstructionLabel()+3;
	emitVmarg( t );

	t1->opcode = assign_v;
	make_booloperand( t1->arg1, 1 );
	//reset_operand( t->arg2 );
	make_operand( quad->result, t1->result );
	emitVmarg( t1 );

	t2->opcode = jump_v;
	//reset_operand( t1->arg1 );
	//reset_operand( t1->arg2 );
	t2->result->type = label_a;
	t2->result->val = nextInstructionLabel()+2;
	emitVmarg( t2 );

	t2->opcode = assign_v;
	make_booloperand( t2->arg1, 0 );
	reset_operand( t2->arg2 );
	make_operand( quad->result, t2->result );
	emitVmarg( t2 );
}



void printConsts( FILE *fp ){
	int i = 0;
	//printf("afsdfsdfsdfsdsd: %d \n",constArrayFilled);
	while(i<constArrayFilled){
		///print///printf("agsdgsdfsdfsdsd: %d \n", constArray[ i ].type);
		if( constArray[ i ].type == 4 ){ // gia const int
			
			fprintf( fp, "%d: %ld\n", i, constArray[ i ].value->inttype );
		}
		else if( constArray[ i ].type == 5 ){
			fprintf( fp, "%d: %Lf\n", i, constArray[ i ].value->doubletype );
		}
		else if( constArray[ i ].type == 6 ){
			fprintf( fp, "%d: %s\n", i, constArray[ i ].value->strtype );
		}
		++i;
	}
	fprintf( fp, "\n\n\n");
}

void printLibFs( FILE *fp ){
	int i;
	for(i=0; i<libFunctionsFilled; i++){
      fprintf(fp, "\n%d: %s", i, libFunctions[ i ]->name);
   }
   fprintf( fp, "\n\n\n");
}


void printUserFs( FILE *fp ){
   int i;
   for(i=0; i<functionUserFilled; i++){ 
      fprintf( fp, "\n%d: %s %d %d ", i, functionUser[ i ]->name, functionUser[ i ]->taddress, functionUser[ i ]->line);
   }
   fprintf(fp,"\n\n\n");
   return;
}

/*
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
*/
void vmType(vmarg *arg,FILE *fp){
   	char *types[12] = { "LABEL","GLOBAL","FORMAL","LOCAL","INTEGER",
      "DOUBLE","STRING","BOOL","NIL","USERFUNC","LIBFUNC","RETVAL"
   	};   
      
   	if(arg==NULL){
      	return;
   	}
   	assert(arg);
   	vmarg_t vmType;
   	vmType = arg->type;
   	switch (vmType){
	case label_a :
      	case global_a :
      	case formal_a :
      	case integer_a:
      	case double_a: 
      	case userfunc_a:
      	case local_a:
	 	{		  
	    	fprintf(fp ,"%s:%d\t ", types[ vmType ] , arg->val );
	 	}
	 	break;	 
      	case retval_a:
	 	fprintf(fp,"%s\t ", types[ vmType ] );
	 	break;	 
      	case string_a :
      	case libfunc_a:
	 	fprintf(fp,"%s:%d\t ", types[ vmType ] , arg->val );
	 	break;
      	case nil_a:
	 	break;
      	case bool_a:
		///print///printf("BOOLEAN VALUE HERE : %d \n",arg->val);
	 	fprintf(fp,"%s:%s\t ", types[ vmType ] , (arg->val == 48 || arg->val == 0) ? "FALSE":"TRUE" );	 
	 	break;

      	default: //return; //TODO: Thelei fix edw.
	 	assert(0);

   	}
}

/*
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
*/
void printBin(){
	FILE *fp;
//	unsigned int i=0,length, j;
//	int op, number, unary=-1;
//	char *str;
//	user_functions *fun;
//	instruction *instruction;
	unsigned int magicNumber = 4;
   
	if(!(fp = fopen("alpha.bin","wb+"))){
		fprintf(stderr, "Cannot open file: alpha.bin\n");
		return;
	}
	fwrite(&magicNumber,sizeof(unsigned int),1,fp);
	fwrite(&constArrayFilled,sizeof(unsigned int),1,fp);
	//fwrite(&functionUserFilled,sizeof(unsigned int), 1, fp);
	//fwrite(&libFunctionsFilled,sizeof(unsigned int), 1, fp);
    	
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int length = 0;
	unsigned int tmp = 0;
	for( i=0; i<constArrayFilled; i++ ){
		if( constArray[ i ].type == 4 ){ 
			j++;
		}
	}
	fwrite(&j,sizeof(unsigned int),1,fp);
	long int intTmp = 0;
	for( i=0; i<constArrayFilled; i++ ){
		if( constArray[ i ].type == 4 ){
			fwrite(&i,sizeof(unsigned int),1,fp);
			intTmp = constArray[i].value->inttype;
			fwrite(&intTmp,sizeof(long int),1,fp);
		}
	}
	j= 0;
	for( i=0; i<constArrayFilled; i++ ){
		if( constArray[ i ].type == 5 ){
			j++;
		}
	}
	fwrite(&j,sizeof(unsigned int),1,fp);
	long double doubleTmp= 0;
	for( i=0; i<constArrayFilled; i++ ){
		if( constArray[ i ].type == 5 ){
			fwrite(&i,sizeof(unsigned int),1,fp);
			doubleTmp = constArray[i].value->doubletype;
			fwrite(&doubleTmp,sizeof(long double),1,fp);
		}
	}
	j =0;
	for( i=0; i<constArrayFilled; i++ ){
		if( constArray[ i ].type == 6 ){
			j++;
		}
	}
	fwrite(&j,sizeof(unsigned int),1,fp);
	for( i=0; i<constArrayFilled; i++ ){
		if( constArray[ i ].type == 6 ){
			length=strlen(constArray[ i ].value->strtype);
			fwrite(&i,sizeof(unsigned int),1,fp);
			fwrite(&length,sizeof(unsigned int),1,fp);
			fwrite(constArray[ i ].value->strtype ,length*sizeof(char),1,fp);
		}
	}
	fwrite(&functionUserFilled,sizeof(unsigned int),1,fp);
	unsigned int taddressTmp;
	unsigned int lineTmp;
	for(i=0; i<functionUserFilled; i++){ 
		length=strlen(functionUser[ i ]->name);
		fwrite(&length,sizeof(unsigned int),1,fp);
		fwrite(functionUser[ i ]->name ,length*sizeof(char),1,fp);
		taddressTmp = functionUser[i]->taddress;
		fwrite(&taddressTmp,sizeof(unsigned int),1,fp);
		lineTmp = functionUser[i]->line;
		fwrite(&lineTmp,sizeof(unsigned int),1,fp);
	}
	fwrite(&libFunctionsFilled,sizeof(unsigned int),1,fp);
	for(i=0; i<libFunctionsFilled; i++){
		length=strlen( libFunctions[ i ]->name);
		fwrite(&length,sizeof(int),1,fp);
		fwrite( libFunctions[ i ]->name ,length*sizeof(char),1,fp);
	}
	



	tmp = nextInstructionLabel()-1; 
	fwrite(&tmp,sizeof(unsigned int),1,fp);
	int type = 0;
	for(i=0; i<nextInstructionLabel()-1; i++){
		if((( ( VmargsTable[i].opcode>=add_v)&&(VmargsTable[i].opcode<=or_v))||
				(VmargsTable[i].opcode==tablegetelem_v)||
				(VmargsTable[i].opcode==tablesetelem_v))||(
				(VmargsTable[i].opcode>=jeq_v)&& (VmargsTable[i].opcode<=jgt_v))){
			type = 5;
			fwrite(&type,sizeof(int),1,fp);
			fwrite(&VmargsTable[i].opcode, sizeof(int),1,fp);
			fwrite(&VmargsTable[i].result->type,sizeof(int),1,fp);
			fwrite(&VmargsTable[i].result->val,sizeof(unsigned int),1,fp);
			fwrite(&VmargsTable[i].arg1->type,sizeof(int),1,fp);
			fwrite(&VmargsTable[i].arg1->val,sizeof(unsigned int),1,fp);
			fwrite(&VmargsTable[i].arg2->type,sizeof(int),1,fp);
			fwrite(&VmargsTable[i].arg2->val,sizeof(unsigned int),1,fp);
			fwrite(&VmargsTable[i].srcLine,sizeof(unsigned int),1,fp);
		}
		else if( ( VmargsTable[i].opcode>=assign_v) && ( VmargsTable[i].opcode<=not_v ) ){
			type = 4;
			fwrite(&type,sizeof(int),1,fp);
			fwrite(&VmargsTable[i].opcode, sizeof(int),1,fp);
			fwrite(&VmargsTable[i].result->type,sizeof(int),1,fp);
			fwrite(&VmargsTable[i].result->val,sizeof(unsigned int),1,fp);
			fwrite(&VmargsTable[i].arg1->type,sizeof(int),1,fp);
			fwrite(&VmargsTable[i].arg1->val,sizeof(unsigned int),1,fp);
			fwrite(&VmargsTable[i].srcLine,sizeof(unsigned int),1,fp);
		}
		else if(( VmargsTable[i].opcode==jump_v )||( ( VmargsTable[i].opcode>=call_v ) && ( VmargsTable[i].opcode<=tablecreate_v ) )/*||( (iHead[i]).result != NULL )*/){
			type = 3;
			fwrite(&type,sizeof(int),1,fp);
			fwrite(&VmargsTable[i].opcode, sizeof(int),1,fp);
			fwrite(&VmargsTable[i].result->type,sizeof(int),1,fp);
			fwrite(&VmargsTable[i].result->val,sizeof(unsigned int),1,fp);
			fwrite(&VmargsTable[i].srcLine,sizeof(unsigned int),1,fp);
		}
	}
	fclose(fp);
	/*
	for(i=0; i<nextinstructionlabel()-1 ; i++){
		op = iHead[i].opcode;
		fprintf(fp, "%5u: %s\t\t ", i, opcodes[op]);
		fflush(fp);
		if( ( ( op>=add_v ) && ( op<=or_v ) ) ||  ( op=tablegetelem_v ) || ( op=tablesetelem_v )  ){
			vmType( (iHead[i]).arg1 , fp );
	 		vmType( (iHead[i]).arg2 , fp );
	 		vmType( (iHead[i]).result , fp );
		}
		else if( ( op>=jeq_v ) && ( op<=jgt_v ) ){
	 		vmType( (iHead[i]).arg1 , fp );
	 		vmType( (iHead[i]).arg2 , fp );
	 		vmType( (iHead[i]).result , fp );
		}
		else if( ( op>=assign_v) && ( op<=not_v ) ){
	 		vmType( (iHead[i]).arg1 , fp );
	 		vmType( (iHead[i]).result , fp );
		}
		else if( op==jump_v ){
	 		vmType( (iHead[i]).result , fp );
		}
		else if( ( op>=call_v ) && ( op<=newtable_v ) ){
	 		vmType( (iHead[i]).result , fp );
		}
		else{
	 		if( (iHead[i]).result != NULL ){
	    		vmType( (iHead[i]).result , fp );
	 		}
		}
		fprintf( fp , "\n\n\n" );
	}*/



}
void printFun(){
	FILE *fp = malloc(sizeof(FILE));
	int i;
	if( ( fp = fopen("instructions.txt", "w") ) == NULL ){
		printf("Error opening file: instructions.txt" );
	}
	vmopcode op;
	char *opcodes[26] = { "ASSIGN","ADD","SUB","MUL","DIV","MOD","UMINUS","AND","OR",
      "NOT","IF_EQ","IF_NOT_EQ","IF_LESSEQ","IF_GREATEREQ",
      "IF_LESS","IF_GREATER","CALL","PUSHARG","RET","GETRETVAL","FUNCENTER","FUNCEXIT",
      "JUMP","TABLECREATE","TABLEGETELEM","TABLESETELEM"}; 

    printConsts(fp);
    printLibFs(fp);
    printUserFs(fp);


    for(i=0; i<nextInstructionLabel()-1 ; ++i){
     	op = VmargsTable[i].opcode;
  	//printf("OOOOP:%d\n",assign_v);
      	fprintf(fp, "%5u: %s\t\t ", i, opcodes[op]);
      	fflush(fp);
      	if( 	( op==add_v ) || 
		( op==sub_v ) ||
		( op==mul_v ) ||
		( op==div_v ) || 
		( op==mod_v ) ||
		( op==and_v ) ||    
		( op==or_v )  ||
	 	( op==tablegetelem_v ) ||
 		( op==tablesetelem_v )
	){
	 		vmType( (VmargsTable[i]).arg1 , fp );
	 		vmType( (VmargsTable[i]).arg2 , fp );
	 		vmType( (VmargsTable[i]).result , fp );
      	}
      	else if(( op==jeq_v ) ||
		( op==jne_v ) ||
		( op==jle_v ) ||
		( op==jge_v ) ||
		( op==jlt_v ) ||	
		( op==jgt_v ) ){
	 		vmType( (VmargsTable[i]).arg1 , fp );
	 		vmType( (VmargsTable[i]).arg2 , fp );
	 		vmType( (VmargsTable[i]).result , fp );
      	}
      	else if( ( op==assign_v) || (op==uminus_v) || ( op==not_v ) ){
	 		vmType( (VmargsTable[i]).arg1 , fp );
	 		vmType( (VmargsTable[i]).result , fp );
      	}
      	else if( op==jump_v ){
	 		vmType( (VmargsTable[i]).result , fp );
      	}
      	else if( ((op==call_v)		||
		(op==pusharg_v) 	||
		(op==getretval_v) 	|| 
		(op==funcenter_v) 	|| 
		(op==funcexit_v) 	|| 
		(op==tablecreate_v))){
	 		vmType( (VmargsTable[i]).result , fp );
      	}
      	else{
	 		if( (VmargsTable[i]).result != NULL ){
	    		vmType( (VmargsTable[i]).result , fp );
	 		}
      	}
      	fprintf( fp , "\n\n\n" );
   }
   
   fclose(fp);

}

void generation(void){
   unsigned int processedquad=0;   
   for(processedquad=0; processedquad<currQuad; processedquad++){
      (*generators[quads[processedquad].op])(quads+processedquad);
   }
   patchIncJump();
   generate_NOP(NULL);
   return;
}

