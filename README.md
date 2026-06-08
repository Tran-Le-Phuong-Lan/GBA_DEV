# Setup environment on Linux (Fedora) to develope Game Boy Advance

- see the **NOTE.md**

# My GBA learning process and document

- is in the branch `learning_practice` of this repository.

- This learning process based on

    - TONC tutorials [Original](https://www.coranac.com/tonc/text/toc.htm) / [Original code](https://www.coranac.com/tonc/text/intro.htm) - [newer verison](https://gbadev.net/tonc/bitmaps.html) / [new code](https://github.com/gbadev-org/libtonc-examples)

    - Some of my own modifications and comments are added to the folders in branch `learning_practice`; in order to improve my understanding of the TONC material and code. 

# DEVKITPRO-DEVKITARM for gba toolchain info

- **Linker map file (i.e file with extension `.map`)**: for each gba project in newer version of libtonc `libtonc-examples`, only after `make` the project, **the linker map file** in the `build` folder of each project. This is known by looking up any `Makefile` , there is a linker flag definition `LDFLAGS` which has `-Map,$(notdir $*.map)`.

- **Dissemble ection information, etc of a gba game rom**: for each gba project in newer version of libtonc `libtonc-examples`, only after `make` the project, besides the file `.gba`, there is another file of the same name with the `.gba` but having extension `.elf`, by using the installed `.../devkitpro/devkitarm/bin/arm-none-eabi-objdump` on the file `.elf` with option `-D` or `-t` etc..

- **The linker script used by devkitarm for generating**: the linker script is usually explicitly passed to the linker/gcc with option `-T <linker_script_name.ld>`, but in all relevant file for `make` the project = the `Makefile` of the project + the `gba_rules` + `base_rules` + `base_tool`: NO WHERE the linker script is explicitly mentioned (even though both `LD` and `LDFLAGS` are defined in the `Makefile`). SO, **WHERE IS THE LINKER SCRIPT PASSED IN?** (the linker script must existed, because if we look at the dissamble code of any gba rom file through its `.elf`, as mentioned above, the address of iwram, ewram, rom are correct)

    - **SOL**
        
        ```
        (by CLAUDE AI)
        When no -T linker script is explicitly passed, arm-none-eabi-gcc finds its linker script through the -specs= file (like gba.specs in your case).
        The specs file itself contains a *link: or *self_spec: section that injects a -T somescript.ld automatically. For example, gba.specs typically contains something like:
        ```

        - therefore, looking at `gba_rules` at `.../devkitpro/devkitARM/gba_rules`, we see that the `.specs` file is mentioned in the section of target `%.elf:`: `-specs=gba.specs`. This specfile is located at `.../devkitpro/devkitARM/arm-none-eabi/lib/gba.specs`

            ```
            (content of gba.specs)

            %include <sync-none.specs>

            *link:
            + -T gba_cart.ld%s --gc-sections --no-warn-rwx-segments

            *startfile:
            gba_crt0%O%s crti%O%s crtbegin%O%s

            ```

            - That means the linker/compiler `.../devkitpro/devkitarm/bin/arm-none-eabi-gcc` uses the linker script `gba_cart.ld` specfied in the `gba.specs` file

            - The linker script `gba_cart.ld` is found at `.../devkitpro/devkitARM/arm-none-eabi/lib/gba_cart.ld`