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
	//if (cycles > 0) {
		uint8_t opcode = memory.read(reg.PC);
		cycles = opcodeCycles[opcode];

		execute(opcode);
	//}
	//cycles--;
}
