/*! @file */
//2007.09.28 kobake Common整理
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_COMMONSETTING_2CBDBC26_EA7F_466E_842F_AFB0D93207D2_H_
#define SAKURA_COMMONSETTING_2CBDBC26_EA7F_466E_842F_AFB0D93207D2_H_
#pragma once

#include "CKeyWordSetMgr.h"
#include "func/CKeyBind.h"
#include "func/CFuncLookup.h" //MacroRec
#include "io/CFile.h" //EShareMode
#include "basis/SakuraBasis.h"

// Apr. 05, 2003 genta WindowCaption用領域（変換前）の長さ
static const int MAX_CAPTION_CONF_LEN = 256;

static const int MAX_DATETIMEFOREMAT_LEN	= 100;
static const int MAX_CUSTOM_MENU			=  25;
static const int MAX_CUSTOM_MENU_NAME_LEN	=  32;
static const int MAX_CUSTOM_MENU_ITEMS		=  48;
static const int MAX_TOOLBAR_BUTTON_ITEMS	= 512;	//ツールバーに登録可能なボタン最大数	
static const int MAX_TOOLBAR_ICON_X			=  32;	//アイコンBMPの桁数
static const int MAX_TOOLBAR_ICON_Y			=  15;	//アイコンBMPの段数
static const int MAX_TOOLBAR_ICON_COUNT		= MAX_TOOLBAR_ICON_X * MAX_TOOLBAR_ICON_Y; // =480
//Oct. 22, 2000 JEPRO アイコンの最大登録数を128個増やした(256→384)	
//2010/3/14 Uchi アイコンの最大登録数を32個増やした(384→416)
//2010/6/26 syat アイコンの最大登録数を15段に増やした(416→480)

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
	int		m_nCaretType = 0;							//!< カーソルのタイプ 0=win 1=dos 
	bool	m_bIsINSMode = true;							//!< 挿入／上書きモード
	bool	m_bIsFreeCursorMode = false;					//!< フリーカーソルモードか
	BOOL	m_bStopsBothEndsWhenSearchWord = FALSE;			//!< 単語単位で移動するときに、単語の両端で止まるか
	BOOL	m_bStopsBothEndsWhenSearchParagraph = FALSE;	//!< 段落単位で移動するときに、段落の両端で止まるか
	BOOL	m_bNoCaretMoveByActivation = FALSE;				//!< マウスクリックにてアクティベートされた時はカーソル位置を移動しない  2007.10.02 nasukoji (add by genta)

	//スクロール
	CLayoutInt	m_nRepeatedScrollLineNum = CLayoutInt(3);		//!< キーリピート時のスクロール行数
	int		m_nRepeatedMoveCaretNum = 2;				//!< キーリピート時の左右移動数
	BOOL	m_nRepeatedScroll_Smooth = FALSE;			//!< キーリピート時のスクロールを滑らかにするか
	int		m_nPageScrollByWheel = 0;					//!< キー/マウスボタン + ホイールスクロールでページUP/DOWNする	// 2009.01.17 nasukoji
	int		m_nHorizontalScrollByWheel = 0;				//!< キー/マウスボタン + ホイールスクロールで横スクロールする	// 2009.01.17 nasukoji

	//タスクトレイ
	BOOL	m_bUseTaskTray = TRUE;										//!< タスクトレイのアイコンを使う
	BOOL	m_bStayTaskTray = FALSE;									//!< タスクトレイのアイコンを常駐
	WORD	m_wTrayMenuHotKeyCode = L'Z';								//!< タスクトレイ左クリックメニュー キー
	WORD	m_wTrayMenuHotKeyMods = HOTKEYF_ALT | HOTKEYF_CONTROL;		//!< タスクトレイ左クリックメニュー キー

	//履歴
	int		m_nMRUArrNum_MAX = 15;				//!< ファイルの履歴MAX
	int		m_nOPENFOLDERArrNum_MAX = 15;		//!< フォルダーの履歴MAX

	//ノーカテゴリ
	BOOL	m_bCloseAllConfirm = FALSE;				//!< [すべて閉じる]で他に編集用のウィンドウがあれば確認する	// 2006.12.25 ryoji
	BOOL	m_bExitConfirm = FALSE;					//!< 終了時の確認をする

	//INI内設定のみ
	BOOL	m_bDispExitingDialog = FALSE;			//!< 終了ダイアログを表示する
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ウィンドウ                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	2004.05.13 Moca
//! ウィンドウサイズ・位置の制御方法
enum EWinSizeMode{
	WINSIZEMODE_DEF		= 0, //!< 指定なし
	WINSIZEMODE_SAVE	= 1, //!< 継承(保存)
	WINSIZEMODE_SET		= 2  //!< 直接指定(固定)
};

