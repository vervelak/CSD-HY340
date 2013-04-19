%{
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "symbolt.h"
#include "functions.h"
 
int scope_flag = 1; //pote anoigei block kai  anevainei to scope kai to scopeF
int function_flag = 0; //pote exoume function.
int flag_lvalue_used = 0;
int funcdef_flag =0;
int call_flag = 0;
int func_call_flag=0;
char * f_name ;
char * tmp_buf ;
int f_prefix =0;
int flag_trasp=0;
int first_flag = 0;
char * a_buf = NULL;
char * b_buf = NULL;	
extern int yylex(void);
extern int yylineno;
extern char *yytext;
extern FILE* yyin;
Symbol * tmp;
Symbol * tmpnext;
extern Symbol *hashtable[ HASH_SIZE ];
extern scopespace_t scopeSpaceCounter;
node * loopStack;
specialJumpStruct *tmpstack;

extern quad*		quads;
extern unsigned int	total;
extern unsigned int 	currQuad;

int isLocalvar=0;
int deepestFunctionScope = 0;
int deepestScope = 0;
int loopScope = 0; //gia na elegxoume an iparxei break ekso apo kapoio loop
int yyerror(const char * yaccProvidedMessage);
int lineF;
offsetStruct *offsetStack;


node * breakWhileStack;
//node * continueStack;
node * quadLoopStack;

specialJumpStruct *breakSpecialStack;
specialJumpStruct *continueSpecialStack;

struct forprefix{
	int enter;
	int test;
};

%}
%error-verbose
%start program

%union{
	int intval;
	char* strval;
	double doubleval;
	struct symT * symval;
	struct expression * exprval;
	struct forprefix *forval;
}

%token <intval> INTEGER;
%token <strval> STRING;
%token <strval> IDENTIFIER;
%token <doubleval> REALNUMBER;

%token <strval> WHILE FOR IF ELSE FUNCTION RETURN BREAK CONTINUE AND OR NOT LOCAL TRUE FALSE NIL PLUS MINUS ASSIGN MUL MOD DIV EQL N_EQL PLUSPLUS MINUSMINUS 
%token <strval> GRTR_EQL LESS_EQL GRTR LESS L_BRACKET R_BRACKET L_BRACE R_BRACE L_PARENTH R_PARENTH COMMA SEMICOLON COLON SCOPE DOT DBL_DOT WRONG_DEFINITION
%type <intval> N M ifprefix whilestart whilecond
%type <strval> stmt tmp_stmt
%type <exprval> expr lvalue 
%type <strval> returnstmt block funcdef
%type <strval> assignexpr term
%type <strval>  for primary call objectdef const member
%type <strval> elist indexed indexedelem
%type <forval> forprefix


%left ASSIGN
%left OR
%left AND
%nonassoc EQL N_EQL
%nonassoc GRTR GRTR_EQL LESS LESS_EQL
%left PLUS MINUS
%left MUL DIV MOD
%right NOT PLUSPLUS MINUSMINUS UMINUS
%left DOT DBL_DOT
%left L_BRACKET R_BRACKET
%left L_PARENTH R_PARENTH


%%
program: stmt program 
	|	{
			printf("Program\n");
		}
        ;


stmt:	expr SEMICOLON		{				if(call_flag ==1){
									printf("Syntax Error --in line %d :Lvalue definition with a same active function is forbidden.\n",yylineno);
									call_flag = 0;

								}
								func_call_flag=0;
								printf("Expresion\n");
							}
	| ifstmt		{
						printf("IF statement\n");
					}
	| whileall		{
						printf("WHILE statement\n");
					}
	| for			{
						printf("FOR statement\n");
					}
	| returnstmt		{
							if(deepestFunctionScope == 0 ){
										printf("Syntax Error --in line %d :RETURN is declared outside of any function.\n" , yylineno);
							}
							printf("RETURN statement\n");
						} 
	| break SEMICOLON	{
							printf("|BREAK\n");
							if (loopScope==0){
								printf("Syntax Error --in line %d :BREAK is defined outside of any loop.\n",yylineno);
							}
						}
	| continue SEMICOLON	{
								printf("|CONTINUE\n");
								if (loopScope==0){
									printf("Syntax Error --in line %d :CONTINUE is defined outside of any loop.\n",yylineno);
								}
				}
	| block			{
						printf("Block\n");
					}
	| funcdef		{
						printf("Function definition\n");
					}
	| SEMICOLON		{
						printf("--> ; <--\n");
					}
	;

