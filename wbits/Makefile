CC := i586-mingw32msvc-gcc
CFLAGS := -Wall -Wextra -Werror -pedantic -std=gnu99 -Wformat=0
SDL := -I/usr/i586-mingw32msvc/include -L. -lm -lSDL_image SDL_image.dll -lSDL_ttf SDL_ttf.dll -I/usr/i586-mingw32msvc/include/SDL `/usr/i586-mingw32msvc/bin/sdl-config --cflags --libs`
BINDIR := bin/
SRCDIR := src/
INCDIR := inc/
LIBDIR := lib/
FNTDIR := font/
IMGDIR := img/
INITDIR := init/
CPPFLAGS := -I$(INCDIR)
FLAGS := $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)
DGFX := -DOSIZ_X=800 -DOSIZ_Y=640 -DOBPP=32
DFONT := -DFONT_FILE=\"$(FNTDIR)Vera.ttf\"

all: designer.exe

designer.exe: $(SRCDIR)designer.c $(LIBDIR)draw.dll $(INCDIR)draw.h $(LIBDIR)dialogs.dll $(INCDIR)dialogs.h $(LIBDIR)map.dll $(INCDIR)map.h $(LIBDIR)bits.dll $(INCDIR)bits.h  $(INCDIR)version.h
	-mkdir $(BINDIR)
	$(CC) $(FLAGS) $(SDL) -o designer.exe $(SRCDIR)designer.c $(LIBDIR)draw.dll $(LIBDIR)dialogs.dll $(LIBDIR)map.dll $(LIBDIR)bits.dll -lm $(DGFX) $(DFONT)

$(LIBDIR)draw.dll: $(SRCDIR)draw.c $(INCDIR)draw.h
	-mkdir $(LIBDIR)
	$(CC) $(FLAGS) $(SDL) -o $(LIBDIR)draw.dll -c $(SRCDIR)draw.c $(DGFX)

$(LIBDIR)dialogs.dll: $(SRCDIR)dialogs.c $(INCDIR)dialogs.h
	-mkdir $(LIBDIR)
	$(CC) $(FLAGS) $(SDL) -o $(LIBDIR)dialogs.dll -c $(SRCDIR)dialogs.c

$(LIBDIR)map.dll: $(SRCDIR)map.c $(INCDIR)map.h $(INCDIR)draw.h $(INCDIR)dialogs.h $(INCDIR)bits.h $(INCDIR)version.h
	-mkdir $(LIBDIR)
	$(CC) $(FLAGS) -o $(LIBDIR)map.dll -c $(SRCDIR)map.c

$(LIBDIR)bits.dll: $(SRCDIR)bits.c $(INCDIR)bits.h
	-mkdir $(LIBDIR)
	$(CC) $(FLAGS) -o $(LIBDIR)bits.dll -c $(SRCDIR)bits.c

