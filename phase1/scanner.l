%{
	#define YY_DECL int alpha_yylex(void* yyval)
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	
	
	int yytokenno = 0;
	int yycount = 0;
	int string_size; 
	int string_curr;
	char* string_from_token;
	char* string_to_node;
	char* new_memory;

	
	
	typedef enum 	type{IDENTIFIER, INTCONST, REALCONST, PUNCTUATION,OPERATOR, WHITESPACE, KEYWORD , NOMATCH, COMMENTS, STR}types;
	typedef enum	key{IF, ELSE, WHILE, FOR, FUNCTION, RETURN, BREAK, CONTINUE, AND, NOT, OR, LOCAL, TRUE, FALSE, NIL, NTHNG}keywords;

	typedef struct node alpha_token_t;

	struct node{
		int 	line_number;
		int 	token_number;
		char* 	token;
		types 	type;
		char*	ch_value;
		float	num_value;
		keywords	key;
		alpha_token_t*	next;
	};

	void insert_node(alpha_token_t *head,int line_number,int token_number,char* token,types type,char* ch_value,float num_value, keywords key);
	keywords keywords_to_enum(char* text);
	void print_results(alpha_token_t *head);
	
%}
%option noyywrap
%option yylineno
%option header-file="./al.h"


identifier		[a-zA-Z_][a-zA-Z_0-9]*

intconst		(0[xX][0-9a-fA-F]+)|[0-9]+

realconst		([0-9]+)\.([0-9](e|E)?[0-9]*)

punctuation		(\{)|(\})|(\()|(\))|(\[)|(\])|(\:)|(\,)|(\:\:)|(\.)|(\.\.)|(\;)

operator		(\+)|(\-)|(\*)|(\/)|(\=)|(\%)|(\+\+)|(\-\-)|(\>)|(\<)|(\>\=)|(\<\=)|(\=\=)|(\!\=)

whitespace		[\r\n \t\v]

keyword			(function)|(while)|(for)|(if)|(else)|(continue)|(break)|(return)|(and)|(or)|(not)|(local)|(true)|(false)|(nil)

commentline		"//".*

wrong_identifier	[0-9][a-zA-Z_0-9]*

%x COMMENT
%x STRING

%% 

\"		{
			string_from_token="";
			string_size=1; 
			string_curr=0; 
			string_from_token=malloc(sizeof(char)*string_size);
			string_to_node=string_from_token; 
			BEGIN(STRING);
		}
<STRING>\"	{
			if (string_size==string_curr){
				string_size=string_size*2; 
				new_memory=(char *)malloc(sizeof(char)*string_size);
				string_from_token='\0';
				strcpy(new_memory, string_to_node);
				string_to_node=new_memory;
				new_memory=&(new_memory[strlen(new_memory)]);
				string_from_token=new_memory;
			}
			*string_from_token='\0';
			insert_node(yyval, yylineno, ++yytokenno, string_to_node, STR, string_to_node, -1, NTHNG);
			BEGIN(INITIAL);
		}
<STRING>\\n	{
			if (string_size==string_curr){
				string_size=string_size*2; 
				new_memory=(char *)malloc(sizeof(char)*string_size);
				string_from_token='\0';
				strcpy(new_memory, string_to_node);
				string_to_node=new_memory;
				new_memory=&(new_memory[strlen(new_memory)]);
				string_from_token=new_memory;
			} 
			*string_from_token='\n';
			string_from_token++;
			string_curr++;
		}
<STRING>\\t	{
			if (string_size==string_curr){
				string_size=string_size*2; 
				new_memory=(char *)malloc(sizeof(char)*string_size);
				string_from_token='\0';
				strcpy(new_memory, string_to_node);
				string_to_node=new_memory;
				new_memory=&(new_memory[strlen(new_memory)]);
				string_from_token=new_memory;
			}
			*string_from_token='\t'; 
			string_from_token++; 
			string_curr++;
		}
<STRING>\\\" 	{
			if (string_size==string_curr){
				string_size=string_size*2; 
				new_memory=(char *)malloc(sizeof(char)*string_size);
				string_from_token='\0';
				strcpy(new_memory, string_to_node);
				string_to_node=new_memory;
				new_memory=&(new_memory[strlen(new_memory)]);
				string_from_token=new_memory;
			}
			*string_from_token='\"';
			string_from_token++; 
			string_curr++;
		}
