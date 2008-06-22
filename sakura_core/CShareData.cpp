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

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CShareData.h"
#include "CControlTray.h"
#include "mymessage.h"
#include "debug/Debug.h"
#include "global.h"
#include "debug/CRunningTimer.h"
#include "charset/charcode.h"
#include <tchar.h>
#include "util/module.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "util/file.h"
#include "util/os.h"

struct ARRHEAD {
	int		nLength;
	int		nItemNum;
};

//!	共有メモリのバージョン
/*!
	共有メモリのバージョン番号。共有メモリの形式を変更したときはここを1増やす。

	この値は共有メモリのバージョンフィールドに格納され、異なる構造の共有メモリを
	使うエディタが同時に起動しないようにする。

	設定可能な値は 1～unsinged intの最大値

	@sa Init()

	Version 1～24:
	いろいろ
	
	Version 25:
	m_bStopsBothEndsWhenSearchWord追加
	
	Version 26:
	MacroRecに、m_bReloadWhenExecuteを追加 2002/03/11 YAZAKI
	EditInfoに、m_szDocType追加 Mar. 7, 2002 genta
	
	Version 27:
	STypeConfigに、m_szOutlineRuleFilenameを追加 2002.04.01 YAZAKI 
	
	Version 28:
	PRINTSETTINGに、m_bPrintKinsokuHead、m_bPrintKinsokuTailを追加 2002.04.09 MIK
	STypeConfigに、m_bKinsokuHead、m_bKinsokuTail、m_szKinsokuHead、m_szKinsokuTailを追加 2002.04.09 MIK

	Version 29:
	PRINTSETTINGに、m_bPrintKinsokuRetを追加 2002.04.13 MIK
	STypeConfigに、m_bKinsokuRetを追加 2002.04.13 MIK

	Version 30:
	PRINTSETTINGに、m_bPrintKinsokuKutoを追加 2002.04.17 MIK
	STypeConfigに、m_bKinsokuKutoを追加 2002.04.17 MIK

	Version 31:
	Commonに、m_bStopsBothEndsWhenSearchParagraphを追加 2002/04/26 YAZAKI

	Version 32:
	CommonからSTypeConfigへ、m_bAutoIndent、m_bAutoIndent_ZENSPACEを移動 2002/04/30 YAZAKI

	Version 33:
	Commonに、m_lf_khを追加 2002/05/21 ai
	m_nDiffFlgOptを追加 2002.05.27 MIK
	STypeConfig-ColorにCOLORIDX_DIFF_APPEND,COLORIDX_DIFF_CHANGE,COLORIDX_DIFF_DELETEを追加

	Version 34:
	STypeConfigにm_bUseDocumentIcon 追加． 2002.09.10 genta
	
	Version 35:
	Commonにm_nLineNumRightSpace 追加．2002.09.18 genta

	Version 36:
	Commonのm_bGrepKanjiCode_AutoDetectを削除、m_nGrepCharSetを追加 2002/09/21 Moca

	Version 37:
	STypeConfigのLineComment関連をm_cLineCommentに変更．  @@@ 2002.09.23 YAZAKI
	STypeConfigのBlockComment関連をm_cBlockCommentに変更．@@@ 2002.09.23 YAZAKI

	Version 38:
	STypeConfigにm_bIndentLayoutを追加. @@@ 2002.09.29 YAZAKI
	2002.10.01 genta m_nIndentLayoutに名前変更

	Version 39:
	Commonにm_nFUNCKEYWND_GroupNumを追加． 2002/11/04 Moca

	Version 40:
	ファイル名簡易表記関連を追加． 2002/12/08～2003/01/15 Moca

	Version 41:
	STypeConfigのm_szTabViewStringサイズ拡張
	m_nWindowSizeX/Y m_nWindowOriginX/Y追加 2003.01.26 aroka

	Version 42:
	STypeConfigに独自TABマークフラグ追加 2003.03.28 MIK

	Version 43:
	最近使ったファイル・フォルダにお気に入りを追加 2003.04.08 MIK

	Version 44:
	Window Caption文字列領域をCommonに追加 2003.04.05 genta

	Version 45:
	タグファイル作成用コマンドオプション保存領域(m_nTagsOpt,m_szTagsCmdLine)を追加 2003.05.12 MIK

	Version 46:
	編集ウインドウ数修正、タブウインドウ用情報追加

	Version 47:
	ファイルからの補完をSTypeConfigに追加 2003.06.28 Moca

	Version 48:
	Grepリアルタイム表示追加 2003.06.28 Moca

	Version 49:
	ファイル情報にIsDebug追加 (タブ表示用) 2003.10.13 MIK
	
	Version 50:
	ウィンドウ位置固定・継承を追加 2004.05.13 Moca

	Version 51:
	タグジャンプ機能追加 2004/06/21 novice

	Version 52:
	前回と異なる文字コードのときに問い合わせを行うかどうかのフラグ追加	2004.10.03 genta

	Version 53:
	存在しないファイルを開こうとした場合に警告するフラグの追加	2004.10.09 genta

	Version 54:
	マウスサイドボタン対応 2004/10/10 novice

	Version 55:
	マウス中ボタン対応 2004/10/11 novice

	Version 56:
	インクリメンタルサーチ(Migemo path用) 2004/10/13 isearch
	
	Version 57:
	強調キーワード指定拡大 2005/01/13 MIK
	
	Version 58:
	強調キーワードセット可変長割り当て 2005/01/25 Moca

	Version 59:
	マクロ数を増やした 2005/01/30 genta
	
	Version 60:
	キーワード指定タグジャンプ履歴保存 2005/04/03 MIK

	Version 61:
	改行で行末の空白を削除するオプション(タイプ別設定) 2005/10/11 ryoji

	Version 62:
	バックアップフォルダ 2005.11.07 aroka

	Version 63:
	指定桁縦線表示追加 2005.11.08 Moca

	Version 64:
	タブの機能拡張(等幅、アイコン表示) 2006/01/28 ryoji
	アウトプットウィンドウ位置 2006.02.01 aroka

	Version 65:
	タブ一覧をソートする 2006/05/10 ryoji

	Version 66:
	キーワードヘルプ機能拡張 2006.04.10 fon
		キーワードヘルプ機能設定を別タブに移動, 辞書の複数化に対応, キャレット位置キーワード検索追加

	Version 67:
	キャレット色指定を追加 2006.12.07 ryoji

	Version 68:
	ファイルダイアログのフィルタ設定 2006.11.16 ryoji

	Version 69:
	「すべて置換」は置換の繰返し 2006.11.16 ryoji

	Version 70:
	[すべて閉じる]で他に編集用のウィンドウがあれば確認する 2006.12.25 ryoji

	Version 71:
	タブを閉じる動作を制御するオプション2つを追加 2007.02.11 genta
		m_bRetainEmptyTab, m_bCloseOneWinInTabMode

	Version 72:
	タブ一覧をフルパス表示する 2007.02.28 ryoji

	Version 73:
	編集ウィンドウ切替中 2007.04.03 ryoji

	Version 74:
	カーソル位置の文字列をデフォルトの検索文字列にする 2006.08.23 ryoji

	Version 75:
	マウスホイールでウィンドウ切り替え 2006.03.26 ryoji

	Version 76:
	タブのグループ化 2007.06.20 ryoji

	Version 77:
	iniフォルダ設定 2007.05.31 ryoji

	Version 78:
	エディタ－トレイ間でのUI特権分離確認のためのバージョン合わせ 2007.06.07 ryoji

	Version 79:
	外部コマンド実行のオプション拡張 2007.04.29 maru

	Version 80:
	正規表現ライブラリの切り替え 2007.07.22 genta

	Version 81:
	マウスクリックにてアクティベートされた時はカーソル位置を移動しない 2007.10.02 nasukoji

	Version 82:
	ラインモード貼り付けを可能にする 2007.10.08 ryoji

	Version 83:
	選択なしでコピーを可能にする 2007.11.18 ryoji

	Version 84:
	画面キャッシュを使う 2007.09.09 Moca

	Version 85:
	キーワードセット最大値増加 2007.12.01 genta

	Version 86:
	タイプ別設定最大値増加 2007.12.13 ryoji

	Version 87:
	テキストの折り返し方法追加 2008.05.30 nasukoji

	Version 1000:
	バージョン1000以降を本家統合までの間、使わせてください。かなり頻繁に構成が変更されると思われるので。by kobake 2008.03.02
*/

const unsigned int uShareDataVersion = 1020;

/*
||	Singleton風
*/
CShareData* CShareData::_instance = NULL;

/*! @brief CShareData::m_pEditArr保護用Mutex

	複数のエディタが非同期に一斉動作しているときでも、CShareData::m_pEditArrを
	安全に操作できるよう操作中はMutexをLock()する。

	@par（非同期一斉動作の例）
		多数のウィンドウを表示していてグループ化を有効にしたタスクバーで「グループを閉じる」操作をしたとき

	@par（保護する箇所の例）
		CShareData::AddEditWndList(): エントリの追加／並び替え
		CShareData::DeleteEditWndList(): エントリの削除
		CShareData::GetOpenedWindowArr(): 配列のコピー作成

	下手にどこにでも入れるとデッドロックする危険があるので入れるときは慎重に。
	（Lock()期間中にSendMessage()などで他ウィンドウの操作をすると危険性大）
	CShareData::m_pEditArrを直接参照したり変更するような箇所には潜在的な危険があるが、
	対話型で順次操作している範囲であればまず問題は起きない。

	@date 2007.07.05 ryoji 新規導入
	@date 2007.07.07 genta CShareDataのメンバへ移動
*/
CMutex CShareData::g_cEditArrMutex( FALSE, GSTR_MUTEX_SAKURA_EDITARR );

CShareData* CShareData::getInstance()
{
	return _instance;
}

/*!
	共有メモリ領域がある場合はプロセスのアドレス空間から､
	すでにマップされているファイル ビューをアンマップする。
*/
CShareData::~CShareData()
{
	if( NULL != m_pShareData ){
		/* プロセスのアドレス空間から､ すでにマップされているファイル ビューをアンマップします */
		::UnmapViewOfFile( m_pShareData );
		m_pShareData = NULL;
	}
	return;
}


