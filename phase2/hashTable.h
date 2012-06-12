#define HASH_SIZE 1000



typedef struct symT {
    char *name; //onoma tis metavlitis
    int scope;  //scope tis metavlitis
    int line;   //grammi tis metavlitis
    char *nameF; //onoma tis function pou vrisketai i metavliti
    int scopeF;  //scope tis function pou vrisketai i metavliti
    int lineF;   //line tis function pou vrisketai i metavliti
    char *type;  //tipos tou node (an einai func , lib_func , var )
    int active;  //0 an einai hide
    struct symT *next;
} Symbol;

typedef struct funcStack {
    char *fname;
    struct funcStack *next;
} fStack;

Symbol *hashtable[ HASH_SIZE ];

/* Creates a new node and returns it */
Symbol *createS(char *name, int scope, int line, char *nameF,
                int scopeF, int lineF, char *type, int active);

/* Insert node to Symbol table */
int insertH(Symbol *newS);

/* Vriskei ton komvo me to katallilo mode ( local gia local metavlites,
global gia global, var an den einai orismeni os global i local)
funcdef an einai function definition , formarg an einai formal arguments,
funcvar an thelo na vro eite function eite variable)
tsekaro to katallilo kritirio( active , scope ,fscope , fsc_ac) */
Symbol *lookUpH(Symbol *S , char *mode , char *crit);

/* makes an active block inactive */
int hideH(Symbol *S);

/* adds library functions on symbol table */
void initializeH();

/* makes active vars in active inside function */
void hideInFunction();

/* hash function */
int hashF(char *name);

/* name unamed functions */
void setFunctionPrefix(char *prefix);

/* prints symbol table */
void printHash();

/* initialization of fStack */
void initfStack();

/* push in fStack */
void pushfStack(char *);

/* pop in fStack */
fStack *popfStack();

