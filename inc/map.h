//	designer - third party architectural design utility for 'Dwarf Fortress'
//	Copyright (C) 2010 Edward Cree (see top of src/designer.c for license details)
//   inc/map.h - header for map manipulation functions
#pragma once

#include "draw.h"
#include "dialogs.h"
#include "bits.h"
#include "version.h"

// Basic map tile unit
typedef struct
{
	char data;
	char object;
}
tile;

// Preprocessed tile data in ASCII+colour form, for eg. DF-Tiles and Export
typedef struct
{
	unsigned char v;
	unsigned char fr,fg,fb;
	unsigned char br,bg,bb;
}
disp_tile;

disp_tile tchar(tile ***map, int x, int y, int z); // Tile preprocessor, produces an ASCII/DF-TILE
int load_map(char *filename, tile ****map, gui guibits, int *zslice, int *uslice, pos *view, pos *dview);
int save_map(char *filename, tile ***map, gui guibits);
int export_map(char *filename, tile ***map, gui guibits, bool qf);
int clear_map(tile ***map, bool alloc);

int levels;
int groundlevel;
int worldx;
int worldy;

// Map data definitions
#define TILE_ROCK	1
#define TILE_DOOR	2
#define TILE_FLOOR	4
#define TILE_GRASS	8
#define TILE_WATER	16
#define TILE_STAIRS	32
#define TILE_FORTS	64
#define TILE_OBJECT	128
#define OBJECT_BED		1
#define OBJECT_CHAIR	2
#define OBJECT_TABLE	3
#define OBJECT_STATUE	4
#define OBJECT_STKPILE	5

// Maximum map sizes
#define MAX_WORLDX	2048
#define MAX_WORLDY	2048
#define MAX_WORLDZ	256
