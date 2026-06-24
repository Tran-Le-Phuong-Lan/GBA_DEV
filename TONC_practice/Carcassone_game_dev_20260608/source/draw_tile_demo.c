//
// reference: obj_demo.c, sbb_aff.c, 
// Carcassonne GBA
// 20250115 - , Tran Le Phuong Lan & David Guttandin
// 

#include <stdlib.h> // abs()
#include <math.h> // ceil(), floor()

#include <tonc.h>
#include "tiles-walllite.h"
#include "carcas_data.h"
#include "tiles-bg1.h"
#include "carcas_libs.h"

// ===========
// OBJ/ SPRITE
// ===========
OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

#define INIT_OBJ_X 104// [pixel]
#define INIT_OBJ_Y 56

// ===========
// BG 2, afiine - main conceptual, dynamic carcassonne game map
// ===========
BG_AFFINE bgaff; // .8f = the last eight bit representing the fractional, 1 fraction = 1/256 [pixel]

#define INIT_BG_X_OFF  0x0088// = 136 [pixel]
#define INIT_BG_Y_OFF  0x00B0// = 176 [pixel]

#define CBB_0  0
// 1 CBB = 8 SBBs
// 1 SBB either = 2048 byte long  = 64 TILEs (1 TILE 8x8p@ 4bpp = 32 bytes) = 32 TILE8s (1 TILE8 8x8p@ 8bpp = 64 bytes)
// 		 or = 32 x 32 [tile] regular map (bcz 1 SE = 16 bits = 2 bytes) = 64 x 32 [tile] aff map (1 SAE = 8 bit = 1 byte)
// In this context, we store 28 Tiles from 0th CBB ~ 0th SBB in 0th CBB 
#define SBB_0  2// 64x64 tile map = 2 SBBs -> this background consumes (se_mem[2], se_mem[3])
// typedef u8  SCR_AFF_ENTRY, SAE;		//!< Type for affine screen entries (tonc_types)
// typedef u16 SCR_ENTRY, SE;			//!< Type for screen entries
SCR_ENTRY *bg2_map= se_mem[SBB_0];
// aff bg 64 x 64 tile = 2 SBBs 
int map_width_unit_tile = 64; // [TILE]
int map_height_unit_tile = 64; // [TILE]
// because of 2 manual tile added manually (background tile, transparent tile)
#define CAR_TILE_OFFSET_IN_VRAM 2

// ===========
// BG 0 - TEXT 
// ===========
#define CBB_0_TEXT 2  // tiles for text	
#define SBB_0_TEXT 31 // map for text

// ===========
// BG 1 - meeples, indications
// ===========
// use DTILE = 8x8p x 8bpp, share the same pallette with BG 2- affine
#define CBB_0_BG1  1
#define SBB_0_BG1  4 // 64x64 tile map = 4 sbbs = (se_mem[4], se_mem[5], se_mem[6], se_mem[7])

SCR_ENTRY *bg1_map= se_mem[SBB_0_BG1];
int bg1_width_unit_tile = 64; // [TILE]
int bg1_height_unit_tile = 64; // [TILE]
BG_POINT bg1_pt = {INIT_BG_X_OFF,INIT_BG_Y_OFF};

// ===========
// GBA SCREEN
// ===========
#define SCR_WIDTH_uPx 240 // [pixel]
#define SCR_HEIGHT_uPx 160  

// ==========
// FUNCS
// ==========
// copy from tonc-example `sbb_reg` (TONC library for GBA),
// mapping the tile-coordinate to multiple-sbb regular background screen entry index,
// pitch = bg width (x_axis) in tile unit.

// === 
// 1. RENDERING 
// ===
u32 map_to_reg_se_index(u32 tx, u32 ty, u32 pitch)
{
	u32 sbb= ((tx>>5)+(ty>>5)*(pitch>>5));

	return sbb*1024 + ((tx&31)+(ty&31)*32);
}
// [ctile] = 3x3 [tile]
// [tile] = 8x8 [pixel]
//					 = sae_curr_x		= sae_curr_y
void map_tile_to_ctile (int tile_x, int tile_y, s32* ctile_x, s32* ctile_y)
{
	// receive unit tile, return unit ctile

	s32 REFERENCE_CTILE = REF_CAR_TILE; // [ctile], ctile_dx = 20, ctile_dy=20, bcz index starts from 0
	s32 INIT_TILE_DX = (INIT_OBJ_X + (INIT_BG_X_OFF)) >> 3; // [tile]
	s32 INIT_TILE_DY = (INIT_OBJ_Y + (INIT_BG_Y_OFF)) >> 3; //

	s32 move_ctile_dx = ((tile_x - INIT_TILE_DX)/3);
	s32 move_ctile_dy = ((tile_y - INIT_TILE_DY)/3);
	s32 reminder_x=(abs(tile_x-INIT_TILE_DX))%3;
	s32 reminder_y=(abs(tile_y-INIT_TILE_DY))%3;
	if (tile_x >= 0)
	{
		*ctile_x = move_ctile_dx + REFERENCE_CTILE; // [ctile]	
	} 
	else
	{
		if (reminder_x == 0)
		{
			*ctile_x = move_ctile_dx + REFERENCE_CTILE;
		}
		else
		{
			*ctile_x = move_ctile_dx -1 + REFERENCE_CTILE;
		}
	}

	if(tile_y >= 0)
	{
		*ctile_y = move_ctile_dy + REFERENCE_CTILE;  
	}
	else
	{
		if (reminder_y == 0)
		{
			*ctile_y = move_ctile_dy + REFERENCE_CTILE;
		}
		else
		{
			*ctile_y = move_ctile_dy -1 + REFERENCE_CTILE;
		}
	}
}


void wrapping_tile_coord (COORD_2D* tile_coord)
{
	s32 tile_x_max = map_width_unit_tile -1, tile_y_max = map_height_unit_tile -1;
	s32 reminder_x = abs(tile_coord->x)%map_width_unit_tile,
		reminder_y = abs(tile_coord->y)%map_height_unit_tile;
	s32 quotient_x = abs(tile_coord->x)/map_width_unit_tile,
		quotient_y = abs(tile_coord->y)/map_height_unit_tile;

	if (tile_coord->x > tile_x_max) 
	{
		tile_coord->x = tile_coord->x - quotient_x*map_width_unit_tile;
	}
	if (tile_coord->x < 0)
	{
		if (reminder_x == 0)
		{
			tile_coord->x = quotient_x*map_width_unit_tile + tile_coord->x;
		}
		else
		{
			tile_coord->x = (quotient_x+1)*map_width_unit_tile + tile_coord->x;
		}
	}


	if (tile_coord->y > tile_y_max) 
	{
		tile_coord->y = tile_coord->y - quotient_y*map_height_unit_tile;
	}
	if (tile_coord->y < 0)
	{
		if (reminder_y == 0)
		{
			tile_coord->y = quotient_y*map_height_unit_tile + tile_coord->y;
		}
		else
		{
			tile_coord->y = (quotient_y+1)*map_height_unit_tile + tile_coord->y;
		}
	}  
}

void map_ctile_to_tile (s32 ctile_x, s32 ctile_y, s32* tile_x, s32* tile_y, bool return_wrap_tile)
{
	// receive unit ctile, return unit tile. Return the tile within the bg size !

	// must be within the bg size
	s32 REFERENCE_CTILE = REF_CAR_TILE; // [ctile], ctile_dx = 20, ctile_dy=20, bcz index starts from 0
	s32 INIT_TILE_DX = (INIT_OBJ_X + (INIT_BG_X_OFF)) >> 3; // [tile]
	s32 INIT_TILE_DY = (INIT_OBJ_Y + (INIT_BG_Y_OFF)) >> 3; // 
	s32 tile_x_max = map_width_unit_tile -1, tile_y_max = map_height_unit_tile -1;

	*tile_x = (ctile_x - REFERENCE_CTILE) * 3 + INIT_TILE_DX;
	*tile_y = (ctile_y - REFERENCE_CTILE)*3 + INIT_TILE_DY;
	if (return_wrap_tile == true)
	{
		COORD_2D wrap_tcoord;
		wrap_tcoord.x = *tile_x;
		wrap_tcoord.y = *tile_y;
		wrapping_tile_coord(&wrap_tcoord);
		*tile_x = wrap_tcoord.x;
		*tile_y = wrap_tcoord.y;
	}
	else
	{
		// nothing
	}

}


void render_bg_v2 (s32 tile_prev_x, s32 tile_prev_y, s32 tile_cur_x, s32 tile_cur_y, CAR_MAP_INFO* car_fmap, SCR_ENTRY *bg_gba, 
	bool* mv_tflg, bool* update_tfg, COORD_2D* tst_start_ctile,COORD_2D* tst_end_ctile, COORD_2D* tst_render_tid)
{
	// move 1 ctile step in any direction x/y -> trigger render 1 col/ 1 row
	s32 SAFE_ZONE_R = (SCR_WIDTH_uPx -INIT_OBJ_X) /24 + 1, // [ctile], 1 ctile = 3x3 tile
		SAFE_ZONE_L = INIT_OBJ_X /24 +1, 
		SAFE_ZONE_T = INIT_OBJ_Y /24 +1,
		SAFE_ZONE_B = (SCR_HEIGHT_uPx - INIT_OBJ_Y) /24 +1;

	//			  false: not moved, true: moved
	bool mv_flg = false;
	
	if (tile_cur_x != tile_prev_x | tile_cur_y != tile_prev_y)
	{
		mv_flg = true;
	}
	else
	{
		mv_flg = false;
	}

	
	*mv_tflg = mv_flg;
	// -> find ctile coordinate for starting rendering
	// s32 ctile_x_str, ctile_x_end, ctile_y_str, ctile_y_end; // [ctile]
	COORD_2D reference_ctile, ctile_str, ctile_end;
	map_tile_to_ctile(tile_cur_x, tile_cur_y, &reference_ctile.x, &reference_ctile.y); // [ctile]
	ctile_str.x = reference_ctile.x - (SAFE_ZONE_L+1);
	ctile_str.y = reference_ctile.y - (SAFE_ZONE_T+1);
	ctile_end.x = ctile_str.x + (SAFE_ZONE_L + SAFE_ZONE_R +1);
	ctile_end.y = ctile_str.y + (SAFE_ZONE_T + SAFE_ZONE_B+1);
	// == DEBUG
		tst_start_ctile->x = ctile_str.x;
		tst_start_ctile->y = ctile_str.y;
		tst_end_ctile->x = ctile_end.x;
		tst_end_ctile->y = ctile_end.y;
	
		// 	COORD_2D tile_curr_coord;
		// 	// tile_curr_coord.x = tile_cur_x; //ERROR !!! HOW THE RENDER TILE = THE CURRENT TID!!!!
		// 	// tile_curr_coord.y = tile_cur_y;
		// 	map_ctile_to_tile(ctile_str.x, ctile_str.y, &tile_curr_coord.x, &tile_curr_coord.y, false);
		// 	// wrapping_tile_coord(&tile_curr_coord);
		// 	COORD_2D tile_render_coord;
		// 			tile_render_coord.x = tile_curr_coord.x + 0;
		// 			tile_render_coord.y = tile_curr_coord.y + 0;
		// 			wrapping_tile_coord(&tile_render_coord);
		// 			int se_idx = 0, sea_idx = 0;
		// 			sea_idx = tile_render_coord.y*map_width_unit_tile + tile_render_coord.x; 
		// tst_render_tid->x = tile_render_coord.x;
		// tst_render_tid->y = tile_render_coord.y;
	// == DEBUG
	if (mv_flg == true) 
	{
		*update_tfg = mv_flg;
		// rendering the whole surrounding of 
		// the (screen+buffer zone (to fix the misalign screen size and conceptual map grid))
		s32 iter_x_ctile, iter_y_ctile;
		for (iter_y_ctile = ctile_str.y; iter_y_ctile < ctile_end.y + 1; iter_y_ctile += 1)
		{
			for (iter_x_ctile= ctile_str.x; iter_x_ctile < ctile_end.x +1; iter_x_ctile +=1)
			{
				int car_map_id;
				CAS_TILE_MAP* car_map_ptr = NULL;
				int car_off_vram = 0;
				int car_map_curr_coord = iter_y_ctile * CAR_MAP_WIDTH_x + iter_x_ctile;
				// must not use 1d idx to assess the 2d border -> it is wrong
				if (iter_y_ctile >= 0 && iter_y_ctile < CAR_MAP_HEIGHT_y 
					&& iter_x_ctile >= 0 && iter_x_ctile < CAR_MAP_WIDTH_x)
				{
					// the ctile is within the conceptual map
					// extract the CAR MAP TID
					car_map_id = CAR_BG_ID;
					int iter =0;
					for(iter; iter < CAR_TILES_MAX; iter +=1)
					{
						if(car_map_curr_coord == car_fmap[iter].car_map_coord)
						{
							// existent a car map tile in the conceptual map
							car_map_id = car_fmap[iter].car_tid;
						}
					}
					
					if (car_map_id == CAR_BG_ID)
					{
						car_map_ptr = bg_tile_map_id;
						car_map_id = 1;
						car_off_vram = 0;
					}
					else
					{
						car_map_ptr = cas_tile_map_id;
						car_off_vram = CAR_TILE_OFFSET_IN_VRAM;
					}
				}
				else
				{
					// outside of the CAR MAP, it should be transparent background
					car_map_ptr = bg_tile_map_id;
					car_map_id = 0;
					car_off_vram = 0;
				}

				if (iter_y_ctile == ctile_str.y | iter_y_ctile == ctile_end.y)
				{
					// start render the whole row
					int se_idx = 0, sea_idx = 0;
					COORD_2D tile_curr_coord;
					map_ctile_to_tile(iter_x_ctile, iter_y_ctile, &tile_curr_coord.x, &tile_curr_coord.y, false);
					COORD_2D tile_render_coord;
				
					// put down the starter tile
					int cas_r, cas_col;
					for (cas_r = 0; cas_r < 3; cas_r++)
					{
						for (cas_col=0; cas_col<3; cas_col++)
						{
							tile_render_coord.x = tile_curr_coord.x + cas_col;
							tile_render_coord.y = tile_curr_coord.y + cas_r;
							wrapping_tile_coord(&tile_render_coord);
							sea_idx = tile_render_coord.y*map_width_unit_tile + tile_render_coord.x; 

							se_idx = sea_idx >> 1;
							if (sea_idx % 2 == 0)
							{
								// write to lower 8-bit of pse, preseve the higher 8-bit of pse
								//														0 = CARCASONNE CAT 0 = STARTER TILE
								bg_gba[se_idx] = (bg_gba[se_idx] & 0xFF00) | ((car_map_ptr[car_map_id][cas_r*3 + cas_col]+car_off_vram) & 0x00FF);
							}
							else
							{
								// write higer 8-bit of pse, preserve the lower 8 bit of pse
								bg_gba[se_idx] = (bg_gba[se_idx] & 0x00FF) | (((car_map_ptr[car_map_id][cas_r*3 + cas_col]+car_off_vram)<<8)  & 0xFF00);
							}
						}
					}
				}

			
				if (iter_x_ctile == ctile_str.x | iter_x_ctile == ctile_end.x)
				{
					// render the whole column
					// start render
					int se_idx = 0, sea_idx = 0;
					COORD_2D tile_curr_coord;
					map_ctile_to_tile(iter_x_ctile, iter_y_ctile, &tile_curr_coord.x, &tile_curr_coord.y, false);
					COORD_2D tile_render_coord;
				
					// put down the starter tile
					int cas_r, cas_col;
					for (cas_r = 0; cas_r < 3; cas_r++)
					{
						for (cas_col=0; cas_col<3; cas_col++)
						{
							tile_render_coord.x = tile_curr_coord.x + cas_col;
							tile_render_coord.y = tile_curr_coord.y + cas_r;
							wrapping_tile_coord(&tile_render_coord);
							sea_idx = tile_render_coord.y*map_width_unit_tile + tile_render_coord.x; 

							se_idx = sea_idx >> 1;
							if (sea_idx % 2 == 0)
							{
								// write to lower 8-bit of pse, preseve the higher 8-bit of pse
								//														0 = CARCASONNE CAT 0 = STARTER TILE
								bg_gba[se_idx] = (bg_gba[se_idx] & 0xFF00) | ((car_map_ptr[car_map_id][cas_r*3 + cas_col]+car_off_vram) & 0x00FF);
							}
							else
							{
								// write higer 8-bit of pse, preserve the lower 8 bit of pse
								bg_gba[se_idx] = (bg_gba[se_idx] & 0x00FF) | (((car_map_ptr[car_map_id][cas_r*3 + cas_col]+car_off_vram)<<8)  & 0xFF00);
							}
						}
					}
				}
			}

		} 

	}
	else
	{
		// no render
	}


}

