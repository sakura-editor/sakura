//	$Id$
/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�}�N���v�y�[�W

	@author genta
	@date Jun. 2, 2001 genta
	$Revision$

*/
/*
	Copyright (C) 1998-2001, genta

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "CPropCommon.h"
#include "memory.h"
#include "stdlib.h"
#include <Shlobj.h>

//! Popup Help�pID
//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//10500
	0, 0
};
//@@@ 2001.02.04 End

/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
BOOL CALLBACK CPropCommon::DlgProc_PROP_MACRO(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc(DispatchEvent_PROP_Macro, hwndDlg, uMsg, wParam, lParam );
}

/*! Macro�y�[�W�̃��b�Z�[�W����
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handlw
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
BOOL CPropCommon::DispatchEvent_PROP_Macro( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
		InitDialog_PROP_Macro( hwndDlg );
		SetData_PROP_Macro( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		return TRUE;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
		case IDC_MACROLIST:
			switch( pNMHDR->code ){
			case LVN_ITEMCHANGED:
				CheckListPosition_Macro( hwndDlg );
				break;
			}
			break;
		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_MACRO );
				return TRUE;
			case PSN_KILLACTIVE:
				/* �_�C�A���O�f�[�^�̎擾 p1 */
				GetData_PROP_Macro( hwndDlg );
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
			case IDC_MACRODIRREF:	// �}�N���f�B���N�g���Q��
				SelectBaseDir_Macro( hwndDlg );
				break;
			case IDC_MACRO_REG:		// �}�N���ݒ�
				SetMacro2List_Macro( hwndDlg );
				break;
			}
			break;
		case CBN_DROPDOWN:
			switch( wID ){
			case IDC_MACROPATH:
				OnFileDropdown_Macro( hwndDlg );
				break;
			}
		}

		break;
//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
		//	HELPINFO *p = (HELPINFO *)lParam;
		//	::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
		break;
//@@@ 2001.02.04 End

	}
	return FALSE;
}


