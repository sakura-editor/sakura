//	$Id$
/*!	@file
	ファイルプロパティダイアログ
	
	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgProperty.h"
#include "debug.h"
#include "CEditDoc.h"
#include "etc_uty.h"
#include "funccode.h"		// Stonee, 2001/03/12

/* モーダルダイアログの表示 */
int CDlgProperty::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	return CDialog::DoModal( hInstance, hwndParent, IDD_PROPERTY_FILE, lParam );
}

BOOL CDlgProperty::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「ファイルのプロパティ」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROPERTY_FILE) );
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


/* ダイアログデータの設定 */
void CDlgProperty::SetData( void )
{
	CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
	CMemory			cmemProp;
//	char*			pWork;
	char			szWork[100];
	char*			pCodeNameArr[] = {
		"SJIS",
		"JIS",
		"EUC",
		"Unicode",
		"UTF-8",
		"UTF-7"
	};
	int				nCodeNameArrNum = sizeof( pCodeNameArr ) / sizeof( pCodeNameArr[0] );
	HANDLE			nFind;
	WIN32_FIND_DATA	wfd;
//	SYSTEMTIME		systimeU;
	SYSTEMTIME		systimeL;
	/* 共有データ構造体のアドレスを返す */
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );

	//	Aug. 16, 2000 genta	全角化
	cmemProp.AppendSz( "ファイル名  " );
	cmemProp.AppendSz( pCEditDoc->m_szFilePath );
	cmemProp.AppendSz( "\r\n" );

	cmemProp.AppendSz( "設定のタイプ  " );
	cmemProp.AppendSz( pCEditDoc->GetDocumentAttribute().m_szTypeName );
	cmemProp.AppendSz( "\r\n" );

	cmemProp.AppendSz( "文字コード  " );
	cmemProp.AppendSz( pCodeNameArr[pCEditDoc->m_nCharCode] );
	cmemProp.AppendSz( "\r\n" );

	wsprintf( szWork, "行数  %d行\r\n", pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendSz( szWork );

	wsprintf( szWork, "レイアウト行数  %d行\r\n", pCEditDoc->m_cLayoutMgr.GetLineCount() );
	cmemProp.AppendSz( szWork );

	if( pCEditDoc->m_bReadOnly ){
		cmemProp.AppendSz( "上書き禁止モードで開いています。\r\n" );
	}
	if( pCEditDoc->m_bIsModified ){
		cmemProp.AppendSz( "変更されています。\r\n" );
	}else{
		cmemProp.AppendSz( "変更されていません。\r\n" );
	}

	wsprintf( szWork, "\r\nコマンド実行回数    %d回\r\n", pCEditDoc->m_nCommandExecNum );
	cmemProp.AppendSz( szWork );

	wsprintf( szWork, "--ファイル情報-----------------\r\n", pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendSz( szWork );

	if( INVALID_HANDLE_VALUE != ( nFind = ::FindFirstFile( pCEditDoc->m_szFilePath, (WIN32_FIND_DATA*)&wfd ) ) ){
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
	int		nEUCMojiNum, nEUCCodeNum;
	int		nSJISMojiNum, nSJISCodeNum;
	int		nUNICODEMojiNum, nUNICODECodeNum;
	int		nJISMojiNum, nJISCodeNum;
	int		nUTF8MojiNum, nUTF8CodeNum;
	int		nUTF7MojiNum, nUTF7CodeNum;

	HFILE					hFile;
	HGLOBAL					hgData;
	const unsigned char*	pBuf;
	int						nBufLen;
	/* メモリ確保 & ファイル読み込み */
	hgData = NULL;
	hFile = _lopen( pCEditDoc->m_szFilePath, OF_READ );
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
	pBuf = (const unsigned char*)::GlobalLock( hgData );
	_lread( hFile, (void *)pBuf, nBufLen );
	_lclose( hFile );
	
	/* 
	||ファイルの日本語コードセット判別: Unicodeか？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_UNICODE( pBuf, nBufLen, &nUNICODEMojiNum, &nUNICODECodeNum ) ){
		if( nUNICODECodeNum!=0 && nUNICODEMojiNum != 0 ){
			wsprintf( szWork, "Unicodeコード検査：文字数%d  Unicode特有文字数%d (%d%%)\r\n", nUNICODEMojiNum, nUNICODECodeNum, nUNICODECodeNum*100/nUNICODEMojiNum );
		}else{
			wsprintf( szWork, "Unicodeコード検査：文字数%d  Unicode特有文字数%d (0%%)\r\n", nUNICODEMojiNum, nUNICODECodeNum );
		}
		cmemProp.AppendSz( szWork );
	}
	/* 
	||ファイルの日本語コードセット判別: EUCか？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_EUC( pBuf, nBufLen, &nEUCMojiNum, &nEUCCodeNum ) ){
		if( nEUCCodeNum!=0 && nEUCMojiNum != 0 ){
			wsprintf( szWork, "EUCコード検査：文字数%d  EUC特有文字数%d (%d%%)\r\n", nEUCMojiNum, nEUCCodeNum, nEUCCodeNum*100/nEUCMojiNum );
		}else{
			wsprintf( szWork, "EUCコード検査：文字数%d  EUC特有文字数%d (0%%)\r\n", nEUCMojiNum, nEUCCodeNum );
		}
		cmemProp.AppendSz( szWork );
	}
	/* 
	||ファイルの日本語コードセット判別: SJISか？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_SJIS( pBuf, nBufLen, &nSJISMojiNum, &nSJISCodeNum ) ){
		if( nSJISCodeNum!=0 && nSJISMojiNum != 0 ){
			wsprintf( szWork, "SJISコード検査：文字数%d  SJIS特有文字数%d (%d%%)\r\n", nSJISMojiNum, nSJISCodeNum, nSJISCodeNum*100/nSJISMojiNum );
		}else{
			wsprintf( szWork, "SJISコード検査：文字数%d  SJIS特有文字数%d (0%%)\r\n", nSJISMojiNum, nSJISCodeNum );
		}
		cmemProp.AppendSz( szWork );
	}


	/* 
	||ファイルの日本語コードセット判別: JISか？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_JIS( pBuf, nBufLen, &nJISMojiNum, &nJISCodeNum ) ){
		if( nJISCodeNum!=0 && nJISMojiNum != 0 ){
			wsprintf( szWork, "JISコード検査：文字数%d  JIS特有文字数%d (%d%%)\r\n", nJISMojiNum, nJISCodeNum, nJISCodeNum*100/nJISMojiNum );
		}else{
			wsprintf( szWork, "JISコード検査：文字数%d  JIS特有文字数%d (0%%)\r\n", nJISMojiNum, nJISCodeNum );
		}
		cmemProp.AppendSz( szWork );
	}

	/* 
	||ファイルの日本語コードセット判別: UTF-8Sか？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_UTF8( pBuf, nBufLen, &nUTF8MojiNum, &nUTF8CodeNum ) ){
		if( nUTF8CodeNum!=0 && nUTF8MojiNum != 0 ){
			wsprintf( szWork, "UTF-8コード検査：文字数%d  UTF-8特有文字数%d (%d%%)\r\n", nUTF8MojiNum, nUTF8CodeNum, nUTF8CodeNum*100/nUTF8MojiNum );
		}else{
			wsprintf( szWork, "UTF-8コード検査：文字数%d  UTF-8特有文字数%d (0%%)\r\n", nUTF8MojiNum, nUTF8CodeNum );
		}
		cmemProp.AppendSz( szWork );
	}

	/* 
	||ファイルの日本語コードセット判別: UTF-7Sか？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_UTF7( pBuf, nBufLen, &nUTF7MojiNum, &nUTF7CodeNum ) ){
		if( nUTF7CodeNum!=0 && nUTF7MojiNum != 0 ){
			wsprintf( szWork, "UTF-7コード検査：文字数%d  UTF-7特有文字数%d (%d%%)\r\n", nUTF7MojiNum, nUTF7CodeNum, nUTF7CodeNum*100/nUTF7MojiNum );
		}else{
			wsprintf( szWork, "UTF-7コード検査：文字数%d  UTF-7特有文字数%d (0%%)\r\n", nUTF7MojiNum, nUTF7CodeNum );
		}
		cmemProp.AppendSz( szWork );
	}
	if( NULL != hgData ){
		::GlobalUnlock( hgData );
		::GlobalFree( hgData );
		hgData = NULL;
	}
end_of_CodeTest:;
#endif //ifdef _DEBUG/////////////////////////////////////////////////////
	::SetDlgItemText( m_hWnd, IDC_EDIT1, cmemProp.GetPtr( NULL ) );

	return;
}


/*[EOF]*/
