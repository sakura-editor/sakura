/*!	@file
	@brief ����ݒ�_�C�A���O

	@author Norio Nakatani
	
	@date 2006.08.14 Moca �p�������R���{�{�b�N�X��p�~���A�{�^����L�����D
		�p�����ꗗ�̏d���폜�D
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor, Stonee
	Copyright (C) 2002, MIK, aroka, YAZAKI
	Copyright (C) 2003, �����
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgPrintSetting.h"
#include "dlg/CDlgInput1.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "util/shell.h"
#include "sakura_rc.h"	// 2002/2/10 aroka
#include "sakura.hh"

// ����ݒ� CDlgPrintSetting.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12500
	IDC_BUTTON_EDITSETTINGNAME,		HIDC_PS_BUTTON_EDITSETTINGNAME,	//�ݒ薼�ύX
	IDOK,							HIDOK_PS,					//OK
	IDCANCEL,						HIDCANCEL_PS,				//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_PS_BUTTON_HELP,		//�w���v
	IDC_CHECK_WORDWRAP,				HIDC_PS_CHECK_WORDWRAP,		//���[�h���b�v
	IDC_CHECK_LINENUMBER,			HIDC_PS_CHECK_LINENUMBER,	//�s�ԍ�
	IDC_COMBO_FONT_HAN,				HIDC_PS_COMBO_FONT_HAN,		//���p�t�H���g
	IDC_COMBO_FONT_ZEN,				HIDC_PS_COMBO_FONT_ZEN,		//�S�p�t�H���g
	IDC_COMBO_SETTINGNAME,			HIDC_PS_COMBO_SETTINGNAME,	//�y�[�W�ݒ�
	IDC_COMBO_PAPER,				HIDC_PS_COMBO_PAPER,		//�p���T�C�Y
	IDC_EDIT_FONTWIDTH,				HIDC_PS_EDIT_FONTWIDTH,		//�t�H���g��
	IDC_EDIT_LINESPACE,				HIDC_PS_EDIT_LINESPACE,		//�s����
	IDC_EDIT_DANSUU,				HIDC_PS_EDIT_DANSUU,		//�i��
	IDC_EDIT_DANSPACE,				HIDC_PS_EDIT_DANSPACE,		//�i�̌���
	IDC_EDIT_MARGINTY,				HIDC_PS_EDIT_MARGINTY,		//�]����
	IDC_EDIT_MARGINBY,				HIDC_PS_EDIT_MARGINBY,		//�]����
	IDC_EDIT_MARGINLX,				HIDC_PS_EDIT_MARGINLX,		//�]����
	IDC_EDIT_MARGINRX,				HIDC_PS_EDIT_MARGINRX,		//�]���E
	IDC_SPIN_FONTWIDTH,				HIDC_PS_EDIT_FONTWIDTH,		//12570,
	IDC_SPIN_LINESPACE,				HIDC_PS_EDIT_LINESPACE,		//12571,
	IDC_SPIN_DANSUU,				HIDC_PS_EDIT_DANSUU,		//12572,
	IDC_SPIN_DANSPACE,				HIDC_PS_EDIT_DANSPACE,		//12573,
	IDC_SPIN_MARGINTY,				HIDC_PS_EDIT_MARGINTY,		//12574,
	IDC_SPIN_MARGINBY,				HIDC_PS_EDIT_MARGINBY,		//12575,
	IDC_SPIN_MARGINLX,				HIDC_PS_EDIT_MARGINLX,		//12576,
	IDC_SPIN_MARGINRX,				HIDC_PS_EDIT_MARGINRX,		//12577,
	IDC_CHECK_PS_KINSOKUHEAD,		HIDC_PS_CHECK_KINSOKUHEAD,	//�s���֑�	//@@@ 2002.04.09 MIK
	IDC_CHECK_PS_KINSOKUTAIL,		HIDC_PS_CHECK_KINSOKUTAIL,	//�s���֑�	//@@@ 2002.04.09 MIK
	IDC_CHECK_PS_KINSOKURET,		HIDC_PS_CHECK_KINSOKURET,	//���s�������Ԃ牺����	//@@@ 2002.04.14 MIK
	IDC_CHECK_PS_KINSOKUKUTO,		HIDC_PS_CHECK_KINSOKUKUTO,	//��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	IDC_EDIT_HEAD1,					HIDC_PS_EDIT_HEAD1,			//�w�b�_�[(����)		// 2006.10.11 ryoji
	IDC_EDIT_HEAD2,					HIDC_PS_EDIT_HEAD2,			//�w�b�_�[(������)	// 2006.10.11 ryoji
	IDC_EDIT_HEAD3,					HIDC_PS_EDIT_HEAD3,			//�w�b�_�[(�E��)		// 2006.10.11 ryoji
	IDC_EDIT_FOOT1,					HIDC_PS_EDIT_FOOT1,			//�t�b�^�[(����)		// 2006.10.11 ryoji
	IDC_EDIT_FOOT2,					HIDC_PS_EDIT_FOOT2,			//�t�b�^�[(������)	// 2006.10.11 ryoji
	IDC_EDIT_FOOT3,					HIDC_PS_EDIT_FOOT3,			//�t�b�^�[(�E��)		// 2006.10.11 ryoji
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

#define IDT_PRINTSETTING 1467

int CALLBACK SetData_EnumFontFamProc(
	ENUMLOGFONT*	pelf,	// pointer to logical-font data
	NEWTEXTMETRIC*	pntm,	// pointer to physical-font data
	int				nFontType,	// type of font
	LPARAM			lParam 	// address of application-defined data
)
{
	CDlgPrintSetting*	pCDlgPrintSetting;
	HWND				hwndComboFontHan;
	HWND				hwndComboFontZen;
	pCDlgPrintSetting = (CDlgPrintSetting*)lParam;
	hwndComboFontHan = ::GetDlgItem( pCDlgPrintSetting->GetHwnd(), IDC_COMBO_FONT_HAN );
	hwndComboFontZen = ::GetDlgItem( pCDlgPrintSetting->GetHwnd(), IDC_COMBO_FONT_ZEN );

	/* LOGFONT */
	if( FIXED_PITCH & pelf->elfLogFont.lfPitchAndFamily ){
//		MYTRACE_A( pelf->elfLogFont.lfFaceName, "%ls\n\n", pelf->elfLogFont.lfFaceName );
		Combo_AddString( hwndComboFontHan, pelf->elfLogFont.lfFaceName  );
		Combo_AddString( hwndComboFontZen, pelf->elfLogFont.lfFaceName  );
	}
	return 1;
}

