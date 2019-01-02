#include "virtualMachine.h"
#include "FunctionMap.h"
#include "Display.h"

VirtualMachine::VirtualMachine()
{
	code = 0;
	memset(&regArray, 0, sizeof(RegArray));
}


VirtualMachine::~VirtualMachine()
{
}

void VirtualMachine::setMemory(unsigned char *code, unsigned char *ram, unsigned char *rom, unsigned char *stack) {
	this->code = code;
	this->rom = rom;
	regArray.PC = (unsigned int)code;
	regArray.SP = (unsigned int)stack;
	regArray.LR = 0;
}

void VirtualMachine::setCodePointerByRomShift(unsigned int shift) {
	printf("Shift to entry: %i\n", shift);
	if (code) {
		regArray.PC = (unsigned int)(code + shift);
	}
}

unsigned int VirtualMachine::getRegister(int num) {
	return ((unsigned int*)&regArray)[num] + (num == 15 ? 4 : 0);
}

void VirtualMachine::setRegister(int num, unsigned int value) {
	((unsigned int*)&regArray)[num] = value;
}

bool VirtualMachine::calcZFlag(unsigned int result) {
	return result == 0 ? true : false;
}

bool VirtualMachine::calcNFlag(int result) {
	return result < 0 ? true : false;
}

bool VirtualMachine::calcCFlag(unsigned int arg1, unsigned int arg2, unsigned char operation) {
	switch (operation) {
	case OPERATION_ADD:
		return (arg1 + arg2 < arg1) ? true : false;

	case OPERATION_SUB:
		return (arg1 - arg2 > arg1) ? false : true;

	default:
		return false;
	}
}

bool VirtualMachine::calcVFlag(int arg1, int arg2, unsigned char operation) {
	switch (operation) {
	case OPERATION_ADD:
		if (arg1 > 0 && arg2 > 0) {
			return (arg1 + arg2 < 0) ? true : false;
		}
		if (arg1 < 0 && arg2 < 0) {
			return (arg1 + arg2 > 0) ? true : false;
		}
		return false;

	case OPERATION_SUB:
		if (arg1 > 0 && -arg2 > 0) {
			return (arg1 + -arg2 < 0) ? false : true;
		}
		if (arg1 < 0 && -arg2 < 0) {
			return (arg1 + -arg2 > 0) ? false : true;
		}
		return false;

	default:
		return false;
	}
}

