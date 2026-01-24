
//{{BLOCK(tiles_gbc)

//======================================================================
//
//	tiles_gbc, 64x16@4, 
//	+ palette 8 entries, not compressed
//	+ 16 tiles not compressed
//	Total size: 16 + 512 = 528
//
//	Time-stamp: 2026-01-24, 21:16:21
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_TILES_GBC_H
#define GRIT_TILES_GBC_H

#define tiles_gbcTilesLen 512
extern const unsigned int tiles_gbcTiles[128];

#define tiles_gbcPalLen 16
extern const unsigned short tiles_gbcPal[8];

#define tiles_gbcMapLen 108
extern const unsigned int tiles_gbcMap[27];

#endif // GRIT_TILES_GBC_H

//}}BLOCK(tiles_gbc)
