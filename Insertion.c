
#include "Insertion.h"
#include "Checkers.h"
#include "Assembler.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>







/*private function that create/expand the Instruction table by type Of Order*/
void createNewSpaceToITtable(int typeOfOrder)
{
    
    if (IC==0)
    {
        instructions_table = (Instruction **)malloc(sizeof(Instruction *));
        allocate_check(instructions_table);
    }
    else
    {
        Instruction **temp;
        temp = (Instruction **)realloc(instructions_table, sizeof(Instruction*)*(IC + 1));
        allocate_check(temp);
        instructions_table = temp;
    }
    
    
    instructions_table[IC] = (Instruction *)malloc(sizeof(Instruction));
    allocate_check(instructions_table[IC]);
    
    switch (typeOfOrder)
    {
        case 1: /*if type Order is InstructOrder*/
        {
            instructions_table[IC]->order = (InstructOrder *)malloc(sizeof(InstructOrder));
            allocate_check(instructions_table[IC]->order);
            break;
        }
            
        case 2: /*if type Order is InstructData*/
        {
            instructions_table[IC]->order = (InstructData *)malloc(sizeof(InstructData));
            allocate_check(instructions_table[IC]->order);
            break;
        }
        case 3: /*if type Order is InstructRegister*/
        {
            instructions_table[IC]->order = (InstructRegisters *)malloc(sizeof(InstructRegisters));
            allocate_check(instructions_table[IC]->order);
            break;
        }
    }
}






/*private function that create/expand the Data table*/
void CreateNewSpaceForDT()
{
    
    if (DC==0)
    {
        data_table = (int *)malloc(sizeof(int));
        allocate_check(data_table);
    }
    else
    {
        int* temp_data;
        temp_data = (int *)realloc(data_table, sizeof(int)*(DC + 1));
        allocate_check(temp_data);
        data_table = temp_data;
    }
}









/*private function ...*/
void insertToItForOperand(char * data, int operand, int isOriginOperand)
{
    int i;
    int * value;
    char ** mat_data;
    InstructData dataOrder;
    InstructRegisters regOrder;
    
    value = NULL;
    /*If the given operand is register*/
    if (operand == 3)
    {
        regOrder.reg1 = 0;
        regOrder.reg2 = 0;
        value = isNumeric(data + 1);
        
        if (isOriginOperand)
            regOrder.reg1 = *value;
        else
            regOrder.reg2 = *value;
        
        createNewSpaceToITtable(3);
        *(((InstructRegisters *)(instructions_table[IC]->order))) = regOrder;
        instructions_table[IC]->type_order = 2;
        IC++;
        free(value);
        return;
    }
    
    /*If the given operand is label,mat or direct addressing*/
    if (operand == 0)
    {
        value = isNumeric(data + 1);
        dataOrder.value = *value;
        free(value);
    }
    
    dataOrder.type_coding = 0;
    createNewSpaceToITtable(2);
    *(((InstructData *)(instructions_table[IC]->order))) = dataOrder;
    instructions_table[IC]->type_order = 1;
    IC++;
    
    if (operand == 2)
    {
        mat_data = isValidMatrix(data);
        regOrder.reg1 = (*isNumeric((mat_data[1] + 1)));
        regOrder.reg2 = (*isNumeric((mat_data[2] + 1)));
        
        /*allocate new place registers coding*/
        createNewSpaceToITtable(3);
        *(((InstructRegisters *)(instructions_table[IC]->order))) = regOrder;
        instructions_table[IC]->type_order = 2;
        IC++;
        
        i = 0;
        while (i<3)
        {
            free(mat_data[i]);
            i++;
        }
        free(mat_data);
    }
}








