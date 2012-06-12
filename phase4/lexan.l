%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
//prepei na ginei include to arxeio toy parser, kai isws kai kapoia alla.
typedef struct node alpha_token_t;
extern YYSTYPE yylval;

int ltokenno = 0;
int lcount =0;
int str_size;
int str_tmp_size;
char * token_string ;
char * string_to_list;
void reallocateIfNeeded(char * ,char * ,int * ,int * );	

%}

%option noyywrap
%option yylineno
%option header-file="./al.h"


identifier		[a-zA-Z][a-zA-Z_0-9]*

integer			[0-9]+

realnumber		([0-9]+)\.([0-9](e|E)?[0-9]*)

whitespace 		[\r\n\t\v ]

commentline		"//".*

wrong_identifier	[0-9$_][a-zA-Z_0-9]*

%x COMMENT
%x STRINGT

%% 

\"		{
			str_size = 128;
			str_tmp_size = 0;
			token_string = malloc(sizeof(char)*str_size);
			string_to_list = token_string;
			BEGIN (STRINGT);
		}

<STRINGT>\"	{
			reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
			token_string[str_tmp_size] = '\0';
			yylval.strval = strdup(token_string);
			BEGIN (INITIAL);
			return STRING;
		}

<STRINGT>\\n     {
                        reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
                        token_string[str_tmp_size]='\n';
                        str_tmp_size++;
                }


<STRINGT>\\t	{
			reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
			token_string[str_tmp_size]='\t';
			str_tmp_size++;
		}

<STRINGT>\\r	{
			reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
			token_string[str_tmp_size]='\r';
			str_tmp_size++;
		}



<STRINGT>\\v	{
			reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
			token_string[str_tmp_size]='\v';
			str_tmp_size++;
		}

<STRINGT>\\b     {
                        reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
                        token_string[str_tmp_size]='\b';
                        str_tmp_size++;
               }


<STRINGT>[\n]    {
                        reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
                        token_string[str_tmp_size]='\n';
                        str_tmp_size++;

                 }

<STRINGT>[\t]    {
                        reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
                        token_string[str_tmp_size]='\t';
                        str_tmp_size++;

                 }
<STRINGT>[\r]    {
                        reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
                        token_string[ str_tmp_size]='\r';
                        str_tmp_size++;

                 }

<STRINGT>[\v]    {
                        reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
                        token_string[str_tmp_size]='\v';
                        str_tmp_size++;

                 }
<STRINGT>[\b]    {
                        reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
                        token_string[str_tmp_size]='\b';
                        str_tmp_size++;

                 }




<STRINGT>\\\" 	{
			reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
			token_string[str_tmp_size]='\"';
			str_tmp_size++;	
		}

<STRINGT>\\\\ 	{
			reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
			token_string [str_tmp_size]='\\';
			str_tmp_size++;
		}


<STRINGT>[^\\\n\"\\\t]+	{
			char * tmp_str=strdup(yytext);
			while (*tmp_str)
			{
				reallocateIfNeeded(token_string, string_to_list, &str_tmp_size, &str_size);
				token_string[str_tmp_size]=*tmp_str;
				/*DEBUG Print printf("%c",token_string[str_tmp_size]);*/
				tmp_str++;
				str_tmp_size++;	
			}
		}

<STRINGT><<EOF>> {
			printf("Lexical Analyzer Error: <Open String Left>. \n");
			return EOF;
		}



"/*"	lcount++; BEGIN(COMMENT);
<COMMENT>[^*^/]*
<COMMENT>"/"+[^*]*
<COMMENT>"*"+[^*/]*
<COMMENT>"/"+"*"	lcount++;
<COMMENT>"*"+"/" 	{
				lcount--;
				if (lcount==0)
				{	
					BEGIN(INITIAL);			
				}
                	}

<COMMENT><<EOF>> 	{
				printf("Lexical Analysis Error: <Open Comment Left>. \n"); 
				return EOF;
		     	}



"while" 	{return WHILE;}
"for" 		{return FOR;}
"if" 		{return IF;}
"else" 		{return ELSE;}
"function" 	{return FUNCTION;}
"return" 	{return RETURN;}
"break" 	{return BREAK;}
"continue" 	{return CONTINUE;}
"and"    	{return AND;}
"&&"		{return AND;}
"or" 		{return OR;}
"||"		{return OR;}
"not"    	{return NOT;}
"!"		{return NOT;}
"local" 	{return LOCAL;}
"true" 		{return TRUE;}
"false" 	{return FALSE;}
"nil" 		{return NIL;}

"="		{return ASSIGN;}
"+"		{return PLUS;}
"-"		{return MINUS;}
"*"		{return MUL;}
"/"		{return DIV;}
"%"		{return MOD;}
"=="		{return EQL;}
"!="		{return N_EQL;}
"++"		{return PLUSPLUS;}
"--"		{return MINUSMINUS;}
">"		{return GRTR;}
"<"		{return LESS;}
">="		{return GRTR_EQL;}
"<="		{return LESS_EQL;}

"{"		{return L_BRACE;}
"}"		{return R_BRACE;}
"["		{return L_BRACKET;}
"]"		{return R_BRACKET;}
"("		{return L_PARENTH;}
")"		{return R_PARENTH;}
";"		{return SEMICOLON;}
","		{return COMMA;}
":"		{return COLON;}
"::"		{return SCOPE;}
"."		{return DOT;}
".."		{return DBL_DOT;}


{commentline}           {}

{whitespace}            {}

{identifier}            {yylval.strval=strdup(yytext);return IDENTIFIER;}

{integer}               {yylval.intval = atoi(yytext);return INTEGER;}

{realnumber}            {yylval.doubleval = atof(yytext);return REALNUMBER;}

{wrong_identifier}      {printf("Line %d: Lexical analysis error:You can not have an identifier starting with number or symbol $ (WRONG DEFINITION). \n",yylineno);return WRONG_DEFINITION;}


<<EOF>>			{return EOF;}

%%

void reallocateIfNeeded(char * token_string,char * string_to_list,int *str_tmp_size,int *str_size)
{
	char * alloc_mem;	
	int l_size=*str_size;
	int l_tmp_size= *str_tmp_size;
	if (l_tmp_size == l_size)
	{
		(*str_size) = l_size * 2;
		alloc_mem = (char *)malloc(sizeof(char)*(l_size));
		strcpy(alloc_mem,token_string);
		token_string = alloc_mem;
		string_to_list = alloc_mem+strlen(alloc_mem);
	}
	return;
}
