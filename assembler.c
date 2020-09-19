#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <stdbool.h>

#define MAX_LINE_LENGTH 255
#define TEMP_FILE_NAME "reformattedInFile"
FILE* reformattedInFile;


char* opcodeList[] = {"ADD", 
                      "AND",
                      "BR", 
                      "BRN", 
                      "BRNZ",
                      "BRNP", 
                      "BRNZP", 
                      "BRZ", 
                      "BRZP",
                      "BRP",
                      "HALT",
                      "JMP",
                      "JSR",
                      "JSRR",
                      "LDB",
                      "LDW",
                      "LEA",
                      "NOP",
                      "NOT",
                      "RET",
                      "LSHF",
                      "RSHFL",
                      "RSHFA",
                      "RTI",
                      "STB",
                      "STW",
                      "TRAP",
                      "XOR"};

char* pseudoOpList[] = {".ORIG",
                        ".FILL",
                        ".END"};

/**
 *  return 0 if valid opCode
 *  return 4 may be label
 *  return 2 is invalid opcode
 **/
int isOpcode(char* inStr){

    int opCodeListLength = sizeof(opcodeList)/sizeof(char*);
    for(int i = 0; i < opCodeListLength; i++){
        if(strcmp(inStr, opcodeList[i]) == 0){
            return 0;
        }
    }

    int inStrLength = strlen(inStr);
    if(inStrLength == 3){
        return 2;
    }
    return 4;
}

/**
 * return 0 if valid label
 * return 4 if invalid label
 **/
int isLabel(char* inStr){

    return 0;
}