/* ���[�_���_�C�A���O�̕\�� */
int CDlgPrintSetting::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	int*			pnCurrentPrintSetting,
	PRINTSETTING*	pPrintSettingArr
)
{
	int		nRet;
	int		i;
	m_nCurrentPrintSetting = *pnCurrentPrintSetting;
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		m_PrintSettingArr[i] = pPrintSettingArr[i];
	}

	nRet = (int)CDialog::DoModal( hInstance, hwndParent, IDD_PRINTSETTING, NULL );
//	nRet = ::DialogBoxParam(
//		m_hInstance,
//		MAKEINTRESOURCE( IDD_PRINTSETTING ),
//		m_hwndParent,
//		PrintSettingDialogProc,
//		(LPARAM)this
//	);
	if( TRUE == nRet ){
		*pnCurrentPrintSetting = m_nCurrentPrintSetting;
		for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
			pPrintSettingArr[i] = m_PrintSettingArr[i];
		}
	}
	return nRet;
}

BOOL CDlgPrintSetting::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );

	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME ), TRUE );
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_HAN ), TRUE );
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_ZEN ), TRUE );
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_PAPER ), TRUE );

	::SetTimer( GetHwnd(), IDT_PRINTSETTING, 500, NULL );

	/* ���N���X�����o */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgPrintSetting::OnDestroy( void )
{
	::KillTimer( GetHwnd(), IDT_PRINTSETTING );
	/* ���N���X�����o */
	return CDialog::OnDestroy();
}


