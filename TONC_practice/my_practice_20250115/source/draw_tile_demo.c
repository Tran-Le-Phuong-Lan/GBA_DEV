//
// obj_demo.c
// testing various sprite related things
//
// (20031003 - 20060924, Cearn)

#include <string.h>
#include <tonc.h>

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

#define CBB_0  0
// ? while some graphic error when SBB_0  = 0 
#define SBB_0  1
SCR_ENTRY *bg0_map= se_mem[SBB_0];

void init_reg_bg ()
{
		// initialize a background
	REG_BG0CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_REG_32x32;
	REG_BG0HOFS= 0;
	REG_BG0VOFS= 0;

		const TILE tiles[1]=
	{
		// bg tile
		{{0x10000001, 0x01111110, 0x01111110, 0x01111110,
		  0x01111110, 0x01111110, 0x01111110, 0x10000001}},
	};

	// Place the bg tile 
	// into VRAM: cbb == 0
	tile_mem[CBB_0][0] = tiles[0];

	// create and place a palette for bg
	//			P  C
	pal_bg_bank[0][1]= RGB15(31,  0,  0);
	pal_bg_bank[1][1]= RGB15(0,  31,  0);
	pal_bg_bank[2][1]= RGB15(0,  0,  31);
	pal_bg_bank[3][1]= RGB15(31,  31,  0);

}

void init_reg_obj ()
{
		const unsigned int tiles[128]=
	{
		// cursor tile 64x64p
			// tile 1 , Graphic RIGHT -> Graphic LEFT
			0x01111111, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
			// tile 2
			0x11111111, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
			//tile 3
			0x11111111, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
			// tile 4
			0x11111110, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000,

			// tile 5
			0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
			// tile 6
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
			// tile 7
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
			// tile 8
			0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000,

			// tile 9
			0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
			// tile 10
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
			// tile 11
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
			// tile 12
			0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000,

			// tile 13
			0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x01111111,
			// tile 14
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x11111111, 
			//tile 15
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x11111111,
			// tile 16
			0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x11111110

	};
	

	// creat a obj palette (already put into the OBJPALMEM)
	pal_obj_bank[0][1]= RGB15(0,  31,  0);
	pal_obj_bank[1][1]= RGB15(0,  0, 31);
	
	// Place the cursor 
	// into VRAM: LOW obj memory (cbb == 4)
	// tile_mem[4][0] = tiles[0];

	memcpy32(&tile_mem[4][0], tiles, 128);
	// memcpy(pal_obj_mem, LinkZeldaPal, LinkZeldaPalLen);
}

