//	$Id$
/*!	@file
	@brief 共通定義

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//////////////////////////////////////////////////////////////
#include <windows.h>
#include <tchar.h>
//#include <locale.h>

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

#ifdef SAKURA_NO_DLL	//@@@ 2001.12.30 add MIK
#undef SAKURA_CORE_API
#define SAKURA_CORE_API
#endif	//SAKURA_NO_DLL

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

//Oct. 31, 2000 JEPRO TeX Keyword のために'\'を追加	//Nov. 9, 2000 JEPRO HSP Keyword のために'@'を追加
//#define IS_KEYWORD_CHAR(c) ((c) == '#' || (c) == '$' || __iscsym( (c) ))
//#define IS_KEYWORD_CHAR(c) ((c) == '#'/*35*/ || (c) == '$'/*36*/ || (c) == '@'/*64*/ || (c) == '\\'/*92*/ || __iscsym( (c) ))
SAKURA_CORE_API	extern const unsigned char gm_keyword_char[256];	//@@@ 2002.04.27
#define IS_KEYWORD_CHAR(c)	((int)(gm_keyword_char[(unsigned char)((c) & 0xff)]))	//@@@ 2002.04.27 ロケールに依存しない


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

//20020108 aroka コントロールプロセスと起動処理のためにミューテックス名を追加
#ifdef _DEBUG
	#ifndef	GSTR_MUTEX_SAKURA
	#define	GSTR_MUTEX_SAKURA "MutexSakuraEditor_DEBUG"
	#define	GSTR_MUTEX_SAKURA_CP _T("MutexSakuraEditorCP_DEBUG")
	#define	GSTR_MUTEX_SAKURA_INIT _T("MutexSakuraEditorInit_DEBUG")
	#endif
#else
	#ifndef	GSTR_MUTEX_SAKURA
	#define	GSTR_MUTEX_SAKURA "MutexSakuraEditor"
	#define	GSTR_MUTEX_SAKURA_CP _T("MutexSakuraEditorCP")
	#define	GSTR_MUTEX_SAKURA_INIT _T("MutexSakuraEditorInit")
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
	CODE_UNICODEBE,			/* Unicode BigEndian */
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
	OUTLINE_VB,	//	June 23, 2001 N.Nakatani
	OUTLINE_FILE,		//	2002.04.01 YAZAKI ルールファイル用
	OUTLINE_CODEMAX,
	OUTLINE_BOOKMARK,	//	2001.12.03 hor
	OUTLINE_DEFAULT =-1,//	2001.12.03 hor
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


#define EOL_TYPE_NUM	6

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




// Stonee 注： 2000/01/12
// ここを変更したときは、global.cpp のcolorIDXKeyNameの定義も変更して下さい。
//	From Here Sept. 18, 2000 JEPRO 順番を大幅に入れ替えた
SAKURA_CORE_API enum enumColorIndexType {
	COLORIDX_TEXT		= 0,	/* テキスト */
	COLORIDX_RULER		= 1,	/* ルーラー */
	COLORIDX_UNDERLINE	= 2,	/* カーソル行アンダーライン */
	COLORIDX_GYOU		= 3,	/* 行番号 */
	COLORIDX_GYOU_MOD	= 4,	/* 行番号(変更行) */
	COLORIDX_TAB		= 5,	/* TAB記号 */
	COLORIDX_SPACE		= 6,	/* 半角空白 */ //2002.04.28 Add by KK 以降全て+1
	COLORIDX_ZENSPACE	= 7,	/* 日本語空白 */
	COLORIDX_CTRLCODE	= 8,	/* コントロールコード */
	COLORIDX_CRLF		= 9,	/* 改行記号 */
	COLORIDX_WRAP		= 10,	/* 折り返し記号 */
	COLORIDX_EOF		= 11,	/* EOF記号 */
	COLORIDX_DIGIT		= 12,	/* 半角数値 */	//@@@ 2001.02.17 by MIK	//色設定Ver.3からユーザファイルに対しては文字列で処理しているのでリナンバリングしてもよい. Mar. 7, 2001 JEPRO noted
	COLORIDX_SEARCH		= 13,	/* 検索文字列 */
	COLORIDX_KEYWORD1	= 14,	/* 強調キーワード1 */ // 2002/03/13 novice
	COLORIDX_KEYWORD2	= 15,	/* 強調キーワード2 */ // 2002/03/13 novice	//MIK ADDED
	COLORIDX_COMMENT	= 16,	/* 行コメント */						//Dec. 4, 2000 shifted by MIK
	COLORIDX_SSTRING	= 17,	/* シングルクォーテーション文字列 */	//Dec. 4, 2000 shifted by MIK
	COLORIDX_WSTRING	= 18,	/* ダブルクォーテーション文字列 */		//Dec. 4, 2000 shifted by MIK
	COLORIDX_URL		= 19,	/* URL */								//Dec. 4, 2000 shifted by MIK
	COLORIDX_REGEX1		= 20,	/* 正規表現キーワード1 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX2		= 21,	/* 正規表現キーワード2 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX3		= 22,	/* 正規表現キーワード3 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX4		= 23,	/* 正規表現キーワード4 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX5		= 24,	/* 正規表現キーワード5 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX6		= 25,	/* 正規表現キーワード6 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX7		= 26,	/* 正規表現キーワード7 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX8		= 27,	/* 正規表現キーワード8 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX9		= 28,	/* 正規表現キーワード9 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX10	= 29,	/* 正規表現キーワード10 */  //@@@ 2001.11.17 add MIK
	COLORIDX_DIFF_APPEND = 30,	/* DIFF追加 */	//@@@ 2002.06.01 MIK
	COLORIDX_DIFF_CHANGE = 31,	/* DIFF追加 */	//@@@ 2002.06.01 MIK
	COLORIDX_DIFF_DELETE = 32,	/* DIFF追加 */	//@@@ 2002.06.01 MIK
	COLORIDX_BRACKET_PAIR = 33,	/* 対括弧 */	// 02/09/18 ai Add

	//カラーの最後
	COLORIDX_LAST		= 34,	//Dec. 4, 2000 @@@2001.02.17 renumber by MIK				/* 02/09/18 ai Mod 33→34 */

	//カラー表示制御用
	COLORIDX_BLOCK1		= 35,	/* ブロックコメント1(文字色と背景色は行コメントと同じ) */	/* 02/09/18 ai Mod 34→35 */
	COLORIDX_BLOCK2		= 36	/* ブロックコメント2(文字色と背景色は行コメントと同じ) */	/* 02/09/18 ai Mod 35→36 */

	//1000-1099 : カラー表示制御用(正規表現キーワード)
};
//	To Here Sept. 18, 2000



/*!< 色タイプ */
SAKURA_CORE_API extern const char* const colorIDXKeyName[];

/*!< 設定値の上限・下限 */
//	ルーラの高さ
#define IDC_SPIN_nRulerHeight_MIN	2
#define IDC_SPIN_nRulerHeight_MAX	32

// novice 2002/09/14
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef BOOL
#define BOOL	int
#endif

///////////////////////////////////////////////////////////////////////
#endif /* _GLOBAL_H_ */


/*[EOF]*/
