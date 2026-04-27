#ifndef CARCAS_INFOS_H
#define CARCAS_INFOS_H

#include <tonc.h>

// ===========
// CARCASSONNE GAME RULES
// ===========
#define CAR_TILES_MAX 72

// FSM for game state
enum GAME_STATE {
	START,
	GET_TILE,
	PUT_DOWN_TILE,
	MEEPLE,
	END
};

// define the access to cacasone-tile map
typedef unsigned short CAS_TILE_MAP[9];
#define cas_tile_map_id	((CAS_TILE_MAP*)tiles_gbcMap_v2)
#define bg_tile_map_id	((CAS_TILE_MAP*)Bg_tile)
#define elem_cas_tile_set	((TILE8*)tiles_wallliteTiles)

#define bg1_tile_map_id	((CAS_TILE_MAP*)Bg1_tile)
#define elem_bg1_tile_set	((TILE8*)tiles_bg1Tiles)

#define CAR_CAT 32// cacarcasonne category number (tile with different graphic features)

#define CAR_BG_ID 150// > the CAR ID available in `tiles-carcas_v20260329.s`

typedef struct COORD_2D
{
	s32 x;
	s32 y;
} ALIGN4 COORD_2D;

#define CAR_MAP_WIDTH_x 91// [ctile]
#define CAR_MAP_HEIGHT_y 91
#define REF_CAR_TILE 45// [ctile]
#define CAR_MAP_COORD_MAX 8281 

typedef struct CAR_MAP_INFO
{
	u32 car_tid;
	s32 car_map_coord;
} ALIGN4 CAR_MAP_INFO;

extern int car_cat_max[32]; // carcas_infos.h
extern enum GAME_STATE current_game_state;

#endif