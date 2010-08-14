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
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "types/CType.h" // use CDlgTypeList定義

#include "typeprop/CDlgTypeList.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/24 Uchi
#include "env/CDocTypeManager.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

//内部使用定数
static const int PROP_TEMPCHANGE_FLAG = 0x10000;

// タイプ別設定一覧 CDlgTypeList.cpp	//@@@ 2002.01.07 add start MIK
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
			List_GetCurSel( GetDlgItem( GetHwnd(), IDC_LIST_TYPES ) )
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
 			List_GetCurSel( GetDlgItem( GetHwnd(), IDC_LIST_TYPES ) )
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	//	Nov. 29, 2000	To Here
	case IDOK:
		::EndDialog( GetHwnd(), List_GetCurSel( GetDlgItem( GetHwnd(), IDC_LIST_TYPES ) ) );
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), -1 );
		return TRUE;
	case IDC_BUTTON_IMPORT:
		Import();
		return TRUE;
	case IDC_BUTTON_EXPORT:
		Export();
		return TRUE;
	case IDC_BUTTON_INITIALIZE:
		InitializeType();
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
	TCHAR	szText[64 + MAX_TYPES_EXTS + 10];
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	List_ResetContent( hwndList );	/* リストを空にする */
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
	List_SetCurSel( hwndList, m_nSettingType.GetIndex() );
	return;
}

//@@@ 2002.01.18 add start
LPVOID CDlgTypeList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end



// タイプ別設定インポート
//		2010/4/12 Uchi
bool CDlgTypeList::Import()
{
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int nIdx = List_GetCurSel( hwndList );
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));

	CImpExpType	cImpExpType( nIdx, types, hwndList );

	// インポート
	cImpExpType.SetBaseName( to_wchar( types.m_szTypeName ) );
	if (!cImpExpType.ImportUI( G_AppInstance(), GetHwnd() )) {
		// インポートをしていない
		return false;
	}

	// リスト再初期化
	SetData();
	List_SetCurSel( hwndList, nIdx );

	return true;
}

// タイプ別設定エクスポート
//		2010/4/12 Uchi
bool CDlgTypeList::Export()
{
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int nIdx = List_GetCurSel( hwndList );
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));

	CImpExpType	cImpExpType( nIdx, types, hwndList );

	// エクスポート
	cImpExpType.SetBaseName( to_wchar( types.m_szTypeName) );
	if (!cImpExpType.ExportUI( G_AppInstance(), GetHwnd() )) {
		// エクスポートをしていない
		return false;
	}

	return true;
}

//void _DefaultConfig(STypeConfig* pType);

// タイプ別設定初期化
//		2010/4/12 Uchi
bool CDlgTypeList::InitializeType( void )
{
	HWND hwndDlg = GetHwnd();
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int iDocType = List_GetCurSel( hwndList );
	if (iDocType == 0) {
		// 基本の場合には何もしない
		return true;
	}
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(iDocType));
	int			nRet;
	if (0 < _tcslen( types.m_szTypeExts )) { 
		nRet = ::MYMESSAGEBOX(
			GetHwnd(),
			MB_YESNO | MB_ICONQUESTION,
			GSTR_APPNAME,
			_T("%ts を初期化します。 よろしいですか？"),
			types.m_szTypeName );
		if (nRet != IDYES) {
			return false;
		}
	}

//	_DefaultConfig(&types);		//規定値をコピー
	types = CDocTypeManager().GetTypeSetting(CTypeConfig(0));	// 基本をコピー

	types.m_nIdx = iDocType;
	auto_sprintf( types.m_szTypeName, _T("設定%d"), iDocType+1 );
	_tcscpy( types.m_szTypeExts, _T("") );

	// リスト再初期化
	SetData();
	List_SetCurSel( hwndList, iDocType );

	InfoMessage( hwndDlg, _T("%ts を初期化しました。"), types.m_szTypeName );

	return true;
}