//! CShareDataクラスの初期化処理
/*!
	CShareDataクラスを利用する前に必ず呼び出すこと。

	@retval true 初期化成功
	@retval false 初期化失敗

	@note 既に存在する共有メモリのバージョンがこのエディタが使うものと
	異なる場合は致命的エラーを防ぐためにfalseを返します。CProcess::Initialize()
	でInit()に失敗するとメッセージを出してエディタの起動を中止します。
*/
bool CShareData::Init( void )
{
	MY_RUNNINGTIMER(cRunningTimer,"CShareData::Init" );

	if (CShareData::_instance == NULL)	//	Singleton風
		CShareData::_instance = this;

	m_hwndTraceOutSource = NULL;	// 2006.06.26 ryoji

	/* ファイルマッピングオブジェクト */
	m_hFileMap = ::CreateFileMapping(
		INVALID_HANDLE_VALUE,	//	Sep. 6, 2003 wmlhq
		NULL,
		PAGE_READWRITE | SEC_COMMIT,
		0,
		sizeof( DLLSHAREDATA ),
		GSTR_CSHAREDATA
	);
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
		m_pShareData->OnInit();

		// 2007.05.19 ryoji 実行ファイルフォルダ->設定ファイルフォルダに変更
		TCHAR	szIniFolder[_MAX_PATH];
		m_pShareData->m_IniFolder.m_bInit = false;
		GetInidir( szIniFolder );
		AddLastChar( szIniFolder, _MAX_PATH, _T('\\') );

		m_pShareData->m_vStructureVersion = uShareDataVersion;
		_tcscpy(m_pShareData->m_szKeyMacroFileName, _T(""));	/* キーワードマクロのファイル名 */ //@@@ 2002.1.24 YAZAKI
		m_pShareData->m_bRecordingKeyMacro = FALSE;		/* キーボードマクロの記録中 */
		m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

		// 2004.05.13 Moca リソースから製品バージョンの取得
		GetAppVersionInfo( NULL, VS_VERSION_INFO,
			&m_pShareData->m_dwProductVersionMS, &m_pShareData->m_dwProductVersionLS );
		m_pShareData->m_hwndTray = NULL;
		m_pShareData->m_hAccel = NULL;
		m_pShareData->m_hwndDebug = NULL;
		m_pShareData->m_nSequences = 0;					/* ウィンドウ連番 */
		m_pShareData->m_nGroupSequences = 0;			/* タブグループ連番 */	// 2007.06.20 ryoji
		m_pShareData->m_nEditArrNum = 0;

		m_pShareData->m_bEditWndChanging = FALSE;	// 編集ウィンドウ切替中	// 2007.04.03 ryoji

		m_pShareData->m_Common.m_sGeneral.m_nMRUArrNum_MAX = 15;	/* ファイルの履歴MAX */	//Oct. 14, 2000 JEPRO 少し増やした(10→15)
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
		CMRU cMRU;
		cMRU.ClearAll();
		m_pShareData->m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX = 15;	/* フォルダの履歴MAX */	//Oct. 14, 2000 JEPRO 少し増やした(10→15)
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
		CMRUFolder cMRUFolder;
		cMRUFolder.ClearAll();

		m_pShareData->m_aSearchKeys.clear();
		m_pShareData->m_aReplaceKeys.clear();
		m_pShareData->m_aGrepFiles.clear();
		m_pShareData->m_aGrepFiles.push_back(_T("*.*"));
		m_pShareData->m_aGrepFolders.clear();

		_tcscpy( m_pShareData->m_szMACROFOLDER, szIniFolder );	/* マクロ用フォルダ */
		_tcscpy( m_pShareData->m_szIMPORTFOLDER, szIniFolder );	/* 設定インポート用フォルダ */

		for( int i = 0; i < MAX_TRANSFORM_FILENAME; ++i ){
			_tcscpy( m_pShareData->m_szTransformFileNameFrom[i], _T("") );
			_tcscpy( m_pShareData->m_szTransformFileNameTo[i], _T("") );
		}
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[0], _T("%DeskTop%\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[0],   _T("デスクトップ\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[1], _T("%Personal%\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[1],   _T("マイドキュメント\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[2], _T("%Cache%\\Content.IE5\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[2],   _T("IEキャッシュ\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[3], _T("%TEMP%\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[3],   _T("TEMP\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[4], _T("%Common DeskTop%\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[4],   _T("共有デスクトップ\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[5], _T("%Common Documents%\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameTo[5],   _T("共有ドキュメント\\") );
		_tcscpy( m_pShareData->m_szTransformFileNameFrom[6], _T("%AppData%\\") );	// 2007.05.19 ryoji 追加
		_tcscpy( m_pShareData->m_szTransformFileNameTo[6],   _T("アプリデータ\\") );	// 2007.05.19 ryoji 追加
		m_pShareData->m_nTransformFileNameArrNum = 7;
		
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

		//	Jan. 30, 2005 genta 関数として独立
		//	2007.11.04 genta 戻り値チェック．falseなら起動中断．
		if( ! InitKeyAssign( m_pShareData )){
			return false;
		}

//	From Here Sept. 19, 2000 JEPRO コメントアウトになっていた初めのブロックを復活しその下をコメントアウト
//	MS ゴシック標準スタイル10ptに設定
//		/* LOGFONTの初期化 */
		memset_raw( &m_pShareData->m_Common.m_sView.m_lf, 0, sizeof( m_pShareData->m_Common.m_sView.m_lf ) );
		m_pShareData->m_Common.m_sView.m_lf.lfHeight			= -13;
		m_pShareData->m_Common.m_sView.m_lf.lfWidth				= 0;
		m_pShareData->m_Common.m_sView.m_lf.lfEscapement		= 0;
		m_pShareData->m_Common.m_sView.m_lf.lfOrientation		= 0;
		m_pShareData->m_Common.m_sView.m_lf.lfWeight			= 400;
		m_pShareData->m_Common.m_sView.m_lf.lfItalic			= 0x0;
		m_pShareData->m_Common.m_sView.m_lf.lfUnderline			= 0x0;
		m_pShareData->m_Common.m_sView.m_lf.lfStrikeOut			= 0x0;
		m_pShareData->m_Common.m_sView.m_lf.lfCharSet			= 0x80;
		m_pShareData->m_Common.m_sView.m_lf.lfOutPrecision		= 0x3;
		m_pShareData->m_Common.m_sView.m_lf.lfClipPrecision		= 0x2;
		m_pShareData->m_Common.m_sView.m_lf.lfQuality			= 0x1;
		m_pShareData->m_Common.m_sView.m_lf.lfPitchAndFamily	= 0x31;
		_tcscpy( m_pShareData->m_Common.m_sView.m_lf.lfFaceName, _T("ＭＳ ゴシック") );

		InitCharWidthCacheCommon();								// 2008/5/17 Uchi

		// キーワードヘルプのフォント ai 02/05/21 Add S
		::SystemParametersInfo(
			SPI_GETICONTITLELOGFONT,				// system parameter to query or set
			sizeof(LOGFONT),						// depends on action to be taken
			(PVOID)&m_pShareData->m_Common.m_sHelper.m_lf_kh,	// depends on action to be taken
			NULL									// user profile update flag
		);
		// ai 02/05/21 Add E

//	To Here Sept. 19,2000

		m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH = TRUE;				/* 現在のフォントは固定幅フォントである */

//		m_pShareData->m_Common.m_bUseCaretKeyWord = FALSE;		/* キャレット位置の単語を辞書検索-機能OFF */	// 2006.03.24 fon sakura起動ごとFALSEとし、初期化しない


		/* バックアップ */
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

		m_pShareData->m_Common.m_sGeneral.m_nCaretType = 0;					/* カーソルのタイプ 0=win 1=dos */
		m_pShareData->m_Common.m_sGeneral.m_bIsINSMode = true;				/* 挿入／上書きモード */
		m_pShareData->m_Common.m_sGeneral.m_bIsFreeCursorMode = false;		/* フリーカーソルモードか */	//Oct. 29, 2000 JEPRO 「なし」に変更

		m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord = FALSE;	/* 単語単位で移動するときに、単語の両端で止まるか */
		m_pShareData->m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph = FALSE;	/* 単語単位で移動するときに、単語の両端で止まるか */

		m_pShareData->m_Common.m_sSearch.m_sSearchOption.Reset();			// 検索オプション
		m_pShareData->m_Common.m_sSearch.m_bConsecutiveAll = 0;			// 「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
		m_pShareData->m_Common.m_sSearch.m_bSelectedArea = FALSE;			// 選択範囲内置換
		m_pShareData->m_Common.m_sHelper.m_szExtHelp[0] = L'\0';			// 外部ヘルプ１
		m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp[0] = L'\0';		// 外部HTMLヘルプ
		
		m_pShareData->m_Common.m_sHelper.m_szMigemoDll[0] = L'\0';			/* migemo dll */
		m_pShareData->m_Common.m_sHelper.m_szMigemoDict[0] = L'\0';		/* migemo dict */

		m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = TRUE;		/* 検索／置換  見つからないときメッセージを表示 */

		m_pShareData->m_Common.m_sGeneral.m_bCloseAllConfirm = FALSE;		/* [すべて閉じる]で他に編集用のウィンドウがあれば確認する */	// 2006.12.25 ryoji
		m_pShareData->m_Common.m_sGeneral.m_bExitConfirm = FALSE;			/* 終了時の確認をする */
		m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum = CLayoutInt(3);	/* キーリピート時のスクロール行数 */
		m_pShareData->m_Common.m_sGeneral.m_nRepeatedScroll_Smooth = FALSE;/* キーリピート時のスクロールを滑らかにするか */

		m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy = FALSE;			/* 折り返し行に改行を付けてコピー */
		m_pShareData->m_Common.m_sSearch.m_bGrepSubFolder = TRUE;			/* Grep: サブフォルダも検索 */
		m_pShareData->m_Common.m_sSearch.m_bGrepOutputLine = TRUE;			/* Grep: 行を出力するか該当部分だけ出力するか */
		m_pShareData->m_Common.m_sSearch.m_nGrepOutputStyle = 1;			/* Grep: 出力形式 */
		m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder=FALSE;		/* Grep: フォルダの初期値をカレントフォルダにする */
		m_pShareData->m_Common.m_sSearch.m_nGrepCharSet = CODE_AUTODETECT;	/* Grep: 文字コードセット */
		m_pShareData->m_Common.m_sSearch.m_bGrepRealTimeView = FALSE;		/* 2003.06.28 Moca Grep結果のリアルタイム表示 */
		m_pShareData->m_Common.m_sSearch.m_bCaretTextForSearch = TRUE;		/* 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする */
		m_pShareData->m_Common.m_sSearch.m_szRegexpLib[0] =_T('\0');		/* 2007.08.12 genta 正規表現DLL */
		m_pShareData->m_Common.m_sSearch.m_bGTJW_RETURN = TRUE;				/* エンターキーでタグジャンプ */
		m_pShareData->m_Common.m_sSearch.m_bGTJW_LDBLCLK = TRUE;			/* ダブルクリックでタグジャンプ */

//キーワード：ツールバー順序
		//	Jan. 30, 2005 genta 関数として独立
		InitToolButtons( m_pShareData );

		m_pShareData->m_Common.m_sWindow.m_bDispTOOLBAR = TRUE;			/* 次回ウィンドウを開いたときツールバーを表示する */
		m_pShareData->m_Common.m_sWindow.m_bDispSTATUSBAR = TRUE;			/* 次回ウィンドウを開いたときステータスバーを表示する */
		m_pShareData->m_Common.m_sWindow.m_bDispFUNCKEYWND = FALSE;		/* 次回ウィンドウを開いたときファンクションキーを表示する */
		m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place = 1;			/* ファンクションキー表示位置／0:上 1:下 */
		m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum = 4;			// 2002/11/04 Moca ファンクションキーのグループボタン数

		m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd = FALSE;			//タブウインドウ表示	//@@@ 2003.05.31 MIK
		m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin = FALSE;	//タブウインドウ表示	//@@@ 2003.05.31 MIK
		wcscpy(	//@@@ 2003.06.13 MIK
			m_pShareData->m_Common.m_sTabBar.m_szTabWndCaption,
			L"${w?【Grep】$h$:【アウトプット】$:$f$}${U?(更新)$}${R?(ビューモード)$:(上書き禁止)$}${M?【キーマクロの記録中】$}"
		);
		m_pShareData->m_Common.m_sTabBar.m_bSameTabWidth = FALSE;			//タブを等幅にする			//@@@ 2006.01.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bDispTabIcon = FALSE;			//タブにアイコンを表示する	//@@@ 2006.01.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bSortTabList = TRUE;			//タブ一覧をソートする		//@@@ 2006.05.10 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bTab_RetainEmptyWin = TRUE;	// 最後のファイルが閉じられたとき(無題)を残す	// 2007.02.11 genta
		m_pShareData->m_Common.m_sTabBar.m_bTab_CloseOneWin = FALSE;	// タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる	// 2007.02.11 genta
		m_pShareData->m_Common.m_sTabBar.m_bTab_ListFull = FALSE;			//タブ一覧をフルパス表示する	//@@@ 2007.02.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bChgWndByWheel = FALSE;		//マウスホイールでウィンドウ切替	//@@@ 2006.03.26 ryoji

		m_pShareData->m_Common.m_sWindow.m_bSplitterWndHScroll = TRUE;	// 2001/06/20 asa-o 分割ウィンドウの水平スクロールの同期をとる
		m_pShareData->m_Common.m_sWindow.m_bSplitterWndVScroll = TRUE;	// 2001/06/20 asa-o 分割ウィンドウの垂直スクロールの同期をとる

		/* カスタムメニュー情報 */
		auto_sprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[0], LTEXT("右クリックメニュー") );
		for( int i = 1; i < MAX_CUSTOM_MENU; ++i ){
			auto_sprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[i], LTEXT("メニュー%d"), i );
			m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[i] = 0;
			for( int j = 0; j < MAX_CUSTOM_MENU_ITEMS; ++j ){
				m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[i][j] = F_0;
				m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr [i][j] = '\0';
			}
		}
		auto_sprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[CUSTMENU_INDEX_FOR_TABWND], LTEXT("タブメニュー") );	//@@@ 2003.06.13 MIK


		/* 見出し記号 */
		wcscpy( m_pShareData->m_Common.m_sFormat.m_szMidashiKigou, L"１２３４５６７８９０（(［[「『【■□▲△▼▽◆◇○◎●§・※☆★第①②③④⑤⑥⑦⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩ一二三四五六七八九十壱弐参伍" );
		/* 引用符 */
		wcscpy( m_pShareData->m_Common.m_sFormat.m_szInyouKigou, L"> " );		/* 引用符 */
		m_pShareData->m_Common.m_sHelper.m_bUseHokan = FALSE;					/* 入力補完機能を使用する */

		// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除
		//	2004.05.13 Moca ウィンドウサイズ固定指定追加に伴う指定方法変更
		m_pShareData->m_Common.m_sWindow.m_nSaveWindowSize = WINSIZEMODE_SAVE;	// ウィンドウサイズ継承
		m_pShareData->m_Common.m_sWindow.m_nWinSizeType = SIZE_RESTORED;
		m_pShareData->m_Common.m_sWindow.m_nWinSizeCX = CW_USEDEFAULT;
		m_pShareData->m_Common.m_sWindow.m_nWinSizeCY = 0;
		
		//	2004.05.13 Moca ウィンドウ位置
		m_pShareData->m_Common.m_sWindow.m_nSaveWindowPos = WINSIZEMODE_DEF;		// ウィンドウ位置固定・継承
		m_pShareData->m_Common.m_sWindow.m_nWinPosX = CW_USEDEFAULT;
		m_pShareData->m_Common.m_sWindow.m_nWinPosY = 0;

		m_pShareData->m_Common.m_sGeneral.m_bUseTaskTray = TRUE;				/* タスクトレイのアイコンを使う */
		m_pShareData->m_Common.m_sGeneral.m_bStayTaskTray = TRUE;				/* タスクトレイのアイコンを常駐 */
		m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyCode = L'Z';		/* タスクトレイ左クリックメニュー キー */
		m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods = HOTKEYF_ALT | HOTKEYF_CONTROL;	/* タスクトレイ左クリックメニュー キー */
		m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop = TRUE;			/* OLEによるドラッグ & ドロップを使う */
		m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource = TRUE;			/* OLEによるドラッグ元にするか */
		m_pShareData->m_Common.m_sGeneral.m_bDispExitingDialog = FALSE;		/* 終了ダイアログを表示する */
		m_pShareData->m_Common.m_sEdit.m_bSelectClickedURL = TRUE;			/* URLがクリックされたら選択するか */
		m_pShareData->m_Common.m_sSearch.m_bGrepExitConfirm = FALSE;			/* Grepモードで保存確認するか */
//		m_pShareData->m_Common.m_bRulerDisp = TRUE;					/* ルーラー表示 */
		m_pShareData->m_Common.m_sWindow.m_nRulerHeight = 13;					/* ルーラーの高さ */
		m_pShareData->m_Common.m_sWindow.m_nRulerBottomSpace = 0;				/* ルーラーとテキストの隙間 */
		m_pShareData->m_Common.m_sWindow.m_nRulerType = 0;					/* ルーラーのタイプ */
		//	Sep. 18, 2002 genta
		m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace = 0;			/* 行番号の右の隙間 */
		m_pShareData->m_Common.m_sWindow.m_nVertLineOffset = -1;			// 2005.11.10 Moca 指定桁縦線
		m_pShareData->m_Common.m_sWindow.m_bUseCompotibleBMP = FALSE;		// 2007.09.09 Moca 画面キャッシュを使う
		m_pShareData->m_Common.m_sEdit.m_bCopyAndDisablSelection = FALSE;	/* コピーしたら選択解除 */
		m_pShareData->m_Common.m_sEdit.m_bEnableNoSelectCopy = TRUE;		/* 選択なしでコピーを可能にする */	// 2007.11.18 ryoji
		m_pShareData->m_Common.m_sEdit.m_bEnableLineModePaste = TRUE;		/* ラインモード貼り付けを可能にする */	// 2007.10.08 ryoji
		m_pShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle = TRUE;		/* HtmlHelpビューアはひとつ */
		m_pShareData->m_Common.m_sCompare.m_bCompareAndTileHorz = TRUE;		/* 文書比較後、左右に並べて表示 */

		//[ファイル]タブ
		{
			//ファイルの排他制御
			m_pShareData->m_Common.m_sFile.m_nFileShareMode = SHAREMODE_DENY_WRITE;	// ファイルの排他制御モード
			m_pShareData->m_Common.m_sFile.m_bCheckFileTimeStamp = true;			// 更新の監視

			//ファイルの保存
			m_pShareData->m_Common.m_sFile.m_bEnableUnmodifiedOverwrite = false;	// 無変更でも上書きするか

			// 「名前を付けて保存」でファイルの種類が[ユーザ指定]のときのファイル一覧表示	//ファイル保存ダイアログのフィルタ設定	// 2006.11.16 ryoji
			m_pShareData->m_Common.m_sFile.m_bNoFilterSaveNew = true;		// 新規から保存時は全ファイル表示
			m_pShareData->m_Common.m_sFile.m_bNoFilterSaveFile = true;		// 新規以外から保存時は全ファイル表示

			//ファイルオープン
			m_pShareData->m_Common.m_sFile.m_bDropFileAndClose = false;		// ファイルをドロップしたときは閉じて開く
			m_pShareData->m_Common.m_sFile.m_nDropFileNumMax = 8;			// 一度にドロップ可能なファイル数
			m_pShareData->m_Common.m_sFile.m_bRestoreCurPosition = true;	// カーソル位置復元	//	Oct. 27, 2000 genta
			m_pShareData->m_Common.m_sFile.m_bRestoreBookmarks = true;		// ブックマーク復元	//2002.01.16 hor
			m_pShareData->m_Common.m_sFile.m_bAutoMIMEdecode = false;		// ファイル読み込み時にMIMEのデコードを行うか	//Jul. 13, 2001 JEPRO
			m_pShareData->m_Common.m_sFile.m_bQueryIfCodeChange = true;		// 前回と異なる文字コードの時に問い合わせを行うか	Oct. 03, 2004 genta
			m_pShareData->m_Common.m_sFile.m_bAlertIfFileNotExist = false;	// 開こうとしたファイルが存在しないとき警告する	Oct. 09, 2004 genta
		}

		m_pShareData->m_Common.m_sEdit.m_bNotOverWriteCRLF = TRUE;			/* 改行は上書きしない */
		::SetRect( &m_pShareData->m_Common.m_sOthers.m_rcOpenDialog, 0, 0, 0, 0 );	/* 「開く」ダイアログのサイズと位置 */
		m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind = TRUE;			/* 検索ダイアログを自動的に閉じる */
		m_pShareData->m_Common.m_sSearch.m_bSearchAll		 = FALSE;			/* 検索／置換／ブックマーク  先頭（末尾）から再検索 2002.01.26 hor */
		m_pShareData->m_Common.m_sWindow.m_bScrollBarHorz = TRUE;				/* 水平スクロールバーを使う */
		m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList = FALSE;		/* アウトライン ダイアログを自動的に閉じる */	//Nov. 18, 2000 JEPRO TRUE→FALSE に変更
		m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgReplace = TRUE;		/* 置換 ダイアログを自動的に閉じる */
		m_pShareData->m_Common.m_sEdit.m_bAutoColmnPaste = TRUE;			/* 矩形コピーのテキストは常に矩形貼り付け */
		m_pShareData->m_Common.m_sGeneral.m_bNoCaretMoveByActivation = FALSE;	/* マウスクリックにてアクティベートされた時はカーソル位置を移動しない 2007.10.02 nasukoji (add by genta) */

		m_pShareData->m_Common.m_sHelper.m_bHokanKey_RETURN	= TRUE;			/* VK_RETURN 補完決定キーが有効/無効 */
		m_pShareData->m_Common.m_sHelper.m_bHokanKey_TAB		= FALSE;		/* VK_TAB   補完決定キーが有効/無効 */
		m_pShareData->m_Common.m_sHelper.m_bHokanKey_RIGHT	= TRUE;			/* VK_RIGHT 補完決定キーが有効/無効 */
		m_pShareData->m_Common.m_sHelper.m_bHokanKey_SPACE	= FALSE;		/* VK_SPACE 補完決定キーが有効/無効 */

		m_pShareData->m_Common.m_sOutline.m_bMarkUpBlankLineEnable	=	FALSE;	//アウトラインダイアログでブックマークの空行を無視			2002.02.08 aroka,hor
		m_pShareData->m_Common.m_sOutline.m_bFunclistSetFocusOnJump	=	FALSE;	//アウトラインダイアログでジャンプしたらフォーカスを移す	2002.02.08 hor

		/*
			書式指定子の意味はWindows SDKのGetDateFormat(), GetTimeFormat()を参照のこと
		*/

		m_pShareData->m_Common.m_sFormat.m_nDateFormatType = 0;	//日付書式のタイプ
		_tcscpy( m_pShareData->m_Common.m_sFormat.m_szDateFormat, _T("yyyy\'年\'M\'月\'d\'日(\'dddd\')\'") );	//日付書式
		m_pShareData->m_Common.m_sFormat.m_nTimeFormatType = 0;	//時刻書式のタイプ
		_tcscpy( m_pShareData->m_Common.m_sFormat.m_szTimeFormat, _T("tthh\'時\'mm\'分\'ss\'秒\'")  );			//時刻書式

		m_pShareData->m_Common.m_sWindow.m_bMenuIcon = TRUE;		/* メニューにアイコンを表示する */

		// [ステータスバー]タブ
		// 表示文字コードの指定		2008/6/21	Uchi
		m_pShareData->m_Common.m_sStatusbar.m_bDispUniInSjis		= FALSE;	// SJISで文字コード値をUnicodeで表示する
		m_pShareData->m_Common.m_sStatusbar.m_bDispUniInJis			= FALSE;	// JISで文字コード値をUnicodeで表示する
		m_pShareData->m_Common.m_sStatusbar.m_bDispUniInEuc			= FALSE;	// EUCで文字コード値をUnicodeで表示する
		m_pShareData->m_Common.m_sStatusbar.m_bDispUtf8Codepoint	= TRUE;		// UTF-8をコードポイントで表示する
		m_pShareData->m_Common.m_sStatusbar.m_bDispSPCodepoint		= TRUE;		// サロゲートペアをコードポイントで表示する


		m_pShareData->m_aCommands.clear();

		InitKeyword( m_pShareData );
		InitTypeConfigs( m_pShareData );
		InitPopupMenu( m_pShareData );

		//	Apr. 05, 2003 genta ウィンドウキャプションの初期値
		//	Aug. 16, 2003 genta $N(ファイル名省略表示)をデフォルトに変更
		_tcscpy( m_pShareData->m_Common.m_sWindow.m_szWindowCaptionActive, 
			_T("${w?$h$:アウトプット$:${I?$f$:$N$}$}${U?(更新)$} -")
			_T(" $A $V ${R?(ビューモード)$:（上書き禁止）$}${M?  【キーマクロの記録中】$}") );
		_tcscpy( m_pShareData->m_Common.m_sWindow.m_szWindowCaptionInactive, 
			_T("${w?$h$:アウトプット$:$f$}${U?(更新)$} -")
			_T(" $A $V ${R?(ビューモード)$:（上書き禁止）$}${M?  【キーマクロの記録中】$}") );

		//	From Here Sep. 14, 2001 genta
		//	Macro登録の初期化
		MacroRec *mptr = m_pShareData->m_MacroTable;
		for( int i = 0; i < MAX_CUSTMACRO; ++i, ++mptr ){
			mptr->m_szName[0] = L'\0';
			mptr->m_szFile[0] = L'\0';
			mptr->m_bReloadWhenExecute = false;
		}
		//	To Here Sep. 14, 2001 genta

		// 2004/06/21 novice タグジャンプ機能追加
		m_pShareData->m_TagJumpNum = 0;
		// 2004.06.22 Moca タグジャンプの先頭
		m_pShareData->m_TagJumpTop = 0;
//@@@ 2002.01.08 YAZAKI 設定を保存するためにShareDataに移動
//		m_pShareData->m_bGetStdout = TRUE;	/* 外部コマンド実行の「標準出力を得る」 */
		m_pShareData->m_nExecFlgOpt = 1;	/* 外部コマンド実行の「標準出力を得る」 */	// 2006.12.03 maru オプションの拡張のため
		m_pShareData->m_bLineNumIsCRLF = TRUE;	/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */

		m_pShareData->m_nDiffFlgOpt = 0;	/* DIFF差分表示 */	//@@@ 2002.05.27 MIK

		m_pShareData->m_nTagsOpt = 0;	/* CTAGS */	//@@@ 2003.05.12 MIK
		_tcscpy( m_pShareData->m_szTagsCmdLine, _T("") );	/* CTAGS */	//@@@ 2003.05.12 MIK
		//From Here 2005.04.03 MIK キーワード指定タグジャンプのHistory保管
		m_pShareData->m_aTagJumpKeywords.clear();
		m_pShareData->m_bTagJumpICase = FALSE;
		m_pShareData->m_bTagJumpAnyWhere = FALSE;
		//To Here 2005.04.03 MIK 

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
		m_pShareData->OnInit();

		InitCharWidthCache(m_pShareData->m_Common.m_sView.m_lf);	// 2008/5/15 Uchi

		//	From Here Oct. 27, 2000 genta
		if( m_pShareData->m_vStructureVersion != uShareDataVersion ){
			//	この共有データ領域は使えない．
			//	ハンドルを解放する
			::UnmapViewOfFile( m_pShareData );
			m_pShareData = NULL;
			return false;
		}
		//	To Here Oct. 27, 2000 genta
	}
	return true;
}






/*!
	ファイル名から、ドキュメントタイプ（数値）を取得する
	
	@param pszFilePath [in] ファイル名
	
	拡張子を切り出して GetDocumentTypeExt に渡すだけ．
*/
CTypeConfig CShareData::GetDocumentType( const TCHAR* pszFilePath )
{
	TCHAR	szExt[_MAX_EXT];

	if( NULL != pszFilePath && 0 < (int)_tcslen( pszFilePath ) ){
		_tsplitpath( pszFilePath, NULL, NULL, NULL, szExt );
		if( szExt[0] == _T('.') )
			return GetDocumentTypeExt( szExt + 1 );
		else
			return GetDocumentTypeExt( szExt );
	}
	return CTypeConfig(0);
}


/*!
	拡張子から、ドキュメントタイプ（数値）を取得する
	
	@param pszExt [in] 拡張子 (先頭の,は含まない)
	
	指定された拡張子の属する文書タイプ番号を返す．
	とりあえず今のところはタイプは拡張子のみに依存すると仮定している．
	ファイル全体の形式に対応させるときは，また考え直す．
*/
CTypeConfig CShareData::GetDocumentTypeExt( const TCHAR* pszExt )
{
	const TCHAR	pszSeps[] = _T(" ;,");	// separator

	int		i;
	TCHAR*	pszToken;
	TCHAR	szText[256];

	for( i = 0; i < MAX_TYPES; ++i ){
		CTypeConfig nType(i);
		_tcscpy( szText, m_pShareData->GetTypeSetting(nType).m_szTypeExts );
		pszToken = _tcstok( szText, pszSeps );
		while( NULL != pszToken ){
			if( 0 == _tcsicmp( pszExt, pszToken ) ){
				return CTypeConfig(i);	//	番号
			}
			pszToken = _tcstok( NULL, pszSeps );
		}
	}
	return CTypeConfig(0);	//	ハズレ
}





/** 編集ウィンドウリストへの登録

	@param hWnd   [in] 登録する編集ウィンドウのハンドル
	@param nGroup [in] 新規登録の場合のグループID

	@date 2003.06.28 MIK CRecent利用で書き換え
	@date 2007.06.20 ryoji 新規ウィンドウにはグループIDを付与する
*/
BOOL CShareData::AddEditWndList( HWND hWnd, int nGroup/* = 0*/ )
{
	int		nSubCommand = TWNT_ADD;
	int		nIndex;
	EditNode	sMyEditNode;
	EditNode	*p;

	memset_raw( &sMyEditNode, 0, sizeof( sMyEditNode ) );
	sMyEditNode.m_hWnd = hWnd;

	{	// 2007.07.07 genta Lock領域
	LockGuard<CMutex> guard( g_cEditArrMutex );

	CRecentEditNode	cRecentEditNode;

	//登録済みか？
	nIndex = cRecentEditNode.FindItemByHwnd( hWnd );
	if( -1 != nIndex )
	{
		//もうこれ以上登録できないか？
		if( cRecentEditNode.GetItemCount() >= cRecentEditNode.GetArrayCount() )
		{
			cRecentEditNode.Terminate();
			return FALSE;
		}
		nSubCommand = TWNT_ORDER;

		//以前の情報をコピーする。
		p = cRecentEditNode.GetItem( nIndex );
		if( p )
		{
			memcpy_raw( &sMyEditNode, p, sizeof( sMyEditNode ) );
		}
	}

	/* ウィンドウ連番 */

	if( 0 == ::GetWindowLongPtr( hWnd, sizeof(LONG_PTR) ) )
	{
		m_pShareData->m_nSequences++;
		::SetWindowLongPtr( hWnd, sizeof(LONG_PTR) , (LONG_PTR)m_pShareData->m_nSequences );

		//連番を更新する。
		sMyEditNode.m_nIndex = m_pShareData->m_nSequences;

		/* タブグループ連番 */
		if( nGroup > 0 )
		{
			sMyEditNode.m_nGroup = nGroup;	// 指定のグループ
		}
		else
		{
			p = cRecentEditNode.GetItem( 0 );
			if( NULL == p )
				sMyEditNode.m_nGroup = ++m_pShareData->m_nGroupSequences;	// 新規グループ
			else
				sMyEditNode.m_nGroup = p->m_nGroup;	// 最近アクティブのグループ
		}

		sMyEditNode.m_showCmdRestore = ::IsZoomed(hWnd)? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
		sMyEditNode.m_bClosing = FALSE;
	}

	//追加または先頭に移動する。
	cRecentEditNode.AppendItem( &sMyEditNode );
	cRecentEditNode.Terminate();
	}	// 2007.07.07 genta Lock領域終わり

	//ウインドウ登録メッセージをブロードキャストする。
	PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY, (WPARAM)nSubCommand, (LPARAM)hWnd, hWnd, GetGroupId( hWnd ) );

	return TRUE;
}





/** 編集ウィンドウリストからの削除

	@date 2003.06.28 MIK CRecent利用で書き換え
	@date 2007.07.05 ryoji mutexで保護
*/
void CShareData::DeleteEditWndList( HWND hWnd )
{
	int nGroup = GetGroupId( hWnd );

	//ウインドウをリストから削除する。
	{	// 2007.07.07 genta Lock領域
		LockGuard<CMutex> guard( g_cEditArrMutex );

		CRecentEditNode	cRecentEditNode;
DBPRINT_A("cnt %d",cRecentEditNode.GetItemCount());
DBPRINT_A("DeleteItemByHwnd %08X",hWnd);
		cRecentEditNode.DeleteItemByHwnd( hWnd );
DBPRINT_A("/cnt %d",cRecentEditNode.GetItemCount());
		cRecentEditNode.Terminate();
	}

	//ウインドウ削除メッセージをブロードキャストする。
	PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY, (WPARAM)TWNT_DEL, (LPARAM)hWnd, hWnd, nGroup );

	return;
}

