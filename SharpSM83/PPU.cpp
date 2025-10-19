#include "PPU.h"
#include "Memory.h"
#include <cstring>
#include <algorithm>

PPU::PPU(Memory* memory) : memory(memory)
{
	memset(framebuffer, 0, sizeof(framebuffer));
	reset();
}

void PPU::reset()
{
	dots = 0;
	ly = 0;
	frameReady = false;

	fetcherState = FETCH_TILE_NUM;
	fetcherCycles = 0;
	fetcherX = 0;
	lx = 0;

	while (!bgFifo.empty()) bgFifo.pop();
	while (!spriteFifo.empty()) spriteFifo.pop();

	oamBuffer.clear();
	fetchedSprites.clear();

	scxStartOfLine = 0;
	pixelsDiscarded = 0;
	windowLineCounter = 0;
	windowTriggeredThisFrame = false;
}

void PPU::resetForNewLine()
{
	fetcherState = FETCH_TILE_NUM;
	fetcherCycles = 0;
	fetcherX = 0;
	lx = 0;

	while (!bgFifo.empty()) bgFifo.pop();
	while (!spriteFifo.empty()) spriteFifo.pop();

	oamBuffer.clear();
	fetchedSprites.clear();

	scxStartOfLine = memory->read(0xFF43);
	pixelsDiscarded = 0;
}

void PPU::step(int cpuCycles)
{
	uint8_t lcdc = memory->read(0xFF40);

	if (!(lcdc & 0x80))
	{
		// LCD off
		return;
	}

	for (int i = 0; i < cpuCycles; i++)
	{
		tick();
	}
}

void PPU::tick()
{
	uint8_t lcdc = memory->read(0xFF40);
	ly = memory->read(0xFF44);

	if (ly >= 144)
	{
		////////////////////////////////////////// MODE 1
		if (dots == 0)
		{
			memory->write(0xFF0F, memory->read(0xFF0F) | 0x01); // call vblank
			setMode(1);
		}

		dots++;
		if (dots >= 456)
		{
			dots = 0;
			ly++;
			if (ly > 153)
			{
				ly = 0;
				frameReady = true;
				windowTriggeredThisFrame = false;
				windowLineCounter = 0;
			}
			memory->io[0x44] = ly;
			checkLYC();
		}
	}
	else
	{

		if (dots == 0)
		{
			resetForNewLine();
			setMode(2);
		}

		if (dots < 80)
		{//////////////////////// MODE 2 
			oamScan();
		}
		else if (dots == 80)
		{ /////////////// MODE 3 (ENTER)

			setMode(3);

			std::sort(oamBuffer.begin(), oamBuffer.end(),
				[](const OAMEntry& a, const OAMEntry& b) { return a.x < b.x; });
		}
		else
		{
			///////////////////////////////////// MODE 3 (PROPER)
			if (lx < 160)
			{
				modeDrawing();
			}
			else if (lx == 160)
			{
				setMode(0);
				lx++;
			}
		}

		dots++;
		if (dots >= 456)
		{
			dots = 0;
			ly++;
			memory->io[0x44] = ly;
			checkLYC();
		}
	}
}

void PPU::oamScan()
{
	if (dots % 2 != 0) return;

	int spriteIndex = dots / 2;
	if (spriteIndex >= 40) return;

	uint8_t lcdc = memory->read(0xFF40);
	if (!(lcdc & 0x02)) return;

	uint8_t spriteHeight = (lcdc & 0x04) ? 16 : 8;

	uint16_t oamAddr = 0xFE00 + spriteIndex * 4;
	uint8_t spriteY = memory->oam[oamAddr - 0xFE00];
	uint8_t spriteX = memory->oam[oamAddr + 1 - 0xFE00];
	uint8_t tileIndex = memory->oam[oamAddr + 2 - 0xFE00];
	uint8_t attributes = memory->oam[oamAddr + 3 - 0xFE00];

	int16_t sy = spriteY - 16;
	if (ly >= sy && ly < sy + spriteHeight)
	{
		if (oamBuffer.size() < 10)
		{
			OAMEntry entry;
			entry.y = sy;
			entry.x = spriteX - 8;
			entry.tile = tileIndex;
			entry.attributes = attributes;
			entry.oamIndex = spriteIndex;
			oamBuffer.push_back(entry);
		}
	}
}

void PPU::modeDrawing()
{

	checkAndFetchSprites();

	runFetcher();


	tryDrawPixel();
}

