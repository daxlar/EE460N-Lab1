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

char* invalidLabels[] = {"IN",
                         "OUT",
                         "GETC",
                         "PUTS"};

/**
 *  return true if valid opCode
 *  return false if  may be label
 **/
bool isOpcode(char* inStr){

    int opCodeListLength = sizeof(opcodeList)/sizeof(char*);
    for(int i = 0; i < opCodeListLength; i++){
        if(strcmp(inStr, opcodeList[i]) == 0){
            return true;
        }
    }

    return false;
}

/**
 * return if valid label
 * exit 4 if invalid label
 **/
bool isLabel(char* inStr){

    /**
     * labels that start with x or are too long
     * labels cannot be IN, OUT, GETC, PUTS
     * labels must have alphanumeric characters a-z, 0-9
     * cannot be pseudo op or be longer than 20 characters
     **/

    int inStrLength = strlen(inStr);
    if(inStrLength > 20){
        exit(4);
    }

    if(inStr[0] == 'x'){
        exit(4);
    }

    int invalidLabelsLength = sizeof(invalidLabels)/(sizeof(char*));
    for(int i = 0; i < invalidLabelsLength; i++){
        if(strcmp(inStr, invalidLabels[i]) == 0){
            exit(4);
        }
    }

    int pseudoOpListLength = sizeof(pseudoOpList)/(sizeof(char*));
    for(int i = 0; i < pseudoOpListLength; i++){
        if(strcmp(inStr, pseudoOpList[i]) == 0){
            return false;
        }
    }

    int opcodeListLength = sizeof(opcodeList)/(sizeof(char*));
    for(int i = 0; i < opcodeListLength; i++){
        if(strcmp(inStr, opcodeList[i]) == 0){
            return false;
        }
    }

    for(int i = 0; i < inStrLength; i++){
        char inStrChar = inStr[i];
        if(inStrChar >= 65 && inStrChar <= 90){
            continue;
        }
        if(inStrChar >= 97 && inStrChar <= 122){
            continue;
        }
        if(inStrChar >= 48 && inStrChar <= 57){
            continue;
        }
        exit(4);
    }

    return true;
}

bool isPseudoOp(char* inStr){

    int pseudoOpListLength = sizeof(pseudoOpList)/sizeof(char*);
    for(int i = 0; i < pseudoOpListLength; i++){
        if(strcmp(inStr, pseudoOpList[i]) == 0){
            return true;
        }
    }
    return false;
}

bool isRegister(char* inStr){
    int inStrLength = strlen(inStr);
    if(inStrLength != 2){
        return false;
    }
    if(inStr[0] != 'R'){
        return false;
    }
    int registerNum = inStr[1] - '0';
    if(registerNum > 7 || registerNum < 0){
        return false;
    }
    return true;
}

int toNum(char * pStr){
  char * t_ptr;
  char * orig_pStr;
  int t_length,k;
  int lNum, lNeg = 0;
  long int lNumLong;

  orig_pStr = pStr;
  if( *pStr == '#' ){
    pStr++;
    if(*pStr == '-'){
      lNeg = 1;
      pStr++;
    }
    t_ptr = pStr;
    t_length = strlen(t_ptr);
    for(k=0;k < t_length; k++){
      if(!isdigit(*t_ptr)){
         printf("Error: invalid decimal operand, %s\n",orig_pStr);
         exit(4);
      }
      t_ptr++;
    }
    lNum = atoi(pStr);
    if (lNeg)
      lNum = -lNum;

    return lNum;
  }
  else if(*pStr == 'x' || *pStr == 'X'){
    pStr++;
    if( *pStr == '-' ){
      lNeg = 1;
      pStr++;
    }
    t_ptr = pStr;
    t_length = strlen(t_ptr);
    for(k=0;k < t_length;k++){
      if (!isxdigit(*t_ptr)){
         printf("Error: invalid hex operand, %s\n",orig_pStr);
         exit(4);
      }
      t_ptr++;
    }
    lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
    lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
    if(lNeg)
      lNum = -lNum;
    return lNum;
  }else{
        printf( "Error: invalid operand, %s\n", orig_pStr);
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
  }
}

