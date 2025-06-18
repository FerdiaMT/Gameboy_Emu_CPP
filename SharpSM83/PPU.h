#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include "Memory.h"
#include "SM83.h"
class PPU
{
public:
	PPU(Memory& memory);
	void resetPPU();

	void executeTick();

	void updateScreenBuffer(uint8_t(&mainScreen)[160][144]);
	
	void writeIntoSTAT(uint8_t x );

	uint16_t getInternalDot();


private:

	Memory& memory;

	void mode2Tick();
	void mode3Tick();
	void mode0Tick();
	void mode1Tick();

	void fetchTileNo();
	void fetchTileL();
	void fetchTileH();
	void fifoPush();
	void shiftPixel();
	uint8_t mergeWithSprite(uint8_t bgPixel, int pixelsX);
	uint8_t getSpritePixel(int spriteIndex, int spritePixelX);
	uint16_t fetchTileData(uint8_t tileIndex);
	void drawPixel();

	void tileFetcher();

};
#endif