expr:	assignexpr		{first_flag = 0;printf("Assign expression \n");resettemp();
				$<exprval>$ = $<exprval>1;

}
	| expr PLUS expr	{printf("expression  + expression\n"); 
				$<exprval>$ = arith_emit(add,$<exprval>1,$<exprval>3,yylineno);	
				}			
	| expr MINUS expr	{printf("expression  - expression\n");
				$<exprval>$ = arith_emit(sub,$<exprval>1,$<exprval>3,yylineno);
				}
	| expr MUL expr		{printf("expression  * expression \n");
				$<exprval>$ = arith_emit(mul,$<exprval>1,$<exprval>3,yylineno);				
				}
	| expr DIV expr		{printf("expression  / expression\n");
				$<exprval>$ = arith_emit(divide,$<exprval>1,$<exprval>3,yylineno);
				}
	| expr MOD expr		{printf("expression %% expression\n");
				$<exprval>$ = arith_emit(mod,$<exprval>1,$<exprval>3,yylineno);
				}
	| expr GRTR expr	{printf("expression  > expression\n");
				$<exprval>$ = emitrelop(if_greater, $<exprval>1, $<exprval>3, yylineno);
				}
	| expr GRTR_EQL expr	{printf("expression >= expression\n");
				$<exprval>$ = emitrelop(if_greatereq, $<exprval>1, $<exprval>3, yylineno);
				}
	| expr LESS expr	{printf("expression  < expression\n");
				$<exprval>$ = emitrelop(if_less, $<exprval>1, $<exprval>3, yylineno);
				}
	| expr LESS_EQL expr	{printf("expression <= expression\n");
				$<exprval>$ = emitrelop(if_lesseq, $<exprval>1, $<exprval>3, yylineno);
				}
	| expr EQL expr		{printf("expression == expression\n");
				$<exprval>$ = emitrelop(if_eq, $<exprval>1, $<exprval>3, yylineno);
				}
	| expr N_EQL expr	{printf("expression != expression\n");
				$<exprval>$ = emitrelop(if_not_eq, $<exprval>1, $<exprval>3, yylineno);
				}
	| expr AND expr		{printf("expression && expression\n");
				$<exprval>$ = emitlogic(and, $<exprval>1, $<exprval>3, yylineno);
				}
	| expr OR expr		{printf("expression || expression\n");
				$<exprval>$ = emitlogic(or, $<exprval>1, $<exprval>3, yylineno);
				}
	| term			{printf("Terminal \n");
				$<exprval>$ = $<exprval>1;	
				}
				
	;


