////
//	RcCvt		: convert MSVC type resource file to Brc32 type.
//		20010820 aroka

#include <stdio.h>

class RcCvt {
public:
	RcCvt();
	~RcCvt();
	bool init(char *infile, char*outfile);
	bool convert(void);

private:
	bool font(char*linestr);
	bool include(char*linestr);
	bool setErr(char*errstr);

private:
	FILE *fpi;
	FILE *fpo;
	int nLine;
	char *strerr;
	char *buffer;
};
