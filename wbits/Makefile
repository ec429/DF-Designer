CC = i586-mingw32msvc-gcc
CFLAGS ?= -Wall
SDL = -I/usr/i586-mingw32msvc/include -L. -lm -lSDL_image SDL_image.dll -lSDL_ttf SDL_ttf.dll -I/usr/i586-mingw32msvc/include/SDL `/usr/i586-mingw32msvc/bin/sdl-config --cflags --libs`
BINDIR = bin/
SRCDIR = src/
INCDIR = inc/
LIBDIR = lib/
DGFX = -DOSIZ_X=800 -DOSIZ_Y=640 -DOBPP=32
DFONT = -DFONT_FILE=\"font/Vera.ttf\"

all: designer.exe

designer.exe: $(BINDIR)designer.exe
	cp $(BINDIR)designer.exe designer.exe

$(BINDIR)designer.exe: $(SRCDIR)designer.c $(LIBDIR)draw.dll $(INCDIR)draw.h $(LIBDIR)dialogs.dll $(INCDIR)dialogs.h $(INCDIR)437.h
	$(CC) $(CFLAGS) -o $(BINDIR)designer.exe $(SRCDIR)designer.c $(LIBDIR)draw.dll $(LIBDIR)dialogs.dll -lm $(DGFX) $(DFONT) $(SDL)

$(LIBDIR)draw.dll: $(SRCDIR)draw.c $(INCDIR)draw.h
	$(CC) $(CFLAGS) -o $(LIBDIR)draw.dll -c $(SRCDIR)draw.c $(DGFX) $(SDL)

$(LIBDIR)dialogs.dll: $(SRCDIR)dialogs.c $(INCDIR)dialogs.h
	$(CC) $(CFLAGS) -o $(LIBDIR)dialogs.dll -c $(SRCDIR)dialogs.c $(SDL)

