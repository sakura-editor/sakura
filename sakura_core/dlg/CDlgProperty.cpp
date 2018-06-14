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
#include "dlg/CDlgProperty.h"
#include "doc/CEditDoc.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "_main/global.h"		// Moca, 2002/05/26
#include "_main/CAppMode.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "charset/charcode.h"	// rastiv, 2006/06/28
#include "charset/CCodePage.h"
#include "charset/CESI.h"
#include "io/CBinaryStream.h"
#include "util/shell.h"
#include "sakura_rc.h"

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
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROPERTY_FILE) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	case IDOK:			/* 下検索 */
		/* ダイアログデータの取得 */
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
//	case IDCANCEL:							// 未使用 del 2008/7/4 Uchi
//		::EndDialog( GetHwnd(), FALSE );
//		return TRUE;
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
	CNativeT		cmemProp;
	TCHAR			szWork[500];

	HANDLE			nFind;
	WIN32_FIND_DATA	wfd;

	//	Aug. 16, 2000 genta	全角化
	cmemProp.AppendString( LS(STR_DLGFLPROP_FILENAME) );
	cmemProp.AppendString( pCEditDoc->m_cDocFile.GetFilePath() );
	cmemProp.AppendString( _T("\r\n") );

	cmemProp.AppendString( LS(STR_DLGFLPROP_FILETYPE) );
	cmemProp.AppendString( pCEditDoc->m_cDocType.GetDocumentAttribute().m_szTypeName );
	cmemProp.AppendString( _T("\r\n") );

	cmemProp.AppendString( LS(STR_DLGFLPROP_ENCODING) );
	{
		TCHAR szCpName[100];
		CCodePage::GetNameNormal(szCpName, pCEditDoc->GetDocumentEncoding());
		cmemProp.AppendString( szCpName );
	}
	//	From Here  2008/4/27 Uchi
	if (pCEditDoc->GetDocumentBomExist()) {
		cmemProp.AppendString( LS(STR_DLGFLPROP_WITH_BOM) );
	}
	//	To Here  2008/4/27 Uchi
	cmemProp.AppendString( _T("\r\n") );

	auto_sprintf( szWork, LS(STR_DLGFLPROP_LINE_COUNT), pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	auto_sprintf( szWork, LS(STR_DLGFLPROP_LAYOUT_LINE), pCEditDoc->m_cLayoutMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	if( CAppMode::getInstance()->IsViewMode() ){
		cmemProp.AppendString( LS(STR_DLGFLPROP_VIEW_MODE) );	// 2009.04.11 ryoji 「上書き禁止モード」→「ビューモード」
	}
	if( pCEditDoc->m_cDocEditor.IsModified() ){
		cmemProp.AppendString( LS(STR_DLGFLPROP_MODIFIED) );
	}else{
		cmemProp.AppendString( LS(STR_DLGFLPROP_NOT_MODIFIED) );
	}

	auto_sprintf( szWork, LS(STR_DLGFLPROP_CMD_COUNT), pCEditDoc->m_nCommandExecNum );
	cmemProp.AppendString( szWork );

	auto_sprintf( szWork, LS(STR_DLGFLPROP_FILE_INFO), pCEditDoc->m_cDocLineMgr.GetLineCount() );
	cmemProp.AppendString( szWork );

	if( INVALID_HANDLE_VALUE != ( nFind = ::FindFirstFile( pCEditDoc->m_cDocFile.GetFilePath(), &wfd ) ) ){
		if( pCEditDoc->m_cDocFile.IsFileLocking() ){
			if( m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_WRITE ){
				auto_sprintf( szWork, LS(STR_DLGFLPROP_W_LOCK) );
			}
			else if( m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_READWRITE ){
				auto_sprintf( szWork, LS(STR_DLGFLPROP_RW_LOCK) );
			}
			else{
				auto_sprintf( szWork, LS(STR_DLGFLPROP_LOCK) );
			}
			cmemProp.AppendString( szWork );
		}
		else{
			auto_sprintf( szWork, LS(STR_DLGFLPROP_NOT_LOCK) );
			cmemProp.AppendString( szWork );
		}

		auto_sprintf( szWork, LS(STR_DLGFLPROP_ATTRIBUTES), pCEditDoc->m_cDocLineMgr.GetLineCount() );
		cmemProp.AppendString( szWork );
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE ){
			cmemProp.AppendString( LS(STR_DLGFLPROP_AT_ARCHIVE) );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED ){
			cmemProp.AppendString( LS(STR_DLGFLPROP_AT_COMPRESS) );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			cmemProp.AppendString( LS(STR_DLGFLPROP_AT_FOLDER) );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ){
			cmemProp.AppendString( LS(STR_DLGFLPROP_AT_HIDDEN) );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ){
			cmemProp.AppendString( LS(STR_DLGFLPROP_AT_NORMAL) );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE ){
			cmemProp.AppendString( LS(STR_DLGFLPROP_AT_OFFLINE) );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY ){
			cmemProp.AppendString( LS(STR_DLGFLPROP_AT_READONLY) );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ){
			cmemProp.AppendString( LS(STR_DLGFLPROP_AT_SYSTEM) );
		}
		if( wfd.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ){
			cmemProp.AppendString( LS(STR_DLGFLPROP_AT_TEMP) );
		}
		cmemProp.AppendString( _T("\r\n") );


		cmemProp.AppendString( LS(STR_DLGFLPROP_CREATE_DT) );
		CFileTime ctimeCreation = wfd.ftCreationTime;
		auto_sprintf( szWork, LS(STR_DLGFLPROP_YMDHMS),
			ctimeCreation->wYear,
			ctimeCreation->wMonth,
			ctimeCreation->wDay,
			ctimeCreation->wHour,
			ctimeCreation->wMinute,
			ctimeCreation->wSecond
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );

		cmemProp.AppendString( LS(STR_DLGFLPROP_UPDATE_DT) );
		CFileTime ctimeLastWrite = wfd.ftLastWriteTime;
		auto_sprintf( szWork, LS(STR_DLGFLPROP_YMDHMS),
			ctimeLastWrite->wYear,
			ctimeLastWrite->wMonth,
			ctimeLastWrite->wDay,
			ctimeLastWrite->wHour,
			ctimeLastWrite->wMinute,
			ctimeLastWrite->wSecond
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );


		cmemProp.AppendString( LS(STR_DLGFLPROP_ACCESS_DT) );
		CFileTime ctimeLastAccess = wfd.ftLastAccessTime;
		auto_sprintf( szWork, LS(STR_DLGFLPROP_YMDHMS),
			ctimeLastAccess->wYear,
			ctimeLastAccess->wMonth,
			ctimeLastAccess->wDay,
			ctimeLastAccess->wHour,
			ctimeLastAccess->wMinute,
			ctimeLastAccess->wSecond
		);
		cmemProp.AppendString( szWork );
		cmemProp.AppendString( _T("\r\n") );

		auto_sprintf( szWork, LS(STR_DLGFLPROP_DOS_NAME), wfd.cAlternateFileName );
		cmemProp.AppendString( szWork );

		auto_sprintf( szWork, LS(STR_DLGFLPROP_FILE_SIZE), wfd.nFileSizeLow );
		cmemProp.AppendString( szWork );

		::FindClose( nFind );
	}




#ifdef _DEBUG/////////////////////////////////////////////////////
	HGLOBAL					hgData;
	char*					pBuf;
	int						nBufLen;
	CNativeT				ctext;
	/* メモリ確保 & ファイル読み込み */
	hgData = NULL;
	CBinaryInputStream in(pCEditDoc->m_cDocFile.GetFilePath());
	if(!in){
		goto end_of_CodeTest;
	}
	nBufLen = in.GetLength();
	if( nBufLen > CheckKanjiCode_MAXREADLENGTH ){
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}
	hgData = ::GlobalAlloc( GHND, nBufLen + 1 );
	if( NULL == hgData ){
		in.Close();
		goto end_of_CodeTest;
	}
	pBuf = GlobalLockChar( hgData );
	in.Read( pBuf, nBufLen );
	in.Close();

	//CESIのデバッグ情報
	CESI::GetDebugInfo(pBuf,nBufLen,&ctext);
	cmemProp.AppendNativeData(ctext);

	if( NULL != hgData ){
		::GlobalUnlock( hgData );
		::GlobalFree( hgData );
		hgData = NULL;
	}
end_of_CodeTest:;
#endif //ifdef _DEBUG/////////////////////////////////////////////////////
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_PROPERTY, cmemProp.GetStringPtr() );

	return;
}

//@@@ 2002.01.18 add start
LPVOID CDlgProperty::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end
