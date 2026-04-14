#include "feature_tracking.h"

GAME_FEATURE_NODE end_node =
{
    0,
    END_FEATURE,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL
};

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
                new_node->child_top_lk = NULL;
                new_node->child_r_lk = NULL;
                new_node->child_bot_lk = NULL;
                new_node->child_l_lk = NULL;
                
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
                new_node->child_top_lk = NULL;
                new_node->child_r_lk = NULL;
                new_node->child_bot_lk = NULL;
                new_node->child_l_lk = NULL;
        }
        new_node->car_tid = tid;

        new_node->game_feature = tile_feature;

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

GAME_FEATURE_NODE_ptr find_node (GAME_FEATURE_NODE_ptr feature_root, GAME_FEATURE_NODE_ptr new_node, DIRECTION* child_direction)
{
    // return NULL if not found suitable position for the new_node,
    // otherwuse return the node pointer where the new node can be linked to.

    // check the current node, before moving on
    if(feature_root == NULL || feature_root->game_feature == END_FEATURE)
    {
        return NULL;
    }

    if(feature_root->top_coord_x == new_node->tx && feature_root->top_coord_y == new_node->ty)
    {
        *child_direction = TOP;
        return feature_root;
    }

    if(feature_root->right_coord_x == new_node->tx && feature_root->right_coord_y == new_node->ty)
    {
        *child_direction = RIGHT;
        return feature_root;
    }

    if(feature_root->bot_coord_x == new_node->tx && feature_root->bot_coord_y == new_node->ty)
    {
        *child_direction = BOT;
        return feature_root;
    }

    if(feature_root->left_coord_x == new_node->tx && feature_root->left_coord_y == new_node->ty)
    {
        *child_direction = LEFT;
        return feature_root;
    }

    // moving on to check all the branches
    GAME_FEATURE_NODE_ptr result;
    result = find_node(feature_root->child_top_lk, new_node, child_direction);
    if (result != NULL)
    {
        return result;
    }
    result = find_node(feature_root->child_r_lk, new_node, child_direction);
    if (result != NULL)
    {
        return result;
    }
    result = find_node(feature_root->child_l_lk, new_node, child_direction);
    if (result != NULL)
    {
        return result;
    }
    result = find_node(feature_root->child_bot_lk, new_node, child_direction);
    if (result != NULL)
    {
        return result;
    }
    return result;
}


GAME_FEATURE_NODE_ptr insert_node (GAME_FEATURE_NODE_ptr feature_root, GAME_FEATURE_NODE_ptr new_node)
{
    // return NULL if insert node is unsuccessful, because the new node does not belong to this feature,
    // otherwise return the root of feature where the new node is added.
    DIRECTION child_direction_new_node = NA_DIR;
    GAME_FEATURE_NODE_ptr location_new_node = NULL;
    location_new_node = find_node(feature_root, new_node, &child_direction_new_node);

    if (location_new_node == NULL)
    {
        return NULL;
    }
    else
    {
        switch(child_direction_new_node)
        {
            case TOP:
                // link the new node to the feature structure
                location_new_node->child_top_lk = new_node;
                // register the parent address and its direction inside the new_node
                new_node->parent_bot_lk = location_new_node;
                break;
            case RIGHT:
                location_new_node->child_r_lk = new_node;
                new_node->parent_l_lk = location_new_node;
                break;
            case BOT:
                location_new_node->child_bot_lk = new_node;
                new_node->parent_top_lk = location_new_node;
                break;
            case LEFT:
                location_new_node->child_l_lk = new_node;
                new_node->parent_r_lk = location_new_node;
                break;
            default: // indicate ERROR
                return NULL;

        }
        return feature_root;
    }
}

void delete_whole_feature (GAME_FEATURE_NODE_ptr feature_root)
{
    // must delete from leaf-node to root
    if(feature_root == NULL || feature_root->game_feature == END_FEATURE)
    {
        return;
    }

    // if (feature_root->parent_top_lk == NULL)
    // {
        // the corresponding child direction might not NULL
        // = search in that direction
        delete_whole_feature(feature_root->child_top_lk);
    // }
    
    // if (feature_root->parent_r_lk == NULL)
    // {
        delete_whole_feature(feature_root->child_r_lk);
    // }

    // if (feature_root->parent_bot_lk == NULL)
    // {
        delete_whole_feature(feature_root->child_bot_lk);
    // }

    // if (feature_root->parent_l_lk == NULL)
    // {
        delete_whole_feature(feature_root->child_l_lk);
    // }

    delete_node(feature_root);
    // For debug purpose
    // feature_root->game_feature = DELETE_FEATURE;
}
void delete_node (GAME_FEATURE_NODE_ptr node)
{
    free(node);
}