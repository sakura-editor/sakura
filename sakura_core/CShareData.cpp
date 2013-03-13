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
	Copyright (C) 2006, aroka, ryoji, genta, fon, rastiv, Moca, maru
	Copyright (C) 2007, ryoji, genta, maru, Moca, nasukoji
	Copyright (C) 2008, ryoji, nasukoji, kobake, novice
	Copyright (C) 2009, nasukoji, ryoji, syat, salarm
	Copyright (C) 2010, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <io.h>
#include "CShareData.h"
#include "CEditApp.h"
#include "mymessage.h"
#include "Debug.h"
#include "global.h"
#include "etc_uty.h"
#include "CRunningTimer.h"
#include "my_icmp.h" // 2002/11/30 Moca 追加
#include "my_tchar.h" // 2003/01/06 Moca
#include "charcode.h"  // 2006/06/28 rastiv
#include "CEol.h"  // 2006/06/28 rastiv

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

	Version 88:
	ホイールスクロールを利用したページスクロール・横スクロール対応 2009.01.12 nasukoji

	Version 89:
	STypeConfigのm_szTabViewStringサイズ拡張（Version 41）の戻し 2009.02.11 ryoji

	Version 90:
	自動実行マクロ 2006/08/31 ryoji

	Version 91:
	句読点ぶら下げ文字 2009.08.07 ryoji

	Version 92:
	ウィンドウ毎にアクセラレータテーブルを作成する(Wine用) 2009.08.15 nasukoji

	Version 93:
	「開こうとしたファイルが大きい場合に警告」機能追加  2010.03.03 Moca Unicode版からbackport

	Version 94:
	ICONをすべてのコマンドに

	Version 95:
	CBlockComment変更
	検索／置換情報SSearchOptionにまとめた
	外部から起動時に新しいウインドウを開く

	Version 96:
	KEYDATAのサイズ変更 2012.11.04 aroka
	デフォルトの文字コード/デフォルト改行コード/デフォルトBOM

	Version 97:
	改行コードを変換して貼り付ける

	Version 98-105:
	CommonSetting構造体整理開始

	Version 106:
	MRUにタイプ別設定追加

	Version 107:
	ウィンドウにメニュー字化け対策設定追加

	Version 108:
	高DPI用にフォントサイズ（1/10ポイント単位）を追加

	Version 109:
	タブバーフォント指定

	Version 110:
	ファイルダイアログの初期位置
*/

extern const unsigned int uShareDataVersion;
const unsigned int uShareDataVersion = 110;

/*
||	Singleton風
*/
CShareData* CShareData::_instance = NULL;

// GetOpenedWindowArr用静的変数／構造体
static BOOL s_bSort;	// ソート指定
static BOOL s_bGSort;	// グループ指定

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
static CMutex g_cEditArrMutex( FALSE, GSTR_MUTEX_SAKURA_EDITARR );

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



//	CShareData_new2.cppと統合
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動
CShareData::CShareData()
{
	m_hFileMap   = NULL;
	m_pShareData = NULL;
	m_nTransformFileNameCount = -1;
}

/*!
	共有メモリ領域がある場合はプロセスのアドレス空間から､
	すでにマップされているファイル ビューをアンマップする。
*/
CShareData::~CShareData()
{
	if( m_pShareData ){
		/* プロセスのアドレス空間から､ すでにマップされているファイル ビューをアンマップします */
		::UnmapViewOfFile( m_pShareData );
		m_pShareData = NULL;
	}
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
bool CShareData::InitShareData()
{
	MY_RUNNINGTIMER(cRunningTimer,"CShareData::InitShareData" );

	if (CShareData::_instance == NULL)	//	Singleton風
		CShareData::_instance = this;

	m_hwndTraceOutSource = NULL;	// 2006.06.26 ryoji

	int		i;
	int		j;

	/* ファイルマッピングオブジェクト */
	m_hFileMap = ::CreateFileMapping(
		INVALID_HANDLE_VALUE,	//	Sep. 6, 2003 wmlhq
		NULL,
		PAGE_READWRITE | SEC_COMMIT,
		0,
		sizeof( DLLSHAREDATA ),
		GSTR_SHAREDATA
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

		// 2007.05.19 ryoji 実行ファイルフォルダ->設定ファイルフォルダに変更
		TCHAR	szIniFolder[_MAX_PATH];
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bInit = false;
		GetInidir( szIniFolder );
		AddLastChar( szIniFolder, _MAX_PATH, _T('\\') );

		m_pShareData->m_vStructureVersion = uShareDataVersion;
		_tcscpy(m_pShareData->m_Common.m_sMacro.m_szKeyMacroFileName, _T(""));	/* キーワードマクロのファイル名 */ //@@@ 2002.1.24 YAZAKI
		m_pShareData->m_sFlags.m_bRecordingKeyMacro = FALSE;		/* キーボードマクロの記録中 */
		m_pShareData->m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

		// 2004.05.13 Moca リソースから製品バージョンの取得
		GetAppVersionInfo( NULL, VS_VERSION_INFO,
			&m_pShareData->m_sVersion.m_dwProductVersionMS, &m_pShareData->m_sVersion.m_dwProductVersionLS );
		m_pShareData->m_sHandles.m_hwndTray = NULL;
		m_pShareData->m_sHandles.m_hAccel = NULL;
		m_pShareData->m_sHandles.m_hwndDebug = NULL;
		m_pShareData->m_sNodes.m_nSequences = 0;					/* ウィンドウ連番 */
		m_pShareData->m_sNodes.m_nGroupSequences = 0;			/* タブグループ連番 */	// 2007.06.20 ryoji
		m_pShareData->m_sNodes.m_nEditArrNum = 0;

		m_pShareData->m_sFlags.m_bEditWndChanging = FALSE;	// 編集ウィンドウ切替中	// 2007.04.03 ryoji

		m_pShareData->m_Common.m_sGeneral.m_nMRUArrNum_MAX = 15;	/* ファイルの履歴MAX */	//Oct. 14, 2000 JEPRO 少し増やした(10→15)
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
		CMRUFile cMRU;
		cMRU.ClearAll();
		m_pShareData->m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX = 15;	/* フォルダの履歴MAX */	//Oct. 14, 2000 JEPRO 少し増やした(10→15)
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
		CMRUFolder cMRUFolder;
		cMRUFolder.ClearAll();

		m_pShareData->m_sSearchKeywords.m_nSEARCHKEYArrNum = 0;
		for( i = 0; i < MAX_SEARCHKEY; ++i ){
			_tcscpy( m_pShareData->m_sSearchKeywords.m_szSEARCHKEYArr[i], _T("") );
		}
		m_pShareData->m_sSearchKeywords.m_nREPLACEKEYArrNum = 0;
		for( i = 0; i < MAX_REPLACEKEY; ++i ){
			_tcscpy( m_pShareData->m_sSearchKeywords.m_szREPLACEKEYArr[i], _T("") );
		}
		m_pShareData->m_sSearchKeywords.m_nGREPFILEArrNum = 0;
		for( i = 0; i < MAX_GREPFILE; ++i ){
			_tcscpy( m_pShareData->m_sSearchKeywords.m_szGREPFILEArr[i], _T("") );
		}
		m_pShareData->m_sSearchKeywords.m_nGREPFILEArrNum = 1;
		_tcscpy( m_pShareData->m_sSearchKeywords.m_szGREPFILEArr[0], _T("*.*") );

		m_pShareData->m_sSearchKeywords.m_nGREPFOLDERArrNum = 0;
		for( i = 0; i < MAX_GREPFOLDER; ++i ){
			_tcscpy( m_pShareData->m_sSearchKeywords.m_szGREPFOLDERArr[i], _T("") );
		}
		_tcscpy( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER, szIniFolder );	/* マクロ用フォルダ */
		_tcscpy( m_pShareData->m_sHistory.m_szIMPORTFOLDER, szIniFolder );	/* 設定インポート用フォルダ */

		for( i = 0; i < MAX_TRANSFORM_FILENAME; ++i ){
			_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i], _T("") );
			_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[i], _T("") );
		}
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[0], _T("%DeskTop%\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[0],   _T("デスクトップ\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[1], _T("%Personal%\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[1],   _T("マイドキュメント\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[2], _T("%Cache%\\Content.IE5\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[2],   _T("IEキャッシュ\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[3], _T("%TEMP%\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[3],   _T("TEMP\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[4], _T("%Common DeskTop%\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[4],   _T("共有デスクトップ\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[5], _T("%Common Documents%\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[5],   _T("共有ドキュメント\\") );
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[6], _T("%AppData%\\") );	// 2007.05.19 ryoji 追加
		_tcscpy( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[6],   _T("アプリデータ\\") );	// 2007.05.19 ryoji 追加
		m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum = 7;
		
		/* m_PrintSettingArr[0]を設定して、残りの1～7にコピーする。
			必要になるまで遅らせるために、CPrintに、CShareDataを操作する権限を与える。
			YAZAKI.
		*/
		{
			/*
				2006.08.16 Moca 初期化単位を PRINTSETTINGに変更。CShareDataには依存しない。
			*/
			TCHAR szSettingName[64];
			i = 0;
			wsprintf( szSettingName, _T("印刷設定 %d"), i + 1 );
			CPrint::SettingInitialize( m_pShareData->m_PrintSettingArr[0], szSettingName );	//	初期化命令。
		}
		for( i = 1; i < MAX_PRINTSETTINGARR; ++i ){
			m_pShareData->m_PrintSettingArr[i] = m_pShareData->m_PrintSettingArr[0];
			wsprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, _T("印刷設定 %d"), i + 1 );	/* 印刷設定の名前 */
		}

		//	Jan. 30, 2005 genta 関数として独立
		//	2007.11.04 genta 戻り値チェック．falseなら起動中断．
		if( ! InitKeyAssign( m_pShareData )){
			return false;
		}

//	From Here Sept. 19, 2000 JEPRO コメントアウトになっていた初めのブロックを復活しその下をコメントアウト
//	MS ゴシック標準スタイル10ptに設定
//		/* LOGFONTの初期化 */
		memset( &m_pShareData->m_Common.m_sView.m_lf, 0, sizeof( m_pShareData->m_Common.m_sView.m_lf ) );
		m_pShareData->m_Common.m_sView.m_lf.lfHeight			= DpiPointsToPixels(-10);	// 2009.10.01 ryoji 高DPI対応（ポイント数から算出）;
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
		m_pShareData->m_Common.m_sView.m_nPointSize = 0;	// フォントサイズ（1/10ポイント単位） ※古いバージョンからの移行を考慮して無効値で初期化	// 2009.10.01 ryoji

		// キーワードヘルプのフォント ai 02/05/21 Add S
		LOGFONT lfIconTitle;	// エクスプローラのファイル名表示に使用されるフォント
		::SystemParametersInfo(
			SPI_GETICONTITLELOGFONT,				// system parameter to query or set
			sizeof(LOGFONT),						// depends on action to be taken
			(PVOID)&lfIconTitle,					// depends on action to be taken
			0										// user profile update flag
		);
		// ai 02/05/21 Add E
		m_pShareData->m_Common.m_sHelper.m_lf_kh = lfIconTitle;
		m_pShareData->m_Common.m_sHelper.m_ps_kh = 0;	// フォントサイズ（1/10ポイント単位） ※古いバージョンからの移行を考慮して無効値で初期化	// 2009.10.01 ryoji

//	To Here Sept. 19,2000

		m_pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH = TRUE;				/* 現在のフォントは固定幅フォントである */

//		m_pShareData->m_Common.m_bUseCaretKeyWord = FALSE;		/* キャレット位置の単語を辞書検索-機能OFF */	// 2006.03.24 fon sakura起動ごとFALSEとし、初期化しない


		/* バックアップ */
		CommonSetting_Backup& sBackup = m_pShareData->m_Common.m_sBackup;
		sBackup.m_bBackUp = false;										/* バックアップの作成 */
		sBackup.m_bBackUpDialog = true;									/* バックアップの作成前に確認 */
		sBackup.m_bBackUpFolder = false;								/* 指定フォルダにバックアップを作成する */
		sBackup.m_szBackUpFolder[0] = _T('\0');							/* バックアップを作成するフォルダ */
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
		m_pShareData->m_Common.m_sSearch.m_bConsecutiveAll = 0;				/* 「すべて置換」は置換の繰返し */	// 2007.01.16 ryoji
		m_pShareData->m_Common.m_sSearch.m_bSelectedArea = FALSE;			/* 選択範囲内置換 */
		m_pShareData->m_Common.m_sHelper.m_szExtHelp[0] = _T('\0');			/* 外部ヘルプ１ */
		m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp[0] = _T('\0');		/* 外部HTMLヘルプ */
		
		m_pShareData->m_Common.m_sHelper.m_szMigemoDll[0] = _T('\0');		/* migemo dll */
		m_pShareData->m_Common.m_sHelper.m_szMigemoDict[0] = _T('\0');		/* migemo dict */

		m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND = TRUE;			/* 検索／置換  見つからないときメッセージを表示 */

		m_pShareData->m_Common.m_sGeneral.m_bCloseAllConfirm = FALSE;		/* [すべて閉じる]で他に編集用のウィンドウがあれば確認する */	// 2006.12.25 ryoji
		m_pShareData->m_Common.m_sGeneral.m_bExitConfirm = FALSE;			/* 終了時の確認をする */
		m_pShareData->m_Common.m_sGeneral.m_nRepeatedScrollLineNum = 3;		/* キーリピート時のスクロール行数 */
		m_pShareData->m_Common.m_sGeneral.m_nRepeatedScroll_Smooth = FALSE;	/* キーリピート時のスクロールを滑らかにするか */
		m_pShareData->m_Common.m_sGeneral.m_nPageScrollByWheel = 0;			/* キー/マウスボタン + ホイールスクロールでページスクロールする */	// 2009.01.12 nasukoji
		m_pShareData->m_Common.m_sGeneral.m_nHorizontalScrollByWheel = 0;	/* キー/マウスボタン + ホイールスクロールで横スクロールする */		// 2009.01.12 nasukoji

		m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy = false;			/* 折り返し行に改行を付けてコピー */
		m_pShareData->m_Common.m_sSearch.m_bGrepSubFolder = TRUE;			/* Grep: サブフォルダも検索 */
		m_pShareData->m_Common.m_sSearch.m_bGrepOutputLine = TRUE;			/* Grep: 行を出力するか該当部分だけ出力するか */
		m_pShareData->m_Common.m_sSearch.m_nGrepOutputStyle = 1;			/* Grep: 出力形式 */
		m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder=FALSE;		/* Grep: フォルダの初期値をカレントフォルダにする */
		m_pShareData->m_Common.m_sSearch.m_nGrepCharSet = CODE_AUTODETECT;	/* Grep: 文字コードセット */
		m_pShareData->m_Common.m_sSearch.m_bGrepRealTimeView = FALSE;		/* 2003.06.28 Moca Grep結果のリアルタイム表示 */
		m_pShareData->m_Common.m_sSearch.m_bCaretTextForSearch = TRUE;		/* 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする */
		m_pShareData->m_Common.m_sSearch.m_szRegexpLib[0] = _T('\0');		/* 2007.08.12 genta 正規表現DLL */
		m_pShareData->m_Common.m_sSearch.m_bGTJW_RETURN = TRUE;				/* エンターキーでタグジャンプ */
		m_pShareData->m_Common.m_sSearch.m_bGTJW_LDBLCLK = TRUE;			/* ダブルクリックでタグジャンプ */

//キーワード：ツールバー順序
		//	Jan. 30, 2005 genta 関数として独立
		InitToolButtons( m_pShareData );

		m_pShareData->m_Common.m_sWindow.m_bDispTOOLBAR = TRUE;				/* 次回ウィンドウを開いたときツールバーを表示する */
		m_pShareData->m_Common.m_sWindow.m_bDispSTATUSBAR = TRUE;			/* 次回ウィンドウを開いたときステータスバーを表示する */
		m_pShareData->m_Common.m_sWindow.m_bDispFUNCKEYWND = FALSE;			/* 次回ウィンドウを開いたときファンクションキーを表示する */
		m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_Place = 1;			/* ファンクションキー表示位置／0:上 1:下 */
		m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum = 4;		// 2002/11/04 Moca ファンクションキーのグループボタン数

		m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd = FALSE;				//タブウインドウ表示	//@@@ 2003.05.31 MIK
		m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin = FALSE;		//タブウインドウ表示	//@@@ 2003.05.31 MIK
		_tcscpy(	//@@@ 2003.06.13 MIK
			m_pShareData->m_Common.m_sTabBar.m_szTabWndCaption,
			_T("${w?【Grep】$h$:【アウトプット】$:$f$}${U?(更新)$}${R?(読み取り専用)$:(上書き禁止)$}${M?【キーマクロの記録中】$}")
		);
		m_pShareData->m_Common.m_sTabBar.m_bSameTabWidth = FALSE;			//タブを等幅にする			//@@@ 2006.01.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bDispTabIcon = FALSE;			//タブにアイコンを表示する	//@@@ 2006.01.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bSortTabList = TRUE;				//タブ一覧をソートする		//@@@ 2006.05.10 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bTab_RetainEmptyWin = TRUE;		// 最後のファイルが閉じられたとき(無題)を残す	// 2007.02.11 genta
		m_pShareData->m_Common.m_sTabBar.m_bTab_CloseOneWin = FALSE;		// タブモードでもウィンドウの閉じるボタンで現在のファイルのみ閉じる	// 2007.02.11 genta
		m_pShareData->m_Common.m_sTabBar.m_bTab_ListFull = FALSE;			//タブ一覧をフルパス表示する	//@@@ 2007.02.28 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bChgWndByWheel = FALSE;			//マウスホイールでウィンドウ切替	//@@@ 2006.03.26 ryoji
		m_pShareData->m_Common.m_sTabBar.m_bNewWindow = FALSE;				// 外部から起動するときは新しいウインドウで開く

		m_pShareData->m_Common.m_sTabBar.m_tabFont = lfIconTitle;
		m_pShareData->m_Common.m_sTabBar.m_tabFontPs = 0;

		m_pShareData->m_Common.m_sWindow.m_bSplitterWndHScroll = TRUE;		// 2001/06/20 asa-o 分割ウィンドウの水平スクロールの同期をとる
		m_pShareData->m_Common.m_sWindow.m_bSplitterWndVScroll = TRUE;		// 2001/06/20 asa-o 分割ウィンドウの垂直スクロールの同期をとる

		/* カスタムメニュー情報 */
		wsprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[0], _T("右クリックメニュー"), i );
		for( i = 1; i < MAX_CUSTOM_MENU; ++i ){
			wsprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[i], _T("メニュー%d"), i );
			m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[i] = 0;
			for( j = 0; j < MAX_CUSTOM_MENU_ITEMS; ++j ){
				m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[i][j] = 0;
				m_pShareData->m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr [i][j] = '\0';
			}
		}
		wsprintf( m_pShareData->m_Common.m_sCustomMenu.m_szCustMenuNameArr[CUSTMENU_INDEX_FOR_TABWND], _T("タブメニュー") );	//@@@ 2003.06.13 MIK


		/* 見出し記号 */
		_tcscpy( m_pShareData->m_Common.m_sFormat.m_szMidashiKigou, _T("１２３４５６７８９０（(［[「『【■□▲△▼▽◆◇○◎●§・※☆★第①②③④⑤⑥⑦⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩ一二三四五六七八九十壱弐参伍") );
		/* 引用符 */
		_tcscpy( m_pShareData->m_Common.m_sFormat.m_szInyouKigou, _T("> ") );	/* 引用符 */
		m_pShareData->m_Common.m_sHelper.m_bUseHokan = FALSE;					/* 入力補完機能を使用する */

		// 2001/06/14 asa-o 補完とキーワードヘルプはタイプ別に移動したので削除
		//	2004.05.13 Moca ウィンドウサイズ固定指定追加に伴う指定方法変更
		m_pShareData->m_Common.m_sWindow.m_eSaveWindowSize = WINSIZEMODE_SAVE;	// ウィンドウサイズ継承
		m_pShareData->m_Common.m_sWindow.m_nWinSizeType = SIZE_RESTORED;
		m_pShareData->m_Common.m_sWindow.m_nWinSizeCX = CW_USEDEFAULT;
		m_pShareData->m_Common.m_sWindow.m_nWinSizeCY = 0;
		
		//	2004.05.13 Moca ウィンドウ位置
		m_pShareData->m_Common.m_sWindow.m_eSaveWindowPos = WINSIZEMODE_DEF;	// ウィンドウ位置固定・継承
		m_pShareData->m_Common.m_sWindow.m_nWinPosX = CW_USEDEFAULT;
		m_pShareData->m_Common.m_sWindow.m_nWinPosY = 0;

		m_pShareData->m_Common.m_sGeneral.m_bUseTaskTray = TRUE;				/* タスクトレイのアイコンを使う */
#ifdef _DEBUG
		m_pShareData->m_Common.m_sGeneral.m_bStayTaskTray = FALSE;				/* タスクトレイのアイコンを常駐 */
#else
		m_pShareData->m_Common.m_sGeneral.m_bStayTaskTray = TRUE;				/* タスクトレイのアイコンを常駐 */
#endif
		m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyCode = _T('Z');		/* タスクトレイ左クリックメニュー キー */
		m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods = HOTKEYF_ALT | HOTKEYF_CONTROL;	/* タスクトレイ左クリックメニュー キー */
		m_pShareData->m_Common.m_sEdit.m_bUseOLE_DragDrop = TRUE;			/* OLEによるドラッグ & ドロップを使う */
		m_pShareData->m_Common.m_sEdit.m_bUseOLE_DropSource = TRUE;			/* OLEによるドラッグ元にするか */
		m_pShareData->m_Common.m_sGeneral.m_bDispExitingDialog = FALSE;		/* 終了ダイアログを表示する */
		m_pShareData->m_Common.m_sEdit.m_bSelectClickedURL = TRUE;			/* URLがクリックされたら選択するか */
		m_pShareData->m_Common.m_sSearch.m_bGrepExitConfirm = FALSE;		/* Grepモードで保存確認するか */
//		m_pShareData->m_Common.m_bRulerDisp = TRUE;							/* ルーラー表示 */
		m_pShareData->m_Common.m_sWindow.m_nRulerHeight = 13;				/* ルーラーの高さ */
		m_pShareData->m_Common.m_sWindow.m_nRulerBottomSpace = 0;			/* ルーラーとテキストの隙間 */
		m_pShareData->m_Common.m_sWindow.m_nRulerType = 0;					/* ルーラーのタイプ */
		//	Sep. 18, 2002 genta
		m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace = 0;			/* 行番号の右の隙間 */
		m_pShareData->m_Common.m_sWindow.m_nVertLineOffset = -1;			// 2005.11.10 Moca 指定桁縦線
		m_pShareData->m_Common.m_sWindow.m_bUseCompatibleBMP = FALSE;		// 2007.09.09 Moca 画面キャッシュを使う
		m_pShareData->m_Common.m_sEdit.m_bCopyAndDisablSelection = FALSE;	/* コピーしたら選択解除 */
		m_pShareData->m_Common.m_sEdit.m_bEnableNoSelectCopy = TRUE;		/* 選択なしでコピーを可能にする */	// 2007.11.18 ryoji
		m_pShareData->m_Common.m_sEdit.m_bEnableLineModePaste = true;		/* ラインモード貼り付けを可能にする */	// 2007.10.08 ryoji
		m_pShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle = TRUE;		/* HtmlHelpビューアはひとつ */
		m_pShareData->m_Common.m_sCompare.m_bCompareAndTileHorz = TRUE;		/* 文書比較後、左右に並べて表示 */
		m_pShareData->m_Common.m_sEdit.m_bConvertEOLPaste = false;			/* 改行コードを変換して貼り付ける */	// 2009.02.28 salarm

		//[ファイル]タブ
		m_pShareData->m_Common.m_sFile.m_nFileShareMode = OF_SHARE_DENY_WRITE;// ファイルの排他制御モード
		m_pShareData->m_Common.m_sFile.m_bCheckFileTimeStamp = true;		// 更新の監視

		//ファイルの保存
		m_pShareData->m_Common.m_sFile.m_bEnableUnmodifiedOverwrite = FALSE;// 無変更でも上書きするか

		//ファイルオープン
		m_pShareData->m_Common.m_sFile.m_bDropFileAndClose = false;			// ファイルをドロップしたときは閉じて開く
		m_pShareData->m_Common.m_sFile.m_nDropFileNumMax = 8;				// 一度にドロップ可能なファイル数
		m_pShareData->m_Common.m_sFile.m_bRestoreCurPosition = true;		//	カーソル位置復元	//	Oct. 27, 2000 genta
		m_pShareData->m_Common.m_sFile.m_bRestoreBookmarks = true;			// ブックマーク復元 //2002.01.16 hor 
		//	Nov. 12, 2000 genta
		m_pShareData->m_Common.m_sFile.m_bAutoMIMEdecode = false;			// ファイル読み込み時にMIMEのデコードを行うか	//Jul. 13, 2001 JEPRO


		m_pShareData->m_Common.m_sEdit.m_bNotOverWriteCRLF = TRUE;			/* 改行は上書きしない */
		::SetRect( &m_pShareData->m_Common.m_sOthers.m_rcOpenDialog, 0, 0, 0, 0 );	/* 「開く」ダイアログのサイズと位置 */
		m_pShareData->m_Common.m_sEdit.m_eOpenDialogDir = OPENDIALOGDIR_CUR;
		_tcscpy(m_pShareData->m_Common.m_sEdit.m_OpenDialogSelDir, _T("%Personal%\\"));
		m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind = TRUE;			/* 検索ダイアログを自動的に閉じる */
		m_pShareData->m_Common.m_sSearch.m_bSearchAll		 = FALSE;			/* 検索／置換／ブックマーク  先頭（末尾）から再検索 2002.01.26 hor */
		m_pShareData->m_Common.m_sWindow.m_bScrollBarHorz = TRUE;				/* 水平スクロールバーを使う */
		m_pShareData->m_Common.m_sOutline.m_bAutoCloseDlgFuncList = FALSE;		/* アウトライン ダイアログを自動的に閉じる */	//Nov. 18, 2000 JEPRO TRUE→FALSE に変更
		m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgReplace = TRUE;		/* 置換 ダイアログを自動的に閉じる */
		m_pShareData->m_Common.m_sEdit.m_bAutoColmnPaste = TRUE;			/* 矩形コピーのテキストは常に矩形貼り付け */
		m_pShareData->m_Common.m_sGeneral.m_bNoCaretMoveByActivation = FALSE;	/* マウスクリックにてアクティベートされた時はカーソル位置を移動しない 2007.10.02 nasukoji (add by genta) */

		m_pShareData->m_Common.m_sHelper.m_bHokanKey_RETURN	= TRUE;			/* VK_RETURN 補完決定キーが有効/無効 */
		m_pShareData->m_Common.m_sHelper.m_bHokanKey_TAB	= FALSE;		/* VK_TAB   補完決定キーが有効/無効 */
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
		m_pShareData->m_Common.m_sWindow.m_bMenuWChar = FALSE;		/* メニューの字化け対策を行う(Win2K以降のみ) */

		//	Oct. 03, 2004 genta 前回と異なる文字コードの時に問い合わせを行うか
		m_pShareData->m_Common.m_sFile.m_bQueryIfCodeChange = true;
		//	Oct. 09, 2004 genta 開こうとしたファイルが存在しないとき警告する
		m_pShareData->m_Common.m_sFile.m_bAlertIfFileNotExist = false;
		m_pShareData->m_Common.m_sFile.m_bAlertIfLargeFile = false;  // 開こうとしたファイルが大きい場合に警告する
		m_pShareData->m_Common.m_sFile.m_nAlertFileSize = 10;        // 警告を始めるファイルサイズ（MB単位）

		// ファイル保存ダイアログのフィルタ設定	// 2006.11.16 ryoji
		m_pShareData->m_Common.m_sFile.m_bNoFilterSaveNew = true;	// 新規から保存時は全ファイル表示
		m_pShareData->m_Common.m_sFile.m_bNoFilterSaveFile = true;	// 新規以外から保存時は全ファイル表示

		m_pShareData->m_Common.m_sKeyBind.m_bCreateAccelTblEachWin = FALSE;	// ウィンドウ毎にアクセラレータテーブルを作成する(Wine用)	// 2009.08.15 nasukoji

		for( i = 0; i < MAX_CMDARR; i++ ){
			/* 初期化 */
			m_pShareData->m_sHistory.m_szCmdArr[i][0] = '\0';
		}
		m_pShareData->m_sHistory.m_nCmdArrNum = 0;

		InitKeyword( m_pShareData );
		InitTypeConfigs( m_pShareData );
		InitPopupMenu( m_pShareData );

		//	Apr. 05, 2003 genta ウィンドウキャプションの初期値
		//	Aug. 16, 2003 genta $N(ファイル名省略表示)をデフォルトに変更
		_tcscpy( m_pShareData->m_Common.m_sWindow.m_szWindowCaptionActive, 
			_T("${w?$h$:アウトプット$:${I?$f$:$N$}$}${U?(更新)$} -")
			_T(" $A $V ${R?(読み取り専用)$:（上書き禁止）$}${M?  【キーマクロの記録中】$}") );
		_tcscpy( m_pShareData->m_Common.m_sWindow.m_szWindowCaptionInactive, 
			_T("${w?$h$:アウトプット$:$f$}${U?(更新)$} -")
			_T(" $A $V ${R?(読み取り専用)$:（上書き禁止）$}${M?  【キーマクロの記録中】$}") );

		//	From Here Sep. 14, 2001 genta
		//	Macro登録の初期化
		MacroRec *mptr = m_pShareData->m_Common.m_sMacro.m_MacroTable;
		for( i = 0; i < MAX_CUSTMACRO; ++i, ++mptr ){
			mptr->m_szName[0] = '\0';
			mptr->m_szFile[0] = '\0';
			mptr->m_bReloadWhenExecute = FALSE;
		}
		//	To Here Sep. 14, 2001 genta
		m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened = -1;	/* オープン後自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji
		m_pShareData->m_Common.m_sMacro.m_nMacroOnTypeChanged = -1;	/* タイプ変更後自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji
		m_pShareData->m_Common.m_sMacro.m_nMacroOnSave = -1;	/* 保存前自動実行マクロ番号 */	//@@@ 2006.09.01 ryoji

		// 2004/06/21 novice タグジャンプ機能追加
		m_pShareData->m_sTagJump.m_TagJumpNum = 0;
		// 2004.06.22 Moca タグジャンプの先頭
		m_pShareData->m_sTagJump.m_TagJumpTop = 0;
		m_pShareData->m_nExecFlgOpt = 1;	/* 外部コマンド実行の「標準出力を得る」 */	// 2006.12.03 maru オプションの拡張のため
		m_pShareData->m_bLineNumIsCRLF = TRUE;	/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */

		m_pShareData->m_nDiffFlgOpt = 0;	/* DIFF差分表示 */	//@@@ 2002.05.27 MIK

		m_pShareData->m_nTagsOpt = 0;	/* CTAGS */	//@@@ 2003.05.12 MIK
		_tcscpy( m_pShareData->m_szTagsCmdLine, _T("") );	/* CTAGS */	//@@@ 2003.05.12 MIK
		//From Here 2005.04.03 MIK キーワード指定タグジャンプのHistory保管
		m_pShareData->m_sTagJump.m_nTagJumpKeywordArrNum = 0;
		for( i = 0; i < MAX_TAGJUMP_KEYWORD; ++i ){
			_tcscpy( m_pShareData->m_sTagJump.m_szTagJumpKeywordArr[i], _T("") );
		}
		m_pShareData->m_sTagJump.m_bTagJumpICase = FALSE;
		m_pShareData->m_sTagJump.m_bTagJumpAnyWhere = FALSE;
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
	char*			pszKeyName,
	short			nFuncCode_0,
	short			nFuncCode_1,
	short			nFuncCode_2,
	short			nFuncCode_3,
	short			nFuncCode_4,
	short			nFuncCode_5,
	short			nFuncCode_6,
	short			nFuncCode_7
)
{
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nKeyCode = nKeyCode;
	_tcscpy( pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_szKeyName, pszKeyName );
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[0] = nFuncCode_0;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[1] = nFuncCode_1;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[2] = nFuncCode_2;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[3] = nFuncCode_3;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[4] = nFuncCode_4;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[5] = nFuncCode_5;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[6] = nFuncCode_6;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[7] = nFuncCode_7;
}

/*!
	ファイル名から、ドキュメントタイプ（数値）を取得する
	
	@param pszFilePath [in] ファイル名
	
	拡張子を切り出して GetDocumentTypeOfExt に渡すだけ．
*/
int CShareData::GetDocumentTypeOfPath( const char* pszFilePath )
{
	char	szExt[_MAX_EXT];

	if( NULL != pszFilePath && 0 < (int)strlen( pszFilePath ) ){
		_splitpath( pszFilePath, NULL, NULL, NULL, szExt );
		if( szExt[0] == '.' )
			return GetDocumentTypeOfExt( szExt + 1 );
		else
			return GetDocumentTypeOfExt( szExt );
	}
	return 0;
}

/*!
	拡張子から、ドキュメントタイプ（数値）を取得する
	
	@param pszExt [in] 拡張子 (先頭の,は含まない)
	
	指定された拡張子の属する文書タイプ番号を返す．
	とりあえず今のところはタイプは拡張子のみに依存すると仮定している．
	ファイル全体の形式に対応させるときは，また考え直す．
*/
int CShareData::GetDocumentTypeOfExt( const char* pszExt )
{
	const char	pszSeps[] = " ;,";	// separator

	int		i;
	char*	pszToken;
	char	szText[256];

	for( i = 0; i < MAX_TYPES; ++i ){
		strcpy( szText, m_pShareData->m_Types[i].m_szTypeExts );
		pszToken = strtok( szText, pszSeps );
		while( NULL != pszToken ){
			if( 0 == my_stricmp( pszExt, pszToken ) ){
				return i;	//	番号
			}
			pszToken = strtok( NULL, pszSeps );
		}
	}
	return 0;	//	ハズレ
}


/** 編集ウィンドウリストへの登録

	@param hWnd [in] 登録する編集ウィンドウのハンドル
	@param nGroup [in] 新規登録の場合のグループID

	@date 2003.06.28 MIK CRecent利用で書き換え
	@date 2007.06.20 ryoji 新規ウィンドウにはグループIDを付与する
*/
BOOL CShareData::AddEditWndList( HWND hWnd, int nGroup )
{
	int		nSubCommand = TWNT_ADD;
	int		nIndex;
	CRecent	cRecentEditNode;
	EditNode	sMyEditNode;
	EditNode	*p;

	memset( &sMyEditNode, 0, sizeof( sMyEditNode ) );
	sMyEditNode.m_hWnd = hWnd;

	{	// 2007.07.07 genta Lock領域
		LockGuard<CMutex> guard( g_cEditArrMutex );

		cRecentEditNode.EasyCreate( RECENT_FOR_EDITNODE );

		//登録済みか？
		nIndex = cRecentEditNode.FindItem( (const char*)&hWnd );
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
			p = (EditNode*)cRecentEditNode.GetItem( nIndex );
			if( p )
			{
				memcpy( &sMyEditNode, p, sizeof( sMyEditNode ) );
			}
		}

		/* ウィンドウ連番 */

		if( 0 == ::GetWindowLongPtr( hWnd, sizeof(LONG_PTR) ) )
		{
			m_pShareData->m_sNodes.m_nSequences++;
			::SetWindowLongPtr( hWnd, sizeof(LONG_PTR) , (LONG_PTR)m_pShareData->m_sNodes.m_nSequences );

			//連番を更新する。
			sMyEditNode.m_nIndex = m_pShareData->m_sNodes.m_nSequences;

			/* タブグループ連番 */
			if( nGroup > 0 )
			{
				sMyEditNode.m_nGroup = nGroup;	// 指定のグループ
			}
			else
			{
				p = (EditNode*)cRecentEditNode.GetItem( 0 );
				if( NULL == p )
					sMyEditNode.m_nGroup = ++m_pShareData->m_sNodes.m_nGroupSequences;	// 新規グループ
				else
					sMyEditNode.m_nGroup = p->m_nGroup;	// 最近アクティブのグループ
			}

			sMyEditNode.m_showCmdRestore = ::IsZoomed(hWnd)? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
			sMyEditNode.m_bClosing = FALSE;
		}

		//追加または先頭に移動する。
		cRecentEditNode.AppendItem( (const char*)&sMyEditNode );
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

		CRecent	cRecentEditNode;
		cRecentEditNode.EasyCreate( RECENT_FOR_EDITNODE );
		cRecentEditNode.DeleteItem( (const char*)&hWnd );
		cRecentEditNode.Terminate();
	}

	//ウインドウ削除メッセージをブロードキャストする。
	PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY, (WPARAM)TWNT_DEL, (LPARAM)hWnd, hWnd, nGroup );
}

