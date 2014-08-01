/*!	@file
	@brief プロセス間共有データへのアクセス

	@author Norio Nakatani
	@date 1998/05/26  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, asa-o, MIK, YAZAKI, hor
	Copyright (C) 2002, genta, aroka, Moca, MIK, YAZAKI, hor
	Copyright (C) 2003, Moca, aroka, MIK, genta
	Copyright (C) 2004, Moca, novice, genta
	Copyright (C) 2005, MIK, genta, ryoji, aroka, Moca
	Copyright (C) 2006, aroka, ryoji, D.S.Koba, fon, maru
	Copyright (C) 2007, ryoji, maru, genta, Moca, nasukoji, kobake
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, nasukoji, ryoji, genta, salarm
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CSHAREDATA_H_
#define _CSHAREDATA_H_

#include <windows.h>
#include <commctrl.h>
#include "CKeyBind.h"
#include "CKeyWordSetMgr.h"
#include "CPrint.h"
#include "CProfile.h"

//@@@ 2001.12.26 YAZAKI CMRU, CMRUFolder
#include "CMRUFile.h"
#include "CMRUFolder.h"

//20020129 aroka
#include "Funccode.h"
#include "CMemory.h"

#include "CMutex.h"	// 2007.07.07 genta

#include "CLineComment.h"	//@@@ 2002.09.22 YAZAKI
#include "CBlockComment.h"	//@@@ 2002.09.22 YAZAKI

#include "maxdata.h"	//@@@ 2002.09.22 YAZAKI

#include "CEol.h"

#include "EditInfo.h"

//! 編集ウィンドウノード
struct EditNode {
	int				m_nIndex;
	int				m_nGroup;					//!< グループID								//@@@ 2007.06.20 ryoji
	HWND			m_hWnd;
	int				m_nId;						//!< 無題Id
	char			m_szTabCaption[_MAX_PATH];	//!< タブウインドウ用：キャプション名		//@@@ 2003.05.31 MIK
	char			m_szFilePath[_MAX_PATH];	//!< タブウインドウ用：ファイル名			//@@@ 2006.01.28 ryoji
	bool			m_bIsGrep;					//!< Grepのウィンドウか						//@@@ 2006.01.28 ryoji
	UINT			m_showCmdRestore;			//!< 元のサイズに戻すときのサイズ種別		//@@@ 2007.06.20 ryoji
	BOOL			m_bClosing;					//!< 終了中か（「最後のファイルを閉じても(無題)を残す」用）	//@@@ 2007.06.20 ryoji
};

//! 拡張構造体
struct EditNodeEx{
	EditNode*	p;			//!< 編集ウィンドウ配列要素へのポインタ
	int			nGroupMru;	//!< グループ単位のMRU番号
};

//! フォント属性
struct SFontAttr{
	bool		m_bBoldFont;		//!< 太字
	bool		m_bUnderLine;		//!< 下線
};

//! 色属性
struct SColorAttr{
	COLORREF	m_cTEXT;			//!< 文字色
	COLORREF	m_cBACK;			//!< 背景色
};

//! 色設定
struct ColorInfo {
	int			m_nColorIdx;
	bool		m_bDisp;			//!< 色分け/表示 をする
	SFontAttr	m_sFontAttr;		//!< フォント属性
	SColorAttr	m_sColorAttr;		//!< 色属性
	char		m_szName[32];		//!< 名前
};

//! 色設定(保存用)
struct ColorInfoIni {
	const TCHAR*	m_pszName;			//!< 色名
	bool			m_bDisp;			//!< 色分け/表示 をする
	SFontAttr		m_sFontAttr;		//!< フォント属性
	SColorAttr		m_sColorAttr;		//!< 色属性
};

//@@@ 2001.11.17 add start MIK
struct RegexKeywordInfo {
	char	m_szKeyword[100];	//正規表現キーワード
	int	m_nColorIndex;		//色指定番号
};
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
const int DICT_ABOUT_LEN = 50; /*!< 辞書の説明の最大長 -1 */
struct KeyHelpInfo {
	int		m_nUse;						/*!< 辞書を 使用する/しない */
	char	m_szAbout[DICT_ABOUT_LEN];	/*!< 辞書の説明(辞書ファイルの1行目から生成) */
	char	m_szPath[_MAX_PATH];		/*!< ファイルパス */
};
//@@@ 2006.04.10 fon ADD-end

//! アウトライン解析の種類
enum EOutlineType {
	OUTLINE_C,
	OUTLINE_CPP,
	OUTLINE_PLSQL,
	OUTLINE_TEXT,
	OUTLINE_JAVA,
	OUTLINE_COBOL,
	OUTLINE_ASM,
	OUTLINE_PERL,		//	Sep. 8, 2000 genta
	OUTLINE_VB,			//	June 23, 2001 N.Nakatani
	OUTLINE_WZTXT,		// 2003.05.20 zenryaku 階層付テキストアウトライン解析
	OUTLINE_HTML,		// 2003.05.20 zenryaku HTMLアウトライン解析
	OUTLINE_TEX,		// 2003.07.20 naoh TeXアウトライン解析
	OUTLINE_FILE,		//	2002.04.01 YAZAKI ルールファイル用
	OUTLINE_PYTHON,		//	2007.02.08 genta Pythonアウトライン解析
	OUTLINE_ERLANG,		//	2009.08.11 genta Erlangアウトライン解析
	//	新しいアウトライン解析は必ずこの直前へ挿入
	OUTLINE_CODEMAX,
	OUTLINE_BOOKMARK,	//	2001.12.03 hor
	OUTLINE_DEFAULT =-1,//	2001.12.03 hor
	OUTLINE_UNKNOWN	= 99,
	OUTLINE_TREE = 100,		// 汎用ツリー 2010.03.28 syat
	OUTLINE_CLSTREE = 200,	// 汎用ツリー(クラス) 2010.03.28 syat
	OUTLINE_LIST = 300,		// 汎用リスト 2010.03.28 syat
};

//! スマートインデント種別
enum ESmartIndentType {
	SMARTINDENT_NONE,		//!< なし
	SMARTINDENT_CPP			//!< C/C++
};

//! エンコードオプション
struct SEncodingConfig{
	ECodeType			m_eDefaultCodetype;				//!< デフォルト文字コード
	EEolType			m_eDefaultEoltype;				//!< デフォルト改行コード	// 2011.01.24 ryoji
	bool				m_bDefaultBom;					//!< デフォルトBOM			// 2011.01.24 ryoji
};

//! タイプ別設定
struct STypeConfig {
	//2007.09.07 変数名変更: m_nMaxLineSize→m_nMaxLineKetas
	int					m_nIdx;
	char				m_szTypeName[64];				//!< タイプ属性：名称
	char				m_szTypeExts[MAX_TYPES_EXTS];	//!< タイプ属性：拡張子リスト
	int					m_nTextWrapMethod;				//!< テキストの折り返し方法		// 2008.05.30 nasukoji
	int					m_nMaxLineKetas;				//!< 折り返し文字数
	int					m_nColumnSpace;					//!< 文字と文字の隙間
	int					m_nLineSpace;					//!< 行間のすきま
	int					m_nTabSpace;					//!< TABの文字数
	BOOL				m_bTabArrow;					//!< タブ矢印表示		//@@@ 2003.03.26 MIK
	char				m_szTabViewString[8+1];			//!< TAB表示文字列	// 2003.1.26 aroka サイズ拡張	// 2009.02.11 ryoji サイズ戻し(17->8+1)
	int					m_bInsSpace;					//!< スペースの挿入	// 2001.12.03 hor
	// 2005.01.13 MIK 配列化
	int					m_nKeyWordSetIdx[MAX_KEYWORDSET_PER_TYPE];	//!< キーワードセット

