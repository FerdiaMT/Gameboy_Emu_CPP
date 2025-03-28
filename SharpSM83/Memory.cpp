#include "Memory.h"
#include <stdio.h>

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
Memory::Memory()
{
}

uint8_t Memory::read(uint16_t& address) // adress auto increment
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
		return wram[address - CARTRAM_LB];
	}
	else if (address <= OAM_UB)
	{
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

		if (address = 0xFF46)
		{
//			dmaTransfer();
			return -1;
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
	}

	//if(address >= ROM_LB &&)

	//if (address >= VRAM_LB && address <= VRAM_UB) // if accesing vram
	//{
	//	uint8_t temp = vram[address-VRAM_LB];
	//	address++;
	//	return temp;
	//}
	//else if (address >= OAM_LB && address <= OAM_UB) // if accesing OAM
	//{ 

	//	if (OAM_DMA_ACTIVE) 
	//	{
	//		return 0xFF; // return junk if OAM DMA is currenlty active
	//	}
	//	else 
	//	{
	//		uint8_t temp = oam[address - OAM_LB];
	//		address++;
	//		return temp;
	//	}
	//}
	//else if (address >= IO_LB && address <= IO_UB) 
	//{
	//	if (address = 0xFF46)
	//	{
	//		//dmaTransfer();
	//	}
	//}
	//else if (address >= HR_LB && address <= HR_UB) 
	//{

	//}
	//else 
	//{
	//	uint8_t temp = ram[address]; // this one needs to be abstrated 
	//	address++;
	//	return temp;
	//}
}

uint16_t Memory::readWord(uint16_t& address)
{
	uint16_t low = ram[address];
	address++;
	uint16_t high = ram[address];
	address++;
	return (high << 8) | low;
}


uint8_t Memory::view(uint16_t address) // adress doesnt increment
{
	uint8_t temp = ram[address];
	return temp;
}

uint16_t Memory::viewWord(uint16_t address) // adress doesnt increment
{
	uint16_t low = ram[address];
	address++;
	uint16_t high = ram[address];
	return (high << 8) | low;
}


void Memory::write(uint16_t address, uint8_t data)
{
	ram[address] = data;
}

void Memory::writeWord(uint16_t address, uint16_t data)
{
	ram[address] = data & 0xFF;
	ram[address + 1] = ((data >> 8) & 0x00FF);
}



void Memory::reset()
{

}
void Memory::reset()
{

}

