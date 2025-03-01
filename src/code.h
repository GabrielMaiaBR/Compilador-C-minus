#ifndef _CODE_H_
#define _CODE_H_

/* pc = program counter  */
#define PC 7

/* registers */
#define r3 3
#define r4 4
#define r5 5

/* accumulator */
#define ac 0

/* 2nd accumulator */
#define ac1 1

void emitComment(char* c);
void emitRO(char* op, int r, int s, int t, char* c);
void emitRM(char* op, int r, int d, int s, char* c);
int  emitSkip(int howMany);
void emitBackup(int loc);
void emitRestore(void);
void emitRM_Abs(char* op, int r, int a, char* c);

#endif