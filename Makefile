CC ?= gcc
CFLAGS ?= -Wall
SDL = `sdl-config --cflags --libs` -lSDL_image -lSDL_ttf
BINDIR = bin/
SRCDIR = src/
INCDIR = inc/
LIBDIR = lib/
FNTDIR = font/
IMGDIR = img/
INITDIR = init/
DGFX = -DOSIZ_X=800 -DOSIZ_Y=640 -DOBPP=32
DFONT = -DFONT_FILE=\"$(FNTDIR)Vera.ttf\"

all: $(BINDIR)designer

designer: $(BINDIR)designer
	-ln $(BINDIR)designer designer --symbolic

$(BINDIR)designer: $(SRCDIR)designer.c $(LIBDIR)draw.o $(INCDIR)draw.h $(LIBDIR)dialogs.o $(INCDIR)dialogs.h $(INCDIR)437.h $(INCDIR)version.h
	-mkdir $(BINDIR)
	$(CC) $(CFLAGS) $(SDL) -o $(BINDIR)designer $(SRCDIR)designer.c $(LIBDIR)draw.o $(LIBDIR)dialogs.o -lm $(DGFX) $(DFONT)

$(LIBDIR)draw.o: $(SRCDIR)draw.c $(INCDIR)draw.h
	-mkdir $(LIBDIR)
	$(CC) $(CFLAGS) $(SDL) -o $(LIBDIR)draw.o -c $(SRCDIR)draw.c $(DGFX)

$(LIBDIR)dialogs.o: $(SRCDIR)dialogs.c $(INCDIR)dialogs.h
	-mkdir $(LIBDIR)
	$(CC) $(CFLAGS) $(SDL) -o $(LIBDIR)dialogs.o -c $(SRCDIR)dialogs.c

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
	-rm dfdesigner_$(VERSION)/$(BINDIR)*~
	-rm dfdesigner_$(VERSION)/$(SRCDIR)*~
	-rm dfdesigner_$(VERSION)/$(INCDIR)*~
	-rm dfdesigner_$(VERSION)/$(LIBDIR)*~
	-rm dfdesigner_$(VERSION)/$(FNTDIR)*~
	-rm dfdesigner_$(VERSION)/$(IMGDIR)*~
	-rm dfdesigner_$(VERSION)/$(INITDIR)*~
	-rm dfdesigner_$(VERSION)/*~
	tar -cvvf dfdesigner_$(VERSION).tar dfdesigner_$(VERSION)/
	gzip -9 dfdesigner_$(VERSION).tar

distw:
	-mkdir dfdw_$(VERSION)
	cp -r $(BINDIR) dfdw_$(VERSION)/$(BINDIR)
	cp -r $(SRCDIR) dfdw_$(VERSION)/$(SRCDIR)
	cp -r $(INCDIR) dfdw_$(VERSION)/$(INCDIR)
	cp -r $(LIBDIR) dfdw_$(VERSION)/$(LIBDIR)
	cp -r $(FNTDIR) dfdw_$(VERSION)/$(FNTDIR)
	cp -r $(IMGDIR) dfdw_$(VERSION)/$(IMGDIR)
	cp -r $(INITDIR) dfdw_$(VERSION)/$(INITDIR)
	cp wbits/Makefile dfdw_$(VERSION)
	for p in `ls wbits`; do cp wbits/$$p dfdw_$(VERSION); done;
	cp readme dfdw_$(VERSION)
	-rm dfdw_$(VERSION)/$(BINDIR)*~
	-rm dfdw_$(VERSION)/$(SRCDIR)*~
	-rm dfdw_$(VERSION)/$(INCDIR)*~
	-rm dfdw_$(VERSION)/$(LIBDIR)*~
	-rm dfdw_$(VERSION)/$(FNTDIR)*~
	-rm dfdw_$(VERSION)/$(IMGDIR)*~
	-rm dfdw_$(VERSION)/$(INITDIR)*~
	-rm dfdw_$(VERSION)/*~
	make -C dfdw_$(VERSION) -fMakefile all
