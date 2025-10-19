#ifndef MBC_H
#define MBC_H

#include <cstdint>

class MBC
{
public:
	virtual ~MBC() = default;

	virtual uint8_t readROM(uint16_t addr) = 0;
	virtual uint8_t readRAM(uint16_t addr) = 0;
	virtual void writeROM(uint16_t addr, uint8_t val) = 0;
	virtual void writeRAM(uint16_t addr, uint8_t val) = 0;
};

class NoMBC : public MBC
{
public:
	uint8_t* rom;

	NoMBC(uint8_t* romData);
	uint8_t readROM(uint16_t addr) override;
	uint8_t readRAM(uint16_t addr) override;
	void writeROM(uint16_t addr, uint8_t val) override;
	void writeRAM(uint16_t addr, uint8_t val) override;
};

class MBC1 : public MBC
{
public:
	uint8_t* rom;
	uint8_t* ram;
	uint32_t romSize;
	uint32_t ramSize;

	uint8_t romBank;
	uint8_t ramBank;
	bool ramEnabled;
	uint8_t bankingMode;

	MBC1(uint8_t* romData, uint32_t romSize, uint32_t ramSize);
	~MBC1();

	uint8_t readROM(uint16_t addr) override;
	uint8_t readRAM(uint16_t addr) override;
	void writeROM(uint16_t addr, uint8_t val) override;
	void writeRAM(uint16_t addr, uint8_t val) override;
};

class MBC3 : public MBC
{
public:
	uint8_t* rom;
	uint8_t* ram;
	uint32_t romSize;
	uint32_t ramSize;

	uint8_t romBank;
	uint8_t ramBank;
	bool ramEnabled;

	MBC3(uint8_t* romData, uint32_t romSize, uint32_t ramSize);
	~MBC3();

	uint8_t readROM(uint16_t addr) override;
	uint8_t readRAM(uint16_t addr) override;
	void writeROM(uint16_t addr, uint8_t val) override;
	void writeRAM(uint16_t addr, uint8_t val) override;
};

class MBC5 : public MBC
{
public:
	uint8_t* rom;
	uint8_t* ram;
	uint32_t romSize;
	uint32_t ramSize;

	uint16_t romBank;
	uint8_t ramBank;
	bool ramEnabled;

	MBC5(uint8_t* romData, uint32_t romSize, uint32_t ramSize);
	~MBC5();

	uint8_t readROM(uint16_t addr) override;
	uint8_t readRAM(uint16_t addr) override;
	void writeROM(uint16_t addr, uint8_t val) override;
	void writeRAM(uint16_t addr, uint8_t val) override;
};




#endif