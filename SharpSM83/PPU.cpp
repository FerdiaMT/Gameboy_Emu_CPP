#include "PPU.h"
#include "SM83.h"
#include "memory"
#include <iostream>
#include <queue>
#include <cstring>
#include <bitset>

uint16_t internalDot{};
uint16_t mode3Dots{};


struct SpritePixel {
	uint8_t color;
	bool bgPriority;
	bool transparent;
};

bool mode3Complete = false;
bool spritesSorted = false;
std::queue<uint8_t> backgroundFifo{};
std::queue<SpritePixel> spriteFifo{};
uint8_t screen[160][144];

uint8_t currentLY{};


// resource used -> https://hacktix.github.io/GBEDG/ppu/#oam-scan-mode-2

PPU::PPU(Memory* memory) : memory(memory) //m cycle (using this) = 4 t states
{
	memset(framebuffer, 0, sizeof(framebuffer));
}

uint8_t currentMode{};


//void PPU::updateLYC() {
//	uint8_t stat = memory->read(0xFF41);
//	uint8_t ly = memory->read(0xFF44);
//	uint8_t lyc = memory->read(0xFF45);
//
//	bool coincBefore = (stat & 0x04) != 0;
//	bool coincNow = (ly == lyc);
//
//	// write coincidence flag
//	stat = coincNow ? (stat | 0x04) : (stat & ~0x04);
//	memory->write(0xFF41, stat);
//
//	// edge-trigger STAT IF if enabled
//	if (!coincBefore && coincNow && (stat & 0x40)) {
//		memory->requestInterrupt(0x02);
//	}
//}
//
//void PPU::updateMode(uint8_t newMode) {
//	static uint8_t prevMode = 0xFF;
//	uint8_t stat = memory->read(0xFF41);
//
//	if (newMode != prevMode) {
//		// set mode bits
//		stat = (stat & ~0x03) | newMode;
//		memory->write(0xFF41, stat);
//
//		bool req = false;
//		if (newMode == 0 && (stat & 0x08)) req = true; // HBlank
//		if (newMode == 1 && (stat & 0x10)) req = true; // VBlank
//		if (newMode == 2 && (stat & 0x20)) req = true; // OAM
//
//		if (req) {
//			memory->requestInterrupt(0x02); // STAT IF
//		}
//		prevMode = newMode;
//	}
//}
//
//
//void initFrame() {
//	mode3Dots = 0;
//	mode3Complete = false;
//}
//
//bool mode2Half = 0;
//
//uint8_t searchX = 0; // up to168
//uint8_t searchY = 0; // up to 160
//uint16_t searchAddr = 0xFE00;
//
//uint8_t curItemsOnScanline = 0;
//
//
//std::vector<Sprite> scanlineQueue{};
//
//void PPU::mode2Tick()  // TODO : DOUBLE TALL SPRITE 
//{
//	if (!mode2Half)
//	{
//
//		// for the oam
//	}
//	else
//	{
//		uint8_t oamY = memory->read(searchAddr);
//
//		if (currentLY >= oamY-16  && currentLY < oamY-16+8 && curItemsOnScanline <=10)
//		{
//			
//			curItemsOnScanline++;
//			Sprite sprite;
//			sprite.y = oamY; // this should be fine,   // y
//			sprite.x = memory->read(searchAddr + 1); // x 
//			sprite.ti = memory->read(searchAddr + 2); // tile index
//			sprite.attr = memory->read(searchAddr + 3); // attributes
//
//			//std::cout << "added at ti: x " <<(int)sprite.x << " y : " << (int) sprite.y << " "<< (int)sprite.ti << std::endl;
//			scanlineQueue.push_back(sprite);
//
//			//4 bytes  , BYTE  = 8 bit
//		}
//		
//
//		searchAddr += 0x04; 
//	}
//	mode2Half = !mode2Half;
//
//}
//
//uint8_t fetcherX{}; 
//
//uint8_t screenX{};
//uint8_t screenY{};
//
//uint16_t memAddr{};
//uint16_t tileNumber{};
//
//uint8_t bgx{};
//uint8_t bgy{};
//
//uint16_t mode3Penalty = 12;
//
//
//bool renderingWindow = false;
//uint16_t windowY{};
//uint16_t windowLineCounter = 0;
//bool windowTriggered = false;
//
//
//void PPU::fetchTileNo()
//{
//	if ((memory->ioFetchLCDC() & 0b1000))
//	{
//		memAddr = 0x9C00;
//	}
//	else
//	{
//		memAddr = 0x9800;
//	}
//
//	bgx = (memory->ioFetchSCX() + fetcherX) % 256;
//	bgy = (memory->ioFetchSCY() + currentLY) % 256;
//	 
//	tileNumber = (memory->read(memAddr + (bgx / 8) + (bgy / 8) *32));
//}
//
//uint8_t tileDataA{};
//uint8_t tileDataB{};
//
//void PPU:: fetchTileL()
//{
//
//	uint8_t fineY = (bgy) % 8;
//
//
//	if ((memory->ioFetchLCDC() & 0x10))
//	{
//		memAddr = 0x8000;
//		tileDataA = memory->read( memAddr + (tileNumber * 0x10) + (fineY *2) );
//	}
//	else
//	{ //8800 mode
//		memAddr = 0x9000;
//		tileDataA = memory->read(memAddr + ((int8_t)tileNumber*16) + fineY * 2);
//
//	}
//}
//
//void PPU::fetchTileH()
//{
//
//	uint8_t fineY = (bgy) % 8;
//
//	if ((memory->ioFetchLCDC() & 0x10))
//	{
//		memAddr = 0x8000;
//		tileDataB = memory->read(1+(memAddr + (tileNumber * 0x10) + (fineY * 2)));
//
//
//	}
//	else
//	{ //8800 mode
//		memAddr = 0x9000;
//		tileDataB = memory->read(1+ (memAddr + ((int8_t)tileNumber * 16) + fineY * 2));
//	}
//}
//
//
//
//
//void PPU::fetchWindowTileNo()
//{
//	if ((memory->ioFetchLCDC() & 0b01000000))
//	{
//		if ((memory->ioFetchLCDC() & 0b1000))
//		{
//			memAddr = 0x9C00;
//		}
//		else
//		{
//			memAddr = 0x9800;
//		}
//
//		uint8_t winx = memory->ioFetchWX() - 7;
//		uint8_t winy = memory->ioFetchWY();
//
//		if (currentLY >= winy)
//		{
//			windowY = currentLY - winy;
//			tileNumber = (memory->read(memAddr + ((fetcherX - (winx)) / 8) + (windowY / 8) * 32));
//			renderingWindow = true;
//		}
//	}
//}
//
//void PPU::fetchWindowTileL()
//{
//	if (!renderingWindow) return;
//
//	uint8_t fineY = windowY % 8;
//
//	if (memory->ioFetchLCDC() & 0x10)
//	{
//		memAddr = 0x8000;
//		tileDataA = memory->read(memAddr + (tileNumber * 0x10) + (fineY * 2));
//	}
//	else
//	{ //8800 mode
//		memAddr = 0x9000;
//		tileDataA = memory->read(memAddr + ((int8_t)tileNumber * 16) + fineY * 2);
//	}
//}
//
//void PPU::fetchWindowTileH()
//{
//	if (!renderingWindow) return;
//
//	uint8_t fineY = windowY % 8;
//
//	if (memory->ioFetchLCDC() & 0x10)
//	{
//		memAddr = 0x8000;
//		tileDataB = memory->read(1 + (memAddr + (tileNumber * 0x10) + (fineY * 2)));
//	}
//	else
//	{ //8800 mode
//		memAddr = 0x9000;
//		tileDataB = memory->read(1 + (memAddr + ((int8_t)tileNumber * 16) + fineY * 2));
//	}
//}
//
//
//bool fifoCanPush = false;
//
//bool tileFetcherFinished = false;
//bool pixelOutputMode = false;
//
//
//void PPU::fetchSpriteTile(const Sprite& sprite)
//{
//	uint8_t spriteHeight = (memory->ioFetchLCDC() & 0x04) ? 16 : 8;
//
//	int16_t spriteY = currentLY - (sprite.y - 16);
//	if (spriteY < 0 || spriteY >= spriteHeight) return;
//
//	uint16_t tileIndex = sprite.ti;
//	if (spriteHeight == 16) 
//	{
//		tileIndex &= 0xFE; 
//	}
//
//	if (sprite.attr & 0x40) 
//	{
//		spriteY = spriteHeight - 1 - spriteY;
//	}
//
//	uint16_t addr = 0x8000 + (tileIndex * 16) + spriteY * 2;
//	uint8_t dataLo = memory->read(addr);
//	uint8_t dataHi = memory->read(addr + 1);
//
//	for (int i = 7; i >= 0; i--)
//	{
//		int pixelBit = (sprite.attr & 0x20) ? 7 - i : i;
//		uint8_t color = ((dataHi >> pixelBit) & 1) << 1 | ((dataLo >> pixelBit) & 1);
//
//		SpritePixel spx;
//		spx.color = color;
//		spx.bgPriority = sprite.attr & 0x80;
//		spx.transparent = (color == 0);
//
//		spriteFifo.push(spx);
//	}
//}
//
//
//void PPU::fifoPush() {
//
//	for (int x = 7; x >= 0; x--) 
//	{
//		backgroundFifo.push(((tileDataB >> x) & 0b1) << 1 | ((tileDataA >> x) & 0b1));
//	}
//	tileFetcherFinished = true;
//}
//
//uint8_t twelveDot{};
//uint8_t fetcherState{};
//
//
//
//void PPU::tileFetcher()
//{
//	if (fetcherX < 160) 
//	{
//		uint8_t winx = memory->ioFetchWX() - 7;
//		bool windowActive = (memory->ioFetchLCDC() & 0b01000000) &&(memory->ioFetchWY() <= currentLY) &&(fetcherX >= winx) && (winx <= 166); 
//
//		if (windowActive && !windowTriggered)
//		{
//			windowTriggered = true;
//			windowLineCounter++;
//			fetcherX = 0; 
//		}
//
//		if (fetcherX % 8 == 0)
//		{
//			if (windowTriggered)
//			{
//				fetchWindowTileNo();
//				fetchWindowTileL();
//				fetchWindowTileH();
//			}
//			else
//			{
//				fetchTileNo();
//				fetchTileL();
//				fetchTileH();
//			}
//			fifoPush();
//
//
//			fifoCanPush = true;
//		}
//
//
//		for (const auto& sprite : scanlineQueue)
//		{
//			//std::cout << "sprite checked"<<std::endl;
//			if (sprite.x - 8 == fetcherX)
//
//			{
//				fetchSpriteTile(sprite);
//			}
//		}
//
//		fetcherX++;
//	}
//	else 
//	{
//		mode3Complete = true;
//		fifoCanPush = false;
//		mode3Dots -= 1;
//	}
//}
//
//
//
//uint8_t internalX{};
//
//void PPU::drawPixel()
//{
//	if (backgroundFifo.empty())
//		return;
//
//	uint8_t bgColor = backgroundFifo.front();
//
//
//	uint8_t BGP = memory->read(0xff47);
//	uint8_t finalColor = (BGP >> (bgColor * 2)) & 0x03;
//
//	//finalColor = (internalX / 8 + currentLY / 8) % 4; // THIS IS DEBUG, RMEOVE LATER, DEBUG
//
//
//
//
//	if (!spriteFifo.empty()) {
//		SpritePixel spx = spriteFifo.front();
//		finalColor = spx.color;
//		spriteFifo.pop();
//	}
//
//	backgroundFifo.pop();
//	screen[internalX][currentLY] = finalColor;
//	internalX++;
//}
//
//
//
//void PPU::mode3Tick() 
//{
//
//	if (!spritesSorted)
//	{
//		std::sort(scanlineQueue.begin(), scanlineQueue.end(), [](const Sprite& a, const Sprite& b) 
//		{
//			return a.x < b.x;
//		});
//
//		spritesSorted = true;
//	}
//
//	tileFetcher();
//	//spriteFetcher();
//	if (!backgroundFifo.empty())
//	{
//		drawPixel();
//	}
//}


