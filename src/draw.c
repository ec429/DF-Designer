//	designer - third party architectural design utility for 'Dwarf Fortress'
//	Copyright (C) 2010-11 Edward Cree (see top of src/designer.c for license details)
//   src/draw.c - provides drawing-functions

#include "draw.h"

#define max(a,b)	((a)>(b)?(a):(b))
#define min(a,b)	((a)<(b)?(a):(b))

SDL_Surface * gf_init(int x, int y)
{
	c3=sqrt(3)/2.0;
	SDL_Surface *screen;
	if(SDL_Init(SDL_INIT_VIDEO|SDL_DOUBLEBUF)<0)
	{
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return(NULL);
	}
	if(!(screen=SDL_SetVideoMode(x, y, OBPP, SDL_HWSURFACE|SDL_ASYNCBLIT|SDL_HWACCEL)))
	{
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		SDL_Quit();
		return(NULL);
	}
	atexit(SDL_Quit);
	return(screen);
}

int pset(SDL_Surface * screen, int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	if((0<=x) && (x<screen->w) && (0<=y) && (y<screen->h))
	{
		if(SDL_MUSTLOCK(screen) && SDL_LockSurface(screen) < 0)
		{
			fprintf(stderr, "SDL_LockSurface: %s\n", SDL_GetError());
			return(2);
		}
		unsigned long int pixval = SDL_MapRGB(screen->format, r, g, b);
		unsigned char *pixloc = (unsigned char *)screen->pixels + y*screen->pitch + x*screen->format->BytesPerPixel;
		*(unsigned long int *)pixloc = pixval;
		if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
		return(0);
	}
	return(1);
}

int line(SDL_Surface * screen, int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b)
{
	if(x2<x1)
	{
		int _t=x1;
		x1=x2;
		x2=_t;
		_t=y1;
		y1=y2;
		y2=_t;
	}
	int dy=y2-y1,
		dx=x2-x1;
	if(dx==0)
	{
		int cy;
		for(cy=y1;(dy>0)?cy-y2:y2-cy<=0;cy+=(dy>0)?1:-1)
		{
			pset(screen, x1, cy, r, g, b);
		}
	}
	else if(dy==0)
	{
		int cx;
		for(cx=x1;cx<=x2;cx++)
		{
			pset(screen, cx, y1, r, g, b);
		}
	}
	else
	{
		double m = (double)dy/(double)dx;
		int cx=x1, cy=y1;
		if(m>0)
		{
			while(cx<x2)
			{
				do {
					pset(screen, cx, cy, r, g, b);
					cx++;
				} while((((cx-x1) * m)<(cy-y1)) && cx<x2);
				do {
					pset(screen, cx, cy, r, g, b);
					cy++;
				} while((((cx-x1) * m)>(cy-y1)) && cy<y2);
			}
		}
		else
		{
			while(cx<x2)
			{
				do {
					pset(screen, cx, cy, r, g, b);
					cx++;
				} while((((cx-x1) * m)>(cy-y1)) && cx<x2);
				do {
					pset(screen, cx, cy, r, g, b);
					cy--;
				} while((((cx-x1) * m)<(cy-y1)) && cy>y2);
			}
		}
	}
	return(0);
}

int dtext(SDL_Surface * scrn, int x, int y, char * text, TTF_Font * font, unsigned char r, unsigned char g, unsigned char b)
{
	SDL_Color clrFg = {r, g, b,0};
	SDL_Rect rcDest = {x, y, OSIZ_X, 36};
	//SDL_FillRect(screen, &rcDest, SDL_MapRGB(screen->format, BACK_COLOUR));
	SDL_Surface *sText = TTF_RenderText_Solid(font, text, clrFg);
	SDL_BlitSurface(sText, NULL, scrn, &rcDest);
	SDL_FreeSurface(sText);
	return(0);
}

