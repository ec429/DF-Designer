//	designer - third party architectural design utility for 'Dwarf Fortress'
//	Copyright (C) 2010-11 Edward Cree (see top of src/designer.c for license details)
//   inc/draw.h - header for drawing-functions
#pragma once
#include <stdbool.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#define CONSOLE_WIDTH 280
#define CONSOLE_HEIGHT OSIZ_Y

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

// Holds enough info for functions needing to write console or dialogues to do so
typedef struct
{
	SDL_Surface * screen, * overlay;
	TTF_Font * small_font, * big_font;
	SDL_Surface * small_button_u, * small_button_p, * box_small, * counter;
}
gui;

double c3; // used by cparms
bool showconsole;

SDL_Surface * gf_init(int x, int y);
int pset(SDL_Surface *screen, int x, int y, unsigned char r, unsigned char g, unsigned char b);
int line(SDL_Surface *screen, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b);
int dtext(SDL_Surface *scrn, int x, int y, char * text, TTF_Font *font, unsigned char r, unsigned char g, unsigned char b);
int dmenu(SDL_Surface *screen, int x, int y, int items, int pressed, int hover, char **text, TTF_Font *font, SDL_Surface *button_u, SDL_Surface *button_p, unsigned char r, unsigned char g, unsigned char b, unsigned char hr, unsigned char hg, unsigned char hb); // TODO use guibits
int dcounter(gui guibits, int x, int y, double val, char what);

cparms cuboid(int dx, int dy, int dz, double theta, double phi); // WARNING!  gf_init must be called first, as it sets c3 (=sqrt(3)/2.0)
double wedge(double a[3], double b[3]); // returns (a x b) . (1,1,1) / (||a x b|| ||1,1,1||)
int dcuboid(SDL_Surface *screen, int x, int y, cparms cbd, unsigned char r, unsigned char g, unsigned char b);

int colconsole(gui guibits, int delay, char *text, unsigned char r, unsigned char g, unsigned char b); // Writes a message to the scrolly console, in the given colour
int console(gui guibits, int delay, char *text); // shortcut for a simple grey console entry
