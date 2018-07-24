/*!	@file
	@brief プロセス間共有データへのアクセス

	@author Norio Nakatani
	@date 1998/05/26  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta, MIK
	Copyright (C) 2001, jepro, genta, asa-o, MIK, YAZAKI, hor
	Copyright (C) 2002, genta, ai, Moca, MIK, YAZAKI, hor, KK, aroka
	Copyright (C) 2003, Moca, aroka, MIK, genta, wmlhq, sui
	Copyright (C) 2004, Moca, novice, genta, isearch, MIK
	Copyright (C) 2005, Moca, MIK, genta, ryoji, りんご, aroka
	Copyright (C) 2006, aroka, ryoji, genta
	Copyright (C) 2007, ryoji, genta, maru
	Copyright (C) 2008, ryoji, Uchi, nasukoji
	Copyright (C) 2009, nasukoji, ryoji
	Copyright (C) 2011, nasukoji
	Copyright (C) 2012, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CShareData_IO.h"
#include "env/CSakuraEnvironment.h"
#include "doc/CDocListener.h" // SLoadInfo
#include "_main/CControlTray.h"
#include "_main/CCommandLine.h"
#include "_main/CMutex.h"
#include "charset/CCodePage.h"
#include "debug/CRunningTimer.h"
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "util/module.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "util/os.h"
#include "CDataProfile.h"
#include "sakura_rc.h"

struct ARRHEAD {
	int		nLength;
	int		nItemNum;
};

const unsigned int uShareDataVersion = N_SHAREDATA_VERSION;

//	CShareData_new2.cppと統合
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動
CShareData::CShareData()
{
	m_hFileMap   = NULL;
	m_pShareData = NULL;
	m_pvTypeSettings = NULL;
}

/*!
	共有メモリ領域がある場合はプロセスのアドレス空間から､
	すでにマップされているファイル ビューをアンマップする。
*/
CShareData::~CShareData()
{
	if( m_pShareData ){
		/* プロセスのアドレス空間から､ すでにマップされているファイル ビューをアンマップします */
		SetDllShareData( NULL );
		::UnmapViewOfFile( m_pShareData );
		m_pShareData = NULL;
	}
	if( m_hFileMap ){
		CloseHandle( m_hFileMap );
	}
	if( m_pvTypeSettings ){
		for( int i = 0; i < (int)m_pvTypeSettings->size(); i++ ){
			delete (*m_pvTypeSettings)[i];
			(*m_pvTypeSettings)[i] = NULL;
		}
		delete m_pvTypeSettings;
		m_pvTypeSettings = NULL;
	}
}


static CMutex g_cMutexShareWork( FALSE, GSTR_MUTEX_SAKURA_SHAREWORK );
 
CMutex& CShareData::GetMutexShareWork(){
	return g_cMutexShareWork;
}