int dmenu(SDL_Surface * screen, int x, int y, int items, int pressed, int hover, char ** text, TTF_Font * font, SDL_Surface * button_u, SDL_Surface * button_p, unsigned char r, unsigned char g, unsigned char b, unsigned char hr, unsigned char hg, unsigned char hb)
{
	int i;
	for(i=0;i<items;i++)
	{
		SDL_Surface * button = (i==pressed)?button_p:button_u;
		SDL_Rect rcDest = {x-(button->w/2), y, 0, 0};
		SDL_BlitSurface(button, NULL, screen, &rcDest);
		SDL_Color clrFg = {(i==hover)?hr:r, (i==hover)?hg:g, (i==hover)?hb:b, 0};
		SDL_Surface *sText = TTF_RenderText_Solid(font, text[i], clrFg);
		rcDest.x=x-(sText->w/2);
		rcDest.y+=4;
		SDL_BlitSurface(sText, NULL, screen, &rcDest);
		SDL_FreeSurface(sText);
		y+=(button->h)+8;
	}
	return(0);
}

int dcounter(gui guibits, int x, int y, double val, char what)
{
	int i;
	for(i=0;i<3;i++)
	{
		double num=val*pow(10, i-2);
		num=floor(num)-10*floor(num/10);
		SDL_Rect numeral={0, num*16, 16, 16};
		SDL_Rect dest={x+(i*16), y, 16, 16};
		SDL_BlitSurface(guibits.counter, &numeral, guibits.screen, &dest);
	}
	char text[2]={what, 0};
	dtext(guibits.screen, x+50, y, text, guibits.small_font, 160, 160, 160);
	return(0);
}

cparms cuboid(int dx, int dy, int dz, double theta, double phi)
{
	cparms rv;
	double vertex[3][3]; // vertices joined to the base point
	vertex[0][0]=dy*sin(theta);vertex[0][1]=dy*cos(theta)*cos(phi);vertex[0][2]=-dy*cos(theta)*sin(phi);
	vertex[1][0]=dx*cos(theta);vertex[1][1]=-dx*sin(theta)*cos(phi);vertex[1][2]=dx*sin(theta)*sin(phi);
	vertex[2][0]=0;vertex[2][1]=dz*sin(phi);vertex[2][2]=dz*cos(phi);
	
	rv.vertex[0].x=0;rv.vertex[0].y=0;
	int i;
	for(i=1;i<7;i++)
	{
		double x=0,y=0,z=0;
		if((i==1)||(i==4)||(i==6))
		{
			x+=vertex[0][0];
			y+=vertex[0][1];
			z+=vertex[0][2];
		}
		if((i==2)||(i==5)||(i==6))
		{
			x+=vertex[1][0];
			y+=vertex[1][1];
			z+=vertex[1][2];
		}
		if(i>2)
		{
			x+=vertex[2][0];
			y+=vertex[2][1];
			z+=vertex[2][2];
		}
		rv.vertex[i].x=x;
		rv.vertex[i].y=-z;
	}
	rv.face[0]=192;//floor(-wedge(vertex[0], vertex[2])*96.0)+159; /* With the randomisation, we don't really need this kind of accuracy, */
	rv.face[1]=192;//floor(-wedge(vertex[1], vertex[2])*96.0)+159; /* which is never visible anyway.  Also the quarter-rotation stuff     */
	rv.face[2]=168;//floor(-wedge(vertex[0], vertex[1])*96.0)+159; /* breaks it, so we may as well simplify (and get more speed).         */
	return(rv);
}

double wedge(double a[3], double b[3])
{
	double x,y,z;
	x=a[2]*b[3]-a[3]*b[2];
	y=a[3]*b[1]-a[1]*b[3];
	z=a[1]*b[2]-a[2]*b[1];
	//fprintf(stderr, "wedge %g\n", (x+y+z)/(3*sqrt((x*x)+(y*y)+(z*z))));
	return((x+(3*y)+(2*z))/(14*sqrt((x*x)+(y*y)+(z*z))));
}

