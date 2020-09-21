/*
    Name 1: Allen Zhang
    Name 2: Yancheng Du
    UTEID 1: acz356
    UTEID 2: yd4339
*/

#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <stdbool.h>

#define MAX_LINE_LENGTH 255
#define TEMP_FILE_NAME "reformattedInFile"
FILE* reformattedInFile;

int numLabels;
char** labels;
int* labelAddresses;

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
        printf("\nlabel length > 20\n");
        exit(4);
    }

    if(inStr[0] == 'x'){
        printf("\nlabel begins with x\n");
        exit(4);
    }

    int invalidLabelsLength = sizeof(invalidLabels)/(sizeof(char*));
    for(int i = 0; i < invalidLabelsLength; i++){
        if(strcmp(inStr, invalidLabels[i]) == 0){
            printf("\nLabel is one of the invalid labels\n");
            exit(4);
        }
    }

    int pseudoOpListLength = sizeof(pseudoOpList)/(sizeof(char*));
    for(int i = 0; i < pseudoOpListLength; i++){
        if(strcmp(inStr, pseudoOpList[i]) == 0){
            //printf("found: %s\n", inStr);
            return false;
        }
    }

    int opcodeListLength = sizeof(opcodeList)/(sizeof(char*));
    //printf("opcodeListLength: %d\n", opcodeListLength);
    for(int i = 0; i < opcodeListLength; i++){
        if(strcmp(inStr, opcodeList[i]) == 0){
            //printf("found: %s\n", inStr);
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
        printf("\nLabel contains something other than a-z 0-9\n");
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
            printf("\nADD with invalid num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            printf("\nADD with invalid registers\n");
            exit(4);
        }else if(!isRegister(tokenList[fourthIndex]) &&
                 (toNum(tokenList[fourthIndex]) > 15 ||
                  toNum(tokenList[fourthIndex]) < -16)){
            printf("\nADD with invalid fourth argument\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "AND") == 0){
        if(tokenListCount != 4){
            printf("\nAND with invalid num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            printf("\nAND with invalid registers\n");
            exit(4);
        }else if(!isRegister(tokenList[fourthIndex]) &&
                 (toNum(tokenList[fourthIndex]) > 15 ||
                  toNum(tokenList[fourthIndex]) < -16)){
            printf("\nAND with invalid fourth argument\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BR") == 0){
        if(tokenListCount != 2){
            printf("\nBR with invalid num arguments\n");
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            printf("\nBR with invalid label\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRN") == 0){
        if(tokenListCount != 2){
            printf("\nBRN with invalid num arguments\n");
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            printf("\nBRN with invalid label\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRNZ") == 0){
         if(tokenListCount != 2){
            printf("\nBRNZ with invalid num arguments\n");
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            printf("\nBRNZ with invalid label\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRNP") == 0){
         if(tokenListCount != 2){
            printf("\nBRNP with invalid num arguments\n");
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            printf("\nBRNP with invalid label\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRNZP") == 0){
        if(tokenListCount != 2){
            printf("\nBRNZP with invalid num arguments\n");
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            printf("\nBRNZP with invalid label\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRZ") == 0){
        if(tokenListCount != 2){
            printf("\nBRZ with invalid num arguments\n");
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            printf("\nBRZ with invalid label\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRZP") == 0){
        if(tokenListCount != 2){
            printf("\nBRZP with invalid num arguments\n");
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            printf("\nBRZP with invalid label\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "BRP") == 0){
        if(tokenListCount != 2){
            printf("\nBRP with invalid num arguments\n");
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            printf("\nBRP with invalid label\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "HALT") == 0){
        if(tokenListCount != 1){
            printf("\nBRP with invalid num arguments\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "JMP") == 0){
        if(tokenListCount != 2){
            printf("\nBRP with invalid num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index])){
            printf("\nBRP with invalid registers\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "JSR") == 0){
        if(tokenListCount != 2){
            printf("\nJSR with invalid num arguments\n");
            exit(4);
        }else if(!isLabel(tokenList[reg1Index])){
            printf("\nJSR with invalid registers\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "JSRR") == 0){
        if(tokenListCount != 2){
            printf("\nJSRR with invalid num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index])){
            printf("\nJSRR with invalid registers\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "LDB") == 0){
        if(tokenListCount != 4){
            printf("\nLDB with invalid num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            printf("\nLDB with invalid registers\n");
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 31 ||
                 toNum(tokenList[fourthIndex]) < -32){
            printf("\nLDB with invalid num arg range\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "LDW") == 0){
        if(tokenListCount != 4){
            printf("\nLDW with invalid num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            printf("\nLDW with invalid registers\n");
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 31 ||
                 toNum(tokenList[fourthIndex]) < -32){
            printf("\nLDW with invalid num arg range\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "LEA") == 0){
         if(tokenListCount != 3){
            printf("\nLEA with invalid num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index])){
            printf("\nLEA with invalid registers\n");
            exit(4);
        }else if(!isLabel(tokenList[reg2Index])){
            printf("\nLEA with invalid label\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "NOP") == 0){
        if(tokenListCount != 1){
            printf("\nNOP with invalid num arguments\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "NOT") == 0){
        if(tokenListCount != 3){
            printf("\nNOT with invalid num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            printf("\nNOT with invalid registers");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "RET") == 0){
        if(tokenListCount != 1){
            printf("\nRTI with invalid num arguments\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "LSHF") == 0){
        if(tokenListCount != 4){
            printf("\nLSHF wrong num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            printf("\nLSFH with invalid registers\n");
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 15 ||
                 toNum(tokenList[fourthIndex]) < 0){
            printf("LSHF with invalid num arg range\n");
            exit(3);
        }
    }else if(strcmp(tokenList[opcodeIndex], "RSHFL") == 0){
        if(tokenListCount != 4){
            printf("\nRSHFL with wrong num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            printf("\nRSHFL with invalid registers\n");
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 15 ||
                 toNum(tokenList[fourthIndex]) < 0){
            printf("\nRSHFL with invalid num arg range\n");
            exit(3);
        }
    }else if(strcmp(tokenList[opcodeIndex], "RSHFA") == 0){
        if(tokenListCount != 4){
            printf("\nRSHFA with invalid registers\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            printf("\nRSHFA with invalid registers\n");
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 15 ||
                 toNum(tokenList[fourthIndex]) < 0){
            printf("\nRSFHA with invalid num arg range\n");
            exit(3);
        }
    }else if(strcmp(tokenList[opcodeIndex], "RTI") == 0){
        if(tokenListCount != 1){
            printf("\nRTI with invalid registers\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "STB") == 0){
        if(tokenListCount != 4){
            printf("\nSTB with wrong num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            printf("\nSTB with invalid registers\n");
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 31 ||
                 toNum(tokenList[fourthIndex]) < -32){
            printf("\nSTB with invalid num arg range\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "STW") == 0){
        if(tokenListCount != 4){
            printf("\nSTW with wrong num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            printf("\nSTW with invalid registers\n");
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 31 ||
                 toNum(tokenList[fourthIndex]) < -32){
            printf("\nSTW with invalid num arg range\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "TRAP") == 0){
        if(tokenListCount != 2){
            printf("\nTRAP with wrong num arguments\n");
            exit(4);
        }

        int trapValue = toNum(tokenList[reg1Index]);
        int trapUpperBound = toNum("x01FF");
        int trapLowerBound = 0;

        if(trapValue > trapUpperBound || trapValue < trapLowerBound){
            printf("\n invalid trap argument\n");
            exit(4);
        }
    }else if(strcmp(tokenList[opcodeIndex], "XOR") == 0){
        if(tokenListCount != 4){
            printf("\nXOR with wrong num arguments\n");
            exit(4);
        }else if(!isRegister(tokenList[reg1Index]) ||
                 !isRegister(tokenList[reg2Index])){
            printf("\nXOR with invalid register\n");
            exit(4);
        }else if(!isRegister(tokenList[fourthIndex])){
            printf("\nXOR with invalid register\n");
            exit(4);
        }else if(toNum(tokenList[fourthIndex]) > 15 ||
                 toNum(tokenList[fourthIndex]) < -16){
            printf("\nXOR invalid num arg range\n");
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
            printf("\n.ORIG with no address\n");
            exit(4);
        }
        int beginAddress = toNum(tokenList[argIndex]);
        if(beginAddress % 2 == 1){
            printf("\n.ORIG with odd address argument\n");
            exit(3);
        }
    }else if(strcmp(tokenList[pseudoOpIndex], ".FILL") == 0){
        if(tokenListCount != 2){
            printf("\n.FILL with no argument\n");
            exit(4);
        }
    }else if(strcmp(tokenList[pseudoOpIndex], ".END") == 0){
        if(tokenListCount != 1){
            printf("\n.END should be alone\n");
            exit(4);
        }
    }else{
        exit(4);
    }
}


int zerothPass(char* inFileName){

    printf("\n==zerothPass==\n");
    printf("\n");
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

        char* token = strtok(lineBufferDuplicate, " \t,");

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

        char* lastToken;
        int signalToBreak = 0;
        int numTokens = 0;
        //stop checking tokens once a comment is reached
        //count number of possible tokens in the string
        while(token && token[0] != ';' && signalToBreak == 0){
            printf("token: %s \n", token);
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
            lastToken = token;
            token = strtok(NULL, " \t,");
        }

        int lastTokenLength = strlen(lastToken);
        if(lastToken[0] < 32){
            numTokens--;
        }

        free(lineBufferDuplicate);

        char** tokenList = (char**)malloc(numTokens * sizeof(char*));
        token = strtok(lineBuffer, " \t,");
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
                //don't capitalize beginning 'x' chars because they should throw errors
                if(i == 0 && tokenList[tokenListCount][i] == 'x'){
                    continue;
                }
                tokenList[tokenListCount][i] = toupper(tokenList[tokenListCount][i]);
            }
            token = strtok(NULL, " \t,");
            tokenListCount++;
        }

        //check the first token
        //printf("numTokens: %d \n", numTokens);
        //printf("tokenListCount: %d \n", tokenListCount);
        
        bool validLabel = isLabel(tokenList[0]);

        if(validLabel){
            // next has to be opcode or pseudoOP
            if(numTokens >= 2){
                bool validOpcode = isOpcode(tokenList[1]);
                bool validPseudoOp = isPseudoOp(tokenList[1]);
                if(validOpcode){
                    processOpcode(tokenList, tokenListCount, 1);
                }else if(validPseudoOp){
                    if(strcmp(tokenList[1], ".ORIG") == 0){
                        printf("\n .ORIG missing operand\n");
                        exit(4);
                    }else if(strcmp(tokenList[1], ".END") == 0){
                        printf("\n label at .END \n");
                        exit(4);
                    }else{
                        processPseudoOp(tokenList, tokenListCount, 1);
                    }
                }else{
                    printf("\noperand after label neither pseudo op or opcode\n");
                    exit(4);
                }
            }else{
                printf("\nbare label\n");
                exit(4);
            }
        }else{
            // next has to be opcode or pseudoOP
            bool validOpcode = isOpcode(tokenList[0]);
            bool validPseudoOp = isPseudoOp(tokenList[0]);
            if(validOpcode){
                //printf("tokenListCount: %d\n", tokenListCount);
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
        /*
        printf("numTokens: %d\n", numTokens);

        int size = sizeof(tokenList)/sizeof(char*);
        printf("size: %d", size);
        for(int i = 0; i < numTokens-1; i++){
            printf("token: %s\t", tokenList[i]);
        }
        */
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

        bool endEarly = false;
        if(strcmp(tokenList[0], ".END") == 0){
            endEarly = true;
        }
        //printf("______________________________");
        for(int i = 0; i < tokenListCount; i++){
            printf("token: %s ", tokenList[i]);
            printf("token length: %d \n", (int)strlen(tokenList[i]));
            free(tokenList[i]);
        }
        free(tokenList);
        printf("\n");

        if(endEarly){
            fclose(reformattedInFile);
            return 0;
        }
    }

    fclose(reformattedInFile);
    return -1;
}


void registerToBinary(char* reg, char* isaBinaryStr, int* isaBinaryStrIndex){

    char leftBit = '0';
    char midBit = '0';
    char rightBit = '0';

    int registerNum = reg[1] - '0';
    leftBit += registerNum % 2;
    registerNum /= 2;
    midBit += registerNum % 2;
    registerNum /= 2;
    rightBit += registerNum % 2;
    registerNum /= 2;

    isaBinaryStr[*isaBinaryStrIndex] = rightBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = midBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = leftBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
}

void shfImmediate4Bit(char* imm, char* isaBinaryStr, int* isaBinaryStrIndex){
    int immVal = toNum(imm);
    if(immVal < 0){
        printf("\nshifting by a negative number\n");
        exit(4);
    }

    int moduloNum = 2;

    char thirdBit = '0';
    char secondBit = '0';
    char firstBit = '0';
    char zerothBit = '0';

    zerothBit += ((immVal % moduloNum));
    immVal = immVal >> 1;
    firstBit += ((immVal % moduloNum));
    immVal = immVal >> 1;
    secondBit += ((immVal % moduloNum));
    immVal = immVal >> 1;
    thirdBit += ((immVal % moduloNum));
    immVal = immVal >> 1;

    isaBinaryStr[*isaBinaryStrIndex] = thirdBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = secondBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = firstBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = zerothBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;

}

void immediateToBinary5Bit(char* imm, char* isaBinaryStr, int* isaBinaryStrIndex){

    int immVal = toNum(imm);
    //printf("immVal: %d \n", immVal);

    char fourthBit = '0';
    char thirdBit = '0';
    char secondBit = '0';
    char firstBit = '0';
    char zerothBit = '0';

    int negativity = 1;
    int moduloNum = 2;
    if(immVal < 0){
        negativity = -1;
    }
    
    zerothBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    firstBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    secondBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    thirdBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fourthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    
    isaBinaryStr[*isaBinaryStrIndex] = fourthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = thirdBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = secondBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = firstBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = zerothBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;

}

void immediateToBinary6Bit(char* imm, char* isaBinaryStr, int* isaBinaryStrIndex){

    int immVal = toNum(imm);
    //printf("immVal: %d \n", immVal);

    char fifthBit = '0';
    char fourthBit = '0';
    char thirdBit = '0';
    char secondBit = '0';
    char firstBit = '0';
    char zerothBit = '0';

    int negativity = 1;
    int moduloNum = 2;
    if(immVal < 0){
        negativity = -1;
    }
    
    zerothBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    firstBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    secondBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    thirdBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fourthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fifthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    
    isaBinaryStr[*isaBinaryStrIndex] = fifthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = fourthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = thirdBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = secondBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = firstBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = zerothBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
}

void immediateToBinary8Bit(char* imm, char* isaBinaryStr, int* isaBinaryStrIndex){

    int immVal = toNum(imm);
    
    int trapLowerBound = 0;
    int trapUpperBound = toNum("x01FF");

    if(immVal < trapLowerBound || immVal > trapUpperBound){
        printf("\nnot within trap vector range\n");
        exit(4);
    }

    char seventhBit = '0';
    char sixthBit = '0';
    char fifthBit = '0';
    char fourthBit = '0';
    char thirdBit = '0';
    char secondBit = '0';
    char firstBit = '0';
    char zerothBit = '0';

    int negativity = 1;
    int moduloNum = 2;
 
    zerothBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    firstBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    secondBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    thirdBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fourthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fifthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    sixthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    seventhBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    
     isaBinaryStr[*isaBinaryStrIndex] = seventhBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
     isaBinaryStr[*isaBinaryStrIndex] = sixthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
     isaBinaryStr[*isaBinaryStrIndex] = fifthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = fourthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = thirdBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = secondBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = firstBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = zerothBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
}

void immediateToBinary9Bit(int imm, char* isaBinaryStr, int* isaBinaryStrIndex){

    int immVal = imm;
    printf("immVal: %d \n", immVal);

    char eighthBit = '0';
    char seventhBit = '0';
    char sixthBit = '0';
    char fifthBit = '0';
    char fourthBit = '0';
    char thirdBit = '0';
    char secondBit = '0';
    char firstBit = '0';
    char zerothBit = '0';

    int negativity = 1;
    int moduloNum = 2;
    if(immVal < 0){
        negativity = -1;
    }
    
    zerothBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    firstBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    secondBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    thirdBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fourthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fifthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    sixthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    seventhBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    eighthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    

    isaBinaryStr[*isaBinaryStrIndex] = eighthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = seventhBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = sixthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = fifthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = fourthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = thirdBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = secondBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = firstBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = zerothBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
}

void immediateToBinary11Bit(int imm, char* isaBinaryStr, int* isaBinaryStrIndex){

    int immVal = imm;
    printf("immVal: %d \n", immVal);

    char tenthBit = '0';
    char ninethBit = '0';
    char eighthBit = '0';
    char seventhBit = '0';
    char sixthBit = '0';
    char fifthBit = '0';
    char fourthBit = '0';
    char thirdBit = '0';
    char secondBit = '0';
    char firstBit = '0';
    char zerothBit = '0';

    int negativity = 1;
    int moduloNum = 2;
    if(immVal < 0){
        negativity = -1;
    }
    
    zerothBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    firstBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    secondBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    thirdBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fourthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fifthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    sixthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    seventhBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    eighthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    ninethBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    tenthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    
    isaBinaryStr[*isaBinaryStrIndex] = tenthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = ninethBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = eighthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = seventhBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = sixthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = fifthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = fourthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = thirdBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = secondBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = firstBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = zerothBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
}

void immediateToBinary16Bit(int imm, char* isaBinaryStr, int* isaBinaryStrIndex){

    int immVal = imm;
    printf("immVal: %d \n", immVal);


    char fifteenthBit = '0';
    char fourteenthBit = '0';
    char thirteenthBit = '0';
    char twelvethBit = '0';
    char eleventhBit = '0';
    char tenthBit = '0';
    char ninethBit = '0';
    char eighthBit = '0';
    char seventhBit = '0';
    char sixthBit = '0';
    char fifthBit = '0';
    char fourthBit = '0';
    char thirdBit = '0';
    char secondBit = '0';
    char firstBit = '0';
    char zerothBit = '0';

    int negativity = 1;
    int moduloNum = 2;
    if(immVal < 0){
        negativity = -1;
    }
    
    zerothBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    firstBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    secondBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    thirdBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fourthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fifthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    sixthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    seventhBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    eighthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    ninethBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    tenthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    eleventhBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    twelvethBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    thirteenthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fourteenthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    fifteenthBit += ((immVal % moduloNum) * negativity);
    immVal = immVal >> 1;
    
    isaBinaryStr[*isaBinaryStrIndex] = fifteenthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = fourteenthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = thirteenthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = twelvethBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = eleventhBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = tenthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = ninethBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = eighthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = seventhBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = sixthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = fifthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = fourthBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = thirdBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = secondBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = firstBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
    isaBinaryStr[*isaBinaryStrIndex] = zerothBit;
    *(isaBinaryStrIndex) = *(isaBinaryStrIndex) + 1;
}

void isaToHex(char** tokenList, int tokenListCount, FILE* outputFile, int currentAddress, int offset){

    //1 more for the null terminating char
    int isaBinaryStrLength = 17;
    char* isaBinaryStr = (char*)malloc(isaBinaryStrLength * sizeof(char));
    isaBinaryStr[isaBinaryStrLength-1] = '\0';
    int isaBinaryStrIndex = 0;

    int tokenListIndex = 0 + offset;
    if(isLabel(tokenList[tokenListIndex])){
        tokenListIndex++;
    }

    if(strcmp(tokenList[tokenListIndex], "ADD") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
        tokenListIndex++;
        bool reg = isRegister(tokenList[tokenListIndex]);
        if(!reg){
            isaBinaryStr[isaBinaryStrIndex] = '1';
            isaBinaryStrIndex++;
            immediateToBinary5Bit(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        }else{
            isaBinaryStr[isaBinaryStrIndex] = '0';
            isaBinaryStrIndex++;
            isaBinaryStr[isaBinaryStrIndex] = '0';
            isaBinaryStrIndex++;
            isaBinaryStr[isaBinaryStrIndex] = '0';
            isaBinaryStrIndex++;
            registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        }
    }else if(strcmp(tokenList[tokenListIndex], "AND") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
        tokenListIndex++;
        bool reg = isRegister(tokenList[tokenListIndex]);
        if(!reg){
            isaBinaryStr[isaBinaryStrIndex] = '1';
            isaBinaryStrIndex++;
            immediateToBinary5Bit(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        }else{
            isaBinaryStr[isaBinaryStrIndex] = '0';
            isaBinaryStrIndex++;
            isaBinaryStr[isaBinaryStrIndex] = '0';
            isaBinaryStrIndex++;
            isaBinaryStr[isaBinaryStrIndex] = '0';
            isaBinaryStrIndex++;
            registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        }
    }else if(strcmp(tokenList[tokenListIndex], "BR") == 0 ||
             strcmp(tokenList[tokenListIndex], "BRNZP") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        //setting the NZP Codes
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        //printf("label: %s\n", tokenList[tokenListIndex]);

        int instructionAddress = currentAddress;
        int labelAddress = -1;
        for(int i = 0; i < numLabels; i++){
            if(strcmp(tokenList[tokenListIndex], labels[i]) == 0){
                labelAddress = labelAddresses[i];
                break;
            }
        }
        if(labelAddress == -1){
            printf("\nundefined label\n");
            exit(1);
        }

        int PCoffset9 = labelAddress - currentAddress - 2;;
        PCoffset9 = PCoffset9 >> 1;
        if(PCoffset9 <-256 || PCoffset9 > 255){
            printf("\nlabel too far\n");
            exit(4);
        }
        immediateToBinary9Bit(PCoffset9, isaBinaryStr, &isaBinaryStrIndex);
    }else if(strcmp(tokenList[tokenListIndex], "BRP") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        //setting the NZP Codes
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        //printf("label: %s\n", tokenList[tokenListIndex]);

        int instructionAddress = currentAddress;
        int labelAddress = -1;
        for(int i = 0; i < numLabels; i++){
            if(strcmp(tokenList[tokenListIndex], labels[i]) == 0){
                labelAddress = labelAddresses[i];
                break;
            }
        }
        if(labelAddress == -1){
            printf("\nundefined label\n");
            exit(1);
        }

        int PCoffset9 = labelAddress - currentAddress - 2;
        PCoffset9 = PCoffset9 >> 1;
        if(PCoffset9 <-256 || PCoffset9 > 255){
            printf("\nlabel too far\n");
            exit(4);
        }
        immediateToBinary9Bit(PCoffset9, isaBinaryStr, &isaBinaryStrIndex);

    }else if(strcmp(tokenList[tokenListIndex], "BRNZ") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        //setting the NZP Codes
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        tokenListIndex++;
        //printf("label: %s\n", tokenList[tokenListIndex]);

        int instructionAddress = currentAddress;
        int labelAddress = -1;
        for(int i = 0; i < numLabels; i++){
            if(strcmp(tokenList[tokenListIndex], labels[i]) == 0){
                labelAddress = labelAddresses[i];
                break;
            }
        }
        if(labelAddress == -1){
            printf("\nundefined label\n");
            exit(1);
        }

       int PCoffset9 = labelAddress - currentAddress - 2;
        PCoffset9 = PCoffset9 >> 1;
        if(PCoffset9 <-256 || PCoffset9 > 255){
            printf("\nlabel too far\n");
            exit(4);
        }
        immediateToBinary9Bit(PCoffset9, isaBinaryStr, &isaBinaryStrIndex);

    }else if(strcmp(tokenList[tokenListIndex], "BRZ") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        //setting the NZP Codes
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        tokenListIndex++;
        //printf("label: %s\n", tokenList[tokenListIndex]);

        int instructionAddress = currentAddress;
        int labelAddress = -1;
        for(int i = 0; i < numLabels; i++){
            if(strcmp(tokenList[tokenListIndex], labels[i]) == 0){
                labelAddress = labelAddresses[i];
                break;
            }
        }
        if(labelAddress == -1){
            printf("\nundefined label\n");
            exit(1);
        }

       int PCoffset9 = labelAddress - currentAddress - 2;
        PCoffset9 = PCoffset9 >> 1;
        if(PCoffset9 <-256 || PCoffset9 > 255){
            printf("\nlabel too far\n");
            exit(4);
        }
        immediateToBinary9Bit(PCoffset9, isaBinaryStr, &isaBinaryStrIndex);

    }else if(strcmp(tokenList[tokenListIndex], "BRNP") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        //setting the NZP Codes
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        //printf("label: %s\n", tokenList[tokenListIndex]);

        int instructionAddress = currentAddress;
        int labelAddress = -1;
        for(int i = 0; i < numLabels; i++){
            if(strcmp(tokenList[tokenListIndex], labels[i]) == 0){
                labelAddress = labelAddresses[i];
                break;
            }
        }
        if(labelAddress == -1){
            printf("\nundefined label\n");
            exit(1);
        }

       int PCoffset9 = labelAddress - currentAddress - 2;
        PCoffset9 = PCoffset9 >> 1;
        if(PCoffset9 <-256 || PCoffset9 > 255){
            printf("\nlabel too far\n");
            exit(4);
        }
        immediateToBinary9Bit(PCoffset9, isaBinaryStr, &isaBinaryStrIndex);

    }else if(strcmp(tokenList[tokenListIndex], "BRN") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        //setting the NZP Codes
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        tokenListIndex++;
        //printf("label: %s\n", tokenList[tokenListIndex]);

        int instructionAddress = currentAddress;
        int labelAddress = -1;
        for(int i = 0; i < numLabels; i++){
            if(strcmp(tokenList[tokenListIndex], labels[i]) == 0){
                labelAddress = labelAddresses[i];
                break;
            }
        }
        if(labelAddress == -1){
            printf("\nundefined label\n");
            exit(1);
        }

        int PCoffset9 = labelAddress - currentAddress - 2;
        PCoffset9 = PCoffset9 >> 1;
        if(PCoffset9 <-256|| PCoffset9 > 255){
            printf("\nlabel too far\n");
            exit(4);
        }
        immediateToBinary9Bit(PCoffset9, isaBinaryStr, &isaBinaryStrIndex);

    }else if(strcmp(tokenList[tokenListIndex], "BRZP") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        //setting the NZP Codes
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        //printf("label: %s\n", tokenList[tokenListIndex]);

        int instructionAddress = currentAddress;
        int labelAddress = -1;
        for(int i = 0; i < numLabels; i++){
            if(strcmp(tokenList[tokenListIndex], labels[i]) == 0){
                labelAddress = labelAddresses[i];
                break;
            }
        }
        if(labelAddress == -1){
            printf("\nundefined label\n");
            exit(1);
        }

        int PCoffset9 = labelAddress - currentAddress - 2;
        PCoffset9 = PCoffset9 >> 1;
        if(PCoffset9 <-256 || PCoffset9 > 255){
            printf("\nlabel too far\n");
            exit(4);
        }
        immediateToBinary9Bit(PCoffset9, isaBinaryStr, &isaBinaryStrIndex);

    }else if(strcmp(tokenList[tokenListIndex], "JMP") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

    }else if(strcmp(tokenList[tokenListIndex], "RET") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

    }else if(strcmp(tokenList[tokenListIndex], "JSR") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        //printf("label: %s\n", tokenList[tokenListIndex]);

        int instructionAddress = currentAddress;
        int labelAddress = -1;
        for(int i = 0; i < numLabels; i++){
            if(strcmp(tokenList[tokenListIndex], labels[i]) == 0){
                labelAddress = labelAddresses[i];
                break;
            }
        }
        if(labelAddress == -1){
            printf("\nundefined label\n");
            exit(1);
        }

        //account for PC - 2
        int PCoffset11 = labelAddress - currentAddress - 2;
        PCoffset11 = PCoffset11 >> 1;
        if(PCoffset11 <-1024 || PCoffset11 > 1023){
            printf("\nlabel too far\n");
            exit(4);
        }
        immediateToBinary11Bit(PCoffset11, isaBinaryStr, &isaBinaryStrIndex);

    }else if(strcmp(tokenList[tokenListIndex], "JSRR") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
    }else if(strcmp(tokenList[tokenListIndex], "LDB") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        tokenListIndex++;
        immediateToBinary6Bit(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

    }else if(strcmp(tokenList[tokenListIndex], "LDW") == 0){
        //printf("LDW\n");
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        tokenListIndex++;
        immediateToBinary6Bit(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

    }else if(strcmp(tokenList[tokenListIndex], "LEA") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        tokenListIndex++;
        //printf("label: %s\n", tokenList[tokenListIndex]);

        int instructionAddress = currentAddress;
        int labelAddress = -1;
        for(int i = 0; i < numLabels; i++){
            if(strcmp(tokenList[tokenListIndex], labels[i]) == 0){
                labelAddress = labelAddresses[i];
                break;
            }
        }
        if(labelAddress == -1){
            printf("\nundefined label\n");
            exit(1);
        }

        //account for PC - 2
        int PCoffset9 = labelAddress - currentAddress - 2;
        PCoffset9 = PCoffset9 >> 1;
        if(PCoffset9 <-256 || PCoffset9 > 255){
            printf("\nlabel too far\n");
            exit(4);
        }
        immediateToBinary9Bit(PCoffset9, isaBinaryStr, &isaBinaryStrIndex);

    }else if(strcmp(tokenList[tokenListIndex], "NOT") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        
    }else if(strcmp(tokenList[tokenListIndex], "RET") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
    
    }else if(strcmp(tokenList[tokenListIndex], "RTI") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        
    }else if(strcmp(tokenList[tokenListIndex], "LSHF") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        tokenListIndex++;
        shfImmediate4Bit(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

    }else if(strcmp(tokenList[tokenListIndex], "RSHFL") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        shfImmediate4Bit(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
    }else if(strcmp(tokenList[tokenListIndex], "RSHFA") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        shfImmediate4Bit(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
    }else if(strcmp(tokenList[tokenListIndex], "STB") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        tokenListIndex++;
        immediateToBinary6Bit(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
    }else if(strcmp(tokenList[tokenListIndex], "STW") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        tokenListIndex++;
        immediateToBinary6Bit(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
    }else if(strcmp(tokenList[tokenListIndex], "TRAP") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        tokenListIndex++;
        immediateToBinary8Bit(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
    }else if(strcmp(tokenList[tokenListIndex], "XOR") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        
        tokenListIndex++;
        registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);

        tokenListIndex++;
        if(isRegister(tokenList[tokenListIndex])){
            isaBinaryStr[isaBinaryStrIndex] = '0';
            isaBinaryStrIndex++;
            isaBinaryStr[isaBinaryStrIndex] = '0';
            isaBinaryStrIndex++;
            isaBinaryStr[isaBinaryStrIndex] = '0';
            isaBinaryStrIndex++;

            registerToBinary(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        }else{
            immediateToBinary5Bit(tokenList[tokenListIndex], isaBinaryStr, &isaBinaryStrIndex);
        }
    }else if(strcmp(tokenList[tokenListIndex], ".END") == 0){
        free(isaBinaryStr);
        return;
    }else if(strcmp(tokenList[tokenListIndex], ".FILL") == 0){
        tokenListIndex++;
        int argVal = toNum(tokenList[tokenListIndex]);
        immediateToBinary16Bit(argVal, isaBinaryStr, &isaBinaryStrIndex);
    }else if(strcmp(tokenList[tokenListIndex], ".ORIG") == 0){
        tokenListIndex++;
        int argVal = toNum(tokenList[tokenListIndex]);
        immediateToBinary16Bit(argVal, isaBinaryStr, &isaBinaryStrIndex);
    }else if(strcmp(tokenList[tokenListIndex], "HALT") == 0){
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '1';
        isaBinaryStrIndex++;

        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;
        isaBinaryStr[isaBinaryStrIndex] = '0';
        isaBinaryStrIndex++;

        immediateToBinary8Bit("x0025", isaBinaryStr, &isaBinaryStrIndex);
        
    }else if(strcmp(tokenList[tokenListIndex], "NOP") == 0){
        fputs("0x0000", outputFile);
        putc('\n', outputFile);
        free(isaBinaryStr);
        return;
    }

    
    //convert to hex and put it in the output!!
    //printf("isaBinaryStr: %s\n", isaBinaryStr);
    int value = (int)strtol(isaBinaryStr, NULL, 2);
    char hexString[8];
    sprintf(hexString, "%x", value);
    //printf("hexString: %s\n", hexString);
    int hexStringLength = strlen(hexString);
    
    char* isaHex = (char*)malloc(7 * sizeof(char));
    int isaHexIndex = 0;
    isaHex[isaHexIndex] = '0';
    isaHexIndex++;
    isaHex[isaHexIndex] = 'x';
    isaHexIndex++;

    int numZeros = 4 - hexStringLength;
    for(int i = 0; i < numZeros; i++){
        isaHex[isaHexIndex] = '0';
        isaHexIndex++;
    }
    
   
    for(int i = 0; i < hexStringLength; i++){
        if(hexString[i] >= 97 && hexString[i] <= 122){
            isaHex[isaHexIndex] = toupper(hexString[i]);
        }else{
            isaHex[isaHexIndex] = hexString[i];
        }
        isaHexIndex++;
    }
    isaHex[isaHexIndex] = '\0';

    fputs(isaHex, outputFile);
    putc('\n', outputFile);
    
    free(isaBinaryStr);
    free(isaHex);
}


void firstAndSecondPass(char* oFileName){
    //create symbol table first

    //throw error 4 if .ORIG missing
    //get starting address
    //count number labels
    //allocate memory and redo this

    printf("\n===first pass===\n");

    char lineBuffer[MAX_LINE_LENGTH];
    reformattedInFile = fopen(TEMP_FILE_NAME, "r");
    
    int startingAddress = 0;
    numLabels = 0;    
    
    while(fgets(lineBuffer, MAX_LINE_LENGTH, reformattedInFile) != NULL){
        
        // only need to check the first token for each line
        char* token = strtok(lineBuffer, " \t");
        //printf("first token: %s \n", token);
        if(strcmp(token, ".ORIG") == 0){
            char* addressToken = strtok(NULL, " \t");
            int secondTokenLength = strlen(addressToken);
            // by default all lines end with a '\n'
            addressToken[secondTokenLength-1] = '\0';
            //printf("second token length: %d \n", (int)strlen(token));
            startingAddress = toNum(addressToken);
            printf("starting address: %d \n", startingAddress);
        }

        int tokenLength = strlen(token);
        if(token[tokenLength-1] == '\n'){
            token[tokenLength-1] = '\0';
        }

        if(isLabel(token)){
            numLabels++;
        }
    }

    rewind(reformattedInFile);

    //-2 because the following logic will increment even at the .ORIG line
    int currentAddress = startingAddress;
    int labelsIndex = 0;
    labels = (char**)malloc(numLabels * sizeof(char*));
    labelAddresses = (int*)malloc(numLabels * sizeof(int));

    while(fgets(lineBuffer, MAX_LINE_LENGTH, reformattedInFile) != NULL){
        
        char* token = strtok(lineBuffer, " \t");

        int tokenLength = strlen(token);
        if(token[tokenLength-1] == '\n'){
            token[tokenLength-1] = '\0';
            tokenLength = strlen(token);
        }

        if(isLabel(token)){
            char* tokenDup = (char*)malloc((tokenLength + 1) * sizeof(char));
            for(int i = 0; i < tokenLength; i++){
                tokenDup[i] = token[i];
            }
            tokenDup[tokenLength] = '\0';
            labels[labelsIndex] = tokenDup;
            labelAddresses[labelsIndex] = currentAddress;
            labelsIndex++;
        }
        if(strcmp(token, ".ORIG") == 0){
            continue;
        }
        currentAddress += 2;
    }

    // check for repeat labels
    for(int i = 0; i < numLabels; i++){
        for(int j = 0; j < numLabels; j++){
            if(i != j){
                if(strcmp(labels[i], labels[j]) == 0){
                    printf("\nrepeat labels: %s \n", labels[i]);
                    exit(4);
                }
            }
        }
    }

    rewind(reformattedInFile);

    //done with the first pass
    printf("\n===second pass===\n");

    //open the outFile here
    FILE* outputFile = fopen(oFileName, "w");

    currentAddress = startingAddress -2;  

    while(fgets(lineBuffer, MAX_LINE_LENGTH, reformattedInFile) != NULL){

        int lineBufferLength = strlen(lineBuffer) + 1;
        char* lineBufferDuplicate = (char*)malloc(lineBufferLength * sizeof(char));
        strcpy(lineBufferDuplicate, lineBuffer);

        int numTokens = 0;
        char* token = strtok(lineBuffer, " \t");
        while(token){
            numTokens++;
            token = strtok(NULL, " \t");
        }

        int tokenListCount = 0;
        char** tokenList = (char**)malloc(numTokens * sizeof(char*));
        token = strtok(lineBufferDuplicate, " \t");
        while(token){
            tokenList[tokenListCount] = (char*)malloc((strlen(token) + 1) * sizeof(char));
            strcpy(tokenList[tokenListCount], token);
            int tokenLength = strlen(token);
            
            // terminate commas, semicolons, and new line characters
            // the last tokens in each line end with /n
            for(int i = 0; i < tokenLength; i++){
                if(tokenList[tokenListCount][i] == '\n'){
                    tokenList[tokenListCount][i] = '\0';
                    break;
                }
            }
            token = strtok(NULL, " \t");
            tokenListCount++;
        }

        if(isLabel(tokenList[0])){
            //printf("2pass label: %s", tokenList[0]);
            isaToHex(tokenList, tokenListCount, outputFile, currentAddress, 1);
        }else{
            isaToHex(tokenList, tokenListCount, outputFile, currentAddress, 0);
        }

        for(int i = 0; i < numTokens; i++){
            free(tokenList[i]);
        }
        free(tokenList);
        free(lineBufferDuplicate);
        currentAddress += 2;
    }

    for(int i = 0; i < numLabels; i++){
        //printf("label: %s \t", labels[i]);
        //printf("address: %d \n", labelAddresses[i]);
        free(labels[i]);
    }
    free(labels);
    free(labelAddresses);

    fclose(outputFile);
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
    
    int retCode = zerothPass(iFileName);
    firstAndSecondPass(oFileName);
  
    return 0;
}