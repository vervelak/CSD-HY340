#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbolt.h"
#include "functions.h"
 

extern int deepestScope;
extern int deepestFunctionScope;
extern scopespace_t scopeSpaceCounter;

Symbol *hashtable[ HASH_SIZE ];
fStack *headfStack;

char *libFN[ 12 ];

int ins_lib_func = 0;
extern int f_prefix;
extern int isLocalvar;


int generate(char * name)
{
	int i = 0;
	int total=0;
	while (i<strlen(name)){
		total+=(int)name[i];
		i++;
	}
	return total;
}

int hashF( char *name )
{
	int i = ((( generate(name))) % HASH_SIZE);
	return i;
}

Symbol *createS( char *name, int scope, int line, char *nameF,
		int scopeF, int lineF, char *type, int active, unsigned int spscope,unsigned int offset)
{
	Symbol *newS = malloc(sizeof(Symbol));
	if( newS == NULL ){
		printf("No space available!\n");
		return NULL;
	}
	newS->name=strdup(name);
	newS->scope = scope;
	newS->line = line;
	
	newS->nameF= strdup(nameF);
	newS->scopeF = scopeF;
	newS->lineF = lineF;
	newS->type=strdup(type);
	newS->active = active;
	newS->spscope = spscope;
	newS->offset = offset;
	newS->next = NULL;
	return newS;
}

int hideH( Symbol *S )
{
	if(S != NULL){
		S->active = 0;
		return 1;
	}
	else{
		return 0;
	}
}

