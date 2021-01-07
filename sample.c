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

int lookup(char* s) {
    int i;
    for (i = 0; i < oMAX; i++)
        if (strcmp(optab[i], s) == 0) return i;
    return -1;
}

int readline() {
    int i = 0;
    int tmp = cMIN;

    printf("SIC Simulator> ");
    do {
        fgets(c_line, 30, stdin);
        i = strlen(c_line);
        c_line[i - 1] = '\0';
        printf("Line = [%s], i = [%d]\n", c_line, i);
    } while (i <= 1);

    sscanf(c_line, "%s", cmd);
    printf("Command = [%s]\n", cmd );

    if (strcmp(cmd, s_command[0]) == 0) tmp = cLOAD;
    else if (strcmp(cmd, s_command[1]) == 0) tmp = cSHOW;
    else if (strcmp(cmd, s_command[2]) == 0) tmp = cUNLOAD;
    else if (strcmp(cmd, s_command[3]) == 0) tmp = cEXIT;
    else if (strcmp(cmd, s_command[4]) == 0) tmp = cRUN;
    printf("Command number is %d.\n", tmp);

    c_line[0] = '\0';
    cmd[0] = '\0';
    return tmp;
}

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

/* Write your own s_load here. */
void s_load() {
    int i;
    printf("    file name > ");
    fgets(fname, 20, stdin);
    i = strlen(fname);
    fname[i - 1] = '\0';
    f = fopen(fname, "r");
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
        printf("%s\n", o_line);
        rd_text();
        fgets(o_line, 80, f);
    }
    printf("%s\n", o_line);
    rd_end();
    o_line[0] = '\0';
    printf("Load Successful!\n");
}

/* Write your own s_show here. */
void s_show() {
    int i, j;
    for (i = 0; i < mem_size; i += 32) {
        for (j = i; j < i + 32; j++) {
                printf("%c", memory[j]);
                if (j % 8 == 7)
                    printf(" ");
        }
        printf("\n");
    }
    printf("\n");
}

/* Write your own s_unload here. */
void s_unload() {
    int i,j;
    for (i = 0; i < mem_size; i++) {
            memory[i] = 'X';
    }
}

void init_run() {
    reg_A = 0;
    reg_X = 0;
    reg_L = 0;
    reg_PC = first_add;
    reg_SW = 0;
    curr_add = (first_add - start_add) * 2;
    running = 1;
}

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
    reg_PC += 3;
}

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

void show_reg() {
    printf("Register A  = [%06X];\n", reg_A);
    printf("Register X  = [%06X];\n", reg_X);
    printf("Register L  = [%06X];\n", reg_L);
    printf("Register SW = [%06X];\n", reg_SW);
    printf("Register PC = [%06X];\n", reg_PC);
}

/* Write your own s_run here. */
void s_run() {
    init_run();
    char CC;
    while (curr_add < start_add + mem_size) {
        get_op();
        printf("%X", operand );
        //printf("%d\n", op);
        switch (op)
        {
        case 0: reg_A += get_value(operand,indexed);
            break;
        case 1: reg_A = reg_A & get_value(operand,indexed);
            break;
        case 2:{
            if (reg_A == get_value(operand, indexed))
                CC = '=';
            else if (reg_A > get_value(operand,indexed))
                CC = '>';
            else
                CC = '<';
        }
            break;
        case 3: reg_A /= get_value(operand,indexed);
            break;
        case 4: reg_PC = operand;
            break;
        case 5: if (CC == '=') reg_PC = operand;
            break;
        case 6: if (CC == '>') reg_PC = operand;
            break;
        case 7 :if (CC == '<') reg_PC = operand;
            break;
        case 8:{
            reg_L = reg_PC;
            reg_PC = operand;
        }
            break;
        case 9: reg_A = get_value(operand, indexed);
            break;
        case 10:reg_A = get_byte(operand, indexed);
            break;
        case 11:reg_L = get_value(operand, indexed);
            break;
        case 12: reg_X = get_value(operand, indexed);
            break;
        case 13: reg_A *= get_value(operand, indexed);
            break;
        case 14: reg_A = reg_A || get_value(operand, indexed);
            break;
        case 15: reg_A = get_byte(operand, indexed);
            break;
        case 16: reg_PC = reg_L;
            break;
        case 17: put_value(reg_A,operand, indexed);
            break;
        case 18: put_byte(reg_A, operand, indexed);
            break;
        case 19: put_value(reg_L, operand, indexed);
            break;
        case 20: put_value(reg_SW, operand, indexed);
            break;
        case 21: put_value(reg_X, operand, indexed);
            break;
        case 22: reg_A -= get_value(operand, indexed);
            break;
        case 23: printf("Test Device %d", operand);
            break;
        case 24:reg_X += 1;
            break;
        case 25: put_byte(reg_A, operand, indexed);
            break;
        default: printf("test");
            break;
        }
    }
}

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
