#include "sm83.h"
#include "memory.h"


SM83::SM83(Memory* memory) : memory(memory)
{
	AF = 0x01B0;
	BC = 0x0013;
	DE = 0x00D8;
	HL = 0x014D;
	SP = 0xFFFE;
	PC = 0x0100;
	IME = false;
	IME_nextCycle = false;
	isHalted = false;
	cycles = 0;
}

bool SM83::getZ() { return (F & 0x80) != 0; }
bool SM83::getN() { return (F & 0x40) != 0; }
bool SM83::getH() { return (F & 0x20) != 0; }
bool SM83::getC() { return (F & 0x10) != 0; }
void SM83::setZ(bool v) { F = v ? (F | 0x80) : (F & ~0x80); }
void SM83::setN(bool v) { F = v ? (F | 0x40) : (F & ~0x40); }
void SM83::setH(bool v) { F = v ? (F | 0x20) : (F & ~0x20); }
void SM83::setC(bool v) { F = v ? (F | 0x10) : (F & ~0x10); }

uint8_t SM83::read8() { return memory->read(PC++); }
uint16_t SM83::read16() { uint8_t lo = read8(); uint8_t hi = read8(); return (hi << 8) | lo; }

void SM83::push(uint16_t val) { SP -= 2; memory->write(SP, val & 0xFF); memory->write(SP + 1, val >> 8); }
uint16_t SM83::pop() { uint16_t val = memory->read(SP) | (memory->read(SP + 1) << 8); SP += 2; return val; }

int SM83::step()
{
	if (isHalted)
	{
		cycles = 4;
		return cycles;
	}

	if (IME_nextCycle)
	{
		IME = true;
		IME_nextCycle = false;
	}

	uint8_t opcode = read8();
	cycles = execute(opcode);
	return cycles;
}