void insertToItForOperandSecond(char * data, int operand)
{
    int index,i;
    char *label;
    char ** mat_fixed;
    mat_fixed=NULL;
    
    if (operand == 2)
    {
        mat_fixed=isValidMatrix(data);
        label=mat_fixed[0];
    }
    else
        label = data;
    
    index = findSymbol(label);
    
    i = 0;
    while ((operand==2)&&(i<3))
    {
        free(mat_fixed[i]);
        i++;
    }
    free(mat_fixed);
    
    
    
    if (index == -1)
        insertNewError("Symbol isn't decleared in Line: %d");
    else
    {
        if (((operand == 1) && (((symbol_table[index]->type) == DATA) || ((symbol_table[index]->type) == STRING) || ((symbol_table[index]->type) == EXTERN))) || ((operand == 2) && ((symbol_table[index]->type) == MAT)))
        {
            InstructData *temp;
            
            temp = ((InstructData *) (instructions_table[IC]->order));
            temp->value = (symbol_table[index]->dec_value) + Total_IC;
            
            if ((symbol_table[index]->type) == EXTERN)
            {
                temp->value = 0;
                temp->type_coding = 1;
                
                if (SymbolExtCount==0)
                {
                    ExtSymbolsTable = (ExternSy **) malloc(sizeof(ExternSy*));
                    allocate_check(ExtSymbolsTable);
                }
                else
                {
                    ExternSy **temp_table;
                    temp_table = (ExternSy **) realloc(ExtSymbolsTable, sizeof(ExternSy*) * (SymbolExtCount + 1));
                    allocate_check(temp_table);
                    ExtSymbolsTable = temp_table;
                }
                ExtSymbolsTable[SymbolExtCount]=(ExternSy*) malloc(sizeof(ExternSy));
                allocate_check(ExtSymbolsTable);
                ExtSymbolsTable[SymbolExtCount]->label_name=(char *)calloc(strlen(symbol_table[index]->label_name)+1,sizeof(char));
                allocate_check(ExtSymbolsTable[SymbolExtCount]->label_name);
                strcpy((ExtSymbolsTable[SymbolExtCount]->label_name),(symbol_table[index]->label_name));
                ExtSymbolsTable[SymbolExtCount]->addr = IC;
                SymbolExtCount++;
            }
            
            else
                temp->type_coding = 2;
        }
        else
            insertNewError("The symbol isn't matched to the type of the decleared symbol Line: %d");
        
        IC = IC + operand;
    }
}










/*Function that insert the symbol into the symbols tabel by the type: 16 or 17 or 18 is data;19 is .entry and 20 is .extern otherwise is instruction type. and update SC counter too*/
void insertSymbolToTable(char *data, int type)
{
    int isDotDot;
    Symbol* temp;
    
    if(!data)
    {
        insertNewError("Invalid instruction command in Line: %d");
        return;
    }
    
    isDotDot = 0;
    if (data[strlen(data) - 1] == ':')
        isDotDot = 1;
    
    temp = (Symbol*)malloc(sizeof(Symbol));
    allocate_check(temp);
    
    temp->label_name = (char *)calloc(strlen(data)-isDotDot+1,sizeof(char));
    allocate_check((temp->label_name));
    
    strncpy(temp->label_name, data,strlen(data)-isDotDot);
    /*need to add check for the copy operation*/
    
    temp->type = type;
    
    if (type == EXTERN)
        temp->dec_value = 0;
    else
    {
        if (type >= DATA)
            temp->dec_value = DC;
        else
            temp->dec_value = IC;
    }
    
    
    if (findSymbol(temp->label_name) != -1) {
        insertNewError("The symbol is already decleared in Line: %d");
        free(temp);
    }
    if (SC==0)
    {
        symbol_table = (Symbol **)malloc(sizeof(Symbol*));
        allocate_check(symbol_table);
    }
    else
    {
        symbol_table = (Symbol **)realloc(symbol_table,sizeof(Symbol*)*(SC + 1));
        allocate_check(symbol_table);
    }
    symbol_table[SC] = temp;
    SC++;
}








