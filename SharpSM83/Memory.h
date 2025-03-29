#ifndef MEMORY_H
#define MEMORY_H
#include <cstdint>

class Memory
{
public:

	uint8_t ram[0x10000]{};


	//might do it this way, not finalized yet
	uint8_t vram[0x2000]{}; // video ram
	uint8_t wram[0x2000]{}; // work ram
	uint8_t oam[0xA0]{};	// object attribute memory, for sprites
	uint8_t io[0x80]{}; //<for cpu commands
	uint8_t hram[0x7F]{}; //higher ram
	//should move these later but im gonna keep them here (cartidge file)


	uint8_t rom[0x4000]{};
	uint8_t romBank[0x4000]{};
	uint8_t cartRam[0x2000]{};

	Memory();

	uint8_t read(uint16_t address);
	uint8_t view(uint16_t address); // phase this out
	uint16_t readWord(uint16_t& address);
	uint16_t viewWord(uint16_t address);
	void writeWord(uint16_t address, uint16_t data);
	void write(uint16_t address, uint8_t data);
	void reset();
};
#endif
