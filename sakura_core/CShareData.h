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
	Copyright (C) 2006, aroka, ryoji, D.S.Koba, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CShareData;

#ifndef _CSHAREDATA_H_
#define _CSHAREDATA_H_

#include <windows.h>
#include <commctrl.h>
#include "CKeyBind.h"
#include "CKeyWordSetMgr.h"
#include "CPrint.h"
#include "CProfile.h"

//@@@ 2001.12.26 YAZAKI CMRU, CMRUFolder
#include "CMRU.h"
#include "CMRUFolder.h"

//20020129 aroka
#include "funccode.h"
#include "CMemory.h"

#include "CLineComment.h"	//@@@ 2002.09.22 YAZAKI
#include "CBlockComment.h"	//@@@ 2002.09.22 YAZAKI

enum maxdata{
	MAX_EDITWINDOWS				= 256,
	MAX_SEARCHKEY				=  30,
	MAX_REPLACEKEY				=  30,
	MAX_GREPFILE				=  30,
	MAX_GREPFOLDER				=  30,
	MAX_TYPES					=  20,	//Jul. 12, 2001 JEPRO タイプ別設定の最大設定数を16から増やした
	MAX_TOOLBARBUTTONS			= 384,	//Oct. 22, 2000 JEPRO アイコンの最大登録数を128個増やした(256→384)
	MAX_CUSTOM_MENU				=  25,
	MAX_CUSTOM_MENU_NAME_LEN	=  32,
	MAX_CUSTOM_MENU_ITEMS		=  48,
	MAX_PRINTSETTINGARR			=   8,

	//	From Here Sep. 14, 2001 genta
	MACRONAME_MAX				= 64,
	MAX_EXTCMDLEN				= 1024,
	MAX_EXTCMDMRUNUM			= 32,

	MAX_DATETIMEFOREMAT_LEN		= 100,

	MAX_CMDLEN					= 1024,
	MAX_CMDARR					= 32,
	MAX_REGEX_KEYWORD			= 100,	//@@@ 2001.11.17 add MIK

	MAX_KEYHELP_FILE			= 20,	//@@@ 2006.04.10 fon

	MAX_MARKLINES_LEN			= 1023,	// 2002.01.18 hor
	MAX_DOCTYPE_LEN				= 7,
	MAX_TRANSFORM_FILENAME		= 16,	/// 2002.11.24 Moca

	/*! 登録できるマクロの数
		@date 2005.01.30 genta 50に増やした
	*/
	MAX_CUSTMACRO				= 50,

	// 2004/06/21 novice タグジャンプ機能追加
	MAX_TAGJUMPNUM				= 100,	// タブジャンプ情報最大値
	MAX_TAGJUMP_KEYWORD			= 30,	//タグジャンプ用キーワード最大登録数2005.04.04 MIK
	MAX_KEYWORDSET_PER_TYPE		= 10,	// 2004.01.23 genta (for MIK) タイプ別設定毎のキーワードセット数
	MAX_VERTLINES = 10,	// 2005.11.08 Moca 指定桁縦線
};



/*! ファイル情報

	@date 2002.03.07 genta m_szDocType追加
	@date 2003.01.26 aroka m_nWindowSizeX/Y m_nWindowOriginX/Y追加
*/
struct FileInfo {
	int		m_nViewTopLine;			/*!< 表示域の一番上の行(0開始) */
	int		m_nViewLeftCol;			/*!< 表示域の一番左の桁(0開始) */
	int		m_nX;					/*!< カーソル  物理位置(行頭からのバイト数) */
	int		m_nY;					/*!< カーソル  物理位置(折り返し無し行位置) */
	int		m_bIsModified;			/*!< 変更フラグ */
	int		m_nCharCode;			/*!< 文字コード種別 */
	char	m_szPath[_MAX_PATH];	/*!< ファイル名 */
	BOOL	m_bIsGrep;				/*!< Grepのウィンドウか */
	BOOL	m_bIsDebug;				/*!< デバッグモードか(アウトプットウインドウ) */
	char	m_szGrepKey[1024];
	char	m_szMarkLines[MAX_MARKLINES_LEN + 1];	/*!< ブックマークの物理行リスト */
	char	m_szDocType[MAX_DOCTYPE_LEN + 1];	/*!< 文書タイプ */
	int		m_nWindowSizeX;			/*!< ウィンドウ  幅(ピクセル数) */
	int		m_nWindowSizeY;			/*!< ウィンドウ  高さ(ピクセル数) */
	int		m_nWindowOriginX;		/*!< ウィンドウ  物理位置(ピクセル数・マイナス値も有効) */
	int		m_nWindowOriginY;		/*!< ウィンドウ  物理位置(ピクセル数・マイナス値も有効) */
	
	// Mar. 7, 2002 genta
	// Constructor 確実に初期化するため
	FileInfo() : m_nViewTopLine( -1 ), m_nViewLeftCol( -1 ),
		m_nX( -1 ), m_nY( -1 ), m_bIsModified( 0 ),
		m_nCharCode( CODE_AUTODETECT ), m_bIsGrep( FALSE ), m_bIsDebug( FALSE ),
		m_nWindowSizeX( -1 ), m_nWindowSizeY( -1 ),
		//	2004.05.13 Moca “指定無し”を-1からCW_USEDEFAULTに変更
		m_nWindowOriginX( CW_USEDEFAULT ), m_nWindowOriginY( CW_USEDEFAULT )
	{
		m_szPath[0] = '\0';
		m_szMarkLines[0] = '\0';
		m_szDocType[0] = '\0';
	}
};

