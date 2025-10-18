#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include "Memory.h"
#include "SM83.h"

struct Sprite {
	uint8_t x;
	uint8_t y;
	uint8_t ti;    
	uint8_t attr;  
};


class PPU
{
public:
	

	Memory* memory;
	uint32_t framebuffer[160 * 144];
	int cycles;
	int scnalineCounter;
	bool frameReady;

	PPU(Memory* memory);
	void resetPPU();
	void step(int cycles);

	void renderScreen();

	uint32_t getColor(uint8_t colorId);


private:

	

	void mode2Tick();
	void mode3Tick();
	void mode0Tick();
	void mode1Tick();

	void fetchTileNo();
	void fetchTileL();
	void fetchTileH();

	void fetchWindowTileNo();
	void fetchWindowTileL();
	void fetchWindowTileH();

	void fifoPush();
	void shiftPixel();
	uint8_t mergeWithSprite(uint8_t bgPixel, int pixelsX);
	uint8_t getSpritePixel(int spriteIndex, int spritePixelX);
	uint16_t fetchTileData(uint8_t tileIndex);
	void drawPixel();

	void tileFetcher();

	void updateMode(uint8_t newMode);
	void fetchSpriteTile(const Sprite& sprite);
	void updateLYC();


};
#endif