struct CommonSetting_Window
{
	//基本設定
	BOOL			m_bDispTOOLBAR = TRUE;					//!< 次回ウィンドウを開いたときツールバーを表示する
	BOOL			m_bDispSTATUSBAR = TRUE;				//!< 次回ウィンドウを開いたときステータスバーを表示する
	BOOL			m_bDispFUNCKEYWND = FALSE;				//!< 次回ウィンドウを開いたときファンクションキーを表示する
	bool			m_bDispMiniMap = FALSE;					//!< ミニマップを表示する
	BOOL			m_bMenuIcon = TRUE;						//!< メニューにアイコンを表示する (アイコン付きメニュー)
	BOOL			m_bScrollBarHorz = TRUE;				//!< 水平スクロールバーを使う
	BOOL			m_bUseCompatibleBMP = TRUE;				//!< 再作画用互換ビットマップを使う

	//位置と大きさの設定
	EWinSizeMode	m_eSaveWindowSize = WINSIZEMODE_SAVE;	//!< ウィンドウサイズ継承・固定 EWinSizeModeに順ずる
	int				m_nWinSizeType = SIZE_RESTORED;			//!< 大きさの指定
	int				m_nWinSizeCX = CW_USEDEFAULT;			//!< 直接指定 幅
	int				m_nWinSizeCY = 0;						//!< 直接指定 高さ
	EWinSizeMode	m_eSaveWindowPos = WINSIZEMODE_DEF;		//!< ウィンドウ位置継承・固定 EWinSizeModeに順ずる
	int				m_nWinPosX = CW_USEDEFAULT;				//!< 直接指定 X座標
	int				m_nWinPosY = 0;							//!< 直接指定 Y座標

	//ファンクションキー
	int				m_nFUNCKEYWND_Place = 1;				//!< ファンクションキー表示位置／0:上 1:下
	int				m_nFUNCKEYWND_GroupNum = 4;				//!< ファンクションキーのグループボタン数

	//ルーラー・行番号
	int				m_nRulerHeight = 13;					//!< ルーラー高さ
	int				m_nRulerBottomSpace = 0;				//!< ルーラーとテキストの隙間
	int				m_nLineNumRightSpace = 0;				//!< 行番号の右のスペース Sep. 18, 2002 genta

	//分割ウィンドウ
	BOOL			m_bSplitterWndHScroll = TRUE;			//!< 分割ウィンドウの水平スクロールの同期をとる
	BOOL			m_bSplitterWndVScroll = TRUE;			//!< 分割ウィンドウの垂直スクロールの同期をとる

	//タイトルバー
	WCHAR			m_szWindowCaptionActive  [MAX_CAPTION_CONF_LEN];	//!< タイトルバー(アクティブ時)
	WCHAR			m_szWindowCaptionInactive[MAX_CAPTION_CONF_LEN];	//!< タイトルバー(非アクティブ時)

	//INI内設定のみ
	int				m_nVertLineOffset = -1;			//!< 縦線の描画座標オフセット 2005.11.10 Moca

	//言語選択
	WCHAR			m_szLanguageDll[MAX_PATH];	//!< 言語DLLファイル名

	//ミニマップ
	int				m_nMiniMapFontSize = -2;
	int				m_nMiniMapQuality = NONANTIALIASED_QUALITY;
	int				m_nMiniMapWidth = 150;

	CommonSetting_Window() noexcept;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         タブバー                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//! 閉じるボタン
enum EDispTabClose{
	DISPTABCLOSE_NO			= 0, //!< なし
	DISPTABCLOSE_ALLWAYS	= 1, //!< 常に表示
	DISPTABCLOSE_AUTO		= 2  //!< 自動表示
};

enum ETabPosition{
	TabPosition_Top,
	TabPosition_Bottom,
	TabPosition_Left,
	TabPosition_Right,
	TabPosition_None = -1,
};

struct CommonSetting_TabBar
{
	BOOL			m_bDispTabWnd = FALSE;						//!< タブウインドウ表示する
	BOOL			m_bDispTabWndMultiWin = FALSE;				//!< タブをまとめない
	BOOL			m_bTab_RetainEmptyWin = TRUE;				//!< 最後の文書が閉じられたとき(無題)を残す
	BOOL			m_bTab_CloseOneWin = FALSE;					//!< タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる
	BOOL			m_bNewWindow = FALSE;						//!< 外部から起動するときは新しいウインドウで開く
	bool			m_bTabMultiLine = false;					//!< タブ多段
	ETabPosition	m_eTabPosition = TabPosition_Top;			//!< タブ位置

