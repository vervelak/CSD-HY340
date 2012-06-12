%{
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "hashTable.h"


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
extern int yylex(void);
extern int yylineno;
extern char *yytext;
extern FILE* yyin;
extern Symbol *hashtable[ HASH_SIZE ];
int isLocalvar=0;
int deepestFunctionScope = 0;
int deepestScope = 0;
int loopScope = 0; //gia na elegxoume an iparxei break ekso apo kapoio loop
int yyerror(const char * yaccProvidedMessage);

%}
%error-verbose
%start program

%union
{
	int intval;
	char* strval;
	float flval;
}

%token <intval> INTEGER;
%token <strval> STRING;
%token <strval> IDENTIFIER;
%token <flval> REALNUMBER;

%token <strval> WHILE FOR IF ELSE FUNCTION RETURN BREAK CONTINUE AND OR NOT LOCAL TRUE FALSE NIL PLUS MINUS ASSIGN MUL MOD DIV EQL N_EQL PLUSPLUS MINUSMINUS 
%token <strval> GRTR_EQL LESS_EQL GRTR LESS L_BRACKET R_BRACKET L_BRACE R_BRACE L_PARENTH R_PARENTH COMMA SEMICOLON COLON SCOPE DOT DBL_DOT WRONG_DEFINITION

%type <strval> stmt tmp_stmt
%type <intval> expr
%type <strval> ifstmt whilestmt forstmt returnstmt block funcdef
%type <strval> assignexpr term
%type <strval> lvalue primary call objectdef const member
%type <strval> elist callsuffix normcall methodcall indexed indexedelem


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


stmt:	expr SEMICOLON		
		{
			if(call_flag ==1){
				printf("Syntax Error --in line %d :Lvalue definition with a same active function is forbidden.\n",yylineno);
				call_flag = 0;
			}
			printf("Expresion\n");
		}
	| ifstmt
		{
			printf("IF statement\n");
		}
	| whilestmt
		{
			printf("WHILE statement\n");
		}
	| forstmt
		{
			printf("FOR statement\n");
		}
	| returnstmt
		{
			if(deepestFunctionScope == 0 ){
				printf("Syntax Error --in line %d :RETURN is declared outside of any function.\n" , yylineno);
			}
			printf("RETURN statement\n");
		} 
	| BREAK SEMICOLON
		{
			printf("|BREAK\n");
			if (loopScope==0){
				printf("Syntax Error --in line %d :BREAK is defined outside of any loop.\n",yylineno);
			}
		}
	| CONTINUE SEMICOLON
		{
			printf("|CONTINUE\n");
			if (loopScope==0){
				printf("Syntax Error --in line %d :CONTINUE is defined outside of any loop.\n",yylineno);
			}
		}
	| block
		{
			printf("Block\n");
		}
	| funcdef
		{
			printf("Function definition\n");
		}
	| SEMICOLON
		{
			printf("--> ; <--\n");
		}
	;

expr:	assignexpr		
		{
			printf("Assignes expression \n");
			if(func_call_flag==1){
				printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : rvalue ASSIGN].\n",yylineno);
				func_call_flag=0;
			}
		}
	| expr PLUS expr	{printf("expression  + expression -> %d+%d\n",$1,$3); 	$$ = $1 + $3;}
	| expr MINUS expr	{printf("expression  - expression -> %d-%d\n",$1,$3); 	$$ = $1 - $3;}
	| expr MUL expr		{printf("expression  * expression -> %d*%d\n",$1,$3); 	$$ = $1 * $3;}
	| expr DIV expr		{printf("expression  / expression -> %d/%d\n",$1,$3); 	$$ = $1 / $3;}
	| expr MOD expr		{printf("expression %% expression -> %d %% %d\n",$1,$3); 	$$ = $1 % $3;}
	| expr GRTR expr	{printf("expression  > expression -> %d>%d\n",$1,$3); 	$$ = ($1 > $3)?1:0;}
	| expr GRTR_EQL expr	{printf("expression >= expression -> %d>=%d\n",$1,$3); 	$$ = ($1>=$3)?1:0;}
	| expr LESS expr	{printf("expression  < expression -> %d<%d\n",$1,$3); 	$$ = ($1<$3)?1:0;}
	| expr LESS_EQL expr	{printf("expression <= expression -> %d<=%d\n",$1,$3); 	$$ = ($1<=$3)?1:0;}
	| expr EQL expr		{printf("expression == expression -> %d==%d\n",$1,$3); 	$$ = ($1==$3)?1:0;}
	| expr N_EQL expr	{printf("expression != expression -> %d!=%d\n",$1,$3); 	$$ = ($1!=$3)?1:0;}
	| expr AND expr		{printf("expression && expression -> %d&&%d\n",$1,$3); 	$$ = ($1&&$3)?1:0;}
	| expr OR expr		{printf("expression || expression -> %d/%d\n",$1,$3); 	$$ = ($1||$3)?1:0;}
	| term			{printf("Terminal\n");}
	;


