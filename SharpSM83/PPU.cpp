#include "PPU.h"
#include "SM83.h"
#include "memory.h"
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

PPU::PPU(Memory& memory) : memory(memory) //m cycle (using this) = 4 t states
{

}

void PPU::resetPPU()
{

}

uint8_t currentMode{};

void PPU::updateSTAT() {
	uint8_t STAT = memory.readPPU(0xFF41);
	uint8_t LY = memory.readPPU(0xFF44);
	uint8_t LYC = memory.readPPU(0xFF45);

	if (LY == LYC) {

		STAT |= 0x04; 
	}
	else {
		STAT &= ~0x04;
	}

	STAT = (STAT & 0xF8) | currentMode;

	memory.writePPU(0xFF41, STAT);
}

void PPU::checkSTATInterrupts() {
	uint8_t STAT = memory.readPPU(0xFF41);
	bool trigger = false;

	if ((currentMode == 0) && (STAT & 0x08)) trigger = true; // HBlank
	if ((currentMode == 1) && (STAT & 0x10)) trigger = true; // VBlank
	if ((currentMode == 2) && (STAT & 0x20)) trigger = true; // OAM
	if ((STAT & 0x04) && (STAT & 0x40)) trigger = true;      // LYC

	if (trigger) {
		uint8_t IF = memory.ioFetchIF();
		memory.ioWriteIF(IF | 0x02); // STAT interrupt
	}
}


void PPU::writeIntoSTAT(uint8_t mode)
{
	uint8_t val = memory.readPPU(0xFF41);
	val = (val & 0b11111100) | mode;
	memory.writePPU(0xFF41, val);

	if (mode == 0b00 && ((val & 0b1000) >> 3 == 1)) //mode 0 trigger
	{
		uint8_t temp = memory.ioFetchIF();
		memory.ioWriteIF(temp |= 0b10);
	}
	else if (mode == 0b01 && ((val & 0b10000) >> 4 == 1)) //mode 1 trigger
	{
		uint8_t temp = memory.ioFetchIF();
		memory.ioWriteIF(temp |= 0b10);
	}
	else if (mode == 0b10 && ((val & 0b100000) >> 5 == 1)) //mode 2 trigger
	{
		uint8_t temp = memory.ioFetchIF();
		memory.ioWriteIF(temp |= 0b10);
	}
	else if (mode == 0b11 && ((val & 0b1000000) >> 6 == 1)) //mode 3 trigger
	{
		uint8_t temp = memory.ioFetchIF();
		memory.ioWriteIF(temp |= 0b10);
	}

	if ((val & 0b100) >> 2 == (val & 0b1000000) >> 6) // LY = LYC
	{
		uint8_t temp = memory.ioFetchIF();
		memory.ioWriteIF(temp |= 0b10);
	}

}



void initFrame() {
	mode3Dots = 0;
	mode3Complete = false;
}

bool mode2Half = 0;

uint8_t searchX = 0; // up to168
uint8_t searchY = 0; // up to 160
uint16_t searchAddr = 0xFE00;

uint8_t curItemsOnScanline = 0;

//struct Sprite {
//	uint8_t x;
//	uint8_t y;
//	uint8_t ti;
//	uint8_t attr;
//};

std::vector<Sprite> scanlineQueue{};

void PPU::mode2Tick()  // TODO : DOUBLE TALL SPRITE 
{
	if (!mode2Half)
	{

		// for the oam
	}
	else
	{
		uint8_t oamY = memory.readPPU(searchAddr);

		if (currentLY >= oamY-16  && currentLY < oamY-16+8 && curItemsOnScanline <=10)
		{
			
			curItemsOnScanline++;
			Sprite sprite;
			sprite.y = oamY; // this should be fine,   // y
			sprite.x = memory.readPPU(searchAddr + 1); // x 
			sprite.ti = memory.readPPU(searchAddr + 2); // tile index
			sprite.attr = memory.readPPU(searchAddr + 3); // attributes

			//std::cout << "added at ti: x " <<(int)sprite.x << " y : " << (int) sprite.y << " "<< (int)sprite.ti << std::endl;
			scanlineQueue.push_back(sprite);

			//4 bytes  , BYTE  = 8 bit
		}
		

		searchAddr += 0x04; 
	}
	mode2Half = !mode2Half;

}

uint8_t fetcherX{}; 

uint8_t screenX{};
uint8_t screenY{};

uint16_t memAddr{};
uint16_t tileNumber{};

uint8_t bgx{};
uint8_t bgy{};

uint16_t mode3Penalty = 12;


bool renderingWindow = false;
uint16_t windowY{};
uint16_t windowLineCounter = 0;
bool windowTriggered = false;


