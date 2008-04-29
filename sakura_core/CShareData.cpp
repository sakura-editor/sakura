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
#include "charcode.h"
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

	Version 1000:
	バージョン1000以降を本家統合までの間、使わせてください。かなり頻繁に構成が変更されると思われるので。by kobake 2008.03.02
*/

const unsigned int uShareDataVersion = 1008;

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

//	MessageBoxA(NULL,"share","init",MB_OK);

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
		::MessageBoxA(
			NULL,
			"CreateFileMapping()に失敗しました",
			"予期せぬエラー",
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
		wcscpy( m_pShareData->m_Common.m_sTabBar.m_szTabWndCaption,
	L"${w?【Grep】$h$:【アウトプット】$:$f$}${U?(更新)$}${R?(ビューモード)$:(上書き禁止)$}${M?【キーマクロの記録中】$}" );	//@@@ 2003.06.13 MIK
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
			mptr->m_bReloadWhenExecute = FALSE;
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




/* KEYDATA配列にデータをセット */
void CShareData::SetKeyNameArrVal(
	DLLSHAREDATA*	pShareData,
	int				nIdx,
	short			nKeyCode,
	TCHAR*			pszKeyName,
	EFunctionCode	nFuncCode_0,
	EFunctionCode	nFuncCode_1,
	EFunctionCode	nFuncCode_2,
	EFunctionCode	nFuncCode_3,
	EFunctionCode	nFuncCode_4,
	EFunctionCode	nFuncCode_5,
	EFunctionCode	nFuncCode_6,
	EFunctionCode	nFuncCode_7
)
{
	pShareData->m_pKeyNameArr[nIdx].m_nKeyCode = nKeyCode;
	_tcscpy( pShareData->m_pKeyNameArr[nIdx].m_szKeyName, pszKeyName );
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[0] = nFuncCode_0;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[1] = nFuncCode_1;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[2] = nFuncCode_2;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[3] = nFuncCode_3;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[4] = nFuncCode_4;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[5] = nFuncCode_5;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[6] = nFuncCode_6;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[7] = nFuncCode_7;
	return;
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

	@date 2007.06.22 ryoji nGroup引数を追加
*/
int CShareData::GetEditorWindowsNum( int nGroup )
{
	int		i;
	int		j;

	j = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			if( nGroup != 0 && nGroup != GetGroupId( m_pShareData->m_pEditArr[i].m_hWnd ) )
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
		sLoadInfo.eCharCode = CODE_SJIS;
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

	return ( m_pShareData->m_MacroTable[idx].m_bReloadWhenExecute == TRUE );
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

static const wchar_t* const	ppszKeyWordsCPP[] = {
	L"#define",
	L"#elif",
	L"#else",
	L"#endif",
	L"#error",
	L"#if",
	L"#ifdef",
	L"#ifndef",
	L"#include",
	L"#line",
	L"#pragma",
	L"#undef",
	L"__FILE__",
	L"__declspec",
	L"asm",
	L"auto",
	L"bool",
	L"break",
	L"case",
	L"catch",
	L"char",
	L"class",
	L"const",
	L"const_cast",
	L"continue",
	L"default",
	L"define",
	L"defined",
	L"delete",
	L"do",
	L"double",
	L"dynamic_cast",
	L"elif",
	L"else",
	L"endif",
	L"enum",
	L"error",
	L"explicit",
	L"export",
	L"extern",
	L"false",
	L"float",
	L"for",
	L"friend",
	L"goto",
	L"if",
	L"ifdef",
	L"ifndef",
	L"include",
	L"inline",
	L"int",
	L"line",
	L"long",
	L"mutable",
	L"namespace",
	L"new",
	L"operator",
	L"pragma",
	L"private",
	L"protected",
	L"public",
	L"register",
	L"reinterpret_cast",
	L"return",
	L"short",
	L"signed",
	L"sizeof",
	L"static",
	L"static_cast",
	L"struct",
	L"switch",
	L"template",
	L"this",
	L"throw",
	L"true",
	L"try",
	L"typedef",
	L"typeid",
	L"typename",
	L"undef",
	L"union",
	L"unsigned",
	L"using",
	L"virtual",
	L"void",
	L"volatile",
	L"wchar_t",
	L"while"
};

static const wchar_t* const	ppszKeyWordsHTML[] = {
	L"_blank",
	L"_parent",
	L"_self",
	L"_top",
	L"A",
	L"ABBR",
	L"ABOVE",
	L"absbottom",
	L"absmiddle",
	L"ACCESSKEY",
	L"ACRONYM",
	L"ACTION",
	L"ADDRESS",
	L"ALIGN",
	L"all",
	L"APPLET",
	L"AREA",
	L"AUTOPLAY",
	L"AUTOSTART",
	L"B",
	L"BACKGROUND",
	L"BASE",
	L"BASEFONT",
	L"baseline",
	L"BEHAVIOR",
	L"BELOW",
	L"BGCOLOR",
	L"BGSOUND",
	L"BIG",
	L"BLINK",
	L"BLOCKQUOTE",
	L"BODY",
	L"BORDER",
	L"BORDERCOLOR",
	L"BORDERCOLORDARK",
	L"BORDERCOLORLIGHT",
	L"BOTTOM",
	L"box",
	L"BR",
	L"BUTTON",
	L"CAPTION",
	L"CELLPADDING",
	L"CELLSPACING",
	L"CENTER",
	L"CHALLENGE",
	L"char",
	L"checkbox",
	L"CHECKED",
	L"CITE",
	L"CLEAR",
	L"CLIP",
	L"CODE",
	L"CODEBASE",
	L"CODETYPE",
	L"COL",
	L"COLGROUP",
	L"COLOR",
	L"COLS",
	L"COLSPAN",
	L"COMMENT",
	L"CONTROLS",
	L"DATA",
	L"DD",
	L"DECLARE",
	L"DEFER",
	L"DEL",
	L"DELAY",
	L"DFN",
	L"DIR",
	L"DIRECTION",
	L"DISABLED",
	L"DIV",
	L"DL",
	L"DOCTYPE",
	L"DT",
	L"EM",
	L"EMBED",
	L"ENCTYPE",
	L"FACE",
	L"FIELDSET",
	L"file",
	L"FONT",
	L"FOR",
	L"FORM",
	L"FRAME",
	L"FRAMEBORDER",
	L"FRAMESET",
	L"GET",
	L"groups",
	L"GROUPS",
	L"GUTTER",
	L"H1",
	L"H2",
	L"H3",
	L"H4",
	L"H5",
	L"H6",
	L"H7",
	L"HEAD",
	L"HEIGHT",
	L"HIDDEN",
	L"Hn",
	L"HR",
	L"HREF",
	L"hsides",
	L"HSPACE",
	L"HTML",
	L"I",
	L"ID",
	L"IFRAME",
	L"ILAYER",
	L"image",
	L"IMG",
	L"INDEX",
	L"inherit",
	L"INPUT",
	L"INS",
	L"ISINDEX",
	L"JavaScript",
	L"justify",
	L"KBD",
	L"KEYGEN",
	L"LABEL",
	L"LANGUAGE",
	L"LAYER",
	L"LEFT",
	L"LEGEND",
	L"lhs",
	L"LI",
	L"LINK",
	L"LISTING",
	L"LOOP",
	L"MAP",
	L"MARQUEE",
	L"MAXLENGTH",
	L"MENU",
	L"META",
	L"METHOD",
	L"METHODS",
	L"MIDDLE",
	L"MULTICOL",
	L"MULTIPLE",
	L"NAME",
	L"NEXT",
	L"NEXTID",
	L"NOBR",
	L"NOEMBED",
	L"NOFRAMES",
	L"NOLAYER",
	L"none",
	L"NOSAVE",
	L"NOSCRIPT",
	L"NOTAB",
	L"NOWRAP",
	L"OBJECT",
	L"OL",
	L"onBlur",
	L"onChange",
	L"onClick",
	L"onFocus",
	L"onLoad",
	L"onMouseOut",
	L"onMouseOver",
	L"onReset",
	L"onSelect",
	L"onSubmit",
	L"OPTION",
	L"P",
	L"PAGEX",
	L"PAGEY",
	L"PALETTE",
	L"PANEL",
	L"PARAM",
	L"PARENT",
	L"password",
	L"PLAINTEXT",
	L"PLUGINSPAGE",
	L"POST",
	L"PRE",
	L"PREVIOUS",
	L"Q",
	L"radio",
	L"REL",
	L"REPEAT",
	L"reset",
	L"REV",
	L"rhs",
	L"RIGHT",
	L"rows",
	L"ROWSPAN",
	L"RULES",
	L"S",
	L"SAMP",
	L"SAVE",
	L"SCRIPT",
	L"SCROLLAMOUNT",
	L"SCROLLDELAY",
	L"SELECT",
	L"SELECTED",
	L"SERVER",
	L"SHAPES",
	L"show",
	L"SIZE",
	L"SMALL",
	L"SONG",
	L"SPACER",
	L"SPAN",
	L"SRC",
	L"STANDBY",
	L"STRIKE",
	L"STRONG",
	L"STYLE",
	L"SUB",
	L"submit",
	L"SUMMARY",
	L"SUP",
	L"TABINDEX",
	L"TABLE",
	L"TARGET",
	L"TBODY",
	L"TD",
	L"TEXT",
	L"TEXTAREA",
	L"textbottom",
	L"TEXTFOCUS",
	L"textmiddle",
	L"texttop",
	L"TFOOT",
	L"TH",
	L"THEAD",
	L"TITLE",
	L"TOP",
	L"TR",
	L"TT",
	L"TXTCOLOR",
	L"TYPE",
	L"U",
	L"UL",
	L"URN",
	L"USEMAP",
	L"VALIGN",
	L"VALUE",
	L"VALUETYPE",
	L"VAR",
	L"VISIBILITY",
	L"void",
	L"vsides",
	L"VSPACE",
	L"WBR",
	L"WIDTH",
	L"WRAP",
	L"XMP"
};

static const wchar_t* const	ppszKeyWordsPLSQL[] = {
	L"AND",
	L"AS",
	L"BEGIN",
	L"BINARY_INTEGER",
	L"BODY",
	L"BOOLEAN",
	L"BY",
	L"CHAR",
	L"CHR",
	L"COMMIT",
	L"COUNT",
	L"CREATE",
	L"CURSOR",
	L"DATE",
	L"DECLARE",
	L"DEFAULT",
	L"DELETE",
	L"ELSE",
	L"ELSIF",
	L"END",
	L"ERRORS",
	L"EXCEPTION",
	L"FALSE",
	L"FOR",
	L"FROM",
	L"FUNCTION",
	L"GOTO",
	L"HTP",
	L"IDENT_ARR",
	L"IF",
	L"IN",
	L"INDEX",
	L"INTEGER",
	L"IS",
	L"LOOP",
	L"NOT",
	L"NO_DATA_FOUND",
	L"NULL",
	L"NUMBER",
	L"OF",
	L"OR",
	L"ORDER",
	L"OUT",
	L"OWA_UTIL",
	L"PACKAGE",
	L"PRAGMA",
	L"PRN",
	L"PROCEDURE",
	L"REPLACE",
	L"RESTRICT_REFERENCES",
	L"RETURN",
	L"ROWTYPE",
	L"SELECT",
	L"SHOW",
	L"SUBSTR",
	L"TABLE",
	L"THEN",
	L"TRUE",
	L"TYPE",
	L"UPDATE",
	L"VARCHAR",
	L"VARCHAR2",
	L"WHEN",
	L"WHERE",
	L"WHILE",
	L"WNDS",
	L"WNPS",
	L"RAISE",
	L"INSERT",
	L"INTO",
	L"VALUES",
	L"SET",
	L"SYSDATE",
	L"RTRIM",
	L"LTRIM",
	L"TO_CHAR",
	L"DUP_VAL_ON_INDEX",
	L"ROLLBACK",
	L"OTHERS",
	L"SQLCODE"
};

//Jul. 10, 2001 JEPRO 追加
static const wchar_t* const	ppszKeyWordsCOBOL[] = {
	L"ACCEPT",
	L"ADD",
	L"ADVANCING",
	L"AFTER",
	L"ALL",
	L"AND",
	L"ARGUMENT",
	L"ASSIGN",
	L"AUTHOR",
	L"BEFORE",
	L"BLOCK",
	L"BY",
	L"CALL",
	L"CHARACTERS",
	L"CLOSE",
	L"COMP",
	L"COMPILED",
	L"COMPUTE",
	L"COMPUTER",
	L"CONFIGURATION",
	L"CONSOLE",
	L"CONTAINS",
	L"CONTINUE",
	L"CONTROL",
	L"COPY",
	L"DATA",
	L"DELETE",
	L"DISPLAY",
	L"DIVIDE",
	L"DIVISION",
	L"ELSE",
	L"END",
	L"ENVIRONMENT",
	L"EVALUATE",
	L"EXAMINE",
	L"EXIT",
	L"EXTERNAL",
	L"FD",
	L"FILE",
	L"FILLER",
	L"FROM",
	L"GIVING",
	L"GO",
	L"GOBACK",
	L"HIGH-VALUE",
	L"IDENTIFICATION"
	L"IF",
	L"INITIALIZE",
	L"INPUT",
	L"INTO",
	L"IS",
	L"LABEL",
	L"LINKAGE",
	L"LOW-VALUE",
	L"MODE",
	L"MOVE",
	L"NOT",
	L"OBJECT",
	L"OCCURS",
	L"OF",
	L"ON",
	L"OPEN",
	L"OR",
	L"OTHER",
	L"OUTPUT",
	L"PERFORM",
	L"PIC",
	L"PROCEDURE",
	L"PROGRAM",
	L"READ",
	L"RECORD",
	L"RECORDING",
	L"REDEFINES",
	L"REMAINDER",
	L"REMARKS",
	L"REPLACING",
	L"REWRITE",
	L"ROLLBACK",
	L"SECTION",
	L"SELECT",
	L"SOURCE",
	L"SPACE",
	L"STANDARD",
	L"STOP",
	L"STORAGE",
	L"SYSOUT",
	L"TEST",
	L"THEN",
	L"TO",
	L"TODAY",
	L"TRANSFORM",
	L"UNTIL",
	L"UPON",
	L"USING",
	L"VALUE",
	L"VARYING",
	L"WHEN",
	L"WITH",
	L"WORKING",
	L"WRITE",
	L"WRITTEN",
	L"ZERO"
};

static const wchar_t*	ppszKeyWordsJAVA[] = {
	L"abstract",
	L"assert",	// Mar. 8, 2003 genta
	L"boolean",
	L"break",
	L"byte",
	L"case",
	L"catch",
	L"char",
	L"class",
	L"const",
	L"continue",
	L"default",
	L"do",
	L"double",
	L"else",
	L"extends",
	L"final",
	L"finally",
	L"float",
	L"for",
	L"goto",
	L"if",
	L"implements",
	L"import",
	L"instanceof",
	L"int",
	L"interface",
	L"long",
	L"native",
	L"new",
	L"package",
	L"private",
	L"protected",
	L"public",
	L"return",
	L"short",
	L"static",
	L"strictfp",	// Mar. 8, 2003 genta
	L"super",
	L"switch",
	L"synchronized",
	L"this",
	L"throw",
	L"throws",
	L"transient",
	L"try",
	L"void",
	L"volatile",
	L"while"
};

static const wchar_t* const	ppszKeyWordsCORBA_IDL[] = {
	L"any",
	L"attribute",
	L"boolean",
	L"case",
	L"char",
	L"const",
	L"context",
	L"default",
	L"double",
	L"enum",
	L"exception",
	L"FALSE",
	L"fixed",
	L"float",
	L"in",
	L"inout",
	L"interface",
	L"long",
	L"module",
	L"Object",
	L"octet",
	L"oneway",
	L"out",
	L"raises",
	L"readonly",
	L"sequence",
	L"short",
	L"string",
	L"struct",
	L"switch",
	L"TRUE",
	L"typedef",
	L"unsigned",
	L"union",
	L"void",
	L"wchar_t",
	L"wstring"
};

static const wchar_t* const	ppszKeyWordsAWK[] = {
	L"BEGIN",
	L"END",
	L"next",
	L"exit",
	L"func",
	L"function",
	L"return",
	L"if",
	L"else",
	L"for",
	L"in",
	L"do",
	L"while",
	L"break",
	L"continue",
	L"$0",
	L"$1",
	L"$2",
	L"$3",
	L"$4",
	L"$5",
	L"$6",
	L"$7",
	L"$8",
	L"$9",
	L"$10",
	L"$11",
	L"$12",
	L"$13",
	L"$14",
	L"$15",
	L"$16",
	L"$17",
	L"$18",
	L"$19",
	L"$20",
	L"FS",
	L"OFS",
	L"NF",
	L"RS",
	L"ORS",
	L"NR",
	L"FNR",
	L"ARGV",
	L"ARGC",
	L"ARGIND",
	L"FILENAME",
	L"ENVIRON",
	L"ERRNO",
	L"OFMT",
	L"CONVFMT",
	L"FIELDWIDTHS",
	L"IGNORECASE",
	L"RLENGTH",
	L"RSTART",
	L"SUBSEP",
	L"delete",
	L"index",
	L"jindex",
	L"length",
	L"jlength",
	L"substr",
	L"jsubstr",
	L"match",
	L"split",
	L"sub",
	L"gsub",
	L"sprintf",
	L"tolower",
	L"toupper",
	L"print",
	L"printf",
	L"getline",
	L"system",
	L"close",
	L"sin",
	L"cos",
	L"atan2",
	L"exp",
	L"log",
	L"int",
	L"sqrt",
	L"srand",
	L"rand",
	L"strftime",
	L"systime"
};

static const wchar_t*	ppszKeyWordsBAT[] = {
	L"PATH",
	L"PROMPT",
	L"TEMP",
	L"TMP",
	L"TZ",
	L"CONFIG",
	L"COMSPEC",
	L"DIRCMD",
	L"COPYCMD",
	L"winbootdir",
	L"windir",
	L"DIR",
	L"CALL",
	L"CHCP",
	L"RENAME",
	L"REN",
	L"ERASE",
	L"DEL",
	L"TYPE",
	L"REM",
	L"COPY",
	L"PAUSE",
	L"DATE",
	L"TIME",
	L"VER",
	L"VOL",
	L"CD",
	L"CHDIR",
	L"MD",
	L"MKDIR",
	L"RD",
	L"RMDIR",
	L"BREAK",
	L"VERIFY",
	L"SET",
	L"EXIT",
	L"CTTY",
	L"ECHO",
	L"@ECHO",	//Oct. 31, 2000 JEPRO '@' を強調可能にしたので追加
	L"LOCK",
	L"UNLOCK",
	L"GOTO",
	L"SHIFT",
	L"IF",
	L"FOR",
	L"DO",	//Nov. 2, 2000 JEPRO 追加
	L"IN",	//Nov. 2, 2000 JEPRO 追加
	L"ELSE",	//Nov. 2, 2000 JEPRO 追加 Win2000で使える
	L"CLS",
	L"TRUENAME",
	L"LOADHIGH",
	L"LH",
	L"LFNFOR",
	L"ON",
	L"OFF",
	L"NOT",
	L"ERRORLEVEL",
	L"EXIST",
	L"NUL",
	L"CON",
	L"AUX",
	L"COM1",
	L"COM2",
	L"COM3",
	L"COM4",
	L"PRN",
	L"LPT1",
	L"LPT2",
	L"LPT3",
	L"CLOCK",
	L"CLOCK$",
	L"CONFIG$"
};

static const wchar_t*	ppszKeyWordsPASCAL[] = {
	L"and",
	L"exports",
	L"mod",
	L"shr",
	L"array",
	L"file",
	L"nil",
	L"string",
	L"as",
	L"finalization",
	L"not",
	L"stringresource",
	L"asm",
	L"finally",
	L"object",
	L"then",
	L"begin",
	L"for",
	L"of",
	L"case",
	L"function",
	L"or",
	L"to",
	L"class",
	L"goto",
	L"out",
	L"try",
	L"const",
	L"if",
	L"packed",
	L"type",
	L"constructor",
	L"implementation",
	L"procedure",
	L"unit",
	L"destructor",
	L"in",
	L"program",
	L"until",
	L"dispinterface",
	L"inherited",
	L"property",
	L"uses",
	L"div",
	L"initialization",
	L"raise",
	L"var",
	L"do",
	L"inline",
	L"record",
	L"while",
	L"downto",
	L"interface",
	L"repeat",
	L"with",
	L"else",
	L"is",
	L"resourcestring",
	L"xor",
	L"end",
	L"label",
	L"set",
	L"except",
	L"library",
	L"shl",
	L"private",
	L"public",
	L"published",
	L"protected",
	L"override"
};

static const wchar_t*	ppszKeyWordsTEX[] = {
//Nov. 20, 2000 JEPRO	大幅追加 & 若干修正・削除 --ほとんどコマンドのみ
	L"error",
	L"Warning",
//			"center",
//			"document",
//			"enumerate",
//			"eqnarray",
//			"figure",
//			"itemize",
//			"minipage",
//			"tabbing",
//			"table",
//			"tabular",
//			"\\!",
//			"\\#",
//			"\\%",
//			"\\&",
//			"\\(",
//			"\\)",
//			"\\+",
//			"\\,",
//			"\\-",
//			"\\:",
//			"\\;",
//			"\\<",
//			"\\=",
//			"\\>",
	L"\\aa",
	L"\\AA",
	L"\\acute",
	L"\\addcontentsline",
	L"\\addtocounter",
	L"\\addtolength",
	L"\\ae",
	L"\\AE",
	L"\\aleph",
	L"\\alpha",
	L"\\alph",
	L"\\Alph",
	L"\\and",
	L"\\angle",
	L"\\appendix",
	L"\\approx",
	L"\\arabic",
	L"\\arccos",
	L"\\arctan",
	L"\\arg",
	L"\\arrayrulewidth",
	L"\\arraystretch",
	L"\\ast",
	L"\\atop",
	L"\\author",
	L"\\b",
	L"\\backslash",
	L"\\bar",
	L"\\baselineskip",
	L"\\baselinestretch",
	L"\\begin",
	L"\\beta",
	L"\\bf",
	L"\\bibitem",
	L"\\bibliography",
	L"\\bibliographystyle",
	L"\\big",
	L"\\Big",
	L"\\bigcap",
	L"\\bigcirc",
	L"\\bigcup",
	L"\\bigg",
	L"\\Bigg",
	L"\\Biggl",
	L"\\Biggm",
	L"\\biggl",
	L"\\biggm",
	L"\\biggr",
	L"\\Biggr",
	L"\\bigl",
	L"\\bigm",
	L"\\Bigm",
	L"\\Bigl",
	L"\\bigodot",
	L"\\bigoplus",
	L"\\bigotimes",
	L"\\bigr",
	L"\\Bigr",
	L"\\bigskip",
	L"\\bigtriangledown",
	L"\\bigtriangleup",
	L"\\boldmath",
	L"\\bot",
	L"\\Box",
	L"\\brace",
	L"\\breve",
	L"\\bullet",
	L"\\bye",
	L"\\c",
	L"\\cal",
	L"\\cap",
	L"\\caption",
	L"\\cc",
	L"\\cdot",
	L"\\cdots",
	L"\\centering",
	L"\\chapter",
	L"\\check",
	L"\\chi",
	L"\\choose",
	L"\\circ",
	L"\\circle",
	L"\\cite",
	L"\\clearpage",
	L"\\cline",
	L"\\closing",
	L"\\clubsuit",
	L"\\colon",
	L"\\columnsep",
	L"\\columnseprule",
	L"\\cong",
	L"\\cot",
	L"\\coth",
	L"\\cr",
	L"\\cup",
	L"\\d",
	L"\\dag",
	L"\\dagger",
	L"\\date",
	L"\\dashbox",
	L"\\ddag",
	L"\\ddot",
	L"\\ddots",
	L"\\def",
	L"\\deg",
	L"\\delta",
	L"\\Delta",
	L"\\det",
	L"\\diamond",
	L"\\diamondsuit",
	L"\\dim",
	L"\\displaystyle",
	L"\\documentclass",
	L"\\documentstyle",
	L"\\dot",
	L"\\doteq",
	L"\\dotfill",
	L"\\Downarrow",
	L"\\downarrow",
	L"\\ell",
	L"\\em",
	L"\\emptyset",
	L"\\encl",
	L"\\end",
	L"\\enspace",
	L"\\enskip",
	L"\\epsilon",
	L"\\eqno",
	L"\\equiv",
	L"\\evensidemargin",
	L"\\eta",
	L"\\exists",
	L"\\exp",
	L"\\fbox",
	L"\\fboxrule",
	L"\\flat",
	L"\\footnote",
	L"\\footnotesize",
	L"\\forall",
	L"\\frac",
	L"\\frame",
	L"\\framebox",
	L"\\gamma",
	L"\\Gamma",
	L"\\gcd",
	L"\\ge",
	L"\\geq",
	L"\\gets",
	L"\\gg",
	L"\\grave",
	L"\\gt",
	L"\\H",
	L"\\hat",
	L"\\hbar",
	L"\\hbox",
	L"\\headsep",
	L"\\heartsuit",
	L"\\hfil",
	L"\\hfill",
	L"\\hline",
	L"\\hom",
	L"\\hrulefill",
	L"\\hskip",
	L"\\hspace",
	L"\\hspace*",
	L"\\huge",
	L"\\Huge",
	L"\\i",
	L"\\Im",
	L"\\imath",
	L"\\in",
	L"\\include",
	L"\\includegraphics",
	L"\\includeonly",
	L"\\indent",
	L"\\index",
	L"\\inf",
	L"\\infty",
	L"\\input",
	L"\\int",
	L"\\iota",
	L"\\it",
	L"\\item",
	L"\\itemsep",
	L"\\j",
	L"\\jmath",
	L"\\kappa",
	L"\\ker",
	L"\\kern",
	L"\\kill",
	L"\\l",
	L"\\L",
	L"\\label",
	L"\\lambda",
	L"\\Lambda",
	L"\\land",
	L"\\langle",
	L"\\large",
	L"\\Large",
	L"\\LARGE",
	L"\\LaTeX",
	L"\\LaTeXe",
	L"\\lceil",
	L"\\ldots",
	L"\\le",
	L"\\leftarrow",
	L"\\Leftarrow",
	L"\\lefteqn",
	L"\\leftharpoondown",
	L"\\leftharpoonup",
	L"\\leftmargin",
	L"\\leftrightarrow",
	L"\\Leftrightarrow",
	L"\\leq",
	L"\\leqno",
	L"\\lfloor",
	L"\\lg",
	L"\\lim",
	L"\\liminf",
	L"\\limsup",
	L"\\line",
	L"\\linebreak",
	L"\\linewidth",
	L"\\listoffigures",
	L"\\listoftables",
	L"\\ll",
	L"\\llap",
	L"\\ln",
	L"\\lnot",
	L"\\log",
	L"\\longleftarrow",
	L"\\Longleftarrow",
	L"\\longleftrightarrow",
	L"\\Longleftrightarrow",
	L"\\longrightarrow",
	L"\\Longrightarrow",
	L"\\lor",
	L"\\lower",
	L"\\magstep",
	L"\\makeatletter",
	L"\\makeatother",
	L"\\makebox",
	L"\\makeindex",
	L"\\maketitle",
	L"\\makelabels",
	L"\\mathop",
	L"\\mapsto",
	L"\\markboth",
	L"\\markright",
	L"\\mathstrut",
	L"\\max",
	L"\\mbox",
	L"\\mc",
	L"\\medskip",
	L"\\mid",
	L"\\min",
	L"\\mit",
	L"\\mp",
	L"\\mu",
	L"\\multicolumn",
	L"\\multispan",
	L"\\multiput",
	L"\\nabla",
	L"\\natural",
	L"\\ne",
	L"\\neg",
	L"\\nearrow",
	L"\\nwarrow",
	L"\\neq",
	L"\\newblock",
	L"\\newcommand",
	L"\\newenvironment",
	L"\\newfont",
	L"\\newlength",
	L"\\newline",
	L"\\newpage",
	L"\\newtheorem",
	L"\\ni",
	L"\\noalign",
	L"\\noindent",
	L"\\nolimits",
	L"\\nolinebreak",
	L"\\nonumber",
	L"\\nopagebreak",
	L"\\normalsize",
	L"\\not",
	L"\\notice",
	L"\\notin",
	L"\\nu",
	L"\\o",
	L"\\O",
	L"\\oddsidemargin",
	L"\\odot",
	L"\\oe",
	L"\\OE",
	L"\\oint",
	L"\\Omega",
	L"\\omega",
	L"\\ominus",
	L"\\oplus",
	L"\\opening",
	L"\\otimes",
	L"\\owns",
	L"\\overleftarrow",
	L"\\overline",
	L"\\overrightarrow",
	L"\\overvrace",
	L"\\oval",
	L"\\P",
	L"\\pagebreak",
	L"\\pagenumbering",
	L"\\pageref",
	L"\\pagestyle",
	L"\\par",
	L"\\parallel",
	L"\\paragraph",
	L"\\parbox",
	L"\\parindent",
	L"\\parskip",
	L"\\partial",
	L"\\perp",
	L"\\phi",
	L"\\Phi",
	L"\\pi",
	L"\\Pi",
	L"\\pm",
	L"\\Pr",
	L"\\prime",
	L"\\printindex",
	L"\\prod",
	L"\\propto",
	L"\\ps",
	L"\\psi",
	L"\\Psi",
	L"\\put",
	L"\\qquad",
	L"\\quad",
	L"\\raisebox",
	L"\\rangle",
	L"\\rceil",
	L"\\Re",
	L"\\ref",
	L"\\renewcommand",
	L"\\renewenvironment",
	L"\\rfloor",
	L"\\rho",
	L"\\right",
	L"\\rightarrow",
	L"\\Rightarrow",
	L"\\rightharpoondown",
	L"\\rightharpoonup",
	L"\\rightleftharpoonup",
	L"\\rightmargin",
	L"\\rm",
	L"\\rule",
	L"\\roman",
	L"\\Roman",
	L"\\S",
	L"\\samepage",
	L"\\sb",
	L"\\sc",
	L"\\scriptsize",
	L"\\scriptscriptstyle",
	L"\\scriptstyle",
	L"\\searrow",
	L"\\sec",
	L"\\section",
	L"\\setcounter",
	L"\\setlength",
	L"\\settowidth",
	L"\\setminus",
	L"\\sf",
	L"\\sharp",
	L"\\sigma",
	L"\\Sigma",
	L"\\signature",
	L"\\sim",
	L"\\simeq",
	L"\\sin",
	L"\\sinh",
	L"\\sl",
	L"\\sloppy",
	L"\\small",
	L"\\smash",
	L"\\smallskip",
	L"\\sp",
	L"\\spadesuit",
	L"\\special",
	L"\\sqrt",
	L"\\ss",
	L"\\star",
	L"\\stackrel",
	L"\\strut",
	L"\\subparagraph",
	L"\\subsection",
	L"\\subset",
	L"\\subseteq",
	L"\\subsubsection",
	L"\\sum",
	L"\\sup",
	L"\\supset",
	L"\\supseteq",
	L"\\swarrow",
	L"\\t",
	L"\\tableofcontents",
	L"\\tan",
	L"\\tanh",
	L"\\tau",
	L"\\TeX",
	L"\\textbf",
	L"\\textgreater",
	L"\\textgt",
	L"\\textheight",
	L"\\textit",
	L"\\textless",
	L"\\textmc",
	L"\\textrm",
	L"\\textsc",
	L"\\textsf",
	L"\\textsl",
	L"\\textstyle",
	L"\\texttt",
	L"\\textwidth",
	L"\\thanks",
	L"\\thebibliography",
	L"\\theequation",
	L"\\thepage",
	L"\\thesection",
	L"\\theta",
	L"\\Theta",
	L"\\thicklines",
	L"\\thinlines",
	L"\\thinspace",
	L"\\thisepage",
	L"\\thisepagestyle",
	L"\\tie",
	L"\\tilde",
	L"\\times",
	L"\\tiny",
	L"\\title",
	L"\\titlepage",
	L"\\to",
	L"\\toaddress",
	L"\\topmargin",
	L"\\triangle",
	L"\\tt",
	L"\\twocolumn",
	L"\\u",
	L"\\underline",
	L"\\undervrace",
	L"\\unitlength",
	L"\\Uparrow",
	L"\\uparrow",
	L"\\updownarrow",
	L"\\Updownarrow",
	L"\\uplus",
	L"\\upsilon",
	L"\\Upsilon",
	L"\\usepackage",
	L"\\v",
	L"\\varepsilon",
	L"\\varphi",
	L"\\varpi",
	L"\\varrho",
	L"\\varsigma",
	L"\\vartheta",
	L"\\vbox",
	L"\\vcenter",
	L"\\vec",
	L"\\vector",
	L"\\vee",
	L"\\verb",
	L"\\verb*",
	L"\\verbatim",
	L"\\vert",
	L"\\Vert",
	L"\\vfil",
	L"\\vfill",
	L"\\vrule",
	L"\\vskip",
	L"\\vspace",
	L"\\vspace*",
	L"\\wedge",
	L"\\widehat",
	L"\\widetilde",
	L"\\wp",
	L"\\wr",
	L"\\wrapfigure",
	L"\\xi",
	L"\\Xi",
	L"\\zeta"//,
//			"\\[",
//			"\\\"",
//			"\\\'",
//			"\\\\",
//			"\\]",
//			"\\^",
//			"\\_",
//			"\\`",
//			"\\{",
//			"\\|",
//			"\\}",
//			"\\~",
};

//Jan. 19, 2001 JEPRO	TeX のキーワード2として新規追加 & 一部復活 --環境コマンドとオプション名が中心
static const wchar_t*	ppszKeyWordsTEX2[] = {
	//	環境コマンド
	//Jan. 19, 2001 JEPRO 本当は{}付きでキーワードにしたかったが単語として認識してくれないので止めた
	L"abstract",
	L"array"
	L"center",
	L"description",
	L"document",
	L"displaymath",
	L"em",
	L"enumerate",
	L"eqnarray",
	L"eqnarray*",
	L"equation",
	L"figure",
	L"figure*",
	L"floatingfigure",
	L"flushleft",
	L"flushright",
	L"itemize",
	L"letter",
	L"list",
	L"math",
	L"minipage",
	L"multicols",
	L"namelist",
	L"picture",
	L"quotation",
	L"quote",
	L"sloppypar",
	L"subeqnarray",
	L"subeqnarray*",
	L"subequations",
	L"subfigure",
	L"tabbing",
	L"table",
	L"table*",
	L"tabular",
	L"tabular*",
	L"tatepage",
	L"thebibliography",
	L"theindex",
	L"titlepage",
	L"trivlist",
	L"verbatim",
	L"verbatim*",
	L"verse",
	L"wrapfigure",
	//	スタイルオプション
	L"a4",
	L"a4j",
	L"a5",
	L"a5j",
	L"Alph",
	L"alph",
	L"annote",
	L"arabic",
	L"b4",
	L"b4j",
	L"b5",
	L"b5j",
	L"bezier",
	L"booktitle",
	L"boxedminipage",
	L"boxit",
//		"bp",
//		"cm",
	L"dbltopnumber",
//		"dd",
	L"eclepsf",
	L"eepic",
	L"enumi",
	L"enumii",
	L"enumiii",
	L"enumiv",
	L"epic",
	L"epsbox",
	L"epsf",
	L"fancybox",
	L"fancyheadings",
	L"fleqn",
	L"footnote",
	L"howpublished",
	L"jabbrv",
	L"jalpha",
//		"article",
	L"jarticle",
	L"jsarticle",
//		"book",
	L"jbook",
	L"jsbook",
//		"letter",
	L"jletter",
//		"plain",
	L"jplain",
//		"report",
	L"jreport",
	L"jtwocolumn",
	L"junsrt",
	L"leqno",
	L"makeidx",
	L"markboth",
	L"markright",
//		"mm",
	L"multicol",
	L"myheadings",
	L"openbib",
//		"pc",
//		"pt",
	L"secnumdepth",
//		"sp",
	L"titlepage",
	L"tjarticle",
	L"topnumber",
	L"totalnumber",
	L"twocolumn",
	L"twoside",
	L"yomi"//,
//		"zh",
//		"zw"
};

static const wchar_t*	ppszKeyWordsPERL[] = {
	//Jul. 10, 2001 JEPRO	変数を第２強調キーワードとして分離した
	L"break",
	L"continue",
	L"do",
	L"elsif",
	L"else",
	L"for",
	L"foreach",
	L"goto",
	L"if",
	L"last",
	L"next",
	L"return",
	L"sub",
	L"undef",
	L"unless",
	L"until",
	L"while",
	L"abs",
	L"accept",
	L"alarm",
	L"atan2",
	L"bind",
	L"binmode",
	L"bless",
	L"caller",
	L"chdir",
	L"chmod",
	L"chomp",
	L"chop",
	L"chown",
	L"chr",
	L"chroot",
	L"close",
	L"closedir",
	L"connect",
	L"continue",
	L"cos",
	L"crypt",
	L"dbmclose",
	L"dbmopen",
	L"defined",
	L"delete",
	L"die",
	L"do",
	L"dump",
	L"each",
	L"eof",
	L"eval",
	L"exec",
	L"exists",
	L"exit",
	L"exp",
	L"fcntl",
	L"fileno",
	L"flock",
	L"fork",
	L"format",
	L"formline",
	L"getc",
	L"getlogin",
	L"getpeername",
	L"getpgrp",
	L"getppid",
	L"getpriority",
	L"getpwnam",
	L"getgrnam",
	L"gethostbyname",
	L"getnetbyname",
	L"getprotobyname",
	L"getpwuid",
	L"getgrgid",
	L"getservbyname",
	L"gethostbyaddr",
	L"getnetbyaddr",
	L"getprotobynumber",
	L"getservbyport",
	L"getpwent",
	L"getgrent",
	L"gethostent",
	L"getnetent",
	L"getprotoent",
	L"getservent",
	L"setpwent",
	L"setgrent",
	L"sethostent",
	L"setnetent",
	L"setprotoent",
	L"setservent",
	L"endpwent",
	L"endgrent",
	L"endhostent",
	L"endnetent",
	L"endprotoent",
	L"endservent",
	L"getsockname",
	L"getsockopt",
	L"glob",
	L"gmtime",
	L"goto",
	L"grep",
	L"hex",
	L"import",
	L"index",
	L"int",
	L"ioctl",
	L"join",
	L"keys",
	L"kill",
	L"last",
	L"lc",
	L"lcfirst",
	L"length",
	L"link",
	L"listen",
	L"local",
	L"localtime",
	L"log",
	L"lstat",
//			"//m",
	L"map",
	L"mkdir",
	L"msgctl",
	L"msgget",
	L"msgsnd",
	L"msgrcv",
	L"my",
	L"next",
	L"no",
	L"oct",
	L"open",
	L"opendir",
	L"ord",
	L"our",	// 2006.04.20 genta
	L"pack",
	L"package",
	L"pipe",
	L"pop",
	L"pos",
	L"print",
	L"printf",
	L"prototype",
	L"push",
//			"//q",
	L"qq",
	L"qr",
	L"qx",
	L"qw",
	L"quotemeta",
	L"rand",
	L"read",
	L"readdir",
	L"readline",
	L"readlink",
	L"readpipe",
	L"recv",
	L"redo",
	L"ref",
	L"rename",
	L"require",
	L"reset",
	L"return",
	L"reverse",
	L"rewinddir",
	L"rindex",
	L"rmdir",
//			"//s",
	L"scalar",
	L"seek",
	L"seekdir",
	L"select",
	L"semctl",
	L"semget",
	L"semop",
	L"send",
	L"setpgrp",
	L"setpriority",
	L"setsockopt",
	L"shift",
	L"shmctl",
	L"shmget",
	L"shmread",
	L"shmwrite",
	L"shutdown",
	L"sin",
	L"sleep",
	L"socket",
	L"socketpair",
	L"sort",
	L"splice",
	L"split",
	L"sprintf",
	L"sqrt",
	L"srand",
	L"stat",
	L"study",
	L"sub",
	L"substr",
	L"symlink",
	L"syscall",
	L"sysopen",
	L"sysread",
	L"sysseek",
	L"system",
	L"syswrite",
	L"tell",
	L"telldir",
	L"tie",
	L"tied",
	L"time",
	L"times",
	L"tr",
	L"truncate",
	L"uc",
	L"ucfirst",
	L"umask",
	L"undef",
	L"unlink",
	L"unpack",
	L"untie",
	L"unshift",
	L"use",
	L"utime",
	L"values",
	L"vec",
	L"wait",
	L"waitpid",
	L"wantarray",
	L"warn",
	L"write"
};

//Jul. 10, 2001 JEPRO	変数を第２強調キーワードとして分離した
static const wchar_t*	ppszKeyWordsPERL2[] = {
	L"$ARGV",
	L"$_",
	L"$1",
	L"$2",
	L"$3",
	L"$4",
	L"$5",
	L"$6",
	L"$7",
	L"$8",
	L"$9",
	L"$0",
	L"$MATCH",
	L"$&",
	L"$PREMATCH",
	L"$`",
	L"$POSTMATCH",
	L"$'",
	L"$LAST_PAREN_MATCH",
	L"$+",
	L"$MULTILINE_MATCHING",
	L"$*",
	L"$INPUT_LINE_NUMBER",
	L"$NR",
	L"$.",
	L"$INPUT_RECORD_SEPARATOR",
	L"$RS",
	L"$/",
	L"$OUTPUT_AUTOFLUSH",
	L"$|",
	L"$OUTPUT_FIELD_SEPARATOR",
	L"$OFS",
	L"$,",
	L"$OUTPUT_RECORD_SEPARATOR",
	L"$ORS",
	L"$\\",
	L"$LIST_SEPARATOR",
	L"$\"",
	L"$SUBSCRIPT_SEPARATOR",
	L"$SUBSEP",
	L"$;",
	L"$OFMT",
	L"$#",
	L"$FORMAT_PAGE_NUMBER",
	L"$%",
	L"$FORMAT_LINES_PER_PAGE",
	L"$=",
	L"$FORMAT_LINES_LEFT",
	L"$-",
	L"$FORMAT_NAME",
	L"$~",
	L"$FORMAT_TOP_NAME",
	L"$^",
	L"$FORMAT_LINE_BREAK_CHARACTERS",
	L"$:",
	L"$FORMAT_FORMFEED",
	L"$^L",
	L"$ACCUMULATOR",
	L"$^A",
	L"$CHILD_ERROR",
	L"$?",
	L"$OS_ERROR",
	L"$ERRNO",
	L"$!",
	L"$EVAL_ERROR",
	L"$@",
	L"$PROCESS_ID",
	L"$PID",
	L"$$",
	L"$REAL_USER_ID",
	L"$UID",
	L"$<",
	L"$EFFECTIVE_USER_ID",
	L"$EUID",
	L"$>",
	L"$REAL_GROUP_ID",
	L"$GID",
	L"$(",
	L"$EFFECTIVE_GROUP_ID",
	L"$EGID",
	L"$)",
	L"$PROGRAM_NAME",
	L"$0",
	L"$[",
	L"$PERL_VERSION",
	L"$]",
	L"$DEBUGGING",
	L"$^D",
	L"$SYSTEM_FD_MAX",
	L"$^F",
	L"$INPLACE_EDIT",
	L"$^I",
	L"$PERLDB",
	L"$^P",
	L"$BASETIME",
	L"$^T",
	L"$WARNING",
	L"$^W",
	L"$EXECUTABLE_NAME",
	L"$^X",
	L"$ARGV",
	L"$ENV",
	L"$SIG"
};

//Jul. 10, 2001 JEPRO 追加
static const wchar_t*	ppszKeyWordsVB[] = {
	L"And",
	L"As",
	L"Attribute",
	L"Begin",
	L"BeginProperty",
	L"Boolean",
	L"ByVal",
	L"Byte",
	L"Call",
	L"Case",
	L"Const",
	L"Currency",
	L"Date",
	L"Declare",
	L"Dim",
	L"Do",
	L"Double",
	L"Each",
	L"Else",
	L"ElseIf",
	L"Empty",
	L"End",
	L"EndProperty",
	L"Error",
	L"Eqv",
	L"Exit",
	L"False",
	L"For",
	L"Friend",
	L"Function",
	L"Get",
	L"GoTo",
	L"If",
	L"Imp",
	L"Integer",
	L"Is",
	L"Let",
	L"Like",
	L"Long",
	L"Loop",
	L"Me",
	L"Mod",
	L"New",
	L"Next",
	L"Not",
	L"Null",
	L"Object",
	L"On",
	L"Option",
	L"Or",
	L"Private",
	L"Property",
	L"Public",
	L"RSet",
	L"ReDim",
	L"Rem",
	L"Resume",
	L"Select",
	L"Set",
	L"Single",
	L"Static",
	L"Step",
	L"Stop",
	L"String",
	L"Sub",
	L"Then",
	L"To",
	L"True",
	L"Type",
	L"Wend",
	L"While",
	L"With",
	L"Xor",
	L"#If",
	L"#Else",
	L"#End",
	L"#Const",
	L"AddressOf",
	L"Alias",
	L"Append",
	L"Array",
	L"ByRef",
	L"Explicit",
	L"Global",
	L"In",
	L"Lib",
	L"Nothing",
	L"Optional",
	L"Output",
	L"Terminate",
	L"Until",
	//=========================================================
	// 以下はVB.NET(VB7)での廃止が決定しているキーワードです
	//=========================================================
	L"DefBool",
	L"DefByte",
	L"DefCur",
	L"DefDate",
	L"DefDbl",
	L"DefInt",
	L"DefLng",
	L"DefObj",
	L"DefSng",
	L"DefStr",
	L"DefVar",
	L"LSet",
	L"GoSub",
	L"Return",
	L"Variant",
	//			"Option Base
	//			"As Any
	//=========================================================
	// 以下はVB.NET用キーワードです
	//=========================================================
	//BitAnd
	//BitOr
	//BitNot
	//BitXor
	//Delegate
	//Short
	//Structure
};

//Jul. 10, 2001 JEPRO 追加
static const wchar_t*	ppszKeyWordsVB2[] = {
	L"AppActivate",
	L"Beep",
	L"BeginTrans",
	L"ChDir",
	L"ChDrive",
	L"Close",
	L"CommitTrans",
	L"CompactDatabase",
	L"Date",
	L"DeleteSetting",
	L"Erase",
	L"FileCopy",
	L"FreeLocks",
	L"Input",
	L"Kill",
	L"Load",
	L"Lock",
	L"Mid",
	L"MidB",
	L"MkDir",
	L"Name",
	L"Open",
	L"Print",
	L"Put",
	L"Randomize",
	L"RegisterDatabase",
	L"RepairDatabase",
	L"Reset",
	L"RmDir",
	L"Rollback",
	L"SavePicture",
	L"SaveSetting",
	L"Seek",
	L"SendKeys",
	L"SetAttr",
	L"SetDataAccessOption",
	L"SetDefaultWorkspace",
	L"Time",
	L"Unload",
	L"Unlock",
	L"Width",
	L"Write",
	L"Array",
	L"Asc",
	L"AscB",
	L"Atn",
	L"CBool",
	L"CByte",
	L"CCur",
	L"CDate",
	L"CDbl",
	L"CInt",
	L"CLng",
	L"CSng",
	L"CStr",
	L"CVErr",
	L"CVar",
	L"Choose",
	L"Chr",
	L"ChrB",
	L"Command",
	L"Cos",
	L"CreateDatabase",
	L"CreateObject",
	L"CurDir",
	L"DDB",
	L"Date",
	L"DateAdd",
	L"DateDiff",
	L"DatePart",
	L"DateSerial",
	L"DateValue",
	L"Day",
	L"Dir",
	L"DoEvents",
	L"EOF",
	L"Environ",
	L"Error",
	L"Exp",
	L"FV",
	L"FileAttr",
	L"FileDateTime",
	L"FileLen",
	L"Fix",
	L"Format",
	L"FreeFile",
	L"GetAllSettings",
	L"GetAttr",
	L"GetObject",
	L"GetSetting",
	L"Hex",
	L"Hour",
	L"IIf",
	L"IMEStatus",
	L"IPmt",
	L"IRR",
	L"InStr",
	L"Input",
	L"Int",
	L"IsArray",
	L"IsDate",
	L"IsEmpty",
	L"IsError",
	L"IsMissing",
	L"IsNull",
	L"IsNumeric",
	L"IsObject",
	L"LBound",
	L"LCase",
	L"LOF",
	L"LTrim",
	L"Left",
	L"LeftB",
	L"Len",
	L"LoadPicture",
	L"Loc",
	L"Log",
	L"MIRR",
	L"Mid",
	L"MidB",
	L"Minute",
	L"Month",
	L"MsgBox",
	L"NPV",
	L"NPer",
	L"Now",
	L"Oct",
	L"OpenDatabase",
	L"PPmt",
	L"PV",
	L"Partition",
	L"Pmt",
	L"QBColor",
	L"RGB",
	L"RTrim",
	L"Rate",
	L"ReadProperty",
	L"Right",
	L"RightB",
	L"Rnd",
	L"SLN",
	L"SYD",
	L"Second",
	L"Seek",
	L"Sgn",
	L"Shell",
	L"Sin",
	L"Space",
	L"Spc",
	L"Sqr",
	L"Str",
	L"StrComp",
	L"StrConv",
	L"Switch",
	L"Tab",
	L"Tan",
	L"Time",
	L"TimeSerial",
	L"TimeValue",
	L"Timer",
	L"Trim",
	L"TypeName",
	L"UBound",
	L"UCase",
	L"Val",
	L"VarType",
	L"Weekday",
	L"Year",
	L"Hide",
	L"Line",
	L"Refresh",
	L"Show",
	//=========================================================
	// 以下はVB.NET(VB7)での廃止が決定しているキーワードです
	//=========================================================
	//$付き関数各種
	L"Dir$",
	L"LCase$",
	L"Left$",
	L"LeftB$",
	L"Mid$",
	L"MidB$",
	L"RightB$",
	L"Right$",
	L"Space$",
	L"Str$",
	L"String$",
	L"Trim$",
	L"UCase$",
	//VB5,6の隠し関数
	L"VarPtr",
	L"StrPtr",
	L"ObjPtr",
	L"VarPrtArray",
	L"VarPtrStringArray"
};

//Jul. 10, 2001 JEPRO 追加
static const wchar_t*	ppszKeyWordsRTF[] = {
	L"\\ansi",
	L"\\b",
	L"\\bin",
	L"\\box",
	L"\\brdrb",
	L"\\brdrbar",
	L"\\brdrdb",
	L"\\brdrdot",
	L"\\brdrl",
	L"\\brdrr",
	L"\\brdrs",
	L"\\brdrsh",
	L"\\brdrt",
	L"\\brdrth",
	L"\\cell",
	L"\\cellx",
	L"\\cf",
	L"\\chftn",
	L"\\clmgf",
	L"\\clmrg",
	L"\\colortbl",
	L"\\deff",
	L"\\f",
	L"\\fi",
	L"\\field",
	L"\\fldrslt",
	L"\\fonttbl",
	L"\\footnote",
	L"\\fs",
	L"\\i"
	L"\\intbl",
	L"\\keep",
	L"\\keepn",
	L"\\li",
	L"\\line",
	L"\\mac",
	L"\\page",
	L"\\par",
	L"\\pard",
	L"\\pc",
	L"\\pich",
	L"\\pichgoal",
	L"\\picscalex",
	L"\\picscaley",
	L"\\pict",
	L"\\picw",
	L"\\picwgoal",
	L"\\plain",
	L"\\qc",
	L"\\ql",
	L"\\qr",
	L"\\ri",
	L"\\row",
	L"\\rtf",
	L"\\sa",
	L"\\sb",
	L"\\scaps",
	L"\\sect",
	L"\\sl",
	L"\\strike",
	L"\\tab",
	L"\\tqc",
	L"\\tqr",
	L"\\trgaph",
	L"\\trleft",
	L"\\trowd",
	L"\\trqc",
	L"\\trql",
	L"\\tx",
	L"\\ul",
	L"\\uldb",
	L"\\v",
	L"\\wbitmap",
	L"\\wbmbitspixel",
	L"\\wbmplanes",
	L"\\wbmwidthbytes",
	L"\\wmetafile",
	L"bmc",
	L"bml",
	L"bmr",
	L"emc",
	L"eml",
	L"emr"
};

/*!	@brief 共有メモリ初期化/強調キーワード

	強調キーワード関連の初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離．
		キーワード定義を関数の外に出し，登録をマクロ化して簡潔に．
*/
void CShareData::InitKeyword(DLLSHAREDATA* pShareData)
{
	/* 強調キーワードのテストデータ */
	pShareData->m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = 0;

	int nSetCount = -1;

#define PopulateKeyword(name,case_sensitive,ary) \
	pShareData->m_CKeyWordSetMgr.AddKeyWordSet( (name), (case_sensitive) );	\
	pShareData->m_CKeyWordSetMgr.SetKeyWordArr( ++nSetCount, _countof(ary), (ary) );
	
	PopulateKeyword( L"C/C++", TRUE, ppszKeyWordsCPP );			/* セット 0の追加 */
	PopulateKeyword( L"HTML", FALSE, ppszKeyWordsHTML );			/* セット 1の追加 */
	PopulateKeyword( L"PL/SQL", FALSE, ppszKeyWordsPLSQL );		/* セット 2の追加 */
	PopulateKeyword( L"COBOL", TRUE ,ppszKeyWordsCOBOL );		/* セット 3の追加 */
	PopulateKeyword( L"Java", TRUE, ppszKeyWordsJAVA );			/* セット 4の追加 */
	PopulateKeyword( L"CORBA IDL", TRUE, ppszKeyWordsCORBA_IDL );/* セット 5の追加 */
	PopulateKeyword( L"AWK", TRUE, ppszKeyWordsAWK );			/* セット 6の追加 */
	PopulateKeyword( L"MS-DOS batch", FALSE, ppszKeyWordsBAT );	/* セット 7の追加 */	//Oct. 31, 2000 JEPRO 'バッチファイル'→'batch' に短縮
	PopulateKeyword( L"Pascal", FALSE, ppszKeyWordsPASCAL );		/* セット 8の追加 */	//Nov. 5, 2000 JEPRO 大・小文字の区別を'しない'に変更
	PopulateKeyword( L"TeX", TRUE, ppszKeyWordsTEX );			/* セット 9の追加 */	//Sept. 2, 2000 jepro Tex →TeX に修正 Bool値は大・小文字の区別
	PopulateKeyword( L"TeX2", TRUE, ppszKeyWordsTEX2 );			/* セット10の追加 */	//Jan. 19, 2001 JEPRO 追加
	PopulateKeyword( L"Perl", TRUE, ppszKeyWordsPERL );			/* セット11の追加 */
	PopulateKeyword( L"Perl2", TRUE, ppszKeyWordsPERL2 );		/* セット12の追加 */	//Jul. 10, 2001 JEPRO Perlから変数を分離・独立
	PopulateKeyword( L"Visual Basic", FALSE, ppszKeyWordsVB );	/* セット13の追加 */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( L"Visual Basic2", FALSE, ppszKeyWordsVB2 );	/* セット14の追加 */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( L"リッチテキスト", TRUE, ppszKeyWordsRTF );	/* セット15の追加 */	//Jul. 10, 2001 JEPRO

#undef PopulateKeyword
}

/*!	@brief 共有メモリ初期化/キー割り当て

	デフォルトキー割り当て関連の初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離
	@date 2007.11.04 genta キー設定数がDLLSHAREの領域を超えたら起動できないように
*/
bool CShareData::InitKeyAssign(DLLSHAREDATA* pShareData)
{
	/********************/
	/* 共通設定の規定値 */
	/********************/
	struct KEYDATAINIT {
		short			nKeyCode;		//!< Key Code (0 for non-keybord button)
		TCHAR*			pszKeyName;		//!< Key Name (for display)
		EFunctionCode	nFuncCode_0;	//!<                      Key
		EFunctionCode	nFuncCode_1;	//!< Shift +              Key
		EFunctionCode	nFuncCode_2;	//!<         Ctrl +       Key
		EFunctionCode	nFuncCode_3;	//!< Shift + Ctrl +       Key
		EFunctionCode	nFuncCode_4;	//!<                Alt + Key
		EFunctionCode	nFuncCode_5;	//!< Shift +        Alt + Key
		EFunctionCode	nFuncCode_6;	//!<         Ctrl + Alt + Key
		EFunctionCode	nFuncCode_7;	//!< Shift + Ctrl + Alt + Key
	};
	static KEYDATAINIT	KeyDataInit[] = {
	//Sept. 1, 2000 Jepro note: key binding
	//Feb. 17, 2001 jepro note 2: 順番は2進で下位3ビット[Alt][Ctrl][Shift]の組合せの順(それに2を加えた値)
	//		0,		1,		 2(000), 3(001),4(010),	5(011),		6(100),	7(101),		8(110),		9(111)
	//		keycode, keyname, なし, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
	//
		/* マウスボタン */
		{ 0, _T("ダブルクリック"),		F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD,	F_SELECTWORD }, //Feb. 19, 2001 JEPRO Altと右クリックの組合せは効かないので右クリックメニューのキー割り当てをはずした
		{ 0, _T("右クリック"),			F_MENU_RBUTTON,	F_MENU_RBUTTON,	F_MENU_RBUTTON,	F_MENU_RBUTTON,	F_0,			F_0,			F_0,			F_0 },
		{ 0, _T("中クリック"),			F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0 }, // novice 2004/10/11 マウス中ボタン対応
		{ 0, _T("左サイドクリック"),	F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0 }, // novice 2004/10/10 マウスサイドボタン対応
		{ 0, _T("右サイドクリック"),	F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0,			F_0 },
	// 2007.11.15 nasukoji	トリプルクリック・クアドラプルクリック対応
		{ 0, _T("トリプルクリック"), F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE },
		{ 0, _T("クアドラプルクリック"), F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL },

		/* ファンクションキー */
	//	From Here Sept. 14, 2000 JEPRO
	//	VK_F1,_T("F1"), F_EXTHTMLHELP, F_0, F_EXTHELP1, 0, 0, 0, 0, 0,
	//	Shift+F1 に「コマンド一覧」, Alt+F1 に「ヘルプ目次」, Shift+Alt+F1 に「キーワード検索」を追加	//Nov. 25, 2000 JEPRO 殺していたのを修正・復活
	//Dec. 25, 2000 JEPRO Shift+Ctrl+F1 に「バージョン情報」を追加
	//	{ VK_F1,_T("F1"), F_EXTHTMLHELP, F_MENU_ALLFUNC, F_EXTHELP1, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F1,_T("F1"), F_EXTHTMLHELP, F_MENU_ALLFUNC, F_EXTHELP1, F_ABOUT, F_HELP_CONTENTS, F_HELP_SEARCH, F_0, F_0 },
	//	To Here Sept. 14, 2000
	// From Here 2001.12.03 hor F2にブックマーク関連を割当
	//	{ VK_F2,_T("F2"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F2,_T("F2"), F_BOOKMARK_NEXT, F_BOOKMARK_PREV, F_BOOKMARK_SET, F_BOOKMARK_RESET, F_BOOKMARK_VIEW, F_0, F_0, F_0 },
	// To Here 2001.12.03 hor
		//Sept. 21, 2000 JEPRO	Ctrl+F3 に「検索マークのクリア」を追加
		//Aug. 12, 2002 ai	Ctrl+Shift+F3 に「検索開始位置へ戻る」を追加
		{ VK_F3,_T("F3"), F_SEARCH_NEXT, F_SEARCH_PREV, F_SEARCH_CLEARMARK, F_JUMP_SRCHSTARTPOS, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Alt+F4 に「ウィンドウを閉じる」, Shift+Alt+F4 に「すべてのウィンドウを閉じる」を追加
		//	Ctrl+F4に割り当てられていた「縦横に分割」を「閉じて(無題)」に変更し Shift+Ctrl+F4 に「閉じて開く」を追加
		//Jan. 14, 2001 Ctrl+Alt+F4 に「テキストエディタの全終了」を追加
		//Jun. 2001「サクラエディタの全終了」に改称
		//2006.10.21 ryoji Alt+F4 には何も割り当てない（デフォルトのシステムコマンド「閉じる」が実行されるように）
		//2007.02.13 ryoji Shift+Ctrl+F4をF_WIN_CLOSEALLからF_EXITALLEDITORSに変更
		//2007.02.22 ryoji Ctrl+F4 への割り当てを削除（デフォルトのコマンドを実行）
		{ VK_F4,_T("F4"), F_SPLIT_V, F_SPLIT_H, F_0, F_FILECLOSE_OPEN, F_0, F_EXITALLEDITORS, F_EXITALL, F_0 },
	//	From Here Sept. 20, 2000 JEPRO Ctrl+F5 に「外部コマンド実行」を追加  なおマクロ名はCMMAND からCOMMAND に変更済み
	//	{ VK_F5,_T("F5"), F_PLSQL_COMPILE_ON_SQLPLUS, F_0, F_EXECCOMMAND_DIALOG, F_0, F_0, F_0, F_0, F_0 },
	//	To Here Sept. 20, 2000
		//Oct. 28, 2000 F5 は「再描画」に変更	//Jan. 14, 2001 Alt+F5 に「uudecodeして保存」, Ctrl+ Alt+F5 に「TAB→空白」を追加
		//	May 28, 2001 genta	S-C-A-F5にSPACE-to-TABを追加
		{ VK_F5,_T("F5"), F_REDRAW, F_0, F_EXECCOMMAND_DIALOG, F_0, F_UUDECODE, F_0, F_TABTOSPACE, F_SPACETOTAB },
		//Jan. 14, 2001 JEPRO	Ctrl+F6 に「小文字」, Alt+F6 に「Base64デコードして保存」を追加
		{ VK_F6,_T("F6"), F_BEGIN_SEL, F_BEGIN_BOX, F_TOLOWER, F_0, F_BASE64DECODE, F_0, F_0, F_0 },
		//Jan. 14, 2001 JEPRO	Ctrl+F7 に「大文字」, Alt+F7 に「UTF-7→SJISコード変換」, Shift+Alt+F7 に「SJIS→UTF-7コード変換」, Ctrl+Alt+F7 に「UTF-7で開き直す」を追加
		{ VK_F7,_T("F7"), F_CUT, F_0, F_TOUPPER, F_0, F_CODECNV_UTF72SJIS, F_CODECNV_SJIS2UTF7, F_FILE_REOPEN_UTF7, F_0 },
		//Nov. 9, 2000 JEPRO	Shift+F8 に「CRLF改行でコピー」を追加
		//Jan. 14, 2001 JEPRO	Ctrl+F8 に「全角→半角」, Alt+F8 に「UTF-8→SJISコード変換」, Shift+Alt+F8 に「SJIS→UTF-8コード変換」, Ctrl+Alt+F8 に「UTF-8で開き直す」を追加
		{ VK_F8,_T("F8"), F_COPY, F_COPY_CRLF, F_TOHANKAKU, F_0, F_CODECNV_UTF82SJIS, F_CODECNV_SJIS2UTF8, F_FILE_REOPEN_UTF8, F_0 },
		//Jan. 14, 2001 JEPRO	Ctrl+F9 に「半角＋全ひら→全角・カタカナ」, Alt+F9 に「Unicode→SJISコード変換」, Ctrl+Alt+F9 に「Unicodeで開き直す」を追加
		{ VK_F9,_T("F9"), F_PASTE, F_PASTEBOX, F_TOZENKAKUKATA, F_0, F_CODECNV_UNICODE2SJIS, F_0, F_FILE_REOPEN_UNICODE, F_0 },
		//Oct. 28, 2000 JEPRO F10 に「SQL*Plusで実行」を追加(F5からの移動)
		//Jan. 14, 2001 JEPRO	Ctrl+F10 に「半角＋全カタ→全角・ひらがな」, Alt+F10 に「EUC→SJISコード変換」, Shift+Alt+F10 に「SJIS→EUCコード変換」, Ctrl+Alt+F10 に「EUCで開き直す」を追加
		{ VK_F10,_T("F10"), F_PLSQL_COMPILE_ON_SQLPLUS, F_DUPLICATELINE, F_TOZENKAKUHIRA, F_0, F_CODECNV_EUC2SJIS, F_CODECNV_SJIS2EUC, F_FILE_REOPEN_EUC, F_0 },
		//Jan. 14, 2001 JEPRO	Shift+F11 に「SQL*Plusをアクティブ表示」, Ctrl+F11 に「半角カタカナ→全角カタカナ」, Alt+F11 に「E-Mail(JIS→SJIS)コード変換」, Shift+Alt+F11 に「SJIS→JISコード変換」, Ctrl+Alt+F11 に「JISで開き直す」を追加
		{ VK_F11,_T("F11"), F_OUTLINE, F_ACTIVATE_SQLPLUS, F_HANKATATOZENKAKUKATA, F_0, F_CODECNV_EMAIL, F_CODECNV_SJIS2JIS, F_FILE_REOPEN_JIS, F_0 },
		//Jan. 14, 2001 JEPRO	Ctrl+F12 に「半角カタカナ→全角ひらがな」, Alt+F12 に「自動判別→SJISコード変換」, Ctrl+Alt+F11 に「SJISで開き直す」を追加
		{ VK_F12,_T("F12"), F_TAGJUMP, F_TAGJUMPBACK, F_HANKATATOZENKAKUHIRA, F_0, F_CODECNV_AUTO2SJIS, F_0, F_FILE_REOPEN_SJIS, F_0 },
		{ VK_F13,_T("F13"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F14,_T("F14"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F15,_T("F15"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F16,_T("F16"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F17,_T("F17"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F18,_T("F18"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F19,_T("F19"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F20,_T("F20"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F21,_T("F21"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F22,_T("F22"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F23,_T("F23"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_F24,_T("F24"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		/* 特殊キー */
		{ VK_TAB,_T("Tab"),F_INDENT_TAB, F_UNINDENT_TAB, F_NEXTWINDOW, F_PREVWINDOW, F_0, F_0, F_0, F_0 },
		//Sept. 1, 2000 JEPRO	Alt+Enter に「ファイルのプロパティ」を追加	//Oct. 15, 2000 JEPRO Ctrl+Enter に「ファイル内容比較」を追加
			{ VK_RETURN,_T("Enter"),F_0, F_0, F_COMPARE, F_0, F_PROPERTY_FILE, F_0, F_0, F_0 },
		{ VK_ESCAPE,_T("Esc"),F_CANCEL_MODE, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
//			{ VK_BACK,_T("BackSpace"),F_DELETE_BACK, F_0, F_WordDeleteToStart, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO 長いので名称を簡略形に変更(BackSpace→BkSp)
		{ VK_BACK,_T("BkSp"),F_DELETE_BACK, F_0, F_WordDeleteToStart, F_0, F_0, F_0, F_0, F_0 },
//			{ VK_INSERT,_T("Insert"),F_CHGMOD_INS, F_PASTE, F_COPY, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(Insert→Ins)
		{ VK_INSERT,_T("Ins"),F_CHGMOD_INS, F_PASTE, F_COPY, F_0, F_0, F_0, F_0, F_0 },
//			{ VK_DELETE,_T("Delete"),F_DELETE, F_0, F_WordDeleteToEnd, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(Delete→Del)
		//Jun. 26, 2001 JEPRO	Shift+Del に「切り取り」を追加
		{ VK_DELETE,_T("Del"),F_DELETE, F_CUT, F_WordDeleteToEnd, F_0, F_0, F_0, F_0, F_0 },
		{ VK_HOME,_T("Home"),F_GOLINETOP, F_GOLINETOP_SEL, F_GOFILETOP, F_GOFILETOP_SEL, F_0, F_0, F_0, F_0 },
		{ VK_END,_T("End(Help)"),F_GOLINEEND, F_GOLINEEND_SEL, F_GOFILEEND, F_GOFILEEND_SEL, F_0, F_0, F_0, F_0 },
		{ VK_LEFT,_T("←"),F_LEFT, F_LEFT_SEL/*F_GOLINETOP*/, F_WORDLEFT, F_WORDLEFT_SEL, F_BEGIN_BOX, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Shift+Ctrl+Alt+↑に「縦方向に最大化」を追加
//			{ VK_UP,_T("↑"),F_UP, F_UP_SEL, F_UP2, F_UP2_SEL, F_BEGIN_BOX, F_0, F_0, F_MAXIMIZE_V },
		//Jun. 27, 2001 JEPRO
		//	Ctrl+↑に割り当てられていた「カーソル上移動(２行ごと)」を「テキストを１行下へスクロール」に変更
		{ VK_UP,_T("↑"),F_UP, F_UP_SEL, F_WndScrollDown, F_UP2_SEL, F_BEGIN_BOX, F_0, F_0, F_MAXIMIZE_V },
		//2001.02.10 by MIK Shift+Ctrl+Alt+→に「横方向に最大化」を追加
		{ VK_RIGHT,_T("→"),F_RIGHT, F_RIGHT_SEL/*F_GOLINEEND*/, F_WORDRIGHT, F_WORDRIGHT_SEL, F_BEGIN_BOX, F_0, F_0, F_MAXIMIZE_H },
		//Sept. 14, 2000 JEPRO
		//	Ctrl+↓に割り当てられていた「右クリックメニュー」を「カーソル下移動(２行ごと)」に変更
		//	それに付随してさらに「右クリックメニュー」をCtrl＋Alt＋↓に変更
//			{ VK_DOWN,_T("↓"),F_DOWN, F_DOWN_SEL, F_DOWN2, F_DOWN2_SEL, F_BEGIN_BOX, F_0, F_MENU_RBUTTON, F_MINIMIZE_ALL },
		//Jun. 27, 2001 JEPRO
		//	Ctrl+↓に割り当てられていた「カーソル下移動(２行ごと)」を「テキストを１行上へスクロール」に変更
		{ VK_DOWN,_T("↓"),F_DOWN, F_DOWN_SEL, F_WndScrollUp, F_DOWN2_SEL, F_BEGIN_BOX, F_0, F_MENU_RBUTTON, F_MINIMIZE_ALL },
//			{ VK_PRIOR,_T("RollDown(PageUp)"),F_ROLLDOWN, F_ROLLDOWN_SEL, F_0, F_0, F_0, F_0, F_0, F_0 },
//			{ VK_NEXT,_T("RollUp(PageDown)"),F_ROLLUP, F_ROLLUP_SEL, F_0, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn にそれぞれ「１ページダウン」, 「(選択)１ページダウン」を追加
		//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に交換(RollUp→PgDn) //Oct. 10, 2000 JEPRO 名称変更
		//2001.12.03 hor 1Page/HalfPage 入替え
		{ VK_NEXT,_T("PgDn(RollUp)"), F_1PageDown, F_1PageDown_Sel,F_HalfPageDown, F_HalfPageDown_Sel, F_0, F_0, F_0, F_0 },
		//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn にそれぞれ「１ページアップ」, 「(選択)１ページアップ」を追加
		//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に交換(RollDown→PgUp) //Oct. 10, 2000 JEPRO 名称変更
		//2001.12.03 hor 1Page/HalfPage 入替え
		{ VK_PRIOR,_T("PgUp(RollDn)"), F_1PageUp, F_1PageUp_Sel,F_HalfPageUp, F_HalfPageUp_Sel, F_0, F_0, F_0, F_0 },
//			{ VK_SPACE,_T("SpaceBar"),F_INDENT_SPACE, F_UNINDENT_SPACE, F_HOKAN, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(SpaceBar→Space)
		{ VK_SPACE,_T("Space"),F_INDENT_SPACE, F_UNINDENT_SPACE, F_HOKAN, F_0, F_0, F_0, F_0, F_0 },
		/* 数字 */
		//Oct. 7, 2000 JEPRO	Ctrl+0 を「タイプ別設定一覧」→「未定義」に変更
		//Jan. 13, 2001 JEPRO	Alt+0 に「カスタムメニュー10」, Shift+Alt+0 に「カスタムメニュー20」を追加
		{ '0', _T("0"),F_0, F_0, F_0, F_0, F_CUSTMENU_10, F_CUSTMENU_20, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+1 を「タイプ別設定」→「ツールバーの表示」に変更
		//Jan. 13, 2001 JEPRO	Alt+1 に「カスタムメニュー1」, Shift+Alt+1 に「カスタムメニュー11」を追加
		//Jan. 19, 2001 JEPRO	Shift+Ctrl+1 に「カスタムメニュー21」を追加
		{ '1', _T("1"),F_0, F_0, F_SHOWTOOLBAR, F_CUSTMENU_21, F_CUSTMENU_1, F_CUSTMENU_11, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+2 を「共通設定」→「ファンクションキーの表示」に変更
		//Jan. 13, 2001 JEPRO	Alt+2 を「アウトプット」→「カスタムメニュー2」に変更し「アウトプット」は Alt+O に移動, Shift+Alt+2 に「カスタムメニュー12」を追加
		//Jan. 19, 2001 JEPRO	Shift+Ctrl+2 に「カスタムメニュー22」を追加
		{ '2', _T("2"),F_0, F_0, F_SHOWFUNCKEY, F_CUSTMENU_22, F_CUSTMENU_2/*F_WIN_OUTPUT*/, F_CUSTMENU_12, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+3 を「フォント設定」→「ステータスバーの表示」に変更
		//Jan. 13, 2001 JEPRO	Alt+3 に「カスタムメニュー3」, Shift+Alt+3 に「カスタムメニュー13」を追加
		//Jan. 19, 2001 JEPRO	Shift+Ctrl+3 に「カスタムメニュー23」を追加
		{ '3', _T("3"),F_0, F_0, F_SHOWSTATUSBAR, F_CUSTMENU_23, F_CUSTMENU_3, F_CUSTMENU_13, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+4 を「ツールバーの表示」→「タイプ別設定一覧」に変更
		//Jan. 13, 2001 JEPRO	Alt+4 に「カスタムメニュー4」, Shift+Alt+4 に「カスタムメニュー14」を追加
		//Jan. 19, 2001 JEPRO	Shift+Ctrl+4 に「カスタムメニュー24」を追加
		{ '4', _T("4"),F_0, F_0, F_TYPE_LIST, F_CUSTMENU_24, F_CUSTMENU_4, F_CUSTMENU_14, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+5 を「ファンクションキーの表示」→「タイプ別設定」に変更
		//Jan. 13, 2001 JEPRO	Alt+5 に「カスタムメニュー5」, Shift+Alt+5 に「カスタムメニュー15」を追加
		{ '5', _T("5"),F_0, F_0, F_OPTION_TYPE, F_0, F_CUSTMENU_5, F_CUSTMENU_15, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+6 を「ステータスバーの表示」→「共通設定」に変更
		//Jan. 13, 2001 JEPRO	Alt+6 に「カスタムメニュー6」, Shift+Alt+6 に「カスタムメニュー16」を追加
		{ '6', _T("6"),F_0, F_0, F_OPTION, F_0, F_CUSTMENU_6, F_CUSTMENU_16, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+7 に「フォント設定」を追加
		//Jan. 13, 2001 JEPRO	Alt+7 に「カスタムメニュー7」, Shift+Alt+7 に「カスタムメニュー17」を追加
		{ '7', _T("7"),F_0, F_0, F_FONT, F_0, F_CUSTMENU_7, F_CUSTMENU_17, F_0, F_0 },
		//Jan. 13, 2001 JEPRO	Alt+8 に「カスタムメニュー8」, Shift+Alt+8 に「カスタムメニュー18」を追加
		{ '8', _T("8"),F_0, F_0, F_0, F_0, F_CUSTMENU_8, F_CUSTMENU_18, F_0, F_0 },
		//Jan. 13, 2001 JEPRO	Alt+9 に「カスタムメニュー9」, Shift+Alt+9 に「カスタムメニュー19」を追加
		{ '9', _T("9"),F_0, F_0, F_0, F_0, F_CUSTMENU_9, F_CUSTMENU_19, F_0, F_0 },
		/* アルファベット */
		//2001.12.06 hor Alt+A を「SORT_ASC」に割当
		{ 'A', _T("A"),F_0, F_0, F_SELECTALL, F_0, F_SORT_ASC, F_0, F_0, F_0 },
		//Jan. 13, 2001 JEPRO	Ctrl+B に「ブラウズ」を追加
		{ 'B', _T("B"),F_0, F_0, F_BROWSE, F_0, F_0, F_0, F_0, F_0 },
		//Jan. 16, 2001 JEPRO	SHift+Ctrl+C に「.hと同名の.c(なければ.cpp)を開く」を追加
		//Feb. 07, 2001 JEPRO	SHift+Ctrl+C を「.hと同名の.c(なければ.cpp)を開く」→「同名のC/C++ヘッダ(ソース)を開く」に変更
		{ 'C', _T("C"),F_0, F_0, F_COPY, F_OPEN_HfromtoC, F_0, F_0, F_0, F_0 },
		//Jan. 16, 2001 JEPRO	Ctrl+D に「単語切り取り」, Shift+Ctrl+D に「単語削除」を追加
		//2001.12.06 hor Alt+D を「SORT_DESC」に割当
		{ 'D', _T("D"),F_0, F_0, F_WordCut, F_WordDelete, F_SORT_DESC, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+Alt+E に「重ねて表示」を追加
		//Jan. 16, 2001	JEPRO	Ctrl+E に「行切り取り(折り返し単位)」, Shift+Ctrl+E に「行削除(折り返し単位)」を追加
		{ 'E', _T("E"),F_0, F_0, F_CUT_LINE, F_DELETE_LINE, F_0, F_0, F_CASCADE, F_0 },
		{ 'F', _T("F"),F_0, F_0, F_SEARCH_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		{ 'G', _T("G"),F_0, F_0, F_GREP_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 07, 2000 JEPRO	Ctrl+Alt+H に「上下に並べて表示」を追加
		//Jan. 16, 2001 JEPRO	Ctrl+H を「カーソル前を削除」→「カーソル行をウィンドウ中央へ」に変更し	Shift+Ctrl+H に「.cまたは.cppと同名の.hを開く」を追加
		//Feb. 07, 2001 JEPRO	SHift+Ctrl+H を「.cまたは.cppと同名の.hを開く」→「同名のC/C++ヘッダ(ソース)を開く」に変更
		{ 'H', _T("H"),F_0, F_0, F_CURLINECENTER, F_OPEN_HfromtoC, F_0, F_0, F_TILE_V, F_0 },
		//Jan. 21, 2001	JEPRO	Ctrl+I に「行の二重化」を追加
		{ 'I', _T("I"),F_0, F_0, F_DUPLICATELINE, F_0, F_0, F_0, F_0, F_0 },
		{ 'J', _T("J"),F_0, F_0, F_JUMP_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		//Jan. 16, 2001	JEPRO	Ctrl+K に「行末まで切り取り(改行単位)」, Shift+Ctrl+E に「行末まで削除(改行単位)」を追加
		{ 'K', _T("K"),F_0, F_0, F_LineCutToEnd, F_LineDeleteToEnd, F_0, F_0, F_0, F_0 },
		//Jan. 14, 2001 JEPRO	Ctrl+Alt+L に「小文字」, Shift+Ctrl+Alt+L に「大文字」を追加
		//Jan. 16, 2001 Ctrl+L を「カーソル行をウィンドウ中央へ」→「キーマクロの読み込み」に変更し「カーソル行をウィンドウ中央へ」は Ctrl+H に移動
		//2001.12.03 hor Alt+L を「LTRIM」に割当
		{ 'L', _T("L"),F_0, F_0, F_LOADKEYMACRO, F_EXECKEYMACRO, F_LTRIM, F_0, F_TOLOWER, F_TOUPPER },
		//Jan. 16, 2001 JEPRO	Ctrl+M に「キーマクロの保存」を追加
		//2001.12.06 hor Alt+M を「MERGE」に割当
		{ 'M', _T("M"),F_0, F_0, F_SAVEKEYMACRO, F_RECKEYMACRO, F_MERGE, F_0, F_0, F_0 },
		//Oct. 20, 2000 JEPRO	Alt+N に「移動履歴: 次へ」を追加
		{ 'N', _T("N"),F_0, F_0, F_FILENEW, F_0, F_JUMPHIST_NEXT, F_0, F_0, F_0 },
		//Jan. 13, 2001 JEPRO	Alt+O に「アウトプット」を追加
		{ 'O', _T("O"),F_0, F_0, F_FILEOPEN, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+P に「印刷」, Shift+Ctrl+P に「印刷プレビュー」, Ctrl+Alt+P に「ページ設定」を追加
		//Oct. 20, 2000 JEPRO	Alt+P に「移動履歴: 前へ」を追加
		{ 'P', _T("P"),F_0, F_0, F_PRINT, F_PRINT_PREVIEW, F_JUMPHIST_PREV, F_0, F_PRINT_PAGESETUP, F_0 },
		//Jan. 24, 2001	JEPRO	Ctrl+Q に「キー割り当て一覧をコピー」を追加
		{ 'Q', _T("Q"),F_0, F_0, F_CREATEKEYBINDLIST, F_0, F_0, F_0, F_0, F_0 },
		//2001.12.03 hor Alt+R を「RTRIM」に割当
		{ 'R', _T("R"),F_0, F_0, F_REPLACE_DIALOG, F_0, F_RTRIM, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Shift+Ctrl+S に「名前を付けて保存」を追加
		{ 'S', _T("S"),F_0, F_0, F_FILESAVE, F_FILESAVEAS_DIALOG, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+Alt+T に「左右に並べて表示」を追加
		//Jan. 21, 2001	JEPRO	Ctrl+T に「タグジャンプ」, Shift+Ctrl+T に「タグジャンプバック」を追加
		{ 'T', _T("T"),F_0, F_0, F_TAGJUMP, F_TAGJUMPBACK, F_0, F_0, F_TILE_H, F_0 },
		//Oct. 7, 2000 JEPRO	Ctrl+Alt+U に「現在のウィンドウ幅で折り返し」を追加
		//Jan. 16, 2001	JEPRO	Ctrl+U に「行頭まで切り取り(改行単位)」, Shift+Ctrl+U に「行頭まで削除(改行単位)」を追加
		{ 'U', _T("U"),F_0, F_0, F_LineCutToStart, F_LineDeleteToStart, F_0, F_0, F_WRAPWINDOWWIDTH, F_0 },
		{ 'V', _T("V"),F_0, F_0, F_PASTE, F_0, F_0, F_0, F_0, F_0 },
		{ 'W', _T("W"),F_0, F_0, F_SELECTWORD, F_0, F_0, F_0, F_0, F_0 },
		//Jan. 13, 2001 JEPRO	Alt+X を「カスタムメニュー1」→「未定義」に変更し「カスタムメニュー1」は Alt+1 に移動
		{ 'X', _T("X"),F_0, F_0, F_CUT, F_0, F_0, F_0, F_0, F_0 },
		{ 'Y', _T("Y"),F_0, F_0, F_REDO, F_0, F_0, F_0, F_0, F_0 },
		{ 'Z', _T("Z"),F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },
		/* 記号 */
		//Oct. 7, 2000 JEPRO	Shift+Ctrl+- に「上下に分割」を追加
		// 2002.02.08 hor Ctrl+-にファイル名をコピーを追加
		{ 0x00bd, _T("-"),F_0, F_0, F_COPYFNAME, F_SPLIT_V, F_0, F_0, F_0, F_0 },
		{ 0x00de, _T("^(英語')"),F_0, F_0, F_COPYTAG, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Shift+Ctrl+\ に「左右に分割」を追加
		{ 0x00dc, _T("\\"),F_0, F_0, F_COPYPATH, F_SPLIT_H, F_0, F_0, F_0, F_0 },
		//Sept. 20, 2000 JEPRO	Ctrl+@ に「ファイル内容比較」を追加  //Oct. 15, 2000 JEPRO「選択範囲内全行コピー」に変更
		{ 0x00c0, _T("@(英語`)"),F_0, F_0, F_COPYLINES, F_0, F_0, F_0, F_0, F_0 },
		//	Aug. 16, 2000 genta
		//	反対向きの括弧にも括弧検索を追加
		{ 0x00db, _T("["),F_0, F_0, F_BRACKETPAIR, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	Shift+Ctrl+; に「縦横に分割」を追加	//Jan. 16, 2001	Alt+; に「日付挿入」を追加
		{ 0x00bb, _T(";"),F_0, F_0, F_0, F_SPLIT_VH, F_INS_DATE, F_0, F_0, F_0 },
		//Sept. 14, 2000 JEPRO	Ctrl+: に「選択範囲内全行行番号付きコピー」を追加	//Jan. 16, 2001	Alt+: に「時刻挿入」を追加
		{ 0x00ba, _T(":"),F_0, F_0, F_COPYLINESWITHLINENUMBER, F_0, F_INS_TIME, F_0, F_0, F_0 },
		{ 0x00dd, _T("]"),F_0, F_0, F_BRACKETPAIR, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00bc, _T(","),F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		//Sept. 14, 2000 JEPRO	Ctrl+. に「選択範囲内全行引用符付きコピー」を追加
		{ 0x00be, _T("."),F_0, F_0, F_COPYLINESASPASSAGE, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00bf, _T("/"),F_0, F_0, F_HOKAN, F_0, F_0, F_0, F_0, F_0 },
		//	Nov. 15, 2000 genta PC/ATキーボードに合わせてキーコードを変更
		//	PC98救済のため，従来のキーコードに対応する項目を追加．
		{ 0x00e2, _T("_"),F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00df, _T("_(PC-98)"),F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },
		//Oct. 7, 2000 JEPRO	長くて表示しきれない所がでてきてしまうのでアプリケーションキー→アプリキーに短縮
		{ VK_APPS, _T("アプリキー"),F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON }
	};
	const int	nKeyDataInitNum = sizeof( KeyDataInit ) / sizeof( KeyDataInit[0] );
	//	From Here 2007.11.04 genta バッファオーバーラン防止
	if( nKeyDataInitNum > sizeof( pShareData->m_pKeyNameArr ) / sizeof( pShareData->m_pKeyNameArr[0])) {
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP, _T("作者に教えて欲しいエラー"),
			_T("キー設定数に対してDLLSHARE::m_nKeyNameArr[]のサイズが不足しています") );
		return false;
	}
	//	To Here 2007.11.04 genta バッファオーバーラン防止
	for( int i = 0; i < nKeyDataInitNum; ++i ){
		SetKeyNameArrVal(
			pShareData,
			i,
			KeyDataInit[i].nKeyCode,
			KeyDataInit[i].pszKeyName,
			KeyDataInit[i].nFuncCode_0,
			KeyDataInit[i].nFuncCode_1,
			KeyDataInit[i].nFuncCode_2,
			KeyDataInit[i].nFuncCode_3,
			KeyDataInit[i].nFuncCode_4,
			KeyDataInit[i].nFuncCode_5,
			KeyDataInit[i].nFuncCode_6,
			KeyDataInit[i].nFuncCode_7
		 );
	}
	pShareData->m_nKeyNameArrNum = nKeyDataInitNum;
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
