/*!	@file
	@brief �v���O�C���ݒ�_�C�A���O�{�b�N�X

	@author Uchi
	@date 2010/3/22
*/
/*
	Copyright (C) 2010, Uchi

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
#include "dlg/CDlgPluginOption.h"
#include "util/shell.h"
#include "util/window.h"
#include "debug/Debug.h"

// BOOL�ϐ��̕\��
#define	BOOL_DISP_TRUE	_T("<True>")
#define	BOOL_DISP_FALSE	_T("<False>")

// �ҏW�̈��\���A��\���ɂ���
static inline void CtrlShow(HWND hwndDlg, int id, BOOL bShow)
{
	HWND hWnd = ::GetDlgItem( hwndDlg, id );
	::ShowWindow( hWnd, bShow? SW_SHOW: SW_HIDE );
	::EnableWindow( hWnd, bShow );
}

#include "sakura.hh"
const DWORD p_helpids[] = {
	IDC_LIST_PLUGIN_OPTIONS,		HIDC_LIST_PLUGIN_OPTIONS,		// �I�v�V�������X�g
	IDC_EDIT_PLUGIN_OPTION,			HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDC_EDIT_PLUGIN_OPTION_NUM,		HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDC_SPIN_PLUGIN_OPTION,			HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDC_CHECK_PLUGIN_OPTION,		HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDC_COMBO_PLUGIN_OPTION,		HIDC_EDIT_PLUGIN_OPTION,		// �I�v�V�����ҏW
	IDOK,							HIDC_FAVORITE_IDOK,				//OK
	IDCANCEL,						HIDC_FAVORITE_IDCANCEL,			//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_BUTTON_FAVORITE_HELP,		//�w���v
//	IDC_STATIC,						-1,
	0, 0
};

CDlgPluginOption::CDlgPluginOption()
{

}

CDlgPluginOption::~CDlgPluginOption()
{

}

/* ���[�_���_�C�A���O�̕\�� */
int CDlgPluginOption::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	int 		ID
)
{
	// �v���O�C���ԍ��i�G�f�B�^���ӂ�ԍ��j
	m_ID = ID;
	m_cPlugin = CPluginManager::Instance()->GetPlugin( m_ID );

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_PLUGIN_OPTION, NULL );
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgPluginOption::SetData( void )
{
	HWND	hwndList;
	int		i;
	LV_ITEM	lvi;
	int		nNewFocus = -1;
	TCHAR	buf[MAX_LENGTH_VALUE+1];

	// �^�C�g��
	auto_sprintf( buf, _T("%ls �v���O�C���̐ݒ�"), m_cPlugin->m_sName.c_str());
	::SetWindowText( GetHwnd(), buf );

	// ���X�g
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PLUGIN_OPTIONS );
	ListView_DeleteAllItems( hwndList );	// ���X�g����ɂ���
	m_Line = -1;							// �s��I��

	CDataProfile* cProfile= new CDataProfile();
	cProfile->SetReadingMode();
	cProfile->ReadProfile( m_cPlugin->GetOptionPath().c_str() );

	CPluginOption* cOpt;
	CPluginOption::ArrayIter it;
	for( i=0, it = m_cPlugin->m_options.begin(); it != m_cPlugin->m_options.end(); i++, it++ ){
		cOpt = *it;

		auto_snprintf_s( buf, _countof(buf), _T("%ls"), cOpt->GetLabel().c_str());
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = buf;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = cOpt->GetIndex();
		ListView_InsertItem( hwndList, &lvi );

		wstring sSection;
		wstring sKey;
		wstring sValue;
		wstring sType;

		cOpt->GetKey(&sSection, &sKey);
		if( sSection.empty() || sKey.empty() ) {
			sValue = L"";
		}
		else {
			cProfile->IOProfileData( sSection.c_str(), sKey.c_str(), sValue );
		}

		if (cOpt->GetType() == OPTION_TYPE_BOOL) {
			_tcscpy( buf, sValue == wstring( L"0") || sValue == wstring( L"") ? BOOL_DISP_FALSE : BOOL_DISP_TRUE );
		}
		else if (cOpt->GetType() == OPTION_TYPE_INT) {
			// ���l�֐��K��
			auto_sprintf( buf, _T("%d"), _wtoi(sValue.c_str()));
		}
		else if (cOpt->GetType() == OPTION_TYPE_SEL) {
			// �l����\����
			wstring	sView;
			wstring	sTrg;
			std::vector<wstring>	selects;
			selects = cOpt->GetSelects();

			_tcscpy( buf, _T("") );
			for (std::vector<wstring>::iterator it = selects.begin(); it != selects.end(); it++) {
				SepSelect(*it, &sView, &sTrg);
				if (sValue == sTrg) {
					auto_snprintf_s( buf, _countof(buf), _T("%ls"), sView.c_str());
					break;
				}
			}
		}
		else {
			auto_snprintf_s( buf, _countof(buf), _T("%ls"), sValue.c_str());
		}
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = buf;
		ListView_SetItem( hwndList, &lvi );
		ListView_SetItemState( hwndList, i, 0, LVIS_SELECTED | LVIS_FOCUSED );
	}

	if (i ==0) {
		// �I�v�V����������
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LIST_PLUGIN_OPTIONS    ), FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDOK                       ), FALSE );
	
		::DlgItem_SetText( GetHwnd(), IDC_STATIC_MSG, _T("�w��ł���I�v�V����������܂���") );
	}

	return;
}

