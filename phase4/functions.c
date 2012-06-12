#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"
#include "symbolt.h"
#include "assert.h"




unsigned int tempcounter;
extern int   deepestScope;
extern int deepestFunctionScope;
unsigned int globalvarOffset = 0;
unsigned int functionLocalOffset = 0;
unsigned int formalArgOffset = 0;
unsigned int scopeSpaceCounter = 1;
//extern char * a_buf;
//extern char * b_buf;

quad*		quads = NULL;
unsigned 	total = 0;
unsigned int 	currQuad = 0;


void expand(void){
	assert(total==currQuad);
	quad* p = (quad*) malloc(NEW_SIZE);
	if (quads){
		memcpy(p, quads, CURR_SIZE);
		free(quads);
	}
	quads = p;
	total += EXPAND_SIZE;
}




void emit(iopcode op ,expr * arg1, expr * arg2, expr * result, int label, unsigned int line){
	if(currQuad == total){
		expand();
	}
	if((quads+currQuad)==NULL){
		return;
	}
			
	quad *p = quads + currQuad;
	p->op = op;
	p->arg1 = arg1;
	p->arg2 = arg2;
	p->result = result;
	p->label = label;
	p->line = line;
	currQuad++;

}

char *newtempname(void){
	char * tempname = malloc(STR_SIZE); 
	sprintf( tempname , "_t%d" , tempcounter++);
	return tempname;
}

void resettemp(void){
	tempcounter = 0;
}
Symbol *newtemp(void){
	char *name = newtempname();
	///print///printf("BIKE EDW\n");
	Symbol *sym = sLookUpH(name,deepestFunctionScope);
	if( sym ){
		//inccurrscopeoffset();
		//sym->offset = currscopeoffset(); //Isws xreiazetai.
		//sym->spscope = currscopespace();
		return sym;
	}
	else{
		inccurrscopeoffset();
		
		Symbol *retval =  createS(name , deepestScope , 0 ,topfStackName(), deepestFunctionScope , 0 , "var" , 1,currscopespace(),currscopeoffset());
		insertH(retval); //TODO: kainourgia periptwsi:tmpvar
		return retval;	
	}
}

