#pragma once
#include "stdafx.h"

struct RegArray {
	unsigned int R0;
	unsigned int R1;
	unsigned int R2;
	unsigned int R3;
	unsigned int R4;
	unsigned int R5;
	unsigned int R6;
	unsigned int R7;
	unsigned int R8;
	unsigned int R9;
	unsigned int R10;
	unsigned int R11;
	unsigned int R12;
	unsigned int SP;
	unsigned int LR;
	unsigned int PC;
};

#define OPERATION_ADD 0
#define OPERATION_SUB 1
#define OPERATION_SHIFT_LEFT 2
#define OPERATION_SHIFT_RIGHT 3
#define OPERATION_AND 4

class VirtualMachine
{
public:
	VirtualMachine();
	~VirtualMachine();

	void setMemory(unsigned char *code, unsigned char *ram, unsigned char *rom, unsigned char *stack);
	void setCodePointerByRomShift(unsigned int shift);
	
	void run();
private:
	RegArray regArray;
	unsigned char *code;
	unsigned char *rom;

	bool flagCF, flagZF, flagNF, flagVF;

	unsigned int getRegister(int num);
	void setRegister(int num, unsigned int value);

	bool calcZFlag(unsigned int result);
	bool calcNFlag(int result);
	bool calcCFlag(unsigned int arg1, unsigned int arg2, unsigned char operation);
	bool calcVFlag(int arg1, int arg2, unsigned char operation);
};