//! CShareDataクラスの初期化処理
/*!
	CShareDataクラスを利用する前に必ず呼び出すこと。

	@retval true 初期化成功
	@retval false 初期化失敗

	@note 既に存在する共有メモリのバージョンがこのエディタが使うものと
	異なる場合は致命的エラーを防ぐためにfalseを返します。CProcess::Initialize()
	でInit()に失敗するとメッセージを出してエディタの起動を中止します。

	@date 2018/06/01 仕様変更 https://github.com/sakura-editor/sakura/issues/29
*/
bool CShareData::InitShareData()
{
	MY_RUNNINGTIMER(cRunningTimer,"CShareData::InitShareData" );

	m_hwndTraceOutSource = NULL;	// 2006.06.26 ryoji

	/* ファイルマッピングオブジェクト */
	{
		std::tstring strProfileName = to_tchar(CCommandLine::getInstance()->GetProfileName());
		std::tstring strShareDataName = GSTR_SHAREDATA;
		strShareDataName += strProfileName;
		m_hFileMap = ::CreateFileMapping(
			INVALID_HANDLE_VALUE,	//	Sep. 6, 2003 wmlhq
			NULL,
			PAGE_READWRITE | SEC_COMMIT,
			0,
			sizeof( DLLSHAREDATA ),
			strShareDataName.c_str()
		);
	}
	if( NULL == m_hFileMap ){
		::MessageBox(
			NULL,
			_T("CreateFileMapping()に失敗しました"),
			_T("予期せぬエラー"),
			MB_OK | MB_APPLMODAL | MB_ICONSTOP
		);
		return false;
	}

	if( GetLastError() != ERROR_ALREADY_EXISTS ){
		/* オブジェクトが存在していなかった場合 */
		/* ファイルのビューを､ 呼び出し側プロセスのアドレス空間にマップします */
		m_pShareData = (DLLSHAREDATA*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		CreateTypeSettings();
		SetDllShareData( m_pShareData );

		// 2007.05.19 ryoji 実行ファイルフォルダ->設定ファイルフォルダに変更
		TCHAR	szIniFolder[_MAX_PATH];
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bInit = false;
		GetInidir( szIniFolder );
		AddLastChar( szIniFolder, _MAX_PATH, _T('\\') );

		m_pShareData->m_vStructureVersion = uShareDataVersion;
		m_pShareData->m_nSize = sizeof(*m_pShareData);

		// 2004.05.13 Moca リソースから製品バージョンの取得
		GetAppVersionInfo( NULL, VS_VERSION_INFO,
			&m_pShareData->m_sVersion.m_dwProductVersionMS, &m_pShareData->m_sVersion.m_dwProductVersionLS );

		m_pShareData->m_sFlags.m_bEditWndChanging = FALSE;	// 編集ウィンドウ切替中	// 2007.04.03 ryoji
		m_pShareData->m_sFlags.m_bRecordingKeyMacro = FALSE;		/* キーボードマクロの記録中 */
		m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

		m_pShareData->m_sNodes.m_nSequences = 0;					/* ウィンドウ連番 */
		m_pShareData->m_sNodes.m_nNonameSequences = 0;
		m_pShareData->m_sNodes.m_nGroupSequences = 0;			/* タブグループ連番 */	// 2007.06.20 ryoji
		m_pShareData->m_sNodes.m_nEditArrNum = 0;

		m_pShareData->m_sHandles.m_hwndTray = NULL;
		m_pShareData->m_sHandles.m_hAccel = NULL;
		m_pShareData->m_sHandles.m_hwndDebug = NULL;

		for( int i = 0; i < _countof(m_pShareData->m_dwCustColors); i++ ){
			m_pShareData->m_dwCustColors[i] = RGB( 255, 255, 255 );
		}

//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
		CMRUFile cMRU;
		cMRU.ClearAll();
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
		CMRUFolder cMRUFolder;
		cMRUFolder.ClearAll();

//	From Here Sept. 19, 2000 JEPRO コメントアウトになっていた初めのブロックを復活しその下をコメントアウト
//	MS ゴシック標準スタイル10ptに設定
//		/* LOGFONTの初期化 */
		LOGFONT lf;
		memset_raw( &lf, 0, sizeof( lf ) );
		lf.lfHeight			= DpiPointsToPixels(-10);	// 2009.10.01 ryoji 高DPI対応（ポイント数から算出）
		lf.lfWidth				= 0;
		lf.lfEscapement		= 0;
		lf.lfOrientation		= 0;
		lf.lfWeight			= 400;
		lf.lfItalic			= 0x0;
		lf.lfUnderline			= 0x0;
		lf.lfStrikeOut			= 0x0;
		lf.lfCharSet			= 0x80;
		lf.lfOutPrecision		= 0x3;
		lf.lfClipPrecision		= 0x2;
		lf.lfQuality			= 0x1;
		lf.lfPitchAndFamily	= 0x31;
		_tcscpy( lf.lfFaceName, _T("ＭＳ ゴシック") );

		// LoadShareDataでフォントが変わる可能性があるので、ここでは不要 // 2013.04.08 aroka
		//InitCharWidthCacheCommon();								// 2008/5/17 Uchi

		// キーワードヘルプのフォント ai 02/05/21 Add S
		LOGFONT lfIconTitle;	// エクスプローラのファイル名表示に使用されるフォント
		::SystemParametersInfo(
			SPI_GETICONTITLELOGFONT,				// system parameter to query or set
			sizeof(LOGFONT),						// depends on action to be taken
			(PVOID)&lfIconTitle,					// depends on action to be taken
			0										// user profile update flag
		);
		// ai 02/05/21 Add E

		INT		nIconPointSize = lfIconTitle.lfHeight >=0 ? lfIconTitle.lfHeight : DpiPixelsToPoints( -lfIconTitle.lfHeight, 10 );	// フォントサイズ（1/10ポイント単位）
//	To Here Sept. 19,2000

		// [全般]タブ
		{
			CommonSetting_General& sGeneral = m_pShareData->m_Common.m_sGeneral;

			sGeneral.m_nMRUArrNum_MAX = 15;	/* ファイルの履歴MAX */	//Oct. 14, 2000 JEPRO 少し増やした(10→15)
			sGeneral.m_nOPENFOLDERArrNum_MAX = 15;	/* フォルダの履歴MAX */	//Oct. 14, 2000 JEPRO 少し増やした(10→15)

			sGeneral.m_nCaretType = 0;					/* カーソルのタイプ 0=win 1=dos */
			sGeneral.m_bIsINSMode = true;				/* 挿入／上書きモード */
			sGeneral.m_bIsFreeCursorMode = false;		/* フリーカーソルモードか */	//Oct. 29, 2000 JEPRO 「なし」に変更

			sGeneral.m_bStopsBothEndsWhenSearchWord = FALSE;	/* 単語単位で移動するときに、単語の両端で止まるか */
			sGeneral.m_bStopsBothEndsWhenSearchParagraph = FALSE;	/* 単語単位で移動するときに、単語の両端で止まるか */

			sGeneral.m_bCloseAllConfirm = FALSE;		/* [すべて閉じる]で他に編集用のウィンドウがあれば確認する */	// 2006.12.25 ryoji
			sGeneral.m_bExitConfirm = FALSE;			/* 終了時の確認をする */
			sGeneral.m_nRepeatedScrollLineNum = CLayoutInt(3);	/* キーリピート時のスクロール行数 */
			sGeneral.m_nRepeatedMoveCaretNum = 2;		// キーリピート時の左右移動数
			sGeneral.m_nRepeatedScroll_Smooth = FALSE;	/* キーリピート時のスクロールを滑らかにするか */
			sGeneral.m_nPageScrollByWheel = 0;			/* キー/マウスボタン + ホイールスクロールでページスクロールする */	// 2009.01.17 nasukoji
			sGeneral.m_nHorizontalScrollByWheel = 0;	/* キー/マウスボタン + ホイールスクロールで横スクロールする */		// 2009.01.17 nasukoji

			sGeneral.m_bUseTaskTray = TRUE;				/* タスクトレイのアイコンを使う */
			sGeneral.m_bStayTaskTray = FALSE;			/* タスクトレイのアイコンを常駐 */
			sGeneral.m_wTrayMenuHotKeyCode = L'Z';		/* タスクトレイ左クリックメニュー キー */
			sGeneral.m_wTrayMenuHotKeyMods = HOTKEYF_ALT | HOTKEYF_CONTROL;	/* タスクトレイ左クリックメニュー キー */

			sGeneral.m_bDispExitingDialog = FALSE;		/* 終了ダイアログを表示する */

			sGeneral.m_bNoCaretMoveByActivation = FALSE;	/* マウスクリックにてアクティベートされた時はカーソル位置を移動しない 2007.10.02 nasukoji (add by genta) */
		}

		// [ウィンドウ]タブ
		{
			CommonSetting_Window& sWindow = m_pShareData->m_Common.m_sWindow;

			sWindow.m_bDispTOOLBAR = TRUE;			/* 次回ウィンドウを開いたときツールバーを表示する */
			sWindow.m_bDispSTATUSBAR = TRUE;			/* 次回ウィンドウを開いたときステータスバーを表示する */
			sWindow.m_bDispFUNCKEYWND = FALSE;		/* 次回ウィンドウを開いたときファンクションキーを表示する */
			sWindow.m_bDispMiniMap = false;			// ミニマップを表示する
			sWindow.m_nFUNCKEYWND_Place = 1;			/* ファンクションキー表示位置／0:上 1:下 */
			sWindow.m_nFUNCKEYWND_GroupNum = 4;			// 2002/11/04 Moca ファンクションキーのグループボタン数
			sWindow.m_nMiniMapFontSize = -1;
			sWindow.m_nMiniMapQuality = NONANTIALIASED_QUALITY;
			sWindow.m_nMiniMapWidth = 150;

			sWindow.m_bSplitterWndHScroll = TRUE;	// 2001/06/20 asa-o 分割ウィンドウの水平スクロールの同期をとる
			sWindow.m_bSplitterWndVScroll = TRUE;	// 2001/06/20 asa-o 分割ウィンドウの垂直スクロールの同期をとる

			// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除
			//	2004.05.13 Moca ウィンドウサイズ固定指定追加に伴う指定方法変更
			sWindow.m_eSaveWindowSize = WINSIZEMODE_SAVE;	// ウィンドウサイズ継承
			sWindow.m_nWinSizeType = SIZE_RESTORED;
			sWindow.m_nWinSizeCX = CW_USEDEFAULT;
			sWindow.m_nWinSizeCY = 0;
		
			sWindow.m_bScrollBarHorz = TRUE;				/* 水平スクロールバーを使う */
			//	2004.05.13 Moca ウィンドウ位置
			sWindow.m_eSaveWindowPos = WINSIZEMODE_DEF;		// ウィンドウ位置固定・継承
			sWindow.m_nWinPosX = CW_USEDEFAULT;
			sWindow.m_nWinPosY = 0;

			sWindow.m_nRulerHeight = 13;					/* ルーラーの高さ */
			sWindow.m_nRulerBottomSpace = 0;				/* ルーラーとテキストの隙間 */
			sWindow.m_nRulerType = 0;					/* ルーラーのタイプ */
			sWindow.m_nLineNumRightSpace = 0;			/* 行番号の右の隙間 */
			sWindow.m_nVertLineOffset = -1;			// 2005.11.10 Moca 指定桁縦線
			sWindow.m_bUseCompatibleBMP = TRUE;		// 2007.09.09 Moca 画面キャッシュを使う	// 2009.06.09 ryoji FALSE->TRUE

			sWindow.m_bMenuIcon = TRUE;		/* メニューにアイコンを表示する */

			//	Apr. 05, 2003 genta ウィンドウキャプションの初期値
			//	Aug. 16, 2003 genta $N(ファイル名省略表示)をデフォルトに変更
			_tcscpy( sWindow.m_szWindowCaptionActive, 
				_T("${w?$h$:アウトプット$:${I?$f$n$:$N$n$}$}${U?(更新)$} -")
				_T(" $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>") );
			_tcscpy( sWindow.m_szWindowCaptionInactive, 
				_T("${w?$h$:アウトプット$:$f$n$}${U?(更新)$} -")
				_T(" $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>") );
		}

		// [タブバー]タブ
		{
			CommonSetting_TabBar& sTabBar = m_pShareData->m_Common.m_sTabBar;

			sTabBar.m_bDispTabWnd = FALSE;			//タブウインドウ表示	//@@@ 2003.05.31 MIK
			sTabBar.m_bDispTabWndMultiWin = FALSE;	//タブウインドウ表示	//@@@ 2003.05.31 MIK
			wcscpy(	//@@@ 2003.06.13 MIK
				sTabBar.m_szTabWndCaption,
				L"${w?【Grep】$h$:【アウトプット】$:$f$n$}${U?(更新)$}${R?(ビューモード)$:(上書き禁止)$}${M?【キーマクロの記録中】$}"
			);
			sTabBar.m_bSameTabWidth = FALSE;			//タブを等幅にする			//@@@ 2006.01.28 ryoji
			sTabBar.m_bDispTabIcon = FALSE;			//タブにアイコンを表示する	//@@@ 2006.01.28 ryoji
			sTabBar.m_bDispTabClose = DISPTABCLOSE_NO;	//タブに閉じるボタンを表示する	//@@@ 2012.04.14 syat
			sTabBar.m_bSortTabList = TRUE;			//タブ一覧をソートする		//@@@ 2006.05.10 ryoji
			sTabBar.m_bTab_RetainEmptyWin = TRUE;	// 最後のファイルが閉じられたとき(無題)を残す	// 2007.02.11 genta
			sTabBar.m_bTab_CloseOneWin = FALSE;	// タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる	// 2007.02.11 genta
			sTabBar.m_bTab_ListFull = FALSE;			//タブ一覧をフルパス表示する	//@@@ 2007.02.28 ryoji
			sTabBar.m_bChgWndByWheel = FALSE;		//マウスホイールでウィンドウ切替	//@@@ 2006.03.26 ryoji
			sTabBar.m_bNewWindow = FALSE;			// 外部から起動するときは新しいウインドウで開く
			sTabBar.m_bTabMultiLine = false;		// タブ多段
			sTabBar.m_eTabPosition = TabPosition_Top;		//タブ位置

			sTabBar.m_lf = lfIconTitle;
			sTabBar.m_nPointSize = nIconPointSize;
			sTabBar.m_nTabMaxWidth = 200;
			sTabBar.m_nTabMinWidth = 60;
			sTabBar.m_nTabMinWidthOnMulti = 100;
		}

		// [編集]タブ
		{
			CommonSetting_Edit& sEdit = m_pShareData->m_Common.m_sEdit;

			sEdit.m_bAddCRLFWhenCopy = false;			/* 折り返し行に改行を付けてコピー */

			sEdit.m_bUseOLE_DragDrop = TRUE;			/* OLEによるドラッグ & ドロップを使う */
			sEdit.m_bUseOLE_DropSource = TRUE;			/* OLEによるドラッグ元にするか */
			sEdit.m_bSelectClickedURL = TRUE;			/* URLがクリックされたら選択するか */
			sEdit.m_bCopyAndDisablSelection = FALSE;	/* コピーしたら選択解除 */
			sEdit.m_bEnableNoSelectCopy = TRUE;		/* 選択なしでコピーを可能にする */	// 2007.11.18 ryoji
			sEdit.m_bEnableLineModePaste = true;		/* ラインモード貼り付けを可能にする */	// 2007.10.08 ryoji
			sEdit.m_bConvertEOLPaste = false;			/* 改行コードを変換して貼り付ける */	// 2009.02.28 salarm
			sEdit.m_bEnableExtEol = false;
			sEdit.m_bBoxSelectLock = true;

			sEdit.m_bNotOverWriteCRLF = TRUE;			/* 改行は上書きしない */
			sEdit.m_bOverWriteFixMode = false;			// 文字幅に合わせてスペースを詰める

			sEdit.m_bOverWriteBoxDelete = false;
			sEdit.m_eOpenDialogDir = OPENDIALOGDIR_CUR;
			auto_strcpy(sEdit.m_OpenDialogSelDir, _T("%Personal%\\"));
			sEdit.m_bAutoColumnPaste = TRUE;			/* 矩形コピーのテキストは常に矩形貼り付け */
		}

		// [ファイル]タブ
		{
			CommonSetting_File& sFile = m_pShareData->m_Common.m_sFile;

			//ファイルの排他制御
			sFile.m_nFileShareMode = SHAREMODE_NOT_EXCLUSIVE;	// ファイルの排他制御モード
			sFile.m_bCheckFileTimeStamp = true;			// 更新の監視
			sFile.m_nAutoloadDelay = 0;					// 自動読込時遅延
			sFile.m_bUneditableIfUnwritable = true;		// 上書き禁止検出時は編集禁止にする

			//ファイルの保存
			sFile.m_bEnableUnmodifiedOverwrite = false;	// 無変更でも上書きするか

			// 「名前を付けて保存」でファイルの種類が[ユーザ指定]のときのファイル一覧表示	//ファイル保存ダイアログのフィルタ設定	// 2006.11.16 ryoji
			sFile.m_bNoFilterSaveNew = true;		// 新規から保存時は全ファイル表示
			sFile.m_bNoFilterSaveFile = true;		// 新規以外から保存時は全ファイル表示

			//ファイルオープン
			sFile.m_bDropFileAndClose = false;		// ファイルをドロップしたときは閉じて開く
			sFile.m_nDropFileNumMax = 8;			// 一度にドロップ可能なファイル数
			sFile.m_bRestoreCurPosition = true;	// カーソル位置復元	//	Oct. 27, 2000 genta
			sFile.m_bRestoreBookmarks = true;		// ブックマーク復元	//2002.01.16 hor
			sFile.m_bAutoMIMEdecode = false;		// ファイル読み込み時にMIMEのデコードを行うか	//Jul. 13, 2001 JEPRO
			sFile.m_bQueryIfCodeChange = true;		// 前回と異なる文字コードの時に問い合わせを行うか	Oct. 03, 2004 genta
			sFile.m_bAlertIfFileNotExist = false;	// 開こうとしたファイルが存在しないとき警告する	Oct. 09, 2004 genta
			sFile.m_bAlertIfLargeFile = false;		// 開こうとしたファイルが大きい場合に警告する
			sFile.m_nAlertFileSize = 10;			// 警告を始めるファイルサイズ（MB単位）
		}

		// [バックアップ]タブ
		{
			CommonSetting_Backup& sBackup = m_pShareData->m_Common.m_sBackup;

			sBackup.m_bBackUp = false;										/* バックアップの作成 */
			sBackup.m_bBackUpDialog = true;									/* バックアップの作成前に確認 */
			sBackup.m_bBackUpFolder = false;								/* 指定フォルダにバックアップを作成する */
			sBackup.m_szBackUpFolder[0] = L'\0';							/* バックアップを作成するフォルダ */
			sBackup.m_nBackUpType = 2;										/* バックアップファイル名のタイプ 1=(.bak) 2=*_日付.* */
			sBackup.m_nBackUpType_Opt1 = BKUP_YEAR | BKUP_MONTH | BKUP_DAY;	/* バックアップファイル名：日付 */
			sBackup.m_nBackUpType_Opt2 = ('b' << 16 ) + 10;					/* バックアップファイル名：連番の数と先頭文字 */
			sBackup.m_nBackUpType_Opt3 = 5;									/* バックアップファイル名：Option3 */
			sBackup.m_nBackUpType_Opt4 = 0;									/* バックアップファイル名：Option4 */
			sBackup.m_nBackUpType_Opt5 = 0;									/* バックアップファイル名：Option5 */
			sBackup.m_nBackUpType_Opt6 = 0;									/* バックアップファイル名：Option6 */
			sBackup.m_bBackUpDustBox = false;								/* バックアップファイルをごみ箱に放り込む */	//@@@ 2001.12.11 add MIK
			sBackup.m_bBackUpPathAdvanced = false;							/* 20051107 aroka バックアップ先フォルダを詳細設定する */
			sBackup.m_szBackUpPathAdvanced[0] = _T('\0');					/* 20051107 aroka バックアップを作成するフォルダの詳細設定 */
		}

		// [書式]タブ
		{
			CommonSetting_Format& sFormat = m_pShareData->m_Common.m_sFormat;

			/* 見出し記号 */
			wcscpy( sFormat.m_szMidashiKigou, L"１２３４５６７８９０（(［[「『【■□▲△▼▽◆◇○◎●§・※☆★第①②③④⑤⑥⑦⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩ一二三四五六七八九十壱弐参伍" );
			/* 引用符 */
			wcscpy( sFormat.m_szInyouKigou, L"> " );		/* 引用符 */

			/*
				書式指定子の意味はWindows SDKのGetDateFormat(), GetTimeFormat()を参照のこと
			*/

			sFormat.m_nDateFormatType = 0;	//日付書式のタイプ
			_tcscpy( sFormat.m_szDateFormat, _T("yyyy\'年\'M\'月\'d\'日(\'dddd\')\'") );	//日付書式
			sFormat.m_nTimeFormatType = 0;	//時刻書式のタイプ
			_tcscpy( sFormat.m_szTimeFormat, _T("tthh\'時\'mm\'分\'ss\'秒\'")  );			//時刻書式
		}

		// [検索]タブ
		{
			CommonSetting_Search& sSearch = m_pShareData->m_Common.m_sSearch;

			sSearch.m_sSearchOption.Reset();			// 検索オプション
			sSearch.m_bConsecutiveAll = 0;			// 「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
			sSearch.m_bSelectedArea = FALSE;			// 選択範囲内置換
			sSearch.m_bNOTIFYNOTFOUND = TRUE;		/* 検索／置換  見つからないときメッセージを表示 */

			sSearch.m_bGrepSubFolder = TRUE;			/* Grep: サブフォルダも検索 */
			sSearch.m_nGrepOutputLineType = 1;			// Grep: 行を出力/該当部分/否マッチ行 を出力
			sSearch.m_nGrepOutputStyle = 1;			/* Grep: 出力形式 */
			sSearch.m_bGrepOutputFileOnly = false;
			sSearch.m_bGrepOutputBaseFolder = false;
			sSearch.m_bGrepSeparateFolder = false;
			sSearch.m_bGrepBackup = true;

			sSearch.m_bGrepDefaultFolder=FALSE;		/* Grep: フォルダの初期値をカレントフォルダにする */
			sSearch.m_nGrepCharSet = CODE_AUTODETECT;	/* Grep: 文字コードセット */
			sSearch.m_bGrepRealTimeView = FALSE;		/* 2003.06.28 Moca Grep結果のリアルタイム表示 */
			sSearch.m_bCaretTextForSearch = TRUE;		/* 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする */
			sSearch.m_bInheritKeyOtherView = true;
			sSearch.m_szRegexpLib[0] = _T('\0');		/* 2007.08.12 genta 正規表現DLL */
			sSearch.m_bGTJW_RETURN = TRUE;				/* エンターキーでタグジャンプ */
			sSearch.m_bGTJW_LDBLCLK = TRUE;			/* ダブルクリックでタグジャンプ */

			sSearch.m_bGrepExitConfirm = FALSE;			/* Grepモードで保存確認するか */

			sSearch.m_bAutoCloseDlgFind = TRUE;			/* 検索ダイアログを自動的に閉じる */
			sSearch.m_bSearchAll		 = FALSE;			/* 検索／置換／ブックマーク  先頭（末尾）から再検索 2002.01.26 hor */
			sSearch.m_bAutoCloseDlgReplace = TRUE;		/* 置換 ダイアログを自動的に閉じる */

			sSearch.m_nTagJumpMode = 1;				//タグジャンプモード
			sSearch.m_nTagJumpModeKeyword = 3;			//タグジャンプモード
		}

		// [キー割り当て]タブ
		{
			//	Jan. 30, 2005 genta 関数として独立
			//	2007.11.04 genta 戻り値チェック．falseなら起動中断．
			if( ! InitKeyAssign( m_pShareData )){
				return false;
			}
		}

		// [カスタムメニュー]タブ
		{
			CommonSetting_CustomMenu& sCustomMenu = m_pShareData->m_Common.m_sCustomMenu;

			for( int i = 0; i < MAX_CUSTOM_MENU; ++i ){
				sCustomMenu.m_szCustMenuNameArr[i][0] = '\0';
				sCustomMenu.m_nCustMenuItemNumArr[i] = 0;
				for( int j = 0; j < MAX_CUSTOM_MENU_ITEMS; ++j ){
					sCustomMenu.m_nCustMenuItemFuncArr[i][j] = F_0;
					sCustomMenu.m_nCustMenuItemKeyArr [i][j] = '\0';
				}
				sCustomMenu.m_bCustMenuPopupArr[i] = true;
			}
			sCustomMenu.m_szCustMenuNameArr[CUSTMENU_INDEX_FOR_TABWND][0] = '\0';	//@@@ 2003.06.13 MIK

			InitPopupMenu( m_pShareData );
		}

		// [ツールバー]タブ
		{
			//	Jan. 30, 2005 genta 関数として独立
			InitToolButtons( m_pShareData );
		}

		// [強調キーワード]タブ
		{
			InitKeyword( m_pShareData );
		}

		// [支援]タブ
		{
			CommonSetting_Helper& sHelper = m_pShareData->m_Common.m_sHelper;

			sHelper.m_lf = lfIconTitle;
			sHelper.m_nPointSize = nIconPointSize;	// フォントサイズ（1/10ポイント単位） ※古いバージョンからの移行を考慮して無効値で初期化	// 2009.10.01 ryoji

			sHelper.m_szExtHelp[0] = L'\0';			// 外部ヘルプ１
			sHelper.m_szExtHtmlHelp[0] = L'\0';		// 外部HTMLヘルプ
		
			sHelper.m_szMigemoDll[0] = L'\0';			/* migemo dll */
			sHelper.m_szMigemoDict[0] = L'\0';		/* migemo dict */

			sHelper.m_bHtmlHelpIsSingle = true;		/* HtmlHelpビューアはひとつ */

			sHelper.m_bHokanKey_RETURN	= TRUE;			/* VK_RETURN 補完決定キーが有効/無効 */
			sHelper.m_bHokanKey_TAB		= FALSE;		/* VK_TAB   補完決定キーが有効/無効 */
			sHelper.m_bHokanKey_RIGHT	= TRUE;			/* VK_RIGHT 補完決定キーが有効/無効 */
			sHelper.m_bHokanKey_SPACE	= FALSE;		/* VK_SPACE 補完決定キーが有効/無効 */
		}

		// [アウトライン]タブ
		{
			CommonSetting_OutLine& sOutline = m_pShareData->m_Common.m_sOutline;

			sOutline.m_nOutlineDockSet = 0;					/* アウトライン解析のドッキング位置継承方法 */
			sOutline.m_bOutlineDockSync = TRUE;				/* アウトライン解析のドッキング位置を同期する */
			sOutline.m_bOutlineDockDisp = FALSE;				/* アウトライン解析表示の有無 */
			sOutline.m_eOutlineDockSide = DOCKSIDE_FLOAT;		/* アウトライン解析ドッキング配置 */
			sOutline.m_cxOutlineDockLeft		=	0;	// アウトラインの左ドッキング幅
			sOutline.m_cyOutlineDockTop		=	0;	// アウトラインの上ドッキング高
			sOutline.m_cxOutlineDockRight		=	0;	// アウトラインの右ドッキング幅
			sOutline.m_cyOutlineDockBottom		=	0;	// アウトラインの下ドッキング高
			sOutline.m_nDockOutline = OUTLINE_TEXT;
			sOutline.m_bAutoCloseDlgFuncList = FALSE;		/* アウトライン ダイアログを自動的に閉じる */	//Nov. 18, 2000 JEPRO TRUE→FALSE に変更
			sOutline.m_bMarkUpBlankLineEnable	=	FALSE;	//アウトラインダイアログでブックマークの空行を無視			2002.02.08 aroka,hor
			sOutline.m_bFunclistSetFocusOnJump	=	FALSE;	//アウトラインダイアログでジャンプしたらフォーカスを移す	2002.02.08 hor

			InitFileTree( &sOutline.m_sFileTree );
			sOutline.m_sFileTreeDefIniName = _T("_sakurafiletree.ini");
		}

		// [ファイル内容比較]タブ
		{
			CommonSetting_Compare& sCompare = m_pShareData->m_Common.m_sCompare;

			sCompare.m_bCompareAndTileHorz = TRUE;		/* 文書比較後、左右に並べて表示 */
		}

		// [ビュー]タブ
		{
			CommonSetting_View& sView = m_pShareData->m_Common.m_sView;

			sView.m_lf = lf;
			sView.m_nPointSize = 0;	// フォントサイズ（1/10ポイント単位） ※古いバージョンからの移行を考慮して無効値で初期化	// 2009.10.01 ryoji

			sView.m_bFontIs_FIXED_PITCH = TRUE;				/* 現在のフォントは固定幅フォントである */
		}

		// [マクロ]タブ
		{
			CommonSetting_Macro& sMacro = m_pShareData->m_Common.m_sMacro;

			sMacro.m_szKeyMacroFileName[0] = _T('\0');	/* キーワードマクロのファイル名 */ //@@@ 2002.1.24 YAZAKI

			//	From Here Sep. 14, 2001 genta
			//	Macro登録の初期化
			MacroRec *mptr = sMacro.m_MacroTable;
			for( int i = 0; i < MAX_CUSTMACRO; ++i, ++mptr ){
				mptr->m_szName[0] = L'\0';
				mptr->m_szFile[0] = L'\0';
				mptr->m_bReloadWhenExecute = false;
			}
			//	To Here Sep. 14, 2001 genta

			_tcscpy( sMacro.m_szMACROFOLDER, szIniFolder );	/* マクロ用フォルダ */

			sMacro.m_nMacroOnOpened = -1;	/* オープン後自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji
			sMacro.m_nMacroOnTypeChanged = -1;	/* タイプ変更後自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji
			sMacro.m_nMacroOnSave = -1;	/* 保存前自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji
			sMacro.m_nMacroCancelTimer = 10;	// マクロ停止ダイアログ表示待ち時間(秒)	// 2011.08.04 syat
		}

		// [ファイル名表示]タブ
		{
			CommonSetting_FileName& sFileName = m_pShareData->m_Common.m_sFileName;

			sFileName.m_bTransformShortPath = true;
			sFileName.m_nTransformShortMaxWidth = 100; // 100'x'幅

			for( int i = 0; i < MAX_TRANSFORM_FILENAME; ++i ){
				sFileName.m_szTransformFileNameFrom[i][0] = _T('\0');
				sFileName.m_szTransformFileNameTo[i][0] = _T('\0');
			}
			_tcscpy( sFileName.m_szTransformFileNameFrom[0], _T("%DeskTop%\\") );
			_tcscpy( sFileName.m_szTransformFileNameTo[0],   _T("デスクトップ\\") );
			_tcscpy( sFileName.m_szTransformFileNameFrom[1], _T("%Personal%\\") );
			_tcscpy( sFileName.m_szTransformFileNameTo[1],   _T("マイドキュメント\\") );
			_tcscpy( sFileName.m_szTransformFileNameFrom[2], _T("%Cache%\\Content.IE5\\") );
			_tcscpy( sFileName.m_szTransformFileNameTo[2],   _T("IEキャッシュ\\") );
			_tcscpy( sFileName.m_szTransformFileNameFrom[3], _T("%TEMP%\\") );
			_tcscpy( sFileName.m_szTransformFileNameTo[3],   _T("TEMP\\") );
			_tcscpy( sFileName.m_szTransformFileNameFrom[4], _T("%Common DeskTop%\\") );
			_tcscpy( sFileName.m_szTransformFileNameTo[4],   _T("共有デスクトップ\\") );
			_tcscpy( sFileName.m_szTransformFileNameFrom[5], _T("%Common Documents%\\") );
			_tcscpy( sFileName.m_szTransformFileNameTo[5],   _T("共有ドキュメント\\") );
			_tcscpy( sFileName.m_szTransformFileNameFrom[6], _T("%AppData%\\") );	// 2007.05.19 ryoji 追加
			_tcscpy( sFileName.m_szTransformFileNameTo[6],   _T("アプリデータ\\") );	// 2007.05.19 ryoji 追加
			sFileName.m_nTransformFileNameArrNum = 7;
		}

		// [その他]タブ
		{
			CommonSetting_Others& sOthers = m_pShareData->m_Common.m_sOthers;

			::SetRect( &sOthers.m_rcOpenDialog, 0, 0, 0, 0 );	/* 「開く」ダイアログのサイズと位置 */
			::SetRect( &sOthers.m_rcCompareDialog, 0, 0, 0, 0 );
			::SetRect( &sOthers.m_rcDiffDialog, 0, 0, 0, 0 );
			::SetRect( &sOthers.m_rcFavoriteDialog, 0, 0, 0, 0 );
			::SetRect( &sOthers.m_rcTagJumpDialog, 0, 0, 0, 0 );
			::SetRect( &sOthers.m_rcWindowListDialog, 0, 0, 0, 0 );

			sOthers.m_bIniReadOnly = false;
		}

		// [ステータスバー]タブ
		{
			CommonSetting_Statusbar& sStatusbar = m_pShareData->m_Common.m_sStatusbar;

			// 表示文字コードの指定		2008/6/21	Uchi
			sStatusbar.m_bDispUniInSjis		= FALSE;	// SJISで文字コード値をUnicodeで表示する
			sStatusbar.m_bDispUniInJis			= FALSE;	// JISで文字コード値をUnicodeで表示する
			sStatusbar.m_bDispUniInEuc			= FALSE;	// EUCで文字コード値をUnicodeで表示する
			sStatusbar.m_bDispUtf8Codepoint	= TRUE;		// UTF-8をコードポイントで表示する
			sStatusbar.m_bDispSPCodepoint		= TRUE;		// サロゲートペアをコードポイントで表示する
			sStatusbar.m_bDispSelCountByByte	= FALSE;	// 選択文字数を文字単位ではなくバイト単位で表示する
			sStatusbar.m_bDispColByChar = FALSE; // 現在桁をルーラー単位ではなく文字単位で表示する
		}

		// [プラグイン]タブ
		{
			CommonSetting_Plugin& sPlugin = m_pShareData->m_Common.m_sPlugin;

			sPlugin.m_bEnablePlugin			= FALSE;	// プラグインを使用する
			for( int nPlugin=0; nPlugin < MAX_PLUGIN; nPlugin++ ){
				sPlugin.m_PluginTable[nPlugin].m_szName[0]	= L'\0';	// プラグイン名
				sPlugin.m_PluginTable[nPlugin].m_szId[0]	= L'\0';	// プラグインID
				sPlugin.m_PluginTable[nPlugin].m_state = PLS_NONE;		// プラグイン状態
			}
		}

		// [メインメニュー]タブ
		{
			CDataProfile	cProfile;
			std::vector<std::wstring> data;
			cProfile.SetReadingMode();
			cProfile.ReadProfileRes( MAKEINTRESOURCE(IDR_MENU1), MAKEINTRESOURCE(ID_RC_TYPE_INI), &data );

			CShareData_IO::IO_MainMenu( cProfile, &data, m_pShareData->m_Common.m_sMainMenu, false );
		}

		{
			InitTypeConfigs( m_pShareData, *m_pvTypeSettings );
		}

		{
			/* m_PrintSettingArr[0]を設定して、残りの1～7にコピーする。
				必要になるまで遅らせるために、CPrintに、CShareDataを操作する権限を与える。
				YAZAKI.
			*/
			{
				/*
					2006.08.16 Moca 初期化単位を PRINTSETTINGに変更。CShareDataには依存しない。
				*/
				TCHAR szSettingName[64];
				int i = 0;
				auto_sprintf( szSettingName, _T("印刷設定 %d"), i + 1 );
				CPrint::SettingInitialize( m_pShareData->m_PrintSettingArr[0], szSettingName );	//	初期化命令。
			}
			for( int i = 1; i < MAX_PRINTSETTINGARR; ++i ){
				m_pShareData->m_PrintSettingArr[i] = m_pShareData->m_PrintSettingArr[0];
				auto_sprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, _T("印刷設定 %d"), i + 1 );	/* 印刷設定の名前 */
			}
		}

		{
			m_pShareData->m_sSearchKeywords.m_aSearchKeys.clear();
			m_pShareData->m_sSearchKeywords.m_aReplaceKeys.clear();
			m_pShareData->m_sSearchKeywords.m_aGrepFiles.clear();
			m_pShareData->m_sSearchKeywords.m_aGrepFiles.push_back(_T("*.*"));
			m_pShareData->m_sSearchKeywords.m_aGrepFolders.clear();

			// 2004/06/21 novice タグジャンプ機能追加
			m_pShareData->m_sTagJump.m_TagJumpNum = 0;
			// 2004.06.22 Moca タグジャンプの先頭
			m_pShareData->m_sTagJump.m_TagJumpTop = 0;
			//From Here 2005.04.03 MIK キーワード指定タグジャンプのHistory保管
			m_pShareData->m_sTagJump.m_aTagJumpKeywords.clear();
			m_pShareData->m_sTagJump.m_bTagJumpICase = FALSE;
			m_pShareData->m_sTagJump.m_bTagJumpAnyWhere = FALSE;
			//To Here 2005.04.03 MIK 

			m_pShareData->m_sHistory.m_aExceptMRU.clear();

			_tcscpy( m_pShareData->m_sHistory.m_szIMPORTFOLDER, szIniFolder );	/* 設定インポート用フォルダ */

			m_pShareData->m_sHistory.m_aCommands.clear();
			m_pShareData->m_sHistory.m_aCurDirs.clear();

			m_pShareData->m_nExecFlgOpt = 1;	/* 外部コマンド実行の「標準出力を得る」 */	// 2006.12.03 maru オプションの拡張のため

			m_pShareData->m_nDiffFlgOpt = 0;	/* DIFF差分表示 */	//@@@ 2002.05.27 MIK

			m_pShareData->m_szTagsCmdLine[0] = _T('\0');	/* CTAGS */	//@@@ 2003.05.12 MIK
			m_pShareData->m_nTagsOpt = 0;	/* CTAGS */	//@@@ 2003.05.12 MIK

			m_pShareData->m_bLineNumIsCRLF_ForJump = true;	/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */
		}
	}else{
		/* オブジェクトがすでに存在する場合 */
		/* ファイルのビューを､ 呼び出し側プロセスのアドレス空間にマップします */
		m_pShareData = (DLLSHAREDATA*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		SetDllShareData( m_pShareData );

		SelectCharWidthCache( CWM_FONT_EDIT, CWM_CACHE_SHARE );
		InitCharWidthCache(m_pShareData->m_Common.m_sView.m_lf);	// 2008/5/15 Uchi

		//	From Here Oct. 27, 2000 genta
		//	2014.01.08 Moca サイズチェック追加
		if( m_pShareData->m_vStructureVersion != uShareDataVersion ||
			m_pShareData->m_nSize != sizeof(*m_pShareData) ){
			//	この共有データ領域は使えない．
			//	ハンドルを解放する
			SetDllShareData( NULL );
			::UnmapViewOfFile( m_pShareData );
			m_pShareData = NULL;
			return false;
		}
		//	To Here Oct. 27, 2000 genta

	}
	return true;
}




static void ConvertLangString( wchar_t* pBuf, size_t chBufSize, std::wstring& org, std::wstring& to )
{
	CNativeW mem;
	mem.SetString(pBuf);
	mem.Replace(org.c_str(), to.c_str());
	auto_strncpy(pBuf, mem.GetStringPtr(), chBufSize);
	pBuf[chBufSize - 1] = L'\0';
}

static void ConvertLangString( char* pBuf, size_t chBufSize, std::wstring& org, std::wstring& to )
{
	CNativeA mem;
	mem.SetString(pBuf);
	mem.Replace_j(to_achar(org.c_str()), to_achar(to.c_str()));
	auto_strncpy(pBuf, mem.GetStringPtr(), chBufSize);
	pBuf[chBufSize - 1] = '\0';
}

static void ConvertLangValueImpl( wchar_t* pBuf, size_t chBufSize, int nStrId, std::vector<std::wstring>& values, int& index, bool setValues, bool bUpdate )
{
	if( setValues ){
		if( bUpdate ){
			values.push_back( std::wstring(LSW(nStrId)) );
		}
		return;
	}
	std::wstring to = LSW(nStrId);
	ConvertLangString( pBuf, chBufSize, values[index], to );
	index++;
}

static void ConvertLangValueImpl( char* pBuf, size_t chBufSize, int nStrId, std::vector<std::wstring>& values, int& index, bool setValues, bool bUpdate )
{
	if( setValues ){
		if( bUpdate ){
			values.push_back( std::wstring(LSW(nStrId)) );
		}
		return;
	}
	std::wstring to = LSW(nStrId);
	ConvertLangString( pBuf, chBufSize, values[index], to );
	index++;
}


#define ConvertLangValue(buf, id)  ConvertLangValueImpl(buf, _countof(buf), id, values, index, bSetValues, true);
#define ConvertLangValue2(buf, id) ConvertLangValueImpl(buf, _countof(buf), id, values, index, bSetValues, false);



/*!
	国際化対応のための文字列を変更する

	1. 1回目呼び出し、setValuesをtrueにして、valuesに旧設定の言語文字列を読み込み
	2. SelectLang呼び出し
	3. 2回目呼び出し、valuesを使って新設定の言語に書き換え
*/
void CShareData::ConvertLangValues(std::vector<std::wstring>& values, bool bSetValues)
{
	DLLSHAREDATA&	shareData = *m_pShareData;
	int i;
	int index = 0;
	int indexBackup;
	CommonSetting& common = shareData.m_Common;
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_TAB_CAPTION_OUTPUT );
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_TAB_CAPTION_GREP );
	indexBackup = index;
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_CAPTION_ACTIVE_OUTPUT );
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_CAPTION_ACTIVE_UPDATE );
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_CAPTION_ACTIVE_VIEW );
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_CAPTION_ACTIVE_OVERWRITE );
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_CAPTION_ACTIVE_KEYMACRO );
	index = indexBackup;
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionActive, STR_CAPTION_ACTIVE_OUTPUT );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionActive, STR_CAPTION_ACTIVE_UPDATE );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionActive, STR_CAPTION_ACTIVE_VIEW );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionActive, STR_CAPTION_ACTIVE_OVERWRITE );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionActive, STR_CAPTION_ACTIVE_KEYMACRO );
	index = indexBackup;
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionInactive, STR_CAPTION_ACTIVE_OUTPUT );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionInactive, STR_CAPTION_ACTIVE_UPDATE );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionInactive, STR_CAPTION_ACTIVE_VIEW );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionInactive, STR_CAPTION_ACTIVE_OVERWRITE );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionInactive, STR_CAPTION_ACTIVE_KEYMACRO );
	ConvertLangValue( common.m_sFormat.m_szDateFormat, STR_DATA_FORMAT );
	ConvertLangValue( common.m_sFormat.m_szTimeFormat, STR_TIME_FORMAT );
	indexBackup = index;
	for( i = 0; i < common.m_sFileName.m_nTransformFileNameArrNum; i++ ){
		index = indexBackup;
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_COMDESKTOP );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_COMDOC );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_DESKTOP );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_MYDOC );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_IE );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_TEMP );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_APPDATA );
		if( bSetValues ){
			break;
		}
	}
	indexBackup = index;
	for( i = 0; i < MAX_PRINTSETTINGARR; i++ ){
		index = indexBackup;
		ConvertLangValue( shareData.m_PrintSettingArr[i].m_szPrintSettingName, STR_PRINT_SET_NAME );
		if( bSetValues ){
			break;
		}
	}
	assert( m_pvTypeSettings != NULL );
	indexBackup = index;
	ConvertLangValue( shareData.m_TypeBasis.m_szTypeName, STR_TYPE_NAME_BASIS );
	for( i = 0; i < (int)GetTypeSettings().size(); i++ ){
		index = indexBackup;
		STypeConfig& type = *(GetTypeSettings()[i]);
		ConvertLangValue2( type.m_szTypeName, STR_TYPE_NAME_BASIS );
		ConvertLangValue( type.m_szTypeName, STR_TYPE_NAME_RICHTEXT );
		ConvertLangValue( type.m_szTypeName, STR_TYPE_NAME_TEXT );
		ConvertLangValue( type.m_szTypeName, STR_TYPE_NAME_DOS );
		ConvertLangValue( type.m_szTypeName, STR_TYPE_NAME_ASM );
		ConvertLangValue( type.m_szTypeName, STR_TYPE_NAME_INI );
		index = indexBackup;
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_BASIS );
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_RICHTEXT );
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_TEXT );
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_DOS );
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_ASM );
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_INI );
		if( bSetValues ){
			break;
		}
	}
}