BOOL CDlgPrintSetting::OnNotify( WPARAM wParam, LPARAM lParam )
{
	CDlgInput1		cDlgInput1;
	NMHDR*			pNMHDR;
	NM_UPDOWN*		pMNUD;
	int				idCtrl;
	BOOL			bSpinDown;
	idCtrl = (int)wParam;
	pNMHDR = (NMHDR*)lParam;
	pMNUD  = (NM_UPDOWN*)lParam;
	if( pMNUD->iDelta < 0 ){
		bSpinDown = FALSE;
	}else{
		bSpinDown = TRUE;
	}
	switch( idCtrl ){
	case IDC_SPIN_FONTWIDTH:
	case IDC_SPIN_LINESPACE:
	case IDC_SPIN_DANSUU:
	case IDC_SPIN_DANSPACE:
	case IDC_SPIN_MARGINTY:
	case IDC_SPIN_MARGINBY:
	case IDC_SPIN_MARGINLX:
	case IDC_SPIN_MARGINRX:
		/* �X�s���R���g���[���̏��� */
		OnSpin( idCtrl, bSpinDown );
		break;
	}
	return TRUE;
}

BOOL CDlgPrintSetting::OnCbnSelChange( HWND hwndCtl, int wID )
{
	if( ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME ) == hwndCtl ){
		/* �ݒ�̃^�C�v���ς���� */
		OnChangeSettingType( TRUE );
		return TRUE;
	}
	return FALSE;

}

BOOL CDlgPrintSetting::OnBnClicked( int wID )
{
	TCHAR			szWork[256];
	CDlgInput1		cDlgInput1;
	HWND			hwndComboSettingName;
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u����y�[�W�ݒ�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( GetHwnd(), m_pszHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PRINT_PAGESETUP) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDC_BUTTON_EDITSETTINGNAME:
		_tcscpy( szWork, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName );
		{
			BOOL bDlgInputResult=cDlgInput1.DoModal(
				m_hInstance,
				GetHwnd(),
				_T("�ݒ薼�̕ύX"),
				_T("�ݒ�̖��̂���͂��Ă��������B"),
				_countof( m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName ) - 1,
				szWork
			);
			if( !bDlgInputResult ){
				return TRUE;
			}
		}
		if( 0 < _tcslen( szWork ) ){
			int		size = _countof(m_PrintSettingArr[0].m_szPrintSettingName) - 1;
			_tcsncpy( m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName, szWork, size);
			m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName[size] = _T('\0');
			/* ����ݒ薼�ꗗ */
			hwndComboSettingName = ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME );
			Combo_ResetContent( hwndComboSettingName );
			int		nSelectIdx;
			int		i;
			int		nItemIdx;
			nSelectIdx = 0;
			for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
				nItemIdx = Combo_AddString( 
					hwndComboSettingName,
					m_PrintSettingArr[i].m_szPrintSettingName
				);
				Combo_SetItemData( hwndComboSettingName, nItemIdx, i );
				if( i == m_nCurrentPrintSetting ){
					nSelectIdx = nItemIdx;
				}
			}
			Combo_SetCurSel( hwndComboSettingName, nSelectIdx );
		}
		return TRUE;
	case IDOK:			/* ������ */
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


