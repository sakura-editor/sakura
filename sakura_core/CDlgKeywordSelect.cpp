/*! @file
	@brief 強調キーワード選択ダイアログ

	@author MIK
	@date 2005/01/13 作成
*/
/*
	Copyright (C) 2005, MIK

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

#include "stdafx.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "CDlgKeywordSelect.h"

static const DWORD p_helpids[] = {
	0, 0
};

static const int keyword_select_target_combo[ KEYWORD_SELECT_NUM ] = {
	IDC_COMBO1,
	IDC_COMBO2,
	IDC_COMBO3,
	IDC_COMBO4,
	IDC_COMBO5,
	IDC_COMBO6,
	IDC_COMBO7,
	IDC_COMBO8,
	IDC_COMBO9,
	IDC_COMBO10
};


CDlgKeywordSelect::CDlgKeywordSelect()
{
	m_pCKeyWordSetMgr = &(m_pShareData->m_CKeyWordSetMgr);

	return;
}

CDlgKeywordSelect::~CDlgKeywordSelect()
{
	return;
}


/* !モーダルダイアログの表示
*/
int CDlgKeywordSelect::DoModal( HINSTANCE hInstance, HWND hwndParent, int* pnSet )
{
	int i;

	for( i = 0; i < KEYWORD_SELECT_NUM; i++ ){
		m_nSet[ i ] = pnSet[ i ];
	}

	(void)CDialog::DoModal( hInstance, hwndParent, IDD_DIALOG_KEYWORD_SELECT, NULL );

	for( i = 0; i < KEYWORD_SELECT_NUM; i++ ){
		pnSet[ i ] = m_nSet[ i ];
	}

	return TRUE;
}

/*! 初期化処理
*/
BOOL CDlgKeywordSelect::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hwndDlg;

	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}


BOOL CDlgKeywordSelect::OnBnClicked( int wID )
{
	switch( wID ){
	case IDOK:
		GetData();
		break;
	case IDCANCEL:
		break;
	}
	return CDialog::OnBnClicked( wID );
}

/*! ダイアログデータの設定
*/
void CDlgKeywordSelect::SetData( void )
{
	HWND	hwndCombo;
	int		i;
	int		index;

	for( index = 0; index < KEYWORD_SELECT_NUM; index++ )
	{
		hwndCombo = ::GetDlgItem( m_hWnd, keyword_select_target_combo[ index ] );

		/* コンボボックスを空にする */
		::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
		
		/* 一行目は空白 */
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T(" ") );

		if( m_pCKeyWordSetMgr->m_nKeyWordSetNum > 0 )
		{
			for( i = 0; i < m_pCKeyWordSetMgr->m_nKeyWordSetNum; i++ )
			{
				::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)m_pCKeyWordSetMgr->GetTypeName( i ) );
			}

			if( -1 == m_nSet[ index ] )
			{
				/* セット名コンボボックスのデフォルト選択 */
				::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)0, 0 );
			}
			else
			{
				/* セット名コンボボックスのデフォルト選択 */
				::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)(m_nSet[ index ] + 1), 0 );
			}
		}
	}
}


/*! ダイアログデータの設定
*/
int CDlgKeywordSelect::GetData( void )
{
	HWND	hwndCombo;
	int		index;
	int		n;

	for( index = 0; index < KEYWORD_SELECT_NUM; index++ )
	{
		hwndCombo = ::GetDlgItem( m_hWnd, keyword_select_target_combo[ index ] );

		n = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
		if( CB_ERR == n || 0 == n )
		{
			m_nSet[ index ] = -1;
		}
		else
		{
			m_nSet[ index ] = n - 1;
		}
	}

	return TRUE;
}

LPVOID CDlgKeywordSelect::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

/*[EOF]*/
