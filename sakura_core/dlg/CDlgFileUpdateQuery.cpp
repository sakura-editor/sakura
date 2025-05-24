/*! @file
	@brief 更新通知及び確認ダイアログ

	ファイルの更新通知と動作の確認を行うダイアログボックス

	@author genta
	@date 2002.12.04
*/
/*
	Copyright (C) 2002, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "dlg/CDlgFileUpdateQuery.h"
#include "apiwrap/StdControl.h"
#include "CSelectLang.h"
#include "sakura_rc.h"
#include "String_define.h"

BOOL CDlgFileUpdateQuery::OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	::DlgItem_SetText( hWnd, IDC_UPDATEDFILENAME, m_pFilename );
	::DlgItem_SetText( hWnd, IDC_QUERYRELOADMSG, m_bModified ?
		LS(STR_ERR_DLGUPQRY1):LS(STR_ERR_DLGUPQRY2) );

	return CDialog::OnInitDialog( hWnd, wParam, lParam );
}

/*!
	ボタンが押されたときの動作
*/
BOOL CDlgFileUpdateQuery::OnBnClicked(int id)
{
	EFileUpdateQuery result;

	switch( id ){
	case IDC_BTN_RELOAD: // 再読込
		result = EFUQ_RELOAD;
		break;
	case IDC_BTN_CLOSE: // 閉じる
		result = EFUQ_CLOSE;
		break;
	case IDC_BTN_NOTIFYONLY: // 以後通知メッセージのみ
		result = EFUQ_NOTIFYONLY;
		break;
	case IDC_BTN_NOSUPERVISION: // 以後更新を監視しない
		result = EFUQ_NOSUPERVISION;
		break;
	case IDC_BTN_AUTOLOAD:		// 以後未編集で再ロード
		result = EFUQ_AUTOLOAD;
		break;
	default:
		result = EFUQ_CLOSE;
		break;
	}
	CloseDialog( (INT_PTR)result );

	return 0;
}
