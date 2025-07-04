#ifndef MEMORY_H
#define MEMORY_H

#include <array>
#include <cstdint>

class Memory
{
public:

	Memory();

	uint8_t vram[0x2000]{}; // video ram
	uint8_t wram[0x2000]{}; // work ram
	uint8_t oam[0xA0]{};	// object attribute memory, for sprites
	uint8_t io[0x80]{}; //<for cpu commands
	uint8_t hram[0x7F]{}; //higher ram
	uint8_t IE;

	//should move these later but im gonna keep them here (cartidge file)


	uint8_t rom[0x4000]{};
	uint8_t romBank[0x4000]{};
	uint8_t cartRam[0x2000]{};

	std::array<uint8_t*, 0x10000> fastMap;

	void initFastMap();


	uint8_t read(uint16_t address);
	uint8_t view(uint16_t address); // phase this out

	uint8_t readPPU(uint16_t address);


	uint16_t readWord(uint16_t address);
	uint16_t viewWord(uint16_t address);
	void writeWord(uint16_t address, uint16_t data);
	void write(uint16_t address, uint8_t data);
	void writePPU(uint16_t address, uint8_t data);
	void reset();

	uint8_t ioFetchJOYP();
	uint8_t ioFetchSB();
	uint8_t ioFetchSC();
	uint8_t ioFetchDIV();
	uint8_t ioFetchTIMA();
	uint8_t ioFetchTMA();
	uint8_t ioFetchTAC();
	uint8_t ioFetchIF();
	uint8_t ioFetchIE();
	uint8_t ioFetchLCDC();
	uint8_t ioFetchSCY();
	uint8_t ioFetchSCX();
	uint8_t ioFetchWX();
	uint8_t ioFetchWY();
	uint8_t ioFetchSTAT();

	uint8_t ioFetchLY();
	uint8_t ioFetchLYC();

	void ioWriteJOYP(uint8_t data);
	void ioWriteSB(uint8_t data);
	void ioWriteSC(uint8_t data);
	void ioWriteDIV(uint8_t data);
	void ioWriteTIMA(uint8_t data);
	void ioWriteTMA(uint8_t data);
	void ioWriteTAC(uint8_t data);
	void ioWriteIF(uint8_t data);
	void ioWriteLY(uint8_t data);
	void ioWriteStat(uint8_t data);

	void ioIncrementDIV();
	void ioIncrementTIMA();
	void ioIncrementLY();

	void setInterruptVBlank();

	void setInterruptLCD();
	void setInterruptTimer();
	void setInterruptSerial();
	void setInterruptJoypad();

	bool vramLocked = false;

	bool dmaPending = false;

	bool badWrite = false;

	void insertKeyboard(bool input[]);

	bool writeToDiv();
	bool writeToLYC();

private:



};
#endif // MEMORY_H