/*!	検索オプション
	20020118 aroka
*/
struct GrepInfo {
	CMemory		cmGrepKey;			/*!< 検索キー */
	CMemory		cmGrepFile;			/*!< 検索対象ファイル */
	CMemory		cmGrepFolder;		/*!< 検索対象フォルダ */
	bool		bGrepWordOnly;		/*!< 単語単位で探す */
	bool		bGrepSubFolder;		/*!< サブフォルダを検索する */
	bool		bGrepNoIgnoreCase;	/*!< 大文字と小文字を区別する */
	bool		bGrepRegularExp;	/*!< 正規表現を使用する */
	bool		bGrepOutputLine;	/*!< 結果出力で該当行を出力する */
	int			nGrepOutputStyle;	/*!< 結果出力形式 */
	int			nGrepCharSet;		/*!< 文字コードセット */
};


struct EditNode {
	int				m_nIndex;
	HWND			m_hWnd;
	char			m_szTabCaption[_MAX_PATH];	/*!< タブウインドウ用：キャプション名 */	//@@@ 2003.05.31 MIK
	char			m_szFilePath[_MAX_PATH];	/*!< タブウインドウ用：ファイル名 */		//@@@ 2006.01.28 ryoji
	BOOL			m_bIsGrep;					/*!< Grepのウィンドウか */					//@@@ 2006.01.28 ryoji
};

//! 印刷設定
#define POS_LEFT	0
#define POS_CENTER	1
#define POS_RIGHT	2
#define HEADER_MAX	100
#define FOOTER_MAX	HEADER_MAX
struct PRINTSETTING {
	char			m_szPrintSettingName[32 + 1];		/*!< 印刷設定の名前 */
	char			m_szPrintFontFaceHan[LF_FACESIZE];	/*!< 印刷フォント */
	char			m_szPrintFontFaceZen[LF_FACESIZE];	/*!< 印刷フォント */
	int				m_nPrintFontWidth;					/*!< 印刷フォント幅(1/10mm単位単位) */
	int				m_nPrintFontHeight;					/*!< 印刷フォント高さ(1/10mm単位単位) */
	int				m_nPrintDansuu;						/*!< 段組の段数 */
	int				m_nPrintDanSpace;					/*!< 段と段の隙間(1/10mm単位) */
	int				m_nPrintLineSpacing;				/*!< 印刷フォント行間 文字の高さに対する割合(%) */
	int				m_nPrintMarginTY;					/*!< 印刷用紙マージン 上(mm単位) */
	int				m_nPrintMarginBY;					/*!< 印刷用紙マージン 下(mm単位) */
	int				m_nPrintMarginLX;					/*!< 印刷用紙マージン 左(mm単位) */
	int				m_nPrintMarginRX;					/*!< 印刷用紙マージン 右(mm単位) */
	int				m_nPrintPaperOrientation;			/*!< 用紙方向 DMORIENT_PORTRAIT (1) または DMORIENT_LANDSCAPE (2) */
	int				m_nPrintPaperSize;					/*!< 用紙サイズ */
	BOOL			m_bPrintWordWrap;					/*!< 英文ワードラップする */
	BOOL			m_bPrintKinsokuHead;				/*!< 行頭禁則する */	//@@@ 2002.04.09 MIK
	BOOL			m_bPrintKinsokuTail;				/*!< 行末禁則する */	//@@@ 2002.04.09 MIK
	BOOL			m_bPrintKinsokuRet;					/*!< 改行文字のぶら下げ */	//@@@ 2002.04.13 MIK
	BOOL			m_bPrintKinsokuKuto;				/*!< 句読点のぶらさげ */	//@@@ 2002.04.17 MIK
	BOOL			m_bPrintLineNumber;					/*!< 行番号を印刷する */


	MYDEVMODE		m_mdmDevMode;						/*!< プリンタ設定 DEVMODE用 */
	BOOL			m_bHeaderUse[3];					/* ヘッダが使われているか？	*/
	char			m_szHeaderForm[3][HEADER_MAX];		/* 0:左寄せヘッダ。1:中央寄せヘッダ。2:右寄せヘッダ。*/
	BOOL			m_bFooterUse[3];					/* フッタが使われているか？	*/
	char			m_szFooterForm[3][FOOTER_MAX];		/* 0:左寄せフッタ。1:中央寄せフッタ。2:右寄せフッタ。*/
};


//! 色設定
struct ColorInfo {
	int			m_nColorIdx;
	BOOL		m_bDisp;			/* 色分け/表示 をする */
	BOOL		m_bFatFont;			/* 太字か */
	BOOL		m_bUnderLine;		/* アンダーラインか */
	COLORREF	m_colTEXT;			/* 前景色(文字色) */
	COLORREF	m_colBACK;			/* 背景色 */
	char		m_szName[32];		/* 名前 */
	char		m_cReserved[60];
};

//! 色設定(保存用)
struct ColorInfoIni {
	const char*	m_pszName;			/* 色名 */
	BOOL		m_bDisp;			/* 色分け/表示 をする */
	BOOL		m_bFatFont;			/* 太字か */
	BOOL		m_bUnderLine;		/* アンダーラインか */
	COLORREF	m_colTEXT;			/* 前景色(文字色) */
	COLORREF	m_colBACK;			/* 背景色 */
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

//! タイプ別設定
struct Types {
	int					m_nIdx;
	char				m_szTypeName[64];				/*!< タイプ属性：名称 */
	char				m_szTypeExts[64];				/*!< タイプ属性：拡張子リスト */
	int					m_nMaxLineSize;					/*!< 折り返し文字数 */
	int					m_nColmSpace;					/*!< 文字と文字の隙間 */
	int					m_nLineSpace;					/*!< 行間のすきま */
	int					m_nTabSpace;					/*!< TABの文字数 */
	int					m_bTabArrow;					/*!< タブ矢印表示 */	//@@@ 2003.03.26 MIK
	char				m_szTabViewString[17];			/*!< TAB表示文字列 */	// 2003.1.26 aroka サイズ拡張
	int					m_bInsSpace;					/* スペースの挿入 */	// 2001.12.03 hor
	// 2005.01.13 MIK 配列化
	int					m_nKeyWordSetIdx[MAX_KEYWORDSET_PER_TYPE];	/*!< キーワードセット */