/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgPrintSetting::SetData( void )
{
	HDC		hdc;
	HWND	hwndComboFont;
	HWND	hwndComboPaper;
	HWND	hwndComboSettingName;
	int	i;
	int	nItemIdx;
	int	nSelectIdx;


	/* �t�H���g�ꗗ */
	hdc = ::GetDC( m_hwndParent );
	hwndComboFont = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_HAN );
	Combo_ResetContent( hwndComboFont );
	hwndComboFont = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_ZEN );
	Combo_ResetContent( hwndComboFont );
	::EnumFontFamilies(
		hdc,
		NULL,
		(FONTENUMPROC)SetData_EnumFontFamProc,
		(LPARAM)this
	);
	::ReleaseDC( m_hwndParent, hdc );

	/* �p���T�C�Y�ꗗ */
	hwndComboPaper = ::GetDlgItem( GetHwnd(), IDC_COMBO_PAPER );
	Combo_ResetContent( hwndComboPaper );
	// 2006.08.14 Moca �p�����ꗗ�̏d���폜
	for( i = 0; i < CPrint::m_nPaperInfoArrNum; ++i ){
		nItemIdx = Combo_AddString( hwndComboPaper, CPrint::m_paperInfoArr[i].m_pszName );
		Combo_SetItemData( hwndComboPaper, nItemIdx, CPrint::m_paperInfoArr[i].m_nId );
	}


	/* ����ݒ薼�ꗗ */
	hwndComboSettingName = ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME );
	Combo_ResetContent( hwndComboSettingName );
	nSelectIdx = 0;
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		nItemIdx = Combo_AddString( hwndComboSettingName, m_PrintSettingArr[i].m_szPrintSettingName );
		Combo_SetItemData( hwndComboSettingName, nItemIdx, i );
		if( i == m_nCurrentPrintSetting ){
			nSelectIdx = nItemIdx;
		}
	}
	Combo_SetCurSel( hwndComboSettingName, nSelectIdx );

	/* �ݒ�̃^�C�v���ς���� */
	OnChangeSettingType( FALSE );

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==���� FALSE==���̓G���[ */
int CDlgPrintSetting::GetData( void )
{
	HWND	hwndCtrl;
	int		nIdx1;
	int		nWork;

	/* �t�H���g�ꗗ */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_HAN );
	nIdx1 = Combo_GetCurSel( hwndCtrl );
	Combo_GetLBText( hwndCtrl, nIdx1,
		m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceHan
	);
	/* �t�H���g�ꗗ */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_ZEN );
	nIdx1 = Combo_GetCurSel( hwndCtrl );
	Combo_GetLBText( hwndCtrl, nIdx1,
		m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceZen
	);

	/* �p���T�C�Y�ꗗ */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_PAPER );
	nIdx1 = Combo_GetCurSel( hwndCtrl );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperSize =
		(short)Combo_GetItemData( hwndCtrl, nIdx1 );

	// �p���̌���
	// 2006.08.14 Moca �p�������R���{�{�b�N�X��p�~���A�{�^����L����
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_PORTRAIT ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation = DMORIENT_PORTRAIT;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation = DMORIENT_LANDSCAPE;
	}

	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_FONTWIDTH, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight = m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth * 2;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, NULL, FALSE ) * 10;

	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_WORDWRAP ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap = true;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap = false;
	}
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_LINENUMBER ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber = TRUE;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber = FALSE;
	}


	/* ���͒l(���l)�̃G���[�`�F�b�N�����Đ������l��Ԃ� */
	nWork = DataCheckAndCrrect( IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
	}

	nWork = DataCheckAndCrrect( IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
	}
	nWork = DataCheckAndCrrect( IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );
	}


//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = DataCheckAndCrrect( IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = DataCheckAndCrrect( IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = DataCheckAndCrrect( IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = DataCheckAndCrrect( IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = DataCheckAndCrrect( IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = DataCheckAndCrrect( IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = DataCheckAndCrrect( IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX );
//	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = DataCheckAndCrrect( IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );

	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight = m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth * 2;

	//@@@ 2002.2.4 YAZAKI
	/* �w�b�_�[ */
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[0], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[1], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[2], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B

	/* �t�b�^�[ */
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[0], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[1], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[2], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B

	//�s���֑�	//@@@ 2002.04.09 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead =
		( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PS_KINSOKUHEAD ) );
	//�s���֑�	//@@@ 2002.04.09 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail =
		( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PS_KINSOKUTAIL ) );
	//���s�������Ԃ牺����	//@@@ 2002.04.13 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet =
		( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PS_KINSOKURET ) );
	//��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuKuto =
		( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PS_KINSOKUKUTO ) );

	return TRUE;
}


