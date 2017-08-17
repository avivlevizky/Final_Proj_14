

#ifndef Checkers_h
#define Checkers_h

#include <stdio.h>

/*Bool Function that checks if the label is valid by return boolean value, if will be an error the function will insert the match error into ErrorsAssembler table */
int isValidLabel(char * label,int flagDotDot);


/*int Function that checks if the given order string is an vald instruction/data defining order: if is .data then ->16, if is .string then ->17, if is .mat then ->18 ,if is .entry then -> 19, if is .extern then -> 20 ,if is mov ->func.mov,otherwise the function will return -1*/
/*flagMessage: indicate if to print an error message or not*/
int isInstruction(char * order, int flagMessage);


/* Function: search and return the index of the given label from the symbol table, if not exist return -1*/
unsigned findSymbol(char * data);



/*Int function that check if the given string is an int value and return is value otherwise the function will return null*/
int * isNumeric(char * data);


/*Boolean Function: check of the given string which type of addressing type it is*/
int isDirectOrRegister(char * data);


/*function that checks if the given string is a valid matrix defining : return the the number of places that the matrix is takes otherwise return -1 if the defining is not valid*/
int isValidMatrixToData(char * mat);


/*function that checks if the given string is a valid matrix : then return the string as array of strings otherwise return null*/
char ** isValidMatrix(char * mat);


int checkAddressingType(char * data);


#endif /* Checkers_h */