	CLineComment		m_cLineComment;					/*!< 行コメントデリミタ */			//@@@ 2002.09.22 YAZAKI
	CBlockComment		m_cBlockComment;				/*!< ブロックコメントデリミタ */	//@@@ 2002.09.22 YAZAKI

	int					m_nStringType;					/*!< 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	char				m_szIndentChars[64];			/*!< その他のインデント対象文字 */

	int					m_nColorInfoArrNum;				/*!< 色設定配列の有効数 */
	ColorInfo			m_ColorInfoArr[64];				/*!< 色設定配列 */

	int					m_bLineNumIsCRLF;				/*!< 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	int					m_nLineTermType;				/*!< 行番号区切り  0=なし 1=縦線 2=任意 */
	char				m_cLineTermChar;				/*!< 行番号区切り文字 */
	int					m_nVertLineIdx[MAX_VERTLINES];	/*!< 指定桁縦線 */

	BOOL				m_bWordWrap;					/*!< 英文ワードラップをする */
	BOOL				m_bKinsokuHead;					/*!< 行頭禁則をする */	//@@@ 2002.04.08 MIK
	BOOL				m_bKinsokuTail;					/*!< 行末禁則をする */	//@@@ 2002.04.08 MIK
	BOOL				m_bKinsokuRet;					/*!< 改行文字のぶら下げ */	//@@@ 2002.04.13 MIK
	BOOL				m_bKinsokuKuto;					/*!< 句読点のぶらさげ */	//@@@ 2002.04.17 MIK
	char				m_szKinsokuHead[200];			/*!< 行頭禁則文字 */	//@@@ 2002.04.08 MIK
	char				m_szKinsokuTail[200];			/*!< 行頭禁則文字 */	//@@@ 2002.04.08 MIK

	int					m_nCurrentPrintSetting;			/*!< 現在選択している印刷設定 */

	int					m_nDefaultOutline;				/*!< アウトライン解析方法 */
	char				m_szOutlineRuleFilename[_MAX_PATH];	/*!< アウトライン解析ルールファイル */

	int					m_nSmartIndent;					/*!< スマートインデント種別 */
	int					m_nImeState;	//	Nov. 20, 2000 genta 初期IME状態

	//	2001/06/14 asa-o 補完のタイプ別設定
	char				m_szHokanFile[_MAX_PATH];		/*!< 入力補完 単語ファイル */
	//	2003.06.23 Moca ファイル内からの入力補完機能
	int					m_bUseHokanByFile;				/*!< 入力補完 開いているファイル内から候補を探す */
	//	2001/06/19 asa-o
	int					m_bHokanLoHiCase;				/*!< 入力補完機能：英大文字小文字を同一視する */

	char				m_szExtHelp[_MAX_PATH];		/* 外部ヘルプ１ */
	char				m_szExtHtmlHelp[_MAX_PATH];		/* 外部HTMLヘルプ */
	BOOL				m_bHtmlHelpIsSingle;			/* HtmlHelpビューアはひとつ */
	
	
//@@@ 2001.11.17 add start MIK
	BOOL	m_bUseRegexKeyword;	/* 正規表現キーワードを使うか*/
	int	m_nRegexKeyMagicNumber;	/* 正規表現キーワード更新マジックナンバー */
	struct RegexKeywordInfo	m_RegexKeywordArr[MAX_REGEX_KEYWORD];	/* 正規表現キーワード */
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	BOOL				m_bUseKeyWordHelp;			/* キーワード辞書セレクト機能を使うか */
	int					m_nKeyHelpNum;					/* キーワード辞書の冊数 */
	struct	KeyHelpInfo	m_KeyHelpArr[MAX_KEYHELP_FILE];	/* キーワード辞書ファイル */
	BOOL				m_bUseKeyHelpAllSearch;			/* ヒットした次の辞書も検索(&A) */
	BOOL				m_bUseKeyHelpKeyDisp;			/* 1行目にキーワードも表示する(&W) */
	BOOL				m_bUseKeyHelpPrefix;			/* 選択範囲で前方一致検索(&P) */
//@@@ 2006.04.10 fon ADD-end

	//	2002/04/30 YAZAKI Commonから移動。
	BOOL				m_bAutoIndent;					/* オートインデント */
	BOOL				m_bAutoIndent_ZENSPACE;			/* 日本語空白もインデント */
	BOOL				m_bRTrimPrevLine;				/* 2005.10.11 ryoji 改行時に末尾の空白を削除 */
	int					m_nIndentLayout;				/* 折り返しは2行目以降を字下げ表示 */
	
	//	Sep. 10, 2002 genta
	int					m_bUseDocumentIcon;	/*!< ファイルに関連づけられたアイコンを使う */

}; /* Types */

//! マクロ情報
struct MacroRec {
	char	m_szName[MACRONAME_MAX];	//<! 表示名
	char	m_szFile[_MAX_PATH+1];	//<! ファイル名(ディレクトリを含まない)
	BOOL	m_bReloadWhenExecute;	//	実行時に読み込みなおすか（デフォルトon）
	
	bool IsEnabled() const { return m_szFile[0] != '\0'; }
};
//	To Here Sep. 14, 2001 genta

// 2004/06/21 novice タグジャンプ機能追加
//! タグジャンプ情報
struct TagJump {
	HWND	hwndReferer;				//<! 参照元ウィンドウ
	POINT	point;						//<! ライン, カラム
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

// Apr. 05, 2003 genta WindowCaption用領域（変換前）の長さ
const int MAX_CAPTION_CONF_LEN = 256;

//	2004.05.13 Moca
//! ウィンドウサイズ・位置の制御方法
enum eWINSIZEMODE{
	WINSIZEMODE_DEF = 0, //!< 指定なし
	WINSIZEMODE_SAVE = 1, //!< 継承(保存)
	WINSIZEMODE_SET = 2   //!< 直接指定(固定)
};


//! 共通設定
struct Common {

