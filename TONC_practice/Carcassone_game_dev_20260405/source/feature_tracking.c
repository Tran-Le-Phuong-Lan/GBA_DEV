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

u32 number_tile_in_vram = 29; // manual update in file `tiles-walllite.s` or mGba -> load rom -> View tiles

GAME_FEATURES tile_vram_description[29] = // manual update 
{
    // 0
    NA_FEATURE,
    // 1 
    NA_FEATURE, 
    // 2
    FIELD,
    // 3
    STREET,
    // 4
    STREET,
    // 5
    STREET,
    // 6
    STREET,
    // 7
    STREET,
    // 8
    STREET,
    // 9
    STREET,
    // 10
    STREET,
    // 11
    STREET,
    // 12
    STREET,
    // 13
    STREET,
    // 14 (all open, open/end based on the adjacent tiles on the game map)
    CITY,
    // 15
    CITY,
    // 16
    CITY,
    // 17
    CITY,
    // 18
    CITY,
    // 19
    CITY,
    // 20
    CITY,
    // 21
    CITY,
    // 22
    CITY,
    // 23 (all open, open/end based on the adjacent tiles on the game map)
    CITY,
    // 24 (special, 2 different nodes on the same tile)
    CITY,
    // 25 (special, 2 different nodes on the same tile)
    CITY,
    // 26
    GARDEN,
    // 27
    CHURCH,
    // 28
    MEEPLE_TILE
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
                
                if (tid == 15 || tid == 19 )
                {
                    // end in right/ bot
                    new_node->child_top_lk = NULL;
                    new_node->child_r_lk = &end_node;
                    new_node->child_bot_lk = &end_node;
                    new_node->child_l_lk = NULL;
                    
                } 
                if (tid == 17 || tid == 21) // tid= 24 handle dynamically
                {
                    new_node->child_top_lk = &end_node;
                    new_node->child_r_lk = NULL;
                    new_node->child_bot_lk = NULL;
                    new_node->child_l_lk = &end_node;
                }
                if (tid == 16 || tid == 20)
                {
                    // end in right/ bot
                    new_node->child_top_lk = NULL;
                    new_node->child_r_lk =NULL;
                    new_node->child_bot_lk = &end_node;
                    new_node->child_l_lk = &end_node;
                    
                }
                if (tid ==18 || tid == 22) // tid = 25 handled hynamically
                {
                    new_node->child_top_lk = &end_node;
                    new_node->child_r_lk = &end_node;
                    new_node->child_bot_lk = NULL;
                    new_node->child_l_lk = NULL;
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

    if(
        feature_root->top_coord_x == new_node->tx 
        && feature_root->top_coord_y == new_node->ty
        && feature_root->child_top_lk==NULL
        && feature_root->parent_top_lk==NULL
        && new_node->parent_bot_lk==NULL
        && new_node->child_bot_lk==NULL
        )
    {
        *child_direction = TOP;
        return feature_root;
    }

    if(
        feature_root->right_coord_x == new_node->tx 
        && feature_root->right_coord_y == new_node->ty
        && feature_root->child_r_lk==NULL
        && feature_root->parent_r_lk==NULL
        && new_node->parent_l_lk==NULL
        && new_node->child_l_lk==NULL
        )
    {
        *child_direction = RIGHT;
        return feature_root;
    }

    if(
        feature_root->bot_coord_x == new_node->tx 
        && feature_root->bot_coord_y == new_node->ty
        && feature_root->child_bot_lk==NULL
        && feature_root->parent_bot_lk==NULL
        && new_node->parent_top_lk==NULL
        && new_node->child_top_lk==NULL
        )
    {
        *child_direction = BOT;
        return feature_root;
    }

    if(
        feature_root->left_coord_x == new_node->tx 
        && feature_root->left_coord_y == new_node->ty
        && feature_root->child_l_lk==NULL
        && feature_root->parent_l_lk==NULL
        && new_node->parent_r_lk==NULL
        && new_node->child_r_lk==NULL
        )
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

GAME_FEATURE_NODE_ptr delete_whole_feature (GAME_FEATURE_NODE_ptr feature_root, unsigned char* debug_del, unsigned char* found_order)
{
    // must delete from leaf-node to root
    if(feature_root == NULL || feature_root->game_feature == END_FEATURE)
    {
        return NULL;
    }

    // if (feature_root->parent_top_lk == NULL)
    // {
        // the corresponding child direction might not NULL
        // = search in that direction
        feature_root->child_top_lk=delete_whole_feature(feature_root->child_top_lk, debug_del, found_order);
    // }
    
    // if (feature_root->parent_r_lk == NULL)
    // {
        feature_root->child_r_lk=delete_whole_feature(feature_root->child_r_lk, debug_del, found_order);
    // }

    // if (feature_root->parent_bot_lk == NULL)
    // {
        feature_root->child_bot_lk=delete_whole_feature(feature_root->child_bot_lk, debug_del, found_order);
    // }

    // if (feature_root->parent_l_lk == NULL)
    // {
        feature_root->child_l_lk=delete_whole_feature(feature_root->child_l_lk, debug_del, found_order);
    // }

    // before delete the node, must make sure that all of links to it from any exisitent parents are null, 
    // otherwise free() might be applied twice on the alread-freed node
    if(feature_root->parent_top_lk!=NULL)
    {
        feature_root->parent_top_lk->child_bot_lk = NULL;
    }
    if(feature_root->parent_r_lk!=NULL)
    {
        feature_root->parent_r_lk->child_l_lk = NULL;
    }
    if(feature_root->parent_bot_lk!=NULL)
    {
        feature_root->parent_bot_lk->child_top_lk = NULL;
    }
    if(feature_root->parent_l_lk!=NULL)
    {
        feature_root->parent_l_lk->child_r_lk = NULL;
    }

    // For debug purpose
    if (
        ((feature_root->parent_top_lk==NULL && feature_root->child_top_lk!=NULL)
        || (feature_root->parent_top_lk!=NULL && feature_root->child_top_lk==NULL)
        || (feature_root->parent_top_lk==NULL && feature_root->child_top_lk==NULL))
        && ((feature_root->parent_r_lk==NULL && feature_root->child_r_lk!=NULL)
        || (feature_root->parent_r_lk!=NULL && feature_root->child_r_lk==NULL)
        || (feature_root->parent_r_lk==NULL && feature_root->child_r_lk==NULL))
        && ((feature_root->parent_bot_lk==NULL && feature_root->child_bot_lk!=NULL)
        || (feature_root->parent_bot_lk!=NULL && feature_root->child_bot_lk==NULL)
        || (feature_root->parent_bot_lk==NULL && feature_root->child_bot_lk==NULL))
        && ((feature_root->parent_l_lk==NULL && feature_root->child_l_lk!=NULL)
        || (feature_root->parent_l_lk!=NULL && feature_root->child_l_lk==NULL)
        || (feature_root->parent_l_lk==NULL && feature_root->child_l_lk==NULL))
        )
    {
        debug_del[*found_order] =  feature_root->car_tid;
        *found_order = *found_order +1;
    }
    else
    {
        debug_del[*found_order] =  feature_root->car_tid;
    }
    free(feature_root);   
    return NULL;
}
void delete_node (GAME_FEATURE_NODE_ptr node)
{
    // free() returns void
    // IMPORTANT: if free() applied twice on the same address -> UB (undefined behavior ~ program might continue/ freeze/ crashes = bad practice).
    // referece: https://www.reddit.com/r/cprogramming/comments/1cebu9x/c_calling_free_twice/
    // but if free(NULL) -> ok!
    free(node);
}

void finish_features_linking (GAME_FEATURE_NODE_ptr new_node, GAME_FEATURE_NODE_ptr feature_root)
{
    if(feature_root == NULL || feature_root->game_feature == END_FEATURE)
    {
        return;
    }

    finish_features_linking(new_node, feature_root->child_top_lk);
    finish_features_linking(new_node, feature_root->child_r_lk);
    finish_features_linking(new_node, feature_root->child_bot_lk);
    finish_features_linking(new_node, feature_root->child_l_lk);
    
    if(new_node->top_coord_x == feature_root->tx 
        && new_node->top_coord_y == feature_root->ty
        && new_node->parent_top_lk == NULL
        && new_node->child_top_lk == NULL
        && feature_root->child_bot_lk == NULL
        && feature_root->parent_bot_lk == NULL
            )
        {
            new_node->parent_top_lk = feature_root;
            feature_root->child_bot_lk = new_node;
            return;
        }

    if(new_node->right_coord_x == feature_root->tx 
        && new_node->right_coord_y == feature_root->ty
        && new_node->parent_r_lk == NULL
        && new_node->child_r_lk == NULL
        && feature_root->child_l_lk==NULL
        && feature_root->parent_l_lk==NULL
            )
        {
            new_node->parent_r_lk = feature_root;
            feature_root->child_l_lk = new_node;
            return;
        }

    if(new_node->bot_coord_x == feature_root->tx 
        && new_node->bot_coord_y == feature_root->ty
        && new_node->parent_bot_lk == NULL
        && new_node->child_bot_lk == NULL
        && feature_root->child_top_lk==NULL
        && feature_root->parent_top_lk==NULL
            )
        {
            new_node->parent_bot_lk = feature_root;
            feature_root->child_top_lk = new_node;
            return;
        }

    if(new_node->left_coord_x == feature_root->tx 
        && new_node->left_coord_y == feature_root->ty
        && new_node->parent_l_lk == NULL
        && new_node->child_l_lk == NULL
        && feature_root->child_r_lk==NULL
        && feature_root->parent_r_lk==NULL
            )
        {
            new_node->parent_l_lk = feature_root;
            feature_root->child_r_lk = new_node;
            return;
        }

}


GAME_FEATURE_NODE_ptr merging_features (GAME_FEATURE_NODE_ptr feature_root_ref, GAME_FEATURE_NODE_ptr feature_root_2, unsigned char* debug_merg_tid, DIRECTION* debug_merg_dir, unsigned char* mrg_order)
{
    // return NULL, if no merge is done.
    // return pointer to the merged feature root, if merge is done

    if(feature_root_2 == NULL || feature_root_2->game_feature == END_FEATURE)
    {
        return NULL;
    }

    // start checking at the leaf-node of feature_root_2 against feature_root_ref
    merging_features(feature_root_ref, feature_root_2->child_top_lk, debug_merg_tid, debug_merg_dir, mrg_order);
    merging_features(feature_root_ref, feature_root_2->child_r_lk, debug_merg_tid, debug_merg_dir, mrg_order);
    merging_features(feature_root_ref, feature_root_2->child_l_lk, debug_merg_tid, debug_merg_dir, mrg_order);
    merging_features(feature_root_ref, feature_root_2->child_bot_lk, debug_merg_tid, debug_merg_dir, mrg_order);
        // at the current leaf-node, find its location in feature_root_ref
    DIRECTION child_direction_new_node = NA_DIR;
    GAME_FEATURE_NODE_ptr location_new_node = NULL;
// GAME_FEATURE_NODE_ptr find_node (GAME_FEATURE_NODE_ptr feature_root, GAME_FEATURE_NODE_ptr new_node, DIRECTION* child_direction)
    location_new_node = find_node(feature_root_ref, feature_root_2, &child_direction_new_node);

    if(location_new_node != NULL)
    {
        // node can be merged,
        switch(child_direction_new_node)
        {

            case TOP:
                if(
                    feature_root_2->child_bot_lk==NULL 
                    || feature_root_2->child_bot_lk->game_feature!=END_FEATURE
                    )
                {
                    // we do not care that the parent lk exists,
                    // because we check the position based on the carcasonne rule adjacent and match, 
                    // therefore no two different carcassonne tiles can be adjacent and match to the same tile at the same postion
                    
                    // start insert/link the new node to the feature structure
                    location_new_node->child_top_lk = feature_root_2;
                    // register the parent address and its direction inside the new_node
                    feature_root_2->parent_bot_lk = location_new_node;
                    // null the child in the opposite of found direction to null, 
                    // because it already has a parent in that direction
                    feature_root_2->child_bot_lk = NULL;
                    // null the existent parent lks of current node except the newly added parent direction, to avoid loop inside the structure
                        // 1 lk between two node defined by 1 parent lk (from child) and 1 child lk (from parent)
                    if (feature_root_2->parent_top_lk!=NULL)
                    {
                        feature_root_2->parent_top_lk->child_bot_lk = NULL;
                        feature_root_2->parent_top_lk = NULL;
                    }
                    if (feature_root_2->parent_r_lk!=NULL)
                    {
                        feature_root_2->parent_r_lk->child_l_lk = NULL;
                        feature_root_2->parent_r_lk = NULL;
                    }
                    if  (feature_root_2->parent_l_lk != NULL)
                    {
                        feature_root_2->parent_l_lk->child_r_lk = NULL;
                        feature_root_2->parent_l_lk = NULL;
                    }
                    // finish_features_linking (GAME_FEATURE_NODE_ptr new_node, GAME_FEATURE_NODE_ptr feature_root)
                    finish_features_linking(feature_root_2, feature_root_ref);
                        // DEBUG
                    debug_merg_tid[*mrg_order] = feature_root_2->car_tid;
                    debug_merg_dir[*mrg_order] = child_direction_new_node;
                    *mrg_order = *mrg_order + 1;
                    return feature_root_ref;
                }     
                break;
            case RIGHT:
                if(
                    (feature_root_2->child_l_lk==NULL
                    || feature_root_2->child_l_lk->game_feature!=END_FEATURE)
                     
                    )
                {
                    // NO NEED FOR THIS CHECK,
                    // IN REALITY IT CAN NOT HAPPEN, it is taken care by the `find_node`
                    // if (location_new_node->child_r_lk==NULL &&
                    // location_new_node->parent_r_lk==NULL)
                    // {
                        location_new_node->child_r_lk = feature_root_2;
                        feature_root_2->parent_l_lk = location_new_node;
                        // null the child in the opposite of found direction to null
                        feature_root_2->child_l_lk = NULL;
                        // null the existent parent lks of current node except the newly added parent direction, to avoid loop inside the structure
                        if(feature_root_2->parent_top_lk != NULL)
                        {
                            feature_root_2->parent_top_lk->child_bot_lk = NULL;
                            feature_root_2->parent_top_lk = NULL;
                        }
                        if (feature_root_2->parent_r_lk != NULL)
                        {
                            feature_root_2->parent_r_lk->child_l_lk = NULL;
                            feature_root_2->parent_r_lk = NULL;
                        }
                        if (feature_root_2->parent_bot_lk != NULL)
                        {
                            feature_root_2->parent_bot_lk->child_top_lk = NULL;
                            feature_root_2->parent_bot_lk = NULL;
                        }
                        // finish_features_linking (GAME_FEATURE_NODE_ptr new_node, GAME_FEATURE_NODE_ptr feature_root)
                        finish_features_linking(feature_root_2, feature_root_ref);
                            // DEBUG
                        debug_merg_tid[*mrg_order] = feature_root_2->car_tid;
                        debug_merg_dir[*mrg_order] = child_direction_new_node;
                        *mrg_order = *mrg_order + 1;
                    // }
                    
                    return feature_root_ref;
                }
                break;
            case BOT:
                if(
                    feature_root_2->child_top_lk==NULL
                    || feature_root_2->child_top_lk->game_feature!=END_FEATURE
                    )
                {
                    location_new_node->child_bot_lk = feature_root_2;
                    feature_root_2->parent_top_lk = location_new_node;
                    // null the child in the opposite of found direction to null
                    feature_root_2->child_top_lk = NULL;
                    // null the existent parent lks of current node except the newly added parent direction, to avoid loop inside the structure
                    if (feature_root_2->parent_r_lk != NULL)
                    {
                        feature_root_2->parent_r_lk->child_l_lk = NULL;
                        feature_root_2->parent_r_lk = NULL;
                    }
                    if (feature_root_2->parent_bot_lk != NULL)
                    {
                        feature_root_2->parent_bot_lk->child_top_lk = NULL;
                        feature_root_2->parent_bot_lk = NULL;
                    }
                    if (feature_root_2->parent_l_lk != NULL)
                    {
                        feature_root_2->parent_l_lk->child_r_lk = NULL;
                        feature_root_2->parent_l_lk = NULL;
                    }
                    // finish_features_linking (GAME_FEATURE_NODE_ptr new_node, GAME_FEATURE_NODE_ptr feature_root)
                    finish_features_linking(feature_root_2, feature_root_ref);
                        // DEBUG
                    debug_merg_tid[*mrg_order] = feature_root_2->car_tid;
                    debug_merg_dir[*mrg_order] = child_direction_new_node;
                    *mrg_order = *mrg_order + 1;
                    return feature_root_ref;
                }
                
                break;
            case LEFT:
                if(
                    feature_root_2->child_r_lk==NULL
                    || feature_root_2->child_r_lk->game_feature!=END_FEATURE
                    )
                {
                    location_new_node->child_l_lk = feature_root_2;
                    feature_root_2->parent_r_lk = location_new_node;
                    // null the child in the opposite of found direction to null
                    feature_root_2->child_r_lk = NULL;
                    // null the existent parent lks of current node except the newly added parent direction, to avoid loop inside the structure
                    if (feature_root_2->parent_top_lk != NULL)
                    {
                        feature_root_2->parent_top_lk->child_bot_lk = NULL;
                        feature_root_2->parent_top_lk = NULL;
                    }
                    if (feature_root_2->parent_bot_lk != NULL)
                    {
                        feature_root_2->parent_bot_lk->child_top_lk = NULL;
                        feature_root_2->parent_bot_lk = NULL;
                    }
                    if (feature_root_2->parent_l_lk != NULL)
                    {
                        feature_root_2->parent_l_lk->child_r_lk = NULL;
                        feature_root_2->parent_l_lk = NULL;
                    }
                    // finish_features_linking (GAME_FEATURE_NODE_ptr new_node, GAME_FEATURE_NODE_ptr feature_root)
                    finish_features_linking(feature_root_2, feature_root_ref);
                        // DEBUG
                    debug_merg_tid[*mrg_order] = feature_root_2->car_tid;
                    debug_merg_dir[*mrg_order] = child_direction_new_node;
                    *mrg_order = *mrg_order + 1;
                    return feature_root_ref;
                }
                break;
            default: // indicate ERROR
                return NULL;
            
        }
        return NULL;

    }
    
}

bool feature_min_max_coord (GAME_FEATURE_NODE_ptr feature_root, MIN_OR_MAX comp_info, COORD_2D* result, s32* cnt)
{
    if (feature_root==NULL || feature_root->game_feature==END_FEATURE)
    {
        return false;
    }
    
    feature_min_max_coord(feature_root->child_top_lk, comp_info, result, cnt);
    feature_min_max_coord(feature_root->child_r_lk, comp_info, result, cnt);
    feature_min_max_coord(feature_root->child_bot_lk, comp_info, result, cnt);
    feature_min_max_coord(feature_root->child_l_lk, comp_info, result, cnt);

    if (*cnt==0)
    {   
        // first time encounter the feature node.
        // initialize the result with values relative to this feature
        result->x= feature_root->tx;
        result->y= feature_root->ty;
        *cnt = *cnt +1;
        return false;
    }

    if(comp_info==MIN)
    {
        if(feature_root->tx < result->x)
        {
            result->x= feature_root->tx;
        }
        if(feature_root->ty < result->y)
        {
            result->y= feature_root->ty;
        }
    }

    if(comp_info==MAX)
    {
        if(feature_root->tx > result->x)
        {
            result->x= feature_root->tx;
        }
        if(feature_root->ty > result->y)
        {
            result->y= feature_root->ty;
        }
    }

    return true;
}

void feature_report_per_cartilemap (GAME_FEATURE_NODE_ptr feature_root, u16* report_flag, 
                                 COORD_2D feature_min_coord, COORD_2D feature_max_coord)
{
    // result is return in the `report_flag`.
    // `report_flag` defined as:
		// 0xffff = no info
		// 0xTRBL; 1=end, 0=open
		// for example, 0x0000 = all sides are open; 0x1010= only T and B are open
    
    // the check for all tiles at each edge, 
    // if any of the tile on the same edge is open, 
    // then the feature per that cartilemap is open on that edge.
    // IMPORTNAT: this type of checking only works for feature per cartilemap !
    // not for checking the whole feature during the game. 
    if (feature_root==NULL || feature_root->game_feature==END_FEATURE)
    {
        return;
    }
    feature_report_per_cartilemap(feature_root->child_top_lk, report_flag, feature_min_coord, feature_max_coord);
    feature_report_per_cartilemap(feature_root->child_r_lk, report_flag, feature_min_coord, feature_max_coord);
    feature_report_per_cartilemap(feature_root->child_bot_lk, report_flag, feature_min_coord, feature_max_coord);
    feature_report_per_cartilemap(feature_root->child_l_lk, report_flag, feature_min_coord, feature_max_coord);

    // tiles on top = min y
    if (feature_root->ty==feature_min_coord.y)
    {
        if (feature_root->child_top_lk!=NULL)
        {
            if(feature_root->child_top_lk->game_feature==END_FEATURE)
            {
                // 0xTRBL; 1=end, 0=open 
                if (((*report_flag) & 0xf000)== 0xf000)
                {
                    // the first time, the flag is set
                    (*report_flag) = ((*report_flag) & 0x0fff) | 0x1000;
                }
                else
                {
                    (*report_flag) = (*report_flag) & 0x1fff;
                }
            }
        }
        else
        {
                // 0xTRBL; 1=end, 0=open
                if (((*report_flag) & 0xf000)== 0xf000)
                {
                    // the first time, the flag is set
                    (*report_flag) = ((*report_flag) & 0x0fff) | 0x0000;
                }
                else
                {
                    (*report_flag) = (*report_flag) & 0x0fff;
                }
        }        
    }

    // tiles on right = max x
    if (feature_root->tx==feature_max_coord.x)
    {
        if (feature_root->child_r_lk!=NULL)
        {
            if(feature_root->child_r_lk->game_feature==END_FEATURE)
            {
                // 0xTRBL; 1=end, 0=open 
                if (((*report_flag) & 0x0f00)== 0x0f00)
                {
                    // the first time, the flag is set
                    (*report_flag) = ((*report_flag) & 0xf0ff) | 0x0100;
                }
                else
                {
                    (*report_flag) = (*report_flag) & 0xf1ff;
                }
            }
        }
        else
        {
                // 0xTRBL; 1=end, 0=open 
                if (((*report_flag) & 0x0f00)== 0x0f00)
                {
                    // the first time, the flag is set
                    (*report_flag) = ((*report_flag) & 0xf0ff) | 0x0000;
                }
                else
                {
                    (*report_flag) = (*report_flag) & 0xf0ff;
                }
        }        
    }

    // tiles on bot = max y
    if (feature_root->ty==feature_max_coord.y)
    {
        if (feature_root->child_bot_lk!=NULL)
        {
            if(feature_root->child_bot_lk->game_feature==END_FEATURE)
            {
                // 0xTRBL; 1=end, 0=open 
                if (((*report_flag) & 0x00f0)== 0x00f0)
                {
                    // the first time, the flag is set
                    (*report_flag) = ((*report_flag) & 0xff0f) | 0x0010;
                }
                else
                {
                    (*report_flag) = (*report_flag) & 0xff1f;
                }
            }
        }
        else
        {
                // 0xTRBL; 1=end, 0=open 
                if (((*report_flag) & 0x00f0)== 0x00f0)
                {
                    // the first time, the flag is set
                    (*report_flag) = ((*report_flag) & 0xff0f) | 0x0000;
                }
                else
                {
                    (*report_flag) = (*report_flag) & 0xff0f;
                }
        }        
    }

    // tiles on left = min x
    if (feature_root->tx==feature_min_coord.x)
    {
        if (feature_root->child_l_lk!=NULL)
        {
            if(feature_root->child_l_lk->game_feature==END_FEATURE)
            {
                // 0xTRBL; 1=end, 0=open 
                if (((*report_flag) & 0x000f)== 0x000f)
                {
                    // the first time, the flag is set
                    (*report_flag) = ((*report_flag) & 0xfff0) | 0x0001;
                }
                else
                {
                    (*report_flag) = (*report_flag) & 0xfff1;
                }
            }
        }
        else
        {
                // 0xTRBL; 1=end, 0=open 
                if (((*report_flag) & 0x000f)== 0x000f)
                {
                    // the first time, the flag is set
                    (*report_flag) = ((*report_flag) & 0xfff0) | 0x0000;
                }
                else
                {
                    (*report_flag) = (*report_flag) & 0xfff0;
                }
        }        
    }

    return;
}

bool feature_complete_check (GAME_FEATURE_NODE_ptr feature_root)
{
    if(feature_root==NULL)
    {
        return false;
    }

    if(feature_root->game_feature==END_FEATURE)
    {
        return true;
    }
    
    if (feature_root->parent_top_lk==NULL)
    {
        if (feature_complete_check(feature_root->child_top_lk))
        {
            // nothing
        }
        else
        {
            return false;
        }    
    }
    
    if(feature_root->parent_r_lk==NULL)
    {
        if(feature_complete_check(feature_root->child_r_lk))
        {
            // nothing
        }
        else
        {
            return false;
        }
        
    }
        
    if(feature_root->parent_bot_lk==NULL)
    {
        if(feature_complete_check(feature_root->child_bot_lk))
        {
            // nothing
        }
        else
        {
            return false;
        }
    }
            
    if(feature_root->parent_l_lk==NULL)
    {
        if(feature_complete_check(feature_root->child_l_lk))
        {
            // nothing
        }
        else
        {
            return false;
        }
    }
                
    return true;
}