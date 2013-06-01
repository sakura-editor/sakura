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


#include "StdAfx.h"
#include <ShellAPI.h>
#include "prop/CPropCommon.h"
#include "util/shell.h"
#include "dlg/CDlgOpenFile.h"
#include "dlg/CDlgPluginOption.h"	// 2010/3/22 Uchi
#include "io/CTextStream.h"
#include "io/CZipFile.h"
#include "sakura_rc.h"
#include "sakura.hh"

//! Popup Help�pID
static const DWORD p_helpids[] = {	//11700
	IDC_CHECK_PluginEnable,	HIDC_CHECK_PluginEnable,	//�v���O�C����L���ɂ���
	IDC_PLUGINLIST,			HIDC_PLUGINLIST,			//�v���O�C�����X�g
	IDC_PLUGIN_INST_ZIP,	HIDC_PLUGIN_INST_ZIP,		//Zip�v���O�C����ǉ�	// 2011/11/2 Uchi
	IDC_PLUGIN_SearchNew,	HIDC_PLUGIN_SearchNew,		//�V�K�v���O�C����ǉ�
	IDC_PLUGIN_OpenFolder,	HIDC_PLUGIN_OpenFolder,		//�t�H���_���J��
	IDC_PLUGIN_Remove,		HIDC_PLUGIN_Remove,			//�v���O�C�����폜
	IDC_PLUGIN_OPTION,		HIDC_PLUGIN_OPTION,			//�v���O�C���ݒ�	// 2010/3/22 Uchi
	IDC_PLUGIN_README,		HIDC_PLUGIN_README,			//ReadMe�\��		// 2011/11/2 Uchi
//	IDC_STATIC,			-1,
	0, 0
};

/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropPlugin::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropPlugin::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}

