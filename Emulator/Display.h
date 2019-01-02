#pragma once
#include "stdafx.h"

#define MAX_SPRITES 100

class Display
{
public:
	Display();
	~Display();

	void setVideoMode(int mode);
	void drawSprite(unsigned char *data, short int x, short int y, char width, char height);
	void drawSpriteArray(unsigned char *data, unsigned char blockSize, short int x, short int y, char width, char height);
	void sync();
	void render();

private:
	int mode;
	unsigned int lastFrame;
	unsigned char *screen;
	unsigned int spriteCount;
};

