# Makefile for MinGW32/MinGW-W64

# Example usages:
#   Out-of-source build (build in the same directory as sakura_core):
#     $ mkdir -p build/MinGW/Release/
#     $ cd build/MinGW/Release
#     $ mingw32-make -f ../../../sakura_lang_en_US/Makefile SAKURA_CORE=. OUTDIR=. -j4
#
#   Out-of-source build (create sakura_lang_en_US and sakura_core in a common directory):
#     $ mkdir -p build/MinGW/Release/sakura_lang_en_US/
#     $ cd build/MinGW/Release/sakura_lang_en_US
#     $ mingw32-make -f ../../../../sakura_lang_en_US/Makefile OUTDIR=.. -j4
#
#   Debug build:
#     $ cd sakura_lang_en_US
#     $ mingw32-make MYDEFINES="-D_DEBUG" -j4

# Path of "sakura_lang_en_US" directory. Compute it from the path of Makefile.
SRCDIR = $(patsubst %/,%,$(subst \,/,$(dir $(firstword $(MAKEFILE_LIST)))))

# If SRCDIR is different from the current directory, set it to VPATH.
# (If SRCDIR ends with a backslash, remove it before set to VPATH.)
ifneq ($(SRCDIR),.)
VPATH = $(patsubst %\,%,$(SRCDIR))
endif

# The directory where "sakura_core" is built.
SAKURA_CORE = ../sakura_core

# The directory where the .dll files will be output.
# If empty, it will be output to the default directory.
OUTDIR =

ifeq ($(SHELL),sh.exe)
# If cmd.exe is used as a shell.
MKDIR = md
RM = del
DIRSEP = $(strip \ )
DEVNULL = NUL
ICONV = iconv.exe
SED = sed.exe
else
# If unix-like shell is used.
MKDIR = mkdir -p
RM = rm -f
DIRSEP = /
DEVNULL = /dev/null
ICONV = iconv
SED = sed
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
RC= $(RCPREFIX)windres

DEFINES= \
 -DWIN32 \
 -D_WIN32_WINNT=_WIN32_WINNT_WIN7 \
 -D_UNICODE \
 -DUNICODE \
 $(MYDEFINES)

ifeq (,$(findstring -D_DEBUG,$(DEFINES)))
ifeq (,$(findstring -DNDEBUG,$(DEFINES)))
DEFINES += -DNDEBUG
endif
endif

LDFLAGS= \
 -static \
 -shared \
 -mwindows \
 -s

dll = $(or $(OUTDIR),.)/sakura_lang_en_US.dll
sakura_rc = $(SAKURA_CORE)/sakura_rc.o

SRCS = $(wildcard $(SRCDIR)/*.rc)
OBJS = $(SRCS:$(SRCDIR)/%.rc=%.o)

all: $(dll)

$(dll): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(sakura_rc): ../sakura_core/Makefile
ifneq ($(SAKURA_CORE),.)
	-$(MKDIR) $(subst /,$(DIRSEP),$(@D)) > $(DEVNULL) 2>&1
endif
	$(MAKE) -f $< -C $(@D) $(@F)

sakura_lang_rc.o: sakura_lang_rc.rc.utf8 sakura_lang_rc.rc2.utf8 $(sakura_rc)
	$(RC) -c utf-8 --language=0409 $(DEFINES) -I$(SAKURA_CORE) -I$(SRCDIR)/../sakura_core $< -o $@

sakura_lang_rc.rc.utf8 sakura_lang_rc.rc2.utf8: sakura_lang_rc.rc sakura_lang_rc.rc2
	$(ICONV) -f utf-16 -t utf-8 $(SRCDIR)/$(subst .utf8,,$(@F)) | $(SED) -e 1i'#pragma code_page(65001)' -e '/#include/s/.rc2\b/.rc2.utf8/' > $@

clean:
	-$(RM) $(subst /,$(DIRSEP),$(dll) $(OBJS))

.SUFFIXES: .o .rc
.PHONY: all clean