/** グループをIDリセットする

	@date 2007.06.20 ryoji
*/
void CShareData::ResetGroupId( void )
{
	int nGroup;
	int	i;

	nGroup = ++m_pShareData->m_sNodes.m_nGroupSequences;
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; i++ )
	{
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) )
		{
			m_pShareData->m_sNodes.m_pEditArr[i].m_nGroup = nGroup;
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

	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; i++ )
	{
		if( hWnd == m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd )
		{
			if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) )
				return &m_pShareData->m_sNodes.m_pEditArr[i];
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

/* 空いているグループ番号を取得する */
int CShareData::GetFreeGroupId( void )
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();

	return ++pShareData->m_sNodes.m_nGroupSequences;	// 新規グループ
}

/** 指定位置の編集ウィンドウ情報を取得する

	@date 2007.06.20 ryoji
*/
EditNode* CShareData::GetEditNodeAt( int nGroup, int nIndex )
{
	int	i;
	int iIndex;

	iIndex = 0;
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; i++ )
	{
		if( nGroup == 0 || nGroup == m_pShareData->m_sNodes.m_pEditArr[i].m_nGroup )
		{
			if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) )
			{
				if( iIndex == nIndex )
					return &m_pShareData->m_sNodes.m_pEditArr[i];
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


/** いくつかのウィンドウへ終了要求を出す

	@param pWndArr [in] EditNodeの配列。m_hWndがNULLの要素は処理しない
	@param nArrCnt [in] pWndArrの長さ
	@param bExit [in] TRUE: 編集の全終了 / FALSE: すべて閉じる
	@param nGroup [in] グループ指定（0:全グループ）
	@param bCheckConfirm [in] FALSE:複数ウィンドウを閉じるときの警告を出さない / TRUE:警告を出す（設定による）
	@param hWndFrom [in] 終了要求元のウィンドウ（警告メッセージの親となる）

	@date 2007.02.13 ryoji 「編集の全終了」を示す引数(bExit)を追加
	@date 2007.06.22 ryoji nGroup引数を追加
	@date 2009.07.20 syat 全て→いくつかに変更。複数ウィンドウを閉じる時の警告メッセージを追加
*/
BOOL CShareData::RequestCloseEditor( EditNode* pWndArr, int nArrCnt, BOOL bExit, int nGroup, BOOL bCheckConfirm, HWND hWndFrom )
{
	int nCloseCount = 0;
	int i;

	/* クローズ対象ウィンドウの数を調べる */
	for( i = 0; i < nArrCnt; i++ ){
		if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
			if( pWndArr[i].m_hWnd ){
				nCloseCount++;
			}
		}
	}

	if( bCheckConfirm && GetShareData()->m_Common.m_sGeneral.m_bCloseAllConfirm ){	//[すべて閉じる]で他に編集用のウィンドウがあれば確認する
		if( 1 < nCloseCount ){
			if( IDYES != ::MYMESSAGEBOX(
				hWndFrom,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				_T("同時に複数の編集用ウィンドウを閉じようとしています。これらを閉じますか?")
			) ){
				return FALSE;
			}
		}
	}

	for( i = 0; i < nArrCnt; ++i ){
		/* m_hWndにNULLを設定したEditNodeはとばす */
		if( pWndArr[i].m_hWnd == NULL )continue;

		if( nGroup == 0 || nGroup == pWndArr[i].m_nGroup ){
			if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
				/* アクティブにする */
				ActivateFrameWindow( pWndArr[i].m_hWnd );
				/* トレイからエディタへの終了要求 */
				if( !::SendMessage( pWndArr[i].m_hWnd, MYWM_CLOSE, bExit, 0 ) ){	// 2007.02.13 ryoji bExitを引き継ぐ
					return FALSE;
				}
			}
		}
	}
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
	int			i;
	EditInfo*	pfi;
	*phwndOwner = NULL;

	// 現在の編集ウィンドウの数を調べる
	if( 0 ==  GetEditorWindowsNum( 0 ) ){
		return FALSE;
	}
	
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) ){
			// トレイからエディタへの編集ファイル名要求通知
			::SendMessage( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd, MYWM_GETFILEINFO, 1, 0 );
			pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

			// 同一パスのファイルが既に開かれているか
			if( 0 == my_stricmp( pfi->m_szPath, pszPath ) ){
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

	@note	CEditDoc::FileReadに先立って実行されることもあるが、
			CEditDoc::FileReadからも実行される必要があることに注意。
			(フォルダ指定の場合やCEditDoc::FileReadが直接実行される場合もあるため)

	@retval	TRUE すでに開いていた
	@retval	FALSE 開いていなかった

	@date 2007.03.12 maru 新規作成
*/
BOOL CShareData::ActiveAlreadyOpenedWindow( const TCHAR* pszPath, HWND* phwndOwner, int nCharCode )
{
	if( IsPathOpened( pszPath, phwndOwner ) ){
		EditInfo*		pfi;
		CMRUFile		cMRU;
		::SendMessage( *phwndOwner, MYWM_GETFILEINFO, 0, 0 );
		pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;
		if(nCharCode != CODE_AUTODETECT){
			char*	pszCodeNameCur = NULL;
			char*	pszCodeNameNew = NULL;
			if( IsValidCodeType(nCharCode) ){
				pszCodeNameNew = (char*)gm_pszCodeNameArr_1[nCharCode];
			}
			if( IsValidCodeType(pfi->m_nCharCode) ){
				pszCodeNameCur = (char*)gm_pszCodeNameArr_1[pfi->m_nCharCode];
			}

			if(NULL != pszCodeNameCur && pszCodeNameNew){
				if(nCharCode != pfi->m_nCharCode){
					TopWarningMessage( *phwndOwner,
						_T("%s\n\n\n既に開いているファイルを違う文字コードで開く場合は、\n")
						_T("ファイルメニューから「開き直す」を使用してください。\n")
						_T("\n")
						_T("現在の文字コードセット=[%s]\n")
						_T("新しい文字コードセット=[%s]"),
						pszPath,
						pszCodeNameCur,
						pszCodeNameNew
					);
				}
			}
			else{
				TopWarningMessage( *phwndOwner,
					_T("%s\n\n多重オープンの確認で不明な文字コードが指定されました。\n")
					_T("\n")
					_T("現在の文字コードセット=%d [%s]\n新しい文字コードセット=%d [%s]"),
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
		cMRU.Add( pfi );
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
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) ){
			if( nGroup != 0 && nGroup != GetGroupId( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) )
				continue;
			if( bExcludeClosing && m_pShareData->m_sNodes.m_pEditArr[i].m_bClosing )
				continue;
			j++;
		}
	}
	return j;
}


/** 全編集ウィンドウへメッセージをポストする

	@date 2005.01.24 genta hWndLast == NULLのとき全くメッセージが送られなかった
	@date 2007.06.22 ryoji nGroup引数を追加、グループ単位で順番に送る
*/
BOOL CShareData::PostMessageToAllEditors(
	UINT		uMsg,		/*!< ポストするメッセージ */
	WPARAM		wParam,		/*!< 第1メッセージ パラメータ */
	LPARAM		lParam,		/*!< 第2メッセージ パラメータ */
	HWND		hWndLast,	/*!< 最後に送りたいウィンドウ */
	int			nGroup/* = 0*/	/*!< 送りたいグループ(0:全グループ) */
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
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
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
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
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

	@date 2005.01.24 genta m_hWndLast == NULLのとき全くメッセージが送られなかった
	@date 2007.06.22 ryoji nGroup引数を追加、グループ単位で順番に送る
*/
BOOL CShareData::SendMessageToAllEditors(
	UINT		uMsg,		/* ポストするメッセージ */
	WPARAM		wParam,		/* 第1メッセージ パラメータ */
	LPARAM		lParam,		/* 第2メッセージ パラメータ */
	HWND		hWndLast,	/* 最後に送りたいウィンドウ */
	int			nGroup		/*!< [in] グループ指定（0:全グループ） */
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
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
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
				if( IsSakuraMainWindow( pWndArr[i].m_hWnd ) ){
					/* メッセージを送る */
					::SendMessage( pWndArr[i].m_hWnd, uMsg, wParam, lParam );
				}
			}
		}
	}

	delete []pWndArr;
	return TRUE;
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
	nRet = _GetOpenedWindowArrCore( ppEditNode, bSort, bGSort );

	return nRet;
}

// GetOpenedWindowArr関数コア処理部
int CShareData::_GetOpenedWindowArrCore( EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */ )
{
	//編集ウインドウ数を取得する。
	EditNodeEx* pNode;	// ソート処理用の拡張リスト
	int		nRowNum;	//編集ウインドウ数
	int		i;

	//編集ウインドウ数を取得する。
	*ppEditNode = NULL;
	if( m_pShareData->m_sNodes.m_nEditArrNum <= 0 )
		return 0;

	//編集ウインドウリスト格納領域を作成する。
	*ppEditNode = new EditNode[ m_pShareData->m_sNodes.m_nEditArrNum ];
	if( NULL == *ppEditNode )
		return 0;

	// 拡張リストを作成する
	pNode = new EditNodeEx[ m_pShareData->m_sNodes.m_nEditArrNum ];
	if( NULL == pNode )
	{
		delete [](*ppEditNode);
		*ppEditNode = NULL;
		return 0;
	}

	// 拡張リストの各要素に編集ウィンドウリストの各要素へのポインタを格納する
	nRowNum = 0;
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; i++ )
	{
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[ i ].m_hWnd ) )
		{
			pNode[ nRowNum ].p = &m_pShareData->m_sNodes.m_pEditArr[ i ];	// ポインタ格納
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
		(*ppEditNode)[i].m_nIndex = pNode[i].p - m_pShareData->m_sNodes.m_pEditArr;	// ポインタ減算＝配列番号
	}

	delete []pNode;

	return nRowNum;
}