term:	L_PARENTH expr R_PARENTH	{printf("( expression )\n");
					$<exprval>$ = $<exprval>2;
}
	| MINUS expr %prec UMINUS	{printf("- expression\n");
					if(func_call_flag==1){
											printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : -rvalue].\n",yylineno);
					func_call_flag=0;
					}
					if($<exprval>2->type == constbool_e ||
					$<exprval>2->type == conststring_e ||
					$<exprval>2->type == nil_e ||
					$<exprval>2->type == newtable_e ||
					$<exprval>2->type == programfunc_e ||
					$<exprval>2->type == libraryfunc_e ||
					$<exprval>2->type == boolexpr_e
					){
						printf("Syntax Error: line: %d : Illegal expression to unary -\n",yylineno);
					}
														
														
					$<exprval>$ = newexpr_t(arithexpr_e);
					$<exprval>$->sym = newtemp();
					emit(uminus,$<exprval>2,NULL,$<exprval>$,-1,yylineno);
}
	| NOT expr			{printf("!expression\n"); 
					expr *tmpvar = newexpr_t(boolexpr_e);
					tmpvar->sym = newtemp();					
					emit(not,$<exprval>2,NULL,tmpvar,-1,yylineno);
					if(func_call_flag==1){
											printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : !rvalue].\n",yylineno);
					func_call_flag=0;
					}
					$<exprval>$ = tmpvar;
					
}
	| PLUSPLUS lvalue		{printf("++ lvalue\n");
					if(func_call_flag==1){
											printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : ++rvalue].\n",yylineno);
					func_call_flag=0;
				}	

				if($<exprval>2->type == tableitem_e){
					$<exprval>$ = emit_iftableitem($<exprval>2,yylineno);
					emit(add,constInt_expr(1),$<exprval>$,$<exprval>$,(unsigned)-1,yylineno);
					emit(tablesetelem,$<exprval>2->index,$<exprval>2,$<exprval>$,-1,yylineno);
				}
				else{
					emit(add,constInt_expr(1),$<exprval>2,$<exprval>2,-1,yylineno);
					$<exprval>$ = newexpr_t(arithexpr_e);
					if(!istemp($<exprval>2->sym)){
						$<exprval>$->sym = newtemp();
						emit(assign,$<exprval>2,NULL,$<exprval>$,-1,yylineno);
					}
					else{
						$<exprval>$->sym = $<exprval>2->sym;
					}
				}		

}

	| lvalue PLUSPLUS		{printf("lvalue ++\n");
					if(func_call_flag==1){
											printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : rvalue++].\n",yylineno);
					func_call_flag=0;
					}
					expr * value;
					$<exprval>$ = newexpr_t(var_e);
					$<exprval>$->sym = newtemp();
					emit(assign, $<exprval>1, NULL, $<exprval>$, -1, yylineno);
					if($<exprval>1->type == tableitem_e){
						value =emit_iftableitem($<exprval>1,yylineno);
						emit(assign,value,NULL,$<exprval>$,-1,yylineno);
						emit(add,value,value,constInt_expr(1),-1,yylineno);
						emit(tablesetelem,$<exprval>1->index,$<exprval>1,value,-1,yylineno);   
					}
					else{	     
						emit(add,constInt_expr(1),$<exprval>1,$<exprval>1,-1,yylineno);
					}

} 
	| MINUSMINUS lvalue		{printf("-- lvalue\n");
					if(func_call_flag==1){
											printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : -rvalue].\n",yylineno);
					func_call_flag=0;
					}
					
					if($<exprval>2->type == tableitem_e){
						$<exprval>$ = emit_iftableitem($<exprval>2,yylineno);
						emit(sub,constInt_expr(1),$<exprval>$,$<exprval>$,-1,yylineno);
						emit(tablesetelem,$<exprval>2->index,$<exprval>2,$<exprval>$,-1,yylineno);
					}
					else{
						emit(sub,$<exprval>2,$<exprval>2,constInt_expr(1),-1,yylineno);
						$<exprval>$ = newexpr_t(arithexpr_e);
						if(!istemp($<exprval>2->sym)){
							$<exprval>$->sym = newtemp();
							emit(assign,$<exprval>2,NULL,$<exprval>$,-1,yylineno);
						}
						else{
							$<exprval>$->sym = $<exprval>2->sym;
						}
					}


} 
	| lvalue MINUSMINUS		{printf("lvalue --\n");
					if(func_call_flag==1){
											printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : rvalue--].\n",yylineno);
					func_call_flag=0;
					}
					
					expr *value;
					
					
															
					$<exprval>$ = newexpr_t(var_e);
					$<exprval>$->sym = newtemp();
					if($<exprval>1->type == tableitem_e){
					value =emit_iftableitem($<exprval>1,yylineno);
					emit(assign,value,NULL,$<exprval>$,-1,yylineno);
					emit(sub,value,value,constInt_expr(1),-1,yylineno);
					emit(tablesetelem,$<exprval>1->index,$<exprval>1,value,-1,yylineno);
															    
															    
															}
															else{
															    emit(assign,$<exprval>1,NULL,$<exprval>$,-1,yylineno);      
															    emit(sub,constInt_expr(1),$<exprval>1,$<exprval>1,-1,yylineno);
															}	
			

} 
	| primary			{printf("Primary in terminal\n");									
					$<exprval>$ = $<exprval>1;
}
	;


assignexpr:	lvalue ASSIGN expr	{
					if($<exprval>3->sym!=NULL)printf("``Func%s\n",$<exprval>3->sym->name);
					if(lookUpH( (createS( $<exprval>1->sym->name , 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1, currscopespace(),currscopeoffset())) , "func"  , "name")!=NULL){
					printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : rvalue ASSIGN].\n",yylineno);
					func_call_flag=0;
					
												
					}
					if(($<exprval>1->type)==tableitem_e){
						emit(tablesetelem,$<exprval>1->index,$<exprval>1,$<exprval>3,-1,yylineno);
						$<exprval>$ =emit_iftableitem($<exprval>1,yylineno);
						$<exprval>$->type=assignexpr_e;
					}
					else{	
						
						emit(assign,$<exprval>3,NULL,$<exprval>1,-1,yylineno);
						$<exprval>$=newexpr_t(assignexpr_e);
						if(!istemp($<exprval>1->sym)){
							$<exprval>$->sym=newtemp();
							emit(assign,$<exprval>1,NULL,$<exprval>$,-1,yylineno);
						}
						else{
							$<exprval>$->sym = $<exprval>1->sym;
						}
					}

									printf("lvalue = expression \n");
								}
	;

	
