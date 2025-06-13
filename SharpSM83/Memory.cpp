#include "Memory.h"
#include "SM83.h"
#include <stdio.h>
#include <iostream>


#define ROM_LB 0x0000
#define ROM_UB 0x3FFF

#define ROMBANK_LB 0x4000
#define ROMBANK_UB 0x7FFF

#define VRAM_LB 0x8000
#define VRAM_UB 0x9FFF

#define CARTRAM_LB 0xA000
#define CARTRAM_UB 0xBFFF

#define WRAM_LB 0xC000
#define WRAM_UB 0xDFFF

// not going to define echo ram for now

#define OAM_LB  0xFE00
#define OAM_UB  0xFE9F

#define IO_LB   0xFF00
#define IO_UB   0xFF7F

#define HR_LB   0xFF80
#define HR_UB   0xFFFE


/*
MAINTENANCE REQUIRED : REMOVE THE VIEW AND VIEWWORD FUNCTIONS
					   FIGURE OUT WHAT TO DO FOR RESET
*/



// 0xFFFF is the interupt enable register

/*
	uint8_t vram[0x2000]{}; // video ram
	uint8_t wram[0x2000]{}; // work ram
	uint8_t oam[0xA0]{};	// object attribute memory, for sprites
	uint8_t io[0x80]{}; //<for cpu commands
	uint8_t hram[0x7F]{}; //higher ram
	//should move these later but im gonna keep them here (cartidge file)
	uint8_t rom[0x4000]{};
	uint8_t romBank[0x4000]{};
	uint8_t cartRam[0x2000]{};
*/

/*
* Memory Blocks
* 0x8000 - 0x9FFF	~	VRAM
* 0xFE00 - 0xFE9F	~	OAM
* 0xFF00 - 0xFF7F	~	IO //DMA INSIDE HERE 
* 0xFF80 - 0xFFFE	~ HIGH ram
*/
Memory::Memory() {};
uint8_t Memory::read(uint16_t address) // adress auto increment
{
	//	ANYTHING WITH THE WORDS ROM OR CART SHOULD BE ABSTRACTED AWAY IN THE FUTURE 

	if (address <= ROM_UB)
	{
		return rom[address];
	}
	else if (address <= ROMBANK_UB)
	{
		return romBank[address - ROMBANK_LB];
	}
	else if (address <= VRAM_UB)
	{
		if (vramLocked)
		{
			return 0xFF;
		}
		else
		{
			return vram[address - VRAM_LB];
		}
	}
	else if (address <= CARTRAM_UB)
	{
		return cartRam[address - CARTRAM_LB];
	}
	else if (address <= WRAM_UB)
	{
		return wram[address - WRAM_LB];
	}
	else if (address <= OAM_UB)
	{
		if (vramLocked) return 0xFF;
		if (-1/*OAM_DMA_ACTIVE*/)
		{
			return 0xFF; // return junk if OAM DMA is currenlty active
		}
		else 
		{
			uint8_t temp = oam[address - OAM_LB];
			address++;
			return temp;
		}
	}
	else if (address <= IO_UB) // i should have no return
	{

		if (address == 0xFF46)
		{
			0x00;
		}
		else {
			return io[address - IO_LB];
		}
		
	}
	else if (address <= HR_UB)
	{
		return hram[address - HR_LB];
	}
	else
	{
		//something went really wrong if it gets to here
		return 0;
	}
}

uint8_t Memory::readPPU(uint16_t address) // adress auto increment
{
	//	ANYTHING WITH THE WORDS ROM OR CART SHOULD BE ABSTRACTED AWAY IN THE FUTURE 

	if (address <= ROM_UB)
	{
		return rom[address];
	}
	else if (address <= ROMBANK_UB)
	{
		return romBank[address - ROMBANK_LB];
	}
	else if (address <= VRAM_UB)
	{
		return vram[address - VRAM_LB];
	}
	else if (address <= CARTRAM_UB)
	{
		return cartRam[address - CARTRAM_LB];
	}
	else if (address <= WRAM_UB)
	{
		return wram[address - WRAM_LB];
	}
	else if (address <= OAM_UB)
	{
		if (vramLocked) return 0xFF;
		if (-1/*OAM_DMA_ACTIVE*/)
		{
			return 0xFF; // return junk if OAM DMA is currenlty active
		}
		else
		{
			uint8_t temp = oam[address - OAM_LB];
			address++;
			return temp;
		}
	}
	else if (address <= IO_UB) // i should have no return
	{

		if (address == 0xFF46)
		{
			0x00;
		}
		else {
			return io[address - IO_LB];
		}

	}
	else if (address <= HR_UB)
	{
		return hram[address - HR_LB];
	}
	else
	{
		//something went really wrong if it gets to here
		return 0;
	}
}

uint16_t Memory::readWord(uint16_t address)
{
	uint16_t low = read(address);
	address++;
	uint16_t high = read(address);
	return (high << 8) | low;
}


uint8_t Memory::view(uint16_t address) // remove this later
{
	return read(address);
}

uint16_t Memory::viewWord(uint16_t address) // same ghere
{
	return readWord(address);
}