Symbol *sLookUpH(char *name, unsigned int currFScope){ //NA ILOPOIITHEI STO hashTable.h
	unsigned int hashpos =  hashF(name);
	Symbol *temp;
	
	if( hashtable[ hashpos ] != NULL ){
		temp = hashtable[ hashpos ];
	}
	else{
		return NULL;
	} 
	
	while( temp != NULL ){
		if( !strcmp( temp->name , name ) && (temp->scopeF == currFScope || temp->scopeF == 0) ){
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

scopespace_t currscopespace( void ){
	if( scopeSpaceCounter == 1){
		return globalvar;
	}
	else if( scopeSpaceCounter % 2 == 0){
		return formalarg;
	}
	else{
		return functionlocal;
	}
}

unsigned int currscopeoffset( void ){
	switch( currscopespace() ){
		case globalvar		: return globalvarOffset;
		case functionlocal 	: return functionLocalOffset;
		case formalarg		: return formalArgOffset;
		default			: assert(0);
	}
}

void inccurrscopeoffset( void ){
	switch( currscopespace() ){
		case globalvar		: ++globalvarOffset; break;
		case functionlocal 	: ++functionLocalOffset; break;
		case formalarg		: ++formalArgOffset; break;
		default			: assert(0);
	}
}

void decrcurrscopeoffset( void ){
	switch( currscopespace() ){
		case globalvar		: --globalvarOffset; break;
		case functionlocal 	: --functionLocalOffset; break;
		case formalarg		: --formalArgOffset; break;
		default			: assert(0);
	}
}
void enterscopespace( void ){
	++scopeSpaceCounter;
}

void exitscopespace( void ){
	//assert(scopeSpaceCounter > 0 );
	//printf("sadfdsfgdfgsfdgdfgdfgdfsg%d",scopeSpaceCounter); 
	--scopeSpaceCounter;
}

unsigned int nextquadlabel( void ){
	return currQuad;
}

void resetformalargoffset( void ){
	formalArgOffset = 0;
}

void resetfunctionlocaloffset( void ){
	functionLocalOffset = 0;
}

void restorecurrscopeoffset( unsigned int n ){
	switch( currscopespace()){
		case globalvar		: globalvarOffset = n; break;
		case functionlocal 	: functionLocalOffset = n; break;
		case formalarg 		: formalArgOffset = n; break;
		default				: assert(0);
	}
}

expr *lvalue_expr(Symbol *sym){   
   assert(sym);
   //printf("%s\n",sym->type);
   //assert(sym->type==NULL);
   expr *tmp;
   tmp = (expr *)malloc(sizeof(expr));
   
   tmp -> next = NULL;
   tmp -> sym = sym;
   if (!strcmp(sym->type, "func")){
	tmp->type = programfunc_e;
	///print///printf("TEST:program func\n");
   }
   else if (!strcmp(sym->type, "lib_func")){
	tmp->type = libraryfunc_e;
	///print///printf("TEST:library func\n");
   } 
   else if (!strcmp(sym->type, "var")){
	tmp->type = var_e;
	///print///printf("TEST:program var\n");
   }
   
   return(tmp);
}

expr *numConst_expr(int numConst){
	expr *tmp = (expr *) malloc(sizeof(expr));
	tmp->numConst = numConst;
	return tmp;


}

expr *numStrConst_expr(int numConst, char * strConst){
	expr *tmp = (expr *) malloc(sizeof(expr));
	tmp->numConst = numConst;
	tmp->strConst = malloc (100);
	sprintf(tmp->strConst,"%s",strConst);
	return tmp;


}



expr *strConst_expr(char * strConst){
	expr *tmp = (expr *) malloc(sizeof(expr));
	///print///printf("PRAKSEIS STRCONST%s\n",strConst);
	tmp->strConst = strdup(strConst);
	return tmp;


}

expr * emitrelop(iopcode op, expr * arg1, expr * arg2, unsigned int line){
	expr *tmp ;
	tmp = newexpr_t(boolexpr_e);
	tmp->sym = newtemp();
	emit(op, arg1, arg2, NULL, (int)(currQuad +3), line);
	emit(assign, constBool_expr('0'), NULL, tmp, 0, line);
	emit(jump, NULL, NULL, NULL, (int)(currQuad +2), line);
	emit(assign, constBool_expr('1'), NULL, tmp, 0, line);
	return (tmp);
}
expr * emitlogic(iopcode op, expr * arg1, expr * arg2, unsigned int line){
	expr *tmp ;
	tmp = newexpr_t(boolexpr_e);
	tmp->sym = newtemp();
	emit(op, arg1, arg2, tmp, 0 , line);
	return (tmp);
}


//TODO:dilwseis mesa eis to .h


expr * constInt_expr(int num){
	expr * tmp = (expr *) malloc (sizeof(expr));
	tmp->type = constint_e;
	tmp->intConst = num;
	return (tmp);
}

expr * constDouble_expr(double num){
	expr * tmp = (expr *) malloc (sizeof(expr));
	tmp->type = constdouble_e;
	tmp->numConst = num;
	return (tmp);
}

expr * constString_expr(char * str){
	expr * tmp = (expr *) malloc (sizeof(expr));
	assert (str);
	tmp->type = conststring_e;
	tmp->strConst = strdup(str);
	return (tmp);
}

expr * constBool_expr( unsigned char boole){
	expr * tmp = (expr *) malloc (sizeof(expr));
	tmp->type = constbool_e;
	tmp->boolConst = boole;
	return (tmp);
}

void backpatch(unsigned quadNumber,unsigned int label){
	quads[quadNumber].label = label;
}


expr *newexpr_t(expr_t type){
   expr *tmp = (expr *)malloc(sizeof(expr));
   tmp->type = type;
   return(tmp);
}


expr *createMember(expr* value, char* name, int line){
	value=emit_iftableitem(value, line);
	expr *tableitem=newexpr_t(tableitem_e);
	tableitem->sym=value->sym;
	tableitem->index=constString_expr(name);
	return tableitem;
}

expr *func_call(expr *lval, expr *args, unsigned int line){
	expr *functionName = emit_iftableitem(lval, line);
	expr * tmp=args;
	expr * paramTable[1024];
	int i = 0;
	while(tmp!=NULL){
		paramTable[i] = tmp;
		i++;
		//emit(param,NULL,NULL, tmp, -1, line);
		tmp=tmp->next;
		assert(i<1024);
	}
	i--;
	for (i; i>=0; i--){
		emit(param,NULL,NULL, paramTable[i],-1,line);
	}
		
	emit(call, NULL, NULL,functionName, -1, line);
	expr *result = newexpr_t(var_e);
	result -> sym = newtemp();
	emit(getretval, NULL, NULL, result, -1, line);
	return result;
}

expr *arith_emit(iopcode op, expr *arg1 , expr *arg2,int line){
   expr *result ;

   
   result = newexpr_t(arithexpr_e);

   result->sym = newtemp();

   emit(op, arg1, arg2,result, -1, line);
	  
   return(result);
}

int isFunction(Symbol * sym, char * type1, char * type2){
	if (lookUpH(sym,type1,type2)!=NULL){	
		return 1;
	}
	return 0;
}

expr* emit_iftableitem( expr* e ,unsigned int line){
	if( e->type != tableitem_e ){
		return e;
	}
	else{
		expr* result =(expr *) newexpr_t( var_e );
		result->sym = newtemp();
		emit( tablegetelem , e , e->index , result, -1 , line );
		return result;
	}
}

expr *emit_table(expr *arg1,expr *arg2,int line){
      expr *tmp = emit_iftableitem(arg1,line);
      expr *new = newexpr_t(tableitem_e);
      new->sym = tmp->sym;
      new->index = arg2;
      return(new);
}


int istemp(Symbol *sym){
   char tmp[2];
   
   tmp[0] = sym->name[0];
   tmp[1] = '\0';
  
   if(!strcmp(tmp,"_s")){
      return(1);
   }
   
   
   return(0);
}


static char *print_expr(expr * expression){
   if (!expression) return " ";
   expr_t type = expression->type;
   char *tmp;
   tmp=malloc(1000*sizeof(char));

   
   if (type==var_e){
      return (expression->sym)->name;
   }else if(type==tableitem_e){
      return (expression->sym)->name;
   }else if(type==libraryfunc_e){
      return (expression->sym)->name;
   }else if(type==arithexpr_e){
      return (expression->sym)->name;
   }else if(type==constint_e){
     sprintf(tmp, "%d", expression->intConst);
     return tmp;
   }else if(type==constdouble_e){
      sprintf(tmp, "%lf", expression->numConst);
      return tmp;
   }else if(type==constbool_e){
            if (expression->boolConst=='0')return "FALSE";
            else return "TRUE";
   }else if(type==conststring_e){
           char *tmp = malloc(strlen(expression->strConst)+2);
           tmp[0]= '\"';
	   tmp[1] ='\0';
	   tmp = strcat(tmp,expression->strConst);
	   tmp[strlen(tmp)-1] = '\"';
	   return tmp;
           // return expression->strConst;
   }else if(type==newtable_e){
                  return (expression->sym)->name;
   }else if(type==nil_e){
      return "NIL";
   }else if(type==programfunc_e){
      return (expression->sym)->name;
   }else if(type==boolexpr_e){
      return (expression->sym)->name;
   }else if(type==assignexpr_e){
      return (expression->sym)->name;
   }else{
	///print///printf("RESULT:%d\n", type); 
      assert(0);
   }
}

void printquads(void){
   FILE *icode_export ;
   int i ;
   iopcode op;
   char *table_op[26] = {"ASSIGN","ADD","SUB","MUL","DIV","MOD","UMINUS","AND","OR","NOT","IF_EQ","IF_NOTEQ",
      "IF_LESSEQ","IF_GREATEREQ","IF_LESS","IF_GREATER","CALL",
      "PARAM","RETURN","GETRETVAL","FUNCSTART","FUNCEND","JUMP","TABLECREATE","TABLEGETELEM",
      "TABLESETELEM"};   
   
   if ((icode_export = fopen("icode.txt","w"))==NULL){
      icode_export = stderr;   
   }
   
   
   for(i=0; i<currQuad; i++){
      op = (quads[i]).op;
      fprintf(icode_export,"%3u: %s\t ",i,table_op[op]);
      if ((op != tablecreate && op != tablesetelem) && op !=tablegetelem ){
      		fprintf(icode_export,"\t");
      }
      fflush(icode_export);
      
      if(
	(op==add)  		||
	(op==and)  		||
	(op==or)  		||
	(op==sub)  		||
	(op==mul)  		||
	(op==divide)  		||
	(op==mod)  		||
	(op==tablegetelem)  	||
	(op==tablesetelem))
	{
	 fprintf(icode_export,"%s\t ", print_expr(quads[i].arg1));
	 fprintf(icode_export,"%s\t ", print_expr(quads[i].arg2));
	 fprintf(icode_export,"%s\t ", print_expr(quads[i].result));
      }
      else if(
	(op==if_greater)	|| 
	(op==if_greatereq) 	||
	(op==if_less) 		||
	(op==if_lesseq) 	||
	(op==if_not_eq) 	||    
	(op==if_eq))
	{
	 fprintf(icode_export,"%s\t ", print_expr(quads[i].arg1));
	 fprintf(icode_export,"%s\t ", print_expr(quads[i].arg2));
	 fprintf(icode_export,"%d ", quads[i].label);
     	}
	
      else if ((op==not) 	|| 
	(op==uminus)		||
	(op==assign))
	{
	 fprintf(icode_export,"%s\t ", print_expr(quads[i].arg1));
	 fprintf(icode_export,"%s\t ", print_expr(quads[i].result));
      }
      else if(op==jump){
	 fprintf(icode_export,"%d ", quads[i].label);	 
      } 
      else if((op==call)	||
	(op==param) 		||
	(op==getretval) 	|| 
	(op==funcstart) 	|| 
	(op==funcend) 		|| 
	(op==tablecreate))
	{	
	 fprintf(icode_export,"%s\t ", print_expr(quads[i].result));
      }
      else{
	 if(quads[i].result!= NULL){
	    fprintf(icode_export,"%s\t ", print_expr(quads[i].result));
	 }
      }
      fprintf(icode_export,"\n");
   }
   fclose(icode_export);
   return;
}


/****Listes kai Stacks ******/


node *  initStack(){
	node *tmp = malloc(sizeof(node));
	tmp->next = NULL;
	return tmp;
}

void    pushStack(node * stack, unsigned int number){
	node *tmp;
	node *newnode = malloc(sizeof(node));
	newnode->next = NULL;
	newnode->info = number;
	tmp = stack;
	while (tmp->next){
		tmp = tmp->next;
	}
	tmp->next = newnode;
}

node *  popStack(node * stack){
	if (isEmptyStack(stack) == 1){
		return NULL;
	}
	else {
		node *tmp;
		node *returnNode;
		tmp = stack;
		while (tmp->next){
			returnNode = tmp;
			tmp = tmp->next;
		}
		returnNode->next = NULL;
		return tmp;
	}
}

node *  topStack(node * stack){
	if (isEmptyStack(stack) == 1){
		return NULL;
	}
	else {
		node *tmp;
		tmp = stack;
		while (tmp->next){
			tmp = tmp->next;
		}
		return tmp;
	}
}


int isEmptyStack(node * stack){
	if (stack->next == NULL){
		return 1;
	}
	else {
		return 0;
	}
}

node *  initList(){
	node *tmp = malloc(sizeof(node));
	tmp->next = NULL;
	return tmp;

}

void    insertList(node * lista, unsigned int number){
	node *tmp;
	node *newnode = malloc(sizeof(node));
	newnode->next = NULL;
	newnode->info = number;
	tmp = lista;
	while (tmp->next){
		tmp = tmp->next;
	}
	tmp->next = newnode;
}

node *  mergeLists(node * lista, node * lista2){
	if ((isEmptyList(lista) == 0) && (isEmptyList(lista2) == 0)){
		node *tmp = lista;
		while (tmp->next){
			tmp = tmp->next;
		}
		tmp->next = lista2->next;
		return lista;
	}
	else if(isEmptyList(lista) == 1){
		return lista2;
	}
	else if(isEmptyList(lista2) == 1){
		return lista;
	}
	else {
		return NULL;
	}
}

int	isEmptyList(node *lista){
	if (lista->next == NULL){
		return 1;
	}
	else {
		return 0;
	}							
}


specialJumpStruct * initSpecialJumpStack(){
	specialJumpStruct *tmp = malloc(sizeof(specialJumpStruct));
	tmp->next = NULL;
	return tmp;
}


void pushSpecialJumpStack(specialJumpStruct *stack, unsigned int start, unsigned int jump){
	specialJumpStruct *tmp;
	specialJumpStruct *newnode = malloc(sizeof(specialJumpStruct));
	newnode->next = NULL;
	newnode->start = start;
	newnode->jump = jump;
	tmp = stack;
	while (tmp->next){
		tmp = tmp->next;
	}
	tmp->next = newnode;
}


specialJumpStruct * topSpecialJumpStack(specialJumpStruct *stack){
	if (isEmptySpecialJumpStack(stack) == 1){
		return NULL;
	}
	else {
		specialJumpStruct *tmp;
		specialJumpStruct *returnNode;
		tmp = stack;
		while (tmp->next){
			returnNode = tmp;
			tmp = tmp->next;
		}
		return tmp;
	}
}


specialJumpStruct * popSpecialJumpStack(specialJumpStruct *stack){
	if (isEmptySpecialJumpStack(stack) == 1){
		return NULL;
	}
	else {
		specialJumpStruct *tmp;
		specialJumpStruct *returnNode;
		tmp = stack;
		while (tmp->next){
			returnNode = tmp;
			tmp = tmp->next;
		}
		returnNode->next = NULL;
		return tmp;
	}
}


int isEmptySpecialJumpStack(specialJumpStruct *stack){
	if (stack->next == NULL){
		return 1;
	}
	else {
		return 0;
	}
}



offsetStruct * initOffsetStack(){
	offsetStruct *tmp = malloc(sizeof(offsetStruct));
	tmp->next = NULL;
	return tmp;
}
void pushOffsetStack(offsetStruct *stack, scopespace_t space){
	offsetStruct *tmp;
	offsetStruct *newnode = malloc(sizeof(offsetStruct));
	newnode->next = NULL;
	newnode->scopespace = space;
	tmp = stack;
	while (tmp->next){
		tmp = tmp->next;
	}
	tmp->next = newnode;
}
scopespace_t topOffsetStack(offsetStruct *stack){
	if (isEmptyOffsetStack(stack) == 1){
		assert(isEmptyOffsetStack(stack) ==1 );
	}
	else {
		offsetStruct *tmp;
		offsetStruct *returnNode;
		tmp = stack;
		while (tmp->next){
			returnNode = tmp;
			tmp = tmp->next;
		}
		return tmp->scopespace;
	}
}
scopespace_t popOffsetStack(offsetStruct *stack){
	if (isEmptyOffsetStack(stack) == 1){
		assert(isEmptyOffsetStack(stack) ==1 );
	}
	else {
		offsetStruct *tmp;
		offsetStruct *returnNode;
		tmp = stack;
		while (tmp->next){
			returnNode = tmp;
			tmp = tmp->next;
		}
		returnNode->next = NULL;
		return tmp->scopespace;
	}
}
int isEmptyOffsetStack(offsetStruct *stack){
	if (stack->next == NULL){
		return 1;
	}
	else {
		return 0;
	}
}


/***********/