void initializeH(){
	int it=0;
	while(it<12){
		libFN[it] = (char *)malloc(sizeof(char)*50) ;
		it++;
	}
	ins_lib_func = 1;
	insertH(createS( "print", 0, 0, "print", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
	libFN[0] = "print";
	insertH(createS( "input", 0, 0, "input", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
	libFN[1] = "input";
	insertH( createS( "objectmemberkeys", 0, 0, "objectmemberkeys", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
	libFN[2] = "objectmemberkeys";
	insertH( createS( "objecttotalmembers", 0, 0, "objecttotalmembers", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
	libFN[3] = "objecttotalmembers";
	insertH( createS( "objectcopy", 0, 0, "objectcopy", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
	libFN[4] = "objectcopy";
 	insertH( createS( "totalarguments", 0, 0, "totalarguments", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
 	libFN[5] = "totalarguments";
	insertH( createS( "argument", 0, 0, "argument", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
	libFN[6] = "argument";
	insertH( createS( "typeof", 0, 0, "typeof", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
	libFN[7] = "typeof";
	insertH( createS( "strtonum", 0, 0, "strtonum", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
	libFN[8] = "sqrtonum";
	insertH( createS( "sqrt", 0, 0, "sqrt", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
	libFN[9] = "sqrt";
	insertH( createS( "cos", 0, 0, "cos", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
	libFN[10] = "cos";
	insertH( createS( "sin", 0, 0, "sin", 0, 0, "lib_func", 1,currscopespace(),currscopeoffset()));
	libFN[11] = "sin";
	ins_lib_func = 0;
}



Symbol *lookUpH( Symbol *S , char *mode , char *crit )
{
	int k;
	Symbol *tmp = malloc( sizeof( Symbol ) );
	if( tmp == NULL ){
		printf("No space available!\n");
		return NULL;
	}
	else if( ((strcmp( mode , "funcdef" ) ) == 0 ) && ( strcmp( crit , "samescope") == 0 ) ){ 	//an psaxnoume function definition active
		k = hashF( S->name );
		tmp = hashtable[k];
		while( tmp != NULL ){
			if (!strcmp(tmp->type,"func")){
				if(strcmp( S->name , tmp->name ) == 0 && (tmp->scopeF==S->scopeF ) && tmp->active == 1) {
					return tmp;
				}
			}
			else if  (!strcmp(tmp->type,"var")){
				if(strcmp( S->name , tmp->name ) == 0 && (tmp->scopeF==S->scopeF) && tmp->active == 1){
					return tmp;
				}
			}
			else if  (!strcmp(tmp->type,"formarg")){
				if(strcmp( S->name , tmp->name ) == 0 && (tmp->scopeF==S->scopeF) && tmp->active == 1){
					return tmp;
			   	}
			}
			if( ((strcmp( S->name , tmp->name ) ) == 0) && 
				(strcmp( tmp->type , "func") == 0) && ( S->scopeF == tmp->scopeF) && tmp->active == 1){
				return tmp;

			}
			tmp = tmp->next;
		}
		return NULL;
	}
	else if( (strcmp( mode , "funcdef" ) ) == 0 ){  //an psaxnoume function definition ***PREPEI NA TSEKARW AN EINAI ERROR AN VRISKW GENIKA FUNCTION ME $
		k = hashF( S->name );                                  
		tmp = hashtable[k];
		while( tmp != NULL ){
			if( ((strcmp( S->name , tmp->name ) ) == 0) && ( strcmp( tmp->type , "func") == 0) ){
				return tmp;     //an vro node me idio onoma kai einai function to epistrefo
			}
			tmp = tmp->next;
		}
		return NULL;
	}
	else if( ( strcmp( mode , "global" ) ) == 0 ){		//an psaxnoume gia global 
		k = hashF( S->name );
		tmp = hashtable[ k ];
		while( tmp!= NULL ){
			if( (strcmp( S->name , tmp->name ) ) == 0 && tmp->scope == 0 ){
				return tmp;
			}
			tmp = tmp->next;
		}
		return NULL;
	}
	else if( ( strcmp( mode , "funcvar" )== 0 ) && (strcmp( crit , "fsc_ac") == 0) ){//psaxnw variable i function me kritirio Fscope kai active
		k = hashF( S->name );
		tmp = hashtable[ k ];
		while( tmp!= NULL ){
			if( ( (strcmp( S->name , tmp->name )  == 0) &&
				(S->scopeF > tmp->scopeF) && (tmp->active == 1) ) ){//an vrw kapoia me to idio onoma alla mikrotero scopeF kai einai kai active
				return tmp;
			}
			tmp = tmp->next;
		}
		return NULL;
	}
	else if( ( strcmp( mode , "func" )== 0 ) && (strcmp( crit , "fsc_ac") == 0) ){//psaxnw variable i function me kritirio Fsc$
		int k=0;
		while (k<HASH_SIZE){
			tmp = hashtable[ k ];
			while( tmp!= NULL ){
				if( ( (strcmp( tmp->type , "func")  == 0) &&
					(S->scopeF == tmp->scopeF) && (tmp->active == 1) )){     //an vrw kapoia me to idio o$
                                	return tmp;

                        	}
				tmp = tmp->next;
			}
			k++;
		}
		return NULL;
	}
	else if( ( strcmp( mode , "funcvar" ) == 0 ) && 
		(strcmp( crit , "name") == 0) ){//psaxnw variable i function me kritirio to onoma
		k = hashF( S->name );
		tmp = hashtable[ k ];
		while( tmp!= NULL ){
			if( (strcmp( S->name , tmp->name )  == 0) ){	//an vrw kapoia me to idio onoma
				return tmp;

			}
			tmp = tmp->next;
		}
		return NULL;
	}
	else if( ( strcmp( mode , "func" ) == 0 ) && (strcmp( crit , "name") == 0) ){	//psaxnw variable i function me kritirio to onoma
		k = hashF( S->name);
		tmp = hashtable[ k ];
		while( tmp!= NULL ){
			if( (strcmp( S->name , tmp->name )  == 0) && (tmp->active==1) &&
				( (strcmp(tmp->type,"func")  == 0)  ||
				(strcmp(tmp->type,"lib_func")  == 0))){	//an vrw kapoia me to idio onoma
				return tmp;
			}
			tmp = tmp->next;
		}
		return NULL;
	}
	else if(( strcmp( mode, "funcvar") ==0) && (strcmp( crit, "illscope") == 0)){ //periptwsi illegal scoping se var.
		k = hashF( S->name );
		tmp = hashtable[ k ];
		while( tmp != NULL ){
			 if  (!strcmp(tmp->type,"var")){
			 	printf("Exei perasei apo dw gia to testing mas.\n");
				if(strcmp( S->name , tmp->name ) == 0 && tmp->scopeF< S->scopeF && tmp->scope != 0){
					return tmp;
			   	}
			}
			else if  (!strcmp(tmp->type,"formarg")){
				printf("Exei perasei apo dw gia to testing mas.\n");
				if(strcmp( S->name , tmp->name ) == 0 && (tmp->scopeF!=S->scopeF)){
					return tmp;
				}
			}
			tmp = tmp->next;
		}		
		return NULL;
	}
	else if(( strcmp( mode , "funcvar" ) == 0 ) && (strcmp( crit , "sscope" ) == 0) ){ //periptwsi valid alla refference scope se var.
		k = hashF( S->name );
		tmp = hashtable[ k ];
		while( tmp != NULL ){
			if (!strcmp(tmp->type,"func")){
				if(strcmp( S->name , tmp->name ) == 0 && (tmp->scopeF==S->scopeF || tmp->scope == 0)) {
					return tmp;
				}
			}
			else if (!strcmp(tmp->type,"var")){
				if(strcmp( S->name , tmp->name ) == 0 && (tmp->scopeF==S->scopeF || tmp->scope == 0)){
					return tmp;
				}
			}
			else if (!strcmp(tmp->type,"formarg")){
				if(strcmp( S->name , tmp->name ) == 0 && (tmp->scopeF==S->scopeF)){
					return tmp;
				}
			}
			tmp = tmp->next;
		}		
		return NULL;
	}
	else if( ( strcmp( mode , "funcvar" ) == 0 ) && (strcmp( crit , "localscope" ) == 0) ){//psaxnw function i var me to idio onoma kai to idio scope
		k = hashF( S->name );
		tmp = hashtable[ k ];
		while( tmp != NULL ){
			if(( strcmp( S->name , tmp->name ) ) == 0 && ( S->scopeF == tmp->scopeF ) ){
				return tmp;
			}
			tmp = tmp->next;
		}
		return NULL;
	}
	else if( ( strcmp( mode , "funcvar" ) == 0 ) && (strcmp( crit , "localillscope" ) == 0) ){//psaxnw function i var me to idio onoma kai to idio scope
		k = hashF( S->name );
		tmp = hashtable[ k ];
		while( tmp != NULL ){
			if(!strcmp(tmp->type,"func") && strcmp( S->name , tmp->name  ) == 0 && ( S->scopeF == tmp->scopeF)){
				return tmp;
			}
			tmp = tmp->next;
		}		
	return NULL;
	}
	else if( ( strcmp( mode , "funcvar" ) == 0 ) && (strcmp( crit , "localscope" ) == 0) ){ //psaxnw function i var me to idio onoma kai to idio scope
		k = hashF( S->name );
		tmp = hashtable[ k ];
		while( tmp != NULL ){
			if(( strcmp( S->name , tmp->name ) ) == 0 && ( S->scopeF == tmp->scopeF ) ){
				return tmp;
			}
			tmp = tmp->next;
		}		
		return NULL;
	}	
	else if( ( strcmp( mode , "funcvar" ) == 0 ) && (strcmp( crit , "globalscope" ) == 0) ){//psaxnw function i var me to idio onoma kai to idio scope
		k = hashF( S->name );
		tmp = hashtable[ k ];
		while( tmp != NULL ){
			if( ( strcmp( S->name , tmp->name ) ) == 0 && ( tmp->scopeF == 0 ) ){
				return tmp;
			}
			tmp = tmp->next;
		}		
		return NULL;
	}
	else if( ( strcmp( mode , "formarg" ) == 0 ) && (strcmp( crit , "fsc_ac" ) == 0) ){//elegxos gia formal arguments me idio onoma sto idio scope kai me to idio nameF
		k = hashF( S->name );
		tmp = hashtable[ k ];
		while( tmp != NULL ){
			if( ( strcmp( S->name , tmp->name ) ) == 0 &&
				( S->scopeF == tmp->scopeF ) && ( tmp->active == 1 ) ){
				return tmp;
			}
			tmp = tmp->next;
		}
		return NULL;
	}
	else{
		return NULL;
	}
}

int insertH( Symbol *newS )
{

	int i;
	Symbol *tmp = malloc( sizeof( Symbol ) );

	if( tmp == NULL ){
		printf("No space available!\n");
		return 0;
	}
	Symbol *nnode = malloc( sizeof( Symbol ) );
	if( nnode == NULL ){
		printf("No space available!\n");
		return 0;
	}
	nnode->name = strdup(newS->name);
	nnode->scope = newS->scope;
	nnode->line = newS->line;
	nnode->nameF = strdup(newS->nameF);
	nnode->scopeF = newS->scopeF;
	nnode->lineF = newS->lineF;
	nnode->type = strdup(newS->type);
	nnode->spscope = newS->spscope;
	nnode->offset = newS->offset;
//na ta valw edw.
	nnode->active = newS->active;
	nnode->next = NULL;
	/*
	*
	*
	* ELEGXOI KANONWN EISAGOGIS STO SYMBOLTABLE
	*
	*/

	if( ins_lib_func == 0 ){
		for( i=0; i<12; i++){	//elegxos an eisagoume variable pou exei to idio onoma me kapoia library function
			if(((strcmp( newS->type , "var")==0) || (strcmp( newS->type , "func")==0) ||
				(strcmp( newS->type , "global")==0) || (strcmp( newS->type , "local")==0) ||
				(strcmp( newS->type, "formarg")==0)) && (strcmp( newS->name, libFN[i]) == 0)){
				printf("Den ginetai eisagogi var i function pou na exei idio onoma me mia library function\n");
				return 0;
			}
		}
		tmp = lookUpH( newS , "funcdef" , "" );	//elegxos an eisagoume node me onoma pou einai idio me mia active function
		if( tmp != NULL ){					
			if( tmp->scopeF == newS->scopeF && tmp->active == 1 ){
				printf("Den ginetai eisagogi var pou na exei idio onoma me funct sto idio scope.\n");
				return 0;
			}
		}
		tmp = lookUpH( newS , "global" , ""); //elegxos an theloume na tsekaroume kolision me mia global metavliti
		if (isLocalvar!=1 && strcmp(newS->type,"formarg")!=0 && strcmp(newS->type,"func")!=0){
			if( tmp != NULL ){
				printf("Den ginetai eisagogi var pou na exei idio onoma me mia global/local metavliti\n");
				return 0;
			}
		}
		tmp = lookUpH( newS , "var" , "fsc_ac"); //elegxos gia metavliti pou kanei conflict me idio onoma metavlitis i function mikroterou FUNCTION scope pou einai kai active ( mesolavei function )
		if( tmp != NULL ){
			printf("Den ginetai eisagogi var pou na exei idio onoma me mia var pou einai se mikrotero function scope ( mesolavei function )\n");
			return 0;
		}
		tmp = lookUpH( newS , "funcvar" , "scope" ); //elegxos gia dilosi function i variable me to idio onoma kai sto idio scope
		if( tmp != NULL ){
			printf("Den ginetai eisagogi stoixeiou pou na exei idio onoma me mia function i ena variable sto idio scope\n");
			return 0;
		}
		tmp = lookUpH( newS , "formarg" , "scope" );
		if( tmp != NULL ){
			printf("Den ginetai eisagogi formal arguments pou na exei idio onoma me allo formal argument idiou scope kai anikoun stin idia sinartisi\n");
			return 0;
		}
	}
	/*
	*
	*
	* EISAGOGI STO SYMBOLTABLE
	*
	*
	*/
	i = hashF( newS->name );
	if( hashtable[ i ] == NULL ){
		hashtable[ i ] = nnode;
		return 1;
	}
	else{
		tmp = hashtable[ i ];
		while( tmp->next != NULL ){
			tmp = tmp->next;
		}
		tmp->next = nnode;
		return 1;
	}
}

void hideInFunction()
{	// kanei hide oles tis metavlites otan kleinei to bracket tis function
	Symbol *temp = (Symbol *)malloc( sizeof( Symbol ) );
	int i;	

	for ( i = 0; i<HASH_SIZE; i++){
		temp = hashtable[i];
		if (temp != NULL ){	
			while(temp!=NULL){
				if((temp->scope > deepestFunctionScope) && temp->active == 1){
					temp->active = 0;
				}
				temp=temp->next;
			}
		}
	}
	return;
}

void setFunctionPrefix( char *prefix )
{
	sprintf( prefix , "$f%d" , f_prefix );
	f_prefix++;
}

void printHash()
{
	Symbol *tmp = malloc( sizeof( Symbol ) );
	int i ;

	for( i = 0 ; i<HASH_SIZE; i++){
		tmp = hashtable[ i ];
		while( tmp != NULL ){
			printf("localVars:%d, name:%s , type:%s, function: %s , scope:%d , fscope:%d , fline:%d\n  active:%d, scopespace:%d, offset: %d \n" ,tmp->line, tmp->name , tmp->type,tmp->nameF , tmp->scope , tmp->scopeF ,tmp->lineF, tmp->active, tmp->spscope, tmp->offset );
			tmp = tmp->next;
		}
	}
}

void initfStack()
{
	headfStack= malloc(sizeof(fStack));
}


void pushfStack( Symbol * sym)
{
	fStack *node = malloc(sizeof(fStack));
	fStack *headref = headfStack;	
	node->sym = sym;
	while(headref->next != NULL){
		headref=headref->next;
	}
	headref->next = node;
}

fStack * popfStack(){
	fStack *noderef = headfStack;
	if(!headfStack) return NULL;
	fStack *prevref;
	fStack *retref;
	while(noderef->next!= NULL){
		prevref = noderef;
		noderef = noderef->next;
	}
	prevref->next = NULL;
	return noderef;
}

char * topfStackName(){
	fStack *noderef = headfStack;
	
	while(noderef->next!= NULL){
		noderef = noderef->next;
	}
	if (noderef->sym==NULL){
		return "_no_func";
	}
	else{
		return strdup(noderef->sym->name);
	}
}
void findLocal(char *name){
	int k = 0;
	Symbol *tmp;
	int i;
	int j;
	for(i =0; i<1000;i++){
		if (hashtable[i]!=NULL){
			tmp = hashtable[i];
			while(tmp !=NULL){
				if ((strcmp(tmp->nameF, name)==0)&&(tmp->active == 1)&&(strcmp(tmp->type,"var")==0)){
					k++;
				}
				tmp = tmp->next;
			}
		}
	}
	for(j =0; j<1000;j++){
		if (hashtable[j]!=NULL){
			tmp = hashtable[j];
			while(tmp !=NULL){
				if ((strcmp(tmp->name, name)==0)&&(tmp->active == 1)){
					tmp->line = k;
					break;
				}
				tmp = tmp->next;
			}
		}
	}
}
