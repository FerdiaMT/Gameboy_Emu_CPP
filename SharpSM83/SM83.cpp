#include "SM83.h"
#include "Memory.h"
#include <cstdint>
#include <unordered_map>

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

	{0x20, 1}, //start clock cycle here
	{0x21, 3}, 
	{0x22, 2}, 
	{0x23, 2}, 
	{0x24, 1}, 
	{0x25, 1}, 
	{0x26, 2}, 
	{0x27, 1}, 
	{0x28, 3}, 
	{0x29, 2}, 
	{0x2A, 2}, 
	{0x2B, 2}, 
	{0x2C, 1}, 
	{0x2D, 1}, 
	{0x2E, 2}, 
	{0x2F, 1}, 

	{0x30, 1},
	{0x31, 3},
	{0x32, 2},
	{0x33, 2},
	{0x34, 1},
	{0x35, 1},
	{0x36, 2},
	{0x37, 1},
	{0x38, 3},
	{0x39, 2},
	{0x3A, 2},
	{0x3B, 2},
	{0x3C, 1},
	{0x3D, 1},
	{0x3E, 2},
	{0x3F, 1},

	{0x40, 1},
	{0x41, 3},
	{0x42, 2},
	{0x43, 2},
	{0x44, 1},
	{0x45, 1},
	{0x46, 2},
	{0x47, 1},
	{0x48, 3},
	{0x49, 2},
	{0x4A, 2},
	{0x4B, 2},
	{0x4C, 1},
	{0x4D, 1},
	{0x4E, 2},
	{0x4F, 1},

	{0x50, 1},
	{0x51, 3},
	{0x52, 2},
	{0x53, 2},
	{0x54, 1},
	{0x55, 1},
	{0x56, 2},
	{0x57, 1},
	{0x58, 3},
	{0x59, 2},
	{0x5A, 2},
	{0x5B, 2},
	{0x5C, 1},
	{0x5D, 1},
	{0x5E, 2},
	{0x5F, 1},

	{ 0x60, 1 },
	{ 0x61, 3 },
	{ 0x62, 2 },
	{ 0x63, 2 },
	{ 0x64, 1 },
	{ 0x65, 1 },
	{ 0x66, 2 },
	{ 0x67, 1 },
	{ 0x68, 3 },
	{ 0x69, 2 },
	{ 0x6A, 2 },
	{ 0x6B, 2 },
	{ 0x6C, 1 },
	{ 0x6D, 1 },
	{ 0x6E, 2 },
	{ 0x6F, 1 },

	{ 0x70, 1 },
	{ 0x71, 3 },
	{ 0x72, 2 },
	{ 0x73, 2 },
	{ 0x74, 1 },
	{ 0x75, 1 },
	{ 0x76, 2 },
	{ 0x77, 1 },
	{ 0x78, 3 },
	{ 0x79, 2 },
	{ 0x7A, 2 },
	{ 0x7B, 2 },
	{ 0x7C, 1 },
	{ 0x7D, 1 },
	{ 0x7E, 2 },
	{ 0x7F, 1 },

	{ 0x80, 1 },
	{ 0x81, 3 },
	{ 0x82, 2 },
	{ 0x83, 2 },
	{ 0x84, 1 },
	{ 0x85, 1 },
	{ 0x86, 2 },
	{ 0x87, 1 },
	{ 0x88, 3 },
	{ 0x89, 2 },
	{ 0x8A, 2 },
	{ 0x8B, 2 },
	{ 0x8C, 1 },
	{ 0x8D, 1 },
	{ 0x8E, 2 },
	{ 0x8F, 1 },

	{ 0x90, 1 },
	{ 0x91, 3 },
	{ 0x92, 2 },
	{ 0x93, 2 },
	{ 0x94, 1 },
	{ 0x95, 1 },
	{ 0x96, 2 },
	{ 0x97, 1 },
	{ 0x98, 3 },
	{ 0x99, 2 },
	{ 0x9A, 2 },
	{ 0x9B, 2 },
	{ 0x9C, 1 },
	{ 0x9D, 1 },
	{ 0x9E, 2 },
	{ 0x9F, 1 },

	{ 0xA0, 1 },
	{ 0xA1, 3 },
	{ 0xA2, 2 },
	{ 0xA3, 2 },
	{ 0xA4, 1 },
	{ 0xA5, 1 },
	{ 0xA6, 2 },
	{ 0xA7, 1 },
	{ 0xA8, 3 },
	{ 0xA9, 2 },
	{ 0xAA, 2 },
	{ 0xAB, 2 },
	{ 0xAC, 1 },
	{ 0xAD, 1 },
	{ 0xAE, 2 },
	{ 0xAF, 1 },

	{ 0xB0, 1 },
	{ 0xB1, 3 },
	{ 0xB2, 2 },
	{ 0xB3, 2 },
	{ 0xB4, 1 },
	{ 0xB5, 1 },
	{ 0xB6, 2 },
	{ 0xB7, 1 },
	{ 0xB8, 3 },
	{ 0xB9, 2 },
	{ 0xBA, 2 },
	{ 0xBB, 2 },
	{ 0xBC, 1 },
	{ 0xBD, 1 },
	{ 0xBE, 2 },
	{ 0xBF, 1 },

	{ 0xC0, 1 },
	{ 0xC1, 3 },
	{ 0xC2, 2 },
	{ 0xC3, 2 },
	{ 0xC4, 1 },
	{ 0xC5, 1 },
	{ 0xC6, 2 },
	{ 0xC7, 1 },
	{ 0xC8, 3 },
	{ 0xC9, 2 },
	{ 0xCA, 2 },
	{ 0xCB, 2 },
	{ 0xCC, 1 },
	{ 0xCD, 1 },
	{ 0xCE, 2 },
	{ 0xCF, 1 },

	{ 0xD0, 1 },
	{ 0xD1, 3 },
	{ 0xD2, 2 },
	{ 0xD3, 2 },
	{ 0xD4, 1 },
	{ 0xD5, 1 },
	{ 0xD6, 2 },
	{ 0xD7, 1 },
	{ 0xD8, 3 },
	{ 0xD9, 2 },
	{ 0xDA, 2 },
	{ 0xDB, 2 },
	{ 0xDC, 1 },
	{ 0xDD, 1 },
	{ 0xDE, 2 },
	{ 0xDF, 1 },

	{ 0xE0, 1 },
	{ 0xE1, 3 },
	{ 0xE2, 2 },
	{ 0xE3, 2 },
	{ 0xE4, 1 },
	{ 0xE5, 1 },
	{ 0xE6, 2 },
	{ 0xE7, 1 },
	{ 0xE8, 3 },
	{ 0xE9, 2 },
	{ 0xEA, 2 },
	{ 0xEB, 2 },
	{ 0xEC, 1 },
	{ 0xED, 1 },
	{ 0xEE, 2 },
	{ 0xEF, 1 },

	{ 0xF0, 1 },
	{ 0xF1, 3 },
	{ 0xF2, 2 },
	{ 0xF3, 2 },
	{ 0xF4, 1 },
	{ 0xF5, 1 },
	{ 0xF6, 2 },
	{ 0xF7, 1 },
	{ 0xF8, 3 },
	{ 0xF9, 2 },
	{ 0xFA, 2 },
	{ 0xFB, 2 },
	{ 0xFC, 1 },
	{ 0xFD, 1 },
	{ 0xFE, 2 },
	{ 0xFF, 1 },

};

