#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           名前                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// アプリ名。2007.09.21 kobake 整理
#ifdef _UNICODE
	#define _APP_NAME_(TYPE) TYPE("sakura")
#else
	#define _APP_NAME_(TYPE) TYPE("sakura")
#endif

#ifdef _DEBUG
	#define _APP_NAME_2_(TYPE) TYPE("(デバッグ版)")
#else
	#define _APP_NAME_2_(TYPE) TYPE("")
#endif

#define _GSTR_APPNAME_(TYPE)  _APP_NAME_(TYPE) _APP_NAME_2_(TYPE) //例:UNICODEデバッグ→_T("sakura(デバッグ版)")

#define GSTR_APPNAME    (_GSTR_APPNAME_(_T)   )
#define GSTR_APPNAME_A  (_GSTR_APPNAME_(ATEXT))
#define GSTR_APPNAME_W  (_GSTR_APPNAME_(LTEXT))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキストエリア                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Feb. 18, 2003 genta 最大値の定数化と値変更
const int LINESPACE_MAX = 128;
const int COLUMNSPACE_MAX = 64;

//	Aug. 14, 2005 genta 定数定義追加
// 2007.09.07 kobake 定数名変更: MAXLINESIZE→MAXLINEKETAS
// 2007.09.07 kobake 定数名変更: MINLINESIZE→MINLINEKETAS
const int MAXLINEKETAS		= 10240;	//!< 1行の桁数の最大値
const int MINLINEKETAS		= 10;		//!< 1行の桁数の最小値