primary:	lvalue			{			$<exprval>$ =emit_iftableitem($<exprval>1,yylineno);
								printf("lvalue\n");
								


								flag_lvalue_used = 1;
							}
	| call				{
							$<exprval>$ = $<exprval>1;
							call_flag = 0;
							printf("Call\n");
						}
	| objectdef			{
							$<exprval>$ = newexpr_t(newtable_e);
							$<exprval>$ = $<exprval>1;
							printf("Object definition\n");
							
						}
	| L_PARENTH funcdef R_PARENTH	{
										printf("(Function definition):%s\n",$<symval>2->type);
										$<exprval>$ = newexpr_t(programfunc_e);
										$<exprval>$->sym = $<symval>2;
										
									}
	| const				{	
							printf("Constant\n");
							$<exprval>$ = $<exprval>1;
						}
	;

	
lvalue:	IDENTIFIER	{
				if(tmp_buf!=NULL)free	(tmp_buf);								
				tmp_buf = malloc( 100 );
                                tmp_buf=strcpy(tmp_buf,$1);
                                strcat(tmp_buf,"");
				printf("|ID: %s\n",tmp_buf);
				
				if(funcdef_flag==1){
					call_flag = 0;
		                        if( lookUpH( (createS( tmp_buf , 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1 ,currscopespace(),currscopeoffset()) ) , "funcdef"  , "samescope")!=NULL ){
						call_flag = 1;		                            

		                        }
					else{printf("Error occured.Line:%d\n",yylineno);}
					
				}
				
                                else{
                               
					if( lookUpH( (createS( tmp_buf , deepestScope , 0 , "" , deepestFunctionScope, 0 , "var" , 1 ,currscopespace(),currscopeoffset()) ) , "funcvar"  , "illscope")!=NULL ){
						printf("Syntax Error:In line %d :Access violation attempted.\n",yylineno);
		                        }
					else if( lookUpH( (createS( tmp_buf , deepestScope , 0 , "" , deepestFunctionScope, 0 , "var" , 1 ,currscopespace(),currscopeoffset()) ) , "funcvar"  , "sscope")!=NULL ){		
						printf("[Notice:Variable is redefined-refferencing to it.]\n");
						if(lookUpH( (createS( tmp_buf , 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1 ,currscopespace(),currscopeoffset())) , "func"  , "name")!=NULL){func_call_flag=1;}else {func_call_flag = 0;}							
		                        }
					
					else{
						
						insertH(createS(tmp_buf,deepestScope,0,topfStackName(),deepestFunctionScope,0,"var",1,currscopespace(),currscopeoffset()));
						if(lookUpH( (createS( tmp_buf , 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1, currscopespace(),currscopeoffset())) , "func"  , "name")!=NULL){func_call_flag=1;
						}
						else {
							func_call_flag = 0;
						}	
					}
                                }
				funcdef_flag==0;
				inccurrscopeoffset();
				$<exprval>$ = lvalue_expr(createS(yytext,0,0,"",0,0,"var",1,currscopespace(),currscopeoffset()));		
			} 
	| LOCAL IDENTIFIER 		{
							
							printf("LOCAL METAVLITI:%s\n",(char *) $<exprval>2);
							$<exprval>$ = lvalue_expr(createS($2,0,0,"",0,0,"var",1,currscopespace(),currscopeoffset()));
							if(tmp_buf!=NULL)free(tmp_buf);
								tmp_buf = malloc( 100 );
								tmp_buf=strcpy(tmp_buf,$2);
								strcat(tmp_buf,"");
								
								
								if( lookUpH( (createS( $2 , 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1 ,currscopespace(),currscopeoffset()) ) , "funcvar"  , "localscope")!=NULL ){
									
									printf("Variable found.Refferencing to it.\n");
									if(lookUpH( (createS( $2 , 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1 ,currscopespace(),currscopeoffset())) , "func"  , "name")!=NULL){func_call_flag=1;}else {func_call_flag = 0;}		
								}
								else if( lookUpH( (createS( $2 , deepestScope , 0 , "" , deepestFunctionScope, 0 , "var" , 1 ,currscopespace(),currscopeoffset()) ) , "funcvar"  , "localillscope")!=NULL ){
                                
									printf("Syntax Error --in line %d Variable definition in the same scope with a function.\n",yylineno);
								}
								else{
									isLocalvar=1;							
									insertH(createS(tmp_buf,deepestScope,0,topfStackName(),deepestFunctionScope,0,"var",1,currscopespace(),currscopeoffset()));
									if(lookUpH( (createS( tmp_buf , 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1 ,currscopespace(),currscopeoffset())) , "func"  , "name")!=NULL){func_call_flag=1;}else {func_call_flag = 0;}	
									isLocalvar=0;
								}
					

								printf("|LOCAL ID: %s\n",yytext);
								inccurrscopeoffset();
							} 
	| SCOPE IDENTIFIER 		{ 	
							
							if(tmp_buf!=NULL)free			(tmp_buf);								
								tmp_buf = malloc( 100 );
								tmp_buf=strcpy(tmp_buf,$2);
								strcat(tmp_buf,"");
							if (lookUpH(createS(tmp_buf,0,0,"" ,0,0,"var",1,currscopespace(),currscopeoffset()),"funcvar","globalscope")!=NULL){
																printf("NOTICE:Global var redefined-refferencing to it.\n");
if(lookUpH( (createS( tmp_buf , 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1 ,currscopespace(),currscopeoffset())) , "func"  , "name")!=NULL){func_call_flag=1;}else {func_call_flag = 0;}		
															}
							else {
								printf("Syntax Error:%s variable/function not defined globally.\n",yytext);
							}

								printf("|GLOBAL ID: %s\n",yytext);
							$<exprval>$ = lvalue_expr(createS(yytext,0,0,topfStackName(),0,0,"var",1,currscopespace(),currscopeoffset()));
							}
	| member			{
							printf("Member: %s\n",yytext);
							$<exprval>$ = $<exprval>1;
						}
	;

	