	//	Jul. 3, 2000 genta
	//	アクセス関数(簡易)
	//	intをビット単位に分割して使う
	//	下4bitをCaretTypeに当てておく(将来の予約で多めに取っておく)
	int		GetCaretType(void) const { return m_nCaretType & 0xf; }
	void	SetCaretType(const int f){ m_nCaretType &= ~0xf; m_nCaretType |= f & 0xf; }

	//	Aug. 15, 2000 genta
	//	Backup設定のアクセス関数
	int		GetBackupType(void) const { return m_nBackUpType; }
	void	SetBackupType(int n){ m_nBackUpType = n; }

	bool	GetBackupOpt(int flag) const { return ( flag & m_nBackUpType_Opt1 ) == flag; }
	void	SetBackupOpt(int flag, bool value){
		m_nBackUpType_Opt1 = value ? ( flag | m_nBackUpType_Opt1) :  ((~flag) & m_nBackUpType_Opt1 );
	}

	//	バックアップ数
	int		GetBackupCount(void) const { return m_nBackUpType_Opt2 & 0xffff; }
	void	SetBackupCount(int value){
		m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xffff0000) | ( value & 0xffff );
	}

	//	バックアップの拡張子先頭文字(1文字)
	int		GetBackupExtChar(void) const { return ( m_nBackUpType_Opt2 >> 16 ) & 0xff ; }
	void	SetBackupExtChar(int value){
		m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xff00ffff) | (( value & 0xff ) << 16 );
	}

	//	Aug. 21, 2000 genta
	//	自動Backup
	bool	IsAutoBackupEnabled(void) const { return GetBackupOpt( BKUP_AUTO ); }
	void	EnableAutoBackup(bool flag){ SetBackupOpt( BKUP_AUTO, flag ); }

	int		GetAutoBackupInterval(void) const { return m_nBackUpType_Opt3; }
	void	SetAutoBackupInterval(int i){ m_nBackUpType_Opt3 = i; }

	//	Backup詳細設定のアクセス関数
	int		GetBackupTypeAdv(void) const { return m_nBackUpType_Opt4; }
	void	SetBackupTypeAdv(int n){ m_nBackUpType_Opt4 = n; }

	//	Oct. 27, 2000 genta
	//	カーソル位置を復元するかどうか
	bool	GetRestoreCurPosition(void) const { return m_bRestoreCurPosition != 0; }
	void	SetRestoreCurPosition(bool i){ m_bRestoreCurPosition = i; }

	// 2002.01.16 hor ブックマークを復元するかどうか
	bool	GetRestoreBookmarks(void) const { return m_bRestoreBookmarks != 0; }
	void	SetRestoreBookmarks(bool i){ m_bRestoreBookmarks = i; }

	//	Nov. 12, 2000 genta
	//	ファイル読み込み時にMIMEのdecodeを行うか
	bool	GetAutoMIMEdecode(void) const { return m_bAutoMIMEdecode != 0; }
	void	SetAutoMIMEdecode(bool i){ m_bAutoMIMEdecode = i; }

	// Oct. 03, 2004 genta 前回と文字コードが異なるときに問い合わせを行う
	bool	GetQueryIfCodeChange(void) const { return m_bQueryIfCodeChange != 0; }
	void	SetQueryIfCodeChange(bool i){ m_bQueryIfCodeChange = i; }
	
	// Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告する
	bool	GetAlertIfFileNotExist(void) const { return m_bAlertIfFileNotExist != 0; }
	void	SetAlertIfFileNotExist(bool i){ m_bAlertIfFileNotExist = i; }
	

	//	注意: 設定ファイルからの読み込み時にINTとして扱うため，bool型を使ってはいけない．
	//	sizeof(int) != sizeof(bool)だとデータを破壊してしまう．

	int					m_nCaretType;					/* カーソルのタイプ 0=win 1=dos  */
	int					m_bIsINSMode;					/* 挿入／上書きモード */
	int					m_bIsFreeCursorMode;			/* フリーカーソルモードか */
	BOOL				m_bStopsBothEndsWhenSearchWord;	/* 単語単位で移動するときに、単語の両端で止まるか */
	BOOL				m_bStopsBothEndsWhenSearchParagraph;	/* 段落単位で移動するときに、段落の両端で止まるか */

	BOOL				m_bRestoreCurPosition;			//	ファイルを開いたときカーソル位置を復元するか
	BOOL				m_bRestoreBookmarks;			// 2002.01.16 hor ブックマークを復元するかどうか

	int					m_bAddCRLFWhenCopy;				/* 折り返し行に改行を付けてコピー */
	int					m_nRepeatedScrollLineNum;		/* キーリピート時のスクロール行数 */
	BOOL				m_nRepeatedScroll_Smooth;		/* キーリピート時のスクロールを滑らかにするか */
	BOOL				m_bCloseAllConfirm;				/* [すべて閉じる]で他に編集用のウィンドウがあれば確認する */	// 2006.12.25 ryoji
	BOOL				m_bExitConfirm;					/* 終了時の確認をする */
	int					m_bRegularExp;					/* 検索／置換  1==正規表現 */
	int					m_bLoHiCase;					/* 検索／置換  1==英大文字小文字の区別 */
	int					m_bWordOnly;					/* 検索／置換  1==単語のみ検索 */
	int					m_bConsecutiveAll;				/* 「すべて置換」は置換の繰返し */	// 2007.01.16 ryoji
	int					m_bNOTIFYNOTFOUND;				/* 検索／置換  見つからないときメッセージを表示 */
	int					m_bSelectedArea;				/* 置換  選択範囲内置換 */
	int					m_bGrepSubFolder;				/* Grep: サブフォルダも検索 */
	BOOL				m_bGrepOutputLine;				/* Grep: 行を出力するか該当部分だけ出力するか */
	int					m_nGrepOutputStyle;				/* Grep: 出力形式 */
	int					m_bGrepDefaultFolder;			/* Grep: フォルダの初期値をカレントフォルダにする */
	int					m_nGrepCharSet;					/* Grep: 文字コードセット */ // 2002/09/20 Moca Add
	int					m_bGrepRealTimeView;			/* 2003.06.16 Moca Grep結果のリアルタイム表示 */

