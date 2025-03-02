# makefile for distribution ...

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

VERSION = 110 # 1.1.0

BINARYNAME = hawcx$(VERSION)

SOURCENAME = $(BINARYNAME)src

DISTROSDIR = .\!Distro

PACKER = ..\pkzip25.exe -add -max -rec 
ARCEXT = zip

SOURCEARCHIVE = $(DISTROSDIR)\$(SOURCENAME).$(ARCEXT)
BINARYARCHIVE = $(DISTROSDIR)\$(BINARYNAME).$(ARCEXT)

SOURCESLIST = Acoder.cpp Archive.cpp Asc.cpp Config.cpp Cpy.cpp \
              Error.cpp HAEngine.cpp Haio.cpp HaWCX.cpp Hsc.cpp \
              routines.cpp StdAfx.cpp Swdict.cpp \
              ..\Include\wcxhead.h \
              config.h HAEngine.h resource.h routines.h StdAfx.h \
              HA.RC sciah.bmp cursor.cur icon.ico \
              HA.dsp HA.dsw distro.dsp distro.mak HA.def \
              copying file_id.diz history.txt readme.txt
 
BINARIESLIST = .\Release\ha.wcx \
              copying file_id.diz history.txt readme.txt

ALL: $(SOURCEARCHIVE) $(BINARYARCHIVE)

$(SOURCEARCHIVE): $(SOURCESLIST)
  if not exist $(DISTROSDIR)\$(NULL) mkdir $(DISTROSDIR)
  $(PACKER) -path=specify $(SOURCEARCHIVE) $(SOURCESLIST)

$(BINARYARCHIVE): $(BINARIESLIST)
  if not exist $(DISTROSDIR)\$(NULL) mkdir $(DISTROSDIR)
  $(PACKER) $(BINARYARCHIVE) $(BINARIESLIST)
 
