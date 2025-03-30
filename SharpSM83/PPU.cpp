#include "PPU.h"
#include "SM83.h"
#include "memory.h"
#include <iostream>

/*
* Pixels are 8x8 squares 
* 0 - 3  bits 
* background is composed of tilemap
* window is second bacground 
* object layer, can be displayed anywhere on screen
* 8*8 or 16*16
* 
* memeory is 0x8000 to 0x97FF
* 
* each tile takes 16 bytes
* 0 - 4 is white to black
* on an object , 0 means transparent
* 
* two types of adressing
* 
* 0x8000 - base pointer 
* unsiged addressing
* tile 0 -127 is in b0
* tile 128 - 255 in b1
* 
* 0x8800 , 0x9000 is the base pointer
* 
* 0-127 in b2
* -128 to -1 in block 1
* 
* objects always use 0x8000
* 
* BG and Window can use both - controlled by the LCDC bit 4 
* 
* each tile is 16 bytes 
* each line is 2 bytes 
* so top line is bite 1 + bite 2 
* 
* palette consists of an array of 4 different colors 
* 
* each tile map contains the 1 byte indexes of the tiles to be displayed 
* 
* Tiles are obtained from the tile 
* data table using either of the two 
* addressing modes 
* 
* one byte , 8 bit 
* 32 * 32 tile map (8 (4  *  4) )
* 
* backGround (bg) SCY SCX
* 
* the background can be disabled with LCDC
* 
* WINDOW is not scrollable
* 
* you can modify pos of window on S creen
* using WX and WY registers 
* Screen co-ords (WX- 7 , WY)
* 
* LCDC  bit 5 ~ toggles wether the window is displayed#
* LCDC bit 0 must also be set 
* 
* Enabling the window makes mode 3 slightly longer when its visible
* 
* The window keeps an internal line counter similair to ly
* it increments along it 
* howwveer it only gets incremented when visiable 
* 
* line counter determines what window line is to be rendered on the current scanline 
*
* obkect attribud memory 
* gameboy ppu can display 40 items at a time 
* each 8*8 or 16*16
* 
* only ten objects can be displayed per scanline
* 
* Screen+16~ is postition on screen
* so y=16 appears at very top pixel
* y = 0 iss 16px off screen
* 
* x is the same but 8px , x=0 is off screen
*  x = 8 is just about on screen
* x can alo be 8 pixels extra of screen
* 
* in 8*8 mode, LCDC bit 2 = 0
* 
* OAM 0 FE00 to FE8F
* 40 attribs with 4 bytes each
* 
* Y = byte 0
* X = byte 1 
* TileIndex = byte 2 (specifies objects tile index)
* if [] byte = 0 -> 8*8 mode
* if [] byte 1 -> 8*16 mode 
* attrib/flag = byte 3 
* 7 -> priority |(0-No,1-bg and window are drawn over obj)
* 6 -> y flip
* 5 -> x flip
* 4 -> DMG palette
* 
* The reccomended way to OAM 
* write data to a buffer in normal ram (0xFF46)
* copy that to OAM using DMA transfer
* //=========================/
* Selection Priority 
* 
* during each scaline OAM scan, PPU compares LY to each objects Y position 
* y- 0 will still draw to the scanline 10 limtit
* BG over OBJ
* 
* 0xFF46 DMA
* Writing to this address starts a DMA transfer from ROM or RAM to OAM. THe written Value specifies the transfer source address divided by ox100
* 
* 
* source : 0xXX00 - 0xXX9F
* destination 0xFE00 - 0xFE9F
* 
* the written value specifies transfer source divided by 0x100
* XX  = 0x00 to 0xDF - the adress is * 0x100
* 
* transfer takes 160 machine cycles
* 
* OAM DMA bus conflicts 
* during OAM DMA , cpu can only access 
* 0xFF80 - 0xFFFE
* 
* for this reason, programmer must copy a short procedure into hram
* use this procedure to start tranfer from inside hram
* wait until transfer is finished
* 
* While an OAM DMA is in progress
* the PPU cannot read OAM properly 
* 
* most programs execute DMA while in mode 1
* also possible to handle during redraw (2 or 3)
* 
* LCD control 
* 0b
* 
* bit 7 - LCD on / PPU active
* bit 6 - Window tile map area 0 : 9800 - 9bFF , 9C00 - 9FFF
* bit 5 - Window Enable 0 off 0 1 on
* bit 4 Bg & window TIles 
* bit 3 BG tile map area 
* bit 2 0 ~ 8x8 , 1 ~ 8x16 (Obj Size )
* bit 1 0 ~ off , 1 ~ on (Obj enable)
* bit 0 0 ~ BG & window enable/ priority 
* 
* LcDC ~ very powerful, since it can be modified any time during the frame 
* 
* 
* frame is drawn in 154 scanlines
* first 144 drawn top to bottom, left to right 
* you can modify rendering paramters halfway through
* 4 dots per M-Cycle
* ppu togles between mode2 (OAM scan) , mode3 (drawing pixels), mode 0 (horizontal blank)
* while ppu is accessing vram, that mem is inacessible to cpu
* 
* Mode 2 ~ searching for objs which overlap current line , can access VRAM
* Mode 3 ~ Sends pixels to the lcd , cant acess anything
* Mode 0 ~ Waits until the end of the scanline 
*/