	BOOL				m_bGTJW_RETURN;					/* エンターキーでタグジャンプ */
	BOOL				m_bGTJW_LDBLCLK;				/* ダブルクリックでタグジャンプ */
	LOGFONT				m_lf;							/* 現在のフォント情報 */
	BOOL				m_bFontIs_FIXED_PITCH;			/* 現在のフォントは固定幅フォントである */
	BOOL				m_bBackUp;						/* バックアップの作成 */
	BOOL				m_bBackUpDialog;				/* バックアップの作成前に確認 */
	BOOL				m_bBackUpFolder;				/* 指定フォルダにバックアップを作成する */
	char				m_szBackUpFolder[_MAX_PATH];	/* バックアップを作成するフォルダ */
	int 				m_nBackUpType;					/* バックアップファイル名のタイプ 1=(.bak) 2=*_日付.* */
	int 				m_nBackUpType_Opt1;				/* バックアップファイル名：オプション1 */
	int 				m_nBackUpType_Opt2;				/* バックアップファイル名：オプション2 */
	int 				m_nBackUpType_Opt3;				/* バックアップファイル名：オプション3 */
	int 				m_nBackUpType_Opt4;				/* バックアップファイル名：オプション4 */
	int 				m_nBackUpType_Opt5;				/* バックアップファイル名：オプション5 */
	int 				m_nBackUpType_Opt6;				/* バックアップファイル名：オプション6 */
	BOOL				m_bBackUpDustBox;			/* バックアップファイルをごみ箱に放り込む */	//@@@ 2001.12.11 add MIK
	BOOL				m_bBackUpPathAdvanced;				/* 20051107 aroka バックアップ先フォルダを詳細設定する */
	char				m_szBackUpPathAdvanced[_MAX_PATH];	/* 20051107 aroka バックアップを作成するフォルダの詳細設定 */
	int					m_nFileShareMode;				/* ファイルの排他制御モード */
	char				m_szExtHelp[_MAX_PATH];		/* 外部ヘルプ１ */
	char				m_szExtHtmlHelp[_MAX_PATH];		/* 外部HTMLヘルプ */
	LOGFONT				m_lf_kh;						/* キーワードヘルプのフォント情報 */		// ai 02/05/21 Add
	BOOL				m_bUseCaretKeyWord;				/* キャレット位置の単語を辞書検索 */		// 2006.03.24 fon

	int					m_nToolBarButtonNum;			/* ツールバーボタンの数 */
	int					m_nToolBarButtonIdxArr[MAX_TOOLBARBUTTONS];	/* ツールバーボタン構造体 */
	int					m_bToolBarIsFlat;				/* フラットツールバーにする／しない */
	int					m_nMRUArrNum_MAX;
	int					m_nOPENFOLDERArrNum_MAX;
	BOOL				m_bDispTOOLBAR;					/* 次回ウィンドウを開いたときツールバーを表示する */
	BOOL				m_bDispSTATUSBAR;				/* 次回ウィンドウを開いたときステータスバーを表示する */

	BOOL				m_bDispFUNCKEYWND;				/* 次回ウィンドウを開いたときファンクションキーを表示する */
	int					m_nFUNCKEYWND_Place;			/* ファンクションキー表示位置／0:上 1:下 */
	int					m_nFUNCKEYWND_GroupNum;			// 2002/11/04 Moca ファンクションキーのグループボタン数

	BOOL				m_bDispTabWnd;					//タブウインドウ表示する	//@@@ 2003.05.31 MIK
	BOOL				m_bDispTabWndMultiWin;			//タブをまとめない	//@@@ 2003.05.31 MIK
	BOOL				m_bTab_RetainEmptyWin;			//!< 最後の文書が閉じられたとき(無題)を残す
	BOOL				m_bTab_CloseOneWin;		//!< タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる

	char				m_szTabWndCaption[MAX_CAPTION_CONF_LEN];	//タブウインドウキャプション	//@@@ 2003.06.13 MIK
	BOOL				m_bSameTabWidth;				//タブを等幅にする			//@@@ 2006.01.28 ryoji
	BOOL				m_bDispTabIcon;					//タブにアイコンを表示する	//@@@ 2006.01.28 ryoji
	BOOL				m_bSortTabList;					//タブ一覧をソートする	//@@@ 2006.03.23 fon
	BOOL				m_bTab_ListFull;				//タブ一覧をフルパス表示する	//@@@ 2007.02.28 ryoji

	BOOL				m_bSplitterWndHScroll;			// 2001/06/20 asa-o 分割ウィンドウの水平スクロールの同期をとる
	BOOL				m_bSplitterWndVScroll;			// 2001/06/20 asa-o 分割ウィンドウの垂直スクロールの同期をとる