//void resetVals()
//{
//	twelveDot = 0;
//	mode3Penalty = 12;
//}

//bool enteredModeOne = true;

void PPU::step(int cpuCycles) {

	cycles += cpuCycles;

	if (cycles >=456){
		uint8_t ly = memory->read(0xFF44);
		uint8_t lcdc = memory->read(0xFF40);

		if (!frameReady && (lcdc & 0x81) == 0x81 && ly < 144) {
			renderScreen();
			frameReady = true;
		}

		ly++;

		if (ly == 144)
		{
			memory->write(0xFF0F, memory->read(0xFF0F) | 0x01); // write to IF for vblank
		}
		else if (ly > 153)
		{
			ly = 0;
			frameReady = false;
		}

		memory->io[0x44] = ly;


		if (lcdc & 0x80) {
			uint8_t lyc = memory->read(0xFF45);
			uint8_t stat = memory->read(0xFF41);
			if (ly == lyc) {
				stat |= 0x04;
				if (stat & 0x40) {
					uint8_t IF = memory->read(0xFF0F);
					memory->write(0xFF0F, IF | 0x02);
				}
			}
			else {
				stat &= ~0x04;
			}
			memory->write(0xFF41, stat);
		}
	}
}

void PPU::renderScreen() {
	uint8_t lcdc = memory->read(0xFF40);

	
	if (!(lcdc & 0x80) || !(lcdc & 0x01)) 
	{ 
		return;  // if lcd and  or bg isnt enabled, return
	}

	uint8_t scy = memory->read(0xFF42);
	uint8_t scx = memory->read(0xFF43);
	uint8_t wy = memory->read(0xFF4A);
	uint8_t wx = memory->read(0xFF4B);
	uint8_t bgp = memory->read(0xFF47);

	for (int y = 0; y < 144; y++) {
		for (int x = 0; x < 160; x++) {
			uint32_t color = getColor(0);

			// Background
			if (lcdc & 0x01) {
				uint16_t tileMapBase = (lcdc & 0x08) ? 0x9C00 : 0x9800;
				uint16_t tileDataBase = (lcdc & 0x10) ? 0x8000 : 0x8800;
				bool unsig = (lcdc & 0x10);

				uint8_t pixelY = y + scy;
				uint8_t pixelX = x + scx;

				uint16_t tileRow = pixelY / 8;
				uint16_t tileCol = pixelX / 8;
				uint16_t tileAddr = tileMapBase + (tileRow * 32) + tileCol;

				uint8_t tileNum = memory->vram[tileAddr - 0x8000];

				uint16_t tileLocation;
				if (unsig) {
					tileLocation = tileDataBase + (tileNum * 16);
				}
				else {
					int8_t signedTileNum = (int8_t)tileNum;
					tileLocation = 0x9000 + (signedTileNum * 16);
				}

				uint8_t line = (pixelY % 8) * 2;
				uint8_t data1 = memory->vram[tileLocation + line - 0x8000];
				uint8_t data2 = memory->vram[tileLocation + line + 1 - 0x8000];

				uint8_t colorBit = 7 - (pixelX % 8);
				uint8_t colorNum = ((data2 >> colorBit) & 1) << 1 | ((data1 >> colorBit) & 1);

				uint8_t paletteColor = (bgp >> (colorNum * 2)) & 0x03;
				color = getColor(paletteColor);
			}

			// Window
			if ((lcdc & 0x20) && wy <= y && wx <= x + 7) {
				uint16_t tileMapBase = (lcdc & 0x40) ? 0x9C00 : 0x9800;
				uint16_t tileDataBase = (lcdc & 0x10) ? 0x8000 : 0x8800;
				bool unsig = (lcdc & 0x10);

				uint8_t pixelY = y - wy;
				uint8_t pixelX = x + 7 - wx;

				uint16_t tileRow = pixelY / 8;
				uint16_t tileCol = pixelX / 8;
				uint16_t tileAddr = tileMapBase + (tileRow * 32) + tileCol;

				uint8_t tileNum = memory->vram[tileAddr - 0x8000];

				uint16_t tileLocation;
				if (unsig) {
					tileLocation = tileDataBase + (tileNum * 16);
				}
				else {
					int8_t signedTileNum = (int8_t)tileNum;
					tileLocation = 0x9000 + (signedTileNum * 16);
				}

				uint8_t line = (pixelY % 8) * 2;
				uint8_t data1 = memory->vram[tileLocation + line - 0x8000];
				uint8_t data2 = memory->vram[tileLocation + line + 1 - 0x8000];

				uint8_t colorBit = 7 - (pixelX % 8);
				uint8_t colorNum = ((data2 >> colorBit) & 1) << 1 | ((data1 >> colorBit) & 1);

				uint8_t paletteColor = (bgp >> (colorNum * 2)) & 0x03;
				color = getColor(paletteColor);
			}

			// Sprites
			if (lcdc & 0x02) {
				uint8_t spriteHeight = (lcdc & 0x04) ? 16 : 8;
				uint8_t obp0 = memory->read(0xFF48);
				uint8_t obp1 = memory->read(0xFF49);

				for (int sprite = 39; sprite >= 0; sprite--) {
					uint8_t index = sprite * 4;
					int16_t spriteY = memory->oam[index] - 16;
					int16_t spriteX = memory->oam[index + 1] - 8;
					uint8_t tileNum = memory->oam[index + 2];
					uint8_t attributes = memory->oam[index + 3];

					if (spriteHeight == 16) {
						tileNum &= 0xFE;
					}

					if (y >= spriteY && y < spriteY + spriteHeight && x >= spriteX && x < spriteX + 8) {
						bool behindBG = attributes & 0x80;

						uint8_t line = y - spriteY;
						if (attributes & 0x40) {
							line = spriteHeight - 1 - line;
						}

						line *= 2;
						uint16_t dataAddr = 0x8000 + (tileNum * 16) + line;
						uint8_t data1 = memory->vram[dataAddr - 0x8000];
						uint8_t data2 = memory->vram[dataAddr + 1 - 0x8000];

						uint8_t colorBit = x - spriteX;
						if (!(attributes & 0x20)) {
							colorBit = 7 - colorBit;
						}

						uint8_t colorNum = ((data2 >> colorBit) & 1) << 1 | ((data1 >> colorBit) & 1);

						if (colorNum != 0) {
							uint8_t palette = (attributes & 0x10) ? obp1 : obp0;
							uint8_t paletteColor = (palette >> (colorNum * 2)) & 0x03;
							color = getColor(paletteColor);
							break;
						}
					}
				}
			}

			framebuffer[y * 160 + x] = color;
		}
	}
}

uint32_t PPU::getColor(uint8_t colorId) {
	switch (colorId) {
	case 0: return 0xFFFFFFFF; 
	case 1: return 0xAAAAAAAA; 
	case 2: return 0x55555555; 
	case 3: return 0x00000000; 
	}
	return 0xFFFFFFFF;
}


//void PPU::mode1Tick()
//{
//	if (enteredModeOne) {
//		currentMode = 0b1;
//		//updateSTAT();
//		updateMode(0b1);
//
//		// request VBlank IF
//		//memory->ioWriteIF(memory->ioFetchIF() | 0x01);
//
//		enteredModeOne = false;
//	}
//
//	if (internalDot >= 456) {
//		resetVals();
//		internalDot = 0;
//
//		//uint8_t ly = memory->ioFetchLY() + 1;
//		if (ly > 153) ly = 0;
//		//memory->ioWriteLY(ly);
//		updateLYC();         
//	}
//}