/*
	stat summary => 
	
	0xFF40 LCDC control 
	0xFF41 LCD Status STAT interrupt ~~ 0b6 , if set, selects LYC = LY condition
										0b5 ~ mode 2, b4 mode 1, b3 mode 0
										0b2 ~ set when LY contains LYC, constantly updated
										0b1 / 0b0 ~ reposrts ppu current status, 0 when ppu is off

	0xFF42 SCY ~ specify topLeft coord of visible 256*256 in 160*144 screen
	0xFF43 SCX ~ same but for x // bottom = (SCY + 143) % 256 and right := (SCX + 159) % 256


	0xFF44 LCD Y co-ord ~~ indicated the current horizontal line being drawn
	0xFF45 LY compare ~~ LYC and LY contantly compares, when both identical, LYC=LY flag is set in STAT

	0xFF46 OAM DMA source address + start

	0xFF47 ~ bgp = BG pallette data (0b76 = ID3, 54->2 , 32-> 1, 10->0)
	0xFF48 ~ obp0 data
	0xFF49 ~ obp1 data

	0xFF4A WindowX ~ on screen coordinate of windows top left pixel
	0xFF4B WindowY ~ same here

	//-------------------------------
	0xFF4c ~ doesnt exist 
	0xFF4D ~ prepare speed switch
*/

uint16_t Mem{}; //<- not sure why i added this, may delete later 

#define LCDC 0xFF40; // <- this is the most important one 
#define STAT 0xFF41; // lcd status
#define	SCY  0xFF42; // screen y 
#define	SCX  0xFF43; // screen x
#define LY   0xFF44; // LCD Y coordinate
#define LYC  0xFF45; // LY compare
#define DMA  0xFF46; // OAM dma
#define BGP  0xFF47; // background pallete
#define OBP0 0xFF48; // obj p 0
#define OBP1 0xFF49; // obj p 1
#define WY   0xFF4A; // window x 
#define WX   0xFF4B; // window y 

PPU::PPU(Memory& memory) : memory(memory) //m cycle (using this) = 4 t states
{

}

void PPU::resetPPU()
{

}
uint8_t screen[160][144];

/*
* 
2	Searching for OBJs which overlap this line	80 dots						VRAM, CGB palettes
3	Sending pixels to the LCD					Between 172 and 289 dots,	see below	None
0	Waiting until the end of the scanline		376 - mode 3’s duration		VRAM, OAM, CGB palettes
1	Waiting until the next frame				4560 dots (10 scanlines)	VRAM, OAM, CGB palettes
*
*/

/*	OAM each sprite is 4 bytes
	BYTE 0 - y (-16)
	BYTE 1 - x (-8)
	BYTE 2 - tile num (always uses 0x8000 indexing)
	BYTE 3 - flag
*/

/*
Bit 7    OBJ-to-BG Priority
		  0 = Sprite is always rendered above background
		  1 = Background colors 1-3 overlay sprite, sprite is still rendered above color 0
Bit 6    Y-Flip
		  If set to 1 the sprite is flipped vertically, otherwise rendered as normal
Bit 5    X-Flip
		  If set to 1 the sprite is flipped horizontally, otherwise rendered as normal
Bit 4    Palette Number
		  If set to 0, the OBP0 register is used as the palette, otherwise OBP1
Bit 3-0  CGB-Only flags
*/


uint8_t spriteBuffer[40]{}; //10 4 byte sprits
uint8_t spriteBufferPointer{};

uint16_t oamSearch = 0;
int internalTick{};
uint8_t currentLY{};

uint8_t backgroundFifo{};
uint8_t spriteFifo{};

bool mode3Finished = false;
int mode3MCycles{};

uint8_t currentTileI{};

int pixelsX{};
uint8_t lcdc;
uint8_t windowLine{};
uint8_t lowerByte{};
uint8_t upperByte{};