	/* カスタムメニュー情報 */
	char				m_szCustMenuNameArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_NAME_LEN + 1];
	int					m_nCustMenuItemNumArr [MAX_CUSTOM_MENU];
	int					m_nCustMenuItemFuncArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	char				m_nCustMenuItemKeyArr [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	char				m_szMidashiKigou[256];			/* 見出し記号 */
	char				m_szInyouKigou[32];				/* 引用符 */
	int					m_bUseHokan;					/* 入力補完機能を使用する */

	int					m_nSaveWindowSize;	// 2004.05.13 Moca ウィンドウサイズ継承・固定 eWINSIZEMODEに順ずる
	int					m_nWinSizeType;
	int					m_nWinSizeCX;
	int					m_nWinSizeCY;
	int					m_nSaveWindowPos;	// 2004.05.13 Moca ウィンドウ位置継承・固定 eWINSIZEMODEに順ずる
	int					m_nWinPosX;
	int					m_nWinPosY;

	BOOL				m_bUseTaskTray;				/* タスクトレイのアイコンを使う */
	BOOL				m_bStayTaskTray;			/* タスクトレイのアイコンを常駐 */

	WORD				m_wTrayMenuHotKeyCode;		/* タスクトレイ左クリックメニュー キー */
	WORD				m_wTrayMenuHotKeyMods;		/* タスクトレイ左クリックメニュー キー */

	BOOL				m_bUseOLE_DragDrop;			/* OLEによるドラッグ & ドロップを使う */
	BOOL				m_bUseOLE_DropSource;		/* OLEによるドラッグ元にするか */


	BOOL				m_bDispExitingDialog;			/* 終了ダイアログを表示する */
	BOOL				m_bEnableUnmodifiedOverwrite;	/* 無変更でも上書きするか */
	BOOL				m_bJumpSingleClickURL;			/* URLのシングルクリックでJump */
	BOOL				m_bSelectClickedURL;			/* URLがクリックされたら選択するか */
	BOOL				m_bGrepExitConfirm;				/* Grepモードで保存確認するか */

	int					m_nRulerHeight;				/* ルーラー高さ */
	int					m_nRulerBottomSpace;		/* ルーラーとテキストの隙間 */
	int					m_nRulerType;				/* ルーラーのタイプ */
	int					m_nLineNumRightSpace;		// 行番号の右のスペース Sep. 18, 2002 genta
	int					m_nVertLineOffset;			// 縦線の描画座標オフセット 2005.11.10 Moca

	BOOL				m_bCopyAndDisablSelection;	/* コピーしたら選択解除 */
	BOOL				m_bHtmlHelpIsSingle;		/* HtmlHelpビューアはひとつ */
	BOOL				m_bCompareAndTileHorz;		/* 文書比較後、左右に並べて表示 */
	BOOL				m_bDropFileAndClose;		/* ファイルをドロップしたときは閉じて開く */
	int					m_nDropFileNumMax;			/* 一度にドロップ可能なファイル数 */
	BOOL				m_bCheckFileTimeStamp;		/* 更新の監視 */
	BOOL				m_bNotOverWriteCRLF;		/* 改行は上書きしない */
	RECT				m_rcOpenDialog;				/* 「開く」ダイアログのサイズと位置 */
	BOOL				m_bAutoCloseDlgFind;		/* 検索ダイアログを自動的に閉じる */
	BOOL				m_bScrollBarHorz;			/* 水平スクロールバーを使う */
	BOOL				m_bAutoCloseDlgFuncList;	/* アウトライン ダイアログを自動的に閉じる */
	BOOL				m_bAutoCloseDlgReplace;		/* 置換 ダイアログを自動的に閉じる */
	BOOL				m_bSearchAll;				/* 先頭（末尾）から再検索 2002.01.26 hor */
	BOOL				m_bAutoColmnPaste;			/* 矩形コピーのテキストは常に矩形貼り付け */

	BOOL				m_bHokanKey_RETURN;	/* VK_RETURN	補完決定キーが有効/無効 */
	BOOL				m_bHokanKey_TAB;	/* VK_TAB		補完決定キーが有効/無効 */
	BOOL				m_bHokanKey_RIGHT;	/* VK_RIGHT		補完決定キーが有効/無効 */
	BOOL				m_bHokanKey_SPACE;	/* VK_SPACE		補完決定キーが有効/無効 */


	int					m_nDateFormatType;						//日付書式のタイプ
	char				m_szDateFormat[MAX_DATETIMEFOREMAT_LEN];//日付書式
	int					m_nTimeFormatType;						//時刻書式のタイプ
	char				m_szTimeFormat[MAX_DATETIMEFOREMAT_LEN];//時刻書式

	BOOL				m_bMenuIcon;		/* メニューにアイコンを表示する */
	BOOL				m_bAutoMIMEdecode;	// ファイル読み込み時にMIMEのdecodeを行うか
	BOOL				m_bQueryIfCodeChange;	// Oct. 03, 2004 genta 前回と文字コードが異なるときに問い合わせを行う
	BOOL				m_bAlertIfFileNotExist;	// Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告する

	BOOL				m_bMarkUpBlankLineEnable;	//2002.02.08 aroka,hor
	BOOL				m_bFunclistSetFocusOnJump;	//2002.02.08 hor

	// Apr. 5, 2003 genta
	char				m_szWindowCaptionActive[MAX_CAPTION_CONF_LEN];
	char				m_szWindowCaptionInactive[MAX_CAPTION_CONF_LEN];

	//2004.10.28
	char				m_szMigemoDll[_MAX_PATH];		/* migemo dll */
	char				m_szMigemoDict[_MAX_PATH];		/* migemo dict */

//	int					m_nTagDepth;	//ダイレクトタグジャンプ階層

	// 20060201 aroka アウトライン/トピックリスト の位置とサイズを記憶
	int					m_bRememberOutlineWindowPos;
	int					m_widthOutlineWindow;
	int					m_heightOutlineWindow;
	int					m_xOutlineWindowPos;
	int					m_yOutlineWindowPos;

	// ファイル保存ダイアログのフィルタ設定	// 2006.11.16 ryoji
	BOOL				m_bNoFilterSaveNew;		// 新規から保存時は全ファイル表示
	BOOL				m_bNoFilterSaveFile;	// 新規以外から保存時は全ファイル表示
}; /* Common */

