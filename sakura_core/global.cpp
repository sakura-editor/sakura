//	$Id$
/************************************************************

global.cpp
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************/
#include "global.h"
#include <memory.h>


#ifdef _DEBUG
//	const char* GSTR_APPNAME = "テキストエディタ(デバッグバージョン)";
	const char* GSTR_APPNAME = "sakura(デバッグ版)";
#else
//	const char* GSTR_APPNAME = "テキストエディタ";
	const char* GSTR_APPNAME = "sakura";
#endif


const char* gm_pszCodeNameArr_1[] = {
	"SJIS",				/* SJIS */
	"JIS",				/* JIS */
	"EUC",				/* EUC */
	"Unicode",			/* Unicode */
	"UTF-8",			/* UTF-8 */
	"UTF-7"				/* UTF-7 */
};

const char* gm_pszCodeNameArr_2[] = {
	"SJIS",				/* SJIS */
	"JIS",				/* JIS */
	"EUC",				/* EUC */
	"Uni",				/* Unicode */
	"UTF-8",			/* UTF-8 */
	"UTF-7"				/* UTF-7 */
};

const char* gm_pszCodeNameArr_3[] = {
	"[SJIS]",			/* SJIS */
	"[JIS]",			/* JIS */
	"[EUC]",			/* EUC */
	"[Unicode]",		/* Unicode */
	"[UTF-8]",			/* UTF-8 */
	"[UTF-7]"			/* UTF-7 */
};

/* 選択領域描画用パラメータ */
const COLORREF	SELECTEDAREA_RGB = RGB( 255, 255, 255 );
const int		SELECTEDAREA_ROP2 = R2_XORPEN;


/* 行終端子の配列 */
const enumEOLType gm_pnEolTypeArr[EOL_TYPE_NUM] = {
	EOL_NONE			,	// == 0
	EOL_CRLF_UNICODE	,	// == 4
	EOL_CRLF			,	// == 2
	EOL_LFCR			,	// == 2
	EOL_LF				,	// == 1
	EOL_CR					// == 1
};

