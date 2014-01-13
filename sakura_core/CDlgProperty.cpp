/*!	@file
	@brief ファイルプロパティダイアログ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee
	Copyright (C) 2002, Moca, MIK, YAZAKI
	Copyright (C) 2006, ryoji, rastiv
	Copyright (C) 2008, Uchi
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "sakura_rc.h"
#include "CDlgProperty.h"
#include "Debug.h"
#include "CEditDoc.h"
#include "etc_uty.h"
#include "shell.h"
#include "Funccode.h"		// Stonee, 2001/03/12
#include "global.h"		// Moca, 2002/05/26
#include "charcode.h"	// rastiv, 2006/06/28

// プロパティ CDlgProperty.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12600
	IDOK,					HIDOK_PROP,
//	IDCANCEL,				HIDCANCEL_PROP,			// 未使用 del 2008/7/4 Uchi
	IDC_BUTTON_HELP,		HIDC_PROP_BUTTON_HELP,
	IDC_EDIT_PROPERTY,		HIDC_PROP_EDIT1,		// IDC_EDIT1->IDC_EDIT_PROPERTY	2008/7/3 Uchi
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

/* モーダルダイアログの表示 */
int CDlgProperty::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_PROPERTY_FILE, lParam );
}

BOOL CDlgProperty::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「ファイルのプロパティ」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp( m_hWnd, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROPERTY_FILE) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	case IDOK:			/* 下検索 */
		/* ダイアログデータの取得 */
		::EndDialog( m_hWnd, FALSE );
		return TRUE;
	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}


/*! ダイアログデータの設定

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
void CDlgProperty::SetData( void )
{
	CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
	CMemory			cmemProp;
	TCHAR			szWork[500];

	HANDLE			nFind;
	WIN32_FIND_DATA	wfd;

	//	Aug. 16, 2000 genta	全角化
	cmemProp.AppendString( _T("ファイル名  ") );
	cmemProp.AppendString( pCEditDoc->GetFilePath() );
	cmemProp.AppendString( _T("\r\n") );

	cmemProp.AppendString( _T("設定のタイプ  ") );
	cmemProp.AppendString( pCEditDoc->GetDocumentAttribute().m_szTypeName );
	cmemProp.AppendString( _T("\r\n") );

	cmemProp.AppendString( _T("文字コード  ") );
	cmemProp.AppendString( gm_pszCodeNameArr_1[pCEditDoc->m_nCharCode] );
	cmemProp.AppendString( _T("\r\n") );

	wsprintf( szWork, _T("行数  %d行\r\n"), pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	wsprintf( szWork, _T("レイアウト行数  %d行\r\n"), pCEditDoc->m_cLayoutMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	if( pCEditDoc->m_bReadOnly ){
		cmemProp.AppendString( _T("読み取り専用モードで開いています。\r\n") );	// 2009.04.11 ryoji 「上書き禁止モード」→「読み取り専用モード」
	}
	if( pCEditDoc->IsModified() ){
		cmemProp.AppendString( _T("変更されています。\r\n") );
	}else{
		cmemProp.AppendString( _T("変更されていません。\r\n") );
	}

	wsprintf( szWork, _T("\r\nコマンド実行回数    %d回\r\n"), pCEditDoc->m_nCommandExecNum );
	cmemProp.AppendString( szWork );

	wsprintf( szWork, _T("--ファイル情報-----------------\r\n"), pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	if( INVALID_HANDLE_VALUE != ( nFind = ::FindFirstFile( pCEditDoc->GetFilePath(), (WIN32_FIND_DATA*)&wfd ) ) ){
		if( INVALID_HANDLE_VALUE != pCEditDoc->m_hLockedFile ){
			if( m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_WRITE ){
				wsprintf( szWork, _T("あなたはこのファイルを、他プロセスからの上書き禁止モードでロックしています。\r\n") );
			}
			else if( m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_READWRITE ){
				wsprintf( szWork, _T("あなたはこのファイルを、他プロセスからの読み書き禁止モードでロックしています。\r\n") );
			}
			else{
				wsprintf( szWork, _T("あなたはこのファイルをロックしています。\r\n") );
			}
			cmemProp.AppendString( szWork );
		}
		else{
			wsprintf( szWork, _T("あなたはこのファイルをロックしていません。\r\n") );
			cmemProp.AppendString( szWork );
		}

		wsprintf( szWork, _T("ファイル属性  "), pCEditDoc->m_cDocLineMgr.GetLineCount() );
		cmemProp.AppendString( szWork );
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ){
			cmemProp.AppendString( _T("/アーカイブ") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ){
			cmemProp.AppendString( _T("/圧縮") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			cmemProp.AppendString( _T("/フォルダ") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ){
			cmemProp.AppendString( _T("/隠し") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ){
			cmemProp.AppendString( _T("/ノーマル") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ){
			cmemProp.AppendString( _T("/オフライン") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY ){
			cmemProp.AppendString( _T("/読み取り専用") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ){
			cmemProp.AppendString( _T("/システム") );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ){
			cmemProp.AppendString( _T("/テンポラリ") );
		}
		cmemProp.AppendString( _T("\r\n") );


		cmemProp.AppendString( _T("作成日時  ") );
		CFileTime ctimeCreation = wfd.ftCreationTime;
		wsprintf( szWork, _T("%d年%d月%d日 %02d:%02d:%02d"),
			ctimeCreation->wYear,
			ctimeCreation->wMonth,
			ctimeCreation->wDay,
			ctimeCreation->wHour,
			ctimeCreation->wMinute,
			ctimeCreation->wSecond
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );

		cmemProp.AppendString( _T("更新日時  ") );
		CFileTime ctimeLastWrite = wfd.ftLastWriteTime;
		wsprintf( szWork, _T("%d年%d月%d日 %02d:%02d:%02d"),
			ctimeLastWrite->wYear,
			ctimeLastWrite->wMonth,
			ctimeLastWrite->wDay,
			ctimeLastWrite->wHour,
			ctimeLastWrite->wMinute,
			ctimeLastWrite->wSecond
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );


		cmemProp.AppendString( _T("アクセス日  ") );
		CFileTime ctimeLastAccess = wfd.ftLastAccessTime;
		wsprintf( szWork, _T("%d年%d月%d日"),
			ctimeLastAccess->wYear,
			ctimeLastAccess->wMonth,
			ctimeLastAccess->wDay
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );

		wsprintf( szWork, _T("MS-DOSファイル名  %s\r\n"), wfd.cAlternateFileName );
		cmemProp.AppendString( szWork );

		wsprintf( szWork, _T("ファイルサイズ  %d バイト\r\n"), wfd.nFileSizeLow );
		cmemProp.AppendString( szWork );

		::FindClose( nFind );
	}




#ifdef _DEBUG/////////////////////////////////////////////////////
	MBCODE_INFO		mbci_tmp;
	UNICODE_INFO	uci_tmp;
	
	HFILE					hFile;
	HGLOBAL					hgData;
	const char*				pBuf;
	int						nBufLen;
	/* メモリ確保 & ファイル読み込み */
	hgData = NULL;
	hFile = _lopen( pCEditDoc->GetFilePath(), OF_READ );
	if( HFILE_ERROR == hFile ){
		goto end_of_CodeTest;
	}
	nBufLen = _llseek( hFile, 0, FILE_END );
	_llseek( hFile, 0, FILE_BEGIN );
	if( nBufLen > CheckKanjiCode_MAXREADLENGTH ){
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}
	hgData = ::GlobalAlloc( GHND, nBufLen + 1 );
	if( NULL == hgData ){
		_lclose( hFile );
		goto end_of_CodeTest;
	}
	pBuf = (const char*)::GlobalLock( hgData );
	_lread( hFile, (void *)pBuf, nBufLen );
	_lclose( hFile );

