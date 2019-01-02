#pragma once
#include "stdafx.h"
#include "../Linker/Headers.h"

class Reader
{
public:
	Reader();
	~Reader();

	bool readFile(char *path);

	short int version;
	short int subVersion;
	unsigned int maxBlockSize;
	unsigned int entry;

	unsigned char *code;
	unsigned int codeLength;
	unsigned char *data;
	unsigned int dataLength;
	unsigned char *rom;
	unsigned int romLength;
	unsigned char *sym;
	unsigned int symLength;

	unsigned short int functionCount;
	unsigned char *fnData;
	unsigned int fnDataLength;
	unsigned int fnCount;

	bool endFound;
};

