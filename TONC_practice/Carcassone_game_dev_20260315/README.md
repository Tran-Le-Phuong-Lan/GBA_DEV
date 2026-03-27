- Mode 1: bg 0 - reg, bg 1 - reg, bg 2 - aff, bg 3 - not used

- bg2 -aff = carcassonne game background, linear map layout (using array row*w + col for addressing SE), 256 color pallete only

    - **SE of aff bg is only 8 bit, which is called SAE (screen affine entry, in TONC). SE, in TONC and in this context, is screen entry of regular background** ! instead of 16-bit SE in case of reg bg

    - each **tile in aff bg** is **8x8p@8bpp = DTILE** instead of STILE (8x8p@4bpp)

    - **SE is in VRAM**, which only allows **16-/32-bit write** => each time, we have to **write 2/4 DTILEs at once, DTILE = SAE (screen affine entry).**

    - **on the screen**, we are seeing **SAE tiles** !. The step of **converting pixel unit into Tile unit = converting pixel unit into SAE tile unit**. Because we can not write to VRAM in 8-bit, we need to write to SAE through SE. Therefore, we need to **convert SAE index to SE index in order to use `pse`** (`SCR_ENTRY *bg0_map= se_mem[SBB_0]; SCR_ENTRY *pse= bg0_map;`). see the code below (extracted from the `./source/draw_tile_demo.c`) 

    ```
    SCR_ENTRY *pse= bg0_map;
    u16 obj_x_coord, obj_y_coord; // in unit [pixel]
    u32 se_curr;
    u32 sae_curr;
    ...
    // calculate the Se_index, map size 32x32t
    // >> 3: divided by 8 to convert to unit [tile]
    //							 32 = width of the map size in unit [tile]	
    sae_curr = (obj_y_coord >> 3)*32 + (obj_x_coord >>3);
    se_curr = sae_curr >> 1;
    ```

    - Because each time we adress 2 adjacent SAEs at once, in order to write to a specific SAE among the two, the code below (extracted from the `./source/draw_tile_demo.c`) is the solution

    ```
        if (sae_curr % 2 == 0)
    {
        // write to lower 8-bit of pse, preseve the higher 8-bit of pse
        pse[se_idx] = (pse[se_idx] & 0xFF00) | ((cas_tile_map_id[rand_cat][0*3 + 0]+1) & 0x00FF);
    }
    else
    {
        // write higer 8-bit of pse, preserve the lower 8 bit of pse
        pse[se_idx] = (pse[se_idx] & 0x00FF) | (((cas_tile_map_id[rand_cat][0*3 + 0]+1)<<8)  & 0xFF00);
    }
    ``` 