member:	lvalue DOT IDENTIFIER			{printf("lvalue.ID\n");  $<exprval>$=emit_table($<exprval>1,constString_expr(yylval.strval),yylineno);}	 //TODO: xreiazetai: table emit.
	| lvalue L_BRACKET expr R_BRACKET	{printf("lvalue[expr]\n");
	$<exprval>$= emit_table($<exprval>1,$<exprval>3,yylineno);
	}
	| call DOT IDENTIFIER				{printf("Call.ID\n");}
	| call L_BRACKET  expr R_BRACKET 	{printf("Call[expr]\n");}
	;
	
	
call:	call L_PARENTH elist R_PARENTH 						{printf("CALL(Elist)\n");
		$<exprval>$ = (expr *) func_call($<exprval>1, $<exprval>3, yylineno);

} 
	| lvalue L_PARENTH elist R_PARENTH	{printf("|lvalue(elist) - Normal Call\n");
						$<exprval>$ = (expr *) func_call($<exprval>1, $<exprval>3, yylineno);
						}
	|lvalue DBL_DOT IDENTIFIER L_PARENTH elist R_PARENTH	{printf("|lvalue..ID(Elist)-Method Call\n");

												
												expr *this=$<exprval>1;
												expr *tmp=emit_iftableitem( createMember(this, $<strval>3, yylineno),yylineno);
												this->next=$<exprval>5;
												$<exprval>5=this;
												$<exprval>$ = (expr *) func_call(tmp, $<exprval>5, yylineno);
	
	}
	
	
	| L_PARENTH funcdef R_PARENTH L_PARENTH elist R_PARENTH {printf("(funcdef)(Elist)\n");
	printf("(FUNCDEF)(ELIST)\n");
												expr *function = newexpr_t(programfunc_e);
												function -> sym = $<symval>2;
												$<exprval>$ = (expr *) func_call(function, $<exprval>5, yylineno);
	}
	;




	
elist:	expr tmp_elist				{printf("Expr\n");
						$<exprval>$ = $<exprval>1;
						$<exprval>$->next = $<exprval>2;
						} 
	|					{printf("Empty Elist\n");
						$<exprval>$ = NULL;						
						}
	;

tmp_elist: COMMA expr tmp_elist			{printf("Expr,....,Expr\n");
						$<exprval>$ = $<exprval>2;
						$<exprval>$->next = $<exprval>3;
						} 
	|					{$<exprval>$ = NULL;}
	;

objectdef:	L_BRACKET elist R_BRACKET	{printf(" [ Elist ] \n");
expr * table = newexpr_t(newtable_e);
expr *tmp = NULL;
int i = 0;
table->sym = newtemp();
   emit(tablecreate, NULL, NULL, table, -1, yylineno);
    tmp = $<exprval>2;
    while (tmp != NULL) {
        emit(tablesetelem,constInt_expr(i) , table,tmp , -1, yylineno);
        tmp = tmp->next;
        i++;
    }
    $<exprval>$ = table;



} 
	| L_BRACKET indexed R_BRACKET		{printf(" [ Indexed ] \n");
	expr *table= newexpr_t(newtable_e);
	table->sym = newtemp();
	emit(tablecreate,NULL,NULL,table,-1,yylineno);
	expr *tmp = $<exprval>2;
	while(tmp!=NULL){
		emit(tablesetelem,table,tmp,tmp->index,-1,yylineno);
		tmp=tmp->next;
	}
	$<exprval>$=table;
	
	}
	;

indexed: indexedelem tmp_indexed 	{printf("indexedelem\n");
					$<exprval>$ = $<exprval>1;
					$<exprval>$->next = $<exprval>2;			
					}
	;