void insertToIT(char **data, int Instruc_type)
{
    int orgOperand, destOperand;
    InstructOrder order;
    
    destOperand = -2;
    orgOperand = checkAddressingType(data[0]);
    if (orgOperand != -2)
        destOperand = checkAddressingType(data[1]);
    
    
    if ((orgOperand == -1) || (destOperand == -1))
    {
    Failure: insertNewError("Invalid operands in Line: %d");
        return;
    }
    
    /*if the instruct type is dual places*/
    if ((Instruc_type <= SUB) || (Instruc_type == LEA))
    {
        if ((orgOperand == -2) || (destOperand == -2))
            goto Failure;
        
        if (Instruc_type == LEA)
        {
            if ((orgOperand == 0) || (orgOperand == 3))
                goto Failure;
        }
        else
        {
            if ((Instruc_type != CMP) && (destOperand == 0))
                goto Failure;
        }
    }
    
    else
    {   /*if the instruct type is sole place*/
        if (((Instruc_type >= NOT) && (Instruc_type <= CLR)) || ((Instruc_type >= INC) && (Instruc_type <= JSR)))
        {
            if ((orgOperand == -2) || (destOperand != -2))
                goto Failure;
            
            if ((Instruc_type != PRN) && (destOperand == 0))
                goto Failure;
        }
        else  /*In the condition of Instruc_type==rts or Instruc_type==stop */
        {
            if ((orgOperand != -2) || (destOperand != -2))
                goto Failure;
        }
    }
    /*if the operand/s are valid then: */
    order.type_coding = 0;
    order.origin_addressing = (orgOperand == -2) ? 0 : orgOperand;
    order.dest_addressing = (destOperand == -2) ? 0 : destOperand;
    order.opcode = Instruc_type;
    
    createNewSpaceToITtable(1);
    *(((InstructOrder *)(instructions_table[IC]->order))) = order;
    instructions_table[IC]->type_order = 0;
    IC++;
    
    /*if both of the operands are registers*/
    if ((orgOperand == 3) && (destOperand == 3))
    {
        InstructRegisters regOrder;
        
        regOrder.reg1 = (*isNumeric(data[0] + 1));
        regOrder.reg2 = (*isNumeric(data[1] + 1));
        
        createNewSpaceToITtable(3);
        *(((InstructRegisters *)(instructions_table[IC]->order))) = regOrder;
        instructions_table[IC]->type_order = 2;
        IC++;
        
    }
    else
    {
        if (orgOperand != -2)
            insertToItForOperand(data[0], orgOperand, 1);
        
        if (destOperand != -2)
            insertToItForOperand(data[1], destOperand, 0);
    }
    
    
    if(IC+DC>255)
        fprintf(stderr,"No enough Memory for the given Assembly file\n");
    
    
    
}










/*Function that insert data by the given Instruc_type argument into the data table*/
void insertToDT(char **data, int type)
{
    int* value;
    int i;
    char *reader;
    
    i = 0;
    value = NULL;
    switch (type) {
        case DATA: /*if the type is .data*/
        {
            while ((reader = data[i]))
            {
                value = isNumeric(reader);
                if (!value)
                {
                    insertNewError("The data defining isn't valid in Line: %d");
                    free(value);
                    return;
                }
                CreateNewSpaceForDT();
                data_table[DC] = *value;
                DC++;
                i++;
            }
        }
            break;
            
        case STRING: /*if the type is .string*/
        {
            char ch;
            reader = data[0];
            
            if ((!reader[0]) || (reader[0] != '"'))
            {
                insertNewError("The string defining isn't valid in Line: %d");
                return;
            }
            i = 1;
            while ((reader[i]) && (ch = reader[i]))
            {
                if (reader[i] == '"')
                {
                    if (reader[i + 1])
                        insertNewError("The string defining isn't valid in Line: %d");
                    
                    CreateNewSpaceForDT();
                    data_table[DC] = '\0';
                    DC++;
                    return;
                }
                CreateNewSpaceForDT();
                data_table[DC] = ch;
                DC++;
                i++;
            }
            insertNewError("The string defining missing \" token in Line: %d");
        }
            break;
            
        case MAT: /*if the type is .mat*/
        {
            int n;
            n = isValidMatrixToData(data[0]);
            
            if (n == -1)
                return;
            
            i = 0;
            
            while ((reader = data[i + 1]))
            {
                value = isNumeric(reader);
                if (!value)
                {
                    insertNewError("The matrix defining isn't valid in Line: %d");
                    free(value);
                    return;
                }
                CreateNewSpaceForDT();
                data_table[DC] = *value;
                free(value);
                DC++;
                i++;
            }
            if (i != n)
                insertNewError("The defining matrix values are not equel to declaring matrix: %d");
            
        }
            break;
   	}
    
    
    if(IC+DC>255)
        fprintf(stderr,"No enough Memory for the given Assembly file\n");
}








/*Update the given instruction command for the second checking*/
void updateInstruction(char **data, int Instruc_type)
{
    int orgOperand, destOperand;
    
    destOperand = -2;
    orgOperand = checkAddressingType(data[0]);
    
    if (orgOperand != -2)
        destOperand = checkAddressingType(data[1]);
    
    IC++;
    
    if ((orgOperand == 3) && (destOperand == 3))
    {
        IC++;
        return;
    }
    
    if (orgOperand != -2)
    {
        if ((orgOperand == 1) || (orgOperand == 2))
            insertToItForOperandSecond(data[0], orgOperand);
        else
            IC++;
    }
    
    if (destOperand != -2)
    {
        if ((destOperand == 1) || (destOperand == 2))
            insertToItForOperandSecond(data[1], destOperand);
        else
            IC++;
    }
}