/* �ݒ�̃^�C�v���ς���� */
void CDlgPrintSetting::OnChangeSettingType( BOOL bGetData )
{
	HWND	hwndComboSettingName;
	HWND	hwndCtrl;
	int		nIdx1;
	int		nItemNum;
	int		nItemData;
	int		i;

	if( bGetData ){
		GetData();
	}

	hwndComboSettingName = ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME );
	nIdx1 = Combo_GetCurSel( hwndComboSettingName );
	if( CB_ERR == nIdx1 ){
		return;
	}
	m_nCurrentPrintSetting = Combo_GetItemData( hwndComboSettingName, nIdx1 );

	::SetDlgItemInt( GetHwnd(), IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );

	/* �w�b�_�[ */
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_LEFT] );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_CENTER] );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_RIGHT] );	//	100�����Ő������Ȃ��ƁB�B�B

	/* �t�b�^�[ */
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_LEFT] );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_CENTER] );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_RIGHT] );	//	100�����Ő������Ȃ��ƁB�B�B

	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_WORDWRAP, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_WORDWRAP, BST_UNCHECKED );
	}
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_LINENUMBER, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_LINENUMBER, BST_UNCHECKED );
	}



	/* �t�H���g�ꗗ */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_HAN );
	nIdx1 = Combo_FindStringExact( hwndCtrl, 0, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceHan );
	Combo_SetCurSel( hwndCtrl, nIdx1 );

	/* �t�H���g�ꗗ */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_ZEN );
	nIdx1 = Combo_FindStringExact( hwndCtrl, 0, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceZen );
	Combo_SetCurSel( hwndCtrl, nIdx1 );

	/* �p���T�C�Y�ꗗ */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_PAPER );
	nItemNum = Combo_GetCount( hwndCtrl );
	for( i = 0; i < nItemNum; ++i ){
		nItemData = Combo_GetItemData( hwndCtrl, i );
		if( m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperSize == nItemData ){
			Combo_SetCurSel( hwndCtrl, i );
			break;
		}
	}

	// �p���̌���
	// 2006.08.14 Moca �p�������R���{�{�b�N�X��p�~���A�{�^����L����
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation == DMORIENT_PORTRAIT ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_PORTRAIT, BST_CHECKED );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LANDSCAPE, BST_UNCHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_PORTRAIT, BST_UNCHECKED );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LANDSCAPE, BST_CHECKED );
	}

	// �s���֑�	//@@@ 2002.04.09 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUHEAD, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUHEAD, BST_UNCHECKED );
	}
	// �s���֑�	//@@@ 2002.04.09 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUTAIL, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUTAIL, BST_UNCHECKED );
	}
	// ���s�������Ԃ牺����	//@@@ 2002.04.13 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKURET, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKURET, BST_UNCHECKED );
	}
	// ��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	if( m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuKuto ){
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUKUTO, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_CHECK_PS_KINSOKUKUTO, BST_UNCHECKED );
	}

	return;
}

/* �X�s���R���g���[���̏��� */
void CDlgPrintSetting::OnSpin( int nCtrlId, BOOL bDown )
{
	bool	bUnknown = false;
	int		nData = 0;
	int		nCtrlIdEDIT = 0;
	switch( nCtrlId ){
	case IDC_SPIN_FONTWIDTH:
		nCtrlIdEDIT = IDC_EDIT_FONTWIDTH;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData-=1;
		}else{
			nData+=1;
		}
		break;
	case IDC_SPIN_LINESPACE:
		nCtrlIdEDIT = IDC_EDIT_LINESPACE;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData-=10;
		}else{
			nData+=10;
		}
		break;
	case IDC_SPIN_DANSUU:
		nCtrlIdEDIT = IDC_EDIT_DANSUU;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_DANSPACE:
		nCtrlIdEDIT = IDC_EDIT_DANSPACE;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINTY:
		nCtrlIdEDIT = IDC_EDIT_MARGINTY;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINBY:
		nCtrlIdEDIT = IDC_EDIT_MARGINBY;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINLX:
		nCtrlIdEDIT = IDC_EDIT_MARGINLX;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	case IDC_SPIN_MARGINRX:
		nCtrlIdEDIT = IDC_EDIT_MARGINRX;
		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
		if( bDown ){
			nData--;
		}else{
			++nData;
		}
		break;
	default:
		bUnknown = true;
		break;
	}
	if( !bUnknown ){
		/* ���͒l(���l)�̃G���[�`�F�b�N�����Đ������l��Ԃ� */
		nData = DataCheckAndCrrect( nCtrlIdEDIT, nData );
		::SetDlgItemInt( GetHwnd(), nCtrlIdEDIT, nData, FALSE );
	}
}


