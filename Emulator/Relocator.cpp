#include "Relocator.h"




Relocator::Relocator()
{
	memset(coreFunctions, 0, 100 * sizeof(char*));
	coreFunctions[CALL_LOG_I] = "logi";
	coreFunctions[CALL_LOG_S] = "logs";
	coreFunctions[CALL_DISPLAY_VIDEO_MODE] = "displayVideoMode";
	coreFunctions[CALL_DISPLAY_SPRITE] = "displaySprite";
	coreFunctions[CALL_DISPLAY_SPRITE_ARRAY] = "displaySpriteArray";
	coreFunctions[CALL_DISPLAY_SYNC] = "displaySync";
	coreFunctions[CALL_INPUT_STATE] = "inputState";
	coreFunctions[CALL_MEMSET] = "memset";
}


Relocator::~Relocator()
{
}

void Relocator::relocateBlock(unsigned char *blockCode, unsigned char *fullCode, unsigned char *blockRam, unsigned char *blockRom, const char *symTable, SaveRelocation *relocations, int relCount) {
	printf("CODE %i, ROM %i\n", blockCode, blockRom);

	for (int rel = 0; rel < relCount; rel++) {
		const char *name = symTable ? symTable + relocations[rel].nameShift : NULL;
		printf("rel %i - %s %i %i %i %i\n", rel, relocations[rel].type == SAVE_SECTION_TYPE_LIB ? name : "NoName", relocations[rel].shift, relocations[rel].targetShift, relocations[rel].type, relocations[rel].bind);

		const unsigned char *targetSection = 0;
		if (relocations[rel].source == SAVE_SOURCE_CODE) {
			targetSection = blockCode;
		}
		if (relocations[rel].source == SAVE_SOURCE_RAM) {
			targetSection = blockRam;
		}

		if (!targetSection) {
			printf("NO TARGET %i\n", rel);
			continue;
		}

		if (relocations[rel].type == SAVE_SECTION_TYPE_UNKNOWN) {
			printf("UNKNOWN BLOCK\n");
		}

		if (relocations[rel].type == SAVE_SECTION_TYPE_RAM) {
			printf("RAM %i %i %i %i %s\n", *((unsigned int*)(targetSection + relocations[rel].shift)), relocations[rel].shift, relocations[rel].targetShift, blockRam, blockRam);
			*((unsigned int*)(targetSection + relocations[rel].shift)) = (unsigned int)blockRam + relocations[rel].targetShift;
		}

		if (relocations[rel].type == SAVE_SECTION_TYPE_ROM) {
			printf("SAVE ROM %i %i %i %i %s\n", *((unsigned int*)(targetSection + relocations[rel].shift)), (unsigned int)blockRom, relocations[rel].targetShift, relocations[rel].shift, blockRom + relocations[rel].targetShift);
			*((unsigned int*)(targetSection + relocations[rel].shift)) = (unsigned int)blockRom + relocations[rel].targetShift;
		}

		if (relocations[rel].type == SAVE_SECTION_TYPE_CODE) {
			printf("SAVE CODE\n");
			unsigned short word = *((unsigned short *)(targetSection + relocations[rel].shift));
			unsigned short word2 = *((unsigned short *)(targetSection + relocations[rel].shift + 2));
			char precode = word >> 13;
			printf("Word %i %i, %i %04X %i\n", word, precode, relocations[rel].shift, word, relocations[rel].targetShift);

			if (precode == 7) {
				printf("Code is full jump\n");
				int target = (((relocations[rel].targetShift) >> 1) + 200) & 0x3FFFFF;
				printf("Target %i, Target shift %i\n", target, relocations[rel].targetShift);
				word &= 0xf800;
				word |= (target >> 11);
				*((unsigned short *)(targetSection + relocations[rel].shift)) = word;
				word2 &= 0xf800;
				word2 |= (target & 0x7ff);
				*((unsigned short *)(targetSection + relocations[rel].shift + 2)) = word2;

				word = *((unsigned short *)(targetSection + relocations[rel].shift));
				word2 = *((unsigned short *)(targetSection + relocations[rel].shift + 2));

				printf("Check %i\n", ((((word & 0x7ff) << 11) + (word2 & 0x7ff)) - 200) << 1);
			}
			else {
				printf("Just a place\n");
				printf("Target %i %i %i %i\n", relocations[rel].shift, relocations[rel].targetShift, fullCode, (unsigned int)fullCode + relocations[rel].targetShift);
				*((unsigned int *)(targetSection + relocations[rel].shift)) = ((unsigned int)fullCode + relocations[rel].targetShift + *((unsigned int *)(targetSection + relocations[rel].shift))) & 0xfffffffe;
			}
			
		}

		if (relocations[rel].type == SAVE_SECTION_TYPE_LIB) {
			if (strlen(name) > 5 && name[0] == 'c' && name[1] == 'o' && name[2] == 'r' && name[3] == 'e' && name[4] == '.') {
				const char *fnName = name + 5;
				printf("Core function %s\n", fnName);
				int fnNum = -1;
				for (int i = 0; i < 100; i++) {
					if (coreFunctions[i] && strcmp(coreFunctions[i], fnName) == 0) {
						fnNum = i;
					}
				}
				if (fnNum != -1) {
					unsigned short word = *((unsigned short *)(targetSection + relocations[rel].shift));
					char precode = word >> 13;
					//printf("Word %i %i, %i %04X\n", word, precode, relocations[rel].shift, word);
					if (precode == 7) {
						word &= 0xf800;
						*((unsigned short *)(targetSection + relocations[rel].shift)) = word;
						word = *((unsigned short *)(targetSection + relocations[rel].shift + 2));
						word &= 0xf800;
						word |= fnNum;
						*((unsigned short *)(targetSection + relocations[rel].shift + 2)) = word;
					}
					else {
						printf("UNKNOWN PRECODE\n");
					}
				}
				else {
					printf("FUNCTION NOT FOUND %s\n", name);
				}
			}
		}
	}
}