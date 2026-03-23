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