void init_map_info(CAR_MAP_INFO* full_map)
{
	int iter;
	for(iter=0; iter < CAR_TILES_MAX; iter+=1)
	{
		full_map[iter].car_tid = CAR_BG_ID;
		full_map[iter].car_map_coord = -1;

	}
}

// === 
// 0. MISC 
// ===

void win_textbox(int bgnr, int left, int top, int right, int bottom, int bldy)
{
	REG_WIN0H= left<<8 | right; 
	REG_WIN0V=  top<<8 | bottom;
	REG_WIN0CNT= WIN_ALL | WIN_BLD;
	REG_WINOUTCNT= WIN_ALL;

	REG_BLDCNT= (BLD_ALL&~BIT(bgnr)) | BLD_BLACK;
	REG_BLDY= bldy; // fade levels

	REG_DISPCNT |= DCNT_WIN0;

	tte_set_margins(left, top, right, bottom);
}

void init_reg_bg ()
{

	// == initialize affine background 2
	// REG_BG2CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_AFF_32x32; 
	REG_BG2CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_AFF_64x64 | BG_WRAP;
	// REG_BG2CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_AFF_64x64; 
	// set the initial position of the screen
	bgaff.pa = bg_aff_default.pa;
	bgaff.pb = bg_aff_default.pb;
	bgaff.pc = bg_aff_default.pc;
	bgaff.pd = bg_aff_default.pd;
	bgaff.dx = INIT_BG_X_OFF<<8; // = ... [pixel] to the right 
	bgaff.dy = INIT_BG_Y_OFF<<8;
	REG_BG_AFFINE[2]= bgaff;

	// Place the bg tile 
	// into VRAM: cbb == 0
	// tid for Carcassonne graphic starts from 1, tid 0 is the bg tile.
	memcpy32(&tile8_mem[CBB_0][0], tiles_wallliteTiles, tiles_wallliteTilesLen/sizeof(u32));

	// no palette, only 256 colors
	memcpy32(pal_bg_mem, tiles_walllitePal, tiles_walllitePalLen/sizeof(u32));
}

void init_reg_obj ()
{	
	// SPrite is 8x8p@8bpp, bcz it shares data with the background (8x8p@8bpp)

	// load in 256 palette (shared data with the background), 
	memcpy32(pal_obj_mem, tiles_walllitePal, tiles_walllitePalLen/sizeof(u32));
	
	
	// Place the cursor/sprite
	// into VRAM: LOW obj memory (cbb == 4)
	// tile_mem[4][0] = tiles[0];
	// first graphic is loaded in the func `draw_func` - game state `PUT_DOWN_TILE`

}

// === 
// 2. FEATURE TRACKING 
// ===
void init_feature_flgs (u16* flag_array, u16 flag_array_size)
{
	int iter;
	for (iter=0; iter<flag_array_size; iter=iter+1)
	{
		flag_array[iter]=0xffff;
	}
	return;
}
void init_features_per_tilemap (GAME_FEATURE_NODE_START* feature_array, u16 feature_array_size)
{
	int iter;
	for (iter=0; iter<feature_array_size; iter=iter+1)
	{
		feature_array[iter].root=NULL;
	}
	return;
}
void check_all_merge_possibilities (GAME_FEATURE_NODE_START* ftr_game_array, u16 ftr_game_array_size)
{
	// merge, delete merged features
	// 	it does not cover all the possibility	
	// -> Probably SOL 1: 
	// a track variable whose value is increased 1 whenever 1 succesfful merge occur,
	// because if 1 successful merge = + 1 mergeable possibility for other exisiting features.
	// the process stop ONLY when track variable is 0, it is begin with 1 = at the begining, there is always 1 possibility that there is 1 merge exists.
	int merg_possibility = 1;
	int fts_iter_ref=0, fts_iter=0;
	while (merg_possibility>0)
	{
		for (fts_iter_ref=0; fts_iter_ref<ftr_game_array_size; fts_iter_ref++)
		{
			for (fts_iter=0; fts_iter<ftr_game_array_size; fts_iter++)
			{
				if (fts_iter_ref!=fts_iter)
				{
					if (ftr_game_array[fts_iter_ref].root!=NULL)
					{
						if (ftr_game_array[fts_iter].root!=NULL)
						{
							
							// GAME_FEATURE_NODE_ptr merging_features_debug (GAME_FEATURE_NODE_ptr feature_root_ref, GAME_FEATURE_NODE_ptr feature_root_2, unsigned char* debug_merg_tid, DIRECTION* debug_merg_dir, unsigned char* mrg_order);
							GAME_FEATURE_NODE_ptr merge_res;
							merge_res=merging_features(ftr_game_array[fts_iter_ref].root, ftr_game_array[fts_iter].root);
							if (merge_res!=NULL)
							{
								ftr_game_array[fts_iter].root=NULL;
								merg_possibility= merg_possibility+1;
							}
							else
							{
								// nothing
							}
						}
						else
						{
							// nothing
						}
					
					}
					else
					{
						break;
					}
				}
				
			}
		}
		merg_possibility= merg_possibility-1;
	}

	return;
}

bool check_all_merge_possibilities_for_fields (GAME_FEATURE_NODE_START* ftr_game_array, u16 ftr_game_array_size, u32* merg_flg_arr)
{
	// merge, delete merged features
	// 	it does not cover all the possibility	
	// -> Probably SOL 1: 
	// a track variable whose value is increased 1 whenever 1 succesfful merge occur,
	// because if 1 successful merge = + 1 mergeable possibility for other exisiting features.
	// the process stop ONLY when track variable is 0, it is begin with 1 = at the begining, there is always 1 possibility that there is 1 merge exists.
	int merg_possibility = 1;
	int fts_iter_ref=0, fts_iter=0;
	bool merg_res = false;
	while (merg_possibility>0)
	{
		for (fts_iter_ref=0; fts_iter_ref<ftr_game_array_size; fts_iter_ref++)
		{
			for (fts_iter=0; fts_iter<ftr_game_array_size; fts_iter++)
			{
				if (fts_iter_ref!=fts_iter)
				{
					if (ftr_game_array[fts_iter_ref].root!=NULL)
					{
						if (ftr_game_array[fts_iter].root!=NULL)
						{
							if (merg_flg_arr[fts_iter_ref]==1
								&& merg_flg_arr[fts_iter]==1)
							{
								// GAME_FEATURE_NODE_ptr merging_features_debug (GAME_FEATURE_NODE_ptr feature_root_ref, GAME_FEATURE_NODE_ptr feature_root_2, unsigned char* debug_merg_tid, DIRECTION* debug_merg_dir, unsigned char* mrg_order);
								GAME_FEATURE_NODE_ptr merge_res;
								merge_res=merging_features(ftr_game_array[fts_iter_ref].root, ftr_game_array[fts_iter].root);
								if (merge_res!=NULL)
								{
									ftr_game_array[fts_iter].root=NULL;
									merg_possibility= merg_possibility+1;
									merg_res = true;
								}
								else
								{
									// nothing
								}
							}	
							
						}
						else
						{
							// nothing
						}
					
					}
					else
					{
						break;
					}
				}
				
			}
		}
		merg_possibility= merg_possibility-1;
	}

	return merg_res;
}

void insert_nodes_into_existent_ftrs (GAME_FEATURE_NODE_START* ftr_game_array, u16 ftr_game_array_sz, 
										GAME_FEATURE_NODE_ptr* new_nodes_array, u16 new_node_array_sz)
{
	// if nodes in new_nodes_array are inserted successfully, they are deleted
	// otherwise, they are kept.

	int fts_iter, new_node_iter=0;
	for (fts_iter=0; fts_iter < ftr_game_array_sz; fts_iter++)
	{
		// reference: [multiple expression in for-loop condition](https://stackoverflow.com/questions/16859029/multiple-conditions-in-a-c-for-loop)
		for (new_node_iter=0; new_node_iter<new_node_array_sz; new_node_iter++)
		{
			// bool node_exist (GAME_FEATURE_NODE_ptr feature_root, GAME_FEATURE_NODE_ptr new_node);
			if (new_nodes_array[new_node_iter]!=NULL && (~node_exist(ftr_game_array[fts_iter].root, new_nodes_array[new_node_iter])))
			{
				if (ftr_game_array[fts_iter].root == NULL)
				{
					
						ftr_game_array[fts_iter].root = new_nodes_array[new_node_iter];
						new_nodes_array[new_node_iter]=NULL;
					
				}
				else
				{
					// GAME_FEATURE_NODE_ptr insert_node (GAME_FEATURE_NODE_ptr feature_root, GAME_FEATURE_NODE_ptr new_node);
					GAME_FEATURE_NODE_ptr insert_res;
					insert_res=insert_node(ftr_game_array[fts_iter].root, new_nodes_array[new_node_iter]);
					if (insert_res!=NULL)
					{
						// void finish_features_linking (GAME_FEATURE_NODE_ptr new_node, GAME_FEATURE_NODE_ptr feature_root);
						finish_features_linking(new_nodes_array[new_node_iter], ftr_game_array[fts_iter].root);
						new_nodes_array[new_node_iter]=NULL;
					}
					
				}
			}		
		}
	}
}

