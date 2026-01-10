# Objectives

- 1. learning how to use GIMP for editing iamge files (png) into desired pixel size and making different iamge files sharing the same color palette.

- 2. importing images of choice into the example.

- 3. what is the effect of `./graphics/<image file>.grit`?

# Failure attempts

- 1. **In GIMPS**: two images 240 pixel x 160 pixels (each image data fits the whole either front/back buffer in mode 4 VRAM), are forced to have the same indexing colors, results are two file `./graphics/Front_pic.png`and `./graphics/Back_pic.png` (with the wrong assumption that the palette data of both will be the same, why wrong? see the result of this attempt below). **In C code**, `frontBitmap` points to the data of one image linked to, for example `Front_pic.h`; while `backBitmap` points to the data of image linked to `Back_pic.h`. The palette data are moved into the PALMEM as follow: `memcpy(&pal_bg_mem[0], Front_picPal, Front_picPalLen/2);` 

  - **result**: In mGBA, the program can flip between the two images, the color of the `Front_pic` is good, but the color of the `Back_pic` is destroyed (i.e not even close to the original color).

- 2. **In GIMPS**: two images 240 pixel x 160 pixels (each image data fits the whole either front/back buffer in mode 4 VRAM), are forced to have the same indexing colors, results are two file `./graphics/Front_pic.png`and `./graphics/Back_pic.png`. **In C code**, `frontBitmap` points to the data of one image linked to, for example `Front_pic.h`; while `backBitmap` points to the data of image linked to `Back_pic.h`. The palette data are moved into the PALMEM as follow: `memcpy(&pal_bg_mem[0], Front_picPal, Front_picPalLen/2); memcpy(&pal_bg_mem[512], Back_picPal, Back_picPalLen/2);` 

  - **result**: In mGBA, the program can flip between the two images, but the color of them are destroyed.

- 3. **In GIMPS**: two images 240 pixel x 160 pixel (each image data fits the whole either front/back buffer in mode 4 VRAM) are put in the same canvas of **size 240 pixel x 320 pixel** (this definite makes sure that both sharing the same palette data), result is the `./graphics/All_pic.png`. **Then**, it **must be flattened** (i.e no more layers), **then** covert to **indexing color** (just like in the previouse two failure attempts (1., 2.) above) **with the maximum color of 255 or even 200**. **In C code**, `frontBitmap` points to half amount of the data of combined image (as explained in tutorials (TONC, PERN), because the iamge data is arranged in array [y_axis*width + x_axis] (x_axis: width axis, y_axis: height axis)), `backBitmap` points to the other half of the data. This is exactly the same as in the new TONC code example (libtonc-examples). The palette data are moved into the PALMEM as follow: `memcpy(&pal_bg_mem[0], All_picPal, All_picPalLen/2);`. Because only the the `All_pic.png` is used, it is best practice to comment out what is not used `#include "Back_pic.h"` and `#include "Front_pic.h"`, these link to the other two images `Front_pic.png` and `Back_pig.png`. 

  - **result**: In mGBA, the program can flip between the two images, but the color of them are destroyed.

# Successful attempts

- 1. create the folder `./graphic_trials`, it contains example image file `page_pic.png`, then in terminal `cd` inside the folder `./graphic_trials`, run `grit page_pic.png -gb -gB8 -m -mLf`. 

  - the `-gb -gB8 -m -mLf` is the content in `page_pic.grit` in the newer TONC example (libtonc-examples/ basic/ pageflip/ graphics)

  - the result: are the  two files `page_pic.s` and `page_pic.h`.
  `page_pic.s`: contains the image data and the color palette data.
  `page_pic.h`: discription/ declaration of the data stored in `page_pic.s`.

- 2. **In GIMPS**: two images 240 pixel x 160 pixel (each image data fits the whole either front/back buffer in mode 4 VRAM) are put in the same canvas of **size 240 pixel x 320 pixel** (this definite makes sure that both sharing the same palette data), result is the `./graphics/All_pic.png`. **Then**, it **must be flattened** (i.e no more layers), **then** covert to **indexing color** (just like in the previouse two failure attempts (1., 2.) above) **with the maximum color of 128**. **In C code**, `frontBitmap` points to half amount of the data of combined image (as explained in tutorials (TONC, PERN), because the iamge data is arranged in array [y_axis*width + x_axis] (x_axis: width axis, y_axis: height axis)), `backBitmap` points to the other half of the data. This is exactly the same as in the new TONC code example (libtonc-examples). The palette data are moved into the PALMEM as follow: `memcpy(&pal_bg_mem[0], All_picPal, All_picPalLen/2);`. Because only the the `All_pic.png` is used, it is best practice to comment out what is not used `#include "Back_pic.h"` and `#include "Front_pic.h"`, these link to the other two images `Front_pic.png` and `Back_pig.png`.

  - result: program can flip between two images and image color of both is preserved well.