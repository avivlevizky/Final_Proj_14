
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "Assembler.h"
#include "Checkers.h"
#include "Insertion.h"


Symbol ** symbol_table;               /*The symbols table*/
Instruction ** instructions_table;   /* for data and instruction order*/
char ** ErrorsAssembler;     /*Error in the compiling*/
int * data_table;             /*Int dynamic array to store all the data instructions*/
EntrySy ** EntSymbolsTable;             /*table that save all the indexes of the entrys labels*/
ExternSy ** ExtSymbolsTable;             /*table that save all the indexes of the externs labels*/
unsigned SymbolEntCount;                 /*entry symbols counter*/
unsigned SymbolExtCount;                 /*extern symbols counter*/
unsigned IC;                 /*Instruction table counter*/
unsigned Total_IC;           /*total of Instructions after the first iteration*/
unsigned DC;                 /*Data table counter*/
unsigned SC;                 /*Symbol counter*/
unsigned EC;                 /*Error counter*/
unsigned LC;                 /*Line counter*/
FILE * fp;                    /*FILE pointer to the given assembly file*/


/*Prototypes*/
void FirstCheckingCommand(char **);
void SecondCheckingCommand(char ** command);









/*function that checks if the new memory allocate was successed, if not the function will print to stderr a new error message and then will exit the program*/
void allocate_check(void * p)
{
    if(!p)
    {
        fprintf(stderr,"Error to allocate new memory\n");
        exit(0);
    }
    
}




/*private function that return the number of digits for the given number*/
int lenOfNum(int n)
{
	int ans;

	ans = 1;

	while ((n = (n / 10)) >= 1)
		ans++;

	return ans;

}




/*fucntion that insert new assembler error into ErrorsAssembler table */
void insertNewError(char * error)
{
    if (EC==0)
    {
        ErrorsAssembler = (char **)malloc(sizeof(char *));
        allocate_check((char **)ErrorsAssembler);
    }
    else
    {
        char ** temp;
        temp = (char **)realloc(ErrorsAssembler, (EC + 1) * sizeof(char *));
        allocate_check(temp);
        ErrorsAssembler = temp;
    }
    
    ErrorsAssembler[EC] = (char *)malloc(strlen(error) + lenOfNum(LC) + 1);
    sprintf(ErrorsAssembler[EC], error, LC); /*( puts string into buffer */
    ErrorsAssembler[EC][strlen(error) + lenOfNum(LC)] = '\0';
    EC++;
    
}





/*get the whole command and transfer it to linked list, if returned ans>0 then there is an error in the input otherwise the input is valid*/
void CommandLineToLinkedList(int NumIteration)
{
    char ** command;           /*dynamic matrix of strings*/
    char reader;              /*char variable to iterate on content std*/
    int chars_len, word_counter, isComa, isQuot, ignore,i;      /*chars_len: the char length of the current word; word_counter: indicate in the current number of word; isComa: indicate if coma was encourted more than one time ; isQueat: indicate if quotation marks apeared for ignore the creation of new string */
    
    LC++;
    ignore = 0;
    word_counter = 0;
    chars_len = 1;
    isComa = 0;
    isQuot = 0;
    command = (char **)malloc(sizeof(char *));
    allocate_check(command);            /*-------------Need to check if (char **)commands is valid------------*/
    command[0] = (char *)calloc(1, sizeof(char));
    allocate_check(command[0]);
    
Loop: while (((reader = fgetc(fp)) != EOF) && (reader != '\n'))
{
    if ((reader == ';') && (!word_counter) && (chars_len == 1))
        ignore = 1;
    
    
    if ((!ignore) && ((!isspace(reader)) && ((reader != ',') || (isComa))))
    {
        isQuot = (reader == '"') ? isQuot ^ 1 : isQuot;
        command[word_counter] = (char *)realloc((command[word_counter]), (chars_len + 1) * sizeof(char));
        allocate_check(command[word_counter]);
        command[word_counter][chars_len - 1] = reader;
        command[word_counter][chars_len] = '\0';
        chars_len++;
        isComa = 0;
    }
    else
    {
        if ((!isQuot) && (chars_len>1))
        {
            word_counter++;
            command = (char **)realloc(command, (word_counter + 1) * sizeof(char *));
            allocate_check(command);
            command[word_counter] = (char *)calloc(1, sizeof(char));
            allocate_check(command[word_counter]);
            chars_len = 1;
            isComa = (reader == ',');
        }
    }
}
    if (ignore)
    {
        LC++;
        ignore = 0;
        goto Loop;
    }
    /*Assign in the last+1 place a null (to indicate the end of the current command) */
    if (chars_len>1)
    {
        char ** tempT;
        
        word_counter++;
        tempT = (char **)realloc(command, (word_counter + 1) * sizeof(char *));
        allocate_check(command);
        command = tempT;
    }
    command[word_counter] = NULL;
    
    
    
    if (reader == '\n')
    {
        if (NumIteration == 1)
            FirstCheckingCommand(command);
        else
            SecondCheckingCommand(command);
        
        
        /*clean the command array*/
        i=0;
        while (i<=word_counter)
        {
            free(command[i]);
            i++;
        }
        free(command);
        
        
        CommandLineToLinkedList(NumIteration);
    }
    else   /*if c is EOF*/
    {
        if (NumIteration == 1)
            FirstCheckingCommand(command);
        else
        {
            unsigned counterEntry;
            counterEntry=0;
            SecondCheckingCommand(command);
            while ((!EC)&&(counterEntry<SymbolEntCount))
            {
                unsigned index;
                index = EntSymbolsTable[counterEntry]->index;
                symbol_table[index]->type = ENTRY;
                counterEntry++;
            }
        }
        /*clean the command array*/
        i=0;
        while (i<=word_counter)
        {
            free(command[i]);
            i++;
        }
        free(command);
        
    }
}






