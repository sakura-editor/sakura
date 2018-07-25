/*!	@file
@brief CViewCommanderクラスのコマンド(ファイル操作系)関数群

	2012/12/20	CViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2003, MIK, genta, かろと, Moca
	Copyright (C) 2004, genta
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji, maru
	Copyright (C) 2007, ryoji, maru, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "_main/CControlTray.h"
#include "uiparts/CWaitCursor.h"
#include "dlg/CDlgProperty.h"
#include "dlg/CDlgCancel.h"// 2002/2/8 hor
#include "dlg/CDlgProfileMgr.h"
#include "doc/CDocReader.h"	//  Command_PROPERTY_FILE for _DEBUG
#include "print/CPrintPreview.h"
#include "io/CBinaryStream.h"
#include "io/CFileLoad.h"
#include "CWriteManager.h"
#include "CEditApp.h"
#include "recent/CMRUFile.h"
#include "util/window.h"
#include "charset/CCodeFactory.h"
#include "plugin/CPlugin.h"
#include "plugin/CJackManager.h"
#include "env/CSakuraEnvironment.h"
#include "debug/CRunningTimer.h"
#include "sakura_rc.h"


/* 新規作成 */
void CViewCommander::Command_FILENEW( void )
{
	/* 新たな編集ウィンドウを起動 */
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = _T("");
	sLoadInfo.eCharCode = CODE_NONE;
	sLoadInfo.bViewMode = false;
	std::tstring curDir = CSakuraEnvironment::GetDlgInitialDir();
	CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo, NULL, false, curDir.c_str(), false );
	return;
}



/* 新規作成（新しいウインドウで開く） */
void CViewCommander::Command_FILENEW_NEWWINDOW( void )
{
	/* 新たな編集ウィンドウを起動 */
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = _T("");
	sLoadInfo.eCharCode = CODE_DEFAULT;
	sLoadInfo.bViewMode = false;
	std::tstring curDir = CSakuraEnvironment::GetDlgInitialDir();
	CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo,
		NULL,
		false,
		curDir.c_str(),
		true
		);
	return;
}



/*! @brief ファイルを開く

	@date 2003.03.30 genta 「閉じて開く」から利用するために引数追加
	@date 2004.10.09 genta 実装をCEditDocへ移動
*/
void CViewCommander::Command_FILEOPEN( const WCHAR* filename, ECodeType nCharCode, bool bViewMode, const WCHAR* defaultName )
{
	if( !IsValidCodeType(nCharCode) && nCharCode != CODE_AUTODETECT ){
		nCharCode = CODE_AUTODETECT;
	}
	//ロード情報
	SLoadInfo sLoadInfo(filename?to_tchar(filename):_T(""), nCharCode, bViewMode);
	std::vector<std::tstring> files;
	std::tstring defName = (defaultName?to_tchar(defaultName):_T(""));

	//必要であれば「ファイルを開く」ダイアログ
	if(!sLoadInfo.cFilePath.IsValidPath()){
		if( !defName.empty() ){
			TCHAR szPath[_MAX_PATH];
			TCHAR szDir[_MAX_DIR];
			TCHAR szName[_MAX_FNAME];
			TCHAR szExt  [_MAX_EXT];
			my_splitpath_t(defName.c_str(), szPath, szDir, szName, szExt);
			auto_strcat(szPath, szDir);
			if( 0 == auto_stricmp(defName.c_str(), szPath) ){
				// defNameはフォルダ名だった
			}else{
				CFilePath path = defName.c_str();
				if( 0 == auto_stricmp(path.GetDirPath().c_str(), szPath) ){
					// フォルダ名までは実在している
					sLoadInfo.cFilePath = defName.c_str();
				}
			}
		}
		bool bDlgResult = GetDocument()->m_cDocFileOperation.OpenFileDialog(
			CEditWnd::getInstance()->GetHwnd(),	//[in]  オーナーウィンドウ
			defName.length()==0 ? NULL : defName.c_str(),	//[in]  フォルダ
			&sLoadInfo,							//[out] ロード情報受け取り
			files								//[out] ファイル名
		);
		if(!bDlgResult)return;

		sLoadInfo.cFilePath = files[0].c_str();
		// 他のファイルは新規ウィンドウ
		int nSize = (int)files.size();
		for( int i = 1; i < nSize; i++ ){
			SLoadInfo sFilesLoadInfo = sLoadInfo;
			sFilesLoadInfo.cFilePath = files[i].c_str();
			CControlTray::OpenNewEditor(
				G_AppInstance(),
				CEditWnd::getInstance()->GetHwnd(),
				sFilesLoadInfo,
				NULL,
				true
			);
		}
	}

	//開く
	GetDocument()->m_cDocFileOperation.FileLoad( &sLoadInfo );
}



