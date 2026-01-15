//
// cbb_demo.c
// showing char-block boundaries for 4 and 8-bit tile-maps
//
// (20031114 - 20060924, cearn)

#include <tonc.h>

#include "cbb_ids.h"

#define CBB_4 0
#define SBB_4 2

#define CBB_8 2
#define SBB_8 4

void load_tiles()
{
	int ii;
	TILE *tl= (TILE*)ids4Tiles;
	TILE8 *tl8= (TILE8*)ids8Tiles;

	// Loading tiles. don't get freaked out on how it looks
	// 4-bit tiles to blocks 0 and 1
	//				TID 1						TID 2
	tile_mem[0][1]= tl[1];		tile_mem[0][2]= tl[2];

	//				TID 512						TID 513
	// 1 CBB = 512 TILE, and this = CBB_4 + 1
	tile_mem[1][0]= tl[3];		tile_mem[1][1]= tl[4];

	//---------
	// and the 8-bit tiles to blocks 2 though 5
	//				 TID 1						 TID 2	
	tile8_mem[2][1]= tl8[1];	tile8_mem[2][2]= tl8[2];
	
	//				 TID 256					 TID 257
	// 1 CBB = 256 TILE8, and this is = CBB_8 + 1
	tile8_mem[3][0]= tl8[3];	tile8_mem[3][1]= tl8[4];
	
	//				 TID 512					 TID 513
	// this is CBB_8 + 2 (== CBB_OBJ_LO)
	tile8_mem[4][0]= tl8[5];	tile8_mem[4][1]= tl8[6];
	
	//				 TID 768					 TID 769
	// this is CBB_8 + 3 (== CBB_OBJ_LO)
	tile8_mem[5][0]= tl8[7];	tile8_mem[5][1]= tl8[8];

	// And let's not forget the palette (yes, obj pal too)
	u16 *src= (u16*)ids4Pal;
	for(ii=0; ii<16; ii++)
		pal_bg_mem[ii]= pal_obj_mem[ii]= *src++;
}

void init_maps()
{
	//							x,y
	// se4 and se8 map coords: (0,2) and (0,8)
	SCR_ENTRY *se4= &se_mem[SBB_4][2*32], *se8= &se_mem[SBB_8][8*32];
	// show first tiles of char-blocks available to bg0
	// tiles 1, 2 of char-block CBB_4
	// se4 is a point start at the pixel (0,2): 2nd row (r), 0th column (col) 
	// 0x01 = 1				|	0x02 = 2
	// = 2nd r, 1st col;    |	= 2nd r, 2nd col;
	// 0x0001 = TID 1		|	0x0002 = TID 2		   				
	se4[0x01]= 0x0001;		se4[0x02]= 0x0002;

	// tiles 0, 1 of char-block CBB_4+1
	// 0x20 = 32			|	0x21 = 33
	// = 3rd r, 0th col;    |	= 3rd r, 1st col;
	// 0x0200 = TID 512;	|	0x201= 513;
	se4[0x20]= 0x0200;		se4[0x21]= 0x0201;

	// show first tiles of char-blocks available to bg1
	// tiles 1, 2 of char-block CBB_8 (== 2)
	se8[0x01]= 0x0001;		se8[0x02]= 0x0002;

	// tiles 1, 2 of char-block CBB_8+1
	//  0x0100 = TID 256	|	0x0101 = TID 257
	// 1 CBB = 256 TILE8					
	se8[0x20]= 0x0100;		se8[0x21]= 0x0101;

	// tiles 1, 2 of char-block CBB_8+2 (== CBB_OBJ_LO)
	//  0x0200 = TID 512	|	0x0201 = TID 513 
	se8[0x40]= 0x0200;		se8[0x41]= 0x0201;

	// tiles 1, 2 of char-block CBB_8+3 (== CBB_OBJ_HI)
	//  0x0200 = TID 512	|	0x0201 = TID 513
	se8[0x60]= 0x0300;		se8[0x61]= 0x0301;
}

int main()
{
	load_tiles();
	init_maps();

	// init backgrounds
	REG_BG0CNT= BG_CBB(CBB_4) | BG_SBB(SBB_4) | BG_4BPP;
	REG_BG1CNT= BG_CBB(CBB_8) | BG_SBB(SBB_8) | BG_8BPP;
	// enable backgrounds
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ;

	while(1);

	return 0;
}