void PPU::checkAndFetchSprites()
{
	uint8_t lcdc = memory->read(0xFF40);
	if (!(lcdc & 0x02)) return;

	for (size_t i = 0; i < oamBuffer.size(); i++)
	{
		const OAMEntry& sprite = oamBuffer[i];

		bool alreadyFetched = false;
		for (size_t j = 0; j < fetchedSprites.size(); j++)
		{
			if (fetchedSprites[j] == i)
			{
				alreadyFetched = true;
				break;
			}
		}

		if (!alreadyFetched && sprite.x == lx)
		{
			fetchSprite(sprite);
			fetchedSprites.push_back(i);
		}
	}
}

void PPU::fetchSprite(const OAMEntry& sprite)
{
	uint8_t lcdc = memory->read(0xFF40);
	uint8_t spriteHeight = (lcdc & 0x04) ? 16 : 8;

	uint8_t tileIndex = sprite.tile;
	if (spriteHeight == 16)
	{
		tileIndex &= 0xFE;
	}

	int lineInSprite = ly - sprite.y;


	if (sprite.attributes & 0x40)
	{
		lineInSprite = spriteHeight - 1 - lineInSprite;
	}

	uint16_t tileAddr = 0x8000 + tileIndex * 16 + lineInSprite * 2;
	uint8_t dataLo = memory->vram[tileAddr - 0x8000];
	uint8_t dataHi = memory->vram[tileAddr + 1 - 0x8000];


	SpritePixel pixels[8];
	for (int i = 0; i < 8; i++)
	{
		int bit = (sprite.attributes & 0x20) ? i : 7 - i; // X flip
		uint8_t colorNum = ((dataHi >> bit) & 1) << 1 | ((dataLo >> bit) & 1);

		pixels[i].color = colorNum;
		pixels[i].palette = (sprite.attributes & 0x10) ? 1 : 0;
		pixels[i].bgPriority = (sprite.attributes & 0x80) != 0;
	}


	if (!spriteFifo.empty())
	{
		SpritePixel temp[8];
		for (int i = 0; i < 8; i++)
		{
			if (!spriteFifo.empty())
			{
				temp[i] = spriteFifo.front();
				spriteFifo.pop();
			}
			else
			{
				temp[i].color = 0;
				temp[i].palette = 0;
				temp[i].bgPriority = false;
			}
		}


		for (int i = 0; i < 8; i++)
		{
			if (temp[i].color != 0)
			{
				spriteFifo.push(temp[i]);
			}
			else
			{
				spriteFifo.push(pixels[i]);
			}
		}
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			spriteFifo.push(pixels[i]);
		}
	}
}

void PPU::runFetcher()
{
	fetcherCycles++;

	if (fetcherCycles >= 2)
	{
		fetcherCycles = 0;

		switch (fetcherState)
		{
		case FETCH_TILE_NUM:
		fetchTileNumber();
		fetcherState = FETCH_TILE_DATA_LOW;
		break;

		case FETCH_TILE_DATA_LOW:
		fetchTileDataLow();
		fetcherState = FETCH_TILE_DATA_HIGH;
		break;

		case FETCH_TILE_DATA_HIGH:
		fetchTileDataHigh();
		fetcherState = PUSH;
		break;

		case PUSH:
		if (bgFifo.size() <= 8)
		{
			pushToFifo();
			fetcherX += 8;
			fetcherState = FETCH_TILE_NUM;
		}
		break;
		}
	}
}

void PPU::fetchTileNumber()
{
	uint8_t lcdc = memory->read(0xFF40);
	uint8_t scy = memory->read(0xFF42);

	//check window
	uint8_t wy = memory->read(0xFF4A);
	uint8_t wx = memory->read(0xFF4B);
	bool windowEnabled = (lcdc & 0x20) && ly >= wy && (fetcherX + 7) >= (wx - 7);

	if (windowEnabled && !windowTriggeredThisFrame)
	{
		windowTriggeredThisFrame = true;
	}

	if (windowTriggeredThisFrame && (fetcherX + 7) >= (wx - 7))
	{
		// window fetch
		uint16_t tileMapBase = (lcdc & 0x40) ? 0x9C00 : 0x9800;
		uint8_t windowX = (fetcherX + 7 - (wx - 7)) / 8;
		uint8_t windowY = windowLineCounter / 8;

		uint16_t tileAddr = tileMapBase + windowY * 32 + windowX;
		tileNumberToFetch = memory->vram[tileAddr - 0x8000];
		yInTile = windowLineCounter % 8;
	}
	else
	{
		// bg fetch
		uint16_t tileMapBase = (lcdc & 0x08) ? 0x9C00 : 0x9800;
		uint8_t scrolledX = (scxStartOfLine + fetcherX) % 256;
		uint8_t scrolledY = (scy + ly) % 256;

		uint16_t tileAddr = tileMapBase + (scrolledY / 8) * 32 + (scrolledX / 8);
		tileNumberToFetch = memory->vram[tileAddr - 0x8000];
		yInTile = scrolledY % 8;
	}
}

