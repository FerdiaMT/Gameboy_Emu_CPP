#include "SM83.h"
#include "Memory.h"
#include <cstdint>
#include <unordered_map>

registers reg;

// FFFFEEEE DDDDCCCC : Thinking about flags
//	        76543210	
//			znhc

/*
* Z ~ Zero flag (set if result of operation is 0)
*
* n ~ Subtraction flag (was previous instruction a subtraction)
* 
* h ~ Half Carry flag (carry for lower 4 bits of result (decimal nums))
* 
* C ~ Carry flag (when byte+byte goes above 0xFF / 16 bit / subtraction <0)
*/

/* sources:
* 
* (opcode info)
*  https://gbdev.io/gb-opcodes/optables/
*  https://rgbds.gbdev.io/docs/v0.9.1/gbz80.7
*  https://gbdev.io/pandocs/CPU_Instruction_Set.html
* 
* (emulation structure)
* 
*  https://cturt.github.io/cinoop.html
*/
std::unordered_map<uint8_t, uint8_t> opcodeCycles = { 


	//opcode , cycle (1 is 4 t cycles in docs)
	
	//0x
	{0x00, 1}, // NOP
	{0x01, 3}, // LD BC, n16
	{0x02, 2}, // LD [BC], A
	{0x03, 2}, // INC BC
	{0x04, 1}, // INC B
	{0x05, 1}, // DEC B
	{0x06, 2}, // LD B, n8
	{0x07, 1}, // RLCA
	{0x08, 5}, // LD [a16], SP
	{0x09, 2}, // ADD HL, BC
	{0x0A, 2}, // LD A, [BC]
	{0x0B, 2}, // DEC BC
	{0x0C, 1}, // INC C
	{0x0D, 1}, // DEC C
	{0x0E, 2}, // LD C, n8
	{0x0F, 1}, // RRCA

	//1x
	{0x10, 1}, // STOP n8
	{0x11, 3}, // LD DE, n16
	{0x12, 2}, // LD [DE], A
	{0x13, 2}, // INC DE
	{0x14, 1}, // INC D
	{0x15, 1}, // DEC D
	{0x16, 2}, // LD D, n8
	{0x17, 1}, // RLA
	{0x18, 3}, // JR, e8
	{0x19, 2}, // ADD HL, DE
	{0x1A, 2}, // LD A, [DE]
	{0x1B, 2}, // DEC DE
	{0x1C, 1}, // INC E
	{0x1D, 1}, // DEC E
	{0x1E, 2}, // LD E, n8
	{0x1F, 1}, // RRA
};

SM83::SM83(Memory& memory) : memory(memory){};

void setZeroFlag()
{		//     znhc
	reg.F |= 0b10000000;
}
void setNegFlag()
{		//     znhc
	reg.F |= 0b01000000;
}
void setHalfFlag()
{		//     znhc
	reg.F |= 0b00100000;
}
void setCarryFlag()
{		//     znhc
	reg.F |= 0b00010000;
}

void incByte(uint8_t& r8) //INC r8
{
	reg.F = 0b00000000;

	if (r8 == 0xFF) // if overflows
	{   //		   znhc
		reg.F |= 0b10010000; // carry / zero flag (only way to get to 0 is through overflow)
	}

	if ((r8 & 0x0F) == 0x0F) { // half carry
		reg.F |= 0b00100000; 
	}

	r8++;
}

void decByte(uint8_t& r8) {

	if (r8 == 0x01) 
	{   //		  znhc
		reg.F = 0b11000000;
	}
	else if (r8 == 0x00)
	{//		      znhc
		reg.F = 0b01010000;
	}
	else if ((r8 & 0x0F) == 0x00) //half carry
	{ //		  znhc
		reg.F = 0b01100000;
	}
	else {
		reg.F = 0b01000000;
	}

	r8--;
}

void addWordReg(uint16_t& r16a, uint16_t& r16b)
{
	reg.F = 0;
	uint16_t r16Val = r16a + r16b;
	if (r16a + r16b > 0xFFFF) {
		setCarryFlag();
	}
	if (r16Val == 0) {
		setZeroFlag();
	}
	if (((r16b & 0x0FFF) + (r16a & 0x0FFF)) > 0x0FFF) {
		setHalfFlag;  // Set Half Carry flag (bit 5)
	}
	r16a = r16Val;
}

void SM83::execute(uint8_t opcode)
{
	switch (opcode) {

	case 0x00: { // no op

	}break;
	case 0x01: { // load into bc, n16
		//load the next word into bc
		reg.BC = memory.readWord(reg.PC);
		//reg.PC += 2;
	}break;
	case 0x02: { // write into address BC, A
		memory.write(reg.BC, reg.A);
	}break;
	case 0x03: {
		reg.BC++;
	}break;
	case 0x04: {
		incByte(reg.B);
	}break;
	case 0x05: {
		decByte(reg.B);
	}break;
	case 0x06: {
		reg.B = memory.read(reg.PC);
		//reg.PC++;
	}break;
	case 0x07: { // rotate register A left 
		//set carry = A leftMost?
		reg.F = 0;
		reg.F |= ((reg.A & 0b10000000) >> 3); // grab most significant bit and position it at the carry flag
		reg.A = (reg.A << 1) & 0xFF;
	}break;
	case 0x08: { // load [a16] , SP
		uint16_t wordAdr = memory.readWord(reg.PC);
		//reg.PC += 2;
		memory.write(wordAdr, (reg.SP) & 0xFF);
		memory.write(wordAdr+1, (reg.SP>>8) & 0xFF);
	}break;
	case 0x09: {
		addWordReg(reg.HL, reg.BC);
	}break;
	case 0x0A: {// load into A, [bc]
		reg.A = memory.read(reg.BC);
	}break;
	case 0x0B: {
		reg.BC--;
	}break;
	case 0x0C: {
		incByte(reg.C);
	}break;
	case 0x0D: {
		decByte(reg.C);
	}break;
	case 0x0E: {
		reg.C = memory.read(reg.PC);
	}break;
	case 0x0F: {   //0b00000001
		reg.F = 0; //0bznhc0000
		reg.F |= ((reg.A & 0b1) << 4); 
		reg.A = (reg.A >> 1) & 0xFF;
	}break;


	default: {
		printf("No opcode implemented for : %d", opcode);
	}break;
	}
}



void SM83::reset()
{
	reg.PC = 0;
	memory.reset();
}

void SM83::executeCycle()
{
	uint8_t opcode = memory.read(reg.PC);
	cycles = opcodeCycles[opcode];

	execute(opcode);
}
