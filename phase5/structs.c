#include "structs.h"

extern unsigned int libFunctionsFilled;
extern unsigned int userFunctionsFilled;
extern Symbol ** libFunctions;
extern Symbol ** userFunctions;

int FindLibFunction(char *fname){
	Symbol ** iter;
	int i =0;
	while (i<libFunctionsFilled){
		iter = libFunctions+i;		
		if ((!strcmp((*iter)->name,fname))){
			return i;
		}
		i++;
	}
	return -1;


}

int FindFunctionUserByAddr(unsigned int addr){
	Symbol ** iter;
	int i =0;
	while (i<userFunctionsFilled){
		iter = userFunctions+i;		
		if (((*iter)->taddress == addr)){
			return i;
		}
		++i;
	}
	return -1;


}