void PPU::fetchTileDataLow()
{
	uint8_t lcdc = memory->read(0xFF40);

	uint16_t tileDataBase;
	if (lcdc & 0x10)
	{
		tileDataBase = 0x8000 + tileNumberToFetch * 16;
	}
	else
	{
		tileDataBase = 0x9000 + ((int8_t)tileNumberToFetch) * 16;
	}

	tileDataLow = memory->vram[tileDataBase + yInTile * 2 - 0x8000];
}

void PPU::fetchTileDataHigh()
{
	uint8_t lcdc = memory->read(0xFF40);

	uint16_t tileDataBase;
	if (lcdc & 0x10)
	{
		tileDataBase = 0x8000 + tileNumberToFetch * 16;
	}
	else
	{
		tileDataBase = 0x9000 + ((int8_t)tileNumberToFetch) * 16;
	}

	tileDataHigh = memory->vram[tileDataBase + yInTile * 2 + 1 - 0x8000];
}

void PPU::pushToFifo()
{
	for (int bit = 7; bit >= 0; bit--)
	{
		uint8_t colorNum = ((tileDataHigh >> bit) & 1) << 1 | ((tileDataLow >> bit) & 1);
		bgFifo.push(colorNum);
	}
}

void PPU::tryDrawPixel()
{
	if (bgFifo.empty()) return;

	uint8_t fineScroll = scxStartOfLine % 8;
	if (pixelsDiscarded < fineScroll)
	{
		bgFifo.pop();
		if (!spriteFifo.empty()) spriteFifo.pop();
		pixelsDiscarded++;
		return;
	}


	uint8_t bgColor = bgFifo.front();
	bgFifo.pop();

	uint8_t bgp = memory->read(0xFF47);
	uint8_t finalColor = (bgp >> (bgColor * 2)) & 0x03;


	if (!spriteFifo.empty())
	{
		SpritePixel sp = spriteFifo.front();
		spriteFifo.pop();

		if (sp.color != 0)
		{

			if (!sp.bgPriority || bgColor == 0)
			{
				uint8_t palette = memory->read(sp.palette ? 0xFF49 : 0xFF48);
				finalColor = (palette >> (sp.color * 2)) & 0x03;
			}
		}
	}

	framebuffer[ly * 160 + lx] = getColor(finalColor);
	lx++;

	if (windowTriggeredThisFrame && lx == 160)
	{
		windowLineCounter++;
	}
}

void PPU::setMode(uint8_t mode)
{
	static uint8_t prevMode = 0xFF;

	if (mode == prevMode) return;

	uint8_t stat = memory->read(0xFF41);
	stat = (stat & ~0x03) | mode;
	memory->write(0xFF41, stat);

	bool interrupt = false;
	if (mode == 0 && (stat & 0x08)) interrupt = true;
	if (mode == 1 && (stat & 0x10)) interrupt = true;
	if (mode == 2 && (stat & 0x20)) interrupt = true;

	if (interrupt)
	{
		memory->write(0xFF0F, memory->read(0xFF0F) | 0x02);
	}

	prevMode = mode;
}

void PPU::checkLYC()
{
	uint8_t stat = memory->read(0xFF41);
	uint8_t lyc = memory->read(0xFF45);

	bool coincBefore = (stat & 0x04) != 0;
	bool coincNow = (ly == lyc);

	stat = coincNow ? (stat | 0x04) : (stat & ~0x04);
	memory->write(0xFF41, stat);

	if (!coincBefore && coincNow && (stat & 0x40))
	{
		memory->write(0xFF0F, memory->read(0xFF0F) | 0x02);
	}
}

uint32_t PPU::getColor(uint8_t colorId)
{
	switch (colorId)
	{
	case 0: return 0xFFFFFFFF;
	case 1: return 0xAAAAAAAA;
	case 2: return 0x55555555;
	case 3: return 0x00000000;
	}
	return 0xFFFFFFFF;
}