void PPU::fetchTileNo()
{
	if ((memory.ioFetchLCDC() & 0b1000))
	{
		memAddr = 0x9C00;
	}
	else
	{
		memAddr = 0x9800;
	}

	bgx = (memory.ioFetchSCX() + fetcherX) % 256;
	bgy = (memory.ioFetchSCY() + currentLY) % 256;
	 
	tileNumber = (memory.readPPU(memAddr + (bgx / 8) + (bgy / 8) *32));
}

uint8_t tileDataA{};
uint8_t tileDataB{};

void PPU:: fetchTileL()
{

	uint8_t fineY = (bgy) % 8;


	if ((memory.ioFetchLCDC() & 0x10))
	{
		memAddr = 0x8000;
		tileDataA = memory.readPPU( memAddr + (tileNumber * 0x10) + (fineY *2) );
	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataA = memory.readPPU(memAddr + ((int8_t)tileNumber*16) + fineY * 2);

	}
}

void PPU::fetchTileH()
{

	uint8_t fineY = (bgy) % 8;

	if ((memory.ioFetchLCDC() & 0x10))
	{
		memAddr = 0x8000;
		tileDataB = memory.readPPU(1+(memAddr + (tileNumber * 0x10) + (fineY * 2)));


	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataB = memory.readPPU(1+ (memAddr + ((int8_t)tileNumber * 16) + fineY * 2));
	}
}




void PPU::fetchWindowTileNo()
{
	if ((memory.ioFetchLCDC() & 0b01000000))
	{
		if ((memory.ioFetchLCDC() & 0b1000))
		{
			memAddr = 0x9C00;
		}
		else
		{
			memAddr = 0x9800;
		}

		uint8_t winx = memory.ioFetchWX() - 7;
		uint8_t winy = memory.ioFetchWY();

		if (currentLY >= winy)
		{
			windowY = currentLY - winy;
			tileNumber = (memory.readPPU(memAddr + ((fetcherX - (winx)) / 8) + (windowY / 8) * 32));
			renderingWindow = true;
		}
	}
}

void PPU::fetchWindowTileL()
{
	if (!renderingWindow) return;

	uint8_t fineY = windowY % 8;

	if (memory.ioFetchLCDC() & 0x10)
	{
		memAddr = 0x8000;
		tileDataA = memory.readPPU(memAddr + (tileNumber * 0x10) + (fineY * 2));
	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataA = memory.readPPU(memAddr + ((int8_t)tileNumber * 16) + fineY * 2);
	}
}

void PPU::fetchWindowTileH()
{
	if (!renderingWindow) return;

	uint8_t fineY = windowY % 8;

	if (memory.ioFetchLCDC() & 0x10)
	{
		memAddr = 0x8000;
		tileDataB = memory.readPPU(1 + (memAddr + (tileNumber * 0x10) + (fineY * 2)));
	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataB = memory.readPPU(1 + (memAddr + ((int8_t)tileNumber * 16) + fineY * 2));
	}
}


bool fifoCanPush = false;

bool tileFetcherFinished = false;
bool pixelOutputMode = false;


void PPU::fetchSpriteTile(const Sprite& sprite)
{
	uint8_t spriteHeight = (memory.ioFetchLCDC() & 0x04) ? 16 : 8;

	int16_t spriteY = currentLY - (sprite.y - 16);
	if (spriteY < 0 || spriteY >= spriteHeight) return;

	uint16_t tileIndex = sprite.ti;
	if (spriteHeight == 16) 
	{
		tileIndex &= 0xFE; 
	}

	if (sprite.attr & 0x40) 
	{
		spriteY = spriteHeight - 1 - spriteY;
	}

	uint16_t addr = 0x8000 + (tileIndex * 16) + spriteY * 2;
	uint8_t dataLo = memory.readPPU(addr);
	uint8_t dataHi = memory.readPPU(addr + 1);

	for (int i = 7; i >= 0; i--)
	{
		int pixelBit = (sprite.attr & 0x20) ? 7 - i : i;
		uint8_t color = ((dataHi >> pixelBit) & 1) << 1 | ((dataLo >> pixelBit) & 1);

		SpritePixel spx;
		spx.color = color;
		spx.bgPriority = sprite.attr & 0x80;
		spx.transparent = (color == 0);

		spriteFifo.push(spx);
	}
}


void PPU::fifoPush() {

	for (int x = 7; x >= 0; x--) 
	{
		backgroundFifo.push(((tileDataB >> x) & 0b1) << 1 | ((tileDataA >> x) & 0b1));
	}
	tileFetcherFinished = true;
}

uint8_t twelveDot{};
uint8_t fetcherState{};



