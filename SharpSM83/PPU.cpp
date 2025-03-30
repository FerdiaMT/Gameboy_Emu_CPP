#include "PPU.h"
#include "SM83.h"
#include "memory.h"

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

#define LCDC 0xFF40;
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

void PPU::executeFrame()
{
	//MODE 2 ~ OAM scan
	// search for OBJS which overlap this line
	
	//MODE 3 ~ one pixel per dot from left to right, up to down
}

// so basically, we will be told the current amount of frames 
// 70224 ticks or 17556 frames 
