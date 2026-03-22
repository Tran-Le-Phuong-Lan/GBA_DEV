- Mode 1: bg 0 - reg, bg 1 - reg, bg 2 - aff, bg 3 - not used

- bg2 -aff = carcassonne game background, linear map layout (using array row*w + col for addressing SE), 256 color pallete only

    - **SE of aff bg is only 8 bit** ! instead of 16-bit SE in case of reg bg

    - each **tile in aff bg** is **8x8p@8bpp = DTILE** instead of STILE (8x8p@4bpp)

    - **SE is in VRAM**, which only allows **16-/32-bit write** => each time, we have to **write 2/4 DTILEs at once**

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


