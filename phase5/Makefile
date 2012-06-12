#	vervelak luludak atroul	 |
#	5th Phase @ Compilers	 |
#	MAKEFILE		 |
#	-------------------------|

CC= gcc 


OFLAGS=-o

GFLAGS=-g

REM=rm -f

XF=.out

CF=.c

all:
	$(CC) $(GFLAGS) $(OFLAGS) avm avm.c exec_arithmetic.c exec_assign.c exec_funcs.c exec_statements.c structs.c

clean:
	$(REM) *.o avm