// From Here  2006.12.17  rastiv
	/*
	||ファイルの日本語コードセット判別: Unicodeか？
	*/
	/*
	||ファイルの日本語コードセット判別: UnicodeBEか？
	*/
	Charcode::GetEncdInf_Uni( pBuf, nBufLen, &uci_tmp );
	wsprintf( szWork, "Unicodeコード調査：改行バイト数=%d  BE改行バイト数=%d ASCII改行バイト数=%d\r\n"
		, uci_tmp.Uni.nCRorLF, uci_tmp.UniBe.nCRorLF, uci_tmp.nCRorLF_ascii );
	cmemProp.AppendString( szWork );
	
	/*
	||ファイルの日本語コードセット判別: EUCか？
	*/
	Charcode::GetEncdInf_EucJp( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "EUCJPコード検査：特有バイト数=%d  ポイント数=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );
	
	/*
	||ファイルの日本語コードセット判別: SJISか？
	*/
	Charcode::GetEncdInf_SJis( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "SJISコード検査：特有バイト数=%d  ポイント数=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );
	
	/*
	||ファイルの日本語コードセット判別: JISか？
	*/
	Charcode::GetEncdInf_Jis( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "JISコード検査：特有バイト数=%d  ポイント数=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );

	/*
	||ファイルの日本語コードセット判別: UTF-8Sか？
	*/
	Charcode::GetEncdInf_Utf8( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "UTF-8コード検査：特有バイト数=%d  ポイント数=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );

	/*
	||ファイルの日本語コードセット判別: UTF-7Sか？
	*/
	Charcode::GetEncdInf_Utf7( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "UTF-7コード検査：特有バイト数=%d  ポイント数=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );
// To Here rastiv 2006.12.17

	if( NULL != hgData ){
		::GlobalUnlock( hgData );
		::GlobalFree( hgData );
		hgData = NULL;
	}
end_of_CodeTest:;
#endif //ifdef _DEBUG/////////////////////////////////////////////////////
	::SetDlgItemText( m_hWnd, IDC_EDIT_PROPERTY, cmemProp.GetStringPtr() );

	return;
}

//@@@ 2002.01.18 add start
LPVOID CDlgProperty::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
