#include <iostream>
#include<string.h>
#include<iomanip>
#define nonterminalNum 4
#define terminalNum 8
#define symbolNum 16
#define expressionNum 9
#define maxItemNum 16
#define maxItemSetNum 20
#define NUM '@'
#define null '#'
#define stackSize 128
char nonterminal[nonterminalNum] = {'S', 'E', 'T', 'F'};
char terminal[terminalNum] = {'+', '-', '*', '/','(',')',NUM,'$'};
//下面代码是计算first和follow集的
struct symbolSet
{
    char nonterminal;
    int symbolCount;
    char terminal[terminalNum];
};
symbolSet firstSet[nonterminalNum];
symbolSet followSet[nonterminalNum];
struct expression
{
    char left[2];
    char right[8];
};
expression expressionArray[expressionNum];
int get_nonterminal_index(char target)
{
    for(int i=0; i<nonterminalNum; i++){
        if(nonterminal[i] == target)
            return i;
    }
    return -1;
}
bool is_in_array(char *array, char target, int arrayLength){
    for(int i=0; i<arrayLength; i++){
        if(array[i] == target)
            return true;
    }
    return false;
}
bool get_first(expression expr, int index)
{
    int left_index = 0;
    bool flag = 0;
    left_index = get_nonterminal_index(expr.left[0]);
    if(get_nonterminal_index(expr.right[index]) == -1){
            if (!is_in_array(firstSet[left_index].terminal, expr.right[index], firstSet[left_index].symbolCount)) {
                flag = 1;
                firstSet[left_index].terminal[firstSet[left_index].symbolCount] = expr.right[index];
                firstSet[left_index].symbolCount++;
            }
    }else{
        int right_index = get_nonterminal_index(expr.right[index]);
        for(int i=0; i<firstSet[right_index].symbolCount; i++){
            if (firstSet[right_index].terminal[i] == null){//处理非终结符first集中有NULL的情况
                if(expr.right[index+1] == 0){
                    if (!is_in_array(firstSet[left_index].terminal, null, firstSet[left_index].symbolCount)) {
                        flag = 1;
                        firstSet[left_index].terminal[firstSet[left_index].symbolCount] = null;
                        firstSet[left_index].symbolCount++;
                    }
                }else {
                    flag = get_first(expr, index + 1);
                }
            }
            if (!is_in_array(firstSet[left_index].terminal, firstSet[right_index].terminal[i], firstSet[left_index].symbolCount) && firstSet[right_index].terminal[i] != null) {
                flag = 1;
                firstSet[left_index].terminal[firstSet[left_index].symbolCount] = firstSet[right_index].terminal[i];
                firstSet[left_index].symbolCount++;
            }
        }
    }
    return flag;
}
void get_firstSet(int num)
{
    bool flag;
    do{
        flag = 0;
        for(int i=0; i<num; i++){
            flag |= get_first(expressionArray[i], 0);
        }
    }while(flag);
}
bool insert_follow_set(char target, int insert_index)
{
    if(is_in_array(followSet[insert_index].terminal, target, followSet[insert_index].symbolCount)){
        return false;
    }else{
        followSet[insert_index].terminal[followSet[insert_index].symbolCount] = target;
        followSet[insert_index].symbolCount ++;
        return true;
    }
}
bool get_follow(expression expr)
{
    bool flag = 0;
    int end = 0;
    for(end=0; expr.right[end] != 0; end++){
        int temp_index = get_nonterminal_index(expr.right[end]);
        if(temp_index != -1){
            if(expr.right[end+1]!=0){
                int check_index = get_nonterminal_index(expr.right[end+1]);
                if(check_index == -1){
                    flag |= insert_follow_set(expr.right[end+1], temp_index);
                }else{
                    for(int i=0; i<firstSet[check_index].symbolCount; i++){
                        if(firstSet[check_index].terminal[i] != null){
                            flag |= insert_follow_set(firstSet[check_index].terminal[i], temp_index);
                        }
                    }
                }
            }
        }
    }
    end--;
    while(end >= 0){
        int temp_index = get_nonterminal_index(expr.right[end]);
        if(temp_index != -1){
            int left_index = get_nonterminal_index(expr.left[0]);
            for(int i=0; i<followSet[left_index].symbolCount; i++){
                flag |= insert_follow_set(followSet[left_index].terminal[i], temp_index);
            }
            bool has_null = false;
            for(int i=0; i<firstSet[temp_index].symbolCount; i++){
                if(firstSet[temp_index].terminal[i] == null){
                    has_null = true;
                }
                if(has_null)
                    end --;
                else
                    return flag;
            }
        }else{
            return flag;
        }
    }
    return flag;
}
void get_follow_set(int num)
{
    bool flag;
    do{
        flag = 0;
        for(int i=0; i<num; i++){
            flag |= get_follow(expressionArray[i]);
        }
    }while(flag);
}

