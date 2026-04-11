// Feature tracking for Carcassonne game
// recognise <feature>, track <feature> throughout the game
// <feature>: church, city, street, field.

#ifndef CARCAS_FEATURE_TRACKING_H
#define CARCAS_FEATURE_TRACKING_H

#include <stdlib.h>
#include <tonc.h>

enum GAME_FEATURES {
	CITY,
	STREET,
	FIELD
};


typedef struct GAME_FEATURE_NODE
{
	u32 car_tid;
    // GAME_FEATURES feature_name;
	s32 top_coord_x, top_coord_y,        // y-1 
        right_coord_x, right_coord_y,    // x+1
        bot_coord_x, bot_coord_y,        // y+1
        left_coord_x, left_coord_y;    // x-1
    struct GAME_FEATURE_NODE *top_lk, *r_lk, *bot_lk, *l_lk; 

} ALIGN4 GAME_FEATURE_NODE, *GAME_FEATURE_NODE_ptr;


GAME_FEATURE_NODE_ptr create_node (s32 tx_coord, s32 ty_coord, u32 tid);
void delete_node (GAME_FEATURE_NODE_ptr node);

#endif