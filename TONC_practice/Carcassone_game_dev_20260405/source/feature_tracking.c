#include "feature_tracking.h"

GAME_FEATURE_NODE_ptr create_node (s32 tx_coord, s32 ty_coord, u32 tid, GAME_FEATURES tile_feature, DIRECTION parent_direction)
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
        switch (tile_feature)
        {
            case CITY:
                // in VRAM
                if (tid == 15 || tid == 19 || tid == 21)
                {
                    // end in right/ bot
                    new_node->child_top_lk = NULL;
                    new_node->child_r_lk = &end_node;
                    new_node->child_bot_lk = &end_node;
                    new_node->child_l_lk = NULL;
                    
                } 
                if (tid == 24 || tid == 17)
                {
                    new_node->child_top_lk = &end_node;
                    new_node->child_r_lk = NULL;
                    new_node->child_bot_lk = NULL;
                    new_node->child_l_lk = &end_node;
                    if (parent_direction == RIGHT || parent_direction == BOT)
                    {
                        new_node->child_top_lk = NULL;
                        new_node->child_r_lk = &end_node;
                        new_node->child_bot_lk = &end_node;
                        new_node->child_l_lk = NULL;
                    }
                }
                if (tid == 16 || tid == 20 || tid == 22)
                {
                    // end in right/ bot
                    new_node->child_top_lk = NULL;
                    new_node->child_r_lk =NULL;
                    new_node->child_bot_lk = &end_node;
                    new_node->child_l_lk = &end_node;
                    
                }
                if (tid == 25 || tid ==18)
                {
                    new_node->child_top_lk = &end_node;
                    new_node->child_r_lk = &end_node;
                    new_node->child_bot_lk = NULL;
                    new_node->child_l_lk = NULL;
                    if (parent_direction == BOT || parent_direction == LEFT)
                    {
                        new_node->child_top_lk = NULL;
                        new_node->child_r_lk =NULL;
                        new_node->child_bot_lk = &end_node;
                        new_node->child_l_lk = &end_node;
                    }
                }
                break;
            
            case STREET:
                //CODE
                break;
            case FIELD:
                //CODE
                break;
            default:
                // CODE
        }
        new_node->car_tid = tid;

        new_node->tx = tx_coord;
        new_node->ty = ty_coord;

        new_node->top_coord_x = tx_coord;
        new_node->top_coord_y = ty_coord-1;
        new_node->right_coord_x = tx_coord+1;
        new_node->right_coord_y = ty_coord;
        new_node->bot_coord_x = tx_coord;
        new_node->bot_coord_y = ty_coord+1;
        new_node->left_coord_x = tx_coord-1;
        new_node->left_coord_y = ty_coord;

        new_node->parent_top_lk = NULL;
        new_node->parent_r_lk = NULL;
        new_node->parent_bot_lk = NULL;
        new_node->parent_l_lk = NULL;
        
        return new_node;
    }
}

void delete_node (GAME_FEATURE_NODE_ptr node)
{
    free(node);
}