/** ウィンドウの並び替え

	@param[in] hwndSrc 移動するウィンドウ
	@param[in] hwndDst 移動先ウィンドウ

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
	nCount = _GetOpenedWindowArrCore( &p, TRUE );	// ロックは自分でやっているので直接コア部呼び出し
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
	nIndex = m_pShareData->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex;
	if( nSrcTab < nDstTab )
	{
		// タブ左方向ローテート
		for( i = nDstTab - 1; i >= nSrcTab; i-- )
		{
			nArr1 = p[ i ].m_nIndex;
			m_pShareData->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex = m_pShareData->m_sNodes.m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	else
	{
		// タブ右方向ローテート
		for( i = nDstTab + 1; i <= nSrcTab; i++ )
		{
			nArr1 = p[ i ].m_nIndex;
			m_pShareData->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex = m_pShareData->m_sNodes.m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	m_pShareData->m_sNodes.m_pEditArr[ nArr0 ].m_nIndex = nIndex;

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
		nDstGroup = ++m_pShareData->m_sNodes.m_nGroupSequences;	// 新規グループ
	}
	else
	{
		nDstGroup = pDstEditNode->m_nGroup;	// 既存グループ
	}

	pSrcEditNode->m_nGroup = nDstGroup;
	pSrcEditNode->m_nIndex = ++m_pShareData->m_sNodes.m_nSequences;	// タブ並びの最後（起動順の最後）にもっていく

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

	if( NULL == m_pShareData->m_sHandles.m_hwndDebug
	|| !IsSakuraMainWindow( m_pShareData->m_sHandles.m_hwndDebug )
	){
		// 2007.06.26 ryoji
		// アウトプットウィンドウを作成元と同じグループに作成するために m_hwndTraceOutSource を使っています
		// （m_hwndTraceOutSource は CEditWnd::Create() で予め設定）
		// ちょっと不恰好だけど、TraceOut() の引数にいちいち起動元を指定するのも．．．
		CEditApp::OpenNewEditor( NULL, m_hwndTraceOutSource, "-DEBUGMODE", CODE_SJIS, false, true );
		//	2001/06/23 N.Nakatani 窓が出るまでウエイトをかけるように修正
		//アウトプットウインドウが出来るまで5秒ぐらい待つ。
		//	Jun. 25, 2001 genta OpenNewEditorの同期機能を利用するように変更

		/* 開いているウィンドウをアクティブにする */
		/* アクティブにする */
		ActivateFrameWindow( m_pShareData->m_sHandles.m_hwndDebug );
	}
	va_list argList;
	va_start( argList, lpFmt );
	wvsprintf( m_pShareData->m_sWorkBuffer.m_szWork, lpFmt, argList );
	va_end( argList );
	::SendMessage( m_pShareData->m_sHandles.m_hwndDebug, MYWM_ADDSTRING, 0, 0 );
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
	if( -1 != idx && !m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].IsEnabled() )
		return 0;
	TCHAR *ptr;
	TCHAR *pszFile;

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
	int nLen = _tcslen( ptr ); // Jul. 21, 2003 genta strlen対象が誤っていたためマクロ実行ができない

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
		int nFolderSep = AddLastChar( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER, sizeof(m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER), _T('\\') );
		int nAllLen;
		TCHAR *pszDir;

		 // 2003.06.24 Moca フォルダも相対パスなら実行ファイルからのパス
		// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
		if( _IS_REL_PATH( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER ) ){
			TCHAR szDir[_MAX_PATH + sizeof( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER )];
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
	if( !m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].IsEnabled() )
		return false;

	return ( m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].m_bReloadWhenExecute == TRUE );
}

/*!	m_szSEARCHKEYArrにpszSearchKeyを追加する。
	YAZAKI
*/
void CShareData::AddToSearchKeyArr( const char* pszSearchKey )
{
	CRecent	cRecentSearchKey;
	cRecentSearchKey.EasyCreate( RECENT_FOR_SEARCH );
	cRecentSearchKey.AppendItem( pszSearchKey );
	cRecentSearchKey.Terminate();
}

/*!	m_szREPLACEKEYArrにpszReplaceKeyを追加する
	YAZAKI
*/
void CShareData::AddToReplaceKeyArr( const char* pszReplaceKey )
{
	CRecent	cRecentReplaceKey;
	cRecentReplaceKey.EasyCreate( RECENT_FOR_REPLACE );
	cRecentReplaceKey.AppendItem( pszReplaceKey );
	cRecentReplaceKey.Terminate();

	return;
}

/*!	m_szGREPFILEArrにpszGrepFileを追加する
	YAZAKI
*/
void CShareData::AddToGrepFileArr( const char* pszGrepFile )
{
	CRecent	cRecentGrepFile;
	cRecentGrepFile.EasyCreate( RECENT_FOR_GREP_FILE );
	cRecentGrepFile.AppendItem( pszGrepFile );
	cRecentGrepFile.Terminate();
}

/*!	m_nGREPFOLDERArrNumにpszGrepFolderを追加する
	YAZAKI
*/
void CShareData::AddToGrepFolderArr( const char* pszGrepFolder )
{
	CRecent	cRecentGrepFolder;
	cRecentGrepFolder.EasyCreate( RECENT_FOR_GREP_FOLDER );
	cRecentGrepFolder.AppendItem( pszGrepFolder );
	cRecentGrepFolder.Terminate();
}

/*!	外部Winヘルプが設定されているか確認。
*/
bool CShareData::ExtWinHelpIsSet( int nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHelp[0] != '\0'){
		return true;	//	共通設定に設定されている
	}
	if (nTypeNo < 0 || MAX_TYPES <= nTypeNo ){
		return false;	//	共通設定に設定されていない＆nTypeNoが範囲外。
	}
	if (m_pShareData->m_Types[nTypeNo].m_szExtHelp[0] != '\0'){
		return true;	//	タイプ別設定に設定されている。
	}
	return false;
}

/*!	設定されている外部Winヘルプのファイル名を返す。
	タイプ別設定にファイル名が設定されていれば、そのファイル名を返します。
	そうでなければ、共通設定のファイル名を返します。
*/
char* CShareData::GetExtWinHelp( int nTypeNo )
{
	if (0 <= nTypeNo && nTypeNo < MAX_TYPES && m_pShareData->m_Types[nTypeNo].m_szExtHelp[0] != '\0'){
		return m_pShareData->m_Types[nTypeNo].m_szExtHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHelp;
}
/*!	外部HTMLヘルプが設定されているか確認。
*/
bool CShareData::ExtHTMLHelpIsSet( int nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp[0] != '\0'){
		return true;	//	共通設定に設定されている
	}
	if (nTypeNo < 0 || MAX_TYPES <= nTypeNo ){
		return false;	//	共通設定に設定されていない＆nTypeNoが範囲外。
	}
	if (m_pShareData->m_Types[nTypeNo].m_szExtHtmlHelp[0] != '\0'){
		return true;	//	タイプ別設定に設定されている。
	}
	return false;
}