/*checking and processing the current command line*/
void FirstCheckingCommand(char ** command)
{
    int flag_symbol_type;
    
    /*if the given string list is null/empty */
    if (!(*command))
        return;
    
    
    /*In the case that the first string on the current command line is a label(symbol) */
    if ((isValidLabel((command[0]), 1)) && (((flag_symbol_type = isInstruction(command[1], 1)) >= 0)))
    {
        /*if the instrct type is an data or extern*/
        if (((flag_symbol_type >= DATA) && (flag_symbol_type <= MAT)) || (flag_symbol_type == EXTERN))
        {
            
            if(flag_symbol_type != EXTERN)
            {
                insertSymbolToTable(command[0], flag_symbol_type);
                insertToDT(&command[2], flag_symbol_type);
            }
            else
                insertSymbolToTable(command[2], EXTERN);
            
            
        }
        /*if the instrct type is an instruction*/
        else if (flag_symbol_type <= 15)
        {
            insertSymbolToTable(command[0], flag_symbol_type);
            insertToIT(&command[2], flag_symbol_type);   /*the command[2] is first operand*/
        }
    }
    else /*if the commands[0] isn't label*/
    {
        if ((flag_symbol_type = isInstruction(command[0], 0)) >= 0)
        {
            if ((flag_symbol_type >= DATA) && (flag_symbol_type <= MAT))
            {
                insertToDT(&command[1], flag_symbol_type);
                return;
            }
            
            if (flag_symbol_type >= 19)
            {
                if (flag_symbol_type == 20) /*if is .extern insruct type then we will enter the command into the symbol table*/
                    insertSymbolToTable(command[1], flag_symbol_type);
                
            }
            else
            {
                insertToIT(&command[1], flag_symbol_type);  /*the command[1] is first operand*/
            }
        }
        else if (!isValidLabel((command[0]), 1))
            insertNewError("Unidentified command line: %d");
    }
}





/*The Secound check of the given command line*/
void SecondCheckingCommand(char ** command)
{
    int flag_symbol_type;
    int flag;  /*if there is a label(symbol) in the current given command line*/
    
    /*if the given string list is null/empty */
    if (!(*command))
        return;
    
    flag = 0;
    if (isValidLabel(command[0], 1))
    {
        /*In the case that the first string on the current command line is a label(symbol) */
        flag = 1;
    }
    
    if ((flag_symbol_type = isInstruction(command[flag], 1)) == 19)
    {
        /*In the case that the second string is .entry*/
        unsigned index;
        
        index = findSymbol(command[flag + 1]);
        
        if (index == -1)
            insertNewError("The entry symbol defining isn't valid: %d");
        else
        {
            if(SymbolEntCount==0)
            {
                EntSymbolsTable=(EntrySy **)malloc(sizeof(EntrySy*));
                allocate_check(EntSymbolsTable);
            }
            else
            {
                EntSymbolsTable=(EntrySy **)realloc(EntSymbolsTable,sizeof(EntrySy**)*(SymbolEntCount+1));
                allocate_check(EntSymbolsTable);
            }
            EntSymbolsTable[SymbolEntCount]=(EntrySy *)malloc(sizeof(EntrySy));
            allocate_check(EntSymbolsTable[SymbolEntCount]);
            EntSymbolsTable[SymbolEntCount]->index = index;
            EntSymbolsTable[SymbolEntCount]->type = symbol_table[index]->type;
            SymbolEntCount++;
        }
    }
    else if (flag_symbol_type <= 15)
    {
        updateInstruction(&command[flag + 1], flag_symbol_type);   /*the command[2] is first operand*/
    }
}



