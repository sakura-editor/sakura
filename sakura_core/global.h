//	$Id$
/************************************************************
  global.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//////////////////////////////////////////////////////////////
#include <windows.h>
// 以下の ifdef ブロックは DLL から簡単にエクスポートさせるマクロを作成する標準的な方法です。 
// この DLL 内のすべてのファイルはコマンドラインで定義された SAKURA_CORE_EXPORTS シンボル
// でコンパイルされます。このシンボルはこの DLL が使用するどのプロジェクト上でも未定義でなけ
// ればなりません。この方法ではソースファイルにこのファイルを含むすべてのプロジェクトが DLL 
// からインポートされたものとして SAKURA_CORE_API 関数を参照し、そのためこの DLL はこのマク 
// ロで定義されたシンボルをエクスポートされたものとして参照します。
#ifdef SAKURA_CORE_EXPORTS
#define SAKURA_CORE_API __declspec(dllexport)
#else
#define SAKURA_CORE_API __declspec(dllimport)
#endif

#if defined(__BORLANDC__)
#define __forceinline
#define _itoa itoa
#define _stricmp stricmp
#define _memicmp memicmp
#ifndef __max
#define __max(a,b)	((a)>=(b)?(a):(b))
#endif
#ifndef STRICT
#define STRICT
#endif
#endif

#define IS_KEYWORD_CHAR(c) ((c) == '#' || (c) == '$' || __iscsym( (c) ))


SAKURA_CORE_API extern const char* GSTR_APPNAME;


#ifdef _DEBUG
	#ifndef	GSTR_EDITWINDOWNAME
	#define	GSTR_EDITWINDOWNAME "TextEditorWindow_DEBUG"
	#endif
#else
	#ifndef	GSTR_EDITWINDOWNAME
	#define	GSTR_EDITWINDOWNAME "TextEditorWindow"
	#endif
#endif

#ifdef _DEBUG
	#ifndef	GSTR_MUTEX_SAKURA
	#define	GSTR_MUTEX_SAKURA "MutexSakuraEditor_DEBUG"
	#endif
#else
	#ifndef	GSTR_MUTEX_SAKURA
	#define	GSTR_MUTEX_SAKURA "MutexSakuraEditor"
	#endif
#endif

#ifdef _DEBUG
	#ifndef	GSTR_CEDITAPP
#define	GSTR_CEDITAPP "CEditApp_DEBUG"
	#endif
#else
	#ifndef	GSTR_CEDITAPP
	#define	GSTR_CEDITAPP "CEditApp"
	#endif
#endif

#ifdef _DEBUG
	#ifndef	GSTR_CSHAREDATA
	#define	GSTR_CSHAREDATA "CShareData_DEBUG"
	#endif
#else
	#ifndef	GSTR_CSHAREDATA
	#define	GSTR_CSHAREDATA "CShareData"
	#endif
#endif

//#ifndef	GSTR_REG_ROOTPATH
//#define	GSTR_REG_ROOTPATH "Software\\BugSoft\\sakura"
//#endif


/* ウィンドウのID */
#define IDW_STATUSBAR 123


#define IDM_SELWINDOW		10000
#define IDM_SELMRU			11000
#define IDM_SELOPENFOLDER	12000


/* 文字コードセット種別 */
SAKURA_CORE_API enum enumCodeType {
	CODE_SJIS,				/* SJIS */
	CODE_JIS,				/* JIS */
	CODE_EUC,				/* EUC */
	CODE_UNICODE,			/* Unicode */
	CODE_UTF8,				/* UTF-8 */
	CODE_UTF7,				/* UTF-7 */
	CODE_CODEMAX,
	CODE_AUTODETECT	= 99	/* 文字コード自動判別 */
};

SAKURA_CORE_API extern const char* gm_pszCodeNameArr_1[];
SAKURA_CORE_API extern const char* gm_pszCodeNameArr_2[];
SAKURA_CORE_API extern const char* gm_pszCodeNameArr_3[];

//const char* GetCodeTypeName( enumCodeType nCodeTYpe )
//{ 
//	const char* pszCodeTypeNameArr[] = {
//		"SJIS",
//		"JIS",
//		"EUC",
//		"Unicode",
//		"UTF-8",
//		"UTF-7"
//	};
//	return pszCodeTypeNameArr[nCodeTYpe];
//}

/* アウトライン解析の種類 */
SAKURA_CORE_API enum enumOutlineType {
	OUTLINE_C,
	OUTLINE_CPP,
	OUTLINE_PLSQL,
	OUTLINE_TEXT,
	OUTLINE_JAVA,
	OUTLINE_COBOL,
	OUTLINE_ASM,
	OUTLINE_PERL,	//	Sep. 8, 2000 genta
	OUTLINE_CODEMAX,
	OUTLINE_UNKNOWN	= 99
};

/* 行終端子の種類 */
SAKURA_CORE_API enum enumEOLType {
	EOL_NONE,
	EOL_CRLF_UNICODE,
	EOL_CRLF,
	EOL_LFCR,
	EOL_LF,
	EOL_CR,
	EOL_CODEMAX,
	EOL_UNKNOWN = -1
};

/* 行終端子のデータ長 */
SAKURA_CORE_API enum enumEOLLen {
	LEN_EOL_NONE			= 0,
	LEN_EOL_CRLF_UNICODE	= 4,
	LEN_EOL_CRLF			= 2,
	LEN_EOL_LFCR			= 2,
	LEN_EOL_LF				= 1,
	LEN_EOL_CR				= 1
//	LEN_EOL_UNKNOWN		0
};


#define EOL_TYPE_NUM			6

/* 行終端子の配列 */
SAKURA_CORE_API extern const enumEOLType gm_pnEolTypeArr[EOL_TYPE_NUM]; 

//	May 15, 2000 genta
//	CEOLへ移動
//	From Here
/* 行終端子のデータの配列 */
// SAKURA_CORE_API extern const char* gm_pszEolDataArr[EOL_TYPE_NUM];

/* 行終端子のデータ長の配列 */
// SAKURA_CORE_API extern const int gm_pnEolLenArr[EOL_TYPE_NUM];

/* 行終端子の表示名の配列 */
// SAKURA_CORE_API extern const char* gm_pszEolNameArr[EOL_TYPE_NUM];

/* 行終端子の種類を調べる */
// SAKURA_CORE_API enumEOLType GetEOLType( const char*, int );
//	To Here

/* 選択領域描画用パラメータ */
SAKURA_CORE_API extern const COLORREF	SELECTEDAREA_RGB;
SAKURA_CORE_API extern const int		SELECTEDAREA_ROP2;


/* スマートインデント種別 */
SAKURA_CORE_API enum enumSmartIndentType {
	SMARTINDENT_NONE,
	SMARTINDENT_CPP
};

///////////////////////////////////////////////////////////////////////
#endif /* _GLOBAL_H_ */
