/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�}�N���v�y�[�W

	@author genta
	@date Jun. 2, 2001 genta
*/
/*
	Copyright (C) 2001, genta, MIK
	Copyright (C) 2002, YAZAKI, MIK, genta, novice
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

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
#include "prop/CPropCommon.h"
#include <memory.h>
#include <stdlib.h>
#include "util/shell.h"
#include "util/file.h"
#include "util/string_ex2.h"
#include "util/module.h"

//! Popup Help�pID
//@@@ 2001.12.22 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//11700
	IDC_MACRODIRREF,	HIDC_MACRODIRREF,	//�}�N���f�B���N�g���Q��
	IDC_MACRO_REG,		HIDC_MACRO_REG,		//�}�N���ݒ�
	IDC_COMBO_MACROID,	HIDC_COMBO_MACROID,	//ID
	IDC_MACROPATH,		HIDC_MACROPATH,		//File
	IDC_MACRONAME,		HIDC_MACRONAME,		//�}�N����
	IDC_MACROLIST,		HIDC_MACROLIST,		//�}�N�����X�g
	IDC_MACRODIR,		HIDC_MACRODIR,		//�}�N���ꗗ
	IDC_CHECK_RELOADWHENEXECUTE,	HIDC_CHECK_RELOADWHENEXECUTE,	//�}�N�������s���邽�тɃt�@�C����ǂݍ��݂Ȃ���	// 2006.08.06 ryoji
	IDC_CHECK_MacroOnOpened,		HIDC_CHECK_MacroOnOpened,		//�I�[�v���㎩�����s�}�N��	// 2006.09.01 ryoji
	IDC_CHECK_MacroOnTypeChanged,	HIDC_CHECK_MacroOnTypeChanged,	//�^�C�v�ύX�㎩�����s�}�N��	// 2006.09.01 ryoji
	IDC_CHECK_MacroOnSave,			HIDC_CHECK_MacroOnSave,			//�ۑ��O�������s�}�N��	// 2006.09.01 ryoji
//	IDC_STATIC,			-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//11700
	IDC_MACRODIRREF,	11700,	//�Q��
	IDC_MACRO_REG,		11701,	//�ݒ�
	IDC_COMBO_MACROID,	11730,	//ID
	IDC_MACROPATH,		11731,	//�p�X
	IDC_MACRONAME,		11740,	//�}�N����
	IDC_MACROLIST,		11741,	//���X�g
	IDC_MACRODIR,		11750,	//�}�N���ꗗ
//	IDC_STATIC,			-1,
	0, 0
};
#endif
//@@@ 2001.12.22 End

/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_MACRO(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc(&CPropCommon::DispatchEvent_PROP_Macro, hwndDlg, uMsg, wParam, lParam );
}

/*! Macro�y�[�W�̃��b�Z�[�W����
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handlw
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CPropCommon::DispatchEvent_PROP_Macro( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		//	Oct. 5, 2002 genta �G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_MACRONAME ),  EM_LIMITTEXT, _countof( m_Common.m_sMacro.m_MacroTable[0].m_szName ) - 1, 0 );
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_MACROPATH ),  EM_LIMITTEXT, _countof( m_Common.m_sMacro.m_MacroTable[0].m_szFile ) - 1, 0 );
		// 2003.06.23 Moca
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_MACRODIR ),  EM_LIMITTEXT, _countof2( m_Common.m_sMacro.m_szMACROFOLDER ) - 1, 0 );

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
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_MACRO;
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
			break;	/* CBN_DROPDOWN */
		// From Here 2003.06.23 Moca �}�N���t�H���_�̍Ō��\���Ȃ���Εt����
		case EN_KILLFOCUS:
			switch( wID ){
			case IDC_MACRODIR:
				{
					TCHAR szDir[_MAX_PATH];
					::DlgItem_GetText( hwndDlg, IDC_MACRODIR, szDir, _MAX_PATH );
					if( 1 == AddLastChar( szDir, _MAX_PATH, _T('\\') ) ){
						::DlgItem_SetText( hwndDlg, IDC_MACRODIR, szDir );
					}
				}
				break;
			}
			break;
		// To Here 2003.06.23 Moca
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
void CPropCommon::SetData_PROP_Macro( HWND hwndDlg )
{
	int index;
	LVITEM sItem;

	//	�}�N���f�[�^
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_MACROLIST );
	
	for( index = 0; index < MAX_CUSTMACRO; ++index ){
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 1;
		sItem.pszText = m_pShareData->m_Common.m_sMacro.m_MacroTable[index].m_szName;
		ListView_SetItem( hListView, &sItem );

		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 2;
		sItem.pszText = m_pShareData->m_Common.m_sMacro.m_MacroTable[index].m_szFile;
		ListView_SetItem( hListView, &sItem );

		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 3;
		sItem.pszText = m_pShareData->m_Common.m_sMacro.m_MacroTable[index].m_bReloadWhenExecute ? _T("on") : _T("off");
		ListView_SetItem( hListView, &sItem );

		// �������s�}�N��	// 2006.09.01 ryoji
		TCHAR szText[8];
		szText[0] = _T('\0');
		if( index == m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened )
			::lstrcat(szText, _T("O"));
		if( index == m_pShareData->m_Common.m_sMacro.m_nMacroOnTypeChanged )
			::lstrcat(szText, _T("T"));
		if( index == m_pShareData->m_Common.m_sMacro.m_nMacroOnSave )
			::lstrcat(szText, _T("S"));
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 4;
		sItem.pszText = szText;
		ListView_SetItem( hListView, &sItem );
	}
	
	//	�}�N���f�B���N�g��
	::DlgItem_SetText( hwndDlg, IDC_MACRODIR, /*m_pShareData->*/m_Common.m_sMacro.m_szMACROFOLDER );

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
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_MACRO;

	int index;
	LVITEM sItem;

	// �������s�}�N���ϐ�������	// 2006.09.01 ryoji
	m_Common.m_sMacro.m_nMacroOnOpened = -1;
	m_Common.m_sMacro.m_nMacroOnTypeChanged = -1;
	m_Common.m_sMacro.m_nMacroOnSave = -1;

	//	�}�N���f�[�^
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_MACROLIST );

	for( index = 0; index < MAX_CUSTMACRO; ++index ){
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 1;
		sItem.cchTextMax = MACRONAME_MAX - 1;
//@@@ 2002.01.03 YAZAKI ���ʐݒ�w�}�N���x���^�u��؂�ւ��邾���Őݒ肪�ۑ�����Ȃ��悤�ɁB
		sItem.pszText = /*m_pShareData->*/m_Common.m_sMacro.m_MacroTable[index].m_szName;
		ListView_GetItem( hListView, &sItem );

		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 2;
		sItem.cchTextMax = _MAX_PATH;
//@@@ 2002.01.03 YAZAKI ���ʐݒ�w�}�N���x���^�u��؂�ւ��邾���Őݒ肪�ۑ�����Ȃ��悤�ɁB
		sItem.pszText = /*m_pShareData->*/m_Common.m_sMacro.m_MacroTable[index].m_szFile;
		ListView_GetItem( hListView, &sItem );

		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 3;
		TCHAR buf[MAX_PATH];
		sItem.pszText = buf;
		sItem.cchTextMax = MAX_PATH;
		ListView_GetItem( hListView, &sItem );
		if ( _tcscmp(buf, _T("on")) == 0){
			m_Common.m_sMacro.m_MacroTable[index].m_bReloadWhenExecute = true;
		}
		else {
			m_Common.m_sMacro.m_MacroTable[index].m_bReloadWhenExecute = false;
		}

		// �������s�}�N��	// 2006.09.01 ryoji
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = index;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 4;
		TCHAR szText[8];
		sItem.pszText = szText;
		sItem.cchTextMax = _countof(szText);
		ListView_GetItem( hListView, &sItem );
		int i;
		int nLen;
		nLen = ::lstrlen(szText);
		for( i = 0; i < nLen; i++)
		{
			if( szText[i] == _T('O') )
				m_Common.m_sMacro.m_nMacroOnOpened = index;
			if( szText[i] == _T('T') )
				m_Common.m_sMacro.m_nMacroOnTypeChanged = index;
			if( szText[i] == _T('S') )
				m_Common.m_sMacro.m_nMacroOnSave = index;
		}
	}

	//	�}�N���f�B���N�g��
