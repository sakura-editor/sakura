/*!	@file
	@brief コントロールコード入力ダイアログボックス

	@author MIK
	@date 2002.6.2
*/
/*
	Copyright (C) 2002-2003, MIK
	Copyright (C) 2006, ryoji

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
#include <stdio.h>
#include "sakura_rc.h"
#include "global.h"
#include "func/Funccode.h"
#include "dlg/CDialog.h"
#include "dlg/CDlgCtrlCode.h"
#include "view/CEditView.h"
#include "debug/Debug.h"
#include "util/shell.h"

#include "sakura.hh"
const DWORD p_helpids[] = {	//13300
	IDC_LIST_CTRLCODE,		HIDC_LIST_CTRLCODE,
	IDOK,					HIDC_CTRLCODE_IDOK,
	IDCANCEL,				HIDC_CTRLCODE_IDCANCEL,
	IDC_BUTTON_HELP,		HIDC_BUTTON_CTRLCODE_HELP,
//	IDC_STATIC,				-1,
	0, 0
};

struct ctrl_info_t {
	wchar_t			code;		//入力する文字コード
	unsigned int	vKey;		//表記
	TCHAR			name[4];	//名前
	TCHAR			*jname;		//説明
} static const p_ctrl_list[] = {
	{ 0x0000, 0x00c0, _T("NUL"), _T("空文字")    }, //NULL
	{ 0x0001, 'A', _T("SOH"), _T("ヘッダ開始")   }, //START OF HEADING
	{ 0x0002, 'B', _T("STX"), _T("テキスト開始") }, //START OF TEXT
	{ 0x0003, 'C', _T("ETX"), _T("テキスト終了") }, //END OF TEXT
	{ 0x0004, 'D', _T("EOT"), _T("転送終了")     }, //END OF TRANSMISSION
	{ 0x0005, 'E', _T("ENQ"), _T("照会")         }, //ENQUIRY
	{ 0x0006, 'F', _T("ACK"), _T("受信OK")       }, //ACKNOWLEDGE
	{ 0x0007, 'G', _T("BEL"), _T("警告(ベル)")   }, //BELL
	{ 0x0008, 'H', _T("BS"),  _T("後退")         }, //BACKSPACE
	{ 0x0009, 'I', _T("HT"),  _T("タブ")         }, //horizontal tabulation (HT)
	{ 0x000a, 'J', _T("LF"),  _T("改行")         },	//LINE FEED (LF); new line (NL); end of line(EOL)
	{ 0x000b, 'K', _T("VT"),  _T("垂直タブ")     }, //vertical tabulation (VT)
	{ 0x000c, 'L', _T("FF"),  _T("改ページ")     },	//FORM FEED (FF)
	{ 0x000d, 'M', _T("CR"),  _T("復帰")         }, //CARRIAGE RETURN
	{ 0x000e, 'N', _T("SO"),  _T("シフトアウト") }, //SHIFT OUT
	{ 0x000f, 'O', _T("SI"),  _T("シフトイン")   }, //SHIFT IN
	{ 0x0010, 'P', _T("DLE"), _T("データリンクエスケープ") }, //DATA LINK ESCAPE
	{ 0x0011, 'Q', _T("DC1"), _T("装置制御1")    }, //DEVICE CONTROL ONE
	{ 0x0012, 'R', _T("DC2"), _T("装置制御2")    }, //DEVICE CONTROL TWO
	{ 0x0013, 'S', _T("DC3"), _T("装置制御3")    }, //DEVICE CONTROL THREE
	{ 0x0014, 'T', _T("DC4"), _T("装置制御4")    }, //DEVICE CONTROL FOUR
	{ 0x0015, 'U', _T("NAK"), _T("受信失敗")     }, //NEGATIVE ACKNOWLEDGE
	{ 0x0016, 'V', _T("SYN"), _T("同期")         }, //SYNCHRONOUS IDLE
	{ 0x0017, 'W', _T("ETB"), _T("転送ブロック終了") }, //END OF TRANSMISSION BLOCK
	{ 0x0018, 'X', _T("CAN"), _T("キャンセル")   }, //CANCEL
	{ 0x0019, 'Y', _T("EM"),  _T("メディア終了") }, //END OF MEDIUM
	{ 0x001a, 'Z', _T("SUB"), _T("置換")         }, //SUBSTITUTE
	{ 0x001b, 0x00db, _T("ESC"), _T("エスケープ")   }, //ESCAPE
	{ 0x001c, 0x00dc, _T("FS"),  _T("フォーム区切") }, //file separator (FS)
	{ 0x001d, 0x00dd, _T("GS"),  _T("グループ区切") }, //group separator (GS)
	{ 0x001e, 0x00de, _T("RS"),  _T("レコード区切") }, //record separator (RS)
	{ 0x001f, 0x00e2, _T("US"),  _T("ユニット区切") }, //unit separator (US)
	{ 0x007f, 0x00bf, _T("DEL"), _T("削除")         }, //DELETE

	//internal data
	{ 0x001f, 0x00df, _T("US"),  NULL }	//PC98 "_"  //unit separator (US)
};
// Feb. 12, 2003 MIK longが抜けていた

CDlgCtrlCode::CDlgCtrlCode()
{
	m_nCode = L'\0';
}

/* モーダルダイアログの表示 */
int CDlgCtrlCode::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_CTRLCODE, lParam );
}

