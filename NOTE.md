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

## install WineHQ (Fedora) to run other GBA tools (e.g Mappy Map editor, No$GBA, etc)

- [WineHQ website - Download - Fedora](https://gitlab.winehq.org/wine/wine/-/wikis/Fedora) 
  
  - checking [fedora version in terminal](https://www.fosslinux.com/121978/how-to-check-your-fedora-version-via-gui-and-command-line.htm):  `cat /etc/fedora-release`

    - then only follow the section **Installing WineHQ packages**

    - after the wine installation process is finished, follow the mentioned wine tutorial above, to run mappy program: 

    - run `cd ~/Downloads/Apps/mapwin1423`, then run `wine mapwin.exe`

## Follow GBA tutorials

- fundamental tutorials (GBA system, C code): 

  - TONC tutorials [Original](https://www.coranac.com/tonc/text/toc.htm) / [Original code](https://www.coranac.com/tonc/text/intro.htm) - [newer verison](https://gbadev.net/tonc/bitmaps.html) / [new code](https://github.com/gbadev-org/libtonc-examples)

    - Newer text version: same as the original, with more information about setting up environments in many OS systems and IDE. The newer version fixes links of the text.

    - Original code converys better the fundamental knowledge of GBA development process (GBA hardware, Makefile, etc); but the two basic examples **tonc-code/basic/bm_modes** and **tonc-code/basic/brin_modes** are erronous. These are fixed in newer code because in the newer code, they use the advance Makefile to use the tonc lib from the beginning.

  - Another source for learning the basic of GBA developments: [new PERN](https://web.archive.org/web/20030329131157fw_/http://www.thepernproject.com/English/tutorial_2.html)

- C++ GBA libraries: [Butano](https://github.com/GValiente/butano)

  - to use Butano follow: https://gvaliente.github.io/butano/getting_started.html

    - in the link, skip the steps for setting up devkitpro-debkitarm-gba-dev if they are already installed.

- nice materials for learning curve of GBA game development: https://forum.gbadev.net/topic/21-getting-started-with-gba-development

- C learning: https://beej.us/guide/bgc/html/split/index.html

- Jasper Vijn: https://www.coranac.com/projects/

- **INLINE function** explantion: https://stackoverflow.com/questions/31108159/what-is-the-use-of-the-inline-keyword-in-c

- setup debug C code GBA in Emacs (Linux) with mGBA: https://themkat.net/2022/05/09/debugging_gba_in_emacs.html

- **No$GBA Debugger version** for nice debugging of GBA game with visual (moving screen on background, etc): https://www.nogba.com/

# Trouble shooting

1. how to install `.AppImg` file (for mGBA): https://askubuntu.com/questions/774490/what-is-an-appimage-how-do-i-install-it

  - step 1: `chmod a+x file.AppImg`

  - step 2: `./file.AppImage` 
