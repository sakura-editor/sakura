/*!	@file
@brief CViewCommanderクラスのコマンド(カスタムメニュー)関数群

	2012/12/20	CViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2006, fon
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"


/* 右クリックメニュー */
void CViewCommander::Command_MENU_RBUTTON( void )
{
	int			nId;
	int			nLength;
//	HGLOBAL		hgClip;
//	char*		pszClip;
	int			i;
	/* ポップアップメニュー(右クリック) */
	nId = m_pCommanderView->CreatePopUpMenu_R();
	if( 0 == nId ){
		return;
	}
	switch( nId ){
	case IDM_COPYDICINFO:
		const TCHAR*	pszStr;
		pszStr = m_pCommanderView->m_cTipWnd.m_cInfo.GetStringPtr( &nLength );

		TCHAR*		pszWork;
		pszWork = new TCHAR[nLength + 1];
		auto_memcpy( pszWork, pszStr, nLength );
		pszWork[nLength] = _T('\0');

		// 見た目と同じように、\n を CR+LFへ変換する
		for( i = 0; i < nLength ; ++i){
			if( pszWork[i] == _T('\\') && pszWork[i + 1] == _T('n')){
				pszWork[i] =     WCODE::CR;
				pszWork[i + 1] = WCODE::LF;
			}
		}
		/* クリップボードにデータを設定 */
		m_pCommanderView->MySetClipboardData( pszWork, nLength, false );
		delete[] pszWork;

		break;

	case IDM_JUMPDICT:
		/* キーワード辞書ファイルを開く */
		if(m_pCommanderView->m_pTypeData->m_bUseKeyWordHelp){		/* キーワード辞書セレクトを使用する */	// 2006.04.10 fon
			//	Feb. 17, 2007 genta 相対パスを実行ファイル基準で開くように
			m_pCommanderView->TagJumpSub(
				m_pCommanderView->m_pTypeData->m_KeyHelpArr[m_pCommanderView->m_cTipWnd.m_nSearchDict].m_szPath,
				CMyPoint(1, m_pCommanderView->m_cTipWnd.m_nSearchLine),
				0,
				true
			);
		}
		break;

	default:
		/* コマンドコードによる処理振り分け */
//		HandleCommand( nId, true, 0, 0, 0, 0 );
		::PostMessageCmd( GetMainWindow(), WM_COMMAND, MAKELONG( nId, 0 ),  (LPARAM)NULL );
		break;
	}
	return;
}



/* カスタムメニュー表示 */
int CViewCommander::Command_CUSTMENU( int nMenuIdx )
{
	HMENU		hMenu;

	GetEditWindow()->GetMenuDrawer().ResetContents();

	//	Oct. 3, 2001 genta

	if( nMenuIdx < 0 || MAX_CUSTOM_MENU <= nMenuIdx ){
		return 0;
	}
	if( 0 == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx] ){
		return 0;
	}
	hMenu = ::CreatePopupMenu();
	return m_pCommanderView->CreatePopUpMenuSub( hMenu, nMenuIdx, NULL, KEYHELP_RMENU_NONE );
}
