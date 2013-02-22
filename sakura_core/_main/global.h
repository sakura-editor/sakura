/*!	@file
	@brief 共通定義

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, Stonee, genta, jepro, MIK
	Copyright (C) 2001, jepro, hor, MIK
	Copyright (C) 2002, MIK, genta, aroka, YAZAKI, Moca, KK, novice
	Copyright (C) 2003, MIK, genta, zenryaku, naoh
	Copyright (C) 2004, Kazika
	Copyright (C) 2005, MIK, Moca, genta
	Copyright (C) 2006, aroka, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//////////////////////////////////////////////////////////////
#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <tchar.h>

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
#define __forceinline inline
#define _itoa itoa
#define _stricmp stricmp
#define _memicmp memicmp
#endif





/* ウィンドウのID */
#define IDW_STATUSBAR 123


#define IDM_SELWINDOW		10000
#define IDM_SELMRU			11000
#define IDM_SELOPENFOLDER	12000

#include "charset/charset.h"

/* ダイアログ表示方法 */ // アウトラインウィンドウ用に作成 20060201 aroka
SAKURA_CORE_API enum enumShowDlg {
	SHOW_NORMAL			= 0,
	SHOW_RELOAD			= 1,
	SHOW_TOGGLE			= 2,
};


/* 選択領域描画用パラメータ */
SAKURA_CORE_API extern const COLORREF	SELECTEDAREA_RGB;
SAKURA_CORE_API extern const int		SELECTEDAREA_ROP2;





//@@@ From Here 2003.05.31 MIK
/*! タブウインドウ用メッセージサブコマンド */
SAKURA_CORE_API enum ETabWndNotifyType {
	TWNT_REFRESH	= 0,		//再表示
	TWNT_ADD		= 1,		//ウインドウ登録
	TWNT_DEL		= 2,		//ウインドウ削除
	TWNT_ORDER		= 3,		//ウインドウ順序変更
	TWNT_FILE		= 4,		//ファイル名変更
	TWNT_MODE_ENABLE= 5,		//タブモード有効化	//2004.07.14 Kazika 追加
	TWNT_MODE_DISABLE= 6,		//タブモード無効化	//2004.08.27 Kazika 追加
	TWNT_WNDPL_ADJUST= 7,		//ウィンドウ位置合わせ	// 2007.04.03 ryoji 追加
};

/*! バーの表示・非表示 */
SAKURA_CORE_API	enum EBarChangeNotifyType {
	MYBCN_TOOLBAR	= 0,		//ツールバー
	MYBCN_FUNCKEY	= 1,		//ファンクションキー
	MYBCN_TAB		= 2,		//タブ
	MYBCN_STATUSBAR	= 3,		//ステータスバー
};
//@@@ To Here 2003.05.31 MIK

//タブで使うカスタムメニューのインデックス	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_TABWND		24
//右クリックメニューで使うカスタムメニューのインデックス	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_RBUTTONUP	0


/*!< 色タイプ */
//@@@ From Here 2006.12.18 ryoji
#define COLOR_ATTRIB_FORCE_DISP		0x00000001
#define COLOR_ATTRIB_NO_TEXT		0x00000010
#define COLOR_ATTRIB_NO_BACK		0x00000020
#define COLOR_ATTRIB_NO_BOLD		0x00000100
#define COLOR_ATTRIB_NO_UNDERLINE	0x00000200
//#define COLOR_ATTRIB_NO_ITALIC		0x00000400	予約値
#define COLOR_ATTRIB_NO_EFFECTS		0x00000F00

struct SColorAttributeData{
	const TCHAR*			szName;
	unsigned int	fAttribute;
};
SAKURA_CORE_API extern const SColorAttributeData g_ColorAttributeArr[];

//@@@ To Here 2006.12.18 ryoji

/*!< 設定値の上限・下限 */
//	ルーラの高さ
const int IDC_SPIN_nRulerHeight_MIN = 2;
const int IDC_SPIN_nRulerHeight_MAX = 32;

/**	マウスクリックとキー定義の対応

	@date 2007.11.04 genta 新規作成．即値回避と範囲サイズ定義のため
*/
enum MOUSEFUNCTION_ASSIGN {
	MOUSEFUNCTION_DOUBLECLICK	= 0,	//!< ダブルクリック
	MOUSEFUNCTION_RIGHT			= 1,	//!< 右クリック
	MOUSEFUNCTION_CENTER		= 2,	//!< 中クリック
	MOUSEFUNCTION_LEFTSIDE		= 3,	//!< 左サイドクリック
	MOUSEFUNCTION_RIGHTSIDE		= 4,	//!< 右サイドクリック
	MOUSEFUNCTION_TRIPLECLICK	= 5,	//!< トリプルクリック
	MOUSEFUNCTION_QUADCLICK		= 6,	//!< クアドラプルクリック
	MOUSEFUNCTION_KEYBEGIN		= 7,	//!< マウスへの割り当て個数＝本当のキー割り当て先頭INDEX
};

// 2008.05.30 nasukoji	テキストの折り返し方法
enum WRAP_TEXT_WRAP_METHOD {
	WRAP_NO_TEXT_WRAP		= 0,		// 折り返さない（スクロールバーをテキスト幅に合わせる）
	WRAP_SETTING_WIDTH,					// 指定桁で折り返す
	WRAP_WINDOW_WIDTH,					// 右端で折り返す
};

// 2009.07.06 syat	文字カウント方法
enum ESelectCountMode {
	SELECT_COUNT_TOGGLE		= 0,	// 文字カウント方法をトグル
	SELECT_COUNT_BY_CHAR	= 1,	// 文字数でカウント
	SELECT_COUNT_BY_BYTE	= 2		// バイト数でカウント
};

//2007.09.06 kobake 追加
//!検索方向
enum ESearchDirection{
	SEARCH_BACKWARD = 0, //前方検索 (前を検索)
	SEARCH_FORWARD  = 1, //後方検索 (次を検索) (普通)
};

//2007.09.06 kobake 追加
struct SSearchOption{
//	ESearchDirection	eDirection;
//	bool	bPrevOrNext;	//!< false==前方検索 true==後方検索
	bool	bRegularExp;	//!< true==正規表現
	bool	bLoHiCase;		//!< true==英大文字小文字の区別
	bool	bWordOnly;		//!< true==単語のみ検索

	SSearchOption() : bRegularExp(false), bLoHiCase(false), bWordOnly(false) { }
	SSearchOption(
		bool _bRegularExp,
		bool _bLoHiCase,
		bool _bWordOnly
	)
	: bRegularExp(_bRegularExp)
	, bLoHiCase(_bLoHiCase)
	, bWordOnly(_bWordOnly)
	{
	}
	void Reset()
	{
		bRegularExp = false;
		bLoHiCase   = false;
		bWordOnly   = false;
	}

	//演算子
	bool operator == (const SSearchOption& rhs) const
	{
		//とりあえずmemcmpでいいや
		return memcmp(this,&rhs,sizeof(*this))==0;
	}
	bool operator != (const SSearchOption& rhs) const
	{
		return !operator==(rhs);
	}

};



//2007.10.02 kobake CEditWndのインスタンスへのポインタをここに保存しておく
class CEditWnd;
extern CEditWnd* g_pcEditWnd;


HWND G_GetProgressBar();
HINSTANCE G_AppInstance();


///////////////////////////////////////////////////////////////////////
#endif /* _GLOBAL_H_ */




