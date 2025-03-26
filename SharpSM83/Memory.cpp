#include "Memory.h"
#include <stdio.h>

#define VRAM_LB 0x8000
#define VRAM_UB 0x9FFF
#define OAM_LB  0xFE00
#define OAM_UB  0xFE9F
#define IO_LB   0xFF00
#define IO_UB   0xFF7F
#define HR_LB   0xFF80
#define HR_UB   0xFFFE
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
	if (address >= VRAM_LB && address <= VRAM_UB) // if accesing vram
	{
		uint8_t temp = vram[address-VRAM_LB];
		address++;
		return temp;
	}
	else if (address >= OAM_LB && address <= OAM_UB) // if accesing OAM
	{ 

		if (OAM_DMA_ACTIVE) 
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
	else if (address >= IO_LB && address <= IO_UB) 
	{
		if (address = 0xFF46)
		{
			dmaTransfer();
		}
	}
	else if (address >= HR_LB && address <= HR_UB) 
	{

	}
	else 
	{
		uint8_t temp = ram[address];
		address++;
		return temp;
	}
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