	wchar_t			m_szTabWndCaption[MAX_CAPTION_CONF_LEN];	//!< タブウインドウキャプション
	BOOL			m_bSameTabWidth = FALSE;					//!< タブを等幅にする
	BOOL			m_bDispTabIcon = FALSE;						//!< タブにアイコンを表示する
	EDispTabClose	m_bDispTabClose = DISPTABCLOSE_NO;			//!< タブに閉じるボタンを表示する
	BOOL			m_bSortTabList = TRUE;						//!< タブ一覧をソートする
	BOOL			m_bTab_ListFull = FALSE;					//!< タブ一覧をフルパス表示する

	BOOL			m_bChgWndByWheel = FALSE;					//!< マウスホイールでウィンドウ切り替え

	LOGFONT			m_lf{};										//!< タブフォント
	INT				m_nPointSize = 10;							//!< フォントサイズ（1/10ポイント単位）
	int				m_nTabMaxWidth = 200;						//!< タブ幅の最大値
	int				m_nTabMinWidth = 60;						//!< タブ幅の最小値
	int				m_nTabMinWidthOnMulti = 100;				//!< タブ幅の最小値(タブ多段時)

	CommonSetting_TabBar() noexcept;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           編集                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//! ファイルダイアログの初期位置
enum EOpenDialogDir{
	OPENDIALOGDIR_CUR, //!< カレントフォルダー
	OPENDIALOGDIR_MRU, //!< 最近使ったフォルダー
	OPENDIALOGDIR_SEL, //!< 指定フォルダー
};

struct CommonSetting_Edit
{
	//コピー
	bool	m_bAddCRLFWhenCopy = false;			//!< 折り返し行に改行を付けてコピー
	BOOL	m_bEnableNoSelectCopy = TRUE;		//!< 選択なしでコピーを可能にする 2007.11.18 ryoji
	BOOL	m_bCopyAndDisablSelection = FALSE;	//!< コピーしたら選択解除
	bool	m_bEnableLineModePaste = true;		//!< ラインモード貼り付けを可能にする  2007.10.08 ryoji
	bool	m_bConvertEOLPaste = false;			//!< 改行コードを変換して貼り付ける  2009.2.28 salarm

	//ドラッグ＆ドロップ
	BOOL	m_bUseOLE_DragDrop = TRUE;			//!< OLEによるドラッグ & ドロップを使う
	BOOL	m_bUseOLE_DropSource = TRUE;		//!< OLEによるドラッグ元にするか

	//上書きモード
	BOOL	m_bNotOverWriteCRLF = TRUE;		//!< 改行は上書きしない
	bool	m_bOverWriteFixMode = false;		//!< 文字幅に合わせてスペースを詰める
	bool	m_bOverWriteBoxDelete = false;		//!< 上書きモードでの矩形入力で選択範囲を削除する

	//クリッカブルURL
	BOOL	m_bSelectClickedURL = TRUE;		//!< URLがクリックされたら選択するか

	EOpenDialogDir	m_eOpenDialogDir = OPENDIALOGDIR_CUR;	//!< ファイルダイアログの初期位置
	SFilePath	m_OpenDialogSelDir = LR"(%Personal%\)";		//!< 指定フォルダー

	bool	m_bEnableExtEol = false;			//!< NEL,PS,LSを改行コードとして利用する
	bool	m_bBoxSelectLock = true;			//!< (矩形選択)移動でロックする

	bool	m_bVistaStyleFileDialog = true;	//!< Vistaスタイルのファイルダイアログ