tmp_indexed: COMMA indexedelem tmp_indexed {
					printf("indexedelem , indexedelem\n");					
					$<exprval>$ = $<exprval>2;
					$<exprval>$->next = $<exprval>3; }
	|	{$<exprval>$ = NULL;}
		
	;

indexedelem:	L_BRACE expr COLON expr R_BRACE	{printf("{ Expr : Expr }\n");
		 $<exprval>$ = $<exprval>2;
		$<exprval>$->index = $<exprval>4;

}
	;

tmp_stmt:	tmp_stmt stmt			{;} 
	|					{;}
	;

block:		L_BRACE{ 
						deepestScope++;					
			} 
		 	tmp_stmt{} 
			R_BRACE{ 
					deepestScope--; 
					printf("{ Statement }\n");
				}
	;	


ident_tmp:	IDENTIFIER       {flag_trasp=0;}
			  
																											
	|	{
			flag_trasp=1;
                        setFunctionPrefix(f_name);
		}
	;

funcdef: 	funcprefix funcargs{enterscopespace();  resetfunctionlocaloffset();} funcbody {
exitscopespace();
fStack *tmp_buf;
tmp_buf = popfStack();
Symbol *tmpS = createS( tmp_buf->sym->name , 0 , 0 ,tmp_buf->sym->name , deepestFunctionScope , 0 , "func" , 1,currscopespace(),currscopeoffset() );
expr *expr_tmp;
expr_tmp = newexpr_t(programfunc_e);
expr_tmp->sym = tmp_buf->sym;				
$<symval>$ = expr_tmp->sym;
emit(funcend,NULL,NULL,lvalue_expr(tmpS),nextquadlabel(),yylineno);
node* tmp = popStack(loopStack);
if (tmp)
	loopScope = tmp->info;
findLocal(tmp_buf->sym->name);
free(tmp_buf);
printf("Funcdef\n");
restorecurrscopeoffset(popOffsetStack(offsetStack));
hideInFunction(); 
}
	;

funcprefix:	FUNCTION ident_tmp {
				
				pushOffsetStack(offsetStack,currscopeoffset());
				inccurrscopeoffset();	
				resetformalargoffset();
				funcdef_flag =1;
				if (flag_trasp==0){
					if(!f_name){
						free(f_name);
					}		
					f_name=strdup(yytext);
				}
				printf("|FUNCTION %s\n",f_name);
				pushfStack(createS( f_name , deepestScope , 0 ,f_name  , deepestFunctionScope, 0 , "func" , 1 ,currscopespace(),currscopeoffset())); 
				
				if( lookUpH( (createS( f_name , 0 , 0 , "" , deepestFunctionScope, 0 , "func" , 1 ,currscopespace(),currscopeoffset()) ) , "funcdef"  , "samescope")!=NULL ){
					printf("Syntax Error --in line %d :Function definition with a same named function in the same scope or library function is forbidden.\n",yylineno);
				}
				else if( lookUpH( (createS( f_name , deepestScope , 0 , "" , deepestFunctionScope, 0 , "func" , 1 ,currscopespace(),currscopeoffset()) ) , "funcvar"  , "scope")!=NULL ){
					printf("EEEEEEEEEEEEEEEEEEEEEEEEERRRRRRRRRRRRRRROR");
					printf("Syntax Error --in line %d :Function definition with a same ID in an equal or smaller scope level is forbidden.\n",yylineno);
				}
				else{
					
					insertH(createS(f_name,deepestScope,0,f_name,deepestFunctionScope, yylineno,"func",1,currscopespace(),currscopeoffset()));
					lineF = yylineno;
					Symbol *tmpS = createS(f_name   , 0 , 0 , "" , deepestFunctionScope , 0 , "func" , 1,currscopespace(),currscopeoffset() );
					emit(funcstart,NULL,NULL,lvalue_expr(tmpS),nextquadlabel(),yylineno);
					pushStack(loopStack, loopScope);
					loopScope = 0;
					 
					if(!f_name){
						free(f_name);
					}	


					if(lookUpH( (createS( f_name , 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1,currscopespace(),currscopeoffset() )) , "func"  , "name")!=NULL){
						func_call_flag=1;
					}
					else{
						func_call_flag=0;
					}
					inccurrscopeoffset();
				}
				funcdef_flag =0;
				
			};

funcargs: 		L_PARENTH { 
				enterscopespace();
				deepestFunctionScope++; 
				deepestScope++;
				pushOffsetStack(offsetStack,currscopeoffset());
			}
			idlist{
				resetfunctionlocaloffset(); 
			}
			R_PARENTH {
				hideInFunction(); 
				deepestFunctionScope--;
				deepestScope--;
				deepestFunctionScope++;
			};
