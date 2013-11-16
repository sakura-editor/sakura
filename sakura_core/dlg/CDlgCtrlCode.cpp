/*!	@file
	@brief �R���g���[���R�[�h���̓_�C�A���O�{�b�N�X

	@author MIK
	@date 2002.6.2
*/
/*
	Copyright (C) 2002-2003, MIK
	Copyright (C) 2006, ryoji
+	Copyright (C) 2011, nasukoji

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
#include "dlg/CDlgCtrlCode.h"
#include "func/Funccode.h"
#include "util/shell.h"
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
	wchar_t			code;		//���͂��镶���R�[�h
	unsigned int	vKey;		//�\�L
	TCHAR			name[4];	//���O
	const TCHAR		*jname;		//����
} static p_ctrl_list[] = {
	{ 0x0000, 0x00c0, _T("NUL"), _T("")	}, //NULL �󕶎�
	{ 0x0001, 'A', _T("SOH"), _T("")	}, //START OF HEADING �w�b�_�J�n
	{ 0x0002, 'B', _T("STX"), _T("")	}, //START OF TEXT �e�L�X�g�J�n
	{ 0x0003, 'C', _T("ETX"), _T("")	}, //END OF TEXT �e�L�X�g�I��
	{ 0x0004, 'D', _T("EOT"), _T("")	}, //END OF TRANSMISSION �]���I��
	{ 0x0005, 'E', _T("ENQ"), _T("")	}, //ENQUIRY �Ɖ�
	{ 0x0006, 'F', _T("ACK"), _T("")	}, //ACKNOWLEDGE ��MOK
	{ 0x0007, 'G', _T("BEL"), _T("")	}, //BELL �x��(�x��)
	{ 0x0008, 'H', _T("BS"),  _T("")	}, //BACKSPACE ���
	{ 0x0009, 'I', _T("HT"),  _T("")	}, //horizontal tabulation (HT) �^�u
	{ 0x000a, 'J', _T("LF"),  _T("")	},	//LINE FEED (LF); new line (NL); end of line(EOL) ���s
	{ 0x000b, 'K', _T("VT"),  _T("")	}, //vertical tabulation (VT) �����^�u
	{ 0x000c, 'L', _T("FF"),  _T("")	},	//FORM FEED (FF) ���y�[�W
	{ 0x000d, 'M', _T("CR"),  _T("")	}, //CARRIAGE RETURN ���A
	{ 0x000e, 'N', _T("SO"),  _T("")	}, //SHIFT OUT �V�t�g�A�E�g
	{ 0x000f, 'O', _T("SI"),  _T("")	}, //SHIFT IN �V�t�g�C��
	{ 0x0010, 'P', _T("DLE"), _T("")	}, //DATA LINK ESCAPE �f�[�^�����N�G�X�P�[�v
	{ 0x0011, 'Q', _T("DC1"), _T("")    }, //DEVICE CONTROL ONE ���u����1
	{ 0x0012, 'R', _T("DC2"), _T("")    }, //DEVICE CONTROL TWO ���u����2
	{ 0x0013, 'S', _T("DC3"), _T("")    }, //DEVICE CONTROL THREE ���u����3
	{ 0x0014, 'T', _T("DC4"), _T("")    }, //DEVICE CONTROL FOUR ���u����4
	{ 0x0015, 'U', _T("NAK"), _T("")	}, //NEGATIVE ACKNOWLEDGE ��M���s
	{ 0x0016, 'V', _T("SYN"), _T("")	}, //SYNCHRONOUS IDLE ����
	{ 0x0017, 'W', _T("ETB"), _T("")	}, //END OF TRANSMISSION BLOCK �]���u���b�N�I��
	{ 0x0018, 'X', _T("CAN"), _T("")	}, //CANCEL �L�����Z��
	{ 0x0019, 'Y', _T("EM"),  _T("")	}, //END OF MEDIUM ���f�B�A�I��
	{ 0x001a, 'Z', _T("SUB"), _T("")	}, //SUBSTITUTE �u��
	{ 0x001b, 0x00db, _T("ESC"), _T("")	}, //ESCAPE �G�X�P�[�v
	{ 0x001c, 0x00dc, _T("FS"),  _T("") }, //file separator (FS) �t�H�[�����
	{ 0x001d, 0x00dd, _T("GS"),  _T("") }, //group separator (GS) �O���[�v���
	{ 0x001e, 0x00de, _T("RS"),  _T("") }, //record separator (RS) ���R�[�h���
	{ 0x001f, 0x00e2, _T("US"),  _T("") }, //unit separator (US) ���j�b�g���
	{ 0x007f, 0x00bf, _T("DEL"), _T("")	}, //DELETE �폜

	//internal data
	{ 0x001f, 0x00df, _T("US"),  NULL }	//PC98 "_"  //unit separator (US)
};
// Feb. 12, 2003 MIK long�������Ă���

// LMP: Added, nasukoji changed
static CLoadString cLabel_jname[ _countof(p_ctrl_list) ];

CDlgCtrlCode::CDlgCtrlCode()
{
	m_nCode = L'\0';
}

/* ���[�_���_�C�A���O�̕\�� */
int CDlgCtrlCode::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_CTRLCODE, lParam );
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgCtrlCode::SetData( void )
{
	HWND	hwndWork;
	int		i, count;
	long	lngStyle;
	LV_ITEM	lvi;

	/* ���X�g */
	hwndWork = ::GetDlgItem( GetHwnd(), IDC_LIST_CTRLCODE );
	ListView_DeleteAllItems( hwndWork );  /* ���X�g����ɂ��� */

	/* �s�I�� */
	lngStyle = ListView_GetExtendedListViewStyle( hwndWork );
	lngStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( hwndWork, lngStyle );

	/* �f�[�^�\�� */
	TCHAR	tmp[10];
	count = 0;
	for( i = 0; i < _countof(p_ctrl_list); i++ )
	{
		if( p_ctrl_list[i].jname == NULL ) continue;
		
		// 2011.06.01 nasukoji	����jname��NULL�̂��̂͂��̂܂܎c��
		if( p_ctrl_list[i].jname ){
			// LMP: Added, nasukoji changed
			p_ctrl_list[i].jname = (LPTSTR)cLabel_jname[i].LoadString(STR_ERR_DLGCTL5 + i);
		}

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
			_tcscpy( tmp, _T("�") );
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
		lvi.pszText  = const_cast<TCHAR*>(p_ctrl_list[i].jname);
		ListView_SetItem( hwndWork, &lvi );
		
		count++;
	}
	ListView_SetItemState( hwndWork, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	return;
}

/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[ */
int CDlgCtrlCode::GetData( void )
{
	int		nIndex;
	HWND	hwndList;

	hwndList = GetDlgItem( GetHwnd(), IDC_LIST_CTRLCODE );
	//�I�𒆂̃L�[�ԍ���T���B
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
	col.pszText  = const_cast<TCHAR*>( LS( STR_DLGCTRLCODE_CODE ) );	// "�R�[�h"
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = const_cast<TCHAR*>( LS( STR_DLGCTRLCODE_SYMBOL ) );	// "�\�L"
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = const_cast<TCHAR*>( LS( STR_DLGCTRLCODE_NAME ) );	// "���O"
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 46 / 100;
	col.pszText  = const_cast<TCHAR*>( LS( STR_DLGCTRLCODE_DESC ) );	// "����"
	col.iSubItem = 3;
	ListView_InsertColumn( hwndList, 3, &col );

	/* ���N���X�����o */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgCtrlCode::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* �w���v */
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_CTRL_CODE_DIALOG) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;

	case IDOK:			/* ���E�ɕ\�� */
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;

	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

#ifdef __MINGW32__
#ifndef BUILD_OPT_NEW_HEADERS
typedef struct tagNMKEY {
  NMHDR hdr;
  UINT  nVKey;
  UINT  uFlags;
} NMKEY, *LPNMKEY;
#endif
#endif

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
						
								/* �_�C�A���O�f�[�^�̎擾 */
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

	/* ���N���X�����o */
	return CDialog::OnNotify( wParam, lParam );
}

LPVOID CDlgCtrlCode::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}