int SM83::execute(uint8_t opcode)
{
	switch (opcode)
	{
	case 0x00: return 4; // NOP
	case 0x01: BC = read16(); return 12;
	case 0x02: memory->write(BC, A); return 8;
	case 0x03: BC++; return 8;
	case 0x04: B = inc8(B); return 4;
	case 0x05: B = dec8(B); return 4;
	case 0x06: B = read8(); return 8;
	case 0x07: { uint8_t carry = (A & 0x80) >> 7; A = (A << 1) | carry; setZ(false); setN(false); setH(false); setC(carry); return 4; }
	case 0x08: { uint16_t addr = read16(); memory->write(addr, SP & 0xFF); memory->write(addr + 1, SP >> 8); return 20; }
	case 0x09: HL = add16(HL, BC); return 8;
	case 0x0A: A = memory->read(BC); return 8;
	case 0x0B: BC--; return 8;
	case 0x0C: C = inc8(C); return 4;
	case 0x0D: C = dec8(C); return 4;
	case 0x0E: C = read8(); return 8;
	case 0x0F: { uint8_t carry = A & 1; A = (A >> 1) | (carry << 7); setZ(false); setN(false); setH(false); setC(carry); return 4; }

	case 0x10: read8(); return 4; // STOP
	case 0x11: DE = read16(); return 12;
	case 0x12: memory->write(DE, A); return 8;
	case 0x13: DE++; return 8;
	case 0x14: D = inc8(D); return 4;
	case 0x15: D = dec8(D); return 4;
	case 0x16: D = read8(); return 8;
	case 0x17: { uint8_t carry = getC() ? 1 : 0; setC(A & 0x80); A = (A << 1) | carry; setZ(false); setN(false); setH(false); return 4; }
	case 0x18: { int8_t offset = (int8_t)read8(); PC += offset; return 12; }
	case 0x19: HL = add16(HL, DE); return 8;
	case 0x1A: A = memory->read(DE); return 8;
	case 0x1B: DE--; return 8;
	case 0x1C: E = inc8(E); return 4;
	case 0x1D: E = dec8(E); return 4;
	case 0x1E: E = read8(); return 8;
	case 0x1F: { uint8_t carry = getC() ? 1 : 0; setC(A & 1); A = (A >> 1) | (carry << 7); setZ(false); setN(false); setH(false); return 4; }

	case 0x20: { int8_t offset = (int8_t)read8(); if (!getZ()) { PC += offset; return 12; } return 8; }
	case 0x21: HL = read16(); return 12;
	case 0x22: memory->write(HL++, A); return 8;
	case 0x23: HL++; return 8;
	case 0x24: H = inc8(H); return 4;
	case 0x25: H = dec8(H); return 4;
	case 0x26: H = read8(); return 8;
	case 0x27: daa(); return 4;
	case 0x28: { int8_t offset = (int8_t)read8(); if (getZ()) { PC += offset; return 12; } return 8; }
	case 0x29: HL = add16(HL, HL); return 8;
	case 0x2A: A = memory->read(HL++); return 8;
	case 0x2B: HL--; return 8;
	case 0x2C: L = inc8(L); return 4;
	case 0x2D: L = dec8(L); return 4;
	case 0x2E: L = read8(); return 8;
	case 0x2F: A = ~A; setN(true); setH(true); return 4;

	case 0x30: { int8_t offset = (int8_t)read8(); if (!getC()) { PC += offset; return 12; } return 8; }
	case 0x31: SP = read16(); return 12;
	case 0x32: memory->write(HL--, A); return 8;
	case 0x33: SP++; return 8;
	case 0x34: memory->write(HL, inc8(memory->read(HL))); return 12;
	case 0x35: memory->write(HL, dec8(memory->read(HL))); return 12;
	case 0x36: memory->write(HL, read8()); return 12;
	case 0x37: setN(false); setH(false); setC(true); return 4;
	case 0x38: { int8_t offset = (int8_t)read8(); if (getC()) { PC += offset; return 12; } return 8; }
	case 0x39: HL = add16(HL, SP); return 8;
	case 0x3A: A = memory->read(HL--); return 8;
	case 0x3B: SP--; return 8;
	case 0x3C: A = inc8(A); return 4;
	case 0x3D: A = dec8(A); return 4;
	case 0x3E: A = read8(); return 8;
	case 0x3F: setN(false); setH(false); setC(!getC()); return 4;

		// LD r,r instructions
	case 0x40: B = B; return 4;
	case 0x41: B = C; return 4;
	case 0x42: B = D; return 4;
	case 0x43: B = E; return 4;
	case 0x44: B = H; return 4;
	case 0x45: B = L; return 4;
	case 0x46: B = memory->read(HL); return 8;
	case 0x47: B = A; return 4;
	case 0x48: C = B; return 4;
	case 0x49: C = C; return 4;
	case 0x4A: C = D; return 4;
	case 0x4B: C = E; return 4;
	case 0x4C: C = H; return 4;
	case 0x4D: C = L; return 4;
	case 0x4E: C = memory->read(HL); return 8;
	case 0x4F: C = A; return 4;

	case 0x50: D = B; return 4;
	case 0x51: D = C; return 4;
	case 0x52: D = D; return 4;
	case 0x53: D = E; return 4;
	case 0x54: D = H; return 4;
	case 0x55: D = L; return 4;
	case 0x56: D = memory->read(HL); return 8;
	case 0x57: D = A; return 4;
	case 0x58: E = B; return 4;
	case 0x59: E = C; return 4;
	case 0x5A: E = D; return 4;
	case 0x5B: E = E; return 4;
	case 0x5C: E = H; return 4;
	case 0x5D: E = L; return 4;
	case 0x5E: E = memory->read(HL); return 8;
	case 0x5F: E = A; return 4;

	case 0x60: H = B; return 4;
	case 0x61: H = C; return 4;
	case 0x62: H = D; return 4;
	case 0x63: H = E; return 4;
	case 0x64: H = H; return 4;
	case 0x65: H = L; return 4;
	case 0x66: H = memory->read(HL); return 8;
	case 0x67: H = A; return 4;
	case 0x68: L = B; return 4;
	case 0x69: L = C; return 4;
	case 0x6A: L = D; return 4;
	case 0x6B: L = E; return 4;
	case 0x6C: L = H; return 4;
	case 0x6D: L = L; return 4;
	case 0x6E: L = memory->read(HL); return 8;
	case 0x6F: L = A; return 4;

	case 0x70: memory->write(HL, B); return 8;
	case 0x71: memory->write(HL, C); return 8;
	case 0x72: memory->write(HL, D); return 8;
	case 0x73: memory->write(HL, E); return 8;
	case 0x74: memory->write(HL, H); return 8;
	case 0x75: memory->write(HL, L); return 8;
	case 0x76: { // HALT
		uint8_t IF = memory->read(0xFF0F);
		uint8_t IE = memory->read(0xFFFF);
		if (!IME && (IF & IE & 0x1F))
		{
			isHalted = false;
		}
		else
		{
			isHalted = true;
		}
		return 4;
	}
	case 0x77: memory->write(HL, A); return 8;
	case 0x78: A = B; return 4;
	case 0x79: A = C; return 4;
	case 0x7A: A = D; return 4;
	case 0x7B: A = E; return 4;
	case 0x7C: A = H; return 4;
	case 0x7D: A = L; return 4;
	case 0x7E: A = memory->read(HL); return 8;
	case 0x7F: A = A; return 4;

		// ALU operations
	case 0x80: add8(B); return 4;
	case 0x81: add8(C); return 4;
	case 0x82: add8(D); return 4;
	case 0x83: add8(E); return 4;
	case 0x84: add8(H); return 4;
	case 0x85: add8(L); return 4;
	case 0x86: add8(memory->read(HL)); return 8;
	case 0x87: add8(A); return 4;
	case 0x88: adc8(B); return 4;
	case 0x89: adc8(C); return 4;
	case 0x8A: adc8(D); return 4;
	case 0x8B: adc8(E); return 4;
	case 0x8C: adc8(H); return 4;
	case 0x8D: adc8(L); return 4;
	case 0x8E: adc8(memory->read(HL)); return 8;
	case 0x8F: adc8(A); return 4;

	case 0x90: sub8(B); return 4;
	case 0x91: sub8(C); return 4;
	case 0x92: sub8(D); return 4;
	case 0x93: sub8(E); return 4;
	case 0x94: sub8(H); return 4;
	case 0x95: sub8(L); return 4;
	case 0x96: sub8(memory->read(HL)); return 8;
	case 0x97: sub8(A); return 4;
	case 0x98: sbc8(B); return 4;
	case 0x99: sbc8(C); return 4;
	case 0x9A: sbc8(D); return 4;
	case 0x9B: sbc8(E); return 4;
	case 0x9C: sbc8(H); return 4;
	case 0x9D: sbc8(L); return 4;
	case 0x9E: sbc8(memory->read(HL)); return 8;
	case 0x9F: sbc8(A); return 4;

	case 0xA0: and8(B); return 4;
	case 0xA1: and8(C); return 4;
	case 0xA2: and8(D); return 4;
	case 0xA3: and8(E); return 4;
	case 0xA4: and8(H); return 4;
	case 0xA5: and8(L); return 4;
	case 0xA6: and8(memory->read(HL)); return 8;
	case 0xA7: and8(A); return 4;
	case 0xA8: xor8(B); return 4;
	case 0xA9: xor8(C); return 4;
	case 0xAA: xor8(D); return 4;
	case 0xAB: xor8(E); return 4;
	case 0xAC: xor8(H); return 4;
	case 0xAD: xor8(L); return 4;
	case 0xAE: xor8(memory->read(HL)); return 8;
	case 0xAF: xor8(A); return 4;

	case 0xB0: or8(B); return 4;
	case 0xB1: or8(C); return 4;
	case 0xB2: or8(D); return 4;
	case 0xB3: or8(E); return 4;
	case 0xB4: or8(H); return 4;
	case 0xB5: or8(L); return 4;
	case 0xB6: or8(memory->read(HL)); return 8;
	case 0xB7: or8(A); return 4;
	case 0xB8: cp8(B); return 4;
	case 0xB9: cp8(C); return 4;
	case 0xBA: cp8(D); return 4;
	case 0xBB: cp8(E); return 4;
	case 0xBC: cp8(H); return 4;
	case 0xBD: cp8(L); return 4;
	case 0xBE: cp8(memory->read(HL)); return 8;
	case 0xBF: cp8(A); return 4;

	case 0xC0: if (!getZ()) { PC = pop(); return 20; } return 8;
	case 0xC1: BC = pop(); return 12;
	case 0xC2: { uint16_t addr = read16(); if (!getZ()) { PC = addr; return 16; } return 12; }
	case 0xC3: PC = read16(); return 16;
	case 0xC4: { uint16_t addr = read16(); if (!getZ()) { push(PC); PC = addr; return 24; } return 12; }
	case 0xC5: push(BC); return 16;
	case 0xC6: add8(read8()); return 8;
	case 0xC7: push(PC); PC = 0x00; return 16;
	case 0xC8: if (getZ()) { PC = pop(); return 20; } return 8;
	case 0xC9: PC = pop(); return 16;
	case 0xCA: { uint16_t addr = read16(); if (getZ()) { PC = addr; return 16; } return 12; }
	case 0xCB: return executeCB();
	case 0xCC: { uint16_t addr = read16(); if (getZ()) { push(PC); PC = addr; return 24; } return 12; }
	case 0xCD: { uint16_t addr = read16(); push(PC); PC = addr; return 24; }
	case 0xCE: adc8(read8()); return 8;
	case 0xCF: push(PC); PC = 0x08; return 16;

	case 0xD0: if (!getC()) { PC = pop(); return 20; } return 8;
	case 0xD1: DE = pop(); return 12;
	case 0xD2: { uint16_t addr = read16(); if (!getC()) { PC = addr; return 16; } return 12; }
	case 0xD4: { uint16_t addr = read16(); if (!getC()) { push(PC); PC = addr; return 24; } return 12; }
	case 0xD5: push(DE); return 16;
	case 0xD6: sub8(read8()); return 8;
	case 0xD7: push(PC); PC = 0x10; return 16;
	case 0xD8: if (getC()) { PC = pop(); return 20; } return 8;
	case 0xD9: PC = pop(); IME = true; return 16;
	case 0xDA: { uint16_t addr = read16(); if (getC()) { PC = addr; return 16; } return 12; }
	case 0xDC: { uint16_t addr = read16(); if (getC()) { push(PC); PC = addr; return 24; } return 12; }
	case 0xDE: sbc8(read8()); return 8;
	case 0xDF: push(PC); PC = 0x18; return 16;

	case 0xE0: memory->write(0xFF00 + read8(), A); return 12;
	case 0xE1: HL = pop(); return 12;
	case 0xE2: memory->write(0xFF00 + C, A); return 8;
	case 0xE5: push(HL); return 16;
	case 0xE6: and8(read8()); return 8;
	case 0xE7: push(PC); PC = 0x20; return 16;
	case 0xE8: SP = addSP(); return 16;
	case 0xE9: PC = HL; return 4;
	case 0xEA: memory->write(read16(), A); return 16;
	case 0xEE: xor8(read8()); return 8;
	case 0xEF: push(PC); PC = 0x28; return 16;

	case 0xF0: A = memory->read(0xFF00 + read8()); return 12;
	case 0xF1: AF = pop() & 0xFFF0; return 12;
	case 0xF2: A = memory->read(0xFF00 + C); return 8;
	case 0xF3: IME = false; IME_nextCycle = false; return 4;
	case 0xF5: push(AF); return 16;
	case 0xF6: or8(read8()); return 8;
	case 0xF7: push(PC); PC = 0x30; return 16;
	case 0xF8: HL = addSP(); return 12;
	case 0xF9: SP = HL; return 8;
	case 0xFA: A = memory->read(read16()); return 16;
	case 0xFB: IME_nextCycle = true; return 4;
	case 0xFE: cp8(read8()); return 8;
	case 0xFF: push(PC); PC = 0x38; return 16;

	default: return 4;
	}
}

