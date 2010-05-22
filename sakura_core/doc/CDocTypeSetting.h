#pragma once


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          色設定                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 色設定
struct ColorInfoBase{
	bool		m_bDisp;			//!< 表示
	bool		m_bFatFont;			//!< 太字
	bool		m_bUnderLine;		//!< 下線
	COLORREF	m_colTEXT;			//!< 文字色
	COLORREF	m_colBACK;			//!< 背景色
};

//! 名前とインデックス付き色設定
struct NamedColorInfo : public ColorInfoBase{
	int			m_nColorIdx;		//!< インデックス
	TCHAR		m_szName[64];		//!< 名前
};


typedef NamedColorInfo ColorInfo;


//デフォルト色設定
void GetDefaultColorInfo(ColorInfo* pColorInfo, int nIndex);
int GetDefaultColorInfoCount();



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           辞書                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//@@@ 2006.04.10 fon ADD-start
const int DICT_ABOUT_LEN = 50; /*!< 辞書の説明の最大長 -1 */
struct KeyHelpInfo {
	bool		m_bUse;						//!< 辞書を 使用する/しない
	TCHAR		m_szAbout[DICT_ABOUT_LEN];	//!< 辞書の説明(辞書ファイルの1行目から生成)
	SFilePath	m_szPath;					//!< ファイルパス
};
//@@@ 2006.04.10 fon ADD-end

// とりあえずコメントアウト
// #include "../types/CType.h"
