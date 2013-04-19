#ifndef HASHTABLE_H_
#define HASHTABLE_H_
 
#define HASH_SIZE 1000
typedef enum Scopespace_t scopespace_t;
typedef enum Symbol_t symbol_t;
typedef struct 	symT Symbol;

enum Scopespace_t{
	globalvar = 1,
	formalarg = 2,
	functionlocal = 3
	
};



enum Symbol_t{
	var_s,
	programfunc_s,
	libraryfunc_s
};



struct 	symT{
	char 	*name; //onoma tis metavlitis
	int 	scope;  //scope tis metavlitis
	int 	line;   //grammi tis metavlitis
	char	 *nameF; //onoma tis function pou vrisketai i metavliti
	int 	scopeF;  //scope tis function pou vrisketai i metavliti
	int 	lineF;   //line tis function pou vrisketai i metavliti
	char	 *type;  //tipos tou node (an einai func , lib_func , var, formarg )
	int 	active;  //0 an einai hide
	scopespace_t spscope;
	unsigned int offset;
	struct symT *next;
};


typedef struct funcStack{
	Symbol *sym;
	struct funcStack *next;
} fStack;

Symbol *hashtable[ HASH_SIZE ];		

/* Creates a new node and returns it */
Symbol *createS( char *name, int scope, int line, char *nameF,
		 int scopeF, int lineF, char *type, int active, unsigned int, unsigned int);	

/* Insert node to Symbol table */
int insertH( Symbol *newS );		

/* Vriskei ton komvo me to katallilo mode ( local gia local metavlites,
global gia global, var an den einai orismeni os global i local)
funcdef an einai function definition , formarg an einai formal arguments,
funcvar an thelo na vro eite function eite variable)
tsekaro to katallilo kritirio( active ,scope, sscope , illscope,
localillscope , localscope,globalscope , fsc_ac) */
Symbol *lookUpH( Symbol *S , char *mode , char *crit);

//Apli lookup me vasi to name kai to scope.
Symbol *sLookUpH(char *, unsigned int);


/* makes an active block inactive */
int hideH( Symbol *S );

/* adds library functions on symbol table */
void initializeH();

/* makes active vars in active inside function */
void hideInFunction( );

/* hash function */
int hashF( char *name );

/* name unamed functions */
void setFunctionPrefix( char *prefix );

/* prints symbol table */
void printHash();

/* initialization of fStack */
void initfStack();

/* push in fStack */
void pushfStack(Symbol *);

/* pop in fStack */
fStack *popfStack();

/*top name in fStack */
char * topfStackName();

/* Vriskei ton arithmo ton local metavliton se mia sinatrisi */
void findLocal(char *name);

#endif