/*!
	@brief	指定ファイルが開かれているか調べる
	
	指定のファイルが開かれている場合は開いているウィンドウのハンドルを返す

	@retval	TRUE すでに開いていた
	@retval	FALSE 開いていなかった
*/
BOOL CShareData::IsPathOpened( const TCHAR* pszPath, HWND* phwndOwner )
{
	EditInfo*	pfi;
	*phwndOwner = NULL;

	//	2007.10.01 genta 相対パスを絶対パスに変換
	//	変換しないとIsPathOpenedで正しい結果が得られず，
	//	同一ファイルを複数開くことがある．
	TCHAR	szBuf[_MAX_PATH];
	if( GetLongFileName( pszPath, szBuf )){
		pszPath = szBuf;
	}

	// 現在の編集ウィンドウの数を調べる
	if( 0 == CAppNodeGroupHandle(0).GetEditorWindowsNum() ){
		return FALSE;
	}
	
	for( int i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) ){
			// トレイからエディタへの編集ファイル名要求通知
			::SendMessageAny( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd, MYWM_GETFILEINFO, 1, 0 );
			pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

			// 同一パスのファイルが既に開かれているか
			if( 0 == _tcsicmp( pfi->m_szPath, pszPath ) ){
				*phwndOwner = m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*!
	@brief	指定ファイルが開かれているか調べつつ、多重オープン時の文字コード衝突も確認

	もしすでに開いていればアクティブにして、ウィンドウのハンドルを返す。
	さらに、文字コードが異なるときのワーニングも処理する。
	あちこちに散らばった多重オープン処理を集結させるのが目的。

	@retval	開かれている場合は開いているウィンドウのハンドル

	@note	CEditDoc::FileLoadに先立って実行されることもあるが、
			CEditDoc::FileLoadからも実行される必要があることに注意。
			(フォルダ指定の場合やCEditDoc::FileLoadが直接実行される場合もあるため)

	@retval	TRUE すでに開いていた
	@retval	FALSE 開いていなかった

	@date 2007.03.12 maru 新規作成
*/
BOOL CShareData::ActiveAlreadyOpenedWindow( const TCHAR* pszPath, HWND* phwndOwner, ECodeType nCharCode )
{
	if( IsPathOpened( pszPath, phwndOwner ) ){
		
		//文字コードの一致確認
		EditInfo*		pfi;
		::SendMessageAny( *phwndOwner, MYWM_GETFILEINFO, 0, 0 );
		pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;
		if(nCharCode != CODE_AUTODETECT){
			TCHAR szCpNameCur[100];
			CCodePage::GetNameLong(szCpNameCur, pfi->m_nCharCode);
			TCHAR szCpNameNew[100];
			CCodePage::GetNameLong(szCpNameNew, pfi->m_nCharCode);
			if(szCpNameCur[0] && szCpNameNew[0]){
				if(nCharCode != pfi->m_nCharCode){
					TopWarningMessage( *phwndOwner,
						LS(STR_ERR_CSHAREDATA20),
						pszPath,
						szCpNameCur,
						szCpNameNew
					);
				}
			}
			else{
				TopWarningMessage( *phwndOwner,
					LS(STR_ERR_CSHAREDATA21),
					pszPath,
					pfi->m_nCharCode,
					0==szCpNameCur[0]?LS(STR_ERR_CSHAREDATA22):szCpNameCur,
					nCharCode,
					0==szCpNameNew[0]?LS(STR_ERR_CSHAREDATA22):szCpNameNew
				);
			}
		}

		// 開いているウィンドウをアクティブにする
		ActivateFrameWindow( *phwndOwner );

		// MRUリストへの登録
		CMRUFile().Add( pfi );
		return TRUE;
	}
	else {
		return FALSE;
	}

}













/*!
	アウトプットウインドウに出力(書式付)

	アウトプットウインドウが無ければオープンする
	@param lpFmt [in] 書式指定文字列(wchar_t版)
	@date 2010.02.22 Moca auto_vsprintfから tchar_vsnprintf_s に変更.長すぎるときは切り詰められる
*/
void CShareData::TraceOut( LPCTSTR lpFmt, ... )
{
	if( false == OpenDebugWindow( m_hwndTraceOutSource, false ) ){
		return;
	}
	
	LockGuard<CMutex> guard( CShareData::GetMutexShareWork() );
	va_list argList;
	va_start( argList, lpFmt );
	int ret = tchar_vsnprintf_s( m_pShareData->m_sWorkBuffer.GetWorkBuffer<WCHAR>(), 
		m_pShareData->m_sWorkBuffer.GetWorkBufferCount<WCHAR>(),
		to_wchar(lpFmt), argList );
	va_end( argList );
	if( -1 == ret ){
		// 切り詰められた
		ret = auto_strlen( m_pShareData->m_sWorkBuffer.GetWorkBuffer<WCHAR>() );
	}else if( ret < 0 ){
		// 保護コード:受け側はwParam→size_tで符号なしのため
		ret = 0;
	}
	DWORD_PTR dwMsgResult;
	::SendMessageTimeout( m_pShareData->m_sHandles.m_hwndDebug, MYWM_ADDSTRINGLEN_W, ret, 0,
		SMTO_NORMAL, 10000, &dwMsgResult );
}

/*!
	アウトプットウインドウに出力(文字列指定)

	長い場合は分割して送る
	アウトプットウインドウが無ければオープンする
	@param  pStr  出力する文字列
	@param  len   pStrの文字数(終端NULを含まない) -1で自動計算
	@date 2010.05.11 Moca 新設
*/
void CShareData::TraceOutString( const wchar_t* pStr, int len )
{
	if( false == OpenDebugWindow( m_hwndTraceOutSource, false ) ){
		return;
	}
	if( -1 == len ){
		len = wcslen(pStr);
	}
	// m_sWorkBufferぎりぎりでも問題ないけれど、念のため\0終端にするために余裕をとる
	// -1 より 8,4バイト境界のほうがコピーが早いはずなので、-4にする
	const int buffLen = (int)m_pShareData->m_sWorkBuffer.GetWorkBufferCount<WCHAR>() - 4;
	wchar_t*  pOutBuffer = m_pShareData->m_sWorkBuffer.GetWorkBuffer<WCHAR>();
	int outPos = 0;
	if(0 == len){
		// 0のときは何も追加しないが、カーソル移動が発生する
		LockGuard<CMutex> guard( CShareData::GetMutexShareWork() );
		pOutBuffer[0] = L'\0';
		::SendMessage( m_pShareData->m_sHandles.m_hwndDebug, MYWM_ADDSTRINGLEN_W, 0, 0 );
	}else{
		while(outPos < len){
			int outLen = buffLen;
			if(len - outPos < buffLen){
				// 残り全部
				outLen = len - outPos;
			}
			// あまりが1文字以上ある
			if( outPos + outLen < len ){
				// CRLF(\r\n)とUTF-16が分離されないように
				if( (pStr[outPos + outLen - 1] == WCODE::CR && pStr[outPos + outLen] == WCODE::LF)
					|| (IsUtf16SurrogHi( pStr[outPos + outLen - 1] ) && IsUtf16SurrogLow( pStr[outPos + outLen] )) ){
					--outLen;
				}
			}
			LockGuard<CMutex> guard( CShareData::GetMutexShareWork() );
			wmemcpy( pOutBuffer, pStr + outPos, outLen );
			pOutBuffer[outLen] = L'\0';
			DWORD_PTR	dwMsgResult;
			if( 0 == ::SendMessageTimeout( m_pShareData->m_sHandles.m_hwndDebug, MYWM_ADDSTRINGLEN_W, outLen, 0,
				SMTO_NORMAL, 10000, &dwMsgResult ) ){
				// エラーかタイムアウト
				break;
			}
			outPos += outLen;
		}
	}
}

/*
	デバッグウィンドウを表示
	@param hwnd 新規ウィンドウのときのデバッグウィンドウの所属グループ
	@param bAllwaysActive 表示済みウィンドウでもアクティブにする
	@return true:表示できた。またはすでに表示されている。
	@date 2010.05.11 Moca TraceOutから分離
*/
bool CShareData::OpenDebugWindow( HWND hwnd, bool bAllwaysActive )
{
	bool ret = true;
	if( NULL == m_pShareData->m_sHandles.m_hwndDebug
	|| !IsSakuraMainWindow( m_pShareData->m_sHandles.m_hwndDebug )
	){
		// 2007.06.26 ryoji
		// アウトプットウィンドウを作成元と同じグループに作成するために m_hwndTraceOutSource を使っています
		// （m_hwndTraceOutSource は CEditWnd::Create() で予め設定）
		// ちょっと不恰好だけど、TraceOut() の引数にいちいち起動元を指定するのも．．．
		// 2010.05.11 Moca m_hwndTraceOutSourceは依然として使っていますが引数にしました
		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = _T("");
		// CODE_SJIS->CODE_UNICODE	2008/6/8 Uchi
		// CODE_UNICODE->CODE_NONE	2010.05.11 Moca デフォルト文字コードで設定できるように無指定に変更
		sLoadInfo.eCharCode = CODE_NONE;
		sLoadInfo.bViewMode = false;
		ret = CControlTray::OpenNewEditor( NULL, hwnd, sLoadInfo, _T("-DEBUGMODE"), true );
		//	2001/06/23 N.Nakatani 窓が出るまでウエイトをかけるように修正
		//アウトプットウインドウが出来るまで5秒ぐらい待つ。
		//	Jun. 25, 2001 genta OpenNewEditorの同期機能を利用するように変更
		bAllwaysActive = true; // 新しく作ったときはactive
	}
	/* 開いているウィンドウをアクティブにする */
	if(ret && bAllwaysActive){
		ActivateFrameWindow( m_pShareData->m_sHandles.m_hwndDebug );
	}
	return ret;
}

/* iniファイルの保存先がユーザ別設定フォルダかどうか */	// 2007.05.25 ryoji
BOOL CShareData::IsPrivateSettings( void ){
	return m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate;
}



/*
	CShareData::CheckMRUandOPENFOLDERList
	MRUとOPENFOLDERリストの存在チェックなど
	存在しないファイルやフォルダはMRUやOPENFOLDERリストから削除する

	@note 現在は使われていないようだ。
	@par History
	2001.12.26 削除した。（YAZAKI）
	
*/
/*!	idxで指定したマクロファイル名（フルパス）を取得する．

	@param pszPath [in]	パス名の出力先．長さのみを知りたいときはNULLを入れる．
	@param idx [in]		マクロ番号
	@param nBufLen [in]	pszPathで指定されたバッファのバッファサイズ

	@retval >0 : パス名の長さ．
	@retval  0 : エラー，そのマクロは使えない，ファイル名が指定されていない．
	@retval <0 : バッファ不足．必要なバッファサイズは -(戻り値)+1

	@author YAZAKI
	@date 2003.06.08 Moca ローカル変数へのポインタを返さないように仕様変更
	@date 2003.06.14 genta 文字列長，ポインタのチェックを追加
	@date 2003.06.24 Moca idxが-1のとき、キーマクロのフルパスを返す.
	
	@note idxは正確なものでなければならない。(内部で正当性チェックを行っていない)
*/
int CShareData::GetMacroFilename( int idx, TCHAR *pszPath, int nBufLen )
{
	if( -1 != idx && !m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].IsEnabled() )
		return 0;
	const TCHAR *ptr;
	const TCHAR *pszFile;

	if( -1 == idx ){
		pszFile = _T("RecKey.mac");
	}else{
		pszFile = m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].m_szFile;
	}
	if( pszFile[0] == _T('\0') ){	//	ファイル名が無い
		if( pszPath != NULL ){
			pszPath[0] = _T('\0');
		}
		return 0;
	}
	ptr = pszFile;
	int nLen = _tcslen( ptr ); // Jul. 21, 2003 genta wcslen対象が誤っていたためマクロ実行ができない

	if( !_IS_REL_PATH( pszFile )	// 絶対パス
		|| m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER[0] == _T('\0') ){	//	フォルダ指定なし
		if( pszPath == NULL || nBufLen <= nLen ){
			return -nLen;
		}
		_tcscpy( pszPath, pszFile );
		return nLen;
	}
	else {	//	フォルダ指定あり
		//	相対パス→絶対パス
		int nFolderSep = AddLastChar( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER, _countof2(m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER), _T('\\') );
		int nAllLen;
		TCHAR *pszDir;
		TCHAR szDir[_MAX_PATH + _countof2( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER )];

		 // 2003.06.24 Moca フォルダも相対パスなら実行ファイルからのパス
		// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
		if( _IS_REL_PATH( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER ) ){
			GetInidirOrExedir( szDir, m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER );
			pszDir = szDir;
		}else{
			pszDir = m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER;
		}

		int nDirLen = _tcslen( pszDir );
		nAllLen = nDirLen + nLen + ( -1 == nFolderSep ? 1 : 0 );
		if( pszPath == NULL || nBufLen <= nAllLen ){
			return -nAllLen;
		}

		_tcscpy( pszPath, pszDir );
		TCHAR *ptr = pszPath + nDirLen;
		if( -1 == nFolderSep ){
			*ptr++ = _T('\\');
		}
		_tcscpy( ptr, pszFile );
		return nAllLen;
	}

}