SM83::SM83(Memory& memory) : memory(memory){};

void clearFlags() {
	reg.F = 0;
}

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

bool isZeroFlag()
{
	return (reg.F >> 7)&0b1;
}
bool isNegFlag()
{
	return (reg.F >> 6) & 0b1;
}
bool isHalfFlag()
{
	return (reg.F >> 5) & 0b1;
}
bool isCarryFlag()
{
	return (reg.F >> 4) & 0b1;
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
	uint8_t r8Val = r8a + r8b;
	if (r8a + r8b > 0xFF)
	{
		setCarryFlag();
	}
	if (r8Val == 0)
	{
		setZeroFlag();
	}
	if (((r8b & 0x0F) + (r8a & 0x0F)) > 0x0F) {
		setHalfFlag();  // Look into this(Shoudl be working)
	}

	r8a = r8Val;
}

void addCarryByteReg(uint8_t& r8a, uint8_t r8b)
{
	uint8_t isCarry = 0;
	if (isCarryFlag()) isCarry = 1;

	clearFlags();
	uint8_t r8Val = r8a + r8b + isCarry;

	if (r8a + r8b+ isCarry > 0xFF)
	{
		setCarryFlag();
	}
	if (r8Val == 0)
	{
		setZeroFlag();
	}
	if (((r8b & 0x0F) + (r8a & 0x0F))+ isCarry > 0x0F) {
		setHalfFlag();  // Look into this(Shoudl be working)
	}

	r8a = r8Val;
}

