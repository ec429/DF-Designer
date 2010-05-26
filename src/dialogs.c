#include "../inc/dialogs.h"

int okbox(SDL_Surface * screen, SDL_Surface * boximage, char ** boxtext, int boxtextlines, SDL_Surface * button_u, SDL_Surface * button_p, TTF_Font * font, TTF_Font * buttonfont, char * buttontext, char r, char g, char b, char br, char bg, char bb)
{
	pos mouse;
	char button;
	SDL_Event event;
	SDL_Rect cls;
	cls.w=boximage->w;
	cls.h=boximage->h;
	cls.x=(screen->w-cls.w)/2;
	cls.y=(screen->h-cls.h)/2;
	int errupt=0;
	char drag=0, dold=drag;
	bool pressed=false;
	while(!errupt)
	{
		SDL_Flip(screen);
		SDL_FillRect(screen, &cls, SDL_MapRGB(screen->format, 0, 0, 0));
		SDL_BlitSurface(boximage, NULL, screen, &cls);
		int y;
		for(y=0;y<boxtextlines;y++)
		{
			dtext(screen, cls.x+32, cls.y+32+(12*y), boxtext[y], font, r, g, b);
		}
		SDL_Surface * button_w = pressed?button_p:button_u;
		SDL_Rect rcDest = {cls.x+((boximage->w-button_w->w)/2), cls.y+(boximage->h-button_w->h-8), 0, 0};
		SDL_BlitSurface(button_w, NULL, screen, &rcDest);
		SDL_Color clrFg = {br, bg, bb, 0};
		SDL_Surface *sText = TTF_RenderText_Solid(buttonfont, buttontext, clrFg);
		rcDest.x=cls.x+((boximage->w-sText->w)/2)+(pressed?3:0);
		rcDest.y+=8+(pressed?2:0);
		SDL_BlitSurface(sText, NULL, screen, &rcDest);
		SDL_FreeSurface(sText);
		pressed=false;
		if((mouse.x-cls.x>=(boximage->w-button_w->w)/2) && (mouse.x-cls.x<=(boximage->w+button_w->w)/2) && (mouse.y-cls.y>=(boximage->h-button_w->h-8) && (mouse.y-cls.y<=boximage->h-8)))
		{
			if(drag & SDL_BUTTON_LEFT)
			{
				pressed=true;
			}
			else if(dold & SDL_BUTTON_LEFT)
			{
				errupt++;
			}
		}
		dold=drag;
		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					errupt++;
				break;
				case SDL_KEYDOWN:
					if(event.key.type==SDL_KEYDOWN)
					{
						SDL_keysym key=event.key.keysym;
						if(key.sym==SDLK_RETURN)
						{
							errupt++;
						}
						/*
						the ascii character is:
						if ((key.unicode & 0xFF80) == 0)
						{
							// it's (char)keysym.unicode & 0x7F;
						}
						else
						{
							// it's not [low] ASCII
						}
						*/
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
				break;
				case SDL_MOUSEBUTTONUP:
					mouse.x=event.button.x;
					mouse.y=event.button.y;
					button=event.button.button;
					drag&=~button;
				break;
			}
		}
		SDL_Delay(18);
	}
	return(0);
}

