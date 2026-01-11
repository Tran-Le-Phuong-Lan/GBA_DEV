# NOTE 

## Failure attempts

 - 0. source image: 64 pixel x 64 pixel, In the GIMP, **convert to 127 index color**. Then, use `make` process with the additional options set in the `./graphics/image.grit` turns the image into data 64x64@4 (= 64 pixel x 64 pixel, 4bpp). The c source code are kept intact, except changing the headers to the correct image.
   
   - **result**: color error in GBA program.

## Successful attemps

 - 0. source image: 64 pixel x 64 pixel, In the GIMP, **convert to 16 index color**. Then, use `make` process with the additional options set in the `./graphics/image.grit` turns the image into data 64x64@4 (= 64 pixel x 64 pixel, 4bpp). The c source code are kept intact, except changing the headers to the correct image.

   - **result**: color appears to be good in GBA program.

## Questions

- because the data image is 64x64@**4bpp**, in the tutorial mentions about the `ATTR2_PALBANK` (the attribute 2, palette bank) should be set accordingly. But I **change the image to another image which uses different palette color**, and I kept the **souce code unchange for the `ATTR2_PALBANK`**, **could that be the reason why the color seems to be a bit not correct?**

  - Q1: how can we have the color palette data from the image or in GIMP, in order to update the `ATTR2_PALBANK` accordingly? 