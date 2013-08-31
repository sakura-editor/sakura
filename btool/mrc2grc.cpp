#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool sjis_1(char* p)
{
	unsigned char* x = reinterpret_cast<unsigned char*>(p);
	return (static_cast<unsigned int>((*x) ^ 0x20) - 0xa1 < 0x3c);
}

bool sjis_2(char *p)
{
	unsigned char* x = reinterpret_cast<unsigned char*>(p);
	return (0x40 <= (*x) && (*x) <= 0xfc && (*x) != 0x7f);
}

void convert(const char* infile, const char* outfile)
{
	FILE* fin = fopen(infile, "r");
	FILE* fout = fopen(outfile, "w");
	if( NULL == fin ){
		fprintf(stderr, "infile cannot open.\n");
		return;
	}
	if( NULL == fout ){
		fprintf(stderr, "outfile cannot open.\n");
		return;
	}
	const int nBufferSize = 100;
	char buffer[nBufferSize];
	char outbuffer[nBufferSize*2];
	int carrierlen = 0;
	while( NULL != fgets(buffer + carrierlen, nBufferSize-carrierlen, fin) ){
		char* p = buffer;
		char* o = outbuffer;
		carrierlen = 0;
		while( '\0' != *p ){
			if( sjis_1(p) && sjis_2(p+1) ){
				*o++ = *p++;
				// SJISの2バイト目が\だったときだけ\を付加する
				if( *p == '\\' ){
					*o++ = '\\';
				}
				*o++ = *p++;
			}else if( sjis_1(p) && *(p+1) == '\0' ){
				// 1byte次回持越し
				buffer[0] = *p++;
				buffer[1] = '\0';
				carrierlen = 1;
			}else{
				*o++ = *p++;
			}
		}
		*o = '\0';
		fputs(outbuffer, fout);
	}
	fclose(fin);
	fclose(fout);
	return;
}

void usage(char* av0)
{
	char* fn;
	
	fn = strrchr( av0, '\\' );
	if( fn != NULL ){
		fn++;
	}else{
		fn = av0;
	}
	
	fprintf(stderr, "%s inputfile outputfile", fn);
	return ;
}

int main( int ac, char *av[] )
{
	if( ac <= 2 )
	{
		usage(av[0]);
		exit(0);
	}
	convert( av[1], av[2] );
	return 0;
}
