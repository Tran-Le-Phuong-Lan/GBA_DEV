# Setup Game Boy Advance (GBA) deveopment in Fedora Linux

## DEVKITPRO - DEVKITARM - GBA lib (now also include TONC lib) 

  - follow https://devkitpro.org/wiki/devkitPro_pacman 

    - section **Customising Existing Pacman Install** (because Fedora Linux is considered as Arch, glibc)

    - section **Fedora**

    - Note: while executing recommended `sudo (dkp-)pacman -Sy`, only run `sudo pacman -Sy`

    - After installing devitpro pacman successfully, at the last section **Predefined Groups**, install the tool for GBA (game boy advance) specific development

      - run `sudo pacman -S gba-dev`

## Install GBA emulator

- Method 1 (prefered): install `mGBA` suggested by Fedora system when searching for the application in the start menu.

- Method 2: See the trouble shooting 1 below

## Follow GBA tutorials

- fundamental tutorials (GBA system, C code): 

  - TONC tutorials [Original](https://www.coranac.com/tonc/text/toc.htm) / [Original code](https://www.coranac.com/tonc/text/intro.htm) - [newer verison](https://gbadev.net/tonc/bitmaps.html) / [new code](https://github.com/gbadev-org/libtonc-examples)

    - Newer text version: same as the original, with more information about setting up environments in many OS systems and IDE. The newer version fixes links of the text.

    - Original code converys better the fundamental knowledge of GBA development process (GBA hardware, Makefile, etc); but the two basic examples **tonc-code/basic/bm_modes** and **tonc-code/basic/brin_modes** are erronous. These are fixed in newer code because in the newer code, they use the advance Makefile to use the tonc lib from the beginning.

  - Another source for learning the basic of GBA developments: [new PERN](https://web.archive.org/web/20030329131157fw_/http://www.thepernproject.com/English/tutorial_2.html)

- C++ GBA libraries: [Butano](https://github.com/GValiente/butano)

  - to use Butano follow: https://gvaliente.github.io/butano/getting_started.html

    - in the link, skip the steps for setting up devkitpro-debkitarm-gba-dev if they are already installed.


# Trouble shooting

1. how to install `.AppImg` file (for mGBA): https://askubuntu.com/questions/774490/what-is-an-appimage-how-do-i-install-it

  - step 1: `chmod a+x file.AppImg`

  - step 2: `./file.AppImage` 
