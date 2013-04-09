/*!	@file

	@author Project Sakura-Editor
	@date 2013.04.03 Uchi 作成
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
	++ 概要 ++

	MinGW用の makefile を自動生成するためのモノ

	makefile を自動生成する
	其れなりに手抜きなんで漢字のファイル名とかには未対応

	++ 使用方法 ++

	usage() を参照
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

int usage()
{
	printf("MakefileMake -file=<makefile> -dir=<トップディレクトリ>\n");
	printf("<makefile>内ののオブジェクトファイル名を");
	printf("トップディレクトリ配下のcppファイル名を");
	printf("オブジェクトファイル名にした物に書替えます。\n");

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


// ファイルリストを作成する
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
				fl_nm.resize(fl_nm.size() - strlen(sexp.exp));	// 拡張子削除
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

// ファイルリストを作成する(top level)
int makeFileListTop(const char* top_dir)
{
	int		res;

	for (int i = 0; i < _countof(s_exp_list); i++) {
		if ((res = makeFileList(top_dir, "", s_exp_list[i])) != 0)
			return res;
	}
	return 0;
}


// ファイルを一つずつ取り出す
// 最後はNULLを返す
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


// オブジェクト行1行作成
const char* makeObjLine(char* mkline, size_t bf_sz, const char* fl_nm)
{
	sprintf_s( mkline, bf_sz, "%s.o \\\n", fl_nm);
	return mkline;
}


// main関数
int main(int argc, char* argv[])
{
	// 引数解釈
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
				printf("Error: 不明な引数[%s]\n", argv[i]);
				return usage();
			}
		}
		else {
			printf("Error: 不明な引数[%s]\n", argv[i]);
			return usage();
		}
	}
	if (makefile == NULL && top_dir != NULL) { return usage(); }
	if (!makefile)	{ printf("Error: makefileを指定してください\n\n");				return usage(); }
	if (!top_dir)	{ printf("Error: トップディレクトリを指定してください\n\n");	return usage(); }


	// トップディレクトリのチェック
	struct stat		st;
	int		ret = stat( top_dir, &st );
	if (ret != 0 || !(st.st_mode & _S_IFDIR)) {
		printf("Error: トップディレクトリ[%s]が見つかりません\n", top_dir);
		return 1;
	}

	// ファイルオープン
	FILE*	in = NULL;
	if (fopen_s( &in, makefile, "rt" ) != 0) {
		printf("Error: 出力ファイル[%s]を開けません\n", makefile);
		return 1;
	}

	// テンポラリファイルの作成
	char	tmp_file[_MAX_PATH];
	char	drive[_MAX_DRIVE], dir[_MAX_DIR];
	if (_splitpath_s( makefile, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0 )) {
		printf("Error: 一時ファイル名を作れません[%s]\n", makefile);
		return 1;
	}
	if (_makepath_s( tmp_file, _countof(tmp_file), drive, dir, "mfXXXXXX", NULL )) {
		printf("Error: 一時ファイル名を作れません[%s, %s]\n", drive, dir);
		return 1;
	}
	if (_mktemp_s(tmp_file, _countof(tmp_file))) {
		printf("Error: 一時ファイル名を作れません[%s]\n", tmp_file);
		return 1;
	}
	FILE*	out = NULL;
	if (fopen_s( &out, tmp_file, "wt" ) != 0) {
		printf("Error: 一時ファイル[%s]を開けません\n", tmp_file);
		return 1;
	}

	// ファイルリストの作成
	makeFileListTop(top_dir);

	// ファイルの書替え
	int			mode = 0;			// 0:.obj前 1:.obj中 2:.obj後
	bool		change = false;		// 変更あり

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
				// リスト終了?
				fl_nm = getFile();
				if (fl_nm != NULL) {
					// ファイルが増えた
					change = true;
					do {
						//出力
						fprintf(out, "%s", makeObjLine( mkline, _countof(mkline), fl_nm ) );
					} while ((fl_nm = getFile()) != NULL);
				}
				mode = 2;
			}
			else {
				fl_nm = getFile();
				if (fl_nm == NULL) {
					// ファイルが減った
					change = true;
					continue;
				}
				makeObjLine( mkline, _countof(mkline), fl_nm );
				// 変更有りか？
				if (!change && strcmp(line, mkline) != 0)
					change = true;
				wtline = mkline;
			}
			break;
		case 2:
			break;
		}
		//出力
		fprintf(out,"%s", wtline);
	}

	// close
	fclose(in);
	fclose(out);
#ifdef _DEBUG
	printf("%d個のオブジェクトファイル名が出力されました\n", file_list.size());
#endif

	// ファイルの入換え
	if (change) {
		if (remove(makefile)) {
			printf("Error: makefile[%s]を削除出来ません\n", tmp_file);
			return 1;
		}
		if (rename( tmp_file, makefile )) {
			printf("Error: 一時ファイル[%s]をmakfile[%s]に出来ません\n", tmp_file, makefile);
			return 1;
		}
	}
	else {
		if (remove(tmp_file)) {
			printf("Warning: 一時ファイル[%s]を削除出来ません\n", tmp_file);
		}
		printf("出力ファイルは最新です\n");
	}

	return 0;
}
