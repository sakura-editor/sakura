/*!	@file
@brief CViewCommanderクラスのコマンド(マクロ系)関数群

	2012/12/20	CViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, MIK, Stonee, Misaka, asa-o, novice, hor, YAZAKI
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2003, Moca
	Copyright (C) 2005, novice
	Copyright (C) 2006, maru
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, syat

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

//@@@ 2002.2.2 YAZAKI マクロはCSMacroMgrに統一
#include "macro/CSMacroMgr.h"
#include "dlg/CDlgExec.h"
#include "dlg/CDlgOpenFile.h"
#include "CEditApp.h"
#include "recent/CRecentCurDir.h"
#include "util/module.h"
#include "env/CShareData.h"
#include "env/CSakuraEnvironment.h"


/* キーマクロの記録開始／終了 */
void CViewCommander::Command_RECKEYMACRO( void )
{
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro ){									/* キーボードマクロの記録中 */
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;							/* キーボードマクロを記録中のウィンドウ */
		//@@@ 2002.1.24 YAZAKI キーマクロをマクロ用フォルダに「RecKey.mac」という名で保存
		TCHAR szInitDir[MAX_PATH];
		int nRet;
		// 2003.06.23 Moca 記録用キーマクロのフルパスをCShareData経由で取得
		nRet = CShareData::getInstance()->GetMacroFilename( -1, szInitDir, MAX_PATH ); 
		if( nRet <= 0 ){
			ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD24), nRet );
			return;
		}else{
			_tcscpy( GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName, szInitDir );
		}
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		int nSaveResult = m_pcSMacroMgr->Save(
			STAND_KEYMACRO,
			G_AppInstance(),
			GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName
		);
		if ( !nSaveResult ){
			ErrorMessage(	m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD25), GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName );
		}
	}else{
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = TRUE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = GetMainWindow();	/* キーボードマクロを記録中のウィンドウ */
		/* キーマクロのバッファをクリアする */
		//@@@ 2002.1.24 m_CKeyMacroMgrをCEditDocへ移動
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		m_pcSMacroMgr->Clear(STAND_KEYMACRO);
//		GetDocument()->m_CKeyMacroMgr.ClearAll();
//		GetDllShareData().m_CKeyMacroMgr.Clear();
	}
	/* 親ウィンドウのタイトルを更新 */
	GetEditWindow()->UpdateCaption();

	/* キャレットの行桁位置を表示する */
	GetCaret().ShowCaretPosInfo();
}



/* キーマクロの保存 */
void CViewCommander::Command_SAVEKEYMACRO( void )
{
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	//	Jun. 16, 2002 genta
	if( !m_pcSMacroMgr->IsSaveOk() ){
		//	保存不可
		ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD26) );
	}

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	szPath[0] = _T('\0');
	// 2003.06.23 Moca 相対パスは実行ファイルからのパス
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	if( _IS_REL_PATH( GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER ) ){
		GetInidirOrExedir( szInitDir, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER );
	}else{
		_tcscpy( szInitDir, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER );	/* マクロ用フォルダ */
	}
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		_T("*.mac"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
//	::SplitPath_FolderAndFile( szPath, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER, NULL );
//	wcscat( GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER, L"\\" );

	/* キーボードマクロの保存 */
	//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
	//@@@ 2002.1.24 YAZAKI
	if ( !m_pcSMacroMgr->Save( STAND_KEYMACRO, G_AppInstance(), szPath ) ){
		ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD27), szPath );
	}
	return;
}



/*! キーマクロの読み込み
	@date 2005.02.20 novice デフォルトの拡張子変更
 */
void CViewCommander::Command_LOADKEYMACRO( void )
{
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	const TCHAR*		pszFolder;
	szPath[0] = _T('\0');
	pszFolder = GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER;
	// 2003.06.23 Moca 相対パスは実行ファイルからのパス
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	if( _IS_REL_PATH( pszFolder ) ){
		GetInidirOrExedir( szInitDir, pszFolder );
	}else{
		_tcscpy( szInitDir, pszFolder );	/* マクロ用フォルダ */
	}
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
// 2005/02/20 novice デフォルトの拡張子変更
// 2005/07/13 novice 多様なマクロをサポートしているのでデフォルトは全て表示にする
		_T("*.*"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath, EFITER_MACRO ) ){
		return;
	}

	/* キーボードマクロの読み込み */
	//@@@ 2002.1.24 YAZAKI 読み込みといいつつも、ファイル名をコピーするだけ。実行直前に読み込む
	_tcscpy(GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName, szPath);
//	GetDllShareData().m_CKeyMacroMgr.LoadKeyMacro( G_AppInstance(), m_pCommanderView->GetHwnd(), szPath );
	return;
}



/* キーマクロの実行 */
void CViewCommander::Command_EXECKEYMACRO( void )
{
	//@@@ 2002.1.24 YAZAKI 記録中は終了してから実行
	if (GetDllShareData().m_sFlags.m_bRecordingKeyMacro){
		Command_RECKEYMACRO();
	}
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */

	/* キーボードマクロの実行 */
	//@@@ 2002.1.24 YAZAKI
	if ( GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName[0] ){
		//	ファイルが保存されていたら
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		BOOL bLoadResult = m_pcSMacroMgr->Load(
			STAND_KEYMACRO,
			G_AppInstance(),
			GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName,
			NULL
		);
		if ( !bLoadResult ){
			ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD28), GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName );
		}
		else {
			//	2007.07.20 genta : flagsオプション追加
			m_pcSMacroMgr->Exec( STAND_KEYMACRO, G_AppInstance(), m_pCommanderView, 0 );
		}
	}
	return;
}



