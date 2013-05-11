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
#include "util/window.h"
#include "sakura_rc.h"	// 2002/2/10 aroka
#include "sakura.hh"

// ����ݒ� CDlgPrintSetting.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12500
	IDC_COMBO_SETTINGNAME,			HIDC_PS_COMBO_SETTINGNAME,	//�y�[�W�ݒ�
	IDC_BUTTON_EDITSETTINGNAME,		HIDC_PS_BUTTON_EDITSETTINGNAME,	//�ݒ薼�ύX
	IDC_COMBO_FONT_HAN,				HIDC_PS_COMBO_FONT_HAN,		//���p�t�H���g
	IDC_COMBO_FONT_ZEN,				HIDC_PS_COMBO_FONT_ZEN,		//�S�p�t�H���g
	IDC_EDIT_FONTWIDTH,				HIDC_PS_EDIT_FONTWIDTH,		//�t�H���g��
	IDC_SPIN_FONTWIDTH,				HIDC_PS_EDIT_FONTWIDTH,		//12570,
	IDC_SPIN_LINESPACE,				HIDC_PS_EDIT_LINESPACE,		//12571,
	IDC_EDIT_LINESPACE,				HIDC_PS_EDIT_LINESPACE,		//�s����
	IDC_EDIT_DANSUU,				HIDC_PS_EDIT_DANSUU,		//�i��
	IDC_SPIN_DANSUU,				HIDC_PS_EDIT_DANSUU,		//12572,
	IDC_EDIT_DANSPACE,				HIDC_PS_EDIT_DANSPACE,		//�i�̌���
	IDC_SPIN_DANSPACE,				HIDC_PS_EDIT_DANSPACE,		//12573,
	IDC_COMBO_PAPER,				HIDC_PS_COMBO_PAPER,		//�p���T�C�Y
	IDC_RADIO_PORTRAIT,				HIDC_PS_STATIC_PAPERORIENT,	//������
	IDC_RADIO_LANDSCAPE,			HIDC_PS_STATIC_PAPERORIENT,	//�c����
	IDC_EDIT_MARGINTY,				HIDC_PS_EDIT_MARGINTY,		//�]����
	IDC_SPIN_MARGINTY,				HIDC_PS_EDIT_MARGINTY,		//12574,
	IDC_EDIT_MARGINBY,				HIDC_PS_EDIT_MARGINBY,		//�]����
	IDC_SPIN_MARGINBY,				HIDC_PS_EDIT_MARGINBY,		//12575,
	IDC_EDIT_MARGINLX,				HIDC_PS_EDIT_MARGINLX,		//�]����
	IDC_SPIN_MARGINLX,				HIDC_PS_EDIT_MARGINLX,		//12576,
	IDC_EDIT_MARGINRX,				HIDC_PS_EDIT_MARGINRX,		//�]���E
	IDC_SPIN_MARGINRX,				HIDC_PS_EDIT_MARGINRX,		//12577,
	IDC_CHECK_WORDWRAP,				HIDC_PS_CHECK_WORDWRAP,		//���[�h���b�v
	IDC_CHECK_LINENUMBER,			HIDC_PS_CHECK_LINENUMBER,	//�s�ԍ�
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
	IDOK,							HIDOK_PS,					//OK
	IDCANCEL,						HIDCANCEL_PS,				//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_PS_BUTTON_HELP,		//�w���v
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
//		MYTRACE( _T("%ls\n\n"), pelf->elfLogFont.lfFaceName );
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
	PRINTSETTING*	pPrintSettingArr,
	int				nLineNumberColmns
)
{
	int		nRet;
	int		i;
	m_nCurrentPrintSetting = *pnCurrentPrintSetting;
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		m_PrintSettingArr[i] = pPrintSettingArr[i];
	}
	m_nLineNumberColmns = nLineNumberColmns;

	nRet = (int)CDialog::DoModal( hInstance, hwndParent, IDD_PRINTSETTING, NULL );
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

	// �^�C�}�[�ł̍X�V����߂āA�\���I�ɍX�V�v������ 2013.5.5 aroka
	// CDialog::OnInitDialog�̉���OnChangeSettingType���Ă΂��̂ł����ł͍X�V�v�����Ȃ�
	//	::SetTimer( GetHwnd(), IDT_PRINTSETTING, 500, NULL );
	//UpdatePrintableLineAndColumn();

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
		UpdatePrintableLineAndColumn();
		break;
	}
	return TRUE;
}

