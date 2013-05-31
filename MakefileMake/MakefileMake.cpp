/*!	@file

	@author Project Sakura-Editor
	@date 2013.04.03 Uchi �쐬
*/
/*
	Copyright (C) 2013, Uchi


	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

/*
	++ �T�v ++

	MinGW�p�� makefile �������������邽�߂̃��m

	makefile ��������������
	����Ȃ�Ɏ蔲���Ȃ�Ŋ����̃t�@�C�����Ƃ��ɂ͖��Ή�

	++ �g�p���@ ++

	usage() ���Q��
*/

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <string>

#ifdef __MINGW32__
#include <windows.h>
#ifndef _countof
#define _countof(A) (sizeof(A)/sizeof(A[0]))
#endif
#define sprintf_s(A, B, C, ...) sprintf((A), (C), (__VA_ARGS__))
#define strncpy_s(A, B, C, D) strncpy((A), (C), (D))

#undef PREPROCESSOR
#define PREPROCESSOR "gcc -x c++ -finput-charset=cp932 -fexec-charset=cp932 -E %s"

void fopen_s( 
   FILE** pFile,
   const char *filename,
   const char *mode 
)
{
	*pFile = fopen(filename, mode);
}
#endif	// __MINGW32__

#ifdef _MSC_VER
#if _MSC_VER < 1400	// VC2003
#ifndef _countof
#define _countof(A) (sizeof(A)/sizeof(A[0]))
#endif
#define sprintf_s(A, B, C, D) sprintf((A), (C), (D))
#define strncpy_s(A, B, C, D) strncpy((A), (C), (D))
#define fopen_s(A, B, C) ( *(A) = fopen((B), (C)), 0 )
#define _splitpath_s(A, B, C, D, E, F, G, H, I) (_splitpath((A), (B), (D), (F), (H)), 0)
#define _makepath_s(A, B, C, D, E, F) (_makepath((A), (C), (D), (E), (F)), 0)
#define _mktemp_s(A, B) (_mktemp((A)), 0)
#endif	// VC2003
#endif	// _MSC_VER

int usage()
{
	printf("MakefileMake -file=<makefile> -dir=<�g�b�v�f�B���N�g��>\n");
	printf("<makefile>���̂̃I�u�W�F�N�g�t�@�C������");
	printf("�g�b�v�f�B���N�g���z����cpp�t�@�C������");
	printf("�I�u�W�F�N�g�t�@�C�����ɂ������ɏ��ւ��܂��B\n");

	return 1;
}

enum exp_type {
	EXP_SRC,
	EXP_DIR,
};

struct SExpList {
	char*		exp;
	exp_type	type;
} s_exp_list[] = {
	{".cpp",	EXP_SRC	},
	{".c",		EXP_SRC	},
	{"",		EXP_DIR	},
	{".rc",		EXP_SRC	},
};


std::vector<std::string> file_list;  // filename_list


// �t�@�C�����X�g���쐬����
int makeFileList(std::string top_dir, std::string dir, SExpList sexp)
{
	std::string			path;
	std::string			fl_nm;
	HANDLE				hFind;
	WIN32_FIND_DATAA	ffData;

	path = top_dir + "/" + dir;
	if (dir != "") {
		path += "/";
	}
	path = path + "*" + sexp.exp;

	hFind = ::FindFirstFileA(path.c_str(), &ffData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (sexp.type == EXP_DIR) {
				// search subdirectory
				if ((ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
						(strcmp(ffData.cFileName, ".") != 0 && strcmp(ffData.cFileName, "..") != 0)) {

					fl_nm = ffData.cFileName;
					if (dir != "") {
						fl_nm = dir + "/" + fl_nm;
					}

					// search subdirectory
					int		res;
					for (int i = 0; i < _countof(s_exp_list); i++) {
						if ((res = makeFileList(top_dir, fl_nm, s_exp_list[i])) != 0) {
							::FindClose(hFind);
							return res;
						}
					}
				}
			}
			else {
				std::string fl_nm;

				fl_nm = ffData.cFileName;
				fl_nm.resize(fl_nm.size() - strlen(sexp.exp));	// �g���q�폜
				if (dir != "") {
					fl_nm = dir + "/" + fl_nm;
				}

				//add to list
				file_list.push_back( fl_nm );
			}
		} while(::FindNextFileA(hFind, &ffData));
		::FindClose(hFind);
	}
	return 0;
}

// �t�@�C�����X�g���쐬����(top level)
int makeFileListTop(const char* top_dir)
{
	int		res;

	for (int i = 0; i < _countof(s_exp_list); i++) {
		if ((res = makeFileList(top_dir, "", s_exp_list[i])) != 0)
			return res;
	}
	return 0;
}


// �t�@�C����������o��
// �Ō��NULL��Ԃ�
const char* getFile()
{
	static int		pt;
	static std::string	r_str;

	if (pt >= (int)file_list.size()) {
		return NULL;
	}

	r_str = file_list[pt];
	pt++;

	return r_str.c_str();
}


// �I�u�W�F�N�g�s1�s�쐬
const char* makeObjLine(char* mkline, size_t bf_sz, const char* fl_nm)
{
	sprintf_s( mkline, bf_sz, "%s.o \\\n", fl_nm);
	return mkline;
}