//@@@ 2002.01.03 YAZAKI ���ʐݒ�w�}�N���x���^�u��؂�ւ��邾���Őݒ肪�ۑ�����Ȃ��悤�ɁB
	::DlgItem_GetText( hwndDlg, IDC_MACRODIR, m_Common.m_sMacro.m_szMACROFOLDER, _MAX_PATH );
	// 2003.06.23 Moca �}�N���t�H���_�̍Ō��\���Ȃ���Εt����
	AddLastChar( m_Common.m_sMacro.m_szMACROFOLDER, _MAX_PATH, _T('\\') );
	
	return TRUE;
}

void CPropCommon::InitDialog_PROP_Macro( HWND hwndDlg )
{
	struct ColumnData {
		TCHAR *title;
		int width;
	} ColumnList[] = {
		{ _T("�ԍ�"), 40 },
		{ _T("�}�N����"), 150 },
		{ _T("�t�@�C����"), 150 },
		{ _T("���s���ɓǂݍ���"), 40 },
		{ _T("�������s"), 40 },
	};

	//	ListView�̏�����
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_MACROLIST );
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

	//	�������̊m��
	//	�K�v�Ȑ�������Ɋm�ۂ���D
	ListView_SetItemCount( hListView, MAX_CUSTMACRO );

	//	Index�����̓o�^
	for( pos = 0; pos < MAX_CUSTMACRO ; ++pos ){
		LVITEM sItem;
		TCHAR buf[4];
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.mask = LVIF_TEXT | LVIF_PARAM;
		sItem.iItem = pos;
		sItem.iSubItem = 0;
		_itot( pos, buf, 10 );
		sItem.pszText = buf;
		sItem.lParam = pos;
		ListView_InsertItem( hListView, &sItem );
	}
	
	// �o�^��w�� ComboBox�̏�����
	HWND hNumCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_MACROID );
	for( pos = 0; pos < MAX_CUSTMACRO ; ++pos ){
		wchar_t buf[10];
		auto_sprintf( buf, L"%d", pos );
		int result = ::SendMessage( hNumCombo, CB_ADDSTRING, (WPARAM)0, (LPARAM)buf );
		if( result == CB_ERR ){
			::MessageBox( hwndDlg, _T("PropComMacro::InitDlg::AddMacroId"), _T("�o�O�񍐂��肢"), MB_OK );
			return;	//	�悭�킩��񂯂ǎ��s����
		}
		else if( result == CB_ERRSPACE ){
			::MessageBox( hwndDlg, _T("PropComMacro::InitDlg::AddMacroId/InsufficientSpace"),
				_T("�o�O�񍐂��肢"), MB_OK );
			return;	//	�悭�킩��񂯂ǎ��s����
		}
	}
	::SendMessageAny( hNumCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );
}

