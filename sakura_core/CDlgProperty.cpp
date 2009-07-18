/*!	@file
	@brief ファイルプロパティダイアログ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee
	Copyright (C) 2002, Moca, MIK, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "sakura_rc.h"
#include "CDlgProperty.h"
#include "debug.h"
#include "CEditDoc.h"
#include "etc_uty.h"
#include "funccode.h"		// Stonee, 2001/03/12
#include "global.h"		// Moca, 2002/05/26
#include "charcode.h"	// rastiv, 2006/06/28

// プロパティ CDlgProperty.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12600
	IDOK,					HIDOK_PROP,
	IDCANCEL,				HIDCANCEL_PROP,
	IDC_BUTTON_HELP,		HIDC_PROP_BUTTON_HELP,
	IDC_EDIT1,				HIDC_PROP_EDIT1,
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
		MyWinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROPERTY_FILE) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
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
//	char*			pWork;
//	char			szWork[100];
	char			szWork[500];

//	gm_pszCodeNameArr_1[] を参照するように変更 Moca, 2002/05/26
#if 0
	char*			pCodeNameArr[] = {
		"SJIS",
		"JIS",
		"EUC",
		"Unicode",
		"UTF-8",
		"UTF-7"
	};
#endif
//	1回も使われていない Moca
//	int				nCodeNameArrNum = sizeof( pCodeNameArr ) / sizeof( pCodeNameArr[0] );
	HANDLE			nFind;
	WIN32_FIND_DATA	wfd;
	SYSTEMTIME		systimeL;
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();

	//	Aug. 16, 2000 genta	全角化
	cmemProp.AppendSz( "ファイル名  " );
	cmemProp.AppendSz( pCEditDoc->GetFilePath() );
	cmemProp.AppendSz( "\r\n" );

	cmemProp.AppendSz( "設定のタイプ  " );
	cmemProp.AppendSz( pCEditDoc->GetDocumentAttribute().m_szTypeName );
	cmemProp.AppendSz( "\r\n" );

	cmemProp.AppendSz( "文字コード  " );
	cmemProp.AppendSz( gm_pszCodeNameArr_1[pCEditDoc->m_nCharCode] );
	cmemProp.AppendSz( "\r\n" );

	wsprintf( szWork, "行数  %d行\r\n", pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendSz( szWork );

	wsprintf( szWork, "レイアウト行数  %d行\r\n", pCEditDoc->m_cLayoutMgr.GetLineCount() );
	cmemProp.AppendSz( szWork );

	if( pCEditDoc->m_bReadOnly ){
		cmemProp.AppendSz( "読み取り専用モードで開いています。\r\n" );	// 2009.04.11 ryoji 「上書き禁止モード」→「読み取り専用モード」
	}
	if( pCEditDoc->IsModified() ){
		cmemProp.AppendSz( "変更されています。\r\n" );
	}else{
		cmemProp.AppendSz( "変更されていません。\r\n" );
	}

	wsprintf( szWork, "\r\nコマンド実行回数    %d回\r\n", pCEditDoc->m_nCommandExecNum );
	cmemProp.AppendSz( szWork );

	wsprintf( szWork, "--ファイル情報-----------------\r\n", pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendSz( szWork );

	if( INVALID_HANDLE_VALUE != ( nFind = ::FindFirstFile( pCEditDoc->GetFilePath(), (WIN32_FIND_DATA*)&wfd ) ) ){
		if( pCEditDoc->m_hLockedFile ){
			if( m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_DENY_WRITE ){
				wsprintf( szWork, "あなたはこのファイルを、他プロセスからの上書き禁止モードでロックしています。\r\n" );
			}else
			if( m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_EXCLUSIVE ){
				wsprintf( szWork, "あなたはこのファイルを、他プロセスからの読み書き禁止モードでロックしています。\r\n" );
			}else{
				wsprintf( szWork, "あなたはこのファイルをロックしています。\r\n" );
			}
			cmemProp.AppendSz( szWork );
		}else{
			wsprintf( szWork, "あなたはこのファイルをロックしていません。\r\n" );
			cmemProp.AppendSz( szWork );
		}

		wsprintf( szWork, "ファイル属性  ", pCEditDoc->m_cDocLineMgr.GetLineCount() );
		cmemProp.AppendSz( szWork );
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ){
			cmemProp.AppendSz( "/アーカイブ" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ){
			cmemProp.AppendSz( "/圧縮" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			cmemProp.AppendSz( "/フォルダ" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ){
			cmemProp.AppendSz( "/隠し" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ){
			cmemProp.AppendSz( "/ノーマル" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ){
			cmemProp.AppendSz( "/オフライン" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY ){
			cmemProp.AppendSz( "/読み取り専用" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ){
			cmemProp.AppendSz( "/システム" );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ){
			cmemProp.AppendSz( "/テンポラリ" );
		}
		cmemProp.AppendSz( "\r\n" );


		cmemProp.AppendSz( "作成日時  " );
		::FileTimeToLocalFileTime( &wfd.ftCreationTime, &wfd.ftCreationTime );
		::FileTimeToSystemTime( &wfd.ftCreationTime, &systimeL );
		wsprintf( szWork, "%d年%d月%d日 %02d:%02d:%02d",
			systimeL.wYear,
			systimeL.wMonth,
			systimeL.wDay,
			systimeL.wHour,
			systimeL.wMinute,
			systimeL.wSecond
		);
		cmemProp.AppendSz( szWork );
		cmemProp.AppendSz( "\r\n" );

		cmemProp.AppendSz( "更新日時  " );
		::FileTimeToLocalFileTime( &wfd.ftLastWriteTime, &wfd.ftLastWriteTime );
		::FileTimeToSystemTime( &wfd.ftLastWriteTime, &systimeL );
		wsprintf( szWork, "%d年%d月%d日 %02d:%02d:%02d",
			systimeL.wYear,
			systimeL.wMonth,
			systimeL.wDay,
			systimeL.wHour,
			systimeL.wMinute,
			systimeL.wSecond
		);
		cmemProp.AppendSz( szWork );
		cmemProp.AppendSz( "\r\n" );


		cmemProp.AppendSz( "アクセス日  " );
		::FileTimeToLocalFileTime( &wfd.ftLastAccessTime, &wfd.ftLastAccessTime );
		::FileTimeToSystemTime( &wfd.ftLastAccessTime, &systimeL );
		wsprintf( szWork, "%d年%d月%d日",
			systimeL.wYear,
			systimeL.wMonth,
			systimeL.wDay
		);
		cmemProp.AppendSz( szWork );
		cmemProp.AppendSz( "\r\n" );

		wsprintf( szWork, "MS-DOSファイル名  %s\r\n", wfd.cAlternateFileName );
		cmemProp.AppendSz( szWork );

		wsprintf( szWork, "ファイルサイズ  %d バイト\r\n", wfd.nFileSizeLow );
		cmemProp.AppendSz( szWork );

		::FindClose( nFind );
	}




#ifdef _DEBUG/////////////////////////////////////////////////////
	MBCODE_INFO		mbci_tmp;
	UNICODE_INFO	uci_tmp;
	
	HFILE					hFile;
	HGLOBAL					hgData;
//	const unsigned char*	pBuf;
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
//	pBuf = (const unsigned char*)::GlobalLock( hgData );
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
	cmemProp.AppendSz( szWork );
	
	/*
	||ファイルの日本語コードセット判別: EUCか？
	*/
	Charcode::GetEncdInf_EucJp( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "EUCJPコード検査：特有バイト数=%d  ポイント数=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendSz( szWork );
	
	/*
	||ファイルの日本語コードセット判別: SJISか？
	*/
	Charcode::GetEncdInf_SJis( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "SJISコード検査：特有バイト数=%d  ポイント数=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendSz( szWork );
	
	/*
	||ファイルの日本語コードセット判別: JISか？
	*/
	Charcode::GetEncdInf_Jis( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "JISコード検査：特有バイト数=%d  ポイント数=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendSz( szWork );

	/*
	||ファイルの日本語コードセット判別: UTF-8Sか？
	*/
	Charcode::GetEncdInf_Utf8( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "UTF-8コード検査：特有バイト数=%d  ポイント数=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendSz( szWork );

	/*
	||ファイルの日本語コードセット判別: UTF-7Sか？
	*/
	Charcode::GetEncdInf_Utf7( pBuf, nBufLen, &mbci_tmp );
	wsprintf( szWork, "UTF-7コード検査：特有バイト数=%d  ポイント数=%d\r\n"
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendSz( szWork );
// To Here rastiv 2006.12.17

	if( NULL != hgData ){
		::GlobalUnlock( hgData );
		::GlobalFree( hgData );
		hgData = NULL;
	}
end_of_CodeTest:;
#endif //ifdef _DEBUG/////////////////////////////////////////////////////
	::SetDlgItemText( m_hWnd, IDC_EDIT1, cmemProp.GetPtr() );

	return;
}

//@@@ 2002.01.18 add start
LPVOID CDlgProperty::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
