CC = i586-mingw32msvc-gcc
CFLAGS ?= -Wall
SDL = -I/usr/i586-mingw32msvc/include -L. -lm -lSDL_image SDL_image.dll -lSDL_ttf SDL_ttf.dll -I/usr/i586-mingw32msvc/include/SDL `/usr/i586-mingw32msvc/bin/sdl-config --cflags --libs`
BINDIR = bin/
SRCDIR = src/
INCDIR = inc/
LIBDIR = lib/
FNTDIR = font/
IMGDIR = img/
INITDIR = init/
DGFX = -DOSIZ_X=800 -DOSIZ_Y=640 -DOBPP=32
DFONT = -DFONT_FILE=\"$(FNTDIR)Vera.ttf\"

all: designer.exe

$(INCDIR)version.h: $(SRCDIR)designer.c $(SRCDIR)draw.c $(INCDIR)draw.h $(SRCDIR)dialogs.c $(INCDIR)dialogs.h $(INCDIR)437.h
	bash ./gitversion

designer.exe: $(SRCDIR)designer.c $(LIBDIR)draw.dll $(INCDIR)draw.h $(LIBDIR)dialogs.dll $(INCDIR)dialogs.h $(INCDIR)437.h $(INCDIR)version.h
	-mkdir $(BINDIR)
	$(CC) $(CFLAGS) $(SDL) -o designer.exe $(SRCDIR)designer.c $(LIBDIR)draw.dll $(LIBDIR)dialogs.dll -lm $(DGFX) $(DFONT)

$(LIBDIR)draw.dll: $(SRCDIR)draw.c $(INCDIR)draw.h
	-mkdir $(LIBDIR)
	$(CC) $(CFLAGS) $(SDL) -o $(LIBDIR)draw.dll -c $(SRCDIR)draw.c $(DGFX)

$(LIBDIR)dialogs.dll: $(SRCDIR)dialogs.c $(INCDIR)dialogs.h
	-mkdir $(LIBDIR)
	$(CC) $(CFLAGS) $(SDL) -o $(LIBDIR)dialogs.dll -c $(SRCDIR)dialogs.c

