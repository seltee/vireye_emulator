// runner.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "virtualMachine.h"
#include "reader.h"

#include <stdlib.h>


#define RAM_SIZE 14 * 1024
#define ROM_SIZE 96 * 1024
#define MAX_FN 128

void logs(unsigned char **stackPointer) {
	printf("%s\n", *((char **)(*stackPointer)));
}

void logi(unsigned char **stackPointer) {
	printf("%i\n", *((int *)(*stackPointer)));
}

int main()
{
	void(*sysfn[MAX_FN])(unsigned char **stackPointer);

	Reader reader;
	VirtualMachine virtualMachine;
	
	if (reader.readFile("./tetris.sce")) {
		printf("CODE:\n");
		for (int i = 0; i < reader.codeLength/2; i++) {
			printf("%04X ", ((unsigned short*)(reader.code))[i]);
		}
		printf("\n");

		virtualMachine.setMemory(reader.code, reader.data, reader.rom, (new unsigned char[2048]) + 2048);
		virtualMachine.setCodePointerByRomShift(reader.entry);
		virtualMachine.run();
	}

	system("pause");
	return 0;
}