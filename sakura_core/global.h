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
#define __forceinline
#define _itoa itoa
#define _stricmp stricmp
#define _memicmp memicmp
#ifndef __max
#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef __min
#define __min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#define __access access
#ifndef STRICT
#define STRICT
#endif
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



// Stonee 注： 2000/01/12
// ここを変更したときは、global.cpp のg_ColorAttributeArrの定義も変更して下さい。
//	From Here Sept. 18, 2000 JEPRO 順番を大幅に入れ替えた
//	2007.09.09 Moca  中間の定義はお任せに変更
SAKURA_CORE_API enum EColorIndexType {
	COLORIDX_TEXT = 0,      /* テキスト */
	COLORIDX_RULER,         /* ルーラー */
	COLORIDX_CARET,         /* キャレット */    // 2006.12.07 ryoji
	COLORIDX_CARET_IME,     /* IMEキャレット */ // 2006.12.07 ryoji
	COLORIDX_UNDERLINE,     /* カーソル行アンダーライン */
	COLORIDX_CURSORVLINE,   /* カーソル位置縦線 */ // 2006.05.13 Moca
	COLORIDX_GYOU,          /* 行番号 */
	COLORIDX_GYOU_MOD,      /* 行番号(変更行) */
	COLORIDX_TAB,           /* TAB記号 */
	COLORIDX_SPACE,         /* 半角空白 */ //2002.04.28 Add by KK 以降全て+1
	COLORIDX_ZENSPACE,      /* 日本語空白 */
	COLORIDX_CTRLCODE,      /* コントロールコード */
	COLORIDX_CRLF,          /* 改行記号 */
	COLORIDX_WRAP,          /* 折り返し記号 */
	COLORIDX_VERTLINE,      /* 指定桁縦線 */    // 2005.11.08 Moca
	COLORIDX_EOF,           /* EOF記号 */
	COLORIDX_DIGIT,         /* 半角数値 */  //@@@ 2001.02.17 by MIK //色設定Ver.3からユーザファイルに対しては文字列で処理しているのでリナンバリングしてもよい. Mar. 7, 2001 JEPRO noted
	COLORIDX_SEARCH,        /* 検索文字列 */
	COLORIDX_KEYWORD1,      /* 強調キーワード1 */ // 2002/03/13 novice
	COLORIDX_KEYWORD2,      /* 強調キーワード2 */ // 2002/03/13 novice  //MIK ADDED
	COLORIDX_KEYWORD3,      /* 強調キーワード3 */ // 2005.01.13 MIK 3-10 added
	COLORIDX_KEYWORD4,      /* 強調キーワード4 */
	COLORIDX_KEYWORD5,      /* 強調キーワード5 */
	COLORIDX_KEYWORD6,      /* 強調キーワード6 */
	COLORIDX_KEYWORD7,      /* 強調キーワード7 */
	COLORIDX_KEYWORD8,      /* 強調キーワード8 */
	COLORIDX_KEYWORD9,      /* 強調キーワード9 */
	COLORIDX_KEYWORD10,     /* 強調キーワード10 */
	COLORIDX_COMMENT,       /* 行コメント */                        //Dec. 4, 2000 shifted by MIK
	COLORIDX_SSTRING,       /* シングルクォーテーション文字列 */    //Dec. 4, 2000 shifted by MIK
	COLORIDX_WSTRING,       /* ダブルクォーテーション文字列 */      //Dec. 4, 2000 shifted by MIK
	COLORIDX_URL,           /* URL */                               //Dec. 4, 2000 shifted by MIK
	COLORIDX_REGEX1,        /* 正規表現キーワード1 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX2,        /* 正規表現キーワード2 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX3,        /* 正規表現キーワード3 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX4,        /* 正規表現キーワード4 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX5,        /* 正規表現キーワード5 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX6,        /* 正規表現キーワード6 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX7,        /* 正規表現キーワード7 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX8,        /* 正規表現キーワード8 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX9,        /* 正規表現キーワード9 */  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX10,       /* 正規表現キーワード10 */  //@@@ 2001.11.17 add MIK
	COLORIDX_DIFF_APPEND,   /* DIFF追加 */  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_CHANGE,   /* DIFF追加 */  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_DELETE,   /* DIFF追加 */  //@@@ 2002.06.01 MIK
	COLORIDX_BRACKET_PAIR,  /* 対括弧 */    // 02/09/18 ai Add
	COLORIDX_MARK,          /* ブックマーク */  // 02/10/16 ai Add

	//カラーの最後
	COLORIDX_LAST,          //Dec. 4, 2000 @@@2001.02.17 renumber by MIK				/* 2002/10/16 ai Mod 34→35 */

	//カラー表示制御用
	COLORIDX_BLOCK1,        /* ブロックコメント1(文字色と背景色は行コメントと同じ) */	/* 02/10/16 ai Mod 35→36 */
	COLORIDX_BLOCK2,        /* ブロックコメント2(文字色と背景色は行コメントと同じ) */	/* 02/10/16 ai Mod 36→37 */

	// -- -- 別名 -- -- //
	COLORIDX_DEFAULT	= COLORIDX_TEXT,

	//1000-1099 : カラー表示制御用(正規表現キーワード)
	COLORIDX_REGEX_FIRST = 1000,
	COLORIDX_REGEX_LAST = 1099,
};
//	To Here Sept. 18, 2000

//正規表現キーワードのEColorIndexType値を作る関数
inline EColorIndexType MakeColorIndexType_RegularExpression(int nRegExpIndex)
{
	return (EColorIndexType)(1000 + nRegExpIndex);
}


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
//#define COLOR_ATTRIB_NO_TEXT		0x00000010	予約値
#define COLOR_ATTRIB_NO_BACK		0x00000020
#define COLOR_ATTRIB_NO_BOLD		0x00000100
#define COLOR_ATTRIB_NO_UNDERLINE	0x00000200
//#define COLOR_ATTRIB_NO_ITALIC		0x00000400	予約値
#define COLOR_ATTRIB_NO_EFFECTS		0x00000F00

struct SColorAttributeData{
	TCHAR*			szName;
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


/* カラー名＜＞インデックス番号の変換 */	//@@@ 2002.04.30
SAKURA_CORE_API int GetColorIndexByName( const TCHAR *name );
SAKURA_CORE_API const TCHAR* GetColorNameByIndex( int index );



///////////////////////////////////////////////////////////////////////
#endif /* _GLOBAL_H_ */