	CLineComment		m_cLineComment;					//!< 行コメントデリミタ			//@@@ 2002.09.22 YAZAKI
	CBlockComment		m_cBlockComments[2];			//!< ブロックコメントデリミタ	//@@@ 2002.09.22 YAZAKI

	int					m_nStringType;					//!< 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""]['']
	char				m_szIndentChars[64];			//!< その他のインデント対象文字

	int					m_nColorInfoArrNum;				//!< 色設定配列の有効数
	ColorInfo			m_ColorInfoArr[64];				//!< 色設定配列

	bool				m_bLineNumIsCRLF;				//!< 行番号の表示 false=折り返し単位／true=改行単位
	int					m_nLineTermType;				//!< 行番号区切り  0=なし 1=縦線 2=任意
	char				m_cLineTermChar;				//!< 行番号区切り文字
	int					m_nVertLineIdx[MAX_VERTLINES];	//!< 指定桁縦線

	BOOL				m_bWordWrap;					//!< 英文ワードラップをする
	bool				m_bKinsokuHead;					//!< 行頭禁則をする		//@@@ 2002.04.08 MIK
	bool				m_bKinsokuTail;					//!< 行末禁則をする		//@@@ 2002.04.08 MIK
	bool				m_bKinsokuRet;					//!< 改行文字のぶら下げ	//@@@ 2002.04.13 MIK
	bool				m_bKinsokuKuto;					//!< 句読点のぶらさげ	//@@@ 2002.04.17 MIK
	char				m_szKinsokuHead[200];			//!< 行頭禁則文字	//@@@ 2002.04.08 MIK
	char				m_szKinsokuTail[200];			//!< 行頭禁則文字	//@@@ 2002.04.08 MIK
	char				m_szKinsokuKuto[200];			//!< 句読点ぶらさげ文字	// 2009.08.07 ryoji

	int					m_nCurrentPrintSetting;			//!< 現在選択している印刷設定

	int					m_nDefaultOutline;				//!< アウトライン解析方法
	char				m_szOutlineRuleFilename[_MAX_PATH];	//!< アウトライン解析ルールファイル

	int					m_nSmartIndent;					//!< スマートインデント種別
	int					m_nImeState;					//!< 初期IME状態	Nov. 20, 2000 genta

	//	2001/06/14 asa-o 補完のタイプ別設定
	char				m_szHokanFile[_MAX_PATH];		//!< 入力補完 単語ファイル
	//	2003.06.23 Moca ファイル内からの入力補完機能
	bool				m_bUseHokanByFile;				//!< 入力補完 開いているファイル内から候補を探す
	//	2001/06/19 asa-o
	bool				m_bHokanLoHiCase;				//!< 入力補完機能：英大文字小文字を同一視する

	char				m_szExtHelp[_MAX_PATH];			//!< 外部ヘルプ１
	char				m_szExtHtmlHelp[_MAX_PATH];		//!< 外部HTMLヘルプ
	bool				m_bHtmlHelpIsSingle;			//!< HtmlHelpビューアはひとつ

	SEncodingConfig		m_encoding;						//!< エンコードオプション


//@@@ 2001.11.17 add start MIK
	bool				m_bUseRegexKeyword;								//!< 正規表現キーワードを使うか
	int					m_nRegexKeyMagicNumber;							//!< 正規表現キーワード更新マジックナンバー
	RegexKeywordInfo	m_RegexKeywordArr[MAX_REGEX_KEYWORD];			//!< 正規表現キーワード
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	bool				m_bUseKeyWordHelp;				//!< キーワード辞書セレクト機能を使うか
	int					m_nKeyHelpNum;					//!< キーワード辞書の冊数
	KeyHelpInfo			m_KeyHelpArr[MAX_KEYHELP_FILE];	//!< キーワード辞書ファイル
	bool				m_bUseKeyHelpAllSearch;			//!< ヒットした次の辞書も検索(&A)
	bool				m_bUseKeyHelpKeyDisp;			//!< 1行目にキーワードも表示する(&W)
	bool				m_bUseKeyHelpPrefix;			//!< 選択範囲で前方一致検索(&P)
//@@@ 2006.04.10 fon ADD-end

	//	2002/04/30 YAZAKI Commonから移動。
	bool				m_bAutoIndent;					//!< オートインデント
	bool				m_bAutoIndent_ZENSPACE;			//!< 日本語空白もインデント
	bool				m_bRTrimPrevLine;				//!< 2005.10.11 ryoji 改行時に末尾の空白を削除
	int					m_nIndentLayout;				//!< 折り返しは2行目以降を字下げ表示

	//	Sep. 10, 2002 genta
	bool				m_bUseDocumentIcon;				//!< ファイルに関連づけられたアイコンを使う

	STypeConfig()
	: m_nMaxLineKetas(10) //	画面折り返し幅がTAB幅以下にならないことを初期値でも保証する	//	2004.04.03 Moca
	{
	}

}; /* STypeConfig */

// Apr. 05, 2003 genta WindowCaption用領域（変換前）の長さ
static const int MAX_CAPTION_CONF_LEN = 256;

static const int MAX_DATETIMEFOREMAT_LEN	= 100;
static const int MAX_CUSTOM_MENU			=  25;
static const int MAX_CUSTOM_MENU_NAME_LEN	=  32;
static const int MAX_CUSTOM_MENU_ITEMS		=  48;
static const int MAX_TOOLBAR_BUTTON_ITEMS	= 384;	//ツールバーに登録可能なボタン最大数
static const int MAX_TOOLBAR_ICON_X			=  32;	//アイコンBMPの桁数
static const int MAX_TOOLBAR_ICON_Y			=  13;	//アイコンBMPの段数
static const int MAX_TOOLBAR_ICON_COUNT		= MAX_TOOLBAR_ICON_X * MAX_TOOLBAR_ICON_Y; // =416
//Oct. 22, 2000 JEPRO アイコンの最大登録数を128個増やした(256→384)	
//2010/6/9 Uchi アイコンの最大登録数を32個増やした(384→416)

//! マクロ情報
struct MacroRec {
	char	m_szName[MACRONAME_MAX];	//!< 表示名
	char	m_szFile[_MAX_PATH+1];		//!< ファイル名(ディレクトリを含まない)
	BOOL	m_bReloadWhenExecute;		//!< 実行時に読み込みなおすか（デフォルトon）
	
	bool IsEnabled() const { return m_szFile[0] != '\0'; }
	const char* GetTitle() const { return m_szName[0] == '\0' ? m_szFile: m_szName; }	// 2007.11.02 ryoji 追加
};
//	To Here Sep. 14, 2001 genta

// 2004/06/21 novice タグジャンプ機能追加
//! タグジャンプ情報
struct TagJump {
	HWND	hwndReferer;				//!< 参照元ウィンドウ
	POINT	point;						//!< ライン, カラム
};

//	Aug. 15, 2000 genta
//	Backup Flags
const int BKUP_YEAR		= 32;
const int BKUP_MONTH	= 16;
const int BKUP_DAY		= 8;
const int BKUP_HOUR		= 4;
const int BKUP_MIN		= 2;
const int BKUP_SEC		= 1;

//	Aug. 21, 2000 genta
const int BKUP_AUTO		= 64;

//	2004.05.13 Moca
//! ウィンドウサイズ・位置の制御方法
enum EWinSizeMode{
	WINSIZEMODE_DEF = 0,	//!< 指定なし
	WINSIZEMODE_SAVE = 1,	//!< 継承(保存)
	WINSIZEMODE_SET = 2		//!< 直接指定(固定)
};

