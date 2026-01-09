//
// pageflip.c
// Shows mode4 page flipping
//
// (20031003 - 20060922, Cearn)

#include <string.h>

#include "toolbox.h"
#include "page_pic.h"

void load_gfx()
{
	const size_t bitmap_size = page_picBitmapLen / (2 * sizeof(unsigned int));
	const unsigned int *frontBitmap = page_picBitmap;
	const unsigned int *backBitmap = frontBitmap + bitmap_size;
	// const unsigned short *frontPal = page_picPal;

	// -------- Q1:
	// Why 120 width (e.g &vid_mem_front[ii*120]) of the VRAM in mode 4, even though in mode 4, the size of the front/back VRAM:  
	// 240 pixel (width) x 160 pixel (height) x 8bpp (bits per pixel)?
	// Explain in the tutorial: TONC tutorial (https://gbadev.net/tonc/bitmaps.html // https://www.coranac.com/tonc/text/toc.htm),
	// VRAM only allows 16-bit (halfword) or 32-bit (word) write. That means, in mode 4, 1 write covers 2 pixels 
	// (better explaination is here: https://web.archive.org/web/20030329131157fw_/http://www.thepernproject.com/English/tutorial_2.html).
	// Therefore, instead of 240 pixel width, from the writing to VRAM perspective, there are in effect only 120 addresses of the width to address,
	// -------- Q2
	// Why &frontBitmap[ii*144/4] or why the address of the image data is divided by 4?
	// This can only be checked after run `make` of this project: so that the command `grit`
	// with all the linked setup in `Makefile` will produce the image data from the image file `.png` in folder `./graphics`.
	// The image data to be used in this code is found in `./build/page_pic.s`, `./build/page_pic.h`.
	// Check the `.png` info: this image is 144 pixels x 32 pixels x 4 bpp (by running `file <image.png>`).
	// Based on `./build/page_pic.s`, the `grit` turns the image into  144 pixels x 32 pixels x 8 bpp, 
	// and data is organized as 32-bit value at each array element (see page_picBitmap array).
	// That is why when look at the description (e.g declaration) of page_picBitmap in `./build/page_pic.h`,
	// the total number of elements in page_picBitmap array is 1152 = 144 pixel x 32 pixel x 8 bpp / 32 bit per elemen. 
	// NOTE: bit per element = bit per address within an array.
	// In conlusion, the image data is organised as array of 1152 elements, each elements is 32-bit (= 4-byte) long.
	// In the perspective of moving the image data into the VRAM: 
		// for example: 
		// each scanline in VRAM mode 4 is an array of (240 pixels x 8 bpp / 16 bit per address) 120 addresses, 
		// each address holds 16 bits (= 2 bytes, in the perspective of writing to VRAM rule); 
		// and each scanline data of the image data is an array of 144 / 4 (= 144 pixels * 8 bit per pixel / 32 bit per address) addresses, 
		// each address holds 32 bits (=4 bytes), in total, an image data scanline = (144 / 4 * 32) / 8 = 144 bytes; 
		// the image data has 16 scanlines for each of its parts (the each image for the front buffer VRAM mode 4, the other for the back buffer VRAM mode 4).
		// So to move each image data scanline into the corresponding scanline in VRAM:
		// memcpy(&vid_mem_front[ii*120], &frontBitmap[ii*144/4], 144); 
	int ii;
	for(ii=0; ii<16; ii++)
	{
		// memcpy(destination address, source address, number of bytes to be transfered)
		memcpy(&vid_mem_front[ii*120], &frontBitmap[ii*144/4], 144); 
		memcpy(&vid_mem_back[ii*120], &backBitmap[ii*144/4], 144);
	}

	// You don't have to do everything with memcpy.
	// In fact, for small blocks it might be better if you didn't.
	// Just mind your types, though. No sense in copying from a 32bit 
	// array to a 16bit one.
	// u32 *dst= (u32*)pal_bg_mem;
	// for(ii=0; ii<8; ii++)
	// 	dst[ii]= frontPal[ii];
	memcpy(&pal_bg_mem[0], page_picPal, page_picPalLen/2);
}

int main()
{
	load_gfx();
	REG_DISPCNT= DCNT_MODE4 | DCNT_BG2;

	int ii=0;

	while(1)
	{		
		while(KEY_DOWN_NOW(KEY_START)) ;	// pause with start

		vid_vsync();
		if(++ii == 60)
		{	
			ii=0;	
			vid_flip();	
		}
	}
	
	return 0;
}