void feature_report_per_cartilemap_implementation (u16* feature_flag_array, u16 feature_flag_array_sz,
													unsigned short *cur_car_tile_map_asm_id
													)
{
	// feature_flag_array[0][feature_flag_array_sz]: cities
	// feature_flag_array[1][feature_flag_array_sz]: street
	// ....

	// === 
	// FEATURE REPORT 
	// 2. checking the available feature 
	// ===
	//			the tile position is relative within the current carcassonne tile
	//			therefore, the car_r and car_col is the coord of the tile as well
	u32 fts_array_size =9, str_fts_array_sz=9;
	GAME_FEATURE_NODE_START city_features_per_tilemap[fts_array_size],
							str_features_per_tilemap[str_fts_array_sz];
	init_features_per_tilemap(city_features_per_tilemap, fts_array_size);
	init_features_per_tilemap(str_features_per_tilemap, str_fts_array_sz);

	GAME_FEATURES tile_type;
	u32 number_new_nodes =4;
	GAME_FEATURE_NODE_ptr new_nodes[number_new_nodes];
	new_nodes[0] = NULL;
	new_nodes[1] = NULL;
	new_nodes[2] = NULL;
	new_nodes[3] = NULL;

	int iter_col, iter_r;
		// CITY + STREETS
	for (iter_r = 0; iter_r < 3; iter_r++)
	{
		for (iter_col=0; iter_col<3; iter_col++)
		{
			unsigned short  tile_vram_id = cur_car_tile_map_asm_id[iter_r*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
			tile_type = tile_vram_description[tile_vram_id];

			// ===
			// create nodes
			if (
				new_nodes[0]==NULL
				&& new_nodes[1]==NULL
				&& new_nodes[2]==NULL
				&& new_nodes[3]==NULL
			)
			{
				switch (tile_vram_id)
				{
					case 3:
						
						new_nodes[0]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						if ((iter_r == 0 && iter_col==0) 
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 2 && iter_col==0)
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 2 && iter_col==1)
							|| (iter_r == 1 && iter_col==1)
							)
						{
							// check right
							unsigned short r_tile = cur_car_tile_map_asm_id[iter_r*3 + (iter_col+1)]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[r_tile] != STREET)
							{
								new_nodes[0]->child_r_lk= &end_node;
							}
						}
						
						if ((iter_r == 0 && iter_col==2) 
							|| (iter_r == 1 && iter_col==2)
							|| (iter_r == 2 && iter_col==2)
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 2 && iter_col==1)
							)
						{
							// check left
							unsigned short l_tile = cur_car_tile_map_asm_id[iter_r*3 + iter_col-1]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[l_tile] != STREET)
							{
								new_nodes[0]->child_l_lk= &end_node;
							}
						}
						
						break;
					case 4:
						
						new_nodes[0]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						if ((iter_r == 2 && iter_col==0) 
							|| (iter_r == 2 && iter_col==1)
							|| (iter_r == 2 && iter_col==2)
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 1 && iter_col==2)
							)
						{
							// check top
							unsigned short top_tile = cur_car_tile_map_asm_id[(iter_r-1)*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[top_tile] != STREET)
							{
								new_nodes[0]->child_top_lk= &end_node;
							}
						}
						
						if ((iter_r == 0 && iter_col==0) 
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 0 && iter_col==2)
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 1 && iter_col==2)
							)
						{
							// check bot
							unsigned short bot_tile = cur_car_tile_map_asm_id[(iter_r+1)*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[bot_tile] != STREET)
							{
								new_nodes[0]->child_bot_lk= &end_node;
							}
						}
						
						break;
					case 9: // 3 independent streets
						
						new_nodes[0]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[1]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[2]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						
						// add end nodes
						new_nodes[0]->child_top_lk=&end_node;
						new_nodes[0]->child_bot_lk=&end_node;
						new_nodes[0]->child_l_lk=&end_node;
						new_nodes[1]->child_top_lk=&end_node;
						new_nodes[1]->child_r_lk=&end_node;
						new_nodes[1]->child_l_lk=&end_node;
						new_nodes[2]->child_top_lk=&end_node;
						new_nodes[2]->child_r_lk=&end_node;
						new_nodes[2]->child_bot_lk=&end_node;
						break;
					case 10: // 3 independent streets
						
						new_nodes[0]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[1]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[2]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						
						// add end nodes
						new_nodes[0]->child_r_lk=&end_node;
						new_nodes[0]->child_bot_lk=&end_node;
						new_nodes[0]->child_l_lk=&end_node;
						new_nodes[1]->child_top_lk=&end_node;
						new_nodes[1]->child_r_lk=&end_node;
						new_nodes[1]->child_l_lk=&end_node;
						new_nodes[2]->child_top_lk=&end_node;
						new_nodes[2]->child_r_lk=&end_node;
						new_nodes[2]->child_bot_lk=&end_node;
						break;
					case 11: // 3 independent streets
						
						new_nodes[0]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[1]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[2]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						
						// add end nodes
						new_nodes[0]->child_r_lk=&end_node;
						new_nodes[0]->child_bot_lk=&end_node;
						new_nodes[0]->child_l_lk=&end_node;
						new_nodes[1]->child_top_lk=&end_node;
						new_nodes[1]->child_bot_lk=&end_node;
						new_nodes[1]->child_l_lk=&end_node;
						new_nodes[2]->child_top_lk=&end_node;
						new_nodes[2]->child_r_lk=&end_node;
						new_nodes[2]->child_bot_lk=&end_node;
						break;
					case 12: // 3 independent streets
						
						new_nodes[0]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[1]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[2]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						
						// add end nodes
						new_nodes[0]->child_r_lk=&end_node;
						new_nodes[0]->child_bot_lk=&end_node;
						new_nodes[0]->child_l_lk=&end_node;
						new_nodes[1]->child_top_lk=&end_node;
						new_nodes[1]->child_bot_lk=&end_node;
						new_nodes[1]->child_l_lk=&end_node;
						new_nodes[2]->child_top_lk=&end_node;
						new_nodes[2]->child_r_lk=&end_node;
						new_nodes[2]->child_l_lk=&end_node;	
						break;
					case 13: // 4 independent streets
						
						new_nodes[0]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[1]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[2]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[3]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						
						// add end nodes
						new_nodes[0]->child_r_lk=&end_node;
						new_nodes[0]->child_bot_lk=&end_node;
						new_nodes[0]->child_l_lk=&end_node;

						new_nodes[1]->child_top_lk=&end_node;
						new_nodes[1]->child_bot_lk=&end_node;
						new_nodes[1]->child_l_lk=&end_node;

						new_nodes[2]->child_top_lk=&end_node;
						new_nodes[2]->child_r_lk=&end_node;
						new_nodes[2]->child_l_lk=&end_node;
						
						new_nodes[3]->child_top_lk=&end_node;
						new_nodes[3]->child_r_lk=&end_node;
						new_nodes[3]->child_bot_lk=&end_node;
						break;
					case 24: // special city tile
					// GAME_FEATURE_NODE_ptr create_node (s32 tx_coord, s32 ty_coord, u32 tid, GAME_FEATURES tile_feature, DIRECTION parent_direction);
						
						new_nodes[0]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[1]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						
						// add the end nodes
						new_nodes[0]->child_r_lk = &end_node;
						new_nodes[0]->child_bot_lk = &end_node;
						new_nodes[1]->child_top_lk = &end_node;
						new_nodes[1]->child_l_lk = &end_node;
						break;
					case 25: // speicial city tile
						
						new_nodes[0]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						new_nodes[1]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						
						// add the end nodes
						new_nodes[0]->child_bot_lk = &end_node;
						new_nodes[0]->child_l_lk = &end_node;
						new_nodes[1]->child_top_lk = &end_node;
						new_nodes[1]->child_r_lk = &end_node;
						break;
					case 23: // all open, check adjacent tile to determine end point 
							// it falls through to case 14, and execute the same thing as in case 14.
					case 14: // all open, check adjacent tile to determine end point
						// reference: 
						// [1](https://alexanderobregon.substack.com/p/c-control-flow-with-if-switch-and)
						
						new_nodes[0]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						
						// checking on the top side
						if ((iter_r == 2 && iter_col==0) 
							|| (iter_r == 2 && iter_col==1)
							|| (iter_r == 2 && iter_col==2)
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 1 && iter_col==2)
							)
						{
							unsigned short top_tile = cur_car_tile_map_asm_id[(iter_r-1)*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[top_tile] != CITY)
							{
								new_nodes[0]->child_top_lk= &end_node;
							}
						}

						// checking on the right side
						if ((iter_r == 0 && iter_col==0) 
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 2 && iter_col==0)
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 2 && iter_col==1)
							|| (iter_r == 1 && iter_col==1)
							)
						{
							unsigned short r_tile = cur_car_tile_map_asm_id[iter_r*3 + (iter_col+1)]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[r_tile] != CITY)
							{
								new_nodes[0]->child_r_lk= &end_node;
							}
						}

						// checking on the bot side
						if ((iter_r == 0 && iter_col==0) 
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 0 && iter_col==2)
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 1 && iter_col==2)
							)
						{
							unsigned short bot_tile = cur_car_tile_map_asm_id[(iter_r+1)*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[bot_tile] != CITY)
							{
								new_nodes[0]->child_bot_lk= &end_node;
							}
						}

						// checking on the left side
						if ((iter_r == 0 && iter_col==2) 
							|| (iter_r == 1 && iter_col==2)
							|| (iter_r == 2 && iter_col==2)
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 2 && iter_col==1)
							)
						{
							unsigned short l_tile = cur_car_tile_map_asm_id[iter_r*3 + iter_col-1]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[l_tile] != CITY)
							{
								new_nodes[0]->child_l_lk= &end_node;
							}
						}

						break;
					default:
						
						if (tile_type == CITY || STREET)
						{
							new_nodes[0]= create_node(iter_col, iter_r, tile_vram_id, tile_type, NA_DIR);
						}

				}
			}
			
			// ===
			// insert node, merge feature, delete the merged feature except the ref feature where everything is merged into.
				// insert,
			switch(tile_type)
			{
				case CITY:
					insert_nodes_into_existent_ftrs(city_features_per_tilemap, fts_array_size,
											new_nodes, number_new_nodes);
						// merge, delete merged features
					check_all_merge_possibilities(city_features_per_tilemap, fts_array_size);
					break;
				case STREET:
					insert_nodes_into_existent_ftrs(str_features_per_tilemap, str_fts_array_sz,
											new_nodes, number_new_nodes);
						// merge, delete merged features
					check_all_merge_possibilities(str_features_per_tilemap, str_fts_array_sz);
					break;	
				default:
					// reset all new_nodes
					if (new_nodes[0]!=NULL)
					{
						free(new_nodes[0]);
						new_nodes[0]=NULL;
					}
					if (new_nodes[1]!=NULL)
					{
						free(new_nodes[1]);
						new_nodes[1]=NULL;
					}
					if (new_nodes[2]!=NULL)
					{
						free(new_nodes[2]);
						new_nodes[2]=NULL;
					}
					if (new_nodes[3]!=NULL)
					{
						free(new_nodes[3]);
						new_nodes[3]=NULL;
					}
			}
						
		}
	}
	
	
	// ==== 
	// FEATURE REPORT
	// 1. at the moment, no need to store just report -> must delete the feature.
	// ====
	// report end-open flags on the exisitng feature
			// CITY
	int fts_iter_a = 0, eoflg_iter=0;
	for (fts_iter_a=0; fts_iter_a < fts_array_size; fts_iter_a++)
	{
		MIN_OR_MAX option_max = MAX, option_min= MIN;
		COORD_2D min_res, max_res;
		if (city_features_per_tilemap[fts_iter_a].root!=NULL)
		{
			s32 encounter =0;
			// bool feature_min_max_coord (GAME_FEATURE_NODE_ptr feature_root, MIN_OR_MAX comp_info, COORD_2D* result, s32* cnt);
			feature_min_max_coord (city_features_per_tilemap[fts_iter_a].root, option_max, &max_res, &encounter);
			encounter =0;
			feature_min_max_coord (city_features_per_tilemap[fts_iter_a].root, option_min, &min_res, &encounter);
			// void feature_report_per_cartilemap (GAME_FEATURE_NODE_ptr feature_root, u16* report_flag, 
			//                  COORD_2D feature_min_coord, COORD_2D feature_max_coord)
			if (eoflg_iter<feature_flag_array_sz)
			{
				feature_report_per_cartilemap (city_features_per_tilemap[fts_iter_a].root, &feature_flag_array[0*feature_flag_array_sz + eoflg_iter], 
								min_res, max_res);
				eoflg_iter= eoflg_iter+1;
			}
			else
			{
				// out of range, for storing city feature. do nothing
			}
			
			
		}
		
	}
			// STREET
	fts_iter_a = 0, eoflg_iter=0;
	for (fts_iter_a=0; fts_iter_a < str_fts_array_sz; fts_iter_a++)
	{
		MIN_OR_MAX option_max = MAX, option_min= MIN;
		COORD_2D min_res, max_res;
		if (str_features_per_tilemap[fts_iter_a].root!=NULL)
		{
			s32 encounter =0;
			// bool feature_min_max_coord (GAME_FEATURE_NODE_ptr feature_root, MIN_OR_MAX comp_info, COORD_2D* result, s32* cnt);
			feature_min_max_coord (str_features_per_tilemap[fts_iter_a].root, option_max, &max_res, &encounter);
			encounter =0;
			feature_min_max_coord (str_features_per_tilemap[fts_iter_a].root, option_min, &min_res, &encounter);
			// void feature_report_per_cartilemap (GAME_FEATURE_NODE_ptr feature_root, u16* report_flag, 
			//                  COORD_2D feature_min_coord, COORD_2D feature_max_coord)
			if (eoflg_iter<feature_flag_array_sz)
			{
				feature_report_per_cartilemap (str_features_per_tilemap[fts_iter_a].root, &feature_flag_array[1*feature_flag_array_sz + eoflg_iter], 
								min_res, max_res);
				eoflg_iter= eoflg_iter+1;
			}
			else
			{
				// out of range, for storing city feature. do nothing
			}
			
			
		}
		
	}
	// delete/ free any feature existing in `features_per_tilemap`
		// CITY
	for (fts_iter_a=0; fts_iter_a < fts_array_size; fts_iter_a++)
	{
		if (city_features_per_tilemap[fts_iter_a].root!=NULL)
		{
			delete_whole_feature(city_features_per_tilemap[fts_iter_a].root);
		}
		
	}
		// STREET
	for (fts_iter_a=0; fts_iter_a < str_fts_array_sz; fts_iter_a++)
	{
		if (str_features_per_tilemap[fts_iter_a].root!=NULL)
		{
			
			delete_whole_feature(str_features_per_tilemap[fts_iter_a].root);
		}
		
	}
	return;	
}
void report_amount_features_per_cartilemap (u16* flag_array, u16 flag_array_size, u16* result, u16 result_sz)
{
	// result is 1D array.
	// result[0] = CITY
	// result[1] = STREET
	u16 iter, iter_r, iter_col;
	u16 flag_array_width=(flag_array_size/result_sz);
	for (iter=0; iter<result_sz; iter=iter+1)
	{
		result[iter]=0;	
	}

	for (iter_r=0; iter_r<result_sz; iter_r=iter_r+1)
	{
		for (iter_col=0; iter_col < flag_array_width; iter_col=iter_col+1)
		{
			if (flag_array[iter_r*flag_array_width + iter_col]!=0xffff)
			{
				result[iter_r]=result[iter_r]+1;
			}
		}
		
	}
	return;
}
void init_cartilemap_node_array(GAME_FEATURE_NODE_ptr* new_nodes_array, u16 new_node_array_sz)
{
	int iter;
	for (iter=0; iter<new_node_array_sz; iter=iter+1)
	{
		new_nodes_array[iter]=NULL;
	}
}

