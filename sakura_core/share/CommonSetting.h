//2007.09.28 kobake Common整理

#pragma once

#include "io/CFile.h" //EShareMode

// Apr. 05, 2003 genta WindowCaption用領域（変換前）の長さ
static const int MAX_CAPTION_CONF_LEN = 256;

//	Aug. 21, 2000 genta
static const int BKUP_AUTO		= 64;

static const int MAX_DATETIMEFOREMAT_LEN	= 100;
static const int MAX_CUSTOM_MENU			=  25;
static const int MAX_CUSTOM_MENU_NAME_LEN	=  32;
static const int MAX_CUSTOM_MENU_ITEMS		=  48;
static const int MAX_TOOLBARBUTTONS			= 384;	//Oct. 22, 2000 JEPRO アイコンの最大登録数を128個増やした(256→384)


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
	int		m_nCaretType;							// カーソルのタイプ 0=win 1=dos 
	bool	m_bIsINSMode;							// 挿入／上書きモード
	bool	m_bIsFreeCursorMode;					// フリーカーソルモードか
	BOOL	m_bStopsBothEndsWhenSearchWord;			// 単語単位で移動するときに、単語の両端で止まるか
	BOOL	m_bStopsBothEndsWhenSearchParagraph;	// 段落単位で移動するときに、段落の両端で止まるか
	BOOL	m_bNoCaretMoveByActivation;				// マウスクリックにてアクティベートされた時はカーソル位置を移動しない  2007.10.02 nasukoji (add by genta)

	//スクロール
	CLayoutInt		m_nRepeatedScrollLineNum;		// キーリピート時のスクロール行数
	BOOL	m_nRepeatedScroll_Smooth;		// キーリピート時のスクロールを滑らかにするか

	//タスクトレイ
	BOOL	m_bUseTaskTray;					// タスクトレイのアイコンを使う
	BOOL	m_bStayTaskTray;				// タスクトレイのアイコンを常駐
	WORD	m_wTrayMenuHotKeyCode;			// タスクトレイ左クリックメニュー キー */
	WORD	m_wTrayMenuHotKeyMods;			// タスクトレイ左クリックメニュー キー */

	//履歴
	int		m_nMRUArrNum_MAX;				// ファイルの履歴MAX
	int		m_nOPENFOLDERArrNum_MAX;		// フォルダの履歴MAX

	//ノーカテゴリ
	BOOL	m_bCloseAllConfirm;				// [すべて閉じる]で他に編集用のウィンドウがあれば確認する	// 2006.12.25 ryoji
	BOOL	m_bExitConfirm;					// 終了時の確認をする

	//INI内設定のみ
	BOOL	m_bDispExitingDialog;			// 終了ダイアログを表示する
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ウィンドウ                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Window
{
	//基本設定
	BOOL	m_bDispTOOLBAR;				// 次回ウィンドウを開いたときツールバーを表示する
	BOOL	m_bDispSTATUSBAR;			// 次回ウィンドウを開いたときステータスバーを表示する
	BOOL	m_bDispFUNCKEYWND;			// 次回ウィンドウを開いたときファンクションキーを表示する
	BOOL	m_bMenuIcon;				// メニューにアイコンを表示する (アイコン付きメニュー)
	BOOL	m_bScrollBarHorz;			// 水平スクロールバーを使う
	BOOL	m_bUseCompotibleBMP;		// 再作画用互換ビットマップを使う 2007.09.09 Moca

	//位置と大きさの設定
	int		m_nSaveWindowSize;			// ウィンドウサイズ継承・固定 eWINSIZEMODEに順ずる 2004.05.13 Moca
	int		m_nWinSizeType;
	int		m_nWinSizeCX;
	int		m_nWinSizeCY;
	int		m_nSaveWindowPos;			// ウィンドウ位置継承・固定 eWINSIZEMODEに順ずる 2004.05.13 Moca
	int		m_nWinPosX;
	int		m_nWinPosY;

	//ファンクションキー
	int		m_nFUNCKEYWND_Place;		// ファンクションキー表示位置／0:上 1:下
	int		m_nFUNCKEYWND_GroupNum;		// 2002/11/04 Moca ファンクションキーのグループボタン数

	//ルーラー・行番号
	int		m_nRulerHeight;				// ルーラー高さ
	int		m_nRulerBottomSpace;		// ルーラーとテキストの隙間
	int		m_nRulerType;				// ルーラーのタイプ $$$未使用っぽい
	int		m_nLineNumRightSpace;		// 行番号の右のスペース Sep. 18, 2002 genta

	//分割ウィンドウ
	BOOL	m_bSplitterWndHScroll;		// 分割ウィンドウの水平スクロールの同期をとる 2001/06/20 asa-o
	BOOL	m_bSplitterWndVScroll;		// 分割ウィンドウの垂直スクロールの同期をとる 2001/06/20 asa-o

	//タイトルバー
	TCHAR	m_szWindowCaptionActive  [MAX_CAPTION_CONF_LEN];
	TCHAR	m_szWindowCaptionInactive[MAX_CAPTION_CONF_LEN];

	//INI内設定のみ
	int		m_nVertLineOffset;			// 縦線の描画座標オフセット 2005.11.10 Moca
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           編集                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Edit
{
	//コピー
	bool	m_bAddCRLFWhenCopy;			// 折り返し行に改行を付けてコピー
	BOOL	m_bEnableNoSelectCopy;		// 選択なしでコピーを可能にする 2007.11.18 ryoji
	BOOL	m_bCopyAndDisablSelection;	// コピーしたら選択解除
	bool	m_bEnableLineModePaste;		// ラインモード貼り付けを可能にする  2007.10.08 ryoji

	//ドラッグ＆ドロップ
	BOOL	m_bUseOLE_DragDrop;			// OLEによるドラッグ & ドロップを使う
	BOOL	m_bUseOLE_DropSource;		// OLEによるドラッグ元にするか

	//上書きモード
	BOOL	m_bNotOverWriteCRLF;		// 改行は上書きしない

	//クリッカブルURL
	BOOL	m_bJumpSingleClickURL;		// URLのシングルクリックでJump $$$未使用
	BOOL	m_bSelectClickedURL;		// URLがクリックされたら選択するか


	// (ダイアログ項目無し)
	BOOL	m_bAutoColmnPaste;			// 矩形コピーのテキストは常に矩形貼り付け
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ファイル                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct CommonSetting_File
{
public:
	// カーソル位置を復元するかどうか  Oct. 27, 2000 genta
	bool	GetRestoreCurPosition() const		{ return m_bRestoreCurPosition; }
	void	SetRestoreCurPosition(bool i)		{ m_bRestoreCurPosition = i; }

	// ブックマークを復元するかどうか  2002.01.16 hor
	bool	GetRestoreBookmarks() const			{ return m_bRestoreBookmarks; }
	void	SetRestoreBookmarks(bool i)			{ m_bRestoreBookmarks = i; }

	// ファイル読み込み時にMIMEのdecodeを行うか  Nov. 12, 2000 genta
	bool	GetAutoMIMEdecode() const			{ return m_bAutoMIMEdecode; }
	void	SetAutoMIMEdecode(bool i)			{ m_bAutoMIMEdecode = i; }

	// 前回と文字コードが異なるときに問い合わせを行う  Oct. 03, 2004 genta
	bool	GetQueryIfCodeChange() const		{ return m_bQueryIfCodeChange; }
	void	SetQueryIfCodeChange(bool i)		{ m_bQueryIfCodeChange = i; }
	
	// 開こうとしたファイルが存在しないとき警告する  Oct. 09, 2004 genta
	bool	GetAlertIfFileNotExist() const		{ return m_bAlertIfFileNotExist; }
	void	SetAlertIfFileNotExist(bool i)		{ m_bAlertIfFileNotExist = i; }

public:
	//ファイルの排他制御モード
	EShareMode		m_nFileShareMode;
	bool			m_bCheckFileTimeStamp;	// 更新の監視

	//ファイルの保存
	bool	m_bEnableUnmodifiedOverwrite;	// 無変更でも上書きするか

	//「名前を付けて保存」でファイルの種類が[ユーザー指定]のときのファイル一覧表示
	//ファイル保存ダイアログのフィルタ設定	// 2006.11.16 ryoji
	bool	m_bNoFilterSaveNew;				// 新規から保存時は全ファイル表示
	bool	m_bNoFilterSaveFile;			// 新規以外から保存時は全ファイル表示

	//ファイルオープン
	bool	m_bDropFileAndClose;			// ファイルをドロップしたときは閉じて開く
	int		m_nDropFileNumMax;				// 一度にドロップ可能なファイル数
	bool	m_bRestoreCurPosition;			// ファイルを開いたときカーソル位置を復元するか
	bool	m_bRestoreBookmarks;			// ブックマークを復元するかどうか 2002.01.16 hor
	bool	m_bAutoMIMEdecode;				// ファイル読み込み時にMIMEのdecodeを行うか
	bool	m_bQueryIfCodeChange;			// 前回と文字コードが異なるときに問い合わせを行う Oct. 03, 2004 genta
	bool	m_bAlertIfFileNotExist;			// 開こうとしたファイルが存在しないとき警告する Oct. 09, 2004 genta
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       バックアップ                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Backup
{
public:
	//	Aug. 15, 2000 genta
	//	Backup設定のアクセス関数
	int		GetBackupType(void) const			{ return m_nBackUpType; }
	void	SetBackupType(int n)				{ m_nBackUpType = n; }

	bool	GetBackupOpt(int flag) const		{ return ( flag & m_nBackUpType_Opt1 ) == flag; }
	void	SetBackupOpt(int flag, bool value)	{ m_nBackUpType_Opt1 = value ? ( flag | m_nBackUpType_Opt1) :  ((~flag) & m_nBackUpType_Opt1 ); }

	//	バックアップ数
	int		GetBackupCount(void) const			{ return m_nBackUpType_Opt2 & 0xffff; }
	void	SetBackupCount(int value)			{ m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xffff0000) | ( value & 0xffff ); }

	//	バックアップの拡張子先頭文字(1文字)
	int		GetBackupExtChar(void) const		{ return ( m_nBackUpType_Opt2 >> 16 ) & 0xff ; }
	void	SetBackupExtChar(int value)			{ m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xff00ffff) | (( value & 0xff ) << 16 ); }

	//	Aug. 21, 2000 genta
	//	自動Backup
	bool	IsAutoBackupEnabled(void) const		{ return GetBackupOpt( BKUP_AUTO ); }
	void	EnableAutoBackup(bool flag)			{ SetBackupOpt( BKUP_AUTO, flag ); }

	int		GetAutoBackupInterval(void) const	{ return m_nBackUpType_Opt3; }
	void	SetAutoBackupInterval(int i)		{ m_nBackUpType_Opt3 = i; }

	//	Backup詳細設定のアクセス関数
	int		GetBackupTypeAdv(void) const { return m_nBackUpType_Opt4; }
	void	SetBackupTypeAdv(int n){ m_nBackUpType_Opt4 = n; }

public:
	bool		m_bBackUp;					// 保存時にバックアップを作成する
	bool		m_bBackUpDialog;			// バックアップの作成前に確認
	bool		m_bBackUpFolder;			// 指定フォルダにバックアップを作成する
	SFilePath	m_szBackUpFolder;			// バックアップを作成するフォルダ
	int 		m_nBackUpType;				// バックアップファイル名のタイプ 1=(.bak) 2=*_日付.*
	int 		m_nBackUpType_Opt1;			// バックアップファイル名：オプション1
	int 		m_nBackUpType_Opt2;			// バックアップファイル名：オプション2
	int 		m_nBackUpType_Opt3;			// バックアップファイル名：オプション3
	int 		m_nBackUpType_Opt4;			// バックアップファイル名：オプション4
	int 		m_nBackUpType_Opt5;			// バックアップファイル名：オプション5
	int 		m_nBackUpType_Opt6;			// バックアップファイル名：オプション6
	bool		m_bBackUpDustBox;			// バックアップファイルをごみ箱に放り込む	//@@@ 2001.12.11 add MIK
	bool		m_bBackUpPathAdvanced;		// バックアップ先フォルダを詳細設定する 20051107 aroka
	SFilePath	m_szBackUpPathAdvanced;		// バックアップを作成するフォルダの詳細設定 20051107 aroka
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           書式                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Format
{
	//日付書式
	int			m_nDateFormatType;							//日付書式のタイプ
	TCHAR		m_szDateFormat[MAX_DATETIMEFOREMAT_LEN];	//日付書式

	//時刻書式
	int			m_nTimeFormatType;							//時刻書式のタイプ
	TCHAR		m_szTimeFormat[MAX_DATETIMEFOREMAT_LEN];	//時刻書式

	//見出し記号
	wchar_t		m_szMidashiKigou[256];

	//引用符
	wchar_t		m_szInyouKigou[32];
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     カスタムメニュー                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_CustomMenu
{
	WCHAR			m_szCustMenuNameArr   [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_NAME_LEN + 1];
	int				m_nCustMenuItemNumArr [MAX_CUSTOM_MENU];
	EFunctionCode	m_nCustMenuItemFuncArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	KEYCODE			m_nCustMenuItemKeyArr [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           支援                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Helper
{
	//入力補完機能
	BOOL		m_bHokanKey_RETURN;				// VK_RETURN	補完決定キーが有効/無効
	BOOL		m_bHokanKey_TAB;				// VK_TAB		補完決定キーが有効/無効
	BOOL		m_bHokanKey_RIGHT;				// VK_RIGHT		補完決定キーが有効/無効
	BOOL		m_bHokanKey_SPACE;				// VK_SPACE		補完決定キーが有効/無効 $$$ほぼ未使用

	//外部ヘルプの設定
	TCHAR		m_szExtHelp[_MAX_PATH];

	//外部HTMLヘルプの設定
	TCHAR		m_szExtHtmlHelp[_MAX_PATH];
	BOOL		m_bHtmlHelpIsSingle;			// HtmlHelpビューアはひとつ (ビューアを複数起動しない)

	//migemo設定
	TCHAR		m_szMigemoDll[_MAX_PATH];		// migemo dll
	TCHAR		m_szMigemoDict[_MAX_PATH];		// migemo dict

	//キーワードヘルプ
	LOGFONT		m_lf_kh;						// キーワードヘルプのフォント情報		// ai 02/05/21 Add

	//INI内設定のみ
	int			m_bUseHokan;					// 入力補完機能を使用する
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           検索                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Search
{
	SSearchOption	m_sSearchOption;			// 検索／置換  条件

	int				m_bConsecutiveAll;			// 「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
	int				m_bNOTIFYNOTFOUND;			// 検索／置換  見つからないときメッセージを表示
	int				m_bSelectedArea;			// 置換  選択範囲内置換

	int				m_bGrepSubFolder;			// Grep: サブフォルダも検索
	BOOL			m_bGrepOutputLine;			// Grep: 行を出力するか該当部分だけ出力するか
	int				m_nGrepOutputStyle;			// Grep: 出力形式
	int				m_bGrepDefaultFolder;		// Grep: フォルダの初期値をカレントフォルダにする
	ECodeType		m_nGrepCharSet;				// Grep: 文字コードセット // 2002/09/20 Moca Add

	//Grep - Grep
	BOOL			m_bCaretTextForSearch;		// カーソル位置の文字列をデフォルトの検索文字列にする 2006.08.23 ryoji
	TCHAR			m_szRegexpLib[_MAX_PATH];	// 使用する正規表現DLL  2007.08.22 genta

	BOOL			m_bGrepExitConfirm;			// Grepモードで保存確認するか
	int				m_bGrepRealTimeView;		// Grep結果のリアルタイム表示 2003.06.16 Moca

	BOOL			m_bGTJW_RETURN;				// エンターキーでタグジャンプ
	BOOL			m_bGTJW_LDBLCLK;			// ダブルクリックでタグジャンプ

	//検索・置換ダイアログ
	BOOL			m_bAutoCloseDlgFind;		// 検索ダイアログを自動的に閉じる
	BOOL			m_bAutoCloseDlgReplace;		// 置換 ダイアログを自動的に閉じる
	BOOL			m_bSearchAll;				// 先頭（末尾）から再検索 2002.01.26 hor

	//INI内設定のみ
	BOOL			m_bUseCaretKeyWord;			// キャレット位置の単語を辞書検索		// 2006.03.24 fon
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         タブバー                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_TabBar
{
	BOOL		m_bDispTabWnd;					//タブウインドウ表示する	//@@@ 2003.05.31 MIK
	BOOL		m_bDispTabWndMultiWin;			//タブをまとめない	//@@@ 2003.05.31 MIK
	BOOL		m_bTab_RetainEmptyWin;			//!< 最後の文書が閉じられたとき(無題)を残す
	BOOL		m_bTab_CloseOneWin;				//!< タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる

	wchar_t		m_szTabWndCaption[MAX_CAPTION_CONF_LEN];	//タブウインドウキャプション	//@@@ 2003.06.13 MIK
	BOOL		m_bSameTabWidth;				//タブを等幅にする			//@@@ 2006.01.28 ryoji
	BOOL		m_bDispTabIcon;					//タブにアイコンを表示する	//@@@ 2006.01.28 ryoji
	BOOL		m_bSortTabList;					//タブ一覧をソートする	//@@@ 2006.03.23 fon
	BOOL		m_bTab_ListFull;				//タブ一覧をフルパス表示する	//@@@ 2007.02.28 ryoji

	BOOL		m_bChgWndByWheel;				//マウスホイールでウィンドウ切り替え	//@@@ 2006.03.26 ryoji
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ツールバー                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct CommonSetting_ToolBar
{
	int			m_nToolBarButtonNum;			// ツールバーボタンの数
	int			m_nToolBarButtonIdxArr[MAX_TOOLBARBUTTONS];	// ツールバーボタン構造体
	int			m_bToolBarIsFlat;				// フラットツールバーにする／しない
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       アウトライン                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_OutLine
{
	// 20060201 aroka アウトライン/トピックリスト の位置とサイズを記憶
	int			m_bRememberOutlineWindowPos;
	int			m_widthOutlineWindow;
	int			m_heightOutlineWindow;
	int			m_xOutlineWindowPos;
	int			m_yOutlineWindowPos;

	//IDD_FUNCLIST (ツール - アウトライン解析)
	BOOL		m_bAutoCloseDlgFuncList;	// アウトラインダイアログを自動的に閉じる
	BOOL		m_bFunclistSetFocusOnJump;	// フォーカスを移す 2002.02.08 hor
	BOOL		m_bMarkUpBlankLineEnable;	// 空行を無視する 2002.02.08 aroka,hor
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ファイル内容比較                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Compare
{
	//ファイル内容比較ダイアログ
	BOOL		m_bCompareAndTileHorz;		// 文書比較後、左右に並べて表示
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ビュー                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_View
{
	//INI内設定のみ
	LOGFONT		m_lf;						// 現在のフォント情報
	BOOL		m_bFontIs_FIXED_PITCH;		// 現在のフォントは固定幅フォントである
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Others
{
	//INI内設定のみ
	RECT		m_rcOpenDialog;				// 「開く」ダイアログのサイズと位置
};



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                                                             //
//                          まとめ                             //
//                                                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 共通設定
struct CommonSetting
{
	CommonSetting_General		m_sGeneral;
	CommonSetting_Window		m_sWindow;
	CommonSetting_Edit			m_sEdit;
	CommonSetting_File			m_sFile;
	CommonSetting_Backup		m_sBackup;
	CommonSetting_Format		m_sFormat;
	CommonSetting_CustomMenu	m_sCustomMenu;
	CommonSetting_Helper		m_sHelper;
	CommonSetting_Search		m_sSearch;
	CommonSetting_TabBar		m_sTabBar;
	CommonSetting_ToolBar		m_sToolBar;
	CommonSetting_OutLine		m_sOutline;
	CommonSetting_Compare		m_sCompare;
	CommonSetting_View			m_sView;
	CommonSetting_Others		m_sOthers;
};


