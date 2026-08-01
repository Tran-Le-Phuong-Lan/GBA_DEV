// SAVE & LOAD states in GBA
// reference: 
// 	1. libtonc-examples/ext/sbb_aff - for knowing how to text engine in tonc.h
// 	2. https://gbadev.org/docs.php?showinfo=13 
// How to save game state (especially with Supercard microSD, which uses SRAM)
// 20250115 - , Tran Le Phuong Lan & David Guttandin
// 

#include <tonc.h>

// ==========
// SAVE GAME
// ==========
#define WAITCNT_SRAM			0x0003	//set wait cycles for SRAM in WAITCNT

// ===========
// BG 0 - TEXT 
// ===========
#define CBB_0_TEXT 2  // tiles for text	
#define SBB_0_TEXT 31 // map for text


// === 
// 0. MISC 
// ===

void win_textbox(int bgnr, int left, int top, int right, int bottom, int bldy)
{
	REG_WIN0H= left<<8 | right; 
	REG_WIN0V=  top<<8 | bottom;
	REG_WIN0CNT= WIN_ALL | WIN_BLD;
	REG_WINOUTCNT= WIN_ALL;

	REG_BLDCNT= (BLD_ALL&~BIT(bgnr)) | BLD_BLACK;
	REG_BLDY= bldy; // fade levels

	REG_DISPCNT |= DCNT_WIN0;

	tte_set_margins(left, top, right, bottom);
}

// === 
// 0. MAIN GAME LOOP
// ===
void game_loop()
{
	u8	count[2] = {0,0};

	// Load the save state
	count[0] = sram_mem[0];
	count[1] = sram_mem[1];

	while(1)
	{
		VBlankIntrWait(); // check at the end of each frame
		key_poll();

		if (key_hit(KEY_B))
		{
			count[0] = count[0]+1;
			count[1] = count[1]+2;
		}

		if (key_hit(KEY_A))
		{
			// save game
			sram_mem[0] = count[0];
			sram_mem[1] = count[1];

		}

		tte_printf("#{es;P}count[0]: %d\ncount[1]: %d",
					count[0],count[1]);

	}
}

int main()
{
	// Init interrupts and VBlank irq.
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	// BG 0 = for text
	REG_DISPCNT= DCNT_MODE1 | DCNT_BG0;

	// init text
	// Init BG 0 for text on screen entries, using CBB 
	tte_init_chr4c_b4_default(0, BG_CBB(CBB_0_TEXT)|BG_SBB(SBB_0_TEXT));
	tte_init_con();
	// left, top, right, bottom
	// tte_set_margins(8, 120, 232, 156);
	//	void win_textbox(int bgnr, int left, int top, int right, int bottom, int bldy)
	//	bgnr = bg for windows; coordinates in unit [pixel]
	//			   l,       r,
	//				  t         b	
	win_textbox(0, 8, 110, 232, 156, 8);

	// set WAITCNT bit0-1 to 3 for SRAM save
	REG_WAITCNT = WAITCNT_SRAM;

	game_loop();

	return 0;
}