/* ダイアログデータの設定 */
void CDlgCtrlCode::SetData( void )
{
	HWND	hwndWork;
	int		i, count;
	long	lngStyle;
	LV_ITEM	lvi;

	/* リスト */
	hwndWork = ::GetDlgItem( GetHwnd(), IDC_LIST_CTRLCODE );
	ListView_DeleteAllItems( hwndWork );  /* リストを空にする */

	/* 行選択 */
	lngStyle = ::SendMessageAny( hwndWork, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
	lngStyle |= LVS_EX_FULLROWSELECT;
	::SendMessageAny( hwndWork, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lngStyle );

	/* データ表示 */
	TCHAR	tmp[10];
	count = 0;
	for( i = 0; i < _countof(p_ctrl_list); i++ )
	{
		if( p_ctrl_list[i].jname == NULL ) continue;
		
		auto_sprintf( tmp, _T("0x%02X"), p_ctrl_list[i].code );
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = tmp;
		lvi.iItem    = count;
		lvi.iSubItem = 0;
		lvi.lParam   = 0;
		ListView_InsertItem( hwndWork, &lvi );
		
		if( p_ctrl_list[i].code <= 0x1f )
			auto_sprintf( tmp, _T("^%tc"), _T('@') + p_ctrl_list[i].code );
		else if( p_ctrl_list[i].code == 0x7f )
			_tcscpy( tmp, _T("^?") );
		else
			_tcscpy( tmp, _T("･") );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = count;
		lvi.iSubItem = 1;
		lvi.pszText  = tmp;
		ListView_SetItem( hwndWork, &lvi );
		
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = count;
		lvi.iSubItem = 2;
		lvi.pszText  = const_cast<TCHAR*>(p_ctrl_list[i].name);
		ListView_SetItem( hwndWork, &lvi );
		
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = count;
		lvi.iSubItem = 3;
		lvi.pszText  = p_ctrl_list[i].jname;
		ListView_SetItem( hwndWork, &lvi );
		
		count++;
	}
	ListView_SetItemState( hwndWork, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	return;
}

/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー */
int CDlgCtrlCode::GetData( void )
{
	int		nIndex;
	HWND	hwndList;

	hwndList = GetDlgItem( GetHwnd(), IDC_LIST_CTRLCODE );
	//選択中のキー番号を探す。
	nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
	if( nIndex == -1 ) return FALSE;

	m_nCode = p_ctrl_list[nIndex].code;

	return TRUE;
}

BOOL CDlgCtrlCode::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	HWND		hwndList;
	LV_COLUMN	col;
	RECT		rc;

	_SetHwnd( hwndDlg );

	hwndList = GetDlgItem( hwndDlg, IDC_LIST_CTRLCODE );
	::GetWindowRect( hwndList, &rc );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = _T("コード");
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = _T("表記");
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = _T("名前");
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 46 / 100;
	col.pszText  = _T("説明");
	col.iSubItem = 3;
	ListView_InsertColumn( hwndList, 3, &col );

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgCtrlCode::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* ヘルプ */
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_CTRL_CODE_DIALOG) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	case IDOK:			/* 左右に表示 */
		/* ダイアログデータの取得 */
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;

	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgCtrlCode::OnNotify( WPARAM wParam, LPARAM lParam )
{
	NMHDR*	pNMHDR;
	HWND	hwndList;

	pNMHDR = (NMHDR*)lParam;

	hwndList = GetDlgItem( GetHwnd(), IDC_LIST_CTRLCODE );

	if( hwndList == pNMHDR->hwndFrom )
	{
		switch( pNMHDR->code )
		{
		case NM_DBLCLK:
			::EndDialog( GetHwnd(), GetData() );
			return TRUE;

		case LVN_KEYDOWN:
			{
				HWND	hwndList;
				NMKEY	*p = (NMKEY*)lParam;
				int		i, j;
				unsigned int	c;
				for( i = 0; i < _countof(p_ctrl_list); i++ )
				{
					c = p_ctrl_list[i].vKey;
					if( c == (p->nVKey & 0xffff) )
					{
						for( j = 0; j < _countof(p_ctrl_list); j++ )
						{
							if( p_ctrl_list[i].code == p_ctrl_list[j].code )
							{
								hwndList = GetDlgItem( GetHwnd(), IDC_LIST_CTRLCODE );
								ListView_SetItemState( hwndList, j, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
								ListView_EnsureVisible( hwndList, j, FALSE );
						
								/* ダイアログデータの取得 */
								::EndDialog( GetHwnd(), GetData() );
								return TRUE;
							}
						}
						break;
					}
				}
			}
			break;
		}
	}

	/* 基底クラスメンバ */
	return CDialog::OnNotify( wParam, lParam );
}

LPVOID CDlgCtrlCode::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}


