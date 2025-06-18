#include "PPU.h"
#include "SM83.h"
#include "memory.h"
#include <iostream>
#include <queue>
#include <cstring>

uint16_t internalDot{};
uint16_t mode3Dots{};

bool mode3Complete = false;

std::queue<uint8_t> backgroundFifo{};
std::queue<uint8_t>  spriteFifo{};
uint8_t screen[160][144];

uint8_t currentLY{};


// resource used -> https://hacktix.github.io/GBEDG/ppu/#oam-scan-mode-2

PPU::PPU(Memory& memory) : memory(memory) //m cycle (using this) = 4 t states
{

}

void PPU::resetPPU()
{

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

uint8_t oamBuffer[40]{};


void PPU::mode2Tick()  // TODO : DOUBLE TALL SPRITE 
{
	if (!mode2Half)
	{

		searchAddr = 0xFE00; // for the oam
	}
	else
	{
		uint8_t oamY = memory.readPPU(searchAddr);

		if (currentLY == oamY && curItemsOnScanline <=10)
		{
			curItemsOnScanline++;

			oamBuffer[(curItemsOnScanline * 4) - 4] = oamY; // this should be fine,   // y
			oamBuffer[(curItemsOnScanline * 4) -3 ] = memory.readPPU(searchAddr + 1); // x 
			oamBuffer[(curItemsOnScanline * 4) -2 ] = memory.readPPU(searchAddr + 2); // tile index
			oamBuffer[(curItemsOnScanline * 4) -1 ] = memory.readPPU(searchAddr + 3); // attributes
			
			//4 bytes  , BYTE  = 8 bit
		}
		

		searchAddr += 0x04; 
	}
	mode2Half = !mode2Half;

	

}

uint8_t fetcherX{}; 
uint8_t windowLineCounter{};

uint8_t screenX{};
uint8_t screenY{};

uint16_t memAddr{};
uint16_t tileNumber{};

uint8_t bgx{};
uint8_t bgy{};



void PPU::fetchTileNo()
{
	//these 2 give us the coordinates of the visible portion of the map
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
	 
	tileNumber = (memory.readPPU(memAddr + (bgx / 8) + ((bgy / 8) *32) ));
}

uint8_t tileDataA{};
uint8_t tileDataB{};

void PPU:: fetchTileL()
{

	uint8_t fineY = (bgy) % 8;


	if (memory.ioFetchLCDC() & 0b10000)
	{
		memAddr = 0x8000;
		tileDataA = memory.readPPU( memAddr + (tileNumber * 0x10) + (fineY *2) );
	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataA = memory.readPPU(memAddr + ((int8_t)tileNumber*16) + currentLY * 2);

	}
}

void PPU::fetchTileH()
{

	uint8_t fineY = (bgy) % 8;

	if (memory.ioFetchLCDC() & 0b10000)
	{
		memAddr = 0x8000;
		tileDataB = memory.readPPU(1+memAddr + (tileNumber * 0x10) + (fineY * 2));
	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataB = memory.readPPU(1+memAddr + ((int8_t)tileNumber * 16) + currentLY * 2);
	}
}

bool fifoCanPush = false;

bool tileFetcherFinished = false;
bool pixelOutputMode = false;


void PPU::fifoPush() {

	for (int x = 7; x >= 0; x--) {
		backgroundFifo.push(((tileDataB >> x) & 0b1) << 1 | ((tileDataA >> x) & 0b1));
	}
	tileFetcherFinished = true;
}

uint8_t twelveDot{};
uint8_t fetcherState{};


void PPU::tileFetcher()
{
	if (fetcherX < 160) {
		if (fetcherX % 8 == 0)
		{
			fetchTileNo();
			fetchTileL();
			fetchTileH();
			fifoPush();
			fifoCanPush = true;
		}
		fetcherX++;
	}
	else {
		mode3Complete = true;
		mode3Dots -= 1;
	}


}

uint8_t internalX{};

void PPU::drawPixel()
{

	if (internalX < 160 )  // Only draw visible pixels
	{
		screen[internalX][currentLY] = backgroundFifo.front();
	}
	backgroundFifo.pop();

	internalX++;
}


void PPU::mode3Tick() 
{
	tileFetcher();
	if (!backgroundFifo.empty())
	{
		drawPixel();
	}
	//if (!tileFetcherFinished)
	//{
	//	tileFetcher();
	//}
	//else
	//{
	//	pixelOutputMode = true;
	//	tileFetcherFinished = false;
	//}

	//if (pixelOutputMode)
	//{
	//	if (!backgroundFifo.empty())
	//	{
	//		drawPixel();

	//	}
	//	else {
	//		pixelOutputMode = false;
	//	}
}


void resetVals()
{
	
	twelveDot = 0;

}


void PPU::executeTick() // measured in m cycles
{


	currentLY = memory.ioFetchLY();

	if (currentLY >= 144) // this is a demo
	{
		mode1Tick();
	}
	else
	{
		if (internalDot < 80)
		{
			writeIntoSTAT(0b10);
			mode2Tick();
			memory.vramLocked = false;
		}
		else 
		{

			//Mode 3 is 172 to 289 dots long
			if (!mode3Complete)
			{

				memory.vramLocked = true;

				mode3Tick();

				writeIntoSTAT(0b11);

				mode3Dots++;
			}
			else
			{

				memory.vramLocked = false;
				//mode 0
				writeIntoSTAT(0b0);
			}

		}

		if (internalDot >= 456) { // end of scanline
			initFrame();
			internalDot = -1;
			memory.ioIncrementLY();
			//while (!backgroundFifo.empty()) backgroundFifo.pop();
			fetcherX = 0;
			internalX = 0;
		}
	}
	//std::cout << "DOT :" << (int)internalDot << " MODE3 dot " << mode3Dots << "   vram:Lock " << (int)memory.vramLocked << "  " << (int)fetcherX << std::endl;
	internalDot++;

}



void PPU::mode1Tick() // 10 scanlines of 456 dots
{
	writeIntoSTAT(0b1);
	uint8_t temp = memory.ioFetchIF();
	memory.ioWriteIF(temp |= 0b1);

	if (internalDot >= 456) 
	{ 

		resetVals();

		internalDot = 0;

		if (currentLY == 153)
		{
			memory.ioWriteLY(0);
		}
		else
		{
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