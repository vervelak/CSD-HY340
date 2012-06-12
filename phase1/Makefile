#	vervelak luludak atroul | 1st Phase @ Compilers | MAKEFILE | 
#	-------------------------|
CC=gcc -g 
FL=flex 
FFLAGS= --outfile 
OFLAGS=-o 
REM=rm -f 
XF=.out 
CF=.c 
HF=.h 

all: al.c
	$(CC) $(OFLAGS) lexan.out al.c 

al.c:	lexan.l
	$(FL) $(FFLAGS)=al.c lexan.l
	
clean:	lexan.out al.c al.h
	$(REM) *$(XF) *$(CF) *$(HF)