/*!	idxで指定したマクロのm_bReloadWhenExecuteを取得する。
	idxは正確なものでなければならない。
	YAZAKI
*/
bool CShareData::BeReloadWhenExecuteMacro( int idx )
{
	if( !m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].IsEnabled() )
		return false;

	return m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].m_bReloadWhenExecute;
}




/*!	@brief 共有メモリ初期化/ツールバー

	ツールバー関連の初期化処理

	@author genta
	@date 2005.01.30 genta CShareData::Init()から分離．
		一つずつ設定しないで一気にデータ転送するように．
*/
void CShareData::InitToolButtons(DLLSHAREDATA* pShareData)
{
		/* ツールバーボタン構造体 */
//Sept. 16, 2000 JEPRO
//	CShareData_new2.cppでできるだけ系ごとに集まるようにアイコンの順番を大幅に入れ替えたのに伴い以下の初期設定値を変更
	// 2010.06.26 Moca 内容は CMenuDrawer::FindToolbarNoFromCommandId の戻り値です
	static const int DEFAULT_TOOL_BUTTONS[] = {
		1,	//新規作成
		25,		//ファイルを開く(DropDown)
		3,		//上書き保存		//Sept. 16, 2000 JEPRO 3→11に変更	//Oct. 25, 2000 11→3
		4,		//名前を付けて保存	//Sept. 19, 2000 JEPRO 追加
		0,

		33,	//元に戻す(Undo)	//Sept. 16, 2000 JEPRO 7→19に変更	//Oct. 25, 2000 19→33
		34,	//やり直し(Redo)	//Sept. 16, 2000 JEPRO 8→20に変更	//Oct. 25, 2000 20→34
		0,

		87,	//移動履歴: 前へ	//Dec. 24, 2000 JEPRO 追加
		88,	//移動履歴: 次へ	//Dec. 24, 2000 JEPRO 追加
		0,

		225,	//検索		//Sept. 16, 2000 JEPRO 9→22に変更	//Oct. 25, 2000 22→225
		226,	//次を検索	//Sept. 16, 2000 JEPRO 16→23に変更	//Oct. 25, 2000 23→226
		227,	//前を検索	//Sept. 16, 2000 JEPRO 17→24に変更	//Oct. 25, 2000 24→227
		228,	//置換		// Oct. 7, 2000 JEPRO 追加
		229,	//検索マークのクリア	//Sept. 16, 2000 JEPRO 41→25に変更(Oct. 7, 2000 25→26)	//Oct. 25, 2000 25→229
		230,	//Grep		//Sept. 16, 2000 JEPRO 14→31に変更	//Oct. 25, 2000 31→230
		232,	//アウトライン解析	//Dec. 24, 2000 JEPRO 追加
		0,

		264,	//タイプ別設定一覧	//Sept. 16, 2000 JEPRO 追加
		265,	//タイプ別設定		//Sept. 16, 2000 JEPRO 18→36に変更	//Oct. 25, 2000 36→265
		266,	//共通設定			//Sept. 16, 2000 JEPRO 10→37に変更 説明を「設定プロパティシート」から変更	//Oct. 25, 2000 37→266
		0,		//Oct. 8, 2000 jepro 次行のために追加
		346,	//コマンド一覧	//Oct. 8, 2000 JEPRO 追加
	};

	//	ツールバーアイコン数の最大値を超えないためのおまじない
	//	最大値を超えて定義しようとするとここでコンパイルエラーになります．
	char dummy[ _countof(DEFAULT_TOOL_BUTTONS) < MAX_TOOLBAR_BUTTON_ITEMS ? 1:0 ];
	dummy[0]=0;

	memcpy_raw(
		pShareData->m_Common.m_sToolBar.m_nToolBarButtonIdxArr,
		DEFAULT_TOOL_BUTTONS,
		sizeof(DEFAULT_TOOL_BUTTONS)
	);

	/* ツールバーボタンの数 */
	pShareData->m_Common.m_sToolBar.m_nToolBarButtonNum = _countof(DEFAULT_TOOL_BUTTONS);
	pShareData->m_Common.m_sToolBar.m_bToolBarIsFlat = !IsVisualStyle();			/* フラットツールバーにする／しない */	// 2006.06.23 ryoji ビジュアルスタイルでは初期値をノーマルにする
	
}