void PPU::tileFetcher()
{
	if (fetcherX < 160) 
	{
		uint8_t winx = memory.ioFetchWX() - 7;
		bool windowActive = (memory.ioFetchLCDC() & 0b01000000) &&(memory.ioFetchWY() <= currentLY) &&(fetcherX >= winx) && (winx <= 166); 

		if (windowActive && !windowTriggered)
		{
			windowTriggered = true;
			windowLineCounter++;
			fetcherX = 0; 
		}

		if (fetcherX % 8 == 0)
		{
			if (windowTriggered)
			{
				fetchWindowTileNo();
				fetchWindowTileL();
				fetchWindowTileH();
			}
			else
			{
				fetchTileNo();
				fetchTileL();
				fetchTileH();
			}
			fifoPush();


			fifoCanPush = true;
		}


		for (const auto& sprite : scanlineQueue)
		{
			//std::cout << "sprite checked"<<std::endl;
			if (sprite.x - 8 == fetcherX)

			{
				fetchSpriteTile(sprite);
			}
		}

		fetcherX++;
	}
	else 
	{
		mode3Complete = true;
		fifoCanPush = false;
		mode3Dots -= 1;
	}
}



uint8_t internalX{};

void PPU::drawPixel()
{
	if (backgroundFifo.empty())
		return;

	uint8_t bgColor = backgroundFifo.front();
	uint8_t finalColor = bgColor;

	if (!spriteFifo.empty()) {
		SpritePixel spx = spriteFifo.front();
		finalColor = spx.color;
		spriteFifo.pop();
	}

	backgroundFifo.pop();
	screen[internalX][currentLY] = finalColor;
	internalX++;
}



void PPU::mode3Tick() 
{

	if (!spritesSorted)
	{
		std::sort(scanlineQueue.begin(), scanlineQueue.end(), [](const Sprite& a, const Sprite& b) 
		{
			return a.x < b.x;
		});

		spritesSorted = true;
	}

	tileFetcher();
	//spriteFetcher();
	if (!backgroundFifo.empty())
	{
		drawPixel();
	}
}


void resetVals()
{
	twelveDot = 0;
	mode3Penalty = 12;
}

bool enteredModeOne = true;

void PPU::executeTick() // measured in m cycles
{


	currentLY = memory.ioFetchLY();

	if (memory.writeToLYC() )
	{
		updateSTAT();

		if (memory.ioFetchLY() == memory.ioFetchLYC())
		{
			memory.ioWriteStat(memory.ioFetchSTAT() | 0x04);
		}
	}

	if (currentLY >= 144) 
	{
		mode1Tick();
	}
	else
	{
		if (internalDot < 80)
		{
			mode2Tick();
			memory.vramLocked = false;
			currentMode = 0b10;
			updateSTAT();
		}
		else 
		{

			//Mode 3 is 172 to 289 dots long
			if (!mode3Complete)
			{

				memory.vramLocked = true;

				mode3Tick();

				currentMode = 0b11;
				updateSTAT();

				mode3Dots++;
			}
			else
			{

				//this area compensates for both mode 3 and mode 0

				if (mode3Penalty > 0) 
				{
					mode3Penalty--;
				}
				else  //mode 0
				{
					memory.vramLocked = false;
					
					currentMode = 0b0;
					updateSTAT();
				}



			}

		}

		if (internalDot >= 456) { // end of scanline

			//this section is when the ly changes and everything increments


			searchAddr = 0xFE00;
			scanlineQueue.clear();
			curItemsOnScanline = 0;
			spritesSorted = false;
			initFrame();
			internalDot = -1;
			enteredModeOne = true;
			
			fetcherX = 0;
			internalX = 0;
			//check if ly == lyc
			if (memory.ioFetchLY() == memory.ioFetchLYC())
			{
				memory.ioWriteStat(memory.ioFetchSTAT() | 0x04);
			}
			else
			{
				memory.ioWriteStat(memory.ioFetchSTAT() & ~0x04);
			}


			memory.ioIncrementLY();

			updateSTAT();

			if (memory.ioFetchLY() == memory.ioFetchLYC())
			{
				memory.ioWriteStat(memory.ioFetchSTAT() | 0x04);
			}

		}
	}
	//std::cout << "DOT :" << (int)internalDot << " MODE3 dot " << mode3Dots << "   vram:Lock " << (int)memory.vramLocked << "  " << (int)fetcherX << std::endl;
	internalDot++;

}



void PPU::mode1Tick()
{
	if (enteredModeOne) {
		currentMode = 0b1;
		updateSTAT();

		// request VBlank IF
		memory.ioWriteIF(memory.ioFetchIF() | 0x01);

		enteredModeOne = false;
	}

	if (internalDot >= 456)
	{
		resetVals();
		internalDot = 0;

		if (currentLY == 153) {
			memory.ioWriteLY(0);
		}
		else {
			memory.ioIncrementLY();
		}
	}
}

void PPU::updateScreenBuffer(uint8_t(&mainScreen)[160][144])
{
	std::memcpy(mainScreen, screen, sizeof(screen));
}

uint16_t PPU::getInternalDot()
{
	return internalDot;
}