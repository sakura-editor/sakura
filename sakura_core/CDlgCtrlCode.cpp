//	$Id$
/*!	@file
	@brief �R���g���[���R�[�h���̓_�C�A���O�{�b�N�X

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
	unsigned char	code;		//�R�[�h
	unsigned int	vKey;			
	char			name[4];	//���O
	char			*jname;		//���{�ꖼ
} static const p_ctrl_list[] = {
	{ 0x00, 0x00c0, "NUL", "�󕶎�"       },
	{ 0x01, 'A', "SOH", "�w�b�_�J�n"   },
	{ 0x02, 'B', "STX", "�e�L�X�g�J�n" },
	{ 0x03, 'C', "ETX", "�e�L�X�g�I��" },
	{ 0x04, 'D', "EOT", "�]���I��"     },
	{ 0x05, 'E', "ENQ", "�Ɖ�"         },
	{ 0x06, 'F', "ACK", "��MOK"       },
	{ 0x07, 'G', "BEL", "�x��(�x��)"   },
	{ 0x08, 'H', "BS",  "���"         },
	{ 0x09, 'I', "HT",  "�^�u"         },
	{ 0x0a, 'J', "LF",  "���s"         },	//NL
	{ 0x0b, 'K', "VT",  "�����^�u"     },
	{ 0x0c, 'L', "FF",  "���y�[�W"     },	//NP
	{ 0x0d, 'M', "CR",  "���A"         },
	{ 0x0e, 'N', "SO",  "�V�t�g�A�E�g" },
	{ 0x0f, 'O', "SI",  "�V�t�g�C��"   },
	{ 0x10, 'P', "DLE", "�f�[�^�����N�G�X�P�[�v" },
	{ 0x11, 'Q', "DC1", "���u����1"    },
	{ 0x12, 'R', "DC2", "���u����2"    },
	{ 0x13, 'S', "DC3", "���u����3"    },
	{ 0x14, 'T', "DC4", "���u����4"    },
	{ 0x15, 'U', "NAK", "��M���s"     },
	{ 0x16, 'V', "SYN", "����"         },
	{ 0x17, 'W', "ETB", "�]���u���b�N�I��" },
	{ 0x18, 'X', "CAN", "�L�����Z��"   },
	{ 0x19, 'Y', "EM",  "���f�B�A�I��" },
	{ 0x1a, 'Z', "SUB", "�u��"         },
	{ 0x1b, 0x00db, "ESC", "�G�X�P�[�v"   },
	{ 0x1c, 0x00dc, "FS",  "�t�H�[�����" },
	{ 0x1d, 0x00dd, "GS",  "�O���[�v���" },
	{ 0x1e, 0x00de, "RS",  "���R�[�h���" },
	{ 0x1f, 0x00e2, "US",  "���j�b�g���" },
	{ 0x7f, 0x00bf, "DEL", "�폜"         },

	//internal data
	{ 0x1f, 0x00df, "US",  NULL }	//PC98 "_"
};
// Feb. 12, 2003 MIK long�������Ă���
static const long p_ctrl_list_num = sizeof(p_ctrl_list) / sizeof(struct ctrl_info_t);

CDlgCtrlCode::CDlgCtrlCode()
{
	m_nCode = 0;
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
	char	tmp[10];
	long	lngStyle;
	LV_ITEM	lvi;

	/* ���X�g */
	hwndWork = ::GetDlgItem( m_hWnd, IDC_LIST_CTRLCODE );
	ListView_DeleteAllItems( hwndWork );  /* ���X�g����ɂ��� */

	/* �s�I�� */
	lngStyle = ::SendMessage( hwndWork, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
	lngStyle |= LVS_EX_FULLROWSELECT;
	::SendMessage( hwndWork, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lngStyle );

	/* �f�[�^�\�� */
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
			strcpy( tmp, "�" );
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

/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[ */
int CDlgCtrlCode::GetData( void )
{
	int		nIndex;
	HWND	hwndList;

	hwndList = GetDlgItem( m_hWnd, IDC_LIST_CTRLCODE );
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

	m_hWnd = hwndDlg;

	hwndList = GetDlgItem( hwndDlg, IDC_LIST_CTRLCODE );
	::GetWindowRect( hwndList, &rc );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = "�R�[�h";
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = "�\�L";
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = "���O";
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 46 / 100;
	col.pszText  = "����";
	col.iSubItem = 3;
	ListView_InsertColumn( hwndList, 3, &col );

	/* ���N���X�����o */
	return CDialog::OnInitDialog( m_hWnd, wParam, lParam );
}

BOOL CDlgCtrlCode::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* �w���v */
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_CTRL_CODE_DIALOG) );
		return TRUE;

	case IDOK:			/* ���E�ɕ\�� */
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( m_hWnd, GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
		return TRUE;

	}

	/* ���N���X�����o */
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
						
								/* �_�C�A���O�f�[�^�̎擾 */
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

	/* ���N���X�����o */
	return CDialog::OnNotify( wParam, lParam );
}

LPVOID CDlgCtrlCode::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

/*[EOF]*/
