// 2000.10.08 JEPRO  背景色を真っ白RGB(255,255,255)→(255,251,240)に変更(眩しさを押さえた)
// 2000.12.09 Jepro  note: color setting (詳細は CshareData.h を参照のこと)
// 2000.09.04 JEPRO  シングルクォーテーション文字列に色を割り当てるが色分け表示はしない
// 2000.10.17 JEPRO  色分け表示するように変更(最初のFALSE→TRUE)
// 2008.03.27 kobake 大整理

#include "stdafx.h"
#include "CDocTypeSetting.h"


//! 色設定(保存用)
struct ColorInfoIni {
	const TCHAR*	m_pszName;			//!< 項目名
	ColorInfoBase	m_sColorInfo;		//!< 色設定
};

static ColorInfoIni ColorInfo_DEFAULT[] = {
//	項目名,									表示,		太字,		下線,		文字色,					背景色,
	_T("テキスト"),							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),
	_T("ルーラー"),							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 239, 239, 239 ),
	_T("カーソル"),							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	// 2006.12.07 ryoji
	_T("カーソル(IME ON)"),					TRUE,		FALSE,		FALSE,		RGB( 255,   0,   0 ),	RGB( 255, 251, 240 ),	// 2006.12.07 ryoji
	_T("カーソル行アンダーライン"),			TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
	_T("カーソル位置縦線"),					FALSE,		FALSE,		FALSE,		RGB( 128, 128, 255 ),	RGB( 255, 251, 240 ),	// 2007.09.09 Moca
	_T("行番号"),							TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ),
	_T("行番号(変更行)"),					TRUE,		TRUE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ),
	_T("TAB記号"),							TRUE,		FALSE,		FALSE,		RGB( 128, 128, 128 ),	RGB( 255, 251, 240 ),	//Jan. 19, 2001 JEPRO RGB(192,192,192)より濃いグレーに変更
	_T("半角空白")		,					FALSE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ), //2002.04.28 Add by KK
	_T("日本語空白"),						TRUE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ),
	_T("コントロールコード"),				TRUE,		FALSE,		FALSE,		RGB( 255, 255,   0 ),	RGB( 255, 251, 240 ),
	_T("改行記号"),							TRUE,		FALSE,		FALSE,		RGB(   0, 128, 255 ),	RGB( 255, 251, 240 ),
	_T("折り返し記号"),						TRUE,		FALSE,		FALSE,		RGB( 255,   0, 255 ),	RGB( 255, 251, 240 ),
	_T("指定桁縦線"),						FALSE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ), //2005.11.08 Moca
	_T("EOF記号"),							TRUE,		FALSE,		FALSE,		RGB(   0, 255, 255 ),	RGB(   0,   0,   0 ),
	_T("半角数値"),							FALSE,		FALSE,		FALSE,		RGB( 235,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2001.02.17 by MIK		//Mar. 7, 2001 JEPRO RGB(0,0,255)を変更  Mar.10, 2001 標準は色なしに
	_T("対括弧の強調表示"),					FALSE,		TRUE,		FALSE,		RGB( 128,   0,   0 ),	RGB( 255, 251, 240 ),	// 02/09/18 ai
	_T("選択範囲"),							TRUE,		FALSE,		FALSE,		RGB(  49, 106, 197 ),	RGB(  49, 106, 197 ),	//2011.05.18
	_T("検索文字列"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 255,   0 ),
	_T("検索文字列2"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 160, 255, 255 ),
	_T("検索文字列3"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 153, 255, 153 ),
	_T("検索文字列4"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 153, 153 ),
	_T("検索文字列5"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 102, 255 ),
	_T("コメント"),							TRUE,		FALSE,		FALSE,		RGB(   0, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("シングルクォーテーション文字列"),	TRUE,		FALSE,		FALSE,		RGB(  64, 128, 128 ),	RGB( 255, 251, 240 ),
	_T("ダブルクォーテーション文字列"),		TRUE,		FALSE,		FALSE,		RGB( 128,   0,  64 ),	RGB( 255, 251, 240 ),
	_T("URL"),								TRUE,		FALSE,		TRUE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
	_T("強調キーワード1"),					TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
	_T("強調キーワード2"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO キーワード1とは違う色に変更
	_T("強調キーワード3"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO キーワード1とは違う色に変更
	_T("強調キーワード4"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("強調キーワード5"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("強調キーワード6"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("強調キーワード7"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("強調キーワード8"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("強調キーワード9"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("強調キーワード10"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
	_T("正規表現キーワード1"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("正規表現キーワード2"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("正規表現キーワード3"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("正規表現キーワード4"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("正規表現キーワード5"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("正規表現キーワード6"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("正規表現キーワード7"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("正規表現キーワード8"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("正規表現キーワード9"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("正規表現キーワード10"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
	_T("DIFF差分表示(追加)"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
	_T("DIFF差分表示(変更)"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
	_T("DIFF差分表示(削除)"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
	_T("ブックマーク"),						TRUE ,		FALSE,		FALSE,		RGB( 255, 251, 240 ),	RGB(   0, 128, 192 ),	// 02/10/16 ai
};

void GetDefaultColorInfo(ColorInfo* pColorInfo, int nIndex)
{
	ColorInfoBase* p = pColorInfo;
	*p = ColorInfo_DEFAULT[nIndex].m_sColorInfo; //ColorInfoBase
	_tcscpy(pColorInfo->m_szName, ColorInfo_DEFAULT[nIndex].m_pszName);
	pColorInfo->m_nColorIdx = nIndex;
}

int GetDefaultColorInfoCount()
{
	return _countof(ColorInfo_DEFAULT);
}
