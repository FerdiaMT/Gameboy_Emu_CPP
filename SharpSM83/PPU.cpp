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


// resource used -> https://hacktix.github.io/GBEDG/ppu/#oam-scan-mode-2

PPU::PPU(Memory& memory) : memory(memory) //m cycle (using this) = 4 t states
{

}

void PPU::resetPPU()
{

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
		
	}
	else
	{
		uint8_t oamY = memory.readPPU(searchAddr);

		if (memory.ioFetchLY() == oamY && curItemsOnScanline <=10)
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

uint16_t testX = 0;

uint8_t tx{};
uint8_t ty{};

void PPU::fetchTileNo()
{
	//these 2 give us the coordinates of the visible portion of the map
	uint8_t scrollX = memory.ioFetchSCX();
	uint8_t scrollY = memory.ioFetchSCY();
	if ((memory.ioFetchLCDC() & 0b1000))
	{
		memAddr = 0x9C00;
	}
	else
	{
		memAddr = 0x9800;
	}


	 tx = (scrollX + fetcherX)/ 8;
	 ty = (scrollY + memory.ioFetchLY()) / 8;
	 
	 uint16_t finalAddr = memAddr ;
	 tileNumber = ((memory.readPPU(finalAddr)));

	 //printf("fetcherX:%d tx:%d finalAddr:%04X tile:%02X\n",fetcherX, tx, finalAddr, tileNumber);
}

uint8_t tileDataA{};
uint8_t tileDataB{};

void PPU:: fetchTileL()
{

	uint8_t fineY = (memory.ioFetchLY() + memory.ioFetchSCY()) % 8;


	if (memory.ioFetchLCDC() & 0b10000)
	{
		memAddr = 0x8000;
		tileDataA = memory.readPPU( memAddr + (tileNumber * 0x10) + fineY *2);
	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataA = memory.readPPU(memAddr + ((int8_t)tileNumber*16) + memory.ioFetchLY() * 2);

	}
}

void PPU::fetchTileH()
{

	uint8_t fineY = (memory.ioFetchLY() + memory.ioFetchSCY()) % 8;

	if (memory.ioFetchLCDC() & 0b10000)
	{
		memAddr = 0x8000;
		tileDataB = memory.readPPU(1+memAddr + (tileNumber * 0x10) + fineY * 2);
	}
	else
	{ //8800 mode
		memAddr = 0x9000;
		tileDataB = memory.readPPU(1+memAddr + ((int8_t)tileNumber * 16) + memory.ioFetchLY() * 2);
	}
}

bool fifoCanPush = false;

bool tileFetcherFinished = false;
bool pixelOutputMode = false;


void PPU::fifoPush() {
	//printf("Pushing tile %02X (Data: %02X %02X)\n",tileNumber, tileDataA, tileDataB); // DEBUG

	for (int x = 7; x >= 0; x--) {
		uint8_t pixel = ((tileDataB >> x) & 1) << 1 | ((tileDataA >> x) & 1);
		backgroundFifo.push(pixel);
	}
	//tileFetcherFinished = true;
}

uint8_t twelveDot{};
uint8_t fetcherState{};


void PPU::tileFetcher()
{



	if (fetcherX < 168) {
		if (fetcherX % 8 == 0)
		{
			fetchTileNo();
			fetchTileL();
			fetchTileH();
			fifoCanPush = true;
		}
		fetcherX++;
	}
	else
	{
		tileFetcherFinished = true;
	}

	if (backgroundFifo.empty())
	{

		fifoPush();
		fifoCanPush = false;
	}

}

uint8_t internalX{};

void PPU::drawPixel()
{

	if (internalX < 163 &&internalX >= 3)  // Only draw visible pixels
	{
		screen[internalX-3][memory.ioFetchLY()] = backgroundFifo.front();
	}
	backgroundFifo.pop();

	internalX++;
}


void PPU::mode3Tick() 
{

	if (!tileFetcherFinished)
	{
		tileFetcher();
	}
	else
	{
		pixelOutputMode = true;
		tileFetcherFinished = false;
	}
	if (pixelOutputMode)
	{
		if (!backgroundFifo.empty())
		{
			drawPixel();

		}
		else {
			pixelOutputMode = false;
		}
	}


}

void resetVals()
{
	internalX = 0;
	twelveDot = 0;

}


void PPU::executeTick(int allCycles) // measured in m cycles
{
	internalDot++;
	

	if (memory.ioFetchLY() >= 144)
	{
		mode1Tick();
	}
	else
	{
		if (internalDot == 0)
		{
			initFrame();
		}

		if (internalDot <= 80)
		{
			mode2Tick();
		}
		else if (internalDot > 80)
		{

			searchAddr = 0xFE00; // for the oam

			//Mode 3 is 172 to 289 dots long
			if (!mode3Complete)
			{
				mode3Tick();
				mode3Dots++;
			}
			else
			{
				//mode 0
			}

		}

		if (internalDot >= 456) { // end of scanline
			internalDot = 0;
			memory.ioIncrementLY();
			fetcherX = 0;
		}
	}

}



void PPU::mode1Tick() // 10 scanlines of 456 dots
{
	if (internalDot >= 456) 
	{ 

		resetVals();

		internalDot = 0;

		if (memory.ioFetchLY() == 153)
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