/*!	設定されている外部Winヘルプのファイル名を返す。
	タイプ別設定にファイル名が設定されていれば、そのファイル名を返します。
	そうでなければ、共通設定のファイル名を返します。
*/
char* CShareData::GetExtHTMLHelp( int nTypeNo )
{
	if (0 <= nTypeNo && nTypeNo < MAX_TYPES && m_pShareData->m_Types[nTypeNo].m_szExtHtmlHelp[0] != '\0'){
		return m_pShareData->m_Types[nTypeNo].m_szExtHtmlHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp;
}
/*!	ビューアを複数起動しないがONかを返す。
*/
bool CShareData::HTMLHelpIsSingle( int nTypeNo )
{
	if (0 <= nTypeNo && nTypeNo < MAX_TYPES && m_pShareData->m_Types[nTypeNo].m_szExtHtmlHelp[0] != '\0'){
		return (m_pShareData->m_Types[nTypeNo].m_bHtmlHelpIsSingle != FALSE);
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
const char* CShareData::MyGetDateFormat( const SYSTEMTIME& systime, char* pszDest, int nDestLen )
{
	return MyGetDateFormat( systime, pszDest, nDestLen, m_pShareData->m_Common.m_sFormat.m_nDateFormatType, m_pShareData->m_Common.m_sFormat.m_szDateFormat );
}

const char* CShareData::MyGetDateFormat( const SYSTEMTIME& systime, char* pszDest, int nDestLen, int nDateFormatType, char* szDateFormat )
{
	const char* pszForm;
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
const char* CShareData::MyGetTimeFormat( const SYSTEMTIME& systime, char* pszDest, int nDestLen )
{
	return MyGetTimeFormat( systime, pszDest, nDestLen, m_pShareData->m_Common.m_sFormat.m_nTimeFormatType, m_pShareData->m_Common.m_sFormat.m_szTimeFormat );
}

/* 時刻をフォーマット */
const char* CShareData::MyGetTimeFormat( const SYSTEMTIME& systime, char* pszDest, int nDestLen, int nTimeFormatType, char* szTimeFormat )
{
	const char* pszForm;
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
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[m_nTransformFileNameOrgId[0]] );
		for( i = 1; i < m_nTransformFileNameCount; i++ ){
			_tcscpy( szBuf, pszDest );
			GetFilePathFormat( szBuf, pszDest, nDestLen,
				m_szTransformFileNameFromExp[i],
				m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[m_nTransformFileNameOrgId[i]] );
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
	for( i = 0; i < m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum; i++ ){
		if( '\0' != m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i][0] ){
			if( ExpandMetaToFolder( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i],
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
	@param pszSrc   [in]  ファイル名
	@param pszDest  [out] 変換後のファイル名の格納先
	@param nDestLen [in]  終端のNULLを含むpszDestのTCHAR単位の長さ
	@param pszFrom  [in]  置換前文字列
	@param pszTo    [in]  置換後文字列
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
		if( 0 == my_strnicmp( &pszSrc[i], pszFrom, nFromLen ) )
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
	LPTSTR  pStr;

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
	for( ps = pszSrc, pd = pszDes; '\0' != *ps; ps++ ){
		if( pd_end <= pd ){
			if( pd_end == pd ){
				*pd = '\0';
			}
			return false;
		}

		if( '%' != *ps ){
			*pd = *ps;
			pd++;
			continue;
		}

		// %% は %
		if( '%' == ps[1] ){
			*pd = '%';
			pd++;
			ps++;
			continue;
		}

		if( '\0' != ps[1] ){
			TCHAR szMeta[_MAX_PATH];
			TCHAR szPath[_MAX_PATH + 1];
			int   nMetaLen;
			int   nPathLen;
			bool  bFolderPath;
			ps++;
			// %SAKURA%
			if( 0 == my_tcsnicmp( _T("SAKURA%"), ps, 7 ) ){
				// exeのあるフォルダ
				GetExedir( szPath );
				nMetaLen = 6;
			// %SAKURADATA%	// 2007.06.06 ryoji
			}else if( 0 == my_tcsnicmp( _T("SAKURADATA%"), ps, 11 ) ){
				// iniのあるフォルダ
				GetInidir( szPath );
				nMetaLen = 10;
			// メタ文字列っぽい
			}else if( NULL != (pStr = _tcschr( (LPTSTR)ps, '%' ) )){
				nMetaLen = pStr - ps;
				if( nMetaLen < _MAX_PATH ){
					_tmemcpy( szMeta, ps, nMetaLen );
					szMeta[nMetaLen] = '\0';
				}else{
					*pd = '\0';
					return false;
				}
#ifdef _USE_META_ALIAS
				// メタ文字列がエイリアス名なら書き換える
				const MetaAlias* pAlias;
				for( pAlias = &AliasList[0]; nMetaLen < pAlias->nLenth; pAlias++ )
					; // 読み飛ばす
				for( ; nMetaLen == pAlias->nLenth; pAlias++ ){
					if( 0 == my_tcsicmp( pAlias->szAlias, szMeta ) ){
						_tcscpy( szMeta, pAlias->szOrig );
						break;
					}
				}
#endif
				// 直接レジストリで調べる
				szPath[0] = '\0';
				bFolderPath = ReadRegistry( HKEY_CURRENT_USER,
					_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
					szMeta, szPath, sizeof( szPath ) );
				if( false == bFolderPath || '\0' == szPath[0] ){
					bFolderPath = ReadRegistry( HKEY_LOCAL_MACHINE,
						_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
						szMeta, szPath, sizeof( szPath ) );
				}
				if( false == bFolderPath || '\0' == szPath[0] ){
					pStr = _tgetenv( szMeta );
					// 環境変数
					if( NULL != pStr ){
						nPathLen = _tcslen( pStr );
						if( nPathLen < _MAX_PATH ){
							_tcscpy( szPath, pStr );
						}else{
							*pd = '\0';
							return false;
						}
					}
					// 未定義のメタ文字列は 入力された%...%を，そのまま文字として処理する
					else if(  pd + ( nMetaLen + 2 ) < pd_end ){
						*pd = '%';
						_tmemcpy( &pd[1], ps, nMetaLen );
						pd[nMetaLen + 1] = '%';
						pd += nMetaLen + 2;
						ps += nMetaLen;
						continue;
					}else{
						*pd = '\0';
						return false;
					}
				}
			}else{
				// %...%の終わりの%がない とりあえず，%をコピー
				*pd = '%';
				pd++;
				ps--; // 先にps++してしまったので戻す
				continue;
			}

			// ロングファイル名にする
			nPathLen = _tcslen( szPath );
			pStr = szPath;
			if( nPathLen < _MAX_PATH && 0 != nPathLen ){
				if( FALSE != GetLongFileName( szPath, szMeta ) ){
					pStr = szMeta;
				}
			}

			// 最後のフォルダ区切り記号を削除する
			// [A:\]などのルートであっても削除
			for(nPathLen = 0; pStr[nPathLen] != '\0'; nPathLen++ ){
#ifdef _MBCS
				if( _IS_SJIS_1( (unsigned char)pStr[nPathLen] ) && _IS_SJIS_2( (unsigned char)pStr[nPathLen + 1] ) ){
					// SJIS読み飛ばし
					nPathLen++; // 2003/01/17 sui
				}else
#endif
				if( '\\' == pStr[nPathLen] && '\0' == pStr[nPathLen + 1] ){
					pStr[nPathLen] = '\0';
					break;
				}
			}

			if( pd + nPathLen < pd_end && 0 != nPathLen ){
				_tmemcpy( pd, pStr, nPathLen );
				pd += nPathLen;
				ps += nMetaLen;
			}else{
				*pd = '\0';
				return false;
			}
		}else{
			// 最後の文字が%だった
			*pd = *ps;
			pd++;
		}
	}
	*pd = '\0';
	return true;
}

static const char* ppszKeyWordsCPP[] = {
	"#define",
	"#elif",
	"#else",
	"#endif",
	"#error",
	"#if",
	"#ifdef",
	"#ifndef",
	"#include",
	"#line",
	"#pragma",
	"#undef",
	"__FILE__",
	"__declspec",
	"asm",
	"auto",
	"bool",
	"break",
	"case",
	"catch",
	"char",
	"class",
	"const",
	"const_cast",
	"continue",
	"default",
	"define",
	"defined",
	"delete",
	"do",
	"double",
	"dynamic_cast",
	"elif",
	"else",
	"endif",
	"enum",
	"error",
	"explicit",
	"export",
	"extern",
	"false",
	"float",
	"for",
	"friend",
	"goto",
	"if",
	"ifdef",
	"ifndef",
	"include",
	"inline",
	"int",
	"line",
	"long",
	"mutable",
	"namespace",
	"new",
	"operator",
	"pragma",
	"private",
	"protected",
	"public",
	"register",
	"reinterpret_cast",
	"return",
	"short",
	"signed",
	"sizeof",
	"static",
	"static_cast",
	"struct",
	"switch",
	"template",
	"this",
	"throw",
	"true",
	"try",
	"typedef",
	"typeid",
	"typename",
	"undef",
	"union",
	"unsigned",
	"using",
	"virtual",
	"void",
	"volatile",
	"wchar_t",
	"while"
};

static const char* ppszKeyWordsHTML[] = {
	"_blank",
	"_parent",
	"_self",
	"_top",
	"A",
	"ABBR",
	"ABOVE",
	"absbottom",
	"absmiddle",
	"ACCESSKEY",
	"ACRONYM",
	"ACTION",
	"ADDRESS",
	"ALIGN",
	"all",
	"APPLET",
	"AREA",
	"AUTOPLAY",
	"AUTOSTART",
	"B",
	"BACKGROUND",
	"BASE",
	"BASEFONT",
	"baseline",
	"BEHAVIOR",
	"BELOW",
	"BGCOLOR",
	"BGSOUND",
	"BIG",
	"BLINK",
	"BLOCKQUOTE",
	"BODY",
	"BORDER",
	"BORDERCOLOR",
	"BORDERCOLORDARK",
	"BORDERCOLORLIGHT",
	"BOTTOM",
	"box",
	"BR",
	"BUTTON",
	"CAPTION",
	"CELLPADDING",
	"CELLSPACING",
	"CENTER",
	"CHALLENGE",
	"char",
	"checkbox",
	"CHECKED",
	"CITE",
	"CLEAR",
	"CLIP",
	"CODE",
	"CODEBASE",
	"CODETYPE",
	"COL",
	"COLGROUP",
	"COLOR",
	"COLS",
	"COLSPAN",
	"COMMENT",
	"CONTROLS",
	"DATA",
	"DD",
	"DECLARE",
	"DEFER",
	"DEL",
	"DELAY",
	"DFN",
	"DIR",
	"DIRECTION",
	"DISABLED",
	"DIV",
	"DL",
	"DOCTYPE",
	"DT",
	"EM",
	"EMBED",
	"ENCTYPE",
	"FACE",
	"FIELDSET",
	"file",
	"FONT",
	"FOR",
	"FORM",
	"FRAME",
	"FRAMEBORDER",
	"FRAMESET",
	"GET",
	"groups",
	"GROUPS",
	"GUTTER",
	"H1",
	"H2",
	"H3",
	"H4",
	"H5",
	"H6",
	"H7",
	"HEAD",
	"HEIGHT",
	"HIDDEN",
	"Hn",
	"HR",
	"HREF",
	"hsides",
	"HSPACE",
	"HTML",
	"I",
	"ID",
	"IFRAME",
	"ILAYER",
	"image",
	"IMG",
	"INDEX",
	"inherit",
	"INPUT",
	"INS",
	"ISINDEX",
	"JavaScript",
	"justify",
	"KBD",
	"KEYGEN",
	"LABEL",
	"LANGUAGE",
	"LAYER",
	"LEFT",
	"LEGEND",
	"lhs",
	"LI",
	"LINK",
	"LISTING",
	"LOOP",
	"MAP",
	"MARQUEE",
	"MAXLENGTH",
	"MENU",
	"META",
	"METHOD",
	"METHODS",
	"MIDDLE",
	"MULTICOL",
	"MULTIPLE",
	"NAME",
	"NEXT",
	"NEXTID",
	"NOBR",
	"NOEMBED",
	"NOFRAMES",
	"NOLAYER",
	"none",
	"NOSAVE",
	"NOSCRIPT",
	"NOTAB",
	"NOWRAP",
	"OBJECT",
	"OL",
	"onBlur",
	"onChange",
	"onClick",
	"onFocus",
	"onLoad",
	"onMouseOut",
	"onMouseOver",
	"onReset",
	"onSelect",
	"onSubmit",
	"OPTION",
	"P",
	"PAGEX",
	"PAGEY",
	"PALETTE",
	"PANEL",
	"PARAM",
	"PARENT",
	"password",
	"PLAINTEXT",
	"PLUGINSPAGE",
	"POST",
	"PRE",
	"PREVIOUS",
	"Q",
	"radio",
	"REL",
	"REPEAT",
	"reset",
	"REV",
	"rhs",
	"RIGHT",
	"rows",
	"ROWSPAN",
	"RULES",
	"S",
	"SAMP",
	"SAVE",
	"SCRIPT",
	"SCROLLAMOUNT",
	"SCROLLDELAY",
	"SELECT",
	"SELECTED",
	"SERVER",
	"SHAPES",
	"show",
	"SIZE",
	"SMALL",
	"SONG",
	"SPACER",
	"SPAN",
	"SRC",
	"STANDBY",
	"STRIKE",
	"STRONG",
	"STYLE",
	"SUB",
	"submit",
	"SUMMARY",
	"SUP",
	"TABINDEX",
	"TABLE",
	"TARGET",
	"TBODY",
	"TD",
	"TEXT",
	"TEXTAREA",
	"textbottom",
	"TEXTFOCUS",
	"textmiddle",
	"texttop",
	"TFOOT",
	"TH",
	"THEAD",
	"TITLE",
	"TOP",
	"TR",
	"TT",
	"TXTCOLOR",
	"TYPE",
	"U",
	"UL",
	"URN",
	"USEMAP",
	"VALIGN",
	"VALUE",
	"VALUETYPE",
	"VAR",
	"VISIBILITY",
	"void",
	"vsides",
	"VSPACE",
	"WBR",
	"WIDTH",
	"WRAP",
	"XMP"
};

static const char* ppszKeyWordsPLSQL[] = {
	"AND",
	"AS",
	"BEGIN",
	"BINARY_INTEGER",
	"BODY",
	"BOOLEAN",
	"BY",
	"CHAR",
	"CHR",
	"COMMIT",
	"COUNT",
	"CREATE",
	"CURSOR",
	"DATE",
	"DECLARE",
	"DEFAULT",
	"DELETE",
	"ELSE",
	"ELSIF",
	"END",
	"ERRORS",
	"EXCEPTION",
	"FALSE",
	"FOR",
	"FROM",
	"FUNCTION",
	"GOTO",
	"HTP",
	"IDENT_ARR",
	"IF",
	"IN",
	"INDEX",
	"INTEGER",
	"IS",
	"LOOP",
	"NOT",
	"NO_DATA_FOUND",
	"NULL",
	"NUMBER",
	"OF",
	"OR",
	"ORDER",
	"OUT",
	"OWA_UTIL",
	"PACKAGE",
	"PRAGMA",
	"PRN",
	"PROCEDURE",
	"REPLACE",
	"RESTRICT_REFERENCES",
	"RETURN",
	"ROWTYPE",
	"SELECT",
	"SHOW",
	"SUBSTR",
	"TABLE",
	"THEN",
	"TRUE",
	"TYPE",
	"UPDATE",
	"VARCHAR",
	"VARCHAR2",
	"WHEN",
	"WHERE",
	"WHILE",
	"WNDS",
	"WNPS",
	"RAISE",
	"INSERT",
	"INTO",
	"VALUES",
	"SET",
	"SYSDATE",
	"RTRIM",
	"LTRIM",
	"TO_CHAR",
	"DUP_VAL_ON_INDEX",
	"ROLLBACK",
	"OTHERS",
	"SQLCODE"
};

//Jul. 10, 2001 JEPRO 追加
static const char* ppszKeyWordsCOBOL[] = {
	"ACCEPT",
	"ADD",
	"ADVANCING",
	"AFTER",
	"ALL",
	"AND",
	"ARGUMENT",
	"ASSIGN",
	"AUTHOR",
	"BEFORE",
	"BLOCK",
	"BY",
	"CALL",
	"CHARACTERS",
	"CLOSE",
	"COMP",
	"COMPILED",
	"COMPUTE",
	"COMPUTER",
	"CONFIGURATION",
	"CONSOLE",
	"CONTAINS",
	"CONTINUE",
	"CONTROL",
	"COPY",
	"DATA",
	"DELETE",
	"DISPLAY",
	"DIVIDE",
	"DIVISION",
	"ELSE",
	"END",
	"ENVIRONMENT",
	"EVALUATE",
	"EXAMINE",
	"EXIT",
	"EXTERNAL",
	"FD",
	"FILE",
	"FILLER",
	"FROM",
	"GIVING",
	"GO",
	"GOBACK",
	"HIGH-VALUE",
	"IDENTIFICATION"
	"IF",
	"INITIALIZE",
	"INPUT",
	"INTO",
	"IS",
	"LABEL",
	"LINKAGE",
	"LOW-VALUE",
	"MODE",
	"MOVE",
	"NOT",
	"OBJECT",
	"OCCURS",
	"OF",
	"ON",
	"OPEN",
	"OR",
	"OTHER",
	"OUTPUT",
	"PERFORM",
	"PIC",
	"PROCEDURE",
	"PROGRAM",
	"READ",
	"RECORD",
	"RECORDING",
	"REDEFINES",
	"REMAINDER",
	"REMARKS",
	"REPLACING",
	"REWRITE",
	"ROLLBACK",
	"SECTION",
	"SELECT",
	"SOURCE",
	"SPACE",
	"STANDARD",
	"STOP",
	"STORAGE",
	"SYSOUT",
	"TEST",
	"THEN",
	"TO",
	"TODAY",
	"TRANSFORM",
	"UNTIL",
	"UPON",
	"USING",
	"VALUE",
	"VARYING",
	"WHEN",
	"WITH",
	"WORKING",
	"WRITE",
	"WRITTEN",
	"ZERO"
};

static const char* ppszKeyWordsJAVA[] = {
	"abstract",
	"assert",	// Mar. 8, 2003 genta
	"boolean",
	"break",
	"byte",
	"case",
	"catch",
	"char",
	"class",
	"const",
	"continue",
	"default",
	"do",
	"double",
	"else",
	"extends",
	"final",
	"finally",
	"float",
	"for",
	"goto",
	"if",
	"implements",
	"import",
	"instanceof",
	"int",
	"interface",
	"long",
	"native",
	"new",
	"package",
	"private",
	"protected",
	"public",
	"return",
	"short",
	"static",
	"strictfp",	// Mar. 8, 2003 genta
	"super",
	"switch",
	"synchronized",
	"this",
	"throw",
	"throws",
	"transient",
	"try",
	"void",
	"volatile",
	"while"
};

static const char* ppszKeyWordsCORBA_IDL[] = {
	"any",
	"attribute",
	"boolean",
	"case",
	"char",
	"const",
	"context",
	"default",
	"double",
	"enum",
	"exception",
	"FALSE",
	"fixed",
	"float",
	"in",
	"inout",
	"interface",
	"long",
	"module",
	"Object",
	"octet",
	"oneway",
	"out",
	"raises",
	"readonly",
	"sequence",
	"short",
	"string",
	"struct",
	"switch",
	"TRUE",
	"typedef",
	"unsigned",
	"union",
	"void",
	"wchar",
	"wstring"
};

static const char* ppszKeyWordsAWK[] = {
	"BEGIN",
	"END",
	"next",
	"exit",
	"func",
	"function",
	"return",
	"if",
	"else",
	"for",
	"in",
	"do",
	"while",
	"break",
	"continue",
	"$0",
	"$1",
	"$2",
	"$3",
	"$4",
	"$5",
	"$6",
	"$7",
	"$8",
	"$9",
	"$10",
	"$11",
	"$12",
	"$13",
	"$14",
	"$15",
	"$16",
	"$17",
	"$18",
	"$19",
	"$20",
	"FS",
	"OFS",
	"NF",
	"RS",
	"ORS",
	"NR",
	"FNR",
	"ARGV",
	"ARGC",
	"ARGIND",
	"FILENAME",
	"ENVIRON",
	"ERRNO",
	"OFMT",
	"CONVFMT",
	"FIELDWIDTHS",
	"IGNORECASE",
	"RLENGTH",
	"RSTART",
	"SUBSEP",
	"delete",
	"index",
	"jindex",
	"length",
	"jlength",
	"substr",
	"jsubstr",
	"match",
	"split",
	"sub",
	"gsub",
	"sprintf",
	"tolower",
	"toupper",
	"print",
	"printf",
	"getline",
	"system",
	"close",
	"sin",
	"cos",
	"atan2",
	"exp",
	"log",
	"int",
	"sqrt",
	"srand",
	"rand",
	"strftime",
	"systime"
};

static const char* ppszKeyWordsBAT[] = {
	"PATH",
	"PROMPT",
	"TEMP",
	"TMP",
	"TZ",
	"CONFIG",
	"COMSPEC",
	"DIRCMD",
	"COPYCMD",
	"winbootdir",
	"windir",
	"DIR",
	"CALL",
	"CHCP",
	"RENAME",
	"REN",
	"ERASE",
	"DEL",
	"TYPE",
	"REM",
	"COPY",
	"PAUSE",
	"DATE",
	"TIME",
	"VER",
	"VOL",
	"CD",
	"CHDIR",
	"MD",
	"MKDIR",
	"RD",
	"RMDIR",
	"BREAK",
	"VERIFY",
	"SET",
	"EXIT",
	"CTTY",
	"ECHO",
	"@ECHO",	//Oct. 31, 2000 JEPRO '@' を強調可能にしたので追加
	"LOCK",
	"UNLOCK",
	"GOTO",
	"SHIFT",
	"IF",
	"FOR",
	"DO",	//Nov. 2, 2000 JEPRO 追加
	"IN",	//Nov. 2, 2000 JEPRO 追加
	"ELSE",	//Nov. 2, 2000 JEPRO 追加 Win2000で使える
	"CLS",
	"TRUENAME",
	"LOADHIGH",
	"LH",
	"LFNFOR",
	"ON",
	"OFF",
	"NOT",
	"ERRORLEVEL",
	"EXIST",
	"NUL",
	"CON",
	"AUX",
	"COM1",
	"COM2",
	"COM3",
	"COM4",
	"PRN",
	"LPT1",
	"LPT2",
	"LPT3",
	"CLOCK",
	"CLOCK$",
	"CONFIG$"
};

static const char* ppszKeyWordsPASCAL[] = {
	"and",
	"exports",
	"mod",
	"shr",
	"array",
	"file",
	"nil",
	"string",
	"as",
	"finalization",
	"not",
	"stringresource",
	"asm",
	"finally",
	"object",
	"then",
	"begin",
	"for",
	"of",
	"case",
	"function",
	"or",
	"to",
	"class",
	"goto",
	"out",
	"try",
	"const",
	"if",
	"packed",
	"type",
	"constructor",
	"implementation",
	"procedure",
	"unit",
	"destructor",
	"in",
	"program",
	"until",
	"dispinterface",
	"inherited",
	"property",
	"uses",
	"div",
	"initialization",
	"raise",
	"var",
	"do",
	"inline",
	"record",
	"while",
	"downto",
	"interface",
	"repeat",
	"with",
	"else",
	"is",
	"resourcestring",
	"xor",
	"end",
	"label",
	"set",
	"except",
	"library",
	"shl",
	"private",
	"public",
	"published",
	"protected",
	"override"
};

static const char* ppszKeyWordsTEX[] = {
//Nov. 20, 2000 JEPRO	大幅追加 & 若干修正・削除 --ほとんどコマンドのみ
	"error",
	"Warning",
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
	"\\aa",
	"\\AA",
	"\\acute",
	"\\addcontentsline",
	"\\addtocounter",
	"\\addtolength",
	"\\ae",
	"\\AE",
	"\\aleph",
	"\\alpha",
	"\\alph",
	"\\Alph",
	"\\and",
	"\\angle",
	"\\appendix",
	"\\approx",
	"\\arabic",
	"\\arccos",
	"\\arctan",
	"\\arg",
	"\\arrayrulewidth",
	"\\arraystretch",
	"\\ast",
	"\\atop",
	"\\author",
	"\\b",
	"\\backslash",
	"\\bar",
	"\\baselineskip",
	"\\baselinestretch",
	"\\begin",
	"\\beta",
	"\\bf",
	"\\bibitem",
	"\\bibliography",
	"\\bibliographystyle",
	"\\big",
	"\\Big",
	"\\bigcap",
	"\\bigcirc",
	"\\bigcup",
	"\\bigg",
	"\\Bigg",
	"\\Biggl",
	"\\Biggm",
	"\\biggl",
	"\\biggm",
	"\\biggr",
	"\\Biggr",
	"\\bigl",
	"\\bigm",
	"\\Bigm",
	"\\Bigl",
	"\\bigodot",
	"\\bigoplus",
	"\\bigotimes",
	"\\bigr",
	"\\Bigr",
	"\\bigskip",
	"\\bigtriangledown",
	"\\bigtriangleup",
	"\\boldmath",
	"\\bot",
	"\\Box",
	"\\brace",
	"\\breve",
	"\\bullet",
	"\\bye",
	"\\c",
	"\\cal",
	"\\cap",
	"\\caption",
	"\\cc",
	"\\cdot",
	"\\cdots",
	"\\centering",
	"\\chapter",
	"\\check",
	"\\chi",
	"\\choose",
	"\\circ",
	"\\circle",
	"\\cite",
	"\\clearpage",
	"\\cline",
	"\\closing",
	"\\clubsuit",
	"\\colon",
	"\\columnsep",
	"\\columnseprule",
	"\\cong",
	"\\cot",
	"\\coth",
	"\\cr",
	"\\cup",
	"\\d",
	"\\dag",
	"\\dagger",
	"\\date",
	"\\dashbox",
	"\\ddag",
	"\\ddot",
	"\\ddots",
	"\\def",
	"\\deg",
	"\\delta",
	"\\Delta",
	"\\det",
	"\\diamond",
	"\\diamondsuit",
	"\\dim",
	"\\displaystyle",
	"\\documentclass",
	"\\documentstyle",
	"\\dot",
	"\\doteq",
	"\\dotfill",
	"\\Downarrow",
	"\\downarrow",
	"\\ell",
	"\\em",
	"\\emptyset",
	"\\encl",
	"\\end",
	"\\enspace",
	"\\enskip",
	"\\epsilon",
	"\\eqno",
	"\\equiv",
	"\\evensidemargin",
	"\\eta",
	"\\exists",
	"\\exp",
	"\\fbox",
	"\\fboxrule",
	"\\flat",
	"\\footnote",
	"\\footnotesize",
	"\\forall",
	"\\frac",
	"\\frame",
	"\\framebox",
	"\\gamma",
	"\\Gamma",
	"\\gcd",
	"\\ge",
	"\\geq",
	"\\gets",
	"\\gg",
	"\\grave",
	"\\gt",
	"\\H",
	"\\hat",
	"\\hbar",
	"\\hbox",
	"\\headsep",
	"\\heartsuit",
	"\\hfil",
	"\\hfill",
	"\\hline",
	"\\hom",
	"\\hrulefill",
	"\\hskip",
	"\\hspace",
	"\\hspace*",
	"\\huge",
	"\\Huge",
	"\\i",
	"\\Im",
	"\\imath",
	"\\in",
	"\\include",
	"\\includegraphics",
	"\\includeonly",
	"\\indent",
	"\\index",
	"\\inf",
	"\\infty",
	"\\input",
	"\\int",
	"\\iota",
	"\\it",
	"\\item",
	"\\itemsep",
	"\\j",
	"\\jmath",
	"\\kappa",
	"\\ker",
	"\\kern",
	"\\kill",
	"\\l",
	"\\L",
	"\\label",
	"\\lambda",
	"\\Lambda",
	"\\land",
	"\\langle",
	"\\large",
	"\\Large",
	"\\LARGE",
	"\\LaTeX",
	"\\LaTeXe",
	"\\lceil",
	"\\ldots",
	"\\le",
	"\\leftarrow",
	"\\Leftarrow",
	"\\lefteqn",
	"\\leftharpoondown",
	"\\leftharpoonup",
	"\\leftmargin",
	"\\leftrightarrow",
	"\\Leftrightarrow",
	"\\leq",
	"\\leqno",
	"\\lfloor",
	"\\lg",
	"\\lim",
	"\\liminf",
	"\\limsup",
	"\\line",
	"\\linebreak",
	"\\linewidth",
	"\\listoffigures",
	"\\listoftables",
	"\\ll",
	"\\llap",
	"\\ln",
	"\\lnot",
	"\\log",
	"\\longleftarrow",
	"\\Longleftarrow",
	"\\longleftrightarrow",
	"\\Longleftrightarrow",
	"\\longrightarrow",
	"\\Longrightarrow",
	"\\lor",
	"\\lower",
	"\\magstep",
	"\\makeatletter",
	"\\makeatother",
	"\\makebox",
	"\\makeindex",
	"\\maketitle",
	"\\makelabels",
	"\\mathop",
	"\\mapsto",
	"\\markboth",
	"\\markright",
	"\\mathstrut",
	"\\max",
	"\\mbox",
	"\\mc",
	"\\medskip",
	"\\mid",
	"\\min",
	"\\mit",
	"\\mp",
	"\\mu",
	"\\multicolumn",
	"\\multispan",
	"\\multiput",
	"\\nabla",
	"\\natural",
	"\\ne",
	"\\neg",
	"\\nearrow",
	"\\nwarrow",
	"\\neq",
	"\\newblock",
	"\\newcommand",
	"\\newenvironment",
	"\\newfont",
	"\\newlength",
	"\\newline",
	"\\newpage",
	"\\newtheorem",
	"\\ni",
	"\\noalign",
	"\\noindent",
	"\\nolimits",
	"\\nolinebreak",
	"\\nonumber",
	"\\nopagebreak",
	"\\normalsize",
	"\\not",
	"\\notice",
	"\\notin",
	"\\nu",
	"\\o",
	"\\O",
	"\\oddsidemargin",
	"\\odot",
	"\\oe",
	"\\OE",
	"\\oint",
	"\\Omega",
	"\\omega",
	"\\ominus",
	"\\oplus",
	"\\opening",
	"\\otimes",
	"\\owns",
	"\\overleftarrow",
	"\\overline",
	"\\overrightarrow",
	"\\overvrace",
	"\\oval",
	"\\P",
	"\\pagebreak",
	"\\pagenumbering",
	"\\pageref",
	"\\pagestyle",
	"\\par",
	"\\parallel",
	"\\paragraph",
	"\\parbox",
	"\\parindent",
	"\\parskip",
	"\\partial",
	"\\perp",
	"\\phi",
	"\\Phi",
	"\\pi",
	"\\Pi",
	"\\pm",
	"\\Pr",
	"\\prime",
	"\\printindex",
	"\\prod",
	"\\propto",
	"\\ps",
	"\\psi",
	"\\Psi",
	"\\put",
	"\\qquad",
	"\\quad",
	"\\raisebox",
	"\\rangle",
	"\\rceil",
	"\\Re",
	"\\ref",
	"\\renewcommand",
	"\\renewenvironment",
	"\\rfloor",
	"\\rho",
	"\\right",
	"\\rightarrow",
	"\\Rightarrow",
	"\\rightharpoondown",
	"\\rightharpoonup",
	"\\rightleftharpoonup",
	"\\rightmargin",
	"\\rm",
	"\\rule",
	"\\roman",
	"\\Roman",
	"\\S",
	"\\samepage",
	"\\sb",
	"\\sc",
	"\\scriptsize",
	"\\scriptscriptstyle",
	"\\scriptstyle",
	"\\searrow",
	"\\sec",
	"\\section",
	"\\setcounter",
	"\\setlength",
	"\\settowidth",
	"\\setminus",
	"\\sf",
	"\\sharp",
	"\\sigma",
	"\\Sigma",
	"\\signature",
	"\\sim",
	"\\simeq",
	"\\sin",
	"\\sinh",
	"\\sl",
	"\\sloppy",
	"\\small",
	"\\smash",
	"\\smallskip",
	"\\sp",
	"\\spadesuit",
	"\\special",
	"\\sqrt",
	"\\ss",
	"\\star",
	"\\stackrel",
	"\\strut",
	"\\subparagraph",
	"\\subsection",
	"\\subset",
	"\\subseteq",
	"\\subsubsection",
	"\\sum",
	"\\sup",
	"\\supset",
	"\\supseteq",
	"\\swarrow",
	"\\t",
	"\\tableofcontents",
	"\\tan",
	"\\tanh",
	"\\tau",
	"\\TeX",
	"\\textbf",
	"\\textgreater",
	"\\textgt",
	"\\textheight",
	"\\textit",
	"\\textless",
	"\\textmc",
	"\\textrm",
	"\\textsc",
	"\\textsf",
	"\\textsl",
	"\\textstyle",
	"\\texttt",
	"\\textwidth",
	"\\thanks",
	"\\thebibliography",
	"\\theequation",
	"\\thepage",
	"\\thesection",
	"\\theta",
	"\\Theta",
	"\\thicklines",
	"\\thinlines",
	"\\thinspace",
	"\\thisepage",
	"\\thisepagestyle",
	"\\tie",
	"\\tilde",
	"\\times",
	"\\tiny",
	"\\title",
	"\\titlepage",
	"\\to",
	"\\toaddress",
	"\\topmargin",
	"\\triangle",
	"\\tt",
	"\\twocolumn",
	"\\u",
	"\\underline",
	"\\undervrace",
	"\\unitlength",
	"\\Uparrow",
	"\\uparrow",
	"\\updownarrow",
	"\\Updownarrow",
	"\\uplus",
	"\\upsilon",
	"\\Upsilon",
	"\\usepackage",
	"\\v",
	"\\varepsilon",
	"\\varphi",
	"\\varpi",
	"\\varrho",
	"\\varsigma",
	"\\vartheta",
	"\\vbox",
	"\\vcenter",
	"\\vec",
	"\\vector",
	"\\vee",
	"\\verb",
	"\\verb*",
	"\\verbatim",
	"\\vert",
	"\\Vert",
	"\\vfil",
	"\\vfill",
	"\\vrule",
	"\\vskip",
	"\\vspace",
	"\\vspace*",
	"\\wedge",
	"\\widehat",
	"\\widetilde",
	"\\wp",
	"\\wr",
	"\\wrapfigure",
	"\\xi",
	"\\Xi",
	"\\zeta"//,
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
static const char* ppszKeyWordsTEX2[] = {
	//	環境コマンド
	//Jan. 19, 2001 JEPRO 本当は{}付きでキーワードにしたかったが単語として認識してくれないので止めた
	"abstract",
	"array"
	"center",
	"description",
	"document",
	"displaymath",
	"em",
	"enumerate",
	"eqnarray",
	"eqnarray*",
	"equation",
	"figure",
	"figure*",
	"floatingfigure",
	"flushleft",
	"flushright",
	"itemize",
	"letter",
	"list",
	"math",
	"minipage",
	"multicols",
	"namelist",
	"picture",
	"quotation",
	"quote",
	"sloppypar",
	"subeqnarray",
	"subeqnarray*",
	"subequations",
	"subfigure",
	"tabbing",
	"table",
	"table*",
	"tabular",
	"tabular*",
	"tatepage",
	"thebibliography",
	"theindex",
	"titlepage",
	"trivlist",
	"verbatim",
	"verbatim*",
	"verse",
	"wrapfigure",
	//	スタイルオプション
	"a4",
	"a4j",
	"a5",
	"a5j",
	"Alph",
	"alph",
	"annote",
	"arabic",
	"b4",
	"b4j",
	"b5",
	"b5j",
	"bezier",
	"booktitle",
	"boxedminipage",
	"boxit",
//		"bp",
//		"cm",
	"dbltopnumber",
//		"dd",
	"eclepsf",
	"eepic",
	"enumi",
	"enumii",
	"enumiii",
	"enumiv",
	"epic",
	"epsbox",
	"epsf",
	"fancybox",
	"fancyheadings",
	"fleqn",
	"footnote",
	"howpublished",
	"jabbrv",
	"jalpha",
//		"article",
	"jarticle",
	"jsarticle",
//		"book",
	"jbook",
	"jsbook",
//		"letter",
	"jletter",
//		"plain",
	"jplain",
//		"report",
	"jreport",
	"jtwocolumn",
	"junsrt",
	"leqno",
	"makeidx",
	"markboth",
	"markright",
//		"mm",
	"multicol",
	"myheadings",
	"openbib",
//		"pc",
//		"pt",
	"secnumdepth",
//		"sp",
	"titlepage",
	"tjarticle",
	"topnumber",
	"totalnumber",
	"twocolumn",
	"twoside",
	"yomi"//,
//		"zh",
//		"zw"
};

static const char* ppszKeyWordsPERL[] = {
	//Jul. 10, 2001 JEPRO	変数を第２強調キーワードとして分離した
	"break",
	"continue",
	"do",
	"elsif",
	"else",
	"for",
	"foreach",
	"goto",
	"if",
	"last",
	"next",
	"return",
	"sub",
	"undef",
	"unless",
	"until",
	"while",
	"abs",
	"accept",
	"alarm",
	"atan2",
	"bind",
	"binmode",
	"bless",
	"caller",
	"chdir",
	"chmod",
	"chomp",
	"chop",
	"chown",
	"chr",
	"chroot",
	"close",
	"closedir",
	"connect",
	"continue",
	"cos",
	"crypt",
	"dbmclose",
	"dbmopen",
	"defined",
	"delete",
	"die",
	"do",
	"dump",
	"each",
	"eof",
	"eval",
	"exec",
	"exists",
	"exit",
	"exp",
	"fcntl",
	"fileno",
	"flock",
	"fork",
	"format",
	"formline",
	"getc",
	"getlogin",
	"getpeername",
	"getpgrp",
	"getppid",
	"getpriority",
	"getpwnam",
	"getgrnam",
	"gethostbyname",
	"getnetbyname",
	"getprotobyname",
	"getpwuid",
	"getgrgid",
	"getservbyname",
	"gethostbyaddr",
	"getnetbyaddr",
	"getprotobynumber",
	"getservbyport",
	"getpwent",
	"getgrent",
	"gethostent",
	"getnetent",
	"getprotoent",
	"getservent",
	"setpwent",
	"setgrent",
	"sethostent",
	"setnetent",
	"setprotoent",
	"setservent",
	"endpwent",
	"endgrent",
	"endhostent",
	"endnetent",
	"endprotoent",
	"endservent",
	"getsockname",
	"getsockopt",
	"glob",
	"gmtime",
	"goto",
	"grep",
	"hex",
	"import",
	"index",
	"int",
	"ioctl",
	"join",
	"keys",
	"kill",
	"last",
	"lc",
	"lcfirst",
	"length",
	"link",
	"listen",
	"local",
	"localtime",
	"log",
	"lstat",
//			"//m",
	"map",
	"mkdir",
	"msgctl",
	"msgget",
	"msgsnd",
	"msgrcv",
	"my",
	"next",
	"no",
	"oct",
	"open",
	"opendir",
	"ord",
	"our",	// 2006.04.20 genta
	"pack",
	"package",
	"pipe",
	"pop",
	"pos",
	"print",
	"printf",
	"prototype",
	"push",
//			"//q",
	"qq",
	"qr",
	"qx",
	"qw",
	"quotemeta",
	"rand",
	"read",
	"readdir",
	"readline",
	"readlink",
	"readpipe",
	"recv",
	"redo",
	"ref",
	"rename",
	"require",
	"reset",
	"return",
	"reverse",
	"rewinddir",
	"rindex",
	"rmdir",
//			"//s",
	"scalar",
	"seek",
	"seekdir",
	"select",
	"semctl",
	"semget",
	"semop",
	"send",
	"setpgrp",
	"setpriority",
	"setsockopt",
	"shift",
	"shmctl",
	"shmget",
	"shmread",
	"shmwrite",
	"shutdown",
	"sin",
	"sleep",
	"socket",
	"socketpair",
	"sort",
	"splice",
	"split",
	"sprintf",
	"sqrt",
	"srand",
	"stat",
	"study",
	"sub",
	"substr",
	"symlink",
	"syscall",
	"sysopen",
	"sysread",
	"sysseek",
	"system",
	"syswrite",
	"tell",
	"telldir",
	"tie",
	"tied",
	"time",
	"times",
	"tr",
	"truncate",
	"uc",
	"ucfirst",
	"umask",
	"undef",
	"unlink",
	"unpack",
	"untie",
	"unshift",
	"use",
	"utime",
	"values",
	"vec",
	"wait",
	"waitpid",
	"wantarray",
	"warn",
	"write"
};

//Jul. 10, 2001 JEPRO	変数を第２強調キーワードとして分離した
// 2008/05/05 novice 重複文字列削除
static const char* ppszKeyWordsPERL2[] = {
	"$ARGV",
	"$_",
	"$1",
	"$2",
	"$3",
	"$4",
	"$5",
	"$6",
	"$7",
	"$8",
	"$9",
	"$0",
	"$MATCH",
	"$&",
	"$PREMATCH",
	"$`",
	"$POSTMATCH",
	"$'",
	"$LAST_PAREN_MATCH",
	"$+",
	"$MULTILINE_MATCHING",
	"$*",
	"$INPUT_LINE_NUMBER",
	"$NR",
	"$.",
	"$INPUT_RECORD_SEPARATOR",
	"$RS",
	"$/",
	"$OUTPUT_AUTOFLUSH",
	"$|",
	"$OUTPUT_FIELD_SEPARATOR",
	"$OFS",
	"$,",
	"$OUTPUT_RECORD_SEPARATOR",
	"$ORS",
	"$\\",
	"$LIST_SEPARATOR",
	"$\"",
	"$SUBSCRIPT_SEPARATOR",
	"$SUBSEP",
	"$;",
	"$OFMT",
	"$#",
	"$FORMAT_PAGE_NUMBER",
	"$%",
	"$FORMAT_LINES_PER_PAGE",
	"$=",
	"$FORMAT_LINES_LEFT",
	"$-",
	"$FORMAT_NAME",
	"$~",
	"$FORMAT_TOP_NAME",
	"$^",
	"$FORMAT_LINE_BREAK_CHARACTERS",
	"$:",
	"$FORMAT_FORMFEED",
	"$^L",
	"$ACCUMULATOR",
	"$^A",
	"$CHILD_ERROR",
	"$?",
	"$OS_ERROR",
	"$ERRNO",
	"$!",
	"$EVAL_ERROR",
	"$@",
	"$PROCESS_ID",
	"$PID",
	"$$",
	"$REAL_USER_ID",
	"$UID",
	"$<",
	"$EFFECTIVE_USER_ID",
	"$EUID",
	"$>",
	"$REAL_GROUP_ID",
	"$GID",
	"$(",
	"$EFFECTIVE_GROUP_ID",
	"$EGID",
	"$)",
	"$PROGRAM_NAME",
	"$[",
	"$PERL_VERSION",
	"$]",
	"$DEBUGGING",
	"$^D",
	"$SYSTEM_FD_MAX",
	"$^F",
	"$INPLACE_EDIT",
	"$^I",
	"$PERLDB",
	"$^P",
	"$BASETIME",
	"$^T",
	"$WARNING",
	"$^W",
	"$EXECUTABLE_NAME",
	"$^X",
	"$ENV",
	"$SIG"
};

//Jul. 10, 2001 JEPRO 追加
static const char* ppszKeyWordsVB[] = {
	"And",
	"As",
	"Attribute",
	"Begin",
	"BeginProperty",
	"Boolean",
	"ByVal",
	"Byte",
	"Call",
	"Case",
	"Const",
	"Currency",
	"Date",
	"Declare",
	"Dim",
	"Do",
	"Double",
	"Each",
	"Else",
	"ElseIf",
	"Empty",
	"End",
	"EndProperty",
	"Error",
	"Eqv",
	"Exit",
	"False",
	"For",
	"Friend",
	"Function",
	"Get",
	"GoTo",
	"If",
	"Imp",
	"Integer",
	"Is",
	"Let",
	"Like",
	"Long",
	"Loop",
	"Me",
	"Mod",
	"New",
	"Next",
	"Not",
	"Null",
	"Object",
	"On",
	"Option",
	"Or",
	"Private",
	"Property",
	"Public",
	"RSet",
	"ReDim",
	"Rem",
	"Resume",
	"Select",
	"Set",
	"Single",
	"Static",
	"Step",
	"Stop",
	"String",
	"Sub",
	"Then",
	"To",
	"True",
	"Type",
	"Wend",
	"While",
	"With",
	"Xor",
	"#If",
	"#Else",
	"#End",
	"#Const",
	"AddressOf",
	"Alias",
	"Append",
	"Array",
	"ByRef",
	"Explicit",
	"Global",
	"In",
	"Lib",
	"Nothing",
	"Optional",
	"Output",
	"Terminate",
	"Until",
	//=========================================================
	// 以下はVB.NET(VB7)での廃止が決定しているキーワードです
	//=========================================================
	"DefBool",
	"DefByte",
	"DefCur",
	"DefDate",
	"DefDbl",
	"DefInt",
	"DefLng",
	"DefObj",
	"DefSng",
	"DefStr",
	"DefVar",
	"LSet",
	"GoSub",
	"Return",
	"Variant",
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
static const char* ppszKeyWordsVB2[] = {
	"AppActivate",
	"Beep",
	"BeginTrans",
	"ChDir",
	"ChDrive",
	"Close",
	"CommitTrans",
	"CompactDatabase",
	"Date",
	"DeleteSetting",
	"Erase",
	"FileCopy",
	"FreeLocks",
	"Input",
	"Kill",
	"Load",
	"Lock",
	"Mid",
	"MidB",
	"MkDir",
	"Name",
	"Open",
	"Print",
	"Put",
	"Randomize",
	"RegisterDatabase",
	"RepairDatabase",
	"Reset",
	"RmDir",
	"Rollback",
	"SavePicture",
	"SaveSetting",
	"Seek",
	"SendKeys",
	"SetAttr",
	"SetDataAccessOption",
	"SetDefaultWorkspace",
	"Time",
	"Unload",
	"Unlock",
	"Width",
	"Write",
	"Array",
	"Asc",
	"AscB",
	"Atn",
	"CBool",
	"CByte",
	"CCur",
	"CDate",
	"CDbl",
	"CInt",
	"CLng",
	"CSng",
	"CStr",
	"CVErr",
	"CVar",
	"Choose",
	"Chr",
	"ChrB",
	"Command",
	"Cos",
	"CreateDatabase",
	"CreateObject",
	"CurDir",
	"DDB",
	"Date",
	"DateAdd",
	"DateDiff",
	"DatePart",
	"DateSerial",
	"DateValue",
	"Day",
	"Dir",
	"DoEvents",
	"EOF",
	"Environ",
	"Error",
	"Exp",
	"FV",
	"FileAttr",
	"FileDateTime",
	"FileLen",
	"Fix",
	"Format",
	"FreeFile",
	"GetAllSettings",
	"GetAttr",
	"GetObject",
	"GetSetting",
	"Hex",
	"Hour",
	"IIf",
	"IMEStatus",
	"IPmt",
	"IRR",
	"InStr",
	"Input",
	"Int",
	"IsArray",
	"IsDate",
	"IsEmpty",
	"IsError",
	"IsMissing",
	"IsNull",
	"IsNumeric",
	"IsObject",
	"LBound",
	"LCase",
	"LOF",
	"LTrim",
	"Left",
	"LeftB",
	"Len",
	"LoadPicture",
	"Loc",
	"Log",
	"MIRR",
	"Mid",
	"MidB",
	"Minute",
	"Month",
	"MsgBox",
	"NPV",
	"NPer",
	"Now",
	"Oct",
	"OpenDatabase",
	"PPmt",
	"PV",
	"Partition",
	"Pmt",
	"QBColor",
	"RGB",
	"RTrim",
	"Rate",
	"ReadProperty",
	"Right",
	"RightB",
	"Rnd",
	"SLN",
	"SYD",
	"Second",
	"Seek",
	"Sgn",
	"Shell",
	"Sin",
	"Space",
	"Spc",
	"Sqr",
	"Str",
	"StrComp",
	"StrConv",
	"Switch",
	"Tab",
	"Tan",
	"Time",
	"TimeSerial",
	"TimeValue",
	"Timer",
	"Trim",
	"TypeName",
	"UBound",
	"UCase",
	"Val",
	"VarType",
	"Weekday",
	"Year",
	"Hide",
	"Line",
	"Refresh",
	"Show",
	//=========================================================
	// 以下はVB.NET(VB7)での廃止が決定しているキーワードです
	//=========================================================
	//$付き関数各種
	"Dir$",
	"LCase$",
	"Left$",
	"LeftB$",
	"Mid$",
	"MidB$",
	"RightB$",
	"Right$",
	"Space$",
	"Str$",
	"String$",
	"Trim$",
	"UCase$",
	//VB5,6の隠し関数
	"VarPtr",
	"StrPtr",
	"ObjPtr",
	"VarPrtArray",
	"VarPtrStringArray"
};

//Jul. 10, 2001 JEPRO 追加
static const char* ppszKeyWordsRTF[] = {
	"\\ansi",
	"\\b",
	"\\bin",
	"\\box",
	"\\brdrb",
	"\\brdrbar",
	"\\brdrdb",
	"\\brdrdot",
	"\\brdrl",
	"\\brdrr",
	"\\brdrs",
	"\\brdrsh",
	"\\brdrt",
	"\\brdrth",
	"\\cell",
	"\\cellx",
	"\\cf",
	"\\chftn",
	"\\clmgf",
	"\\clmrg",
	"\\colortbl",
	"\\deff",
	"\\f",
	"\\fi",
	"\\field",
	"\\fldrslt",
	"\\fonttbl",
	"\\footnote",
	"\\fs",
	"\\i"
	"\\intbl",
	"\\keep",
	"\\keepn",
	"\\li",
	"\\line",
	"\\mac",
	"\\page",
	"\\par",
	"\\pard",
	"\\pc",
	"\\pich",
	"\\pichgoal",
	"\\picscalex",
	"\\picscaley",
	"\\pict",
	"\\picw",
	"\\picwgoal",
	"\\plain",
	"\\qc",
	"\\ql",
	"\\qr",
	"\\ri",
	"\\row",
	"\\rtf",
	"\\sa",
	"\\sb",
	"\\scaps",
	"\\sect",
	"\\sl",
	"\\strike",
	"\\tab",
	"\\tqc",
	"\\tqr",
	"\\trgaph",
	"\\trleft",
	"\\trowd",
	"\\trqc",
	"\\trql",
	"\\tx",
	"\\ul",
	"\\uldb",
	"\\v",
	"\\wbitmap",
	"\\wbmbitspixel",
	"\\wbmplanes",
	"\\wbmwidthbytes",
	"\\wmetafile",
	"bmc",
	"bml",
	"bmr",
	"emc",
	"eml",
	"emr"
};

/*!	@brief 共有メモリ初期化/強調キーワード

	強調キーワード関連の初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離．
		キーワード定義を関数の外に出し，登録をマクロ化して簡潔に．
*/
void CShareData::InitKeyword(DLLSHAREDATA* pShareData)
{
	/* 強調キーワードのテストデータ */
	pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = 0;

	int nSetCount = -1;

#define PopulateKeyword(name,case_sensitive,ary) \
	pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWordSet( (name), (case_sensitive) );	\
	pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordArr( ++nSetCount, _countof(ary), (ary) );
	
	PopulateKeyword( "C/C++", true, ppszKeyWordsCPP );			/* セット 0の追加 */
	PopulateKeyword( "HTML", false, ppszKeyWordsHTML );			/* セット 1の追加 */
	PopulateKeyword( "PL/SQL", false, ppszKeyWordsPLSQL );		/* セット 2の追加 */
	PopulateKeyword( "COBOL", true ,ppszKeyWordsCOBOL );		/* セット 3の追加 */
	PopulateKeyword( "Java", true, ppszKeyWordsJAVA );			/* セット 4の追加 */
	PopulateKeyword( "CORBA IDL", true, ppszKeyWordsCORBA_IDL );/* セット 5の追加 */
	PopulateKeyword( "AWK", true, ppszKeyWordsAWK );			/* セット 6の追加 */
	PopulateKeyword( "MS-DOS batch", false, ppszKeyWordsBAT );	/* セット 7の追加 */	//Oct. 31, 2000 JEPRO 'バッチファイル'→'batch' に短縮
	PopulateKeyword( "Pascal", false, ppszKeyWordsPASCAL );		/* セット 8の追加 */	//Nov. 5, 2000 JEPRO 大・小文字の区別を'しない'に変更
	PopulateKeyword( "TeX", true, ppszKeyWordsTEX );			/* セット 9の追加 */	//Sept. 2, 2000 jepro Tex →TeX に修正 Bool値は大・小文字の区別
	PopulateKeyword( "TeX2", true, ppszKeyWordsTEX2 );			/* セット10の追加 */	//Jan. 19, 2001 JEPRO 追加
	PopulateKeyword( "Perl", true, ppszKeyWordsPERL );			/* セット11の追加 */
	PopulateKeyword( "Perl2", true, ppszKeyWordsPERL2 );		/* セット12の追加 */	//Jul. 10, 2001 JEPRO Perlから変数を分離・独立
	PopulateKeyword( "Visual Basic", false, ppszKeyWordsVB );	/* セット13の追加 */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( "Visual Basic2", false, ppszKeyWordsVB2 );	/* セット14の追加 */	//Jul. 10, 2001 JEPRO
	PopulateKeyword( "リッチテキスト", true, ppszKeyWordsRTF );	/* セット15の追加 */	//Jul. 10, 2001 JEPRO

#undef PopulateKeyword
}

//	Sep. 14, 2000 JEPRO
//	Shift+F1 に「コマンド一覧」, Alt+F1 に「ヘルプ目次」, Shift+Alt+F1 に「キーワード検索」を追加	//Nov. 25, 2000 JEPRO 殺していたのを修正・復活
//Dec. 25, 2000 JEPRO Shift+Ctrl+F1 に「バージョン情報」を追加
// 2001.12.03 hor F2にブックマーク関連を割当
//Sept. 21, 2000 JEPRO	Ctrl+F3 に「検索マークのクリア」を追加
//Aug. 12, 2002 ai	Ctrl+Shift+F3 に「検索開始位置へ戻る」を追加
//Oct. 7, 2000 JEPRO	Alt+F4 に「ウィンドウを閉じる」, Shift+Alt+F4 に「すべてのウィンドウを閉じる」を追加
//	Ctrl+F4に割り当てられていた「縦横に分割」を「閉じて(無題)」に変更し Shift+Ctrl+F4 に「閉じて開く」を追加
//Jan. 14, 2001 Ctrl+Alt+F4 に「テキストエディタの全終了」を追加
//Jun. 2001「サクラエディタの全終了」に改称
//2006.10.21 ryoji Alt+F4 には何も割り当てない（デフォルトのシステムコマンド「閉じる」が実行されるように）
//2007.02.13 ryoji Shift+Ctrl+F4をF_WIN_CLOSEALLからF_EXITALLEDITORSに変更
//2007.02.22 ryoji Ctrl+F4 への割り当てを削除（デフォルトのコマンドを実行）
//	Sep. 20, 2000 JEPRO Ctrl+F5 に「外部コマンド実行」を追加  なおマクロ名はCMMAND からCOMMAND に変更済み
//Oct. 28, 2000 F5 は「再描画」に変更	//Jan. 14, 2001 Alt+F5 に「uudecodeして保存」, Ctrl+ Alt+F5 に「TAB→空白」を追加
//	May 28, 2001 genta	S-C-A-F5にSPACE-to-TABを追加
//Jan. 14, 2001 JEPRO	Ctrl+F6 に「小文字」, Alt+F6 に「Base64デコードして保存」を追加
// 2007.10.05 nasukoji	トリプルクリック・クアドラプルクリック対応
//Jan. 14, 2001 JEPRO	Ctrl+F7 に「大文字」, Alt+F7 に「UTF-7→SJISコード変換」, Shift+Alt+F7 に「SJIS→UTF-7コード変換」, Ctrl+Alt+F7 に「UTF-7で開き直す」を追加
//Nov. 9, 2000 JEPRO	Shift+F8 に「CRLF改行でコピー」を追加
//Jan. 14, 2001 JEPRO	Ctrl+F8 に「全角→半角」, Alt+F8 に「UTF-8→SJISコード変換」, Shift+Alt+F8 に「SJIS→UTF-8コード変換」, Ctrl+Alt+F8 に「UTF-8で開き直す」を追加
//Jan. 14, 2001 JEPRO	Ctrl+F9 に「半角＋全ひら→全角・カタカナ」, Alt+F9 に「Unicode→SJISコード変換」, Ctrl+Alt+F9 に「Unicodeで開き直す」を追加
//Oct. 28, 2000 JEPRO F10 に「SQL*Plusで実行」を追加(F5からの移動)
//Jan. 14, 2001 JEPRO	Ctrl+F10 に「半角＋全カタ→全角・ひらがな」, Alt+F10 に「EUC→SJISコード変換」, Shift+Alt+F10 に「SJIS→EUCコード変換」, Ctrl+Alt+F10 に「EUCで開き直す」を追加
//Jan. 14, 2001 JEPRO	Shift+F11 に「SQL*Plusをアクティブ表示」, Ctrl+F11 に「半角カタカナ→全角カタカナ」, Alt+F11 に「E-Mail(JIS→SJIS)コード変換」, Shift+Alt+F11 に「SJIS→JISコード変換」, Ctrl+Alt+F11 に「JISで開き直す」を追加
//Jan. 14, 2001 JEPRO	Ctrl+F12 に「半角カタカナ→全角ひらがな」, Alt+F12 に「自動判別→SJISコード変換」, Ctrl+Alt+F11 に「SJISで開き直す」を追加
//Sept. 1, 2000 JEPRO	Alt+Enter に「ファイルのプロパティ」を追加	//Oct. 15, 2000 JEPRO Ctrl+Enter に「ファイル内容比較」を追加
//Oct. 7, 2000 JEPRO 長いので名称を簡略形に変更(BackSpace→BkSp)
//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(Insert→Ins)
//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(Delete→Del)
//Jun. 26, 2001 JEPRO	Shift+Del に「切り取り」を追加
//Oct. 7, 2000 JEPRO	Shift+Ctrl+Alt+↑に「縦方向に最大化」を追加
//Jun. 27, 2001 JEPRO
//	Ctrl+↑に割り当てられていた「カーソル上移動(２行ごと)」を「テキストを１行下へスクロール」に変更
//2001.02.10 by MIK Shift+Ctrl+Alt+→に「横方向に最大化」を追加
//Sept. 14, 2000 JEPRO
//	Ctrl+↓に割り当てられていた「右クリックメニュー」を「カーソル下移動(２行ごと)」に変更
//	それに付随してさらに「右クリックメニュー」をCtrl＋Alt＋↓に変更
//Jun. 27, 2001 JEPRO
//	Ctrl+↓に割り当てられていた「カーソル下移動(２行ごと)」を「テキストを１行上へスクロール」に変更
//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn にそれぞれ「１ページダウン」, 「(選択)１ページダウン」を追加
//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に交換(RollUp→PgDn) //Oct. 10, 2000 JEPRO 名称変更
//2001.12.03 hor 1Page/HalfPage 入替え
//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn にそれぞれ「１ページアップ」, 「(選択)１ページアップ」を追加
//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に交換(RollDown→PgUp) //Oct. 10, 2000 JEPRO 名称変更
//2001.12.03 hor 1Page/HalfPage 入替え
//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(SpaceBar→Space)
//Oct. 7, 2000 JEPRO	Ctrl+0 を「タイプ別設定一覧」→「未定義」に変更
//Jan. 13, 2001 JEPRO	Alt+0 に「カスタムメニュー10」, Shift+Alt+0 に「カスタムメニュー20」を追加
//Oct. 7, 2000 JEPRO	Ctrl+1 を「タイプ別設定」→「ツールバーの表示」に変更
//Jan. 13, 2001 JEPRO	Alt+1 に「カスタムメニュー1」, Shift+Alt+1 に「カスタムメニュー11」を追加
//Jan. 19, 2001 JEPRO	Shift+Ctrl+1 に「カスタムメニュー21」を追加
//Oct. 7, 2000 JEPRO	Ctrl+2 を「共通設定」→「ファンクションキーの表示」に変更
//Jan. 13, 2001 JEPRO	Alt+2 を「アウトプット」→「カスタムメニュー2」に変更し「アウトプット」は Alt+O に移動, Shift+Alt+2 に「カスタムメニュー12」を追加
//Jan. 19, 2001 JEPRO	Shift+Ctrl+2 に「カスタムメニュー22」を追加
//Oct. 7, 2000 JEPRO	Ctrl+3 を「フォント設定」→「ステータスバーの表示」に変更
//Jan. 13, 2001 JEPRO	Alt+3 に「カスタムメニュー3」, Shift+Alt+3 に「カスタムメニュー13」を追加
//Jan. 19, 2001 JEPRO	Shift+Ctrl+3 に「カスタムメニュー23」を追加
//Oct. 7, 2000 JEPRO	Ctrl+4 を「ツールバーの表示」→「タイプ別設定一覧」に変更
//Jan. 13, 2001 JEPRO	Alt+4 に「カスタムメニュー4」, Shift+Alt+4 に「カスタムメニュー14」を追加
//Jan. 19, 2001 JEPRO	Shift+Ctrl+4 に「カスタムメニュー24」を追加
//Oct. 7, 2000 JEPRO	Ctrl+5 を「ファンクションキーの表示」→「タイプ別設定」に変更
//Jan. 13, 2001 JEPRO	Alt+5 に「カスタムメニュー5」, Shift+Alt+5 に「カスタムメニュー15」を追加
//Oct. 7, 2000 JEPRO	Ctrl+6 を「ステータスバーの表示」→「共通設定」に変更
//Jan. 13, 2001 JEPRO	Alt+6 に「カスタムメニュー6」, Shift+Alt+6 に「カスタムメニュー16」を追加
//Oct. 7, 2000 JEPRO	Ctrl+7 に「フォント設定」を追加
//Jan. 13, 2001 JEPRO	Alt+7 に「カスタムメニュー7」, Shift+Alt+7 に「カスタムメニュー17」を追加
//Jan. 13, 2001 JEPRO	Alt+8 に「カスタムメニュー8」, Shift+Alt+8 に「カスタムメニュー18」を追加
//Jan. 13, 2001 JEPRO	Alt+9 に「カスタムメニュー9」, Shift+Alt+9 に「カスタムメニュー19」を追加
//2001.12.06 hor Alt+A を「SORT_ASC」に割当
//Jan. 13, 2001 JEPRO	Ctrl+B に「ブラウズ」を追加
//Jan. 16, 2001 JEPRO	SHift+Ctrl+C に「.hと同名の.c(なければ.cpp)を開く」を追加
//Feb. 07, 2001 JEPRO	SHift+Ctrl+C を「.hと同名の.c(なければ.cpp)を開く」→「同名のC/C++ヘッダ(ソース)を開く」に変更
//Jan. 16, 2001 JEPRO	Ctrl+D に「単語切り取り」, Shift+Ctrl+D に「単語削除」を追加
//2001.12.06 hor Alt+D を「SORT_DESC」に割当
//Oct. 7, 2000 JEPRO	Ctrl+Alt+E に「重ねて表示」を追加
//Jan. 16, 2001	JEPRO	Ctrl+E に「行切り取り(折り返し単位)」, Shift+Ctrl+E に「行削除(折り返し単位)」を追加
//Oct. 07, 2000 JEPRO	Ctrl+Alt+H に「上下に並べて表示」を追加
//Jan. 16, 2001 JEPRO	Ctrl+H を「カーソル前を削除」→「カーソル行をウィンドウ中央へ」に変更し	Shift+Ctrl+H に「.cまたは.cppと同名の.hを開く」を追加
//Feb. 07, 2001 JEPRO	SHift+Ctrl+H を「.cまたは.cppと同名の.hを開く」→「同名のC/C++ヘッダ(ソース)を開く」に変更
//Jan. 21, 2001	JEPRO	Ctrl+I に「行の二重化」を追加
//Jan. 16, 2001	JEPRO	Ctrl+K に「行末まで切り取り(改行単位)」, Shift+Ctrl+E に「行末まで削除(改行単位)」を追加
//Jan. 14, 2001 JEPRO	Ctrl+Alt+L に「小文字」, Shift+Ctrl+Alt+L に「大文字」を追加
//Jan. 16, 2001 Ctrl+L を「カーソル行をウィンドウ中央へ」→「キーマクロの読み込み」に変更し「カーソル行をウィンドウ中央へ」は Ctrl+H に移動
//2001.12.03 hor Alt+L を「LTRIM」に割当
//Jan. 16, 2001 JEPRO	Ctrl+M に「キーマクロの保存」を追加
//2001.12.06 hor Alt+M を「MERGE」に割当
//Oct. 20, 2000 JEPRO	Alt+N に「移動履歴: 次へ」を追加
//Jan. 13, 2001 JEPRO	Alt+O に「アウトプット」を追加
//Oct. 7, 2000 JEPRO	Ctrl+P に「印刷」, Shift+Ctrl+P に「印刷プレビュー」, Ctrl+Alt+P に「ページ設定」を追加
//Oct. 20, 2000 JEPRO	Alt+P に「移動履歴: 前へ」を追加
//Jan. 24, 2001	JEPRO	Ctrl+Q に「キー割り当て一覧をコピー」を追加
//2001.12.03 hor Alt+R を「RTRIM」に割当
//Oct. 7, 2000 JEPRO	Shift+Ctrl+S に「名前を付けて保存」を追加
//Oct. 7, 2000 JEPRO	Ctrl+Alt+T に「左右に並べて表示」を追加
//Jan. 21, 2001	JEPRO	Ctrl+T に「タグジャンプ」, Shift+Ctrl+T に「タグジャンプバック」を追加
//Oct. 7, 2000 JEPRO	Ctrl+Alt+U に「現在のウィンドウ幅で折り返し」を追加
//Jan. 16, 2001	JEPRO	Ctrl+U に「行頭まで切り取り(改行単位)」, Shift+Ctrl+U に「行頭まで削除(改行単位)」を追加
//Jan. 13, 2001 JEPRO	Alt+X を「カスタムメニュー1」→「未定義」に変更し「カスタムメニュー1」は Alt+1 に移動
//Oct. 7, 2000 JEPRO	Shift+Ctrl+- に「上下に分割」を追加
// 2002.02.08 hor Ctrl+-にファイル名をコピーを追加
//Oct. 7, 2000 JEPRO	Shift+Ctrl+\ に「左右に分割」を追加
//Sept. 20, 2000 JEPRO	Ctrl+@ に「ファイル内容比較」を追加  //Oct. 15, 2000 JEPRO「選択範囲内全行コピー」に変更
//	Aug. 16, 2000 genta
//	反対向きの括弧にも括弧検索を追加
//Oct. 7, 2000 JEPRO	Shift+Ctrl+; に「縦横に分割」を追加	//Jan. 16, 2001	Alt+; に「日付挿入」を追加
//Sept. 14, 2000 JEPRO	Ctrl+: に「選択範囲内全行行番号付きコピー」を追加	//Jan. 16, 2001	Alt+: に「時刻挿入」を追加
//Sept. 14, 2000 JEPRO	Ctrl+. に「選択範囲内全行引用符付きコピー」を追加
//	Nov. 15, 2000 genta PC/ATキーボードに合わせてキーコードを変更
//	PC98救済のため，従来のキーコードに対応する項目を追加．
//Oct. 7, 2000 JEPRO	長くて表示しきれない所がでてきてしまうのでアプリケーションキー→アプリキーに短縮
//2008.05.03 kobake 可読性が著しく低下していたので、書式を整理。
// 2008.05.30 nasukoji	Ctrl+Alt+S に「指定桁で折り返す」を追加
// 2008.05.30 nasukoji	Ctrl+Alt+W に「右端で折り返す」を追加
// 2008.05.30 nasukoji	Ctrl+Alt+X に「折り返さない」を追加

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
		short			nFuncCode_0;	//!<                      Key
		short			nFuncCode_1;	//!< Shift +              Key
		short			nFuncCode_2;	//!<         Ctrl +       Key
		short			nFuncCode_3;	//!< Shift + Ctrl +       Key
		short			nFuncCode_4;	//!<                Alt + Key
		short			nFuncCode_5;	//!< Shift +        Alt + Key
		short			nFuncCode_6;	//!<         Ctrl + Alt + Key
		short			nFuncCode_7;	//!< Shift + Ctrl + Alt + Key
	};
#define _SQL_RUN	F_PLSQL_COMPILE_ON_SQLPLUS
#define _COPYWITHLINENUM	F_COPYLINESWITHLINENUMBER
	static KEYDATAINIT	KeyDataInit[] = {
	//Sept. 1, 2000 Jepro note: key binding
	//Feb. 17, 2001 jepro note 2: 順番は2進で下位3ビット[Alt][Ctrl][Shift]の組合せの順(それに2を加えた値)
	//		0,		1,		 2(000), 3(001),4(010),	5(011),		6(100),	7(101),		8(110),		9(111)

		/* マウスボタン */
		//keycode, keyname, なし, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ 0, _T("ダブルクリック"), F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD }, //Feb. 19, 2001 JEPRO Altと右クリックの組合せは効かないので右クリックメニューのキー割り当てをはずした
		{ 0, _T("右クリック"), F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_0, F_0, F_0, F_0 },
		{ 0, _T("中クリック"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 }, // novice 2004/10/11 マウス中ボタン対応
		{ 0, _T("左サイドクリック"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 }, // novice 2004/10/10 マウスサイドボタン対応
		{ 0, _T("右サイドクリック"), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ 0, _T("トリプルクリック"), F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE, F_SELECTLINE },
		{ 0, _T("クアドラプルクリック"), F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL, F_SELECTALL },

		/* ファンクションキー */
		//keycode, keyname, なし, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ VK_F1,_T("F1"), F_EXTHTMLHELP, F_MENU_ALLFUNC, F_EXTHELP1, F_ABOUT, F_HELP_CONTENTS, F_HELP_SEARCH, F_0, F_0 },
		{ VK_F2,_T("F2"), F_BOOKMARK_NEXT, F_BOOKMARK_PREV, F_BOOKMARK_SET, F_BOOKMARK_RESET, F_BOOKMARK_VIEW, F_0, F_0, F_0 },
		{ VK_F3,_T("F3"), F_SEARCH_NEXT, F_SEARCH_PREV, F_SEARCH_CLEARMARK, F_JUMP_SRCHSTARTPOS, F_0, F_0, F_0, F_0 },
		{ VK_F4,_T("F4"), F_SPLIT_V, F_SPLIT_H, F_0, F_FILECLOSE_OPEN, F_0, F_EXITALLEDITORS, F_EXITALL, F_0 },
		{ VK_F5,_T("F5"), F_REDRAW, F_0, F_EXECMD_DIALOG, F_0, F_UUDECODE, F_0, F_TABTOSPACE, F_SPACETOTAB },
		{ VK_F6,_T("F6"), F_BEGIN_SEL, F_BEGIN_BOX, F_TOLOWER, F_0, F_BASE64DECODE, F_0, F_0, F_0 },
		{ VK_F7,_T("F7"), F_CUT, F_0, F_TOUPPER, F_0, F_CODECNV_UTF72SJIS, F_CODECNV_SJIS2UTF7, F_FILE_REOPEN_UTF7, F_0 },
		{ VK_F8,_T("F8"), F_COPY, F_COPY_CRLF, F_TOHANKAKU, F_0, F_CODECNV_UTF82SJIS, F_CODECNV_SJIS2UTF8, F_FILE_REOPEN_UTF8, F_0 },
		{ VK_F9,_T("F9"), F_PASTE, F_PASTEBOX, F_TOZENKAKUKATA, F_0, F_CODECNV_UNICODE2SJIS, F_0, F_FILE_REOPEN_UNICODE, F_0 },
		{ VK_F10,_T("F10"), _SQL_RUN, F_DUPLICATELINE, F_TOZENKAKUHIRA, F_0, F_CODECNV_EUC2SJIS, F_CODECNV_SJIS2EUC, F_FILE_REOPEN_EUC, F_0 },
		{ VK_F11,_T("F11"), F_OUTLINE, F_ACTIVATE_SQLPLUS, F_HANKATATOZENKATA, F_0, F_CODECNV_EMAIL, F_CODECNV_SJIS2JIS, F_FILE_REOPEN_JIS, F_0 },
		{ VK_F12,_T("F12"), F_TAGJUMP, F_TAGJUMPBACK, F_HANKATATOZENHIRA, F_0, F_CODECNV_AUTO2SJIS, F_0, F_FILE_REOPEN_SJIS, F_0 },
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
		//keycode, keyname, なし, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ VK_TAB,_T("Tab"), F_INDENT_TAB, F_UNINDENT_TAB, F_NEXTWINDOW, F_PREVWINDOW, F_0, F_0, F_0, F_0 },
			{ VK_RETURN,_T("Enter"), F_0, F_0, F_COMPARE, F_0, F_PROPERTY_FILE, F_0, F_0, F_0 },
		{ VK_ESCAPE,_T("Esc"), F_CANCEL_MODE, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ VK_BACK,_T("BkSp"), F_DELETE_BACK, F_0, F_WordDeleteToStart, F_0, F_0, F_0, F_0, F_0 },
		{ VK_INSERT,_T("Ins"), F_CHGMOD_INS, F_PASTE, F_COPY, F_0, F_0, F_0, F_0, F_0 },
		{ VK_DELETE,_T("Del"), F_DELETE, F_CUT, F_WordDeleteToEnd, F_0, F_0, F_0, F_0, F_0 },
		{ VK_HOME,_T("Home"), F_GOLINETOP, F_GOLINETOP_SEL, F_GOFILETOP, F_GOFILETOP_SEL, F_0, F_0, F_0, F_0 },
		{ VK_END,_T("End(Help)"), F_GOLINEEND, F_GOLINEEND_SEL, F_GOFILEEND, F_GOFILEEND_SEL, F_0, F_0, F_0, F_0 },
		{ VK_LEFT,_T("←"), F_LEFT, F_LEFT_SEL, F_WORDLEFT, F_WORDLEFT_SEL, F_BEGIN_BOX, F_0, F_0, F_0 },
		{ VK_UP,_T("↑"), F_UP, F_UP_SEL, F_WndScrollDown, F_UP2_SEL, F_BEGIN_BOX, F_0, F_0, F_MAXIMIZE_V },
		{ VK_RIGHT,_T("→"), F_RIGHT, F_RIGHT_SEL, F_WORDRIGHT, F_WORDRIGHT_SEL, F_BEGIN_BOX, F_0, F_0, F_MAXIMIZE_H },
		{ VK_DOWN,_T("↓"), F_DOWN, F_DOWN_SEL, F_WndScrollUp, F_DOWN2_SEL, F_BEGIN_BOX, F_0, F_MENU_RBUTTON, F_MINIMIZE_ALL },
		{ VK_NEXT,_T("PgDn(RollUp)"), F_1PageDown, F_1PageDown_Sel, F_HalfPageDown, F_HalfPageDown_Sel, F_0, F_0, F_0, F_0 },
		{ VK_PRIOR,_T("PgUp(RollDn)"), F_1PageUp, F_1PageUp_Sel, F_HalfPageUp, F_HalfPageUp_Sel, F_0, F_0, F_0, F_0 },
		{ VK_SPACE,_T("Space"), F_INDENT_SPACE, F_UNINDENT_SPACE, F_HOKAN, F_0, F_0, F_0, F_0, F_0 },

		/* 数字 */
		//keycode, keyname, なし, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ '0', _T("0"), F_0, F_0, F_0, F_0, F_CUSTMENU_10, F_CUSTMENU_20, F_0, F_0 },
		{ '1', _T("1"), F_0, F_0, F_SHOWTOOLBAR, F_CUSTMENU_21, F_CUSTMENU_1, F_CUSTMENU_11, F_0, F_0 },
		{ '2', _T("2"), F_0, F_0, F_SHOWFUNCKEY, F_CUSTMENU_22, F_CUSTMENU_2, F_CUSTMENU_12, F_0, F_0 },
		{ '3', _T("3"), F_0, F_0, F_SHOWSTATUSBAR, F_CUSTMENU_23, F_CUSTMENU_3, F_CUSTMENU_13, F_0, F_0 },
		{ '4', _T("4"), F_0, F_0, F_TYPE_LIST, F_CUSTMENU_24, F_CUSTMENU_4, F_CUSTMENU_14, F_0, F_0 },
		{ '5', _T("5"), F_0, F_0, F_OPTION_TYPE, F_0, F_CUSTMENU_5, F_CUSTMENU_15, F_0, F_0 },
		{ '6', _T("6"), F_0, F_0, F_OPTION, F_0, F_CUSTMENU_6, F_CUSTMENU_16, F_0, F_0 },
		{ '7', _T("7"), F_0, F_0, F_FONT, F_0, F_CUSTMENU_7, F_CUSTMENU_17, F_0, F_0 },
		{ '8', _T("8"), F_0, F_0, F_0, F_0, F_CUSTMENU_8, F_CUSTMENU_18, F_0, F_0 },
		{ '9', _T("9"), F_0, F_0, F_0, F_0, F_CUSTMENU_9, F_CUSTMENU_19, F_0, F_0 },

		/* アルファベット */
		//keycode, keyname, なし, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ 'A', _T("A"), F_0, F_0, F_SELECTALL, F_0, F_SORT_ASC, F_0, F_0, F_0 },
		{ 'B', _T("B"), F_0, F_0, F_BROWSE, F_0, F_0, F_0, F_0, F_0 },
		{ 'C', _T("C"), F_0, F_0, F_COPY, F_OPEN_HfromtoC, F_0, F_0, F_0, F_0 },
		{ 'D', _T("D"), F_0, F_0, F_WordCut, F_WordDelete, F_SORT_DESC, F_0, F_0, F_0 },
		{ 'E', _T("E"), F_0, F_0, F_CUT_LINE, F_DELETE_LINE, F_0, F_0, F_CASCADE, F_0 },
		{ 'F', _T("F"), F_0, F_0, F_SEARCH_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		{ 'G', _T("G"), F_0, F_0, F_GREP_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		{ 'H', _T("H"), F_0, F_0, F_CURLINECENTER, F_OPEN_HfromtoC, F_0, F_0, F_TILE_V, F_0 },
		{ 'I', _T("I"), F_0, F_0, F_DUPLICATELINE, F_0, F_0, F_0, F_0, F_0 },
		{ 'J', _T("J"), F_0, F_0, F_JUMP_DIALOG, F_0, F_0, F_0, F_0, F_0 },
		{ 'K', _T("K"), F_0, F_0, F_LineCutToEnd, F_LineDeleteToEnd, F_0, F_0, F_0, F_0 },
		{ 'L', _T("L"), F_0, F_0, F_LOADKEYMACRO, F_EXECKEYMACRO, F_LTRIM, F_0, F_TOLOWER, F_TOUPPER },
		{ 'M', _T("M"), F_0, F_0, F_SAVEKEYMACRO, F_RECKEYMACRO, F_MERGE, F_0, F_0, F_0 },
		{ 'N', _T("N"), F_0, F_0, F_FILENEW, F_0, F_JUMPHIST_NEXT, F_0, F_0, F_0 },
		{ 'O', _T("O"), F_0, F_0, F_FILEOPEN, F_0, F_0, F_0, F_0, F_0 },
		{ 'P', _T("P"), F_0, F_0, F_PRINT, F_PRINT_PREVIEW, F_JUMPHIST_PREV, F_0, F_PRINT_PAGESETUP, F_0 },
		{ 'Q', _T("Q"), F_0, F_0, F_CREATEKEYBINDLIST, F_0, F_0, F_0, F_0, F_0 },
		{ 'R', _T("R"), F_0, F_0, F_REPLACE_DIALOG, F_0, F_RTRIM, F_0, F_0, F_0 },
		{ 'S', _T("S"), F_0, F_0, F_FILESAVE, F_FILESAVEAS_DIALOG, F_0, F_0, F_TMPWRAPSETTING, F_0 },
		{ 'T', _T("T"), F_0, F_0, F_TAGJUMP, F_TAGJUMPBACK, F_0, F_0, F_TILE_H, F_0 },
		{ 'U', _T("U"), F_0, F_0, F_LineCutToStart, F_LineDeleteToStart, F_0, F_0, F_WRAPWINDOWWIDTH, F_0 },
		{ 'V', _T("V"), F_0, F_0, F_PASTE, F_0, F_0, F_0, F_0, F_0 },
		{ 'W', _T("W"), F_0, F_0, F_SELECTWORD, F_0, F_0, F_0, F_TMPWRAPWINDOW, F_0 },
		{ 'X', _T("X"), F_0, F_0, F_CUT, F_0, F_0, F_0, F_TMPWRAPNOWRAP, F_0 },
		{ 'Y', _T("Y"), F_0, F_0, F_REDO, F_0, F_0, F_0, F_0, F_0 },
		{ 'Z', _T("Z"), F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },

		/* 記号 */
		//keycode, keyname, なし, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		{ 0x00bd, _T("-"), F_0, F_0, F_COPYFNAME, F_SPLIT_V, F_0, F_0, F_0, F_0 },
		{ 0x00de, _T("^(英語')"), F_0, F_0, F_COPYTAG, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00dc, _T("\\"), F_0, F_0, F_COPYPATH, F_SPLIT_H, F_0, F_0, F_0, F_0 },
		{ 0x00c0, _T("@(英語`)"), F_0, F_0, F_COPYLINES, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00db, _T("["), F_0, F_0, F_BRACKETPAIR, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00bb, _T(";"), F_0, F_0, F_0, F_SPLIT_VH, F_INS_DATE, F_0, F_0, F_0 },
		{ 0x00ba, _T(":"), F_0, F_0, _COPYWITHLINENUM, F_0, F_INS_TIME, F_0, F_0, F_0 },
		{ 0x00dd, _T("]"), F_0, F_0, F_BRACKETPAIR, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00bc, _T(","), F_0, F_0, F_0, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00be, _T("."), F_0, F_0, F_COPYLINESASPASSAGE, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00bf, _T("/"), F_0, F_0, F_HOKAN, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00e2, _T("_"), F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },
		{ 0x00df, _T("_(PC-98)"), F_0, F_0, F_UNDO, F_0, F_0, F_0, F_0, F_0 },
		{ VK_APPS, _T("アプリキー"), F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON }
	};
	const int	nKeyDataInitNum = _countof( KeyDataInit );
	//	From Here 2007.11.04 genta バッファオーバーラン防止
	if( nKeyDataInitNum > _countof( pShareData->m_Common.m_sKeyBind.m_pKeyNameArr ) ) {
		PleaseReportToAuthor( NULL, _T("キー設定数に対してDLLSHARE::m_nKeyNameArr[]のサイズが不足しています") );
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
	pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum = nKeyDataInitNum;
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
	char dummy[ _countof(DEFAULT_TOOL_BUTTONS) < MAX_TOOLBAR_BUTTON_ITEMS ? 1:0 ];
	dummy[0]=0;

	memcpy(
		(void*)pShareData->m_Common.m_sToolBar.m_nToolBarButtonIdxArr,
		DEFAULT_TOOL_BUTTONS,
		sizeof(DEFAULT_TOOL_BUTTONS)
	);

	/* ツールバーボタンの数 */
	pShareData->m_Common.m_sToolBar.m_nToolBarButtonNum = _countof(DEFAULT_TOOL_BUTTONS);
	pShareData->m_Common.m_sToolBar.m_bToolBarIsFlat = !IsVisualStyle();			/* フラットツールバーにする／しない */	// 2006.06.23 ryoji ビジュアルスタイルでは初期値をノーマルにする
	
}

/*!	@brief 共有メモリ初期化/タイプ別設定

	タイプ別設定の初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離．
*/
void CShareData::InitTypeConfigs(DLLSHAREDATA* pShareData)
{
//キーワード：デフォルトカラー設定
/************************/
/* タイプ別設定の規定値 */
/************************/
	int nIdx = 0;
	int i;
	pShareData->m_Types[nIdx].m_nTextWrapMethod = WRAP_SETTING_WIDTH;	// テキストの折り返し方法		// 2008.05.30 nasukoji
	pShareData->m_Types[nIdx].m_nMaxLineKetas = MAXLINEKETAS;			/* 折り返し文字数 */
	pShareData->m_Types[nIdx].m_nColmSpace = 0;					/* 文字と文字の隙間 */
	pShareData->m_Types[nIdx].m_nLineSpace = 1;					/* 行間のすきま */
	pShareData->m_Types[nIdx].m_nTabSpace = 4;					/* TABの文字数 */
	for( i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		pShareData->m_Types[nIdx].m_nKeyWordSetIdx[i] = -1;
	}
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	strcpy( pShareData->m_Types[nIdx].m_szTabViewString, "^       " );	/* TAB表示文字列 */
//#endif
	pShareData->m_Types[nIdx].m_bTabArrow = FALSE;				/* タブ矢印表示 */	// 2001.12.03 hor
	pShareData->m_Types[nIdx].m_bInsSpace = FALSE;				/* スペースの挿入 */	// 2001.12.03 hor
	
	//@@@ 2002.09.22 YAZAKI 以下、m_cLineCommentとm_cBlockCommentを使うように修正
	pShareData->m_Types[nIdx].m_cLineComment.CopyTo(0, "", -1);	/* 行コメントデリミタ */
	pShareData->m_Types[nIdx].m_cLineComment.CopyTo(1, "", -1);	/* 行コメントデリミタ2 */
	pShareData->m_Types[nIdx].m_cLineComment.CopyTo(2, "", -1);	/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
	pShareData->m_Types[nIdx].m_cBlockComments[0].SetBlockCommentRule("", "");	/* ブロックコメントデリミタ */
	pShareData->m_Types[nIdx].m_cBlockComments[1].SetBlockCommentRule("", "");	/* ブロックコメントデリミタ2 */

	pShareData->m_Types[nIdx].m_nStringType = 0;					/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
	strcpy( pShareData->m_Types[nIdx].m_szIndentChars, _T("") );		/* その他のインデント対象文字 */

	pShareData->m_Types[nIdx].m_nColorInfoArrNum = COLORIDX_LAST;

	// 2001/06/14 Start by asa-o
	_tcscpy( pShareData->m_Types[nIdx].m_szHokanFile, _T("") );		/* 入力補完 単語ファイル */
	// 2001/06/14 End

	// 2001/06/19 asa-o
	pShareData->m_Types[nIdx].m_bHokanLoHiCase = false;			/* 入力補完機能：英大文字小文字を同一視する */

	//	2003.06.23 Moca ファイル内からの入力補完機能
	pShareData->m_Types[nIdx].m_bUseHokanByFile = false;			/*! 入力補完 開いているファイル内から候補を探す */

	// 文字コード設定
	pShareData->m_Types[nIdx].m_eDefaultCodetype = CODE_SJIS;
	pShareData->m_Types[nIdx].m_eDefaultEoltype = EOL_CRLF;
	pShareData->m_Types[nIdx].m_bDefaultBom = false;

	//@@@2002.2.4 YAZAKI
	pShareData->m_Types[nIdx].m_szExtHelp[0] = _T('\0');
	pShareData->m_Types[nIdx].m_szExtHtmlHelp[0] = _T('\0');
	pShareData->m_Types[nIdx].m_bHtmlHelpIsSingle = TRUE;

	pShareData->m_Types[nIdx].m_bAutoIndent = TRUE;			/* オートインデント */
	pShareData->m_Types[nIdx].m_bAutoIndent_ZENSPACE = TRUE;	/* 日本語空白もインデント */
	pShareData->m_Types[nIdx].m_bRTrimPrevLine = FALSE;			/* 2005.10.11 ryoji 改行時に末尾の空白を削除 */

	pShareData->m_Types[nIdx].m_nIndentLayout = 0;	/* 折り返しは2行目以降を字下げ表示 */

	static ColorInfoIni ColorInfo_DEFAULT[] = {
	//	Nov. 9, 2000 Jepro note: color setting (詳細は CshareData.h を参照のこと)
	//	0,							1(Disp),	 2(FatFont),3(UnderLIne) , 4(colTEXT),	5(colBACK),
	//	szName(項目名),				色分け／表示, 太字,		下線,		文字色,		背景色,
	//
	//Oct. 8, 2000 JEPRO 背景色を真っ白RGB(255,255,255)→(255,251,240)に変更(眩しさを押さえた)
		"テキスト",							TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 251, 240 ),
		"ルーラー",							TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 239, 239, 239 ),
		"カーソル",							TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 251, 240 ),	// 2006.12.07 ryoji
		"カーソル(IME ON)",					TRUE , FALSE, FALSE, RGB( 255, 0, 0 )		, RGB( 255, 251, 240 ),	// 2006.12.07 ryoji
		"カーソル行アンダーライン",			TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
 		"カーソル位置縦線",					FALSE, FALSE, FALSE, RGB( 128, 128, 255 )	, RGB( 255, 251, 240 ),// 2007.09.09 Moca
		"行番号",							TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
		"行番号(変更行)",					TRUE , TRUE , FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
		"TAB記号",							TRUE , FALSE, FALSE, RGB( 128, 128, 128 )	, RGB( 255, 251, 240 ),	//Jan. 19, 2001 JEPRO RGB(192,192,192)より濃いグレーに変更
		"半角空白"		,					FALSE , FALSE, FALSE , RGB( 192, 192, 192 )	, RGB( 255, 251, 240 ), //2002.04.28 Add by KK
		"日本語空白",						TRUE , FALSE, FALSE, RGB( 192, 192, 192 )	, RGB( 255, 251, 240 ),
		"コントロールコード",				TRUE , FALSE, FALSE, RGB( 255, 255, 0 )		, RGB( 255, 251, 240 ),
		"改行記号",							TRUE , FALSE, FALSE, RGB( 0, 128, 255 )		, RGB( 255, 251, 240 ),
		"折り返し記号",						TRUE , FALSE, FALSE, RGB( 255, 0, 255 )		, RGB( 255, 251, 240 ),
		"指定桁縦線",						FALSE, FALSE, FALSE, RGB( 192, 192, 192 )	, RGB( 255, 251, 240 ), //2005.11.08 Moca
		"EOF記号",							TRUE , FALSE, FALSE, RGB( 0, 255, 255 )		, RGB( 0, 0, 0 ),
//#ifdef COMPILE_COLOR_DIGIT
		"半角数値",							FALSE, FALSE, FALSE, RGB( 235, 0, 0 )		, RGB( 255, 251, 240 ),	//@@@ 2001.02.17 by MIK		//Mar. 7, 2001 JEPRO RGB(0,0,255)を変更  Mar.10, 2001 標準は色なしに
//#endif
		"検索文字列",						TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 255, 0 ),
		"強調キーワード1",					TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
		"強調キーワード2",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO キーワード1とは違う色に変更
		"強調キーワード3",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO キーワード1とは違う色に変更
		"強調キーワード4",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"強調キーワード5",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"強調キーワード6",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"強調キーワード7",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"強調キーワード8",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"強調キーワード9",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"強調キーワード10",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),
		"コメント",							TRUE , FALSE, FALSE, RGB( 0, 128, 0 )		, RGB( 255, 251, 240 ),
	//Sept. 4, 2000 JEPRO シングルクォーテーション文字列に色を割り当てるが色分け表示はしない
	//Oct. 17, 2000 JEPRO 色分け表示するように変更(最初のFALSE→TRUE)
	//"シングルクォーテーション文字列", FALSE, FALSE, FALSE, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ),
		"シングルクォーテーション文字列",	TRUE , FALSE, FALSE, RGB( 64, 128, 128 )	, RGB( 255, 251, 240 ),
		"ダブルクォーテーション文字列",		TRUE , FALSE, FALSE, RGB( 128, 0, 64 )		, RGB( 255, 251, 240 ),
		"URL",								TRUE , FALSE, TRUE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
		"正規表現キーワード1",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"正規表現キーワード2",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"正規表現キーワード3",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"正規表現キーワード4",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"正規表現キーワード5",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"正規表現キーワード6",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"正規表現キーワード7",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"正規表現キーワード8",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"正規表現キーワード9",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"正規表現キーワード10",		FALSE , FALSE, FALSE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),	//@@@ 2001.11.17 add MIK
		"DIFF差分表示(追加)",		FALSE , FALSE, FALSE, RGB( 0, 0, 0 )		, RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
		"DIFF差分表示(変更)",		FALSE , FALSE, FALSE, RGB( 0, 0, 0 )		, RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
		"DIFF差分表示(削除)",		FALSE , FALSE, FALSE, RGB( 0, 0, 0 )		, RGB( 255, 251, 240 ),	//@@@ 2002.06.01 MIK
		"対括弧の強調表示",			FALSE , TRUE,  FALSE, RGB( 128, 0, 0 )		, RGB( 255, 251, 240 ),	// 02/09/18 ai
		"ブックマーク",				TRUE  , FALSE, FALSE, RGB( 255, 251, 240 )	, RGB( 0, 128, 192 ),	// 02/10/16 ai
	};
//	To Here Sept. 18, 2000


	for( i = 0; i < COLORIDX_LAST; ++i ){
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_nColorIdx		= i;
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bDisp			= ColorInfo_DEFAULT[i].m_bDisp;
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bFatFont		= ColorInfo_DEFAULT[i].m_bFatFont;
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bUnderLine		= ColorInfo_DEFAULT[i].m_bUnderLine;
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_colTEXT			= ColorInfo_DEFAULT[i].m_colTEXT;
		pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_colBACK			= ColorInfo_DEFAULT[i].m_colBACK;
		strcpy( pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_szName, ColorInfo_DEFAULT[i].m_pszName );
	}
	pShareData->m_Types[nIdx].m_bLineNumIsCRLF = TRUE;				/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	pShareData->m_Types[nIdx].m_nLineTermType = 1;					/* 行番号区切り 0=なし 1=縦線 2=任意 */
	pShareData->m_Types[nIdx].m_cLineTermChar = ':';					/* 行番号区切り文字 */
	pShareData->m_Types[nIdx].m_bWordWrap = FALSE;					/* 英文ワードラップをする */
	pShareData->m_Types[nIdx].m_nCurrentPrintSetting = 0;				/* 現在選択している印刷設定 */
	pShareData->m_Types[nIdx].m_nDefaultOutline = OUTLINE_TEXT;		/* アウトライン解析方法 */
	pShareData->m_Types[nIdx].m_nSmartIndent = SMARTINDENT_NONE;		/* スマートインデント種別 */
	pShareData->m_Types[nIdx].m_nImeState = IME_CMODE_NOCONVERSION;	/* IME入力 */

	pShareData->m_Types[nIdx].m_szOutlineRuleFilename[0] = '\0';	//Dec. 4, 2000 MIK
	pShareData->m_Types[nIdx].m_bKinsokuHead = FALSE;				/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	pShareData->m_Types[nIdx].m_bKinsokuTail = FALSE;				/* 行末禁則 */	//@@@ 2002.04.08 MIK
	pShareData->m_Types[nIdx].m_bKinsokuRet  = FALSE;				/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
	pShareData->m_Types[nIdx].m_bKinsokuKuto = FALSE;				/* 句読点をぶら下げる */	//@@@ 2002.04.17 MIK
	strcpy( pShareData->m_Types[nIdx].m_szKinsokuHead, "" );		/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	strcpy( pShareData->m_Types[nIdx].m_szKinsokuTail, "" );		/* 行末禁則 */	//@@@ 2002.04.08 MIK
	strcpy( pShareData->m_Types[nIdx].m_szKinsokuKuto, "、。，．､｡,." );	/* 句読点ぶら下げ文字 */	// 2009.08.07 ryoji

	pShareData->m_Types[nIdx].m_bUseDocumentIcon = FALSE;			/* 文書に関連づけられたアイコンを使う */

//@@@ 2001.11.17 add start MIK
	for(i = 0; i < 100; i++)
	{
		pShareData->m_Types[nIdx].m_RegexKeywordArr[i].m_szKeyword[0] = _T('\0');
		pShareData->m_Types[nIdx].m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
	}
	pShareData->m_Types[nIdx].m_bUseRegexKeyword = FALSE;
//		pShareData->m_Types[nIdx].m_nRegexKeyMagicNumber = 1;
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	for(i = 0; i < MAX_KEYHELP_FILE; i++){
		pShareData->m_Types[nIdx].m_KeyHelpArr[i].m_nUse = 0;
		pShareData->m_Types[nIdx].m_KeyHelpArr[i].m_szAbout[0] = _T('\0');
		pShareData->m_Types[nIdx].m_KeyHelpArr[i].m_szPath[0] = _T('\0');
	}
	pShareData->m_Types[nIdx].m_bUseKeyWordHelp = FALSE;	/* 辞書選択機能の使用可否 */
	pShareData->m_Types[nIdx].m_nKeyHelpNum = 0;			/* 登録辞書数 */
	pShareData->m_Types[nIdx].m_bUseKeyHelpAllSearch = FALSE;	/* ヒットした次の辞書も検索(&A) */
	pShareData->m_Types[nIdx].m_bUseKeyHelpKeyDisp = FALSE;		/* 1行目にキーワードも表示する(&W) */
	pShareData->m_Types[nIdx].m_bUseKeyHelpPrefix = FALSE;		/* 選択範囲で前方一致検索(&P) */
//@@@ 2006.04.10 fon ADD-end

	// 2005.11.08 Moca 指定位置縦線の設定
	for( i = 0; i < MAX_VERTLINES; i++ ){
		pShareData->m_Types[nIdx].m_nVertLineIdx[i] = 0;
	}

	static char* pszTypeNameArr[] = {
		"基本",
		"テキスト",
		"C/C++",
		"HTML",
		"PL/SQL",
		"COBOL",
		"Java",
		"アセンブラ",
		"AWK",
		"MS-DOSバッチファイル",
		"Pascal",
		"TeX",				//Oct. 31, 2000 JEPRO TeX  ユーザに贈る
		"Perl",				//Jul. 08, 2001 JEPRO Perl ユーザに贈る
		"Visual Basic",		//JUl. 10, 2001 JEPRO VB   ユーザに贈る
		"リッチテキスト",	//JUl. 10, 2001 JEPRO WinHelp作るのにいるケンね
		"設定ファイル",		//Nov. 9, 2000 JEPRO Windows標準のini, inf, cnfファイルとsakuraキーワード設定ファイル.kwd, 色設定ファイル.col も読めるようにする
	};
	static char* pszTypeExts[] = {
		"",
		//Nov. 15, 2000 JEPRO PostScriptファイルも読めるようにする
		//Jan. 12, 2001 JEPRO readme.1st も読めるようにする
		//Feb. 12, 2001 JEPRO .err エラーメッセージ
		//Nov.  6, 2002 genta docはMS Wordに譲ってここからは外す（関連づけ防止のため）
		//Nov.  6, 2002 genta log を追加
		"txt,log,1st,err,ps",
		//	Jan. 24, 2004 genta 関連づけ上好ましくないのでdsw,dsp,dep,makははずす
		"c,cpp,cxx,cc,cp,c++,h,hpp,hxx,hh,hp,h++,rc,hm",	//Oct. 31, 2000 JEPRO VC++の生成するテキストファイルも読めるようにする
			//Feb. 7, 2001 JEPRO .cc/cp/c++/.hpp/hxx/hh/hp/h++を追加	//Mar. 15, 2001 JEPRO .hmを追加
		"html,htm,shtml,plg",	//Oct. 31, 2000 JEPRO VC++の生成するテキストファイルも読み込めるようにする
		"sql,plsql",
		"cbl,cpy,pco,cob",	//Jun. 04, 2001 JEPRO KENCH氏の助言に従い追加
		"java,jav",
		"asm",
		"awk",
		"bat",
		"dpr,pas",
		"tex,ltx,sty,bib,log,blg,aux,bbl,toc,lof,lot,idx,ind,glo",		//Oct. 31, 2000 JEPRO TeX ユーザに贈る	//Mar. 10, 2001 JEPRO 追加
		"cgi,pl,pm",			//Jul. 08, 2001 JEPRO 追加
		"bas,frm,cls,ctl,pag,dob,dsr,vb",	//Jul. 09, 2001 JEPRO 追加 //Dec. 16, 2002 MIK追加 // Feb. 19, 2006 genta .vb追加
		"rtf",					//Jul. 10, 2001 JEPRO 追加
		"ini,inf,cnf,kwd,col",	//Nov. 9, 2000 JEPRO Windows標準のini, inf, cnfファイルとsakuraキーワード設定ファイル.kwd, 色設定ファイル.col も読めるようにする
	};

	pShareData->m_Types[0].m_nIdx = 0;
	strcpy( pShareData->m_Types[0].m_szTypeName, pszTypeNameArr[0] );				/* タイプ属性：名称 */
	strcpy( pShareData->m_Types[0].m_szTypeExts, pszTypeExts[0] );				/* タイプ属性：拡張子リスト */
	for( nIdx = 1; nIdx < MAX_TYPES; ++nIdx ){
		pShareData->m_Types[nIdx] = pShareData->m_Types[0];
		pShareData->m_Types[nIdx].m_nIdx = nIdx;

		//	From Here 2005.02.20 りんご 配列数が設定数より小さいケースの考慮
		const char* pszTypeName;
		const char* pszTypeExt;
		char szTypeName[20];
		if(nIdx < (sizeof(pszTypeNameArr)/sizeof(char*)))
			pszTypeName = pszTypeNameArr[nIdx];
		else{
			sprintf( szTypeName, "設定%d", nIdx + 1 );
			pszTypeName = szTypeName;
		}
		if(nIdx < (sizeof(pszTypeExts)/sizeof(char*)))
			pszTypeExt = pszTypeExts[nIdx];
		else
			pszTypeExt = "";
		strcpy( m_pShareData->m_Types[nIdx].m_szTypeName, pszTypeName );
		strcpy( m_pShareData->m_Types[nIdx].m_szTypeExts, pszTypeExt );
		//	To Here 2005.02.20 りんご
	}


	/* 基本 */
	pShareData->m_Types[0].m_nMaxLineKetas = MAXLINEKETAS;			/* 折り返し文字数 */
//		pShareData->m_Types[0].m_nDefaultOutline = OUTLINE_UNKNOWN;	/* アウトライン解析方法 */	//Jul. 08, 2001 JEPRO 使わないように変更
	pShareData->m_Types[0].m_nDefaultOutline = OUTLINE_TEXT;		/* アウトライン解析方法 */
	//Oct. 17, 2000 JEPRO	シングルクォーテーション文字列を色分け表示しない
	pShareData->m_Types[0].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	//Sept. 4, 2000 JEPRO	ダブルクォーテーション文字列を色分け表示しない
	pShareData->m_Types[0].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;

//		nIdx = 0;
	/* テキスト */
	//From Here Sept. 20, 2000 JEPRO テキストの規定値を80→120に変更(不具合一覧.txtがある程度読みやすい桁数)
	pShareData->m_Types[1].m_nMaxLineKetas = 120;					/* 折り返し文字数 */
	//To Here Sept. 20, 2000
	pShareData->m_Types[1].m_nDefaultOutline = OUTLINE_TEXT;		/* アウトライン解析方法 */
	//Oct. 17, 2000 JEPRO	シングルクォーテーション文字列を色分け表示しない
	pShareData->m_Types[1].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	//Sept. 4, 2000 JEPRO	ダブルクォーテーション文字列を色分け表示しない
	pShareData->m_Types[1].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
	pShareData->m_Types[1].m_bKinsokuHead = FALSE;				/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	pShareData->m_Types[1].m_bKinsokuTail = FALSE;				/* 行末禁則 */	//@@@ 2002.04.08 MIK
	pShareData->m_Types[1].m_bKinsokuRet  = FALSE;				/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
	pShareData->m_Types[1].m_bKinsokuKuto = FALSE;				/* 句読点をぶら下げる */	//@@@ 2002.04.17 MIK
//		strcpy( pShareData->m_Types[1].m_szKinsokuHead, "!%),.:;?]}￠°’”‰′″℃、。々〉》」』】〕ぁぃぅぇぉっゃゅょゎ゛゜ゝゞァィゥェォッャュョヮヵヶ・ーヽヾ！％），．：；？］｝｡｣､･ｧｨｩｪｫｬｭｮｯｰﾞﾟ￠" );		/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	strcpy( pShareData->m_Types[1].m_szKinsokuHead, "!%),.:;?]}￠°’”‰′″℃、。々〉》」』】〕゛゜ゝゞ・ヽヾ！％），．：；？］｝｡｣､･ﾞﾟ￠" );		/* 行頭禁則 */	//@@@ 2002.04.13 MIK
	strcpy( pShareData->m_Types[1].m_szKinsokuTail, "$([{￡\\‘“〈《「『【〔＄（［｛｢￡￥" );		/* 行末禁則 */	//@@@ 2002.04.08 MIK
	// pShareData->m_Types[1].m_szKinsokuKuto（句読点ぶら下げ文字）はここではなく全タイプにデフォルト設定	// 2009.08.07 ryoji
	//Jun. 2, 2009 syat     クリッカブルパス用正規表現キーワード //Jan.17, 2012 syat 基本からテキストに移動
	pShareData->m_Types[1].m_bUseRegexKeyword = TRUE;							// 正規表現キーワードを使うか
	pShareData->m_Types[1].m_RegexKeywordArr[0].m_nColorIndex = COLORIDX_URL;	// 色指定番号
	strcpy(pShareData->m_Types[1].m_RegexKeywordArr[0].m_szKeyword,				// 正規表現キーワード
		   "/(?<=\")(\\b[a-zA-Z]:|\\B\\\\\\\\)[^\"\\r\\n]*/k");					// ""で挟まれた C:\～, \\～ にマッチするパターン
	pShareData->m_Types[1].m_RegexKeywordArr[1].m_nColorIndex = COLORIDX_URL;	// 色指定番号
	strcpy(pShareData->m_Types[1].m_RegexKeywordArr[1].m_szKeyword,				// 正規表現キーワード
		   "/(\\b[a-zA-Z]:\\\\|\\B\\\\\\\\)[\\w\\-_.\\\\\\/$%~]*/k");			// C:\～, \\～ にマッチするパターン


	// nIdx = 1;
	/* C/C++ */
	pShareData->m_Types[2].m_cLineComment.CopyTo( 0, "//", -1 );			/* 行コメントデリミタ */
	pShareData->m_Types[2].m_cBlockComments[0].SetBlockCommentRule("/*", "*/" );		/* ブロックコメントデリミタ */
	pShareData->m_Types[2].m_cBlockComments[1].SetBlockCommentRule("#if 0", "#endif" );	/* ブロックコメントデリミタ2 */	//Jul. 11, 2001 JEPRO
	pShareData->m_Types[2].m_nKeyWordSetIdx[0] = 0;						/* キーワードセット */
	pShareData->m_Types[2].m_nDefaultOutline = OUTLINE_CPP;			/* アウトライン解析方法 */
	pShareData->m_Types[2].m_nSmartIndent = SMARTINDENT_CPP;			/* スマートインデント種別 */
	//Mar. 10, 2001 JEPRO	半角数値を色分け表示
	pShareData->m_Types[2].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;
	//	Sep. 21, 2002 genta 対括弧の強調をデフォルトONに
	pShareData->m_Types[2].m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp	= TRUE;
	//	2003.06.23 Moca ファイル内からの入力補完機能
	pShareData->m_Types[2].m_bUseHokanByFile = true;			/*! 入力補完 開いているファイル内から候補を探す */

	/* HTML */
	pShareData->m_Types[3].m_cBlockComments[0].SetBlockCommentRule("<!--", "-->" );	/* ブロックコメントデリミタ */
	pShareData->m_Types[3].m_nStringType = 0;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	pShareData->m_Types[3].m_nKeyWordSetIdx[0] = 1;						/* キーワードセット */
	pShareData->m_Types[3].m_nDefaultOutline = OUTLINE_HTML;			/* アウトライン解析方法 */
	// Feb. 2, 2005 genta 苦情が多いのでシングルクォートの色分けはHTMLでは行わない
	pShareData->m_Types[3].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;

	// nIdx = 3;
	/* PL/SQL */
	pShareData->m_Types[4].m_cLineComment.CopyTo( 0, "--", -1 );		/* 行コメントデリミタ */
	pShareData->m_Types[4].m_cBlockComments[0].SetBlockCommentRule("/*", "*/" );	/* ブロックコメントデリミタ */
	pShareData->m_Types[4].m_nStringType = 1;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	strcpy( pShareData->m_Types[4].m_szIndentChars, "|★" );			/* その他のインデント対象文字 */
	pShareData->m_Types[4].m_nKeyWordSetIdx[0] = 2;						/* キーワードセット */
	pShareData->m_Types[4].m_nDefaultOutline = OUTLINE_PLSQL;			/* アウトライン解析方法 */

	/* COBOL */
	pShareData->m_Types[5].m_cLineComment.CopyTo( 0, "*", 6 );	//Jun. 02, 2001 JEPRO 修正
	pShareData->m_Types[5].m_cLineComment.CopyTo( 1, "D", 6 );	//Jun. 04, 2001 JEPRO 追加
	pShareData->m_Types[5].m_nStringType = 1;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	strcpy( pShareData->m_Types[5].m_szIndentChars, "*" );			/* その他のインデント対象文字 */
	pShareData->m_Types[5].m_nKeyWordSetIdx[0] = 3;						/* キーワードセット */		//Jul. 10, 2001 JEPRO
	pShareData->m_Types[5].m_nDefaultOutline = OUTLINE_COBOL;			/* アウトライン解析方法 */

	// 2005.11.08 Moca 指定桁縦線
	pShareData->m_Types[5].m_ColorInfoArr[COLORIDX_VERTLINE].m_bDisp = TRUE;
	pShareData->m_Types[5].m_nVertLineIdx[0] = 7;
	pShareData->m_Types[5].m_nVertLineIdx[1] = 8;
	pShareData->m_Types[5].m_nVertLineIdx[2] = 12;
	pShareData->m_Types[5].m_nVertLineIdx[3] = 73;


	/* Java */
	pShareData->m_Types[6].m_cLineComment.CopyTo( 0, "//", -1 );		/* 行コメントデリミタ */
	pShareData->m_Types[6].m_cBlockComments[0].SetBlockCommentRule("/*", "*/" );	/* ブロックコメントデリミタ */
	pShareData->m_Types[6].m_nKeyWordSetIdx[0] = 4;						/* キーワードセット */
	pShareData->m_Types[6].m_nDefaultOutline = OUTLINE_JAVA;			/* アウトライン解析方法 */
	pShareData->m_Types[6].m_nSmartIndent = SMARTINDENT_CPP;			/* スマートインデント種別 */
	//Mar. 10, 2001 JEPRO	半角数値を色分け表示
	pShareData->m_Types[6].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;
	//	Sep. 21, 2002 genta 対括弧の強調をデフォルトONに
	pShareData->m_Types[6].m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp	= TRUE;

	/* アセンブラ */
	//	2004.05.01 MIK/genta
	pShareData->m_Types[7].m_cLineComment.CopyTo( 0, ";", -1 );		/* 行コメントデリミタ */
	pShareData->m_Types[7].m_nDefaultOutline = OUTLINE_ASM;			/* アウトライン解析方法 */
	//Mar. 10, 2001 JEPRO	半角数値を色分け表示
	pShareData->m_Types[7].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;

	/* awk */
	pShareData->m_Types[8].m_cLineComment.CopyTo( 0, "#", -1 );		/* 行コメントデリミタ */
	pShareData->m_Types[8].m_nDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
	pShareData->m_Types[8].m_nKeyWordSetIdx[0] = 6;						/* キーワードセット */

	/* MS-DOSバッチファイル */
	pShareData->m_Types[9].m_cLineComment.CopyTo( 0, "REM ", -1 );	/* 行コメントデリミタ */
	pShareData->m_Types[9].m_nDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
	pShareData->m_Types[9].m_nKeyWordSetIdx[0] = 7;						/* キーワードセット */

	/* Pascal */
	pShareData->m_Types[10].m_cLineComment.CopyTo( 0, "//", -1 );		/* 行コメントデリミタ */		//Nov. 5, 2000 JEPRO 追加
	pShareData->m_Types[10].m_cBlockComments[0].SetBlockCommentRule("{", "}" );	/* ブロックコメントデリミタ */	//Nov. 5, 2000 JEPRO 追加
	pShareData->m_Types[10].m_cBlockComments[1].SetBlockCommentRule("(*", "*)" );	/* ブロックコメントデリミタ2 */	//@@@ 2001.03.10 by MIK
	pShareData->m_Types[10].m_nStringType = 1;						/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */	//Nov. 5, 2000 JEPRO 追加
	pShareData->m_Types[10].m_nKeyWordSetIdx[0] = 8;						/* キーワードセット */
	//Mar. 10, 2001 JEPRO	半角数値を色分け表示
	pShareData->m_Types[10].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	//@@@ 2001.11.11 upd MIK

	//From Here Oct. 31, 2000 JEPRO
	/* TeX */
	pShareData->m_Types[11].m_cLineComment.CopyTo( 0, "%", -1 );		/* 行コメントデリミタ */
	pShareData->m_Types[11].m_nDefaultOutline = OUTLINE_TEX;			/* アウトライン解析方法 */
	pShareData->m_Types[11].m_nKeyWordSetIdx[0]  = 9;					/* キーワードセット */
	pShareData->m_Types[11].m_nKeyWordSetIdx[1] = 10;					/* キーワードセット2 */	//Jan. 19, 2001 JEPRO
	//シングルクォーテーション文字列を色分け表示しない
	pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	//ダブルクォーテーション文字列を色分け表示しない
	pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
	//URLにアンダーラインを引かない(やっぱりやめた)
//		pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_URL].m_bDisp = FALSE;
	//To Here Oct. 31, 2000

	//From Here Jul. 08, 2001 JEPRO
	/* Perl */
	pShareData->m_Types[12].m_cLineComment.CopyTo( 0, "#", -1 );		/* 行コメントデリミタ */
	pShareData->m_Types[12].m_nDefaultOutline = OUTLINE_PERL;			/* アウトライン解析方法 */
	pShareData->m_Types[12].m_nKeyWordSetIdx[0]  = 11;					/* キーワードセット */
	pShareData->m_Types[12].m_nKeyWordSetIdx[1] = 12;					/* キーワードセット2 */
	pShareData->m_Types[12].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* 半角数値を色分け表示 */
	//To Here Jul. 08, 2001
	//	Sep. 21, 2002 genta 対括弧の強調をデフォルトONに
	pShareData->m_Types[12].m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp	= TRUE;

	//From Here Jul. 10, 2001 JEPRO
	/* Visual Basic */
	pShareData->m_Types[13].m_cLineComment.CopyTo( 0, "'", -1 );		/* 行コメントデリミタ */
	pShareData->m_Types[13].m_nDefaultOutline = OUTLINE_VB;			/* アウトライン解析方法 */
	pShareData->m_Types[13].m_nKeyWordSetIdx[0]  = 13;					/* キーワードセット */
	pShareData->m_Types[13].m_nKeyWordSetIdx[1] = 14;					/* キーワードセット2 */
	pShareData->m_Types[13].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* 半角数値を色分け表示 */
	pShareData->m_Types[13].m_nStringType = 1;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	//シングルクォーテーション文字列を色分け表示しない
	pShareData->m_Types[13].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;

	/* リッチテキスト */
	pShareData->m_Types[14].m_nDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
	pShareData->m_Types[14].m_nKeyWordSetIdx[0]  = 15;					/* キーワードセット */
	pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* 半角数値を色分け表示 */
	pShareData->m_Types[14].m_nStringType = 0;							/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	//シングルクォーテーション文字列を色分け表示しない
	pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	//ダブルクォーテーション文字列を色分け表示しない
	pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
	//URLにアンダーラインを引かない
	pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_URL].m_bDisp = FALSE;
	//To Here Jul. 10, 2001

	//From Here Nov. 9, 2000 JEPRO
	/* 設定ファイル */
	pShareData->m_Types[15].m_cLineComment.CopyTo( 0, "//", -1 );		/* 行コメントデリミタ */
	pShareData->m_Types[15].m_cLineComment.CopyTo( 1, ";", -1 );		/* 行コメントデリミタ2 */
	pShareData->m_Types[15].m_nDefaultOutline = OUTLINE_TEXT;			/* アウトライン解析方法 */
	//シングルクォーテーション文字列を色分け表示しない
	pShareData->m_Types[15].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	//ダブルクォーテーション文字列を色分け表示しない
	pShareData->m_Types[15].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
	//To Here Nov. 9, 2000
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

/* 指定ウィンドウが、編集ウィンドウのフレームウィンドウかどうか調べる */
BOOL IsSakuraMainWindow( HWND hWnd )
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
	if(0 == strcmp( GSTR_EDITWINDOWNAME, szClassName ) ){
		return TRUE;
	}else{
		return FALSE;
	}
}

/*[EOF]*/
