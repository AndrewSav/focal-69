#
# Makefile for FOCAL
#

CFLAGS = -O -DUNIX -DANSICRT $(DEBUG)
CC = gcc

VERSION = 1.0.5
RELEASECPU = `uname -m`
RELEASEOS = `uname -s`

OBJ = .o

#MVSCFLAGS = -O2 -DOPENEDITION -DANSICRT $(DEBUG)
#MVSCC = gcc-uss
MVSCFLAGS = -O2 -DOS390 $(DEBUG)
MVSCC = gcc-mvsle
MVSCOPTS = -S

TARGETS = focal$(EXE)

SRCS =	focal.c parser.c screen.c 
OBJS =	focal$(OBJ) parser$(OBJ) screen$(OBJ) 
SSRC =	focal.s parser.s screen.s
HDRS =  parser.h errors.h ptables.h stables.h psemant.h scanner.h

INSTDIR = /usr/local
BINDIR = $(INSTDIR)/bin
MANDIR = $(INSTDIR)/share/man/man1

.c.s :
	$(MVSCC) $(MVSCFLAGS) $(MVSCOPTS) $<
	@mv $@ t.t
	@detab 9 16 +8 t.t >$@
	@rm t.t

all : 
	@if [ "`uname -s`" = "Linux" ] ; then \
		echo "Making Linux on a $(RELEASECPU)" ;\
		make $(TARGETS) \
		   "CFLAGS = -O2 -DSYSVDIR -DSTRERROR -DANSICRT $(DEBUG)" ;\
	elif [ "`uname -s`" = "GNU/kFreeBSD" ]; then \
		echo "Making GNU/kFreeBSD on a $(RELEASECPU)";\
		make $(TARGETS) \
		   "CFLAGS = -O2 -DSYSVDIR -DSTRERROR -DANSICRT $(DEBUG)" ;\
	elif [ "`uname -s`" = "GNU" ] ; then \
		echo "Making GNU/Hurd on a $(RELEASECPU)";\
		make $(TARGETS) \
		   "CFLAGS = -O2 -DSYSVDIR -DSTRERROR -DANSICRT $(DEBUG)" ;\
	elif [ "`uname -s`" = "OS/390" ] ; then \
		echo "Making OS/390 USS" ;\
		make $(TARGETS) "CC=cc" \
		   "CFLAGS = -O -DOPENEDITION -DSYSVDIR -DSTRERROR -DANSICRT \
		   	$(DEBUG)" ;\
	elif [ "`uname -s`" = "SunOS" ] ; then \
		echo "Making Solaris" ;\
		make $(TARGETS) \
		   "CFLAGS = -O -DSYSVDIR -DSTRERROR -DANSICRT $(DEBUG)" ;\
	else \
		echo "OS type `uname -s` is unknown" ;\
		echo "You must enter an OS type. OS types are:" ;\
		echo "   linux | nt | openmvs | openvms | os2 | riscos | " ;\
		echo "   solaris | sunos | gnuhurd | gnukfreebsd | dx10" ;\
		echo " " ;\
		echo "For IBM OS/390 you have the choices:" ;\
		echo "   dignusdcc | dignusgcc | mvs" ;\
		echo " " ;\
	fi

nt :
	@nmake /nologo /f Makefile.nt $(PARM)

openmvs :
	@make focal$(EXE) \
		"CFLAGS = -O -DOPENEDITION -DSYSVDIR -DSTRERROR -DANSICRT \
			$(DEBUG)" \
		"CC = cc" \
		$(PARM)

openvms vaxvms :
	make /file=Makefile.vms $(PARM)

os2 :
	@nmake /nologo /f Makefile.os2 $(PARM)

dx10 :
	@make -f Makefile.dx10 $(PARM)

dnos :
	@make -f Makefile.dnos $(PARM)

linux :
	@make focal$(EXE) \
		"CFLAGS = -O2 -DSYSVDIR -DSTRERROR -DANSICRT $(DEBUG)" \
		$(PARM)

gnuhurd :
	@make focal$(EXE) \
		"CFLAGS = -O2 -DSYSVDIR -DSTRERROR -DANSICRT $(DEBUG)" \
		$(PARM)

gnukfreebsd :
	@make focal$(EXE) \
		"CFLAGS = -O2 -DSYSVDIR -DSTRERROR -DANSICRT $(DEBUG)" \
		$(PARM)

solaris :
	@make focal$(EXE) \
		"CFLAGS = -O -DSYSVDIR -DSTRERROR -DANSICRT $(DEBUG)" \
		$(PARM)

