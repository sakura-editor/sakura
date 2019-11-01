## Process this file with automake to produce Makefile.in

if MCPP_LIB

bin_PROGRAMS = mcpp
mcpp_SOURCES = main_libmcpp.c
mcpp_DEPENDENCIES = libmcpp.la
mcpp_LDADD = -lmcpp
lib_LTLIBRARIES = libmcpp.la
libmcpp_la_SOURCES = system.H configed.H internal.H main.c directive.c  \
    eval.c expand.c mbchar.c support.c system.c
libmcpp_la_LDFLAGS = -no-undefined -version-info 3:0:3
include_HEADERS = mcpp_lib.h mcpp_out.h

main.lo directive.lo eval.lo expand.lo mbohar.lo support.lo system.lo	\
    : system.H internal.H mcpp_lib.h mcpp_out.h
system.H    :   configed.H

else

bin_PROGRAMS = mcpp
mcpp_SOURCES = system.H configed.H internal.H main.c directive.c eval.c \
    expand.c mbchar.c support.c system.c mcpp_out.h
mcpp_CFLAGS = $(AM_CFLAGS)		## dummy to avoid conflict with MCPP_LIB

main.o directive.o eval.o expand.o mbohar.o support.o system.o    :     \
    system.H internal.H mcpp_out.h
system.H    :   configed.H

endif

if REPLACE_CPP
if GNUC

if MINGW
cc1_PROGRAMS = cc1
cc1_SOURCES = cc1.c
cc1dir = $(bindir)
endif

install-exec-hook:
	@if test x"$(gcc_path)" = x""; then	exit 1; fi
	@$(srcdir)/set_mcpp.sh '$(gcc_path)' $(gcc_maj_ver) $(gcc_min_ver)  \
            '$(cpp_call)' '$(CC)' '$(CXX)' 'x$(CPPFLAGS)' 'x$(EXEEXT)'  \
			'$(LN_S)' '$(inc_dir)' $(host_system) $(Target_Cpu) 		\
			$(target_cc)
uninstall-hook:
	@$(srcdir)/unset_mcpp.sh '$(gcc_path)' $(gcc_maj_ver) $(gcc_min_ver)\
             '$(cpp_call)' '$(CC)' '$(CXX)' 'x$(EXEEXT)' '$(LN_S)'      \
             '$(inc_dir)' $(host_system) $(target_cc)
endif
endif
