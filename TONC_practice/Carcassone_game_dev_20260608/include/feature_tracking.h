// Feature tracking for Carcassonne game
// recognise <feature>, track <feature> throughout the game
// <feature>: church, city, street, field.
#ifndef CARCAS_FEATURE_TRACKING_H
#define CARCAS_FEATURE_TRACKING_H

#include <stdlib.h>
#include <tonc.h>
#include "carcas_infos.h"

typedef enum {
    MIN,
    MAX
} MIN_OR_MAX;

typedef enum {
    X,
    Y
} X_OR_Y;

typedef enum GAME_FEATURES {
	CITY,
	STREET,
	FIELD,
    CHURCH,
    GARDEN,
    MEEPLE_TILE,
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

    // RULE: child and parent lk to the same direction must not be (NOT NULL) at the same time,
    // for example, if child_top_lk != NULL, parent_top_lk must == NULL or vice versa
    // or both are NULL; but the RULE is violated, if at the same time,
    // child_top_lk !=NULL and parent_top_lk !=NULL.
    struct GAME_FEATURE_NODE *child_top_lk, *child_r_lk, *child_bot_lk, *child_l_lk;
    struct GAME_FEATURE_NODE *parent_top_lk, *parent_r_lk, *parent_bot_lk, *parent_l_lk; 

} ALIGN4 GAME_FEATURE_NODE, *GAME_FEATURE_NODE_ptr;

typedef struct GAME_FEATURE_NODE_START
{
    GAME_FEATURE_NODE_ptr root;

} ALIGN4 GAME_FEATURE_NODE_START;

// compute the adjacent coordinates 
// top: y-1 
// right: x+1
// bot: y+1
// left: x-1
COORD_2D adj_top_coord(GAME_FEATURE_NODE_ptr curr_node_ptr);
COORD_2D adj_right_coord(GAME_FEATURE_NODE_ptr curr_node_ptr);
COORD_2D adj_bot_coord(GAME_FEATURE_NODE_ptr curr_node_ptr);
COORD_2D adj_left_coord(GAME_FEATURE_NODE_ptr curr_node_ptr);

GAME_FEATURE_NODE_ptr create_node (s32 tx_coord, s32 ty_coord, u32 tid, GAME_FEATURES tile_feature, DIRECTION parent_direction);
    // return NULL, if no new node created; otherwise, 
    // return pointer to the newly created node.

bool node_exist (GAME_FEATURE_NODE_ptr feature_root, GAME_FEATURE_NODE_ptr new_node);
    // return true, if node already exists in the feature; otherwise,
    // return false

GAME_FEATURE_NODE_ptr find_node (GAME_FEATURE_NODE_ptr feature_root, GAME_FEATURE_NODE_ptr new_node, DIRECTION* child_direction);
    // return NULL, if can not find where in the feature to add the new node; otherwise,
    // return pointer to where the new node can be inserted, and which direction to insert it in `*child_direction`.

GAME_FEATURE_NODE_ptr insert_node (GAME_FEATURE_NODE_ptr feature_root, GAME_FEATURE_NODE_ptr new_node);
    // return NULL, if can not insert the new node; otherwise
    // return pointer to the feature_root which the new node is inserted.

    // relink for the newly added node, if any exisiting node in the feature structure is its parent
void finish_features_linking (GAME_FEATURE_NODE_ptr new_node, GAME_FEATURE_NODE_ptr feature_root);
GAME_FEATURE_NODE_ptr merging_features (GAME_FEATURE_NODE_ptr feature_root_ref, GAME_FEATURE_NODE_ptr feature_root_2);
    // return NULL, if no merge is done.
    // return pointer to the merged feature root, if merge is done     

    // must initialize cnt=0, before passing inside the function. Otherwise, function returns logically wrong result.
bool feature_min_max_coord (GAME_FEATURE_NODE_ptr feature_root, MIN_OR_MAX comp_info, COORD_2D* result, s32* cnt);
    // the min, max finding must always have result returned in `*result`, as long as the `feature_root` is not NULL.

void feature_report_per_cartilemap (GAME_FEATURE_NODE_ptr feature_root, u16* report_flag, 
                                 COORD_2D feature_min_coord, COORD_2D feature_max_coord);
    // the end/open flag for each indepedent feature on a cartilemap must always returned in array `report_flag`,
    // if any values in the array `report_flag` == `0xffff`, that means no feature exists for that index.

bool feature_complete_check (GAME_FEATURE_NODE_ptr feature_root);
    // return true, if feature is complete (all direction is ended with `end_node`)
    // return false, otherwise.

GAME_FEATURE_NODE_ptr delete_whole_feature (GAME_FEATURE_NODE_ptr feature_root);
    // free/ delete the whole feature.
    // it uses the `delete_node` function.
    // return NULL, regardless the delete process is successful or not.
void delete_node (GAME_FEATURE_NODE_ptr node);

GAME_FEATURE_NODE_ptr node_exist_return_node (GAME_FEATURE_NODE_ptr feature_root, GAME_FEATURE_NODE_ptr new_node);
    // return pointer to node, if node already exists in the feature; otherwise,
    // return NULL

extern GAME_FEATURE_NODE end_node;
extern u32 number_tile_in_vram;
extern GAME_FEATURES tile_vram_description[29];
extern GAME_FEATURES tile_vram_field_description[29];

// === 
// DEBUG VERSION 
// ===

GAME_FEATURE_NODE_ptr merging_features_debug (GAME_FEATURE_NODE_ptr feature_root_ref, GAME_FEATURE_NODE_ptr feature_root_2, unsigned char* debug_merg_tid, DIRECTION* debug_merg_dir, unsigned char* mrg_order);
GAME_FEATURE_NODE_ptr delete_whole_feature_debug (GAME_FEATURE_NODE_ptr feature_root, unsigned char* debug_del, unsigned char* found_order);

#endif