#include "SM83.h"
#include "Memory.h"
#include <cstdint>
#include <unordered_map>
#include <iostream>
#include <bitset>

registers reg;

// FFFFEEEE DDDDCCCC : Thinking about flags
//	        76543210	
//			znhc

//things to look into
// - on gbdev for  carry for inc/dec byte, 
// - on gbdev for zero in addTwoReg
// - stop and halt have a "low power mode" ? 0x10 0x76
// - 0xCB prefix mode : (
// IME 

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
//std::unordered_map<uint8_t, uint8_t> opcodeCycles = { };

uint8_t opcodeCycles[0x100]
{
	/////////////////////////////////////////////////////////////
	//*//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F   //x//
	/////////////////////////////////////////////////////////////
	/*0*/  1 ,3 ,2 ,2 ,1 ,1 ,2 ,1 ,5 ,2 ,2 ,2 ,1 ,1 ,2 ,1 , /*0*/
	/*1*/  1 ,3 ,2 ,2 ,1 ,1 ,2 ,1 ,3 ,2 ,2 ,2 ,1 ,1 ,2 ,1 , /*1*/
	/*2*/  2 ,3 ,2 ,2 ,1 ,1 ,2 ,1 ,2 ,2 ,2 ,2 ,1 ,1 ,2 ,1 , /*2*/
	/*3*/  2 ,3 ,2 ,2 ,3 ,3 ,3 ,1 ,2 ,2 ,2 ,2 ,1 ,1 ,2 ,1 , /*3*/
	/*4*/  1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 , /*4*/
	/*5*/  1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 , /*5*/
	/*6*/  1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 , /*6*/
	/*7*/  2 ,2 ,2 ,2 ,2 ,2 ,1 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 , /*7*/
	/*8*/  1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 , /*8*/
	/*9*/  1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 , /*9*/
	/*A*/  1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 , /*A*/
	/*B*/  1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,2 ,1 , /*B*/
	/*C*/  2 ,3 ,3 ,4 ,3 ,4 ,2 ,4 ,2 ,4 ,3 ,1 ,3 ,6 ,2 ,4 , /*C*/
	/*D*/  2 ,3 ,3 ,0 ,3 ,4 ,2 ,4 ,2 ,4 ,3 ,0 ,3 ,0 ,2 ,4 , /*D*/
	/*E*/  3 ,3 ,2 ,0 ,0 ,4 ,2 ,4 ,4 ,1 ,4 ,0 ,0 ,0 ,2 ,4 , /*E*/
	/*F*/  3 ,3 ,2 ,1 ,0 ,4 ,2 ,4 ,3 ,2 ,4 ,1 ,0 ,0 ,2 ,4 , /*F*/
	/////////////////////////////////////////////////////////////
	//x//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F   //*//
	/////////////////////////////////////////////////////////////
};

inline void SM83::addCycle()
{
	cycles ++;
}

inline void SM83::addCycle(uint8_t cyclesAdded)
{
	cycles += cyclesAdded ;
}


//the following add an extra cycle if a term is met
/*
*
* 20 , 28
* 30 , 38
* c0 , c2, c4, c8, cA , CC, 
* d0 , d2, d4, d8, dA , dC, 
* 
*/

SM83::SM83(Memory& memory) : memory(memory){};

inline void clearFlags() { //inline is replace the code directly during compile
	reg.F = 0;
}

inline void setZeroFlag()
{		//     znhc
	reg.F |= 0b10000000;
}
inline void unsetZeroFlag()
{
	reg.F &= 0b01111111;
}

inline void setNegFlag()
{		//     znhc
	reg.F |= 0b01000000;
}
inline void unsetNegFlag()
{
	reg.F &= 0b10111111; 
}

inline void setHalfFlag()
{		//     znhc
	reg.F |= 0b00100000;
}
inline void unsetHalfFlag()
{		//     znhc
	reg.F &= 0b11011111;
}

inline void setCarryFlag()
{		//     znhc
	reg.F |= 0b00010000;
}
inline void unsetCarryFlag()
{		//     znhc
	reg.F &= 0b11101111;
}

inline bool isZeroFlag()
{
	return (reg.F >> 7) & 0b1;
}
inline bool isNegFlag()
{
	return (reg.F >> 6) & 0b1;
}
inline bool isHalfFlag()
{
	return (reg.F >> 5) & 0b1;
}
inline bool isCarryFlag()
{
	return (reg.F >> 4) & 0b1;
}




void incByte(uint8_t& r8) //INC r8
{
	unsetNegFlag();
	if ((r8 & 0x0F) == 0x0F) setHalfFlag();
	r8++;
	if (r8 == 0) setZeroFlag(); 
}

void decByte(uint8_t& r8) 
{
	setNegFlag();
	if ((r8 & 0x0F) == 0) setHalfFlag();
	r8--;
	if (r8 == 0) setZeroFlag(); 
}

void addWordReg(uint16_t& r16a, uint16_t r16b)
{
	clearFlags();

	uint16_t r16Val = r16a + r16b;
	if (r16a + r16b > 0xFFFF) {
		setCarryFlag();
	}
	if (r16Val == 0) {
		setZeroFlag();
	}
	if (((r16b & 0xFFF) + (r16a & 0xFFF)) > 0xFFF) {
		setHalfFlag();  // Set Half Carry flag (bit 5)
	}
	r16a = r16Val;
}

void addWordRegSigned(uint16_t& r16a, int8_t r8b)
{
	clearFlags();

	uint16_t r16Val = r16a + r8b;
	if (r16a + r8b > 0xFFFF) {
		setCarryFlag();
	}
	if (r16Val == 0) {
		setZeroFlag();
	}
	if (((r8b & 0xFFF) + (r16a & 0xFFF)) > 0xFFF) {
		setHalfFlag();  // Set Half Carry flag (bit 5)
	}
	r16a = r16Val;
}

void addByteReg(uint8_t& r8a , uint8_t r8b)
{
	clearFlags();
	uint16_t r16= r8a + r8b;

	if (r16 & 0xFF00) setCarryFlag();
	if ((r8a & 0x0F) + (r8b & 0x0F) > 0x0F) setHalfFlag();
	if ((uint8_t)r16 == 0) setZeroFlag();

	r8a = (uint8_t)r16;
}

void addCarryByteReg(uint8_t& r8a, uint8_t r8b)
{
	uint8_t carry = isCarryFlag() ? 1 : 0;
    clearFlags();
    uint16_t r16 = r8a + r8b + carry;

    if (r16 & 0xFF00) setCarryFlag(); 
    if ((r8a & 0x0F) + (r8b & 0x0F) + carry > 0x0F) setHalfFlag();  
    if ((uint8_t)r16 == 0) setZeroFlag(); 

    r8a = (uint8_t)r16;
}

void subByteReg(uint8_t& r8a , uint8_t r8b)
{
	clearFlags();
	setNegFlag(); 
	if (r8b > r8a) setCarryFlag(); 
	if ((r8a & 0x0F) < (r8b & 0x0F)) setHalfFlag();
	r8a -= r8b;
	if (r8a == 0) setZeroFlag(); 
}

