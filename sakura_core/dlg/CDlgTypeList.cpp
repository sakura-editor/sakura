/*!	@file
	@brief ファイルタイプ一覧ダイアログ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, Stonee
	Copyright (C) 2002, MIK
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include "sakura_rc.h"
#include "dlg/CDlgTypeList.h"
#include "debug/Debug.h"
#include "func/Funccode.h"	//Stonee, 2001/03/12
#include "util/shell.h"

//内部使用定数
static const int PROP_TEMPCHANGE_FLAG = 0x10000;

// タイプ別設定一覧 CDlgTypeList.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12700
	IDC_BUTTON_TEMPCHANGE,	HIDC_TL_BUTTON_TEMPCHANGE,	//一時適用
	IDOK,					HIDOK_TL,					//設定
	IDCANCEL,				HIDCANCEL_TL,				//キャンセル
	IDC_BUTTON_HELP,		HIDC_TL_BUTTON_HELP,		//ヘルプ
	IDC_LIST_TYPES,			HIDC_TL_LIST_TYPES,			//リスト
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

/* モーダルダイアログの表示 */
int CDlgTypeList::DoModal( HINSTANCE hInstance, HWND hwndParent, SResult* psResult )
{
	int	nRet;
	m_nSettingType = psResult->cDocumentType;
	nRet = (int)CDialog::DoModal( hInstance, hwndParent, IDD_TYPELIST, NULL );
	if( -1 == nRet ){
		return FALSE;
	}
	else{
		//結果
		psResult->cDocumentType = CTypeConfig(nRet & ~PROP_TEMPCHANGE_FLAG);
		psResult->bTempChange   = ((nRet & PROP_TEMPCHANGE_FLAG) != 0);
		return TRUE;
	}
	return nRet;
}

BOOL CDlgTypeList::OnLbnDblclk( int wID )
{
	switch( wID ){
	case IDC_LIST_TYPES:
		//	Nov. 29, 2000	genta
		//	動作変更: 指定タイプの設定ダイアログ→一時的に別の設定を適用
		::EndDialog(
			GetHwnd(),
			::SendMessageAny(
				GetDlgItem( GetHwnd(), IDC_LIST_TYPES ),
				LB_GETCURSEL,
				0,
				0
			)
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	}
	return FALSE;
}

BOOL CDlgTypeList::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* 「タイプ別設定一覧」のヘルプ */
		//Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TYPE_LIST) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	//	Nov. 29, 2000	From Here	genta
	//	適用する型の一時的変更
	case IDC_BUTTON_TEMPCHANGE:
		::EndDialog(
			GetHwnd(),
			::SendMessageAny(
				GetDlgItem( GetHwnd(), IDC_LIST_TYPES ),
				LB_GETCURSEL,
				0,
				0
			)
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	//	Nov. 29, 2000	To Here
	case IDOK:
		::EndDialog( GetHwnd(), ::SendMessageAny( GetDlgItem( GetHwnd(), IDC_LIST_TYPES ), LB_GETCURSEL, (WPARAM)0, (LPARAM)0 ) );
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), -1 );
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );

}


/* ダイアログデータの設定 */
void CDlgTypeList::SetData( void )
{
	int		nIdx;
	HWND	hwndList;
	TCHAR	szText[130];
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	for( nIdx = 0; nIdx < MAX_TYPES; ++nIdx ){
		STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));
		if( 0 < _tcslen( types.m_szTypeExts ) ){		/* タイプ属性：拡張子リスト */
			auto_sprintf( szText, _T("%ts ( %ts )"),
				types.m_szTypeName,	/* タイプ属性：名称 */
				types.m_szTypeExts	/* タイプ属性：拡張子リスト */
			);
		}else{
			auto_sprintf( szText, _T("%ts"),
				types.m_szTypeName	/* タイプ属性：拡称 */
			);
		}
		::List_AddString( hwndList, szText );
	}
	::SendMessageAny( hwndList, LB_SETCURSEL, (WPARAM)m_nSettingType.GetIndex(), (LPARAM)0 );
	return;
}

//@@@ 2002.01.18 add start
LPVOID CDlgTypeList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


