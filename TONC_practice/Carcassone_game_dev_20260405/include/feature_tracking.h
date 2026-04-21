// Feature tracking for Carcassonne game
// recognise <feature>, track <feature> throughout the game
// <feature>: church, city, street, field.

#ifndef CARCAS_FEATURE_TRACKING_H
#define CARCAS_FEATURE_TRACKING_H

#include <stdlib.h>
#include <tonc.h>

typedef enum GAME_FEATURES {
	CITY,
	STREET,
	FIELD,
    END_FEATURE,
    NA_FEATURE,
    DELETE_FEATURE
} GAME_FEATURES;

typedef enum DIRECTION {
	TOP,
	LEFT,
	RIGHT,
    BOT,
    NA_DIR
} DIRECTION;

typedef struct GAME_FEATURE_NODE
{
	u32 car_tid;
    GAME_FEATURES game_feature;             // only consider car_tid, if the game_feature != END,
                                            // END = ending of feature.
    s32 tx, ty;                             // coordinate of the node = screen-entry idx
	s32 top_coord_x, top_coord_y,           // y-1 
        right_coord_x, right_coord_y,       // x+1
        bot_coord_x, bot_coord_y,           // y+1
        left_coord_x, left_coord_y;         // x-1

    struct GAME_FEATURE_NODE *child_top_lk, *child_r_lk, *child_bot_lk, *child_l_lk;
    struct GAME_FEATURE_NODE *parent_top_lk, *parent_r_lk, *parent_bot_lk, *parent_l_lk; 

} ALIGN4 GAME_FEATURE_NODE, *GAME_FEATURE_NODE_ptr;

typedef struct GAME_FEATURE_NODE_START
{
    GAME_FEATURE_NODE_ptr root;

} ALIGN4 GAME_FEATURE_NODE_START;

GAME_FEATURE_NODE_ptr create_node (s32 tx_coord, s32 ty_coord, u32 tid, GAME_FEATURES tile_feature, DIRECTION parent_direction);
GAME_FEATURE_NODE_ptr find_node (GAME_FEATURE_NODE_ptr feature_root, GAME_FEATURE_NODE_ptr new_node, DIRECTION* child_direction);
GAME_FEATURE_NODE_ptr insert_node (GAME_FEATURE_NODE_ptr feature_root, GAME_FEATURE_NODE_ptr new_node);
// relink for the newly added node, if any exisiting node in the feature structure is its child
void finish_features_linking (GAME_FEATURE_NODE_ptr new_node, GAME_FEATURE_NODE_ptr feature_root);
GAME_FEATURE_NODE_ptr merging_features (GAME_FEATURE_NODE_ptr feature_root_1, GAME_FEATURE_NODE_ptr feature_root_2);
bool feature_complete_check (GAME_FEATURE_NODE_ptr feature_root);
GAME_FEATURE_NODE_ptr delete_whole_feature (GAME_FEATURE_NODE_ptr feature_root, unsigned char* debug_del, unsigned char* found_order);
void delete_node (GAME_FEATURE_NODE_ptr node);

#endif