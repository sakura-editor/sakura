////
//	rmc2brc		: convert MSVC type resource file to Brc32 type.
//		20010731 aroka

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RcCvt.h"

#ifdef _DEBUG
# define trail(c)	fputs(c,stdout)
#else
# define trail(c)
#endif

void usage(char*av0)
{
	char* fn;
	
	fn = strrchr( av0, '\\' );
	if( fn != NULL ){
		fn++;
	}else{
		fn = av0;
	}
	
	fprintf(stderr, "%s inputfile [outputfile]", fn);
	return ;
}

int main( int ac, char *av[] )
{
	if( ac < 2 )
	{
		usage(av[0]);
		exit(0);
	}
	
	RcCvt cvt;
	trail("#1");
	if( !cvt.init(av[1], av[2]) ){
		fprintf(stderr,"Error in opening files.\n");
		exit(1);
	}
	trail("#2");
	if( !cvt.convert() ){
		fprintf(stderr,"Error in opening files.\n");
		exit(1);
	}
	trail("#3");

	return 0;
}

