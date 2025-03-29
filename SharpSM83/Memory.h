#ifndef MEMORY_H
#define MEMORY_H
#include <cstdint>

class Memory
{
public:



	//might do it this way, not finalized yet

	//should move these later but im gonna keep them here (cartidge file)


	uint8_t rom[0x4000]{};
	uint8_t romBank[0x4000]{};
	uint8_t cartRam[0x2000]{};

	Memory();

	uint8_t read(uint16_t address);
	uint8_t view(uint16_t address); // phase this out
	uint16_t readWord(uint16_t address);
	uint16_t viewWord(uint16_t address);
	void writeWord(uint16_t address, uint16_t data);
	void write(uint16_t address, uint8_t data);
	void reset();

	uint8_t ioFetchJOYP();
	uint8_t ioFetchSB();
	uint8_t ioFetchSC();
	uint8_t ioFetchDIV();
	uint8_t ioFetchTIMA();
	uint8_t ioFetchTMA();
	uint8_t ioFetchTAC();
	uint8_t ioFetchIF();

	void ioWriteJOYP(uint8_t data);
	void ioWriteSB(uint8_t data);
	void ioWriteSC(uint8_t data);
	void ioWriteDIV(uint8_t data);
	void ioWriteTIMA(uint8_t data);
	void ioWriteTMA(uint8_t data);
	void ioWriteTAC(uint8_t data);
	void ioWriteIF(uint8_t data);

	void ioIncrementDIV();
	void ioIncrementTIMA();

	void setInterruptVBlank();

	void setInterruptLCD();
	void setInterruptTimer();
	void setInterruptSerial();
	void setInterruptJoypad();

private:

	uint8_t vram[0x2000]{}; // video ram
	uint8_t wram[0x2000]{}; // work ram
	uint8_t oam[0xA0]{};	// object attribute memory, for sprites
	uint8_t io[0x80]{}; //<for cpu commands
	uint8_t hram[0x7F]{}; //higher ram

};
#endif