// cartilemap (= cartlmp) ~ carcassonne category (car cat)
// cartilemap id = car cat id = `rand_cat_id`
// cartilemap coord = car cat coord = `car_coord`
void create_cartilemap_node (u16* flag_array, u16 flag_array_size_width, u16 feature_type_idx,
							GAME_FEATURES cartilemap_id, COORD_2D cartilemap_coord,
							GAME_FEATURE_NODE_ptr* cartilemap_node_array, u16 cartilemap_node_array_sz)
{
	// reset the array
	init_cartilemap_node_array(cartilemap_node_array, cartilemap_node_array_sz);
	// compute results
	s32 iter=0, node_iter=0;
	for(iter=0; iter<flag_array_size_width; iter++)
	{
		if(flag_array[feature_type_idx*flag_array_size_width+ iter]!=0xffff)
		{
			// exists an indepedent feature
			// cartilemap node <> car tile (= graphical tile to create a cartilemap),
			// but the function `create_node` can be reused for cartilemap node,
			// as long as the GAME_FEATURES argument <> any existent defined GAME_FEATURES in `tile_vram_description`,
			// and the tid = car cat id
			if (node_iter< cartilemap_node_array_sz)
			{
				cartilemap_node_array[node_iter]=create_node(cartilemap_coord.x, cartilemap_coord.y, cartilemap_id, NA_FEATURE, NA_DIR);
				switch(flag_array[feature_type_idx*flag_array_size_width+ iter])
				{
					// 	   TRBL
					case 0x0111: // 273
						cartilemap_node_array[node_iter]->child_r_lk=&end_node;
						cartilemap_node_array[node_iter]->child_bot_lk=&end_node;
						cartilemap_node_array[node_iter]->child_l_lk=&end_node;
						break;
					case 0x1101: // 4353
						cartilemap_node_array[node_iter]->child_top_lk=&end_node;
						cartilemap_node_array[node_iter]->child_r_lk=&end_node;
						cartilemap_node_array[node_iter]->child_l_lk=&end_node;
						break;
					case 0x1110: // 4368
						cartilemap_node_array[node_iter]->child_top_lk=&end_node;
						cartilemap_node_array[node_iter]->child_r_lk=&end_node;
						cartilemap_node_array[node_iter]->child_bot_lk=&end_node;
						break;	
					case 0x1011: // 4113
						cartilemap_node_array[node_iter]->child_top_lk=&end_node;
						cartilemap_node_array[node_iter]->child_bot_lk=&end_node;
						cartilemap_node_array[node_iter]->child_l_lk=&end_node;
						break;
					case 0x0101: // 257
						cartilemap_node_array[node_iter]->child_r_lk=&end_node;
						cartilemap_node_array[node_iter]->child_l_lk=&end_node;
						break;
					case 0x1010: // 4112
						cartilemap_node_array[node_iter]->child_top_lk=&end_node;
						cartilemap_node_array[node_iter]->child_bot_lk=&end_node;
						break;
					case 0x0010: // 16
						cartilemap_node_array[node_iter]->child_bot_lk=&end_node;
						break;
					case 0x0001: // 1
						cartilemap_node_array[node_iter]->child_l_lk=&end_node;
						break;
					case 0x1000: // 4096
						cartilemap_node_array[node_iter]->child_top_lk=&end_node;
						break;
					case 0x0100: // 256
						cartilemap_node_array[node_iter]->child_r_lk=&end_node;
						break;
					case 0x0011: // 17
						cartilemap_node_array[node_iter]->child_bot_lk=&end_node;
						cartilemap_node_array[node_iter]->child_l_lk=&end_node;
						break;
					case 0x1001: // 4097
						cartilemap_node_array[node_iter]->child_top_lk=&end_node;
						cartilemap_node_array[node_iter]->child_l_lk=&end_node;
						break;
					case 0x1100: // 4352
						cartilemap_node_array[node_iter]->child_top_lk=&end_node;
						cartilemap_node_array[node_iter]->child_r_lk=&end_node;
						break;
					case 0x0110: // 272
						cartilemap_node_array[node_iter]->child_r_lk=&end_node;
						cartilemap_node_array[node_iter]->child_bot_lk=&end_node;
						break;
					default:
						// nothing
				}
				node_iter= node_iter+1;
			}
			
		}
	}
}
void report_num_game_features (GAME_FEATURE_NODE_START* game_city_array, u16 game_city_array_sz, 
							u32* all_cities, u32* finished_cities)
{
	s32 iter=0;
	bool status=false;
	*all_cities = 0;
	// *finished_cities=0;
	for(iter=0; iter<game_city_array_sz; iter++)
	{
		if(game_city_array[iter].root!=NULL)
		{
			*all_cities= *all_cities+1;
			// bool feature_complete_check (GAME_FEATURE_NODE_ptr feature_root);
			status = feature_complete_check (game_city_array[iter].root);
			if (status == true)
			{
				*finished_cities=*finished_cities+1;
				// delete the complete game cities
				// GAME_FEATURE_NODE_ptr delete_whole_feature (GAME_FEATURE_NODE_ptr feature_root);
				game_city_array[iter].root=delete_whole_feature(game_city_array[iter].root);
				*all_cities= *all_cities-1;
				
			}
		}
	}

}
bool is_only_city_tile (unsigned short tile_vram_id)
{
	return (
		tile_vram_id == 14 // only city, all open
		|| tile_vram_id == 23
	);
}
bool is_city_and_field_tile (unsigned short tile_vram_id)
{
	return (
		tile_vram_id == 15 // city with field
		|| tile_vram_id == 16 // //
		|| tile_vram_id == 17 // //
		|| tile_vram_id == 18 // //
		|| tile_vram_id == 19 // //
		|| tile_vram_id == 20 // //
		|| tile_vram_id == 21 // //
		|| tile_vram_id == 22 // //
		|| tile_vram_id == 24 // //
		|| tile_vram_id == 25 // //
	);
}
bool valid_adj_for_field_node (unsigned short adj_tile_vram_id)
{
	return (
		tile_vram_description[adj_tile_vram_id] == FIELD
		|| adj_tile_vram_id == 15 // city with field
		|| adj_tile_vram_id == 16 // //
		|| adj_tile_vram_id == 17 // //
		|| adj_tile_vram_id == 18 // //
		|| adj_tile_vram_id == 19 // //
		|| adj_tile_vram_id == 20 // //
		|| adj_tile_vram_id == 21 // //
		|| adj_tile_vram_id == 22 // //
		|| adj_tile_vram_id == 24 // //
		|| adj_tile_vram_id == 25 // //
		|| tile_vram_description[adj_tile_vram_id] == GARDEN
		|| tile_vram_description[adj_tile_vram_id] == CHURCH
	);
}
void track_fields_game (GAME_FEATURE_NODE_START* field_feature_array, u16 field_feature_array_sz,
							unsigned short *cur_car_tile_map_asm_id, COORD_2D curr_tile_coord_wo_wrap)
{
	// RULE:
	// 1. CITY tile WITH FIELD, IS a FIELD tile node 
	// with OPEN DIR where FIELD is, END DIR where CITY is 
	// 1.1 CITY ONLY tile (NO FIELD), number of sides ADJ FIELDS/ STREET = newly created FIELD node,
	// with OPEN DIR where the tile ADJ FIELD/CITY WITH FIELD, END FIR where tile ADJ CITY/STREET.
	// 2. STREET type tile is NOT FIELD tile. = end condition for field structure.
	// 3. GARDEN, CHURCH are FIELD type tile as well.
	// 4. 1 cartilemap = 4 field node 
	// => Coordinate of field node.x = Cartilemap coordinate.x /2
	// && Coordinate of field node.y = Cartilemap coordinate.y /2 
	// 5. Merge rule: only field feature structure adj to the newly created nodes -> related to the merging check !
	u32 field_for_merg_chk_sz=4;
	GAME_FEATURE_NODE_START field_for_merg_chk[field_for_merg_chk_sz];
	field_for_merg_chk[0].root=NULL;
	field_for_merg_chk[1].root=NULL;
	field_for_merg_chk[2].root=NULL;
	field_for_merg_chk[3].root=NULL;
	u32 idx_field_for_merg_chk_vs_idx_field_feature_arr[field_feature_array_sz];
	s32 idx_ftr_arr_for_merg_chk[field_for_merg_chk_sz];


	GAME_FEATURES tile_type;
	u32 number_new_nodes =4;
	GAME_FEATURE_NODE_ptr new_nodes[number_new_nodes];
	new_nodes[0] = NULL;
	new_nodes[1] = NULL;
	new_nodes[2] = NULL;
	new_nodes[3] = NULL;

	int iter_col, iter_r, node_idx=0;

	for (iter_r = 0; iter_r < 3; iter_r++)
	{
		for (iter_col=0; iter_col<3; iter_col++)
		{
			unsigned short  tile_vram_id = cur_car_tile_map_asm_id[iter_r*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
			tile_type = tile_vram_description[tile_vram_id];
			COORD_2D car_coord;
			map_tile_to_ctile(curr_tile_coord_wo_wrap.x, curr_tile_coord_wo_wrap.y, &car_coord.x, &car_coord.y);
			COORD_2D node_coord;
			if (iter_r==0 && iter_col==0) // tl tile
			{
				node_coord.x=car_coord.x*2;
				node_coord.y=car_coord.y*2;
			}
			else if (iter_r==0 && iter_col==2) // tr tile
			{
				node_coord.x=car_coord.x*2+1;
				node_coord.y=car_coord.y*2;
			}
			else if (iter_r==2 && iter_col==0) // bl tile
			{
				node_coord.x=car_coord.x*2;
				node_coord.y=car_coord.y*2+1;
			}
			else if (iter_r==2 && iter_col==2) // br tile
			{
				node_coord.x=car_coord.x*2+1;
				node_coord.y=car_coord.y*2+1;
			}
			
				// middle tile of cartilemap for checking later
			unsigned short mid_tile = cur_car_tile_map_asm_id[1*3 + 1]+CAR_TILE_OFFSET_IN_VRAM;

			// ===
			// create nodes
			
			// only create/check node only at the corner
			if ((iter_r==0 && iter_col==0)
				|| (iter_r==0 && iter_col==2)
				|| (iter_r==2 && iter_col==0)
				|| (iter_r==2 && iter_col==2))
			{
				if (node_idx<number_new_nodes-1
					&& new_nodes[node_idx]!=NULL)
				{
					node_idx=node_idx+1;	
				}

				switch (tile_vram_id)
				{
					case 26: // GARDEN == FIELD
					case 27: // CHURCH == FIELD
					case 2: // FIELD, the only field type tile.
						new_nodes[node_idx]= create_node(node_coord.x, node_coord.y, tile_vram_id, NA_FEATURE, NA_DIR);
						
						// checking on the top side
						if ((iter_r == 2 && iter_col==0) 
							|| (iter_r == 2 && iter_col==1)
							|| (iter_r == 2 && iter_col==2)
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 1 && iter_col==2)
							)
						{
							unsigned short top_tile = cur_car_tile_map_asm_id[(iter_r-1)*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
							if (!valid_adj_for_field_node(top_tile))
							{
								new_nodes[node_idx]->child_top_lk= &end_node;
							}
						}

						// checking on the right side
						if ((iter_r == 0 && iter_col==0) 
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 2 && iter_col==0)
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 2 && iter_col==1)
							|| (iter_r == 1 && iter_col==1)
							)
						{
							unsigned short r_tile = cur_car_tile_map_asm_id[iter_r*3 + (iter_col+1)]+CAR_TILE_OFFSET_IN_VRAM;
							if (!valid_adj_for_field_node(r_tile))
							{
								new_nodes[node_idx]->child_r_lk= &end_node;
							}
						}

						// checking on the bot side
						if ((iter_r == 0 && iter_col==0) 
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 0 && iter_col==2)
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 1 && iter_col==2)
							)
						{
							unsigned short bot_tile = cur_car_tile_map_asm_id[(iter_r+1)*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
							if (!valid_adj_for_field_node(bot_tile))
							{
								new_nodes[node_idx]->child_bot_lk= &end_node;
							}
						}

						// checking on the left side
						if ((iter_r == 0 && iter_col==2) 
							|| (iter_r == 1 && iter_col==2)
							|| (iter_r == 2 && iter_col==2)
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 2 && iter_col==1)
							)
						{
							unsigned short l_tile = cur_car_tile_map_asm_id[iter_r*3 + iter_col-1]+CAR_TILE_OFFSET_IN_VRAM;
							if (!valid_adj_for_field_node(l_tile))
							{
								new_nodes[node_idx]->child_l_lk= &end_node;
							}
						}
						break;
					case 23: // ONLY CITY (all open)
					case 14: // ONLY CITY (all open)
						// checking the middle tile of the cartilemap
						// unsigned short mid_tile = cur_car_tile_map_asm_id[1*3 + 1]+CAR_TILE_OFFSET_IN_VRAM;
						if (tile_vram_description[mid_tile]!=CITY)
						{
							// According to carcassonne tile map rule,
							// 1. the position of this tile should either be @
							// cartilemap corners or
							// cartile map edge middle.
							// 2. this city tile is ~ field node/tile
							new_nodes[node_idx]= create_node(node_coord.x, node_coord.y, tile_vram_id, NA_FEATURE, NA_DIR);
							new_nodes[node_idx]->child_top_lk=&end_node;
							new_nodes[node_idx]->child_r_lk=&end_node;
							new_nodes[node_idx]->child_bot_lk=&end_node;
							new_nodes[node_idx]->child_l_lk=&end_node;

							// check open/end dir condition 
							unsigned short same_row_right_tile = cur_car_tile_map_asm_id[iter_r*3 + 2]+CAR_TILE_OFFSET_IN_VRAM;
							unsigned short same_row_left_tile = cur_car_tile_map_asm_id[iter_r*3 + 0]+CAR_TILE_OFFSET_IN_VRAM;
							unsigned short same_col_top_tile = cur_car_tile_map_asm_id[0*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
							unsigned short same_col_bot_tile = cur_car_tile_map_asm_id[2*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
								// tile position @ cartilemap corner
							if (iter_r==0 && iter_col==0)
							{
								// tl corner
								if (is_city_and_field_tile(same_row_right_tile))
								{
									new_nodes[node_idx]->child_r_lk=NULL;
								}
								if (is_city_and_field_tile(same_col_bot_tile))
								{
									new_nodes[node_idx]->child_bot_lk=NULL;
								}
							}
							if (iter_r==0 && iter_col==2)
							{
								// tr corner
								if (is_city_and_field_tile(same_row_left_tile))
								{
									new_nodes[node_idx]->child_l_lk=NULL;
								}
								if (is_city_and_field_tile(same_col_bot_tile))
								{
									new_nodes[node_idx]->child_bot_lk=NULL;
								}
							}
							if (iter_r==2 && iter_col==2)
							{
								// br corner
								if (is_city_and_field_tile(same_row_left_tile))
								{
									new_nodes[node_idx]->child_l_lk=NULL;
								}
								if (is_city_and_field_tile(same_col_top_tile))
								{
									new_nodes[node_idx]->child_top_lk=NULL;
								}
							}
							if (iter_r==2 && iter_col==0)
							{
								// bl corner
								if (is_city_and_field_tile(same_row_right_tile))
								{
									new_nodes[node_idx]->child_r_lk=NULL;
								}
								if (is_city_and_field_tile(same_col_top_tile))
								{
									new_nodes[node_idx]->child_top_lk=NULL;
								}
							}
							if((iter_r==0 && iter_col==1)
								|| (iter_r==2 && iter_col==1)
								)
							{
								// middle t/b edge
								if (is_city_and_field_tile(same_row_right_tile)
									|| is_city_and_field_tile(same_row_left_tile)
									)
								{
									new_nodes[node_idx]->child_r_lk=NULL;
									new_nodes[node_idx]->child_l_lk=NULL;
								}
							}
							if((iter_r==1 && iter_col==0)
								|| (iter_r==1 && iter_col==2)
								)
							{
								// middle l/r edge
								if (is_city_and_field_tile(same_col_top_tile)
									|| is_city_and_field_tile(same_col_bot_tile)
									)
								{
									new_nodes[node_idx]->child_top_lk=NULL;
									new_nodes[node_idx]->child_bot_lk=NULL;
								}
							}	
						}
						break;
					case 15: // city+filed, field open r&bot
							// According to carcassonne graphic tile rule,
							// this type of tile ONLY @ cartilemap corners
					case 19:
						new_nodes[node_idx]= create_node(node_coord.x, node_coord.y, tile_vram_id, NA_FEATURE, NA_DIR);
						new_nodes[node_idx]->child_top_lk=&end_node;
						new_nodes[node_idx]->child_l_lk=&end_node;
						// check adj tile along the field open dir
							// right dir
						if ((iter_r == 0 && iter_col==0) 
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 2 && iter_col==0)
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 2 && iter_col==1)
							|| (iter_r == 1 && iter_col==1)
							)
						{
							unsigned short r_tile = cur_car_tile_map_asm_id[iter_r*3 + (iter_col+1)]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[r_tile]==STREET)
							{
								new_nodes[node_idx]->child_r_lk= &end_node;
							}
						}
							// the bot dir
						if ((iter_r == 0 && iter_col==0) 
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 0 && iter_col==2)
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 1 && iter_col==2)
							)
						{
							unsigned short bot_tile = cur_car_tile_map_asm_id[(iter_r+1)*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[bot_tile]==STREET)
							{
								new_nodes[node_idx]->child_bot_lk= &end_node;
							}
						}

						// check the middle tile of cartilemap,
						// if the middle is street/field/garden/church,
						// one/both of the city side of this tile will be open for field connection
						if ( 
							tile_vram_description[mid_tile]== STREET
							|| tile_vram_description[mid_tile]== GARDEN
							|| tile_vram_description[mid_tile]== CHURCH
							|| tile_vram_description[mid_tile]== FIELD
							)
						{
							// city side @ t/l side
							if (iter_r==0 && iter_col==0)
							{
								// position tl corner
								// all city side must stay as `end_node`
							}
							if (iter_r==0 && iter_col==2)
							{
								// position tr corner
								new_nodes[node_idx]->child_l_lk=NULL;
							}
							if (iter_r==2 && iter_col==0)
							{
								// pos bl corner
								new_nodes[node_idx]->child_top_lk=NULL;
							}
							if (iter_r==2 && iter_col==2)
							{
								// pos br
								// ? This not happends according to carcassonne graphic tile rule
								new_nodes[node_idx]->child_top_lk=NULL;
								new_nodes[node_idx]->child_l_lk=NULL;
							}
						}
						break;
					case 16: // city+filed, field open bot&l
							// According to carcassonne graphic tile rule,
							// this type of tile ONLY @ cartilemap corners
					case 20:
						new_nodes[node_idx]= create_node(node_coord.x, node_coord.y, tile_vram_id, NA_FEATURE, NA_DIR);
						new_nodes[node_idx]->child_top_lk=&end_node;
						new_nodes[node_idx]->child_r_lk=&end_node;
						// check adj tile along the field open dir
							// checking on the left side
						if ((iter_r == 0 && iter_col==2) 
							|| (iter_r == 1 && iter_col==2)
							|| (iter_r == 2 && iter_col==2)
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 2 && iter_col==1)
							)
						{
							unsigned short l_tile = cur_car_tile_map_asm_id[iter_r*3 + iter_col-1]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[l_tile]==STREET)
							{
								new_nodes[node_idx]->child_l_lk= &end_node;
							}
						}
							// the bot dir
						if ((iter_r == 0 && iter_col==0) 
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 0 && iter_col==2)
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 1 && iter_col==2)
							)
						{
							unsigned short bot_tile = cur_car_tile_map_asm_id[(iter_r+1)*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[bot_tile]==STREET)
							{
								new_nodes[node_idx]->child_bot_lk= &end_node;
							}
						}

						// check the middle tile of cartilemap,
						// if the middle is street/field/garden/church,
						// one/both of the city side of this tile will be open for field connection
						if (tile_vram_description[mid_tile]== STREET
							|| tile_vram_description[mid_tile]== GARDEN
							|| tile_vram_description[mid_tile]== CHURCH
							|| tile_vram_description[mid_tile]== FIELD
							)
						{
							// city side @ t/r side
							if (iter_r==0 && iter_col==0)
							{
								// position tl corner
								new_nodes[node_idx]->child_r_lk=NULL;
							}
							if (iter_r==0 && iter_col==2)
							{
								// position tr corner
								// nothing
							}
							if (iter_r==2 && iter_col==0)
							{
								// pos bl corner
								// ? This not happends according to carcassonne graphic tile rule
								new_nodes[node_idx]->child_top_lk=NULL;
								new_nodes[node_idx]->child_r_lk=NULL;
							}
							if (iter_r==2 && iter_col==2)
							{
								// pos br
								new_nodes[node_idx]->child_top_lk=NULL;
							}
						}
						break;
					case 17: // city+filed, field open t&l
							// According to carcassonne graphic tile rule,
							// this type of tile ONLY @ cartilemap corners
					case 21:
						new_nodes[node_idx]= create_node(node_coord.x, node_coord.y, tile_vram_id, NA_FEATURE, NA_DIR);
						new_nodes[node_idx]->child_r_lk=&end_node;
						new_nodes[node_idx]->child_bot_lk=&end_node;
						// check adj tile along the field open dir
							// checking on the left side
						if ((iter_r == 0 && iter_col==2) 
							|| (iter_r == 1 && iter_col==2)
							|| (iter_r == 2 && iter_col==2)
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 2 && iter_col==1)
							)
						{
							unsigned short l_tile = cur_car_tile_map_asm_id[iter_r*3 + iter_col-1]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[l_tile]==STREET)
							{
								new_nodes[node_idx]->child_l_lk= &end_node;
							}
						}
						// checking on the top side
						if ((iter_r == 2 && iter_col==0) 
							|| (iter_r == 2 && iter_col==1)
							|| (iter_r == 2 && iter_col==2)
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 1 && iter_col==2)
							)
						{
							unsigned short top_tile = cur_car_tile_map_asm_id[(iter_r-1)*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[top_tile]==STREET)
							{
								new_nodes[node_idx]->child_top_lk= &end_node;
							}
						}

						// check the middle tile of cartilemap,
						// if the middle is street/field/garden/church,
						// one/both of the city side of this tile will be open for field connection
						if (tile_vram_description[mid_tile]== STREET
							|| tile_vram_description[mid_tile]== GARDEN
							|| tile_vram_description[mid_tile]== CHURCH
							|| tile_vram_description[mid_tile]== FIELD
							)
						{
							// city side @ r&b side
							if (iter_r==0 && iter_col==0)
							{
								// position tl corner
								// ? This not happends according to carcassonne graphic tile rule
								new_nodes[node_idx]->child_r_lk=NULL;
								new_nodes[node_idx]->child_bot_lk=NULL;
							}
							if (iter_r==0 && iter_col==2)
							{
								// position tr corner
								new_nodes[node_idx]->child_bot_lk=NULL;
							}
							if (iter_r==2 && iter_col==0)
							{
								// pos bl corner
								new_nodes[node_idx]->child_r_lk=NULL;
							}
							if (iter_r==2 && iter_col==2)
							{
								// pos br
								// nothing
							}
						}
						break;
					case 18: // city+filed, field open t&r
							// According to carcassonne graphic tile rule,
							// this type of tile ONLY @ cartilemap corners
					case 22:
						new_nodes[node_idx]= create_node(node_coord.x, node_coord.y, tile_vram_id, NA_FEATURE, NA_DIR);
						new_nodes[node_idx]->child_l_lk=&end_node;
						new_nodes[node_idx]->child_bot_lk=&end_node;
						// check adj tile along the field open dir
							// right dir
						if ((iter_r == 0 && iter_col==0) 
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 2 && iter_col==0)
							|| (iter_r == 0 && iter_col==1)
							|| (iter_r == 2 && iter_col==1)
							|| (iter_r == 1 && iter_col==1)
							)
						{
							unsigned short r_tile = cur_car_tile_map_asm_id[iter_r*3 + (iter_col+1)]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[r_tile]==STREET)
							{
								new_nodes[node_idx]->child_r_lk= &end_node;
							}
						}
						// checking on the top side
						if ((iter_r == 2 && iter_col==0) 
							|| (iter_r == 2 && iter_col==1)
							|| (iter_r == 2 && iter_col==2)
							|| (iter_r == 1 && iter_col==0)
							|| (iter_r == 1 && iter_col==1)
							|| (iter_r == 1 && iter_col==2)
							)
						{
							unsigned short top_tile = cur_car_tile_map_asm_id[(iter_r-1)*3 + iter_col]+CAR_TILE_OFFSET_IN_VRAM;
							if (tile_vram_description[top_tile]==STREET)
							{
								new_nodes[node_idx]->child_top_lk= &end_node;
							}
						}

						// check the middle tile of cartilemap,
						// if the middle is street/field/garden/church,
						// one/both of the city side of this tile will be open for field connection
						if (tile_vram_description[mid_tile]== STREET
							|| tile_vram_description[mid_tile]== GARDEN
							|| tile_vram_description[mid_tile]== CHURCH
							|| tile_vram_description[mid_tile]== FIELD
							)
						{
							// city side @ b&l side
							if (iter_r==0 && iter_col==0)
							{
								// position tl corner
								new_nodes[node_idx]->child_bot_lk=NULL;
							}
							if (iter_r==0 && iter_col==2)
							{
								// position tr corner
								// ? This not happends according to carcassonne graphic tile rule
								new_nodes[node_idx]->child_bot_lk=NULL;
								new_nodes[node_idx]->child_l_lk=NULL;
							}
							if (iter_r==2 && iter_col==0)
							{
								// pos bl corner
								// nothing	
							}
							if (iter_r==2 && iter_col==2)
							{
								// pos br
								new_nodes[node_idx]->child_l_lk=NULL;
							}
						}
						break;
					case 24: // a strand of field in the middle
							// this tile always @ cartilemap corner positions.
					case 25:
						new_nodes[node_idx]= create_node(node_coord.x, node_coord.y, tile_vram_id, NA_FEATURE, NA_DIR);
						new_nodes[node_idx]->child_top_lk=&end_node;
						new_nodes[node_idx]->child_r_lk=&end_node;
						new_nodes[node_idx]->child_bot_lk=&end_node;
						new_nodes[node_idx]->child_l_lk=&end_node;
						
						// Base on the position, the city side will be opened 
						// to connect to the field
						if (iter_r==0 && iter_col==0)
						{
							// position tl corner
							// ? according to carcassonne graphic tile rule: 
							//  only happens for case 25
							new_nodes[node_idx]->child_bot_lk=NULL;
							new_nodes[node_idx]->child_r_lk=NULL;
						}
						if (iter_r==0 && iter_col==2)
						{
							// position tr corner
							// ? according to carcassonne graphic tile rule: 
							//  only happens for case 24
							new_nodes[node_idx]->child_bot_lk=NULL;
							new_nodes[node_idx]->child_l_lk=NULL;
						}
						if (iter_r==2 && iter_col==0)
						{
							// pos bl corner
							// ? according to carcassonne graphic tile rule: 
							//  only happens for case 24
							new_nodes[node_idx]->child_top_lk=NULL;
							new_nodes[node_idx]->child_r_lk=NULL;
						}
						if (iter_r==2 && iter_col==2)
						{
							// pos br
							// ? according to carcassonne graphic tile rule: 
							//  only happens for case 25
							new_nodes[node_idx]->child_top_lk=NULL;
							new_nodes[node_idx]->child_l_lk=NULL;
						}
						break;
					default:
						// do nothing = no new node is created

				}
			}			
		}
	}
	// insert node, merge feature, delete the merged feature except the ref feature where everything is merged into.
		// insert,
	GAME_FEATURE_NODE_ptr copied_nodes[number_new_nodes];
	for (iter_r=0; iter_r<number_new_nodes; iter_r++)
	{
		copied_nodes[iter_r]= new_nodes[iter_r];
	}
		// after node inserted, it is deleted to NULL
	insert_nodes_into_existent_ftrs(field_feature_array, field_feature_array_sz,
							new_nodes, number_new_nodes);

	u32 iter_new_node=0, iter_ftr_arr=0, iter_meg_chk_arr=0;
		// init the flag arr
	for (iter_ftr_arr=0; iter_ftr_arr<field_feature_array_sz; iter_ftr_arr++)
	{
		idx_field_for_merg_chk_vs_idx_field_feature_arr[iter_ftr_arr]=0;
	}

	for (iter_ftr_arr=0; iter_ftr_arr<field_feature_array_sz; iter_ftr_arr++)
	{
		for (iter_new_node=0; iter_new_node<number_new_nodes; iter_new_node++)
		{
			DIRECTION find_dir;
			if(find_node(field_feature_array[iter_ftr_arr].root, copied_nodes[iter_new_node], &find_dir)!=NULL
				|| node_exist(field_feature_array[iter_ftr_arr].root, copied_nodes[iter_new_node]))
			{
				// field_for_merg_chk[iter_meg_chk_arr].root= field_feature_array[iter_ftr_arr].root;
				idx_field_for_merg_chk_vs_idx_field_feature_arr[iter_ftr_arr]=1;
			}
		}
		
	}
		//merge, delete merged features
	check_all_merge_possibilities_for_fields(field_feature_array, field_feature_array_sz, idx_field_for_merg_chk_vs_idx_field_feature_arr);

}
// === 
// 3. POINT COUNT 
// ===