// main�֐�
int main(int argc, char* argv[])
{
	// ��������
	const char*	makefile	= NULL;
	const char*	top_dir		= NULL;

	for (int i=1; i < argc; i++) {
		char*	p = argv[i];
		if (*p == '/' || *p =='-') {
			p++;
			if (_strnicmp(p,"file",4) == 0) {
				p += 4;
				if (*p == '\0') {
					if (i < argc) {
						makefile = argv[++i];
					}
				}
				else {
					if (*p == '=') p++;
					makefile = p;
				}
			}
			else if (_strnicmp(p,"dir",3) == 0) {
				p += 3;
				if (*p == '\0') {
					if (i < argc) {
						top_dir = argv[++i];
					}
				}
				else {
					if (*p == '=') p++;
					top_dir = p;
				}
			}
			else {
				printf("Error: �s���Ȉ���[%s]\n", argv[i]);
				return usage();
			}
		}
		else {
			printf("Error: �s���Ȉ���[%s]\n", argv[i]);
			return usage();
		}
	}
	if (makefile == NULL && top_dir != NULL) { return usage(); }
	if (!makefile)	{ printf("Error: makefile���w�肵�Ă�������\n\n");				return usage(); }
	if (!top_dir)	{ printf("Error: �g�b�v�f�B���N�g�����w�肵�Ă�������\n\n");	return usage(); }


	// �g�b�v�f�B���N�g���̃`�F�b�N
	struct stat		st;
	int		ret = stat( top_dir, &st );
	if (ret != 0 || !(st.st_mode & _S_IFDIR)) {
		printf("Error: �g�b�v�f�B���N�g��[%s]��������܂���\n", top_dir);
		return 1;
	}

	// �t�@�C���I�[�v��
	FILE*	in = NULL;
	if (fopen_s( &in, makefile, "rt" ) != 0) {
		printf("Error: �o�̓t�@�C��[%s]���J���܂���\n", makefile);
		return 1;
	}

	// �e���|�����t�@�C���̍쐬
	char	tmp_file[_MAX_PATH];
	char	drive[_MAX_DRIVE], dir[_MAX_DIR];
	if (_splitpath_s( makefile, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0 )) {
		printf("Error: �ꎞ�t�@�C���������܂���[%s]\n", makefile);
		return 1;
	}
	if (_makepath_s( tmp_file, _countof(tmp_file), drive, dir, "mfXXXXXX", NULL )) {
		printf("Error: �ꎞ�t�@�C���������܂���[%s, %s]\n", drive, dir);
		return 1;
	}
	if (_mktemp_s(tmp_file, _countof(tmp_file))) {
		printf("Error: �ꎞ�t�@�C���������܂���[%s]\n", tmp_file);
		return 1;
	}
	FILE*	out = NULL;
	if (fopen_s( &out, tmp_file, "wt" ) != 0) {
		printf("Error: �ꎞ�t�@�C��[%s]���J���܂���\n", tmp_file);
		return 1;
	}

	// �t�@�C�����X�g�̍쐬
	makeFileListTop(top_dir);

	// �t�@�C���̏��ւ�
	int			mode = 0;			// 0:.obj�O 1:.obj�� 2:.obj��
	bool		change = false;		// �ύX����

	char		line[1024];
	char		mkline[1024];
	char*		wtline;
	const char*	fl_nm;

	while (fgets(line, _countof(line), in) != NULL) {
		wtline = line;
		switch (mode) {
		case 0:
			if (strstr(line, "OBJS")) {
				mode = 1;
			}
			break;
		case 1:
			if (line[0] == '\n' || line[0] == '\0') {
				// ���X�g�I��?
				fl_nm = getFile();
				if (fl_nm != NULL) {
					// �t�@�C����������
					change = true;
					do {
						//�o��
						fprintf(out, "%s", makeObjLine( mkline, _countof(mkline), fl_nm ) );
					} while ((fl_nm = getFile()) != NULL);
				}
				mode = 2;
			}
			else {
				fl_nm = getFile();
				if (fl_nm == NULL) {
					// �t�@�C����������
					change = true;
					continue;
				}
				makeObjLine( mkline, _countof(mkline), fl_nm );
				// �ύX�L�肩�H
				if (!change && strcmp(line, mkline) != 0)
					change = true;
				wtline = mkline;
			}
			break;
		case 2:
			break;
		}
		//�o��
		fprintf(out,"%s", wtline);
	}

	// close
	fclose(in);
	fclose(out);
#ifdef _DEBUG
	printf("%d�̃I�u�W�F�N�g�t�@�C�������o�͂���܂���\n", file_list.size());
#endif

	// �t�@�C���̓�����
	if (change) {
		if (remove(makefile)) {
			printf("Error: makefile[%s]���폜�o���܂���\n", tmp_file);
			return 1;
		}
		if (rename( tmp_file, makefile )) {
			printf("Error: �ꎞ�t�@�C��[%s]��makfile[%s]�ɏo���܂���\n", tmp_file, makefile);
			return 1;
		}
	}
	else {
		if (remove(tmp_file)) {
			printf("Warning: �ꎞ�t�@�C��[%s]���폜�o���܂���\n", tmp_file);
		}
		printf("�o�̓t�@�C���͍ŐV�ł�\n");
	}

	return 0;
}
