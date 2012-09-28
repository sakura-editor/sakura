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
	Copyright (C) 2006, aroka, ryoji, Moca
	Copyright (C) 2007, ryoji, kobake, Moca, genta
	Copyright (C) 2008, nasukoji

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

#ifndef _countof
#define _countof(A) (sizeof(A)/sizeof(A[0]))
#endif

#if defined(__BORLANDC__)
#define __forceinline inline
#define _itoa itoa
#define _strlwr strlwr
#ifndef __max
#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef __min
#define __min(a,b)  (((a) < (b)) ? (a) : (b))
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
// 2006.04.10 ryoji コントロールプロセス初期化完了を示すイベントフラグ名を追加
#ifdef _DEBUG
	#ifndef	GSTR_SYNCOBJ_SAKURA
	#define	GSTR_SYNCOBJ_SAKURA
	#define	GSTR_MUTEX_SAKURA_OLD "MutexSakuraEditor_DEBUG"
	#define	GSTR_MUTEX_SAKURA_CP _T("MutexSakuraEditorCP_DEBUG")
	#define	GSTR_EVENT_SAKURA_CP_INITIALIZED _T("EventSakuraEditorCPInitialized_DEBUG")
	#define	GSTR_MUTEX_SAKURA_INIT _T("MutexSakuraEditorInit_DEBUG")
	#define	GSTR_MUTEX_SAKURA_EDITARR _T("MutexSakuraEditorEditArr_DEBUG")
	#endif
#else
	#ifndef	GSTR_SYNCOBJ_SAKURA
	#define	GSTR_SYNCOBJ_SAKURA
	#define	GSTR_MUTEX_SAKURA_OLD "MutexSakuraEditor"
	#define	GSTR_MUTEX_SAKURA_CP _T("MutexSakuraEditorCP")
	#define	GSTR_EVENT_SAKURA_CP_INITIALIZED _T("EventSakuraEditorCPInitialized")
	#define	GSTR_MUTEX_SAKURA_INIT _T("MutexSakuraEditorInit")
	#define	GSTR_MUTEX_SAKURA_EDITARR _T("MutexSakuraEditorEditArr")
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

//	Dec. 2, 2002 genta
//	固定ファイル名
#define FN_APP_ICON "my_appicon.ico"
#define FN_GREP_ICON "my_grepicon.ico"
#define FN_TOOL_BMP "my_icons.bmp"

//	標準アプリケーションアイコンリソース名
#ifdef _DEBUG
	#define ICON_DEFAULT_APP IDI_ICON_DEBUG
	#define ICON_DEFAULT_GREP IDI_ICON_GREP
#else
	#define ICON_DEFAULT_APP IDI_ICON_STD
	#define ICON_DEFAULT_GREP IDI_ICON_GREP
#endif


/* ウィンドウのID */
#define IDW_STATUSBAR 123


#define IDM_SELWINDOW		10000
#define IDM_SELMRU			11000
#define IDM_SELOPENFOLDER	12000


// 文字コードセット種別
//2007.08.14 kobake CODE_ERROR, CODE_DEFAULT 追加
SAKURA_CORE_API enum ECodeType {
	CODE_SJIS,						//!< SJIS				(MS-CP932(Windows-31J), シフトJIS(Shift_JIS))
	CODE_JIS,						//!< JIS				(MS-CP5022x(ISO-2022-JP-MS))
	CODE_EUC,						//!< EUC				(MS-CP51932, eucJP-ms(eucJP-open))
	CODE_UNICODE,					//!< Unicode			(UTF-16 LittleEndian(UCS-2))
	CODE_UTF8,						//!< UTF-8(UCS-2)
	CODE_UTF7,						//!< UTF-7(UCS-2)
	CODE_UNICODEBE,					//!< Unicode BigEndian	(UTF-16 BigEndian(UCS-2))
	CODE_CODEMAX,
	CODE_AUTODETECT	= 99,			//!< 文字コード自動判別
	CODE_ERROR      = -1,			//!< エラー
	CODE_NONE       = -1,			//!< 未検出
	CODE_DEFAULT    = CODE_SJIS,	//!< デフォルトの文字コード
	/*
		- MS-CP50220 
			Unicode から cp50220 への変換時に、
			JIS X 0201 片仮名は JIS X 0208 の片仮名に置換される
		- MS-CP50221
			Unicode から cp50221 への変換時に、
			JIS X 0201 片仮名は、G0 集合への指示のエスケープシーケンス ESC ( I を用いてエンコードされる
		- MS-CP50222
			Unicode から cp50222 への変換時に、
			JIS X 0201 片仮名は、SO/SI を用いてエンコードされる
		
		参考
		http://legacy-encoding.sourceforge.jp/wiki/
	*/
};