//! 共有データ領域
struct DLLSHAREDATA {
	//	Oct. 27, 2000 genta
	//!	データ構造 Version
	/*	データ構造の異なるバージョンの同時起動を防ぐため
		必ず先頭になくてはならない．
	*/
	unsigned int		m_vStructureVersion;
	/* 共通作業域(保存しない) */
	char				m_szWork[32000];
	FileInfo			m_FileInfo_MYWM_GETFILEINFO;

	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
	HWND				m_hwndTray;
	HWND				m_hwndDebug;
	HACCEL				m_hAccel;
	LONG				m_nSequences;	/* ウィンドウ連番 */
	/**** 共通作業域(保存する) ****/
	int					m_nEditArrNum;	//short->intに修正	//@@@ 2003.05.31 MIK
	EditNode			m_pEditArr[MAX_EDITWINDOWS];	//最大値修正	@@@ 2003.05.31 MIK

	//From Here 2003.05.31 MIK
	WINDOWPLACEMENT		m_TabWndWndpl;					//タブウインドウ時のウインドウ情報
	//To Here 2003.05.31 MIK

//@@@ 2001.12.26 YAZAKI	以下の2つは、直接アクセスしないでください。CMRUを経由してください。
	int					m_nMRUArrNum;
	FileInfo			m_fiMRUArr[MAX_MRU];
	bool				m_bMRUArrFavorite[MAX_MRU];	//お気に入り	//@@@ 2003.04.08 MIK

//@@@ 2001.12.26 YAZAKI	以下の2つは、直接アクセスしないでください。CMRUFolderを経由してください。
	int					m_nOPENFOLDERArrNum;
	char				m_szOPENFOLDERArr[MAX_OPENFOLDER][_MAX_PATH];
	bool				m_bOPENFOLDERArrFavorite[MAX_OPENFOLDER];	//お気に入り	//@@@ 2003.04.08 MIK

