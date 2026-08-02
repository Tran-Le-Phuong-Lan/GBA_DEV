- To save game (i.e write) to SRAM

    - Reference: [GBATEK](https://problemkaputt.de/gbatek.htm#gbacartridges)

    - step 1: edit bit 0-1 @ 4000204h - WAITCNT - Waitstate Control (R/W) to be 3 for 8 wait.

        - There is already defined WAITCNT register in tonc library `TONC_tutorial/TONC_tutorial/tonc-code/code/tonclib/include/tonc_memmap.h` as `REG_WAITCNT`.

    - step 2: write data to SRAM @ E000000h-E007FFFh. 
        
        - The base address of SRAM is defined in tonc library `libtonc/include/tonc_memmap.h` as `MEM_SRAM`. The data must be written in 8-bit size.

        - there is alrady a defined 8bit pointer to this SRAM memory region in tonc library `TONC_tutorial/TONC_tutorial/tonc-code/code/tonclib/include/tonc_memmap.h`. The defined pointer is `sram_mem`.
    
- To load the saved data from sram

    - **M1 (NOT ADVISED, why?-> see the question below)**: simply read out the 8-bit data size stored in sram using code executed as 16-bit thumb code from ROM (see funcion `load_game` in [this file](./source/gba_save_game_exp.c)).  

        - **Question**: why in [GBATEK](https://problemkaputt.de/gbatek.htm#gbacartridges) stated "Reading from SRAM/FRAM should be performed by code executed in WRAM only (but not by code executed in ROM).", but in [my code](./source/gba_save_game_exp.c), The code to load the data is compiled as thumb code, and saved in ROM area, so it is executed from ROM. That is contrary to what is said in GBATEK.

            - CLAUDE AI reply (?? no fact-checking of how much this reply is correct yet)

            ```
            Good catch — this is a real gap between what GBATEK says and what actually happens in practice, and it's worth understanding why.

            What the restriction is actually about

            The GBA cartridge slot's SRAM/FRAM chip shares physical bus lines with the ROM chip (same address/data bus, different chip-select lines). GBATEK notes that reading from SRAM/FRAM should be performed by code executed in WRAM only, not by code executed in ROM, while there is no such restriction for writing. The mechanism behind this, as documented by hardware researchers, is the ROM prefetch/cache circuitry sitting between the cart bus and the CPU: this is a bit of hardware designed to speed up executing code from the Game Pak ROM, and in practice this usually ends up with ROM thumb instructions sitting in the prefetch buffer because the next CPU read is usually the next instruction in ROM. If your code executes from ROM, the CPU is constantly issuing bus cycles to fetch upcoming instructions from the same physical bus that SRAM sits on. When you then issue an LDRB to SRAM in the middle of that stream, there's a real possibility of the prefetch logic and the SRAM access colliding on that shared bus in a way that can corrupt the byte read back — this is exactly the class of hazard Nintendo's own docs (which GBATEK derives its wording from) are warning about. 
            
            Why your code still "works"

            A few practical reasons this restriction doesn't bite you here:

            1. Prefetch buffer is off by default. The GBA prefetch buffer for ROM has to be explicitly enabled via bit 6 of WAITCNT. Your code only sets REG_WAITCNT = WAITCNT_SRAM; (i.e. 0x0003), which sets the SRAM wait-state bits but leaves the prefetch enable bit clear. With prefetch off, the specific bus-collision scenario the warning is about is far less likely to occur, since the ROM isn't opportunistically grabbing bus cycles in the background.
            2. Simple, isolated byte loads. sram_mem[0] / sram_mem[1] compile down to single LDRB instructions with nothing time-critical happening immediately around them. The hazard is more of a timing/electrical risk than a strict logical impossibility — it doesn't corrupt data on every access, every cart, every timing pattern. It's a "this can glitch under certain conditions" caution, not "this always fails."
            Emulators vs. real hardware. If you tested this in an emulator (mGBA, no$gba, etc.), most emulators don't model the cart-bus contention at the level of fidelity needed to reproduce this hazard at all — reads just work. Real hardware, and especially specific flashcart controllers, is where this kind of thing has historically bitten people intermittently.
            3. Cart hardware varies. Different SRAM/FRAM chips and cart designs (official Nintendo boards vs. third-party flashcarts vs. emulated SRAM) have different bus behavior and tolerances, so "works on my cart" doesn't mean "guaranteed to work on all carts."
            Practical takeaway

            Your code isn't wrong per se — it's just relying on a hazard not manifesting rather than being provably safe. If you want to be fully correct per the hardware guidance, the standard fix is to mark the load function with IWRAM_CODE (a libtonc/devkitARM macro that places the function in IWRAM and forces the compiler to emit a long-call so it actually executes from there) so that the SRAM read genuinely happens from WRAM, e.g.:

            IWRAM_CODE u8 sram_read_byte(int i) {
                return sram_mem[i];
            }
            ```

    - **M2 (STRONGLY ADVISED)**: Regardless of whatever Claude Ai suggested and explained above, we follow the guidance from GBATEK to implement a sram-load function excuted from WRAM.

        - To know how to specify c-code section belonging to wram, and compiled then linked into the correct address range of wram, see [tonc- interrupt section](https://gbadev.net/tonc/interrupts.html). This solution is implemented in function `load_game_wram`-[load_game_iwram.iwram.c](./source/load_game_iwram.iwram.c). There are two important steps:

            - step 1: for the function which we want to put in the wram, e.g the internal work ram (i.e iwram), we need to use the special GCC macro `IWRAM_CODE` (which is already defined by TONCLIB) in the function declaration (see the function `load_game_wram` in [this code](./source/gba_save_game_exp.c)). This macros will guide the GCC linker to put the code of function `load_game_wram` in the right address range of IWRAM.

            - step 2: must guide the compiler (e.g `arm-none-abi-gcc` in this case) to compile the function definition of `load_game_wram` in [load_game_iwram.iwram.c](./source/load_game_iwram.iwram.c) as arm code (`-marm`) instead of (`-mthumb`) from the Makefile. **Fortunately, this is already taken care of by the devkitpro/devkitarm-gba installed toolset (we do not have to change any thing in the `Makefile` of the project), Why??**. **Answer:** By looking at the current project -**my_practive_save_game**- [Makefile](./Makefile), the method for compiling and linking for all of the source files is specified in the `$(DEVKITARM)/gba_rules`, which then shows the details of the compilation options are specified in the `$(DEVKITPRO)/devkitARM/base_rules`. This section in `base_rules` explains why files ending in `.iwram.c` are compiled with option `-marm` instead of `-mthumb`

            ```
            %.iwram.o: CFLAGS    += -marm -mlong-calls
            %.iwram.o: CXXFLAGS  += -marm -mlong-calls
            %.iwram.o: OBJCFLAGS += -marm -mlong-calls
            ``` 
    - **To prove that the function `load_game` is 16-bit thumb code executed from ROM, and function `load_game_wram` is 32-bit arm code executed from IWRAM**, see the file [`my_practice_save_game_elf_v3.txt`](./my_practice_save_game_elf_v3.txt). The file `my_practice_save_game_elf_v3.txt` is the result of executing `arm-none-eabi-objdump` on the file `my_practice_save_game.elf` (which is the original code before being turned into the hex-dump file `my_practice_save_game.gba` to be run on the GBA system).