sunos :
	@make focal$(EXE) \
		"CFLAGS = -O -DBSDDIR -DANSICRT $(DEBUG)" \
		$(PARM)

riscos :
	@make focal$(EXE) \
		"CFLAGS = -O -DBSDDIR -DANSICRT $(DEBUG)" \
		$(PARM)

focal$(EXE) : $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) -lm

mvs : $(SSRC)

dignusdcc :
	@make focal.po \
		"LIBS = -lmvs" \
		"CFLAGS = -DDIGNUS -DOS390 -I/usr/local/dignus/include $(DEBUG)" \
		"CC = dcc"

dignusgcc :
	@make focal.po \
		"LIBS = -lmvs" \
		"CFLAGS = -S -DOS390 $(DEBUG)" \
		"CC = gcc-mvsdignus"

install :
	@if [ ! -d $(BINDIR) ] ; then \
	   mkdir -p $(BINDIR) ;\
	fi
	cp $(TARGETS) $(BINDIR)
	@if [ ! -d $(MANDIR) ] ; then \
	   mkdir -p $(MANDIR) ;\
	fi
	cp focal.1 $(MANDIR)

clean :
	rm -f $(OBJS) $(SSRC) core $(TARGETS)
	rm -f *.obj *.asm *.po *.lst *.exe *.map

distclean : clean
	rm -rf focaltar focalrpm
	rm -f focal-$(VERSION).tgz

tar : all
	rm -rf focaltar
	mkdir -p focaltar/focal-$(VERSION)/bin
	mkdir -p focaltar/focal-$(VERSION)/share/man/man1
	cp $(TARGETS) focaltar/focal-$(VERSION)/bin/
	cp focal.1 focaltar/focal-$(VERSION)/share/man/man1/
	tar czf focal-$(VERSION).tgz -C focaltar focal-$(VERSION)

rpm : tar
	mkdir -p focalrpm/BUILD focalrpm/RPMS focalrpm/SOURCES focalrpm/SPECS \
	   focalrpm/SRPMS
	cp focal-$(VERSION).tgz focalrpm/SOURCES/
	sed -e "s/Version: 0.0.0/Version: $(VERSION)/g" \
	   -e "s/Release: 0/Release: $(RELEASEOS)/g" \
	   focal.spec >focalrpm/SPECS/focal.spec
	rpmbuild --define "_topdir `pwd`/focalrpm" \
	   -bb focalrpm/SPECS/focal.spec

depend : $(SRCS)
	mkdep $(SRCS) > makedep
	@echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	@echo '$$r makedep' >>eddep
	@echo 'w' >>eddep
	@cp Makefile Makefile.bak
	@ex - Makefile < eddep
	-@rm eddep makedep

lint : $(SRCS)
	lint $(SRCS)

tags : $(SRCS)
	ctags $(SRCS)

focalc.tok focalc.err focalc.ptb focalc.sem : focalc.bnf
	chat focalc

perrors.h : focalc.err
	cp focalc.err perrors.h

ptables.h : focalc.ptb
	cp focalc.ptb ptables.h

psemant.h : focalc.sem
	cp focalc.sem psemant.h

ptokens.h : focalc.tok
	cp focalc.tok ptokens.h

focal.s : focal.c parser.h errors.h perrors.h
parser.s : parser.c parser.h errors.h ptables.h stables.h psemant.h scanner.h
screen.s : screen.c

focal.asm : focal.c parser.h errors.h perrors.h
	$(CC) $(CFLAGS) -o $@ $<
parser.asm : parser.c parser.h errors.h ptables.h stables.h psemant.h scanner.h
	$(CC) $(CFLAGS) -o $@ $<
screen.asm : screen.c
	$(CC) $(CFLAGS) -o $@ $<

focal.obj : focal.asm
	dasm -xa -xr -xrld -L /usr/local/dignus/maclib -macext . -o $@ $<
parser.obj : parser.asm
	dasm -xa -xr -xrld -L /usr/local/dignus/maclib -macext . -o $@ $<
screen.obj : screen.asm
	dasm -xa -xr -xrld -L /usr/local/dignus/maclib -macext . -o $@ $<

focal.po : focal.obj parser.obj screen.obj
	plink -px -o $@ focal.obj parser.obj screen.obj -L/usr/local/dignus/lib -lc $(LIBS)

# DO NOT DELETE THIS LINE
focal$(OBJ) : focal.c parser.h errors.h perrors.h
parser$(OBJ) : parser.c parser.h errors.h ptables.h stables.h psemant.h \
	scanner.h
screen$(OBJ) : screen.c