/*private function that calculate the given integer number to the special base 4 number*/
char * base4 (int i, int j)
{
    int res,k,f,length;
    char * temp;
    char * temp1;
    char * ans ;
    
    k = 0;
    
    if (i == 0){
        ans = (char *)calloc(j+1,sizeof(char));
        allocate_check(ans);
        ans[0] = 'a';
        for(f = 1; f < j; f++)
            ans[f]='a';
        
        ans[f] = '\0';
        return ans;
    }
    
    ans = (char *)calloc(k+1,sizeof(char));
    allocate_check(ans);
    
    while (i > 0){
        res = i%4;
        i=i/4;
        
        if(res == 0){ans[k++] = 'a';}
        else if(res == 1){ans[k++] = 'b';}
        else if(res == 2){ans[k++] = 'c';}
        else if(res == 3){ans[k++] = 'd';}
        
        
        temp = (char*)realloc(ans,(k+1));
        allocate_check(ans);
        ans = temp;
    }
    
    temp = (char*)realloc(ans,(k+1));
    allocate_check(temp);
    ans = temp;
    temp = (char *)calloc(k+1,sizeof(char));
    allocate_check(temp);
    
    for(f = 0; f < k; f++)
        temp[f] = ans[(k-1)-f];
    
    temp[k]='\0';
    free(ans);
    ans = temp;
    length = strlen(ans);
    
    if (j > length)
    {
        temp1 = (char *)calloc(j+1,sizeof(char));
        for (f = 0; f < j; f++)
        {
            if (f <(j-length))
                temp1[f] = 'a';
            else
                temp1[f] = temp[f-(j-length)];
        }
        free(ans);
        return temp1;
    }
    
    return ans;
}



/*private function that print all the instructions from the instructions table into .ob file*/
void printInstructionsToFile(FILE *output)
{
    int i;
    char *temp;
    char *addr;
    
    i = 0;
    while (i < IC)
    {
        char ans[6] = "";
        addr = base4(i+100,4);
        
        if ((instructions_table[i]->type_order) == 0)
        {
            temp = base4(((InstructOrder *)instructions_table[i]->order)->opcode,2);
            strcat(ans, temp);
            free(temp);
            temp = base4(((InstructOrder *)instructions_table[i]->order)->origin_addressing,1);
            strcat(ans, temp);
            free(temp);
            temp = base4(((InstructOrder *)instructions_table[i]->order)->dest_addressing,1);
            strcat(ans, temp);
            free(temp);
            temp = base4(((InstructOrder *)instructions_table[i]->order)->type_coding,1);
            strcat(ans, temp);
            free(temp);
            
        }
        if ((instructions_table[i]->type_order) == 1)
        {
            temp = base4((((InstructData*)instructions_table[i]->order)->value)+100,4);
            strcat(ans, temp);
            free(temp);
            temp = base4(((InstructData*)instructions_table[i]->order)->type_coding,1);
            strcat(ans, temp);
            free(temp);
        }
        if ((instructions_table[i]->type_order) == 2)
        {
            temp = base4(((InstructRegisters*)instructions_table[i]->order)->reg1,2);
            strcat(ans, temp);
            free(temp);
            temp = base4(((InstructRegisters*)instructions_table[i]->order)->reg2,2);
            strcat(ans, temp);
            free(temp);
            strcat(ans, "a");
        }
        
        fprintf(output, "%s    %s\n", addr, ans);
        free(addr);
        i++;
    }
    
}




/*private function that print all the instructions data from the data table into .ob file*/
void printDataToFile(FILE *output)
{
    int i;
    char *ans;
    char *addr;
    
    i = 100+IC;
    while (i < DC+IC+100)
    {
        addr = base4(i,4);
        ans = base4(data_table[i-100-IC],5);
        
        fprintf(output, "%s    %s\n", addr, ans);
        free(addr);
        free(ans);
        i++;
    }
    
}




/*private function that free all the global variables and structs*/
void cleanAllmem()
{
    int i;
    
    i=0;
    
    if(SC>0)
    {
        while(i<SC)
        {
            free(symbol_table[i]->label_name);
            free(symbol_table[i]);
            i++;
        }
        free(symbol_table);
        i=0;
    }
    
   
    if(IC>0)
    {
        while(i<IC)
        {
            free(instructions_table[i]->order);
            free(instructions_table[i]);
            i++;
        }
        
        free(instructions_table);
        i=0;
    }
    
    if(EC>0)
    {
        while(i<EC)
        {
            free(ErrorsAssembler[i]);
            i++;
        }
        free(ErrorsAssembler);
    }
    
    if(DC>0)
        free(data_table);
    
    SymbolEntCount=0;
    SymbolExtCount=0;
    IC=0;
    SC=0;
    EC=0;
    LC=0;
    DC=0;
    Total_IC=0;
}