/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[ */
int CDlgPluginOption::GetData( void )
{
	// .ini �t�@�C���ւ̏�������
	HWND	hwndList;
	int		i;
	LV_ITEM	lvi;

	// ���X�g
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PLUGIN_OPTIONS );

	CDataProfile* cProfile= new CDataProfile();
	cProfile->SetReadingMode();
	cProfile->ReadProfile( m_cPlugin->GetOptionPath().c_str() );
	cProfile->SetWritingMode();

	CPluginOption* cOpt;
	TCHAR	buf[MAX_LENGTH_VALUE+1];
	CPluginOption::ArrayIter it;
	for( i=0, it = m_cPlugin->m_options.begin(); it != m_cPlugin->m_options.end(); i++, it++ ){
		cOpt = *it;

		memset_raw( &lvi, 0, sizeof( lvi ));
		lvi.mask       = LVIF_TEXT;
		lvi.iItem      = i;
		lvi.iSubItem   = 1;
		lvi.pszText    = buf;
		lvi.cchTextMax = MAX_LENGTH_VALUE+1;
		ListView_GetItem( hwndList, &lvi );

		if (cOpt->GetType() == OPTION_TYPE_BOOL) {
			if (_tcscmp(buf,  BOOL_DISP_FALSE) == 0) {
				_tcscpy (buf, _T("0"));
			}
			else {
				_tcscpy (buf, _T("1"));
			}
		}
		else if (cOpt->GetType() == OPTION_TYPE_SEL) {
			// �\������l��
			wstring	sView;
			wstring	sTrg;
			std::vector<wstring>	selects;
			selects = cOpt->GetSelects();
			wstring sWbuf = to_wchar(buf);

			for (std::vector<wstring>::iterator it = selects.begin(); it != selects.end(); it++) {
				SepSelect(*it, &sView, &sTrg);
				if (sView == sWbuf) {
					auto_sprintf( buf, _T("%ls"), sTrg.c_str());
					break;
				}
			}
		}

		wstring sSection;
		wstring sKey;
		wstring sValue;

		cOpt->GetKey(&sSection, &sKey);
		if( sSection.empty() || sKey.empty() ) {
			continue;
		}

		sValue = to_wchar(buf);

		cProfile->IOProfileData( sSection.c_str(), sKey.c_str(), sValue );
	}

	cProfile->WriteProfile( m_cPlugin->GetOptionPath().c_str() ,(m_cPlugin->m_sName + L" �v���O�C���ݒ�t�@�C��").c_str());

	return TRUE;
}