SAKURA_CORE_API extern const char* gm_pszCodeNameArr_1[];
SAKURA_CORE_API extern const char* gm_pszCodeNameArr_2[];
SAKURA_CORE_API extern const char* gm_pszCodeNameArr_3[];

/* コンボボックス用 自動判別を含む配列 */
SAKURA_CORE_API extern const int gm_nCodeComboValueArr[];
SAKURA_CORE_API extern const char* const gm_pszCodeComboNameArr[];
SAKURA_CORE_API extern const int gm_nCodeComboNameArrNum;

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
	COLORIDX_TEXT = 0,      // テキスト
	COLORIDX_RULER,         // ルーラー
	COLORIDX_CARET,         // キャレット    // 2006.12.07 ryoji
	COLORIDX_CARET_IME,     // IMEキャレット // 2006.12.07 ryoji
	COLORIDX_UNDERLINE,     // カーソル行アンダーライン
	COLORIDX_CURSORVLINE,   // カーソル位置縦線 // 2006.05.13 Moca
	COLORIDX_GYOU,          // 行番号
	COLORIDX_GYOU_MOD,      // 行番号(変更行)
	COLORIDX_TAB,           // TAB記号
	COLORIDX_SPACE,         // 半角空白 //2002.04.28 Add by KK 以降全て+1
	COLORIDX_ZENSPACE,      // 日本語空白
	COLORIDX_CTRLCODE,      // コントロールコード
	COLORIDX_EOL,           // 改行記号
	COLORIDX_WRAP,          // 折り返し記号
	COLORIDX_VERTLINE,      // 指定桁縦線    // 2005.11.08 Moca
	COLORIDX_EOF,           // EOF記号
	COLORIDX_DIGIT,         // 半角数値  //@@@ 2001.02.17 by MIK //色設定Ver.3からユーザファイルに対しては文字列で処理しているのでリナンバリングしてもよい. Mar. 7, 2001 JEPRO noted
	COLORIDX_SEARCH,        // 検索文字列
	COLORIDX_KEYWORD1,      // 強調キーワード1 // 2002/03/13 novice
	COLORIDX_KEYWORD2,      // 強調キーワード2 // 2002/03/13 novice  //MIK ADDED
	COLORIDX_KEYWORD3,      // 強調キーワード3 // 2005.01.13 MIK 3-10 added
	COLORIDX_KEYWORD4,      // 強調キーワード4
	COLORIDX_KEYWORD5,      // 強調キーワード5
	COLORIDX_KEYWORD6,      // 強調キーワード6
	COLORIDX_KEYWORD7,      // 強調キーワード7
	COLORIDX_KEYWORD8,      // 強調キーワード8
	COLORIDX_KEYWORD9,      // 強調キーワード9
	COLORIDX_KEYWORD10,     // 強調キーワード10
	COLORIDX_COMMENT,       // 行コメント                        //Dec. 4, 2000 shifted by MIK
	COLORIDX_SSTRING,       // シングルクォーテーション文字列    //Dec. 4, 2000 shifted by MIK
	COLORIDX_WSTRING,       // ダブルクォーテーション文字列      //Dec. 4, 2000 shifted by MIK
	COLORIDX_URL,           // URL                               //Dec. 4, 2000 shifted by MIK
	COLORIDX_REGEX1,        // 正規表現キーワード1  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX2,        // 正規表現キーワード2  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX3,        // 正規表現キーワード3  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX4,        // 正規表現キーワード4  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX5,        // 正規表現キーワード5  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX6,        // 正規表現キーワード6  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX7,        // 正規表現キーワード7  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX8,        // 正規表現キーワード8  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX9,        // 正規表現キーワード9  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX10,       // 正規表現キーワード10  //@@@ 2001.11.17 add MIK
	COLORIDX_DIFF_APPEND,   // DIFF追加  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_CHANGE,   // DIFF追加  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_DELETE,   // DIFF追加  //@@@ 2002.06.01 MIK
	COLORIDX_BRACKET_PAIR,  // 対括弧    // 02/09/18 ai Add
	COLORIDX_MARK,          // ブックマーク  // 02/10/16 ai Add

	//カラーの最後
	COLORIDX_LAST,

	//カラー表示制御用
	COLORIDX_BLOCK1,        // ブロックコメント1(文字色と背景色は行コメントと同じ)
	COLORIDX_BLOCK2,        // ブロックコメント2(文字色と背景色は行コメントと同じ)

	//1000- : カラー表示制御用(正規表現キーワード)
	COLORIDX_REGEX_FIRST = 1000,
	COLORIDX_REGEX_LAST		= COLORIDX_REGEX_FIRST + COLORIDX_LAST - 1,

	// -- -- 別名 -- -- //
	COLORIDX_DEFAULT		= COLORIDX_TEXT,
};
//	To Here Sept. 18, 2000



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