void CPropCommon::SetMacro2List_Macro( HWND hwndDlg )
{
	int index;
	LVITEM sItem;
	
	HWND hListView = ::GetDlgItem( hwndDlg, IDC_MACROLIST );
	HWND hNum = ::GetDlgItem( hwndDlg, IDC_COMBO_MACROID );

	//	�ݒ��擾
	index = ::SendMessageAny( hNum, CB_GETCURSEL, 0, 0 );
	if( index == CB_ERR ){
		::MessageBox( hwndDlg, _T("PropComMacro::SetMacro2List::GetCurSel"),
			_T("�o�O�񍐂��肢"), MB_OK );
		return;	//	�悭�킩��񂯂ǎ��s����
	}

	// �}�N����
	memset_raw( &sItem, 0, sizeof( sItem ));
	sItem.iItem = index;
	sItem.mask = LVIF_TEXT;
	sItem.iSubItem = 1;
	
	TCHAR buf[256];
	::DlgItem_GetText( hwndDlg, IDC_MACRONAME, buf, MACRONAME_MAX );
	sItem.pszText = buf;
	ListView_SetItem( hListView, &sItem );

	// �t�@�C����
	memset_raw( &sItem, 0, sizeof( sItem ));
	sItem.iItem = index;
	sItem.mask = LVIF_TEXT;
	sItem.iSubItem = 2;

	::DlgItem_GetText( hwndDlg, IDC_MACROPATH, buf, _MAX_PATH );
	sItem.pszText = buf;
	ListView_SetItem( hListView, &sItem );

	// �`�F�b�N
	memset_raw( &sItem, 0, sizeof( sItem ));
	sItem.iItem = index;
	sItem.mask = LVIF_TEXT;
	sItem.iSubItem = 3;
	sItem.pszText = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_RELOADWHENEXECUTE ) ? _T("on") : _T("off");
	ListView_SetItem( hListView, &sItem );

	// �������s�}�N��	// 2006.09.01 ryoji
	int nMacroOnOpened = -1;
	int nMacroOnTypeChanged = -1;
	int nMacroOnSave = -1;
	TCHAR szText[8];
	int iItem;
	for( iItem = 0; iItem < MAX_CUSTMACRO; iItem++){
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = iItem;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 4;
		sItem.pszText = szText;
		sItem.cchTextMax = _countof(szText);
		ListView_GetItem( hListView, &sItem );
		int i;
		int nLen;
		nLen = ::lstrlen(szText);
		for( i = 0; i < nLen; i++)
		{
			if( szText[i] == _T('O') )
				nMacroOnOpened = iItem;
			if( szText[i] == _T('T') )
				nMacroOnTypeChanged = iItem;
			if( szText[i] == _T('S') )
				nMacroOnSave = iItem;
		}
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_MacroOnOpened ) )
		nMacroOnOpened = index;
	else if( nMacroOnOpened == index )
		nMacroOnOpened = -1;
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_MacroOnTypeChanged ) )
		nMacroOnTypeChanged = index;
	else if( nMacroOnTypeChanged == index )
		nMacroOnTypeChanged = -1;
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_MacroOnSave ) )
		nMacroOnSave = index;
	else if( nMacroOnSave == index )
		nMacroOnSave = -1;
	for( iItem = 0; iItem < MAX_CUSTMACRO; iItem++){
		szText[0] = _T('\0');
		if( iItem == nMacroOnOpened )
			::lstrcat(szText, _T("O"));
		if( iItem == nMacroOnTypeChanged )
			::lstrcat(szText, _T("T"));
		if( iItem == nMacroOnSave )
			::lstrcat(szText, _T("S"));
		memset_raw( &sItem, 0, sizeof( sItem ));
		sItem.iItem = iItem;
		sItem.mask = LVIF_TEXT;
		sItem.iSubItem = 4;
		sItem.pszText = szText;
		ListView_SetItem( hListView, &sItem );
	}
}