/*!	@brief 共有メモリ初期化/ポップアップメニュー

	ポップアップメニューの初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離．
*/
void CShareData::InitPopupMenu(DLLSHAREDATA* pShareData)
{
	/* カスタムメニュー 規定値 */
	
	CommonSetting_CustomMenu& rMenu = m_pShareData->m_Common.m_sCustomMenu;

	/* 右クリックメニュー */
	int n = 0;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_UNDO;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'U';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_REDO;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'R';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_CUT;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'T';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPY;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'C';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_PASTE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'P';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_DELETE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'D';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPY_CRLF;	//Nov. 9, 2000 JEPRO 「CRLF改行でコピー」を追加
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'L';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPY_ADDCRLF;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'H';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_PASTEBOX;	//Nov. 9, 2000 JEPRO 「矩形貼り付け」を復活
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'X';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_SELECTALL;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'A';
	n++;

	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;		//Oct. 3, 2000 JEPRO 以下に「タグジャンプ」と「タグジャンプバック」を追加
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_TAGJUMP;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'G';		//Nov. 9, 2000 JEPRO 「コピー」とバッティングしていたアクセスキーを変更(T→G)
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_TAGJUMPBACK;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'B';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;		//Oct. 15, 2000 JEPRO 以下に「選択範囲内全行コピー」と「引用符付きコピー」を追加
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPYLINES;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '@';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPYLINESASPASSAGE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '.';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_COPYPATH;
	rMenu.m_nCustMenuItemKeyArr [0][n] = '\\';
	n++;
	rMenu.m_nCustMenuItemFuncArr[0][n] = F_PROPERTY_FILE;
	rMenu.m_nCustMenuItemKeyArr [0][n] = 'F';		//Nov. 9, 2000 JEPRO 「やり直し」とバッティングしていたアクセスキーを変更(R→F)
	n++;
	rMenu.m_nCustMenuItemNumArr[0] = n;

	/* カスタムメニュー１ */
	rMenu.m_nCustMenuItemNumArr[1] = 7;
	rMenu.m_nCustMenuItemFuncArr[1][0] = F_FILEOPEN;
	rMenu.m_nCustMenuItemKeyArr [1][0] = 'O';		//Sept. 14, 2000 JEPRO できるだけ標準設定値に合わせるように変更 (F→O)
	rMenu.m_nCustMenuItemFuncArr[1][1] = F_FILESAVE;
	rMenu.m_nCustMenuItemKeyArr [1][1] = 'S';
	rMenu.m_nCustMenuItemFuncArr[1][2] = F_NEXTWINDOW;
	rMenu.m_nCustMenuItemKeyArr [1][2] = 'N';		//Sept. 14, 2000 JEPRO できるだけ標準設定値に合わせるように変更 (O→N)
	rMenu.m_nCustMenuItemFuncArr[1][3] = F_TOLOWER;
	rMenu.m_nCustMenuItemKeyArr [1][3] = 'L';
	rMenu.m_nCustMenuItemFuncArr[1][4] = F_TOUPPER;
	rMenu.m_nCustMenuItemKeyArr [1][4] = 'U';
	rMenu.m_nCustMenuItemFuncArr[1][5] = F_0;
	rMenu.m_nCustMenuItemKeyArr [1][5] = '\0';
	rMenu.m_nCustMenuItemFuncArr[1][6] = F_WINCLOSE;
	rMenu.m_nCustMenuItemKeyArr [1][6] = 'C';

	/* タブメニュー */	//@@@ 2003.06.14 MIK
	n = 0;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'S';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVEAS_DIALOG;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'A';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'R';	// 2007.06.26 ryoji B -> R
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE_OPEN;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'L';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_WINCLOSE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'C';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILE_REOPEN;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'W';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_GROUPCLOSE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'G';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSEOTHER;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'O';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSELEFT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'H';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSERIGHT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'M';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVERIGHT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '0';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVELEFT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '1';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_SEPARATE;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'E';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTNEXT;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'X';
	n++;
	rMenu.m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTPREV;
	rMenu.m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'V';
	n++;
	rMenu.m_nCustMenuItemNumArr[CUSTMENU_INDEX_FOR_TABWND] = n;
}

