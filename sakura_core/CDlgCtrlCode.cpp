//	$Id$
/*!	@file
	@brief コントロールコード入力ダイアログボックス

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 2002, MIK

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

#include <stdio.h>
#include "sakura_rc.h"
#include "global.h"
#include "funccode.h"
#include "mymessage.h"
#include "CDialog.h"
#include "CDlgCtrlCode.h"
#include "CEditView.h"
#include "etc_uty.h"
#include "debug.h"

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
	unsigned char	code;		//コード
	unsigned int	vKey;			
	char			name[4];	//名前
	char			*jname;		//日本語名
} static const p_ctrl_list[] = {
	{ 0x00, 0x00c0, "NUL", "空文字"       },
	{ 0x01, 'A', "SOH", "ヘッダ開始"   },
	{ 0x02, 'B', "STX", "テキスト開始" },
	{ 0x03, 'C', "ETX", "テキスト終了" },
	{ 0x04, 'D', "EOT", "転送終了"     },
	{ 0x05, 'E', "ENQ", "照会"         },
	{ 0x06, 'F', "ACK", "受信OK"       },
	{ 0x07, 'G', "BEL", "警告(ベル)"   },
	{ 0x08, 'H', "BS",  "後退"         },
	{ 0x09, 'I', "HT",  "タブ"         },
	{ 0x0a, 'J', "LF",  "改行"         },	//NL
	{ 0x0b, 'K', "VT",  "垂直タブ"     },
	{ 0x0c, 'L', "FF",  "改ページ"     },	//NP
	{ 0x0d, 'M', "CR",  "復帰"         },
	{ 0x0e, 'N', "SO",  "シフトアウト" },
	{ 0x0f, 'O', "SI",  "シフトイン"   },
	{ 0x10, 'P', "DLE", "データリンクエスケープ" },
	{ 0x11, 'Q', "DC1", "装置制御1"    },
	{ 0x12, 'R', "DC2", "装置制御2"    },
	{ 0x13, 'S', "DC3", "装置制御3"    },
	{ 0x14, 'T', "DC4", "装置制御4"    },
	{ 0x15, 'U', "NAK", "受信失敗"     },
	{ 0x16, 'V', "SYN", "同期"         },
	{ 0x17, 'W', "ETB", "転送ブロック終了" },
	{ 0x18, 'X', "CAN", "キャンセル"   },
	{ 0x19, 'Y', "EM",  "メディア終了" },
	{ 0x1a, 'Z', "SUB", "置換"         },
	{ 0x1b, 0x00db, "ESC", "エスケープ"   },
	{ 0x1c, 0x00dc, "FS",  "フォーム区切" },
	{ 0x1d, 0x00dd, "GS",  "グループ区切" },
	{ 0x1e, 0x00de, "RS",  "レコード区切" },
	{ 0x1f, 0x00e2, "US",  "ユニット区切" },
	{ 0x7f, 0x00bf, "DEL", "削除"         },

	//internal data
	{ 0x1f, 0x00df, "US",  NULL }	//PC98 "_"
};
// Feb. 12, 2003 MIK longが抜けていた
static const long p_ctrl_list_num = sizeof(p_ctrl_list) / sizeof(struct ctrl_info_t);

CDlgCtrlCode::CDlgCtrlCode()
{
	m_nCode = 0;
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
	char	tmp[10];
	long	lngStyle;
	LV_ITEM	lvi;

	/* リスト */
	hwndWork = ::GetDlgItem( m_hWnd, IDC_LIST_CTRLCODE );
	ListView_DeleteAllItems( hwndWork );  /* リストを空にする */

	/* 行選択 */
	lngStyle = ::SendMessage( hwndWork, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
	lngStyle |= LVS_EX_FULLROWSELECT;
	::SendMessage( hwndWork, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lngStyle );

	/* データ表示 */
	count = 0;
	for( i = 0; i < p_ctrl_list_num; i++ )
	{
		if( p_ctrl_list[i].jname == NULL ) continue;
		
		sprintf( tmp, "0x%02X", p_ctrl_list[i].code );
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = tmp;
		lvi.iItem    = count;
		lvi.iSubItem = 0;
		lvi.lParam   = 0;
		ListView_InsertItem( hwndWork, &lvi );
		
		if( p_ctrl_list[i].code <= 0x1f )
			sprintf( tmp, "^%c", '@' + p_ctrl_list[i].code );
		else if( p_ctrl_list[i].code == 0x7f )
			strcpy( tmp, "^?" );
		else
			strcpy( tmp, "･" );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = count;
		lvi.iSubItem = 1;
		lvi.pszText  = tmp;
		ListView_SetItem( hwndWork, &lvi );
		
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = count;
		lvi.iSubItem = 2;
		lvi.pszText  = (char*)p_ctrl_list[i].name;
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

	hwndList = GetDlgItem( m_hWnd, IDC_LIST_CTRLCODE );
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

	m_hWnd = hwndDlg;

	hwndList = GetDlgItem( hwndDlg, IDC_LIST_CTRLCODE );
	::GetWindowRect( hwndList, &rc );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = "コード";
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = "表記";
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = "名前";
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 46 / 100;
	col.pszText  = "説明";
	col.iSubItem = 3;
	ListView_InsertColumn( hwndList, 3, &col );

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( m_hWnd, wParam, lParam );
}

BOOL CDlgCtrlCode::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* ヘルプ */
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_CTRL_CODE_DIALOG) );
		return TRUE;

	case IDOK:			/* 左右に表示 */
		/* ダイアログデータの取得 */
		::EndDialog( m_hWnd, GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
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

	hwndList = GetDlgItem( m_hWnd, IDC_LIST_CTRLCODE );

	if( hwndList == pNMHDR->hwndFrom )
	{
		switch( pNMHDR->code )
		{
		case NM_DBLCLK:
			::EndDialog( m_hWnd, GetData() );
			return TRUE;

		case LVN_KEYDOWN:
			{
				HWND	hwndList;
				NMKEY	*p = (NMKEY*)lParam;
				int		i, j;
				unsigned int	c;
				for( i = 0; i < p_ctrl_list_num; i++ )
				{
					c = p_ctrl_list[i].vKey;
					if( c == (p->nVKey & 0xffff) )
					{
						for( j = 0; j < p_ctrl_list_num; j++ )
						{
							if( p_ctrl_list[i].code == p_ctrl_list[j].code )
							{
								hwndList = GetDlgItem( m_hWnd, IDC_LIST_CTRLCODE );
								ListView_SetItemState( hwndList, j, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
								ListView_EnsureVisible( hwndList, j, FALSE );
						
								/* ダイアログデータの取得 */
								::EndDialog( m_hWnd, GetData() );
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

/*[EOF]*/
