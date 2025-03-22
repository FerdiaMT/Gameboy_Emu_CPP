#ifndef SM83_H
#define SM83_H

#include <cstdint>
#include "Memory.h"

struct registers
{
	struct {
		union {
			struct {//little endian so f goes first
				uint8_t F;
				uint8_t A;
			};
			uint16_t AF;
		};
	};

	struct {
		union {
			struct {
				uint8_t C;
				uint8_t B;
			};
			uint16_t BC;
		};
	};

	struct {
		union {
			struct {
				uint8_t E;
				uint8_t D;
			};
			uint16_t DE;
		};
	};

	struct {
		union {
			struct {
				uint8_t L;
				uint8_t H;
			};
			uint16_t HL;
		};
	};

	uint16_t PC;
	uint16_t SP;
};

extern registers reg;

class SM83 {
public:
	SM83(Memory& memory);
	void reset();
	void executeCycle();
	void execute(uint8_t opcode);
	bool IME; // interrupt system

private:
	uint8_t cycles;
	Memory& memory;
	uint16_t popStack();
	void call(uint16_t jumpAddr);
	void push(uint16_t pushData);
};
#endif