int SM83::executeCB()
{
	uint8_t opcode = read8();
	uint8_t regIdx = opcode & 7;
	uint8_t bit = (opcode >> 3) & 7;

	auto getReg = [&]() -> uint8_t
		{
			switch (regIdx)
			{
			case 0: return B;
			case 1: return C;
			case 2: return D;
			case 3: return E;
			case 4: return H;
			case 5: return L;
			case 6: return memory->read(HL);
			case 7: return A;
			}
			return 0;
		};

	auto setReg = [&](uint8_t val)
		{
			switch (regIdx)
			{
			case 0: B = val; break;
			case 1: C = val; break;
			case 2: D = val; break;
			case 3: E = val; break;
			case 4: H = val; break;
			case 5: L = val; break;
			case 6: memory->write(HL, val); break;
			case 7: A = val; break;
			}
		};

	if (opcode < 0x40)
	{
		uint8_t val = getReg();
		uint8_t result = val;

		switch ((opcode >> 3) & 7)
		{
		case 0: {
			uint8_t carry = (val & 0x80) >> 7;
			result = (val << 1) | carry;
			setC(carry);
			break;
		}
		case 1: {
			uint8_t carry = val & 1;
			result = (val >> 1) | (carry << 7);
			setC(carry);
			break;
		}
		case 2: {
			uint8_t carry = getC() ? 1 : 0;
			result = (val << 1) | carry;
			setC(val & 0x80);
			break;
		}
		case 3: {
			uint8_t carry = getC() ? 1 : 0;
			result = (val >> 1) | (carry << 7);
			setC(val & 1);
			break;
		}
		case 4: {
			result = val << 1;
			setC(val & 0x80);
			break;
		}
		case 5: {
			result = (val >> 1) | (val & 0x80);
			setC(val & 1);
			break;
		}
		case 6: {
			result = ((val & 0x0F) << 4) | ((val & 0xF0) >> 4);
			setC(false);
			break;
		}
		case 7: {
			result = val >> 1;
			setC(val & 1);
			break;
		}
		}

		setZ(result == 0);
		setN(false);
		setH(false);
		setReg(result);
		return regIdx == 6 ? 16 : 8;
	}
	else if (opcode < 0x80)
	{
		// BIT
		uint8_t val = getReg();
		setZ((val & (1 << bit)) == 0);
		setN(false);
		setH(true);
		return regIdx == 6 ? 12 : 8;
	}
	else if (opcode < 0xC0)
	{
		// RES
		uint8_t val = getReg();
		val &= ~(1 << bit);
		setReg(val);
		return regIdx == 6 ? 16 : 8;
	}
	else
	{
		// SET
		uint8_t val = getReg();
		val |= (1 << bit);
		setReg(val);
		return regIdx == 6 ? 16 : 8;
	}
}