void PPU::mode2Tick() // search for objs on this current line 
{ // each sprite takes up 4 bytes 
	/*
	
	This mode is entered at the start of every scanline (except for V-Blank) before pixels are actually drawn to the screen. 
	During this mode the PPU searches OAM memory for sprites that should be rendered on the current scanline and stores
	them in a buffer. This procedure takes a total amount of 80 T-Cycles,
	meaning that the PPU checks a new OAM entry every 2 T-Cycles.

	A sprite is only added to the buffer if all of the following conditions apply:

	Sprite X-Position must be greater than 0
	LY + 16 must be greater than or equal to Sprite Y-Position
	LY + 16 must be less than Sprite Y-Position + Sprite Height (8 in Normal Mode, 16 in Tall-Sprite-Mode)
	The amount of sprites already stored in the OAM Buffer must be less than 10
	
	
	*/
	//FE00-$FE9F

	//sprite is composed of 4 bytes / 16 bits
		//we are checking if there is a sprite currently on this line
		//that is contained in byte 0 (-16)
		// so for LY = 0 , were looking for any byte 0=16
		// for LY = 1 , we are looking for any byte 0 = 17
		//if so, add the entire 4 byte to an array
	//std::cout << "currently searching" <<  << std::endl;

	uint8_t spriteY = memory.read(0xFFE0 + oamSearch);

	uint8_t currentModeHeight = 8;

	if (memory.ioFetchLCDC() & 0b00000100) //grab if its in tall mode or not 
	{
		currentModeHeight = 16; // make this swap later 
	}
	

	if (currentLY + 16 >= spriteY && currentLY + 16 <= spriteY + currentModeHeight) // this is the byte 0
	{
		// add this entire sprite to the buffer
		uint8_t spriteX = memory.read(0xFFE1 + oamSearch);
		if (spriteX > 0) // not sure if this means 0 as in 0 or 8
		{
			if (spriteBufferPointer <= 36) // if theres room for another 4 bytes (36,37,38,39)
			{

				spriteBuffer[spriteBufferPointer] = spriteY;
				spriteBufferPointer++;
				spriteBuffer[spriteBufferPointer] = spriteX;
				spriteBufferPointer++;
				spriteBuffer[spriteBufferPointer] = memory.read(0xFFE2 + oamSearch);
				spriteBufferPointer++;
				spriteBuffer[spriteBufferPointer] = memory.read(0xFFE3 + oamSearch);
				spriteBufferPointer++;

				std::cout << "Added sprite to the pointer" << std::endl;

			}
		}
	}

	oamSearch += 0x10;
	
}


void PPU::mode3Tick() // send pixels to the lcd
{ // should take between 172 and 289 t cycles  (43 and 72.25 m cycles)

	//pixelFetcher is ran continuesly through mode3
	// has 4 steps, each taking 2 t cycles
	// 2 steps per m cycle
	if (pixelsX >= 160)
	{
		mode3Finished = true;
		return;
	}

	lcdc = memory.ioFetchLCDC();
	if (internalTick % 2 == 0) // if even ticks
	{
		fetchTileNo();
		fetchTileL();
	}
	else
	{
		fetchTileH();
		fifoPush();
	}
}

void PPU::mode0Tick()
{

}

