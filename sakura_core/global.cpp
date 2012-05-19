/*!	@file
	@brief 文字列共通定義

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, Stonee, jepro
	Copyright (C) 2002, KK
	Copyright (C) 2003, MIK
	Copyright (C) 2005, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
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
const SColorAttributeData g_ColorAttributeArr[] =
{
	{_T("TXT"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_EFFECTS},
	{_T("RUL"), COLOR_ATTRIB_NO_EFFECTS},
	{_T("CAR"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// キャレット		// 2006.12.07 ryoji
	{_T("IME"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// IMEキャレット	// 2006.12.07 ryoji
	{_T("UND"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{_T("CVL"), COLOR_ATTRIB_NO_BACK | ( COLOR_ATTRIB_NO_EFFECTS & ~COLOR_ATTRIB_NO_BOLD )}, // 2007.09.09 Moca カーソル位置縦線
	{_T("LNO"), 0},
	{_T("MOD"), 0},
	{_T("TAB"), 0},
	{_T("SPC"), 0},	//2002.04.28 Add By KK
	{_T("ZEN"), 0},
	{_T("CTL"), 0},
	{_T("EOL"), 0},
	{_T("RAP"), 0},
	{_T("VER"), 0},  // 2005.11.08 Moca 指定桁縦線
	{_T("EOF"), 0},
	{_T("NUM"), 0},	//@@@ 2001.02.17 by MIK 半角数値の強調
	{_T("FND"), 0},
	{_T("KW1"), 0},
	{_T("KW2"), 0},
	{_T("KW3"), 0},	//@@@ 2003.01.13 by MIK 強調キーワード3-10
	{_T("KW4"), 0},
	{_T("KW5"), 0},
	{_T("KW6"), 0},
	{_T("KW7"), 0},
	{_T("KW8"), 0},
	{_T("KW9"), 0},
	{_T("KWA"), 0},
	{_T("CMT"), 0},
	{_T("SQT"), 0},
	{_T("WQT"), 0},
	{_T("URL"), 0},
	{_T("RK1"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK2"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK3"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK4"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK5"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK6"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK7"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK8"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK9"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RKA"), 0},	//@@@ 2001.11.17 add MIK
	{_T("DFA"), 0},	//DIFF追加	//@@@ 2002.06.01 MIK
	{_T("DFC"), 0},	//DIFF変更	//@@@ 2002.06.01 MIK
	{_T("DFD"), 0},	//DIFF削除	//@@@ 2002.06.01 MIK
	{_T("BRC"), 0},	//対括弧	// 02/09/18 ai Add
	{_T("MRK"), 0},	//ブックマーク	// 02/10/16 ai Add
	{_T("LAST"), 0}	// Not Used
};


/*[EOF]*/