term:	L_PARENTH expr R_PARENTH	{printf("( expression )\n");}
	| MINUS expr %prec UMINUS	
		{
			printf("- expression\n");
			if(func_call_flag==1){
				printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : -rvalue].\n",yylineno);
				func_call_flag=0;
			}
		}		
	| NOT expr
		{
			printf("!expression\n");
			if(func_call_flag==1){
				printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : !rvalue].\n",yylineno);
				func_call_flag=0;
			}
		}//isfunction check
	| PLUSPLUS lvalue
		{
			printf("++ lvalue\n");
			if(func_call_flag==1){
				printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : ++rvalue].\n",yylineno);
				func_call_flag=0;
			}						
		}//isfunction check
	| lvalue PLUSPLUS
		{
			printf("lvalue ++\n");
			if(func_call_flag==1){
				printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : rvalue++].\n",yylineno);
				func_call_flag=0;
			}
		} //isfunction check
	| MINUSMINUS lvalue
		{
			printf("-- lvalue\n");
			if(func_call_flag==1){
				printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : -rvalue].\n",yylineno);
				func_call_flag=0;
			}
		} //isfunction check
	| lvalue MINUSMINUS
		{
			printf("lvalue --\n");
			if(func_call_flag==1){
				printf("Syntax Error: In line %d .Trying to make an lvalue action to function[invalid action : rvalue--].\n",yylineno);
				func_call_flag=0;
			}
		} //isfunction check
	| primary	
		{
			printf("Primary\n");
		}
	;


assignexpr:	lvalue ASSIGN expr
		{
			printf("lvalue = expression \n");
		}
	;

	
primary:	lvalue
		{
			printf("lvalue\n");
			flag_lvalue_used = 1;
		}
	| call
		{
			call_flag = 0;
			printf("Call\n");
		}
	| objectdef
		{
			printf("Object definition\n");
		}
	| L_PARENTH funcdef R_PARENTH
		{
			printf("(Function definition)\n");
		}
	| const	
		{
			printf("Constant\n");
		}
	;

	
lvalue:	IDENTIFIER
		{
			/*TODO XXX*/
			if(tmp_buf!=NULL)free	(tmp_buf);								
				tmp_buf = malloc( 100 );
                                tmp_buf=strcpy(tmp_buf,$1);
                                strcat(tmp_buf,"");
				printf("|ID: %s\n",yytext);
			if(funcdef_flag==1){
				call_flag = 0;
				if( lookUpH((createS( tmp_buf, 0, 0, "", deepestFunctionScope, 0, "var", 1)), "funcdef", "samescope")!=NULL){
					call_flag = 1;
				}
				else{
					printf("Error occured.Line:%d\n",yylineno);
				}
			}
			else{
				if( lookUpH((createS( tmp_buf, deepestScope, 0, "", deepestFunctionScope, 0, "var", 1)), "funcvar", "illscope")!=NULL){
					printf("Syntax Error:In line %d :Access violation attempted.\n",yylineno);
				}
				else if( lookUpH((createS( tmp_buf, deepestScope, 0, "", deepestFunctionScope, 0, "var", 1)), "funcvar", "sscope")!=NULL){
					printf("[Notice:Variable is redefined-refferencing to it.]\n");
					if(lookUpH((createS( tmp_buf, 0, 0, "", deepestFunctionScope, 0, "var", 1)), "func", "name")!=NULL){
						func_call_flag=1;
					}
				}
				else{
					insertH(createS(tmp_buf,deepestScope,0,"",deepestFunctionScope,0,"var",1));
					if(lookUpH((createS( tmp_buf, 0, 0, "", deepestFunctionScope, 0, "var", 1)), "func", "name")!=NULL){
						func_call_flag=1;
					}	
				}
			}
			funcdef_flag==0;
		} 
	| LOCAL IDENTIFIER 
		{	/*TODO XXX*/
			if(tmp_buf!=NULL)free(tmp_buf);
			tmp_buf = malloc( 100 );
			tmp_buf=strcpy(tmp_buf,$2);
			strcat(tmp_buf,"");
			if( lookUpH((createS( $2, 0, 0, "", deepestFunctionScope, 0, "var", 1)), "funcvar", "localscope")!=NULL){
				printf("Variable found.Refferencing to it.\n");
				if(lookUpH((createS( $2, 0, 0, "", deepestFunctionScope, 0, "var", 1)), "func", "name")!=NULL){
					func_call_flag=1;
				}	
			}
			else if( lookUpH((createS( $2, deepestScope, 0, "", deepestFunctionScope, 0, "var", 1)), "funcvar", "localillscope")!=NULL){
				printf("Syntax Error --in line %d Variable definition in the same scope with a function.\n",yylineno);
			}
			else{
				isLocalvar=1;								
				insertH(createS( tmp_buf, deepestScope, 0, "", deepestFunctionScope, 0, "var", 1));
				if(lookUpH((createS( tmp_buf, 0, 0, "", deepestFunctionScope, 0, "var", 1)), "func", "name")!=NULL){
					func_call_flag=1;
				}
				isLocalvar=0;
			}
			printf("|LOCAL ID: %s\n",yytext);
		} 
	| SCOPE IDENTIFIER
		{ 	/*TODO XXX*/
			if(tmp_buf!=NULL)free			(tmp_buf);								
			tmp_buf = malloc( 100 );
			tmp_buf=strcpy(tmp_buf,$2);
			strcat(tmp_buf,"");
			if (lookUpH(createS( tmp_buf, 0, 0, "", 0, 0, "var", 1), "funcvar", "globalscope")!=NULL){
				printf("NOTICE:Global var redefined-refferencing to it.\n");
				if(lookUpH((createS( tmp_buf, 0, 0, "", deepestFunctionScope, 0, "var", 1)), "func", "name")!=NULL){
					func_call_flag=1;
				}	
			}
			else {
				printf("Syntax Error:%s variable/function not defined globally.\n",yytext);
			}
			printf("|GLOBAL ID: %s\n",yytext);
		}
	| member
		{
			printf("Member: %s\n",yytext);
		}
	;

	
