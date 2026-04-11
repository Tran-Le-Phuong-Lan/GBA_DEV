#include "feature_tracking.h"

GAME_FEATURE_NODE_ptr create_node (s32 tx_coord, s32 ty_coord, u32 tid)
{
    GAME_FEATURE_NODE_ptr new_node;
    new_node = (GAME_FEATURE_NODE_ptr)malloc(sizeof(GAME_FEATURE_NODE));
    if(new_node == NULL)
    {
        // malloc fails do not know how to handle yet
        return NULL;
    }
    else
    {
        new_node->car_tid = tid;

        new_node->top_coord_x = tx_coord;
        new_node->top_coord_y = ty_coord-1;
        new_node->right_coord_x = tx_coord+1;
        new_node->right_coord_y = ty_coord;
        new_node->bot_coord_x = tx_coord;
        new_node->bot_coord_y = ty_coord+1;
        new_node->left_coord_x = tx_coord-1;
        new_node->left_coord_y = ty_coord;

        new_node->top_lk = NULL;
        new_node->r_lk = NULL;
        new_node->bot_lk = NULL;
        new_node->l_lk = NULL;

        return new_node;
    }
}

void delete_node (GAME_FEATURE_NODE_ptr node)
{
    free(node);
}