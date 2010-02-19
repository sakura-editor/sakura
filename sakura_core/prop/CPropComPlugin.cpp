/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�v���O�C���v�y�[�W

	@author syat
*/
/*
	Copyright (C) 2009, syat

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
#include <WindowsX.h>
#include "prop/CPropCommon.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "sakura_rc.h"
#include "plugin/CPluginManager.h"

//! Popup Help�pID
#include "sakura.hh"
static const DWORD p_helpids[] = {	//11700
	IDC_CHECK_PluginEnable,	HIDC_CHECK_PluginEnable,	//�v���O�C����L���ɂ���
	IDC_PLUGINLIST,			HIDC_PLUGINLIST,			//�v���O�C�����X�g
	IDC_PLUGIN_SearchNew,	HIDC_PLUGIN_SearchNew,		//�V�K�v���O�C����ǉ�
	IDC_PLUGIN_Remove,		HIDC_PLUGIN_Remove,			//�V�K�v���O�C����ǉ�
//	IDC_STATIC,			-1,
	0, 0
};

/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_PLUGIN(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc(&CPropCommon::DispatchEvent_PROP_PLUGIN, hwndDlg, uMsg, wParam, lParam );
}

/*! Plugin�y�[�W�̃��b�Z�[�W����
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handlw
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CPropCommon::DispatchEvent_PROP_PLUGIN( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;

	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p1 */
		InitDialog_PROP_PLUGIN( hwndDlg );
		SetData_PROP_PLUGIN( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		return TRUE;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
		case IDC_PLUGINLIST:
			switch( pNMHDR->code ){
			case LVN_ITEMCHANGED:
				{
					HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );
					int sel = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
					if( sel >= 0 ){
						CPlugin* plugin = CPluginManager::Instance()->GetPlugin(sel);
						if( plugin != NULL ){
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Description ), to_tchar(plugin->m_sDescription.c_str()) );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Author ), to_tchar(plugin->m_sAuthor.c_str()) );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Version ), to_tchar(plugin->m_sVersion.c_str()) );
						}else{
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Description ), _T("") );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Author ), _T("") );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Version ), _T("") );
						}
					}
				}
			}
			break;
		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_PLUGIN );
				return TRUE;
			case PSN_KILLACTIVE:
				/* �_�C�A���O�f�[�^�̎擾 p1 */
				GetData_PROP_PLUGIN( hwndDlg );
				return TRUE;
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_PLUGIN;
				return TRUE;
			}
			break;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID = LOWORD(wParam);			/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl = (HWND) lParam;		/* �R���g���[���̃n���h�� */

		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_PLUGIN_SearchNew:		// �V�K�v���O�C����ǉ�
				CPluginManager::Instance()->SearchNewPlugin( m_Common );
				SetData_PROP_PLUGIN_LIST( hwndDlg );	//���X�g�̍č\�z
				break;
			case IDC_CHECK_PluginEnable:	// �v���O�C����L���ɂ���
				EnablePluginPropInput( hwndDlg );
				break;
			case IDC_PLUGIN_Remove:			// �v���O�C�����폜
				{
					HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );
					int sel = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
					if( sel >= 0 ){
						CPlugin* plugin = CPluginManager::Instance()->GetPlugin( sel );
						if( plugin && MYMESSAGEBOX( hwndDlg, MB_YESNO, GSTR_APPNAME, to_tchar((plugin->m_sName + std::wstring(L" ���폜���܂���")).c_str()) ) == IDYES ){
							CPluginManager::Instance()->UninstallPlugin( m_Common, sel );
							SetData_PROP_PLUGIN_LIST( hwndDlg );
						}
					}
				}
				break;
			}
			break;
		case CBN_DROPDOWN:
			//switch( wID ){
			//default:
			//	break;
			//}
			break;	/* CBN_DROPDOWN */
		case EN_KILLFOCUS:
			//switch( wID ){
			//default:
			//	break;
			//}
			break;
		}

		break;	/* WM_COMMAND */
//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
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
void CPropCommon::SetData_PROP_PLUGIN( HWND hwndDlg )
{
	//�v���O�C����L���ɂ���
	::CheckDlgButton( hwndDlg, IDC_CHECK_PluginEnable, m_Common.m_sPlugin.m_bEnablePlugin );

	//�v���O�C�����X�g
	SetData_PROP_PLUGIN_LIST( hwndDlg );
	
	EnablePluginPropInput( hwndDlg );
	return;
}

