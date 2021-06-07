# Makefile for MinGW32/MinGW-W64

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
P7Z = 7z
else
# If unix-like shell is used.
MKDIR = mkdir -p
RM = rm -f
DIRSEP = /
DEVNULL = /dev/null
P7Z = 7z
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

RC= $(RCPREFIX)windres

TARGET = $(or $(OUTDIR),.)/resources.ja-JP.zip

SRC_DIRS = test-plugin

all: $(TARGET)

$(TARGET):
	$(P7Z) u $@ -tzip -mcu=on -r $(SRC_DIRS)

clean:
	-$(RM) $(TARGET)

.PHONY: all clean
