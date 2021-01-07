# SIC Simulator實作
因系統程式的需求，使用C語言實作SIC Simulator

## 初始值、變數宣告
```
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define cMIN -1
#define cLOAD 0
#define cSHOW 1
#define cUNLOAD 2
#define cEXIT 3
#define cRUN 4
#define cMAX 5
#define oMIN -1
#define oADD 0
#define oAND 1
#define oCOMP 2
#define oDIV 3
#define oJ 4
#define oJEQ 5
#define oJGT 6
#define oJLT 7
#define oJSUB 8
#define oLDA 9
#define oLDCH 10
#define oLDL 11
#define oLDX 12
#define oMUL 13
#define oOR 14
#define oRD 15
#define oRSUB 16
#define oSTA 17
#define oSTCH 18
#define oSTL 19
#define oSTSW 20
#define oSTX 21
#define oSUB 22
#define oTD 23
#define oTIX 24
#define oWD 25
#define oMAX 26
#define empty 0
#define eq 1
#define gt 2
#define lt 3


FILE* f;
//filename
char fname[20];
//
char tname[20];
//command line
char c_line[30];
//object code line
char o_line[80];
// command load,show,unload,exit,run
char cmd[8];
//program length
int prog_len = 0;
//start address
int start_add = 0;
//record first address
int first_add = 0;
//cursor address
int curr_add = 0;

//virtual memory block
char* memory;
int mem_size = 0;
int loaded = 0;
int op = 0;
int indexed = 0;
int operand = 0;
int running = 0;

//register A,register X,register L,reister PC,register SW
int reg_A, reg_X, reg_L, reg_PC, reg_SW;

//80分
//show出命令提示列
//unload : 把前面都忘掉，再show要把前面的都丟掉
//exit : 程式結束
//100分(難)
//提示符號打run，執行object code，變數有被修改。



const char s_command[5][7] = { "load", "show", "unload", "exit", "run" };
const int MAXADD = 0x7FFF;
const int IKEY = 0x8000;

const char optab[26][3] = { "18", "40", "28", "24", "3C", "30", "34", "38", "48", "00", "50", "08", "04", "20", "44", "D8", "4C", "0C", "54", "14", "E8", "10", "1C", "E0", "2C", "DC" };
```
除了SIC指令在初始實都先給上數字之外，還設定SW的值，**未設定** = 0、**相等(eq)** = 1、**大於(gt)** = 2及**小於(lt)** = 3。
## Load
### Read Record
```
void rd_header() {
    char tmp[7];
    int i, j, s;

    for (i = 7, j = 0; i < 13; i++, j++)  tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf(tmp, "%x", &start_add);

    for (i = 13, j = 0; i < 19; i++, j++)  tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf(tmp, "%x", &prog_len);

    s = prog_len * 2 + 1;
    memory = (char*)malloc(sizeof(char) * s);
    if (memory) {
        for (i = 0; i < s; i++)  memory[i] = 'X';
        memory[s - 1] = '\0';
        loaded = 1;
        mem_size = s;
    }
    else {
        printf("Loading Failed! (Memory allocation error)\n");
    }
}

void rd_text() {
    char tmp[7];
    int i, j, l, s;

    for (i = 1, j = 0; i < 7; i++, j++)  tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf(tmp, "%x", &s);
    //printf("%X",s);

    for (i = 7, j = 0; i < 9; i++, j++)  tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf(tmp, "%x", &l);

    l = 9 + l * 2;
    for (i = 9, j = (s - start_add) * 2; i < l; i++, j++)
        memory[j] = o_line[i];
}

void rd_end() {
    char tmp[7];
    int i, j;

    for (i = 1, j = 0; i < 7; i++, j++)  tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf(tmp, "%x", &first_add);
}
```
這部分的程式碼為老師提供範例碼就有提供，就仔細看過理解後用於load的操作上，在讀record的部分本來就有完成**Relocation**的部分，因此在Absolute Loader的演算法上，很多部分已經在讀Record時完成，也算是一個很大的幫助。
接下來就是將剩下的部分依照演算法完成。
### Load
```
void s_load() {
    int i;
    if(loaded) printf("ERROR : There is already an object program loaded in memory !\n");
    else{
        printf("    file name > ");
        fgets(fname, 20, stdin);
        i = strlen(fname);
        fname[i - 1] = '\0';
        f = fopen(fname, "r");
        if(f == NULL) printf("ERROR : Failed to open dile!\n");
        else{
            fgets(o_line, 80, f);
            i = strlen(o_line);
            o_line[i - 1] = '\0';
            char prog_name[7];
            if (o_line[0] == 'H') {
                int i, j;
                for (i = 1, j = 0; i < 7; i++, j++)  prog_name[j] = o_line[i];
                prog_name[j] = '\0';
                rd_header();
            }
            fgets(o_line, 80, f);
            //verify program name and length
            printf("Program name = [%s], Program Length = [%x]\n", prog_name, prog_len);
            while (o_line[0] != 'E') {
                int i;
                i = strlen(o_line);
                o_line[i - 1] = '\0';
                //printf("%s\n", o_line);
                rd_text();
                fgets(o_line, 80, f);
            }
            rd_end();
            loaded = 1;
            o_line[0] = '\0';
            printf("Load Successful!\n");
        }
        fclose(f);
    }
}
```
看到其他人的程式碼都是將開檔案部分直接就寫在load的command之後，但是由於當初所想就沒想那麼多，就在load的部分上加上一個輸入的部分。
**loaded**用於紀錄是否已經有程式在內部，如果已經有城市在內部就沒有辦法load並且回報錯誤資訊。
由於每個Record的第一個字元會提示這個Record是哪種Record，可用於判斷是否將程式讀完，並且在讀完程式時回報Load Successful!。
這個部份在課本上有提供演算法，就在這邊也順便附上。
![](https://i.imgur.com/IZn3RFl.png)
其實簡單來說就是把每個Record讀完之後跳到結束位址。
## Show
```
void s_show() {
    int i, j = 0;
    for(curr_add = first_add;curr_add < first_add + prog_len;curr_add += 16){
        printf("%x      ",curr_add);
        for(i = 0 + j * 32;i < 8 + j * 32 && memory[i] != '\0';i++) printf("%c",memory[i]);
        printf("      ");
        for(;i < 16 + j * 32 && memory[i] != '\0';i++) printf("%c",memory[i]);
        printf("      ");
        for(;i < 24 + j * 32 && memory[i] != '\0';i++) printf("%c",memory[i]);
        printf("      ");
        for(;i < 32 + j * 32 && memory[i] != '\0';i++) printf("%c",memory[i]);
        printf("\n");
        j++;
    }
}
```
顯示目前Memory內所包含的資料，由於前便是使用Dynamic Allocated來進行陣列配置，因此會依照程式的大小配置陣列大小。在未配置的空間會以'X'裝入。
## Unload
主要目的就是將已經讀入的部分清除。
```
void s_unload() {
    if(loaded){
        fclose(f);
        fname[0] = '\0';
        o_line[0] = '\0';
        free(memory);
        prog_len = 0;
        start_add = 0;
        first_add = 0;
        curr_add = 0;
        mem_size = 0;
        loaded = 0;
        op = 0;
        indexed = 0;
        operand = 0;
    }
}
```
這部分主要就是將已經讀入的東西清除，因此這部分就十分直覺。
## Run
### initial_run
```
void init_run() {
    reg_A = 0;
    reg_X = 0;
    reg_L = 0;
    reg_PC = first_add;
    reg_SW = 0;
    curr_add = (first_add - start_add) * 2;
    running = 1;
}
```
初始執行前的狀態，將暫存器歸零，Program Counter調到目前即將執行的位址。
### get_op
```
void get_op() {
    char s[3];
    char t[5];
    s[0] = memory[curr_add];
    s[1] = memory[curr_add + 1];
    s[2] = '\0';
    op = lookup(s);
    t[0] = memory[curr_add + 2];
    t[1] = memory[curr_add + 3];
    t[2] = memory[curr_add + 4];
    t[3] = memory[curr_add + 5];
    t[4] = '\0';
    sscanf(t, "%X", &operand);
    if (operand >= IKEY) {
        indexed = 1;
        operand -= IKEY;
    }
    else {
        indexed = 0;
    }
    curr_add += 6;
    //printf("*%X*", curr_add );
    reg_PC += 3;
}
```
針對每一個object code進行分析，將opode讀出，operand讀出，並進行轉換。
由於前面有對每一個opcode進行數字編號，這個部分主要就是轉換，並且進行位置的調控。
### get、put value/byte
```
int get_value(int r, int x) {
    int tmp = 0;
    char s[7];
    int i, j;
    if (x) r += reg_X;
    i = (r - start_add) * 2;
    for (j = 0; j < 6; j++) s[j] = memory[i++];
    s[6] = '\0';
    sscanf(s, "%X", &tmp);
    return tmp;
}

int get_byte(int r, int x) {
    int tmp = 0;
    char s[3];
    int i, j;
    if (x) r += reg_X;
    i = (r - start_add) * 2;
    j = 0;
    s[j++] = memory[i++];
    s[j++] = memory[i++];
    s[2] = '\0';
    sscanf(s, "%X", &tmp);
    printf("LDCH: r = [%X], x = [%d], tmp = [%c]\n", r, x, tmp);
    return tmp;
}

void put_byte(int k, int r, int x) {
    int tmp = 0;
    char s[3];
    int i, j;
    if (x) r += reg_X;
    sprintf(s, "%02X", k);
    i = (r - start_add) * 2;
    j = 0;
    memory[i++] = s[j++];;
    memory[i++] = s[j++];;
}

void put_value(int k, int r, int x) {
    int tmp = 0;
    char s[7];
    int i, j;
    printf("put_value ( %X, %X, %X ) starts.\n", k, r, x);
    if (x) r += reg_X;
    sprintf(s, "%06X", k);
    i = (r - start_add) * 2;
    for (j = 0; j < 6; j++) memory[i++] = s[j];
    printf("put_value ( %X, %X, %X ) finishes.\n", k, r, x);
}
```
在進行LD及ST暫存器時會使用到，並且在進行PC relative的指令取值時也會使用到。
### show_reg
```
void show_reg() {
    printf("Register A  = [%06X];\n", reg_A);
    printf("Register X  = [%06X];\n", reg_X);
    printf("Register L  = [%06X];\n", reg_L);
    printf("Register SW = [%06X];\n", reg_SW);
    printf("Register PC = [%06X];\n", reg_PC);
}
```
展示目前reg的狀況，用於執行完後展示。
### s_run
```
void s_run() {
    init_run();
    printf("*%X*", curr_add );
    while (running) {
        get_op();
        //printf("%X", operand );
        //printf("%d\n", op);
        switch(op){
        case 0:
            reg_A += get_value(operand,indexed);
            break;
        case 1:
            reg_A &= get_value(operand,indexed);
            break;
        case 2:
            if(reg_A > get_value(operand,indexed)) reg_SW = gt;
            else if(reg_A < get_value(operand,indexed)) reg_SW = lt;
            else reg_SW = eq;
            break;
        case 3:
            reg_A /= get_value(operand,indexed);
            break;
        case 4:
            reg_PC = operand;
            curr_add = (reg_PC - start_add)*2;
            break;
        case 5:
            if(reg_SW == eq) reg_PC = operand,curr_add = (reg_PC - start_add)*2;
            break;
        case 6:
            if(reg_SW == gt) reg_PC = operand,curr_add = (reg_PC - start_add)*2;
            break;
        case 7:
            if(reg_SW == lt) reg_PC = operand,curr_add = (reg_PC - start_add)*2;
            break;
        case 8:
            reg_L = reg_PC,reg_PC = operand, curr_add = (reg_PC - start_add)*2;
            break;
        case 9:
            reg_A = get_value(operand,indexed);
            break;
        case 10:
            reg_A = (reg_A & 16776960) | get_byte(operand,indexed);
            break;
        case 11:
            reg_L = get_value(operand,indexed);
            break;
        case 12:
            reg_X = get_value(operand,indexed);
        case 13:
            reg_A *= get_value(operand,indexed);
            break;
        case 14:
            reg_A |= get_value(operand,indexed);
            break;
        case 15:
            reg_A = reg_A;
            break;
        case 16:
            if(reg_L == 0) running = 0;
            else reg_PC = reg_L,curr_add = (reg_PC - start_add) * 2;
            break;
        case 17:
            put_value(reg_A,operand,indexed);
            break;
        case 18:
            put_byte(reg_A,operand,indexed);
            break;
        case 19:
            put_value(reg_L,operand,indexed);
            break;
        case 20 :
            put_value(reg_SW,operand,indexed);
            break;
        case 21:
            put_value(reg_X,operand,indexed);
            break;
        case 22:
            reg_A -= get_value(operand,indexed);
            break;
        case 23:
            reg_SW = gt;
            break;
        case 24:
            reg_X += 1;
            if(reg_X > get_value(operand,indexed)) reg_SW = gt;
            else if(reg_X < get_value(operand,indexed)) reg_SW = lt;
            else reg_SW = eq;
            break;
        case 25:
            reg_A = reg_A;
            break;
        default:
            break;
        }
    }
    show_reg();
}
```
進行程式碼的執行，這個部分可以參考[SIC指令集](http://solomon.ipv6.club.tw/Course/SP.941/sic-instruction.html)來進行，由於程式碼較多就不一一解釋，大部分的函式都可以從前面的介紹中找到，比較特別的部分是在測試Device的部分，由於無法真正進行，只有進行準備或者跳過。
## 主程式
```
int main() {
    int comm = 0;
    comm = readline();
    while (comm != cEXIT) {
        switch (comm) {
        case cLOAD: s_load();
            break;
        case cSHOW: s_show();
            break;
        case cUNLOAD: s_unload();
            break;
        case cRUN: s_run();
            break;
        default:   printf("Unknown Command!\n");
            break;
        }
        comm = cMIN;
        comm = readline();
    }
    if (loaded) s_unload();
}
```
當輸入exit時會結束程式並進行歸零，其餘就按照上面指令碼名稱去呼叫函式。
至此，SIC Simulator的介紹及實作也告一個段落。

## Sample Run
![](https://i.imgur.com/4UQg7Gp.png)
![](https://i.imgur.com/sE5tLVH.png)
![](https://i.imgur.com/c0Ci3Vr.png)
此為將程式碼在精簡後的結果(將一些檢查部分去除)