void init_reg_bg1 ()
{

	// == initialize regular background 1
	REG_BG1CNT= BG_CBB(CBB_0_BG1) | BG_SBB(SBB_0_BG1) | BG_REG_64x64 | BG_WRAP;
	// set the initial position of the screen same as bg2-aff
	REG_BG_OFS[1]=bg1_pt;

	// == initialize regular background 1, 8bpp. Because it will share graphic data with affine background, affine bg is always 8bpp.
	REG_BG1CNT= BG_CBB(CBB_0_BG1) | BG_SBB(SBB_0_BG1) | BG_REG_64x64 | BG_8BPP;
											  // BG size: 32x32 DTILE
	// The first tile is an empty tile. Because when inited, the bg uses its first tile to render the whole background as default
	// share palette with bg2-aff
	memcpy32(&tile8_mem[CBB_0_BG1][0], tiles_bg1Tiles, tiles_bg1TilesLen/sizeof(u32));
}

// === 
// 0. MAIN GAME LOOP
// ===
void game_loop()
{
	// === 
	// POINT COUNT 
	// ===
	// === reg, bg 1
	BG_POINT prev_bg1_pt;
	AFF_SRC_EX asx_prev;
	BG_POINT bg1_pt_prev;
	
	// === 
	// FEATURE REPORT 
	// 3. variables/flags to report found features 
	// IMPORTANT: In carcassonne, maximum 4 indepdent features of the same type (city/ street/ field) can exist.
	// ===
	u16 city_idx = 0, str_idx= 1;
	u16 amount_type_features_percatile_sz=2;
	u16 amount_type_features_percatile[amount_type_features_percatile_sz];
	amount_type_features_percatile[city_idx]=0;
	amount_type_features_percatile[str_idx]=0;
		// amount_type_features_percatile[0]: CITY
		// amount_type_features_percatile[1]: STREET
	u16 feature_end_open_flgs_width = 10;
	u16 feature_flgs_size = 20, prev_feature_flgs_size=8;
	u16 feature_end_open_flgs[feature_flgs_size], prev_feature_end_open_flgs[prev_feature_flgs_size];
		// 0xffff = no info
		// 0xTRBL; 1=end, 0=open
		// for example, 0x0000 = all sides are open; 0x1010= only T and B are open
		// feature_end_open_flgs is 2D array, index as row*width+col
		// feature_end_open_flgs[0][10] = feature_end_open_flgs[0*feature_end_open_flgs_width + i] (i=0..9) : CITY
		// feature_end_open_flgs[1][10] = feature_end_open_flgs[0*feature_end_open_flgs_width + i] (i=0..9) : STREET
		// feature_flgs_size = number feature type (i.e CITY, STREET) * feature_end_open_flgs_width
	init_feature_flgs(feature_end_open_flgs, feature_flgs_size);
	init_feature_flgs(prev_feature_end_open_flgs, prev_feature_flgs_size);

	// tracking features in game
		// CITY
	u16 track_game_cities_sz= 20;
	GAME_FEATURE_NODE_START track_game_cities[track_game_cities_sz];
	init_features_per_tilemap(track_game_cities, track_game_cities_sz);
	u16 track_game_city_nodes_sz= 10;
	GAME_FEATURE_NODE_ptr track_game_city_nodes[track_game_city_nodes_sz];
	init_cartilemap_node_array(track_game_city_nodes,track_game_city_nodes_sz);
	u32 num_game_cities=0, prev_num_game_cities=0;
	u32 num_game_fcities=0, prev_num_game_fcities=0;

		// STREET
	u16 track_game_strs_sz= 30;
	GAME_FEATURE_NODE_START track_game_strs[track_game_strs_sz];
	init_features_per_tilemap(track_game_strs, track_game_strs_sz);
	u16 track_game_str_nodes_sz= 10;
	GAME_FEATURE_NODE_ptr track_game_str_nodes[track_game_str_nodes_sz];
	init_cartilemap_node_array(track_game_str_nodes,track_game_str_nodes_sz);
	u32 num_game_strs=0, prev_num_game_strs=0;
	u32 num_game_fstrs=0, prev_num_game_fstrs=0;

		// FIELD
	u16 track_game_field_sz= 40;
	GAME_FEATURE_NODE_START track_game_fds[track_game_field_sz];
	init_features_per_tilemap(track_game_fds, track_game_field_sz);
	u32 num_game_fds=0, prev_num_game_fds=0;
	u32 num_game_ffds=0, prev_num_game_ffds=0;

	// === Carcassonne data
	int car_cat_track[32] = {0};
	int carcassonne_number_of_tiles = 0; // assume 72 carcasonne tiles
	// (virtual/conceptual) carcassonne map
	// graphical stored tiles in VRAM.
	CAR_MAP_INFO carcassonne_full_map[CAR_TILES_MAX];
	init_map_info (carcassonne_full_map);
		// carcassonne tracking position
	COORD_2D prev_ctile_coord;


	// === aff bg
	AFF_SRC_EX asx=
	{
		INIT_BG_X_OFF<<8, INIT_BG_Y_OFF<<8,			// Map coords.
		0, 0,				// Screen coords.
		0x0100, 0x0100, 0		// Scales (= x1) and angle (= 0 degree).
	};

	// ==== obj
	// set the inital position of the cursor sprite
	int x= INIT_OBJ_X, y= INIT_OBJ_Y; // [pixel] 

	u32 tid= 0, pb= 0;		// tile id, pal-bank

	OBJ_ATTR *cursor= &obj_buffer[0];
	obj_set_attr(cursor, 
		ATTR0_SQUARE | ATTR0_8BPP,				// Square, regular sprite
												// & 8bpp
		ATTR1_SIZE_32,							// 32x32p, = 4x4 Tile
		// the object priority is zero
		ATTR2_ID(tid));				// tile 0 = base tile of the sprite
									// tile 0 = &tile_mem[4][0]

	// set the starting position of the obj
	// x,y [pixel] = position of the top-left pixel of the obj
	obj_set_pos(cursor, x, y);

	// === enable a timer
	REG_TM2D =  -0x0667; // 100 ms = -0x0667, 1s = -0x4009
	REG_TM2CNT= TM_ENABLE | TM_FREQ_1024; 
	// 1 period = 1024 default clock cycle
	// = 1024 * (1/16.78 Mhz) = 61 us 
	// timer 2 overflows -> re-start at the value set by REG_TM2D. in cascade, it enables the timer 3 to tick.
	// timer overflows at 0xFFFF.
	// set REG_TM2D = -n = starting value of timer 2 @ 0x10000 -n (two complement)

	// tick every ~ (n (in decimal) * 61us * 1000) ms
	REG_TM3CNT=  TM_ENABLE | TM_CASCADE; 
	
	u32 sec = -1;

	// ==== render bg
	COORD_2D sae_prev;

	while(1)
	{
		VBlankIntrWait(); // check at the end of each frame
		key_poll();

		//====
		// POINT COUNT
		//====
		// === BG 1, info
		SCR_ENTRY *pse_1= bg1_map;

		// === obj
		u16 obj_x_coord, obj_y_coord; // in unit [pixel]
		int obj_x_min=INIT_OBJ_X, obj_x_max=INIT_OBJ_X + 2*8, // 2 tiles * 8 pixel per tile 
			obj_y_min=INIT_OBJ_Y, obj_y_max=INIT_OBJ_Y + 2*8;

		// === BG 2, aff
		SCR_ENTRY *pse= bg2_map;
		s32 se_curr;
		s32 sae_curr;
		// int scr_x_offset = 0, scr_y_offset =0;
		int bg_x_offset = 0, bg_y_offset =0;

		// === FUNCS/ ACTIONS
		// get the cursor (object) position (using Tonc BF_GET())
		obj_x_coord = BFN_GET(cursor->attr1, ATTR1_X);
		obj_y_coord = BFN_GET(cursor->attr0, ATTR0_Y);
		// calculate the Se_index, map size 32x32t
		// >> 3: divided by 8 to convert to unit [tile]
		//							 * 32 = width of the map size in unit [tile]	
		s32 sae_curr_x = (obj_x_coord + (bgaff.dx >> 8)) >>3;
		s32 sae_curr_y = (obj_y_coord+ (bgaff.dy >> 8)) >> 3;


		if (REG_TM3D != sec)
		{
			sec = REG_TM3D;

			sae_prev.x = sae_curr_x;
			sae_prev.y = sae_curr_y;

			if (current_game_state == MEEPLE_DECISION)
			{
				// == OBJ, moving relative to the screen = screen is static
				// left/right
				x += 8*key_tri_horz(); // [pixel]
				// move up/down
				y += 8*key_tri_vert(); // [pixel]
				if (x > obj_x_max)
				{
					x = obj_x_max;
				}
				if (x < obj_x_min)
				{
					x = obj_x_min;
				}
				if (y > obj_y_max)
				{
					y = obj_y_max;
				}
				if (y < obj_y_min)
				{
					y = obj_y_min;
				}
				obj_set_pos(cursor, x, y);

				// BG MUST BE STATIC
			}

			
			if (current_game_state == PUT_DOWN_TILE | current_game_state == END | current_game_state == MEEPLE_EXPLORE)
			{
				// === allow bg rolling, sprite static
				x += 0; // 24*key_tri_horz(); // [pixel]
				// move up/down
				y += 0; // 24*key_tri_vert(); // [pixel]
				obj_set_pos(cursor, x, y);
				
				// == BG rolling, i am moving the map, the screen/camera is static
				asx.tex_x += (24*key_tri_horz()) << 8;
				asx.tex_y += (24*key_tri_vert()) << 8;

				// == SCR rolling, i am moving the screen, the map is static
				// scr_x_offset = 24*key_tri_horz();  // [pixel]
				// scr_y_offset = 24*key_tri_vert(); 
				// asx.scr_x += (scr_x_offset); // converted to unit [pixel] for REG_BGX
				// asx.scr_y += (scr_y_offset); // converted to unit [pixel] for REG_BGY

				bg_rotscale_ex(&bgaff, &asx);
				REG_BG_AFFINE[2]= bgaff;

				// === BG 1, info rolling
				bg1_pt.x += (24*key_tri_horz());
				bg1_pt.y += (24*key_tri_vert());
				REG_BG_OFS[1]=bg1_pt;

			}

			// get the cursor (object) position (using Tonc BF_GET())
			obj_x_coord = BFN_GET(cursor->attr1, ATTR1_X);
			obj_y_coord = BFN_GET(cursor->attr0, ATTR0_Y);
			// calculate the Se_index
			// >> 3: divided by 8 to convert to unit [tile]
			//							 * 32 = width of the map size in unit [tile]	
			sae_curr_x = (obj_x_coord + (bgaff.dx >> 8)) >>3;
			sae_curr_y = (obj_y_coord+ (bgaff.dy >> 8)) >> 3;
			// sae_curr = sae_curr_y*map_width_unit_tile + sae_curr_x;
			// se_curr = sae_curr >> 1;
		}


		bool tst_mvflag, tst_updflg;
		COORD_2D tst_start_ct, tst_end_ct, tst_rd_tid;		
		render_bg_v2(sae_prev.x, sae_prev.y, sae_curr_x, sae_curr_y, carcassonne_full_map, pse, 
			&tst_mvflag, &tst_updflg, &tst_start_ct, &tst_end_ct, &tst_rd_tid);

		int cas_r, cas_col;
		int rand_cat, rand_cat_min, rand_cat_max, rand_cat_id;
		
		// game start
		if (current_game_state == START)
		{
			int se_idx = 0, sea_idx = 0;
			COORD_2D tile_wrap_coord;	
			// put down the starter tile
			for (cas_r = 0; cas_r < 3; cas_r++)
			{
				for (cas_col=0; cas_col<3; cas_col++)
				{
					tile_wrap_coord.x = sae_curr_x + cas_col;
					tile_wrap_coord.y = sae_curr_y + cas_r;
					wrapping_tile_coord(&tile_wrap_coord);
					sea_idx = tile_wrap_coord.y*map_width_unit_tile + tile_wrap_coord.x; 
					se_idx = sea_idx >> 1;
					if (sea_idx % 2 == 0)
					{
						// write to lower 8-bit of pse, preseve the higher 8-bit of pse
						//														0 = CARCASONNE CAT 0 = STARTER TILE
						pse[se_idx] = (pse[se_idx] & 0xFF00) | ((cas_tile_map_id[0][cas_r*3 + cas_col]+CAR_TILE_OFFSET_IN_VRAM) & 0x00FF);
					}
					else
					{
						// write higer 8-bit of pse, preserve the lower 8 bit of pse
						pse[se_idx] = (pse[se_idx] & 0x00FF) | (((cas_tile_map_id[0][cas_r*3 + cas_col]+CAR_TILE_OFFSET_IN_VRAM)<<8)  & 0xFF00);
					}
				}
			}

			// game state is allowed to change only when the tile is put down
			carcassonne_number_of_tiles = carcassonne_number_of_tiles +1;
			car_cat_track[0] = car_cat_track[0] + 1; 
			// save to the conceptual carmap
			COORD_2D car_coord;
			map_tile_to_ctile(sae_curr_x, sae_curr_y, &car_coord.x, &car_coord.y);
			carcassonne_full_map[carcassonne_number_of_tiles-1].car_tid = 0;
			carcassonne_full_map[carcassonne_number_of_tiles-1].car_map_coord = car_coord.y*CAR_MAP_WIDTH_x + car_coord.x;
			// current_game_state = GET_TILE;
			current_game_state = MEEPLE_START;

			prev_ctile_coord.x = car_coord.x;
			prev_ctile_coord.y = car_coord.y;
			
			// ===
			// TRACK GAME FEATURE
			// ===
				// CITY
			init_feature_flgs(feature_end_open_flgs, feature_flgs_size);
			feature_end_open_flgs[city_idx*feature_end_open_flgs_width+0]=0x0111;
			// void create_cartilemap_node (u16* flag_array, u16 flag_array_size_width, u16 feature_type_idx,
			// 				GAME_FEATURES cartilemap_id, COORD_2D cartilemap_coord,
			// 				GAME_FEATURE_NODE_ptr* cartilemap_node_array, u16 cartilemap_node_array_sz)
			create_cartilemap_node(feature_end_open_flgs, feature_end_open_flgs_width, city_idx,
									0, car_coord, 
									track_game_city_nodes, track_game_city_nodes_sz);
			// void insert_nodes_into_existent_ftrs (GAME_FEATURE_NODE_START* ftr_game_array, u16 ftr_game_array_sz, 
			// 							GAME_FEATURE_NODE_ptr* new_nodes_array, u16 new_node_array_sz)

			insert_nodes_into_existent_ftrs(track_game_cities, track_game_cities_sz,
											track_game_city_nodes, track_game_city_nodes_sz);
			// void check_all_merge_possibilities (GAME_FEATURE_NODE_START* ftr_game_array, u16 ftr_game_array_size)
			check_all_merge_possibilities(track_game_cities, track_game_cities_sz);
			// void report_num_game_features (GAME_FEATURE_NODE_START* game_city_array, u16 game_city_array_sz, u32* result)
			report_num_game_features(track_game_cities, track_game_cities_sz, 
								&num_game_cities, &num_game_fcities);

				// STREET
			feature_end_open_flgs[str_idx*feature_end_open_flgs_width+0]=0x1010;
			// void create_cartilemap_node (u16* flag_array, u16 flag_array_size_width, u16 feature_type_idx,
			// 				GAME_FEATURES cartilemap_id, COORD_2D cartilemap_coord,
			// 				GAME_FEATURE_NODE_ptr* cartilemap_node_array, u16 cartilemap_node_array_sz)
			create_cartilemap_node(feature_end_open_flgs, feature_end_open_flgs_width, str_idx,
									0, car_coord, 
									track_game_str_nodes, track_game_str_nodes_sz);
			// void insert_nodes_into_existent_ftrs (GAME_FEATURE_NODE_START* ftr_game_array, u16 ftr_game_array_sz, 
			// 							GAME_FEATURE_NODE_ptr* new_nodes_array, u16 new_node_array_sz)
			insert_nodes_into_existent_ftrs(track_game_strs, track_game_strs_sz,
											track_game_str_nodes, track_game_str_nodes_sz);
			// void check_all_merge_possibilities (GAME_FEATURE_NODE_START* ftr_game_array, u16 ftr_game_array_size)
			check_all_merge_possibilities(track_game_strs, track_game_strs_sz);
			// void report_num_game_features (GAME_FEATURE_NODE_START* game_city_array, u16 game_city_array_sz, u32* result)
			report_num_game_features(track_game_strs, track_game_strs_sz, 
								&num_game_strs, &num_game_fstrs);
			
				// FIELDs
			// void track_fields_game (GAME_FEATURE_NODE_START* field_feature_array, u16 field_feature_array_sz,
			// 				unsigned short *cur_car_tile_map_asm_id, COORD_2D curr_tile_coord_wo_wrap)
			COORD_2D cur_tile_wo_wrap_coord;
			cur_tile_wo_wrap_coord.x = sae_curr_x;
			cur_tile_wo_wrap_coord.y = sae_curr_y;
			track_fields_game(track_game_fds, track_game_field_sz,
								cas_tile_map_id[0], cur_tile_wo_wrap_coord);
			report_num_game_features(track_game_fds, track_game_field_sz, 
								&num_game_fds, &num_game_ffds);
			
			//====
			// POINT COUNT
			//====
			// draw something on the bg1
			for (cas_r = 0; cas_r < 3; cas_r++)
			{
				for (cas_col=0; cas_col<3; cas_col++)
				{
					tile_wrap_coord.x = sae_curr_x + cas_col;
					tile_wrap_coord.y = sae_curr_y + cas_r;
					wrapping_tile_coord(&tile_wrap_coord);
					se_idx = map_to_reg_se_index(tile_wrap_coord.x, tile_wrap_coord.y, bg1_width_unit_tile); 
					pse_1[se_idx] = bg1_tile_map_id[0][cas_r*3 + cas_col];
				}
			}

		}

		// allow to get a tile, only if there is still carcassonne tiles to take
		if (current_game_state == GET_TILE && carcassonne_number_of_tiles < CAR_TILES_MAX)
		{
			// update the palette according to Se_index
			// use the tonc `qran_range` to generate the
			// random Carcassonne category.
			rand_cat = qran_range(0, CAR_CAT);
			rand_cat_id = rand_cat;
				// generate another carcasonne category, if the generated category is out of tiles.
			while(car_cat_track[rand_cat_id] == car_cat_max[rand_cat_id])
			{
				rand_cat = qran_range(0, CAR_CAT);
				rand_cat_id = rand_cat;
			}
			// real Carcassonne Tile ID
			rand_cat = rand_cat << 2;
			rand_cat_min = rand_cat;
			rand_cat_max = rand_cat + 3;

			// load the cas graphics into the cas_tile_buffer = buffer of the cursor
			for (cas_r = 0; cas_r < 3; cas_r++)
			{
				for (cas_col=0; cas_col<3; cas_col++)
				{
					unsigned short tile_vram_id = cas_tile_map_id[rand_cat][cas_r*3 + cas_col] + CAR_TILE_OFFSET_IN_VRAM;
					//=== 
					// cursor boject 
					// ====
					//				  CBB TILE_index	
					memcpy32(&tile_mem[4][cas_r*8 + cas_col*2], 
						&elem_cas_tile_set[tile_vram_id], 
						16 // 1 DTILE = 16 x u32
						);
					
				}
			}

			// ==== 
			// change to a new game state 
			// ====
			current_game_state = PUT_DOWN_TILE;

			// ==== 
			// FEATURE REPORT
			// 1. at the moment, no need to store just report -> must delete the feature.
			// ====
				// void feature_report_per_cartilemap_implementation (u16* feature_flag_array, u16 feature_flag_array_sz,
				// 									unsigned short *cur_car_tile_map_asm_id
				// 									)
			init_feature_flgs(feature_end_open_flgs, feature_flgs_size);
			feature_report_per_cartilemap_implementation(feature_end_open_flgs, feature_end_open_flgs_width,
														 cas_tile_map_id[rand_cat]);
				// void report_amount_features_per_cartilemap (u16* flag_array, u16 flag_array_size, u16* result, u16 result_sz)
			report_amount_features_per_cartilemap(feature_end_open_flgs, feature_flgs_size, amount_type_features_percatile, amount_type_features_percatile_sz);
		};
		

		// put allow to draw/putdown a tile
		if(current_game_state == PUT_DOWN_TILE && carcassonne_number_of_tiles < CAR_TILES_MAX)
			{
				int se_idx = 0, sea_idx = 0;
				int cur_se_tid;
				if (key_hit(KEY_A))
				// allow to put down the drawn tile + with conditions
				{
					// pse[se_curr] = 2 SAE at once = the current one and the one adjacent to it
					// , because SAE is 8-bit, while pse[se_curr] is 16-bit.
					COORD_2D tile_wrap_coord;
					tile_wrap_coord.x = sae_curr_x;
					tile_wrap_coord.y = sae_curr_y;
					wrapping_tile_coord(&tile_wrap_coord);
					sae_curr = tile_wrap_coord.y * map_width_unit_tile + tile_wrap_coord.x;
					se_curr = sae_curr >>1;
					if (sae_curr % 2 == 0)
					{
						// lower 8-bit of pse
						cur_se_tid = pse[se_curr] & 0x00FF;
					}
					else
					{
						// higer 8-bit of pse
						cur_se_tid = pse[se_curr] & 0xFF00;
					}
					
					// Tile allowed to be put down if the back ground is empty, and its the first carcas tile
					if (cur_se_tid == 0)
					{

						// must be adjacent to another tile, and match its edge
						//    MSB                      LSB
						// 0x ADL_ML_ADR_MR_ADB_MB_ADT_MT : cnd_flg
						// ADx: adjacent @ x (x: Top, Bottom, Right, Left)
						// Mx: match @ x
						// 1 = match/ adj; 0 = not match/ not adj
						// 0x 0000_CL_CR_CB_CT: cnd_chk
						// Cx: check @ condition x = ADx == 1 & Mx == 1 
						// 1 = valid; 0 = not valid
						// => the condition for not first carcassonne tile to be put down
						// cnd_flg != 0 && cnd_chk == 0x00001111 == 15
						int cnd_flg = 0x00000000, cnd_chk = 0x00000000;
						
						// check AdT and MT => CT
						COORD_2D sae_indx_top_coord;
						sae_indx_top_coord.x = sae_curr_x;
						sae_indx_top_coord.y = sae_curr_y -1;
						wrapping_tile_coord(&sae_indx_top_coord);
						int sea_indx_top = sae_indx_top_coord.y*map_width_unit_tile + sae_indx_top_coord.x;
						int se_indx_top = sea_indx_top >> 1;
						unsigned short sae_tid_top;
						int lower_or_higher;
						lower_or_higher = sea_indx_top % 2; 
						if (lower_or_higher == 0)
						{
							// extract lower 8-bit of pse
							sae_tid_top = pse[se_indx_top] & 0x00FF;
						}
						else
						{
							// extract higher 8-bit of pse
							sae_tid_top = (pse[se_indx_top] & 0xFF00) >> 8;
						}
						if (sae_tid_top != 0 )
						{
							cnd_flg = cnd_flg | 0x00000010;
								// - middle top tile of obj
								// - middle tile of bg
							COORD_2D sae_indx_top_next_coord;
							sae_indx_top_next_coord.x = sae_indx_top_coord.x +1;
							sae_indx_top_next_coord.y = sae_indx_top_coord.y;
							wrapping_tile_coord(&sae_indx_top_next_coord);
							int sea_indx_top_next = sae_indx_top_next_coord.y*map_width_unit_tile + sae_indx_top_next_coord.x;
							int se_indx_top_next = sea_indx_top_next >> 1;
							unsigned short sae_tid_top_next;
							lower_or_higher = sea_indx_top_next % 2;
							if (lower_or_higher == 0)
							{
								// extract lower 8-bit of pse
								sae_tid_top_next = pse[se_indx_top_next] & 0x00FF;
							}
							else
							{
								// extract lower 8-bit of pse
								sae_tid_top_next = (pse[se_indx_top_next] & 0xFF00) >> 8;
							}
						
							if ((cas_tile_map_id[rand_cat][0*3 + 1]+CAR_TILE_OFFSET_IN_VRAM) == sae_tid_top_next)
							{
								cnd_chk = cnd_chk | 0x00000001;
							}  
							else
							{
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in carcas_data.s file]
								// CARCAS TID 26 (garden) == CARCAS TID 2 (green) [index in tiles-carcas_v20260329.s file / VRAM]
								if ((sae_tid_top_next == 26 && cas_tile_map_id[rand_cat][0*3 + 1] == 0) 
									|| (sae_tid_top_next == 2 && cas_tile_map_id[rand_cat][0*3 + 1] == 24))
								{
									cnd_chk = cnd_chk | 0x00000001;
								} 
							}
		
						}
						else // no top adjacent
						{
							cnd_chk = cnd_chk | 0x00000001;
						}

						// check AdB and MB => CB
						COORD_2D sae_indx_bot_coord;
						sae_indx_bot_coord.x = sae_curr_x;
						sae_indx_bot_coord.y = sae_curr_y +3;
						wrapping_tile_coord(&sae_indx_bot_coord);
						int sae_indx_bot = sae_indx_bot_coord.y*map_width_unit_tile + sae_indx_bot_coord.x;
						int se_indx_bot = sae_indx_bot >> 1;
						lower_or_higher = sae_indx_bot % 2;
						unsigned short sae_tid_bot;
						if (lower_or_higher == 0)
						{
							// extract lower 8-bit of pse
							sae_tid_bot = pse[se_indx_bot] & 0x00FF;
						}
						else
						{
							// extract higher 8-bit of pse
							sae_tid_bot = (pse[se_indx_bot] & 0xFF00) >> 8;
						}
						if (sae_tid_bot != 0 )
						{
							cnd_flg = cnd_flg | 0x00001000;
								// - middle bot tile of obj
								// - middle tile of bg
							COORD_2D sae_indx_bot_next_coord;
							sae_indx_bot_next_coord.x = sae_indx_bot_coord.x +1;
							sae_indx_bot_next_coord.y = sae_indx_bot_coord.y;
							wrapping_tile_coord(&sae_indx_bot_next_coord);
							int sea_indx_bot_next = sae_indx_bot_next_coord.y*map_width_unit_tile + sae_indx_bot_next_coord.x;
							int se_indx_bot_next = sea_indx_bot_next >> 1;
							unsigned short sae_tid_bot_next;
							lower_or_higher = sea_indx_bot_next % 2;
							if (lower_or_higher == 0)
							{
								// extract lower 8-bit of pse
								sae_tid_bot_next = pse[se_indx_bot_next] & 0x00FF;
							}
							else
							{
								// extract lower 8-bit of pse
								sae_tid_bot_next = (pse[se_indx_bot_next] & 0xFF00) >> 8;
							}

							if ((cas_tile_map_id[rand_cat][2*3 + 1]+CAR_TILE_OFFSET_IN_VRAM) == sae_tid_bot_next)
							{
								cnd_chk = cnd_chk | 0x00000010;
							}
							else
							{
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in carcas_data.s file]
								// CARCAS TID 26 (garden) == CARCAS TID 2 (green) [index in tiles-carcas_v20260329.s file / VRAM]
								if ((sae_tid_bot_next == 26 && cas_tile_map_id[rand_cat][2*3 + 1] == 0) 
									|| (sae_tid_bot_next == 2 && cas_tile_map_id[rand_cat][2*3 + 1] == 24))
								{
									cnd_chk = cnd_chk | 0x00000010;
								} 
							}  
						}
						else // no bot adjacent
						{
							cnd_chk = cnd_chk | 0x00000010;
						}

						// check AdR and MR => CR
						COORD_2D sae_indx_r_coord;
						sae_indx_r_coord.x = sae_curr_x +3;
						sae_indx_r_coord.y = sae_curr_y;
						wrapping_tile_coord(&sae_indx_r_coord);
						int sae_indx_r = sae_indx_r_coord.y*map_width_unit_tile + sae_indx_r_coord.x;
						int se_indx_r = sae_indx_r >> 1;
						lower_or_higher = sae_indx_r % 2;
						unsigned short sae_tid_r;
						if (lower_or_higher == 0)
						{
							// extract lower 8-bit of pse
							sae_tid_r = pse[se_indx_r] & 0x00FF;
						}
						else
						{
							// extract higher 8-bit of pse
							sae_tid_r = (pse[se_indx_r] & 0xFF00) >> 8;
						}
						if (sae_tid_r != 0 )
						{
							cnd_flg = cnd_flg | 0x00100000;
						
							//	miidle r tiles of obj
							//  middle l tiles of bg
							COORD_2D sae_indx_r_mid_coord;
							sae_indx_r_mid_coord.x = sae_indx_r_coord.x;
							sae_indx_r_mid_coord.y = sae_indx_r_coord.y +1;
							wrapping_tile_coord(&sae_indx_r_mid_coord);
							int sea_indx_r_mid = sae_indx_r_mid_coord.y*map_width_unit_tile + sae_indx_r_mid_coord.x;
							int se_indx_r_mid = sea_indx_r_mid >> 1;
							unsigned short sae_tid_r_mid;
							lower_or_higher = sea_indx_r_mid % 2;
							if (lower_or_higher == 0)
							{
								// extract lower 8-bit of pse
								sae_tid_r_mid = pse[se_indx_r_mid] & 0x00FF;
							}
							else
							{
								// extract lower 8-bit of pse
								sae_tid_r_mid = (pse[se_indx_r_mid] & 0xFF00) >> 8;
							}

							if ((cas_tile_map_id[rand_cat][1*3 + 2]+CAR_TILE_OFFSET_IN_VRAM) == sae_tid_r_mid)
							{
								cnd_chk = cnd_chk | 0x00000100;
							}
							else
							{
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in carcas_data.s file]
								// CARCAS TID 26 (garden) == CARCAS TID 2 (green) [index in tiles-carcas_v20260329.s file / VRAM]
								if ((sae_tid_r_mid == 26 && cas_tile_map_id[rand_cat][1*3 + 2] == 0) 
									|| (sae_tid_r_mid == 2 && cas_tile_map_id[rand_cat][1*3 + 2] == 24))
								{
									cnd_chk = cnd_chk | 0x00000100;
								} 
							}  
						}
						else // no r adjacent
						{
							cnd_chk = cnd_chk | 0x00000100;
						}

						// check AdL and ML => CL
						COORD_2D sae_indx_l_coord;
						sae_indx_l_coord.x = sae_curr_x-1;
						sae_indx_l_coord.y = sae_curr_y;
						wrapping_tile_coord(&sae_indx_l_coord);
						int sae_indx_l = sae_indx_l_coord.y*map_width_unit_tile + sae_indx_l_coord.x;
						int se_indx_l = sae_indx_l >> 1;
						lower_or_higher = sae_indx_l % 2;
						unsigned short sae_tid_l;
						if (lower_or_higher == 0)
						{
							// extract lower 8-bit of pse
							sae_tid_l = pse[se_indx_l] & 0x00FF;
						}
						else
						{
							// extract higher 8-bit of pse
							sae_tid_l = (pse[se_indx_l] & 0xFF00) >> 8;
						}
						if (sae_tid_l != 0 )
						{
							cnd_flg = cnd_flg | 0x10000000;
							//	middle l tiles of obj
							//  middle r tiles of bg
							COORD_2D sae_indx_l_mid_coord;
							sae_indx_l_mid_coord.x = sae_indx_l_coord.x;
							sae_indx_l_mid_coord.y = sae_indx_l_coord.y +1;
							wrapping_tile_coord(&sae_indx_l_mid_coord);
							int sea_indx_l_mid = sae_indx_l_mid_coord.y*map_width_unit_tile + sae_indx_l_mid_coord.x;
							int se_indx_l_mid = sea_indx_l_mid >> 1;
							unsigned short sae_tid_l_mid;
							lower_or_higher = sea_indx_l_mid % 2;
							if (lower_or_higher == 0)
							{
								// extract lower 8-bit of pse
								sae_tid_l_mid = pse[se_indx_l_mid] & 0x00FF;
							}
							else
							{
								// extract lower 8-bit of pse
								sae_tid_l_mid = (pse[se_indx_l_mid] & 0xFF00) >> 8;
							}

							if ((cas_tile_map_id[rand_cat][1*3 + 0]+CAR_TILE_OFFSET_IN_VRAM) == sae_tid_l_mid)
							{
								cnd_chk = cnd_chk | 0x00001000;
							}
							else
							{
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in carcas_data.s file]
								// CARCAS TID 26 (garden) == CARCAS TID 2 (green) [index in tiles-carcas_v20260329.s file / VRAM]
								if ((sae_tid_l_mid == 26 && cas_tile_map_id[rand_cat][1*3 + 0] == 0) 
									|| (sae_tid_l_mid == 2 && cas_tile_map_id[rand_cat][1*3 + 0] == 24))
								{
									cnd_chk = cnd_chk | 0x00001000;
								} 
							}
							
						}
						else // no r adjacent
						{
							cnd_chk = cnd_chk | 0x00001000;
						}

						// cnd_flg = 0x00000000 | 0x00001000;
						// cnd_chk =  0x00001111;
						// adj and match conds are valid, then allow put down the tile
						if (cnd_flg != 0 && cnd_chk == 0x00001111)
						{
							COORD_2D tile_wrap_coord;
							for (cas_r = 0; cas_r < 3; cas_r++)
							{
								for (cas_col=0; cas_col<3; cas_col++)
								{
									tile_wrap_coord.x = sae_curr_x + cas_col;
									tile_wrap_coord.y = sae_curr_y + cas_r;
									wrapping_tile_coord(&tile_wrap_coord);
									sea_idx = tile_wrap_coord.y*map_width_unit_tile + tile_wrap_coord.x; 
									// sea_idx = sae_curr + cas_r*map_width_unit_tile + cas_col; 
									se_idx = sea_idx >> 1;
									if (sea_idx % 2 == 0)
									{
										// write to lower 8-bit of pse, preseve the higher 8-bit of pse
										pse[se_idx] = (pse[se_idx] & 0xFF00) | ((cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+CAR_TILE_OFFSET_IN_VRAM) & 0x00FF);
									}
									else
									{
										// write higer 8-bit of pse, preserve the lower 8 bit of pse
										pse[se_idx] = (pse[se_idx] & 0x00FF) | (((cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+CAR_TILE_OFFSET_IN_VRAM)<<8)  & 0xFF00);
									}
								}
							}

							// game state is allowed to change only when the tile is put down
							carcassonne_number_of_tiles = carcassonne_number_of_tiles +1;
							car_cat_track[rand_cat_id] = car_cat_track[rand_cat_id] + 1;
							// save the tile into conceptual carcassonne map
							COORD_2D car_coord;
							map_tile_to_ctile(sae_curr_x, sae_curr_y, &car_coord.x, &car_coord.y);
							carcassonne_full_map[carcassonne_number_of_tiles-1].car_tid = rand_cat;
							carcassonne_full_map[carcassonne_number_of_tiles-1].car_map_coord = car_coord.y*CAR_MAP_WIDTH_x + car_coord.x;
							current_game_state = MEEPLE_START;
							// current_game_state = GET_TILE;

							// ===
							// TRACK GAME FEATURE
							// ===
								// CITY
							// void create_cartilemap_node (u16* flag_array, u16 flag_array_size_width, u16 feature_type_idx,
							// GAME_FEATURES cartilemap_id, COORD_2D cartilemap_coord,
							// GAME_FEATURE_NODE_ptr* cartilemap_node_array, u16 cartilemap_node_array_sz)
							create_cartilemap_node(feature_end_open_flgs, feature_end_open_flgs_width, city_idx,
													rand_cat_id, car_coord, 
													track_game_city_nodes, track_game_city_nodes_sz);
							// void insert_nodes_into_existent_ftrs (GAME_FEATURE_NODE_START* ftr_game_array, u16 ftr_game_array_sz, 
							// 							GAME_FEATURE_NODE_ptr* new_nodes_array, u16 new_node_array_sz)

							insert_nodes_into_existent_ftrs(track_game_cities, track_game_cities_sz,
															track_game_city_nodes, track_game_city_nodes_sz);
							// void check_all_merge_possibilities (GAME_FEATURE_NODE_START* ftr_game_array, u16 ftr_game_array_size)
							check_all_merge_possibilities(track_game_cities, track_game_cities_sz);
							// void report_num_game_features (GAME_FEATURE_NODE_START* game_city_array, u16 game_city_array_sz, u32* result)
							report_num_game_features(track_game_cities, track_game_cities_sz, 
												&num_game_cities, &num_game_fcities);
								// STREET
							// void create_cartilemap_node (u16* flag_array, u16 flag_array_size_width, u16 feature_type_idx,
							// 				GAME_FEATURES cartilemap_id, COORD_2D cartilemap_coord,
							// 				GAME_FEATURE_NODE_ptr* cartilemap_node_array, u16 cartilemap_node_array_sz)
							create_cartilemap_node(feature_end_open_flgs, feature_end_open_flgs_width, str_idx,
													rand_cat_id, car_coord, 
													track_game_str_nodes, track_game_str_nodes_sz);
							// void insert_nodes_into_existent_ftrs (GAME_FEATURE_NODE_START* ftr_game_array, u16 ftr_game_array_sz, 
							// 							GAME_FEATURE_NODE_ptr* new_nodes_array, u16 new_node_array_sz)

							insert_nodes_into_existent_ftrs(track_game_strs, track_game_strs_sz,
															track_game_str_nodes, track_game_str_nodes_sz);
							// void check_all_merge_possibilities (GAME_FEATURE_NODE_START* ftr_game_array, u16 ftr_game_array_size)
							check_all_merge_possibilities(track_game_strs, track_game_strs_sz);
							// void report_num_game_features (GAME_FEATURE_NODE_START* game_city_array, u16 game_city_array_sz, u32* result)
							report_num_game_features(track_game_strs, track_game_strs_sz, 
												&num_game_strs, &num_game_fstrs);
								
									// FIELDs
							// void track_fields_game (GAME_FEATURE_NODE_START* field_feature_array, u16 field_feature_array_sz,
							// 				unsigned short *cur_car_tile_map_asm_id, COORD_2D curr_tile_coord_wo_wrap)
							COORD_2D cur_tile_wo_wrap_coord;
							cur_tile_wo_wrap_coord.x = sae_curr_x;
							cur_tile_wo_wrap_coord.y = sae_curr_y;
							track_fields_game(track_game_fds, track_game_field_sz,
												cas_tile_map_id[rand_cat], cur_tile_wo_wrap_coord);
							report_num_game_features(track_game_fds, track_game_field_sz, 
												&num_game_fds, &num_game_ffds);

						}

					};

				}
				else
				{
					// (tonc_input.h) INLINE int key_tri_shoulder()   // R/L : +/-
					// rot_dir = rot_dir + 1*key_tri_shoulder();
					if (key_hit(KEY_R))
					{
						if ( rand_cat == rand_cat_max)
							rand_cat = rand_cat_min;
						else
							rand_cat = rand_cat + 1;
					}
					else if (key_hit(KEY_L))
					{
						if (rand_cat == rand_cat_min)
							rand_cat = rand_cat_max;
						else
							rand_cat = rand_cat -1;
					}
					else
					{
						// nothing
					};
					
					//rand_cat = rand_cat + rot_dir;
					// load the cas graphics into the cas_tile_buffer = buffer of the cursor
					for (cas_r = 0; cas_r < 3; cas_r++)
					{
						for (cas_col=0; cas_col<3; cas_col++)
						{
							//				  CBB TILE_index	
							memcpy32(&tile_mem[4][cas_r*8 + cas_col*2], 
								&elem_cas_tile_set[cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+CAR_TILE_OFFSET_IN_VRAM], 
								16 // 1 DTILE = 16 x u32
							);
						}
					}

					// ==== 
					// FEATURE REPORT
					// 1. at the moment, no need to store just report -> must delete the feature.
					// ====
						// void feature_report_per_cartilemap_implementation (u16* feature_flag_array, u16 feature_flag_array_sz,
						// 							unsigned short *cur_car_tile_map_asm_id
						// 							)
					init_feature_flgs(feature_end_open_flgs, feature_flgs_size);
					feature_report_per_cartilemap_implementation(feature_end_open_flgs, feature_end_open_flgs_width,
																 cas_tile_map_id[rand_cat]);
						// void report_amount_features_per_cartilemap (u16* flag_array, u16 flag_array_size, u16* result, u16 result_sz)
					report_amount_features_per_cartilemap(feature_end_open_flgs, feature_flgs_size, amount_type_features_percatile, amount_type_features_percatile_sz);
			
						};

			};
		
		if (current_game_state == MEEPLE_START && carcassonne_number_of_tiles < CAR_TILES_MAX)
		{
			asx_prev = asx;
			bg1_pt_prev = bg1_pt;
			current_game_state = MEEPLE_EXPLORE;	
			
		}

		if (current_game_state == MEEPLE_EXPLORE && carcassonne_number_of_tiles < CAR_TILES_MAX)
		{

			for (cas_r = 0; cas_r < 3; cas_r++)
			{
				for (cas_col=0; cas_col<3; cas_col++)
				{
					unsigned short tile_bg1_sid = bg1_tile_map_id[0][cas_r*3 + cas_col];
					//				  CBB TILE_index	
					memcpy32(&tile_mem[4][cas_r*8 + cas_col*2], 
						&elem_bg1_tile_set[tile_bg1_sid], 
						16 // 1 DTILE = 16 x u32
					);
				}
			}

			if (key_hit(KEY_SELECT))
			{
				// CANCEL THE ACTION
				// change game state
				current_game_state = MEEPLE_DECISION;

				// set the BGs position back to current putdown tile
				bg_rotscale_ex(&bgaff, &asx_prev);
				REG_BG_AFFINE[2]= bgaff;
				
				REG_BG_OFS[1]= bg1_pt_prev;
			}

			if (key_hit(KEY_B))
			{
				// CANCEL THE ACTION
				// change game state
				current_game_state = GET_TILE;

				// set the obj position back to original
				x = INIT_OBJ_X;
				y = INIT_OBJ_Y;
			}
		}

		if (current_game_state == MEEPLE_DECISION && carcassonne_number_of_tiles < CAR_TILES_MAX)
		{

			// load the cas graphics into the buffer of the cursor: the ARROW
			for (cas_r = 0; cas_r < 3; cas_r++)
			{
				for (cas_col=0; cas_col<3; cas_col++)
				{
					if (cas_r == 0 && cas_col == 0)
					{
						// arrow TID = 10 [index in tiles-bg1.s file]
						memcpy32(&tile_mem[4][cas_r*8 + cas_col*2], 
							&elem_bg1_tile_set[10], 
							16 // 1 DTILE = 16 x u32
						);						
					}
					else
					{
						//  transparent tile
						memcpy32(&tile_mem[4][cas_r*8 + cas_col*2], 
							&elem_bg1_tile_set[0], 
							16 // 1 DTILE = 16 x u32
						);					
					}

				}
			}

			if (key_hit(KEY_B))
			{
				// CANCEL THE ACTION
				// change game state
				current_game_state = GET_TILE;

				// set the obj position back to original
				x = INIT_OBJ_X;
				y = INIT_OBJ_Y;
			}

		}
		
		if (carcassonne_number_of_tiles == CAR_TILES_MAX)
		{
			current_game_state = END;
		}

		if (current_game_state == END)
		{
				const TILE8 end_tiles[1]=
			{
				// bg tile8: 8bit/pixel = 2hex/pixel
				// {{0x10000001, 0x01111110, 0x01111110, 0x01111110,
				//   0x01111110, 0x01111110, 0x01111110, 0x10000001}},

			{{0x00000050, 0x05000000, 
			  0x05050500, 0x00050505,
			  0x05050500, 0x00050505, 
			  0x05050500, 0x00050505,
			  0x05050500, 0x00050505, 
			  0x05050500, 0x00050505,
			  0x05050500, 0x00050505, 
			  0x00000050, 0x05000000}}
			};
			for (cas_r = 0; cas_r < 3; cas_r++)
			{
				for (cas_col=0; cas_col<3; cas_col++)
				{
					//				  CBB TILE_index	
					memcpy32(&tile_mem[4][cas_r*8 + cas_col*2], 
						&end_tiles[0], 
						16 // 1 DTILE = 16 x u32
					);
				}
			}	
		}
	
		// In this example, there is only 1 sprite of size 64 pixel x 64 pixel (= 8 TILE x 8 TILE)
		oam_copy(oam_mem, obj_buffer, 1);	// only need to update one

		// write text
		s32 ctile_idx=0, ctile_idy=0;
		s32 render_tile_idx=0, render_tile_idy=0;	
		map_tile_to_ctile(sae_curr_x, sae_curr_y, &ctile_idx, &ctile_idy);
		map_ctile_to_tile(ctile_idx, ctile_idy, &render_tile_idx, &render_tile_idy, true);

		// tte_printf("#{es;P}ctidx/y:%ld/%ld-ectid:%ld/%ld\ntile_dx/y: %ld/%ld ctile_dx/y: %ld/%ld\nrender_tile_dx/y: %ld/%ld",
		// 	tst_start_ct.x, tst_start_ct.y, tst_end_ct.x, tst_end_ct.y, 
		// 	sae_curr_x, sae_curr_y,
		// 	ctile_idx, ctile_idy,
		// 	tst_rd_tid.x, tst_rd_tid.y);

		// ====
		// to make sure that the printed text is not updated every frame!
		// if the printed text is updated per every frame, which is so fast -> flickering effect.
		// ====
		u16 eo_flgs_iter=0, type_idx=0;
		char* eoflgs[amount_type_features_percatile_sz*4];
		for (type_idx=0; type_idx<amount_type_features_percatile_sz; type_idx++)
		{
			for (eo_flgs_iter=0; eo_flgs_iter<4; eo_flgs_iter++)
			{
				switch(feature_end_open_flgs[type_idx*feature_end_open_flgs_width + eo_flgs_iter])
				{
					case 0x0111: // 273
						eoflgs[type_idx*4 + eo_flgs_iter]= "torcbclc";
						break;
					case 0x1101: // 4353
						eoflgs[type_idx*4 +eo_flgs_iter]= "tcrcbolc";
						break;
					case 0x1110: // 4368
						eoflgs[type_idx*4 +eo_flgs_iter]= "tcrcbclo";
						break;	
					case 0x1011: // 4113
						eoflgs[type_idx*4 +eo_flgs_iter]= "tcrobclc";
						break;
					case 0x0101: // 257
						eoflgs[type_idx*4 +eo_flgs_iter]= "torcbolc";
						break;
					case 0x1010: // 4112
						eoflgs[type_idx*4 +eo_flgs_iter]= "tcrobclo";
						break;
					case 0x0010: // 16
						eoflgs[type_idx*4 +eo_flgs_iter]= "torobclo";
						break;
					case 0x0001: // 1
						eoflgs[type_idx*4 +eo_flgs_iter]= "torobolc";
						break;
					case 0x1000: // 4096
						eoflgs[type_idx*4 +eo_flgs_iter]= "tcrobolo";
						break;
					case 0x0100: // 256
						eoflgs[type_idx*4 +eo_flgs_iter]= "torcbolo";
						break;
					case 0x0011: // 17
						eoflgs[type_idx*4 +eo_flgs_iter]= "torobclc";
						break;
					case 0x1001: // 4097
						eoflgs[type_idx*4 +eo_flgs_iter]= "tcrobolc";
						break;
					case 0x1100: // 4352
						eoflgs[type_idx*4 +eo_flgs_iter]= "tcrcbolo";
						break;
					case 0x0110: // 272
						eoflgs[type_idx*4 +eo_flgs_iter]= "torcbclo";
						break;
					case 0x0000: // 0
						eoflgs[type_idx*4 +eo_flgs_iter]= "torobolo";
						break;		
					default:
						eoflgs[type_idx*4 +eo_flgs_iter]="NA";
				}
			}
		}
		
		if (prev_num_game_cities!=num_game_cities
			|| prev_num_game_fcities!=num_game_fcities
			|| prev_feature_end_open_flgs[0]!=feature_end_open_flgs[city_idx*feature_end_open_flgs_width +0]
			|| prev_feature_end_open_flgs[1]!=feature_end_open_flgs[city_idx*feature_end_open_flgs_width +1]
			|| prev_feature_end_open_flgs[2]!=feature_end_open_flgs[city_idx*feature_end_open_flgs_width +2]
			|| prev_feature_end_open_flgs[3]!=feature_end_open_flgs[city_idx*feature_end_open_flgs_width +3]
			|| prev_num_game_strs!=num_game_strs
			|| prev_num_game_fstrs!=num_game_fstrs
			|| prev_feature_end_open_flgs[4]!=feature_end_open_flgs[str_idx*feature_end_open_flgs_width +0]
			|| prev_feature_end_open_flgs[5]!=feature_end_open_flgs[str_idx*feature_end_open_flgs_width +1]
			|| prev_feature_end_open_flgs[6]!=feature_end_open_flgs[str_idx*feature_end_open_flgs_width +2]
			|| prev_feature_end_open_flgs[7]!=feature_end_open_flgs[str_idx*feature_end_open_flgs_width +3]
			|| prev_ctile_coord.x != ctile_idx
			|| prev_ctile_coord.y != ctile_idy
			|| prev_num_game_fds!=num_game_fds
			|| prev_num_game_ffds!=num_game_ffds
			) 
		{
		// cpt =city per tile
		// oc = open game city
		// fc = finised game city
		// tte_printf("#{es;P}tid-ct_x/y-left-oc/fc-ost/fst-of/ff:\n%d-%ld/%ld-%d/%d-%d/%d-%d/%d-%d/%d\ncpt:%d-%s/%s/%s/%s\nstpt:%d-%s/%s/%s/%s",
		// 	rand_cat_id, ctile_idx, ctile_idy,
		// 	carcassonne_number_of_tiles, CAR_TILES_MAX, 
		// 	num_game_cities, num_game_fcities,
		// 	num_game_strs, num_game_fstrs,
		// 	num_game_fds, num_game_ffds,
		// 	amount_type_features_percatile[city_idx],eoflgs[city_idx*4+0], eoflgs[city_idx*4+1], eoflgs[city_idx*4+2], eoflgs[city_idx*4+3],
		// 	amount_type_features_percatile[str_idx],eoflgs[str_idx*4+0], eoflgs[str_idx*4+1], eoflgs[str_idx*4+2], eoflgs[str_idx*4+3]
		// 	);
		
		tte_printf("#{es;P}tid-ct_x/y-left-oc/fc-ost/fst-of/ff:\n%d-%ld/%ld-%d/%d-%d/%d-%d/%d-%d/%d",
			rand_cat_id, ctile_idx, ctile_idy,
			carcassonne_number_of_tiles, CAR_TILES_MAX, 
			num_game_cities, num_game_fcities,
			num_game_strs, num_game_fstrs,
			num_game_fds, num_game_ffds
			);
		
		// no need to update the rand_cat_id, 
		// because it is updated automatically with `num_game_cities`  
		// or `feature_end_open_flgs`
		
		// update the num_game_cities
			// city
		prev_num_game_cities= num_game_cities;
		prev_num_game_fcities= num_game_fcities;
			// street
		prev_num_game_strs= num_game_strs;
		prev_num_game_fstrs= num_game_fstrs;
			// field
		prev_num_game_fds = num_game_fds;
		prev_num_game_ffds = num_game_ffds;

		// update the end/open feature per cartilemap flags
			// city
		prev_feature_end_open_flgs[0]=feature_end_open_flgs[city_idx*feature_end_open_flgs_width +0];
		prev_feature_end_open_flgs[1]=feature_end_open_flgs[city_idx*feature_end_open_flgs_width +1];
		prev_feature_end_open_flgs[2]=feature_end_open_flgs[city_idx*feature_end_open_flgs_width +2];
		prev_feature_end_open_flgs[3]=feature_end_open_flgs[city_idx*feature_end_open_flgs_width +3];
			// street
		prev_feature_end_open_flgs[4]=feature_end_open_flgs[str_idx*feature_end_open_flgs_width +0];
		prev_feature_end_open_flgs[5]=feature_end_open_flgs[str_idx*feature_end_open_flgs_width +1];
		prev_feature_end_open_flgs[6]=feature_end_open_flgs[str_idx*feature_end_open_flgs_width +2];
		prev_feature_end_open_flgs[7]=feature_end_open_flgs[str_idx*feature_end_open_flgs_width +3];

		// update the ctile coord
		prev_ctile_coord.x = ctile_idx;
		prev_ctile_coord.y = ctile_idy;
		}

	}
}