/**
 * @param tokenList list of tokens
 * @param tokenListCount number of tokens in list
 * @param offset offset indicies into tokenList when label first then opcode vs solo opcode 
 * 
 **/

void processOpcode(char** tokenList, int tokenListCount, int offset){
    // set indices for when labels exist and when they don't
    int opcodeIndex = 0 + offset;
    int reg1Index = 1 + offset;
    int reg2Index = 2 + offset;
    int fourthIndex = 3 + offset;
    tokenListCount -= offset;

    if(strcmp(tokenList[opcodeIndex], "ADD") == 0){
        // has to be opcode, reg, reg, reg/valid num
        if(tokenListCount != 4){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            exit(4);
        }else if(!isRegister(tokenList[fourthIndex]) &&
                 (toNum(tokenList[fourthIndex]) > 15 ||
                  toNum(tokenList[fourthIndex]) < -16)){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "AND") == 0){
        if(tokenListCount != 4){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            exit(4);
        }else if(!isRegister(tokenList[fourthIndex]) &&
                 (toNum(tokenList[fourthIndex]) > 15 ||
                  toNum(tokenList[fourthIndex]) < -16)){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BR") == 0){
        if(tokenListCount != 2){
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRN") == 0){
        if(tokenListCount != 2){
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRNZ") == 0){
         if(tokenListCount != 2){
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRNP") == 0){
         if(tokenListCount != 2){
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRNZP") == 0){
        if(tokenListCount != 2){
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRZ") == 0){
        if(tokenListCount != 2){
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRZP") == 0){
        if(tokenListCount != 2){
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRP") == 0){
        if(tokenListCount != 2){
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "HALT") == 0){
        if(tokenListCount != 1){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "JMP") == 0){
        if(tokenListCount != 2){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "JSR") == 0){
        if(tokenListCount != 2){
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "JSRR") == 0){
        if(tokenListCount != 2){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "LDB") == 0){
        if(tokenListCount != 4){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 31 ||
                 toNum(tokenList[fourthIndex]) < -32){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "LDW") == 0){
        if(tokenListCount != 4){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 31 ||
                 toNum(tokenList[fourthIndex]) < -32){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "LEA") == 0){
         if(tokenListCount != 3){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index])){
            exit(4);
        }else if(!isLabel(tokenList[reg2Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "NOP") == 0){
        if(tokenListCount != 1){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "NOT") == 0){
        if(tokenListCount != 3){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "RET") == 0){
        if(tokenListCount != 1){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "LSHF") == 0){
        if(tokenListCount != 4){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 7){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "RSHFL") == 0){
        if(tokenListCount != 4){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 7){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "RSHFA") == 0){
        if(tokenListCount != 4){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 7){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "RTI") == 0){
        if(tokenListCount != 1){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "STB") == 0){
        if(tokenListCount != 4){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 31 ||
                 toNum(tokenList[fourthIndex]) < -32){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "STW") == 0){
        if(tokenListCount != 4){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 31 ||
                 toNum(tokenList[fourthIndex]) < -32){
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "TRAP") == 0){

    }else if(strcmp(tokenList[opcodeIndex], "XOR") == 0){
        if(tokenListCount != 4){
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            exit(4);
        }else if(!isRegister(tokenList[fourthIndex])){
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 15 ||
                 toNum(tokenList[fourthIndex]) < -16){
            exit(4);
        }
    }else{
        exit(4);
    }
}

void processPseudoOp(char** tokenList, int tokenListCount, int offset){
    int pseudoOpIndex = 0 + offset;
    int argIndex = 1 + offset;
    tokenListCount -= offset;

    if(strcmp(tokenList[pseudoOpIndex], ".ORIG") == 0){
        if(tokenListCount != 2){
            exit(4);
        }
        int beginAddress = toNum(tokenList[argIndex]);
        if(beginAddress % 2 == 1){
            exit(3);
        }
    }else if(strcmp(tokenList[pseudoOpIndex], ".FILL") == 0){
        if(tokenListCount != 2){
            exit(4);
        }
    }else if(strcmp(tokenList[pseudoOpIndex], ".END") == 0){
        if(tokenListCount != 1){
            exit(4);
        }
    }else{
        exit(4);
    }
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
    reformattedInFile = fopen(TEMP_FILE_NAME, "w");    
    
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
            //printf("empty string \n");
            printf("\n");
            continue;
        }

        //if a comment line, continue
        if(token[0] == ';'){
            //printf("comment string \n");
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
            //printf("token: %d \n", (int)strlen(token));
            //printf("token cpy: %d \n", (int)strlen(tokenList[tokenListCount]));
            //printf("token: %s \n", token);

            // terminate commas, semicolons, and new line characters
            // the last tokens in each line end with /n
            for(int i = 0; i < tokenLength; i++){
                if(tokenList[tokenListCount][i] == ';' ||
                   tokenList[tokenListCount][i] == ',' ||
                   tokenList[tokenListCount][i] == '\n'
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
        //printf("numTokens: %d \n", numTokens);
        //printf("tokenListCount: %d \n", tokenListCount);
        
        bool validLabel = isLabel(tokenList[0]);

        if(validLabel){
            // next has to be opcode or pseudoOP
            if(numTokens > 2){
                bool validOpcode = isOpcode(tokenList[1]);
                bool validPseudoOp = isPseudoOp(tokenList[1]);
                if(validOpcode){
                    processOpcode(tokenList, tokenListCount, 1);
                }else if(validPseudoOp){
                    if(strcmp(tokenList[1], ".ORIG") == 0){
                        exit(4);
                    }else if(strcmp(tokenList[1], ".END") == 0){
                        exit(4);
                    }else{
                        processPseudoOp(tokenList, tokenListCount, 1);
                    }
                }else{
                    exit(4);
                }
            }else{
                exit(4);
            }
        }else{
            // next has to be opcode or pseudoOP
            bool validOpcode = isOpcode(tokenList[0]);
            bool validPseudoOp = isPseudoOp(tokenList[0]);
            if(validOpcode){
                processOpcode(tokenList, tokenListCount, 0);
            }else if(validPseudoOp){
                processPseudoOp(tokenList, tokenListCount, 0);
            }else{
                exit(4);
            }
        }

        printf("valid line of assembly \n");


        // if everything is good and has not exited, write line to temp file for first pass processing
        // use spaces to delimit the corrected strings to be placed into buffer
        int correctedStringLength = numTokens;
        for(int i = 0; i < numTokens; i++){
            correctedStringLength += strlen(tokenList[i]);
        }

        char* correctedString = (char*)malloc(correctedStringLength * sizeof(char));
        int correctedStringIndex = 0;
        for(int i = 0; i < numTokens; i++){
            //printf("%s ", tokenList[i]);
            for(int j = 0; j < strlen(tokenList[i]); j++){
                correctedString[correctedStringIndex] = tokenList[i][j];
                correctedStringIndex++;
            }
            correctedString[correctedStringIndex] = ' ';
            correctedStringIndex++;
        }

        correctedString[correctedStringLength-1] = '\0';

        fputs(correctedString, reformattedInFile);
        putc('\n', reformattedInFile);
        free(correctedString);

        //printf("______________________________");
        for(int i = 0; i < tokenListCount; i++){
            //printf("token: %s ", tokenList[i]);
            //printf("token length: %d \n", (int)strlen(tokenList[i]));
            free(tokenList[i]);
        }
        free(tokenList);
        printf("\n");
        
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
    //firstPass();
    //secondPass();

    return 0;
}