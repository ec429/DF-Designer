//	designer - third party architectural design utility for 'Dwarf Fortress'
//	Copyright (C) 2010-11 Edward Cree (see top of designer.c for license details)
//   inc/dialogs.h - header for gui dialogue boxes
#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include "draw.h"

// TODO: use guibits for these

int okbox(SDL_Surface * screen, SDL_Surface * boximage, char ** boxtext, unsigned int boxtextlines, SDL_Surface * button_u, SDL_Surface * button_p, TTF_Font * font, TTF_Font * buttonfont, char * buttontext, unsigned char r, unsigned char g, unsigned char b, unsigned char br, unsigned char bg, unsigned char bb);

bool ynbox(SDL_Surface * screen, SDL_Surface * boximage, char ** boxtext, unsigned int boxtextlines, SDL_Surface * button_u, SDL_Surface * button_p, TTF_Font * font, TTF_Font * buttonfont, char * ytext, char * ntext, bool def, unsigned char r, unsigned char g, unsigned char b, unsigned char br, unsigned char bg, unsigned char bb);

char * textentry(SDL_Surface * screen, SDL_Surface * boximage, char ** boxtext, unsigned int boxtextlines, TTF_Font * font, unsigned char r, unsigned char g, unsigned char b);