member:	lvalue DOT IDENTIFIER			{printf("lvalue.ID\n");}	
	| lvalue L_BRACKET expr R_BRACKET	{printf("lvalue[expr]\n");}
	| call DOT IDENTIFIER			{printf("Call.ID\n");}
	| call L_BRACKET  expr R_BRACKET 	{printf("Call[expr]\n");}
	;
	
	
call:	call L_PARENTH elist R_PARENTH 				{printf("CALL(Elist)\n");}
	| lvalue callsuffix					{printf("lvalue callsuffix\n");}
	| L_PARENTH funcdef R_PARENTH L_PARENTH elist R_PARENTH {printf("(funcdef)(Elist)\n");}
	;

	
callsuffix:	normcall	{printf("Normall call\n");}
	| methodcall		{printf("Method call\n");}
	;
	
	
normcall:	L_PARENTH elist R_PARENTH {printf("(Elist)\n");}
	;

	
methodcall:	DBL_DOT IDENTIFIER L_PARENTH elist R_PARENTH	{printf("|..ID(Elist)\n");}
	;

	
elist:	expr			{printf("Expr\n");}
	| elist COMMA expr	{printf("Expr,..,Expr\n");} 
	|			{printf("Empty Elist\n");}
	;

objectdef:	L_BRACKET elist R_BRACKET	{printf(" [ Elist ] \n");}
	| L_BRACKET indexed R_BRACKET		{printf(" [ Indexed ] \n");}
	;

indexed:	indexedelem		{printf("Idexedelem\n");}
	|indexed COMMA indexedelem	{printf("indexedelem , indexedelem\n");}
	;

indexedelem:	L_BRACE expr COLON expr R_BRACE	{printf("{ Expr : Expr }\n");}
	;

tmp_stmt:	tmp_stmt stmt	{;} 
	|			{;}
	;

block:	L_BRACE 
		{deepestScope++;} 
	 	tmp_stmt{} 
		R_BRACE{ 
			deepestScope--; 
			printf("{ Statement }\n");
		}
	;

ident_tmp:	IDENTIFIER	{}
	|	{
			flag_trasp=1;
                        setFunctionPrefix(f_name);
		}
	;

funcdef:	FUNCTION ident_tmp 
		{
			funcdef_flag =1;
			{
				if (flag_trasp==0){
					f_name=strdup(yytext);
				}
				pushfStack(f_name);  
                        	flag_trasp=0;
				printf("|FUNCTION %s\n",f_name);
			}
			fStack *tmp_buf;
			tmp_buf = popfStack();
			if( lookUpH( (createS(tmp_buf->fname, 0, 0, "", deepestFunctionScope, 0, "func", 1)), "funcdef", "samescope")!=NULL){
				printf("Syntax Error --in line %d :Function definition with a same named function in the same scope or library function is forbidden.\n",yylineno);
			}
			else if( lookUpH((createS( tmp_buf->fname, deepestScope, 0, "", deepestFunctionScope, 0, "func", 1)), "funcvar", "scope")!=NULL){
				printf("Syntax Error --in line %d :Function definition with a same ID in an equal or smaller scope level is forbidden.\n",yylineno);
			}
			else{
				insertH(createS(tmp_buf->fname,deepestScope,0,"",deepestFunctionScope,0,"func",1));
				if(lookUpH((createS(tmp_buf->fname, 0, 0, "", deepestFunctionScope, 0, "var", 1)), "func", "name")!=NULL){
					func_call_flag=1;
				}
			}
			free(tmp_buf);
			funcdef_flag =0;
		}
		L_PARENTH 
		{ 
			deepestFunctionScope++; 
			deepestScope++;
		}
		idlist
		R_PARENTH 
		{
			hideInFunction(); 
			deepestFunctionScope--;
			deepestScope--;
			deepestFunctionScope++;
		} 
		{} 
		block 
		{
			funcdef_flag=0;
			hideInFunction(); 
			deepestFunctionScope--;
			printf("Function block\n");
		} 
		;