int main(int argc,char ** argv) {
    int argCounter;
    FILE * objectFile;
    FILE * entryFile;
    FILE * externFile;
    
    cleanAllmem();
    
    argCounter=1;
    
    if(argc<2)
        fprintf(stderr,"No entered .as file name\n");
    
    while(argCounter<argc)
    {
        char *asName;
        char *objectName;
        char *entryName ;
        char *externName;
        
        asName=(char *)calloc(strlen(argv[argCounter])+4,sizeof(char));
        allocate_check(asName);
        
        strcpy(asName,argv[argCounter]);
        
        strcat(asName,".as");
        
        fp = fopen (asName, "r");
        if(!fp)
        {
            printf("The file name \"%s\" -isn't valid or not found\n",argv[argc]);
            argCounter++;
            free(asName);
            continue;
        }
        
        free(asName);
        
        /*First checking of the assembly*/
        CommandLineToLinkedList(1);
        
        /*sets the file position to the beginning of the assembly file*/
        rewind(fp);
        
        if (EC>0)
        {
            /*Print all the compile error from ErrorsAssembler and exit*/
            int i;
            i=0;
            while(i<EC)
            {
                printf("%s\n",ErrorsAssembler[i]);
                i++;
            }
            goto Final;
        }
        
        /*Second checking of the assembly*/
        Total_IC=IC;
        IC=0;
        LC=0;
        CommandLineToLinkedList(2);
        
        if (EC>0)
        {
            /*Print all the compile error from ErrorsAssembler and exit*/
            int i;
            
            i=0;
            
            while(i<EC)
            {
                printf("%s\n",ErrorsAssembler[i]);
                i++;
                
            }
            goto Final;
        }
        
        fclose(fp);
        objectName=(char *)calloc(strlen(argv[argCounter])+4,sizeof(char));
        strcpy(objectName,argv[argCounter]);
        strcat(objectName,".ob");
        objectFile = fopen(objectName, "w");
        
        
        if(!fp)
        {
            fprintf(stderr,"Can't write the object file\n");
            exit(1);
        }
        
        
        printInstructionsToFile(objectFile);
        printDataToFile(objectFile);
        fclose(objectFile);
        free(objectName);
        
        
        
        if(SymbolEntCount>0)
        {
            unsigned i,index;
            char *addr;
            char *label;
            
            i=0;
            entryName=(char *)calloc(strlen(argv[argCounter])+5,sizeof(char));
            strcpy(entryName,argv[argCounter]);
            strcat(entryName,".ent");
            entryFile=fopen(entryName,"w");
            
            if(!entryFile)
            {
                fprintf(stderr,"Can't write the entry file\n");
                exit(1);
            }
            
            while(i<SymbolEntCount)
            {
                index=EntSymbolsTable[i]->index;
                label=symbol_table[index]->label_name;
                
                if(((EntSymbolsTable[i]->type)>=16)&&((EntSymbolsTable[i]->type)<=18))
                    addr=base4((symbol_table[index]->dec_value)+100+IC,4);
                else
                    addr=base4((symbol_table[index]->dec_value)+100,4);
                
                fprintf(entryFile,"%s    %s\n",label,addr);
                free(addr);
                free(EntSymbolsTable[i]);
                i++;
            }
            
            free(EntSymbolsTable);
            fclose(entryFile);
            free(entryName);
        }
        
        if(SymbolExtCount>0)
        {
            int i;
            char *addr;
            char *label;
            
            externName=(char *)calloc(strlen(argv[argCounter])+5,sizeof(char));
            strcpy(externName,argv[argCounter]);
            strcat(externName,".ext");
            
            i=0;
            externFile=fopen(externName,"w");
            
            if(!externFile)
            {
                fprintf(stderr,"Can't write the extern file\n");
                exit(1);
            }
            
            while(i<SymbolExtCount)
            {
                label=ExtSymbolsTable[i]->label_name;
                addr=base4((ExtSymbolsTable[i])->addr+100,4);
                fprintf(externFile,"%s    %s\n",label,addr);
                free(label);
                free(addr);
                free(ExtSymbolsTable[i]);
                i++;
            }
            
            free(ExtSymbolsTable);
            fclose(externFile);
            free(externName);
        }
        
        
    Final:cleanAllmem();
        argCounter++;
    }
    
    
    
    
    return 0;
    
}



























