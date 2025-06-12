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

PPU::PPU(Memory& memory) : memory(memory) //m cycle (using this) = 4 t states
{

}

void PPU::resetPPU()
{

}

uint8_t screen[160][144];

void PPU::updateScreenBuffer(uint8_t(&mainScreen)[160][144])
{
	std::memcpy(mainScreen, screen, sizeof(screen));
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

void PPU::mode2Tick() 
{
	//FE00 , FE9F is OAM
	// only 10 per scanline
	// each object is 4 bytes (40 possible entries)

	//std::cout << (int)memory.ioFetchLY()<< "  ,  "<< (int)internalDot << std::endl;

	//search the current scanline
	
	//so we start of by searching through all the 40 entries with out 80 ticks
	//so it takes 2 ticks per entry ?

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
			std::cout << "item added " << (int)oamBuffer[(curItemsOnScanline * 4) - 1]<<std::endl;
			
			//4 bytes  , BYTE  = 8 bit
		}


		searchAddr += 0x04; 
	}
	mode2Half = !mode2Half;

	

}

void PPU::mode3Tick() 
{
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
		}
	}

}

void PPU::mode1Tick() // 10 scanlines of 456 dots
{
	if (internalDot >= 456) 
	{ 
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