/** グループをIDリセットする

	@date 2007.06.20 ryoji
*/
void CShareData::ResetGroupId( void )
{
	int nGroup;
	int	i;

	nGroup = ++m_pShareData->m_nGroupSequences;
	for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
	{
		if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) )
		{
			m_pShareData->m_pEditArr[i].m_nGroup = nGroup;
		}
	}
}

/** 編集ウィンドウ情報を取得する

	@date 2007.06.20 ryoji

	@warning この関数はm_pEditArr内の要素へのポインタを返す．
	m_pEditArrが変更された後ではアクセスしないよう注意が必要．
*/
EditNode* CShareData::GetEditNode( HWND hWnd )
{
	int	i;

	for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
	{
		if( hWnd == m_pShareData->m_pEditArr[i].m_hWnd )
		{
			if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) )
				return &m_pShareData->m_pEditArr[i];
		}
	}

	return NULL;
}

/** グループIDを取得する

	@date 2007.06.20 ryoji
*/
int CShareData::GetGroupId( HWND hWnd )
{
	EditNode* pEditNode;
	pEditNode = GetEditNode( hWnd );
	return (pEditNode != NULL)? pEditNode->m_nGroup: -1;
}

/** 同一グループかどうかを調べる

	@param[in] hWnd1 比較するウィンドウ1
	@param[in] hWnd2 比較するウィンドウ2
	
	@return 2つのウィンドウが同一グループに属していればtrue

	@date 2007.06.20 ryoji
*/
bool CShareData::IsSameGroup( HWND hWnd1, HWND hWnd2 )
{
	int nGroup1;
	int nGroup2;

	if( hWnd1 == hWnd2 )
		return true;

	nGroup1 = GetGroupId( hWnd1 );
	if( nGroup1 < 0 )
		return false;

	nGroup2 = GetGroupId( hWnd2 );
	if( nGroup2 < 0 )
		return false;

	return ( nGroup1 == nGroup2 );
}