uint8_t SM83::inc8(uint8_t val)
{
	setH((val & 0x0F) == 0x0F);
	val++;
	setZ(val == 0);
	setN(false);
	return val;
}

uint8_t SM83::dec8(uint8_t val)
{
	setH((val & 0x0F) == 0);
	val--;
	setZ(val == 0);
	setN(true);
	return val;
}

void SM83::add8(uint8_t val)
{
	int result = A + val;
	setH((A & 0x0F) + (val & 0x0F) > 0x0F);
	setC(result > 0xFF);
	A = result & 0xFF;
	setZ(A == 0);
	setN(false);
}

void SM83::adc8(uint8_t val)
{
	int carry = getC() ? 1 : 0;
	int result = A + val + carry;
	setH((A & 0x0F) + (val & 0x0F) + carry > 0x0F);
	setC(result > 0xFF);
	A = result & 0xFF;
	setZ(A == 0);
	setN(false);
}

void SM83::sub8(uint8_t val)
{
	setH((A & 0x0F) < (val & 0x0F));
	setC(A < val);
	A -= val;
	setZ(A == 0);
	setN(true);
}

void SM83::sbc8(uint8_t val)
{
	int carry = getC() ? 1 : 0;
	int result = A - val - carry;
	setH((A & 0x0F) < (val & 0x0F) + carry);
	setC(result < 0);
	A = result & 0xFF;
	setZ(A == 0);
	setN(true);
}

