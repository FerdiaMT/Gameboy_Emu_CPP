#include "Memory.h"

Memory::Memory()
{
}

uint8_t Memory::read(uint16_t& address)
{
	uint8_t temp = ram[address];
	address++;
	return temp;
}

uint8_t Memory::view(uint16_t& address)
{
	uint8_t temp = ram[address];
	return temp;
}

uint16_t Memory::readWord(uint16_t& address)
{
	uint16_t data = ram[address];
	address++;
	data |= ram[address] << 8;
	address++;
	return data;
}

void Memory::write(uint16_t address, uint8_t data)
{
	ram[address] = data;
}

void Memory::reset()
{

}