/* ���͒l(���l)�̃G���[�`�F�b�N�����Đ������l��Ԃ� */
int CDlgPrintSetting::DataCheckAndCrrect( int nCtrlId, int nData )
{
	switch( nCtrlId ){
	case IDC_EDIT_FONTWIDTH:
		if( 7 >= nData ){
			nData = 7;
		}
		if( 100 < nData ){
			nData = 100;
		}
		break;
	case IDC_EDIT_LINESPACE:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 150 < nData ){
			nData = 150;
		}
		break;
	case IDC_EDIT_DANSUU:
		if( 1 >= nData ){
			nData = 1;
		}
		if( 4 < nData ){
			nData = 4;
		}
		break;
	case IDC_EDIT_DANSPACE:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 30 < nData ){
			nData = 30;
		}
		break;
	case IDC_EDIT_MARGINTY:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 50 < nData ){
			nData = 50;
		}
		break;
	case IDC_EDIT_MARGINBY:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 50 < nData ){
			nData = 50;
		}
		break;
	case IDC_EDIT_MARGINLX:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 50 < nData ){
			nData = 50;
		}
		break;
	case IDC_EDIT_MARGINRX:
		if( 0 >= nData ){
			nData = 0;
		}
		if( 50 < nData ){
			nData = 50;
		}
		break;
	}
	return nData;
}

/* �^�C�}�[���� */
BOOL CDlgPrintSetting::OnTimer( WPARAM wParam )
{
	int nTimer;
	int				nEnableColmns;		/* �s������̕����� */
	int				nEnableLines;		/* �c�����̍s�� */
	MYDEVMODE		dmDummy;			// 2003.05.18 ����� �^�ύX
	short			nPaperAllWidth;		/* �p���� */
	short			nPaperAllHeight;	/* �p������ */
	PRINTSETTING*	pPS;
	nTimer = (int)wParam;

	if( nTimer != IDT_PRINTSETTING ){
		return FALSE;
	}
	/* �_�C�A���O�f�[�^�̎擾 */
	GetData();
//	SetData( GetHwnd() );
	pPS = &m_PrintSettingArr[m_nCurrentPrintSetting];

	dmDummy.dmFields = DM_PAPERSIZE | DMORIENT_LANDSCAPE;
	dmDummy.dmPaperSize = pPS->m_nPrintPaperSize;
	dmDummy.dmOrientation = pPS->m_nPrintPaperOrientation;
	/* �p���̕��A���� */
	if( !CPrint::GetPaperSize(
		&nPaperAllWidth,
		&nPaperAllHeight,
		&dmDummy
	) ){
	// 2001.12.21 hor GetPaperSize���s���͂��̂܂܏I��
	//	nPaperAllWidth = 210 * 10;		/* �p���� */
	//	nPaperAllHeight = 297 * 10;		/* �p������ */
		return FALSE;
	}
	/* �s������̕�����(�s�ԍ�����) */
	nEnableColmns =
		( nPaperAllWidth - pPS->m_nPrintMarginLX - pPS->m_nPrintMarginRX
		- ( pPS->m_nPrintDansuu - 1 ) * pPS->m_nPrintDanSpace
//		- ( pPS->m_nPrintDansuu ) *  ( ( m_nPreview_LineNumberColmns /*+ (m_nPreview_LineNumberColmns?1:0)*/ ) * pPS->m_nPrintFontWidth )
		) / pPS->m_nPrintFontWidth / pPS->m_nPrintDansuu;	/* �󎚉\����/�y�[�W */
	/* �c�����̍s�� */
	nEnableLines =
		( nPaperAllHeight - pPS->m_nPrintMarginTY - pPS->m_nPrintMarginBY ) /
		( pPS->m_nPrintFontHeight + ( pPS->m_nPrintFontHeight * pPS->m_nPrintLineSpacing / 100 ) ) - 4;	/* �󎚉\�s��/�y�[�W */
//	/* �y�[�W������̍s�� */
//	nEnableLinesAll = nEnableLines * pPS->m_nPrintDansuu;

	::SetDlgItemInt( GetHwnd(), IDC_STATIC_ENABLECOLMNS, nEnableColmns, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_STATIC_ENABLELINES, nEnableLines, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_STATIC_ENABLELINESALL, nEnableLinesAll, FALSE );
	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgPrintSetting::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