/*! 上書き保存

	F_FILESAVEALLとの組み合わせのみで使われるコマンド．
	@param warnbeep [in] true: 保存不要 or 保存禁止のときに警告を出す
	@param askname	[in] true: ファイル名未設定の時に入力を促す

	@date 2004.02.28 genta 引数warnbeep追加
	@date 2005.01.24 genta 引数askname追加

*/
bool CViewCommander::Command_FILESAVE( bool warnbeep, bool askname )
{
	CEditDoc* pcDoc = GetDocument();

	//ファイル名が指定されていない場合は「名前を付けて保存」のフローへ遷移
	if( !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		if( !askname )
			return false;	// 保存しない
		return pcDoc->m_cDocFileOperation.FileSaveAs();
	}

	//セーブ情報
	SSaveInfo sSaveInfo;
	pcDoc->GetSaveInfo(&sSaveInfo);
	sSaveInfo.cEol = EOL_NONE; //改行コード無変換
	sSaveInfo.bOverwriteMode = true; //上書き要求

	//上書き処理
	if(!warnbeep)CEditApp::getInstance()->m_cSoundSet.MuteOn();
	bool bRet = pcDoc->m_cDocFileOperation.DoSaveFlow(&sSaveInfo);
	if(!warnbeep)CEditApp::getInstance()->m_cSoundSet.MuteOff();

	return bRet;
}



/* 名前を付けて保存ダイアログ */
bool CViewCommander::Command_FILESAVEAS_DIALOG(const WCHAR* fileNameDef,ECodeType eCodeType, EEolType eEolType)
{
	return 	GetDocument()->m_cDocFileOperation.FileSaveAs(fileNameDef, eCodeType, eEolType, true);
}



/* 名前を付けて保存
	filenameで保存。NULLは厳禁。
*/
BOOL CViewCommander::Command_FILESAVEAS( const WCHAR* filename, EEolType eEolType )
{
	return 	GetDocument()->m_cDocFileOperation.FileSaveAs(filename, CODE_NONE, eEolType, false);
}



/*!	全て上書き保存

	編集中の全てのウィンドウで上書き保存を行う．
	ただし，上書き保存の指示を出すのみで実行結果の確認は行わない．

	上書き禁止及びファイル名未設定のウィンドウでは何も行わない．

	@date 2005.01.24 genta 新規作成
*/
BOOL CViewCommander::Command_FILESAVEALL( void )
{
	CAppNodeGroupHandle(0).SendMessageToAllEditors(
		WM_COMMAND,
		MAKELONG( F_FILESAVE_QUIET, 0 ),
		0,
		NULL
	);
	return TRUE;
}



/* 閉じて(無題) */	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
void CViewCommander::Command_FILECLOSE( void )
{
	GetDocument()->m_cDocFileOperation.FileClose();
}



/*! @brief 閉じて開く

	@date 2003.03.30 genta 開くダイアログでキャンセルしたとき元のファイルが残るように。
				ついでにFILEOPENと同じように引数を追加しておく
*/
void CViewCommander::Command_FILECLOSE_OPEN( LPCWSTR filename, ECodeType nCharCode, bool bViewMode )
{
	GetDocument()->m_cDocFileOperation.FileCloseOpen( SLoadInfo(to_tchar(filename), nCharCode, bViewMode) );

	//プラグイン：DocumentOpenイベント実行
	CPlug::Array plugs;
	CWSHIfObj::List params;
	CJackManager::getInstance()->GetUsablePlug( PP_DOCUMENT_OPEN, 0, &plugs );
	for( CPlug::ArrayIter it = plugs.begin(); it != plugs.end(); it++ ){
		(*it)->Invoke(&GetEditWindow()->GetActiveView(), params);
	}
}