/*! 名前を指定してマクロ実行
	@param pszPath	マクロのファイルパス、またはマクロのコード。
	@param pszType	種別。NULLの場合ファイル指定、それ以外の場合は言語の拡張子を指定

	@date 2008.10.23 syat 新規作成
	@date 2008.12.21 syat 引数「種別」を追加
 */
void CViewCommander::Command_EXECEXTMACRO( const WCHAR* pszPathW, const WCHAR* pszTypeW )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];	//ファイル選択ダイアログの初期フォルダ
	const TCHAR*	pszFolder;					//マクロフォルダ
	const TCHAR*	pszPath = NULL;				//第1引数をTCHAR*に変換した文字列
	const TCHAR*	pszType = NULL;				//第2引数をTCHAR*に変換した文字列
	HWND			hwndRecordingKeyMacro = NULL;

	if ( pszPathW != NULL ) {
		//to_tchar()で取得した文字列はdeleteしないこと。
		pszPath = to_tchar( pszPathW );
		pszType = to_tchar( pszTypeW );

	} else {
		// ファイルが指定されていない場合、ダイアログを表示する
		szPath[0] = _T('\0');
		pszFolder = GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER;

		if( _IS_REL_PATH( pszFolder ) ){
			GetInidirOrExedir( szInitDir, pszFolder );
		}else{
			_tcscpy( szInitDir, pszFolder );	/* マクロ用フォルダ */
		}
		/* ファイルオープンダイアログの初期化 */
		cDlgOpenFile.Create(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			_T("*.*"),
			szInitDir
		);
		if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath, EFITER_MACRO ) ){
			return;
		}
		pszPath = szPath;
		pszType = NULL;
	}

	//キーマクロ記録中の場合、追加する
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro &&									/* キーボードマクロの記録中 */
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow()	/* キーボードマクロを記録中のウィンドウ */
	){
		LPARAM lparams[] = {(LPARAM)pszPath, 0, 0, 0};
		m_pcSMacroMgr->Append( STAND_KEYMACRO, F_EXECEXTMACRO, lparams, m_pCommanderView );

		//キーマクロの記録を一時停止する
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
		hwndRecordingKeyMacro = GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* キーボードマクロを記録中のウィンドウ */
	}

	//古い一時マクロの退避
	CMacroManagerBase* oldMacro = m_pcSMacroMgr->SetTempMacro( NULL );

	BOOL bLoadResult = m_pcSMacroMgr->Load(
		TEMP_KEYMACRO,
		G_AppInstance(),
		pszPath,
		pszType
	);
	if ( !bLoadResult ){
		ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_MACROERR1), pszPath );
	}
	else {
		m_pcSMacroMgr->Exec( TEMP_KEYMACRO, G_AppInstance(), m_pCommanderView, FA_NONRECORD | FA_FROMMACRO );
	}

	// 終わったら開放
	m_pcSMacroMgr->Clear( TEMP_KEYMACRO );
	if ( oldMacro != NULL ) {
		m_pcSMacroMgr->SetTempMacro( oldMacro );
	}

	// キーマクロ記録中だった場合は再開する
	if ( hwndRecordingKeyMacro != NULL ) {
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = TRUE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = hwndRecordingKeyMacro;	/* キーボードマクロを記録中のウィンドウ */
	}
	return;
}



/*! 外部コマンド実行ダイアログ表示
	@date 2002.02.02 YAZAKI.
*/
void CViewCommander::Command_EXECCOMMAND_DIALOG( void )
{
	CDlgExec cDlgExec;

	/* モードレスダイアログの表示 */
	if( !cDlgExec.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0 ) ){
		return;
	}

	m_pCommanderView->AddToCmdArr( cDlgExec.m_szCommand );
	const WCHAR* cmd_string = to_wchar(cDlgExec.m_szCommand);
	const WCHAR* curDir = to_wchar(cDlgExec.m_szCurDir);
	const WCHAR* pszDir = curDir;
	if( curDir[0] == L'\0' ){
		pszDir = NULL;
	}else{
		CRecentCurDir cRecentCurDir;
		cRecentCurDir.AppendItem( cDlgExec.m_szCurDir );
		cRecentCurDir.Terminate();
	}

	//HandleCommand( F_EXECMD, true, (LPARAM)cmd_string, 0, 0, 0);	//	外部コマンド実行コマンドの発行
	HandleCommand( F_EXECMD, true, (LPARAM)cmd_string, (LPARAM)(GetDllShareData().m_nExecFlgOpt), (LPARAM)pszDir, 0);	//	外部コマンド実行コマンドの発行
}



//外部コマンド実行
//	Sept. 20, 2000 JEPRO  名称CMMANDをCOMMANDに変更
//	Oct. 9, 2001   genta  マクロ対応のため引数追加
//  2002.2.2       YAZAKI ダイアログ呼び出し部とコマンド実行部を分離
//void CEditView::Command_EXECCOMMAND( const char *cmd_string )
void CViewCommander::Command_EXECCOMMAND( LPCWSTR cmd_string, const int nFlgOpt, LPCWSTR pszCurDir)	//	2006.12.03 maru 引数の拡張
{
	//	From Here Aug. 21, 2001 genta
	//	パラメータ置換 (超暫定)
	const int bufmax = 1024;
	wchar_t buf[bufmax + 1];
	CSakuraEnvironment::ExpandParameter(cmd_string, buf, bufmax);

	// 子プロセスの標準出力をリダイレクトする
	std::tstring buf2 = to_tchar(buf);
	std::tstring buf3;
	if( pszCurDir ){
		buf3 = to_tchar(pszCurDir);
	}
	m_pCommanderView->ExecCmd( buf2.c_str(), nFlgOpt, (pszCurDir ? buf3.c_str() : NULL) );
	//	To Here Aug. 21, 2001 genta
	return;
}