//下面代码是计算识别活前缀的DFA的
struct item
{
    expression expr;
    int point_index = 0;
    bool checked = 0;
};
struct itemset
{
    item kernel[maxItemNum];
    int kernelSize = 0;
    item closure[maxItemNum];
    int closureSize = 0;
};
itemset itemSetForDFA[maxItemSetNum];
int itemSetIndex = 0;
struct relation
{
    char gotoToken;
    int gotoEntity;
};
struct entity
{
    relation relations[8];
    int relationNum = 0;
};
entity entitySet[maxItemNum];
bool is_item_equal(item a, item b)
{
    if(strcmp(a.expr.left, b.expr.left) == 0 && strcmp(a.expr.right, b.expr.right) == 0){
        if(a.point_index == b.point_index){
            return true;
        }
        return false;
    }
    return false;
}
bool is_itemSet_equal(itemset a, itemset b)
{
    //先比较数量，然后再比较kernel是否相同
    // closure 包含了kernel
    if(a.kernelSize != b.kernelSize)
        return false;

    for(int i=0; i<a.kernelSize; i++)
    {
        if(is_item_equal(a.kernel[i], b.kernel[i]) == false)
            return false;
    }
    return true;
}
void get_closure(int tempIndex)
{
    bool flag;
    do{
        flag = 0;
        for (int i=0; i<itemSetForDFA[tempIndex].closureSize; i++){
            int pointIndex = itemSetForDFA[tempIndex].closure[i].point_index;
            char tempToken = itemSetForDFA[tempIndex].closure[i].expr.right[pointIndex];
            int pos = get_nonterminal_index(tempToken);
            if (pos != 0) {
                for (int t = 0; t < expressionNum; t++) {
                    if (expressionArray[t].left[0] == tempToken) {
                        bool alreadyExist = 0;
                        for (int m = 0; m < itemSetForDFA[tempIndex].closureSize; m++) {
                            item temp_item;
                            temp_item.expr = expressionArray[t];
                            temp_item.point_index = 0;
                            alreadyExist |= is_item_equal(itemSetForDFA[tempIndex].closure[m], temp_item);
                        }
                        if (alreadyExist == 0) {
                            int tempSize = itemSetForDFA[tempIndex].closureSize;
                            itemSetForDFA[tempIndex].closure[tempSize].expr = expressionArray[t];
                            itemSetForDFA[tempIndex].closure[tempSize].point_index = 0;
                            itemSetForDFA[tempIndex].closureSize++;
                            flag = 1;
                        }
                    }
                }
            }
        }
    }while(flag);
}
void set_new_item(int temp_index, int pointIndex, expression expr)
{
    itemSetForDFA[itemSetIndex].closure[temp_index].expr = expr;
    itemSetForDFA[itemSetIndex].closure[temp_index].checked = false;
    itemSetForDFA[itemSetIndex].closure[temp_index].point_index = pointIndex;
    itemSetForDFA[itemSetIndex].closureSize += 1;
    itemSetForDFA[itemSetIndex].kernel[temp_index].expr = expr;
    itemSetForDFA[itemSetIndex].kernel[temp_index].checked = false;
    itemSetForDFA[itemSetIndex].kernel[temp_index].point_index = pointIndex;
    itemSetForDFA[itemSetIndex].kernelSize += 1;
}
int get_new_itemSet(int old_index)
{
    char gotoToken;
    int i;
    bool allChecked = true;
    int itemcount = 0;
    for(i=0; i<itemSetForDFA[old_index].closureSize; i++){
        if(itemSetForDFA[old_index].closure[i].checked == false){
            itemSetForDFA[old_index].closure[i].checked = true;
            int pointIndex = itemSetForDFA[old_index].closure[i].point_index;
            gotoToken = itemSetForDFA[old_index].closure[i].expr.right[pointIndex];
            if(gotoToken == 0)
                continue;
            allChecked = false;
            if(gotoToken != null) {
                itemSetIndex++;
                set_new_item(itemcount, pointIndex+1, itemSetForDFA[old_index].closure[i].expr);
                itemcount++;
            }
            break;
        }
    }
    if(allChecked)
        return -1;
    for(i+=1; i<itemSetForDFA[old_index].closureSize; i++){
        int pointIndex = itemSetForDFA[old_index].closure[i].point_index;
        if(itemSetForDFA[old_index].closure[i].expr.right[pointIndex] == gotoToken){
            itemSetForDFA[old_index].closure[i].checked = true;
            if(gotoToken != null) {
                set_new_item(itemcount, pointIndex + 1, itemSetForDFA[old_index].closure[i].expr);
                itemcount++;
            }
        }
    }
    int gotoEntity = itemSetIndex;
    for(int m=0; m<itemSetIndex-1; m++){
        //如果状态已经存在，则消除该状态
        if(is_itemSet_equal(itemSetForDFA[m], itemSetForDFA[itemSetIndex]) == true){
            gotoEntity = m;
            itemSetForDFA[itemSetIndex].kernelSize = 0;
            itemSetForDFA[itemSetIndex].closureSize = 0;
            itemSetIndex --;
        }
    }
    if(gotoToken != null) {
        entitySet[old_index].relations[entitySet[old_index].relationNum].gotoToken = gotoToken;
        entitySet[old_index].relations[entitySet[old_index].relationNum].gotoEntity = gotoEntity;
        entitySet[old_index].relationNum++;
    }
    return itemSetIndex;
}
void get_DFA()
{
    int index = 0;
    do{
        get_closure(index);
        while(get_new_itemSet(index) != -1);
        index ++;
    }while(index < itemSetIndex);
}
//以下代码构造SLR(1)分析表
char SLR_ACTION[maxItemSetNum][terminalNum][4];
char SLR_GOTO[maxItemSetNum][nonterminalNum];
int get_terminal_index(char target)
{
    if(target >= '0' && target <= '9' )
        target = NUM;
    for(int i=0; i<terminalNum; i++){
        if(terminal[i] == target)
            return i;
    }
    return -1;
}
int get_expression_index(expression expr)
{
    for(int i=0; i<expressionNum; i++){
        if(strcmp(expr.left, expressionArray[i].left) == 0){
            if(strcmp(expr.right, expressionArray[i].right) == 0){
                return i;
            }
        }
    }
    return -1;
}
void get_SLR_table()
{
    for(int i=0; i<itemSetIndex; i++){
        for(int t=0; t<entitySet[i].relationNum; t++){
            char target = entitySet[i].relations[t].gotoToken;
            int entity = entitySet[i].relations[t].gotoEntity;
            int temp_index = get_nonterminal_index(target);
            if(temp_index == -1){
                //是终结符
                temp_index = get_terminal_index(target);
                SLR_ACTION[i][temp_index][0] = 's';
                SLR_ACTION[i][temp_index][1] = entity;
            }else{
                //是非终结符
                SLR_GOTO[i][temp_index] = entity;
            }
        }
    }
    //处理规约项
    for(int i=0; i<itemSetIndex; i++){
        for(int t=0; t<itemSetForDFA[i].closureSize; t++){
            int pointIndex = itemSetForDFA[i].closure[t].point_index;
            if(itemSetForDFA[i].closure[t].expr.right[pointIndex] == 0){
                if(itemSetForDFA[i].closure[t].expr.left[0] == 'S'){
                    int temp_index = get_terminal_index('$');
                    strcpy(SLR_ACTION[i][temp_index], "acc");
                }
                else{
                    char tempNonterminal = itemSetForDFA[i].closure[t].expr.left[0];
                    int tempIndex = get_nonterminal_index(tempNonterminal);
                    for(int m =0; m<followSet[tempIndex].symbolCount; m++){
                        char tempterminal = followSet[tempIndex].terminal[m];
                        int terminalIndex = get_terminal_index(tempterminal);
                        SLR_ACTION[i][terminalIndex][0] = 'r';
                        SLR_ACTION[i][terminalIndex][1] = get_expression_index(itemSetForDFA[i].closure[t].expr);
                    }
                }
            }
        }
    }
}
//接下来是文法分析程序
bool LR_analysis(char* str)
{
    char tokenStack[stackSize];
    char statusStack[stackSize];
    statusStack[0] = 0;
    int stackPointer = 0;
    int strPointer = 0;
    int terminalIndex;
    std::cout<<"栈"<<"\t"<<"输入"<<"\t"<<"分析动作"<<std::endl;
    while(true){
        terminalIndex = get_terminal_index(str[strPointer]);
        for(int i=0; i<stackPointer+1; i++){
            std::cout<<int(statusStack[i]);
        }
        std::cout<<"\n";
        std::cout<<" ";
        for(int i=1; i<stackPointer+1; i++){
            std::cout<<tokenStack[i];
        }
        std::cout<<"\t";
        for(int i=strPointer; i<strlen(str); i++){
            std::cout<<str[i];
        }
        std::cout<<"\t";
        if(SLR_ACTION[statusStack[stackPointer]][terminalIndex][0] == 's'){
            std::cout<<char(SLR_ACTION[statusStack[stackPointer]][terminalIndex][0])<<int(SLR_ACTION[statusStack[stackPointer]][terminalIndex][1])<<'\n';
            stackPointer ++;
            tokenStack[stackPointer] = str[strPointer];
            statusStack[stackPointer] = SLR_ACTION[statusStack[stackPointer-1]][terminalIndex][1];
            strPointer ++;
        }else if(SLR_ACTION[statusStack[stackPointer]][terminalIndex][0] == 'r'){
            int temp_index = SLR_ACTION[statusStack[stackPointer]][terminalIndex][1];
            int tempLen = strlen(expressionArray[temp_index].right);
            stackPointer -= tempLen;
            int tempNonterminal = get_nonterminal_index(expressionArray[temp_index].left[0]);
            int gotoStatus =  SLR_GOTO[statusStack[stackPointer]][tempNonterminal];
            stackPointer ++;
            tokenStack[stackPointer] = nonterminal[tempNonterminal];
            statusStack[stackPointer] = gotoStatus;
            std::cout<<"reduce by "<<expressionArray[temp_index].left<<"->"<<expressionArray[temp_index].right<<'\t';
            std::cout<<"goto "<<gotoStatus<<std::endl;
        }else if(strcmp(SLR_ACTION[statusStack[stackPointer]][terminalIndex], "acc") == 0){
            std::cout<<"acc"<<std::endl;
            return true;
        }else
            return false;
    }
}
void init_expression()
{
    strcpy(expressionArray[0].left, "S");
    strcpy(expressionArray[0].right, "E");
    strcpy(expressionArray[1].left, "E");
    strcpy(expressionArray[1].right, "E+T");
    strcpy(expressionArray[2].left, "E");
    strcpy(expressionArray[2].right, "E-T");
    strcpy(expressionArray[3].left, "E");
    strcpy(expressionArray[3].right, "T");
    strcpy(expressionArray[4].left, "T");
    strcpy(expressionArray[4].right, "T*F");
    strcpy(expressionArray[5].left, "T");
    strcpy(expressionArray[5].right, "T/F");
    strcpy(expressionArray[6].left, "T");
    strcpy(expressionArray[6].right, "F");
    strcpy(expressionArray[7].left, "F");
    strcpy(expressionArray[7].right, "(E)");
    strcpy(expressionArray[8].left, "F");
    strcpy(expressionArray[8].right, "@");
    for(int i=0; i<nonterminalNum; i++){
        firstSet[i].nonterminal = nonterminal[i];
        followSet[i].nonterminal = nonterminal[i];
    }
    followSet[0].terminal[0] = '$';
    followSet[0].symbolCount ++;
    itemSetForDFA[0].kernel[0].expr = expressionArray[0];
    itemSetForDFA[0].kernel[0].point_index = 0;
    itemSetForDFA[0].kernelSize = 1;
    itemSetForDFA[0].closure[0].expr = expressionArray[0];
    itemSetForDFA[0].closure[0].point_index = 0;
    itemSetForDFA[0].closureSize = 1;
}
int main() {
    memset(firstSet, 0, sizeof(char)*nonterminalNum*symbolNum);
    memset(followSet, 0, sizeof(char)*nonterminalNum*symbolNum);
    memset(expressionArray, 0, sizeof(expression)*expressionNum);
    memset(itemSetForDFA, 0, sizeof(itemset)*maxItemSetNum);
    init_expression();
    get_firstSet(expressionNum);
    get_follow_set(expressionNum);
    std::cout<<"First集"<<std::endl;
    for(int i=0; i<nonterminalNum; i++){
        std::cout<<"symbol:"<<firstSet[i].nonterminal<<"\t";
        for(int t=0; t<firstSet[i].symbolCount; t++) {
            if(firstSet[i].terminal[t] == NUM)
                std::cout << "NUM"<<" ";
            else
                std::cout << firstSet[i].terminal[t]<<" ";
        }
        std::cout<<std::endl;
    }
    std::cout<<"Folllow集"<<std::endl;
    for(int i=0; i<nonterminalNum; i++){
        std::cout<<"symbol:"<<followSet[i].nonterminal<<"\t";
        for(int t=0; t<followSet[i].symbolCount; t++) {
            if(followSet[i].terminal[t] == NUM)
                std::cout << "NUM"<<" ";
            else
                std::cout << followSet[i].terminal[t]<<" ";
        }
        std::cout<<std::endl;
    }
    get_DFA();
    itemSetIndex ++;
    for(int i=0; i<itemSetIndex; i++){
        std::cout<<"项目规范集:"<<i<<'\t';
        for(int t=0; t<itemSetForDFA[i].closureSize; t++){
            std::cout<<itemSetForDFA[i].closure[t].expr.left<<"->";
            for(int m=0; itemSetForDFA[i].closure[t].expr.right[m]!=0; m++){
                if(itemSetForDFA[i].closure[t].point_index == m)
                    std::cout<<"·";
                if(itemSetForDFA[i].closure[t].expr.right[m] == NUM)
                    std::cout << "NUM";
                else
                    std::cout<<itemSetForDFA[i].closure[t].expr.right[m];
            }
            std::cout<<'\t';
        }
        std::cout<<std::endl;
    }
    for(int i=0; i<itemSetIndex; i++){
        std::cout<<"项目规范集"<<i<<"的goto关系如下:";
        for(int t=0; t<entitySet[i].relationNum; t++){
            char gotoToken = entitySet[i].relations[t].gotoToken;
            std::cout<<i<<"--";
            if(gotoToken == NUM)
                std::cout<<"NUM";
            else
                std::cout<<gotoToken;
            std::cout<<"--"<<entitySet[i].relations[t].gotoEntity<<'\t';
        }
        std::cout<<std::endl;
    }
    get_SLR_table();
    std::cout<<"SLR(1)分析表："<<std::endl;
    std::cout<<"状态"<<"\t\t";
    for(int i=0; i<terminalNum; i++){
        if(terminal[i] == NUM)
            std::cout<<"NUM"<<'\t';
        else
            std::cout<<terminal[i]<<'\t';
    }
    std::cout<<"|";
    for(int i=0; i<nonterminalNum; i++){
        if(nonterminal[i] == 'S')
            continue;
        std::cout<<nonterminal[i]<<'\t';
    }
    std::cout<<std::endl;
    for(int i=0; i<itemSetIndex; i++){
        std::cout<<"状态"<<i<<'\t';
        for(int t=0; t<terminalNum; t++){
            if(SLR_ACTION[i][t][0] == 's' || SLR_ACTION[i][t][0] == 'r')
                std::cout<<SLR_ACTION[i][t][0]<<int(SLR_ACTION[i][t][1])<<'\t';
            else if(strcmp(SLR_ACTION[i][t], "acc") == 0)
                std::cout<<"acc"<<'\t';
            else
                std::cout<<'\t';
        }
        for(int t=0; t<nonterminalNum; t++){
            if(nonterminal[t] == 'S')
                continue;
            std::cout<<int(SLR_GOTO[i][t])<<'\t';
        }
        std::cout<<std::endl;
    }
    char str[128];
    memset(str, 0, sizeof(char)*128);
    std::cout<<"please input your string"<<std::endl;
    std::cin>>str;
    str[strlen(str)] = '$';
    if(!LR_analysis(str)){
        std::cout<<"Not accept"<<std::endl;
    }else{
        std::cout<<"accept"<<std::endl;
    }
    return 0;
}