<STRING>\\\\ 	{
			if (string_size==string_curr){
				string_size=string_size*2; 
				new_memory=(char *)malloc(sizeof(char)*string_size);
				string_from_token='\0';
				strcpy(new_memory, string_to_node);
				string_to_node=new_memory;
				new_memory=&(new_memory[strlen(new_memory)]);
				string_from_token=new_memory;
			}
			*string_from_token='\\';
			string_from_token++; 
			string_curr++;
		}
<STRING>[^\\\n\"\\\t]+	{
				char* tmp=yytext;
				while(*tmp){
					if (string_size==string_curr){
						string_size=string_size*2; 
						new_memory=(char *)malloc(sizeof(char)*string_size);
						string_from_token='\0';
						strcpy(new_memory, string_to_node);
						string_to_node=new_memory;
						new_memory=&(new_memory[strlen(new_memory)]);
						string_from_token=new_memory;
					}
					*string_from_token=*tmp;
					string_from_token++;
					tmp++;
					string_curr++;
				}
			}
<STRING><<EOF>> {
			printf("STRING ERROR <<Open String Left>>. \n");
			return -1;
		}

"/*"	yycount++; BEGIN(COMMENT);

<COMMENT>[^*^/]*
<COMMENT>"/"+[^*]*
<COMMENT>"/"+"*"	yycount++;
<COMMENT>"*"+[^*/]*
<COMMENT>"*"+"/" {
			yycount--;
			if(yycount==0) {	
				insert_node(yyval,yylineno, ++yytokenno, "/**/", COMMENTS, "/**/", -1, NTHNG);
				BEGIN(INITIAL);
			}
			else{
				insert_node(yyval,yylineno, ++yytokenno, "/**/", COMMENTS, "/**/", -1, NTHNG);
			}
		}
<COMMENT><<EOF>> {
			printf("COMMENT ERROR <<Open Comment >>. \n"); 
			return -1;
		 }



{commentline}		{insert_node(yyval, yylineno, ++yytokenno, "//", COMMENTS, "//", -1, NTHNG); }

{whitespace}		{  }


{keyword}		{ keywords keywrd = keywords_to_enum(yytext);
					insert_node(yyval, yylineno, ++yytokenno, yytext, KEYWORD, yytext, -1, keywrd); }

{identifier}		{ insert_node(yyval, yylineno, ++yytokenno, yytext, IDENTIFIER, yytext, -1, NTHNG);}

{intconst}		{ insert_node(yyval, yylineno, ++yytokenno, yytext, INTCONST, NULL, atoi(yytext), NTHNG); }


{realconst}		{ insert_node(yyval, yylineno, ++yytokenno, yytext, REALCONST, NULL, atof(yytext), NTHNG); }

{punctuation}		{ insert_node(yyval, yylineno, ++yytokenno, yytext, PUNCTUATION, yytext, -1, NTHNG); }

{operator}		{ insert_node(yyval, yylineno, ++yytokenno, yytext, OPERATOR, yytext, -1, NTHNG); }

<<EOF>>			{printf("The whole file has been analyzed.\n"); return 1;}

.			{ 
				printf("%d:  #%d      '%s'     NOMATCH     '%s'\n",yylineno, ++yytokenno, yytext, yytext );


//insert_node(yyval, yylineno, ++yytokenno, yytext, UNKNOWN, yytext, -1, NTHNG); 

			}

{wrong_identifier}	{
				printf("Error.You can not have an identifier starting with number. \n"); 
				return -1;
			}

%%

void allocate_str_mem(){
	if (string_size==string_curr){
		string_size=string_size*2; 
		new_memory=(char *)malloc(sizeof(char)*string_size);
		string_from_token='\0';
		strcpy(new_memory, string_to_node);
		string_to_node=new_memory;
		new_memory=&(new_memory[strlen(new_memory)]);
		string_from_token=new_memory;
	}

}

void insert_node(alpha_token_t *head,int line_number,int token_number,char* token,types type,char* ch_value,float num_value, keywords key){

	alpha_token_t *new_node, *tmp, *here;

	new_node=(alpha_token_t *)malloc(sizeof(alpha_token_t));
	new_node->line_number=line_number;
	new_node->token_number=token_number;
	new_node->token=strdup(token);
	new_node->type=type;
	new_node->key=key;
	new_node->next=NULL;
	if (ch_value!=(char *)NULL){
		new_node->ch_value=strdup(ch_value);
	}
	else if (num_value>-1){
		new_node->num_value=num_value;
	}
	tmp=head;
	while(tmp){
		here=tmp;
		tmp=tmp->next;
	}
	here->next=new_node;
	
	return;
}