/* 言語選択後に共有メモリ内の文字列を更新する */
void CShareData::RefreshString()
{

	RefreshKeyAssignString( m_pShareData );
}

void CShareData::CreateTypeSettings()
{
	if( NULL == m_pvTypeSettings ){
		m_pvTypeSettings = new std::vector<STypeConfig*>();
	}
}

std::vector<STypeConfig*>& CShareData::GetTypeSettings()
{
	return *m_pvTypeSettings;
}


void CShareData::InitFileTree( SFileTree* setting )
{
	setting->m_bProject = true;
	for(int i = 0; i < (int)_countof(setting->m_aItems); i++){
		SFileTreeItem& item = setting->m_aItems[i];
		item.m_eFileTreeItemType = EFileTreeItemType_Grep;
		item.m_szTargetPath = _T("");
		item.m_szLabelName = _T("");
		item.m_szTargetPath = _T("");
		item.m_nDepth = 0;
		item.m_szTargetFile = _T("");
		item.m_bIgnoreHidden = true;
		item.m_bIgnoreReadOnly = false;
		item.m_bIgnoreSystem = false;
	}
	setting->m_nItemCount = 1;
	setting->m_aItems[0].m_szTargetPath = _T(".");
	setting->m_aItems[0].m_szTargetFile = _T("*.*");
}
