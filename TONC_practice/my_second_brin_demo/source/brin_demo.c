//
// brin_demo.c
// Essential tilemap loading: the start of brinstar from metroid 1
//
// (20060221 - 20070216, cearn)

#include <string.h>

// #include "toolbox.h"
// #include "input.h"
// #include "brin.h"
#include <tonc.h>
#include "First_mapscrn.h"

int main()
{
		// Init interrupts and VBlank irq.
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	// Load palette
	memcpy16(pal_bg_mem, First_mapscrnPal, First_mapscrnPalLen / sizeof(u16));
	// Load tiles into CBB 0 = 0x600:000
	memcpy32(&tile_mem[0][0], First_mapscrnTiles, First_mapscrnTilesLen / sizeof(u32));

	// Load map into SBB 30 = 0x600:F000
	memcpy32(&se_mem[30][0], First_mapscrnMap, First_mapscrnMapLen / sizeof(u32));

	// set up BG0 for a 4bpp 32x32t map, using
	//   using charblock 0 and screenblock 31
	REG_BG0CNT= BG_CBB(0) | BG_SBB(30) | BG_4BPP | BG_REG_32x32;
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0;

	// Scroll around some
	// int x= 0, y= 0;
	while(1)
	{
		// vid_vsync();
		// key_poll();

		// x += key_tri_horz();
		// y += key_tri_vert();

		// REG_BG0HOFS= x;
		// REG_BG0VOFS= y;
	}

	return 0;
}