// Feb. 18, 2003 genta 最大値の定数化と値変更
const int LINESPACE_MAX = 128;
const int COLUMNSPACE_MAX = 64;


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

//	Aug. 14, 2005 genta 定数定義追加
// 2007.09.07 kobake 定数名変更: MAXLINESIZE→MAXLINEKETAS
// 2007.09.07 kobake 定数名変更: MINLINESIZE→MINLINEKETAS
const int MAXLINEKETAS	= 10240;	//!<	1行の桁数の最大値
const int MINLINEKETAS	= 10;		//!<	1行の桁数の最小値

const int LINEREADBUFSIZE	= 10240;	//!<	ファイルから1行分データを読み込むときのバッファサイズ

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

//2007.09.06 kobake 追加
//!検索方向
enum ESearchDirection{
	SEARCH_BACKWARD = 0, //前方検索 (前を検索)
	SEARCH_FORWARD  = 1, //後方検索 (次を検索) (普通)
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      １次元型の定義                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
// -- -- 通常のintで単位型を定義

//ロジック単位
typedef int CLogicInt;

//レイアウト単位
typedef int CLayoutInt;

namespace ApiWrap
{
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       よく使う用法                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//! SHIFTを押しているかどうか
	inline bool GetKeyState_Shift()
	{
		return (::GetKeyState(VK_SHIFT)&0x8000)!=0;
	}

	//! CTRLを押しているかどうか
	inline bool GetKeyState_Control()
	{
		return (::GetKeyState(VK_CONTROL)&0x8000)!=0;
	}

	//! ALTを押しているかどうか
	inline bool GetKeyState_Alt()
	{
		return (::GetKeyState(VK_MENU)&0x8000)!=0;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           定数                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//	Jun. 29, 2002 こおり
	//	Windows 95対策．Property SheetのサイズをWindows95が認識できる物に固定する．
	#if defined(_WIN64) || defined(_UNICODE)
		static const size_t sizeof_old_PROPSHEETHEADER = sizeof(PROPSHEETHEADER);
	#else
		static const size_t sizeof_old_PROPSHEETHEADER = 40;
	#endif

	//	Jan. 29, 2002 genta
	//	Win95/NTが納得するsizeof( MENUITEMINFO )
	//	これ以外の値を与えると古いOSでちゃんと動いてくれない．
	#if defined(_WIN64) || defined(_UNICODE)
		static const int SIZEOF_MENUITEMINFO = sizeof(MENUITEMINFO);
	#else
		static const int SIZEOF_MENUITEMINFO = 44;
	#endif
}
using namespace ApiWrap;




//	Sep. 22, 2003 MIK
//	古いSDK対策．新しいSDKでは不要
#ifndef _WIN64
#ifndef DWORD_PTR
#define DWORD_PTR DWORD
#endif
#ifndef ULONG_PTR
#define ULONG_PTR ULONG
#endif
#ifndef LONG_PTR
#define LONG_PTR LONG
#endif
#ifndef UINT_PTR
#define UINT_PTR UINT
#endif
#ifndef INT_PTR
#define INT_PTR INT
#endif
#ifndef SetWindowLongPtr
#define SetWindowLongPtr SetWindowLong
#endif
#ifndef GetWindowLongPtr
#define GetWindowLongPtr GetWindowLong
#endif
#ifndef DWLP_USER
#define DWLP_USER DWL_USER
#endif
#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC GWL_WNDPROC
#endif
#ifndef GWLP_USERDATA
#define GWLP_USERDATA GWL_USERDATA
#endif
#ifndef GWLP_HINSTANCE
#define GWLP_HINSTANCE GWL_HINSTANCE
#endif
#ifndef DWLP_MSGRESULT
#define DWLP_MSGRESULT DWL_MSGRESULT
#endif
#endif  //_WIN64

#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT 29
#endif
#ifndef COLOR_MENUBAR
#define COLOR_MENUBAR 30
#endif

///////////////////////////////////////////////////////////////////////
#endif /* _GLOBAL_H_ */


/*[EOF]*/