BOOL CDlgPluginOption::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	HWND		hwndList;
	LV_COLUMN	col;
	RECT		rc;
	long		lngStyle;

	_SetHwnd( hwndDlg );

	hwndList = GetDlgItem( hwndDlg, IDC_LIST_PLUGIN_OPTIONS );
	::GetWindowRect( hwndList, &rc );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 40 / 100;
	col.pszText  = _T("����");
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 55 / 100;
	col.pszText  = _T("�l");
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col );

	/* �s�I�� */
	lngStyle = ::SendMessageAny( hwndList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
	lngStyle |= LVS_EX_FULLROWSELECT;
	::SendMessageAny( hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lngStyle );

	// �ҏW�̈�̔�A�N�e�B�u��
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_PLUGIN_OPTION ), FALSE );
	CtrlShow( hwndDlg, IDC_EDIT_PLUGIN_OPTION_NUM, FALSE );
	CtrlShow( hwndDlg, IDC_SPIN_PLUGIN_OPTION,     FALSE );
	CtrlShow( hwndDlg, IDC_CHECK_PLUGIN_OPTION,    FALSE );
	CtrlShow( hwndDlg, IDC_COMBO_PLUGIN_OPTION,    FALSE );

	// ��������
	::SendMessageAny( GetDlgItem( hwndDlg, IDC_EDIT_PLUGIN_OPTION     ), EM_LIMITTEXT, MAX_LENGTH_VALUE, 0 );
	::SendMessageAny( GetDlgItem( hwndDlg, IDC_EDIT_PLUGIN_OPTION_NUM ), EM_LIMITTEXT, 11, 0 );

	
	/* ���N���X�����o */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgPluginOption::OnNotify( WPARAM wParam, LPARAM lParam )
{
	NMHDR*		pNMHDR;
	int			idCtrl;

	idCtrl = (int)wParam;
	switch( idCtrl ){
	case IDC_LIST_PLUGIN_OPTIONS:
		pNMHDR = (NMHDR*)lParam;
		switch( pNMHDR->code ){
		case LVN_ITEMCHANGED:
			ChangeListPosition( );
			break;
		}
		return TRUE;;

	case IDC_SPIN_PLUGIN_OPTION:
		int			nVal;
		NM_UPDOWN*	pMNUD;
		
		pMNUD  = (NM_UPDOWN*)lParam;

		nVal = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM, NULL, TRUE );
		if( pMNUD->iDelta < 0 ){
			if (nVal < INT_MAX)		++nVal;
		}else
		if( pMNUD->iDelta > 0 ){
			// INT_MIN�� SetDlgItemInt �ň����Ȃ�
			if (nVal > -INT_MAX)	--nVal;
		}
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM, nVal, TRUE );
		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnNotify( wParam, lParam );
}



BOOL CDlgPluginOption::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* �w���v */
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID( F_OPTION_PLUGIN ) );	// ??? 
		return TRUE;

	case IDOK:
		// �ҏW���̃f�[�^�̖߂�
		SetFromEdit( m_Line );
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( GetHwnd(), (BOOL)GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgPluginOption::OnActivate( WPARAM wParam, LPARAM lParam )
{
	switch( LOWORD( wParam ) )
	{
	case WA_INACTIVE:
		SetFromEdit( m_Line );
		break;

	case WA_ACTIVE:
	case WA_CLICKACTIVE:
	default:
		break;
	}

	/* ���N���X�����o */
	return CDialog::OnActivate( wParam, lParam );
}


LPVOID CDlgPluginOption::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}


void CDlgPluginOption::ChangeListPosition( void )
{
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PLUGIN_OPTIONS );

	//	���݂�Focus�擾
	int current = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED);

	if( current == -1 || current == m_Line ){
		return;
	}

	TCHAR	buf[MAX_LENGTH_VALUE+1];
	LVITEM	lvi;

// �߂�
	if (m_Line >= 0) {
		SetFromEdit( m_Line );
	}

	m_Line = current;

// �ҏW�̈�ɏ�������
	SetToEdit(current);

	memset_raw( &lvi, 0, sizeof( lvi ));
	lvi.mask       = LVIF_TEXT;
	lvi.iItem      = current;
	lvi.iSubItem   = 1;
	lvi.pszText    = buf;
	lvi.cchTextMax = MAX_LENGTH_VALUE+1;

	ListView_GetItem( hwndList, &lvi );
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION, buf );
}

// �ҏW�̈�ɏ�������
void CDlgPluginOption::SetToEdit( int iLine )
{
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PLUGIN_OPTIONS );

	TCHAR	buf[MAX_LENGTH_VALUE+1];
	LVITEM	lvi;
	wstring	sType;

	if (iLine >= 0) {
		::DlgItem_GetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE+1);
		memset_raw( &lvi, 0, sizeof( lvi ));
		lvi.mask       = LVIF_TEXT;
		lvi.iItem      = iLine;
		lvi.iSubItem   = 1;
		lvi.pszText    = buf;
		lvi.cchTextMax = MAX_LENGTH_VALUE+1;
		ListView_GetItem( hwndList, &lvi );

		sType = m_cPlugin->m_options[iLine]->GetType();
		transform (sType.begin (), sType.end (), sType.begin (), tolower);
		if (sType == OPTION_TYPE_BOOL) {
			::CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PLUGIN_OPTION, _tcscmp(buf,  BOOL_DISP_FALSE) != 0 );
			::DlgItem_SetText( GetHwnd(), IDC_CHECK_PLUGIN_OPTION, m_cPlugin->m_options[iLine]->GetLabel().c_str() );

			// �ҏW�̈�̐؂�ւ�
			SelectEdit(IDC_CHECK_PLUGIN_OPTION);
		}
		else if (sType == OPTION_TYPE_INT) {
			::DlgItem_SetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM, buf );

			// �ҏW�̈�̐؂�ւ�
			SelectEdit(IDC_EDIT_PLUGIN_OPTION_NUM);
		}
		else if (sType == OPTION_TYPE_SEL) {
			// CONBO �ݒ�
			std::vector<wstring>	selects;
			selects = m_cPlugin->m_options[iLine]->GetSelects();

			HWND	hwndCombo;
			hwndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_PLUGIN_OPTION );
			::SendMessageAny( hwndCombo, CB_RESETCONTENT, 0, 0 );

			int		nSelIdx;
			int		i;
			int		nItemIdx;
			wstring	sView;
			wstring	sValue;
			wstring	sWbuf = to_wchar(buf);
			nSelIdx = -1;		// �I��
			i = 0;
			for (std::vector<wstring>::iterator it = selects.begin(); it != selects.end(); it++) {
				SepSelect(*it, &sView, &sValue);
				nItemIdx = ::SendMessageW( hwndCombo, CB_ADDSTRING, 0,(LPARAM)sView.c_str() );
				if (sView == sWbuf) {
					nSelIdx = i;
				}
				::SendMessageAny( hwndCombo, CB_SETITEMDATA, nItemIdx, (LPARAM)i++ );
			}
			::SendMessageAny( hwndCombo, CB_SETCURSEL, nSelIdx, 0 );

			// �ҏW�̈�̐؂�ւ�
			SelectEdit(IDC_COMBO_PLUGIN_OPTION);
		}
		else {
			::DlgItem_SetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION, buf );

			// �ҏW�̈�̐؂�ւ�
			SelectEdit(IDC_EDIT_PLUGIN_OPTION);
		}
	}
}

