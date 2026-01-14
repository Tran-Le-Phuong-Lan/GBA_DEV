//
// map_block1.c
// showing map-block boundaries regular backgrounds
//
// (20031114 - 20060317, Cearn)

#include <tonc.h>

#define CBB_0  0
#define SBB_0 28

#define CROSS_TX 15
#define CROSS_TY 10

BG_POINT bg0_pt= { 0, 0 };
SCR_ENTRY *bg0_map= se_mem[SBB_0];


u32 se_index(u32 tx, u32 ty, u32 pitch)
{
	u32 sbb= ((tx>>5)+(ty>>5)*(pitch>>5));

	return sbb*1024 + ((tx&31)+(ty&31)*32);
}

void init_map()
{
	int ii, jj;

	// initialize a background
	REG_BG0CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_REG_64x64;
	REG_BG0HOFS= 0;
	REG_BG0VOFS= 0;

	// create the tiles: basic tile and a cross
	// 
	// According to GBATEK, 
	// For 4bpp Tile, 
	// 1 byte = 2 pixel;
	// in the memory, the first 4 byte = graphic of the first row,
	// the second 4 byte = graphic of the second row, and so on;
	// the lower 4 bit = color of the left pixel = LP
	// the upper 4 bit = color of the right pixel = RP
	// Therefore, in this example, 
	// the tile is 4bpp tile (because the type is `TILE`),
	// and the tiles[0] graphic is like this
	//   RP            LP
	// 0x1 1 1 1 1 1 1 1
	// 0x0 1 1 1 1 1 1 1
	// 0x0 1 1 1 1 1 1 1
	// 0x0 1 1 1 1 1 1 1
	const TILE tiles[2]=
	{
		{{0x11111111, 0x01111111, 0x01111111, 0x01111111,
		  0x01111111, 0x01111111, 0x01111111, 0x00000001}},
		{{0x00000000, 0x00100100, 0x01100110, 0x00011000,
		  0x00011000, 0x01100110, 0x00100100, 0x00000000}},
	};
	// ---original example code---
	// tile_mem[CBB_0][0]= tiles[0]; // pane tile
	// ---1. Modification to draw certain tile @ certain place ----
	// change the pane tile to another index to avoid the pane
	// automatically drawn everywhere by GBA.
	// So we can take control of where to plot the pane,
	// therefore understand better the tile data.
	tile_mem[CBB_0][2]= tiles[0]; // pane tile
	tile_mem[CBB_0][1]= tiles[1]; // cross tile

	// create a palette
	pal_bg_bank[0][1]= RGB15(31,  0,  0);
	pal_bg_bank[1][1]= RGB15( 0, 31,  0);
	pal_bg_bank[2][1]= RGB15( 0,  0, 31);
	pal_bg_bank[3][1]= RGB15(16, 16, 16);

	// Create a map: four contingent blocks of
	//   0x0000, 0x1000, 0x2000, 0x3000.
	SCR_ENTRY *pse= bg0_map;
	// ---original example code -----
	// for(ii=0; ii<4; ii++)
	// 	for(jj=0; jj<32*32; jj++)
	// 		*pse++= SE_PALBANK(ii) | 0;
	// ---1. Modification to draw certain tile @ certain place ---- 
	pse[0] = SE_PALBANK(0) | 0x0002; // SBB 0
	pse[1] = SE_PALBANK(0) | 0x0002; // SBB 0
	pse[31] = SE_PALBANK(0) | 0x0002; // SBB 0
	pse[32] = SE_PALBANK(0) | 0x0002; // SBB 0
	pse[1023] = SE_PALBANK(1) | 0x0002; // SBB 0, but green color to test
	pse[1024] = SE_PALBANK(1) | 0x0002; // SBB 1
	pse[1025] = SE_PALBANK(1) | 0x0002; // SBB 1
	pse[1055] = SE_PALBANK(1) | 0x0002; // SBB 1
	pse[2016] = SE_PALBANK(1) | 0x0002; // SBB 1
	// as mentioned in TONC tutorial - chapter 9 regular tiled background,
	// SE_PALBANK = bits C->F in SE 16-bit data, to set the pallette in case of 4bpp, no effect for 8bpp
	// C = 12th bit
	// F = 15th bit
	// SE_PALBANK(n) = n << 12 (shift 12 bit to the left) 
	//				 = n written in form of bits then shift the bits to the left 12 steps
	//				 = the author is using the bits in n and shift all of them to the position of C-> F bit
	//			     = in the SE 16-bit data.

}

int main()
{
	// Init interrupts and VBlank irq.
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	init_map();
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ;

	u32 tx, ty, se_curr, se_prev= CROSS_TY*32+CROSS_TX;

	bg0_map[se_prev]++;	// initial position of cross
	while(1)
	{
		VBlankIntrWait();

		key_poll();
		bg0_pt.x += key_tri_horz();
		bg0_pt.y += key_tri_vert();

		// Testing bg_se_id()
		// If all goes well the cross should be around the center of
		// the screen at all times.
		tx= ((bg0_pt.x>>3)+CROSS_TX) & 0x3F;
		ty= ((bg0_pt.y>>3)+CROSS_TY) & 0x3F;

		se_curr= se_index(tx, ty, 64);
		if(se_curr != se_prev)
		{
			bg0_map[se_prev]--;
			bg0_map[se_curr]++;
			se_prev= se_curr;
		}

		REG_BG_OFS[0]= bg0_pt;	// write new position
	}
	return 0;
}