/** 指定位置の編集ウィンドウ情報を取得する

	@date 2007.06.20 ryoji
*/
EditNode* CShareData::GetEditNodeAt( int nGroup, int nIndex )
{
	int	i;
	int iIndex;

	iIndex = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
	{
		if( nGroup == 0 || nGroup == m_pShareData->m_pEditArr[i].m_nGroup )
		{
			if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) )
			{
				if( iIndex == nIndex )
					return &m_pShareData->m_pEditArr[i];
				iIndex++;
			}
		}
	}

	return NULL;
}

/** 先頭の編集ウィンドウ情報を取得する

	@date 2007.06.20 ryoji
*/
EditNode* CShareData::GetTopEditNode( HWND hWnd )
{
	int nGroup;

	nGroup = GetGroupId( hWnd );
	return GetEditNodeAt( nGroup, 0 );
}

/** 先頭の編集ウィンドウを取得する

	@return 与えられたエディタウィンドウと同一グループに属す
	先頭ウィンドウのハンドル

	@date 2007.06.20 ryoji
*/
HWND CShareData::GetTopEditWnd( HWND hWnd )
{
	EditNode* p = GetTopEditNode( hWnd );

	return ( p != NULL )? p->m_hWnd: NULL;
}

/* 共有データのロード */
BOOL CShareData::LoadShareData( void )
{
	return ShareData_IO_2( true );
}