void subCarryByteReg(uint8_t& r8a, uint8_t r8b)
{
	uint8_t carry = isCarryFlag() ? 1 : 0;
	clearFlags();
	setNegFlag();

	uint16_t r16 = r8a - r8b - carry;

	if (r8b + carry > r8a) setCarryFlag();
	if ((r8a & 0x0F) < (r8b & 0x0F) + carry) setHalfFlag();
	if ((uint8_t)r16 == 0) setZeroFlag();

	r8a = (uint8_t)r16;
}

void andByteReg(uint8_t& r8a, uint8_t r8b)
{
	clearFlags();
	setHalfFlag();
	r8a = r8a & r8b;
	if (r8a == 0x0)setZeroFlag();
}

void xorByteReg(uint8_t& r8a, uint8_t r8b)
{
	clearFlags();
	r8a ^= r8b;
	if (r8a == 0x0)setZeroFlag();
}

void orByteReg(uint8_t& r8a, uint8_t r8b)
{
	clearFlags();
	r8a = r8a | r8b;
	if (r8a == 0x0)setZeroFlag();
}

void compareByteReg(uint8_t r8a, uint8_t r8b)//this discards the result
{//basically only cares about flags
	clearFlags();
	setNegFlag();
	uint8_t r8Val = r8a - r8b;
	if (r8b > r8a)
	{
		setCarryFlag();
	}
	if (r8Val == 0)
	{
		setZeroFlag();
	}
	if ((r8a & 0x0F) < (r8b & 0x0F)) {
		setHalfFlag();
	}
}





uint16_t SM83::popStack()
{
	uint16_t word  = memory.readWord(reg.SP);
	reg.SP += 2;
	return word;
}

void SM83::call(uint16_t jumpAddr)
{
	reg.SP -= 2;//minus two bytes for word
	memory.writeWord(reg.SP, reg.PC); //T HIS MIGHT NEED TO BE PC
	reg.PC = jumpAddr;
}
void SM83::push(uint16_t pushData)
{
	reg.SP-=2;
	memory.writeWord(reg.SP, pushData);
}


void rlc(uint8_t& r8) // rotate left, r8  just bitshifted no other change
{
	clearFlags();
	reg.F = ((r8 & 0b10000000) >> 3); // grab most significant bit and position it at the carry flag
	r8 = (r8 << 1) & 0xFF;
	if (r8 == 0) setZeroFlag();
}

void rl(uint8_t& r8) // rotate left, r8 dig 1 = prev carry flag
{
	uint8_t Cbit = (reg.F & 0b00010000) >> 4; // if carry is present set it otherwise dont
	clearFlags();
	reg.F = ((r8 & 0b10000000) >> 3); // grab most significant bit and position it at the carry flag
	r8 = ((r8 << 1) | Cbit) & 0xFF;
	if (r8 == 0) setZeroFlag();
}

void rrc(uint8_t& r8)
{
	clearFlags();
	reg.F = ((r8 & 0b1) << 4);
	r8 = (r8 >> 1) & 0xFF;
	if (r8 == 0) setZeroFlag();
}

void rr(uint8_t& r8)
{
	uint8_t Cbit = (reg.F & 0b00010000) << 3;
	clearFlags();
	reg.F = ((r8 & 0b1) << 4);// grab most significant bit and position it at the carry flag
	r8 = ((r8 >> 1) | Cbit) & 0xFF;
	if (r8 == 0) setZeroFlag();
}

void sla(uint8_t& r8)
{
	clearFlags();
	if ((r8 & 0b10000000) >> 7) setCarryFlag();
	r8 = (r8 << 1)  & 0b11111110; //force last val into 0
	if (r8 == 0) setZeroFlag();
}

void sra(uint8_t& r8)
{
	clearFlags();
	if (r8 & 0b00000001) setCarryFlag();
	r8 = (r8 >> 1) & (r8 & 0b10000000) & 0xFF;
	if (r8 == 0) setZeroFlag();
}

void swap(uint8_t& r8) 
{
	clearFlags();
	r8 = ((r8 >> 4) & 0x0F) | ((r8 & 0x0F) << 4);
	if (r8 == 0) setZeroFlag();
}

void srl(uint8_t& r8)
{
	clearFlags();
	if (r8 & 0b00000001) setCarryFlag();
	r8 = (r8 >> 1) & 0b01111111;
	if (r8 == 0) setZeroFlag();
}

void bit(uint8_t u3, uint8_t r8)
{
	unsetNegFlag();
	setHalfFlag();
	if (r8 & (1 << u3)) // if u3 bit is set
	{
		unsetZeroFlag();
	}
	else
	{
		setZeroFlag();
	}
}

inline void res(uint8_t u3, uint8_t& r8)
{
	r8 &= ~(1 << u3);
}

inline void set(uint8_t u3, uint8_t& r8)
{
	r8 |= (1 << u3);
}