//! ファイルの再オープン
void CViewCommander::Command_FILE_REOPEN(
	ECodeType	nCharCode,	//!< [in] 開き直す際の文字コード
	bool		bNoConfirm	//!< [in] ファイルが更新された場合に確認を行わ「ない」かどうか。true:確認しない false:確認する
)
{
	CEditDoc* pcDoc = GetDocument();
	if( !bNoConfirm && fexist(pcDoc->m_cDocFile.GetFilePath()) && pcDoc->m_cDocEditor.IsModified() ){
		int nDlgResult = MYMESSAGEBOX(
			m_pCommanderView->GetHwnd(),
			MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			LS(STR_ERR_CEDITVIEW_CMD29),
			pcDoc->m_cDocFile.GetFilePath()
		);
		if( IDOK == nDlgResult ){
			//継続。下へ進む
		}else{
			return; //中断
		}
	}

	// 同一ファイルの再オープン
	pcDoc->m_cDocFileOperation.ReloadCurrentFile( nCharCode );
}



/* 印刷 */
void CViewCommander::Command_PRINT( void )
{
	// 使っていない処理を削除 2003.05.04 かろと
	Command_PRINT_PREVIEW();

	/* 印刷実行 */
	GetEditWindow()->m_pPrintPreview->OnPrint();
}



/* 印刷プレビュー */
void CViewCommander::Command_PRINT_PREVIEW( void )
{
	/* 印刷プレビューモードのオン/オフ */
	GetEditWindow()->PrintPreviewModeONOFF();
	return;
}



/* 印刷のページレイアウトの設定 */
void CViewCommander::Command_PRINT_PAGESETUP( void )
{
	/* 印刷ページ設定 */
	GetEditWindow()->OnPrintPageSetting();
	return;
}



//From Here Feb. 10, 2001 JEPRO 追加
/* C/C++ヘッダファイルまたはソースファイル オープン機能 */
BOOL CViewCommander::Command_OPEN_HfromtoC( BOOL bCheckOnly )
{
	if ( Command_OPEN_HHPP( bCheckOnly, FALSE ) )	return TRUE;
	if ( Command_OPEN_CCPP( bCheckOnly, FALSE ) )	return TRUE;
	ErrorBeep();
	return FALSE;
// 2002/03/24 YAZAKI コードの重複を削減
// 2003.06.28 Moca コメントとして残っていたコードを削除
}



/* C/C++ヘッダファイル オープン機能 */		//Feb. 10, 2001 jepro	説明を「インクルードファイル」から変更
//BOOL CViewCommander::Command_OPENINCLUDEFILE( BOOL bCheckOnly )
BOOL CViewCommander::Command_OPEN_HHPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca ヘッダ・ソースのコードを統合＆削除
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return m_pCommanderView->OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, source_ext, header_ext,
		_countof(source_ext), _countof(header_ext),
		LS(STR_ERR_CEDITVIEW_CMD08) );
}



/* C/C++ソースファイル オープン機能 */
//BOOL CViewCommander::Command_OPENCCPP( BOOL bCheckOnly )	//Feb. 10, 2001 JEPRO	コマンド名を若干変更
BOOL CViewCommander::Command_OPEN_CCPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca ヘッダ・ソースのコードを統合＆削除
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return m_pCommanderView->OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, header_ext, source_ext,
		_countof(header_ext), _countof(source_ext),
		LS(STR_ERR_CEDITVIEW_CMD09));
}



/* Oracle SQL*Plusをアクティブ表示 */
void CViewCommander::Command_ACTIVATE_SQLPLUS( void )
{
	HWND		hwndSQLPLUS;
	hwndSQLPLUS = ::FindWindow( _T("SqlplusWClass"), _T("Oracle SQL*Plus") );
	if( NULL == hwndSQLPLUS ){
		ErrorMessage( m_pCommanderView->GetHwnd(), LS( STR_SQLERR_ACTV_BUT_NOT_RUN ) );	//"Oracle SQL*Plusをアクティブ表示します。\n\n\nOracle SQL*Plusが起動されていません。\n"
		return;
	}
	/* Oracle SQL*Plusをアクティブにする */
	/* アクティブにする */
	ActivateFrameWindow( hwndSQLPLUS );
	return;
}



