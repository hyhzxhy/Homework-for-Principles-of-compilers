#include <iostream>
#include<regex>
#include<stdio.h>
#include<cstring>
#include<cstdlib>
#include <cmath>

#define BUFFLEN 4000
#define ADDRESSlen 100
#define FIRST_HALF 0
#define LAST_HALF 1
#define TOKEN_NAME_LEN 50
#define TOKEN_LEN 15
#define TOKEN_MAX 10000
#define STR_LEN 100
#define RESERVE_LEN 35
#define VAR_NUM 4096
#define ERROR_NUM 256
#define ERROR_REA_NUM 15
#define SYMBOL_lEN 6

char buff [BUFFLEN+2];
char inputAddress[100] = {0};
char stringBuff[STR_LEN];
const char *keyword[RESERVE_LEN] = { "if", "else", "while", "print", "for", "then", "do", "void",
                   "char", "double", "enum", "float", "int", "long", "short",
                   "signed", "struct", "union", "unsigned", "auto", "return", "assert", "NULL", "break",
                   "case", "continue", "default", "extern", "goto", "register", "sizeof", "static", "switch", "typedf"
                   , "volatile"};
short variableID[VAR_NUM];
int varID;
FILE *pf = NULL;
int varNums;
int lineNums;
int errorNums;
int characterPerline;
struct token
{
    char name [TOKEN_NAME_LEN];
    char symbol [TOKEN_LEN];
    char property [TOKEN_LEN];
};
struct token tokenArray[TOKEN_MAX];
struct err
{
    int position[2];
    char reason[ERROR_REA_NUM];
};
struct err errorRecord[ERROR_NUM];

//document and pointer deal function
void read_to_buff(unsigned int location, long* filelength)
{
    if(*filelength==0){
        memset(buff, 0, BUFFLEN+2);
    }
    if(location == FIRST_HALF){
        if(*filelength < BUFFLEN/2) {
            fread(buff, 1, *filelength, pf);
            //fseek(pf, *filelength, SEEK_CUR);
            buff[*filelength] = 0;
            *filelength -= *filelength;
        }else{
            fread(buff, 1, BUFFLEN/2-1, pf);
            *filelength -= BUFFLEN/2-1;
            //fseek(pf, int(BUFFLEN/2-1), SEEK_CUR);
        }
    }else if(location == LAST_HALF){
        if(*filelength < BUFFLEN/2){
            fread(buff+int(0.5*BUFFLEN)+1, 1, *filelength, pf);
            buff[int(0.5*BUFFLEN)+*filelength] = 0;
            //fseek(pf, *filelength, SEEK_CUR);
        }else{
            fread(buff+int(0.5*BUFFLEN)+1, 1, BUFFLEN/2-1, pf);
            *filelength -= BUFFLEN/2-1;
            //fseek(pf, int(BUFFLEN/2-1), SEEK_CUR);
        }
    }
}
int forward_pointer(int pointer, long* file_length)
{
    pointer ++;
    characterPerline ++;
    if(buff[pointer] == 0){
        if(pointer == BUFFLEN/2 -1) {
            read_to_buff(LAST_HALF, file_length);
            pointer += 2;
        }else if(pointer == BUFFLEN){
            read_to_buff(FIRST_HALF, file_length);
            pointer = 0;
        }else{
            return pointer;
        }
    }else if(pointer == BUFFLEN){
        pointer = 0;
    }
    return pointer;

}
int retract_pointer(int pointer)
{
    characterPerline --;
    if(pointer==0){
        pointer = BUFFLEN;
        buff[BUFFLEN] = buff[BUFFLEN-1];
    }else if(pointer == BUFFLEN/2+1){
        pointer--;
        buff[pointer] = buff[pointer-2];
    }else{
        pointer--;
    }
    return pointer;
}
int error(int pointer, long *filelength, const char* rea)
{
    errorRecord[errorNums].position[0] = lineNums;
    errorRecord[errorNums].position[1] = characterPerline;
    strcpy(errorRecord[errorNums].reason, rea);
    errorNums++;
    while(buff[pointer] != ' ' && buff[pointer] != '\n' && buff[pointer] != 0 && buff[pointer] == '\t'){
        pointer = forward_pointer(pointer, filelength);
    }
    pointer = retract_pointer(pointer);
    return pointer;
}

