# Makefile for MinGW32/MinGW-W64

# Relative path of "sakura_core" directory from the current directory.
# Useful when doing out-of-source build.  E.g.
#   $ cd sakura_core
#   $ mkdir _build
#   $ cd _build
#   $ mingw32-make -f ../Makefile SRCDIR=..
# Normally, this is derived from the path of Makefile.
SRCDIR = $(patsubst %/,%,$(dir $(word 1,$(MAKEFILE_LIST))))

# If SRCDIR is different from the current directory, set it to VPATH.
ifneq ($(SRCDIR),.)
VPATH = $(SRCDIR)
endif

# The directory where the .exe files will be output.
# If empty, they will be output to the default directories.
OUTDIR =

ifeq ($(SHELL),sh.exe)
# If cmd.exe is used as a shell.
MKDIR = md
RM = del
DIRSEP = $(strip \ )
DEVNULL = NUL
else
# If unix-like shell is used.
MKDIR = mkdir -p
RM = rm -f
DIRSEP = /
DEVNULL = /dev/null
endif

ifndef PREFIX
PREFIX=
RCPREFIX=
else ifeq ($(PREFIX),x86_64-w64-mingw32-)
RCPREFIX=$(PREFIX)
else ifeq ($(PREFIX),i686-w64-mingw32-)
ifeq ($(OS),Windows_NT)
RCPREFIX=
else
RCPREFIX=$(PREFIX)
endif
endif

CC= $(PREFIX)gcc
CXX= $(PREFIX)g++
RC= $(RCPREFIX)windres

DEFINES= \
 -DWIN32 \
 -D_WIN32_WINNT=_WIN32_WINNT_WIN7 \
 -D_UNICODE \
 -DUNICODE \
 $(MYDEFINES)
CFLAGS= \
 -finput-charset=utf-8 \
 -fexec-charset=cp932 \
 -MMD \
 -I. \
 -I$(SRCDIR) \
 $(DEFINES) $(MYCFLAGS)
CXXFLAGS= $(CFLAGS) \
 -std=c++17 \
 $(MYCXXFLAGS)
LIBS= \
 -static \
 -lwinspool \
 -lole32 \
 -loleaut32 \
 -luuid \
 -lcomctl32 \
 -limm32 \
 -lmpr \
 -limagehlp \
 -lshlwapi \
 -lwinmm \
 -lwindowscodecs \
 -lmsimg32 \
 -mwindows \
 -municode \
 $(MYLIBS)

exe= $(or $(OUTDIR),.)/sakura.exe

SRCS = $(wildcard $(SRCDIR)/*.cpp) \
       $(wildcard $(SRCDIR)/*/*.cpp) \
       $(wildcard $(SRCDIR)/*/*/*.cpp)
OBJS = $(SRCS:$(SRCDIR)/%.cpp=%.o) sakura_rc.o

_DIRS = $(filter %/, $(wildcard $(SRCDIR)/*/)) \
        $(filter %/, $(wildcard $(SRCDIR)/*/*/))
DIRS = $(_DIRS:$(SRCDIR)/%/=%)

DEPS= $(OBJS:%.o=%.d) StdAfx.h.d

GENERATED_FILES= \
 Funccode_define.h \
 Funccode_enum.h \
 githash.h \

HEADERMAKETOOLDIR= $(SRCDIR)/../HeaderMake
HEADERMAKE= $(or $(OUTDIR),$(HEADERMAKETOOLDIR))/HeaderMake.exe

all: $(exe)

$(exe): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)

Funccode_define.h: Funccode_x.hsrc $(HEADERMAKE)
	$(HEADERMAKE) -in=$< -out=$@ -mode=define

Funccode_enum.h: Funccode_x.hsrc $(HEADERMAKE)
	$(HEADERMAKE) -in=$< -out=$@ -mode=enum -enum=EFunctionCode

githash.h:
	cmd /c $(subst /,\,$(SRCDIR))\..\sakura\githash.bat .

StdAfx.h.gch: StdAfx.h githash.h Funccode_enum.h
ifneq ($(SRCDIR),.)
	-$(MKDIR) $(subst /,$(DIRSEP),$(DIRS)) > $(DEVNULL) 2>&1
endif
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJS): StdAfx.h.gch

$(HEADERMAKE): $(HEADERMAKETOOLDIR)/HeaderMake.cpp
	$(CXX) $(CXXFLAGS:-MMD=) $< -o $@ -static-libgcc

sakura_rc.o: sakura_rc.rc githash.h Funccode_define.h
	$(RC) -c utf-8 --language=0411 $(DEFINES) -I. -I$(SRCDIR) $< -o $@

clean:
	-$(RM) $(subst /,$(DIRSEP),$(exe) $(OBJS) $(HEADERMAKE)) StdAfx.h.gch $(GENERATED_FILES)
	-$(RM) $(subst /,$(DIRSEP),$(DEPS))

.SUFFIXES: .cpp .o .rc
.PHONY: all clean

-include $(DEPS)
