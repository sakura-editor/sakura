/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�E�B���h�E�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, genta, MIK, asa-o
	Copyright (C) 2002, YAZAKI, genta, Moca, aroka
	Copyright (C) 2003, MIK, KEITA, genta
	Copyright (C) 2004, Moca
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
//#include "debug/Debug.h" // 2002/2/10 aroka
//#include "global.h"
#include "dlg/CDlgWinSize.h"	//	2004.05.13 Moca
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//11200
	IDC_CHECK_DispFUNCKEYWND,		HIDC_CHECK_DispFUNCKEYWND,		//�t�@���N�V�����L�[�\��
	IDC_CHECK_DispSTATUSBAR,		HIDC_CHECK_DispSTATUSBAR,		//�X�e�[�^�X�o�[�\��
	IDC_CHECK_DispTOOLBAR,			HIDC_CHECK_DispTOOLBAR,			//�c�[���o�[�\��
	IDC_CHECK_bScrollBarHorz,		HIDC_CHECK_bScrollBarHorz,		//�����X�N���[���o�[
	IDC_CHECK_bMenuIcon,			HIDC_CHECK_bMenuIcon,			//�A�C�R���t�����j���[
	IDC_CHECK_SplitterWndVScroll,	HIDC_CHECK_SplitterWndVScroll,	//�����X�N���[���̓���	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_CHECK_SplitterWndHScroll,	HIDC_CHECK_SplitterWndHScroll,	//�����X�N���[���̓���	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_EDIT_nRulerBottomSpace,		HIDC_EDIT_nRulerBottomSpace,	//���[���[�̍���
	IDC_EDIT_nRulerHeight,			HIDC_EDIT_nRulerHeight,			//���[���[�ƃe�L�X�g�̊Ԋu
	IDC_EDIT_nLineNumberRightSpace,	HIDC_EDIT_nLineNumberRightSpace,	//�s�ԍ��ƃe�L�X�g�̌���
	IDC_RADIO_FUNCKEYWND_PLACE1,	HIDC_RADIO_FUNCKEYWND_PLACE1,	//�t�@���N�V�����L�[�\���ʒu
	IDC_RADIO_FUNCKEYWND_PLACE2,	HIDC_RADIO_FUNCKEYWND_PLACE2,	//�t�@���N�V�����L�[�\���ʒu
	IDC_EDIT_FUNCKEYWND_GROUPNUM,	HIDC_EDIT_FUNCKEYWND_GROUPNUM,	//�t�@���N�V�����L�[�̃O���[�v�{�^����
	IDC_SPIN_nRulerBottomSpace,		HIDC_EDIT_nRulerBottomSpace,
	IDC_SPIN_nRulerHeight,			HIDC_EDIT_nRulerHeight,
	IDC_SPIN_nLineNumberRightSpace,	HIDC_EDIT_nLineNumberRightSpace,
	IDC_SPIN_FUNCKEYWND_GROUPNUM,	HIDC_EDIT_FUNCKEYWND_GROUPNUM,
	IDC_WINCAPTION_ACTIVE,			HIDC_WINCAPTION_ACTIVE,			//�A�N�e�B�u��	//@@@ 2003.06.15 MIK
	IDC_WINCAPTION_INACTIVE,		HIDC_WINCAPTION_INACTIVE,		//��A�N�e�B�u��	//@@@ 2003.06.15 MIK
	IDC_BUTTON_WINSIZE,				HIDC_BUTTON_WINSIZE,			//�ʒu�Ƒ傫���̐ݒ�	// 2006.08.06 ryoji
	//	Feb. 11, 2007 genta TAB�֘A�́u�^�u�o�[�v�V�[�g�ֈړ�
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropWin::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta


/* ���b�Z�[�W���� */
INT_PTR CPropWin::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,	// message
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
//	WORD		wNotifyCode;
//	WORD		wID;
//	HWND		hwndCtl;

