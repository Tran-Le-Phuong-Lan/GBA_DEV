#include <tonc.h>

void load_game(u8* loaded_arr, u32 loaded_arr_sz)
{
    u32 iter;
    for (iter=0; iter< loaded_arr_sz; iter++)
    {
        loaded_arr[iter]= sram_mem[iter];
    }
    return;
}