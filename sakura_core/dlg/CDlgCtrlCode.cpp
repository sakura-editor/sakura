/*!	@file
	@brief コントロールコード入力ダイアログボックス

	@author MIK
	@date 2002.6.2
*/
/*
	Copyright (C) 2002-2003, MIK
	Copyright (C) 2006, ryoji
+	Copyright (C) 2011, nasukoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "dlg/CDlgCtrlCode.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "CSelectLang.h"
#include "basis/CMyString.h"
#include "sakura_rc.h"
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
	WCHAR			code;		//入力する文字コード
	unsigned int	vKey;		//表記
	WCHAR			name[4];	//名前
	const WCHAR		*jname;		//説明
} static p_ctrl_list[] = {
	{ 0x0000, 0x00c0, LTEXT("NUL"), LTEXT("")	}, //NULL 空文字
	{ 0x0001, 'A', LTEXT("SOH"), LTEXT("")	}, //START OF HEADING ヘッダー開始
	{ 0x0002, 'B', LTEXT("STX"), LTEXT("")	}, //START OF TEXT テキスト開始
	{ 0x0003, 'C', LTEXT("ETX"), LTEXT("")	}, //END OF TEXT テキスト終了
	{ 0x0004, 'D', LTEXT("EOT"), LTEXT("")	}, //END OF TRANSMISSION 転送終了
	{ 0x0005, 'E', LTEXT("ENQ"), LTEXT("")	}, //ENQUIRY 照会
	{ 0x0006, 'F', LTEXT("ACK"), LTEXT("")	}, //ACKNOWLEDGE 受信OK
	{ 0x0007, 'G', LTEXT("BEL"), LTEXT("")	}, //BELL 警告(ベル)
	{ 0x0008, 'H', LTEXT("BS"),  LTEXT("")	}, //BACKSPACE 後退
	{ 0x0009, 'I', LTEXT("HT"),  LTEXT("")	}, //horizontal tabulation (HT) タブ
	{ 0x000a, 'J', LTEXT("LF"),  LTEXT("")	},	//LINE FEED (LF); new line (NL); end of line(EOL) 改行
	{ 0x000b, 'K', LTEXT("VT"),  LTEXT("")	}, //vertical tabulation (VT) 垂直タブ
	{ 0x000c, 'L', LTEXT("FF"),  LTEXT("")	},	//FORM FEED (FF) 改ページ
	{ 0x000d, 'M', LTEXT("CR"),  LTEXT("")	}, //CARRIAGE RETURN 復帰
	{ 0x000e, 'N', LTEXT("SO"),  LTEXT("")	}, //SHIFT OUT シフトアウト
	{ 0x000f, 'O', LTEXT("SI"),  LTEXT("")	}, //SHIFT IN シフトイン
	{ 0x0010, 'P', LTEXT("DLE"), LTEXT("")	}, //DATA LINK ESCAPE データリンクエスケープ
	{ 0x0011, 'Q', LTEXT("DC1"), LTEXT("")    }, //DEVICE CONTROL ONE 装置制御1
	{ 0x0012, 'R', LTEXT("DC2"), LTEXT("")    }, //DEVICE CONTROL TWO 装置制御2
	{ 0x0013, 'S', LTEXT("DC3"), LTEXT("")    }, //DEVICE CONTROL THREE 装置制御3
	{ 0x0014, 'T', LTEXT("DC4"), LTEXT("")    }, //DEVICE CONTROL FOUR 装置制御4
	{ 0x0015, 'U', LTEXT("NAK"), LTEXT("")	}, //NEGATIVE ACKNOWLEDGE 受信失敗
	{ 0x0016, 'V', LTEXT("SYN"), LTEXT("")	}, //SYNCHRONOUS IDLE 同期
	{ 0x0017, 'W', LTEXT("ETB"), LTEXT("")	}, //END OF TRANSMISSION BLOCK 転送ブロック終了
	{ 0x0018, 'X', LTEXT("CAN"), LTEXT("")	}, //CANCEL キャンセル
	{ 0x0019, 'Y', LTEXT("EM"),  LTEXT("")	}, //END OF MEDIUM メディア終了
	{ 0x001a, 'Z', LTEXT("SUB"), LTEXT("")	}, //SUBSTITUTE 置換
	{ 0x001b, 0x00db, LTEXT("ESC"), LTEXT("")	}, //ESCAPE エスケープ
	{ 0x001c, 0x00dc, LTEXT("FS"),  LTEXT("") }, //file separator (FS) フォーム区切
	{ 0x001d, 0x00dd, LTEXT("GS"),  LTEXT("") }, //group separator (GS) グループ区切
	{ 0x001e, 0x00de, LTEXT("RS"),  LTEXT("") }, //record separator (RS) レコード区切
	{ 0x001f, 0x00e2, LTEXT("US"),  LTEXT("") }, //unit separator (US) ユニット区切
	{ 0x007f, 0x00bf, LTEXT("DEL"), LTEXT("")	}, //DELETE 削除

	//internal data
	{ 0x001f, 0x00df, LTEXT("US"),  nullptr }	//PC98 "_"  //unit separator (US)
};
// Feb. 12, 2003 MIK longが抜けていた

// LMP: Added, nasukoji changed
static CLoadString cLabel_jname[std::size(p_ctrl_list) ];

CDlgCtrlCode::CDlgCtrlCode()
{
	m_nCode = LCHAR('\0');
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
	hwndWork = GetItemHwnd( IDC_LIST_CTRLCODE );
	ListView_DeleteAllItems( hwndWork );  /* リストを空にする */

	/* 行選択 */
	lngStyle = ListView_GetExtendedListViewStyle( hwndWork );
	lngStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( hwndWork, lngStyle );

	/* データ表示 */
	WCHAR	tmp[10];
	count = 0;
	for( i = 0; i < int(std::size(p_ctrl_list)); i++ )
	{
		if( p_ctrl_list[i].jname == nullptr ) continue;
		
		// 2011.06.01 nasukoji	元のjnameがNULLのものはそのまま残す
		if( p_ctrl_list[i].jname ){
			// LMP: Added, nasukoji changed
			p_ctrl_list[i].jname = cLabel_jname[i].LoadString(STR_ERR_DLGCTL5 + i);
		}

		auto_sprintf( tmp, LTEXT("0x%02X"), p_ctrl_list[i].code );
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = tmp;
		lvi.iItem    = count;
		lvi.iSubItem = 0;
		lvi.lParam   = 0;
		ListView_InsertItem( hwndWork, &lvi );
		
		if( p_ctrl_list[i].code <= 0x1f )
			auto_sprintf( tmp, LTEXT("^%c"), LCHAR('@') + p_ctrl_list[i].code );
		else if( p_ctrl_list[i].code == 0x7f )
			wcscpy( tmp, LTEXT("^?") );
		else
			wcscpy( tmp, LTEXT("･") );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = count;
		lvi.iSubItem = 1;
		lvi.pszText  = tmp;
		ListView_SetItem( hwndWork, &lvi );
		
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = count;
		lvi.iSubItem = 2;
		lvi.pszText  = const_cast<WCHAR*>(p_ctrl_list[i].name);
		ListView_SetItem( hwndWork, &lvi );
		
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = count;
		lvi.iSubItem = 3;
		lvi.pszText  = const_cast<WCHAR*>(p_ctrl_list[i].jname);
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

	hwndList = GetItemHwnd( IDC_LIST_CTRLCODE );
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
	col.pszText  = const_cast<WCHAR*>( LS( STR_DLGCTRLCODE_CODE ) );	// "コード"
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = const_cast<WCHAR*>( LS( STR_DLGCTRLCODE_SYMBOL ) );	// "表記"
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = const_cast<WCHAR*>( LS( STR_DLGCTRLCODE_NAME ) );	// "名前"
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 46 / 100;
	col.pszText  = const_cast<WCHAR*>( LS( STR_DLGCTRLCODE_DESC ) );	// "説明"
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
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_CTRL_CODE_DIALOG) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	case IDOK:			/* 左右に表示 */
		/* ダイアログデータの取得 */
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	default:
		break;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgCtrlCode::OnNotify( NMHDR* pNMHDR )
{
	HWND	hwndList;

	hwndList = GetItemHwnd( IDC_LIST_CTRLCODE );

	if( hwndList == pNMHDR->hwndFrom )
	{
		switch( pNMHDR->code )
		{
		case NM_DBLCLK:
			::EndDialog( GetHwnd(), GetData() );
			return TRUE;

		case LVN_KEYDOWN:
			{
				NMKEY	*p = (NMKEY*)pNMHDR;
				int		i, j;
				unsigned int	c;
				for( i = 0; i < int(std::size(p_ctrl_list)); i++ )
				{
					c = p_ctrl_list[i].vKey;
					if( c == (p->nVKey & 0xffff) )
					{
						for( j = 0; j < int(std::size(p_ctrl_list)); j++ )
						{
							if( p_ctrl_list[i].code == p_ctrl_list[j].code )
							{
								hwndList = GetItemHwnd( IDC_LIST_CTRLCODE );
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
		default:
			break;
		}
	}

	/* 基底クラスメンバ */
	return CDialog::OnNotify(pNMHDR);
}

LPVOID CDlgCtrlCode::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}