/*! Plugin�y�[�W�̃��b�Z�[�W����
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handlw
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CPropPlugin::DispatchEvent( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;

	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� Plugin */
		InitDialog( hwndDlg );
		SetData( hwndDlg );
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
						CPlugin* plugin = CPluginManager::getInstance()->GetPlugin(sel);
						if( plugin != NULL ){
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Description ), to_tchar(plugin->m_sDescription.c_str()) );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Author ), to_tchar(plugin->m_sAuthor.c_str()) );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Version ), to_tchar(plugin->m_sVersion.c_str()) );
						}else{
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Description ), _T("") );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Author ), _T("") );
							::SetWindowText( ::GetDlgItem( hwndDlg, IDC_LABEL_PLUGIN_Version ), _T("") );
						}
						// 2010.08.21 ���炩�Ɏg���Ȃ��Ƃ���Disable�ɂ���
						EPluginState state = m_Common.m_sPlugin.m_PluginTable[sel].m_state;
						BOOL bEdit = (state != PLS_DELETED && state != PLS_NONE);
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_Remove ), bEdit );
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_OPTION ), state == PLS_LOADED && plugin && plugin->m_options.size() > 0 );
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_README ), 
							(state == PLS_INSTALLED || state == PLS_UPDATED || state == PLS_LOADED || state == PLS_DELETED)
							&& !GetReadMeFile(to_tchar(m_Common.m_sPlugin.m_PluginTable[sel].m_szName)).empty());
					}
				}
				break;
			case NM_DBLCLK:
				// ���X�g�r���[�ւ̃_�u���N���b�N�Łu�v���O�C���ݒ�v���Ăяo��
				if (::IsWindowEnabled(::GetDlgItem( hwndDlg, IDC_PLUGIN_OPTION )))
				{
					DispatchEvent( hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_PLUGIN_OPTION, BN_CLICKED), (LPARAM)::GetDlgItem( hwndDlg, IDC_PLUGIN_OPTION ) );
				}
				break;
			}
			break;
		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_PLUGIN );
				return TRUE;
			case PSN_KILLACTIVE:
				/* �_�C�A���O�f�[�^�̎擾 Plugin */
				GetData( hwndDlg );
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
				CPluginManager::getInstance()->SearchNewPlugin( m_Common, hwndDlg );
				SetData_LIST( hwndDlg );	//���X�g�̍č\�z
				break;
			case IDC_PLUGIN_INST_ZIP:		// ZIP�v���O�C����ǉ�
				{
					static std::tstring	sTrgDir;
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					_tcscpy( szPath, (sTrgDir.empty() ? CPluginManager::getInstance()->GetBaseDir().c_str() : sTrgDir.c_str()));
					// �t�@�C���I�[�v���_�C�A���O�̏�����
					cDlgOpenFile.Create(
						G_AppInstance(),
						hwndDlg,
						_T("*.zip"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						CPluginManager::getInstance()->InstZipPlugin( m_Common, hwndDlg, szPath );
						SetData_LIST( hwndDlg );	//���X�g�̍č\�z
					}
					// �t�H���_���L��
					TCHAR	szFolder[_MAX_PATH + 1];
					TCHAR	szFname[_MAX_PATH + 1];
					SplitPath_FolderAndFile(szPath, szFolder, szFname);
					sTrgDir = szFolder;
				}
				break;
			case IDC_CHECK_PluginEnable:	// �v���O�C����L���ɂ���
				EnablePluginPropInput( hwndDlg );
				break;
			case IDC_PLUGIN_Remove:			// �v���O�C�����폜
				{
					HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );
					int sel = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
					if( sel >= 0 ){
						if( MYMESSAGEBOX( hwndDlg, MB_YESNO, GSTR_APPNAME, to_tchar((m_Common.m_sPlugin.m_PluginTable[sel].m_szName + std::wstring(L" ���폜���܂���")).c_str()) ) == IDYES ){
							CPluginManager::getInstance()->UninstallPlugin( m_Common, sel );
							SetData_LIST( hwndDlg );
						}
					}
				}
				break;
			case IDC_PLUGIN_OPTION:		// �v���O�C���ݒ�	// 2010/3/22 Uchi
				{
					HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );
					int sel = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
					if( sel >= 0 && m_Common.m_sPlugin.m_PluginTable[sel].m_state == PLS_LOADED ){
						// 2010.08.21 �v���O�C����(�t�H���_��)�̓��ꐫ�̊m�F
						CPlugin* plugin = CPluginManager::getInstance()->GetPlugin(sel);
						wstring sDirName = to_wchar(plugin->GetFolderName().c_str());
						if( plugin && 0 == auto_stricmp(sDirName.c_str(), m_Common.m_sPlugin.m_PluginTable[sel].m_szName ) ){
							CDlgPluginOption cDlgPluginOption;
							cDlgPluginOption.DoModal( ::GetModuleHandle(NULL), hwndDlg, this, sel );
						}else{
							WarningMessage( hwndDlg, _T("�v���O�C���͂��̃E�B���h�E�œǂݍ��܂�Ă��Ȃ����A�t�H���_���قȂ邽��\n�ݒ��ύX�ł��܂���") );
						}
					}
				}
				break;
			case IDC_PLUGIN_OpenFolder:			// �t�H���_���J��
				{
					std::tstring sBaseDir = CPluginManager::getInstance()->GetBaseDir() + _T(".");
					if( ! IsDirectory(sBaseDir.c_str()) ){
						if( ::CreateDirectory(sBaseDir.c_str(), NULL) == 0 ){
							break;
						}
					}
					::ShellExecute( NULL, _T("open"), sBaseDir.c_str(), NULL, NULL, SW_SHOW );
				}
				break;
			case IDC_PLUGIN_README:		// ReadMe�\��	// 2011/11/2 Uchi
				{
					HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );
					int sel = ListView_GetNextItem( hListView, -1, LVNI_SELECTED );
					std::tstring sName = to_tchar(m_Common.m_sPlugin.m_PluginTable[sel].m_szName);	// �ʃt�H���_��
					std::tstring sReadMeName = GetReadMeFile(sName);
					if (!sReadMeName.empty()) {
						if (!BrowseReadMe(sReadMeName)) {
							WarningMessage( hwndDlg, _T("ReadMe�t�@�C�����J���܂���") );
						}
					}else{
						WarningMessage( hwndDlg, _T("ReadMe�t�@�C����������܂��� ") );
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
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


/*!
	�_�C�A���O��̃R���g���[���Ƀf�[�^��ݒ肷��

	@param hwndDlg �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void CPropPlugin::SetData( HWND hwndDlg )
{
	//�v���O�C����L���ɂ���
	::CheckDlgButton( hwndDlg, IDC_CHECK_PluginEnable, m_Common.m_sPlugin.m_bEnablePlugin );

	//�v���O�C�����X�g
	SetData_LIST( hwndDlg );
	
	EnablePluginPropInput( hwndDlg );
	return;
}

/*!
	�_�C�A���O��̃R���g���[���Ƀf�[�^��ݒ肷��

	@param hwndDlg �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void CPropPlugin::SetData_LIST( HWND hwndDlg )
{
	int index;
	LVITEM sItem;
	PluginRec* plugin_table = m_Common.m_sPlugin.m_PluginTable;

	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_Remove ), FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_OPTION ), FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_README ), FALSE );

	//�v���O�C�����X�g
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );

	ListView_DeleteAllItems( hListView );

	for( index = 0; index < MAX_PLUGIN; ++index ){
		std::basic_string<TCHAR> sDirName;	//CPlugin.GetDirName()�̌��ʕێ��ϐ�
		CPlugin* plugin = CPluginManager::getInstance()->GetPlugin( index );

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
		if( plugin ){
			sItem.pszText = const_cast<LPTSTR>( to_tchar(plugin->m_sName.c_str()) );
		}else{
			sItem.pszText = _T("-");
		}
		ListView_SetItem( hListView, &sItem );

		//���
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 2;
		switch( plugin_table[index].m_state ){
		case PLS_INSTALLED: sItem.pszText = _T("�ǉ�"); break;
		case PLS_UPDATED:   sItem.pszText = _T("�X�V"); break;
		case PLS_STOPPED:   sItem.pszText = _T("��~"); break;
		case PLS_LOADED:    sItem.pszText = _T("�ғ�"); break;
		case PLS_DELETED:   sItem.pszText = _T("�폜"); break;
		case PLS_NONE:      sItem.pszText = _T(""); break;
		default:            sItem.pszText = _T("����`"); break;
		}
		ListView_SetItem( hListView, &sItem );
		
		//�Ǎ�
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 3;
		if( plugin_table[index].m_state != PLS_NONE ){
			sItem.pszText = const_cast<TCHAR*>(plugin ? _T("�Ǎ�") : _T(""));
		}else{
			sItem.pszText = _T("");
		}
		ListView_SetItem( hListView, &sItem );

		//�t�H���_
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 4;
		switch( plugin_table[index].m_state ){
		case PLS_INSTALLED:
		case PLS_UPDATED:
		case PLS_STOPPED:
		case PLS_LOADED:
			if( plugin ){
				sDirName = plugin->GetFolderName();
				sItem.pszText = const_cast<LPTSTR>( sDirName.c_str() );
			}else{
				sItem.pszText = const_cast<LPTSTR>( to_tchar(plugin_table[index].m_szName) );
			}
			break;
		default:
			sItem.pszText = _T("");
		}
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
int CPropPlugin::GetData( HWND hwndDlg )
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
void CPropPlugin::InitDialog( HWND hwndDlg )
{
	struct ColumnData {
		const TCHAR *title;
		int width;
	} ColumnList[] = {
		{ _T("�ԍ�"), 40 },
		{ _T("�v���O�C����"), 200 },
		{ _T("���"), 40 },
		{ _T("�Ǎ�"), 40 },
		{ _T("�t�H���_"), 150 },
	};

	//	ListView�̏�����
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_PLUGINLIST );

	LVCOLUMN sColumn;
	int pos;
	
	for( pos = 0; pos < _countof( ColumnList ); ++pos ){
		
		memset_raw( &sColumn, 0, sizeof( sColumn ));
		sColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
		sColumn.pszText = const_cast<TCHAR*>(ColumnList[pos].title);
		sColumn.cx = ColumnList[pos].width;
		sColumn.iSubItem = pos;
		sColumn.fmt = LVCFMT_LEFT;
		
		if( ListView_InsertColumn( hListView, pos, &sColumn ) < 0 ){
			PleaseReportToAuthor( hwndDlg, _T("PropComMacro::InitDlg::ColumnRegistrationFail") );
			return;	//	�悭�킩��񂯂ǎ��s����
		}
	}

}

/*! �u�v���O�C���v�V�[�g��̃A�C�e���̗L���E������K�؂ɐݒ肷��

	@date 2009.12.06 syat �V�K�쐬
	@date 2010.08.21 Moca �v���O�C��������Ԃł��폜����Ȃǂ��\�ɂ���
*/
void CPropPlugin::EnablePluginPropInput(HWND hwndDlg)
{
	if( !::IsDlgButtonChecked( hwndDlg, IDC_CHECK_PluginEnable ) )
	{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_SearchNew         ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_INST_ZIP          ), FALSE );
	}
	else
	{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_SearchNew         ), TRUE );
		CZipFile	cZipFile;
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_PLUGIN_INST_ZIP          ), cZipFile.IsOk() );
	}
}

