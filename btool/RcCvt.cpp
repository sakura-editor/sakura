////
//	RcCvt		: convert MSVC type resource file to Brc32 type.
//		20010820 aroka

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RcCvt.h"

#ifdef _DEBUG
# define trail(c)	fputs(c,stdout)
#else
# define trail(c)
#endif


RcCvt::RcCvt()
	: fpi(0), fpo(0), nLine(100),
	 strerr(0), buffer(0)
{
}
RcCvt::~RcCvt()
{
	if( fpi )	fclose( fpi );
	if( fpo )	fclose( fpo );
	if( strerr ){
		fprintf( stderr, "RcCvt! %s\n", strerr );
		free( strerr );
	}
	if( buffer )
		free( buffer );
	
}

bool RcCvt::setErr(char*errstr)
{
	char *t;
	int lcr = strlen(errstr)+2;
	if( strerr ){
		t = (char*)realloc( strerr , strlen(strerr)+lcr );	// 2003.10.31 moca �s�v��+1���폜
	}else{
		// 2003.10.31 moca �s�v��+1���폜
		// 2003.11.01 genta calloc��0�N���A��ۏ؂���
		t = (char*)calloc( lcr, 1 );
	}
	if( t==NULL){
		return false;
	}

	strcat( t, "\n" );
	strcat( t, errstr );
	strerr = t;
	return true;
}

bool RcCvt::init(char *infile, char*outfile)
{
	fpi = fopen( infile, "r" );
	fpo = fopen( outfile, "w" );

	trail("##1");
	if( fpi==NULL ){
		setErr( "infile cannot open." );
		return false;
	}
	trail("##2");
	if( fpo==NULL ){
		setErr( "outfile cannot open." );
		return false;
	}
	trail("##3");
	if(!buffer){
		buffer = (char*)malloc(nLine);
		if( buffer == NULL ){
			setErr( "cannot allocate buffer" );
			return false;
		}
	}
	
	return true;
}

// convert check.
bool RcCvt::convert(void)
{
	while( !feof(fpi) )
	{
		//	2007.04.03 genta �t�@�C�������œǂݍ��݃G���[�ɂȂ�ƍŏI�s���d�����ďo�͂����
		if( NULL == fgets(buffer, nLine, fpi )){
			break;
		}
		
		if( strncmp( buffer, "FONT", 4 ) == 0 ){
			font(buffer);
		}
		if( strncmp( buffer, "#include", 8 ) == 0 ){
			include(buffer);
		}
		
		fputs(buffer, fpo );
	}
	return true;
}

// Convert stuff.
bool RcCvt::font(char*linestr)
{
	char* cp, *cp2;
	trail("$1");
	cp = strchr( linestr, ',' );
	if( cp==NULL ){
		setErr( "',' not found." );
		return false;
	}
	cp++;
	trail("$2");
	cp2 = strchr( cp, ',' );
	trail("$3");
	if( cp2==NULL ){
		setErr( "',' not found." );
		return false;
	}
	trail("$4");
	
	*cp2 = '\n';
	cp2++;
	*cp2 = '\0';

	return true;
}

bool RcCvt::include(char*linestr)
{
	if( strstr( linestr, "afxres.h" ) != NULL )
	{
		strcpy( linestr, "\n" );
	}
	//2002.01.10 ���납 ���\�[�X��Dlgs.h���폜
	else if( strstr( linestr, "Dlgs.h" ) != NULL )	//2002.01.15 ���납 elsif -> else if
	{
		strcpy( linestr, "\n" );
	}
	else if( strstr( linestr, "windows.h" ) != NULL )	//2006.11.14 ryoji
	{
		strcpy( linestr, "\n" );
	}
	return true;
}
