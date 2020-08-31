# Makefile for MinGW32/MinGW-W64

# Example usages:
#   Out-of-source build:
#     $ mkdir -p build/MinGW/Release
#     $ cd build/MinGW/Release
#     $ mingw32-make -f ../../../sakura_core/Makefile MYCFLAGS=-O2 OUTDIR=. -j4
#
#   Debug build with coverage:
#     $ cd sakura_core
#     $ mingw32-make MYCFLAGS="-g --coverage" MYLIBS=--coverage -j4

# Path of "sakura_core" directory. Compute it from the path of Makefile.
SRCDIR = $(patsubst %/,%,$(subst \,/,$(dir $(firstword $(MAKEFILE_LIST)))))

# If SRCDIR is different from the current directory, set it to VPATH.
# (If SRCDIR ends with a backslash, remove it before set to VPATH.)
ifneq ($(SRCDIR),.)
VPATH = $(patsubst %\,%,$(SRCDIR))
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
RC= $(RCPREFIX)windres

DEFINES= \
 -DWIN32 \
 -D_WIN32_WINNT=_WIN32_WINNT_WIN7 \
 -D_UNICODE \
 -DUNICODE \
 $(MYDEFINES)
LIBS= \
 -static \
 -shared \
 -mwindows \
 $(MYLIBS)

dll = $(or $(OUTDIR),.)/sakura_lang_en_US.dll
sakura_rc = ../sakura_core/sakura_rc.o

SRCS = $(wildcard $(SRCDIR)/*.rc)
OBJS = $(SRCS:$(SRCDIR)/%.rc=%.o)
DEPS = $(OBJS:%.o=%.d)

all: $(dll)

$(dll): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@

$(sakura_rc): ../sakura_core/Makefile
	$(MAKE) -f $< -C $(patsubst %/,%,$(dir $<)) $(@F)

sakura_lang_rc.o: sakura_lang_rc.rc $(sakura_rc)
	$(RC) -c utf-8 --language=0409 $(DEFINES) -I../sakura_core -I$(SRCDIR)/../sakura_core $< -o $@

clean:
	-$(RM) $(subst /,$(DIRSEP),$(dll) $(OBJS) $(DEPS))

.SUFFIXES: .o .rc
.PHONY: all clean $(sakura_rc)

-include $(DEPS)