/* Oracle SQL*Plusで実行 */
void CViewCommander::Command_PLSQL_COMPILE_ON_SQLPLUS( void )
{
//	HGLOBAL		hgClip;
//	char*		pszClip;
	HWND		hwndSQLPLUS;
	int			nRet;
	BOOL		nBool;
	TCHAR		szPath[MAX_PATH + 2];
	BOOL		bResult;

	hwndSQLPLUS = ::FindWindow( _T("SqlplusWClass"), _T("Oracle SQL*Plus") );
	if( NULL == hwndSQLPLUS ){
		ErrorMessage( m_pCommanderView->GetHwnd(), LS( STR_SQLERR_EXEC_BUT_NOT_RUN ) );	//"Oracle SQL*Plusで実行します。\n\n\nOracle SQL*Plusが起動されていません。\n"
		return;
	}
	/* テキストが変更されている場合 */
	if( GetDocument()->m_cDocEditor.IsModified() ){
		nRet = ::MYMESSAGEBOX(
			m_pCommanderView->GetHwnd(),
			MB_YESNOCANCEL | MB_ICONEXCLAMATION,
			GSTR_APPNAME,
			LS(STR_ERR_CEDITVIEW_CMD18),
			GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ? GetDocument()->m_cDocFile.GetFilePath() : LS(STR_NO_TITLE1)
		);
		switch( nRet ){
		case IDYES:
			if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
				//nBool = HandleCommand( F_FILESAVE, true, 0, 0, 0, 0 );
				nBool = Command_FILESAVE();
			}else{
				//nBool = HandleCommand( F_FILESAVEAS_DIALOG, true, 0, 0, 0, 0 );
				nBool = Command_FILESAVEAS_DIALOG(NULL, CODE_NONE, EOL_NONE);
			}
			if( !nBool ){
				return;
			}
			break;
		case IDNO:
			return;
		case IDCANCEL:
		default:
			return;
		}
	}
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* ファイルパスに空白が含まれている場合はダブルクォーテーションで囲む */
		//	2003.10.20 MIK コード簡略化
		if( _tcschr( GetDocument()->m_cDocFile.GetFilePath(), TCODE::SPACE ) ? TRUE : FALSE ){
			auto_sprintf( szPath, _T("@\"%ts\"\r\n"), GetDocument()->m_cDocFile.GetFilePath() );
		}else{
			auto_sprintf( szPath, _T("@%ts\r\n"), GetDocument()->m_cDocFile.GetFilePath() );
		}
		/* クリップボードにデータを設定 */
		m_pCommanderView->MySetClipboardData( szPath, _tcslen( szPath ), false );

		/* Oracle SQL*Plusをアクティブにする */
		/* アクティブにする */
		ActivateFrameWindow( hwndSQLPLUS );

		/* Oracle SQL*Plusにペーストのコマンドを送る */
		DWORD_PTR	dwResult;
		bResult = ::SendMessageTimeout(
			hwndSQLPLUS,
			WM_COMMAND,
			MAKELONG( 201, 0 ),
			0,
			SMTO_ABORTIFHUNG | SMTO_NORMAL,
			3000,
			&dwResult
		);
		if( !bResult ){
			TopErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD20) );
		}
	}else{
		ErrorBeep();
		ErrorMessage( m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD21) );
		return;
	}
	return;
}



