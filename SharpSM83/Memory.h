#ifndef MEMORY_H
#define MEMORY_H
#include <cstdint>

class Memory
{
public:

	uint8_t ram[0x1000];

	Memory();
	uint8_t read(uint16_t& address);
	uint8_t view(uint16_t& address);
	uint16_t readWord(uint16_t& address);
	void write(uint16_t address, uint8_t data);
	void reset();
};
#endif
