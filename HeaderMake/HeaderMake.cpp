/*!	@file

	@author Project Sakura-Editor
	@date 2007.09.30 kobake 作成
	@date 2009.07.06 rastiv #pragma once を除去して #ifndef-#endif 文に変更
*/
/*
	Copyright (C) 2007-2008, kobake
	Copyright (C) 2009, rastiv


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

	enum と define の2種類の定数定義ヘッダを生成するためのモノ

	enum定数はソースコードから参照し、(デバッグがしやすくなる)
	define定数はリソースから参照する  (リソース内ではenum定数を利用できない)


	++ 依存 ++

	Visual Studio の cl.exe に依存。
	cl.exe のプリプロセッサ機能を利用しています。


	++ 使用方法 ++

	usage() を参照
*/

#ifdef __MINGW32__
#include <_mingw.h>
#ifdef MINGW_HAS_SECURE_API
#undef MINGW_HAS_SECURE_API
#endif  // MINGW_HAS_SECURE_API
#endif  // __MINGW32__

#include <stdio.h>
#include <string>
#include <vector>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
using namespace std;

#define PREPROCESSOR "cl.exe /EP %s"

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
#define fopen_s(A, B, C) ( *(A) = fopen((B), (C)) )
#endif	// VC2003
#endif	// _MSC_VER

enum EMode{
	MODE_INVALID,

	MODE_ENUM,
	MODE_DEFINE,
};

int usage()
{
	printf(
		"\nUsage: HeaderMake -in=<InputFile.hsrc> -out=<OutputFile.h> -mode=<Mode> [-enum=<EnumName>]\n"
		"\n"
		"  Argument parameters\n"
		"    InputFile           : Input .hsrc file path\n"
		"    OutputFile          : Output .h file path\n"
		"    Mode                : define|enum\n"
		"    EnumName (Optional) : Enum name (when enum mode only)\n"
		"\n"
		"  Mode\n"
		"    define : Output .h file as #define list\n"
		"    enum   : Output .h file as enum list\n"
	);
	return 1;
}

inline bool is_token(char c)
{
	return __iscsym(c) || c=='-';
}

//! 後ろから検索し、c1 か c2 、先に見つかったほうのアドレスを返す
const char* strrchr2(const char* str, char c1, char c2)
{
	const char* p=strchr(str,'\0');
	//※whileを先行すると、c1,c2が'\0'のときにNULLを返してしまうので、do～whileとした。
	do{
		if(*p==c1 || *p==c2)return p;
	}while(--p>=str);
	return NULL;
}

const char* GetFileTitlePointer(const char* fpath)
{
	const char* p=strrchr2(fpath,'\\','/');
	if(p){
		return p+1;
	}
	else{
		return fpath;
	}
}

int main_impl(
	const char* in_file,
	const char* out_file,
	const char* mode_name,
	const char* enum_name
);

int main(int argc, char* argv[])
{
	if (argc <= 1) {
		return usage();
	}

	//引数解釈
	const char* in_file = NULL;
	const char* out_file = NULL;
	const char* mode_name = NULL;
	const char* enum_name = "";
	for (int i = 1; i<argc; i++) {
		char* p = argv[i];
		if (*p == '/')*p = '-';
		if (strncmp(p, "-in", 3) == 0) {
			p += 3;
			if (*p != '\0') { if (*p == '=')p++; in_file = p; }
			else in_file = argv[++i];
		}
		else if (strncmp(p, "-out", 4) == 0) {
			p += 4;
			if (*p != '\0') { if (*p == '=')p++; out_file = p; }
			else out_file = argv[++i];
		}
		else if (strncmp(p, "-mode", 5) == 0) {
			p += 5;
			if (*p != '\0') { if (*p == '=')p++; mode_name = p; }
			else mode_name = argv[++i];
		}
		else if (strncmp(p, "-enum", 5) == 0) {
			p += 5;
			if (*p != '\0') { if (*p == '=')p++; enum_name = p; }
			else enum_name = argv[++i];
		}
		else {
			printf("Error: Unknown argument[%s]\n", p);
			return 1;
		}
	}
	if (!in_file)   { printf("Error: Specify <InputFile> argument.\n");  return usage(); }
	if (!out_file)  { printf("Error: Specify <OutputFile> argument.\n"); return usage(); }
	if (!mode_name) { printf("Error: Specify <Mode> argument.\n");       return usage(); }

	// 開始メッセージ
	printf("\nSTART HeaderMake.\n");
	printf("CMDLINE: ");
	for (int i = 0; i < argc; i++) {
		printf("%s ", argv[i]);
	}
	printf("\n\n");

	// 処理
	main_impl(in_file, out_file, mode_name, enum_name);

	// 終了メッセージ
	printf("\nEND HeaderMake.\n\n");
}

