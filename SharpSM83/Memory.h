#ifndef MEMORY_H
#define MEMORY_H

#include <array>
#include <cstdint>

// forward decleration to remove wierd loop
class Input;

class Memory
{
public:

	Memory();
	Input* input;

	uint8_t rom[0x8000]{};
	uint8_t vram[0x2000]{}; // video ram
	uint8_t wram[0x2000]{}; // work ram
	uint8_t oam[0xA0]{};	// object attribute memory, for sprites
	uint8_t io[0x80]{}; //<for cpu commands
	uint8_t hram[0x7F]{}; //higher ram
	uint8_t IE;

	uint8_t ram[0x2000]; //external ram

	uint8_t read(uint16_t address);
	void write(uint16_t address, uint8_t data);
	
	bool loadROM(const char* filename);

private:



};
#endif // MEMORY_H
