#include "reader.h"
#include "Relocator.h"

Reader::Reader()
{
	code = 0;
	data = 0;
	rom = 0;
	sym = 0;
	codeLength = 0;
	dataLength = 0;
	romLength = 0;
	symLength = 0;
}


Reader::~Reader()
{
}

bool Reader::readFile(char *path) {
	FILE *file = fopen(path, "rb");
	endFound = false;

	SaveMainHeader mainHeader;
	SaveUsualHeader usualHeader;
	SaveCodePartHeader saveCodePartHeader;
	Relocator relocator;
	int i;
	SaveRelocation *relocations;
	char megaFile[128 * 1024], *tableNames;
	char *reader;

	if (file) {
		int readed = fread(megaFile, 1, 128 * 1024, file);
		fclose(file);
		printf("File %s, readed %i\n", path, readed);

		reader = megaFile;
		memcpy(&mainHeader, reader, sizeof(SaveMainHeader));
		reader += sizeof(SaveMainHeader);
		if (mainHeader.mark[0] != 'W' || mainHeader.mark[1] != 'U' || mainHeader.mark[2] != 'M' || mainHeader.mark[3] != 'C') {
			printf("NOT A WUMC EXECUTABLE %s\n", path);
			return false;
		}

		version = mainHeader.version;
		subVersion = mainHeader.subVersion;
		maxBlockSize = mainHeader.maxCodeBlockSize;
		entry = mainHeader.entry;

		code = new unsigned char[mainHeader.codeSize];
		unsigned char *codePart = new unsigned char[maxBlockSize];
		codeLength = mainHeader.codeSize;

		romLength = mainHeader.romSize;
		rom = new unsigned char[romLength];

		dataLength = mainHeader.ramSize;
		data = new unsigned char[dataLength];
		memset(data, 0, dataLength);

		if (mainHeader.architecture != ARHITECTURE_THUMB) {
			printf("THIS VM RUNS ONLY THUMB CODE\n");
			return false;
		}

		while (1)
		{
			memcpy(&usualHeader, reader, sizeof(SaveUsualHeader));
			reader += sizeof(SaveUsualHeader);

			switch (usualHeader.type) {
			case SAVE_BLOCK_TYPE_CODE_PART:
				for (i = 0; i < usualHeader.size; i++) {
					printf("Reading block %i\n", i);
					memcpy(&saveCodePartHeader, reader, sizeof(SaveCodePartHeader));
					reader += sizeof(SaveCodePartHeader);

					memcpy(codePart, reader, saveCodePartHeader.codeLength);
					reader += saveCodePartHeader.codeLength;

					tableNames = reader;
					reader += saveCodePartHeader.symNameTableLength;

					relocations = (SaveRelocation *)reader;
					reader += saveCodePartHeader.relocationsCount * sizeof(SaveRelocation);

					printf("block %i %i %i %i\n", saveCodePartHeader.codeLength, saveCodePartHeader.globalShift, saveCodePartHeader.relocationsCount, saveCodePartHeader.symNameTableLength);

					relocator.relocateBlock(codePart, code, data, rom, tableNames, relocations, saveCodePartHeader.relocationsCount);

					memcpy(code + saveCodePartHeader.globalShift, codePart, saveCodePartHeader.codeLength);
				}
				break;

			case SAVE_BLOCK_TYPE_RODATA:
				memcpy(rom, reader, usualHeader.size);
				reader += usualHeader.size;
				break;

			case SAVE_BLOCK_TYPE_RAM:
				memcpy(data, reader, usualHeader.size);
				printf("DATA %s\n", data);
				reader += usualHeader.size;
				break;

			case SAVE_BLOCK_TYPE_RAM_RELOCATION:
				printf("RAM REL %i\n", usualHeader.size);
				relocations = (SaveRelocation *)reader;
				reader += usualHeader.size * sizeof(SaveRelocation);
				relocator.relocateBlock(NULL, code, data, rom, NULL, relocations, usualHeader.size);
				break;

			case SAVE_BLOCK_TYPE_END:
				printf("end block\n");
				endFound = true;
				return true;

			default:
				printf("uknown block type %i\n", usualHeader.type);
				return false;
			}
		}

		delete codePart;

		return false;
	}
	else {
		printf("ENABLE TO OPEN FILE %s\n", path);
		return false;
	}
}