void VirtualMachine::run() {
	printf("All Reg is 0, code shift: %i\n", regArray.PC - (unsigned int)code);

	unsigned short int cmd;
	unsigned char preCode;
	unsigned char opt;
	unsigned char sp;
	unsigned char src;
	unsigned char dst;
	unsigned char baseReg;
	unsigned char offReg;
	unsigned char srcDstReg;
	unsigned char immidiate;
	unsigned char isHigh;
	unsigned char bit4;
	unsigned char sign;
	unsigned char im;
	unsigned char h1;
	unsigned char h2;
	unsigned char b, l, s, h;
	unsigned char doStore;
	unsigned char rList;
	unsigned char condition;

	char cnst;
	int addr = 0;
	unsigned int uaddr = 0;
	unsigned int jmpAddr = 0;
	int result;
	int regValue;
	char cBit = 0;
	bool conditionSet = false;
	
	Display display;

	flagCF = false;
	flagZF = false;
	flagNF = false;
	flagVF = false;


	while(1) {
		cmd = ((unsigned char *)regArray.PC)[0];
		cmd |= ((unsigned char *)regArray.PC)[1] << 8;

		preCode = cmd >> 13;
		printf("- CMD %04x, %i - %i, R %i %i %i %i %i %i %i %i %i %i %i %i %i, %i %i, %s %s\n", cmd, regArray.PC - (unsigned int)code, preCode,
			regArray.R0, regArray.R1, regArray.R2, regArray.R3, regArray.R4, regArray.R5, regArray.R6, regArray.R7, regArray.R8, regArray.R9, regArray.R10, regArray.R11, regArray.R12,
			regArray.PC, regArray.SP, flagZF ? "SET" : "NOT", flagCF ? "SET" : "NOT"
		);

		if (cmd == 0) {
			regArray.PC += 2;
			continue;
		}

		switch (preCode) {
		case 0:
			printf("MOV/ADD/SUBSTRACT\n");
			opt = (cmd >> 11) & 0x03;
			src = (cmd >> 3) & 0x07;
			dst = cmd & 0x07;

			if (opt == 3) {
				im = (cmd >> 10) & 1;
				opt = (cmd >> 9) & 1;
				immidiate = (cmd >> 6) & 0x07;
				printf("ADD/SUBSTRUCT, im flag: %i, op: %i, num: %i, src: %i, dst %i\n", im, opt, immidiate, src, dst);

				if (!im && !opt) {
					flagCF = calcCFlag(getRegister(src), getRegister(immidiate), OPERATION_ADD);
					flagVF = calcVFlag(getRegister(src), getRegister(immidiate), OPERATION_ADD);
					setRegister(dst, getRegister(src) + getRegister(immidiate));
				}

				if (im && !opt) {
					flagCF = calcCFlag(getRegister(src), immidiate, OPERATION_ADD);
					flagVF = calcVFlag(getRegister(src), immidiate, OPERATION_ADD);
					setRegister(dst, getRegister(src) + immidiate);
				}

				if (!im && opt) {
					flagCF = calcCFlag(getRegister(src), getRegister(immidiate), OPERATION_SUB);
					flagVF = calcVFlag(getRegister(src), getRegister(immidiate), OPERATION_SUB);
					setRegister(dst, getRegister(src) - getRegister(immidiate));
				}

				if (!im && opt) {
					flagCF = calcCFlag(getRegister(src), immidiate, OPERATION_SUB);
					flagVF = calcVFlag(getRegister(src), immidiate, OPERATION_SUB);
					setRegister(dst, getRegister(src) - immidiate);
				}
				flagZF = calcZFlag(getRegister(dst));
				flagNF = calcNFlag(getRegister(dst));
			}
			else {				
				immidiate = (cmd >> 6) & 0x1f;
				printf("MOV/SHIFT %i %i %i %i\n", immidiate, opt, src, dst);
				if (opt == 0) {
					flagCF = calcCFlag(getRegister(src), immidiate, OPERATION_SHIFT_LEFT);
					flagVF = calcVFlag(getRegister(src), immidiate, OPERATION_SHIFT_LEFT);
					setRegister(dst, getRegister(src) << immidiate);
				}

				if (opt == 1) {
					flagCF = calcCFlag(getRegister(src), immidiate, OPERATION_SHIFT_RIGHT);
					flagVF = calcVFlag(getRegister(src), immidiate, OPERATION_SHIFT_RIGHT);
					setRegister(dst, getRegister(src) >> immidiate);
				}

				if (opt == 2) {
					flagCF = calcCFlag(getRegister(src), immidiate, OPERATION_SHIFT_RIGHT);
					flagVF = calcVFlag(getRegister(src), immidiate, OPERATION_SHIFT_RIGHT);
					setRegister(dst, ((int)getRegister(src)) >> immidiate);
				}
				flagZF = calcZFlag(getRegister(dst));
				flagNF = calcNFlag(getRegister(dst));
			}
			break;

		case 1:
			opt = (cmd >> 11) & 0x03;
			dst = (cmd >> 8) & 0x07;
			immidiate = cmd & 0xff;
			printf("move, sub, cmp %i %i %i\n", opt, dst, immidiate);
			if (opt == 0) {
				printf("move1 - todo - is signed or unsigned %i %i\n", dst, immidiate);
				setRegister(dst, immidiate);
				flagZF = calcZFlag(dst);
				flagNF = calcNFlag(dst);
				flagCF = false;
				flagVF = false;
			}
			if (opt == 1) {
				printf("cmp %i %i\n", getRegister(dst), immidiate);
				flagZF = calcZFlag(getRegister(dst) - immidiate);
				flagNF = calcNFlag(getRegister(dst) - immidiate);
				flagCF = calcCFlag(getRegister(dst), immidiate, OPERATION_SUB);
				flagVF = calcVFlag(getRegister(dst), immidiate, OPERATION_SUB);
				printf("FLAGS %i %i %i %i\n", flagZF, flagNF, flagCF, flagVF);
			}
			if (opt == 2) {
				flagCF = calcCFlag(getRegister(dst), immidiate, OPERATION_ADD);
				flagVF = calcVFlag(getRegister(dst), immidiate, OPERATION_ADD);
				setRegister(dst, getRegister(dst) + immidiate);
				flagZF = calcZFlag(getRegister(dst));
				flagNF = calcNFlag(getRegister(dst));
			}
			if (opt == 3) {
				flagCF = calcCFlag(getRegister(dst), immidiate, OPERATION_ADD);
				flagVF = calcVFlag(getRegister(dst), immidiate, OPERATION_ADD);
				setRegister(dst, getRegister(dst) - immidiate);
				flagZF = calcZFlag(getRegister(dst));
				flagNF = calcNFlag(getRegister(dst));
			}
			break;

		case 2:
			//printf("ALU\n");
			opt = (cmd >> 12) & 1;
			if (opt) {
				opt = (cmd >> 9) & 1;
				if (opt) {
					immidiate = (cmd >> 6) & 0x07;
					src = (cmd >> 3) & 0x07;
					dst = cmd & 0x07;
					h = (cmd >> 11) & 0x01;
					s = (cmd >> 10) & 0x01;

					printf("CASE2 - full1, %i %i %i %i %i\n", immidiate, src, dst, h, s);

					if (s == 0 && h == 0) {
						*((unsigned short int*)(getRegister(src) + immidiate)) = getRegister(dst) & 0xffff;
					}
					if (s == 0 && h == 1) {
						setRegister(dst, *((unsigned short int*)(getRegister(src) + immidiate)));
					}
					if (s == 1 && h == 0) {
						result = *((unsigned char*)(getRegister(src) + immidiate));
						if (result & 0x80) {
							result |= 0xffffff00;
						}
						setRegister(dst, result);
					}
					if (s == 1 && h == 1) {
						result = *((unsigned short int*)(getRegister(src) + immidiate));
						if (result & 0x8000) {
							result |= 0xffff0000;
						}
						setRegister(dst, result);
					}
				}
				else {
					offReg = (cmd >> 6) & 0x07;
					baseReg = (cmd >> 3) & 0x07;
					srcDstReg = cmd & 0x07;
					l = (cmd >> 11) & 0x01;
					b = (cmd >> 10) & 0x01;

					printf("Load/store with register offset %i %i %i %i %i\n", l, b, offReg, baseReg, srcDstReg);

					if (l == 0 && b == 0) {
						*((unsigned int*)( getRegister(offReg) + getRegister(baseReg) )) = getRegister(srcDstReg);
					}
					if (l == 0 && b == 1) {
						*((unsigned char*)(getRegister(offReg) + getRegister(baseReg) )) = getRegister(srcDstReg) & 0xff;
					}
					if (l == 1 && b == 0) {
						setRegister(srcDstReg, *((unsigned int*)(getRegister(offReg) + getRegister(baseReg)) ));
					}
					if (l == 1 && b == 1) {
						setRegister(srcDstReg, *((unsigned char*)(getRegister(offReg) + getRegister(baseReg))));
					}
				}
			}
			else {
				opt = (cmd >> 11) & 1;
				if (opt) {
					dst = (cmd >> 8) & 0x07;
					uaddr = (((cmd & 0xff) << 2)) + (regArray.PC & 0xfffffffc) + 4;
					result = *((unsigned int*)uaddr);
					printf("PC relative load %i %i %i %i %i\n", dst, regArray.PC, uaddr, result, ((cmd & 0xff) << 2));
					setRegister(dst, result);
				}
				else {
					src = (cmd >> 3) & 0x07;
					dst = cmd & 0x07;
					opt = (cmd >> 10) & 1;
					if (opt) {
						opt = (cmd >> 8) & 0x03;
						h1 = (cmd >> 7) & 0x01;
						h2 = (cmd >> 6) & 0x01;

						if (opt == 0) {
							printf("opt0 - %i %i, %i %i\n", h1, h2, src, dst);
							if (h1 == 0 && h2 == 1) {
								setRegister(dst, getRegister(dst) + getRegister(src + 8));
							}

							if (h1 == 1 && h2 == 0) {
								setRegister(dst + 8, getRegister(src) + getRegister(dst + 8));
							}

							if (h1 == 1 && h2 == 1) {
								setRegister(dst + 8, getRegister(src + 8) + getRegister(dst + 8));
							}
						} 

						if (opt == 1) {
							printf("opt1 - %i %i, %i %i\n", h1, h2, src, dst);
							if (h1 == 0 && h2 == 1) {
								flagZF = getRegister(dst) == getRegister(src + 8);
							}

							if (h1 == 1 && h2 == 0) {
								flagZF = getRegister(dst + 8) == getRegister(src);
							}

							if (h1 == 1 && h2 == 1) {
								flagZF = getRegister(dst + 8) == getRegister(src + 8);
							}
						}

						if (opt == 2) {
							printf("opt2 - %i %i, %i %i\n", h1, h2, src, dst);
							if (h1 == 0 && h2 == 1) {
								((unsigned int*)&regArray)[dst] = ((unsigned int*)&regArray)[src + 8];
							}

							if (h1 == 1 && h2 == 0) {
								((unsigned int*)&regArray)[dst + 8] = ((unsigned int*)&regArray)[src];
							}

							if (h1 == 1 && h2 == 1) {
								((unsigned int*)&regArray)[dst + 8] = ((unsigned int*)&regArray)[src + 8];
							}
						}

						if (opt == 3) {
							printf("Branch reg jump %i %i %i %i %i\n", src, ((unsigned int*)&regArray)[src], ((unsigned int*)&regArray)[src + 8], ((unsigned int*)&regArray)[src], h2);
							if (h2) {
								regArray.PC = ((unsigned int*)&regArray)[src + 8];
							}
							else {
								regArray.PC = ((unsigned int*)&regArray)[src];
							}
							if (regArray.PC == 0) {
								printf("Program ended\n");
								return;
							}
							continue;
						}
					}
					else {
						opt = (cmd >> 6) & 0x0f;
						
						flagVF = false;
						flagCF = false;
						if (opt == 0) {
							setRegister(dst, getRegister(dst) & getRegister(src));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
						if (opt == 1) {
							setRegister(dst, getRegister(dst) ^ getRegister(src));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
						if (opt == 2) {
							setRegister(dst, getRegister(dst) << getRegister(src));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
						if (opt == 3) {
							setRegister(dst, getRegister(dst) >> getRegister(src));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
						if (opt == 4) {
							setRegister(dst, ((int)getRegister(dst)) >> getRegister(src));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
						if (opt == 5) {
							flagVF = calcVFlag(getRegister(dst), getRegister(src) + (flagCF ? 1 : 0), OPERATION_ADD);
							flagCF = calcCFlag(getRegister(dst), getRegister(src) + (flagCF ? 1 : 0), OPERATION_ADD);
							setRegister(dst, getRegister(dst) + getRegister(src) + (flagCF ? 1 : 0));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
						if (opt == 6) {
							flagVF = calcVFlag(getRegister(dst), getRegister(src) - (flagCF ? 0 : 1), OPERATION_SUB);
							flagCF = calcCFlag(getRegister(dst), getRegister(src) - (flagCF ? 0 : 1), OPERATION_SUB);
							setRegister(dst, getRegister(dst) - getRegister(src) - (flagCF ? 0 : 1));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
						if (opt == 7) {
							printf("not implemented x123\n");
							return;
						}
						if (opt == 8) {
							flagVF = calcVFlag(getRegister(dst), getRegister(src), OPERATION_AND);
							flagCF = calcCFlag(getRegister(dst), getRegister(src), OPERATION_AND);
							flagZF = calcZFlag(getRegister(dst) & getRegister(src));
							flagNF = calcNFlag(getRegister(dst) & getRegister(src));
						}
						if (opt == 9) {
							setRegister(dst, -getRegister(dst));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
						if (opt == 10) {
							flagVF = calcVFlag(getRegister(dst), getRegister(src), OPERATION_SUB);
							flagCF = calcCFlag(getRegister(dst), getRegister(src), OPERATION_SUB);
							flagZF = calcZFlag(getRegister(dst) - getRegister(src));
							flagNF = calcNFlag(getRegister(dst) - getRegister(src));
						}
						if (opt == 11) {
							flagVF = calcVFlag(getRegister(dst), getRegister(src), OPERATION_ADD);
							flagCF = calcCFlag(getRegister(dst), getRegister(src), OPERATION_ADD);
							flagZF = calcZFlag(getRegister(dst) + getRegister(src));
							flagNF = calcNFlag(getRegister(dst) + getRegister(src));
						}
						if (opt == 12) {
							setRegister(dst, getRegister(dst) | getRegister(src));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
						if (opt == 13) {
							setRegister(dst, getRegister(dst) * getRegister(src));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
						if (opt == 14) {
							setRegister(dst, getRegister(dst) & ~getRegister(src));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
						if (opt == 15) {
							setRegister(dst, ~getRegister(src));
							flagZF = calcZFlag(dst);
							flagNF = calcNFlag(dst);
						}
					}
				}
			}
			break;

		case 3:
			b = (cmd >> 12) & 0x01;
			l = (cmd >> 11) & 0x01;
			immidiate = ((cmd >> 6) & 0x1f);
			baseReg = (cmd >> 3) & 0x07;
			dst = cmd & 0x07;
			uaddr = (getRegister(baseReg) + (immidiate << 2)) & 0xfffffffe;

			printf("case 3 - %i %i %i %i %i %i\n", b, l, immidiate, baseReg, dst, uaddr);

			if (l == 0 && b == 0) {
				*((unsigned int*)uaddr) = getRegister(dst);
			}

			if (l == 1 && b == 0) {
				setRegister(dst, *((unsigned int*)uaddr));
			}

			if (l == 0 && b == 1) {
				*((unsigned char*)uaddr) = getRegister(dst) & 0xff;
			}

			if (l == 1 && b == 1) {
				setRegister(dst, *((unsigned char*)uaddr));
			}
			break;

		case 4:
			opt = (cmd >> 12) & 0x01;
			if (opt) {
				l = (cmd >> 11) & 0x01;
				dst = (cmd >> 8) & 0x07;
				immidiate = cmd & 0xff;
				uaddr = (regArray.SP + (immidiate << 2)) & 0xfffffffe;

				if (l) {
					setRegister(dst, *((unsigned int*)uaddr));
					printf("SP relative LOAD %i %i %i %i %i\n", dst, immidiate, uaddr, getRegister(dst));
				}
				else {
					printf("SP relative STORE %i %i %i\n", dst, immidiate, uaddr);
					*((unsigned int*)uaddr) = getRegister(dst);
				}
			}
			else {
				l = (cmd >> 11) & 0x01;
				immidiate = ((cmd >> 6) & 0x1f) << 1;
				baseReg = (cmd >> 3) & 0x07;
				dst = cmd & 0x07;
				uaddr = (baseReg + immidiate) & 0xfffffffe;

				if (l) {
					*((unsigned short int *)uaddr) = getRegister(dst) & 0xffff;
				}
				else {
					setRegister(dst, *((unsigned short int *)uaddr));
				}
			}
			break;

		case 5:
			opt = (cmd >> 12) & 0x01;
			if (opt) {
				opt = (cmd >> 10) & 0x01;
				if (opt) {
					opt = (cmd >> 11) & 0x01;
					doStore = (cmd >> 8) & 0x01;
					rList = cmd & 0xff;

					printf("push/pop register %04X\n", rList);

					if (opt) {
						//load from memory
						for (int i = 0; i < 8; i++) {
							if ((1 << i) & rList) {
								setRegister(i, *((unsigned int *)regArray.SP));
								regArray.SP += 4;
								printf("LOAD REG %i %i\n", i, getRegister(i));
							}
						}
						if (doStore) {
							regArray.PC = (*((unsigned int *)regArray.SP));
							regArray.SP += 4;
							printf("LOAD REG PC %i\n", regArray.PC);
						}
					}
					else {
						//store to memory
						if (doStore) {
							regArray.SP -= 4;
							*((unsigned int *)regArray.SP) = regArray.LR;
							printf("STORE REG LR %i\n", regArray.LR);
						}
						for (int i = 0; i < 8; i++) {
							if ((1 << (7 - i)) & rList) {
								regArray.SP -= 4;
								*((unsigned int *)regArray.SP) = getRegister(7 - i);
								printf("STORE REG %i %i\n", 7 - i, *((unsigned int *)regArray.SP));
							}
						}
					}
				}
				else {
					sign = (cmd >> 7) & 0x01;
					opt = (cmd & 0x7f) << 1;

					if (sign) {
						regArray.SP -= opt;
					} else {
						regArray.SP += opt;
					}
				}
			}
			else {
				sp = (cmd >> 11) & 1;
				dst = (cmd >> 8) & 0x07;
				immidiate = cmd & 0xff;
				uaddr = ((unsigned int)immidiate) << 2;
				printf("Load address, sp %i, dst %i, addr %i\n", sp, dst, uaddr);

				if (sp) {
					setRegister(dst, regArray.SP + uaddr);
				}
				else {
					setRegister(dst, regArray.PC + uaddr + 4);
				}
			}
			break;

		case 6:
			opt = (cmd >> 12) & 0x01;
			
			if (opt) {
				condition = (cmd >> 8) & 0x0f;
				addr = ((int)((char)(cmd & 0xff))) << 1;
				printf("case 6 opt 1 cond %i %i\n", condition, addr);
				switch (condition) {
				case 0:
					conditionSet = flagZF == true;
					break;

				case 1:
					conditionSet = flagZF == false;
					break;

				case 2:
					conditionSet = flagCF == true;
					break;

				case 3:
					conditionSet = flagCF == false;
					break;

				case 4:
					conditionSet = flagNF == true;
					break;

				case 5:
					conditionSet = flagNF == false;
					break;

				case 6:
					conditionSet = flagVF == true;
					break;

				case 7:
					conditionSet = flagVF == false;
					break;

				case 8:
					conditionSet = (flagCF == true && flagZF == false);
					break;

				case 9:
					conditionSet = (flagCF == false || flagZF == true);
					break;

				case 10:
					conditionSet = flagNF == flagVF;
					break;

				case 11:
					conditionSet = (flagZF != flagNF);
					break;

				case 12:
					conditionSet = (flagZF == false && flagNF == flagVF);
					break;

				default:
					printf("not implemented condition %i\n", condition);
					return;
				}

				if (conditionSet) {
					printf("CONDITIONAL JUMP %i\n", addr + 4);
					regArray.PC += (addr + 4);
					continue;
				}
				else {
					printf("NO JUMP\n");
				}
			}
			else {
				l = (cmd >> 11) & 1;
				baseReg = (cmd >> 8) & 0x07;
				rList = cmd & 0xff;
				uaddr = getRegister(baseReg);

				printf("MULTIPLE LOAD/STORE %i %i %i %i\n", l, baseReg, rList, uaddr);

				if (l) {
					for (int i = 0; i < 8; i++) {
						if ((1 << i) & rList) {
							uaddr -= 4;
							setRegister(i, *((unsigned int *)uaddr));
						}
					}
					setRegister(baseReg, uaddr);
				}
				else {
					for (int i = 0; i < 8; i++) {
						if ((1 << (7 - i)) & rList) {
							*((unsigned int *)uaddr) = getRegister(7 - i);
							uaddr += 4;
						}
					}
					setRegister(baseReg, uaddr);
				}
			}
			
			break;
			
		case 7:
			bit4 = (cmd >> 12) & 1;
			if (bit4) {
				printf("Full Jump with link\n");
				isHigh = (cmd >> 11) & 1;
				uaddr = cmd & 0x7ff;
				printf("isHigh: %i, addr: %i, jmpAddr: %i\n", isHigh, uaddr, regArray.LR);

				if (isHigh) {
					regArray.LR |= uaddr;
					

					if (regArray.LR < 200) {
						switch (regArray.LR) {
						case CALL_LOG_I:
							printf("LOGi %i\n", regArray.R0);
							break;

						case CALL_LOG_S:
							printf("LOGs %s\n", regArray.R0);
							break;

						case CALL_DISPLAY_VIDEO_MODE:
							printf("Set Video Mode %i\n", regArray.R0);
							display.setVideoMode(regArray.R0);
							break;

						case CALL_DISPLAY_SPRITE:
							display.drawSprite((unsigned char*)regArray.R0, regArray.R1, regArray.R2, regArray.R3, ((unsigned int*)regArray.SP)[0]);
							break;

						case CALL_DISPLAY_SPRITE_ARRAY:
							display.drawSpriteArray((unsigned char*)regArray.R0, regArray.R1, regArray.R2, regArray.R3, ((unsigned int*)regArray.SP)[0], ((unsigned int*)regArray.SP)[1]);
							break;

						case CALL_DISPLAY_SYNC:
							printf("Display sync\n");
							display.sync();
							break;

						case CALL_INPUT_STATE:
							printf("Input State\n");
							regArray.R0 = 0;
							break;

						case CALL_MEMSET:
							printf("Mem Set %i %i %i\n", regArray.R0, regArray.R1, regArray.R2);
							memset((void*)regArray.R0, regArray.R1, regArray.R2);
							break;

						default:
							printf("unknown function %i\n", regArray.LR);
							return;
						}
						regArray.LR = uaddr + 2;
					}
					else {
						uaddr = ((unsigned int)code + ((regArray.LR - 200) << 1));
						printf("jump to %i %i %i\n", uaddr, uaddr - (unsigned int)code, (regArray.LR - 200) << 1);
						addr = regArray.PC + 2;
						regArray.PC = uaddr;
						regArray.LR = addr;
						continue;
					}
				}
				else {
					regArray.LR = uaddr << 11;
				}
			} 
			else {
				addr = (cmd & 0x7ff) << 1;
				if (addr & 0x0800) {
					addr |= 0xfffff000;
				}
				printf("Unconditional Jump on %i\n", addr);
				regArray.PC = regArray.PC + 4 + addr;
				continue;
			}
			break;

		default:
			printf("UNKNOWN PRECODE %i\n", preCode);
			return;
		}

		regArray.PC += 2;
	}
}

