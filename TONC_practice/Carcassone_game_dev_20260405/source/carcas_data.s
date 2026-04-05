
@{{BLOCK(carcas_data)

	@ the index of the car tile id in vram must be offset by 2
	@ , because of manual added 2 tiles (bg tile, transparent tile) (see tiles-carcas_v20260329.s)
	.section .rodata 			@ rodata = read-only data
	.align	2
	.global tiles_gbcMap_v2		@  Max 288 unsigned chars
	.hidden tiles_gbcMap_v2
tiles_gbcMap_v2:				@  Map = Carcassone Tile graphic, Carcassonne starter category ID = 0
	@ starter, CARCAS CAT TILE ID 17 in database /home/tlplan/GBA_DEV/Carcassone_Data/tile-map.xlsx
	.hword 14,12,13,1,1,1,0,0,0
	.hword 0,2,15,0,2,12,0,2,14
	.hword 0,0,0,1,1,1,15,12,16
	.hword 16,2,0,12,2,0,13,2,0
	 @ CARCAS TID 1
	.hword 0,0,0,1,3,0,0,2,0
	.hword 0,2,0,1,4,0,0,0,0
	.hword 0,2,0,0,5,1,0,0,0
	.hword 0,0,0,0,6,1,0,2,0
	@ CARCAS TID  2
	.hword 0,0,24,1,3,0,0,2,0
	.hword 0,2,0,1,4,0,0,0,24
	.hword 0,2,0,0,5,1,24,0,0
	.hword 24,0,0,0,6,1,0,2,0
	@ CARCAS TID 3
	.hword 0,2,0,0,2,0,0,2,0
	.hword 0,0,0,1,1,1,0,0,0
	.hword 0,2,0,0,2,0,0,2,0
	.hword 0,0,0,1,1,1,0,0,0
	@ CARCAS TID 4
	.hword 0,2,0,0,2,24,0,2,0
	.hword 0,0,0,1,1,1,0,24,0
	.hword 0,2,0,24,2,0,0,2,0
	.hword 0,24,0,1,1,1,0,0,0
	@ CARCAS TID 5
	.hword 0,0,0,1,7,1,0,2,0
	.hword 0,2,0,1,8,0,0,2,0
	.hword 0,2,0,1,9,1,0,0,0
	.hword 0,2,0,0,10,1,0,2,0
	@ CARCAS TID 6
	.hword 0,2,0,1,11,1,0,2,0
	.hword 0,2,0,1,11,1,0,2,0
	.hword 0,2,0,1,11,1,0,2,0
	.hword 0,2,0,1,11,1,0,2,0
	@ CARCAS TID 7
	.hword 0,0,0,0,25,0,0,0,0
	.hword 0,0,0,0,25,0,0,0,0
	.hword 0,0,0,0,25,0,0,0,0
	.hword 0,0,0,0,25,0,0,0,0
	@ CARCAS TID 8
	.hword 0,0,0,0,25,1,0,0,0
	.hword 0,0,0,0,25,0,0,2,0
	.hword 0,0,0,1,25,0,0,0,0
	.hword 0,2,0,0,25,0,0,0,0
	@ CARCAS TID 9
	.hword 14,12,13,0,0,0,0,0,0
	.hword 0,0,15,0,0,12,0,0,14
	.hword 0,0,0,0,0,0,15,12,16
	.hword 16,0,0,12,0,0,13,0,0
	@ CARCAS TID 10
	.hword 14,12,13,0,24,0,0,0,0
	.hword 0,0,15,0,24,12,0,0,14
	.hword 0,0,0,0,24,0,15,12,16
	.hword 16,0,0,12,24,0,13,0,0
	@ CARCAS TID 11
	.hword 14,12,13,0,6,1,0,2,0
	.hword 0,0,15,1,3,12,0,2,14
	.hword 0,2,0,1,4,0,15,12,16
	.hword 16,2,0,12,5,1,13,0,0
	@ CARCAS TID 12
	.hword 14,12,13,1,3,0,0,2,0
	.hword 0,2,15,1,4,12,0,0,14
	.hword 0,2,0,0,5,1,15,12,16
	.hword 16,0,0,12,6,1,13,2,0
	@ CARCAS TID 13
	.hword 23,12,13,12,0,0,13,0,0
	.hword 14,12,22,0,0,12,0,0,14
	.hword 0,0,15,0,0,12,15,12,23
	.hword 16,0,0,12,0,0,22,12,16
	@ CARCAS TID 14
	.hword 23,12,13,12,24,0,13,0,0
	.hword 14,12,22,0,24,12,0,0,14
	.hword 0,0,15,0,24,12,15,12,23
	.hword 16,0,0,12,24,0,22,12,16
	@ CARCAS TID 15
	.hword 14,12,13,0,0,0,15,12,16
	.hword 16,0,15,12,0,12,13,0,14
	.hword 14,12,13,0,0,0,15,12,16
	.hword 16,0,15,12,0,12,13,0,14
	@ CARCAS TID 16
	.hword 14,12,13,0,24,0,15,12,16
	.hword 16,0,15,12,24,12,13,0,14
	.hword 14,12,13,0,24,0,15,12,16
	.hword 16,0,15,12,24,12,13,0,14
	@ CARCAS TID 18
	.hword 14,12,13,1,7,1,0,2,0
	.hword 0,2,15,1,8,12,0,2,14
	.hword 0,2,0,1,9,1,15,12,16
	.hword 16,2,0,12,10,1,13,2,0
	@ CARCAS TID 19
	.hword 14,12,12,0,0,12,0,0,14
	.hword 0,0,15,0,0,12,15,12,12
	.hword 16,0,0,12,0,0,12,12,16
	.hword 12,12,13,12,0,0,13,0,0
	@ CARCAS TID 20
	.hword 14,12,21,0,0,12,0,0,14
	.hword 0,0,15,0,0,12,15,12,21
	.hword 16,0,0,12,0,0,21,12,16
	.hword 21,12,13,12,0,0,13,0,0
	@ CARCAS TID 21
	.hword 14,12,12,0,24,12,0,0,14
	.hword 0,0,15,0,24,12,15,12,12
	.hword 16,0,0,12,24,0,12,12,16
	.hword 12,12,13,12,24,0,13,0,0
	@ CARCAS TID 22
	.hword 14,12,21,0,24,12,0,0,14
	.hword 0,0,15,0,24,12,15,12,21
	.hword 16,0,0,12,24,0,21,12,16
	.hword 21,12,13,12,24,0,13,0,0
	@ CARCAS TID 23
	.hword 14,12,12,1,3,12,0,2,14
	.hword 0,2,15,1,4,12,15,12,12
	.hword 16,2,0,12,5,1,12,12,16
	.hword 12,12,13,12,6,1,13,2,0
	@ CARCAS TID 24
	.hword 14,12,21,1,3,12,0,2,14
	.hword 0,2,15,1,4,12,15,12,21
	.hword 16,2,0,12,5,1,21,12,16
	.hword 21,12,13,12,6,1,13,2,0
	@ CARCAS TID 25
	.hword 14,12,13,0,12,0,15,12,16
	.hword 16,0,15,12,12,12,13,0,14
	.hword 14,12,13,0,12,0,15,12,16
	.hword 16,0,15,12,12,12,13,0,14
	@ CARCAS TID 26
	.hword 14,12,13,0,21,0,15,12,16
	.hword 16,0,15,12,21,12,13,0,14
	.hword 14,12,13,0,21,0,15,12,16
	.hword 16,0,15,12,21,12,13,0,14
	@ CARCAS TID 27
	.hword 12,12,12,12,21,12,12,12,12
	.hword 12,12,12,12,21,12,12,12,12
	.hword 12,12,12,12,21,12,12,12,12
	.hword 12,12,12,12,21,12,12,12,12
	@ CARCAS TID 28
	.hword 12,12,12,12,12,12,17,0,18
	.hword 18,12,12,0,12,12,19,12,12
	.hword 20,0,19,12,12,12,12,12,12
	.hword 12,12,17,12,12,0,12,12,20
	@ CARCAS TID 29
	.hword 12,12,12,12,21,12,17,0,18
	.hword 18,12,12,0,21,12,19,12,12
	.hword 20,0,19,12,21,12,12,12,12
	.hword 12,12,17,12,21,0,12,12,20
	@ CARCAS TID 30
	.hword 12,12,12,12,12,12,17,24,18
	.hword 18,12,12,24,12,12,19,12,12
	.hword 20,24,19,12,12,12,12,12,12
	.hword 12,12,17,12,12,24,12,12,20
	@ CARCAS TID 31
	.hword 12,12,12,12,12,12,17,2,18
	.hword 18,12,12,1,12,12,19,12,12
	.hword 20,2,19,12,12,12,12,12,12
	.hword 12,12,17,12,12,1,12,12,20
	@ CARCAS TID 32
	.hword 12,12,12,12,21,12,17,2,18
	.hword 18,12,12,1,21,12,19,12,12
	.hword 20,2,19,12,21,12,12,12,12
	.hword 12,12,17,12,21,1,12,12,20

	@ for Bg_tile, their id in vram has no offset,
	@ because they are the 2 manual added tiles
	.section .rodata 			@ rodata = read-only data
	.align	2
	.global Bg_tile		@  2 unsigned chars
	.hidden Bg_tile
Bg_tile:				@  Map = Carcassone Tile graphic, but for bg purpose
	@ transparent tile
	.hword 1,1,1,1,1,1,1,1,1	
	@ background tile
	.hword 0,0,0,0,0,0,0,0,0

@}}BLOCK(carcas_data)