/* 共有データの保存 */
void CShareData::SaveShareData( void )
{
	ShareData_IO_2( false );
	return;
}




/** 全編集ウィンドウへ終了要求を出す

	@param bExit [in] TRUE: 編集の全終了 / FALSE: すべて閉じる
	@param nGroup [in] グループ指定（0:全グループ）

	@date 2007.02.13 ryoji 「編集の全終了」を示す引数(bExit)を追加
	@date 2007.06.22 ryoji nGroup引数を追加
*/
BOOL CShareData::RequestCloseAllEditor( BOOL bExit, int nGroup )
{
	EditNode*	pWndArr;
	int		i;
	int		n;

	n = GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	for( i = 0; i < n; ++i ){
		if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
			if( IsEditWnd( pWndArr[i].m_hWnd ) ){
				/* アクティブにする */
				ActivateFrameWindow( pWndArr[i].m_hWnd );
				/* トレイからエディタへの終了要求 */
				if( !::SendMessageAny( pWndArr[i].m_hWnd, MYWM_CLOSE, bExit, 0 ) ){	// 2007.02.13 ryoji bExitを引き継ぐ
					delete []pWndArr;
					return FALSE;
				}
			}
		}
	}

	delete []pWndArr;
	return TRUE;
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
	if( 0 ==  GetEditorWindowsNum( 0 ) ){
		return FALSE;
	}
	
	for( int i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			// トレイからエディタへの編集ファイル名要求通知
			::SendMessageAny( m_pShareData->m_pEditArr[i].m_hWnd, MYWM_GETFILEINFO, 1, 0 );
			pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;

			// 同一パスのファイルが既に開かれているか
			if( 0 == _tcsicmp( pfi->m_szPath, pszPath ) ){
				*phwndOwner = m_pShareData->m_pEditArr[i].m_hWnd;
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
		pfi = (EditInfo*)&m_pShareData->m_EditInfo_MYWM_GETFILEINFO;
		if(nCharCode != CODE_AUTODETECT){
			LPCTSTR pszCodeNameNew = CCodeTypeName(nCharCode).Normal();
			LPCTSTR pszCodeNameCur = CCodeTypeName(pfi->m_nCharCode).Normal();

			if(pszCodeNameCur && pszCodeNameNew){
				if(nCharCode != pfi->m_nCharCode){
					TopWarningMessage( *phwndOwner,
						_T("%ts\n\n\n既に開いているファイルを違う文字コードで開く場合は、\n")
						_T("ファイルメニューから「開き直す」を使用してください。\n")
						_T("\n")
						_T("現在の文字コードセット=[%ts]\n")
						_T("新しい文字コードセット=[%ts]"),
						pszPath,
						pszCodeNameCur,
						pszCodeNameNew
					);
				}
			}
			else{
				TopWarningMessage( *phwndOwner,
					_T("%ts\n\n多重オープンの確認で不明な文字コードが指定されました。\n")
					_T("\n")
					_T("現在の文字コードセット=%d [%ts]\n新しい文字コードセット=%d [%ts]"),
					pszPath,
					pfi->m_nCharCode,
					NULL==pszCodeNameCur?_T("不明"):pszCodeNameCur,
					nCharCode,
					NULL==pszCodeNameNew?_T("不明"):pszCodeNameNew
				);
			}
		}

		// 開いているウィンドウをアクティブにする
		ActivateFrameWindow( *phwndOwner );

		// MRUリストへの登録
		CMRU().Add( pfi );
		return TRUE;
	}
	else {
		return FALSE;
	}

}




/** 現在の編集ウィンドウの数を調べる

	@param nGroup [in] グループ指定（0:全グループ）
	@param bExcludeClosing [in] 終了中の編集ウィンドウはカウントしない

	@date 2007.06.22 ryoji nGroup引数を追加
	@date 2008.04.19 ryoji bExcludeClosing引数を追加
*/
int CShareData::GetEditorWindowsNum( int nGroup, bool bExcludeClosing/* = true */ )
{
	int		i;
	int		j;

	j = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			if( nGroup != 0 && nGroup != GetGroupId( m_pShareData->m_pEditArr[i].m_hWnd ) )
				continue;
			if( bExcludeClosing && m_pShareData->m_pEditArr[i].m_bClosing )
				continue;
			j++;
		}
	}
	return j;

}