int main_impl(
	const char* in_file,
	const char* out_file,
	const char* mode_name,
	const char* enum_name
)
{
	//モード解釈
	EMode mode=MODE_INVALID;
	if(_stricmp(mode_name,"DEFINE")==0)mode=MODE_DEFINE;
	else if(_stricmp(mode_name,"ENUM")==0)mode=MODE_ENUM;
	else{
		printf("Error: Unknown mode[%s].\n", mode_name);
		return 2;
	}


	//ファイル更新時刻比較 (なんか、VSのカスタムビルドがうまくいかないので、ここで判定(汗) )
	struct _stat in_stat;
	struct _stat out_stat;
	if(-1==_stat(in_file, &in_stat ))goto next;
	if(-1==_stat(out_file,&out_stat))goto next;
	//入力ファイルが新しければ処理を続行する
	if(in_stat.st_mtime>out_stat.st_mtime)goto next;
	else{ printf("OutputFile[%s] needs no change.\n", out_file); return 0; }


next:
	// プリプロセス済みストリームオープン
	char in_file2[_MAX_PATH];
	sprintf_s(in_file2,_countof(in_file2),PREPROCESSOR,in_file);
	FILE* in = _popen(in_file2, "rt");
	if (!in) { printf("Error: Failed to open process[%s]\n", in_file2); return 1; }
	
	// 出力ファイルオープン
	FILE* out = NULL; fopen_s(&out, out_file, "wt");
	if (!out) { printf("Error: Failed to open OutputFile[%s] as write mode\n", out_file); return 1; }


	//処理

	if(mode==MODE_DEFINE){
		fprintf(out,
			"#ifndef SAKURA_HEADERMAKE_98B26AB2_D5C9_4884_8D15_D1F3A2936253_H_\n"
			"#define SAKURA_HEADERMAKE_98B26AB2_D5C9_4884_8D15_D1F3A2936253_H_\n"
			"\n"
		);	// インクルードガードを挿入（define 用）
	}else if(mode==MODE_ENUM){
		fprintf(out,
			"#ifndef SAKURA_HEADERMAKE_2034D8F5_AE65_408D_9F53_D3DEA240C67BI_H_\n"
			"#define SAKURA_HEADERMAKE_2034D8F5_AE65_408D_9F53_D3DEA240C67BI_H_\n"
			"\n"
		);	// インクルードガードを挿入（enum 用）
	}else{
		; // 何も出力しない
	}

	char line[1024];
	fprintf(out,
		"// This file is generated by HeaderMake with %s.\n"
		"// Don't edit this file manually.\n\n",
		GetFileTitlePointer(in_file)
	);
	if(mode==MODE_ENUM)fprintf(out,"enum %s{\n",enum_name); //enum開始
	while(NULL!=fgets(line,_countof(line),in))
	{
		// #無視(for MinGW)
		if('#' == line[0]){
			continue;
		}

		//改行除去
		{
			char* p=strrchr(line,'\n');
			if(p && *(p+1)=='\0')*p='\0';
		}

		//トークン抽出開始
		const char* p=line;
		const char* q;

		//ID文字列 -> id
		char id[256];
		while(*p && !is_token(*p))p++;
		q=p;
		while(*q && is_token(*q))q++;
		strncpy_s(id,_countof(id),p,q-p);
		id[q-p]='\0';
		p=q;

		//値 -> value
		char value[256];
		while(*p && !is_token(*p))p++;
		q=p;
		while(*q && is_token(*q))q++;
		strncpy_s(value,_countof(value),p,q-p);
		value[q-p]='\0';
		p=q;

		//出力
		if(*id && *value){
			if(mode==MODE_ENUM){
				fprintf(out,"\t%s = %s,\n",id,value);
			}
			else if(mode==MODE_DEFINE){
				fprintf(out,"#define %s %s\n",id,value);
			}else{
				; // mode が不正の場合は何も出力しない
			}
		}
	}

	if(mode==MODE_DEFINE){
		fprintf(out,
			"\n"
			"#endif /* SAKURA_HEADERMAKE_98B26AB2_D5C9_4884_8D15_D1F3A2936253_H_ */\n"
			"/*[EOF]*/\n"
		);	// インクルードガード終了（define 用）
	}else if(mode==MODE_ENUM){
		fprintf(out,
			"};\n"
			"\n"
			"#endif /* SAKURA_HEADERMAKE_2034D8F5_AE65_408D_9F53_D3DEA240C67BI_H_ */\n"
			"/*[EOF]*/\n"
		);	// インクルードガード終了（enum 用）
	}else{
		; // 何も出力しない
	}


	//後始末
	fclose(out);
	fclose(in);

	return 0;
}

