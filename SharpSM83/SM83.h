#ifndef SM83_H
#define SM83_H

#include <cstdint>
#include "Memory.h"


class SM83
{
public:

	union
	{
		struct
		{//little endian so f goes first
			uint8_t F;
			uint8_t A;
		};
		uint16_t AF;
	};

	union
	{
		struct
		{
			uint8_t C;
			uint8_t B;
		};
		uint16_t BC;
	};



	union
	{
		struct
		{
			uint8_t E;
			uint8_t D;
		};
		uint16_t DE;
	};


	union
	{
		struct
		{
			uint8_t L;
			uint8_t H;
		};
		uint16_t HL;
	};

	uint16_t PC;
	uint16_t SP;


	bool IME;
	bool IME_nextCycle;
	bool isHalted;
	int cycles;
	Memory* memory;
	SM83(Memory* memory);



	void handleInterrupts();

	bool getZ();
	bool getN();
	bool getH();
	bool getC();
	void setZ(bool v);
	void setN(bool v);
	void setH(bool v);
	void setC(bool v);

	uint8_t read8();
	uint16_t read16();
	void push(uint16_t val);
	uint16_t pop();

	int step();
	int execute(uint8_t opcode);
	int executeCB();
	uint8_t inc8(uint8_t val);
	uint8_t dec8(uint8_t val);
	void add8(uint8_t val);
	void adc8(uint8_t val);
	void sub8(uint8_t val);
	void sbc8(uint8_t val);
	void and8(uint8_t val);
	void xor8(uint8_t val);
	void or8(uint8_t val);
	void cp8(uint8_t val);
	void daa();
	uint16_t add16(uint16_t a, uint16_t b);
	uint16_t addSP();


private:

};
#endif