/*!
	Macro�i�[�p�f�B���N�g����I������
	
	@param hwndDlg [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void CPropCommon::SelectBaseDir_Macro( HWND hwndDlg )
{

// 2002/04/14 novice
//	SelectDir()�ɕ������ꂽ�������폜
	TCHAR szDir[MAX_PATH + 1]; // �ǉ�����\\�p��1������

	/* �����t�H���_ */
	::DlgItem_GetText( hwndDlg, IDC_MACRODIR, szDir, _MAX_PATH );

	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	if( _IS_REL_PATH( szDir ) ){
		TCHAR folder[_MAX_PATH];
		_tcscpy( folder, szDir );
		GetInidirOrExedir( szDir, folder );
	}

	if( SelectDir( hwndDlg, _T("Macro�f�B���N�g���̑I��"), szDir, szDir ) ){
		//	������\\�}�[�N��ǉ�����D
		AddLastChar( szDir, _MAX_PATH, _T('\\') );
		::DlgItem_SetText( hwndDlg, IDC_MACRODIR, szDir );
	}
}


/*!
	�}�N���t�@�C���w��p�R���{�{�b�N�X�̃h���b�v�_�E�����X�g���J�����Ƃ��ɁC
	�w��f�B���N�g���̃t�@�C���ꗗ������𐶐�����D

	@param hwndDlg [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void CPropCommon::OnFileDropdown_Macro( HWND hwndDlg )
{
	HANDLE hFind;
	HWND hCombo = ::GetDlgItem( hwndDlg, IDC_MACROPATH );

	TCHAR path[_MAX_PATH * 2 ];
	::DlgItem_GetText( hwndDlg, IDC_MACRODIR, path, _MAX_PATH );

	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	if( _IS_REL_PATH( path ) ){
		TCHAR folder[_MAX_PATH];
		_tcscpy( folder, path );
		GetInidirOrExedir( path, folder );
	}
	_tcscat( path, _T("*.*") );	//	2002/05/01 YAZAKI �ǂ�ȃt�@�C�����ǂ�Ɨ����B

	//	���̏�����
	::SendMessageAny( hCombo, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0 );

	//	�t�@�C���̌���
	WIN32_FIND_DATA wf;
	hFind = FindFirstFile(path, &wf);

	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}
	
	do {
		//	�R���{�{�b�N�X�ɐݒ�
		//	�ł�.��..�͊��فB
		//if (_tcscmp( wf.cFileName, _T(".") ) != 0 && _tcscmp( wf.cFileName, _T("..") ) != 0){
		if( (wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ){	// 2009.02.12 ryoji �t�H���_�����O
			int result = ::SendMessage( hCombo, CB_ADDSTRING, (WPARAM)0, (LPARAM)wf.cFileName );
			if( result == CB_ERR || result == CB_ERRSPACE )
				break;
		}
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
	::SendMessageAny( hNum, CB_SETCURSEL, nLastPos_Macro, 0 );
	
	TCHAR buf[MAX_PATH + MACRONAME_MAX];	// MAX_PATH��MACRONAME_MAX�̗������傫���l
	LVITEM sItem;

	memset_raw( &sItem, 0, sizeof( sItem ));
	sItem.iItem = current;
	sItem.mask = LVIF_TEXT;
	sItem.iSubItem = 1;
	sItem.pszText = buf;
	sItem.cchTextMax = MACRONAME_MAX;

	ListView_GetItem( hListView, &sItem );
	::DlgItem_SetText( hwndDlg, IDC_MACRONAME, buf );

	memset_raw( &sItem, 0, sizeof( sItem ));
	sItem.iItem = current;
	sItem.mask = LVIF_TEXT;
	sItem.iSubItem = 2;
	sItem.pszText = buf;
	sItem.cchTextMax = MAX_PATH;

	ListView_GetItem( hListView, &sItem );
	::DlgItem_SetText( hwndDlg, IDC_MACROPATH, buf );

	memset_raw( &sItem, 0, sizeof( sItem ));
	sItem.iItem = current;
	sItem.mask = LVIF_TEXT;
	sItem.iSubItem = 3;
	sItem.pszText = buf;
	sItem.cchTextMax = MAX_PATH;
	ListView_GetItem( hListView, &sItem );
	if ( _tcscmp(buf, _T("on")) == 0){
		::CheckDlgButton( hwndDlg, IDC_CHECK_RELOADWHENEXECUTE, true );
	}
	else {
		::CheckDlgButton( hwndDlg, IDC_CHECK_RELOADWHENEXECUTE, false );
	}

	// �������s�}�N��	// 2006.09.01 ryoji
	memset_raw( &sItem, 0, sizeof( sItem ));
	sItem.iItem = current;
	sItem.mask = LVIF_TEXT;
	sItem.iSubItem = 4;
	TCHAR szText[8];
	sItem.pszText = szText;
	sItem.cchTextMax = _countof(szText);
	ListView_GetItem( hListView, &sItem );
	int i;
	int nLen;
	nLen = ::lstrlen(szText);
	::CheckDlgButton( hwndDlg, IDC_CHECK_MacroOnOpened, false );
	::CheckDlgButton( hwndDlg, IDC_CHECK_MacroOnTypeChanged, false );
	::CheckDlgButton( hwndDlg, IDC_CHECK_MacroOnSave, false );
	for( i = 0; i < nLen; i++)
	{
		if( szText[i] == _T('O') )
			::CheckDlgButton( hwndDlg, IDC_CHECK_MacroOnOpened, true );
		if( szText[i] == _T('T') )
			::CheckDlgButton( hwndDlg, IDC_CHECK_MacroOnTypeChanged, true );
		if( szText[i] == _T('S') )
			::CheckDlgButton( hwndDlg, IDC_CHECK_MacroOnSave, true );
	}
}



