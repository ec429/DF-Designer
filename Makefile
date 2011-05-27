CC := gcc
CFLAGS := -Wall -Wextra -Werror -pedantic -std=gnu99
SDL := `sdl-config --cflags --libs` -lSDL_image -lSDL_ttf
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
VERSION := `git describe --tags`

all: designer $(BINDIR)designer

designer: $(BINDIR)designer
	-ln $(BINDIR)designer designer --symbolic

$(INCDIR)version.h: $(SRCDIR)designer.c $(SRCDIR)draw.c $(INCDIR)draw.h $(SRCDIR)dialogs.c $(INCDIR)dialogs.h $(SRCDIR)map.c $(INCDIR)map.h $(SRCDIR)bits.c $(INCDIR)bits.h 
	bash ./gitversion

$(BINDIR)designer: $(SRCDIR)designer.c $(LIBDIR)draw.o $(INCDIR)draw.h $(LIBDIR)dialogs.o $(INCDIR)dialogs.h $(LIBDIR)map.o $(INCDIR)map.h $(LIBDIR)bits.o $(INCDIR)bits.h  $(INCDIR)version.h
	-mkdir $(BINDIR)
	$(CC) $(FLAGS) $(SDL) -o $(BINDIR)designer $(SRCDIR)designer.c $(LIBDIR)draw.o $(LIBDIR)dialogs.o $(LIBDIR)map.o $(LIBDIR)bits.o -lm $(DGFX) $(DFONT)

$(LIBDIR)draw.o: $(SRCDIR)draw.c $(INCDIR)draw.h
	-mkdir $(LIBDIR)
	$(CC) $(FLAGS) $(SDL) -o $(LIBDIR)draw.o -c $(SRCDIR)draw.c $(DGFX)

$(LIBDIR)dialogs.o: $(SRCDIR)dialogs.c $(INCDIR)dialogs.h
	-mkdir $(LIBDIR)
	$(CC) $(FLAGS) $(SDL) -o $(LIBDIR)dialogs.o -c $(SRCDIR)dialogs.c

$(LIBDIR)map.o: $(SRCDIR)map.c $(INCDIR)map.h $(INCDIR)draw.h $(INCDIR)dialogs.h $(INCDIR)bits.h $(INCDIR)version.h
	-mkdir $(LIBDIR)
	$(CC) $(FLAGS) -o $(LIBDIR)map.o -c $(SRCDIR)map.c $(DGFX)

$(LIBDIR)bits.o: $(SRCDIR)bits.c $(INCDIR)bits.h
	-mkdir $(LIBDIR)
	$(CC) $(FLAGS) -o $(LIBDIR)bits.o -c $(SRCDIR)bits.c

dist: all
	mkdir dfdesigner_$(VERSION)
	cp -r $(BINDIR) dfdesigner_$(VERSION)/$(BINDIR)
	cp -r $(SRCDIR) dfdesigner_$(VERSION)/$(SRCDIR)
	cp -r $(INCDIR) dfdesigner_$(VERSION)/$(INCDIR)
	cp -r $(LIBDIR) dfdesigner_$(VERSION)/$(LIBDIR)
	cp -r $(FNTDIR) dfdesigner_$(VERSION)/$(FNTDIR)
	cp -r $(IMGDIR) dfdesigner_$(VERSION)/$(IMGDIR)
	cp -r $(INITDIR) dfdesigner_$(VERSION)/$(INITDIR)
	cp Makefile dfdesigner_$(VERSION)
	cp readme dfdesigner_$(VERSION)
	-ln $(BINDIR)designer dfdesigner_$(VERSION)/designer --symbolic
	tar -cvvf dfdesigner_$(VERSION).tar dfdesigner_$(VERSION)/
	gzip -9 dfdesigner_$(VERSION).tar

distw:
	-mkdir dfdw_$(VERSION)
	cp -r $(SRCDIR) dfdw_$(VERSION)/$(SRCDIR)
	cp -r $(INCDIR) dfdw_$(VERSION)/$(INCDIR)
	cp -r $(LIBDIR) dfdw_$(VERSION)/$(LIBDIR)
	cp -r $(FNTDIR) dfdw_$(VERSION)/$(FNTDIR)
	cp -r $(IMGDIR) dfdw_$(VERSION)/$(IMGDIR)
	cp -r $(INITDIR) dfdw_$(VERSION)/$(INITDIR)
	cp wbits/Makefile dfdw_$(VERSION)
	for p in `ls wbits`; do cp wbits/$$p dfdw_$(VERSION); done;
	cp readme dfdw_$(VERSION)
	-rm dfdw_$(VERSION)/$(LIBDIR)*.o
	make -C dfdw_$(VERSION) -fMakefile all