const:	INTEGER			{printf("|NUMBER %s\n",yytext);} 
	| REALNUMBER		{printf("|REALNUMBER\n");}
	| STRING		{printf("|STRING\n");}
	| NIL			{printf("|NIL\n");}
	| TRUE			{printf("|TRUE\n");}
	| FALSE			{printf("|FALSE\n");}
	;

idlist:	IDENTIFIER
		{
			if (lookUpH(createS($1,deepestScope,0,"",deepestFunctionScope,0,"formarg",1),"formarg","fsc_ac")==NULL){
				insertH(createS($1,deepestScope,0,"",deepestFunctionScope,0,"formarg",1));
				if(lookUpH( (createS($1 , 0 , 0 , "" , deepestFunctionScope , 0 , "var" , 1 )) , "func"  , "name")!=NULL){
					func_call_flag=1;
				}
			} 
			else if (lookUpH(createS($1,deepestScope,0,"",deepestFunctionScope,0,"formarg",1),"formarg","fsc_ac")!=NULL){
				printf("Syntax Error --in line %d :Argument redefined in this scope.\n",yylineno);
			} 					
			printf("|ID:%s\n",yytext);
		}
	|idlist COMMA IDENTIFIER
		{
			if (lookUpH(createS($3,deepestScope,0,"",deepestFunctionScope,0,"formarg",1),"formarg","fsc_ac")==NULL){
				insertH(createS($3,deepestScope,0,"",deepestFunctionScope,0,"formarg",1));
				if(lookUpH((createS($3, 0, 0, "", deepestFunctionScope, 0, "var", 1)), "func", "name")!=NULL){
					func_call_flag=1;
				}
			}
			else if (lookUpH(createS($3,deepestScope,0,"",deepestFunctionScope,0,"formarg",1),"formarg","fsc_ac")!=NULL){
				printf("Syntax Error --in line %d :Argument redefined in this scope.\n",yylineno);
			} 
			printf("|ID,...,ID: %s\n",yytext);
		} 
	|	{;}
	;

ifstmt:		IF L_PARENTH expr R_PARENTH
		{
			deepestScope--;
		}
		stmt %prec N_EQL
		{
			deepestScope++; 
			scope_flag = 0;
			printf("|IF (EXPR) STMT\n");
		}
	| ifstmt ELSE
		{
			deepestScope--;
		}
		stmt
		{
			deepestScope++; 
			printf("|IF (EXPR) STMT ELSE STMT\n");
		}
	;	 

whilestmt:	WHILE 
		{
			loopScope++;
		}
		L_PARENTH expr R_PARENTH
		{
		 	deepestScope--;
		}
		stmt
		{
			deepestScope++; 
			loopScope--;
			scope_flag = 0;
			printf("|WHILE(EXPR)STMT\n");
		} 
	;  	

	
forstmt:	FOR
		{
			loopScope++;
		}
		L_PARENTH elist SEMICOLON expr SEMICOLON elist R_PARENTH 
		{
			deepestScope--;
		}
		stmt
		{
			deepestScope++; 
			loopScope--;
			scope_flag = 0; 
			printf("|FOR (ELIST;EXPR;ELIST)STMT\n");
		} 
	;	 

returnstmt:	RETURN SEMICOLON
		{
			printf("|RETURN;\n");
		} //tsekarisma gia an einai mesa se function
		| RETURN expr SEMICOLON
		{
			printf("|RETURN EXPR;\n");
		}
	;	 

%%

int yyerror (const char * YaccProvidedMessage)
{	
	if ((strstr(YaccProvidedMessage,"$end")))
		printf("Unexpected reach of the EOF.\n");
	else
		printf("%s -- in line %d , token %s \n ",YaccProvidedMessage,yylineno,yytext);
	return 0;
}

int main(int argc,char **argv)
{
	f_name = malloc(100);
	initfStack();	
	initializeH();
	yyparse();
	printHash();	
	return 0;
}