	// (ダイアログ項目無し)
	BOOL	m_bAutoColumnPaste = TRUE;			//!< 矩形コピーのテキストは常に矩形貼り付け
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ファイル                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_File {
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

	//ファイルの排他制御
	EShareMode		m_nFileShareMode = SHAREMODE_NOT_EXCLUSIVE;		//!< ファイルの排他制御モード
	bool			m_bCheckFileTimeStamp = true;					//!< 更新の監視
	int 			m_nAutoloadDelay = 0;							//!< 自動読込時遅延
	bool			m_bUneditableIfUnwritable = true;				//!< 上書き禁止検出時は編集禁止にする

	//ファイルの保存
	bool			m_bEnableUnmodifiedOverwrite = false;			//!< 無変更でも上書きするか

	//「名前を付けて保存」でファイルの種類が[ユーザー指定]のときのファイル一覧表示
	//ファイル保存ダイアログのフィルタ設定	// 2006.11.16 ryoji
	bool			m_bNoFilterSaveNew = true;						//!< 新規から保存時は全ファイル表示
	bool			m_bNoFilterSaveFile = true;						//!< 新規以外から保存時は全ファイル表示

	//ファイルオープン
	bool			m_bDropFileAndClose = false;					//!< ファイルをドロップしたときは閉じて開く
	int				m_nDropFileNumMax = 8;							//!< 一度にドロップ可能なファイル数
	bool			m_bRestoreCurPosition = true;					//!< ファイルを開いたときカーソル位置を復元するか
	bool			m_bRestoreBookmarks = true;						//!< ブックマークを復元するかどうか 2002.01.16 hor
	bool			m_bAutoMIMEdecode = false;						//!< ファイル読み込み時にMIMEのdecodeを行うか
	bool			m_bQueryIfCodeChange = true;					//!< 前回と文字コードが異なるときに問い合わせを行う Oct. 03, 2004 genta
	bool			m_bAlertIfFileNotExist = false;					//!< 開こうとしたファイルが存在しないとき警告する Oct. 09, 2004 genta
	bool			m_bAlertIfLargeFile = false;					//!< 開こうとしたファイルサイズが大きい場合に警告する
	int				m_nAlertFileSize = 10;							//!< 警告を始めるファイルサイズ(MB)
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       バックアップ                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//	Aug. 15, 2000 genta
//! Backup Flags
enum EBackupOptionFlag{
	BKUP_YEAR		= 32,
	BKUP_MONTH		= 16,
	BKUP_DAY		= 8,
	BKUP_HOUR		= 4,
	BKUP_MIN		= 2,
	BKUP_SEC		= 1,

	//	Aug. 21, 2000 genta
	BKUP_AUTO		= 64,
};

struct CommonSetting_Backup
{
	//	Aug. 15, 2000 genta
	//	Backup設定のアクセス関数
	int		GetBackupType(void) const			{ return m_nBackUpType; }
	void	SetBackupType(int n)				{ m_nBackUpType = n; }

	bool	GetBackupOpt(EBackupOptionFlag flag) const			{ return ( flag & m_nBackUpType_Opt1 ) == flag; }
	void	SetBackupOpt(EBackupOptionFlag flag, bool value)	{ m_nBackUpType_Opt1 = value ? ( flag | m_nBackUpType_Opt1) :  ((~flag) & m_nBackUpType_Opt1 ); }

	//	バックアップ数
	int		GetBackupCount(void) const			{ return m_nBackUpType_Opt2 & 0xffff; }
	void	SetBackupCount(int value)			{ m_nBackUpType_Opt2 = (m_nBackUpType_Opt2 & 0xffff0000) | ( value & 0xffff ); }

	//	バックアップの拡張子先頭文字(1文字)
	WCHAR	GetBackupExtChar(void) const		{ return (WCHAR)(( m_nBackUpType_Opt2 >> 16 ) & 0xff) ; }
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

	bool		m_bBackUp = false;					//!< 保存時にバックアップを作成する
	bool		m_bBackUpDialog = true;				//!< バックアップの作成前に確認
	bool		m_bBackUpFolder = false;			//!< 指定フォルダーにバックアップを作成する
	bool		m_bBackUpFolderRM = false;			//!< 指定フォルダーにバックアップを作成する(リムーバブルメディアのみ)
	SFilePath	m_szBackUpFolder;					//!< バックアップを作成するフォルダー
	int 		m_nBackUpType = 2;					//!< バックアップファイル名のタイプ 1=(.bak) 2=*_日付.*
	int 		m_nBackUpType_Opt1 = BKUP_YEAR | BKUP_MONTH | BKUP_DAY;			//!< バックアップファイル名：オプション1
	int 		m_nBackUpType_Opt2 = (L'b' << 16) + 10;			//!< バックアップファイル名：オプション2
	int 		m_nBackUpType_Opt3 = 5;				//!< バックアップファイル名：オプション3
	int 		m_nBackUpType_Opt4 = 0;				//!< バックアップファイル名：オプション4
	int 		m_nBackUpType_Opt5 = 0;				//!< バックアップファイル名：オプション5
	int 		m_nBackUpType_Opt6 = 0;				//!< バックアップファイル名：オプション6
	bool		m_bBackUpDustBox = false;			//!< バックアップファイルをごみ箱に放り込む	//@@@ 2001.12.11 add MIK
	bool		m_bBackUpPathAdvanced = false;		//!< バックアップ先フォルダーを詳細設定する 20051107 aroka
	SFilePath	m_szBackUpPathAdvanced;				//!< バックアップを作成するフォルダーの詳細設定 20051107 aroka
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           書式                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Format
{
	//日付書式
	int			m_nDateFormatType = 0;							//!< 日付書式のタイプ
	WCHAR		m_szDateFormat[MAX_DATETIMEFOREMAT_LEN];	//!< 日付書式

	//時刻書式
	int			m_nTimeFormatType = 0;							//!< 時刻書式のタイプ
	WCHAR		m_szTimeFormat[MAX_DATETIMEFOREMAT_LEN];	//!< 時刻書式

	//見出し記号
	wchar_t		m_szMidashiKigou[256];						//!< 見出し記号

	//引用符
	wchar_t		m_szInyouKigou[32];							//!< 引用符

	CommonSetting_Format() noexcept;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           検索                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Search
{
	int				m_nSearchKeySequence = 0;			//!< 検索シーケンス(未保存)
	SSearchOption	m_sSearchOption{};					//!< 検索／置換  条件

	int				m_nReplaceKeySequence = 0;			//!< 置換後シーケンス(未保存)
	int				m_bConsecutiveAll = 0;				//!< 「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
	int				m_bNOTIFYNOTFOUND = TRUE;			//!< 検索／置換  見つからないときメッセージを表示
	int				m_bSelectedArea = FALSE;			//!< 置換  選択範囲内置換

	int				m_bGrepSubFolder = TRUE;			//!< Grep: サブフォルダーも検索
	int				m_nGrepOutputLineType = 1;			//!< Grep: 行を出力/該当部分/否マッチ行 を出力
	int				m_nGrepOutputStyle = 1;				//!< Grep: 出力形式
	int				m_bGrepDefaultFolder = FALSE;		//!< Grep: フォルダーの初期値をカレントフォルダーにする
	ECodeType		m_nGrepCharSet = CODE_AUTODETECT;				//!< Grep: 文字コードセット // 2002/09/20 Moca Add
	bool			m_bGrepOutputFileOnly = false;		//!< Grep: ファイル毎最初のみ検索
	bool			m_bGrepOutputBaseFolder = false;	//!< Grep: ベースフォルダー表示
	bool			m_bGrepSeparateFolder = false;		//!< Grep: フォルダー毎に表示
	bool			m_bGrepBackup = true;				//!< Grep: バックアップ作成

	BOOL			m_bCaretTextForSearch = TRUE;		//!< カーソル位置の文字列をデフォルトの検索文字列にする 2006.08.23 ryoji
	bool			m_bInheritKeyOtherView = true;		//!< 次・前検索で他のビューの検索条件を引き継ぐ
	WCHAR			m_szRegexpLib[_MAX_PATH]{};			//!< 使用する正規表現DLL  2007.08.22 genta

	//Grep
	BOOL			m_bGrepExitConfirm = FALSE;			//!< Grepモードで保存確認するか
	BOOL			m_bGrepRealTimeView = FALSE;		//!< Grep結果のリアルタイム表示 2003.06.16 Moca

	BOOL			m_bGTJW_RETURN = TRUE;				//!< エンターキーでタグジャンプ
	BOOL			m_bGTJW_LDBLCLK = TRUE;				//!< ダブルクリックでタグジャンプ

	//検索・置換ダイアログ
	BOOL			m_bAutoCloseDlgFind = TRUE;			//!< 検索ダイアログを自動的に閉じる
	BOOL			m_bAutoCloseDlgReplace = TRUE;		//!< 置換 ダイアログを自動的に閉じる
	BOOL			m_bSearchAll = FALSE;				//!< 先頭（末尾）から再検索 2002.01.26 hor

	int				m_nTagJumpMode = 1;					//!< タグジャンプモード(0-3)
	int				m_nTagJumpModeKeyword = 3;			//!< タグジャンプモード(0-3)

	//INI内設定のみ
	BOOL			m_bUseCaretKeyWord = FALSE;			//!< キャレット位置の単語を辞書検索		// 2006.03.24 fon
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       キー割り当て                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_KeyBind
{
	// キー割り当て
	int					m_nKeyNameArrNum = 100;		//!< キー割り当て表の有効データ数
	KEYDATA				m_pKeyNameArr[100+1];		//!< キー割り当て表 未割り当てキーコード用にダミーを追加
	BYTE				m_VKeyToKeyNameArr[256+10];	//!< キーコード→割り当て表インデックス // 2012.11.25 aroka

	CommonSetting_KeyBind() noexcept;
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
	bool			m_bCustMenuPopupArr   [MAX_CUSTOM_MENU];

	CommonSetting_CustomMenu() noexcept;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ツールバー                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_ToolBar
{
	int			m_nToolBarButtonNum = 25;							//!< ツールバーボタンの数
	int			m_nToolBarButtonIdxArr[MAX_TOOLBAR_BUTTON_ITEMS];	//!< ツールバーボタン構造体
	bool		m_bToolBarIsFlat = false;							//!< フラットツールバーにする／しない

	CommonSetting_ToolBar() noexcept;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      強調キーワード                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_SpecialKeyword
{
	// 強調キーワード設定
	CKeyWordSetMgr		m_CKeyWordSetMgr;					//!< 強調キーワード
	char				m_szKeyWordSetDir[MAX_PATH];		//!< 強調キーワードファイルのディレクトリ

	CommonSetting_SpecialKeyword() noexcept;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           支援                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Helper
{
	//入力補完機能
	BOOL		m_bHokanKey_RETURN = TRUE;				//!< VK_RETURN	補完決定キーが有効/無効
	BOOL		m_bHokanKey_TAB = FALSE;				//!< VK_TAB		補完決定キーが有効/無効
	BOOL		m_bHokanKey_RIGHT = TRUE;				//!< VK_RIGHT	補完決定キーが有効/無効

	//外部ヘルプの設定
	WCHAR		m_szExtHelp[_MAX_PATH]{};				//!< 外部ヘルプ１

	//外部HTMLヘルプの設定
	WCHAR		m_szExtHtmlHelp[_MAX_PATH]{};			//!< 外部HTMLヘルプ
	bool		m_bHtmlHelpIsSingle = true;				//!< HtmlHelpビューアはひとつ (ビューアを複数起動しない)

	//migemo設定
	WCHAR		m_szMigemoDll[_MAX_PATH]{};				//!< migemo dll
	WCHAR		m_szMigemoDict[_MAX_PATH]{};			//!< migemo dict

	//キーワードヘルプ
	LOGFONT		m_lf{};									//!< キーワードヘルプのフォント情報		// ai 02/05/21 Add
	INT			m_nPointSize = 10;						//!< キーワードヘルプのフォントサイズ（1/10ポイント単位）	// 2009.10.01 ryoji

	CommonSetting_Helper() noexcept;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          マクロ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Macro
{
	WCHAR			m_szKeyMacroFileName[MAX_PATH]{};	//!< キーボードマクロのファイル名
	MacroRec		m_MacroTable[MAX_CUSTMACRO]{};		//!< キー割り当て用マクロテーブル		Sep. 14, 2001 genta
	SFilePath		m_szMACROFOLDER;					//!< マクロ用フォルダー
	int				m_nMacroOnOpened = -1;				//!< オープン後自動実行マクロ番号	@@@ 2006.09.01 ryoji
	int				m_nMacroOnTypeChanged = -1;			//!< タイプ変更後自動実行マクロ番号	@@@ 2006.09.01 ryoji
	int				m_nMacroOnSave = -1;				//!< 保存前自動実行マクロ番号	@@@ 2006.09.01 ryoji
	int				m_nMacroCancelTimer = 10;			//!< マクロ停止ダイアログ表示待ち時間	@@@ 2011.08.04 syat

	explicit CommonSetting_Macro(
		const std::filesystem::path& iniFolder
	) noexcept;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ファイル名表示                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_FileName
{
	bool		m_bTransformShortPath = true;									//!< ファイル名の省略表記
	int			m_nTransformShortMaxWidth = 100;								//!< ファイル名の省略表記の最大長
	int			m_nTransformFileNameArrNum = 7;									//!< ファイル名の簡易表示登録数
	WCHAR		m_szTransformFileNameFrom[MAX_TRANSFORM_FILENAME][_MAX_PATH]{};	//!< ファイル名の簡易表示変換前文字列
	WCHAR		m_szTransformFileNameTo[MAX_TRANSFORM_FILENAME][_MAX_PATH]{};	//!< ファイル名の簡易表示変換後文字列	//@@@ 2003.04.08 MIK

	CommonSetting_FileName() noexcept;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       アウトライン                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ドッキング配置
enum EDockSide{
	DOCKSIDE_FLOAT,				//!< フローティング
	DOCKSIDE_LEFT,				//!< 左ドッキング
	DOCKSIDE_TOP,				//!< 上ドッキング
	DOCKSIDE_RIGHT,				//!< 右ドッキング
	DOCKSIDE_BOTTOM,			//!< 下ドッキング
	DOCKSIDE_UNDOCKABLE = -1,	//!< ドッキング禁止
};

enum EFileTreeItemType{
	EFileTreeItemType_Grep,
	EFileTreeItemType_File,
	EFileTreeItemType_Folder
};

struct SFileTreeItem{
public:
	EFileTreeItemType m_eFileTreeItemType = EFileTreeItemType_Grep;
	SFilePath		m_szTargetPath;					//!< フォルダーorファイルパス
	SFilePath		m_szLabelName;					//!< ラベル名(""のときはファイル名を使う)
	int				m_nDepth = 0;					//!< 階層

	// GrepタイプTreeItem
	SFilePath		m_szTargetFile;					//!< ファイル一覧
	bool			m_bIgnoreHidden = true;			//!< 隠しファイルを除く
	bool			m_bIgnoreReadOnly = false;		//!< 読み取り専用ファイルを除く
	bool			m_bIgnoreSystem = false;		//!< システムファイルを除く
};

struct SFileTree{
	bool			m_bProject = true;		//!< プロジェクトファイルモード
	SFilePath		m_szProjectIni;			//!< デフォルトiniパス
	int				m_nItemCount = 0;		//!< ファイルパス数
	SFileTreeItem	m_aItems[20];			//!< ツリーアイテム

	SFileTree() noexcept;
};

struct CommonSetting_OutLine
{
	// 20060201 aroka アウトライン/トピックリスト の位置とサイズを記憶
	int			m_bRememberOutlineWindowPos = 0;					//!< アウトライン/トピックリスト の位置とサイズを記憶する
	int			m_widthOutlineWindow = 0;							//!< アウトライン/トピックリスト のサイズ(幅)
	int			m_heightOutlineWindow = 0;							//!< アウトライン/トピックリスト のサイズ(高さ)
	int			m_xOutlineWindowPos = 0;							//!< アウトライン/トピックリスト の位置(X座標)
	int			m_yOutlineWindowPos = 0;							//!< アウトライン/トピックリスト の位置(Y座標)

	int			m_nOutlineDockSet = 0;								//!< アウトライン解析のドッキング位置継承方法(0:共通設定, 1:タイプ別設定)
	BOOL		m_bOutlineDockSync = TRUE;							//!< アウトライン解析のドッキング位置を同期する
	BOOL		m_bOutlineDockDisp = FALSE;							//!< アウトライン解析表示の有無
	EDockSide	m_eOutlineDockSide = DOCKSIDE_FLOAT;				//!< アウトライン解析ドッキング配置
	int			m_cxOutlineDockLeft = 0;							//!< アウトラインの左ドッキング幅
	int			m_cyOutlineDockTop = 0;								//!< アウトラインの上ドッキング高
	int			m_cxOutlineDockRight = 0;							//!< アウトラインの右ドッキング幅
	int			m_cyOutlineDockBottom = 0;							//!< アウトラインの下ドッキング高
	int			m_nDockOutline = 3;									//!< アウトラインタイプ

	//IDD_FUNCLIST (ツール - アウトライン解析)
	BOOL		m_bAutoCloseDlgFuncList = FALSE;					//!< アウトラインダイアログを自動的に閉じる
	BOOL		m_bFunclistSetFocusOnJump = FALSE;					//!< フォーカスを移す 2002.02.08 hor
	BOOL		m_bMarkUpBlankLineEnable = FALSE;					//!< 空行を無視する 2002.02.08 aroka,hor

	SFileTree	m_sFileTree;										//!< ファイルツリー設定
	SFilePath	m_sFileTreeDefIniName = L"_sakurafiletree.ini";		//!< ファイルツリー設定のデフォルトファイル名(GUIなし)
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ファイル内容比較                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Compare
{
	//ファイル内容比較ダイアログ
	BOOL		m_bCompareAndTileHorz = TRUE;		//!< 文書比較後、左右に並べて表示
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ビュー                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_View
{
	//INI内設定のみ
	LOGFONT		m_lf{};							//!< 現在のフォント情報
	BOOL		m_bFontIs_FIXED_PITCH = true;	//!< 現在のフォントは固定幅フォントである
	INT			m_nPointSize = 0;				//!< フォントサイズ（1/10ポイント単位）	// 2009.10.01 ryoji

	CommonSetting_View() noexcept;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Others
{
	//INI内設定のみ
	RECT		m_rcOpenDialog{};			//!< 「開く」ダイアログのサイズと位置
	RECT		m_rcCompareDialog{};		//!< 「ファイル比較」ダイアログボックスのサイズと位置
	RECT		m_rcDiffDialog{};			//!< 「DIFF差分表示」ダイアログボックスのサイズと位置
	RECT		m_rcFavoriteDialog{};		//!< 「履歴とお気に入りの管理」ダイアログボックスのサイズと位置
	RECT		m_rcTagJumpDialog{};		//!< 「ダイレクトタグジャンプ候補一覧」ダイアログボックスのサイズと位置
	RECT		m_rcWindowListDialog{};		//!< 「ウィンドウ一覧」ダイアログボックスのサイズと位置

	bool		m_bIniReadOnly = false;		//!< sakura.iniの読み取り専用
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ステータスバー                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	2008/6/21	Uchi
struct CommonSetting_Statusbar
{
	// 示文字コードの指定
	BOOL		m_bDispUniInSjis = FALSE;		//!< SJISで文字コード値をUnicodeで表示する
	BOOL		m_bDispUniInJis = FALSE;		//!< JISで文字コード値をUnicodeで表示する
	BOOL		m_bDispUniInEuc = FALSE;		//!< EUCで文字コード値をUnicodeで表示する
	BOOL		m_bDispUtf8Codepoint = TRUE;	//!< UTF-8をコードポイントで表示する
	BOOL		m_bDispSPCodepoint = TRUE;		//!< サロゲートペアをコードポイントで表示する
	BOOL		m_bDispSelCountByByte = FALSE;	//!< 選択文字数を文字単位ではなくバイト単位で表示する
	BOOL		m_bDispColByChar = FALSE;		//!< 現在桁をルーラー単位ではなく文字単位で表示する
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        プラグイン                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! プラグイン状態
enum EPluginState {
	PLS_NONE,			//!< プラグインテーブルに登録がない
	PLS_INSTALLED,		//!< 追加された
	PLS_UPDATED,		//!< 更新された
	PLS_STOPPED,		//!< 停止している
	PLS_LOADED,			//!< 読み込まれた
	PLS_DELETED			//!< 削除された
};

struct PluginRec
{
	WCHAR			m_szId[MAX_PLUGIN_ID];		//!< プラグインID
	WCHAR			m_szName[MAX_PLUGIN_NAME];	//!< プラグインフォルダー/設定ファイル名
	EPluginState	m_state = PLS_NONE;			//!< プラグイン状態。設定ファイルに保存せずメモリ上のみ。
	int 			m_nCmdNum = 0;					//!< プラグイン コマンドの数	// 2010/7/3 Uchi
};

struct CommonSetting_Plugin
{
	BOOL			m_bEnablePlugin = FALSE;	//!< プラグインを使用するかどうか
	PluginRec		m_PluginTable[MAX_PLUGIN];	//!< プラグインテーブル
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        メインメニュー                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//! メインメニュー種類
enum EMainMenuType {
	T_NODE,			//!< Node
	T_LEAF,			//!< 機能コマンド
	T_SEPARATOR,	//!< 区切線
	T_SPECIAL,		//!< 特殊機能コマンド
}; 

class CMainMenu {
public:
	EMainMenuType	m_nType;		//!< 種類
	EFunctionCode	m_nFunc;		//!< Function
	WCHAR			m_sKey[2];		//!< アクセスキー
	WCHAR			m_sName[MAX_MAIN_MENU_NAME_LEN+1];	//!< 名前
	int 			m_nLevel;		//!< レベル
};

struct CommonSetting_MainMenu
{
	int				m_nVersion = 0;							//!< メインメニューバージョン
	int				m_nMenuTopIdx[MAX_MAINMENU_TOP];	//!< メインメニュートップレベル
	int 			m_nMainMenuNum = 336;						//!< メインメニューデータの数
	CMainMenu		m_cMainMenuTbl[MAX_MAINMENU];		//!< メインメニューデータ
	bool 			m_bMainMenuKeyParentheses = true;			//!< アクセスキーを( )付で表示

	CommonSetting_MainMenu() noexcept;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                                                             //
//                          まとめ                             //
//                                                             //
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
	//
	CommonSetting_Statusbar			m_sStatusbar;		//!< ステータスバー		// 2008/6/21 Uchi
	CommonSetting_Plugin			m_sPlugin;			//!< プラグイン 2009/11/30 syat
	CommonSetting_MainMenu			m_sMainMenu;		//!< メインメニュー		// 2010/5/15 Uchi

	explicit CommonSetting(
		const std::filesystem::path& iniFolder
	) noexcept;
};

#endif /* SAKURA_COMMONSETTING_2CBDBC26_EA7F_466E_842F_AFB0D93207D2_H_ */