// 旧版と違い、bool型使えるようにしてあります by kobake

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           全般                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_General
{
	//	Jul. 3, 2000 genta
	//	アクセス関数(簡易)
	//	intをビット単位に分割して使う
	//	下4bitをCaretTypeに当てておく(将来の予約で多めに取っておく)
	int		GetCaretType(void) const { return m_nCaretType & 0xf; }
	void	SetCaretType(const int f){ m_nCaretType &= ~0xf; m_nCaretType |= f & 0xf; }

	//カーソル
	int					m_nCaretType;							//!< カーソルのタイプ 0=win 1=dos
	bool				m_bIsINSMode;							//!< 挿入／上書きモード
	bool				m_bIsFreeCursorMode;					//!< フリーカーソルモードか
	BOOL				m_bStopsBothEndsWhenSearchWord;			//!< 単語単位で移動するときに、単語の両端で止まるか
	BOOL				m_bStopsBothEndsWhenSearchParagraph;	//!< 段落単位で移動するときに、段落の両端で止まるか
	BOOL				m_bNoCaretMoveByActivation;				//!< マウスクリックにてアクティベートされた時はカーソル位置を移動しない 2007.10.02 nasukoji (add by genta)

	//スクロール
	int					m_nRepeatedScrollLineNum;		//!< キーリピート時のスクロール行数
	BOOL				m_nRepeatedScroll_Smooth;		//!< キーリピート時のスクロールを滑らかにするか
	int					m_nPageScrollByWheel;			//!< キー/マウスボタン + ホイールスクロールでページUP/DOWNする	// 2009.01.12 nasukoji
	int					m_nHorizontalScrollByWheel;		//!< キー/マウスボタン + ホイールスクロールで横スクロールする		// 2009.01.12 nasukoji

	//タスクトレイ
	BOOL				m_bUseTaskTray;					//!< タスクトレイのアイコンを使う
	BOOL				m_bStayTaskTray;				//!< タスクトレイのアイコンを常駐
	WORD				m_wTrayMenuHotKeyCode;			//!< タスクトレイ左クリックメニュー キー
	WORD				m_wTrayMenuHotKeyMods;			//!< タスクトレイ左クリックメニュー キー

	//履歴
	int					m_nMRUArrNum_MAX;				//!< ファイルの履歴MAX
	int					m_nOPENFOLDERArrNum_MAX;		//!< フォルダの履歴MAX

	//ノーカテゴリ
	BOOL				m_bCloseAllConfirm;				//!< [すべて閉じる]で他に編集用のウィンドウがあれば確認する	// 2006.12.25 ryoji
	BOOL				m_bExitConfirm;					//!< 終了時の確認をする

	//INI内設定のみ
	BOOL				m_bDispExitingDialog;			//!< 終了ダイアログを表示する
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ウィンドウ                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Window
{
	//基本設定
	BOOL				m_bDispTOOLBAR;					//!< 次回ウィンドウを開いたときツールバーを表示する
	BOOL				m_bDispSTATUSBAR;				//!< 次回ウィンドウを開いたときステータスバーを表示する
	BOOL				m_bDispFUNCKEYWND;				//!< 次回ウィンドウを開いたときファンクションキーを表示する
	BOOL				m_bMenuIcon;					//!< メニューにアイコンを表示する
	BOOL				m_bMenuWChar;					//!< メニューの字化け対策を行う(Win2K以降のみ)
	BOOL				m_bScrollBarHorz;				//!< 水平スクロールバーを使う
	BOOL				m_bUseCompatibleBMP;			//!< 再作画用互換ビットマップを使う 2007.09.09 Moca

	//位置と大きさの設定
	EWinSizeMode		m_eSaveWindowSize;				//!< ウィンドウサイズ継承・固定 EWinSizeModeに順ずる 2004.05.13 Moca
	int					m_nWinSizeType;					//!< 大きさの指定
	int					m_nWinSizeCX;					//!< 直接指定 幅
	int					m_nWinSizeCY;					//!< 直接指定 高さ
	EWinSizeMode		m_eSaveWindowPos;				//!< ウィンドウ位置継承・固定 EWinSizeModeに順ずる 2004.05.13 Moca
	int					m_nWinPosX;						//!< 直接指定 X座標
	int					m_nWinPosY;						//!< 直接指定 Y座標

	//ファンクションキー
	int					m_nFUNCKEYWND_Place;			//!< ファンクションキー表示位置／0:上 1:下
	int					m_nFUNCKEYWND_GroupNum;			//!< 2002/11/04 Moca ファンクションキーのグループボタン数

	//ルーラー・行番号
	int					m_nRulerHeight;					//!< ルーラー高さ
	int					m_nRulerBottomSpace;			//!< ルーラーとテキストの隙間
	int					m_nRulerType;					//!< ルーラーのタイプ
	int					m_nLineNumRightSpace;			//!< 行番号の右のスペース Sep. 18, 2002 genta

	//分割ウィンドウ
	BOOL				m_bSplitterWndHScroll;			//!< 分割ウィンドウの水平スクロールの同期をとる 2001/06/20 asa-o
	BOOL				m_bSplitterWndVScroll;			//!< 分割ウィンドウの垂直スクロールの同期をとる 2001/06/20 asa-o

	//タイトルバー
	char				m_szWindowCaptionActive[MAX_CAPTION_CONF_LEN];		//!< タイトルバー(アクティブ時)
	char				m_szWindowCaptionInactive[MAX_CAPTION_CONF_LEN];	//!< タイトルバー(非アクティブ時)

	//INI内設定のみ
	int					m_nVertLineOffset;				//!< 縦線の描画座標オフセット 2005.11.10 Moca
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         タブバー                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_TabBar
{
	BOOL				m_bDispTabWnd;					//!< タブウインドウ表示する	//@@@ 2003.05.31 MIK
	BOOL				m_bDispTabWndMultiWin;			//!< タブをまとめない	//@@@ 2003.05.31 MIK
	BOOL				m_bTab_RetainEmptyWin;			//!< 最後の文書が閉じられたとき(無題)を残す
	BOOL				m_bTab_CloseOneWin;				//!< タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる
	BOOL				m_bNewWindow;					//!< 外部から起動するときは新しいウインドウで開く

	char				m_szTabWndCaption[MAX_CAPTION_CONF_LEN];	//!< タブウインドウキャプション	//@@@ 2003.06.13 MIK
	BOOL				m_bSameTabWidth;				//!< タブを等幅にする			//@@@ 2006.01.28 ryoji
	BOOL				m_bDispTabIcon;					//!< タブにアイコンを表示する	//@@@ 2006.01.28 ryoji
	BOOL				m_bSortTabList;					//!< タブ一覧をソートする	//@@@ 2006.03.23 fon
	BOOL				m_bTab_ListFull;				//!< タブ一覧をフルパス表示する	//@@@ 2007.02.28 ryoji

	BOOL				m_bChgWndByWheel;				//!< マウスホイールでウィンドウ切り替え	//@@@ 2006.03.26 ryoji

	LOGFONT				m_lf;							//!< タブフォント // 2011.12.01 Moca
	INT					m_nPointSize;					//!< フォントサイズ（1/10ポイント単位）
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           編集                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//! ファイルダイアログの初期位置
enum EOpenDialogDir{
	OPENDIALOGDIR_CUR, //!< カレントフォルダ
	OPENDIALOGDIR_MRU, //!< 最近使ったフォルダ
	OPENDIALOGDIR_SEL, //!< 指定フォルダ
};

struct CommonSetting_Edit
{
	//コピー
	bool				m_bAddCRLFWhenCopy;			//!< 折り返し行に改行を付けてコピー
	BOOL				m_bEnableNoSelectCopy;		//!< 選択なしでコピーを可能にする	2007.11.18 ryoji
	BOOL				m_bCopyAndDisablSelection;	//!< コピーしたら選択解除
	bool				m_bEnableLineModePaste;		//!< ラインモード貼り付けを可能にする 2007.10.08 ryoji
	bool				m_bConvertEOLPaste;			//!< 改行コードを変換して貼り付ける  2009.2.28 salarm

	//ドラッグ＆ドロップ
	BOOL				m_bUseOLE_DragDrop;			//!< OLEによるドラッグ & ドロップを使う
	BOOL				m_bUseOLE_DropSource;		//!< OLEによるドラッグ元にするか

	//上書きモード
	BOOL				m_bNotOverWriteCRLF;		//!< 改行は上書きしない

	//クリッカブルURL
	BOOL				m_bJumpSingleClickURL;		//!< URLのシングルクリックでJump // 未使用
	BOOL				m_bSelectClickedURL;		//!< URLがクリックされたら選択するか

	EOpenDialogDir		m_eOpenDialogDir;			//!< ファイルダイアログの初期位置
	TCHAR				m_OpenDialogSelDir[_MAX_PATH];	//!< 指定フォルダ

	// (ダイアログ項目無し)
	BOOL				m_bAutoColumnPaste;			//!< 矩形コピーのテキストは常に矩形貼り付け
};

//! ファイルの排他制御モード  2007.10.11 kobake 作成
enum EShareMode{
	SHAREMODE_NOT_EXCLUSIVE,	//!< 排他制御しない
	SHAREMODE_DENY_WRITE,		//!< 他プロセスからの上書きを禁止
	SHAREMODE_DENY_READWRITE,	//!< 他プロセスからの読み書きを禁止
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ファイル                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_File
{
	// カーソル位置を復元するかどうか  Oct. 27, 2000 genta
	bool	GetRestoreCurPosition() const		{ return m_bRestoreCurPosition; }
	void	SetRestoreCurPosition(bool i)		{ m_bRestoreCurPosition = i; } // 未使用

	// ブックマークを復元するかどうか  2002.01.16 hor
	bool	GetRestoreBookmarks() const			{ return m_bRestoreBookmarks; }
	void	SetRestoreBookmarks(bool i)			{ m_bRestoreBookmarks = i; } // 未使用

	// ファイル読み込み時にMIMEのdecodeを行うか  Nov. 12, 2000 genta
	bool	GetAutoMIMEdecode() const			{ return m_bAutoMIMEdecode; }
	void	SetAutoMIMEdecode(bool i)			{ m_bAutoMIMEdecode = i; } // 未使用

	// 前回と文字コードが異なるときに問い合わせを行う  Oct. 03, 2004 genta
	bool	GetQueryIfCodeChange() const		{ return m_bQueryIfCodeChange; }
	void	SetQueryIfCodeChange(bool i)		{ m_bQueryIfCodeChange = i; } // 未使用
	
	// 開こうとしたファイルが存在しないとき警告する  Oct. 09, 2004 genta
	bool	GetAlertIfFileNotExist() const		{ return m_bAlertIfFileNotExist; }
	void	SetAlertIfFileNotExist(bool i)		{ m_bAlertIfFileNotExist = i; } // 未使用

	//ファイルの排他制御
	EShareMode			m_nFileShareMode;		//!< ファイルの排他制御モード
	bool				m_bCheckFileTimeStamp;	//!< 更新の監視

	//ファイルの保存
	bool				m_bEnableUnmodifiedOverwrite;	//!< 無変更でも上書きするか

	//「名前を付けて保存」でファイルの種類が[ユーザー指定]のときのファイル一覧表示
	//ファイル保存ダイアログのフィルタ設定	// 2006.11.16 ryoji
	bool				m_bNoFilterSaveNew;		//!< 新規から保存時は全ファイル表示
	bool				m_bNoFilterSaveFile;	//!< 新規以外から保存時は全ファイル表示

	//ファイルオープン
	bool				m_bDropFileAndClose;	//!< ファイルをドロップしたときは閉じて開く
	int					m_nDropFileNumMax;		//!< 一度にドロップ可能なファイル数
	bool				m_bRestoreCurPosition;	//!< ファイルを開いたときカーソル位置を復元するか
	bool				m_bRestoreBookmarks;	//!< ブックマークを復元するかどうか 2002.01.16 hor
	bool				m_bAutoMIMEdecode;		//!< ファイル読み込み時にMIMEのdecodeを行うか
	bool				m_bQueryIfCodeChange;	//!< 前回と文字コードが異なるときに問い合わせを行う Oct. 03, 2004 genta 
	bool				m_bAlertIfFileNotExist;	//!< 開こうとしたファイルが存在しないとき警告する Oct. 09, 2004 genta
	bool				m_bAlertIfLargeFile;    //!< 開こうとしたファイルサイズが大きい場合に警告する
	int 				m_nAlertFileSize;       //!< 警告を始めるファイルサイズ(MB)
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       バックアップ                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Backup
{
	//	Aug. 15, 2000 genta
	//	Backup設定のアクセス関数
	int		GetBackupType(void) const { return m_nBackUpType; }
	void	SetBackupType(int n){ m_nBackUpType = n; }

	bool	GetBackupOpt(int flag) const { return ( flag & m_nBackUpType_Opt1 ) == flag; }
	void	SetBackupOpt(int flag, bool value){ m_nBackUpType_Opt1 = value ? ( flag | m_nBackUpType_Opt1) :  ((~flag) & m_nBackUpType_Opt1 ); }

	//	バックアップ数
	int		GetBackupCount(void) const { return m_nBackUpType_Opt2 & 0xffff; }
	void	SetBackupCount(int value){ m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xffff0000) | ( value & 0xffff ); }

	//	バックアップの拡張子先頭文字(1文字)
	int		GetBackupExtChar(void) const { return ( m_nBackUpType_Opt2 >> 16 ) & 0xff ; }
	void	SetBackupExtChar(int value){ m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xff00ffff) | (( value & 0xff ) << 16 ); } // 未使用

	//	Aug. 21, 2000 genta
	//	自動Backup
	bool	IsAutoBackupEnabled(void) const { return GetBackupOpt( BKUP_AUTO ); }
	void	EnableAutoBackup(bool flag){ SetBackupOpt( BKUP_AUTO, flag ); }

	int		GetAutoBackupInterval(void) const { return m_nBackUpType_Opt3; }
	void	SetAutoBackupInterval(int i){ m_nBackUpType_Opt3 = i; }

	//	Backup詳細設定のアクセス関数
	int		GetBackupTypeAdv(void) const { return m_nBackUpType_Opt4; }
	void	SetBackupTypeAdv(int n){ m_nBackUpType_Opt4 = n; }

	bool				m_bBackUp;							//!< 保存時にバックアップを作成する
	bool				m_bBackUpDialog;					//!< バックアップの作成前に確認
	bool				m_bBackUpFolder;					//!< 指定フォルダにバックアップを作成する
	bool				m_bBackUpFolderRM;					//!< 指定フォルダにバックアップを作成する(リムーバブルメディアのみ)
	char				m_szBackUpFolder[_MAX_PATH];		//!< バックアップを作成するフォルダ
	int 				m_nBackUpType;						//!< バックアップファイル名のタイプ 1=(.bak) 2=*_日付.*
	int 				m_nBackUpType_Opt1;					//!< バックアップファイル名：オプション1
	int 				m_nBackUpType_Opt2;					//!< バックアップファイル名：オプション2
	int 				m_nBackUpType_Opt3;					//!< バックアップファイル名：オプション3
	int 				m_nBackUpType_Opt4;					//!< バックアップファイル名：オプション4
	int 				m_nBackUpType_Opt5;					//!< バックアップファイル名：オプション5
	int 				m_nBackUpType_Opt6;					//!< バックアップファイル名：オプション6
	bool				m_bBackUpDustBox;					//!< バックアップファイルをごみ箱に放り込む	//@@@ 2001.12.11 add MIK
	bool				m_bBackUpPathAdvanced;				//!< バックアップ先フォルダを詳細設定する 20051107 aroka 
	char				m_szBackUpPathAdvanced[_MAX_PATH];	//!< バックアップを作成するフォルダの詳細設定 20051107 aroka 
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           書式                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Format
{
	//日付書式
	int					m_nDateFormatType;							//!< 日付書式のタイプ
	TCHAR				m_szDateFormat[MAX_DATETIMEFOREMAT_LEN];	//!< 日付書式

	//時刻書式
	int					m_nTimeFormatType;							//!< 時刻書式のタイプ
	TCHAR				m_szTimeFormat[MAX_DATETIMEFOREMAT_LEN];	//!< 時刻書式

	//見出し記号
	char				m_szMidashiKigou[256];						//!< 見出し記号

	//引用符
	char				m_szInyouKigou[32];							//!< 引用符
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           検索                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Search
{
	SSearchOption		m_sSearchOption;				//!< 検索／置換  条件
	int					m_bConsecutiveAll;				//!< 「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
	int					m_bNOTIFYNOTFOUND;				//!< 検索／置換  見つからないときメッセージを表示
	int					m_bSelectedArea;				//!< 置換  選択範囲内置換

	int					m_bGrepSubFolder;				//!< Grep: サブフォルダも検索
	BOOL				m_bGrepOutputLine;				//!< Grep: 行を出力するか該当部分だけ出力するか
	int					m_nGrepOutputStyle;				//!< Grep: 出力形式
	int					m_bGrepDefaultFolder;			//!< Grep: フォルダの初期値をカレントフォルダにする
	ECodeType			m_nGrepCharSet;					//!< Grep: 文字コードセット // 2002/09/20 Moca Add
	BOOL				m_bCaretTextForSearch;			//!< カーソル位置の文字列をデフォルトの検索文字列にする 2006.08.23 ryoji
	TCHAR				m_szRegexpLib[_MAX_PATH];		//!< 使用する正規表現DLL 2007.08.22 genta

	//Grep
	BOOL				m_bGrepExitConfirm;				//!< Grepモードで保存確認するか
	BOOL				m_bGrepRealTimeView;			//!< Grep結果のリアルタイム表示 2003.06.16 Moca

	BOOL				m_bGTJW_RETURN;					//!< エンターキーでタグジャンプ
	BOOL				m_bGTJW_LDBLCLK;				//!< ダブルクリックでタグジャンプ

	//検索・置換ダイアログ
	BOOL				m_bAutoCloseDlgFind;			//!< 検索ダイアログを自動的に閉じる
	BOOL				m_bAutoCloseDlgReplace;			//!< 置換 ダイアログを自動的に閉じる
	BOOL				m_bSearchAll;					//!< 先頭（末尾）から再検索 2002.01.26 hor

	//INI内設定のみ
	BOOL				m_bUseCaretKeyWord;				//!< キャレット位置の単語を辞書検索		// 2006.03.24 fon
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       キー割り当て                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_KeyBind
{
	// キー割り当て
	int					m_nKeyNameArrNum;			//!< キー割り当て表の有効データ数
	KEYDATA				m_pKeyNameArr[100];			//!< キー割り当て表
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     カスタムメニュー                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_CustomMenu
{
	char				m_szCustMenuNameArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_NAME_LEN + 1];
	int					m_nCustMenuItemNumArr [MAX_CUSTOM_MENU];
	int					m_nCustMenuItemFuncArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	char				m_nCustMenuItemKeyArr [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ツールバー                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_ToolBar
{
	int					m_nToolBarButtonNum;								//!< ツールバーボタンの数
	int					m_nToolBarButtonIdxArr[MAX_TOOLBAR_BUTTON_ITEMS];	//!< ツールバーボタン構造体
	int					m_bToolBarIsFlat;									//!< フラットツールバーにする／しない
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      強調キーワード                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_SpecialKeyword
{
	// 強調キーワード設定
	CKeyWordSetMgr		m_CKeyWordSetMgr;					//!< 強調キーワード
	char				m_szKeyWordSetDir[MAX_PATH];		//!< 強調キーワードファイルのディレクトリ //未使用
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           支援                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Helper
{
	//入力補完機能
	BOOL				m_bHokanKey_RETURN;				//!< VK_RETURN	補完決定キーが有効/無効
	BOOL				m_bHokanKey_TAB;				//!< VK_TAB		補完決定キーが有効/無効
	BOOL				m_bHokanKey_RIGHT;				//!< VK_RIGHT	補完決定キーが有効/無効
	BOOL				m_bHokanKey_SPACE;				//!< VK_SPACE	補完決定キーが有効/無効

	//外部ヘルプの設定
	char				m_szExtHelp[_MAX_PATH];			//!< 外部ヘルプ１

	//外部HTMLヘルプの設定
	char				m_szExtHtmlHelp[_MAX_PATH];		//!< 外部HTMLヘルプ
	bool				m_bHtmlHelpIsSingle;			//!< HtmlHelpビューアはひとつ

	//migemo設定
	TCHAR				m_szMigemoDll[_MAX_PATH];		//!< migemo dll
	TCHAR				m_szMigemoDict[_MAX_PATH];		//!< migemo dict

	//キーワードヘルプ
	LOGFONT				m_lf;							//!< キーワードヘルプのフォント情報 		// ai 02/05/21 Add
	INT					m_nPointSize;					//!< キーワードヘルプのフォントサイズ（1/10ポイント単位）	// 2009.10.01 ryoji

	//INI内設定のみ
	int					m_bUseHokan;					//!< 入力補完機能を使用する
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          マクロ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Macro
{
	char				m_szKeyMacroFileName[MAX_PATH];	//!< キーボードマクロのファイル名
	MacroRec			m_MacroTable[MAX_CUSTMACRO];	//!< キー割り当て用マクロテーブル	Sep. 14, 2001 genta
	char				m_szMACROFOLDER[_MAX_PATH];		//!< マクロ用フォルダ
	int					m_nMacroOnOpened;				//!< オープン後自動実行マクロ番号	@@@ 2006.09.01 ryoji
	int					m_nMacroOnTypeChanged;			//!< タイプ変更後自動実行マクロ番号	@@@ 2006.09.01 ryoji
	int					m_nMacroOnSave;					//!< 保存前自動実行マクロ番号	@@@ 2006.09.01 ryoji
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ファイル名表示                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_FileName
{
	int					m_nTransformFileNameArrNum;										//!< ファイル名の簡易表示登録数
	TCHAR				m_szTransformFileNameFrom[MAX_TRANSFORM_FILENAME][_MAX_PATH];	//!< ファイル名の簡易表示変換前文字列
	TCHAR				m_szTransformFileNameTo[MAX_TRANSFORM_FILENAME][_MAX_PATH];		//!< ファイル名の簡易表示変換後文字列	//@@@ 2003.04.08 MIK
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       アウトライン                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_OutLine
{
	// 20060201 aroka アウトライン/トピックリスト の位置とサイズを記憶
	int					m_bRememberOutlineWindowPos;//!< アウトライン/トピックリスト の位置とサイズを記憶する
	int					m_widthOutlineWindow;		//!< アウトライン/トピックリスト のサイズ(幅)
	int					m_heightOutlineWindow;		//!< アウトライン/トピックリスト のサイズ(高さ)
	int					m_xOutlineWindowPos;		//!< アウトライン/トピックリスト の位置(X座標)
	int					m_yOutlineWindowPos;		//!< アウトライン/トピックリスト の位置(Y座標)

	//IDD_FUNCLIST (ツール - アウトライン解析)
	BOOL				m_bAutoCloseDlgFuncList;	//!< アウトラインダイアログを自動的に閉じる
	BOOL				m_bFunclistSetFocusOnJump;	//!< フォーカスを移す 2002.02.08 hor
	BOOL				m_bMarkUpBlankLineEnable;	//!< 空行を無視する 2002.02.08 aroka,hor
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ファイル内容比較                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Compare
{
	//ファイル内容比較ダイアログ
	BOOL				m_bCompareAndTileHorz;		//!< 文書比較後、左右に並べて表示
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ビュー                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_View
{
	//INI内設定のみ
	LOGFONT				m_lf;						//!< 現在のフォント情報
	BOOL				m_bFontIs_FIXED_PITCH;		//!< 現在のフォントは固定幅フォントである
	INT					m_nPointSize;				//!< フォントサイズ（1/10ポイント単位）	// 2009.10.01 ryoji
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Others
{
	//INI内設定のみ
	RECT				m_rcOpenDialog;				//!< 「開く」ダイアログのサイズと位置
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ステータスバー                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        プラグイン                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        メインメニュー                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 共通設定
struct CommonSetting
{
	CommonSetting_General			m_sGeneral;			//!< 全般
	CommonSetting_Window			m_sWindow;			//!< ウィンドウ
	CommonSetting_TabBar			m_sTabBar;			//!< タブバー
	CommonSetting_Edit				m_sEdit;			//!< 編集
	CommonSetting_File				m_sFile;			//!< ファイル
	CommonSetting_Backup			m_sBackup;			//!< バックアップ
	CommonSetting_Format			m_sFormat;			//!< 書式
	CommonSetting_Search			m_sSearch;			//!< 検索
	CommonSetting_KeyBind			m_sKeyBind;			//!< キー割り当て
	//
	CommonSetting_CustomMenu		m_sCustomMenu;		//!< カスタムメニュー
	CommonSetting_ToolBar			m_sToolBar;			//!< ツールバー
	CommonSetting_SpecialKeyword	m_sSpecialKeyword;	//!< 強調キーワード
	CommonSetting_Helper			m_sHelper;			//!< 支援
	CommonSetting_Macro				m_sMacro;			//!< マクロ
	CommonSetting_FileName			m_sFileName;		//!< ファイル名表示
	//
	CommonSetting_OutLine			m_sOutline;			//!< アウトライン
	CommonSetting_Compare			m_sCompare;			//!< ファイル内容比較
	CommonSetting_View				m_sView;			//!< ビュー
	CommonSetting_Others			m_sOthers;			//!< その他
};


//! iniフォルダ設定	// 2007.05.31 ryoji
struct IniFolder {
	bool m_bInit;							// 初期化済フラグ
	bool m_bReadPrivate;					// マルチユーザ用iniからの読み出しフラグ
	bool m_bWritePrivate;					// マルチユーザ用iniへの書き込みフラグ
	TCHAR m_szIniFile[_MAX_PATH];			// EXE基準のiniファイルパス
	TCHAR m_szPrivateIniFile[_MAX_PATH];	// マルチユーザ用のiniファイルパス
};	/* iniフォルダ設定 */

//! 共有フラグ
struct SShare_Flags{
	BOOL				m_bEditWndChanging;				// 編集ウィンドウ切替中	// 2007.04.03 ryoji
	/*	@@@ 2002.1.24 YAZAKI
		キーボードマクロは、記録終了した時点でファイル「m_szKeyMacroFileName」に書き出すことにする。
		m_bRecordingKeyMacroがTRUEのときは、キーボードマクロの記録中なので、m_szKeyMacroFileNameにアクセスしてはならない。
	*/
	BOOL				m_bRecordingKeyMacro;		/* キーボードマクロの記録中 */
	HWND				m_hwndRecordingKeyMacro;	/* キーボードマクロを記録中のウィンドウ */
};

//! 共有ワークバッファ
struct SShare_WorkBuffer{
	char				m_szWork[32000];
	EditInfo			m_EditInfo_MYWM_GETFILEINFO;	//MYWM_GETFILEINFOデータ受け渡し用	####美しくない
};

//! 共有ハンドル
struct SShare_Handles{
	HWND				m_hwndTray;
	HWND				m_hwndDebug;
	HACCEL				m_hAccel;
};

//! 共有メモリ内構造体
struct SShare_Nodes{
	int					m_nEditArrNum;	//short->intに修正	//@@@ 2003.05.31 MIK
	EditNode			m_pEditArr[MAX_EDITWINDOWS];	//最大値修正	@@@ 2003.05.31 MIK
	LONG				m_nSequences;	/* ウィンドウ連番 */
	LONG				m_nNonameSequences;	/* 無題連番 */
	LONG				m_nGroupSequences;	// タブグループ連番	// 2007.06.20 ryoji
};

//! EXE情報
struct SShare_Version{
	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
};

//! 共有メモリ内構造体
struct SShare_SearchKeywords{
	// -- -- 検索キー -- -- //
	int					m_nSEARCHKEYArrNum;
	char				m_szSEARCHKEYArr[MAX_SEARCHKEY][_MAX_PATH];
	int					m_nREPLACEKEYArrNum;
	char				m_szREPLACEKEYArr[MAX_REPLACEKEY][_MAX_PATH];
	int					m_nGREPFILEArrNum;
	char				m_szGREPFILEArr[MAX_GREPFILE][_MAX_PATH];
	int					m_nGREPFOLDERArrNum;
	char				m_szGREPFOLDERArr[MAX_GREPFOLDER][_MAX_PATH];
};

//! 共有メモリ内構造体
struct SShare_TagJump{
	//データ
	int					m_TagJumpNum;					//!< タグジャンプ情報の有効データ数
	int					m_TagJumpTop;					//!< スタックの一番上の位置
	TagJump				m_TagJump[MAX_TAGJUMPNUM];		//!< タグジャンプ情報
	int					m_nTagJumpKeywordArrNum;
	char				m_szTagJumpKeywordArr[MAX_TAGJUMP_KEYWORD][_MAX_PATH];
	BOOL				m_bTagJumpICase;	//!< 大文字小文字を同一視
	BOOL				m_bTagJumpAnyWhere;	//!< 文字列の途中にマッチ
};

//! 共有メモリ内構造体
struct SShare_FileNameManagement{
	IniFolder			m_IniFolder;	/**** iniフォルダ設定 ****/
};

//! 共有メモリ内構造体
struct SShare_History{
	//@@@ 2001.12.26 YAZAKI	以下の2つは、直接アクセスしないでください。CMRUを経由してください。
	int					m_nMRUArrNum;
	EditInfo			m_fiMRUArr[MAX_MRU];
	bool				m_bMRUArrFavorite[MAX_MRU];	//お気に入り	//@@@ 2003.04.08 MIK

	//@@@ 2001.12.26 YAZAKI	以下の2つは、直接アクセスしないでください。CMRUFolderを経由してください。
	int					m_nOPENFOLDERArrNum;
	char				m_szOPENFOLDERArr[MAX_OPENFOLDER][_MAX_PATH];
	bool				m_bOPENFOLDERArrFavorite[MAX_OPENFOLDER];	//お気に入り	//@@@ 2003.04.08 MIK

	//MRU以外の情報
	char				m_szIMPORTFOLDER[_MAX_PATH];	/* 設定インポート用フォルダ */
	int					m_nCmdArrNum;
	char				m_szCmdArr[MAX_CMDARR][MAX_CMDLEN];
};

//! 共有データ領域
struct DLLSHAREDATA {
	// -- -- バージョン -- -- //
	/*!
		データ構造 Version	//	Oct. 27, 2000 genta
		データ構造の異なるバージョンの同時起動を防ぐため
		必ず先頭になくてはならない．
	*/
	unsigned int				m_vStructureVersion;
	unsigned int				m_nSize;

	// -- -- 非保存対象 -- -- //
	SShare_Version				m_sVersion;	//※読込は行わないが、書込は行う
	SShare_WorkBuffer			m_sWorkBuffer;
	SShare_Flags				m_sFlags;
	SShare_Nodes				m_sNodes;
	SShare_Handles				m_sHandles;

	DWORD						m_dwCustColors[16];						//!< フォントDialogカスタムパレット

	// -- -- 保存対象 -- -- //
	//設定
	CommonSetting				m_Common;								// 共通設定
	STypeConfig					m_Types[MAX_TYPES];						// タイプ別設定
	PRINTSETTING				m_PrintSettingArr[MAX_PRINTSETTINGARR];	// 印刷ページ設定

	//その他
	SShare_SearchKeywords		m_sSearchKeywords;
	SShare_TagJump				m_sTagJump;
	SShare_FileNameManagement	m_sFileNameManagement;
	SShare_History				m_sHistory;

	//外部コマンド実行ダイアログのオプション
	int							m_nExecFlgOpt;		/* 外部コマンド実行オプション */	//	2006.12.03 maru オプションの拡張のため
	//DIFF差分表示ダイアログのオプション
	int							m_nDiffFlgOpt;		/* DIFF差分表示 */	//@@@ 2002.05.27 MIK
	//タグファイルの作成ダイアログのオプション
	TCHAR						m_szTagsCmdLine[_MAX_PATH];	/* TAGSコマンドラインオプション */	//@@@ 2003.05.12 MIK
	int							m_nTagsOpt;			/* TAGSオプション(チェック) */	//@@@ 2003.05.12 MIK


	// -- -- テンポラリ -- -- //
	//指定行へジャンプダイアログのオプション
	bool						m_bLineNumIsCRLF_ForJump;	/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */
};


/*!	@brief 共有データの管理

	CShareDataはCProcessのメンバであるため，両者の寿命は同一です．
	本来はCProcessオブジェクトを通じてアクセスするべきですが，
	CProcess内のデータ領域へのポインタをstatic変数に保存することで
	Singletonのようにどこからでもアクセスできる構造になっています．

	共有メモリへのポインタをm_pShareDataに保持します．このメンバは
	公開されていますが，CShareDataによってMap/Unmapされるために
	ChareDataの消滅によってポインタm_pShareDataも無効になることに
	注意してください．

	@date 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
*/
class CShareData
{
public:
	static CShareData* getInstance(){
		static CShareData instance;

		return &instance;
	}

	CShareData();
	CShareData(CShareData const&);
	void operator=(CShareData const&);

public:
	~CShareData();

	/*
	||  Attributes & Operations
	*/
	bool InitShareData();	/* CShareDataクラスの初期化処理 */
	DLLSHAREDATA* GetShareData(){ return m_pShareData; }		/* 共有データ構造体のアドレスを返す */
	int GetDocumentTypeOfPath( const char* pszFilePath );		/* ファイルパスを渡して、ドキュメントタイプ（数値）を取得する */
	int GetDocumentTypeOfExt( const char* pszExt );				/* 拡張子を渡して、ドキュメントタイプ（数値）を取得する */
	BOOL AddEditWndList( HWND, int nGroup = 0 );				/* 編集ウィンドウの登録 */	// 2007.06.26 ryoji nGroup引数追加
	void DeleteEditWndList( HWND );								/* 編集ウィンドウリストからの削除 */
	void ResetGroupId( void );									/* グループをIDリセットする */
	EditNode* GetEditNode( HWND hWnd );							/* 編集ウィンドウ情報を取得する */
	int GetNoNameNumber( HWND hWnd );
	int GetGroupId( HWND hWnd );								/* グループIDを取得する */
	bool IsSameGroup( HWND hWnd1, HWND hWnd2 );					/* 同一グループかどうかを調べる */
	int GetFreeGroupId( void );									/* 空いているグループ番号を取得する */
	bool ReorderTab( HWND hSrcTab, HWND hDstTab );				/* タブ移動に伴うウィンドウの並び替え 2007.07.07 genta */
	HWND SeparateGroup( HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[] );/* タブ分離に伴うウィンドウ処理 2007.07.07 genta */
	EditNode* GetEditNodeAt( int nGroup, int nIndex );			/* 指定位置の編集ウィンドウ情報を取得する */
	EditNode* GetTopEditNode( HWND hWnd );						/* 先頭の編集ウィンドウ情報を取得する */
	HWND GetTopEditWnd( HWND hWnd );							/* 先頭の編集ウィンドウを取得する */
	bool IsTopEditWnd( HWND hWnd ){ return (GetTopEditWnd( hWnd ) == hWnd); }	/* 先頭の編集ウィンドウかどうかを調べる */

	BOOL RequestCloseEditor( EditNode* pWndArr, int nArrCnt, BOOL bExit, int nGroup, BOOL bCheckConfirm, HWND hWndFrom );
																/* いくつかのウィンドウへ終了要求を出す */	// 2007.02.13 ryoji 「編集の全終了」を示す引数(bExit)を追加	// 2007.06.20 ryoji nGroup引数追加
	BOOL IsPathOpened( const TCHAR*, HWND* );					/* 指定ファイルが開かれているか調べる */
	BOOL ActiveAlreadyOpenedWindow( const TCHAR*, HWND*, int );	/* 指定ファイルが開かれているか調べつつ、多重オープン時の文字コード衝突も確認 */	// 2007.03.16
	int GetEditorWindowsNum( int nGroup, bool bExcludeClosing = true );				/* 現在の編集ウィンドウの数を調べる */	// 2007.06.20 ryoji nGroup引数追加	// 2008.04.19 ryoji bExcludeClosing引数追加
	BOOL PostMessageToAllEditors( UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast, int nGroup = 0 );	/* 全編集ウィンドウへメッセージをポストする */	// 2007.06.20 ryoji nGroup引数追加
	BOOL SendMessageToAllEditors( UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast, int nGroup = 0 );	/* 全編集ウィンドウへメッセージを送るする */	// 2007.06.20 ryoji nGroup引数追加
	int GetOpenedWindowArr( EditNode** , BOOL, BOOL bGSort = FALSE );				/* 現在開いている編集ウィンドウの配列を返す */
	static void SetKeyNameArrVal(
		DLLSHAREDATA*, int, short, const char*,
		short, short, short, short,
		short, short, short, short );									/* KEYDATA配列にデータをセット */
	static LONG MY_RegSetVal(
		HKEY hKey,				// handle of key to set value for
		LPCTSTR lpValueName,	// address of value to set
		CONST BYTE *lpData,		// address of value data
		DWORD cbData 			// size of value data
	);
	static LONG MY_RegQuerVal(
		HKEY hKey,				// handle of key to set value for
		LPCTSTR lpValueName,	// address of value to set
		BYTE *lpData,			// address of value data
		DWORD cbData 			// size of value data
	);
	void TraceOut( LPCTSTR lpFmt, ...);	/* デバッグモニタに出力 */
	void SetTraceOutSource( HWND hwnd ){ m_hwndTraceOutSource = hwnd; }	/* TraceOut起動元ウィンドウの設定 */
	bool LoadShareData( void );	/* 共有データのロード */
	void SaveShareData( void );	/* 共有データの保存 */

	//メニュー類のファイル名作成
	bool GetMenuFullLabel_WinList(TCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, int index){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, false, index, false);
	}
	bool GetMenuFullLabel_MRU(TCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, bool bFavorite, int index){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, bFavorite, index, true);
	}
	bool GetMenuFullLabel_WinListNoEscape(TCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, int index){
		return GetMenuFullLabel(pszOutput, nBuffSize, false, editInfo, id, false, index, false);
	}
	bool GetMenuFullLabel_File(TCHAR* pszOutput, int nBuffSize, const TCHAR* pszFile, int id, bool bModified = false, ECodeType nCharCode = CODE_NONE){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, pszFile, id, false, nCharCode, false, -1, false);
	}
	bool GetMenuFullLabel_FileNoEscape(TCHAR* pszOutput, int nBuffSize, const TCHAR* pszFile, int id, bool bModified = false, ECodeType nCharCode = CODE_NONE){
		return GetMenuFullLabel(pszOutput, nBuffSize, false, pszFile, id, false, nCharCode, false, -1, false);
	}
	bool GetMenuFullLabel(TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp, const EditInfo* editInfo, int id, bool bFavorite, int index, bool bAccKeyZeroOrigin);
	bool GetMenuFullLabel(TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp, const TCHAR* pszFile, int id, bool bModified, ECodeType nCharCode, bool bFavorite, int index, bool bAccKeyZeroOrigin);
	static TCHAR GetAccessKeyByIndex(int index, bool bZeroOrigin);
	static void GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile );	/* 構成設定ファイルからiniファイル名を取得する */	// 2007.09.04 ryoji
	void GetIniFileName( LPTSTR pszIniFileName, BOOL bRead = FALSE );	/* iniファイル名の取得 */	// 2007.05.19 ryoji
	BOOL IsPrivateSettings( void ){ return m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate; }			/* iniファイルの保存先がユーザ別設定フォルダかどうか */	// 2007.05.25 ryoji
	bool ShareData_IO_2( bool );	/* 共有データの保存 */
	static void IO_ColorSet( CProfile* , const char* , ColorInfo* );	/* 色設定 I/O */ // Feb. 12, 2006 D.S.Koba

	//@@@ 2002.2.2 YAZAKI
	//	Jun. 14, 2003 genta 引数追加．書式変更
	int			GetMacroFilename( int idx, TCHAR* pszPath, int nBufLen ); // idxで指定したマクロファイル名（フルパス）を取得する
	bool		BeReloadWhenExecuteMacro( int idx );	//	idxで指定したマクロは、実行するたびにファイルを読み込む設定か？
	void		AddToSearchKeyArr( const char* pszSearchKey );	//	m_szSEARCHKEYArrにpszSearchKeyを追加する
	void		AddToReplaceKeyArr( const char* pszReplaceKey );	//	m_szREPLACEKEYArrにpszReplaceKeyを追加する
	void		AddToGrepFileArr( const char* pszGrepFile );		//	m_szGREPFILEArrにpszGrepFileを追加する
	void		AddToGrepFolderArr( const char* pszGrepFolder );	//	m_nGREPFOLDERArrNumにpszGrepFolderを追加する

	//@@@ 2002.2.3 YAZAKI
	bool		ExtWinHelpIsSet( int nType = -1 );	//	タイプがnTypeのときに、外部ヘルプが設定されているか。
	char*		GetExtWinHelp( int nType = -1 );	//	タイプがnTypeのときの、外部ヘルプファイル名を取得。
	bool		ExtHTMLHelpIsSet( int nType = -1 );	//	タイプがnTypeのときに、外部HTMLヘルプが設定されているか。
	char*		GetExtHTMLHelp( int nType = -1 );	//	タイプがnTypeのときの、外部HTMLヘルプファイル名を取得。
	bool		HTMLHelpIsSingle( int nType = -1 );	//	タイプがnTypeのときの、外部HTMLヘルプ「ビューアを複数起動しない」がONかを取得。

	//@@@ 2002.2.9 YAZAKI
	const TCHAR* MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen );
	const TCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen );
	const TCHAR* MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nDateFormatType, TCHAR* szDateFormat );
	const TCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nTimeFormatType, TCHAR* szTimeFormat );

	LPTSTR GetTransformFileNameFast( LPCTSTR, LPTSTR, int );
	int TransformFileName_MakeCache( void );
	static LPCTSTR GetFilePathFormat( LPCTSTR, LPTSTR, int, LPCTSTR, LPCTSTR );
	static bool ExpandMetaToFolder( LPCTSTR, LPTSTR, int );

	// 2004/06/21 novice タグジャンプ機能追加
	void PushTagJump(const TagJump *);		//!< タグジャンプ情報の保存
	bool PopTagJump(TagJump *);				//!< タグジャンプ情報の参照

protected:
	/*
	||  実装ヘルパ関数
	*/
	HANDLE			m_hFileMap;
	DLLSHAREDATA*	m_pShareData;
	HWND			m_hwndTraceOutSource;	// TraceOut()起動元ウィンドウ（いちいち起動元を指定しなくてすむように）

	// ファイル名簡易表示用キャッシュ
	int m_nTransformFileNameCount; // 有効数
	TCHAR m_szTransformFileNameFromExp[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	int m_nTransformFileNameOrgId[MAX_TRANSFORM_FILENAME];

	//	Jan. 30, 2005 genta 初期化関数の分割
	void InitKeyword(DLLSHAREDATA*);
	bool InitKeyAssign(DLLSHAREDATA*); // 2007.11.04 genta 起動中止のため値を返す
	void InitToolButtons(DLLSHAREDATA*);
	void InitTypeConfigs(DLLSHAREDATA*);
	void InitPopupMenu(DLLSHAREDATA*);

	// Feb. 12, 2006 D.S.Koba
	void ShareData_IO_Mru( CProfile& );
	void ShareData_IO_Keys( CProfile& );
	void ShareData_IO_Grep( CProfile& );
	void ShareData_IO_Folders( CProfile& );
	void ShareData_IO_Cmd( CProfile& );
	void ShareData_IO_Nickname( CProfile& );
	void ShareData_IO_Common( CProfile& );
	void ShareData_IO_Toolbar( CProfile& );
	void ShareData_IO_CustMenu( CProfile& );
	void ShareData_IO_Font( CProfile& );
	void ShareData_IO_KeyBind( CProfile& );
	void ShareData_IO_Print( CProfile& );
	void ShareData_IO_Types( CProfile& );
	void ShareData_IO_Type_One( CProfile& cProfile, int nType, const char* pszSecName);
	void ShareData_IO_KeyWords( CProfile& );
	void ShareData_IO_Macro( CProfile& );
	void ShareData_IO_Other( CProfile& );

	int _GetOpenedWindowArrCore( EditNode** , BOOL, BOOL bGSort = FALSE );			/* 現在開いている編集ウィンドウの配列を返す（コア処理部） */
};

/* 指定ウィンドウが、編集ウィンドウのフレームウィンドウかどうか調べる */
BOOL IsSakuraMainWindow( HWND hWnd );


///////////////////////////////////////////////////////////////////////
#endif /* _CSHAREDATA_H_ */


/*[EOF]*/