/* ブラウズ */
void CViewCommander::Command_BROWSE( void )
{
	if( !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		ErrorBeep();
		return;
	}
//	char	szURL[MAX_PATH + 64];
//	auto_sprintf( szURL, L"%ls", GetDocument()->m_cDocFile.GetFilePath() );
	/* URLを開く */
//	::ShellExecuteEx( NULL, L"open", szURL, NULL, NULL, SW_SHOW );

    SHELLEXECUTEINFO info; 
    info.cbSize =sizeof(info);
    info.fMask = 0;
    info.hwnd = NULL;
    info.lpVerb = NULL;
    info.lpFile = GetDocument()->m_cDocFile.GetFilePath();
    info.lpParameters = NULL;
    info.lpDirectory = NULL;
    info.nShow = SW_SHOWNORMAL;
    info.hInstApp = 0;
    info.lpIDList = NULL;
    info.lpClass = NULL;
    info.hkeyClass = 0; 
    info.dwHotKey = 0;
    info.hIcon =0;

	::ShellExecuteEx(&info);

	return;
}



/* ビューモード */
void CViewCommander::Command_VIEWMODE( void )
{
	//ビューモードを反転
	CAppMode::getInstance()->SetViewMode(!CAppMode::getInstance()->IsViewMode());

	// 排他制御の切り替え
	// ※ビューモード ON 時は排他制御 OFF、ビューモード OFF 時は排他制御 ON の仕様（>>data:5262）を即時反映する
	GetDocument()->m_cDocFileOperation.DoFileUnlock();	// ファイルの排他ロック解除
	GetDocument()->m_cDocLocker.CheckWritable(!CAppMode::getInstance()->IsViewMode());	// ファイル書込可能のチェック
	if( GetDocument()->m_cDocLocker.IsDocWritable() ){
		GetDocument()->m_cDocFileOperation.DoFileLock();	// ファイルの排他ロック
	}

	// 親ウィンドウのタイトルを更新
	GetEditWindow()->UpdateCaption();
}



/* ファイルのプロパティ */
void CViewCommander::Command_PROPERTY_FILE( void )
{
#ifdef _DEBUG
	{
		/* 全行データを返すテスト */
		wchar_t*	pDataAll;
		int		nDataAllLen;
		CRunningTimer cRunningTimer( "CViewCommander::Command_PROPERTY_FILE 全行データを返すテスト" );
		cRunningTimer.Reset();
		pDataAll = CDocReader(GetDocument()->m_cDocLineMgr).GetAllData( &nDataAllLen );
//		MYTRACE( _T("全データ取得             (%dバイト) 所要時間(ミリ秒) = %d\n"), nDataAllLen, cRunningTimer.Read() );
		free( pDataAll );
		pDataAll = NULL;
//		MYTRACE( _T("全データ取得のメモリ開放 (%dバイト) 所要時間(ミリ秒) = %d\n"), nDataAllLen, cRunningTimer.Read() );
	}
#endif


	CDlgProperty	cDlgProperty;
//	cDlgProperty.Create( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument() );
	cDlgProperty.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() );
	return;
}



void CViewCommander::Command_PROFILEMGR( void )
{
	CDlgProfileMgr profMgr;
	if( profMgr.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0 ) ){
		TCHAR szOpt[MAX_PATH+10];
		auto_sprintf( szOpt, _T("-PROF=\"%ts\""), profMgr.m_strProfileName.c_str() );
		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = _T("");
		sLoadInfo.eCharCode = CODE_DEFAULT;
		sLoadInfo.bViewMode = false;
		CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo, szOpt, false, NULL, false );
	}
}

/* 編集の全終了 */	// 2007.02.13 ryoji 追加
void CViewCommander::Command_EXITALLEDITORS( void )
{
	CControlTray::CloseAllEditor( TRUE, GetMainWindow(), TRUE, 0 );
	return;
}



/* サクラエディタの全終了 */	//Dec. 27, 2000 JEPRO 追加
void CViewCommander::Command_EXITALL( void )
{
	CControlTray::TerminateApplication( GetMainWindow() );	// 2006.12.25 ryoji 引数追加
	return;
}



