#include "Memory.h"
#include <stdio.h>
Memory::Memory()
{
}

uint8_t Memory::read(uint16_t& address)
{
	uint8_t temp = ram[address];
	address++;
	return temp;
}

uint8_t Memory::view(uint16_t address)
{
	uint8_t temp = ram[address];
	return temp;
}

//uint16_t Memory::readWord(uint16_t& address)
//{
//	uint16_t data = ram[address];
//	address++;
//	data |= ram[address] << 8;
//	address++;
//	return data;
//}


uint16_t Memory::readWord(uint16_t& address)
{
	uint16_t low = ram[address];
	address++;
	uint16_t high = ram[address];
	address++;
	return (high << 8) | low;
}


void Memory::writeWord(uint16_t address, uint16_t data)
{
	ram[address] = data & 0xFF;
	ram[address+1] = ((data >>8) & 0x00FF);
}

//void Memory::writeWord(uint16_t& address, uint16_t data)
//{
//	ram[address++] = data & 0xFF;       // Low byte
//	ram[address++] = (data >> 8) & 0xFF; // High byte
//}


void Memory::write(uint16_t address, uint8_t data)
{
	ram[address] = data;
}

void Memory::reset()
{

}