/*!
	�_�C�A���O��̃R���g���[���Ƀf�[�^��ݒ肷��

	@param hwndDlg �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void CPropCommon::SetData_PROP_Macro( HWND hwndDlg )
{
	int index;
	LVITEM sItem;

	//	�}�N���f�[�^
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_MACROLIST );
	
	for( index = 0; index < MAX_CUSTMACRO; ++index ){
		memset( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 1;
		sItem.pszText = m_pShareData->m_MacroTable[index].m_szName;
		ListView_SetItem( hListView, &sItem );

		memset( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 2;
		sItem.pszText = m_pShareData->m_MacroTable[index].m_szFile;
		ListView_SetItem( hListView, &sItem );
	}
	
	//	�}�N���f�B���N�g��
	::SetDlgItemText( hwndDlg, IDC_MACRODIR, m_pShareData->m_szMACROFOLDER );

	nLastPos_Macro = -1;
	
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

int CPropCommon::GetData_PROP_Macro( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_MACRO;

	int index;
	LVITEM sItem;

	//	�}�N���f�[�^
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_MACROLIST );

	for( index = 0; index < MAX_CUSTMACRO; ++index ){
		memset( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 1;
		sItem.cchTextMax = MACRONAME_MAX - 1;
		sItem.pszText = m_pShareData->m_MacroTable[index].m_szName;
		ListView_GetItem( hListView, &sItem );

		memset( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 2;
		sItem.cchTextMax = _MAX_PATH;
		sItem.pszText = m_pShareData->m_MacroTable[index].m_szFile;
		ListView_GetItem( hListView, &sItem );
	}

	//	�}�N���f�B���N�g��
	::GetDlgItemText( hwndDlg, IDC_MACRODIR, m_pShareData->m_szMACROFOLDER, _MAX_PATH );

	return TRUE;
}

void CPropCommon::InitDialog_PROP_Macro( HWND hwndDlg )
{
	struct ColumnData {
		char *title;
		int width;
	} ColumnList[] = {
		{ "�ԍ�", 40 },
		{ "�}�N����", 200 },
		{ "�t�@�C����", 200 },
	};

	//	ListView�̏�����
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_MACROLIST );
	if( hListView == NULL ){
		::MessageBox( hwndDlg, "PropComMacro::InitDlg::NoListView", "�o�O�񍐂��肢", MB_OK );
		return;	//	�悭�킩��񂯂ǎ��s����	
	}

	LVCOLUMN sColumn;
	int pos;
	
	for( pos = 0; pos < sizeof( ColumnList ) / sizeof( ColumnList[0] ); ++pos ){
		
		memset( &sColumn, 0, sizeof( sColumn ));
		sColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
		sColumn.pszText = ColumnList[pos].title;
		sColumn.cx = ColumnList[pos].width;
		sColumn.iSubItem = pos;
		sColumn.fmt = LVCFMT_LEFT;
		
		if( ListView_InsertColumn( hListView, pos, &sColumn ) < 0 ){
			::MessageBox( hwndDlg, "PropComMacro::InitDlg::ColumnRegistrationFail", "�o�O�񍐂��肢", MB_OK );
			return;	//	�悭�킩��񂯂ǎ��s����
		}
	}

	//	�������̊m��
	//	�K�v�Ȑ�������Ɋm�ۂ���D
	ListView_SetItemCount( hListView, MAX_CUSTMACRO );

	//	Index�����̓o�^
	for( pos = 0; pos < MAX_CUSTMACRO ; ++pos ){
		LVITEM sItem;
		char buf[4];
		memset( &sItem, 0, sizeof( sItem ));
		sItem.mask = LVIF_TEXT | LVIF_PARAM;
		sItem.iItem = pos;
		sItem.iSubItem = 0;
		itoa( pos, buf, 10 );
		sItem.pszText = buf;
		sItem.lParam = pos;
		ListView_InsertItem( hListView, &sItem );
	}
	
	// �o�^��w�� ComboBox�̏�����
	HWND hNumCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_MACROID );
	for( pos = 0; pos < MAX_CUSTMACRO ; ++pos ){
		char buf[10];
		wsprintf( buf, "%d", pos );
		int result = ::SendMessage( hNumCombo, CB_ADDSTRING, (WPARAM)0, (LPARAM)buf );
		if( result == CB_ERR ){
			::MessageBox( hwndDlg, "PropComMacro::InitDlg::AddMacroId", "�o�O�񍐂��肢", MB_OK );
			return;	//	�悭�킩��񂯂ǎ��s����
		}
		else if( result == CB_ERRSPACE ){
			::MessageBox( hwndDlg, "PropComMacro::InitDlg::AddMacroId/InsufficientSpace",
				"�o�O�񍐂��肢", MB_OK );
			return;	//	�悭�킩��񂯂ǎ��s����
		}
	}
	::SendMessage( hNumCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );
}

void CPropCommon::SetMacro2List_Macro( HWND hwndDlg )
{
	int index;
	LVITEM sItem;
	char buf[256];
	
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_MACROLIST );
	HWND hNum = ::GetDlgItem( hwndDlg, IDC_COMBO_MACROID );

	//	�ݒ��擾
	index = ::SendMessage( hNum, CB_GETCURSEL, 0, 0 );
	if( index == CB_ERR ){
		::MessageBox( hwndDlg, "PropComMacro::SetMacro2List::GetCurSel",
			"�o�O�񍐂��肢", MB_OK );
		return;	//	�悭�킩��񂯂ǎ��s����
	}

	// �}�N����
	memset( &sItem, 0, sizeof( sItem ));
	sItem.iItem = index;
	sItem.mask = LVIF_TEXT;
	sItem.iSubItem = 1;
	
	::GetDlgItemText( hwndDlg, IDC_MACRONAME, buf, MACRONAME_MAX );
	sItem.pszText = buf;
	ListView_SetItem( hListView, &sItem );

	// �t�@�C����
	memset( &sItem, 0, sizeof( sItem ));
	sItem.iItem = index;
	sItem.mask = LVIF_TEXT;
	sItem.iSubItem = 2;

	::GetDlgItemText( hwndDlg, IDC_MACROPATH, buf, _MAX_PATH );
	sItem.pszText = buf;
	ListView_SetItem( hListView, &sItem );
}

/*!
	Macro�i�[�p�f�B���N�g����I������
	
	@param hwndDlg [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void CPropCommon::SelectBaseDir_Macro( HWND hwndDlg )
{

	LPMALLOC pMalloc;
	BROWSEINFO bi;
	TCHAR szDir[MAX_PATH + 1]; // �ǉ�����\\�p��1������
	LPITEMIDLIST pidl;

	char szInitial[MAX_PATH];

	if( SHGetMalloc(&pMalloc) == E_FAIL ){
		::MessageBox( hwndDlg, "PropComMacro::SelectBaseDir_Macro::SHGetMalloc",
			"�o�O�񍐂��肢", MB_OK );
		return;	//	�悭�킩��񂯂ǎ��s����
	}

	::GetDlgItemText( hwndDlg, IDC_MACRODIR, szInitial, MAX_PATH );

	ZeroMemory(&bi,sizeof(bi));
	bi.hwndOwner = hwndDlg;	 // �I�[�i�[�E�B���h�E�n���h����ݒ�
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szDir;
	bi.lpszTitle = "Macro�f�B���N�g���̑I��";
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = CPropCommon::DirCallback_Macro;
	bi.lParam = (LPARAM)szInitial;	// �����f�B���N�g��

	// �t�H���_�̎Q�ƃ_�C�A���O�{�b�N�X�̕\��
	pidl = SHBrowseForFolder(&bi);

	if (pidl)
	{
		if (SHGetPathFromIDList(pidl,szDir))
		{
			//	������\\�}�[�N��ǉ�����D
			int pos = strlen( szDir );
			if( szDir[ pos - 1 ] != '\\' ){
				szDir[ pos ] = '\\';
				szDir[ pos + 1 ] = '\0';
			}
			::SetDlgItemText( hwndDlg, IDC_MACRODIR, szDir );
		}

		// SHBrowseForFolder �ɂ���Ċ��蓖�Ă�ꂽ PIDL �����
		pMalloc->Free(pidl);
	}

	// Shell �̃A���P�[�^���J��
	pMalloc->Release();
}

/*!
	�t�H���_�I���_�C�A���O�{�b�N�X�pCallback�֐�

	SHBrowseForFolder�̏����f�B���N�g�����w�肷�邽�߂̃R�[���o�b�N�֐�
	
	@param hwnd [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
	@param uMsg [in] �ʒm���
	@param lParam [in] 
	@param lpData [in] BROWSEINFO�œn���ꂽ�l�D
					�����ł́C�����f�B���N�g���ւ̃|�C���^(const char*)���L���X�g����ē����Ă���D
*/
int CALLBACK CPropCommon::DirCallback_Macro( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
	if( uMsg == BFFM_INITIALIZED ){	//	����������
		::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}
/*!
	�}�N���t�@�C���w��p�R���{�{�b�N�X�̃h���b�v�_�E�����X�g���J�����Ƃ��ɁC
	�w��f�B���N�g���̃t�@�C���ꗗ������𐶐�����D

	@param hwndDlg [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void CPropCommon::OnFileDropdown_Macro( HWND hwndDlg )
{
	char path[_MAX_PATH * 2 ];
	WIN32_FIND_DATA wf;
	HANDLE hFind;
	HWND hCombo = ::GetDlgItem( hwndDlg, IDC_MACROPATH );

	::GetDlgItemText( hwndDlg, IDC_MACRODIR, path, _MAX_PATH );
	strcat( path, "*.mac" );

	//	���̏�����
	::SendMessage( hCombo, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0 );

	//	�t�@�C���̌���
	hFind = FindFirstFile(path, &wf);

	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}
	
	do {
		//	�R���{�{�b�N�X�ɐݒ�
		int result = ::SendMessage( hCombo, CB_ADDSTRING, (WPARAM)0, (LPARAM)wf.cFileName );
		if( result == CB_ERR || result == CB_ERRSPACE )
			break;
	} while( FindNextFile( hFind, &wf ));

    FindClose(hFind);
}

void CPropCommon::CheckListPosition_Macro( HWND hwndDlg )
{
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_MACROLIST );
	HWND hNum = ::GetDlgItem( hwndDlg, IDC_COMBO_MACROID );
	
	//	���݂�Focus�擾
	int current = ListView_GetNextItem( hListView, -1, LVNI_SELECTED);

	if( current == -1 || current == nLastPos_Macro )
		return;

	nLastPos_Macro = current;
	
	//	�����l�̐ݒ�
	::SendMessage( hNum, CB_SETCURSEL, nLastPos_Macro, 0 );
	
	char buf[MAX_PATH + MACRONAME_MAX];	// MAX_PATH��MACRONAME_MAX�̗������傫���l
	LVITEM sItem;

	memset( &sItem, 0, sizeof( sItem ));
	sItem.iItem = current;
	sItem.mask = LVIF_TEXT;
	sItem.iSubItem = 1;
	sItem.pszText = buf;
	sItem.cchTextMax = MACRONAME_MAX;

	ListView_GetItem( hListView, &sItem );
	::SetDlgItemText( hwndDlg, IDC_MACRONAME, buf );

	memset( &sItem, 0, sizeof( sItem ));
	sItem.iItem = current;
	sItem.mask = LVIF_TEXT;
	sItem.iSubItem = 2;
	sItem.pszText = buf;
	sItem.cchTextMax = MAX_PATH;

	ListView_GetItem( hListView, &sItem );
	::SetDlgItemText( hwndDlg, IDC_MACROPATH, buf );

}