- From [GBATEK](https://problemkaputt.de/gbatek.htm#lcdcolorpalettes), 

> Note that some OBJs may access palette RAM in 16 color mode, while other OBJs may use 256 color mode at the same time. Same for BG0-BG3 layers.

- FRom TONC - `tonc_memdef.h`, the **7th bit (Color mode (16/16 or 256/1)) in REG_BGxCNT has no effect when the BG is AFF**.

>#define BG_4BPP				 0	//!< 4bpp (16 color) bg (no effect on affine bg)
>
>#define BG_8BPP			0x0080	//!< 8bpp (256 color) bg (no effect on affine bg)

- The **data tile is 8x8p@8bpp for bg**, which is **reused for sprite cursor in this context**. **Therefore**, the **sprite** object **must be 8x8p@8bpp** as well

- `grit` can be used as independent program, enter `grit` in terminal to check its existence.

    - Example of using `grit` independently:

        - `cd` into the a folder such as `graphic_process`, in this case

        - prepare the `.png` source file and the `.grit` file contains all the `grit` optional flags in `graphic_process` folder 

        - Inside the folder `graphic_process`: `grit tiles-gbc-v2.png -fts -ff tiles-gbc-v2.grit`

- in this context, tile = 8x8p. GBA screen size = X_Width x Y_Height = 30 x 20 tiles. This could be check by:
    
    1. open a game ROM in NO$GGBA: such as tonc-example `brin_demo` -> open the BG view map, count the tiles.
    
    2. looking at the tonc-example `sbb_reg`:

```
		bg0_pt.x += key_tri_horz(); // unit [pixel]
		bg0_pt.y += key_tri_vert();

		// Testing bg_se_id()
		// If all goes well the cross should be around the center of
		// the screen at all times.
		tx= ((bg0_pt.x>>3)+CROSS_TX) & 0x3F; // unit [tile], because bg0_pt.x>>3 = bg0_pt.x [pixel] / 8 [pixel/ tile] = [tile] 
                                             // Therefore, the  CROSS_TX and CROSS_TY are also in unit [tile].
                                             // The author wants to cross to always be in the middle, so CROSS_TX = screen width in unit tile /2 => screen X_Width = 30 [tile]; similarly the CROSS_TY = screen height in unit tile /2 => screen Y_Height = 20 [tile]
		ty= ((bg0_pt.y>>3)+CROSS_TY) & 0x3F;
```

- BG2 in mode 1 - aff bg, according to GBATEK:

    - for rolling, BG2 has 4 registers: 

```
4000028h - BG2X_L - BG2 Reference Point X-Coordinate, lower 16 bit (W)
400002Ah - BG2X_H - BG2 Reference Point X-Coordinate, upper 12 bit (W)
400002Ch - BG2Y_L - BG2 Reference Point Y-Coordinate, lower 16 bit (W)
400002Eh - BG2Y_H - BG2 Reference Point Y-Coordinate, upper 12 bit (W)

  Bit   Expl.
  0-7   Fractional portion (8 bits)
  8-26  Integer portion    (19 bits)
  27    Sign               (1 bit)
  28-31 Not used

Because values are shifted left by eight, fractional portions may be specified in steps of 1/256 pixels (this would be relevant only if the screen is actually rotated or scaled). Normal signed 32bit values may be written to above registers (the most significant bits will be ignored and the value will be cut-down to 28bits, but this is no actual problem because signed values have set all MSBs to the same value).
```
- with practice in `./source/draw_tile_demo.c`, **in simple case**, we do **not roate/scale the bg**, so the **bit 8-26 integer potion of the `REG_BGX` and `REG_BG_Y` =** value in **pixel unit** movement.

    - **IMPORTANT**: the inital position of the sprite is relative to the screen position, because when the screen is moved, the sprite is also moved the same distance refering to the map/text/static graphic background.

```
BG_AFFINE bgaff;
//-- tonc_types.h
//! Affine parameters for backgrounds; range : 0400:0020 - 0400:003F
typedef struct AFF_DST_EX BG_AFFINE;
typedef struct AFF_DST_EX
{
	s16 pa, pb;
	s16 pc, pd;
	s32 dx, dy;
} ALIGN4 AFF_DST_EX, BgAffineDest;
.....
bgaff= bg_aff_default;
//-- tonc_core.c
const BG_AFFINE bg_aff_default= { 256, 0, 0, 256, 0, 0 };
....
AFF_SRC_EX asx=
{
    32<<8, 64<<8,			// Map coords.
    120, 80,				// Screen coords.
    0x0100, 0x0100, 0		// Scales and angle.
};

// dir + A : move map in screen coords
if(key_is_down(KEY_A))
{
    asx.scr_x += key_tri_horz();
    asx.scr_y += key_tri_vert();
}
else	// dir : move map in map coords
{
    asx.tex_x -= DX*key_tri_horz();
    asx.tex_y -= DX*key_tri_vert();
}

bg_rotscale_ex(&bgaff, &asx);
REG_BG_AFFINE[2]= bgaff;

//-- tonc_types.h
typedef struct AFF_SRC_EX
{
	s32 tex_x;	//!< Texture-space anchor, x coordinate	(.8f)
	s32 tex_y;	//!< Texture-space anchor, y coordinate	(.8f)
	s16 scr_x;	//!< Screen-space anchor, x coordinate	(.0f)
	s16 scr_y;	//!< Screen-space anchor, y coordinate	(.0f)
	s16 sx;		//!< Horizontal zoom	(8.8f)
	s16 sy;		//!< Vertical zoom		(8.8f)
	u16 alpha;	//!< Counter-clockwise angle ( range [0, 0xFFFF] )
} ALIGN4 AFF_SRC_EX, BgAffineSource;

//-- tonc_memmap.h
//! \name Affine background parameters. (write only!)
#define REG_BG_AFFINE		((BG_AFFINE*)(REG_BASE+0x0000))	//!< Bg affine array

//-- tonc_video.h
void bg_rotscale_ex(BG_AFFINE *bgaff, const AFF_SRC_EX *asx);

//-- tonc_legacy.h 
#define bga_rs_ex				bg_rotscale_ex

//-- tonc_bg_affine.c
void bg_rotscale_ex(BG_AFFINE *bgaff, const AFF_SRC_EX *asx)
{
	int sx= asx->sx, sy= asx->sy;
	int sina= lu_sin(asx->alpha), cosa= lu_cos(asx->alpha);

	FIXED pa, pb, pc, pd;
	pa=  sx*cosa>>12;	pb=-sx*sina>>12;	// .8f
	pc=  sy*sina>>12;	pd= sy*cosa>>12;	// .8f
	
	bgaff->pa= pa;	bgaff->pb= pb;
	bgaff->pc= pc;	bgaff->pd= pd;

	bgaff->dx= asx->tex_x - (pa*asx->scr_x + pb*asx->scr_y);
	bgaff->dy= asx->tex_y - (pc*asx->scr_x + pd*asx->scr_y);
}


        
```