/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�t�@�C�����\���v�y�[�W

	@author Moca
	@date 2002.12.09 Moca CPropTypesRegex.cpp���Q�l�ɂ��č쐬
*/
/*
	Copyright (C) 2001, MIK
	Copyright (C) 2002, Moca, YAZAKI
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2004, D.S.Koba
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
#include "StdAfx.h"
#include "prop/CPropCommon.h"
//#include "env/CommonSetting.h"
#include "util/shell.h"
//#include "env/CShareData.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {	//13400
	IDC_LIST_FNAME,			HIDC_LIST_FNAME, 		// �t�@�C�����u�����X�g
	IDC_EDIT_FNAME_FROM,	HIDC_EDIT_FNAME_FROM,	// �u���O
	IDC_EDIT_FNAME_TO,		HIDC_EDIT_FNAME_TO,		// �u����
	IDC_BUTTON_FNAME_INS,	HIDC_BUTTON_FNAME_INS,	// �}��
	IDC_BUTTON_FNAME_ADD,	HIDC_BUTTON_FNAME_ADD,	// �ǉ�
	IDC_BUTTON_FNAME_UPD,	HIDC_BUTTON_FNAME_UPD,	// �X�V
	IDC_BUTTON_FNAME_DEL,	HIDC_BUTTON_FNAME_DEL,	// �폜
	IDC_BUTTON_FNAME_TOP,	HIDC_BUTTON_FNAME_TOP,	// �擪
	IDC_BUTTON_FNAME_UP,	HIDC_BUTTON_FNAME_UP,	// ���
	IDC_BUTTON_FNAME_DOWN,	HIDC_BUTTON_FNAME_DOWN,	// ����
	IDC_BUTTON_FNAME_LAST,	HIDC_BUTTON_FNAME_LAST,	// �ŏI
//	IDC_CHECK_FNAME,		HIDC_CHECK_FNAME,	// �t�@�C�������ȈՕ\������
	0, 0 // 
};



INT_PTR CALLBACK CPropFileName::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}

INT_PTR CPropFileName::DispatchEvent( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	HWND	hListView;
	int		nIndex;
	TCHAR	szFrom[_MAX_PATH];
	TCHAR	szTo[_MAX_PATH];

	switch( uMsg ){

	case WM_INITDIALOG:
		{
			RECT		rc;
			LV_COLUMN	col;
			hListView = GetDlgItem( hwndDlg, IDC_LIST_FNAME );

			// Modified by KEITA for WIN64 2003.9.6
			::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );
			::GetWindowRect( hListView, &rc );
			col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			col.fmt      = LVCFMT_LEFT;
			col.cx       = ( rc.right - rc.left ) * 60 / 100;
			col.pszText  = _T("�u���O");
			col.iSubItem = 0;
			ListView_InsertColumn( hListView, 0, &col );
			col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			col.fmt      = LVCFMT_LEFT;
			col.cx       = ( rc.right - rc.left ) * 35 / 100;
			col.pszText  = _T("�u����");
			col.iSubItem = 1;
			ListView_InsertColumn( hListView, 1, &col );

			// Apr. 28, 2003 Moca �������R��C��
			// �_�C�A���O���J�����Ƃ��Ƀ��X�g���I������Ă��Ă��t�B�[���h����̏ꍇ��������
			m_nLastPos_FILENAME = -1;

			// �_�C�A���O�f�[�^�̐ݒ�
			SetData( hwndDlg );

			// �G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
			::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_FNAME_FROM ),  EM_LIMITTEXT, (WPARAM)( _MAX_PATH - 1 ), 0 );
			::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_FNAME_TO ),    EM_LIMITTEXT, (WPARAM)( _MAX_PATH - 1 ), 0 );
		}
		return TRUE;

	case WM_NOTIFY:
		{
			NMHDR*		pNMHDR = (NMHDR*)lParam;
			NM_UPDOWN*	pMNUD  = (NM_UPDOWN*)lParam;
			int			idCtrl = (int)wParam;

			switch( idCtrl ){
			case IDC_LIST_FNAME:
				switch( pNMHDR->code ){
				case LVN_ITEMCHANGED:
					hListView = GetDlgItem( hwndDlg, IDC_LIST_FNAME );
					nIndex = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
					// ���I��
					if( -1 == nIndex ){
						::DlgItem_SetText( hwndDlg, IDC_EDIT_FNAME_FROM, _T("") );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_FNAME_TO, _T("") );
					}
					else if( nIndex != m_nLastPos_FILENAME ){
						GetListViewItem_FILENAME( hListView, nIndex, szFrom, szTo );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_FNAME_FROM, szFrom );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_FNAME_TO, szTo );
					}
					else{
						// nIndex == m_nLastPos_FILENAME�̂Ƃ�
						// ���X�g���G�f�B�b�g�{�b�N�X�Ƀf�[�^���R�s�[�����[�X�V]�����܂����܂����삵�Ȃ�
					}
					m_nLastPos_FILENAME = nIndex;
					break;
				}
				break;
			default:
				switch( pNMHDR->code ){
				case PSN_HELP:
					OnHelp( hwndDlg, IDD_PROP_FNAME );
					return TRUE;
				case PSN_KILLACTIVE:
					// �_�C�A���O�f�[�^�̎擾
					GetData( hwndDlg );
					return TRUE;
	//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
				case PSN_SETACTIVE:
					m_nPageNum = ID_PAGENUM_FILENAME;
					return TRUE;
				}
				break;
			}
		}
		break;

	case WM_COMMAND:
		{
			WORD	wNotifyCode = HIWORD(wParam);	// �ʒm�R�[�h
			WORD	wID = LOWORD(wParam);			// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
			HWND	hwndCtl = (HWND) lParam;		// �R���g���[���̃n���h��
			int		nCount;

			switch( wNotifyCode ){
			// �{�^�����N���b�N���ꂽ
			case BN_CLICKED:
				hListView = GetDlgItem( hwndDlg, IDC_LIST_FNAME );
				nIndex = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
				switch( wID ){
				case IDC_BUTTON_FNAME_INS:	// �}��
					// �I�𒆂̃L�[��T��
					nCount = ListView_GetItemCount( hListView );
					if( -1 == nIndex ){
						// �I�𒆂łȂ���΍Ō�ɒǉ�
						nIndex = nCount;
					}
					::DlgItem_GetText( hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH );
					::DlgItem_GetText( hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH );

					if( -1 != SetListViewItem_FILENAME( hListView, nIndex, szFrom, szTo, true ) ){
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_ADD:	// �ǉ�
					nCount = ListView_GetItemCount( hListView );

					::DlgItem_GetText( hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH );
					::DlgItem_GetText( hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH );
					
					if( -1 != SetListViewItem_FILENAME( hListView, nCount, szFrom, szTo, true ) ){
						return TRUE;
					}
					break;

				case IDC_BUTTON_FNAME_UPD:	// �X�V
					if( -1 != nIndex ){
						::DlgItem_GetText( hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH );
						::DlgItem_GetText( hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH );
						if( -1 != SetListViewItem_FILENAME( hListView, nIndex, szFrom, szTo, false ) ){
							return TRUE;
						}
					}else{
						// ���I���Ń��X�g�ɂЂƂ����ڂ��Ȃ��ꍇ�͒ǉ����Ă���
						if( 0 == ListView_GetItemCount( hListView ) ){
							if( -1 != SetListViewItem_FILENAME( hListView, 0, szFrom, szTo, true ) ){
								return TRUE;
							}
						}
					}
					break;
				case IDC_BUTTON_FNAME_DEL:	// �폜
					if( -1 != nIndex ){
						ListView_DeleteItem( hListView, nIndex );	//�Â��L�[���폜
						ListView_SetItemState( hListView, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_TOP:	// �擪
					if( -1 != MoveListViewItem_FILENAME( hListView, nIndex, 0 ) ){
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_UP: 	// ���
					if( -1 != MoveListViewItem_FILENAME( hListView, nIndex, nIndex - 1 ) ){
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_DOWN:	// ����
					if( -1 != MoveListViewItem_FILENAME( hListView, nIndex, nIndex + 1 ) ){
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_LAST:	// �ŏI
					nCount = ListView_GetItemCount( hListView );
					if( -1 != MoveListViewItem_FILENAME( hListView, nIndex, nCount - 1 ) ){
						return TRUE;
					}
					break;
				// default:
				}
				break;
			// default:
			}
		}

		break;	/* WM_COMMAND */
//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}



/*!
	�_�C�A���O��̃R���g���[���Ƀf�[�^��ݒ肷��

	@param hwndDlg �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void CPropFileName::SetData( HWND hwndDlg )
{
	int nIndex;
	int i;
	LVITEM lvItem;


	// �t�@�C�����u�����X�g
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_LIST_FNAME );
	ListView_DeleteAllItems( hListView ); // ���X�g����ɂ���

	// ���X�g�Ƀf�[�^���Z�b�g
	for( i = 0, nIndex = 0; i < m_Common.m_sFileName.m_nTransformFileNameArrNum; i++ ){
		if( '\0' == m_Common.m_sFileName.m_szTransformFileNameFrom[i][0] ) continue;

		::ZeroMemory( &lvItem, sizeof_raw( lvItem ));
		lvItem.mask     = LVIF_TEXT;
		lvItem.iItem    = nIndex;
		lvItem.iSubItem = 0;
		lvItem.pszText  = m_Common.m_sFileName.m_szTransformFileNameFrom[i];
		ListView_InsertItem( hListView, &lvItem );

		::ZeroMemory( &lvItem, sizeof_raw( lvItem ));
		lvItem.mask     = LVIF_TEXT;
		lvItem.iItem    = nIndex;
		lvItem.iSubItem = 1;
		lvItem.pszText  = m_Common.m_sFileName.m_szTransformFileNameTo[i];
		ListView_SetItem( hListView, &lvItem );

		nIndex++;
	}

	// ��ԏ��I�����Ă���
	if( 0 != nIndex ){
		ListView_SetItemState( hListView, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}
	//	���X�g�r���[�̍s�I�����\�ɂ���D
	DWORD dwStyle;
	dwStyle = ListView_GetExtendedListViewStyle( hListView );
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( hListView, dwStyle );

	return;
}

/*!
	�_�C�A���O��̃R���g���[������f�[�^���擾���ă������Ɋi�[����

	@param hwndDlg �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/

int CPropFileName::GetData( HWND hwndDlg )
{

	int nIndex;
	int nCount;

	// �t�@�C�������ȈՕ\������
//	if( IsDlgButtonChecked( hwndDlg, IDC_CHECK_FNAME ) ){
//		m_bUseTransformFileName = TRUE;
//	}else{
//		m_bUseRegexKeyword = FALSE;
//	}

	// �t�@�C�����u�����X�g
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_LIST_FNAME );
	m_Common.m_sFileName.m_nTransformFileNameArrNum = ListView_GetItemCount( hListView );

	for( nIndex = 0, nCount = 0; nIndex < MAX_TRANSFORM_FILENAME ; ++nIndex ){
		if( nIndex < m_Common.m_sFileName.m_nTransformFileNameArrNum ){
			ListView_GetItemText( hListView, nIndex, 0, m_Common.m_sFileName.m_szTransformFileNameFrom[nCount], _MAX_PATH );

			// �u���O������NULL��������̂Ă�
			if( L'\0' == m_Common.m_sFileName.m_szTransformFileNameFrom[nCount][0] ){
				m_Common.m_sFileName.m_szTransformFileNameTo[nIndex][0] = L'\0';
			}else{
				ListView_GetItemText( hListView, nIndex, 1, m_Common.m_sFileName.m_szTransformFileNameTo[nCount], _MAX_PATH );
				nCount++;
			}
		}else{
			m_Common.m_sFileName.m_szTransformFileNameFrom[nIndex][0] = L'\0';
			m_Common.m_sFileName.m_szTransformFileNameTo[nIndex][0] = L'\0';
		}
	}

	return TRUE;
}


int CPropFileName::SetListViewItem_FILENAME( HWND hListView, int nIndex, LPTSTR szFrom, LPTSTR szTo, bool bInsMode )
{
	LV_ITEM	Item;
	int nCount;

	if( _T('\0') == szFrom[0] || -1 == nIndex ) return -1;

	nCount = ListView_GetItemCount( hListView );

	// ����ȏ�ǉ��ł��Ȃ�
	if( bInsMode && MAX_TRANSFORM_FILENAME <= nCount ){
		ErrorMessage( GetParent( hListView ), _T("����ȏ�o�^�ł��܂���B") );
		return -1;
	}

	::ZeroMemory( &Item, sizeof_raw( Item ));
	Item.mask     = LVIF_TEXT;
	Item.iItem    = nIndex;
	Item.iSubItem = 0;
	Item.pszText  = szFrom;
	if( bInsMode ){
		ListView_InsertItem( hListView, &Item );
	}else{
		ListView_SetItem( hListView, &Item );
	}

	::ZeroMemory( &Item, sizeof_raw( Item ));
	Item.mask     = LVIF_TEXT;
	Item.iItem    = nIndex;
	Item.iSubItem = 1;
	Item.pszText  = szTo;
	ListView_SetItem( hListView, &Item );

	ListView_SetItemState( hListView, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	return nIndex;
}


void CPropFileName::GetListViewItem_FILENAME( HWND hListView, int nIndex, LPTSTR szFrom, LPTSTR szTo )
{
	ListView_GetItemText( hListView, nIndex, 0, szFrom, _MAX_PATH );
	ListView_GetItemText( hListView, nIndex, 1, szTo, _MAX_PATH );
}


int CPropFileName::MoveListViewItem_FILENAME( HWND hListView, int nIndex, int nIndex2 )
{
	TCHAR szFrom[_MAX_PATH];
	TCHAR szTo[_MAX_PATH];
	int nCount = ListView_GetItemCount( hListView );

	//	2004.03.24 dskoba
	if( nIndex > nCount - 1 ){
		nIndex = nCount - 1;
	}
	if( nIndex2 > nCount - 1 ){
		nIndex2 = nCount - 1;
	}
	if( nIndex < 0 ){
		nIndex = 0;
	}
	if( nIndex2 < 0 ){
		nIndex2 = 0;
	}
	
	if( nIndex == nIndex2 ){
		return -1;
	}

	GetListViewItem_FILENAME( hListView, nIndex, szFrom, szTo );
	ListView_DeleteItem( hListView, nIndex );	//�Â��L�[���폜
	SetListViewItem_FILENAME( hListView, nIndex2, szFrom, szTo, true );
	return nIndex2;
}