void obj_test()
{
	int x= 0, y= 0; // [pixel]

	u32 tid= 0, pb= 0;		// tile id, pal-bank

	OBJ_ATTR *cursor= &obj_buffer[0];
	obj_set_attr(cursor, 
		ATTR0_SQUARE,				// Square, regular sprite
		ATTR1_SIZE_32,					// 32x32p,
		// the object priority is zero
		ATTR2_PALBANK(pb) | tid);		// palbank 0, tile 0

	// set the starting position of the obj
	// x,y [pixel] = position of the top-left pixel of the obj
	obj_set_pos(cursor, x, y);

	// enable a timer
	REG_TM2D =  -0x065E;
	REG_TM2CNT= TM_ENABLE | TM_FREQ_1024; 
	// 1 period = 1024 default clock cycle
	// = 1024 * (1/16.78 Mhz) = 61 us 

	// tick every ~ 100ms
	REG_TM3CNT=  TM_ENABLE | TM_CASCADE; 
	
	u32 sec = -1;

	while(1)
	{
		VBlankIntrWait();
		key_poll();
		
		// // increment/decrement starting tile with R/L
		// tid += bit_tribool(key_hit(-1), KI_R, KI_L);

		
		SCR_ENTRY *pse= bg0_map;
		u16 obj_x_coord, obj_y_coord;
		u32 se_curr;
		// get the cursor (object) position (using Tonc BF_GET())
		obj_x_coord = BFN_GET(cursor->attr1, ATTR1_X);
		obj_y_coord = BFN_GET(cursor->attr0, ATTR0_Y);
		// calculate the Se_index, map size 32x32t
		se_curr = (obj_y_coord >> 3)*32 + (obj_x_coord >>3);
		// draw a green tile
		if(key_hit(KEY_A))
			{
				// update the palette according to Se_index
				// use the tonc `qran_range` to generate the
				// random palette.
				pse[se_curr] = SE_PALBANK(qran_range(1, 4)) | 0;
			};
		// redo a wrong green tile	
		if(key_hit(KEY_B))
			{
				pse[se_curr] = SE_PALBANK(0) | 0;
			};
		
		// make it change color (via palette swapping)
		pb= key_is_down(KEY_SELECT) ? 1 : 0;

		// // toggle mapping mode
		// if(key_hit(KEY_START))
		// 	REG_DISPCNT ^= DCNT_OBJ_1D;

		// Hey look, it's one of them build macros!
		cursor->attr2= ATTR2_BUILD(tid, pb, 0);

		// debounce key algorithm
		// https://www.digikey.com/en/maker/tutorials/2024/how-to-implement-a-software-based-debounce-algorithm-for-button-inputs-on-a-microcontroller
		// The easiest solution is to have a timer (to mark a debouncing period, during which no button state is updated)
		// So we need to use timers?
		// More easy solution, but not the best:
		// move left/right
		
		// start timer, wait for 10ms ~ 163 ticks
		if (REG_TM3D != sec)
		{
			sec = REG_TM3D;

			x += 32*key_tri_horz();
			// move up/down
			y += 32*key_tri_vert();
			obj_set_pos(cursor, x, y);
		}
		// x += 1*horz_prev;
		// move up/down
		// y += 1*vert_prev;
		// snap to 8x8 pixel TILE grid (defined by the bg)
		// snap_x = (x >> 3) << 3; 
		// snap_y = (y >> 3) << 3;
		// // if (snap_x+ )
		// obj_set_pos(cursor, snap_x, snap_y);
		

		// count = how many sprites to update.
		// even though there are 128 sprite objs are initiated 
		// (as the author already explains, this step of initialization of all 128
		// sprite obj is to avoid the weird behavior of the hardware. The weird behavior is
		// when OAM is all 0s, the screen will be populated with the graphic sprite tid 0.
		// The initialization is important to hide these unmeaningful graphics on the screen),
		// we actually have only one obj metr starting at tid 0 defined 
		// (see the code line 27 of this file). 
		// In this example, there is only 1 sprite of size 64 pixel x 64 pixel (= 8 TILE x 8 TILE)
		oam_copy(oam_mem, obj_buffer, 1);	// only need to update one
	}
}

int main()
{
	// Init interrupts and VBlank irq.
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	init_reg_obj();
	init_reg_bg();

	// init the OAM, and make sure that 
	// the all the sprites (all 128 sprites) are hidden at the beginning.
	// The author inits 128 sprites, 
	// even though there is only one sprite (= 1 data of a sprite image stored in VRAM).
	// the function is in `./toolbox.c` 
	oam_init(obj_buffer, 128);
	
	// #define DCNT_OBJ			0x1000	//!< Enable objects
	// DCNT_OBJ = set 12th-bit to 1 =
	// in GBATEK- LCD I/O Display Control, 
	// it is defined as Screen Display OBJ  (0=Off, 1=On).
	// #define DCNT_OBJ_1D			0x0040	//!< OBJ-VRAM as array
	// DCNT_OBJ_1D = set 6th-bit to 1 =
	// using 1D sprite map mode to load the tile data.
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 |DCNT_OBJ | DCNT_OBJ_1D;

	obj_test();

	while(1);

	return 0;
}
