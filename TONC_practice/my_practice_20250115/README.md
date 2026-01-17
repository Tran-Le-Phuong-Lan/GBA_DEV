# NOTE 

## Emphasized information 

- **VRAM for storing object (i.e sprite) graphics** starts at **Character block 4 (CB 4) (0601:0000h)** and **CB 5 (0601:4000h)**. OVRAM (object VRAM) = CB 4 + CB 5

- TILE (8x8p x 4bpp) = 32 byte = 20h

- TILE8 (8x8p x 8bpp) = 64 byte = 40h

- Index of stored TILE and TILE 8 in OVRAM

|VRAM 0601:|0000h|0020h|0040h|0060h|0080h|...|
|---|---|---|---|---|---|---|
|4bpp|0|1|2|3|4|...|
|8bpp|0| |2| |4|...|

- **Sprite palette** @ 0500:0200h

- **sprite map mode (how it is stored in OVRAM)**: 

  - by default 2D

  - **preferred 1D** (by the author, and even author of PERN. Because it is easy): **setting `REG_DISPCNT{6}`** = set 1 to the 6th bit of Display Control Register. 

- **OAM (Object Attribute memory)** to store object attribute, based on which the hardware know how to draw the object on the screen

  - **OAM start @ 0700:0000h, 1024 bytes long**

```
typedef struct tagOBJ_ATTR
{
    u16 attr0;
    u16 attr1;
    u16 attr2;
    s16 fill;
} ALIGN4 OBJ_ATTR;

typedef struct OBJ_AFFINE
{
    u16 fill0[3];
    s16 pa;
    u16 fill1[3];
    s16 pb;
    u16 fill2[3];
    s16 pc;
    u16 fill3[3];
    s16 pd;
} ALIGN4 OBJ_AFFINE;
```

|OAM (u16)|0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|OBJ_ATTR  | 0|1|2|  |4|5|6|  |7|8|9|  |10|11|12|  |...|
|OBJ_AFFINE|  | | |pa| | | |pb| | | |pc|  |  |  |pd|...|

Therefore, one pointer to OAM, we can access both OBJ_ATTR and OBJ_AFFINE.

OAM 1024 byte long = 128 OBJ_ATTR = 32 OBJ_AFFINE

  - **Attribute 0**:

    - **OBJ_ATTR.attr0** (**MSB(F)->LSB(0)**): **Shape (F,E)** **(combined with Size of Attribute 1)** | **Color/palette mode (D)**: 0=16/16, 1=256/1| **Mosaic (C)** | **GFx mode (B, A)**: 00=normal rendering, 01= ... | **Object Mode (9,8)**: 00: normal rendering, 01: affine, ... | **Y-Coord (7-0)**: marks top of the sprite.

  - **Attribute 1**:

    - **OBJ_ATTR.attr1** (**MSB(F) ->LSB(0)**): **Size (F,E)** | **HF/VF (D, C)**: used if attr0.{9,8}=00; otherwise, these bits are part of affine index| **Affine Index (D, C, B, A, 9)**: used only if attr0.{8}=1 | **X-Coord (8-0)**: marks left of the sprite.
  
    - Combination of **Shape Attribute 0** and **Size Attribute 1** determines the sprite size (**in unit of pixel. Format: X x Y, for example 8x8 = 8 pixel wide (x-axis) x 8 pixel tall (y-axis)**)


|**shape (MSB(15), LSB(14))** \ **size (MSB(F), LSB(E))**|00|01|10|11|
|---|---|---|---|---|
|00 (SQUARE)           |8x8 |16x16|32x32|64x64|
|01 (WIDE = HORIZONTAL)|16x8|32x8 |32x16|64x32|
|10 (TALL = VERTICAL)  |8x16|8x32 |16x32|32x64|

  - **Attribute 2**:

    - **OBJ_ATTR.attr2** (**MSB(F) ->LSB(0)**): **Palette Bank (F,E,D,C)**: only valid in 16-color mode, otherwise, no effect | **Priority (B, A)**: **higher priority** drawn first (**=back**), therefore they can be **covered by lower priority (= in the front)** | **Base Tile Index (9-0)**: Note in bitmap mode, the base tile index must be 512 or higher.

- **OAM double buffering**

  - Can not access OAM during VDRAW

  - Solution (usually): a buffer of OAM (known as object shadow) can be modified at anytime, then copy that to OAM during VBLANK

- **MACROs for bit manipulation (in OBJ_ATTR, or REG_... or etc)**: To understand better the code, refer to [Beej's Guide to C Programming](https://beej.us/guide/bgc/html/split/the-c-preprocessor.html#the-c-preprocessor) - Chapter 19 The C preprocessor -  19.5.4 Stringification & 19.5.5 Concatenation

  - the `##` : concatenation

```
// bit field set and get routines
#define BF_PREP(x, name)         ( ((x)<<name##_SHIFT)& name##_MASK  )
#define BF_GET(x, name)          ( ((x) & name##_MASK)>> name##_SHIFT )
#define BF_SET(y, x, name)       (y = ((y)&~name##_MASK) | BF_PREP(x,name) )

#define BF_PREP2(x, name)        ( (x) & name##_MASK )
#define BF_GET2(y, name)         ( (y) & name##_MASK )
#define BF_SET2(y, x, name)      (y = ((y)&~name##_MASK) | BF_PREP2(x, name) )

// Create bitfield:
attr2 |= BF_PREP(id, ATTR2_SHAPE);
// becomes:
attr2 |= (id<<ATTR2_ID_SHIFT) & ATTR2_ID_MASK;

// Retrieve bitfield:
id= BF_GET(attr2, ATTR2_ID);
// becomes:
id= (attr2 & ATTR2_ID_MASK)>>ATTR2_ID_SHIFT;

// Insert bitfield:
BF_SET(attr2, id, ATTR2_ID);
// becomes:
attr2= (attr&~ATTR2_ID_MASK) | ((id<<ATTR2_ID_SHIFT) & ATTR2_ID_MASK);


```

- To understand the **keypad manipulation (`./include/input.h`): see TONC tutorial - chapter 6. The GBA keypad**.

## Successful attemps


## Questions

- because the data image is 64x64@**4bpp**, in the tutorial mentions about the `ATTR2_PALBANK` (the attribute 2, palette bank) should be set accordingly. But I **change the image to another image which uses different palette color**, and I kept the **souce code unchange for the `ATTR2_PALBANK`**, **could that be the reason why the color seems to be a bit not correct?**

  - Q1: how can we have the color palette data from the image or in GIMP, in order to update the `ATTR2_PALBANK` accordingly? 