//	Readme �t�@�C���̎擾	2011/11/2 Uchi
std::tstring CPropPlugin::GetReadMeFile(const std::tstring& sName)
{
	std::tstring sReadMeName = CPluginManager::getInstance()->GetBaseDir()
		+ sName + _T("\\ReadMe.txt");
	CFile* fl = new CFile(sReadMeName.c_str());
	if (!fl->IsFileExist()) {
		sReadMeName = CPluginManager::getInstance()->GetBaseDir()
			+ sName + _T("\\") + sName + _T(".txt");
		fl = new CFile(sReadMeName.c_str());
	}
	if (!fl->IsFileExist()) {
		// exe�t�H���_�z��
		sReadMeName = CPluginManager::getInstance()->GetExePluginDir()
			+ sName + _T("\\ReadMe.txt");
		fl = new CFile(sReadMeName.c_str());
		if (!fl->IsFileExist()) {
			sReadMeName = CPluginManager::getInstance()->GetExePluginDir()
				+ sName + _T("\\") + sName + _T(".txt");
			fl = new CFile(sReadMeName.c_str());
		}
	}

	if (!fl->IsFileExist()) {
		sReadMeName = _T("");
	}
	return sReadMeName;
}

//	Readme �t�@�C���̕\��	2011/11/2 Uchi
bool CPropPlugin::BrowseReadMe(const std::tstring& sReadMeName)
{
	// -- -- -- -- �R�}���h���C��������𐶐� -- -- -- -- //
	CCommandLineString cCmdLineBuf;

	//�A�v���P�[�V�����p�X
	TCHAR szExePath[MAX_PATH + 1];
	::GetModuleFileName( NULL, szExePath, _countof( szExePath ) );
	cCmdLineBuf.AppendF( _T("\"%ts\""), szExePath );

	// �t�@�C����
	cCmdLineBuf.AppendF( _T(" \"%ts\""), sReadMeName.c_str() );

	// �R�}���h���C���I�v�V����
	cCmdLineBuf.AppendF(_T(" -R -CODE=99"));

	// �O���[�vID
	int nGroup = GetDllShareData().m_sNodes.m_nGroupSequences;
	if( nGroup > 0 ){
		cCmdLineBuf.AppendF( _T(" -GROUP=%d"), nGroup+1 );
	}

	//CreateProcess�ɓn��STARTUPINFO���쐬
	STARTUPINFO	sui;
	::GetStartupInfo(&sui);

	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );

	TCHAR	szCmdLine[1024];
	auto_strcpy_s(szCmdLine, _countof(szCmdLine), cCmdLineBuf.c_str());
	return (::CreateProcess( NULL, szCmdLine, NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) != 0);
}
