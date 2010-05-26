#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include "../inc/draw.h"

int okbox(SDL_Surface * screen, SDL_Surface * boximage, char ** boxtext, int boxtextlines, SDL_Surface * button_u, SDL_Surface * button_p, TTF_Font * font, TTF_Font * buttonfont, char * buttontext, char r, char g, char b, char br, char bg, char bb);

bool ynbox(SDL_Surface * screen, SDL_Surface * boximage, char ** boxtext, int boxtextlines, SDL_Surface * button_u, SDL_Surface * button_p, TTF_Font * font, TTF_Font * buttonfont, char * ytext, char * ntext, bool def, char r, char g, char b, char br, char bg, char bb);

char * textentry(SDL_Surface * screen, SDL_Surface * boximage, char ** boxtext, int boxtextlines, TTF_Font * font, char r, char g, char b);