funcbody:		block {
				funcdef_flag=0;
				 
				deepestFunctionScope--;
				printf("Function block\n");
				restorecurrscopeoffset(popOffsetStack(offsetStack));
				exitscopespace();
			};

const:	INTEGER			{printf("|NUMBER %s\n",yytext);
				$<exprval>$ = constInt_expr(yylval.intval);	
				} 
	| REALNUMBER		{printf("|REALNUMBER\n");
				$<exprval>$ = constDouble_expr(yylval.doubleval);
				}
	| STRING		{printf("|STRING\n");
				$<exprval>$ = constString_expr(yylval.strval);	
				}
	| NIL			{printf("|NIL\n");
				$<exprval>$ = newexpr_t(nil_e);
				}
	| TRUE			{printf("|TRUE\n");
				$<exprval>$ = constBool_expr('1');
				}
	| FALSE			{printf("|FALSE\n");
				$<exprval>$ = constBool_expr('0');				
				}
	;

idlist:	IDENTIFIER	{
				if (lookUpH(createS($1,deepestScope,0,"",deepestFunctionScope,0,"formarg",1,currscopespace(),currscopeoffset()),"formarg","fsc_ac")==NULL){
					insertH(createS($1,deepestScope,0,topfStackName(),deepestFunctionScope,0,"formarg",1,currscopespace(),currscopeoffset()));
					if(lookUpH( (createS($1 , 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1 ,currscopespace(),currscopeoffset())) , "func"  , "name")!=NULL){
						func_call_flag=1;
					}
					else {
						func_call_flag = 0;
					}	
					inccurrscopeoffset();
				} 
				else if (lookUpH(createS($1,deepestScope,0,"",deepestFunctionScope,0,"formarg",1,currscopespace(),currscopeoffset()),"formarg","fsc_ac")!=NULL){
					printf("Syntax Error --in line %d :Argument redefined in this scope.\n",yylineno);
                               	} 					
				printf("|ID:%s\n",yytext);
			}
	|idlist COMMA IDENTIFIER {
					if (lookUpH(createS($3,deepestScope,0,"",deepestFunctionScope,0,"formarg",1,currscopespace(),currscopeoffset()),"formarg","fsc_ac")==NULL){
						insertH(createS($3,deepestScope,0,topfStackName(),deepestFunctionScope,0,"formarg",1,currscopespace(),currscopeoffset()));
						if(lookUpH( (createS( $3, 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1 ,currscopespace(),currscopeoffset())) , "func"  , "name")!=NULL){
							func_call_flag=1;
						}
						else {
							func_call_flag = 0;
						}	
					inccurrscopeoffset();
				}
				else if (lookUpH(createS($3,deepestScope,0,"",deepestFunctionScope,0,"formarg",1,currscopespace(),currscopeoffset()),"formarg","fsc_ac")!=NULL){
					printf("Syntax Error --in line %d :Argument redefined in this scope.\n",yylineno);
				}
				printf("|ID,...,ID: %s\n",yytext);				
			} 
	|			{;};


ifprefix:	IF L_PARENTH expr R_PARENTH{
			emit(if_eq,$<exprval>3,constBool_expr('1'),NULL,currQuad+2,yylineno);
			$<intval>$ =  currQuad;
			emit(jump,NULL,NULL,NULL,-1,yylineno);
		}
		;


elseprefix:	ELSE	{
			$<intval>$ =  currQuad;
			emit(jump,NULL,NULL,NULL,-1,yylineno);}
		;

ifstmt:		ifprefix stmt{
			backpatch($<intval>$,currQuad);
}
		| ifprefix stmt elseprefix stmt	{
			backpatch($<intval>1,$<intval>3+1);
			backpatch($<intval>3,currQuad);
		}
		;

whilestart:	WHILE	{
			$<intval>$ = currQuad;
			pushStack(quadLoopStack,currQuad);
		};

whilestmt:		L_PARENTH expr R_PARENTH	{
				emit(if_eq,$<exprval>2 , constBool_expr('1'),NULL , currQuad + 2, yylineno);
				$<intval>$ = currQuad;
				pushStack(breakWhileStack, $<intval>$);
				emit(jump, (expr *)NULL, (expr *)NULL, (expr *)NULL, -1, yylineno);
		};

whilecond:		{
					loopScope++;
					}
;

whileblock:	stmt {
			loopScope--;
			
		};