// From Here Sept. 9, 2000 JEPRO
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
// To Here Sept. 9, 2000

	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
	int			nVal;	//Sept.21, 2000 JEPRO �X�s���v�f���������̂ŕ���������
//	LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� Window */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		/* ���[���[���� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_nRulerHeight ), EM_LIMITTEXT, (WPARAM)2, 0 );
		/* ���[���[�ƃe�L�X�g�̌��� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_nRulerBottomSpace ), EM_LIMITTEXT, (WPARAM)2, 0 );

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_WIN );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE_A( "Window PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 Window */
				GetData( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_WIN;
				return TRUE;
			}
			break;
		case IDC_SPIN_nRulerHeight:
			/* ���[���|�̍��� */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < IDC_SPIN_nRulerHeight_MIN ){
				nVal = IDC_SPIN_nRulerHeight_MIN;
			}
			if( nVal > IDC_SPIN_nRulerHeight_MAX ){
				nVal = IDC_SPIN_nRulerHeight_MAX;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_nRulerBottomSpace:
			/* ���[���[�ƃe�L�X�g�̌��� */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > 32 ){
				nVal = 32;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_nLineNumberRightSpace:
			/* ���[���[�ƃe�L�X�g�̌��� */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nLineNumberRightSpace, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > 32 ){
				nVal = 32;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_nLineNumberRightSpace, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_FUNCKEYWND_GROUPNUM:
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 12 ){
				nVal = 12;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, nVal, FALSE );
			return TRUE;
		}
		break;
//****	To Here Sept. 21, 2000
//	From Here Sept. 9, 2000 JEPRO
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			//	�t�@���N�V�����L�[��\�����鎞�������̈ʒu�w���Enable�ɐݒ�
			case IDC_CHECK_DispFUNCKEYWND:
				EnableWinPropInput( hwndDlg );
				break;

			// From Here 2004.05.13 Moca �u�ʒu�Ƒ傫���̐ݒ�v�{�^��
			//	�E�B���h�E�ݒ�_�C�A���O�ɂċN�����̃E�B���h�E��Ԏw��
			case IDC_BUTTON_WINSIZE:
				{
					CDlgWinSize cDlgWinSize;
					RECT rc;
					rc.right  = m_Common.m_sWindow.m_nWinSizeCX;
					rc.bottom = m_Common.m_sWindow.m_nWinSizeCY;
					rc.top    = m_Common.m_sWindow.m_nWinPosX;
					rc.left   = m_Common.m_sWindow.m_nWinPosY;
					cDlgWinSize.DoModal(
						::GetModuleHandle(NULL),
						hwndDlg,
						m_Common.m_sWindow.m_eSaveWindowSize,
						m_Common.m_sWindow.m_eSaveWindowPos,
						m_Common.m_sWindow.m_nWinSizeType,
						rc
					);
					m_Common.m_sWindow.m_nWinSizeCX = rc.right;
					m_Common.m_sWindow.m_nWinSizeCY = rc.bottom;
					m_Common.m_sWindow.m_nWinPosX = rc.top;
					m_Common.m_sWindow.m_nWinPosY = rc.left;
				}
				break;
			// To Here 2004.05.13 Moca
			}
			break;
		}
		break;
//	To Here Sept. 9, 2000

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

/* �_�C�A���O�f�[�^�̐ݒ� */
void CPropWin::SetData( HWND hwndDlg )
{
//	BOOL	bRet;

	/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispTOOLBAR, m_Common.m_sWindow.m_bDispTOOLBAR );

	/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispFUNCKEYWND, m_Common.m_sWindow.m_bDispFUNCKEYWND );

	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
	if( 0 == m_Common.m_sWindow.m_nFUNCKEYWND_Place ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, TRUE );
	}
	// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����
	::SetDlgItemInt( hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum, FALSE );

	//From Here@@@ 2003.06.13 MIK
	//	Feb. 12, 2007 genta TAB�֘A�́u�^�u�o�[�v�V�[�g�ֈړ�

	//To Here@@@ 2003.06.13 MIK
	//	Feb. 11, 2007 genta TAB�֘A�́u�^�u�o�[�v�V�[�g�ֈړ�

	/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_DispSTATUSBAR, m_Common.m_sWindow.m_bDispSTATUSBAR );

	/* ���[���[���� */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, m_Common.m_sWindow.m_nRulerHeight, FALSE );
	/* ���[���[�ƃe�L�X�g�̌��� */
	::SetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, m_Common.m_sWindow.m_nRulerBottomSpace, FALSE );
	//	Sep. 18. 2002 genta �s�ԍ��ƃe�L�X�g�̌���
	::SetDlgItemInt( hwndDlg, IDC_EDIT_nLineNumberRightSpace, m_Common.m_sWindow.m_nLineNumRightSpace, FALSE );

	/* ���[���[�̃^�C�v *///	del 2008/7/4 Uchi