void subByteReg(uint8_t& r8a , uint8_t r8b)
{
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

	r8a = r8Val;
}

void subCarryByteReg(uint8_t& r8a, uint8_t r8b)
{
	uint8_t isCarry = 0;
	if (isCarryFlag()) isCarry = 1;

	clearFlags();
	setNegFlag();
	uint8_t r8Val = r8a - r8b - isCarry;
	if (r8b+isCarry > r8a)
	{
		setCarryFlag();
	}
	if (r8Val == 0)
	{
		setZeroFlag();
	}
	if (((r8a & 0x0F) + isCarry) > (r8b & 0x0F)) {
		setHalfFlag();
	}

	r8a = r8Val;
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
	r8a = r8a ^ r8b;
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
	memory.writeWord(reg.SP, reg.PC);
	reg.PC = jumpAddr;
}
void SM83::push(uint16_t pushData)
{
	reg.SP--;
	memory.writeWord(reg.SP, pushData);
}


void SM83::execute(uint8_t opcode)
{
	switch (opcode) {

	case 0x00: { // no op

	}break;
	case 0x01: { // load into bc, n16
		reg.BC = memory.readWord(reg.PC);
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
		reg.F = ((reg.A & 0b10000000) >> 3); // grab most significant bit and position it at the carry flag
		reg.A = (reg.A << 1) & 0xFF;
	}break;
	case 0x08: { // load [a16] , SP
		uint16_t wordAdr = memory.readWord(reg.PC);
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
		reg.F = ((reg.A & 0b1) << 4); 
		reg.A = (reg.A >> 1) & 0xFF;
	}break;

	//====0x1?===================================================

	case 0x10: { // STOP, this is a funny one TODO: look into this in pandocs
		//i think we take in another byte ?
		memory.read(reg.PC);
	}break;
	case 0x11: { // load into bc, n16
		reg.DE = memory.readWord(reg.PC);
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
		reg.D = memory.read(reg.PC);
	}break;
	case 0x17: { // rotate register A left ,carry goes into a
		//0bznhc0000
		uint8_t Cbit = (reg.F & 0b00010000) >> 4; // if carry is present set it otherwise dont

		reg.F = ((reg.A & 0b10000000) >> 3); // grab most significant bit and position it at the carry flag
		reg.A = ((reg.A << 1) | Cbit) & 0xFF;
	}break;
	case 0x18: { // jump to e8

		int8_t offset = memory.read(reg.PC); // pc = 129
		reg.PC += offset;

	}break;
	case 0x19: {
		addWordReg(reg.HL, reg.DE);
	}break;
	case 0x1A: {
		reg.A = memory.read(reg.DE);
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
		reg.E = memory.read(reg.PC);
	}break;
	case 0x1F: {   //RRA //0bznhc0000

		uint8_t Cbit = (reg.F & 0b00010000) << 3;
		reg.F = ((reg.A & 0b1) << 4);// grab most significant bit and position it at the carry flag
		reg.A = ((reg.A >> 1) | Cbit) & 0xFF;

	}break;

	//====0x2?===================================================

	case 0x20: { // JR NZ , e8
		// jump to e8 if NotZ is met
		int8_t offset = memory.read(reg.PC); // pc = 129
		if (!isZeroFlag)
		{
			reg.PC += offset;
		}
	}break;
	case 0x21: { // load into bc, n16
		reg.HL = memory.readWord(reg.PC);
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
		reg.H = memory.read(reg.PC);
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

		int8_t offset = memory.read(reg.PC); // pc = 129
		if (isZeroFlag())
		{
			reg.PC += offset;
		}

	}break;
	case 0x29: {
		addWordReg(reg.HL, reg.HL);
	}break;
	case 0x2A: {
		reg.A = memory.read(reg.HL);
		reg.HL++;
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
		reg.L = memory.read(reg.PC);
	}break;
	case 0x2F: {   // COMPLEMENT
		setNegFlag();
		setHalfFlag();
		reg.A = ~reg.A;

	}break;

	//====0x3?===================================================

	case 0x30: { // JR Z , e8
		int8_t offset = memory.read(reg.PC); // pc = 129
		if (!isCarryFlag())
		{
			reg.PC += offset;
		}
	}break;
	case 0x31: { // load into bc, n16
		reg.SP = memory.readWord(reg.PC);
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
		//UNSURE IF I SHOULD INCREMENT PROGRAMCOUNTER FROM HERE 
	}break;
	case 0x35: {
		uint8_t byte = memory.view(reg.HL); // this one wont increment
		decByte(byte);
		memory.write(reg.HL, byte);
	}break;
	case 0x36: {
		memory.write(reg.HL, memory.read(reg.PC));// load into address hl, data held in next pc
	}break;
	case 0x37: { // SCF (confusing one)
		bool z = isZeroFlag;
		clearFlags();
		setCarryFlag();
		if (z) setZeroFlag;
	}break;
	case 0x38: { // jump to e8

		int8_t offset = memory.read(reg.PC); // pc = 129
		if (isCarryFlag())
		{
			reg.PC += offset;
		}

	}break;
	case 0x39: {
		addWordReg(reg.HL, reg.SP);
	}break;
	case 0x3A: {
		reg.A = memory.read(reg.HL);
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
		reg.A = memory.read(reg.PC);
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
			reg.PC = popStack();
		}
	} break;
	case 0xC1: {
		reg.BC = popStack();
	} break;
	case 0xC2: {
		uint16_t addr = memory.readWord(reg.PC);
		if (!isZeroFlag())
		{
			reg.PC = addr;
		}
	} break;
	case 0xC3: {
		uint16_t addr = memory.readWord(reg.PC);
		reg.PC = addr;
	} break;
	case 0xC4: {//call nz a16
		uint16_t addr = memory.readWord(reg.PC);
		if (!isZeroFlag())
		{
			call(addr);
		}
	} break;
	case 0xC5: { // push BC 
		push(reg.BC);
	} break;
	case 0xC6: { //ADD A n8
		addByteReg(reg.A, memory.read(reg.PC));
	} break;
	case 0xC7: { //RST $00
		call(0x00);
	} break;
	case 0xC8: { 
		if (isZeroFlag())
		{
			reg.PC = popStack();
		}
	} break;
	case 0xC9: {
	//RETURN SUBROUTINE
		reg.PC = popStack();
	} break;
	case 0xCA: {
		uint16_t addr = memory.readWord(reg.PC);
		if (isZeroFlag())
		{
			reg.PC = addr;
		}
	} break;
	case 0xCB: { // sneaky 256 function thing
		//DO THIS AFTER BREAK
	} break;
	case 0xCC: {
		uint16_t addr = memory.readWord(reg.PC);
		if (isZeroFlag())
		{
			call(addr);
		}
	} break;
	case 0xCD: { // call n16
		uint16_t jumpAddr = memory.readWord(reg.PC);
		call(jumpAddr);
	} break;
	case 0xCE: {
		addCarryByteReg(reg.A, memory.read(reg.PC));
	} break;
	case 0xCF: {
		call(0x08);
	} break;

	//====0xD?===============================================================

	case 0xD0: { // ret NZ 
		if (!isCarryFlag())
		{
			reg.PC = popStack();
		}
	} break;
	case 0xD1: {
		reg.DE = popStack();
	} break;
	case 0xD2: {
		uint16_t addr = memory.readWord(reg.PC);
		if (!isCarryFlag())
		{
			reg.PC = addr;
		}
	} break;
	case 0xD3: { //DO NOTHING
	
	} break;
	case 0xD4: {//call nc a16
		uint16_t addr = memory.readWord(reg.PC);
		if (!isCarryFlag())
		{
			call(addr);
		}
	} break;
	case 0xD5: { // push BC 
		push(reg.DE);
	} break;
	case 0xD6: { //SUB A n8
		subByteReg(reg.A, memory.read(reg.PC));
	} break;
	case 0xD7: { //RST $10
		call(0x10);
	} break;
	case 0xD8: {
		if (isCarryFlag())
		{
			reg.PC = popStack();
		}
	} break;
	case 0xD9: {
		//RETURN I  SUBROUTINE
		reg.PC = popStack();
		IME = true;
	} break;
	case 0xDA: {
		uint16_t addr = memory.readWord(reg.PC);
		if (isCarryFlag())
		{
			reg.PC = addr;
		}
	} break;
	case 0xDB: { //empty
	} break;
	case 0xDC: {
		uint16_t addr = memory.readWord(reg.PC);
		if (isCarryFlag())
		{
			call(addr);
		}
	} break;
	case 0xDD: { // empty
	} break;
	case 0xDE: {
		subCarryByteReg(reg.A, memory.read(reg.PC));
	} break;
	case 0xDF: {
		call(0x18);
	} break;

	//====0xE?===============================================================

	case 0xE0: { // LDH [a8] a
		uint8_t addr = memory.read(reg.PC);
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
		andByteReg(reg.A, memory.read(reg.PC));
	} break;
	case 0xE7: { //RST $10
		call(0x20);
	} break;
	case 0xE8: {//add sp ,e8
		int8_t e8 = memory.read(reg.PC);
		addWordRegSigned(reg.SP, e8);
	} break;
	case 0xE9: {
		reg.PC = reg.HL;
	} break;
	case 0xEA: { // load to a16
		uint16_t addr = memory.readWord(reg.PC);
		memory.write(addr, reg.A);
	} break;
	case 0xEB: { //empty
	} break;
	case 0xEC: { //empty
	} break;
	case 0xED: { // empty
	} break;
	case 0xEE: {
		xorByteReg(reg.A, memory.read(reg.PC));
	} break;
	case 0xEF: {
		call(0x28);
	} break;

	//====0xF?===============================================================

	case 0xF0: { // LDH  a [a8]
		reg.A = memory.read(reg.PC);
	} break;
	case 0xF1: {
		reg.AF = popStack();
	} break;
	case 0xF2: {
		reg.C = memory.read(reg.PC);
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
		orByteReg(reg.A, memory.read(reg.PC));
	} break;
	case 0xF7: { //RST $10
		call(0x30);
	} break;
	case 0xF8: {//load into reg.hl, SP + e8
		int8_t byte = memory.read(reg.PC);
		reg.HL = reg.SP + byte;
	} break;
	case 0xF9: {
		reg.SP = reg.HL;
	} break;
	case 0xFA: { // load into a , [16]
		uint16_t tempAddr = memory.readWord(reg.PC);
		reg.A = memory.read(tempAddr);
	} break;
	case 0xFB: { //empty
		IME = true;
	} break;
	case 0xFC: { //empty
	} break;
	case 0xFD: { // empty
	} break;
	case 0xFE: {
		compareByteReg(reg.A, memory.read(reg.PC));
	} break;
	case 0xFF: {
		call(0x38);
	} break;

	default: {
		printf("No opcode implemented for : %d", opcode);
	}break;
	}
}



void SM83::reset()
{
	reg.PC = 0;
	reg.SP = 0xFFFE;
	reg.AF = 0;
	reg.BC = 0;
	reg.DE = 0;
	reg.HL = 0;
	IME = 0;
	memory.reset();
}

void SM83::executeCycle()
{
	//if (cycles > 0) {
		uint8_t opcode = memory.read(reg.PC);
		cycles = opcodeCycles[opcode];

		execute(opcode);
	//}
	//cycles--;
}