whileall: whilestart whilestmt whilecond whileblock {printf("WHILE (EXPR) STMT\n");

			node * whilenum;
			unsigned int tmpquadnum = popStack(quadLoopStack)->info;
			node *tmpquad;

			whilenum = popStack(breakWhileStack);
			tmpstack = topSpecialJumpStack(breakSpecialStack);
			while((tmpstack)&& (tmpstack->jump == tmpquadnum))
			{
				tmpstack = popSpecialJumpStack(breakSpecialStack);
				if (tmpstack)				
					backpatch(tmpstack->start,currQuad+1);
					
			}
			tmpstack = topSpecialJumpStack(continueSpecialStack);
			while((tmpstack)&& (tmpstack->jump == tmpquadnum))
			{
				tmpstack = popSpecialJumpStack(continueSpecialStack);
				if (tmpstack)					
					backpatch(tmpstack->start,tmpquadnum);
					
			}
			node * break_patch;
			node * continue_patch;
			emit(jump, (expr *)NULL, (expr *)NULL, (expr *)NULL, tmpquadnum, yylineno);
			backpatch(whilenum->info, nextquadlabel());
			}

;	


loopsc:		{loopScope++;} 
;

dsc:		{deepestScope--; } 
;

forpr:		FOR loopsc L_PARENTH elist SEMICOLON {pushStack(quadLoopStack,currQuad);}
;


forprefix: 	forpr M expr SEMICOLON 
		{
			struct forprefix *res = malloc(sizeof(struct forprefix));
			
			res->test = $<intval>2;
			res->enter = currQuad ;
			
			emit(if_eq,$<exprval>3 , constBool_expr('1'),  (expr*)NULL, -1, yylineno);
			$<forval>$ = res;
		}
;  

nsc:	{deepestScope++; loopScope--;scope_flag = 0;printf("|FOR (ELIST;EXPR;ELIST)STMT\n");}
;

forend:		{
			unsigned int tmpquadnum = popStack(quadLoopStack)->info;
			tmpstack = topSpecialJumpStack(breakSpecialStack);
			while((tmpstack)&& (tmpstack->jump == tmpquadnum))
			{
				tmpstack = popSpecialJumpStack(breakSpecialStack);
				if (tmpstack)	
					backpatch(tmpstack->start,currQuad+1);
					
			}
			tmpstack = topSpecialJumpStack(continueSpecialStack);
			while((tmpstack)&& (tmpstack->jump == tmpquadnum))
			{
				tmpstack = popSpecialJumpStack(continueSpecialStack);
				if (tmpstack)		
					backpatch(tmpstack->start,tmpquadnum);
				
			}


}
;

for:		forprefix N elist R_PARENTH dsc N stmt nsc N forend
		{
						
			backpatch($<forval>1->enter,$<intval>6+1);			
			backpatch($<intval>2,currQuad);
			backpatch($<intval>6,$<forval>1->test);
			backpatch($<intval>9,$<intval>2+1);
			
			

			
		}
;

N:	{
		$<intval>$ = currQuad;
		printf("BENEI KAI STO N1: %d \n",$<intval>$);
		emit(jump, (expr*)NULL, (expr*)NULL,  (expr*)NULL, -1, yylineno);
		printf("BENEI KAI STO N2: %d \n",$<intval>$);
	}
;

M:	{
		$<intval>$ = currQuad;
	}
	;



break: 	BREAK {pushSpecialJumpStack(breakSpecialStack,currQuad,topStack(quadLoopStack)->info);
		emit(jump, (expr*)NULL, (expr*)NULL, (expr*)NULL, -1, yylineno);
		
		}
;


continue:	CONTINUE{pushSpecialJumpStack(continueSpecialStack,currQuad,(topStack(quadLoopStack)->info));
			emit(jump, (expr*)NULL, (expr*)NULL, (expr*)NULL, -1, yylineno);
}
;

returnstmt:	RETURN SEMICOLON	{			emit(ret, (expr*)NULL, (expr*)NULL, (expr*)NULL, -1, yylineno);
								    	printf("|RETURN;\n");
								} 
	| RETURN expr SEMICOLON		{		emit(ret, (expr*)NULL, (expr*)NULL, $<exprval>2, -1, yylineno);
									printf("|RETURN EXPR;\n");
								}
	;	 

%%

int yyerror (const char * YaccProvidedMessage){	
	if ((strstr(YaccProvidedMessage,"$end")))
		printf("Unexpected reach of the EOF.\n");
	else
		printf("%s -- in line %d , token %s \n ",YaccProvidedMessage,yylineno,yytext);
	return 0;
}

int main(int argc,char **argv){
	
	breakWhileStack = initStack();
	loopStack = initStack();
	quadLoopStack = initStack();
	breakSpecialStack = initSpecialJumpStack();
	continueSpecialStack = initSpecialJumpStack();
	f_name = malloc(100);
	offsetStack= initOffsetStack();
	initfStack();	
	
	initializeH();
	yyparse();
	printHash();
	printquads();	
	
	
	return 0;
}
