/*
	designer - third party architectural design utility for 'Dwarf Fortress'
	Copyright (C) 2010 Edward Cree
	(contributions are copyright their respective owners)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "draw.h"
#include "dialogs.h"
#include "437.h"
#include "version.h"

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

#define max(a,b)	((a)>(b)?(a):(b))
#define min(a,b)	((a)<(b)?(a):(b))

#define CONSOLE_WIDTH 280
#define CONSOLE_HEIGHT OSIZ_Y

// Maximum map sizes
#define MAX_WORLDX	2048
#define MAX_WORLDY	2048
#define MAX_WORLDZ	256

typedef struct
{
	char *text;
	char key;
	char flags;
}
menuitem;

#define MI_FLAG_DISABLED	1

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

// Useful tile definitions (blank and error)
disp_tile wb={' ', 255, 255, 255, 0, 0, 0}, err={'!', 255, 0, 0, 0, 0, 192};

bool showconsole=true;

// TODO: bud these off into appropriate lib files
int console(SDL_Surface * screen, SDL_Surface * overlay, int delay, char * text, TTF_Font * font); // draw
int colconsole(SDL_Surface * screen, SDL_Surface * overlay, int delay, char * text, TTF_Font * font, unsigned char r, unsigned char g, unsigned char b); // draw
disp_tile tchar(tile ***map, int x, int y, int z, int worldx, int worldy, int groundlevel); // map
int load_map(char *filename, tile ****map, gui guibits, int *worldx, int *worldy, int *levels, int *groundlevel, int *zslice, int *uslice, pos *view, pos *dview); // map
int save_map(char *filename, tile ***map, gui guibits, int worldx, int worldy, int levels, int groundlevel); // map
int export_map(char *filename, tile ***map, gui guibits, int worldx, int worldy, int levels, int groundlevel, bool qf); // map
int clear_map(tile ***map, bool alloc, int worldx, int worldy, int levels, int groundlevel); // map
char * getl(FILE *); // bits

int main(int argc, char *argv[])
{
	// Set up DF & setting vars
	bool confirms=true, shelp=true, isonly=false;
	int levels=24;
	int groundlevel=9;
	int worldx=96;
	int worldy=96;
	char *lfn=NULL;
	// parse arguments
	int arg;
	for(arg=1; arg<argc; arg++)
	{
		if((strcasecmp(argv[arg], "-n")==0) || (strcasecmp(argv[arg], "--no-confirm")==0))
		{
			confirms=false;
		}
		else if((strcasecmp(argv[arg], "-i")==0) || (strcasecmp(argv[arg], "--no-full-3d")==0))
		{
			isonly=true;
		}
		else if((strcasecmp(argv[arg], "-h")==0) || (strcasecmp(argv[arg], "--no-help")==0))
		{
			shelp=false;
		}
		else if(strncasecmp(argv[arg], "-z=", 3)==0)
		{
			sscanf(argv[arg]+3, "%d", &levels);
		}
		else if(strncasecmp(argv[arg], "-x=", 3)==0)
		{
			sscanf(argv[arg]+3, "%d", &worldx);
			worldx=min(worldx, MAX_WORLDX);
		}
		else if(strncasecmp(argv[arg], "-y=", 3)==0)
		{
			sscanf(argv[arg]+3, "%d", &worldy);
			worldy=min(worldy, MAX_WORLDY);
		}
		else if(strncasecmp(argv[arg], "-z0=", 4)==0)
		{
			sscanf(argv[arg]+4, "%d", &groundlevel);
		}
		else
			lfn=argv[arg]; // Assume it's a file to open
	}
	
	// Allocate RAM for the map, and set it to a flat grass surface
	tile ***map = (tile ***)malloc(levels*sizeof(tile **));
	if(map==NULL)
	{
		fprintf(stderr, "Not enough mem / couldn't alloc!\n");
		return(2);
	}
	else
	{
		int e=clear_map(map, true, worldx, worldy, levels, groundlevel);
		if(e==2)
			return(2);
	}
	
	// Set up the fonts
	TTF_Init();
	atexit(TTF_Quit);
	TTF_Font *tiny_font=TTF_OpenFont(FONT_FILE, 8);
	TTF_Font *small_font=TTF_OpenFont(FONT_FILE, 11);
	TTF_Font *big_font=TTF_OpenFont(FONT_FILE, 24);
	if((!small_font) || (!big_font))
	{
		fprintf(stderr, "Failed to load fonts!\n");
		fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
		return(1);
	}

	// SDL stuff
	SDL_Surface * screen = gf_init(OSIZ_X, OSIZ_Y);
	SDL_WM_SetCaption("DF Designer by soundandfury", "DF Designer");
	SDL_EnableUNICODE(1);
	SDL_Event event;
	SDL_Rect cls;
	cls.x=0;
	cls.y=0;
	cls.w=OSIZ_X;
	cls.h=OSIZ_Y+21;
	SDL_Rect over;
	over.x=0;
	over.y=OSIZ_Y-CONSOLE_HEIGHT;
	over.w=OSIZ_X;
	over.h=CONSOLE_HEIGHT+20;
	int errupt = 0;
	
	// Set up the overlay screen for the scrolly console
	SDL_Surface * overlay = SDL_CreateRGBSurface(SDL_SWSURFACE, CONSOLE_WIDTH, CONSOLE_HEIGHT+20, OBPP, 0, 0, 0, 0);
	SDL_FillRect(overlay, &cls, SDL_MapRGB(overlay->format, 0, 0, 0));
	
	// Version & startup messages
	cls.h-=20;
	{
		char vermsg[32+strlen(VERSION_GIT)];
		sprintf(vermsg, "DF Designer %hhu.%hhu.%hhu%s%s", VERSION_MAJ, VERSION_MIN, VERSION_REV, VERSION_GIT[0]?"-":"", VERSION_GIT);
		console(screen, overlay, 20, vermsg, small_font);
		fprintf(stderr, "%s\n", vermsg);
	}
	
	fprintf(stderr, "Reading init/menu\n");
	console(screen, overlay, 20, "Reading init/menu", small_font);
	FILE *mfp = fopen("init/menu", "r");
	char ** mfile=NULL;int nlines=0;
	while(!feof(mfp))
	{
		nlines++;
		mfile=(char **)realloc(mfile, nlines*sizeof(char *));
		mfile[nlines-1]=getl(mfp);
	}
	/* parse init/menu (newgui text & shortcuts) with a state machine */
	int n, state=0;
	int i=0,j;
	menuitem ** menus = NULL;
	int nmenus=0;
	int * nitems = NULL;
	for(n=0;n<nlines;n++)
	{
		if(!((mfile[n][0]=='#') || (mfile[n][0]==0)))
		{
			switch(state)
			{
				case 0: // looking for a MENU:i:x
					if(strncmp(mfile[n], "MENU:", 5)==0)
					{
						int p,q;
						if(sscanf(mfile[n]+5, "%d:%d", &p, &q)==2)
						{
							if(p==i)
							{
								if(i==0)
								{
									nmenus=q;
									menus = (menuitem **)malloc(nmenus*sizeof(menuitem *));
									nitems = (int *)malloc(nmenus*sizeof(int));
								}
								nitems[i]=q;
								menus[i]=(menuitem *)malloc(nitems[i]*sizeof(menuitem));
								state=1;
								j=0;
							}
							else
							{
								fprintf(stderr, "%d Wrong number on MENU line, init/menu:%d\n", state, n+1);
								return(3);
							}
						}
						else
						{
							fprintf(stderr, "%d Malformed MENU line, init/menu:%d\n", state, n+1);
							return(3);
						}
					}
					else
					{
						fprintf(stderr, "%d Unexpected line, init/menu:%d\n", state, n+1);
						return(3);
					}
				break;
				case 1: // looking for an ITEM:j:...
					if(strncmp(mfile[n], "ITEM:", 5)==0)
					{
						int p;
						if(sscanf(mfile[n]+5, "%d:", &p)==1)
						{
							if(p==j)
							{
								char * left=strchr(mfile[n]+5, ':'), * right=strchr(left+1, ':');
								if(right==NULL)
								{
									menus[i][j].text=strdup(left+1);
									menus[i][j].key=0;
									menus[i][j].flags=0;
								}
								else
								{
									*right=0;
									menus[i][j].text=strdup(left+1);
									right++;
									if(*right!=':')
									{
										menus[i][j].key=*right;
										right++;
									}
									else
									{
										menus[i][j].key=0;
									}
									menus[i][j].flags=0;
									if(*right==':')
									{
										if(strstr(right, "DISABLED")!=NULL)
										{
											menus[i][j].flags|=MI_FLAG_DISABLED;
										}
									}
								}
								j++;
								if(j==nitems[i])
								{
									i++;
									if(i==nmenus)
									{
										state=2;
									}
									state=0;
								}
							}
							else
							{
								fprintf(stderr, "%d Wrong number on ITEM line, init/menu:%d\n", state, n+1);
								return(3);
							}
						}
						else
						{
							fprintf(stderr, "%d Malformed ITEM line, init/menu:%d\n", state, n+1);
							return(3);
						}
					}
					else
					{
						fprintf(stderr, "%d Unexpected line, init/menu:%d\n", state, n+1);
						return(3);
					}
				break;
				default:
					fprintf(stderr, "%d Bad state in parser-fsm, init/menu:%d\n", state, n+1);
					return(3);
				break;
			}
		}
		free(mfile[n]);
	}
	free(mfile);
	
	fprintf(stderr, "Loading gui images...\n");
	console(screen, overlay, 20, "Loading gui images...", small_font);
	
	// Load in GUI images
	SDL_Surface * small_button_u = IMG_Load("img/small_button_u.png");
	SDL_Surface * small_button_p = IMG_Load("img/small_button_p.png");
	SDL_Surface * box_small = IMG_Load("img/box.png");
	SDL_Surface * counter = IMG_Load("img/counter.png");
	if(!small_button_u || !small_button_p || !box_small || !counter)
	{
		imgfail:
		fprintf(stderr, "Failed to read images!\n");
		fprintf(stderr, "IMG_Load: %s\n", IMG_GetError());
		return(1);
	}
	
	/* read in the newgui images - first the 'base' ones, then try to open the art ones and if not, procedurally generate from the base ones & text */
	SDL_Surface * menubase[nmenus], * menubtn[nmenus][10];
	{
		char * basename=strdup("img/menu/ / .png");
		int i,j;
		for(i=0;i<nmenus;i++)
		{
			basename[9]=i+'0';
			basename[11]='b';
			menubase[i]=IMG_Load(basename);
			if(!menubase[i])
				goto imgfail; // there's nothing wrong with gotos.  Please don't get stroppy about them.
			for(j=0;j<nitems[i];j++)
			{
				basename[11]=j+'0';
				SDL_Surface * button=IMG_Load(basename);
				if(!button)
				{
					menubtn[i][j]=SDL_CreateRGBSurface(SDL_SWSURFACE, 48, 48, OBPP, 0, 0, 0, 0);
					SDL_BlitSurface(j?menubase[i]:menubase[0], NULL, menubtn[i][j], &cls);
					SDL_Rect txtrect = {2, 20, 44, 8};
					char *text=menus[i][j].text;
					int br=(menus[i][j].flags&MI_FLAG_DISABLED)?127:255;
					SDL_Color clrFg = {br, br, br, 0};
					SDL_Surface *sText = TTF_RenderText_Solid(tiny_font, text, clrFg);
					SDL_BlitSurface(sText, NULL, menubtn[i][j], &txtrect);
					SDL_FreeSurface(sText);
				}
				else
				{
					menubtn[i][j]=SDL_CreateRGBSurface(SDL_SWSURFACE, 48, 56, OBPP, 0, 0, 0, 0);
					SDL_BlitSurface(button, NULL, menubtn[i][j], &cls);
					SDL_Rect txtrect = {2, 46, 44, 10};
					char *text=menus[i][j].text;
					SDL_Color clrFg = {127, 127, 127, 0};
					SDL_Surface *sText = TTF_RenderText_Solid(tiny_font, text, clrFg);
					SDL_BlitSurface(sText, NULL, menubtn[i][j], &txtrect);
					SDL_FreeSurface(sText);
				}
				SDL_Rect txtrect = {4, 4, 8, 8};
				char text[2]={j+'0',0};
				int br=(menus[i][j].flags&MI_FLAG_DISABLED)?127:255;
				SDL_Color clrFg = {br, br, br, 0};
				SDL_Surface *sText = TTF_RenderText_Solid(tiny_font, text, clrFg);
				SDL_BlitSurface(sText, NULL, menubtn[i][j], &txtrect);
				SDL_FreeSurface(sText);
			}
		}
		free(basename);
	}
	
	// Load in the tilemap for DF-TILES editview
	SDL_Surface * dftiles=IMG_Load("img/df_tiles.png");
	if(!dftiles)
	{
		fprintf(stderr, "Failed to read img/df_tiles.png!\nASCII-TILES mode will not be available!\n");
		fprintf(stderr, "IMG_Load: %s\n", IMG_GetError());
	}
	
	// Warning about how beta we are
	if(confirms)
	{
		char * boxtextlines[] = {"Warning!  DF Designer is still beta!", "Use at your own risk.  For support:", "ask soundnfury on #bay12games or", "post in the forum thread"};
		okbox(screen, box_small, boxtextlines, 4, small_button_u, small_button_p, small_font, big_font, "OK", 24, 48, 96, 48, 96, 192);
	}
	else
	{
		console(screen, overlay, 20, "Warning!  DF Designer is still beta!", small_font);
	}
	
	// Send the key-help through the console, unless -h was given
	if(shelp)
	{
		SDL_Event makekey;
		makekey.type=SDL_KEYDOWN;
		makekey.key.type=SDL_KEYDOWN;
		makekey.key.keysym.sym=SDLK_h;
		SDL_PushEvent(&makekey);
		makekey.type=SDL_KEYUP;
		makekey.key.type=SDL_KEYUP;
		SDL_PushEvent(&makekey);
	}
	else
		colconsole(screen, overlay, 20, "Press h for key help.", small_font, 255, 255, 255);
	console(screen, overlay, 20, "Press ; for list of colour-codes.", small_font);
	
	console(screen, overlay, 8, "In Editing mode, using COLOUR tiles.", small_font);
	console(screen, overlay, 8, "Shadowing is ON - 4,6 to toggle.", small_font);
	
	// Set up control vars
	char button;
	pos mouse;
	char drag=0, dold=drag;
	
	pos view;
	view.x=max(0, (worldx-64)/2);
	view.y=max(0, (worldy-64)/2);
	pos dview={0, 0};
	
	int viewmode=0;
	int editmode=0;
	int zslice=groundlevel, uslice=max(zslice-2, 0); // current slice
	int ths=30,phs=6;int dth=0,dph=0;
	bool semislice=false;
	bool keyactive=false;
	bool keyplace=false;
	char lastkey='r';
	int lastx,lasty;
	int lastr=0;
	double ctrx=0, ctry=0;
	pos lastview;
	cls.x=0;
	cls.y=0;
	cls.w=OSIZ_X;
	cls.h=OSIZ_Y;
	
	int menu=0;
	int msel=-1;
	int mdo=-1;
	
	// Fill out the guibits information
	gui guibits;
	guibits.screen=screen;
	guibits.overlay=overlay;
	guibits.small_font=small_font;
	guibits.big_font=big_font;
	guibits.small_button_u=small_button_u;
	guibits.small_button_p=small_button_p;
	guibits.box_small=box_small;
	guibits.counter=counter;
	
	// If a filename was given on the cmdline, load it now
	if(lfn!=NULL)
	{
		int e=load_map(lfn, &map, guibits, &worldx, &worldy, &levels, &groundlevel, &zslice, &uslice, &view, &dview);
		if(e==2)
			return(2);
	}
	
	// Main event loop (TODO: there is too much in here which needs refactoring out)
	while(!errupt)
	{
		SDL_FillRect(screen, &cls, SDL_MapRGB(screen->format, 0, 0, 0));
		// minimap
		if(!showconsole)
		{
			int dx,dy;
			dx=ceil(worldx/256.0);
			dy=ceil(worldy/256.0);
			SDL_Rect minimap={2, 2, worldx/dx, worldy/dy};
			int ff=dx*dy;
			int x,y;
			for(x=0;x<floor(worldx/dx);x++)
			{
				for(y=0;y<floor(worldy/dx);y++)
				{
					int sx=minimap.x+x,sy=minimap.y+y;
					if(sy<OSIZ_Y)
					{
						unsigned char r=0,g=0,b=0;
						if((x==floor(view.x/dx)) || (x==floor((view.x+63)/dx)) || (y==floor(view.y/dy)) || (y==floor((view.y+63)/dy)))
						{
							r=255;g=255;b=0;
						}
						else
						{
							int z;
							for(z=min(zslice+1, levels-1);z>=max(zslice-3, 0);z--)
							{
								int delx, dely;
								for(delx=0;delx<dx;delx++)
								{
									for(dely=0;dely<dy;dely++)
									{
										char here=map[z][x*dx+delx][y*dy+dely].data;
										int br=pow(2, max(6-abs(zslice-z), 0))/ff;
										if(here&TILE_ROCK)
										{
											if(zslice==z)
											{
												r=min(r+72/ff, 255);
												g=min(g+72/ff, 255);
												b=min(b+172/ff, 255);
											}
											else
											{
												r=min(r+br, 255);
												g=min(g+br, 255);
												b=min(b+br, 255);
											}
										}
										else if(here&TILE_FORTS)
										{
											if(zslice==z)
											{
												r=min(r+72/ff, 255);
												g=min(g+172/ff, 255);
												b=min(b+172/ff, 255);
											}
											else
											{
												g=min(g+br, 255);
												b=min(b+br, 255);
											}
										}
										else if(here&TILE_WATER)
										{
											if(zslice==z)
											{
												r=min(r, 255);
												g=min(g, 255);
												b=min(b+128/ff, 255);
											}
											else
											{
												r=min(r, 255);
												g=min(g, 255);
												b=min(b+(br*2/3), 255);
											}
										}
										else if(here&TILE_FLOOR)
										{
											if(zslice==z)
											{
												r=min(r+172/ff, 255);
												g=min(g+172/ff, 255);
												b=min(b+72/ff, 255);
											}
											else
											{
												r=min(r+br/3, 255);
												g=min(g+br/3, 255);
												b=min(b+br/6, 255);
											}
										}
										else if(here&TILE_GRASS)
										{
											if(zslice==z)
											{
												g=min(g+128/ff, 255);
											}
											else
											{
												g=min(g+br, 255);
											}
										}
										else if(here&TILE_STAIRS)
										{
											if(zslice==z)
											{
												g=min((int)g+128/ff, 255);
												b=min((int)b+128/ff, 255);
											}
											else
											{
												g=min((int)g+br, 255);
												b=min((int)b+br, 255);
											}
										}
										else if(here&TILE_DOOR)
										{
											if(zslice==z)
												r=min(r+128/ff, 255);
											else
												r=min(r+br, 255);
										}
									}
								}
							}
						}
						pset(screen, sx, sy, r, g, b);
					}
				}
			}
		}
		
		// Main view.  Voodoo stuff, just trust it to work
		switch(viewmode)
		{
			case 0: // Edit mode
				{
					SDL_Rect maparea={280, 8, 512, 512};
					SDL_FillRect(screen, &maparea, SDL_MapRGB(screen->format, 0, 0, 0));
					SDL_Rect maptile;
					maptile.w=8;
					maptile.h=8;
					int x,y,z;
					for(x=0;x<min(worldx-view.x, 64);x++)
					{
						for(y=0;y<min(worldy-view.y, 64);y++)
						{
							maptile.x=maparea.x+(x*maptile.w);
							maptile.y=maparea.y+(y*maptile.h);
							if(((keyactive || (drag & SDL_BUTTON_LEFT)) && (mouse.x>=maptile.x) && (mouse.y>=maptile.y) && (mouse.x<maptile.x+maptile.w) && (mouse.y<maptile.y+maptile.h)) || ((lastr>1) && ((((mouse.x>=maptile.x) && (lastx+maptile.w*(lastview.x-view.x)<maptile.x+maptile.w)) || ((lastx+maptile.w*(lastview.x-view.x)>=maptile.x) && (mouse.x<maptile.x+maptile.w))) && (((mouse.y>=maptile.y) && (lasty+maptile.h*(lastview.y-view.y)<maptile.y+maptile.h)) || ((lasty+maptile.h*(lastview.y-view.y)>=maptile.y) && (mouse.y<maptile.y+maptile.h)))))) // clicking and dragging (hellish complicated-looking conditional, just trust me on this one :S ...    Here be dragons!)
							{
								switch(lastkey)
								{
									case 'r':
										map[zslice][x+view.x][y+view.y].data=keyplace?TILE_ROCK:0;
									break;
									case 'w':
										map[zslice][x+view.x][y+view.y].data=keyplace?TILE_WATER:0;
									break;
									case 'f':
										map[zslice][x+view.x][y+view.y].data=keyplace?TILE_FLOOR:0;
									break;
									case 'g':
										map[zslice][x+view.x][y+view.y].data=keyplace?TILE_GRASS:0;
									break;
									case 't':
										map[zslice][x+view.x][y+view.y].data=keyplace?TILE_STAIRS:0;
									break;
									case 'd':
										map[zslice][x+view.x][y+view.y].data=keyplace?TILE_DOOR:0;
									break;
									case 'o':
										map[zslice][x+view.x][y+view.y].data=keyplace?TILE_FORTS:0;
									break;
									case 'b':
										if(!keyplace)
											map[zslice][x+view.x][y+view.y].data&=~TILE_OBJECT;
										else
										{
											map[zslice][x+view.x][y+view.y].data=(map[zslice][x+view.x][y+view.y].data&(TILE_FLOOR|TILE_GRASS))|TILE_OBJECT;
											map[zslice][x+view.x][y+view.y].object=OBJECT_BED;
										}
									break;
									case 'c':
										if(!keyplace)
											map[zslice][x+view.x][y+view.y].data&=~TILE_OBJECT;
										else
										{
											map[zslice][x+view.x][y+view.y].data=(map[zslice][x+view.x][y+view.y].data&(TILE_FLOOR|TILE_GRASS))|TILE_OBJECT;
											map[zslice][x+view.x][y+view.y].object=OBJECT_CHAIR;
										}
									break;
									case 'a':
										if(!keyplace)
											map[zslice][x+view.x][y+view.y].data&=~TILE_OBJECT;
										else
										{
											map[zslice][x+view.x][y+view.y].data=(map[zslice][x+view.x][y+view.y].data&(TILE_FLOOR|TILE_GRASS))|TILE_OBJECT;
											map[zslice][x+view.x][y+view.y].object=OBJECT_TABLE;
										}
									break;
									case 'p':
										if(!keyplace)
											map[zslice][x+view.x][y+view.y].data&=~TILE_OBJECT;
										else
										{
											map[zslice][x+view.x][y+view.y].data=(map[zslice][x+view.x][y+view.y].data&(TILE_FLOOR|TILE_GRASS))|TILE_OBJECT;
											map[zslice][x+view.x][y+view.y].object=OBJECT_STKPILE;
										}
									break;
									case 'u':
										if(!keyplace)
											map[zslice][x+view.x][y+view.y].data&=~TILE_OBJECT;
										else
										{
											map[zslice][x+view.x][y+view.y].data=(map[zslice][x+view.x][y+view.y].data&(TILE_FLOOR|TILE_GRASS))|TILE_OBJECT;
											map[zslice][x+view.x][y+view.y].object=OBJECT_STATUE;
										}
									break;
									default: // ignore
									break;
								}
							}
							unsigned char r=0, g=0, b=0;
							switch(editmode)
							{
								case 0:
									for(z=(semislice?zslice:min(zslice+1, levels-1));z>=(semislice?zslice:max(uslice, 0));z--)
									{
										unsigned char here=map[z][x+view.x][y+view.y].data;
										int br=pow(2, max(6-abs(zslice-z), 0));
										if(here&TILE_ROCK)
										{
											if(zslice==z)
											{
												r=min(r+72, 255);
												g=min(g+72, 255);
												b=min(b+172, 255);
											}
											else
											{
												r=min(r+br, 255);
												g=min(g+br, 255);
												b=min(b+br, 255);
											}
										}
										else if(here&TILE_FORTS)
										{
											if(zslice==z)
											{
												r=min(r+72, 255);
												g=min(g+172, 255);
												b=min(b+172, 255);
											}
											else
											{
												g=min(g+br, 255);
												b=min(b+br, 255);
											}
										}
										else if(here&TILE_WATER)
										{
											if(zslice==z)
											{
												r=min(r, 255);
												g=min(g, 255);
												b=min(b+128, 255);
											}
											else
											{
												r=min(r, 255);
												g=min(g, 255);
												b=min(b+(br*2/3), 255);
											}
										}
										else if(here&TILE_FLOOR)
										{
											if(zslice==z)
											{
												r=min(r+172, 255);
												g=min(g+172, 255);
												b=min(b+72, 255);
											}
											else
											{
												r=min(r+br/3, 255);
												g=min(g+br/3, 255);
												b=min(b+br/6, 255);
											}
										}
										else if(here&TILE_GRASS)
										{
											if(zslice==z)
											{
												g=min(g+128, 255);
											}
											else
											{
												g=min(g+br, 255);
											}
										}
										else if(here&TILE_STAIRS)
										{
											if(zslice==z)
											{
												g=min((int)g+128, 255);
												b=min((int)b+128, 255);
											}
											else
											{
												g=min((int)g+br, 255);
												b=min((int)b+br, 255);
											}
										}
										else if(here&TILE_DOOR)
										{
											if(zslice==z)
												r=min(r+128, 255);
											else
												r=min(r+br, 255);
										}
										if(semislice)
										{
											r=min(r*2, 255);
											g=min(g*2, 255);
											b=min(b*2, 255);
										}
									}
									if((mouse.x>=maptile.x) && (mouse.y>=maptile.y) && (mouse.x<maptile.x+maptile.w) && (mouse.y<maptile.y+maptile.h))
									{
										SDL_FillRect(screen, &maptile, SDL_MapRGB(screen->format, 255, 255, 0));
										SDL_Rect mtile;
										mtile.x=maptile.x+1;
										mtile.y=maptile.y+1;
										mtile.w=maptile.w-2;
										mtile.h=maptile.h-2;
										SDL_FillRect(screen, &mtile, SDL_MapRGB(screen->format, r, g, b));
									}
									else
									{
										SDL_FillRect(screen, &maptile, SDL_MapRGB(screen->format, 0, semislice?192:255, 0));
										SDL_Rect mtile;
										mtile.x=maptile.x+1;
										mtile.y=maptile.y+1;
										mtile.w=maptile.w-1;
										mtile.h=maptile.h-1;
										SDL_FillRect(screen, &mtile, SDL_MapRGB(screen->format, r, g, b));
									}
									if(map[zslice][x+view.x][y+view.y].data&TILE_OBJECT)
									{
										int object=map[zslice][x+view.x][y+view.y].object;
										unsigned int tb=max(r, max(g, b));
										unsigned char or,og,ob;
										switch(object)
										{
											case OBJECT_BED:
												or=192;og=160;ob=0;
												pset(screen, maptile.x+3, maptile.y+2, or, og, ob);
												pset(screen, maptile.x+4, maptile.y+2, or, og, ob);
												pset(screen, maptile.x+2, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+2, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+4, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+2, maptile.y+5, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+5, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+6, or, og, ob);
												pset(screen, maptile.x+4, maptile.y+6, or, og, ob);
											break;
											case OBJECT_CHAIR:
												if(tb>160)
												{
													or=og=ob=128;
												}
												else
												{
													or=og=ob=255;
												}
												pset(screen, maptile.x+2, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+4, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+6, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+5, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+5, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+6, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+6, or, og, ob);
											break;
											case OBJECT_TABLE:
												if(tb>200)
												{
													or=og=ob=160;
												}
												else
												{
													or=og=ob=255;
												}
												pset(screen, maptile.x+1, maptile.y+2, or, og, ob);
												pset(screen, maptile.x+2, maptile.y+2, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+2, or, og, ob);
												pset(screen, maptile.x+4, maptile.y+2, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+2, or, og, ob);
												pset(screen, maptile.x+6, maptile.y+2, or, og, ob);
												pset(screen, maptile.x+7, maptile.y+2, or, og, ob);
												pset(screen, maptile.x+1, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+2, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+4, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+6, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+7, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+4, maptile.y+5, or, og, ob);
												pset(screen, maptile.x+4, maptile.y+6, or, og, ob);
											break;
											case OBJECT_STATUE:
												if(tb>200)
												{
													or=og=ob=128;
												}
												else
												{
													or=og=ob=255;
												}
												pset(screen, maptile.x+2, maptile.y+6, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+6, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+5, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+2, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+2, maptile.y+2, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+1, or, og, ob);
												pset(screen, maptile.x+4, maptile.y+1, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+1, or, og, ob);
												pset(screen, maptile.x+6, maptile.y+2, or, og, ob);
												pset(screen, maptile.x+6, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+4, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+5, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+6, or, og, ob);
												pset(screen, maptile.x+6, maptile.y+6, or, og, ob);
											break;
											case OBJECT_STKPILE:
												if(tb>160)
												{
													or=og=ob=96;
												}
												else
												{
													or=og=ob=224;
												}
												pset(screen, maptile.x+2, maptile.y+3, or, og, ob);										
												pset(screen, maptile.x+3, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+4, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+6, maptile.y+3, or, og, ob);
												pset(screen, maptile.x+2, maptile.y+5, or, og, ob);
												pset(screen, maptile.x+3, maptile.y+5, or, og, ob);
												pset(screen, maptile.x+4, maptile.y+5, or, og, ob);
												pset(screen, maptile.x+5, maptile.y+5, or, og, ob);
												pset(screen, maptile.x+6, maptile.y+5, or, og, ob);
											break;
											default:
												pset(screen, maptile.x+1, maptile.y+1, 255, 0, 0);
												pset(screen, maptile.x+2, maptile.y+2, 255, 0, 0);
												pset(screen, maptile.x+3, maptile.y+3, 255, 0, 0);
												pset(screen, maptile.x+4, maptile.y+4, 255, 0, 0);
												pset(screen, maptile.x+5, maptile.y+5, 255, 0, 0);
												pset(screen, maptile.x+1, maptile.y+5, 255, 0, 0);
												pset(screen, maptile.x+2, maptile.y+4, 255, 0, 0);
												pset(screen, maptile.x+4, maptile.y+2, 255, 0, 0);
												pset(screen, maptile.x+5, maptile.y+1, 255, 0, 0);
											break;
										}
									}
								break;
								case 1:
									if(!dftiles)
									{
										fprintf(stderr, "Error: mode 1 (ASCII-TILES) unavailable.  Reverting to mode 0 (COLOURS)");
										console(screen, overlay, 20, "Error: mode 1 (ASCII-TILES) unavailable.  Reverting to mode 0 (COLOURS)", small_font);
										editmode=0;
									}
									else
									{
										disp_tile xtile = tchar(map, x+view.x, y+view.y, zslice, worldx, worldy, groundlevel);
										SDL_FillRect(screen, &maptile, SDL_MapRGB(screen->format, xtile.br, xtile.bg, xtile.bb));
										SDL_Rect itile;
										itile.x=8*(xtile.v%16);
										itile.y=8*floor(xtile.v/16.0);
										itile.w=itile.h=8;
										SDL_Surface * fg=SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, 8, 8, 24, 0, 0, 0, 0);
										SDL_FillRect(fg, NULL, SDL_MapRGB(fg->format, xtile.fr, xtile.fg, xtile.fb));
										SDL_BlitSurface(dftiles, &itile, fg, NULL);
										SDL_SetColorKey(fg, SDL_SRCCOLORKEY, SDL_MapRGB(fg->format, 0, 0, 0));
										SDL_BlitSurface(fg, NULL, screen, &maptile);
										SDL_FreeSurface(fg);
										if((mouse.x>=maptile.x) && (mouse.y>=maptile.y) && (mouse.x<maptile.x+maptile.w) && (mouse.y<maptile.y+maptile.h))
										{
											line(screen, maptile.x, maptile.y, maptile.x+7, maptile.y, 255, 255, 0);
											line(screen, maptile.x, maptile.y, maptile.x, maptile.y+7, 255, 255, 0);
											line(screen, maptile.x+7, maptile.y, maptile.x+7, maptile.y+7, 255, 255, 0);
											line(screen, maptile.x, maptile.y+7, maptile.x+7, maptile.y+7, 255, 255, 0);
										}
									}
								break;
								default:
									;
									char string[100];
									sprintf(string, "Error: No such editmode %u.  Reverting to mode 0 (COLOURS)", editmode);
									fprintf(stderr, "%s\n", string);
									console(screen, overlay, 20, string, small_font);
									editmode=0;
								break;
							}
						}
					}
				}
			break;
			case 1: // Iso/3d view
				{
					SDL_Rect maparea={280, 0, 520, 520};
					SDL_FillRect(screen, &maparea, SDL_MapRGB(screen->format, 0, 0, 0));
					int x,y,z;
					int mx=min(worldx-view.x, 64);
					int my=min(worldy-view.y, 64);
					if(ths<0)
						ths+=72;
					phs=min(max(phs,0),18);
					double theta=(19+(ths%18))*M_PI/36,phi=phs*M_PI/36;
					int ord=(ths/18)%4;
					cparms c_rock=cuboid(8, 8, 10, theta, phi), c_flr=cuboid(8, 8, 0, theta, phi);
					for(z=uslice;z<=zslice;z++)
					{
						for(x=(isonly?(((ord%2)?my:mx)-1):0);isonly?(x>=0):(x<((ord%2)?my:mx));isonly?x--:x++)
						{
							for(y=0;y<((ord%2)?mx:my);y++)
							{
								unsigned char here;
								int dsx,dsy;
								int bx,by;
								switch(ord)
								{
									case 0:
										here=map[z][x+view.x][y+view.y].data;
										dsx=64-mx;dsy=64-my;
										bx=x+view.x;by=y+view.y;
									break;
									case 3:
										here=map[z][y+view.x][(my-x-1)+view.y].data;
										dsx=64-my;dsy=64-mx;
										bx=y+view.x;by=(my-x-1)+view.y;
									break;
									case 2:
										here=map[z][(mx-x-1)+view.x][(my-y-1)+view.y].data;
										dsx=64-mx;
										dsy=64-my;
										bx=(mx-x-1)+view.x;by=(my-y-1)+view.y;
									break;
									case 1:
										here=map[z][(mx-y-1)+view.x][x+view.y].data;
										dsx=64-my;dsy=64-mx;
										bx=(mx-y-1)+view.x;by=x+view.y;
									break;
								}
								int sx, sy;
								if(isonly)
								{
									sx=320+(x+y-12)*5;
									sy=180+(8*(uslice+20-z)-(x-y)*3);
								}
								else
								{
									sx=540-c_rock.vertex[2].x*(x+dsx-31)*.8-c_rock.vertex[1].x*(y+dsy-31)*.8;
									sy=120-c_rock.vertex[2].y*(x+dsx-31)-c_rock.vertex[1].y*(y+dsy-31)-(c_rock.vertex[3].y+1)*(uslice+20-z);
								}
								if(sy>8)
								{
									double brs=0.96+(0.04*sin(16.0*sin(bx+by*(3+z))));
									if(!semislice)
									{
										if(here&TILE_FLOOR)
										{
											if(isonly)
											{
												int dx;
												for(dx=0;dx<6;dx++)
												{
													line(screen, sx+dx+1, sy-dx/2+8, sx+dx-6, sy-dx/2+5, 224*50/(50+x+my-y), 224*50/(50+x+my-y), 96*20/(20+x+my-y));
												}
											}
											else
											{
												dcuboid(screen, sx, sy, c_flr, 255*brs, 255*brs, 108*brs);
											}
										}
										else if(here&TILE_GRASS)
										{
											if(isonly)
											{
												int dx;
												for(dx=0;dx<6;dx++)
												{
													line(screen, sx+dx, sy-dx/2+8, sx+dx-6, sy-dx/2+5, 96*60/(60+x+my-y), 224*60/(60+x+my-y), 96*60/(60+x+my-y));
												}
											}
											else
											{
												dcuboid(screen, sx, sy, c_flr, 108*brs, 255*brs, 108*brs);
											}
										}
									}
									if(here&TILE_ROCK)
									{
										if(isonly)
										{
											int dx;
											for(dx=0;dx<7;dx++)
											{
												line(screen, sx-dx, sy-dx/2, sx-dx, sy-dx/2+9, 172*75/(75+x+my-y), 172*75/(75+x+my-y), 224*75/(75+x+my-y));
												line(screen, sx+dx, sy-dx/2, sx+dx, sy-dx/2+9, 160*75/(75+x+my-y), 160*75/(75+x+my-y), 224*75/(75+x+my-y));
												line(screen, sx+dx+1, sy-(dx+1)/2, sx+dx-5, sy-(dx+1)/2-3, 184*75/(75+x+my-y), 184*75/(75+x+my-y), 224*75/(75+x+my-y));
											}
										}
										else
										{
											dcuboid(screen, sx, sy, c_rock, 224*brs, 224*brs, 255*brs);
										}
									}
									if(here&TILE_FORTS)
									{
										if(isonly)
										{
											int dx;
											for(dx=0;dx<7;dx++)
											{
												line(screen, sx-dx, sy-dx/2, sx-dx, sy-dx/2+9, 144*75/(75+x+my-y), 224*75/(75+x+my-y), 224*75/(75+x+my-y));
												line(screen, sx+dx, sy-dx/2, sx+dx, sy-dx/2+9, 128*75/(75+x+my-y), 224*75/(75+x+my-y), 224*75/(75+x+my-y));
												line(screen, sx+dx+1, sy-(dx+1)/2, sx+dx-5, sy-(dx+1)/2-3, 160*75/(75+x+my-y), 224*75/(75+x+my-y), 224*75/(75+x+my-y));
											}
										}
										else
										{
											dcuboid(screen, sx, sy, c_rock, 184*brs, 255*brs, 255*brs);
										}
									}
									if(here&TILE_WATER)
									{
										if(isonly)
										{
											int dx;
											for(dx=0;dx<7;dx++)
											{
												line(screen, sx-dx, sy-dx/2, sx-dx, sy-dx/2+9, 0, 0, 224*75/(75+x+my-y));
												line(screen, sx+dx, sy-dx/2, sx+dx, sy-dx/2+9, 0, 0, 224*75/(75+x+my-y));
												line(screen, sx+dx+1, sy-(dx+1)/2, sx+dx-5, sy-(dx+1)/2-3, 48*75/(75+x+my-y), 48*75/(75+x+my-y), 240*75/(75+x+my-y));
											}
										}
										else
										{
											dcuboid(screen, sx, sy, c_rock, 0, 0, 240*brs);
										}
									}
									if(here&TILE_DOOR)
									{
										if(isonly)
										{
											int dx;
											for(dx=0;dx<7;dx++)
											{
												line(screen, sx-dx, sy-dx/2, sx-dx, sy-dx/2+9, 224*50/(50+x+my-y), 112*50/(50+x+my-y), 112*50/(50+x+my-y));
												line(screen, sx+dx, sy-dx/2, sx+dx, sy-dx/2+9, 224*50/(50+x+my-y), 96*50/(50+x+my-y), 96*50/(50+x+my-y));
												line(screen, sx+dx, sy-dx/2, sx+dx-6, sy-dx/2-3, 224*50/(50+x+my-y), 128*50/(50+x+my-y), 128*50/(50+x+my-y));
											}
										}
										else
										{
											dcuboid(screen, sx, sy, c_rock, 255*brs, 127*brs, 127*brs);
										}
									}
									if(here&TILE_STAIRS)
									{
										if(isonly)
										{
											int dx;
											for(dx=0;dx<7;dx++)
											{
												line(screen, sx-dx, sy-dx/2, sx-dx, sy-dx/2+9, 0, 112*80/(50+x+my-y), 112*80/(50+x+my-y));
												line(screen, sx+dx, sy-dx/2, sx+dx, sy-dx/2+9, 0, 96*80/(50+x+my-y), 96*80/(50+x+my-y));
												line(screen, sx+dx, sy-dx/2, sx+dx-6, sy-dx/2-3, 0, 128*80/(50+x+my-y), 128*80/(50+x+my-y));
											}
										}
										else
										{
											dcuboid(screen, sx, sy, c_rock, 0, 144*brs, 144*brs);
										}
									}
								}
							}
						}
						if(semislice)
						{
							if(z+1<levels)
							{
								for(x=(isonly?(((ord%2)?my:mx)-1):0);isonly?(x>=0):(x<((ord%2)?my:mx));isonly?x--:x++)
								{
									for(y=0;y<((ord%2)?mx:my);y++)
									{
										unsigned char here;
										int dsx,dsy;
										int bx,by;
										switch(ord)
										{
											case 0:
												here=map[z+1][x+view.x][y+view.y].data;
												dsx=64-mx;dsy=64-my;
												bx=x+view.x;by=y+view.y;
											break;
											case 3:
												here=map[z+1][y+view.x][(my-x-1)+view.y].data;
												dsx=64-my;dsy=64-mx;
												bx=y+view.x;by=(my-x-1)+view.y;
											break;
											case 2:
												here=map[z+1][(mx-x-1)+view.x][(my-y-1)+view.y].data;
												dsx=64-mx;
												dsy=64-my;
												bx=(mx-x-1)+view.x;by=(my-y-1)+view.y;
											break;
											case 1:
												here=map[z+1][(mx-y-1)+view.x][x+view.y].data;
												dsx=64-my;dsy=64-mx;
												bx=(mx-y-1)+view.x;by=x+view.y;
											break;
										}
										int sx, sy;
										if(isonly)
										{
											sx=320+(x+y-12)*5;
											sy=180+(8*(uslice+19-z)-(x-y)*3);
										}
										else
										{
											sx=540-c_rock.vertex[2].x*(x+dsx-31)*.8-c_rock.vertex[1].x*(y+dsy-31)*.8;
											sy=120-c_rock.vertex[2].y*(x+dsx-31)-c_rock.vertex[1].y*(y+dsy-31)-(c_rock.vertex[3].y+1)*(uslice+19-z);
										}
										if(sy>8)
										{
											double brs=0.96+(0.04*sin(16.0*sin(bx+by*(4+z))));
											if(here&TILE_FLOOR)
											{
												if(isonly)
												{
													int dx;
													for(dx=0;dx<6;dx++)
													{
														line(screen, sx+dx+1, sy-(dx+1)/2, sx+dx-6, sy-(dx+1)/2-3, 224*50/(50+x+my-y), 224*50/(50+x+my-y), 96*20/(20+x+my-y));
													}
												}
												else
												{
													dcuboid(screen, sx, sy, c_flr, 255*brs, 255*brs, 108*brs);
												}
											}
											else if(here&TILE_GRASS)
											{
												if(isonly)
												{
													int dx;
													for(dx=0;dx<6;dx++)
													{
														line(screen, sx+dx, sy-dx/2, sx+dx-6, sy-dx/2-3, 96*60/(60+x+my-y), 224*60/(60+x+my-y), 96*60/(60+x+my-y));
													}
												}
												else
												{
													dcuboid(screen, sx, sy, c_flr, 108*brs, 255*brs, 108*brs);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			break;
			default:
				{
					char string[100];
					sprintf(string, "Error: No such viewmode %u.  Reverting to mode 0 (slice-down/edit)", viewmode);
					fprintf(stderr, "%s\n", string);
					console(screen, overlay, 20, string, small_font);
					viewmode=0;
				}
			break;
		}
		
		if(lastr>1)
			lastr=0; // to do with rectangle fills and the Dragon Conditional
		
		// Display the menu
		int j;
		for(j=0;j<nitems[menu];j++)
		{
			SDL_Rect loc;
			loc.x=(j==0)?732:252+(j*48);
			loc.y=528;
			SDL_BlitSurface(menubtn[menu][j], NULL, screen, &loc);
		}
		
		// Show current tool
		int tooli=0,toolj=0;
		switch(lastkey)
		{
			case 'r':
				tooli=2;toolj=1;
			break;
			case 'f':
				tooli=2;toolj=2;
			break;
			case 'd':
				tooli=2;toolj=3;
			break;
			case 'g':
				tooli=2;toolj=4;
			break;
			case 'w':
				tooli=2;toolj=5;
			break;
			case 't':
				tooli=2;toolj=6;
			break;
			case 'o':
				tooli=2;toolj=7;
			break;
			case 'a':
				tooli=3;toolj=1;
			break;
			case 'b':
				tooli=3;toolj=2;
			break;
			case 'c':
				tooli=3;toolj=3;
			break;
			case 'u':
				tooli=3;toolj=4;
			break;
			case 'p':
				tooli=3;toolj=5;
			break;
			default:
				tooli=0;
			break;
		}
		if(tooli)
		{
			SDL_Rect loc;
			loc.x=300;
			loc.y=584;
			SDL_BlitSurface(menubtn[tooli][toolj], NULL, screen, &loc);
		}
		
		// indicate current co-ordinates
		int posx=((mouse.x-280)/8)+view.x;
		int posy=((mouse.y-8)/8)+view.y;
		if((viewmode==0) && (mouse.x>=280) && (mouse.y>=8) && (mouse.x<792) && (mouse.y<520))
		{
			if(fabs(posx-ctrx)>.1) ctrx=(posx>ctrx?ctrx+.2:ctrx-.2);
			if(fabs(posy-ctry)>.1) ctry=(posy>ctry?ctry+.2:ctry-.2);
			if(fabs(posx-ctrx)>1) ctrx=(posx>ctrx?ctrx+2:ctrx-2);
			if(fabs(posy-ctry)>1) ctry=(posy>ctry?ctry+2:ctry-2);
			if(fabs(posx-ctrx)>10) ctrx=(posx>ctrx?ctrx+20:ctrx-20);
			if(fabs(posy-ctry)>10) ctry=(posy>ctry?ctry+20:ctry-20);
			if(fabs(posx-ctrx)>100) ctrx=(posx>ctrx?ctrx+200:ctrx-200);
			if(fabs(posy-ctry)>100) ctry=(posy>ctry?ctry+200:ctry-200);
			dcounter(guibits, 400, 584, max(ctrx+.001, 0), 'X');
			dcounter(guibits, 400, 600, max(ctry+.001, 0), 'Y');
		}
		else
		{
			SDL_Rect ctr={400, 584, 64, 32};
			SDL_FillRect(screen, &ctr, SDL_MapRGB(screen->format, 0, 0, 0));
		}
		dcounter(guibits, 400, 616, max(zslice+.001, 0), 'Z');
		
		// apply console overlay
		if(showconsole)
			SDL_BlitSurface(overlay, NULL, screen, &over);
		SDL_Flip(screen);
		
		dold=drag; // old buttons-state
		
		// Loop through events
		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					errupt++;
				break;
				case SDL_KEYDOWN: // All the keypresses
					if(event.key.type==SDL_KEYDOWN)
					{
						SDL_keysym key=event.key.keysym;
						if(key.sym==SDLK_LEFT) // move viewport
						{
							dview.x=(key.mod&KMOD_SHIFT)?-8:-1;
						}
						if(key.sym==SDLK_RIGHT)
						{
							dview.x=(key.mod&KMOD_SHIFT)?8:1;
						}
						if(key.sym==SDLK_DOWN)
						{
							dview.y=(key.mod&KMOD_SHIFT)?8:1;
						}
						if(key.sym==SDLK_UP)
						{
							dview.y=(key.mod&KMOD_SHIFT)?-8:-1;
						}
						int x=view.x+(mouse.x-280)/8,
							y=view.y+(mouse.y-8)/8; // x and y coords of current cursor position
						if(key.sym==SDLK_r)
						{
							if(viewmode==0)
							{
								if((x>=0) && (x<worldx) && (y>=0) && (y<worldy))
								{
									keyplace=!(map[zslice][x][y].data & TILE_ROCK);
									if(key.mod & (KMOD_LCTRL | KMOD_RCTRL))
										keyactive=true;
								}
							}
							lastkey='r';
						}
						if(key.sym==SDLK_w)
						{
							if(viewmode==0)
							{
								if((x>=0) && (x<worldx) && (y>=0) && (y<worldy))
								{
									keyplace=!(map[zslice][x][y].data & TILE_WATER);
									if(key.mod & (KMOD_LCTRL | KMOD_RCTRL))
										keyactive=true;
								}
							}
							lastkey='w';
						}
						if(key.sym==SDLK_f)
						{
							if(viewmode==0)
							{
								if((x>=0) && (x<worldx) && (y>=0) && (y<worldy))
								{
									keyplace=!(map[zslice][x][y].data & TILE_FLOOR);
									if(key.mod & (KMOD_LCTRL | KMOD_RCTRL))
										keyactive=true;
								}
							}
							lastkey='f';
						}
						if(key.sym==SDLK_g)
						{
							if(viewmode==0)
							{
								if((x>=0) && (x<worldx) && (y>=0) && (y<worldy))
								{
									keyplace=!(map[zslice][x][y].data & TILE_GRASS);
									if(key.mod & (KMOD_LCTRL | KMOD_RCTRL))
										keyactive=true;
								}
							}
							lastkey='g';
						}
						if(key.sym==SDLK_t)
						{
							if(viewmode==0)
							{
								if((x>=0) && (x<worldx) && (y>=0) && (y<worldy))
								{
									keyplace=!(map[zslice][x][y].data & TILE_STAIRS);
									if(key.mod & (KMOD_LCTRL | KMOD_RCTRL))
										keyactive=true;
								}
							}
							lastkey='t';
						}
						if(key.sym==SDLK_d)
						{
							if(viewmode==0)
							{
								if((x>=0) && (x<worldx) && (y>=0) && (y<worldy))
								{
									keyplace=!(map[zslice][x][y].data & TILE_DOOR);
									if(key.mod & (KMOD_LCTRL | KMOD_RCTRL))
										keyactive=true;
								}
							}
							lastkey='d';
						}
						if(key.sym==SDLK_o)
						{
							if(viewmode==0)
							{
								if((x>=0) && (x<worldx) && (y>=0) && (y<worldy))
								{
									keyplace=!(map[zslice][x][y].data & TILE_FORTS);
									if(key.mod & (KMOD_LCTRL | KMOD_RCTRL))
										keyactive=true;
								}
							}
							lastkey='o';
						}
						if(key.sym==SDLK_SPACE)
						{
							if(viewmode==0)
							{
								if((x>=0) && (x<worldx) && (y>=0) && (y<worldy))
								{
									keyplace=!(map[zslice][x][y].data);
									keyactive=true;
								}
							}
						}
						if((key.sym==SDLK_GREATER) || (key.sym==SDLK_PERIOD)) // Zslice - which Zlevel to edit (Editmode) / top Zlevel to show (Iso-mode)
						{
							zslice=max(zslice-1, 0);
							uslice=min(uslice, zslice);
						}
						if((key.sym==SDLK_LESS) || (key.sym==SDLK_COMMA))
						{
							zslice=min(zslice+1, levels-1);
						}
						if(key.sym==SDLK_RIGHTBRACKET) // Rotate 3d view
						{
							if(key.mod&(KMOD_LSHIFT | KMOD_RSHIFT))
								dph=1;
							else
								dth=1;
						}
						if(key.sym==SDLK_LEFTBRACKET)
						{
							if(key.mod&(KMOD_LSHIFT | KMOD_RSHIFT))
								dph=-1;
							else
								dth=-1;
						}
						if((key.sym==SDLK_PLUS) || (key.sym==SDLK_EQUALS)) // Uslice - lowest Zlevel to show in Iso/3d mode
						{
							uslice=max(uslice-1, 0);
							char string[16];
							sprintf(string, "uslice %u", uslice);
							console(screen, overlay, 8, string, small_font);
						}
						if(key.sym==SDLK_MINUS)
						{
							uslice=min(uslice+1, zslice);
							char string[16];
							sprintf(string, "uslice %u", uslice);
							console(screen, overlay, 8, string, small_font);
						}
						if(key.sym==SDLK_v)
						{
							menu=4;mdo=viewmode?1:2;
						}
						if(key.sym==SDLK_h)
						{
							showconsole=true;
							console(screen, overlay, 0, "==DF Designer Keystroke Help==", small_font);
							console(screen, overlay, 0, "h    this Help", small_font);
							console(screen, overlay, 0, ";     list colour codes", small_font);
							console(screen, overlay, 0, "s    Save map", small_font);
							console(screen, overlay, 0, "l     Load map", small_font);
							console(screen, overlay, 0, "?    toggle console/minimap", small_font);
							console(screen, overlay, 0, "< >  zslice up/down", small_font);
							console(screen, overlay, 0, "z    zslice to groundlevel", small_font);
							console(screen, overlay, 0, "cursors move viewport", small_font);
							console(screen, overlay, 0, "- +  uslice up/down", small_font);
							console(screen, overlay, 0, "r     tool Rock", small_font);
							console(screen, overlay, 0, "f     tool Floor", small_font);
							console(screen, overlay, 0, "d    tool Door", small_font);
							console(screen, overlay, 0, "g    tool Grass", small_font);
							console(screen, overlay, 0, "w    tool Water", small_font);
							console(screen, overlay, 0, "t     tool sTairs", small_font);
							console(screen, overlay, 0, "o    tool fOrtifications", small_font);
							console(screen, overlay, 0, "Ctrl-toolkey use tool at cursor", small_font);
							console(screen, overlay, 0, "spc use current tool at cursor", small_font);
							console(screen, overlay, 0, "e    Edit mode", small_font);
							console(screen, overlay, 0, "c     --COLOURS", small_font);
							console(screen, overlay, 0, "x     --DF-TILES", small_font);
							console(screen, overlay, 0, "i     Isometric mode", small_font);
							console(screen, overlay, 0, "[]     Rotate horizontal", small_font);
							console(screen, overlay, 0, "{}   Rotate vertical", small_font);
							console(screen, overlay, 0, "v     Mode toggle", small_font);
							//console(screen, overlay, 8, "", small_font);
						}
						if((key.sym==SDLK_SEMICOLON) && !(key.mod & (KMOD_LSHIFT | KMOD_RSHIFT)))
						{
							showconsole=true;
							console(screen, overlay, 8, "==DF Designer Colour Codes==", small_font);
							colconsole(screen, overlay, 8, "# Rock", small_font, 128, 128, 128);
							colconsole(screen, overlay, 8, "# Floor", small_font, 224, 224, 96);
							colconsole(screen, overlay, 8, "# Door", small_font, 224, 112, 112);
							colconsole(screen, overlay, 8, "# Stairs", small_font, 0, 112, 112);
							colconsole(screen, overlay, 8, "# Water", small_font, 0, 0, 224);
							colconsole(screen, overlay, 8, "# Grass", small_font, 96, 224, 96);
							colconsole(screen, overlay, 8, "# Fortifications", small_font, 112, 224, 192);
							//colconsole(screen, overlay, 8, "# ", small_font, );
						}
						if ((key.unicode & 0xFF80) == 0) // shortcut keys
						{
							char what = (char)key.unicode & 0x7F;
							int which = what-'0';
							if((0<=which) && (which<nitems[menu]))
							{
								if(which)
								{
									if(menu)
										mdo=which;
									else
										menu=which;
								}
								else
								{
									menu=0;
								}
							}
							else if(what!=0)
							{
								int i,j;
								for(i=0;i<nmenus;i++)
								{
									for(j=0;j<nitems[menu];j++)
									{
										if(menus[i][j].key==what)
										{
											menu=i;
											mdo=j;
											i=10;
											j=10;
										}
									}
								}
							}
						}
					}
				break;
				case SDL_KEYUP:
					if(event.key.type==SDL_KEYUP)
					{
						SDL_keysym key=event.key.keysym;
						if((key.sym==SDLK_r) || (key.sym==SDLK_w) || (key.sym==SDLK_f) || (key.sym==SDLK_g) || (key.sym==SDLK_t) || (key.sym==SDLK_d) || (key.sym==SDLK_o) || (key.sym==SDLK_SPACE))
							keyactive=false;
						if(key.sym==SDLK_LEFT)
							dview.x=max(dview.x, 0);
						if(key.sym==SDLK_RIGHT)
							dview.x=min(dview.x, 0);
						if(key.sym==SDLK_UP)
							dview.y=max(dview.y, 0);
						if(key.sym==SDLK_DOWN)
							dview.y=min(dview.y, 0);
						if((key.sym==SDLK_LEFTBRACKET) || (key.sym==SDLK_RIGHTBRACKET))
							dth=dph=0;
					}
				break;
				case SDL_MOUSEMOTION:
					mouse.x=event.motion.x;
					mouse.y=event.motion.y;
				break;
				case SDL_MOUSEBUTTONDOWN:
					mouse.x=event.button.x;
					mouse.y=event.button.y;
					button=event.button.button;
					drag|=button;
					SDL_Event makekey;
					switch(button)
					{
						case SDL_BUTTON_LEFT:
							// maparea 280, 8, 512, 512
							// menuarea 300, 528, 480, 48
							if((mouse.x>=280) && (mouse.x<792) && (mouse.y>=8) && (mouse.y<=520))
							{
								if(viewmode==0)
								{
									int x=view.x+(mouse.x-280)/8,
										y=view.y+(mouse.y-8)/8;
									if((x>=0) && (x<worldx) && (y>=0) && (y<worldy))
									{
										switch(lastkey)
										{
											case 'r':
												keyplace=!(map[zslice][x][y].data & TILE_ROCK);
												keyactive=true;
											break;
											case 'f':
												keyplace=!(map[zslice][x][y].data & TILE_FLOOR);
												keyactive=true;
											break;
											case 'd':
												keyplace=!(map[zslice][x][y].data & TILE_DOOR);
												keyactive=true;
											break;
											case 'g':
												keyplace=!(map[zslice][x][y].data & TILE_GRASS);
												keyactive=true;
											break;
											case 'w':
												keyplace=!(map[zslice][x][y].data & TILE_WATER);
												keyactive=true;
											break;
											case 't':
												keyplace=!(map[zslice][x][y].data & TILE_STAIRS);
												keyactive=true;
											break;
											case 'o':
												keyplace=!(map[zslice][x][y].data & (TILE_FORTS|TILE_ROCK));
												keyactive=true;
											break;
											case 'a':
											case 'b':
											case 'c':
											case 'p':
											case 'u':
												keyplace=!(map[zslice][x][y].data & TILE_OBJECT);
												keyactive=true;
											break;
											default:
												keyplace=true;
												keyactive=true;
											break;
										}
									}
								}
								else
								{
									keyplace=false;
								}
							}
							if((mouse.x>=300) && (mouse.x<780) && (mouse.y>=528) && (mouse.y<576))
							{
								int which=((mouse.x-252)/48)%10;
								if(which<nitems[menu])
								{
									msel=which;
								}
							}
							if(lastr)
								colconsole(screen, overlay, 8, " Cancelled fill.", small_font, 64, 144, 64);
							lastr=0;
						break;
						case SDL_BUTTON_RIGHT:
							if(lastr==0)
							{
								lastx=mouse.x;
								lasty=mouse.y;
								lastview=view;
								colconsole(screen, overlay, 8, " Filling rectangle!", small_font, 192, 144, 96);
								if((mouse.x>=280) && (mouse.x<792) && (mouse.y>=8) && (mouse.y<=520))
								{
									if(viewmode==0)
									{
										int x=view.x+(mouse.x-280)/8,
											y=view.y+(mouse.y-8)/8;
										if((x>=0) && (x<worldx) && (y>=0) && (y<worldy))
										{
											switch(lastkey)
											{
												case 'r':
													keyplace=!(map[zslice][x][y].data & TILE_ROCK);
													keyactive=true;
												break;
												case 'f':
													keyplace=!(map[zslice][x][y].data & TILE_FLOOR);
													keyactive=true;
												break;
												case 'd':
													keyplace=!(map[zslice][x][y].data & TILE_DOOR);
													keyactive=true;
												break;
												case 'g':
													keyplace=!(map[zslice][x][y].data & TILE_GRASS);
													keyactive=true;
												break;
												case 'w':
													keyplace=!(map[zslice][x][y].data & TILE_WATER);
													keyactive=true;
												break;
												case 't':
													keyplace=!(map[zslice][x][y].data & TILE_STAIRS);
													keyactive=true;
												break;
												case 'o':
													keyplace=!(map[zslice][x][y].data & (TILE_FORTS|TILE_ROCK));
													keyactive=true;
												break;
												case 'a':
												case 'b':
												case 'c':
												case 'p':
												case 'u':
													keyplace=!(map[zslice][x][y].data & TILE_OBJECT);
													keyactive=true;
												break;
												default:
													keyplace=true;
													keyactive=true;
												break;
											}
										}
									}
									else
									{
										keyplace=false;
									}
								}
							}
							else
								colconsole(screen, overlay, 8, " Filled rectangle.", small_font, 144, 224, 96);
							lastr++;
						break;
						case SDL_BUTTON_WHEELUP:
						break;
						case SDL_BUTTON_WHEELDOWN:
						break;
					}
				break;
				case SDL_MOUSEBUTTONUP:
					mouse.x=event.button.x;
					mouse.y=event.button.y;
					button=event.button.button;
					drag&=~button;
					switch(button)
					{
						case SDL_BUTTON_LEFT:
							makekey.type=SDL_KEYUP;
							makekey.key.type=SDL_KEYUP;
							makekey.key.keysym.sym=lastkey;
							SDL_PushEvent(&makekey);
							if((mouse.x>=300) && (mouse.x<780) && (mouse.y>=528) && (mouse.y<576))
							{
								int which=((mouse.x-252)/48)%10;
								if(which==msel)
								{
									if(menu==0)
									{
										menu=msel;
										msel=-1;
									}
									else if(msel==0)
									{
										menu=0;
										msel=-1;
									}
									else
									{
										mdo=msel;
									}
								}
							}
							keyactive=false;
						break;
						case SDL_BUTTON_RIGHT:
							makekey.type=SDL_KEYUP;
							makekey.key.type=SDL_KEYUP;
							makekey.key.keysym.sym=lastkey;
							SDL_PushEvent(&makekey);
						break;
						case SDL_BUTTON_WHEELUP:
						break;
						case SDL_BUTTON_WHEELDOWN:
						break;
					}
				break;
			}
		}
		
		// Menu actions
		if(mdo!=-1)
		{
			switch((10*menu)+mdo)
			{
				case 11: // New/clear
				{
					bool clear=true;
					if(confirms)
					{
						char * boxtextlines[] = {"Are you sure you want to clear the map?", "(This cannot be undone)"};
						clear=ynbox(screen, box_small, boxtextlines, 2, small_button_u, small_button_p, small_font, small_font, "Yes, Go Ahead!", "No, Hold On!", 0, 24, 48, 96, 48, 96, 192);
					}
					if(clear)
					{
						int e=clear_map(map, false, worldx, worldy, levels, groundlevel);
						if(e==2)
							return(2);
					}
				}
				break;
				case 12: // Load
				{
					bool load=true;
					if(confirms)
					{
						char * boxtextlines[] = {"Are you sure you want to load a map?", "(Don't forget to save your old map first)"};
						load=ynbox(screen, box_small, boxtextlines, 2, small_button_u, small_button_p, small_font, small_font, "Yes, Go Ahead!", "No, Hold On!", 0, 24, 48, 96, 48, 96, 192);
					}
					if(load)
					{
						char * boxtextlines[] = {"Load Map:", "Enter a filename (max 28 chars)"};
						char * filename = textentry(screen, box_small, boxtextlines, 2, small_font, 192, 224, 255);
						if(*filename==0)
						{
							colconsole(screen, overlay, 20, "Load cancelled", small_font, 128, 104, 32);
						}
						else
						{
							int e=load_map(filename, &map, guibits, &worldx, &worldy, &levels, &groundlevel, &zslice, &uslice, &view, &dview);
							if(e==2)
								return(2);
						}
						free(filename);
					}
				}
				break;
				case 13: // Save
				{
					char * boxtextlines[] = {"Save Map:", "Enter a filename (max 28 chars)", "Blank to cancel"};
					char * filename = textentry(screen, box_small, boxtextlines, 3, small_font, 192, 224, 255);
					if(*filename==0)
					{
						colconsole(screen, overlay, 20, "Save cancelled", small_font, 128, 104, 32);
					}
					else
					{
						save_map(filename, map, guibits, worldx, worldy, levels, groundlevel);
					}
					free(filename);
				}
				break;
				case 14: // Export
				{
					char * boxtextlines[] = {"Export Design:", "Enter a filename (max 28 chars)", "Blank to cancel"};
					char * filename = textentry(screen, box_small, boxtextlines, 3, small_font, 192, 224, 255);
					if(*filename==0)
					{
						colconsole(screen, overlay, 20, "Export cancelled", small_font, 128, 104, 32);
					}
					else
					{
						export_map(filename, map, guibits, worldx, worldy, levels, groundlevel, false);
					}
					free(filename);
				}
				break;
				case 15: // QF-Export
				{
					char * boxtextlines[] = {"Export Quickfort:", "Enter a filename (max 28 chars)", ".csv extension will be added", "automatically.  Blank to cancel"};
					char * filename = textentry(screen, box_small, boxtextlines, 4, small_font, 192, 224, 255);
					if(*filename==0)
					{
						colconsole(screen, overlay, 20, "Export cancelled", small_font, 128, 104, 32);
					}
					else
					{
						char csvfile[strlen(filename)+5];
						sprintf(csvfile, "%s.csv", filename);
						free(filename);
						export_map(csvfile, map, guibits, worldx, worldy, zslice, groundlevel, true);
					}
				}
				break;
				case 16: // Quit
				{
					bool quit=true;
					if(confirms)
					{
						char * boxtextlines[] = {"Are you sure you want to ***QUIT***?", "(Don't forget to save your work)"};
						quit=ynbox(screen, box_small, boxtextlines, 2, small_button_u, small_button_p, small_font, small_font, "Yes, I'm Outta Here!", "No, Hold On!", 0, 24, 48, 96, 48, 96, 192);
					}
					if(quit)
						errupt++;
				}
				break;
				case 21:
					lastkey='r';
				break;
				case 22:
					lastkey='f';
				break;
				case 23:
					lastkey='d';
				break;
				case 24:
					lastkey='g';
				break;
				case 25:
					lastkey='w';
				break;
				case 26:
					lastkey='t';
				break;
				case 27:
					lastkey='o';
				break;
				case 31:
					lastkey='a';
				break;
				case 32:
					lastkey='b';
				break;
				case 33:
					lastkey='c';
				break;
				case 34:
					lastkey='u';
				break;
				case 35:
					lastkey='p';
				break;
				case 41: // Edit-mode
					viewmode=0;
					console(screen, overlay, 8, "Editing mode selected", small_font);
					switch(editmode)
					{
						case 0:
							console(screen, overlay, 8, "Edit-mode COLOURS selected", small_font);
							if(semislice)
								console(screen, overlay, 8, "shadowing OFF", small_font);
							else
								console(screen, overlay, 8, "shadowing ON", small_font);
						break;
						case 1:
							console(screen, overlay, 8, "Edit-mode DF-TILES selected", small_font);
						break;
						default:
							console(screen, overlay, 8, "Don't know what edit-mode this is... error!", small_font);
						break;
					}
				break;
				case 42: // Iso/3D-mode
					viewmode=1;
					console(screen, overlay, 8, "Isometric View mode selected", small_font);
					if(semislice)
						console(screen, overlay, 8, "semislice ON", small_font);
					else
						console(screen, overlay, 8, "semislice OFF", small_font);
				break;
				case 43: // Edit-Colours
					editmode=0;
					console(screen, overlay, 8, "Edit-mode COLOURS selected", small_font);
					if(semislice)
						console(screen, overlay, 8, "shadowing OFF", small_font);
					else
						console(screen, overlay, 8, "shadowing ON", small_font);
				break;
				case 44: // Edit-DFTILES
					editmode=1;
					console(screen, overlay, 8, "Edit-mode DF-TILES selected", small_font);
				break;
				case 45: // Toggle Console
					showconsole=!showconsole;
				break;
				case 46: // Shadowing/Semislice
					semislice=!semislice;
					if(viewmode==1)
					{
						if(semislice)
							console(screen, overlay, 8, "semislice ON", small_font);
						else
							console(screen, overlay, 8, "semislice OFF", small_font);
					}
					else
					{
						if(semislice)
							console(screen, overlay, 8, "shadowing OFF", small_font);
						else
							console(screen, overlay, 8, "shadowing ON", small_font);
					}
				break;
				case 52:
				case 54:
				{
					bool copy=true;
					bool shft=(mdo==4);
					if(confirms)
					{
						char * boxtextlines[] = {"Copy from z-level below --", "are you sure?", "There is no undo!"};
						if(shft)
							boxtextlines[0]="Shift up one z-level --";
						copy=ynbox(screen, box_small, boxtextlines, 3, small_button_u, small_button_p, small_font, big_font, "Yes, Do it!", "No, Stop!", 0, 24, 48, 96, 48, 96, 192);
					}
					if(copy)
					{
						colconsole(screen, overlay, 8, shft?"Shifting up one z-level...":"Copying from z-level below...", small_font, 96, 96, 96);
						int x,y,z;
						tile fill={TILE_ROCK,0};
						for(z=shft?levels-2:zslice;z>=(shft?0:zslice);z--)
						{
							for(x=0;x<worldx;x++)
							{
								for(y=0;y<worldy;y++)
								{
									map[z][x][y]=(z==0)?fill:map[z-1][x][y]; // If there are ever pointers, we'll have to watch out for this :S
								}
							}
						}
						if(shft)
							groundlevel++;
						colconsole(screen, overlay, 20, "Done!", small_font, 32, 128, 32);
					}
					else
					{
						colconsole(screen, overlay, 20, "Z-level copy cancelled.", small_font, 128, 104, 32);
					}
				}
				break;
				case 51:
				case 53:
				{
					bool copy=true;
					bool shft=(mdo==3);
					if(confirms)
					{
						char * boxtextlines[] = {"Copy from z-level above --", "are you sure?", "There is no undo!"};
						if(shft)
							boxtextlines[0]="Shift down one z-level --";
						copy=ynbox(screen, box_small, boxtextlines, 3, small_button_u, small_button_p, small_font, big_font, "Yes, Do it!", "No, Stop!", 0, 24, 48, 96, 48, 96, 192);
					}
					if(copy)
					{
						colconsole(screen, overlay, 8, shft?"Shifting down one z-level...":"Copying from z-level above...", small_font, 96, 96, 96);
						int x,y,z;
						tile fill={0,0};
						for(z=shft?1:zslice;z<(shft?levels:zslice+1);z++)
						{
							for(x=0;x<worldx;x++)
							{
								for(y=0;y<worldy;y++)
								{
									map[z][x][y]=(z==levels-1)?fill:map[z+1][x][y]; // If there are ever pointers, we'll have to watch out for this :S
								}
							}
						}
						if(shft)
							groundlevel--;
						colconsole(screen, overlay, 20, "Done!", small_font, 32, 128, 32);
					}
					else
					{
						colconsole(screen, overlay, 20, "Z-level copy cancelled.", small_font, 128, 104, 32);
					}
				}
				break;
				case 55: // Count Materials
				{
					console(screen, overlay, 8, "Counting materials...", small_font);
					int walls=0,floors=0,doors=0,stairs=0,beds=0,chairs=0,tables=0,statues=0;
					int x,y,z;
					for(z=0;z<levels;z++)
					{
						for(y=0;y<worldy;y++)
						{
							for(x=0;x<worldx;x++)
							{
								int here=map[z][x][y].data;
								if((z>=groundlevel) && (here & (TILE_ROCK|TILE_FORTS)))
									walls++;
								else if(here & TILE_FLOOR)
									floors++;
								else if(here & TILE_DOOR)
									doors++;
								else if(here & TILE_STAIRS)
									stairs++;
								if(here & TILE_OBJECT)
								{
									int object=map[z][x][y].object;
									if(object==OBJECT_BED)
										beds++;
									else if(object==OBJECT_CHAIR)
										chairs++;
									else if(object==OBJECT_TABLE)
										tables++;
									else if(object==OBJECT_STATUE)
										statues++;
								}
							}
						}
					}
					char string[100];
					sprintf(string, " %u walls, %u floors, %u doors, %u stairs", walls, floors, doors, stairs);
					colconsole(screen, overlay, 8, string, small_font, 255, 255, 255);
					sprintf(string, " %u beds, %u chairs, %u tables, %u statues", beds, chairs, tables, statues);
					colconsole(screen, overlay, 8, string, small_font, 255, 255, 255);
				}
				break;
				case 56: // Ground to Zslice
					groundlevel=zslice;
				break;
				case 57: // About
				{
					showconsole=true;
					colconsole(screen, overlay, 20, "About DF Designer", small_font, 255, 255, 255);
					char string[100];
					sprintf(string, "Version %hhu.%hhu.%hhu", VERSION_MAJ, VERSION_MIN, VERSION_REV); // TODO: VERSION_GIT
					colconsole(screen, overlay, 20, string, small_font, 127, 255, 127);
					FILE *fp=fopen("init/credits", "r");
					while(!feof(fp))
					{
						char *line=getl(fp);
						colconsole(screen, overlay, 20, line, small_font, 127, 127, 255);
						free(line);
					}
					fclose(fp);
				}
				break;
				default:
					fprintf(stderr, "Error - menuitem %d/%d undefined\n", menu, mdo);
				break;
			}
		}
		
		mdo=-1;
		// Apply viewport movement / 3dview rotation
		view.x=max(min(view.x+dview.x, worldx-1), 0);
		view.y=max(min(view.y+dview.y, worldy-1), 0);
		ths+=dth;
		phs+=dph;
		
		SDL_Delay(18);
	}

	// clean up
	TTF_CloseFont(small_font);
	TTF_CloseFont(big_font);
	if(SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
	return(0);
}

int console(SDL_Surface * screen, SDL_Surface * overlay, int delay, char * text, TTF_Font * font) // shortcut for a simple grey console entry
{
	return(colconsole(screen, overlay, delay, text, font, 192, 192, 192));
}

// Writes a message to the scrolly console, in the given colour
int colconsole(SDL_Surface * screen, SDL_Surface * overlay, int delay, char * text, TTF_Font * font, unsigned char r, unsigned char g, unsigned char b)
{
	if(SDL_MUSTLOCK(overlay))
		SDL_LockSurface(overlay);
	dtext(overlay, 8, CONSOLE_HEIGHT, text, font, r, g, b);
	SDL_Rect cls;
	cls.x=0;
	cls.y=OSIZ_Y-CONSOLE_HEIGHT;
	cls.w=OSIZ_X;
	cls.h=CONSOLE_HEIGHT+20;
	if(showconsole)
	{
		int i,y;
		for(i=0;i<16;i+=4)
		{
			for(y=0;y+4<CONSOLE_HEIGHT+20;y++)
			{
				memcpy((char *)overlay->pixels+(y*overlay->pitch), (char *)overlay->pixels+((y+4)*overlay->pitch), overlay->pitch);
			}
			if(delay)
			{
				SDL_BlitSurface(overlay, NULL, screen, &cls);
				SDL_Flip(screen);
				SDL_Delay(delay);
			}
		}
		if(SDL_MUSTLOCK(overlay))
			SDL_UnlockSurface(overlay);
		SDL_BlitSurface(overlay, NULL, screen, &cls);
		SDL_Flip(screen);
	}
	else
	{
		int y;
		for(y=0;y<CONSOLE_HEIGHT;y++)
		{
			memcpy((char *)overlay->pixels+(y*overlay->pitch), (char *)overlay->pixels+((y+16)*overlay->pitch), overlay->pitch);
		}
		for(y=CONSOLE_HEIGHT;y<CONSOLE_HEIGHT+20;y++)
		{
			memset((char *)overlay->pixels+(y*overlay->pitch), 0, overlay->pitch);
		}
		if(SDL_MUSTLOCK(overlay))
			SDL_UnlockSurface(overlay);
	}
	return(0);
}

// Tile preprocessor, produces an ASCII/DF-TILE
disp_tile tchar(tile ***map, int x, int y, int z, int worldx, int worldy, int groundlevel)
{
	disp_tile ret=wb;
	if(map[z][x][y].data&TILE_OBJECT)
	{
		int object=map[z][x][y].object;
		switch(object)
		{
			case OBJECT_BED:
				ret.v=233;
				ret.fr=96; ret.fg=64; ret.fb=0;
			break;
			case OBJECT_CHAIR:
				ret.v=210;
			break;
			case OBJECT_TABLE:
				ret.v=209;
			break;
			case OBJECT_STATUE:
				ret.v=234;
			break;
			case OBJECT_STKPILE:
				ret.v='=';
				ret.fr=ret.fg=160; ret.fb=192;
			break;
			default:
				ret=err;
			break;
		}
	}
	else if(map[z][x][y].data&TILE_ROCK)
	{	
		if(z>=groundlevel) // We have some fun matching up walls to the directions they've got to connect
		{
			int dirs=0;
			if(x>0 && map[z][x-1][y].data&(TILE_ROCK|TILE_DOOR|TILE_FORTS))
			{	
				dirs|=1;
			}
			if(x<worldx-1 && map[z][x+1][y].data&(TILE_ROCK|TILE_DOOR|TILE_FORTS))
			{	
				dirs|=2;
			}
			if(y>0 && map[z][x][y-1].data&(TILE_ROCK|TILE_DOOR|TILE_FORTS))
			{	
				dirs|=4;
			}
			if(y<worldy-1 && map[z][x][y+1].data&(TILE_ROCK|TILE_DOOR|TILE_FORTS))
			{	
				dirs|=8;
			}
			switch(dirs)
			{
				case 0:
				case 1:
				case 2:
				case 4:
				case 8:
					ret.v='O';
				break;
				case 3:
					ret.v=205;
				break;
				case 5:
					ret.v=188;
				break;
				case 6:
					ret.v=200;
				break;
				case 7:
					ret.v=202;
				break;
				case 9:
					ret.v=187;
				break;
				case 10:
					ret.v=201;
				break;
				case 11:
					ret.v=203;
				break;
				case 12:
					ret.v=186;
				break;
				case 13:
					ret.v=185;
				break;
				case 14:
					ret.v=204;
				break;
				case 15:
					ret.v=206;
				break;
				default:
					ret.v='O';
				break;
			}
		}
		else
			ret.v=178;
	}
	else if(map[z][x][y].data&TILE_FORTS)
	{
		ret.v=206;
		ret.fr=72;ret.fg=ret.fb=172;
		ret.br=128;ret.bg=ret.bb=224;
	}
	else if(map[z][x][y].data&TILE_FLOOR)
	{
		ret.v='+';
		ret.fr=ret.fg=255;ret.fb=128;
	}
	else if(map[z][x][y].data&TILE_DOOR)
	{
		ret.v=197;
		ret.fr=255;ret.fg=ret.fb=72;
	}
	else if(map[z][x][y].data&TILE_GRASS)
	{
		ret.v=',';
		ret.fr=0;ret.fg=255;ret.fb=0;
		ret.br=32;ret.bg=24;ret.bb=0;
	}
	else if(map[z][x][y].data&TILE_WATER)
	{
		ret.v=247;
		ret.fr=0;ret.fg=64;ret.fb=224;
		ret.br=0;ret.bg=32;ret.bb=64;
	}
	else if(map[z][x][y].data&TILE_STAIRS)
	{
		ret.fr=0;ret.fg=192;ret.fb=224;
		ret.br=0;ret.bg=64;ret.bb=32;
		if((z>0) && (map[z-1][x][y].data&TILE_STAIRS))
			ret.v='X';
		else
			ret.v='<';
	}
	else if(map[z][x][y].data==0)
	{
		if(z>0)
		{
			if(map[z-1][x][y].data&TILE_STAIRS)
			{
				ret.v='>';
				ret.fr=0;ret.fg=192;ret.fb=224;
				ret.br=0;ret.bg=64;ret.bb=32;
			}
			else if(map[z-1][x][y].data&TILE_WATER)
			{
				ret.v='~';
				ret.fr=0;ret.fg=64;ret.fb=224;
				ret.br=0;ret.bg=32;ret.bb=64;
			}
			else if(map[z-1][x][y].data&TILE_FLOOR)
			{
				ret.v=250;
				ret.fr=ret.fg=255;ret.fb=0;
			}
			else if(map[z-1][x][y].data&TILE_GRASS)
			{
				ret.v=250;
				ret.fr=0;
				ret.fg=255;
				ret.fb=0;
			}
			else if((map[z-1][x][y].data&(TILE_ROCK|TILE_FORTS)) || z<groundlevel)
			{
				ret.v='+';
				ret.fr=ret.fg=ret.fb=192;
				ret.br=ret.bg=ret.bb=64;
			}
			else if(map[z-1][x][y].data==0)
			{
				ret.v=176;
				ret.fr=64;ret.fg=96;ret.fb=224;
				ret.br=ret.bg=ret.bb=32;
			}
			else
			{
				ret.v=250;
			}
		}
		else
		{
			ret.v='+';
			ret.fr=ret.fg=ret.fb=192;
			ret.br=ret.bg=ret.bb=64;
		}
	}
	else
	{
		ret=err;
	}
	return(ret);
}

int load_map(char *filename, tile ****map, gui guibits, int *worldx, int *worldy, int *levels, int *groundlevel, int *zslice, int *uslice, pos *view, pos *dview)
{
	char string[100];
	sprintf(string, "Loading from: %s...", filename);
	fprintf(stderr, "%s\n", string);
	console(guibits.screen, guibits.overlay, 20, string, guibits.small_font);
	FILE * fp=fopen(filename, "r");
	if(fp==NULL)
	{
		fprintf(stderr, "Couldn't open file for reading!\n");
		perror("fopen");
		colconsole(guibits.screen, guibits.overlay, 20, "Couldn't open file for reading!", guibits.small_font, 224, 192, 96);
	}
	else
	{
		bool ok=true;
		char id[4];
		fread(id, 1, 4, fp);
		if(strncmp(id, "DFDM", 4)!=0)
		{
			fprintf(stderr, "File corrupted, or not a DFD Map!\n");
			colconsole(guibits.screen, guibits.overlay, 20, "File corrupted, or not a DFD Map!", guibits.small_font, 224, 192, 96);
			ok=false;
		}
		else
		{
			unsigned char va,vb,vc,vn;
			va=fgetc(fp);
			vb=fgetc(fp);
			vc=fgetc(fp);
			vn=fgetc(fp);
			if(vn!='\n')
			{
				fprintf(stderr, "File corrupted, or not a DFD Map!\n");
				colconsole(guibits.screen, guibits.overlay, 20, "File corrupted, or not a DFD Map!", guibits.small_font, 224, 192, 96);
				ok=false;
			}
			else
			{
				char vermsg[32];
				sprintf(vermsg, "  File is version %hhu.%hhu.%hhu", va, vb, vc);
				fprintf(stderr, "%s\n", vermsg);
				colconsole(guibits.screen, guibits.overlay, 20, vermsg, guibits.small_font, 96, 96, 96);
				int x,y,z;
				for(z=0;z<*levels;z++)
				{
					for(x=0;x<*worldx;x++)
					{
						free((*map)[z][x]);
					}
					free((*map)[z]);
				}
				if((va>0) || (vb>=9))
					fscanf(fp, "%d,%d,%d,%d\n", levels, worldx, worldy, groundlevel);
				else
					fscanf(fp, "%d,%d,%d\n", levels, worldx, worldy);
				*uslice=max(*groundlevel-1, 0);
				(*map) = (tile ***)realloc((*map), *levels*sizeof(tile **));
				if((*map)==NULL)
				{
					fprintf(stderr, "Memory exhausted.\n");
					char * boxtext[] = {"Memory exhausted - ", "map load failed."};
					okbox(guibits.screen, guibits.box_small, boxtext, 2, guibits.small_button_u, guibits.small_button_p, guibits.small_font, guibits.big_font, "Quit", 192, 224, 255, 0, 255, 0);
					return(2);
				}
				else
				{
					for(z=0;z<*levels;z++)
					{
						(*map)[z] = (tile **)malloc(*worldx*sizeof(tile *));
						if((*map)[z]==NULL)
						{
							fprintf(stderr, "Not enough mem / couldn't alloc!\n");
							return(2);
						}
						for(x=0;x<*worldx;x++)
						{
							(*map)[z][x] = (tile *)malloc(*worldy*sizeof(tile));
							if((*map)[z][x]==NULL)
							{
								fprintf(stderr, "Not enough mem / couldn't alloc!\n");
								return(2);
							}
						}
					}
				}
				for(z=0;z<*levels;z++)
				{
					for(y=0;y<*worldy;y++)
					{
						for(x=0;x<*worldx;x++)
						{
							if(feof(fp))
								ok=false;
							(*map)[z][x][y].data=ok?fgetc(fp):0;
							if(feof(fp))
								ok=false;
							(*map)[z][x][y].object=ok?fgetc(fp):0;
						}
					}
					if(ok)
					{
						char nl=fgetc(fp);
						if(nl!='\n')
						{
							fprintf(stderr, "File corrupted!  Map only partially loaded\n");
							colconsole(guibits.screen, guibits.overlay, 20, "File corrupted!  Map only partially loaded", guibits.small_font, 224, 0, 0);
							ok=false;
						}
					}
				}
			}
		}
		fclose(fp);
		if(ok)
		{
			colconsole(guibits.screen, guibits.overlay, 20, "Loaded successfully!", guibits.small_font, 96, 224, 96);
			*zslice=*groundlevel;
			view->x=max(0, (*worldx-64)/2);
			view->y=max(0, (*worldy-64)/2);
			dview->x=dview->y=0;
			return(0);
		}
	}
	return(1);
}

int save_map(char *filename, tile ***map, gui guibits, int worldx, int worldy, int levels, int groundlevel)
{
	char string[100];
	sprintf(string, "Saving to: %s...", filename);
	fprintf(stderr, "%s\n", string);
	console(guibits.screen, guibits.overlay, 20, string, guibits.small_font);
	FILE * fp=fopen(filename, "w");
	if(fp==NULL)
	{
		fprintf(stderr, "Couldn't open file for writing!\n");
		perror("fopen");
		colconsole(guibits.screen, guibits.overlay, 20, "Couldn't open file for writing!", guibits.small_font, 224, 192, 96);
		return(2);
	}
	else
	{
		fprintf(fp, "DFDM%c%c%c\n", VERSION_MAJ, VERSION_MIN, VERSION_REV);
		fprintf(fp, "%u,%u,%u,%u\n", levels, worldx, worldy, groundlevel);
		int x,y,z;
		for(z=0;z<levels;z++)
		{
			for(y=0;y<worldy;y++)
			{
				for(x=0;x<worldx;x++)
				{
					fputc(map[z][x][y].data, fp);
					fputc(map[z][x][y].object, fp);
				}
			}
			fputc('\n', fp);
		}
		fclose(fp);
		colconsole(guibits.screen, guibits.overlay, 20, "Saved successfully!", guibits.small_font, 96, 224, 96);
	}
	return(0);
}

int export_map(char *filename, tile ***map, gui guibits, int worldx, int worldy, int levels, int groundlevel, bool qf)
{
	int zslice=levels; // if qf==true then we store zslice in levels else we don't need zslice
	char string[100];
	if(qf)
		sprintf(string, "yxport: Exporting current zslice to: %s...", filename);
	else
		sprintf(string, "Exporting to: %s...", filename);
	fprintf(stderr, "%s\n", string);
	console(guibits.screen, guibits.overlay, 20, string, guibits.small_font);
	FILE * fp=fopen(filename, "w");
	if(fp==NULL)
	{
		fprintf(stderr, "Couldn't open file for writing!\n");
		perror("fopen");
		colconsole(guibits.screen, guibits.overlay, 20, "Couldn't open file for writing!", guibits.small_font, 224, 192, 96);
		return(2);
	}
	else
	{
		if(qf)
		{
			if(zslice>=groundlevel)
				fprintf(fp, "#build Generated by DF Designer %hhu.%hhu.%hhu\n", VERSION_MAJ, VERSION_MIN, VERSION_REV);
			else
			{
				colconsole(guibits.screen, guibits.overlay, 20, "yxport: Underground exports not done yet!  Sorry", guibits.small_font, 224, 160, 96);
				fclose(fp);
				return(4);
			}
		}
		else
			fprintf(fp, "Generated by DF Designer %hhu.%hhu.%hhu\n", VERSION_MAJ, VERSION_MIN, VERSION_REV);
		int x,y,z;
		int nx=worldx,ny=worldy,nz=levels,mx=0,my=0,mz=0;
		bool same[levels];
		for(z=qf?zslice:0;qf?z<levels:z==zslice;z++)
		{
			if((!qf) && (z>0))
			{
				same[z]=true;
				for(y=0;(y<worldy) && same[z];y++)
				{
					for(x=0;(x<worldx) && same[z];x++)
					{
						if(map[z][x][y].data!=map[z-1][x][y].data)
							same[z]=false;
						else if((map[z][x][y].data & TILE_OBJECT) && (map[z][x][y].object!=map[z-1][x][y].object))
							same[z]=false;
					}
				}
			}
			else
				same[z]=false;
			for(y=0;y<worldy;y++)
			{
				for(x=0;x<worldx;x++)
				{
					int here=map[z][x][y].data;
					if(z>groundlevel)
					{
						if(here!=0)
						{
							nx=min(nx, x);
							ny=min(ny, y);
							nz=min(nz, z);
							mx=max(mx, x);
							my=max(my, y);
							mz=max(mz, z);
						}
					}
					else if(z==groundlevel)
					{
						if(here&~TILE_GRASS)
						{
							nx=min(nx, x);
							ny=min(ny, y);
							nz=min(nz, z);
							mx=max(mx, x);
							my=max(my, y);
							mz=max(mz, z);
						}
					}
					else
					{
						if(here&~TILE_ROCK)
						{
							nx=min(nx, x);
							ny=min(ny, y);
							nz=min(nz, z);
							mx=max(mx, x);
							my=max(my, y);
							mz=max(mz, z);
						}
					}
				}
			}
		}
		if(qf)
			nz=mz=zslice;
		same[nz]=false; // we need to ensure that we at least get something
		if(!qf)
			fputc('\n', fp);
		bool hitstock=false;
		for(z=mz;z>=nz;z--)
		{
			if(!qf)
				fprintf(fp, "Z-level %d\n", z-groundlevel);
			if(!same[z])
			{
				for(y=ny;y<=my;y++)
				{
					if(!qf)
						fputc('\n', fp);
					for(x=nx;x<=mx;x++)
					{
						if(qf)
						{
							int here=map[zslice][x][y].data;
							char *qfbuild;
							if(here&TILE_ROCK)
							{
								qfbuild="Cw";
							}
							else if(here&TILE_FLOOR)
							{
								qfbuild="Cf";
							}
							else if(here&TILE_DOOR)
							{
								qfbuild="d";
							}
							else if(here&TILE_STAIRS)
							{
								disp_tile xtile = tchar(map, x, y, zslice, worldx, worldy, groundlevel);
								switch(xtile.v)
								{
									case '>':
										qfbuild="Cd";
									break;
									case '<':
										qfbuild="Cu";
									break;
									case 'X':
										qfbuild="Cx";
									break;
									default:
										fprintf(stderr, "yxport: A staircase could not be properly deduced\n");
										colconsole(guibits.screen, guibits.overlay, 20, "yxport: A staircase could not be properly deduced", guibits.small_font, 160, 160, 128);
										qfbuild="Cx";
									break;
								}
							}
							else if(here&TILE_FORTS)
							{
								qfbuild="CF";
							}
							else if(here&TILE_OBJECT)
							{
								switch(map[zslice][x][y].object)
								{
									case OBJECT_BED:
										qfbuild="b";
									break;
									case OBJECT_CHAIR:
										qfbuild="c";
									break;
									case OBJECT_TABLE:
										qfbuild="t";
									break;
									case OBJECT_STATUE:
										qfbuild="s";
									break;
									case OBJECT_STKPILE:
										qfbuild="`";
										if(!hitstock)
										{
											fprintf(stderr, "yxport: stockpiles not yet supported (ignored)\n");
											colconsole(guibits.screen, guibits.overlay, 20, "yxport: stockpiles not yet supported (ignored)", guibits.small_font, 160, 160, 128);
										}
										hitstock=true;
									break;
									default:
										qfbuild="`";
										fprintf(stderr, "yxport: unrecognised object %d (ignored)\n", map[zslice][x][y].object);
										colconsole(guibits.screen, guibits.overlay, 20, "yxport: unrecognised object (ignored)", guibits.small_font, 160, 160, 128);
									break;
								}
							}
							else
							{
								qfbuild="`";
							}
							fprintf(fp, "%s,", qfbuild);
						}
						else
						{
							disp_tile xtile = tchar(map, x, y, z, worldx, worldy, groundlevel);
							if(xtile.v>127)
								fprintf(fp, "%s", xatiles[xtile.v-128]);
							else
								fputc(xtile.v, fp);
						}
					}
					if(qf)
						fprintf(fp, "#\n");
				}
				if(!qf)
				{
					fputc('\n', fp);
					fputc('\n', fp);
				}
			}
		}
		fclose(fp);
		if(qf)
			colconsole(guibits.screen, guibits.overlay, 20, "yxport: Exported successfully!", guibits.small_font, 96, 32, 96);
		else
			colconsole(guibits.screen, guibits.overlay, 20, "Exported successfully!", guibits.small_font, 96, 32, 96);
	}
	return(0);
}

int clear_map(tile ***map, bool alloc, int worldx, int worldy, int levels, int groundlevel)
{
	int x,y,z;
	for(z=0;z<levels;z++)
	{
		if(alloc)
			map[z] = (tile **)malloc(worldx*sizeof(tile *));
		if(map[z]==NULL)
		{
			fprintf(stderr, "Not enough mem / couldn't alloc!\n");
			return(2);
		}
		for(x=0;x<worldx;x++)
		{
			if(alloc)
				map[z][x] = (tile *)malloc(worldy*sizeof(tile));
			if(map[z]==NULL)
			{
				fprintf(stderr, "Not enough mem / couldn't alloc!\n");
				return(2);
			}
			for(y=0;y<worldy;y++)
			{
				map[z][x][y].data=(z<groundlevel)?TILE_ROCK:(z==groundlevel)?TILE_GRASS:0;
				map[z][x][y].object=0;
			}
		}
	}
	return(0);
}

char * getl(FILE *fp)
{
	// gets a line of string data, {re}alloc()ing as it goes, so you don't need to make a buffer for it, nor must thee fret thyself about overruns!
	char * lout = (char *)malloc(81);
	int i=0;
	signed int c;
	while(!feof(fp))
	{
		c=fgetc(fp);
		if((c==10)||(c==EOF))
			break;
		if(c!=0)
		{
			lout[i++]=c;
			if((i%80)==0)
			{
				if((lout=(char *)realloc(lout, i+81))==NULL)
				{
					printf("\nNot enough memory to store input!\n");
					free(lout);
					return(NULL);
				}
			}
		}
	}
	lout[i]=0;
	char *nlout=(char *)realloc(lout, i+1);
	if(nlout==NULL)
	{
		return(lout); // it doesn't really matter (assuming realloc is a decent implementation and hasn't nuked the original pointer), we'll just have to temporarily waste a bit of memory
	}
	return(nlout);
}
