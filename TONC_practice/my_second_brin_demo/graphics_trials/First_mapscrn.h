
//{{BLOCK(First_mapscrn)

//======================================================================
//
//	First_mapscrn, 256x256@4, 
//	+ palette 256 entries, not compressed
//	+ 8 tiles (t|f|p reduced) not compressed
//	+ regular map (flat), not compressed, 32x32 
//	Total size: 512 + 256 + 2048 = 2816
//
//	Time-stamp: 2026-01-13, 19:32:56
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_FIRST_MAPSCRN_H
#define GRIT_FIRST_MAPSCRN_H

#define First_mapscrnTilesLen 256
extern const unsigned int First_mapscrnTiles[64];

#define First_mapscrnMapLen 2048
extern const unsigned short First_mapscrnMap[1024];

#define First_mapscrnPalLen 512
extern const unsigned short First_mapscrnPal[256];

#endif // GRIT_FIRST_MAPSCRN_H

//}}BLOCK(First_mapscrn)