void SM83::and8(uint8_t val)
{
	A &= val;
	setZ(A == 0);
	setN(false);
	setH(true);
	setC(false);
}

void SM83::xor8(uint8_t val)
{
	A ^= val;
	setZ(A == 0);
	setN(false);
	setH(false);
	setC(false);
}

void SM83::or8(uint8_t val)
{
	A |= val;
	setZ(A == 0);
	setN(false);
	setH(false);
	setC(false);
}

void SM83::cp8(uint8_t val)
{
	setH((A & 0x0F) < (val & 0x0F));
	setC(A < val);
	setZ(A == val);
	setN(true);
}

void SM83::daa()
{
	uint8_t correction = 0;
	bool setCarry = false;

	if (getH() || (!getN() && (A & 0x0F) > 0x09))
	{
		correction |= 0x06;
	}

	if (getC() || (!getN() && A > 0x99))
	{
		correction |= 0x60;
		setCarry = true;
	}

	if (getN())
	{
		A -= correction;
	}
	else
	{
		A += correction;
	}

	setZ(A == 0);
	setH(false);
	setC(setCarry);
}

uint16_t SM83::add16(uint16_t a, uint16_t b)
{
	int result = a + b;
	setN(false);
	setH((a & 0x0FFF) + (b & 0x0FFF) > 0x0FFF);
	setC(result > 0xFFFF);
	return result & 0xFFFF;
};

uint16_t SM83::addSP()
{
	int8_t offset = (int8_t)read8();
	int result = SP + offset;
	setZ(false);
	setN(false);
	setH((SP & 0x0F) + (offset & 0x0F) > 0x0F);
	setC((SP & 0xFF) + (offset & 0xFF) > 0xFF);
	return result & 0xFFFF;
};

void SM83::handleInterrupts()
{
	if (!IME && !isHalted) return;

	uint8_t IF = memory->read(0xFF0F);
	uint8_t IE = memory->read(0xFFFF);
	uint8_t triggered = IF & IE & 0x1F;

	if (triggered)
	{
		isHalted = false;

		if (IME)
		{
			IME = false;

			for (int i = 0; i < 5; i++)
			{
				if (triggered & (1 << i))
				{
					memory->write(0xFF0F, IF & ~(1 << i));
					push(PC);
					PC = 0x40 + (i * 8);
					cycles += 20;
					break;
				}
			}
		}
	}
}