keywords keywords_to_enum(char* text){

	if(!strcmp(text,"if")) return IF;
	else if(!strcmp(text,"else")) return ELSE;
	else if(!strcmp(text,"while")) return WHILE;
	else if(!strcmp(text,"for")) return FOR;
	else if(!strcmp(text,"function")) return FUNCTION;
	else if(!strcmp(text,"return")) return RETURN;
	else if(!strcmp(text,"break")) return BREAK;
	else if(!strcmp(text,"continue")) return CONTINUE;
	else if(!strcmp(text,"and"))	return AND;
	else if(!strcmp(text,"not"))	return NOT;
	else if(!strcmp(text,"or")) return OR;	
	else if(!strcmp(text,"local")) return LOCAL;
	else if(!strcmp(text,"true")) return TRUE;
	else if(!strcmp(text,"false")) return FALSE;
	else if(!strcmp(text,"nil")) return NIL;
}

void print_results(alpha_token_t *head){

	alpha_token_t* tmp;
	char* token;
	tmp=head->next;
	int integer;
	
	while(tmp){
		integer=tmp->num_value;
		switch(tmp->type){
			case OPERATOR: printf("%d:  #%d      '%s'     OPERATOR     %s\n",tmp->line_number, tmp->token_number, tmp->token, tmp->ch_value ); break;
			case INTCONST: printf("%d:  #%d      '%s'     INTCONST     %d\n",tmp->line_number, tmp->token_number, tmp->token, integer ); break;
			case REALCONST: printf("%d:  #%d      '%s'    REALCONST     %f\n",tmp->line_number, tmp->token_number, tmp->token, tmp->num_value ); break;
			case PUNCTUATION: printf("%d:  #%d      '%s'     PUNCTUATION     %s\n",tmp->line_number, tmp->token_number, tmp->token, tmp->ch_value ); break;
			case IDENTIFIER: printf("%d:  #%d      '%s'    IDENTIFIER     '%s'\n",tmp->line_number, tmp->token_number, tmp->token, tmp->ch_value ); break;
			case KEYWORD:
				switch (tmp->key){
					case IF: token=strdup("IF"); break;
					case ELSE: token=strdup("ELSE"); break;
					case WHILE: token=strdup("WHILE"); break;
					case FOR: token=strdup("FOR"); break;
					case FUNCTION: token=strdup("FUNCTION"); break;
					case RETURN: token=strdup("RETURN"); break;
					case BREAK: token=strdup("BREAK"); break;
					case CONTINUE: token=strdup("CONTINUE"); break;
					case AND: token=strdup("AND"); break;
					case NOT: token=strdup("NOT"); break;
					case OR: token=strdup("OR"); break;
					case LOCAL: token=strdup("LOCAL"); break;
					case TRUE: token=strdup("TRUE"); break;
					case FALSE: token=strdup("FALSE"); break;
					case NIL: token=strdup("NIL"); break;
					case NTHNG: token=strdup("NTHING"); break;
					default: printf("Case Keyword, Default state\n");
				}
				printf("%d:  #%d      '%s'    KEYWORD     %s\n",tmp->line_number, tmp->token_number, tmp->token, token ); break;
			case WHITESPACE: printf("%d:  #%d      '%s'     WHITESPACE     '%s'\n",tmp->line_number, tmp->token_number, tmp->token, tmp->ch_value ); break;
			case COMMENTS:  printf("%d:  #%d      '%s'     COMMENT     '%s'\n",tmp->line_number, tmp->token_number, tmp->token, tmp->ch_value ); break;
			case STR: 	printf("%d:  #%d      '%s'   STRING     '%s'\n",tmp->line_number, tmp->token_number, tmp->token, tmp->ch_value ); break;
			case NOMATCH: printf("%d:  #%d      '%s'     NOMATCH     '%s'\n",tmp->line_number, tmp->token_number, tmp->token, tmp->ch_value ); break;
			default: printf("Case Default on State Type\n");
		}
		tmp=tmp->next;
	}
}



int main (int argc, char* argv[]){
	system("clear");
	
	alpha_token_t *head=NULL;
	char c;
	head=(alpha_token_t *)malloc(sizeof(alpha_token_t));
	head->next=NULL;
	
	

	if(argc>1){//diorthwsi gia to plithos twn args
		if(!(yyin = fopen(argv[1],"r"))){
			fprintf(stderr, "Cannot read file %s\n",argv[1]);
			return -1;
		}
	}
	else{
		yyin = stdin;
	}
	
	alpha_yylex(head);

	printf("\n\nValid TOKENS from file:\n\n");
	print_results(head);
	return 0;
}