//	if( 0 == m_Common.m_sWindow.m_nRulerType ){
//		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_0, TRUE );
//		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_1, FALSE );
//	}else{
//		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_0, FALSE );
//		::CheckDlgButton( hwndDlg, IDC_RADIO_nRulerType_1, TRUE );
//	}

	/* �����X�N���[���o�[ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bScrollBarHorz, m_Common.m_sWindow.m_bScrollBarHorz );

	/* �A�C�R���t�����j���[ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_bMenuIcon, m_Common.m_sWindow.m_bMenuIcon );

	//	2001/06/20 Start by asa-o:	�X�N���[���̓���
	::CheckDlgButton( hwndDlg, IDC_CHECK_SplitterWndVScroll, m_Common.m_sWindow.m_bSplitterWndVScroll );
	::CheckDlgButton( hwndDlg, IDC_CHECK_SplitterWndHScroll, m_Common.m_sWindow.m_bSplitterWndHScroll );
	//	2001/06/20 End

	//	Apr. 05, 2003 genta �E�B���h�E�L���v�V�����̃J�X�^�}�C�Y
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_WINCAPTION_ACTIVE   ), EM_LIMITTEXT, _countof( m_Common.m_sWindow.m_szWindowCaptionActive   ) - 1, (LPARAM)0 );	//@@@ 2003.06.13 MIK
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_WINCAPTION_INACTIVE ), EM_LIMITTEXT, _countof( m_Common.m_sWindow.m_szWindowCaptionInactive ) - 1, (LPARAM)0 );	//@@@ 2003.06.13 MIK
	::DlgItem_SetText( hwndDlg, IDC_WINCAPTION_ACTIVE, m_Common.m_sWindow.m_szWindowCaptionActive );
	::DlgItem_SetText( hwndDlg, IDC_WINCAPTION_INACTIVE, m_Common.m_sWindow.m_szWindowCaptionInactive );

	//	Fronm Here Sept. 9, 2000 JEPRO
	//	�t�@���N�V�����L�[��\�����鎞�������̈ʒu�w���Enable�ɐݒ�
	EnableWinPropInput( hwndDlg );
	//	To Here Sept. 9, 2000

	return;
}





/* �_�C�A���O�f�[�^�̎擾 */
int CPropWin::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_WIN;

	/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
	m_Common.m_sWindow.m_bDispTOOLBAR = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTOOLBAR );

	/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
	m_Common.m_sWindow.m_bDispFUNCKEYWND = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispFUNCKEYWND );

	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1 ) ){
		m_Common.m_sWindow.m_nFUNCKEYWND_Place = 0;
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2) ){
		m_Common.m_sWindow.m_nFUNCKEYWND_Place = 1;
	}

	// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����
	m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum = ::GetDlgItemInt( hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, NULL, FALSE );
	if( m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum < 1 ){
		m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum = 1;
	}
	if( m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum > 12 ){
		m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum = 12;
	}

	//From Here@@@ 2003.06.13 MIK
	//	Feb. 12, 2007 genta TAB�֘A�́u�^�u�o�[�v�V�[�g�ֈړ�
	//To Here@@@ 2003.06.13 MIK

	/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */
	m_Common.m_sWindow.m_bDispSTATUSBAR = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispSTATUSBAR );

	/* ���[���[�̃^�C�v *///	del 2008/7/4 Uchi