int zerothPass(char* inFileName){

    // cull empty lines and comments
    // each line must be:
    /**
     * label with opcode + args
     * pseudo op with one arg
     * opcode + args
     **/

    // check for syntax problems
    /**
     * missing .ORIG and .END
     * labels that start with x or are too long
     * labels cannot be IN, OUT, GETC, PUTS
     * labels must have alphanumeric characters a-z, 0-9
     **/
    // convert everything to uppercase
    
    char lineBuffer[MAX_LINE_LENGTH];
    char lineBufferCopy[MAX_LINE_LENGTH];
    FILE* inFile = fopen(inFileName, "r");
    reformattedInFile = fopen(TEMP_FILE_NAME, "w+");    
    
    while(fgets(lineBuffer, MAX_LINE_LENGTH, inFile) != NULL){
        
        printf("lineBuffer: %s", lineBuffer);

        int lineBufferLength = strlen(lineBuffer) + 1;
        char* lineBufferDuplicate = (char*)malloc(lineBufferLength * sizeof(char));
        strcpy(lineBufferDuplicate, lineBuffer);
    
        //break it up into labels and opcodes

        char* token = strtok(lineBufferDuplicate, " \t");

        if(!token){
            continue;
        }
        //if empty line, continue
        if(strcmp(token, "\n") == 0){
            printf("empty string \n");
            printf("\n");
            continue;
        }

        //if a comment line, continue
        if(token[0] == ';'){
            printf("comment string \n");
            printf("\n");
            continue;
        }

        int signalToBreak = 0;
        int numTokens = 0;
        //stop checking tokens once a comment is reached
        //count number of possible tokens in the string
        while(token && token[0] != ';' && signalToBreak == 0){
            numTokens++;
            //printf("token: %s \t", token);
            int tokenLength = strlen(token);
            // check for the case of: operand;comment
            for(int i = 0; i < tokenLength; i++){
                if(token[i] == ';'){
                    signalToBreak = 1;
                    break;
                }
            }
            token = strtok(NULL, " \t");
        }
        free(lineBufferDuplicate);

        char** tokenList = (char**)malloc(numTokens * sizeof(char*));
        token = strtok(lineBuffer, " \t");
        int tokenListCount = 0;

        //separate the tokens into array of tokens and capitalize them
        while(token && tokenListCount < numTokens){
            tokenList[tokenListCount] = (char*)malloc((strlen(token) + 1) * sizeof(char));
            strcpy(tokenList[tokenListCount], token);
            int tokenLength = strlen(token);
            // terminate commas and semicolons
            for(int i = 0; i < tokenLength; i++){
                if(tokenList[tokenListCount][i] == ';' ||
                   tokenList[tokenListCount][i] == ','
                   ){
                    tokenList[tokenListCount][i] = '\0';
                    break;
                }
                tokenList[tokenListCount][i] = toupper(tokenList[tokenListCount][i]);
            }
            token = strtok(NULL, " \t");
            tokenListCount++;
        }

        //check the first token
        printf("numTokens: %d \n", numTokens);
        printf("tokenListCount: %d \n", tokenListCount);
        
        int errCode = 0;
        errCode = isOpcode(tokenList[0]);

        if(errCode == 0){
            // do opCode stuff here
            if(strcmp(tokenList[0], "ADD") == 0){
                if(tokenListCount != 4){
                    errCode = 4;
                }
                //registers can only be within 0-7 
                //next two tokens must be registers


            }else if(strcmp(tokenList[0], "AND") == 0){

            }else if(strcmp(tokenList[0], "BR") == 0){

            }else if(strcmp(tokenList[0], "BRN") == 0){

            }else if(strcmp(tokenList[0], "BRNZ") == 0){

            }else if(strcmp(tokenList[0], "BRNP") == 0){

            }else if(strcmp(tokenList[0], "BRNZP") == 0){

            }else if(strcmp(tokenList[0], "BRZ") == 0){

            }else if(strcmp(tokenList[0], "BRZP") == 0){

            }else if(strcmp(tokenList[0], "BRP") == 0){

            }else if(strcmp(tokenList[0], "HALT") == 0){

            }else if(strcmp(tokenList[0], "JMP") == 0){

            }else if(strcmp(tokenList[0], "JSR") == 0){

            }else if(strcmp(tokenList[0], "JSRR") == 0){

            }else if(strcmp(tokenList[0], "LDB") == 0){

            }else if(strcmp(tokenList[0], "LDW") == 0){

            }else if(strcmp(tokenList[0], "LEA") == 0){

            }else if(strcmp(tokenList[0], "NOP") == 0){

            }else if(strcmp(tokenList[0], "NOT") == 0){

            }else if(strcmp(tokenList[0], "RET") == 0){

            }else if(strcmp(tokenList[0], "LSHF") == 0){

            }else if(strcmp(tokenList[0], "RSHFL") == 0){

            }else if(strcmp(tokenList[0], "RSHFA") == 0){

            }else if(strcmp(tokenList[0], "RTI") == 0){

            }else if(strcmp(tokenList[0], "STB") == 0){

            }else if(strcmp(tokenList[0], "STW") == 0){

            }else if(strcmp(tokenList[0], "TRAP") == 0){

            }else if(strcmp(tokenList[0], "XOR") == 0){

            }
        }else if(errCode == 2){
            printf("%s is not a valid opcode \n", tokenList[0]);
        }else{
            errCode = isLabel(tokenList[0]);
            if(errCode == 0){
                // do label stuff here
            }else{
                printf("%s is not a valid label \n", tokenList[0]);
            }   
        }
        
        for(int i = 0; i < tokenListCount; i++){
            printf("token: %s \t", tokenList[i]);
            free(tokenList[i]);
        }
        free(tokenList);
        printf("\n");

        if(errCode != 0){
            return errCode;
        }
    }

    return 0;
}



        
int main(int argc, char** argv){

    if(argc != 3){
        printf("invalid parameters");
        return -1;
    }

    char* iFileName = argv[1];
    char* oFileName = argv[2];

    printf("input file name: %s \n", iFileName);
    printf("output file name: %s \n", oFileName);

    zerothPass(iFileName);



    return 0;
}