void PPU::mode1Tick()
{

}
//The component responsible for loading the FIFO registers with data is the Pixel Fetcher.This fetcher is continuously active throughout PPU Mode 3 and keeps supplying the FIFO with new pixels to shift out.The process of fetching pixels is split up into 4 different steps, which take 2 T - Cycles each to complete :
//
//1) Fetch Tile No.: During the first step the fetcher fetches and stores the tile number of the tile which should be used.Which Tilemap is used depends on whether the PPU is currently rendering Background or Window pixels and on the bits 3 and 5 of the LCDC register.Additionally, the address which the tile number is read from is offset by the fetcher - internal X - Position - Counter, which is incremented each time the last step is completed.The value of SCX / 8 is also added if the Fetcher is not fetching Window pixels.In order to make the wrap - around with SCX work, this offset is ANDed with 0x1f. An offset of 32 * (((LY + SCY) & 0xFF) / 8) is also added if background pixels are being fetched, otherwise, if window pixels are being fetched, this offset is determined by 32 * (WINDOW_LINE_COUNTER / 8).The Window Line Counter is a fetcher - internal variable which is incremented each time a scanline had any window pixels on it and reset when entering VBlank mode.
//
//Note: The sum of both the X - POS + SCX and LY + SCY offsets is ANDed with 0x3ff in order to ensure that the address stays within the Tilemap memory regions.
//
//2) Fetch Tile Data(Low) : Using the Tile Number from the previous step the fetcher now fetches the first byte of tile data(with an offset of 2 * ((LY + SCY) mod 8)) and stores it.The address which the tile data is read from depends on bit 4 of the LCDC register.
//
//Note : While fetching window pixels, the offset of 2 * ((LY + SCY) mod 8) is replaced with 2 * (WINDOW_LINE_COUNTER mod 8).
//
//3) Fetch Tile Data(High) : This step is the same as the previous, however, the next byte after the previously read address(containing the second byte of Tile Data) is read and stored.
//
//Note : The first time the background fetcher completes this step on a scanline the status is fully reset and operation restarts at Step 1. Due to the 3 steps up to this point taking up 6 T - cycles in total, and the same steps repeating again taking the same amount of time, this causes a delay of 12 T - cycles before the background FIFO is first filled with pixel data.
//
//4) Push to FIFO : During this step the previously fetched pixel data is decoded into actual pixels(containing all the attributes mentioned previously) and loaded into the corresponding FIFO, depending on whether the Fetcher is currently fetching Background / Window or Sprite pixels.
//
//Note : While fetching background pixels, this step is only executed if the background FIFO is fully empty.If it is not, this step repeats every cycle until it succeeds.Since all steps up to this point usually only take 6 T - cycles, and the PPU takes 8 T - cycles to shift out all 8 pixels, this step usually has to restart twice before succeeding.


//$9800-$9BFF ~ Tile Maps 
//$9C00-$9FFF ~ Tile Maps



void PPU::fetchTileNo()
{
	// just before we fetched the current lcdc
	//now check if we are rendering window or bakcground
	uint16_t startingAddr{};
	uint8_t wy = memory.read(0xFF4A);
	uint8_t wx = memory.read(0xFF4B);

	uint8_t tileX, tileY;


	if ((!(lcdc & 0b01000000)) && currentLY >= wy && pixelsX >= wx - 8)
	{
		//windowTileMap ~ depends on bit 6 
		if (lcdc & 0b1000000)	startingAddr = 0x9800;
		else					startingAddr = 0x9C00;

		tileX = (pixelsX - (wx - 7)) / 8;  
		tileY = (windowLine / 8); 
		windowLine++;
	}
	else
	{
		//backgroundTileMap ~ depends on bit 3 
		if (lcdc & 0b1000)		startingAddr = 0x9800;
		else					startingAddr = 0x9C00;

		tileX = ((pixelsX + memory.read(0xFF43)) / 8) & 0x1F;  
		tileY = ((currentLY + memory.read(0xFF42)) / 8) & 0x1F; 
	}

	uint16_t tileIndexAddr = startingAddr + (tileY * 32) + tileX;
	currentTileI = memory.read(tileIndexAddr); 
}
void PPU::fetchTileL()
{
	//first thing, check if were using standard indexing mode or alternate
	uint16_t tileBase = 0x8000;
	if (lcdc & 0b10000) //0x8000 mode 
	{
		tileBase += (currentTileI * 16);
	}
	else
	{
		tileBase = (0x9000+((int8_t)currentTileI * 16)); // cast to signed int 
	}

	lowerByte = memory.read(tileBase + (((currentLY + memory.read(0xFF42)) % 8) * 2) );
}
void PPU::fetchTileH()
{
	uint16_t tileBase = 0x8000;
	if (lcdc & 0b10000) //0x8000 mode 
	{
		tileBase += (currentTileI * 16);
	}
	else
	{
		tileBase = (0x9000 + ((int8_t)currentTileI * 16)); // cast to signed int 
	}

	upperByte = memory.read(tileBase + (((currentLY + memory.read(0xFF42)) % 8) * 2));
}
void PPU::fifoPush()
{

}


void PPU::executeTick(int allCycles) // measured in m cycles
{									 // 4 dots = 1 m cycle
	
	//mode 2, always 80 dots long  / 20 m cycles
	
	//figure out the current scanline LY  (0xFF44)
	if (internalTick == 0) {
		currentLY = memory.ioFetchLY();
	}
	
	if (internalTick < 20) 
	{
		mode2Tick();
		mode2Tick();
	}
	else if(!mode3Finished)
	{
		mode3Tick();
		mode3MCycles++;
	}
	else
	{

	}
	internalTick++;
}



//void PPU::executeFrame()
//{
//	//MODE 2 ~ OAM scan
//	// search for OBJS which overlap this line
//	
//	//MODE 3 ~ one pixel per dot from left to right, up to down
//}

// so basically, we will be told the current amount of frames 
// 70224 ticks or 17556 frames 