void Memory::write(uint16_t address, uint8_t data)
{

	if (address <= ROM_UB)
	{
		rom[address] = data;
	}
	else if (address <= ROMBANK_UB)
	{
		romBank[address - ROMBANK_LB] = data;
	}
	else if (address <= VRAM_UB)
	{
		//if (vramLocked)
		//{
		//	
		//	return;
		//}
		
		vram[address - VRAM_LB] = data;
	}
	else if (address <= CARTRAM_UB)
	{
		cartRam[address - CARTRAM_LB] = data;
	}
	else if (address <= WRAM_UB)
	{
		wram[address - WRAM_LB] = data;
	}
	else if (address <= OAM_UB)
	{
		if (vramLocked) return;

		oam[address - OAM_LB] = data;

	}
	else if (address <= IO_UB) // i should have no return
	{


		if (address == 0xFF02 && data == 0x81) {
			char c = read(0xFF01);
			std::cout << c; 
		}

		if (address == 0xFF46) //OAM DMA START
		{
			//writing here starts transfer of rom to oam
			//supposed to take 160 machine cycles ? how do i do this
			dmaPending = true;

			//Source:      $XX00 - $XX9F;XX = $00 to $DF
			//Destination : $FE00 - $FE9F
			uint16_t endAddr = 0xFE00;
			for (uint16_t i = (data << 2); i <= ((data << 2) | 0x9F); i++) // from source start to source end
			{
				write(endAddr, read(i));
				endAddr++;
			}

			
		}

		io[address - IO_LB] = data;

	}
	else if (address <= HR_UB)
	{
		hram[address - HR_LB] = data;
	}
	else
	{
		//something went really wrong if it gets to here
	}

}

void Memory::writePPU(uint16_t address, uint8_t data)
{

	if (address <= ROM_UB)
	{
		rom[address] = data;
	}
	else if (address <= ROMBANK_UB)
	{
		romBank[address - ROMBANK_LB] = data;
	}
	else if (address <= VRAM_UB)
	{
		vram[address - VRAM_LB] = data;
	}
	else if (address <= CARTRAM_UB)
	{
		cartRam[address - CARTRAM_LB] = data;
	}
	else if (address <= WRAM_UB)
	{
		wram[address - WRAM_LB] = data;
	}
	else if (address <= OAM_UB)
	{
		if (vramLocked) return;

		oam[address - OAM_LB] = data;

	}
	else if (address <= IO_UB) // i should have no return
	{
		if (address == 0xFF46) //OAM DMA START
		{
			//writing here starts transfer of rom to oam
			//supposed to take 160 machine cycles ? how do i do this
			dmaPending = true;

			//Source:      $XX00 - $XX9F;XX = $00 to $DF
			//Destination : $FE00 - $FE9F
			uint16_t endAddr = 0xFE00;
			for (uint16_t i = (data << 2); i <= ((data << 2) | 0x9F); i++) // from source start to source end
			{
				write(endAddr, read(i));
				endAddr++;
			}


		}

		io[address - IO_LB] = data;

	}
	else if (address <= HR_UB)
	{
		hram[address - HR_LB] = data;
	}
	else
	{
		//something went really wrong if it gets to here
	}

}

void Memory::writeWord(uint16_t address, uint16_t data)
{
	write(address,(data & 0xFF));
	write(address+1, (data >> 8) & 0x00FF);
}



void Memory::reset()
{

}

//THE 0 PAGE HAS DIRECT FETCHES , TO BETTER EMULATE GAMRBOY FETCH SPEEDS

uint8_t Memory::ioFetchJOYP()
{
	return io[0];
}

uint8_t Memory::ioFetchSB()
{
	return io[1];
}

uint8_t Memory::ioFetchSC()
{
	return io[2];
}

uint8_t Memory::ioFetchDIV()
{
	return io[4];
}

uint8_t Memory::ioFetchTIMA()
{
	return io[5];
}

uint8_t Memory::ioFetchTMA()
{
	return io[6];
}

uint8_t Memory::ioFetchTAC()
{
	return io[7];
}

uint8_t Memory::ioFetchIF()
{
	return io[0x0F];
}

uint8_t Memory::ioFetchLCDC()
{
	return io[0x40];
}

uint8_t Memory::ioFetchSCY()
{
	return io[0x42];
}
uint8_t Memory::ioFetchSCX()
{
	return io[0x43];
}


uint8_t Memory::ioFetchLY()
{
	return io[0x44];
}

void Memory::ioIncrementLY()
{
	io[0x44]++;
}

void Memory::ioWriteJOYP(uint8_t data)
{
	io[0x0] = data;
}

void Memory::ioWriteSB(uint8_t data)
{
	io[0x1] = data;
}

void Memory::ioWriteSC(uint8_t data)
{
	io[0x2] = data;
}

void Memory::ioWriteDIV(uint8_t data)
{
	io[0x4] = data;
}

void Memory::ioWriteTIMA(uint8_t data)
{
	io[0x5] = data;
}

void Memory::ioWriteTMA(uint8_t data)
{
	io[0x6] = data;
}

void Memory::ioWriteTAC(uint8_t data)
{
	io[0x7] = data;
}

void Memory::ioWriteIF(uint8_t data)
{
	io[0xF] = data;
}

void Memory::ioWriteLY(uint8_t data)
{
	io[0x44] = data;
}

void Memory::ioIncrementDIV()
{
	io[0x4]++;
}


void Memory::ioIncrementTIMA()
{
	io[0x5]++;
}

void Memory::setInterruptVBlank()
{
	io[0x0F] |= 0b00001;
}

void Memory::setInterruptLCD()
{
	io[0x0F] |= 0b00010;
}

void Memory::setInterruptTimer()
{
	io[0x0F] |= 0b00100;
}

void Memory::setInterruptSerial()
{
	io[0x0F] |= 0b01000;
}

void Memory::setInterruptJoypad()
{
	io[0x0F] |= 0b10000;
}