void SM83::executePrefix(uint8_t opcode)
{
	switch (opcode) {

	case 0x00: {
		rlc(reg.B);
	} break;
	case 0x01: {
		rlc(reg.C);
	} break;
	case 0x02: {
		rlc(reg.D);
	} break;
	case 0x03: {
		rlc(reg.E);
	} break;
	case 0x04: {
		rlc(reg.H);
	} break;
	case 0x05: {
		rlc(reg.L);
	} break;
	case 0x06: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		rlc(a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x07: {
		rlc(reg.A);
	} break;
	case 0x08: {
		rrc(reg.B);
	} break;
	case 0x09: {
		rrc(reg.C);
	} break;
	case 0x0A: {
		rrc(reg.D);
	} break;
	case 0x0B: {
		rrc(reg.E);
	} break;
	case 0x0C: {
		rrc(reg.H);
	} break;
	case 0x0D: {
		rrc(reg.L);
	} break;
	case 0x0E: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		rrc(a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x0F: {
		rrc(reg.A);
	} break;
	//line 1 ===RL RR================================================
	case 0x10: {
		rl(reg.B);
	} break;
	case 0x11: {
		rl(reg.C);
	} break;
	case 0x12: {
		rl(reg.D);
	} break;
	case 0x13: {
		rl(reg.E);
	} break;
	case 0x14: {
		rl(reg.H);
	} break;
	case 0x15: {
		rl(reg.L);
	} break;
	case 0x16: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		rl(a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x17: {
		rl(reg.A);
	} break;
	case 0x18: {
		rr(reg.B);
	} break;
	case 0x19: {
		rr(reg.C);
	} break;
	case 0x1A: {
		rr(reg.D);
	} break;
	case 0x1B: {
		rr(reg.E);
	} break;
	case 0x1C: {
		rr(reg.H);
	} break;
	case 0x1D: {
		rr(reg.L);
	} break;
	case 0x1E: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		rr(a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x1F: {
		rr(reg.A);
	} break;
//line 2 ===SLA SRA================================================
	
	case 0x20: {
		sla(reg.B);
	} break;
	case 0x21: {
		sla(reg.C);
	} break;
	case 0x22: {
		sla(reg.D);
	} break;
	case 0x23: {
		sla(reg.E);
	} break;
	case 0x24: {
		sla(reg.H);
	} break;
	case 0x25: {
		sla(reg.L);
	} break;
	case 0x26: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		sla(a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x27: {
		sla(reg.A);
	} break;
	case 0x28: {
		sra(reg.B);
	} break;
	case 0x29: {
		sra(reg.C);
	} break;
	case 0x2A: {
		sra(reg.D);
	} break;
	case 0x2B: {
		sra(reg.E);
	} break;
	case 0x2C: {
		sra(reg.H);
	} break;
	case 0x2D: {
		sra(reg.L);
	} break;
	case 0x2E: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		sra(a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x2F: {
		sra(reg.A);
	} break;
//line 3 ===SWAP SRL================================================

	case 0x30: {
		swap(reg.B);
	} break;
	case 0x31: {
		swap(reg.C);
	} break;
	case 0x32: {
		swap(reg.D);
	} break;
	case 0x33: {
		swap(reg.E);
	} break;
	case 0x34: {
		swap(reg.H);
	} break;
	case 0x35: {
		swap(reg.L);
	} break;
	case 0x36: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		swap(a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x37: {
		swap(reg.A);
	} break;
	case 0x38: {
		srl(reg.B);
	} break;
	case 0x39: {
		srl(reg.C);
	} break;
	case 0x3A: {
		srl(reg.D);
	} break;
	case 0x3B: {
		srl(reg.E);
	} break;
	case 0x3C: {
		srl(reg.H);
	} break;
	case 0x3D: {
		srl(reg.L);
	} break;
	case 0x3E: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		srl(a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x3F: {
		srl(reg.A);
	} break;
//	line 4 bit (0x6, 0xe should be 1 not 2) =======================
	
	case 0x40: {
		bit(0,reg.B);
	} break;
	case 0x41: {
		bit(0, reg.C);
	} break;
	case 0x42: {
		bit(0, reg.D);
	} break;
	case 0x43: {
		bit(0, reg.E);
	} break;
	case 0x44: {
		bit(0, reg.H);
	} break;
	case 0x45: {
		bit(0, reg.L);
	} break;
	case 0x46: {// whatever hl is pointing to
		addCycle();
		uint8_t a8 = memory.view(reg.HL);
		bit(0, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x47: {
		bit(0, reg.A);
	} break;
	case 0x48: {
		bit(1, reg.B);
	} break;
	case 0x49: {
		bit(1, reg.C);
	} break;
	case 0x4A: {
		bit(1, reg.D);
	} break;
	case 0x4B: {
		bit(1, reg.E);
	} break;
	case 0x4C: {
		bit(1, reg.H);
	} break;
	case 0x4D: {
		bit(1, reg.L);
	} break;
	case 0x4E: {// whatever hl is pointing to
		addCycle();
		uint8_t a8 = memory.view(reg.HL);
		bit(1, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x4F: {
		bit(1, reg.A);
	} break;
//	line 5 bit (0x6, 0xe should be 1 not 2) =======================

	case 0x50: {
		bit(2, reg.B);
	} break;
	case 0x51: {
		bit(2, reg.C);
	} break;
	case 0x52: {
		bit(2, reg.D);
	} break;
	case 0x53: {
		bit(2, reg.E);
	} break;
	case 0x54: {
		bit(2, reg.H);
	} break;
	case 0x55: {
		bit(2, reg.L);
	} break;
	case 0x56: {// whatever hl is pointing to
		addCycle();
		uint8_t a8 = memory.view(reg.HL);
		bit(2, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x57: {
		bit(2, reg.A);
	} break;
	case 0x58: {
		bit(3, reg.B);
	} break;
	case 0x59: {
		bit(3, reg.C);
	} break;
	case 0x5A: {
		bit(3, reg.D);
	} break;
	case 0x5B: {
		bit(3, reg.E);
	} break;
	case 0x5C: {
		bit(3, reg.H);
	} break;
	case 0x5D: {
		bit(3, reg.L);
	} break;
	case 0x5E: {// whatever hl is pointing to
		addCycle();
		uint8_t a8 = memory.view(reg.HL);
		bit(3, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x5F: {
		bit(3, reg.A);
	} break;

//	line 6 bit (0x6, 0xe should be 1 not 2) =======================

	case 0x60: {
		bit(4, reg.B);
	} break;
	case 0x61: {
		bit(4, reg.C);
	} break;
	case 0x62: {
		bit(4, reg.D);
	} break;
	case 0x63: {
		bit(4, reg.E);
	} break;
	case 0x64: {
		bit(4, reg.H);
	} break;
	case 0x65: {
		bit(4, reg.L);
	} break;
	case 0x66: {// whatever hl is pointing to
		addCycle();
		uint8_t a8 = memory.view(reg.HL);
		bit(4, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x67: {
		bit(4, reg.A);
	} break;
	case 0x68: {
		bit(5, reg.B);
	} break;
	case 0x69: {
		bit(5, reg.C);
	} break;
	case 0x6A: {
		bit(5, reg.D);
	} break;
	case 0x6B: {
		bit(5, reg.E);
	} break;
	case 0x6C: {
		bit(5, reg.H);
	} break;
	case 0x6D: {
		bit(5, reg.L);
	} break;
	case 0x6E: {// whatever hl is pointing to
		addCycle();
		uint8_t a8 = memory.view(reg.HL);
		bit(5, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x6F: {
		bit(5, reg.A);
	} break;

//	line 7 bit (0x6, 0xe should be 1 not 2) =======================

	case 0x70: {
		bit(6, reg.B);
	} break;
	case 0x71: {
		bit(6, reg.C);
	} break;
	case 0x72: {
		bit(6, reg.D);
	} break;
	case 0x73: {
		bit(6, reg.E);
	} break;
	case 0x74: {
		bit(6, reg.H);
	} break;
	case 0x75: {
		bit(6, reg.L);
	} break;
	case 0x76: {// whatever hl is pointing to
		addCycle();
		uint8_t a8 = memory.view(reg.HL);
		bit(6, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x77: {
		bit(6, reg.A);
	} break;
	case 0x78: {
		bit(7, reg.B);
	} break;
	case 0x79: {
		bit(7, reg.C);
	} break;
	case 0x7A: {
		bit(7, reg.D);
	} break;
	case 0x7B: {
		bit(7, reg.E);
	} break;
	case 0x7C: {
		bit(7, reg.H);
	} break;
	case 0x7D: {
		bit(7, reg.L);
	} break;
	case 0x7E: {// whatever hl is pointing to
		addCycle();
		uint8_t a8 = memory.view(reg.HL);
		bit(7, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x7F: {
		bit(7, reg.A);
	} break;

//	line 8 bit (0x6, 0xe should be 2 not 1) =======================

	case 0x80: {
		res(0, reg.B);
	} break;
	case 0x81: {
		res(0, reg.C);
	} break;
	case 0x82: {
		res(0, reg.D);
	} break;
	case 0x83: {
		res(0, reg.E);
	} break;
	case 0x84: {
		res(0, reg.H);
	} break;
	case 0x85: {
		res(0, reg.L);
	} break;
	case 0x86: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		res(0, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x87: {
		res(0, reg.A);
	} break;
	case 0x88: {
		res(1, reg.B);
	} break;
	case 0x89: {
		res(1, reg.C);
	} break;
	case 0x8A: {
		res(1, reg.D);
	} break;
	case 0x8B: {
		res(1, reg.E);
	} break;
	case 0x8C: {
		res(1, reg.H);
	} break;
	case 0x8D: {
		res(1, reg.L);
	} break;
	case 0x8E: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		res(1, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x8F: {
		res(1, reg.A);
	} break;

//	line 9 bit (0x6, 0xe should be 2 not 1) =======================

	case 0x90: {
		res(2, reg.B);
	} break;
	case 0x91: {
		res(2, reg.C);
	} break;
	case 0x92: {
		res(2, reg.D);
	} break;
	case 0x93: {
		res(2, reg.E);
	} break;
	case 0x94: {
		res(2, reg.H);
	} break;
	case 0x95: {
		res(2, reg.L);
	} break;
	case 0x96: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		res(0, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x97: {
		res(2, reg.A);
	} break;
	case 0x98: {
		res(3, reg.B);
	} break;
	case 0x99: {
		res(3, reg.C);
	} break;
	case 0x9A: {
		res(3, reg.D);
	} break;
	case 0x9B: {
		res(3, reg.E);
	} break;
	case 0x9C: {
		res(3, reg.H);
	} break;
	case 0x9D: {
		res(3, reg.L);
	} break;
	case 0x9E: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		res(3, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0x9F: {
		res(3, reg.A);
	} break;
//	line A bit (0x6, 0xe should be 2 not 1) =======================

	case 0xA0: {
		res(4, reg.B);
	} break;
	case 0xA1: {
		res(4, reg.C);
	} break;
	case 0xA2: {
		res(4, reg.D);
	} break;
	case 0xA3: {
		res(4, reg.E);
	} break;
	case 0xA4: {
		res(4, reg.H);
	} break;
	case 0xA5: {
		res(4, reg.L);
	} break;
	case 0xA6: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		res(4, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xA7: {
		res(4, reg.A);
	} break;
	case 0xA8: {
		res(5, reg.B);
	} break;
	case 0xA9: {
		res(5, reg.C);
	} break;
	case 0xAA: {
		res(5, reg.D);
	} break;
	case 0xAB: {
		res(5, reg.E);
	} break;
	case 0xAC: {
		res(5, reg.H);
	} break;
	case 0xAD: {
		res(5, reg.L);
	} break;
	case 0xAE: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		res(5, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xAF: {
		res(5, reg.A);
	} break;

//	line b bit (0x6, 0xe should be 2 not 1) =======================

	case 0xB0: {
		res(6, reg.B);
	} break;
	case 0xB1: {
		res(6, reg.C);
	} break;
	case 0xB2: {
		res(6, reg.D);
	} break;
	case 0xB3: {
		res(6, reg.E);
	} break;
	case 0xB4: {
		res(6, reg.H);
	} break;
	case 0xB5: {
		res(6, reg.L);
	} break;
	case 0xB6: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		res(6, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xB7: {
		res(6, reg.A);
	} break;
	case 0xB8: {
		res(7, reg.B);
	} break;
	case 0xB9: {
		res(7, reg.C);
	} break;
	case 0xBA: {
		res(7, reg.D);
	} break;
	case 0xBB: {
		res(7, reg.E);
	} break;
	case 0xBC: {
		res(7, reg.H);
	} break;
	case 0xBD: {
		res(7, reg.L);
	} break;
	case 0xBE: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		res(7, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xBF: {
		res(7, reg.A);
	} break;

//	line c bit (0x6, 0xe should be 2 not 1) =======================

	case 0xC0: {
		set(0, reg.B);
	} break;
	case 0xC1: {
		set(0, reg.C);
	} break;
	case 0xC2: {
		set(0, reg.D);
	} break;
	case 0xC3: {
		set(0, reg.E);
	} break;
	case 0xC4: {
		set(0, reg.H);
	} break;
	case 0xC5: {
		set(0, reg.L);
	} break;
	case 0xC6: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		set(0, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xC7: {
		set(0, reg.A);
	} break;
	case 0xC8: {
		set(1, reg.B);
	} break;
	case 0xC9: {
		set(1, reg.C);
	} break;
	case 0xCA: {
		set(1, reg.D);
	} break;
	case 0xCB: {
		set(1, reg.E);
	} break;
	case 0xCC: {
		set(1, reg.H);
	} break;
	case 0xCD: {
		set(1, reg.L);
	} break;
	case 0xCE: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		set(1, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xCF: {
		set(1, reg.A);
	} break;

//	line D bit (0x6, 0xe should be 2 not 1) =======================

	case 0xD0: {
		set(2, reg.B);
	} break;
	case 0xD1: {
		set(2, reg.C);
	} break;
	case 0xD2: {
		set(2, reg.D);
	} break;
	case 0xD3: {
		set(2, reg.E);
	} break;
	case 0xD4: {
		set(2, reg.H);
	} break;
	case 0xD5: {
		set(2, reg.L);
	} break;
	case 0xD6: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		set(2, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xD7: {
		set(2, reg.A);
	} break;
	case 0xD8: {
		set(3, reg.B);
	} break;
	case 0xD9: {
		set(3, reg.C);
	} break;
	case 0xDA: {
		set(3, reg.D);
	} break;
	case 0xDB: {
		set(3, reg.E);
	} break;
	case 0xDC: {
		set(3, reg.H);
	} break;
	case 0xDD: {
		set(3, reg.L);
	} break;
	case 0xDE: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		set(3, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xDF: {
		set(3, reg.A);
	} break;

//	line E bit (0x6, 0xe should be 2 not 1) =======================

	case 0xE0: {
		set(4, reg.B);
	} break;
	case 0xE1: {
		set(4, reg.C);
	} break;
	case 0xE2: {
		set(4, reg.D);
	} break;
	case 0xE3: {
		set(4, reg.E);
	} break;
	case 0xE4: {
		set(4, reg.H);
	} break;
	case 0xE5: {
		set(4, reg.L);
	} break;
	case 0xE6: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		set(4, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xE7: {
		set(4, reg.A);
	} break;
	case 0xE8: {
		set(5, reg.B);
	} break;
	case 0xE9: {
		set(5, reg.C);
	} break;
	case 0xEA: {
		set(5, reg.D);
	} break;
	case 0xEB: {
		set(5, reg.E);
	} break;
	case 0xEC: {
		set(5, reg.H);
	} break;
	case 0xED: {
		set(5, reg.L);
	} break;
	case 0xEE: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		set(5, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xEF: {
		set(5, reg.A);
	} break;

//	line F bit (0x6, 0xe should be 2 not 1) =======================

	case 0xF0: {
		set(6, reg.B);
	} break;
	case 0xF1: {
		set(6, reg.C);
	} break;
	case 0xF2: {
		set(6, reg.D);
	} break;
	case 0xF3: {
		set(6, reg.E);
	} break;
	case 0xF4: {
		set(6, reg.H);
	} break;
	case 0xF5: {
		set(6, reg.L);
	} break;
	case 0xF6: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		set(6, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xF7: {
		set(6, reg.A);
	} break;
	case 0xF8: {
		set(7, reg.B);
	} break;
	case 0xF9: {
		set(7, reg.C);
	} break;
	case 0xFA: {
		set(7, reg.D);
	} break;
	case 0xFB: {
		set(7, reg.E);
	} break;
	case 0xFC: {
		set(7, reg.H);
	} break;
	case 0xFD: {
		set(7, reg.L);
	} break;
	case 0xFE: {// whatever hl is pointing to
		addCycle(2);
		uint8_t a8 = memory.view(reg.HL);
		set(7, a8);
		memory.write(reg.HL, a8);
	} break;
	case 0xFF: {
		set(7, reg.A);
	} break;


	default: {

	} break;


	}
}

void SM83::execute(uint8_t opcode)
{
	switch (opcode) {

	case 0x00: { // no op

	}break;
	case 0x01: { // load into bc, n16
		reg.BC = memory.readWord(reg.PC); reg.PC+=2;
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
		reg.B = memory.read(reg.PC); reg.PC++;
		//reg.PC++;
	}break;
	case 0x07: { // rotate register A left 
		rlc(reg.A);
	}break;
	case 0x08: { // load [a16] , SP
		uint16_t wordAdr = memory.readWord(reg.PC); reg.PC+=2;
		memory.write(wordAdr, (reg.SP) & 0xFF);
		memory.write(wordAdr+1, (reg.SP>>8) & 0xFF);
	}break;
	case 0x09: {
		addWordReg(reg.HL, reg.BC);
	}break;
	case 0x0A: {// load into A, [bc]
		reg.A = memory.read(reg.BC); reg.BC++;
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
		reg.C = memory.read(reg.PC); reg.PC++;
	}break;
	case 0x0F: {   //0b00000001
		rrc(reg.A);
	}break;

	//====0x1?===================================================

	case 0x10: { // STOP, this is a funny one TODO: look into this in pandocs
		//i think we take in another byte ?
		//make it so as long as stop mode is on, div timer cant increment and is reset to 0
		memory.write(0xFF04, 0);

		memory.read(reg.PC); reg.PC++;

	}break;
	case 0x11: { // load into bc, n16
		reg.DE = memory.readWord(reg.PC); reg.PC+=2;
	}break;
	case 0x12: { // write into address BC, A
		memory.write(reg.DE, reg.A);
	}break;
	case 0x13: {
		reg.DE++;
	}break;
	case 0x14: {
		incByte(reg.D);
	}break;
	case 0x15: {
		decByte(reg.D);
	}break;
	case 0x16: {
		reg.D = memory.read(reg.PC); reg.PC++;
	}break;
	case 0x17: { // rotate register A left ,carry goes into a
		rl(reg.A);
	}break;
	case 0x18: { // jump to e8

		int8_t offset = memory.read(reg.PC); reg.PC++; // pc = 129
		reg.PC += offset;

	}break;
	case 0x19: {
		addWordReg(reg.HL, reg.DE);
	}break;
	case 0x1A: {
		reg.A = memory.read(reg.DE); reg.DE++;
	}break;
	case 0x1B: {
		reg.DE--;
	}break;
	case 0x1C: {
		incByte(reg.E);
	}break;
	case 0x1D: {
		decByte(reg.E);
	}break;
	case 0x1E: {
		reg.E = memory.read(reg.PC) ; reg.PC++;
	}break;
	case 0x1F: {   //RRA //0bznhc0000
		rr(reg.A);
	}break;

	//====0x2?===================================================

	case 0x20: { // JR NZ , e8
		// jump to e8 if NotZ is met
		int8_t offset = memory.read(reg.PC);  reg.PC++; // pc = 129
		if (!isZeroFlag())
		{
			addCycle();
			reg.PC += offset;
		}
		//std::cout << "offset is " << (int)offset << " with pc now equal to " << int(reg.PC) << std::endl;
	}break;
	case 0x21: { // load into bc, n16
		reg.HL = memory.readWord(reg.PC); reg.PC+=2;
	}break;
	case 0x22: { // write into address BC, A
		memory.write(reg.HL, reg.A);
		reg.HL++;
	}break;
	case 0x23: {
		reg.HL++;
	}break;
	case 0x24: {
		incByte(reg.H);
	}break;
	case 0x25: {
		decByte(reg.H);
	}break;
	case 0x26: {
		reg.H = memory.read(reg.PC); reg.PC++;
	}break;
	case 0x27: { // DAA (confusing one)
		//check if n is set
		//0bznhc0000
		//   x
		bool setCarry = false;
		if (isNegFlag()) //if neg flag is set
		{
			uint8_t adjustment = 0;
			if (isHalfFlag())//half carry is set
			{
				adjustment += 0x6;
			}
			if (isCarryFlag()) {//carry flag
				adjustment += 0x60;
			}
			reg.A -= adjustment;

		}
		else {
			uint8_t adjustment = 0;
			if (isHalfFlag() || (reg.A & 0xF) > 0x9)
			{
				adjustment += 0x6;
			}
			if (isCarryFlag() || (reg.A > 0x99)) {//carry flag
				adjustment += 0x60;
				setCarry = true;
			}
			reg.A += adjustment;
		}
		reg.F &= 0b01000000;
		if (reg.A == 0)
		{
			setZeroFlag();
		}
		if (setCarry)
		{
			setCarryFlag();
		}

	}break;
	case 0x28: { // jump to e8

		int8_t offset = memory.read(reg.PC);  reg.PC++; // pc = 129
		if (isZeroFlag())
		{
			addCycle();
			reg.PC += offset;
		}

	}break;
	case 0x29: {
		addWordReg(reg.HL, reg.HL);
	}break;
	case 0x2A: {
		reg.A = memory.read(reg.HL); reg.HL++;
	}break;
	case 0x2B: {
		reg.HL--;
	}break;
	case 0x2C: {
		incByte(reg.L);
	}break;
	case 0x2D: {
		decByte(reg.L);
	}break;
	case 0x2E: {
		reg.L = memory.read(reg.PC);  reg.PC++;
	}break;
	case 0x2F: {   // COMPLEMENT
		setNegFlag();
		setHalfFlag();
		reg.A = ~reg.A;

	}break;

	//====0x3?===================================================

	case 0x30: { // JR Z , e8
		int8_t offset = memory.read(reg.PC);  reg.PC++; // pc = 129
		if (!isCarryFlag())
		{
			addCycle();
			reg.PC += offset;
		}
	}break;
	case 0x31: { // load into bc, n16
		reg.SP = memory.readWord(reg.PC); reg.PC+=2;
	}break;
	case 0x32: { // write into address BC, A
		memory.write(reg.HL, reg.A);
		reg.HL--;
	}break;
	case 0x33: {
		reg.SP++;
	}break;
	case 0x34: { // increment whatever hl is pointing to by 1
		uint8_t byte = memory.view(reg.HL); // this one wont increment
		incByte(byte);
		memory.write(reg.HL, byte);
		//UNSURE IF I SHOULD INCREMENT PROGRAM COUNTER FROM HERE 
	}break;
	case 0x35: {
		uint8_t byte = memory.view(reg.HL); // this one wont increment
		decByte(byte);
		memory.write(reg.HL, byte);
	}break;
	case 0x36: {
		memory.write(reg.HL, memory.read(reg.PC));  reg.PC++;// load into address hl, data held in next pc
	}break;
	case 0x37: { // SCF (confusing one)
		bool z = isZeroFlag;
		clearFlags();
		setCarryFlag();
		if (z) setZeroFlag;
	}break;
	case 0x38: { // jump to e8

		int8_t offset = memory.read(reg.PC);  reg.PC++; // pc = 129
		if (isCarryFlag())
		{
			addCycle();
			reg.PC += offset;
		}

	}break;
	case 0x39: {
		addWordReg(reg.HL, reg.SP);
	}break;
	case 0x3A: {
		reg.A = memory.read(reg.HL);  reg.HL++;
		reg.HL--;
	}break;
	case 0x3B: {
		reg.SP--;
	}break;
	case 0x3C: {
		incByte(reg.A);
	}break;
	case 0x3D: {
		decByte(reg.A);
	}break;
	case 0x3E: {
		reg.A = memory.read(reg.PC);  reg.PC++;
	}break;
	case 0x3F: {   // CCF

		bool z = isZeroFlag();
		bool c = isCarryFlag();
		clearFlags();
		if (z) setZeroFlag;
		if (!c)setCarryFlag();
	}break;

	//====0x4?========THIS IS START OF LD A B======================================

	case 0x40: { 
		reg.B = reg.B;
	} break;
	case 0x41: {
		reg.B = reg.C;
	} break;
	case 0x42: {
		reg.B = reg.D;
	} break;
	case 0x43: {
		reg.B = reg.E;
	} break;
	case 0x44: {
		reg.B = reg.H;
	} break;
	case 0x45: {
		reg.B = reg.L;
	} break;
	case 0x46: {//hl pointer 
		reg.B = memory.view(reg.HL);
	} break;
	case 0x47: {
		reg.B = reg.A;
	} break;
	case 0x48: {
		reg.C = reg.B;
	} break;
	case 0x49: {
		reg.C = reg.C;
	} break;
	case 0x4A: {
		reg.C = reg.D;
	} break;
	case 0x4B: {
		reg.C = reg.E;
	} break;
	case 0x4C: {
		reg.C = reg.H;
	} break;
	case 0x4D: {
		reg.C = reg.L;
	} break;
	case 0x4E: {
		reg.C = memory.view(reg.HL);
	} break;
	case 0x4F: {
		reg.C = reg.A;
	} break;

	//====0x5?==================================================================

	case 0x50: {
		reg.D = reg.B;
	} break;
	case 0x51: {
		reg.D = reg.C;
	} break;
	case 0x52: {
		reg.D = reg.D;
	} break;
	case 0x53: {
		reg.D = reg.E;
	} break;
	case 0x54: {
		reg.D = reg.H;
	} break;
	case 0x55: {
		reg.D = reg.L;
	} break;
	case 0x56: {//hl pointer 
		reg.D = memory.view(reg.HL);
	} break;
	case 0x57: {
		reg.D = reg.A;
	} break;
	case 0x58: {
		reg.E = reg.B;
	} break;
	case 0x59: {
		reg.E = reg.C;
	} break;
	case 0x5A: {
		reg.E = reg.D;
	} break;
	case 0x5B: {
		reg.E = reg.E;
	} break;
	case 0x5C: {
		reg.E = reg.H;
	} break;
	case 0x5D: {
		reg.E = reg.L;
	} break;
	case 0x5E: {
		reg.E = memory.view(reg.HL);
	} break;
	case 0x5F: {
		reg.E = reg.A;
	} break;

	//====0x6?==================================================================

	case 0x60: {
		reg.H = reg.B;
	} break;
	case 0x61: {
		reg.H = reg.C;
	} break;
	case 0x62: {
		reg.H = reg.D;
	} break;
	case 0x63: {
		reg.H = reg.E;
	} break;
	case 0x64: {
		reg.H = reg.H;
	} break;
	case 0x65: {
		reg.H = reg.L;
	} break;
	case 0x66: {//hl pointer 
		reg.H = memory.view(reg.HL);
	} break;
	case 0x67: {
		reg.H = reg.A;
	} break;
	case 0x68: {
		reg.L = reg.B;
	} break;
	case 0x69: {
		reg.L = reg.C;
	} break;
	case 0x6A: {
		reg.L = reg.D;
	} break;
	case 0x6B: {
		reg.L = reg.E;
	} break;
	case 0x6C: {
		reg.L = reg.H;
	} break;
	case 0x6D: {
		reg.L = reg.L;
	} break;
	case 0x6E: {
		reg.L = memory.view(reg.HL);
	} break;
	case 0x6F: {
		reg.L = reg.A;
	} break;

	//====0x7?==================================================================

	case 0x70: {
		memory.write(reg.HL, reg.B);
	} break;
	case 0x71: {
		memory.write(reg.HL, reg.C);
	} break;
	case 0x72: {
		memory.write(reg.HL, reg.D);
	} break;
	case 0x73: {
		memory.write(reg.HL, reg.E);
	} break;
	case 0x74: {
		memory.write(reg.HL, reg.H);
	} break;
	case 0x75: {
		memory.write(reg.HL, reg.L);
	} break;
	case 0x76: {// HALT
		//DO STUFF HERE (IME MODE)
		isHalted = true;
	} break;
	case 0x77: {
		memory.write(reg.HL, reg.A);
	} break;
	case 0x78: {
		reg.A = reg.B;
	} break;
	case 0x79: {
		reg.A = reg.C;
	} break;
	case 0x7A: {
		reg.A = reg.D;
	} break;
	case 0x7B: {
		reg.A = reg.E;
	} break;
	case 0x7C: {
		reg.A = reg.H;
	} break;
	case 0x7D: {
		reg.A = reg.L;
	} break;
	case 0x7E: {
		reg.A = memory.view(reg.HL);
	} break;
	case 0x7F: {
		reg.A = reg.A;
	} break;

	//====0x8?==================================================================

	case 0x80: {
		addByteReg(reg.A, reg.B);
	} break;
	case 0x81: {
		addByteReg(reg.A, reg.C);
	} break;
	case 0x82: {
		addByteReg(reg.A, reg.D);
	} break;
	case 0x83: {
		addByteReg(reg.A, reg.E);
	} break;
	case 0x84: {
		addByteReg(reg.A, reg.H);
	} break;
	case 0x85: {
		addByteReg(reg.A, reg.L);
	} break;
	case 0x86: {
		addByteReg(reg.A, memory.view(reg.HL));
	} break;
	case 0x87: {
		addByteReg(reg.A, reg.A);
	} break;
	case 0x88: { // start of ADC (add w/ carry flag)
		addCarryByteReg(reg.A, reg.B);
	} break;
	case 0x89: {
		addCarryByteReg(reg.A, reg.C);
	} break;
	case 0x8A: {
		addCarryByteReg(reg.A, reg.D);
	} break;
	case 0x8B: {
		addCarryByteReg(reg.A, reg.E);
	} break;
	case 0x8C: {
		addCarryByteReg(reg.A, reg.H); 
	} break;
	case 0x8D: {
		addCarryByteReg(reg.A, reg.L);
	} break;
	case 0x8E: {
		addCarryByteReg(reg.A, memory.view(reg.HL));
	} break;
	case 0x8F: {
		addCarryByteReg(reg.A, reg.A);
	} break;

	//====0x9?======SUB========================================================

	case 0x90: {
		subByteReg(reg.A, reg.B);
	} break;
	case 0x91: {
		subByteReg(reg.A, reg.C);
	} break;
	case 0x92: {
		subByteReg(reg.A, reg.D);
	} break;
	case 0x93: {
		subByteReg(reg.A, reg.E);
	} break;
	case 0x94: {
		subByteReg(reg.A, reg.H);
	} break;
	case 0x95: {
		subByteReg(reg.A, reg.L);
	} break;
	case 0x96: {
		subByteReg(reg.A, memory.view(reg.HL));
	} break;
	case 0x97: {
		subByteReg(reg.A, reg.A);
	} break;
	case 0x98: { // start of ADC (add w/ carry flag)
		subCarryByteReg(reg.A, reg.B);
	} break;
	case 0x99: {
		subCarryByteReg(reg.A, reg.C);
	} break;
	case 0x9A: {
		subCarryByteReg(reg.A, reg.D);
	} break;
	case 0x9B: {
		subCarryByteReg(reg.A, reg.E);
	} break;
	case 0x9C: {
		subCarryByteReg(reg.A, reg.H);
	} break;
	case 0x9D: {
		subCarryByteReg(reg.A, reg.L);
	} break;
	case 0x9E: {
		subCarryByteReg(reg.A, memory.view(reg.HL));
	} break;
	case 0x9F: {
		subCarryByteReg(reg.A, reg.A);
	} break;

	//====0xA?======AND XOR====================================================

	case 0xA0: {
		andByteReg(reg.A, reg.B);
	} break;
	case 0xA1: {
		andByteReg(reg.A, reg.C);
	} break;
	case 0xA2: {
		andByteReg(reg.A, reg.D);
	} break;
	case 0xA3: {
		andByteReg(reg.A, reg.E);
	} break;
	case 0xA4: {
		andByteReg(reg.A, reg.H);
	} break;
	case 0xA5: {
		andByteReg(reg.A, reg.L);
	} break;
	case 0xA6: {
		andByteReg(reg.A, memory.view(reg.HL));
	} break;
	case 0xA7: {
		andByteReg(reg.A, reg.A);
	} break;
	case 0xA8: { // start of ADC (add w/ carry flag)
		xorByteReg(reg.A, reg.B);
	} break;
	case 0xA9: {
		xorByteReg(reg.A, reg.C);
	} break;
	case 0xAA: {
		xorByteReg(reg.A, reg.D);
	} break;
	case 0xAB: {
		xorByteReg(reg.A, reg.E);
	} break;
	case 0xAC: {
		xorByteReg(reg.A, reg.H);
	} break;
	case 0xAD: {
		xorByteReg(reg.A, reg.L);
	} break;
	case 0xAE: {
		xorByteReg(reg.A, memory.view(reg.HL));
	} break;
	case 0xAF: {
		xorByteReg(reg.A, reg.A);
	} break;

	//====0xB?======OR   CP====================================================

	case 0xB0: {
		orByteReg(reg.A, reg.B);
	} break;
	case 0xB1: {
		orByteReg(reg.A, reg.C);
	} break;
	case 0xB2: {
		orByteReg(reg.A, reg.D);
	} break;
	case 0xB3: {
		orByteReg(reg.A, reg.E);
	} break;
	case 0xB4: {
		orByteReg(reg.A, reg.H);
	} break;
	case 0xB5: {
		orByteReg(reg.A, reg.L);
	} break;
	case 0xB6: {
		orByteReg(reg.A, memory.view(reg.HL));
	} break;
	case 0xB7: {
		orByteReg(reg.A, reg.A);
	} break;
	case 0xB8: { // start of ADC (add w/ carry flag)
		compareByteReg(reg.A, reg.B);
	} break;
	case 0xB9: {
		compareByteReg(reg.A, reg.C);
	} break;
	case 0xBA: {
		compareByteReg(reg.A, reg.D);
	} break;
	case 0xBB: {
		compareByteReg(reg.A, reg.E);
	} break;
	case 0xBC: {
		compareByteReg(reg.A, reg.H);
	} break;
	case 0xBD: {
		compareByteReg(reg.A, reg.L);
	} break;
	case 0xBE: {
		compareByteReg(reg.A, memory.view(reg.HL));
	} break;
	case 0xBF: {//these are the same so just set flags here
		clearFlags();
		setZeroFlag();
		setNegFlag();
	} break;

	//====0xC?======end of repetitive regs====================================================

	case 0xC0: { // ret NZ 
		if (!isZeroFlag())
		{
			addCycle(3);
			reg.PC = popStack();
		}
	} break;
	case 0xC1: {
		reg.BC = popStack();
	} break;
	case 0xC2: {
		uint16_t addr = memory.readWord(reg.PC); reg.PC+=2;
		if (!isZeroFlag())
		{
			addCycle();
			reg.PC = addr;
		}
	} break;
	case 0xC3: {
		uint16_t addr = memory.readWord(reg.PC); reg.PC+=2;
		reg.PC = addr;
	} break;
	case 0xC4: {//call nz a16
		uint16_t addr = memory.readWord(reg.PC); reg.PC+=2;
		if (!isZeroFlag())
		{
			addCycle(3);
			call(addr);
		}
	} break;
	case 0xC5: { // push BC 
		push(reg.BC);
	} break;
	case 0xC6: { //ADD A n8
		addByteReg(reg.A, memory.read(reg.PC));  reg.PC++;
	} break;
	case 0xC7: { //RST $00
		call(0x00);
	} break;
	case 0xC8: { 
		if (isZeroFlag())
		{
			addCycle(3);
			reg.PC = popStack();
		}
	} break;
	case 0xC9: {
	//RETURN SUBROUTINE
		reg.PC = popStack();
	} break;
	case 0xCA: {
		uint16_t addr = memory.readWord(reg.PC); reg.PC+=2;
		if (isZeroFlag())
		{
			addCycle();
			reg.PC = addr;
		}
	} break;
	case 0xCB: { 

		uint8_t opcodeCB = memory.read(reg.PC);  reg.PC++;
		//minimum added cycle of 8 by the looks of things
		addCycle(2); //(does this include that fetch ?)
		//std::cout << (int)opcodeCB << " Is the opcode Cb val" << std::endl;
		executePrefix(opcodeCB);
	} break;
	case 0xCC: {
		uint16_t addr = memory.readWord(reg.PC); reg.PC+=2;
		if (isZeroFlag())
		{
			addCycle(3);
			call(addr);
		}
	} break;
	case 0xCD: { // call n16 THIS MIGHT BE BREAKING BOOT
		uint16_t jumpAddr = memory.readWord(reg.PC); reg.PC+=2;
		call(jumpAddr);
	} break;
	case 0xCE: {
		addCarryByteReg(reg.A, memory.read(reg.PC));  reg.PC++;
	} break;
	case 0xCF: {
		call(0x08);
	} break;

	//====0xD?===============================================================

	case 0xD0: { // ret NZ 
		if (!isCarryFlag())
		{
			addCycle(3);
			reg.PC = popStack();
		}
	} break;
	case 0xD1: {
		reg.DE = popStack();
	} break;
	case 0xD2: {
		uint16_t addr = memory.readWord(reg.PC); reg.PC+=2;
		if (!isCarryFlag())
		{
			addCycle();
			reg.PC = addr;
		}
	} break;
	case 0xD3: { //DO NOTHING
	
	} break;
	case 0xD4: {//call nc a16
		uint16_t addr = memory.readWord(reg.PC); reg.PC+=2;
		if (!isCarryFlag())
		{
			addCycle(3);
			call(addr);
		}
	} break;
	case 0xD5: { // push BC 
		push(reg.DE);
	} break;
	case 0xD6: { //SUB A n8
		subByteReg(reg.A, memory.read(reg.PC));  reg.PC++;
	} break;
	case 0xD7: { //RST $10
		call(0x10);
	} break;
	case 0xD8: {
		if (isCarryFlag())
		{
			addCycle(3);
			reg.PC = popStack();
		}
	} break;
	case 0xD9: {
		//RETURN I  SUBROUTINE
		reg.PC = popStack();
		IME = true;
	} break;
	case 0xDA: {
		uint16_t addr = memory.readWord(reg.PC); reg.PC+=2;
		if (isCarryFlag())
		{
			addCycle(1);
			reg.PC = addr;
		}
	} break;
	case 0xDB: { //empty
	} break;
	case 0xDC: {
		uint16_t addr = memory.readWord(reg.PC); reg.PC+=2;
		if (isCarryFlag())
		{
			addCycle(3);
			call(addr);
		}
	} break;
	case 0xDD: { // empty
	} break;
	case 0xDE: {
		subCarryByteReg(reg.A, memory.read(reg.PC));  reg.PC++;
	} break;
	case 0xDF: {
		call(0x18);
	} break;

	//====0xE?===============================================================

	case 0xE0: { // LDH [a8] a
		uint8_t addr = memory.read(reg.PC);  reg.PC++;
		uint16_t memAddr = addr + 0xFF00; // not sure on this part
		memory.write(memAddr, reg.A);
	} break;
	case 0xE1: {
		reg.HL = popStack();
	} break;
	case 0xE2: {
		uint16_t memAddr = reg.C + 0xFF00; // not sure on this part
		memory.write(memAddr, reg.A);
	} break;
	case 0xE3: { //DO NOTHING
	} break;
	case 0xE4: {//DO NOTHING
	} break;
	case 0xE5: { // push HL
		push(reg.HL);
	} break;
	case 0xE6: { //AND A n8
		andByteReg(reg.A, memory.read(reg.PC));  reg.PC++;
	} break;
	case 0xE7: { //RST $10
		call(0x20);
	} break;
	case 0xE8: {//add sp ,e8
		int8_t e8 = memory.read(reg.PC);  reg.PC++;
		addWordRegSigned(reg.SP, e8);
	} break;
	case 0xE9: {
		reg.PC = reg.HL;
	} break;
	case 0xEA: { // load to a16
		uint16_t addr = memory.readWord(reg.PC); reg.PC+=2;
		memory.write(addr, reg.A);
	} break;
	case 0xEB: { //empty
	} break;
	case 0xEC: { //empty
	} break;
	case 0xED: { // empty
	} break;
	case 0xEE: {
		xorByteReg(reg.A, memory.read(reg.PC));  reg.PC++;
	} break;
	case 0xEF: {
		call(0x28);
	} break;

	//====0xF?===============================================================

	case 0xF0: { // LDH  a [a8]
		reg.A = memory.read(reg.PC); reg.PC++;
	} break;
	case 0xF1: {
		reg.AF = popStack();
	} break;
	case 0xF2: {
		reg.C = memory.read(reg.PC);  reg.PC++;
	} break;
	case 0xF3: { // DI (what does this mean)
		IME = false;
	} break;
	case 0xF4: {//DO NOTHING
	} break;
	case 0xF5: { // push AF
		push(reg.AF);
	} break;
	case 0xF6: { //AND A n8
		orByteReg(reg.A, memory.read(reg.PC));  reg.PC++;
	} break;
	case 0xF7: { //RST $10
		call(0x30);
	} break;
	case 0xF8: {//load into reg.hl, SP + e8
		int8_t byte = memory.read(reg.PC);  reg.PC++;
		reg.HL = reg.SP + byte;
	} break;
	case 0xF9: {
		reg.SP = reg.HL;
	} break;
	case 0xFA: { // load into a , [16]
		uint16_t tempAddr = memory.readWord(reg.PC); reg.PC+=2;
		reg.A = memory.read(tempAddr);
	} break;
	case 0xFB: { //empty
		IME_nextCycle = true;
	} break;
	case 0xFC: { //empty
	} break;
	case 0xFD: { // empty
	} break;
	case 0xFE: {
		compareByteReg(reg.A, memory.read(reg.PC));  reg.PC++;
	} break;
	case 0xFF: {
		call(0x38);
	} break;

	default: {
		printf("No opcode implemented for : %d", opcode);
	}break;
	}
	/*									/*
	*	end of giant switch statement	 *
	*/									//

	if (IME_nextCycle)
	{
		IME = true;
		IME_nextCycle = false;
	}
}

void SM83::reset()
{
	reg.PC = 0; // put this to 0x100 for skipping boot rom
	reg.SP = 0xFFFE;
	reg.AF = 0;
	reg.BC = 0;
	reg.DE = 0;
	reg.HL = 0;
	IME = 0;
}

void SM83::handleInterrupts()
{
	if (!IME) return; // check is interrupt requested
	uint8_t IF = memory.read(0xFF0F);
	if (!IF)return;
	uint8_t IE = memory.read(0xFF0F);
	if (!IE)return;
	
	//IF ~ Interrupt Flag, 
	//IE ~ Interrupt Enable


	//interruptsd are checked before fetching new instruction
	// if any IF flag and corresponding IE flag are both 1
	// and IME is 1
	//CPU will push the current PC to the stack
	//will jump to the corresponding interrupt vector
	//set IME to 0
	//flags are only cleared when CPU jumps to interrupt vector
	//if both called same time, lower bank has priority (vBlank has highest)
	//takes 20 clocks to dispath interrupt
	//if cpu is in HALT mode , another 4 clocks are needed

	//so, if we got this far, IME is on
	//IF and IE have A value

	//can only do one, in which lowest priority first (vBlank)

	if (IF & 0b1 && IE & 0b1) //bit0 vBlank
	{
		//the corresponding IF bit and IME flag are reset
		// 0xFF0F IF address
		memory.write(0xFF0F, (IF &= 0b11111110));
		IME = false;
		call(0x0040);
		addCycle(5);
	}
	if (IF & 0b10 && IE & 0b10) //bit1 LCDstat
	{
		memory.write(0xFF0F, (IF &= 0b11111101));
		IME = false;
		call(0x0048);
		addCycle(5);
	}
	if (IF & 0b100 && IE & 0b100) //bit2 Timer
	{
		memory.write(0xFF0F, (IF &= 0b11111011));
		IME = false;
		call(0x0050);
		addCycle(5);
	}
	if (IF & 0b1000 && IE & 0b1000) //bit3 Serial
	{
		memory.write(0xFF0F, (IF &= 0b11110111));
		IME = false;
		call(0x0058);
		addCycle(5);
	}
	if (IF & 0b10000 && IE & 0b10000) //bit4 Joypad
	{
		memory.write(0xFF0F, (IF &= 0b11101111));
		IME = false;
		call(0x0060);
		addCycle(5);
	}

	// mabye do this : addCycle(5);


}



uint8_t SM83::executeInstruction()
{
	handleInterrupts();

	uint8_t opcode = memory.read(reg.PC); //fetch
	reg.PC++;

	execute(opcode); // decode - execute
	return opcodeCycles[opcode];
}

void SM83::executeCycle(double cyclesAvailable)
{
	while (cycles < cyclesAvailable)
	{
		cycles += executeInstruction();
	}

}