/*!	@brief 編集中の内容を別名保存

	主に編集中の一時ファイル出力などの目的に使用する．
	現在開いているファイル(m_szFilePath)には影響しない．

	@retval	TRUE 正常終了
	@retval	FALSE ファイル作成に失敗

	@author	maru
	@date	2006.12.10 maru 新規作成
*/
BOOL CViewCommander::Command_PUTFILE(
	LPCWSTR		filename,	//!< [in] filename 出力ファイル名
	ECodeType	nCharCode,	//!< [in] nCharCode 文字コード指定
							//!<  @li CODE_xxxxxxxxxx:各種文字コード
							//!<  @li CODE_AUTODETECT:現在の文字コードを維持
	int			nFlgOpt		//!< [in] nFlgOpt 動作オプション
							//!<  @li 0x01:選択範囲を出力 (非選択状態でも空ファイルを出力する)
)
{
	BOOL		bResult = TRUE;
	ECodeType	nSaveCharCode = nCharCode;
	if(filename[0] == L'\0') {
		return FALSE;
	}

	if(nSaveCharCode == CODE_AUTODETECT) nSaveCharCode = GetDocument()->GetDocumentEncoding();

	//	2007.09.08 genta CEditDoc::FileWrite()にならって砂時計カーソル
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	std::auto_ptr<CCodeBase> pcSaveCode( CCodeFactory::CreateCodeBase(nSaveCharCode,0) );

	bool bBom = false;
	if (CCodeTypeName(nSaveCharCode).UseBom()) {
		bBom = GetDocument()->GetDocumentBomExist();
	}

	if(nFlgOpt & 0x01)
	{	/* 選択範囲を出力 */
		try
		{
			CBinaryOutputStream out(to_tchar(filename),true);

			// 選択範囲の取得 -> cMem
			CNativeW cMem;
			m_pCommanderView->GetSelectedDataSimple(cMem);

			// BOM追加
			CNativeW cMem2;
			const CNativeW* pConvBuffer;
			if( bBom ){
				CNativeW cmemBom;
				std::auto_ptr<CCodeBase> pcUtf16( CCodeFactory::CreateCodeBase(CODE_UNICODE,0) );
				pcUtf16->GetBom(cmemBom._GetMemory());
				cMem2.AppendNativeData(cmemBom);
				cMem2.AppendNativeData(cMem);
				cMem.Clear();
				pConvBuffer = &cMem2;
			}else{
				pConvBuffer = &cMem;
			}

			// 書き込み時のコード変換 -> cDst
			CMemory cDst;
			pcSaveCode->UnicodeToCode(*pConvBuffer, &cDst);

			//書込
			if( 0 < cDst.GetRawLength() )
				out.Write(cDst.GetRawPtr(),cDst.GetRawLength());
		}
		catch(CError_FileOpen)
		{
			WarningMessage(
				NULL,
				LS(STR_SAVEAGENT_OTHER_APP),
				to_tchar(filename)
			);
			bResult = FALSE;
		}
		catch(CError_FileWrite)
		{
			WarningMessage(
				NULL,
				LS(STR_ERR_DLGEDITVWCMDNW11)
			);
			bResult = FALSE;
		}
	}
	else {	/* ファイル全体を出力 */
		HWND		hwndProgress;
		CEditWnd*	pCEditWnd = GetEditWindow();

		if( NULL != pCEditWnd ){
			hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
		}else{
			hwndProgress = NULL;
		}
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}

		// 一時ファイル出力
		EConvertResult eRet = CWriteManager().WriteFile_From_CDocLineMgr(
			GetDocument()->m_cDocLineMgr,
			SSaveInfo(
				to_tchar(filename),
				nSaveCharCode,
				EOL_NONE,
				bBom
			)
		);
		bResult = (eRet != RESULT_FAILURE);

		if(hwndProgress) ::ShowWindow( hwndProgress, SW_HIDE );
	}
	return bResult;
}



