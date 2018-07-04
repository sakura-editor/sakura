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
#include <regex>
#include <unordered_map>
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

#define BUF_SIZE	1024

/*** Funccode_x.hsrc の引数の順番を定義 *************************************/
/*
	TYPE, ID の位置は全 type 共通
	ALIAS 以外は，TYPE ～ VAL の位置は共通
	引数の _MIN, _MAX 数も定義する (引数が省略できる type 用)
*/
enum {
	ID_TYPE,
	ID_ID,
	ID_VAL,
	ID__MAX,
	ID__MIN = ID__MAX,
};

enum {
	CMD_TYPE,
	CMD_ID,
	CMD_VAL,
	CMD_NAME,
	CMD_RETARG,
	CMD_EXT,
	CMD__MAX,
	CMD__MIN = CMD_NAME + 1,
};

enum {
	FUNC_TYPE,
	FUNC_ID,
	FUNC_VAL,
	FUNC_NAME,
	FUNC_RETARG,
	FUNC_EXT,
	FUNC__MAX,
	FUNC__MIN = FUNC_NAME + 1,
};

enum {
	ALIAS_TYPE,
	ALIAS_ID,
	ALIAS_NAME,
	ALIAS__MAX,
	ALIAS__MIN = ALIAS__MAX,
};

// retarg の最大長，MacroFuncInfo.m_varArguments[] の要素数 + 1
#define MAX_RETARG_NUM	5

/*** 正規表現を使用する場所で wregex するためのヘルパ関数 *******************/
// 書きやすさ最優先で，実行効率は求めない．

unordered_map<wstring, wregex *> RegTbl;

wregex &regex_helper( const wstring& strRe, const char *file, const int line ){
	wregex **regex = &RegTbl[ strRe ];
	
	if( *regex == NULL ){
		try {
			*regex = new wregex( strRe );
		}catch( std::regex_error& e ){
			printf( "%s(%d): %s\n", file, line, e.what());
			exit( 1 );
		}
	}
	
	return **regex;
}

#define re( reg ) regex_helper( wstring( L##reg ), __FILE__, __LINE__ )

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
	if(
		-1 != _stat(in_file, &in_stat ) &&
		-1 != _stat(out_file,&out_stat) &&
		//入力ファイルが新しければ処理を続行する
		in_stat.st_mtime < out_stat.st_mtime
	){ printf("OutputFile[%s] needs no change.\n", out_file); return 0; }

	// プリプロセス済みストリームオープン
	char in_file2[_MAX_PATH];
	sprintf_s(in_file2,_countof(in_file2),PREPROCESSOR,in_file);
	FILE* in = _popen(in_file2, "rt");
	if (!in) { printf("Error: Failed to open process[%s]\n", in_file2); return 1; }
	
	/*** 入力 *****************************************************************/
	
	wchar_t line[ BUF_SIZE ];
	
	// Funccode_x.hsrc の内容を単なる 2次元配列的に収めたもの
	vector<vector<wstring>> FuncTbl;
	
	// ID をキーとして 1行分のデータを参照できる map
	unordered_map<wstring, unsigned int> FuncTblHash;
	
	while( fgetws( line, BUF_SIZE, in )){
		
		wstring str( line );
		
		// 改行削除
		str = regex_replace( str, re( "[\\x0D\\x0A]" ), L"" );
		
		// コメント, # 削除
		str = regex_replace( str, re( "\\s*(?://|#).*" ), L"" );
		
		// 行末スペース削除
		str = regex_replace( str, re( "\\s+$" ), L"" );
		
		// 空行スキップ
		if( regex_match( str, re( "\\s*" ))) continue;
		
		// ',' で split
		wsregex_token_iterator begin{ str.begin(), str.end(), re( "\\s*,\\s*" ), -1 }, end;
		FuncTbl.emplace_back();
		auto& Data = FuncTbl.back();
		copy( begin, end, back_inserter( Data ));
		
		// type の確認
		if(
			Data[ ID_TYPE ] != L"ID"	&&
			Data[ ID_TYPE ] != L"CMD"	&&
			Data[ ID_TYPE ] != L"FUNC"	&&
			Data[ ID_TYPE ] != L"ALIAS"
		){
			wprintf( L"unknown type: %s", line );
			continue;
		}
		
		// 引数の数チェック
		#define CheckArgNum( type ) \
			( Data[ ID_TYPE ] == L## #type && type##__MIN <= Data.size() && Data.size() <= type##__MAX)
		
		if(
			!CheckArgNum( ID ) &&
			!CheckArgNum( CMD ) &&
			!CheckArgNum( FUNC ) &&
			!CheckArgNum( ALIAS )
		){
			wprintf( L"invalid argument num: %s", line );
			continue;
		}
		
		// id と Data の map 登録
		FuncTblHash.emplace( Data[ ID_ID ], FuncTbl.size() - 1 );
	}
	
	fclose(in);
	
	/*** 出力 ***************************************************************/
	
	// 出力ファイルオープン
	FILE* out = NULL; fopen_s(&out, out_file, "wt");
	if (!out) { printf("Error: Failed to open OutputFile[%s] as write mode\n", out_file); return 1; }


	//処理
	fprintf(out,
		"#pragma once\n"
		"/* HeaderMake generate start */\n"
		"\n"
		"// This file is generated by HeaderMake with %s.\n"
		"// Don't edit this file manually.\n\n",
		GetFileTitlePointer(in_file)
	);
	
	// define, enum の出力
	// ALIAS 以外の id, val を出力する
	
	if(mode==MODE_ENUM)fprintf(out,"enum %s{\n",enum_name); //enum開始
	
	for( auto itr = FuncTbl.begin(); itr != FuncTbl.end(); ++itr ){
		auto Data = *itr;
		
		if( Data[ ID_TYPE ] == L"ALIAS" ) continue;
		
		if(mode==MODE_ENUM){
			fwprintf( out, L"\t%s = %s,\n", Data[ ID_ID ].c_str(), Data[ ID_VAL ].c_str());
		}else if(mode==MODE_DEFINE){
			fwprintf( out, L"#define %s %s\n", Data[ ID_ID ].c_str(), Data[ ID_VAL ].c_str());
		}
	}

	if(mode==MODE_ENUM) fprintf( out, "};\n" );
	fprintf(out,
		"\n"
		"/* HeaderMake generate end */\n"
	);

	//後始末
	fclose(out);

	return 0;
}

