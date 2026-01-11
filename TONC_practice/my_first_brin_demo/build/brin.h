
//{{BLOCK(brin)

//======================================================================
//
//	brin, 512x256@4, 
//	+ palette 256 entries, not compressed
//	+ 31 tiles (t|f|p reduced) not compressed
//	+ regular map (in SBBs), not compressed, 64x32 
//	Total size: 512 + 992 + 4096 = 5600
//
//	Time-stamp: 2026-01-11, 18:36:52
//	Exported by Cearn's GBA Image Transmogrifier, v0.9.2
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_BRIN_H
#define GRIT_BRIN_H

#define brinTilesLen 992
extern const unsigned int brinTiles[248];

#define brinMapLen 4096
extern const unsigned short brinMap[2048];

#define brinPalLen 512
extern const unsigned short brinPal[256];

#endif // GRIT_BRIN_H

//}}BLOCK(brin)
