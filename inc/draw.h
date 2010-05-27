//	designer - third party architectural design utility for 'Dwarf Fortress'
//	Copyright (C) 2010 Edward Cree (see top of src/designer.c for license details)
//   inc/draw.h - header for drawing-functions
#pragma once
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

typedef struct
{
	int x;
	int y;
} pos;

typedef struct
{
	double x;
	double y;
} dpos;

typedef struct
{
	dpos vertex[7]; // offset on screen of visible 7 vertices
	unsigned char face[3]; // shaded brightness of visible 3 faces
}
cparms;

double c3; // used by cparms

SDL_Surface * gf_init(int x, int y);
int pset(SDL_Surface * screen, int x, int y, unsigned char r, unsigned char g, unsigned char b);
int line(SDL_Surface * screen, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b);
int dtext(SDL_Surface * scrn, int x, int y, char * text, TTF_Font * font, unsigned char r, unsigned char g, unsigned char b);
int dmenu(SDL_Surface * screen, int x, int y, int items, int pressed, int hover, char ** text, TTF_Font * font, SDL_Surface * button_u, SDL_Surface * button_p, unsigned char r, unsigned char g, unsigned char b, char hr, char hg, char hb);

cparms cuboid(int dx, int dy, int dz, double theta, double phi); // WARNING!  gf_init must be called first, as it sets c3 (=sqrt(3)/2.0)
double wedge(double a[3], double b[3]); // returns (a x b) . (1,1,1) / (||a x b|| ||1,1,1||)
int dcuboid(SDL_Surface * screen, int x, int y, cparms cbd, unsigned char r, unsigned char g, unsigned char b);
