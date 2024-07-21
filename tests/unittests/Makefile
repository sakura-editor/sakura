# Makefile for MinGW32/MinGW-W64

# Example usages:
#   Out-of-source build (build in the same directory as sakura_core):
#     $ mkdir -p build/MinGW/Release/
#     $ cd build/MinGW/Release
#     $ mingw32-make -f ../../../tests/unittests/Makefile SAKURA_CORE=. OUTDIR=. -j4
#
#   Out-of-source build (create tests1 and sakura_core in a common directory):
#     $ mkdir -p build/MinGW/Release/tests1/
#     $ cd build/MinGW/Release/tests1
#     $ mingw32-make -f ../../../../tests/unittests/Makefile OUTDIR=.. -j4
#
#   Debug build with coverage:
#     $ cd tests/unittests
#     $ mingw32-make MYCFLAGS="-g --coverage" MYLIBS=--coverage -j4

# Path of "sakura_core" directory. Compute it from the path of Makefile.
SRCDIR = $(patsubst %/,%,$(subst \,/,$(dir $(firstword $(MAKEFILE_LIST)))))

# If SRCDIR is different from the current directory, set it to VPATH.
# (If SRCDIR ends with a backslash, remove it before set to VPATH.)
ifneq ($(SRCDIR),.)
VPATH = $(patsubst %\,%,$(SRCDIR))
endif

# The directory where "sakura_core" is built.
SAKURA_CORE = ../sakura_core

# The directory where the .exe files will be output.
# If empty, they will be output to the default directories.
OUTDIR =

CURRENT_INSTALLED_DIR = ../../../vcpkg_installed

TARGET_TRIPLET = x64-mingw-static

ifeq ($(SHELL),sh.exe)
# If cmd.exe is used as a shell.
MKDIR = md
RM = del
DIRSEP = $(strip \ )
DEVNULL = NUL
ICONV = iconv.exe
SED = sed.exe
P7Z = 7z.exe
else
# If unix-like shell is used.
MKDIR = mkdir -p
RM = rm -f
DIRSEP = /
DEVNULL = /dev/null
ICONV = iconv
SED = sed
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

CC= $(PREFIX)gcc
CXX= $(PREFIX)g++
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

CFLAGS= \
 -finput-charset=utf-8 \
 -fexec-charset=cp932 \
 -MMD \
 -I. \
 -I$(CURRENT_INSTALLED_DIR)/$(TARGET_TRIPLET)/include \
 -I$(SAKURA_CORE) \
 -I$(SRCDIR)/../../sakura_core \
 -I$(SRCDIR) \
 $(DEFINES) $(MYCFLAGS)

CXXFLAGS= $(CFLAGS) \
 -std=c++17 \
 $(MYCXXFLAGS)

LDFLAGS= \
 -static \
 -mwindows \
 -municode \
 -L$(CURRENT_INSTALLED_DIR)/$(TARGET_TRIPLET)/lib

LIBS= \
 -lwinspool \
 -lole32 \
 -loleaut32 \
 -luuid \
 -lcomctl32 \
 -limm32 \
 -lmpr \
 -limagehlp \
 -lshlwapi \
 -ldwmapi \
 -lwinmm \
 -lwindowscodecs \
 -lmsimg32 \
 -lurlmon \
 -lkernel32 \
 -lgdi32 \
 -lcomdlg32 \
 -lgtest \
 -lgmock \
 $(MYLIBS)

exe= $(or $(OUTDIR),.)/tests1.exe

SRCS = $(wildcard $(SRCDIR)/test-*.cpp) \
       $(wildcard $(SRCDIR)/code-*.cpp)
OBJS = $(SRCS:$(SRCDIR)/%.cpp=%.o) \
       tests1_rc.o

DEPS= $(OBJS:%.o=%.d)

SAKURA_SRCS = $(wildcard $(SRCDIR)/../../sakura_core/*.cpp) \
			  $(wildcard $(SRCDIR)/../../sakura_core/*/*.cpp) \
			  $(wildcard $(SRCDIR)/../../sakura_core/*/*/*.cpp)
SAKURA_OBJS = $(SAKURA_SRCS:$(SRCDIR)/../../sakura_core/%.cpp=$(SAKURA_CORE)/%.o) \
              $(SAKURA_CORE)/sakura_rc.o

all: $(exe)

$(exe): $(OBJS) $(SAKURA_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SAKURA_OBJS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJS):

tests1_rc.o: tests1_rc.rc.utf8 $(sakura_rc) resources.ja-JP.zip resources.en-US.zip
	$(RC) -c utf-8 -I. -I$(SRCDIR) $(DEFINES) $< -o $@

tests1_rc.rc.utf8: tests1_rc.rc
	$(ICONV) -f utf-16 -t utf-8 $(SRCDIR)/$(subst .utf8,,$(@F)) | $(SED) -e 1i'#pragma code_page(65001)' > $@

clean:
	-$(RM) $(subst /,$(DIRSEP),$(exe) $(OBJS))
	-$(RM) $(subst /,$(DIRSEP),$(DEPS))

.SUFFIXES: .cpp .o
.PHONY: all clean

-include $(DEPS)