	int					m_nTransformFileNameArrNum;
	char				m_szTransformFileNameFrom[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	char				m_szTransformFileNameTo[MAX_TRANSFORM_FILENAME][_MAX_PATH];	//お気に入り	//@@@ 2003.04.08 MIK

	int					m_nSEARCHKEYArrNum;
	char				m_szSEARCHKEYArr[MAX_SEARCHKEY][_MAX_PATH];
//	bool				m_bSEARCHKEYArrFavorite[MAX_SEARCHKEY];	//お気に入り	//@@@ 2003.04.08 MIK
	int					m_nREPLACEKEYArrNum;
	char				m_szREPLACEKEYArr[MAX_REPLACEKEY][_MAX_PATH];
//	bool				m_bREPLACEKEYArrFavorite[MAX_REPLACEKEY];	//お気に入り	//@@@ 2003.04.08 MIK
	int					m_nGREPFILEArrNum;
	char				m_szGREPFILEArr[MAX_GREPFILE][_MAX_PATH];
//	bool				m_bGREPFILEArrFavorite[MAX_GREPFILE];	//お気に入り	//@@@ 2003.04.08 MIK
	int					m_nGREPFOLDERArrNum;
	char				m_szGREPFOLDERArr[MAX_GREPFOLDER][_MAX_PATH];
//	bool				m_bGREPFOLDERArrFavorite[MAX_GREPFOLDER];	//お気に入り	//@@@ 2003.04.08 MIK

	char				m_szMACROFOLDER[_MAX_PATH];		/* マクロ用フォルダ */
	char				m_szIMPORTFOLDER[_MAX_PATH];	/* 設定インポート用フォルダ */
	
	//	Sep. 14, 2001 genta
	MacroRec			m_MacroTable[MAX_CUSTMACRO];	//!< キー割り当て用マクロテーブル

	// 2004/06/21 タグジャンプ機能追加
	int					m_TagJumpNum;					//!< タグジャンプ情報の有効データ数
	int					m_TagJumpTop;					//!< スタックの一番上の位置
	TagJump				m_TagJump[MAX_TAGJUMPNUM];		//!< タグジャンプ情報

//	MRU_ExtCmd			m_MRU_ExtCmd;	/* MRU 外部コマンド */
	char				m_szCmdArr[MAX_CMDARR][MAX_CMDLEN];
	int					m_nCmdArrNum;
//	bool				m_bCmdArrFavorite[MAX_CMDARR];	//お気に入り	//@@@ 2003.04.08 MIK


	/**** 共通設定 ****/
	Common				m_Common;

	/* キー割り当て */
	int					m_nKeyNameArrNum;			/* キー割り当て表の有効データ数 */
	KEYDATA				m_pKeyNameArr[100];			/* キー割り当て表 */

	/**** 印刷ページ設定 ****/
	PRINTSETTING		m_PrintSettingArr[MAX_PRINTSETTINGARR];

	/* 強調キーワード設定 */
	CKeyWordSetMgr		m_CKeyWordSetMgr;					/* 強調キーワード */
	char				m_szKeyWordSetDir[MAX_PATH];		/* 強調キーワードファイルのディレクトリ */

	/* **** タイプ別設定 **** */
	Types				m_Types[MAX_TYPES];

	/*	@@@ 2002.1.24 YAZAKI
		キーボードマクロは、記録終了した時点でファイル「m_szKeyMacroFileName」に書き出すことにする。
		m_bRecordingKeyMacroがTRUEのときは、キーボードマクロの記録中なので、m_szKeyMacroFileNameにアクセスしてはならない。
	*/
	BOOL				m_bRecordingKeyMacro;		/* キーボードマクロの記録中 */
	HWND				m_hwndRecordingKeyMacro;	/* キーボードマクロを記録中のウィンドウ */
	char				m_szKeyMacroFileName[MAX_PATH];	/* キーボードマクロのファイル名 */

//@@@ 2002.01.08 YAZAKI 設定を保存するためにShareDataに移動
	/* **** その他のダイアログ **** */
	BOOL				m_bGetStdout;		/* 外部コマンド実行の「標準出力を得る」 */
	BOOL				m_bLineNumIsCRLF;	/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */

	int					m_nDiffFlgOpt;		/* DIFF差分表示 */	//@@@ 2002.05.27 MIK
	
	char				m_szTagsCmdLine[_MAX_PATH];	/* TAGSコマンドラインオプション */	//@@@ 2003.05.12 MIK
	int					m_nTagsOpt;			/* TAGSオプション(チェック) */	//@@@ 2003.05.12 MIK
	//From Here 2005.04.03 MIK キーワード指定タグジャンプ
	int					m_nTagJumpKeywordArrNum;
	char				m_szTagJumpKeywordArr[MAX_TAGJUMP_KEYWORD][_MAX_PATH];
//	bool				m_bTagJumpKeywordArrFavorite[MAX_TAGJUMPKEYWORD];	//お気に入り
	BOOL				m_bTagJumpICase;	//!< 大文字小文字を同一視
	BOOL				m_bTagJumpAnyWhere;	//!< 文字列の途中にマッチ
	//To Here 2005.04.03 MIK
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
class SAKURA_CORE_API CShareData
{
public:
	/*
	||	Singleton風
	*/
	static CShareData* getInstance();

protected:
	static CShareData* _instance;

public:
	/*
	||  Constructors
	*/
	CShareData();
	~CShareData();

	/*
	||  Attributes & Operations
	*/
	bool Init(void);	/* CShareDataクラスの初期化処理 */
	DLLSHAREDATA* GetShareData(){ return m_pShareData; }		/* 共有データ構造体のアドレスを返す */
	int GetDocumentType( const char* pszFilePath );				/* ファイルパスを渡して、ドキュメントタイプ（数値）を取得する */
	int GetDocumentTypeExt( const char* pszExt );				/* 拡張子を渡して、ドキュメントタイプ（数値）を取得する */
	BOOL AddEditWndList( HWND );								/* 編集ウィンドウの登録 */
	void DeleteEditWndList( HWND );								/* 編集ウィンドウリストからの削除 */

	BOOL RequestCloseAllEditor( BOOL bExit );					/* 全編集ウィンドウへ終了要求を出す */	// 2007.02.13 ryoji 「編集の全終了」を示す引数(bExit)を追加
	BOOL IsPathOpened( const char*, HWND* );					/* 指定ファイルが開かれているか調べる */
	int GetEditorWindowsNum( void );							/* 現在の編集ウィンドウの数を調べる */
	BOOL PostMessageToAllEditors( UINT, WPARAM, LPARAM, HWND );	/* 全編集ウィンドウへメッセージをポストする */
	BOOL SendMessageToAllEditors( UINT, WPARAM, LPARAM, HWND );	/* 全編集ウィンドウへメッセージを送るする */
	int GetOpenedWindowArr( EditNode** , BOOL );				/* 現在開いている編集ウィンドウの配列を返す */
	static BOOL IsEditWnd( HWND );								/* 指定ウィンドウが、編集ウィンドウのフレームウィンドウかどうか調べる */
	static void SetKeyNameArrVal(
		DLLSHAREDATA*, int, short, char*,
		short, short, short, short,
		short, short, short, short );									/* KEYDATA配列にデータをセット */
//	static void SetKeyNameArrVal( DLLSHAREDATA*, int, short, char* );	/* KEYDATA配列にデータをセット */ // 20050818 aroka 未使用なので削除
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
	BOOL LoadShareData( void );	/* 共有データのロード */
	void SaveShareData( void );	/* 共有データの保存 */
	BOOL ShareData_IO_2( bool );	/* 共有データの保存 */
	static void IO_ColorSet( CProfile* , const char* , ColorInfo* );	/* 色設定 I/O */ // Feb. 12, 2006 D.S.Koba

//	int			m_nStdToolBarButtons; 2004.03.30 Moca 未使用

	//@@@ 2002.2.2 YAZAKI
	//	Jun. 14, 2003 genta 引数追加．書式変更
	int		GetMacroFilename( int idx, char* pszPath, int nBufLen ); // idxで指定したマクロファイル名（フルパス）を取得する
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
	const char* MyGetDateFormat( SYSTEMTIME& systime, char* pszDest, int nDestLen );
	const char* MyGetTimeFormat( SYSTEMTIME& systime, char* pszDest, int nDestLen );
	const char* MyGetDateFormat( SYSTEMTIME& systime, char* pszDest, int nDestLen, int nDateFormatType, char* szDateFormat );
	const char* MyGetTimeFormat( SYSTEMTIME& systime, char* pszDest, int nDestLen, int nTimeFormatType, char* szTimeFormat );
	
	// 2002.11.24 Moca Add
//	LPTSTR GetTransformFileList( LPCTSTR*, LPTSTR*, int );
//	LPTSTR GetTransformFileName( LPCTSTR, LPTSTR, int );
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

//	long GetModuleDir(char* , long );	/* この実行ファイルのあるディレクトリを返します */
	/* MRUとOPENFOLDERリストの存在チェックなど
	存在しないファイルやフォルダはMRUやOPENFOLDERリストから削除する
	 */
//@@@ 2002.01.03 YAZAKI CMRU、CMRUFolderに移動した。
//	void CheckMRUandOPENFOLDERList( void );

	// ファイル名簡易表示用キャッシュ
	int m_nTransformFileNameCount; // 有効数
	TCHAR m_szTransformFileNameFromExp[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	int m_nTransformFileNameOrgId[MAX_TRANSFORM_FILENAME];

	//	Jan. 30, 2005 genta 初期化関数の分割
	void InitKeyword(DLLSHAREDATA*);
	void InitKeyAssign(DLLSHAREDATA*);
	void InitToolButtons(DLLSHAREDATA*);
	void InitTypeConfig(DLLSHAREDATA*);
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
	void ShareData_IO_KeyWords( CProfile& );
	void ShareData_IO_Macro( CProfile& );
	void ShareData_IO_Other( CProfile& );
};



///////////////////////////////////////////////////////////////////////
#endif /* _CSHAREDATA_H_ */


/*[EOF]*/