/*!	@brief カーソル位置にファイルを挿入

	現在のカーソル位置に指定のファイルを読み込む．

	@param[in] filename 入力ファイル名
	@param[in] nCharCode 文字コード指定
		@li	CODE_xxxxxxxxxx:各種文字コード
		@li	CODE_AUTODETECT:前回文字コードもしくは自動判別の結果による
	@param[in] nFlgOpt 動作オプション（現在は未定義．0を指定のこと）

	@retval	TRUE 正常終了
	@retval	FALSE ファイルオープンに失敗

	@author	maru
	@date	2006.12.10 maru 新規作成
*/
BOOL CViewCommander::Command_INSFILE( LPCWSTR filename, ECodeType nCharCode, int nFlgOpt )
{
	CFileLoad	cfl(m_pCommanderView->m_pTypeData->m_encoding);
	CEol cEol;
	int			nLineNum = 0;

	CDlgCancel*	pcDlgCancel = NULL;
	HWND		hwndCancel = NULL;
	HWND		hwndProgress = NULL;
	int			nOldPercent = -1;
	BOOL		bResult = TRUE;

	if(filename[0] == L'\0') {
		return FALSE;
	}

	//	2007.09.08 genta CEditDoc::FileLoad()にならって砂時計カーソル
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	// 範囲選択中なら挿入後も選択状態にするため	/* 2007.04.29 maru */
	BOOL	bBeforeTextSelected = m_pCommanderView->GetSelectionInfo().IsTextSelected();
	CLayoutPoint ptFrom;
	if (bBeforeTextSelected){
		ptFrom = m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom();
	}


	ECodeType	nSaveCharCode = nCharCode;
	if(nSaveCharCode == CODE_AUTODETECT) {
		EditInfo    fi;
		const CMRUFile  cMRU;
		if ( cMRU.GetEditInfo( to_tchar(filename), &fi ) ){
				nSaveCharCode = fi.m_nCharCode;
		} else {
			nSaveCharCode = GetDocument()->GetDocumentEncoding();
		}
	}

	/* ここまできて文字コードが決定しないならどこかおかしい */
	if( !IsValidCodeOrCPType(nSaveCharCode) ) nSaveCharCode = CODE_SJIS;

	try{
		bool bBigFile;
#ifdef _WIN64
		bBigFile = true;
#else
		bBigFile = false;
#endif
		// ファイルを開く
		cfl.FileOpen( to_tchar(filename), bBigFile, nSaveCharCode, 0 );

		/* ファイルサイズが65KBを越えたら進捗ダイアログ表示 */
		if ( 0x10000 < cfl.GetFileSize() ) {
			pcDlgCancel = new CDlgCancel;
			if( NULL != ( hwndCancel = pcDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
				hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
				Progress_SetRange( hwndProgress, 0, 101 );
				Progress_SetPos( hwndProgress, 0);
			}
		}

		// ReadLineはファイルから 文字コード変換された1行を読み出します
		// エラー時はthrow CError_FileRead を投げます
		CNativeW cBuf;
		while( RESULT_FAILURE != cfl.ReadLine( &cBuf, &cEol ) ){

			const wchar_t*	pLine = cBuf.GetStringPtr();
			int			nLineLen = cBuf.GetStringLength();

			++nLineNum;
			Command_INSTEXT( false, pLine, CLogicInt(nLineLen), true);

			/* 進捗ダイアログ有無 */
			if( NULL == pcDlgCancel ){
				continue;
			}
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
				break;
			}
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				break;
			}
			if( 0 == ( nLineNum & 0xFF ) ){
				if( nOldPercent != cfl.GetPercent() ){
					Progress_SetPos( hwndProgress, cfl.GetPercent() + 1 );
					Progress_SetPos( hwndProgress, cfl.GetPercent() );
					nOldPercent = cfl.GetPercent();
				}
				m_pCommanderView->Redraw();
			}
		}
		// ファイルを明示的に閉じるが、ここで閉じないときはデストラクタで閉じている
		cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		WarningMessage( NULL, LS(STR_GREP_ERR_FILEOPEN), to_tchar(filename) );
		bResult = FALSE;
	}
	catch( CError_FileRead ){
		WarningMessage( NULL, LS(STR_ERR_DLGEDITVWCMDNW12) );
		bResult = FALSE;
	} // 例外処理終わり

	delete pcDlgCancel;

	if (bBeforeTextSelected){	// 挿入された部分を選択状態に
		m_pCommanderView->GetSelectionInfo().SetSelectArea(
			CLayoutRange(
				ptFrom,
				GetCaret().GetCaretLayoutPos()
				/*
				m_nCaretPosY, m_nCaretPosX
				*/
			)
		);
		m_pCommanderView->GetSelectionInfo().DrawSelectArea();
	}
	m_pCommanderView->Redraw();
	return bResult;
}
