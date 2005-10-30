//	$Id$
/*!	@file
	@brief 文字列共通定義

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "global.h"


#ifdef _DEBUG
	const char* GSTR_APPNAME = "sakura(デバッグ版)";
#else
	const char* GSTR_APPNAME = "sakura";
#endif


const char* gm_pszCodeNameArr_1[] = {
	"SJIS",				/* SJIS */
	"JIS",				/* JIS */
	"EUC",				/* EUC */
	"Unicode",			/* Unicode */
	"UTF-8",			/* UTF-8 */
	"UTF-7",			/* UTF-7 */
	"UniBE"				/* Unicode BigEndian */
};

const char* gm_pszCodeNameArr_2[] = {
	"SJIS",				/* SJIS */
	"JIS",				/* JIS */
	"EUC",				/* EUC */
	"Uni",				/* Unicode */
	"UTF-8",			/* UTF-8 */
	"UTF-7",			/* UTF-7 */
	"UniBE"				/* Unicode BigEndian */
};

const char* gm_pszCodeNameArr_3[] = {
	"  [SJIS]",			/* SJIS */
	"  [JIS]",			/* JIS */
	"  [EUC]",			/* EUC */
	"  [Unicode]",		/* Unicode */
	"  [UTF-8]",		/* UTF-8 */
	"  [UTF-7]",		/* UTF-7 */
	"  [UniBE]"			/* Unicode BigEndian */
};

const int gm_nCodeComboValueArr[] = {
	CODE_AUTODETECT,	/* 文字コード自動判別 */
	CODE_SJIS,
	CODE_JIS,
	CODE_EUC,
	CODE_UNICODE,
	CODE_UNICODEBE,
	CODE_UTF8,
	CODE_UTF7
};
const char* const	gm_pszCodeComboNameArr[] = {
	"自動選択",
	"SJIS",
	"JIS",
	"EUC",
	"Unicode",
	"UnicodeBE",
	"UTF-8",
	"UTF-7"
};

const int gm_nCodeComboNameArrNum = sizeof( gm_nCodeComboValueArr ) / sizeof( gm_nCodeComboValueArr[0] );


/*! 選択領域描画用パラメータ */
const COLORREF	SELECTEDAREA_RGB = RGB( 255, 255, 255 );
const int		SELECTEDAREA_ROP2 = R2_XORPEN;

/*! 行終端子の配列 */
const enumEOLType gm_pnEolTypeArr[EOL_TYPE_NUM] = {
	EOL_NONE			,	// == 0
	EOL_CRLF			,	// == 2
	EOL_LFCR			,	// == 2
	EOL_LF				,	// == 1
	EOL_CR					// == 1
};

/*! キーワードキャラクタ */
const unsigned char gm_keyword_char[256] = {
 /* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F      : 0123456789ABCDEF */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 0: ................ */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 1: ................ */
	0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 2:  !"#$%&'()*+,-./ */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,	/* 3: 0123456789:;<=>? */
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* 4: @ABCDEFGHIJKLMNO */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 1,	/* 5: PQRSTUVWXYZ[\]^_ */
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* 6: `abcdefghijklmno */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,	/* 7: pqrstuvwxyz{|}~. */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 8: ................ */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 9: ................ */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* A: .｡｢｣､･ｦｧｨｩｪｫｬｭｮｯ */	//setlocal( LC_ALL, "C" )
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* B: ｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿ */	//setlocal( LC_ALL, "C" )
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* C: ﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏ */	//setlocal( LC_ALL, "C" )
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* D: ﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝﾞﾟ */	//setlocal( LC_ALL, "C" )
//	0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* A: .｡｢｣､･ｦｧｨｩｪｫｬｭｮｯ */	//setlocal( LC_ALL, "Japanese" )
//	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* B: ｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿ */	//setlocal( LC_ALL, "Japanese" )
//	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* C: ﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏ */	//setlocal( LC_ALL, "Japanese" )
//	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* D: ﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝﾞﾟ */	//setlocal( LC_ALL, "Japanese" )
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* E: ................ */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* F: ................ */
	/* 0: not-keyword, 1:__iscsym(), 2:user-define */
};

/*!
  iniの色設定を番号でなく文字列で書き出す。(added by Stonee, 2001/01/12, 2001/01/15)
  配列の順番は共有メモリ中のデータの順番と一致している。

  @note 数値による内部的対応はglobal.hで行っているので参照のこと。(Mar. 7, 2001 jepro)
  CShareDataからglobalに移動
*/
const char* const colorIDXKeyName[] =
{
	"TXT",
	"RUL",
	"UND",
	"LNO",
	"MOD",
	"TAB",
	"SPC",	//2002.04.28 Add By KK
	"ZEN",
	"CTL",
	"EOL",
	"RAP",
	"EOF",
	"NUM",	//@@@ 2001.02.17 by MIK 半角数値の強調
	"FND",
	"KW1",
	"KW2",
	"KW3",	//@@@ 2003.01.13 by MIK 強調キーワード3-10
	"KW4",
	"KW5",
	"KW6",
	"KW7",
	"KW8",
	"KW9",
	"KWA",
	"CMT",
	"SQT",
	"WQT",
	"URL",
	"RK1",	//@@@ 2001.11.17 add MIK
	"RK2",	//@@@ 2001.11.17 add MIK
	"RK3",	//@@@ 2001.11.17 add MIK
	"RK4",	//@@@ 2001.11.17 add MIK
	"RK5",	//@@@ 2001.11.17 add MIK
	"RK6",	//@@@ 2001.11.17 add MIK
	"RK7",	//@@@ 2001.11.17 add MIK
	"RK8",	//@@@ 2001.11.17 add MIK
	"RK9",	//@@@ 2001.11.17 add MIK
	"RKA",	//@@@ 2001.11.17 add MIK
	"DFA",	//DIFF追加	//@@@ 2002.06.01 MIK
	"DFC",	//DIFF変更	//@@@ 2002.06.01 MIK
	"DFD",	//DIFF削除	//@@@ 2002.06.01 MIK
	"BRC",	//対括弧	// 02/09/18 ai Add
	"MRK",	//ブックマーク	// 02/10/16 ai Add
	"LAST"	// Not Used
};


/*[EOF]*/