BOOL CDlgPrintSetting::OnCbnSelChange( HWND hwndCtl, int wID )
{
//	if( ::GetDlgItem( GetHwnd(), IDC_COMBO_SETTINGNAME ) == hwndCtl ){
	switch( wID ){
	case IDC_COMBO_SETTINGNAME:
		/* �ݒ�̃^�C�v���ς���� */
		OnChangeSettingType( TRUE );
		return TRUE;
	case IDC_COMBO_FONT_HAN:
	case IDC_COMBO_FONT_ZEN:
	case IDC_COMBO_PAPER:
		UpdatePrintableLineAndColumn();
		break;	// �����ł͍s�ƌ��̍X�V�v���̂݁B��̏�����CDialog�ɔC����B
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
	case IDOK:
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	case IDC_RADIO_PORTRAIT:
	case IDC_RADIO_LANDSCAPE:
		UpdatePrintableLineAndColumn();
		break;	// �����ł͍s�ƌ��̍X�V�v���̂݁B��̏�����CDialog�ɔC����B
	case IDC_CHECK_LINENUMBER:
		UpdatePrintableLineAndColumn();
		break;	// �����ł͍s�ƌ��̍X�V�v���̂݁B��̏�����CDialog�ɔC����B
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}


BOOL CDlgPrintSetting::OnStnClicked( int wID )
{
	switch( wID ){
	case IDC_STATIC_ENABLECOLMNS:
	case IDC_STATIC_ENABLELINES:
		// ����N���b�N�͎󂯕t���Ă��Ȃ����A���b�Z�[�W�����������̂ł����ɔz�u 2013.5.5 aroka
		CalcPrintableLineAndColumn();
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnStnClicked( wID );
}


BOOL CDlgPrintSetting::OnEnChange( HWND hwndCtl, int wID )
{
	switch( wID ){
//	case IDC_EDIT_FONTWIDTH:	// �t�H���g���̍ŏ��l����O�̂���'12'�Ɠ��͂����'1'�̂Ƃ���ŏR���Ă��܂� 2013.5.5 aroka
	case IDC_EDIT_LINESPACE:
	case IDC_EDIT_DANSUU:
	case IDC_EDIT_DANSPACE:
	case IDC_EDIT_MARGINTY:
	case IDC_EDIT_MARGINBY:
	case IDC_EDIT_MARGINLX:
	case IDC_EDIT_MARGINRX:
		UpdatePrintableLineAndColumn();
		break;	// �����ł͍s�ƌ��̍X�V�v���̂݁B��̏�����CDialog�ɔC����B
	}
	/* ���N���X�����o */
	return CDialog::OnEnChange( hwndCtl, wID );
}


BOOL CDlgPrintSetting::OnEnKillFocus( HWND hwndCtl, int wID )
{
	switch( wID ){
	case IDC_EDIT_FONTWIDTH:
	//case IDC_EDIT_LINESPACE:	// EN_CHANGE �Ōv�Z���Ă���̂ŏ璷���ȁA�Ǝv���R�����g�A�E�g 2013.5.5 aroka
	//case IDC_EDIT_DANSUU:
	//case IDC_EDIT_DANSPACE:
	//case IDC_EDIT_MARGINTY:
	//case IDC_EDIT_MARGINBY:
	//case IDC_EDIT_MARGINLX:
	//case IDC_EDIT_MARGINRX:
		UpdatePrintableLineAndColumn();
		break;	// �����ł͍s�ƌ��̍X�V�v���̂݁B��̏�����CDialog�ɔC����B
	}
	/* ���N���X�����o */
	return CDialog::OnEnKillFocus( hwndCtl, wID );
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

	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_FONTWIDTH, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, NULL, FALSE );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, NULL, FALSE ) * 10;

	/* ���͒l(���l)�̃G���[�`�F�b�N�����Đ������l��Ԃ� */
	nWork = DataCheckAndCorrect( IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth, FALSE );
	}
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontHeight = m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth * 2;

	nWork = DataCheckAndCorrect( IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
	}
	nWork = DataCheckAndCorrect( IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu = nWork;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
	}
	nWork = DataCheckAndCorrect( IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );
	}

	/* �p���T�C�Y�ꗗ */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_PAPER );
	nIdx1 = Combo_GetCurSel( hwndCtrl );
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperSize =
		(short)Combo_GetItemData( hwndCtrl, nIdx1 );

	// �p���̌���
	// 2006.08.14 Moca �p�������R���{�{�b�N�X��p�~���A�{�^����L����
	if( IsDlgButtonCheckedBool( GetHwnd(), IDC_RADIO_PORTRAIT ) ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation = DMORIENT_PORTRAIT;
	}else{
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation = DMORIENT_LANDSCAPE;
	}

	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, NULL, FALSE ) * 10;
	m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, NULL, FALSE ) * 10;

	/* ���͒l(���l)�̃G���[�`�F�b�N�����Đ������l��Ԃ� */
	nWork = DataCheckAndCorrect( IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
	}
	nWork = DataCheckAndCorrect( IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
	}
	nWork = DataCheckAndCorrect( IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
	}
	nWork = DataCheckAndCorrect( IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10 );
	if( nWork != m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10 ){
		m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX = nWork * 10;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );
	}

	// �s�ԍ������
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_LINENUMBER );
	// �p�����[�h���b�v
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_WORDWRAP );

	//�s���֑�	//@@@ 2002.04.09 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_PS_KINSOKUHEAD );
	//�s���֑�	//@@@ 2002.04.09 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_PS_KINSOKUTAIL );
	//���s�������Ԃ牺����	//@@@ 2002.04.13 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_PS_KINSOKURET );
	//��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuKuto = IsDlgButtonCheckedBool( GetHwnd(), IDC_CHECK_PS_KINSOKUKUTO );

	//@@@ 2002.2.4 YAZAKI
	/* �w�b�_�[ */
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[0], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[1], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_HEAD3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[2], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B

	/* �t�b�^�[ */
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[0], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[1], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_GetText( GetHwnd(), IDC_EDIT_FOOT3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[2], HEADER_MAX );	//	100�����Ő������Ȃ��ƁB�B�B

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

	/* �t�H���g�ꗗ */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_HAN );
	nIdx1 = Combo_FindStringExact( hwndCtrl, 0, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceHan );
	Combo_SetCurSel( hwndCtrl, nIdx1 );

	/* �t�H���g�ꗗ */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_FONT_ZEN );
	nIdx1 = Combo_FindStringExact( hwndCtrl, 0, m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintFontFaceZen );
	Combo_SetCurSel( hwndCtrl, nIdx1 );

	::SetDlgItemInt( GetHwnd(), IDC_EDIT_FONTWIDTH, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintFontWidth, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINESPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintLineSpacing, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSUU, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDansuu, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_DANSPACE, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintDanSpace / 10, FALSE );

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
	bool bIsPortrait = ( m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintPaperOrientation == DMORIENT_PORTRAIT );
	CheckDlgButtonBool( GetHwnd(), IDC_RADIO_PORTRAIT, bIsPortrait );
	CheckDlgButtonBool( GetHwnd(), IDC_RADIO_LANDSCAPE, !bIsPortrait );

	// �]��
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINTY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginTY / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINBY, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginBY / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINLX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginLX / 10, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_MARGINRX, m_PrintSettingArr[m_nCurrentPrintSetting].m_nPrintMarginRX / 10, FALSE );

	// �s�ԍ������
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_LINENUMBER, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintLineNumber );
	// �p�����[�h���b�v
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_WORDWRAP, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintWordWrap );

	// �s���֑�	//@@@ 2002.04.09 MIK
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PS_KINSOKUHEAD, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuHead );
	// �s���֑�	//@@@ 2002.04.09 MIK
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PS_KINSOKUTAIL, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuTail );

	// ���s�������Ԃ牺����	//@@@ 2002.04.13 MIK
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PS_KINSOKURET, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuRet );
	// ��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	CheckDlgButtonBool( GetHwnd(), IDC_CHECK_PS_KINSOKUKUTO, m_PrintSettingArr[m_nCurrentPrintSetting].m_bPrintKinsokuKuto );

	/* �w�b�_�[ */
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_LEFT] );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_CENTER] );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_HEAD3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szHeaderForm[POS_RIGHT] );	//	100�����Ő������Ȃ��ƁB�B�B

	/* �t�b�^�[ */
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT1, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_LEFT] );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT2, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_CENTER] );	//	100�����Ő������Ȃ��ƁB�B�B
	::DlgItem_SetText( GetHwnd(), IDC_EDIT_FOOT3, m_PrintSettingArr[m_nCurrentPrintSetting].m_szFooterForm[POS_RIGHT] );	//	100�����Ő������Ȃ��ƁB�B�B

	UpdatePrintableLineAndColumn();
	return;
}


