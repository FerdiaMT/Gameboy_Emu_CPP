#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include <queue>
#include <vector>
#include "Memory.h"

struct OAMEntry
{
	int16_t y;
	int16_t x;
	uint8_t tile;
	uint8_t attributes;
	uint8_t oamIndex;
};

struct SpritePixel
{
	uint8_t color;
	uint8_t palette;
	bool bgPriority;
};

class PPU
{
public:
	Memory* memory;
	uint32_t framebuffer[160 * 144];
	bool frameReady;

	PPU(Memory* memory);
	void step(int cpuCycles);
	uint32_t getColor(uint8_t colorId);

private:
	uint16_t dots;
	uint8_t ly;

	enum FetcherState
	{
		FETCH_TILE_NUM,
		FETCH_TILE_DATA_LOW,
		FETCH_TILE_DATA_HIGH,
		PUSH
	};

	FetcherState fetcherState;
	uint8_t fetcherCycles;
	uint8_t fetcherX;
	uint8_t lx;


	uint8_t tileNumberToFetch;
	uint8_t yInTile;
	uint8_t tileDataLow;
	uint8_t tileDataHigh;

	std::queue<uint8_t> bgFifo;
	std::queue<SpritePixel> spriteFifo;


	std::vector<OAMEntry> oamBuffer;
	std::vector<size_t> fetchedSprites;


	uint8_t scxStartOfLine;
	uint8_t pixelsDiscarded;


	uint8_t windowLineCounter;
	bool windowTriggeredThisFrame;


	void reset();
	void resetForNewLine();
	void tick();

	void oamScan();
	void modeDrawing();

	void checkAndFetchSprites();
	void fetchSprite(const OAMEntry& sprite);

	void runFetcher();
	void fetchTileNumber();
	void fetchTileDataLow();
	void fetchTileDataHigh();
	void pushToFifo();

	void tryDrawPixel();

	void setMode(uint8_t mode);
	void checkLYC();
};

#endif