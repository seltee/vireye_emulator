#pragma once
#include "stdafx.h"
#include "../Linker/Headers.h"
#include "FunctionMap.h"

class Relocator
{
public:
	Relocator();
	~Relocator();

	void relocateBlock(unsigned char *blockCode, unsigned char *fullCode, unsigned char *blockRam, unsigned char *blockRom, const char *symTable, SaveRelocation *relocations, int relCount);

private:
	char *coreFunctions[100];
};

