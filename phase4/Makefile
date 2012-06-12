#	vervelak luludak atroul	 |
#	4nd Phase @ Compilers	 |
#	MAKEFILE		 |
#	-------------------------|

CC= gcc 

FL=flex
BS=bison
BSFLAGS= -v --yacc --defines
BSOUT=--output
FFLAGS= --outfile

OFLAGS=-o

GFLAGS=-g

REM=rm -f

XF=.out

CF=.c

all:	lexan.c parser.c symbolt.c functions.c target.c arrays.c
	$(CC) $(GFLAGS) $(OFLAGS) execu lexan.c parser.c symbolt.c functions.c target.c arrays.c

phase3:	lexan.c parser.c symbolt.c functions.c
	$(CC) $(GFLAGS) $(OFLAGS) execu lexan.c parser.c symbolt.c functions.c

lexan.c:	lexan.l
	$(FL) $(FFLAGS)=lexan.c lexan.l
	
parser.c:	parser.y
	$(BS) $(BSFLAGS) $(BSOUT)=parser.c parser.y
	 
clean:	
	$(REM) lexan.h parser.output parser.c parser.h execu lexan.c al.h icode.txt alpha.bin instructions.txt
