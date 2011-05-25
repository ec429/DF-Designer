//	designer - third party architectural design utility for 'Dwarf Fortress'
//	Copyright (C) 2010-11 Edward Cree (see top of src/designer.c for license details)
//   src/map.h - provides map manipulation functions

#include "map.h"

// Useful tile definitions (blank and error)
disp_tile wb={' ', 255, 255, 255, 0, 0, 0}, err={'!', 255, 0, 0, 0, 0, 192};

disp_tile tchar(tile ***map, int x, int y, int z)
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

int load_map(char *filename, tile ****map, gui guibits, int *zslice, int *uslice, pos *view, pos *dview)
{
	char string[100];
	sprintf(string, "Loading from: %s...", filename);
	fprintf(stderr, "%s\n", string);
	console(guibits, 20, string);
	FILE * fp=fopen(filename, "r");
	if(fp==NULL)
	{
		fprintf(stderr, "Couldn't open file for reading!\n");
		perror("fopen");
		colconsole(guibits, 20, "Couldn't open file for reading!", 224, 192, 96);
	}
	else
	{
		bool ok=true;
		char id[4];
		fread(id, 1, 4, fp);
		if(strncmp(id, "DFDM", 4)!=0)
		{
			fprintf(stderr, "File corrupted, or not a DFD Map!\n");
			colconsole(guibits, 20, "File corrupted, or not a DFD Map!", 224, 192, 96);
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
				colconsole(guibits, 20, "File corrupted, or not a DFD Map!", 224, 192, 96);
				ok=false;
			}
			else
			{
				char vermsg[32];
				sprintf(vermsg, "  File is version %hhu.%hhu.%hhu", va, vb, vc);
				fprintf(stderr, "%s\n", vermsg);
				colconsole(guibits, 20, vermsg, 96, 96, 96);
				int x,y,z;
				for(z=0;z<levels;z++)
				{
					for(x=0;x<worldx;x++)
					{
						free((*map)[z][x]);
					}
					free((*map)[z]);
				}
				if((va>0) || (vb>=9))
					fscanf(fp, "%d,%d,%d,%d\n", &levels, &worldx, &worldy, &groundlevel);
				else
					fscanf(fp, "%d,%d,%d\n", &levels, &worldx, &worldy);
				*uslice=max(groundlevel-1, 0);
				(*map) = (tile ***)realloc((*map), levels*sizeof(tile **));
				if((*map)==NULL)
				{
					fprintf(stderr, "Memory exhausted.\n");
					char * boxtext[] = {"Memory exhausted - ", "map load failed."};
					okbox(guibits.screen, guibits.box_small, boxtext, 2, guibits.small_button_u, guibits.small_button_p, guibits.small_font, guibits.big_font, "Quit", 192, 224, 255, 0, 255, 0);
					return(2);
				}
				else
				{
					for(z=0;z<levels;z++)
					{
						(*map)[z] = (tile **)malloc(worldx*sizeof(tile *));
						if((*map)[z]==NULL)
						{
							fprintf(stderr, "Not enough mem / couldn't alloc!\n");
							return(2);
						}
						for(x=0;x<worldx;x++)
						{
							(*map)[z][x] = (tile *)malloc(worldy*sizeof(tile));
							if((*map)[z][x]==NULL)
							{
								fprintf(stderr, "Not enough mem / couldn't alloc!\n");
								return(2);
							}
						}
					}
				}
				for(z=0;z<levels;z++)
				{
					for(y=0;y<worldy;y++)
					{
						for(x=0;x<worldx;x++)
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
							colconsole(guibits, 20, "File corrupted!  Map only partially loaded", 224, 0, 0);
							ok=false;
						}
					}
				}
			}
		}
		fclose(fp);
		if(ok)
		{
			colconsole(guibits, 20, "Loaded successfully!", 96, 224, 96);
			*zslice=groundlevel;
			view->x=max(0, (worldx-64)/2);
			view->y=max(0, (worldy-64)/2);
			dview->x=dview->y=0;
			return(0);
		}
	}
	return(1);
}

int save_map(char *filename, tile ***map, gui guibits)
{
	char string[100];
	sprintf(string, "Saving to: %s...", filename);
	fprintf(stderr, "%s\n", string);
	console(guibits, 20, string);
	FILE * fp=fopen(filename, "w");
	if(fp==NULL)
	{
		fprintf(stderr, "Couldn't open file for writing!\n");
		perror("fopen");
		colconsole(guibits, 20, "Couldn't open file for writing!", 224, 192, 96);
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
		colconsole(guibits, 20, "Saved successfully!", 96, 224, 96);
	}
	return(0);
}

int export_map(char *filename, tile ***map, gui guibits, bool qf)
{
	int zslice=levels; // if qf==true then we store zslice in levels else we don't need zslice
	char string[100];
	if(qf)
		sprintf(string, "yxport: Exporting current zslice to: %s...", filename);
	else
		sprintf(string, "Exporting to: %s...", filename);
	fprintf(stderr, "%s\n", string);
	console(guibits, 20, string);
	FILE * fp=fopen(filename, "w");
	if(fp==NULL)
	{
		fprintf(stderr, "Couldn't open file for writing!\n");
		perror("fopen");
		colconsole(guibits, 20, "Couldn't open file for writing!", 224, 192, 96);
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
				colconsole(guibits, 20, "yxport: Underground exports not done yet!  Sorry", 224, 160, 96);
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
								disp_tile xtile = tchar(map, x, y, zslice);
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
										colconsole(guibits, 20, "yxport: A staircase could not be properly deduced", 160, 160, 128);
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
											colconsole(guibits, 20, "yxport: stockpiles not yet supported (ignored)", 160, 160, 128);
										}
										hitstock=true;
									break;
									default:
										qfbuild="`";
										fprintf(stderr, "yxport: unrecognised object %d (ignored)\n", map[zslice][x][y].object);
										colconsole(guibits, 20, "yxport: unrecognised object (ignored)", 160, 160, 128);
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
							disp_tile xtile = tchar(map, x, y, z);
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
			colconsole(guibits, 20, "yxport: Exported successfully!", 96, 32, 96);
		else
			colconsole(guibits, 20, "Exported successfully!", 96, 32, 96);
	}
	return(0);
}

int clear_map(tile ***map, bool alloc)
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