const struct {
	int ctrlid;
	int minval;
	int maxval;
} sDataRange[] = {
	IDC_EDIT_FONTWIDTH,	7,	100,	//!< 1/10mm
	IDC_EDIT_LINESPACE,	0,	150,	//!< %
	IDC_EDIT_DANSUU,	1,	4,
	IDC_EDIT_DANSPACE,	0,	30,		//!< mm
	IDC_EDIT_MARGINTY,	0,	50,		//!< mm
	IDC_EDIT_MARGINBY,	0,	50,		//!< mm
	IDC_EDIT_MARGINLX,	0,	50,		//!< mm
	IDC_EDIT_MARGINRX,	0,	50,		//!< mm
};

/* �X�s���R���g���[���̏��� */
void CDlgPrintSetting::OnSpin( int nCtrlId, BOOL bDown )
{
	int		nData = 0;
	int		nCtrlIdEDIT = 0;
	int		nDiff = 1;
	int		nIdx = -1;
	switch( nCtrlId ){
	case IDC_SPIN_FONTWIDTH:	nIdx = 0;				break;
	case IDC_SPIN_LINESPACE:	nIdx = 1;	nDiff=10;	break;
	case IDC_SPIN_DANSUU:		nIdx = 2;				break;
	case IDC_SPIN_DANSPACE:		nIdx = 3;				break;
	case IDC_SPIN_MARGINTY:		nIdx = 4;				break;
	case IDC_SPIN_MARGINBY:		nIdx = 5;				break;
	case IDC_SPIN_MARGINLX:		nIdx = 6;				break;
	case IDC_SPIN_MARGINRX:		nIdx = 7;				break;
	}
	if( nIdx >= 0 ){
		nCtrlIdEDIT = sDataRange[nIdx].ctrlid;
 		nData = ::GetDlgItemInt( GetHwnd(), nCtrlIdEDIT, NULL, FALSE );
 		if( bDown ){
			nData -= nDiff;
 		}else{
			nData += nDiff;
 		}
		/* ���͒l(���l)�̃G���[�`�F�b�N�����Đ������l��Ԃ� */
		nData = DataCheckAndCorrect( nCtrlIdEDIT, nData );
		::SetDlgItemInt( GetHwnd(), nCtrlIdEDIT, nData, FALSE );
	}
}


