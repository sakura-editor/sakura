# Makefile for MinGW32/MinGW-W64

ifndef PREFIX
PREFIX=
RCPREFIX=
else
ifeq ($(PREFIX),x86_64-w64-mingw32-)
RCPREFIX=$(PREFIX)
else
ifeq ($(PREFIX),i686-w64-mingw32-)
ifeq ($(OS),Windows_NT)
RCPREFIX=
else
RCPREFIX=$(PREFIX)
endif
endif
endif
endif

CC= $(PREFIX)gcc
CXX= $(PREFIX)g++
RC= $(RCPREFIX)windres
SUBWCREV= SubWCRev.exe

DEFINES= \
 -DWIN32 \
 -DWINVER=0x500 \
 -D_WIN32_WINNT=0x500 \
 -D_WIN32_IE=0x501 \
 -DNOMINMAX \
 -D_UNICODE \
 -DUNICODE \
 -DNDEBUG
CFLAGS= -O2 \
 -finput-charset=cp932 -fexec-charset=cp932 \
 -I. \
 $(DEFINES) $(MYCFLAGS)
CXXFLAGS= $(CFLAGS) $(MYCXXFLAGS)
LIBS= \
 -lkernel32 -luser32 -lgdi32 \
 -lwinspool -lcomdlg32 -ladvapi32 \
 -lshell32 -lole32 -loleaut32 \
 -luuid -lcomctl32 -limm32 \
 -lmpr -limagehlp \
 -static-libgcc -static-libstdc++ -mwindows -s

exe= sakura_lang_en_US.dll

# ls *.cpp */*.cpp */*/*.cpp | sed -E -e "s/([[:alnum:]_]+)\.[[:alnum:]]+/\1.o \\\\/"
OBJS= \
sakura_lang_rc.o \

RCTOOLDIR=../btool
RCTOOL=$(RCTOOLDIR)/mrc2grc.exe

all: $(RCTOOL) $(exe)

$(exe): sakura_lang.h $(OBJS)
	$(CXX) -shared -o $@ $(OBJS) $(LIBS)

stdafx:
	$(CXX) $(CXXFLAGS) -c StdAfx.h

$(RCTOOL): $(RCTOOLDIR)/mrc2grc.cpp
	$(CXX) $(CXXFLAGS) $(RCTOOLDIR)/mrc2grc.cpp -o $@ -static-libgcc

.rc.o:
	$(RCTOOL) $< sakura_grc.rc
	$(RC) --language=0411 $(DEFINES) sakura_grc.rc -o $@
	$(RM) sakura_grc.rc

clean:
	$(RM) $(exe) $(OBJS) $(RCTOOL)

depend: svnrev
	$(CXX) -E -MM -w $(DEFINES) $(CXXFLAGS) *.cpp */*.cpp */*/*.cpp > depend.mak

.SUFFIXES: .cpp .o .rc
.PHONY: all clean depend

-include depend.mak