/** 全編集ウィンドウへメッセージをポストする

	@param nGroup [in] グループ指定（0:全グループ）

	@date 2005.01.24 genta hWndLast == NULLのとき全くメッセージが送られなかった
	@date 2007.06.22 ryoji nGroup引数を追加、グループ単位で順番に送る
*/
BOOL CShareData::PostMessageToAllEditors(
	UINT		uMsg,		/*!< ポストするメッセージ */
	WPARAM		wParam,		/*!< 第1メッセージ パラメータ */
	LPARAM		lParam,		/*!< 第2メッセージ パラメータ */
	HWND		hWndLast,	/*!< 最後に送りたいウィンドウ */
	int			nGroup/* = 0*/	/*!< 送りたいグループ */
 )
{
	EditNode*	pWndArr;
	int		i;
	int		n;

	n = GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	// hWndLast以外へのメッセージ
	for( i = 0; i < n; ++i ){
		//	Jan. 24, 2005 genta hWndLast == NULLのときにメッセージが送られるように
		if( hWndLast == NULL || hWndLast != pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsEditWnd( pWndArr[i].m_hWnd ) ){
					/* メッセージをポスト */
					::PostMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	// hWndLastへのメッセージ
	for( i = 0; i < n; ++i ){
		if( hWndLast == pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsEditWnd( pWndArr[i].m_hWnd ) ){
					/* メッセージをポスト */
					::PostMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	delete []pWndArr;
	return TRUE;
}


/** 全編集ウィンドウへメッセージを送る

	@param nGroup [in] グループ指定（0:全グループ）

	@date 2005.01.24 genta m_hWndLast == NULLのとき全くメッセージが送られなかった
	@date 2007.06.22 ryoji nGroup引数を追加、グループ単位で順番に送る
*/
BOOL CShareData::SendMessageToAllEditors(
	UINT		uMsg,		/* ポストするメッセージ */
	WPARAM		wParam,		/* 第1メッセージ パラメータ */
	LPARAM		lParam,		/* 第2メッセージ パラメータ */
	HWND		hWndLast,	/* 最後に送りたいウィンドウ */
	int			nGroup/* = 0*/	/*!< 送りたいグループ */
 )
{
	EditNode*	pWndArr;
	int		i;
	int		n;

	n = GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	// hWndLast以外へのメッセージ
	for( i = 0; i < n; ++i ){
		//	Jan. 24, 2005 genta hWndLast == NULLのときにメッセージが送られるように
		if( hWndLast == NULL || hWndLast != pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsEditWnd( pWndArr[i].m_hWnd ) ){
					/* メッセージを送る */
					::SendMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	// hWndLastへのメッセージ
	for( i = 0; i < n; ++i ){
		if( hWndLast == pWndArr[i].m_hWnd ){
			if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
				if( IsEditWnd( pWndArr[i].m_hWnd ) ){
					/* メッセージを送る */
					::SendMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	delete []pWndArr;
	return TRUE;
}


/* 指定ウィンドウが、編集ウィンドウのフレームウィンドウかどうか調べる */
BOOL CShareData::IsEditWnd( HWND hWnd )
{
	TCHAR	szClassName[64];
	if( hWnd == NULL ){	// 2007.06.20 ryoji 条件追加
		return FALSE;
	}
	if( !::IsWindow( hWnd ) ){
		return FALSE;
	}
	if( 0 == ::GetClassName( hWnd, szClassName, _countof(szClassName) - 1 ) ){
		return FALSE;
	}
	if(0 == _tcscmp( GSTR_EDITWINDOWNAME, szClassName ) ){
		return TRUE;
	}else{
		return FALSE;
	}

}

// GetOpenedWindowArr用静的変数／構造体
static BOOL s_bSort;	// ソート指定
static BOOL s_bGSort;	// グループ指定
struct EditNodeEx{	// 拡張構造体
	EditNode* p;	// 編集ウィンドウ配列要素へのポインタ
	int nGroupMru;	// グループ単位のMRU番号
};

// GetOpenedWindowArr用ソート関数
static int __cdecl cmpGetOpenedWindowArr(const void *e1, const void *e2)
{
	// 異なるグループのときはグループ比較する
	int nGroup1;
	int nGroup2;

	if( s_bGSort )
	{
		// オリジナルのグループ番号のほうを見る
		nGroup1 = ((EditNodeEx*)e1)->p->m_nGroup;
		nGroup2 = ((EditNodeEx*)e2)->p->m_nGroup;
	}
	else
	{
		// グループのMRU番号のほうを見る
		nGroup1 = ((EditNodeEx*)e1)->nGroupMru;
		nGroup2 = ((EditNodeEx*)e2)->nGroupMru;
	}
	if( nGroup1 != nGroup2 )
	{
		return nGroup1 - nGroup2;	// グループ比較
	}

	// グループ比較が行われなかったときはウィンドウ比較する
	if( s_bSort )
		return ( ((EditNodeEx*)e1)->p->m_nIndex - ((EditNodeEx*)e2)->p->m_nIndex );	// ウィンドウ番号比較
	return ( ((EditNodeEx*)e1)->p - ((EditNodeEx*)e2)->p );	// ウィンドウMRU比較（ソートしない）
}

/** 現在開いている編集ウィンドウの配列を返す

	@param[out] ppEditNode 配列を受け取るポインタ
		戻り値が0の場合はNULLが返されるが，それを期待しないこと．
		また，不要になったらdelete []しなくてはならない．
	@param[in] bSort TRUE: ソートあり / FALSE: ソート無し
	@param[in]bGSort TRUE: グループソートあり / FALSE: グループソート無し

	もとの編集ウィンドウリストはソートしなければウィンドウのMRU順に並んでいる
	-------------------------------------------------
	bSort	bGSort	処理結果
	-------------------------------------------------
	FALSE	FALSE	グループMRU順－ウィンドウMRU順
	TRUE	FALSE	グループMRU順－ウィンドウ番号順
	FALSE	TRUE	グループ番号順－ウィンドウMRU順
	TRUE	TRUE	グループ番号順－ウィンドウ番号順
	-------------------------------------------------

	@return 配列の要素数を返す
	@note 要素数>0 の場合は呼び出し側で配列をdelete []してください

	@date 2003.06.28 MIK CRecent利用で書き換え
	@date 2007.06.20 ryoji bGroup引数追加、ソート処理を自前のものからqsortに変更
*/
int CShareData::GetOpenedWindowArr( EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */ )
{
	int nRet;

	LockGuard<CMutex> guard( g_cEditArrMutex );
	nRet = GetOpenedWindowArrCore( ppEditNode, bSort, bGSort );

	return nRet;
}

// GetOpenedWindowArr関数コア処理部
int CShareData::GetOpenedWindowArrCore( EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */ )
{
	//編集ウインドウ数を取得する。
	EditNodeEx *pNode;	// ソート処理用の拡張リスト
	int		nRowNum;	//編集ウインドウ数
	int		i;

	//編集ウインドウ数を取得する。
	*ppEditNode = NULL;
	if( m_pShareData->m_nEditArrNum <= 0 )
		return 0;

	//編集ウインドウリスト格納領域を作成する。
	*ppEditNode = new EditNode[ m_pShareData->m_nEditArrNum ];
	if( NULL == *ppEditNode )
		return 0;

	// 拡張リストを作成する
	pNode = new EditNodeEx[ m_pShareData->m_nEditArrNum ];
	if( NULL == pNode )
	{
		delete [](*ppEditNode);
		*ppEditNode = NULL;
		return 0;
	}

	// 拡張リストの各要素に編集ウィンドウリストの各要素へのポインタを格納する
	nRowNum = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
	{
		if( IsEditWnd( m_pShareData->m_pEditArr[ i ].m_hWnd ) )
		{
			pNode[ nRowNum ].p = &m_pShareData->m_pEditArr[ i ];	// ポインタ格納
			pNode[ nRowNum ].nGroupMru = -1;	// グループ単位のMRU番号初期化
			nRowNum++;
		}
	}
	if( nRowNum <= 0 )
	{
		delete []pNode;
		delete [](*ppEditNode);
		*ppEditNode = NULL;
		return 0;
	}

	// 拡張リスト上でグループ単位のMRU番号をつける
	if( !bGSort )
	{
		int iGroupMru = 0;	// グループ単位のMRU番号
		int nGroup = -1;
		for( i = 0; i < nRowNum; i++ )
		{
			if( pNode[ i ].nGroupMru == -1 && nGroup != pNode[ i ].p->m_nGroup )
			{
				nGroup = pNode[ i ].p->m_nGroup;
				iGroupMru++;
				pNode[ i ].nGroupMru = iGroupMru;	// MRU番号付与

				// 同一グループのウィンドウに同じMRU番号をつける
				int j;
				for( j = i + 1; j < nRowNum; j++ )
				{
					if( pNode[ j ].p->m_nGroup == nGroup )
						pNode[ j ].nGroupMru = iGroupMru;
				}
			}
		}
	}

	// 拡張リストをソートする
	// Note. グループが１個だけの場合は従来（bGSort 引数無し）と同じ結果が得られる
	//       （グループ化する設定でなければグループは１個）
	s_bSort = bSort;
	s_bGSort = bGSort;
	qsort( pNode, nRowNum, sizeof(EditNodeEx), cmpGetOpenedWindowArr );

	// 拡張リストのソート結果をもとに編集ウインドウリスト格納領域に結果を格納する
	for( i = 0; i < nRowNum; i++ )
	{
		(*ppEditNode)[i] = *pNode[i].p;

		//インデックスを付ける。
		//このインデックスは m_pEditArr の配列番号です。
		(*ppEditNode)[i].m_nIndex = pNode[i].p - m_pShareData->m_pEditArr;	// ポインタ減算＝配列番号
	}

	delete []pNode;

	return nRowNum;
}

/** ウィンドウの並び替え

	@param[in] hSrcTab 移動するウィンドウ
	@param[in] hSrcTab 移動先ウィンドウ

	@author ryoji
	@date 2007.07.07 genta ウィンドウ配列操作部をCTabWndより移動
*/
bool CShareData::ReorderTab( HWND hwndSrc, HWND hwndDst )
{
	EditNode	*p = NULL;
	int			nCount;
	int			i;

	int nSrcTab = -1;
	int nDstTab = -1;
	LockGuard<CMutex> guard( g_cEditArrMutex );
	nCount = GetOpenedWindowArrCore( &p, TRUE );	// ロックは自分でやっているので直接コア部呼び出し
	for( i = 0; i < nCount; i++ )
	{
		if( hwndSrc == p[i].m_hWnd )
			nSrcTab = i;
		if( hwndDst == p[i].m_hWnd )
			nDstTab = i;
	}

	if( 0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab )
	{
		if( p ) delete []p;
		return false;
	}

	// タブの順序を入れ替えるためにウィンドウのインデックスを入れ替える
	int nArr0, nArr1;
	int	nIndex;

	nArr0 = p[ nDstTab ].m_nIndex;
	nIndex = m_pShareData->m_pEditArr[ nArr0 ].m_nIndex;
	if( nSrcTab < nDstTab )
	{
		// タブ左方向ローテート
		for( i = nDstTab - 1; i >= nSrcTab; i-- )
		{
			nArr1 = p[ i ].m_nIndex;
			m_pShareData->m_pEditArr[ nArr0 ].m_nIndex = m_pShareData->m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	else
	{
		// タブ右方向ローテート
		for( i = nDstTab + 1; i <= nSrcTab; i++ )
		{
			nArr1 = p[ i ].m_nIndex;
			m_pShareData->m_pEditArr[ nArr0 ].m_nIndex = m_pShareData->m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	m_pShareData->m_pEditArr[ nArr0 ].m_nIndex = nIndex;

	if( p ) delete []p;
	return true;
}

/** タブ移動に伴うウィンドウ処理

	@param[in] hwndSrc 移動するウィンドウ
	@param[in] hwndDst 移動先ウィンドウ．新規独立時はNULL．
	@param[in] bSrcIsTop 移動するウィンドウが可視ウィンドウならtrue
	@param[in] notifygroups タブの更新が必要なグループのグループID．int[2]を呼び出し元で用意する．

	@return 更新されたhwndDst (移動先が既に閉じられた場合などにNULLに変更されることがある)

	@author ryoji
	@date 2007.07.07 genta CTabWnd::SeparateGroup()より独立
*/
HWND CShareData::SeparateGroup( HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[] )
{
	LockGuard<CMutex> guard( g_cEditArrMutex );

	EditNode* pSrcEditNode = GetEditNode( hwndSrc );
	EditNode* pDstEditNode = GetEditNode( hwndDst );
	int nSrcGroup = pSrcEditNode->m_nGroup;
	int nDstGroup;
	if( pDstEditNode == NULL )
	{
		hwndDst = NULL;
		nDstGroup = ++m_pShareData->m_nGroupSequences;	// 新規グループ
	}
	else
	{
		nDstGroup = pDstEditNode->m_nGroup;	// 既存グループ
	}

	pSrcEditNode->m_nGroup = nDstGroup;
	pSrcEditNode->m_nIndex = ++m_pShareData->m_nSequences;	// タブ並びの最後（起動順の最後）にもっていく

	// 非表示のタブを既存グループに移動するときは非表示のままにするので
	// 内部情報も先頭にはならないよう、必要なら先頭ウィンドウと位置を交換する。
	if( !bSrcIsTop && pDstEditNode != NULL )
	{
		if( pSrcEditNode < pDstEditNode )
		{
			EditNode en = *pDstEditNode;
			*pDstEditNode = *pSrcEditNode;
			*pSrcEditNode = en;
		}
	}
	
	notifygroups[0] = nSrcGroup;
	notifygroups[1] = nDstGroup;
	
	return hwndDst;
}

/*!
	アウトプットウインドウに出力

	アウトプットウインドウが無ければオープンする
	@param lpFmt [in] 書式指定文字列
*/
void CShareData::TraceOut( LPCTSTR lpFmt, ... )
{

	if( NULL == m_pShareData->m_hwndDebug
	|| !IsEditWnd( m_pShareData->m_hwndDebug )
	){
		// 2007.06.26 ryoji
		// アウトプットウィンドウを作成元と同じグループに作成するために m_hwndTraceOutSource を使っています
		// （m_hwndTraceOutSource は CEditWnd::Create() で予め設定）
		// ちょっと不恰好だけど、TraceOut() の引数にいちいち起動元を指定するのも．．．
		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = _T("");
		sLoadInfo.eCharCode = CODE_UNICODE;		// CODE_SJIS->	2008/6/8 Uchi
		sLoadInfo.bViewMode = false;
		CControlTray::OpenNewEditor( NULL, m_hwndTraceOutSource, sLoadInfo, _T("-DEBUGMODE"), true );
		//	2001/06/23 N.Nakatani 窓が出るまでウエイトをかけるように修正
		//アウトプットウインドウが出来るまで5秒ぐらい待つ。
		//	Jun. 25, 2001 genta OpenNewEditorの同期機能を利用するように変更

		/* 開いているウィンドウをアクティブにする */
		/* アクティブにする */
		ActivateFrameWindow( m_pShareData->m_hwndDebug );
	}
	va_list argList;
	va_start( argList, lpFmt );
	auto_vsprintf( m_pShareData->GetWorkBuffer<EDIT_CHAR>(), to_wchar(lpFmt), argList );
	va_end( argList );
	::SendMessage( m_pShareData->m_hwndDebug, MYWM_ADDSTRING, 0, 0 );
	return;
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
	if( -1 != idx && !m_pShareData->m_MacroTable[idx].IsEnabled() )
		return 0;
	TCHAR *ptr;
	TCHAR *pszFile;

	if( -1 == idx ){
		pszFile = _T("RecKey.mac");
	}else{
		pszFile = m_pShareData->m_MacroTable[idx].m_szFile;
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
		|| m_pShareData->m_szMACROFOLDER[0] == _T('\0') ){	//	フォルダ指定なし
		if( pszPath == NULL || nBufLen <= nLen ){
			return -nLen;
		}
		_tcscpy( pszPath, pszFile );
		return nLen;
	}
	else {	//	フォルダ指定あり
		//	相対パス→絶対パス
		int nFolderSep = AddLastChar( m_pShareData->m_szMACROFOLDER, _countof2(m_pShareData->m_szMACROFOLDER), _T('\\') );
		int nAllLen;
		TCHAR *pszDir;

		 // 2003.06.24 Moca フォルダも相対パスなら実行ファイルからのパス
		// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
		if( _IS_REL_PATH( m_pShareData->m_szMACROFOLDER ) ){
			TCHAR szDir[_MAX_PATH + _countof2( m_pShareData->m_szMACROFOLDER )];
			GetInidirOrExedir( szDir, m_pShareData->m_szMACROFOLDER );
			pszDir = szDir;
		}else{
			pszDir = m_pShareData->m_szMACROFOLDER;
		}

		int nDirLen = _tcslen( pszDir );
		nAllLen = nDirLen + nLen + ( -1 == nFolderSep ? 1 : 0 );
		if( pszPath == NULL || nBufLen <= nAllLen ){
			return -nAllLen;
		}

		_tcscpy( pszPath, pszDir );
		ptr = pszPath + nDirLen;
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
	if( !m_pShareData->m_MacroTable[idx].IsEnabled() )
		return false;

	return m_pShareData->m_MacroTable[idx].m_bReloadWhenExecute;
}

/*!	m_aSearchKeysにpszSearchKeyを追加する。
	YAZAKI
*/
void CShareData::AddToSearchKeyArr( const wchar_t* pszSearchKey )
{
	CRecentSearch	cRecentSearchKey;
	cRecentSearchKey.AppendItem( pszSearchKey );
	cRecentSearchKey.Terminate();
}

/*!	m_aReplaceKeysにpszReplaceKeyを追加する
	YAZAKI
*/
void CShareData::AddToReplaceKeyArr( const wchar_t* pszReplaceKey )
{
	CRecentReplace	cRecentReplaceKey;
	cRecentReplaceKey.AppendItem( pszReplaceKey );
	cRecentReplaceKey.Terminate();

	return;
}

/*!	m_aGrepFilesにpszGrepFileを追加する
	YAZAKI
*/
void CShareData::AddToGrepFileArr( const TCHAR* pszGrepFile )
{
	CRecentGrepFile	cRecentGrepFile;
	cRecentGrepFile.AppendItem( pszGrepFile );
	cRecentGrepFile.Terminate();
}

/*!	m_aGrepFolders.size()にpszGrepFolderを追加する
	YAZAKI
*/
void CShareData::AddToGrepFolderArr( const TCHAR* pszGrepFolder )
{
	CRecentGrepFolder	cRecentGrepFolder;
	cRecentGrepFolder.AppendItem( pszGrepFolder );
	cRecentGrepFolder.Terminate();
}

/*!	外部Winヘルプが設定されているか確認。
*/
bool CShareData::ExtWinHelpIsSet( CTypeConfig nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHelp[0] != L'\0'){
		return true;	//	共通設定に設定されている
	}
	if (!nTypeNo.IsValid()) {
		return false;	//	共通設定に設定されていない＆nTypeNoが範囲外。
	}
	if (m_pShareData->GetTypeSetting(nTypeNo).m_szExtHelp[0] != L'\0'){
		return true;	//	タイプ別設定に設定されている。
	}
	return false;
}

/*!	設定されている外部Winヘルプのファイル名を返す。
	タイプ別設定にファイル名が設定されていれば、そのファイル名を返します。
	そうでなければ、共通設定のファイル名を返します。
*/
const TCHAR* CShareData::GetExtWinHelp( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && m_pShareData->GetTypeSetting(nTypeNo).m_szExtHelp[0] != _T('\0')){
		return m_pShareData->GetTypeSetting(nTypeNo).m_szExtHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHelp;
}

/*!	外部HTMLヘルプが設定されているか確認。
*/
bool CShareData::ExtHTMLHelpIsSet( CTypeConfig nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp[0] != L'\0'){
		return true;	//	共通設定に設定されている
	}
	if (!nTypeNo.IsValid()){
		return false;	//	共通設定に設定されていない＆nTypeNoが範囲外。
	}
	if (nTypeNo->m_szExtHtmlHelp[0] != L'\0'){
		return true;	//	タイプ別設定に設定されている。
	}
	return false;
}

/*!	設定されている外部Winヘルプのファイル名を返す。
	タイプ別設定にファイル名が設定されていれば、そのファイル名を返します。
	そうでなければ、共通設定のファイル名を返します。
*/
const TCHAR* CShareData::GetExtHTMLHelp( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && m_pShareData->GetTypeSetting(nTypeNo).m_szExtHtmlHelp[0] != _T('\0')){
		return m_pShareData->GetTypeSetting(nTypeNo).m_szExtHtmlHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp;
}

/*!	ビューアを複数起動しないがONかを返す。
*/
bool CShareData::HTMLHelpIsSingle( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && m_pShareData->GetTypeSetting(nTypeNo).m_szExtHtmlHelp[0] != L'\0'){
		return (m_pShareData->GetTypeSetting(nTypeNo).m_bHtmlHelpIsSingle != FALSE);
	}
	
	return (m_pShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle != FALSE);
}

/*! 日付をフォーマット
	systime：時刻データ
	
	pszDest：フォーマット済みテキスト格納用バッファ
	nDestLen：pszDestの長さ
	
	pszDateFormat：
		カスタムのときのフォーマット
*/
const TCHAR* CShareData::MyGetDateFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen )
{
	return MyGetDateFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_Common.m_sFormat.m_nDateFormatType,
		m_pShareData->m_Common.m_sFormat.m_szDateFormat
	);
}

const TCHAR* CShareData::MyGetDateFormat(
	const SYSTEMTIME&		systime,
	TCHAR*		pszDest,
	int				nDestLen,
	int				nDateFormatType,
	const TCHAR*	szDateFormat
)
{
	const TCHAR* pszForm;
	DWORD dwFlags;
	if( 0 == nDateFormatType ){
		dwFlags = DATE_LONGDATE;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = szDateFormat;
	}
	::GetDateFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}



/* 時刻をフォーマット */
const TCHAR* CShareData::MyGetTimeFormat( const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen )
{
	return MyGetTimeFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_Common.m_sFormat.m_nTimeFormatType,
		m_pShareData->m_Common.m_sFormat.m_szTimeFormat
	);
}

/* 時刻をフォーマット */
const TCHAR* CShareData::MyGetTimeFormat(
	const SYSTEMTIME&	systime,
	TCHAR*			pszDest,
	int					nDestLen,
	int					nTimeFormatType,
	const TCHAR*		szTimeFormat
)
{
	const TCHAR* pszForm;
	DWORD dwFlags;
	if( 0 == nTimeFormatType ){
		dwFlags = 0;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = szTimeFormat;
	}
	::GetTimeFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}



/*!	共有データの設定に従ってパスを縮小表記に変換する
	@param pszSrc   [in]  ファイル名
	@param pszDest  [out] 変換後のファイル名の格納先
	@param nDestLen [in]  終端のNULLを含むpszDestのTCHAR単位の長さ _MAX_PATH まで
	@date 2003.01.27 Moca 新規作成
	@note 連続して呼び出す場合のため、展開済みメタ文字列をキャッシュして高速化している。
*/
LPTSTR CShareData::GetTransformFileNameFast( LPCTSTR pszSrc, LPTSTR pszDest, int nDestLen )
{
	int i;
	TCHAR szBuf[_MAX_PATH + 1];

	if( -1 == m_nTransformFileNameCount ){
		TransformFileName_MakeCache();
	}

	if( 0 < m_nTransformFileNameCount ){
		GetFilePathFormat( pszSrc, pszDest, nDestLen,
			m_szTransformFileNameFromExp[0],
			m_pShareData->m_szTransformFileNameTo[m_nTransformFileNameOrgId[0]]
		);
		for( i = 1; i < m_nTransformFileNameCount; i++ ){
			_tcscpy( szBuf, pszDest );
			GetFilePathFormat( szBuf, pszDest, nDestLen,
				m_szTransformFileNameFromExp[i],
				m_pShareData->m_szTransformFileNameTo[m_nTransformFileNameOrgId[i]] );
		}
	}else{
		// 変換する必要がない コピーだけする
		_tcsncpy( pszDest, pszSrc, nDestLen - 1 );
		pszDest[nDestLen - 1] = '\0';
	}
	return pszDest;
}

/*!	展開済みメタ文字列のキャッシュを作成・更新する
	@retval 有効な展開済み置換前文字列の数
	@date 2003.01.27 Moca 新規作成
	@date 2003.06.23 Moca 関数名変更
*/
int CShareData::TransformFileName_MakeCache( void ){
	int i;
	int nCount = 0;
	for( i = 0; i < m_pShareData->m_nTransformFileNameArrNum; i++ ){
		if( L'\0' != m_pShareData->m_szTransformFileNameFrom[i][0] ){
			if( ExpandMetaToFolder( m_pShareData->m_szTransformFileNameFrom[i],
			 m_szTransformFileNameFromExp[nCount], _MAX_PATH ) ){
				// m_szTransformFileNameToとm_szTransformFileNameFromExpの番号がずれることがあるので記録しておく
				m_nTransformFileNameOrgId[nCount] = i;
				nCount++;
			}
		}
	}
	m_nTransformFileNameCount = nCount;
	return nCount;
}


/*!	ファイル・フォルダ名を置換して、簡易表示名を取得する
	@date 2002.11.27 Moca 新規作成
	@note 大小文字を区別しない。nDestLenに達したときは後ろを切り捨てられる
*/
LPCTSTR CShareData::GetFilePathFormat( LPCTSTR pszSrc, LPTSTR pszDest, int nDestLen, LPCTSTR pszFrom, LPCTSTR pszTo )
{
	int i, j;
	int nSrcLen;
	int nFromLen, nToLen;
	int nCopy;

	nSrcLen  = _tcslen( pszSrc );
	nFromLen = _tcslen( pszFrom );
	nToLen   = _tcslen( pszTo );

	nDestLen--;

	for( i = 0, j = 0; i < nSrcLen && j < nDestLen; i++ ){
#if defined(_MBCS)
		if( 0 == strnicmp( &pszSrc[i], pszFrom, nFromLen ) )
#else
		if( 0 == _tcsncicmp( &pszSrc[i], pszFrom, nFromLen ) )
#endif
		{
			nCopy = __min( nToLen, nDestLen - j );
			memcpy( &pszDest[j], pszTo, nCopy * sizeof( TCHAR ) );
			j += nCopy;
			i += nFromLen - 1;
		}else{
#if defined(_MBCS)
// SJIS 専用処理
			if( _IS_SJIS_1( (unsigned char)pszSrc[i] ) && i + 1 < nSrcLen && _IS_SJIS_2( (unsigned char)pszSrc[i + 1] ) ){
				if( j + 1 < nDestLen ){
					pszDest[j] = pszSrc[i];
					j++;
					i++;
				}else{
					// SJISの先行バイトだけコピーされるのを防ぐ
					break;// goto end_of_func;
				}
			}
#endif
			pszDest[j] = pszSrc[i];
			j++;
		}
	}
// end_of_func:;
	pszDest[j] = '\0';
	return pszDest;
}


/*!	%MYDOC%などのパラメータ指定を実際のパス名に変換する

	@param pszSrc  [in]  変換前文字列
	@param pszDes  [out] 変換後文字列
	@param nDesLen [in]  pszDesのNULLを含むTCHAR単位の長さ
	@retval true  正常に変換できた
	@retval false バッファが足りなかった，またはエラー。pszDesは不定
	@date 2002.11.27 Moca 作成開始
*/
bool CShareData::ExpandMetaToFolder( LPCTSTR pszSrc, LPTSTR pszDes, int nDesLen )
{
	LPCTSTR ps;
	LPTSTR  pd, pd_end;

#define _USE_META_ALIAS
#ifdef _USE_META_ALIAS
	struct MetaAlias{
		LPTSTR szAlias;
		int nLenth;
		LPTSTR szOrig;
	};
	static const MetaAlias AliasList[] = {
		{  _T("COMDESKTOP"), 10, _T("Common Desktop") },
		{  _T("COMMUSIC"), 8, _T("CommonMusic") },
		{  _T("COMVIDEO"), 8, _T("CommonVideo") },
		{  _T("MYMUSIC"),  7, _T("My Music") },
		{  _T("MYVIDEO"),  7, _T("Video") },
		{  _T("COMPICT"),  7, _T("CommonPictures") },
		{  _T("MYPICT"),   6, _T("My Pictures") },
		{  _T("COMDOC"),   6, _T("Common Documents") },
		{  _T("MYDOC"),    5, _T("Personal") },
		{ NULL, 0 , NULL }
	};
#endif

	pd_end = pszDes + ( nDesLen - 1 );
	for( ps = pszSrc, pd = pszDes; _T('\0') != *ps; ps++ ){
		if( pd_end <= pd ){
			if( pd_end == pd ){
				*pd = _T('\0');
			}
			return false;
		}

		if( _T('%') != *ps ){
			*pd = *ps;
			pd++;
			continue;
		}

		// %% は %
		if( _T('%') == ps[1] ){
			*pd = _T('%');
			pd++;
			ps++;
			continue;
		}

		if( _T('\0') != ps[1] ){
			TCHAR szMeta[_MAX_PATH];
			TCHAR szPath[_MAX_PATH + 1];
			int   nMetaLen;
			int   nPathLen;
			bool  bFolderPath;
			LPCTSTR  pStr;
			ps++;
			// %SAKURA%
			if( 0 == auto_strnicmp( _T("SAKURA%"), ps, 7 ) ){
				// exeのあるフォルダ
				GetExedir( szPath );
				nMetaLen = 6;
			}
			// %SAKURADATA%	// 2007.06.06 ryoji
			else if( 0 == auto_strnicmp( _T("SAKURADATA%"), ps, 11 ) ){
				// iniのあるフォルダ
				GetInidir( szPath );
				nMetaLen = 10;
			}
			// メタ文字列っぽい
			else if( NULL != (pStr = _tcschr( ps, _T('%') ) )){
				nMetaLen = pStr - ps;
				if( nMetaLen < _MAX_PATH ){
					auto_memcpy( szMeta, ps, nMetaLen );
					szMeta[nMetaLen] = _T('\0');
				}
				else{
					*pd = _T('\0');
					return false;
				}
#ifdef _USE_META_ALIAS
				// メタ文字列がエイリアス名なら書き換える
				const MetaAlias* pAlias;
				for( pAlias = &AliasList[0]; nMetaLen < pAlias->nLenth; pAlias++ )
					; // 読み飛ばす
				for( ; nMetaLen == pAlias->nLenth; pAlias++ ){
					if( 0 == auto_stricmp( pAlias->szAlias, szMeta ) ){
						_tcscpy( szMeta, pAlias->szOrig );
						break;
					}
				}
#endif
				// 直接レジストリで調べる
				szPath[0] = _T('\0');
				bFolderPath = ReadRegistry( HKEY_CURRENT_USER,
					_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
					szMeta, szPath, _countof( szPath ) );
				if( false == bFolderPath || _T('\0') == szPath[0] ){
					bFolderPath = ReadRegistry( HKEY_LOCAL_MACHINE,
						_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
						szMeta, szPath, _countof( szPath ) );
				}
				if( false == bFolderPath || _T('\0') == szPath[0] ){
					pStr = _tgetenv( szMeta );
					// 環境変数
					if( NULL != pStr ){
						nPathLen = _tcslen( pStr );
						if( nPathLen < _MAX_PATH ){
							_tcscpy( szPath, pStr );
						}else{
							*pd = _T('\0');
							return false;
						}
					}
					// 未定義のメタ文字列は 入力された%...%を，そのまま文字として処理する
					else if(  pd + ( nMetaLen + 2 ) < pd_end ){
						*pd = _T('%');
						auto_memcpy( &pd[1], ps, nMetaLen );
						pd[nMetaLen + 1] = _T('%');
						pd += nMetaLen + 2;
						ps += nMetaLen;
						continue;
					}else{
						*pd = _T('\0');
						return false;
					}
				}
			}else{
				// %...%の終わりの%がない とりあえず，%をコピー
				*pd = _T('%');
				pd++;
				ps--; // 先にps++してしまったので戻す
				continue;
			}

			// ロングファイル名にする
			nPathLen = _tcslen( szPath );
			LPTSTR pStr2 = szPath;
			if( nPathLen < _MAX_PATH && 0 != nPathLen ){
				if( FALSE != GetLongFileName( szPath, szMeta ) ){
					pStr2 = szMeta;
				}
			}

			// 最後のフォルダ区切り記号を削除する
			// [A:\]などのルートであっても削除
			for(nPathLen = 0; pStr2[nPathLen] != _T('\0'); nPathLen++ ){
#ifdef _MBCS
				if( _IS_SJIS_1( (unsigned char)pStr2[nPathLen] ) && _IS_SJIS_2( (unsigned char)pStr2[nPathLen + 1] ) ){
					// SJIS読み飛ばし
					nPathLen++; // 2003/01/17 sui
				}else
#endif
				if( _T('\\') == pStr2[nPathLen] && _T('\0') == pStr2[nPathLen + 1] ){
					pStr2[nPathLen] = _T('\0');
					break;
				}
			}

			if( pd + nPathLen < pd_end && 0 != nPathLen ){
				auto_memcpy( pd, pStr2, nPathLen );
				pd += nPathLen;
				ps += nMetaLen;
			}else{
				*pd = _T('\0');
				return false;
			}
		}else{
			// 最後の文字が%だった
			*pd = *ps;
			pd++;
		}
	}
	*pd = _T('\0');
	return true;
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
	char dummy[ _countof(DEFAULT_TOOL_BUTTONS) < MAX_TOOLBARBUTTONS ? 1:0 ];
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


#include "CNormalProcess.h"

//GetDllShareData用グローバル変数
DLLSHAREDATA* g_theDLLSHAREDATA = NULL;

//DLLSHAREDATAへの簡易アクセサ
DLLSHAREDATA& GetDllShareData()
{
	assert(g_theDLLSHAREDATA);
	return *g_theDLLSHAREDATA;
}

//DLLSHAREDATAを確保したら、まずこれを呼ぶ。
void DLLSHAREDATA::OnInit()
{
	g_theDLLSHAREDATA = this;
}