/* ���͒l(���l)�̃G���[�`�F�b�N�����Đ������l��Ԃ� */
int CDlgPrintSetting::DataCheckAndCorrect( int nCtrlId, int nData )
{
	int nIdx = -1;
	switch( nCtrlId ){
	case IDC_EDIT_FONTWIDTH:	nIdx = 0;		break;
	case IDC_EDIT_LINESPACE:	nIdx = 1;		break;
	case IDC_EDIT_DANSUU:		nIdx = 2;		break;
	case IDC_EDIT_DANSPACE:		nIdx = 3;		break;
	case IDC_EDIT_MARGINTY:		nIdx = 4;		break;
	case IDC_EDIT_MARGINBY:		nIdx = 5;		break;
	case IDC_EDIT_MARGINLX:		nIdx = 6;		break;
	case IDC_EDIT_MARGINRX:		nIdx = 7;		break;
	}
	if( nIdx >= 0 ){
		if( nData <= sDataRange[nIdx].minval ){
			nData = sDataRange[nIdx].minval;
 		}
		if( nData > sDataRange[nIdx].maxval ){
			nData = sDataRange[nIdx].maxval;
 		}
	}
	return nData;
}


/* �^�C�}�[���� */
BOOL CDlgPrintSetting::CalcPrintableLineAndColumn()
{
	int				nEnableColmns;		/* �s������̕����� */
	int				nEnableLines;		/* �c�����̍s�� */
	MYDEVMODE		dmDummy;			// 2003.05.18 ����� �^�ύX
	short			nPaperAllWidth;		/* �p���� */
	short			nPaperAllHeight;	/* �p������ */
	PRINTSETTING*	pPS;

	// ���b�Z�[�W���A�����đ���ꂽ�Ƃ��͈�񂾂��Ή����� 2013.5.5 aroka
	if( m_bPrintableLinesAndColumnInvalid == false ){
		return TRUE;
	}
	m_bPrintableLinesAndColumnInvalid = false;

	/* �_�C�A���O�f�[�^�̎擾 */
	GetData();
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
	nEnableColmns = CPrint::CalculatePrintableColumns( pPS, nPaperAllWidth, pPS->m_bPrintLineNumber?m_nLineNumberColmns:0 );	/* �󎚉\����/�y�[�W */
	/* �c�����̍s�� */
	nEnableLines = CPrint::CalculatePrintableLines( pPS, nPaperAllHeight );			/* �󎚉\�s��/�y�[�W */

	::SetDlgItemInt( GetHwnd(), IDC_STATIC_ENABLECOLMNS, nEnableColmns, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_STATIC_ENABLELINES, nEnableLines, FALSE );

	// �󎚉\�̈悪�Ȃ��ꍇ�� OK �������Ȃ����� 2013.5.10 aroka
	if( nEnableColmns == 0 || nEnableLines == 0 ){
		::EnableWindow( GetDlgItem( GetHwnd(), IDOK ), FALSE );
	}else{
		::EnableWindow( GetDlgItem( GetHwnd(), IDOK ), TRUE );
	}

	return TRUE;
}


// �s���ƌ����̍X�V��v���i���b�Z�[�W�L���[�Ƀ|�X�g����j
// �_�C�A���O�������̓r���� EN_CHANGE �ɔ�������ƌv�Z�����������Ȃ邽�߁A�֐��Ăяo���ł͂Ȃ�PostMessage�ŏ��� 2013.5.5 aroka
void CDlgPrintSetting::UpdatePrintableLineAndColumn()
{
	m_bPrintableLinesAndColumnInvalid = true;
	::PostMessageA( GetHwnd(), WM_COMMAND, MAKELONG( IDC_STATIC_ENABLECOLMNS, STN_CLICKED ), (LPARAM)::GetDlgItem( GetHwnd(), IDC_STATIC_ENABLECOLMNS ) );
}


//@@@ 2002.01.18 add start
LPVOID CDlgPrintSetting::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