// �ҏW�̈�̐؂�ւ�
void CDlgPluginOption::SelectEdit( int IDCenable )
{
	CtrlShow( GetHwnd(), IDC_EDIT_PLUGIN_OPTION,     (IDCenable == IDC_EDIT_PLUGIN_OPTION) );
	CtrlShow( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM, (IDCenable == IDC_EDIT_PLUGIN_OPTION_NUM) );
	CtrlShow( GetHwnd(), IDC_SPIN_PLUGIN_OPTION,     (IDCenable == IDC_EDIT_PLUGIN_OPTION_NUM) );
	CtrlShow( GetHwnd(), IDC_CHECK_PLUGIN_OPTION,    (IDCenable == IDC_CHECK_PLUGIN_OPTION)  );
	CtrlShow( GetHwnd(), IDC_COMBO_PLUGIN_OPTION,    (IDCenable == IDC_COMBO_PLUGIN_OPTION)  );
}

// �ҏW�̈悩��߂�
void CDlgPluginOption::SetFromEdit( int iLine )
{
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_PLUGIN_OPTIONS );

	TCHAR	buf[MAX_LENGTH_VALUE+1];
	int		nVal;
	LVITEM	lvi;
	wstring	sType;

	if (iLine >= 0) {
		sType = m_cPlugin->m_options[iLine]->GetType();
		transform (sType.begin (), sType.end (), sType.begin (), tolower);
		if (sType == OPTION_TYPE_BOOL) {
			if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PLUGIN_OPTION ) ) {
				_tcscpy( buf, BOOL_DISP_TRUE );
			}
			else {
				_tcscpy( buf, BOOL_DISP_FALSE );
			}
			lvi.mask     = LVIF_TEXT;
			lvi.iItem    = iLine;
			lvi.iSubItem = 1;
			lvi.pszText  = buf;
			ListView_SetItem( hwndList, &lvi );
		}
		else if (sType == OPTION_TYPE_INT) {
			nVal = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_PLUGIN_OPTION_NUM, NULL, TRUE );
			auto_sprintf( buf, _T("%d"), nVal);
		}
		else if (sType == OPTION_TYPE_SEL) {
			::DlgItem_GetText( GetHwnd(), IDC_COMBO_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE+1);
		}
		else {
			::DlgItem_GetText( GetHwnd(), IDC_EDIT_PLUGIN_OPTION, buf, MAX_LENGTH_VALUE+1);
		}
		memset_raw( &lvi, 0, sizeof( lvi ));
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = iLine;
		lvi.iSubItem = 1;
		lvi.pszText  = buf;
		ListView_SetItem( hwndList, &lvi );
	}
}

// �I��p�����񕪉�
void CDlgPluginOption::SepSelect( wstring sTrg, wstring* spView, wstring* spValue )
{
	int		ix;
	ix = sTrg.find(L':');
	if (ix == std::wstring::npos) {
		*spView = *spValue = sTrg;
	}
	else {
#ifdef _DEBUG
		*spView  = sTrg;
#else
		*spView  = sTrg.substr( 0, ix );
#endif
		*spValue = sTrg.substr( ix + 1 );
	}
}