bool is_digit(int pos)
{
    if(buff[pos] >= '0' and buff[pos] <= '9')
        return true;
    else
        return  false;
}
bool is_letter(int pos)
{
    if((buff[pos] >= 'a' and buff[pos] <='z') or (buff[pos] >= 'A' and buff[pos] <= 'Z'))
        return true;
    else
        return false;
}
bool is_reserved(char *stringBuff)
{
    bool temp = 1;
    for(int i=0; i<RESERVE_LEN; i++){
        temp = temp && strcmp(stringBuff, keyword[i]);
    }
    return !temp;
}
int fm_deal_with_num(int pointer, long *filelength, int &token_num)
{
    int state = 1;
    int temp_count = 0;
    bool is_float = false;
    // first identify HEX
    if(buff[pointer] == '0') {
        pointer = forward_pointer(pointer, filelength);
        if(buff[pointer] == 'x' or buff[pointer] == 'X'){
            stringBuff[0] = '0';
            stringBuff[1] = buff[pointer];
            temp_count += 2;
            pointer = forward_pointer(pointer, filelength);
            while(is_digit(pointer) || (buff[pointer] >= 'a' && buff[pointer] <= 'e') || (buff[pointer] >= 'A' && buff[pointer] <= 'E')){
                stringBuff[temp_count] = buff[pointer];
                temp_count += 1;
                pointer = forward_pointer(pointer, filelength);
            }
            pointer = retract_pointer(pointer);
            stringBuff[temp_count] = 0;
            strcpy(tokenArray[token_num].name, stringBuff);
            strcpy(tokenArray[token_num].symbol, "constant");
            strcpy(tokenArray[token_num].property, "int");
            token_num++;
            return pointer;
        }else{
            pointer = retract_pointer(pointer);
        }
    }
    while(true) {
        switch (state) {
            case 1:
                if(is_digit(pointer)){
                    state = 1;
                }else if(buff[pointer] == 'E' or buff[pointer] == 'e'){
                    is_float = true;
                    state = 4;
                }else if(buff[pointer] == '.'){
                    is_float = true;
                    state = 2;
                }else if(is_letter(pointer)){
                    pointer = error(pointer, filelength, "NumConvWro");
                    return pointer;
                }else{
                    state = 7;
                    pointer = retract_pointer(pointer);
                    stringBuff[temp_count] = 0;
                    strcpy(tokenArray[token_num].name, stringBuff);
                    strcpy(tokenArray[token_num].symbol, "constant");
                    if(is_float)
                        strcpy(tokenArray[token_num].property, "float");
                    else
                        strcpy(tokenArray[token_num].property, "int");
                    token_num++;
                    return pointer;
                }
                break;
            case 2:
                if(is_digit(pointer)){
                    state = 3;
                }else{
                    pointer = error(pointer, filelength, "NumConvWro");
                    return pointer;
                }
                if(temp_count >= STR_LEN){
                    pointer = error(pointer, filelength, "Overflow");
                    return pointer;
                }
                break;
            case 3:
                if(is_digit(pointer)){
                    state = 3;
                }else if(buff[pointer] == 'E' or buff[pointer] == 'e'){
                    state = 4;
                }else if(is_letter(pointer)){
                    pointer = error(pointer, filelength, "NumConvWro");
                    return pointer;
                }else{
                    state = 7;
                    stringBuff[temp_count] = 0;
                    pointer = retract_pointer(pointer);
                    strcpy(tokenArray[token_num].name, stringBuff);
                    strcpy(tokenArray[token_num].symbol, "constant");
                    if(is_float)
                        strcpy(tokenArray[token_num].property, "float");
                    else
                        strcpy(tokenArray[token_num].property, "int");
                    token_num ++;
                    return pointer;
                }
                if(temp_count >= STR_LEN){
                    pointer = error(pointer, filelength, "Overflow");
                    return pointer;
                }
                break;
            case 4:
                if(is_digit(pointer)){
                    state = 6;
                }else if (buff[pointer] == '+' or buff[pointer] == '-'){
                    state = 5;
                }else{
                    pointer = error(pointer, filelength, "NumConvWro");
                    return pointer;
                }
                if(temp_count >= STR_LEN){
                    pointer = error(pointer, filelength, "Overflow");
                    return pointer;
                }
                break;
            case 5:
                if(is_digit(pointer)){
                    state = 6;
                }else{
                    pointer = error(pointer, filelength, "NumConvWro");
                    return pointer;
                }
                if(temp_count >= STR_LEN){
                    pointer = error(pointer, filelength, "Overflow");
                    return pointer;
                }
                break;
            case 6:
                if(is_digit(pointer)){
                    state = 6;
                }else if(is_letter(pointer)){
                    pointer = error(pointer, filelength, "NumConvWro");
                    return pointer;
                }else{
                    state = 7;
                    stringBuff[temp_count] = 0;
                    pointer = retract_pointer(pointer);
                    strcpy(tokenArray[token_num].name, stringBuff);
                    strcpy(tokenArray[token_num].symbol, "constant");
                    if(is_float)
                        strcpy(tokenArray[token_num].property, "float");
                    else
                        strcpy(tokenArray[token_num].property, "int");
                    token_num ++;
                    return pointer;
                }
                break;
            default:
                pointer = error(pointer, filelength, "NumConvWro");
                return pointer;
        }
        if(temp_count >= STR_LEN){
            pointer = error(pointer, filelength, "Overflow");
            return pointer;
        }
        stringBuff[temp_count] = buff[pointer];
        temp_count++;
        pointer = forward_pointer(pointer, filelength);
    }
}
int deal_with_num(int pointer, long *filelength, int &token_num)
{
    char *end;
    int temp_count = 0;
    bool is_float = false;
    while(is_digit(pointer) || buff[pointer] == '.' || buff[pointer] == 'E' || buff[pointer] == 'e'
    || buff[pointer] == '+' || buff[pointer] == '-' || is_letter((pointer))){
        if(buff[pointer] == '.'){
            is_float = true;
        }
        stringBuff[temp_count] = buff[pointer];
        pointer = forward_pointer(pointer, filelength);
        temp_count += 1;
    }
    pointer = retract_pointer(pointer);
    stringBuff[temp_count] = 0;
    if(!is_float){
        long lnum;
        lnum = strtol(stringBuff, &end, 10);
        if (end != temp_count + stringBuff){
            pointer = error(pointer, filelength, "NumConvWro");
        }else if((lnum == LONG_MAX || lnum == LONG_MIN) && errno == ERANGE){
            pointer = error(pointer, filelength, "Overflow");
        }else{
            strcpy(tokenArray[token_num].name, stringBuff);
            strcpy(tokenArray[token_num].symbol, "constant");
            strcpy(tokenArray[token_num].property, "int");
            token_num ++;
        }
    }else{
        double result;
        result = strtod(stringBuff, &end);
        if(end != temp_count +stringBuff)
            pointer = error(pointer, filelength, "NumConvWro");
        else if((result == HUGE_VAL || result == - HUGE_VAL) && errno ==ERANGE)
            pointer = error(pointer, filelength, "Overflow");
        else{
            strcpy(tokenArray[token_num].name, stringBuff);
            strcpy(tokenArray[token_num].symbol, "constant");
            strcpy(tokenArray[token_num].property, "float");
            token_num ++;
        }
    }
    return pointer;
}
int deal_with_letter(int pointer, long *filelength, int &token_num)
{
    int temp_count = 0;
    while(is_letter(pointer) || is_digit(pointer) || buff[pointer] == '_'){
        stringBuff[temp_count] = buff[pointer];
        pointer = forward_pointer(pointer, filelength);
        temp_count ++;
    }
    stringBuff[temp_count] = 0;
    pointer = retract_pointer(pointer);
    if(is_reserved(stringBuff)){
        strcpy(tokenArray[token_num].name, stringBuff);
        strcpy(tokenArray[token_num].symbol, "keyword");
        strcpy(tokenArray[token_num].property, "-");
        token_num ++;
    }
    else{
        int already_have = -1;
        for(int i=0; i<varNums ;i++){
            if(strcmp(tokenArray[variableID[i]].name, stringBuff) == 0){
                already_have = variableID[i];
            }
        }
        strcpy(tokenArray[token_num].name, stringBuff);
        strcpy(tokenArray[token_num].symbol, "ID");
        if (already_have != -1){
            int temp_id = strtol(tokenArray[already_have].property, NULL, 10);
            sprintf(tokenArray[token_num].property, "%d--%d", temp_id, already_have);
        }else {
            sprintf(tokenArray[token_num].property, "%d--%d", varNums, token_num);
            variableID[varNums] = token_num;
            varNums ++;
        }
        token_num ++;
    }
    return pointer;
}
int deal_with_word(long *filelength)
{
    int token_num = 0;
    int pointer = 0;
    while(buff[pointer] != 0){
        // read in one word
        while(buff[pointer] == ' ' or buff[pointer] == '\n' or buff[pointer] == '\t'){
            if (buff[pointer] == '\n') {
                lineNums++;
                characterPerline = 0;
            }
            pointer = forward_pointer(pointer, filelength);
        }
        if(is_digit(pointer)){
            pointer = fm_deal_with_num(pointer, filelength, token_num);
        }else if(is_letter(pointer)){
            pointer = deal_with_letter(pointer, filelength, token_num);
        }
        else{
            int temp_num = 0;
            switch(buff[pointer]) {
                case '(':
                    strcpy(tokenArray[token_num].name, "(");
                    strcpy(tokenArray[token_num].symbol, "symbol");
                    strcpy(tokenArray[token_num].property, "LeftParen");
                    token_num++ ;
                    break;
                case ')':
                    strcpy(tokenArray[token_num].name, ")");
                    strcpy(tokenArray[token_num].symbol, "symbol");
                    strcpy(tokenArray[token_num].property, "RightParen");
                    token_num++ ;
                    break;
                case '{':
                    strcpy(tokenArray[token_num].name, "{");
                    strcpy(tokenArray[token_num].symbol, "symbol");
                    strcpy(tokenArray[token_num].property, "LeftBrace");
                    token_num++ ;
                    break;
                case '}':
                    strcpy(tokenArray[token_num].name, "}");
                    strcpy(tokenArray[token_num].symbol, "symbol");
                    strcpy(tokenArray[token_num].property, "RightBrace");
                    token_num++ ;
                    break;
                case ';':
                    strcpy(tokenArray[token_num].name, ";");
                    strcpy(tokenArray[token_num].symbol, "symbol");
                    strcpy(tokenArray[token_num].property, "Semicolon");
                    token_num++ ;
                    break;
                case ',':
                    strcpy(tokenArray[token_num].name, ",");
                    strcpy(tokenArray[token_num].symbol, "symbol");
                    strcpy(tokenArray[token_num].property, "Comma");
                    token_num++ ;
                    break;
                case '*':
                    strcpy(tokenArray[token_num].name, "*");
                    strcpy(tokenArray[token_num].symbol, "op");
                    strcpy(tokenArray[token_num].property, "multiply");
                    token_num++ ;
                    break;
                case '/':
                    pointer = forward_pointer(pointer, filelength);
                    if(buff[pointer] == '*'){
                        while(true) {
                            while (buff[pointer] != '*') {
                                if(buff[pointer] == '\n'){
                                    lineNums++;
                                    characterPerline = 0;
                                }
                                if(buff[pointer] == 0 && *filelength == 0){
                                    error(pointer, filelength, "pairing error");
                                    return token_num;
                                }
                                pointer = forward_pointer(pointer, filelength);
                            }
                            pointer = forward_pointer(pointer, filelength);
                            if(buff[pointer] == '/')
                                break;
                            if(buff[pointer] == 0 && *filelength == 0){
                                error(pointer, filelength, "pairing error");
                                return token_num;
                            }
                        }
                    }else if(buff[pointer] == '='){
                        strcpy(tokenArray[token_num].name, "/=");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "divAssign");
                        token_num++;
                    }else{
                        pointer = retract_pointer(pointer);
                        strcpy(tokenArray[token_num].name, "/");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "divide");
                        token_num++;
                    }
                    break;
                case '%':
                    pointer = forward_pointer(pointer, filelength);
                    if (buff[pointer] == '='){
                        strcpy(tokenArray[token_num].name, "%=");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "modAssign");
                    }else {
                        pointer = retract_pointer(pointer);
                        strcpy(tokenArray[token_num].name, "%");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "mod");
                    }
                    token_num++;
                    break;
                case '+':
                    pointer = forward_pointer(pointer, filelength);
                    if (buff[pointer] == '+'){
                        strcpy(tokenArray[token_num].name, "++");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "selfAdd");
                    }else if(buff[pointer] == '='){
                        strcpy(tokenArray[token_num].name, "+=");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "selfAssign");
                    } else{
                        pointer = retract_pointer(pointer);
                        strcpy(tokenArray[token_num].name, "+");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "add");
                    }
                    token_num++;
                    break;
                case '-':
                    pointer = forward_pointer(pointer, filelength);
                    if (buff[pointer] == '-'){
                        strcpy(tokenArray[token_num].name, "--");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "selfSubtract");
                    }else if(buff[pointer] == '='){
                        strcpy(tokenArray[token_num].name, "-=");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "subAssign");
                    }else{
                        pointer = retract_pointer(pointer);
                        strcpy(tokenArray[token_num].name, "-");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "subtract");
                    }
                    token_num++;
                    break;
                case '<':
                    pointer = forward_pointer(pointer, filelength);
                    if (buff[pointer] == '=') {
                        strcpy(tokenArray[token_num].name, "<=");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "lessequal");
                    } else if(buff[pointer] == '<') {
                        pointer = forward_pointer(pointer, filelength);
                        if(buff[pointer] == '='){
                            strcpy(tokenArray[token_num].name, "<<=");
                            strcpy(tokenArray[token_num].symbol, "op");
                            strcpy(tokenArray[token_num].property, "shiftLAssign");
                        }else {
                            pointer = retract_pointer(pointer);
                            strcpy(tokenArray[token_num].name, "<<");
                            strcpy(tokenArray[token_num].symbol, "op");
                            strcpy(tokenArray[token_num].property, "shiftLeft");
                        }
                    }else {
                        pointer = retract_pointer(pointer);
                        strcpy(tokenArray[token_num].name, "<");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "less");
                    }
                    token_num++;
                    break;
                case '>':
                    pointer = forward_pointer(pointer, filelength);
                    if (buff[pointer] == '=') {
                        strcpy(tokenArray[token_num].name, ">=");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "greaterequal");
                    } else if(buff[pointer] == '>'){
                        pointer = forward_pointer(pointer, filelength);
                        if(buff[pointer] == '='){
                            strcpy(tokenArray[token_num].name, ">>=");
                            strcpy(tokenArray[token_num].symbol, "op");
                            strcpy(tokenArray[token_num].property, "shiftRAssign");
                        }else {
                            pointer = retract_pointer(pointer);
                            strcpy(tokenArray[token_num].name, ">>");
                            strcpy(tokenArray[token_num].symbol, "op");
                            strcpy(tokenArray[token_num].property, "shiftRight");
                        }
                    }else {
                        pointer = retract_pointer(pointer);
                        strcpy(tokenArray[token_num].name, ">");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "greater");
                    }
                    token_num++;
                    break;
                case '=':
                    pointer = forward_pointer(pointer, filelength);
                    if (buff[pointer] == '=') {
                        strcpy(tokenArray[token_num].name, "==");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "equal");
                        token_num++;
                    } else {
                        pointer = retract_pointer(pointer);
                        strcpy(tokenArray[token_num].name, "=");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "assign");
                        token_num++;
                    }
                    break;
                case '!':
                    pointer = forward_pointer(pointer, filelength);
                    if (buff[pointer] == '=') {
                        strcpy(tokenArray[token_num].name, "!=");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "notequal");
                        token_num++;
                    } else {
                        pointer = retract_pointer(pointer);
                        strcpy(tokenArray[token_num].name, "!");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "not");
                        token_num++;
                    }
                    break;
                case '&':
                    pointer = forward_pointer(pointer, filelength);
                    if (buff[pointer] == '&') {
                        strcpy(tokenArray[token_num].name, "&&");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "and");
                    } else if(buff[pointer] == '='){
                        strcpy(tokenArray[token_num].name, "&=");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "bitAndAssign");
                    }else {
                        pointer = retract_pointer(pointer);
                        strcpy(tokenArray[token_num].name, "&");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "bitAnd");
                    }
                    token_num++;
                    break;
                case '|':
                    pointer = forward_pointer(pointer, filelength);
                    if (buff[pointer] == '|') {
                        strcpy(tokenArray[token_num].name, "||");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "or");
                    } else if(buff[pointer] == '='){
                        strcpy(tokenArray[token_num].name, "|=");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "bitOrAssign");
                    }else {
                        pointer = retract_pointer(pointer);
                        strcpy(tokenArray[token_num].name, "|");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "bitOr");
                    }
                    token_num++;
                    break;
                case '"':
                    stringBuff[temp_num] = buff[pointer];
                    pointer = forward_pointer(pointer, filelength);
                    temp_num ++;
                    while(true){
                        if(temp_num >= TOKEN_NAME_LEN or (buff[pointer] == '"' and stringBuff[temp_num-1] != '\\'))
                            break;
                        if(temp_num >= 2 and (buff[pointer] == '"' and
                        stringBuff[temp_num-1] == '\\' and buff[pointer] == '\\'))
                            break;
                        stringBuff[temp_num] = buff[pointer];
                        pointer = forward_pointer(pointer, filelength);
                        temp_num ++;
                        if(*filelength == 0 and buff[pointer] == 0){
                            error(pointer,filelength, "pairing error");
                            return token_num;
                        }
                    }
                    stringBuff[temp_num] = buff[pointer];
                    stringBuff[++temp_num] = 0;
                    if(temp_num >= TOKEN_NAME_LEN){
                        pointer = error(pointer, filelength, "StrTooLarge");
                    }else{
                        strcpy(tokenArray[token_num].name, stringBuff);
                        strcpy(tokenArray[token_num].symbol, "constant");
                        strcpy(tokenArray[token_num].property, "string");
                        token_num ++;
                    }
                    break;
                case '\'':
                    stringBuff[temp_num] = buff[pointer];
                    pointer = forward_pointer(pointer, filelength);
                    temp_num ++;
                    while(true){
                        if(temp_num >= 4)
                            break;
                        if(buff[pointer] == '\'' and stringBuff[temp_num-1] != '\\')
                            break;
                        if(temp_num > 2 and (buff[pointer] == '\'' and stringBuff[temp_num-1] == '\\'
                        and stringBuff[temp_num-2] == '\\'))
                            break;
                        stringBuff[temp_num] = buff[pointer];
                        pointer = forward_pointer(pointer, filelength);
                        temp_num ++;
                    }
                    stringBuff[temp_num] = buff[pointer];
                    stringBuff[++temp_num] = 0;
                    if(temp_num >= 5){
                        pointer = error(pointer, filelength, "InvalidCha");
                    }else{
                        strcpy(tokenArray[token_num].name, stringBuff);
                        strcpy(tokenArray[token_num].symbol, "constant");
                        strcpy(tokenArray[token_num].property, "literal");
                        token_num ++;
                    }
                    break;
                case '^':
                    pointer = forward_pointer(pointer, filelength);
                    if (buff[pointer] == '=') {
                        strcpy(tokenArray[token_num].name, "^=");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "bitXorAssign");
                        token_num ++;
                    } else {
                        pointer = retract_pointer(pointer);
                        strcpy(tokenArray[token_num].name, "^");
                        strcpy(tokenArray[token_num].symbol, "op");
                        strcpy(tokenArray[token_num].property, "bitXor");
                        token_num ++;
                    }
                    break;
                case 0:
                    break;
                default:
                    pointer = error(pointer, filelength, "InvalidTok");
                }
            }
        pointer = forward_pointer(pointer, filelength);
    }
    return token_num;
}
int main()
{
    long * filelength;
    *filelength = 0;
    varID = 0;
    lineNums = 0;
    errorNums = 0;
    characterPerline = 0;
    buff[BUFFLEN/2-1] = 0;
    buff[BUFFLEN-1] = 0;
    varNums = 0;
    printf("please input your file adddress\n");
    scanf("%[^\n]", inputAddress);
    pf = fopen(inputAddress,"r");
    if(pf==NULL) {
        printf("File open fail\n");
        return 0;
    }
    fseek(pf,0L,SEEK_END);
    long temp = 0;
    temp = ftell(pf);
    *filelength = temp;
    rewind(pf);
    read_to_buff(FIRST_HALF, filelength);
    int token_num;
    token_num = deal_with_word(filelength);

    int op_num = 0, symbol_num = 0, constant_num = 0, keyword_num = 0, variable_num = 0;
    for(int i=0; i< token_num; i++){
        if(strcmp(tokenArray[i].symbol, "op") == 0)
            op_num ++;
        else if (strcmp(tokenArray[i].symbol, "symbol") == 0)
            symbol_num ++;
        else if (strcmp(tokenArray[i].symbol, "constant") == 0)
            constant_num ++;
        else if (strcmp(tokenArray[i].symbol, "keyword") == 0)
            keyword_num ++;
        else if (strcmp(tokenArray[i].symbol, "ID") == 0)
            variable_num ++;
    }

    std::cout<<"line num:"<<lineNums+1<<"\t"<<"chracter num:"<<temp<<std::endl;
    std::cout<<"operator num:"<<op_num<<"\t"<<"symbol num:"<<symbol_num<<"\t"<<"constant num:"<<constant_num<<'\t';
    std::cout<<"keyword num:"<<keyword_num<<'\t'<<"variable num:"<<variable_num<<std::endl;
    std::cout.width(20);
    std::cout<<"name"<<"\t";
    std::cout.width(10);
    std::cout<<"symbol"<<"\t";
    std::cout.width(10);
    std::cout<<"property"<<std::endl;
    for(int i=0; i<token_num; i++){
        std::cout.width(20);
        std::cout<<tokenArray[i].name<<"\t";
        std::cout.width(10);
        std::cout<<tokenArray[i].symbol<<"\t";
        std::cout.width(10);
        std::cout<<tokenArray[i].property<<std::endl;
    }
    std::cout<<"error Report:"<<"\t"<<errorNums<<" error"<<std::endl;

    for(int i=0; i<errorNums; i++){
        std::cout<<"Wrong on line "<<errorRecord[i].position[0]+1<<'\t'<<"col "<<errorRecord[i].position[1]<<'\t';
        std::cout<<errorRecord[i].reason<<std::endl;
    }
    fclose(pf);
    return 0;
}