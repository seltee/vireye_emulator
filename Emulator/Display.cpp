#include "Display.h"
#include <SDL.h>

struct SpriteCash {
	const unsigned char *sprite;
	short int x;
	short int y;
	unsigned char type;
	unsigned char attr;
	unsigned char width;
	unsigned char height;
};

struct SpriteArray {
	const unsigned char *sprites;
	unsigned char *field;
};

#define SPRITE_TYPE_NONE 0
#define SPRITE_TYPE_PALLETE 1
#define SPRITE_TYPE_MASK 2
#define SPRITE_TYPE_ARRAY 3

#define RED             0xff0000ff 
#define GREEN           0xff00ff00
#define BLUE            0xffff0000

const unsigned int colorPallet[] = {
	0xff000000, 0xff000000, 0xffffffff, RED, GREEN, BLUE,
};

Display *display = NULL;

int winMouseX = 0, winMouseY = 0;
bool winMouseClick = false;
bool keysPress[512];
bool keysDown[512];

struct SpriteCash spriteCash[MAX_SPRITES];

SDL_Window* window = NULL;
SDL_Renderer *renderer = NULL;

Display::Display()
{
	if (!display) {
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
			printf("SDL_Init Error: %s\n", SDL_GetError());
		}
		display = this;
	} 
	else{
		printf("display error: second display crieated");
	}
	
	mode = 0;
	spriteCount = 0;
	lastFrame = 0;
	screen = new unsigned char[320 * 240 * 4];
}


Display::~Display()
{
	display = NULL;
}

void Display::setVideoMode(int mode) {
	if (this->mode != mode) {
		if (window) {
			SDL_DestroyWindow(window);
		}

		window = SDL_CreateWindow("Super emulator", 100, 100, 640, 480, SDL_WINDOW_SHOWN);

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		memset(keysPress, 0, 512);
		memset(keysDown, 0, 512);

		sync();
	}
}

void Display::drawSprite(unsigned char *data, short int x, short int y, char width, char height) {
	printf("DISPLAY SPRITE %i, %i %i\n", spriteCount, x, y);
	if (spriteCount < MAX_SPRITES) {
		spriteCash[spriteCount].sprite = data;
		spriteCash[spriteCount].type = SPRITE_TYPE_PALLETE;
		spriteCash[spriteCount].x = x;
		spriteCash[spriteCount].y = y;
		spriteCash[spriteCount].width = width << 1;
		spriteCash[spriteCount].height = height << 1;
		spriteCount++;
	}
}

void Display::drawSpriteArray(unsigned char *data, unsigned char blockSize, short int x, short int y, char width, char height) {
	printf("DISPLAY SPRITE ARRAY %i, %i %i, %i %i, %i\n", spriteCount, x, y, width, height, blockSize);
	for (int i = 0; i < width * height; i++) {
		printf("%i ", ((SpriteArray*)data)->field[i]);
	}
	printf("\n");
	printf("%i %i %i\n\n", data, ((SpriteArray*)data)->field, ((SpriteArray*)data)->sprites);


	if (spriteCount < MAX_SPRITES) {
		spriteCash[spriteCount].sprite = data;
		spriteCash[spriteCount].type = SPRITE_TYPE_ARRAY;
		spriteCash[spriteCount].x = x;
		spriteCash[spriteCount].y = y;
		spriteCash[spriteCount].width = width;
		spriteCash[spriteCount].height = height;
		spriteCash[spriteCount].attr = blockSize;
		spriteCount++;
	}
}

void Display::sync() {
	//render
	if (window && screen) {
		while (SDL_GetTicks() - lastFrame < (1000 / 25));
		lastFrame = SDL_GetTicks();

		for (int c = 0; c < 320 * 240; c++) {
			*((unsigned int*)(screen + (c<<2))) = colorPallet[0];
		}

		for (int i = 0; i < spriteCount; i++) {
			if (spriteCash[i].type == SPRITE_TYPE_PALLETE) {
				for (int iy = 0; iy < spriteCash[i].height; iy++) {
					for (int ix = 0; ix < spriteCash[i].width; ix++) {
						unsigned char color = spriteCash[i].sprite[(ix >> 1) + (iy >> 1) * (spriteCash[i].width >> 1)];
						if (color) {
							int dx = spriteCash[i].x + ix;
							int dy = spriteCash[i].y + iy;
							if (dx >= 0 && dx < 320 && dy >= 0 && dy < 240) {
								*((unsigned int *)&screen[(dx + dy * 320) * 4]) = colorPallet[color];
							}
						}
					}
				}
			}

			if (spriteCash[i].type == SPRITE_TYPE_ARRAY){
				int length = spriteCash[i].width * spriteCash[i].height;
				SpriteArray *sArray = (SpriteArray*)spriteCash[i].sprite;
				const unsigned char *sprites = sArray->sprites;
				unsigned char *field = sArray->field;
				int spriteSideSize = spriteCash[i].attr << 1;
				int spriteSize = spriteCash[i].attr * spriteCash[i].attr;
				
				for (int element = 0; element < length; element++) {
					if (field[element]) {
						const unsigned char *sprite = &sprites[(field[element] - 1)*spriteSize];
						int x = spriteCash[i].x + ((element % spriteCash[i].width) * (spriteSideSize));
						int y = spriteCash[i].y + ((element / spriteCash[i].width) * (spriteSideSize));

						for (int iy = 0; iy < spriteSideSize; iy++) {
							for (int ix = 0; ix < spriteSideSize; ix++) {
								unsigned char color = sprite[(ix >> 1) + (iy >> 1) * (spriteSideSize >> 1)];
								if (color) {
									int dx = x + ix;
									int dy = y + iy;
									if (dx >= 0 && dx < 320 && dy >= 0 && dy < 240) {
										*((unsigned int *)&screen[(dx + dy * 320) * 4]) = colorPallet[color];
									}
								}
							}
						}
					}
				}
			}
			
		}
		spriteCount = 0;
	}

	//messages
	if (window) {
		SDL_Event event;
		memset(keysPress, 0, 512);
		render();

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT) {
				exit(0);
			}
		}
	}
}

void Display::render() {
	if (window) {
		SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(screen, 320, 240, 32, 4*320, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
		SDL_Texture* screenTexture = SDL_CreateTextureFromSurface(renderer, surface);

		SDL_Rect src = { 0, 0, 320, 240 };
		SDL_Rect dst = { 0, 0, 640, 480};

		SDL_RenderCopy(renderer, screenTexture, &src, &dst);
		SDL_RenderPresent(renderer);

		SDL_DestroyTexture(screenTexture);
		SDL_FreeSurface(surface);
	}
}