/*!
	�_�C�A���O��̃R���g���[���Ƀf�[�^��ݒ肷��

	@param hwndDlg �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void CPropCommon::SetData_PROP_PLUGIN_LIST( HWND hwndDlg )
{
	int index;
	LVITEM sItem;
	PluginRec* plugin_table = m_Common.m_sPlugin.m_PluginTable;

	//�v���O�C�����X�g
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );

	ListView_DeleteAllItems( hListView );

	for( index = 0; index < MAX_PLUGIN; ++index ){
		CPlugin* plugin = CPluginManager::Instance()->GetPlugin( index );

		//�ԍ�
		TCHAR buf[4];
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.mask = LVIF_TEXT | LVIF_PARAM;
		sItem.iItem = index;
		sItem.iSubItem = 0;
		_itot( index, buf, 10 );
		sItem.pszText = buf;
		sItem.lParam = index;
		ListView_InsertItem( hListView, &sItem );

		//���O
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 1;
		sItem.pszText = plugin ? const_cast<LPTSTR>( to_tchar(plugin->m_sName.c_str()) ) : _T("");
		ListView_SetItem( hListView, &sItem );

		//���
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 2;
		if( plugin ){
			sItem.pszText = _T("�ғ�");
		}else{
			sItem.pszText = _T("��~");
		}
		ListView_SetItem( hListView, &sItem );

		//�t�H���_
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 3;
		sItem.pszText = const_cast<LPTSTR>( to_tchar(plugin_table[index].m_szName) );
		ListView_SetItem( hListView, &sItem );
	}
	
	//	���X�g�r���[�̍s�I�����\�ɂ���D
	//	IE 3.x�ȍ~�������Ă���ꍇ�̂ݓ��삷��D
	//	���ꂪ�����Ă��C�ԍ����������I���ł��Ȃ������ő��쎩�͉̂\�D
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
int CPropCommon::GetData_PROP_PLUGIN( HWND hwndDlg )
{
	//�v���O�C����L���ɂ���
	m_Common.m_sPlugin.m_bEnablePlugin = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_PluginEnable );

	//�v���O�C�����X�g�͍��̂Ƃ���ύX�ł��镔�����Ȃ�
	//�u�V�K�v���O�C���ǉ��v��m_Common�ɒ��ڏ������ނ̂ŁA���̊֐��ł��邱�Ƃ͂Ȃ�

	return TRUE;
}

/*!
	�_�C�A���O��̃R���g���[��������������

	@param hwndDlg �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void CPropCommon::InitDialog_PROP_PLUGIN( HWND hwndDlg )
{
	struct ColumnData {
		TCHAR *title;
		int width;
	} ColumnList[] = {
		{ _T("�ԍ�"), 40 },
		{ _T("�v���O�C����"), 150 },
		{ _T("���"), 40 },
		{ _T("�t�H���_"), 80 },
	};

	//	ListView�̏�����
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );
	if( hListView == NULL ){
		::MessageBox( hwndDlg, _T("PropComMacro::InitDlg::NoListView"), _T("�o�O�񍐂��肢"), MB_OK );
		return;	//	�悭�킩��񂯂ǎ��s����	
	}

	LVCOLUMN sColumn;
	int pos;
	
	for( pos = 0; pos < _countof( ColumnList ); ++pos ){
		
		memset_raw( &sColumn, 0, sizeof( sColumn ));
		sColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
		sColumn.pszText = ColumnList[pos].title;
		sColumn.cx = ColumnList[pos].width;
		sColumn.iSubItem = pos;
		sColumn.fmt = LVCFMT_LEFT;
		
		if( ListView_InsertColumn( hListView, pos, &sColumn ) < 0 ){
			::MessageBox( hwndDlg, _T("PropComMacro::InitDlg::ColumnRegistrationFail"), _T("�o�O�񍐂��肢"), MB_OK );
			return;	//	�悭�킩��񂯂ǎ��s����
		}
	}

}

/*! �u�v���O�C���v�V�[�g��̃A�C�e���̗L���E������K�؂ɐݒ肷��

	@date 2009.12.06 syat �V�K�쐬
*/
void CPropCommon::EnablePluginPropInput(HWND hwndDlg)
{
	if( !::IsDlgButtonChecked( hwndDlg, IDC_CHECK_PluginEnable ) )
	{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGINLIST               ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_SearchNew         ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_Remove            ), FALSE );
	}
	else
	{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGINLIST               ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_SearchNew         ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_Remove            ), TRUE );
	}
}