int main()
{
	// Init interrupts and VBlank irq.
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	// #define DCNT_OBJ			0x1000	//!< Enable objects
	// DCNT_OBJ = set 12th-bit to 1 =
	// in GBATEK- LCD I/O Display Control, 
	// it is defined as Screen Display OBJ  (0=Off, 1=On).
	// #define DCNT_OBJ_1D			0x0040	//!< OBJ-VRAM as array
	// DCNT_OBJ_1D = set 6th-bit to 1 =
	// using 1D sprite map mode to load the tile data.

	// BG 0 = for text
	// BG 1 = meeples, indications..
	// BG 2 = Carcassonne game map
	REG_DISPCNT= DCNT_MODE1 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;

	// init text
	// Init BG 0 for text on screen entries, using CBB 
	tte_init_chr4c_b4_default(0, BG_CBB(CBB_0_TEXT)|BG_SBB(SBB_0_TEXT));
	tte_init_con();
	// left, top, right, bottom
	// tte_set_margins(8, 120, 232, 156);
	//	void win_textbox(int bgnr, int left, int top, int right, int bottom, int bldy)
	//	bgnr = bg for windows; coordinates in unit [pixel]
	//			   l,       r,
	//				  t         b	
	win_textbox(0, 8, 110, 232, 156, 8);

	init_reg_obj();
	init_reg_bg();

	// init the OAM, and make sure that 
	// the all the sprites (all 128 sprites) are hidden at the beginning.
	// The author inits 128 sprites, 
	// even though there is only one sprite (= 1 data of a sprite image stored in VRAM).
	// the function is in `./toolbox.c` 
	oam_init(obj_buffer, 128);
	
	// == 
	// BG2 aff testing
	// ==
	// test background tiles
	// SAE (screen affine entry) only 8 bit, and we have to write 16/32 bit !!!! because SEs is in VRAM
	// write 2 DTILEs at once
	SCR_ENTRY *pse_tiles= bg2_map;
	// 0th row
					    // SAE 1, SAE 0 
	pse_tiles[0*(map_width_unit_tile >> 1) + 0] = 0x0001;
	pse_tiles[0*(map_width_unit_tile >> 1) + 0] = 0x0200 | pse_tiles[0*(map_width_unit_tile >> 1) + 0];
						// SAE 3, SAE 2
	pse_tiles[0*16 + 1] = 0x0302; 
	pse_tiles[0*16 + 2] = 0x0506; 
	pse_tiles[0*16 + 3] = 0x0807; 
	pse_tiles[0*16 + 4] = 0x0A09; 
	pse_tiles[0*16 + 5] = 0x0C0B; 
	pse_tiles[0*16 + 6] = 0x0E0D; 
	pse_tiles[0*16 + 7] = 0x100F;
					    // SAE 13, SAE 12
	pse_tiles[0*16 + ((map_width_unit_tile >> 1) - 2)] = 0x1413;
				// the screen fits only 30 tiles, 
				// so last 2 tiles can not be seen,
				// if no screen rolling is applied.
	pse_tiles[0*16 + 15] = 0x1413;

	// 1st row
	pse_tiles[1*(map_width_unit_tile >> 1) + 0] = 0x1211;
	pse_tiles[1*(map_width_unit_tile >> 1) + 1] = 0x1615; 

	// == 
	// BG1 regular, testing
	// ==
	init_reg_bg1 ();
	// IMPORTANT: bg1 has not been rolled to the same position as bg2 yet,
	// at the moment bg1 top left corner = screen top left corner. 
	// regular background SE is always 16-bit, whether reg background is 4bpp/ 8pp
	SCR_ENTRY *pse_tiles_bg1= bg1_map;
	// pse_tiles_bg1[0] = bg1_tile_map_id[0][0];
	// pse_tiles_bg1[1] = bg1_tile_map_id[0][1];
	// draw the cursor on bg 1
		int iter_r, iter_col;
	// int str_idx =1;
	for (iter_r = 0; iter_r < 3; iter_r++)
	{
		for (iter_col=0; iter_col<3; iter_col++)
		{
			COORD_2D iter_coord;
			u32 sea_idx,se_idx;
			iter_coord.x = 50 + iter_col;
			iter_coord.y = 0 + iter_r;
			wrapping_tile_coord(&iter_coord);
			sea_idx = map_to_reg_se_index(iter_coord.x, iter_coord.y, bg1_width_unit_tile); 
			pse_tiles_bg1[sea_idx] = bg1_tile_map_id[0][iter_r*3 + iter_col];
		}
	}


	game_loop();

	return 0;
}