bool ynbox(SDL_Surface * screen, SDL_Surface * boximage, char ** boxtext, int boxtextlines, SDL_Surface * button_u, SDL_Surface * button_p, TTF_Font * font, TTF_Font * buttonfont, char * ytext, char * ntext, bool def, char r, char g, char b, char br, char bg, char bb)
{
	pos mouse;
	char button;
	SDL_Event event;
	SDL_Rect cls;
	cls.w=boximage->w;
	cls.h=boximage->h;
	cls.x=(screen->w-cls.w)/2;
	cls.y=(screen->h-cls.h)/2;
	int errupt=0;
	char drag=0, dold=drag;
	bool ypressed=false,npressed=false;
	while(!errupt)
	{
		SDL_Flip(screen);
		SDL_FillRect(screen, &cls, SDL_MapRGB(screen->format, 0, 0, 0));
		SDL_BlitSurface(boximage, NULL, screen, &cls);
		int y;
		for(y=0;y<boxtextlines;y++)
		{
			dtext(screen, cls.x+32, cls.y+32+(12*y), boxtext[y], font, r, g, b);
		}
		SDL_Surface * button_w = ypressed?button_p:button_u;
		SDL_Rect rcDest = {cls.x+((boximage->w-(button_w->w*2.2))/2), cls.y+(boximage->h-button_w->h-8), 0, 0};
		SDL_BlitSurface(button_w, NULL, screen, &rcDest);
		SDL_Color clrFg = {br, bg, bb, 0};
		SDL_Surface *sText = TTF_RenderText_Solid(buttonfont, ytext, clrFg);
		rcDest.x=cls.x+((boximage->w-(button_w->w*1.2)-sText->w)/2)+(ypressed?3:0);
		rcDest.y+=8+(ypressed?2:0);
		SDL_BlitSurface(sText, NULL, screen, &rcDest);
		rcDest.y-=8+(ypressed?2:0);
		SDL_FreeSurface(sText);
		button_w = npressed?button_p:button_u;
		rcDest.x=cls.x+((boximage->w+(button_w->w*0.2))/2);
		SDL_BlitSurface(button_w, NULL, screen, &rcDest);
		sText = TTF_RenderText_Solid(buttonfont, ntext, clrFg);
		rcDest.x=cls.x+((boximage->w+(button_w->w*1.2)-sText->w)/2)+(npressed?3:0);
		rcDest.y+=8+(npressed?2:0);
		SDL_BlitSurface(sText, NULL, screen, &rcDest);
		SDL_FreeSurface(sText);
		ypressed=npressed=false;
		if((mouse.x-cls.x>=(boximage->w-button_w->w*2.2)/2) && (mouse.x-cls.x<=(boximage->w-button_w->w*0.2)/2) && (mouse.y-cls.y>=(boximage->h-button_w->h-8) && (mouse.y-cls.y<=boximage->h-8)))
		{
			if(drag & SDL_BUTTON_LEFT)
			{
				ypressed=true;
			}
			else if(dold & SDL_BUTTON_LEFT)
			{
				ypressed=true;
				errupt++;
			}
		}
		else if((mouse.x-cls.x>=(boximage->w+button_w->w*0.5)/2) && (mouse.x-cls.x<=(boximage->w+button_w->w*1.5)/2) && (mouse.y-cls.y>=(boximage->h-button_w->h-8) && (mouse.y-cls.y<=boximage->h-8)))
		{
			if(drag & SDL_BUTTON_LEFT)
			{
				npressed=true;
			}
			else if(dold & SDL_BUTTON_LEFT)
			{
				npressed=true;
				errupt++;
			}
		}
		dold=drag;
		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					errupt++;
				break;
				case SDL_KEYDOWN:
					if(event.key.type==SDL_KEYDOWN)
					{
						SDL_keysym key=event.key.keysym;
						if(key.sym==SDLK_RETURN)
						{
							if(def)
								ypressed=true;
							else
								npressed=true;
							errupt++;
						}
						if(key.sym==SDLK_y)
						{
							ypressed=true;
							errupt++;
						}
						if(key.sym==SDLK_n)
						{
							npressed=true;
							errupt++;
						}
						/*
						the ascii character is:
						if ((key.unicode & 0xFF80) == 0)
						{
							// it's (char)keysym.unicode & 0x7F;
						}
						else
						{
							// it's not [low] ASCII
						}
						*/
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
				break;
				case SDL_MOUSEBUTTONUP:
					mouse.x=event.button.x;
					mouse.y=event.button.y;
					button=event.button.button;
					drag&=~button;
				break;
			}
		}
		SDL_Delay(18);
	}
	return(ypressed);
}

char * textentry(SDL_Surface * screen, SDL_Surface * boximage, char ** boxtext, int boxtextlines, TTF_Font * font, char r, char g, char b)
{
	SDL_EnableUNICODE(1);
	char * inputtext=(char *)malloc(30);
	int inptr=0;
	inputtext[inptr]=0;
	pos mouse;
	char button;
	SDL_Event event;
	SDL_Rect cls;
	cls.w=boximage->w;
	cls.h=boximage->h;
	cls.x=(screen->w-cls.w)/2;
	cls.y=(screen->h-cls.h)/2;
	int errupt=0;
	while(!errupt)
	{
		SDL_FillRect(screen, &cls, SDL_MapRGB(screen->format, 0, 0, 0));
		SDL_BlitSurface(boximage, NULL, screen, &cls);
		int y;
		for(y=0;y<boxtextlines;y++)
		{
			dtext(screen, cls.x+32, cls.y+32+(12*y), boxtext[y], font, r, g, b);
		}
		dtext(screen, cls.x+32, cls.y+36+(12*boxtextlines), inputtext, font, r, g, b);
		SDL_Flip(screen);
		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					errupt++;
				break;
				case SDL_KEYDOWN:
					if(event.key.type==SDL_KEYDOWN)
					{
						SDL_keysym key=event.key.keysym;
						if(key.sym==SDLK_RETURN)
						{
							errupt++;
						}
						else if(key.sym==SDLK_BACKSPACE)
						{
							if(inptr>0)
								inputtext[--inptr]=0;
						} 
						else if(((key.unicode & 0xFF80) == 0) && (key.unicode!=0))
						{
							if(inptr<28)
							{
								inputtext[inptr]=(char)key.unicode & 0xFF;
								inputtext[++inptr]=0;
							}
						}
						else
						{
							// it's not ASCII :S
							fprintf(stderr, "ASCII only!\n");
						}
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
				break;
				case SDL_MOUSEBUTTONUP:
					mouse.x=event.button.x;
					mouse.y=event.button.y;
					button=event.button.button;
				break;
			}
		}
		SDL_Delay(18);
	}
	return(inputtext);
}