//	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_nRulerType_0 ) ){
//		m_Common.m_sWindow.m_nRulerType = 0;
//	}
//	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_nRulerType_1 ) ){
//		m_Common.m_sWindow.m_nRulerType = 1;
//	}

	/* ���[���[���� */
	m_Common.m_sWindow.m_nRulerHeight = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerHeight, NULL, FALSE );
	if( m_Common.m_sWindow.m_nRulerHeight < IDC_SPIN_nRulerHeight_MIN ){
		m_Common.m_sWindow.m_nRulerHeight = IDC_SPIN_nRulerHeight_MIN;
	}
	if( m_Common.m_sWindow.m_nRulerHeight > IDC_SPIN_nRulerHeight_MAX ){
		m_Common.m_sWindow.m_nRulerHeight = IDC_SPIN_nRulerHeight_MAX;
	}
	/* ���[���[�ƃe�L�X�g�̌��� */
	m_Common.m_sWindow.m_nRulerBottomSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nRulerBottomSpace, NULL, FALSE );
	if( m_Common.m_sWindow.m_nRulerBottomSpace < 0 ){
		m_Common.m_sWindow.m_nRulerBottomSpace = 0;
	}
	if( m_Common.m_sWindow.m_nRulerBottomSpace > 32 ){
		m_Common.m_sWindow.m_nRulerBottomSpace = 32;
	}

	//	Sep. 18. 2002 genta �s�ԍ��ƃe�L�X�g�̌���
	m_Common.m_sWindow.m_nLineNumRightSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_nLineNumberRightSpace, NULL, FALSE );
	if( m_Common.m_sWindow.m_nLineNumRightSpace < 0 ){
		m_Common.m_sWindow.m_nLineNumRightSpace = 0;
	}
	if( m_Common.m_sWindow.m_nLineNumRightSpace > 32 ){
		m_Common.m_sWindow.m_nLineNumRightSpace = 32;
	}

	/* �����X�N���[���o�[ */
	m_Common.m_sWindow.m_bScrollBarHorz = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bScrollBarHorz );

	/* �A�C�R���t�����j���[ */
	m_Common.m_sWindow.m_bMenuIcon = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_bMenuIcon );

	//	2001/06/20 Start by asa-o:	�X�N���[���̓���
	m_Common.m_sWindow.m_bSplitterWndVScroll = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SplitterWndVScroll );
	m_Common.m_sWindow.m_bSplitterWndHScroll = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SplitterWndHScroll );
	//	2001/06/20 End

	//	Apr. 05, 2003 genta �E�B���h�E�L���v�V�����̃J�X�^�}�C�Y
	::DlgItem_GetText( hwndDlg, IDC_WINCAPTION_ACTIVE, m_Common.m_sWindow.m_szWindowCaptionActive,
		_countof( m_Common.m_sWindow.m_szWindowCaptionActive ) );
	::DlgItem_GetText( hwndDlg, IDC_WINCAPTION_INACTIVE, m_Common.m_sWindow.m_szWindowCaptionInactive,
		_countof( m_Common.m_sWindow.m_szWindowCaptionInactive ) );

	return TRUE;
}





//	From Here Sept. 9, 2000 JEPRO
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void CPropWin::EnableWinPropInput( HWND hwndDlg )
{
	//	�t�@�N�V�����L�[��\�����邩�ǂ���
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispFUNCKEYWND ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM ), TRUE );	// IDC_GROUP_FUNCKEYWND_POSITION->IDC_EDIT_FUNCKEYWND_GROUPNUM 2008/7/4 Uchi
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2 ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM ), FALSE );	// IDC_GROUP_FUNCKEYWND_POSITION->IDC_EDIT_FUNCKEYWND_GROUPNUM 2008/7/4 Uchi
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2 ), FALSE );
	}
}
//	To Here Sept. 9, 2000