int dcuboid(SDL_Surface * screen, int x, int y, cparms cbd, unsigned char r, unsigned char g, unsigned char b)
{
	int dx;
	if(cbd.vertex[3].y)
	{
		for(dx=0;dx<abs(cbd.vertex[1].x);dx++)
		{
			int sdx,x1,y1;
			sdx=(cbd.vertex[1].x>0)?dx:-dx;
			x1=x+sdx;
			y1=y+(sdx*cbd.vertex[1].y)/cbd.vertex[1].x;
			line(screen, x1, y1, x1, y1+cbd.vertex[3].y, (r*cbd.face[0])/256, (g*cbd.face[0])/256, (b*cbd.face[0])/256);
		}
		for(dx=0;dx<abs(cbd.vertex[2].x);dx++)
		{
			int sdx,x1,y1;
			sdx=(cbd.vertex[2].x>0)?dx:-dx;
			x1=x+sdx;
			y1=y+(sdx*cbd.vertex[2].y)/cbd.vertex[2].x;
			line(screen, x1, y1, x1, y1+cbd.vertex[3].y, (r*cbd.face[1])/256, (g*cbd.face[1])/256, (b*cbd.face[1])/256);
		}
	}
	for(dx=-abs(cbd.vertex[4].x);dx<=abs(cbd.vertex[5].x);dx++) // [4].x and [5].x always have opposite signs (assuming the cparms are right)
	{
		int sdx,x1,y1,fdx,y2;
		sdx=(cbd.vertex[5].x>0)?dx:-dx;
		x1=x+sdx;
		y1=y+cbd.vertex[3].y+min((sdx*cbd.vertex[1].y)/(cbd.vertex[1].x+.01), (sdx*cbd.vertex[2].y)/(cbd.vertex[2].x+.01));
		fdx=cbd.vertex[6].x-sdx;
		y2=y+cbd.vertex[6].y-min((fdx*cbd.vertex[1].y)/(cbd.vertex[1].x+.01), (fdx*cbd.vertex[2].y)/(cbd.vertex[2].x+.01));
		if(y2<y1)
			line(screen, x1, y1, x1, y2, (r*cbd.face[2])/256, (g*cbd.face[2])/256, (b*cbd.face[2])/256);
	}
	return(0);
}

int console(gui guibits, int delay, char *text)
{
	return(colconsole(guibits, delay, text, 192, 192, 192));
}

int colconsole(gui guibits, int delay, char *text, unsigned char r, unsigned char g, unsigned char b)
{
	if(SDL_MUSTLOCK(guibits.overlay))
		SDL_LockSurface(guibits.overlay);
	dtext(guibits.overlay, 8, CONSOLE_HEIGHT, text, guibits.small_font, r, g, b);
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
				memcpy((char *)guibits.overlay->pixels+(y*guibits.overlay->pitch), (char *)guibits.overlay->pixels+((y+4)*guibits.overlay->pitch), guibits.overlay->pitch);
			}
			if(delay)
			{
				SDL_BlitSurface(guibits.overlay, NULL, guibits.screen, &cls);
				SDL_Flip(guibits.screen);
				SDL_Delay(delay);
			}
		}
		if(SDL_MUSTLOCK(guibits.overlay))
			SDL_UnlockSurface(guibits.overlay);
		SDL_BlitSurface(guibits.overlay, NULL, guibits.screen, &cls);
		SDL_Flip(guibits.screen);
	}
	else
	{
		int y;
		for(y=0;y<CONSOLE_HEIGHT;y++)
		{
			memcpy((char *)guibits.overlay->pixels+(y*guibits.overlay->pitch), (char *)guibits.overlay->pixels+((y+16)*guibits.overlay->pitch), guibits.overlay->pitch);
		}
		for(y=CONSOLE_HEIGHT;y<CONSOLE_HEIGHT+20;y++)
		{
			memset((char *)guibits.overlay->pixels+(y*guibits.overlay->pitch), 0, guibits.overlay->pitch);
		}
		if(SDL_MUSTLOCK(guibits.overlay))
			SDL_UnlockSurface(guibits.overlay);
	}
	return(0);
}
