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
	//! �G���[������̒ǉ�
	bool setErr(char*errstr);

private:
	FILE *fpi;	//!<�ϊ���(����)�t�@�C���̃t�@�C���|�C���^
	FILE *fpo;	//!<�ϊ���(�o��)�t�@�C���̃t�@�C���|�C���^
	int nLine;	//!<1�s�̍ő咷�D�����o�b�t�@�̏��������Ƀo�b�t�@�T�C�Y�Ƃ��Ďg����D
	char *strerr;	//!< �G���[������ւ̃|�C���^
	char *buffer;	//!< �ǂݍ��݃o�b�t�@�ւ̃|�C���^�D
};
