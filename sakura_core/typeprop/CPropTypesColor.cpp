/*! @file
	@brief �^�C�v�ʐݒ� - �J���[

	@date 2008.04.12 kobake CPropTypes.cpp���番��
	@date 2009.02.22 ryoji
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, MIK, hor, Stonee, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta, ������, Moca
	Copyright (C) 2003, MIK, zenryaku, Moca, naoh, KEITA, genta
	Copyright (C) 2005, MIK, genta, Moca, ryoji
	Copyright (C) 2006, ryoji, fon, novice
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, nasukoji
	Copyright (C) 2009, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CPropTypes.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/23 Uchi
#include "CDlgSameColor.h"
#include "CDlgKeywordSelect.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "prop/CPropCommon.h"

using namespace std;

WNDPROC	m_wpColorListProc;

static const DWORD p_helpids2[] = {	//11400
	IDC_LIST_COLORS,				HIDC_LIST_COLORS,				//�F�w��
	IDC_CHECK_DISP,					HIDC_CHECK_DISP,				//�F�����\��
	IDC_CHECK_BOLD,					HIDC_CHECK_BOLD,				//����
	IDC_CHECK_UNDERLINE,			HIDC_CHECK_UNDERLINE,			//����
	IDC_BUTTON_TEXTCOLOR,			HIDC_BUTTON_TEXTCOLOR,			//�����F
	IDC_BUTTON_BACKCOLOR,			HIDC_BUTTON_BACKCOLOR,			//�w�i�F
	IDC_BUTTON_SAMETEXTCOLOR,		HIDC_BUTTON_SAMETEXTCOLOR,		//�����F����
	IDC_BUTTON_SAMEBKCOLOR,			HIDC_BUTTON_SAMEBKCOLOR,		//�w�i�F����
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_COLOR,		//�C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_COLOR,		//�G�N�X�|�[�g
	IDC_COMBO_SET,					HIDC_COMBO_SET_COLOR,			//�����L�[���[�h�P�Z�b�g��
	IDC_BUTTON_KEYWORD_SELECT,		HIDC_BUTTON_KEYWORD_SELECT,		//�����L�[���[�h2�`10	// 2006.08.06 ryoji
	IDC_EDIT_BLOCKCOMMENT_FROM,		HIDC_EDIT_BLOCKCOMMENT_FROM,	//�u���b�N�R�����g�P�J�n
	IDC_EDIT_BLOCKCOMMENT_TO,		HIDC_EDIT_BLOCKCOMMENT_TO,		//�u���b�N�R�����g�P�I��
	IDC_EDIT_BLOCKCOMMENT_FROM2,	HIDC_EDIT_BLOCKCOMMENT_FROM2,	//�u���b�N�R�����g�Q�J�n
	IDC_EDIT_BLOCKCOMMENT_TO2,		HIDC_EDIT_BLOCKCOMMENT_TO2,		//�u���b�N�R�����g�Q�I��
	IDC_EDIT_LINECOMMENT,			HIDC_EDIT_LINECOMMENT,			//�s�R�����g�P
	IDC_EDIT_LINECOMMENT2,			HIDC_EDIT_LINECOMMENT2,			//�s�R�����g�Q
	IDC_EDIT_LINECOMMENT3,			HIDC_EDIT_LINECOMMENT3,			//�s�R�����g�R
	IDC_EDIT_LINECOMMENTPOS,		HIDC_EDIT_LINECOMMENTPOS,		//�����P
	IDC_EDIT_LINECOMMENTPOS2,		HIDC_EDIT_LINECOMMENTPOS2,		//�����Q
	IDC_EDIT_LINECOMMENTPOS3,		HIDC_EDIT_LINECOMMENTPOS3,		//�����R
	IDC_CHECK_LCPOS,				HIDC_CHECK_LCPOS,				//���w��P
	IDC_CHECK_LCPOS2,				HIDC_CHECK_LCPOS2,				//���w��Q
	IDC_CHECK_LCPOS3,				HIDC_CHECK_LCPOS3,				//���w��R
	IDC_RADIO_ESCAPETYPE_1,			HIDC_RADIO_ESCAPETYPE_1,		//������G�X�P�[�v�iC���ꕗ�j
	IDC_RADIO_ESCAPETYPE_2,			HIDC_RADIO_ESCAPETYPE_2,		//������G�X�P�[�v�iPL/SQL���j
	IDC_EDIT_VERTLINE,				HIDC_EDIT_VERTLINE,				//�c���̌��w��	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};


//	�s�R�����g�Ɋւ�����
struct {
	int nEditID;
	int nCheckBoxID;
	int nTextID;
} const cLineComment[COMMENT_DELIMITER_NUM] = {
	{ IDC_EDIT_LINECOMMENT	, IDC_CHECK_LCPOS , IDC_EDIT_LINECOMMENTPOS },
	{ IDC_EDIT_LINECOMMENT2	, IDC_CHECK_LCPOS2, IDC_EDIT_LINECOMMENTPOS2},
	{ IDC_EDIT_LINECOMMENT3	, IDC_CHECK_LCPOS3, IDC_EDIT_LINECOMMENTPOS3}
};

/* �F�̐ݒ���C���|�[�g */
// 2010/4/23 Uchi Import�̊O�o��
bool CPropTypesColor::Import( HWND hwndDlg )
{
	ColorInfo		ColorInfoArr[64];
	CImpExpColors	cImpExpColors( ColorInfoArr );

	/* �F�ݒ� I/O */
	for( int i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
		ColorInfoArr[i] = m_Types.m_ColorInfoArr[i];
		_tcscpy( ColorInfoArr[i].m_szName, m_Types.m_ColorInfoArr[i].m_szName );
	}

	// �C���|�[�g
	if (!cImpExpColors.ImportUI(m_hInstance, hwndDlg)) {
		// �C���|�[�g�����Ă��Ȃ�
		return false;
	}

	/* �f�[�^�̃R�s�[ */
	m_Types.m_nColorInfoArrNum = COLORIDX_LAST;
	for( int i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
		m_Types.m_ColorInfoArr[i] =  ColorInfoArr[i];
		_tcscpy( m_Types.m_ColorInfoArr[i].m_szName, ColorInfoArr[i].m_szName );
	}
	/* �_�C�A���O�f�[�^�̐ݒ� color */
	SetData( hwndDlg );

	return true;
}


/* �F�̐ݒ���G�N�X�|�[�g */
// 2010/4/23 Uchi Export�̊O�o��
bool CPropTypesColor::Export( HWND hwndDlg )
{
	CImpExpColors	cImpExpColors( m_Types.m_ColorInfoArr);

	// �G�N�X�|�[�g
	return cImpExpColors.ExportUI(m_hInstance, hwndDlg);
}



LRESULT APIENTRY ColorList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int			xPos = 0;
	int			yPos;
	int			nIndex = -1;
	int			nItemNum;
	RECT		rcItem = {0,0,0,0};
	int			i;
	POINT		poMouse;
	ColorInfo*	pColorInfo;

	switch( uMsg ){
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONUP:
		xPos = LOWORD(lParam);	// horizontal position of cursor
		yPos = HIWORD(lParam);	// vertical position of cursor

		poMouse.x = xPos;
		poMouse.y = yPos;
		nItemNum = List_GetCount( hwnd );
		for( i = 0; i < nItemNum; ++i ){
			List_GetItemRect( hwnd, i, &rcItem );
			if( ::PtInRect( &rcItem, poMouse ) ){
//				MYTRACE( _T("hit at i==%d\n"), i );
//				MYTRACE( _T("\n") );
				nIndex = i;
				break;
			}
		}
		break;
	}
	switch( uMsg ){
	case WM_RBUTTONDOWN:

		if( -1 == nIndex ){
			break;
		}
		if( 18 <= xPos && xPos <= rcItem.right - 29 ){	// 2009.02.22 ryoji �L���͈͂̐����ǉ�
			List_SetCurSel( hwnd, nIndex );
			::SendMessageCmd( ::GetParent( hwnd ), WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwnd );
			pColorInfo = (ColorInfo*)List_GetItemData( hwnd, nIndex );
			/* ���� */
			if( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_UNDERLINE) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
			{
				if( pColorInfo->m_bUnderLine ){	/* �����ŕ\�� */
					pColorInfo->m_bUnderLine = false;
					::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_UNDERLINE, FALSE );
				}else{
					pColorInfo->m_bUnderLine = true;
					::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_UNDERLINE, TRUE );
				}
				::InvalidateRect( hwnd, &rcItem, TRUE );
			}
		}
		break;

	case WM_LBUTTONDBLCLK:
		if( -1 == nIndex ){
			break;
		}
		if( 18 <= xPos && xPos <= rcItem.right - 29 ){	// 2009.02.22 ryoji �L���͈͂̐����ǉ�
			pColorInfo = (ColorInfo*)List_GetItemData( hwnd, nIndex );
			/* �����ŕ\�� */
			if( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_BOLD) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
			{
				if( pColorInfo->m_bBoldFont ){	/* �����ŕ\�� */
					pColorInfo->m_bBoldFont = false;
					::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_BOLD, FALSE );
				}else{
					pColorInfo->m_bBoldFont = true;
					::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_BOLD, TRUE );
				}
				::InvalidateRect( hwnd, &rcItem, TRUE );
			}
		}
		break;
	case WM_LBUTTONUP:
		if( -1 == nIndex ){
			break;
		}
		pColorInfo = (ColorInfo*)List_GetItemData( hwnd, nIndex );
		/* �F����/�\�� ���� */
		if( 2 <= xPos && xPos <= 16
			&& ( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_FORCE_DISP) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
			)
		{
			if( pColorInfo->m_bDisp ){	/* �F����/�\������ */
				pColorInfo->m_bDisp = false;
			}else{
				pColorInfo->m_bDisp = true;
			}
			if( COLORIDX_GYOU == nIndex ){
				pColorInfo = (ColorInfo*)List_GetItemData( hwnd, nIndex );

			}

			::InvalidateRect( hwnd, &rcItem, TRUE );
		}else
		/* �O�i�F���{ ��` */
		if( rcItem.right - 27 <= xPos && xPos <= rcItem.right - 27 + 12
			&& ( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_TEXT) ) )
		{
			/* �F�I���_�C�A���O */
			// 2005.11.30 Moca �J�X�^���F�ێ�
			DWORD* pColors = (DWORD*)::GetProp( hwnd, _T("ptrCustomColors") );
			if( CPropTypesColor::SelectColor( hwnd, &pColorInfo->m_colTEXT, pColors ) ){
				::InvalidateRect( hwnd, &rcItem, TRUE );
				::InvalidateRect( ::GetDlgItem( ::GetParent( hwnd ), IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
			}
		}else
		/* �O�i�F���{ ��` */
		if( rcItem.right - 13 <= xPos && xPos <= rcItem.right - 13 + 12
			&& ( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
			)
		{
			/* �F�I���_�C�A���O */
			// 2005.11.30 Moca �J�X�^���F�ێ�
			DWORD* pColors = (DWORD*)::GetProp( hwnd, _T("ptrCustomColors") );
			if( CPropTypesColor::SelectColor( hwnd, &pColorInfo->m_colBACK, pColors ) ){
				::InvalidateRect( hwnd, &rcItem, TRUE );
				::InvalidateRect( ::GetDlgItem( ::GetParent( hwnd ), IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
			}
		}
		break;
	// 2005.11.30 Moca �J�X�^���F�ێ�
	case WM_DESTROY:
		if( ::GetProp( hwnd, _T("ptrCustomColors") ) ){
			::RemoveProp( hwnd, _T("ptrCustomColors") );
		}
		break;
	}
	return CallWindowProc( m_wpColorListProc, hwnd, uMsg, wParam, lParam );
}





/* color ���b�Z�[�W���� */
INT_PTR CPropTypesColor::DispatchEvent(
	HWND				hwndDlg,	// handle to dialog box
	UINT				uMsg,		// message
	WPARAM				wParam,		// first message parameter
	LPARAM				lParam 		// second message parameter
)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	NMHDR*				pNMHDR;
	NM_UPDOWN*			pMNUD;
	int					idCtrl;
	int					nVal;
	int					nIndex;
	static HWND			hwndListColor;
	LPDRAWITEMSTRUCT	pDis;

	switch( uMsg ){
	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		hwndListColor = ::GetDlgItem( hwndDlg, IDC_LIST_COLORS );

		/* �_�C�A���O�f�[�^�̐ݒ� color */
		SetData( hwndDlg );

		/* �F���X�g���t�b�N */
		// Modified by KEITA for WIN64 2003.9.6
		m_wpColorListProc = (WNDPROC) ::SetWindowLongPtr( hwndListColor, GWLP_WNDPROC, (LONG_PTR)ColorList_SubclassProc );
		// 2005.11.30 Moca �J�X�^���F��ێ�
		::SetProp( hwndListColor, _T("ptrCustomColors"), m_dwCustColors );
		
		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD( wParam );	/* �ʒm�R�[�h */
		wID			= LOWORD( wParam );	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		if( hwndListColor == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = List_GetCurSel( hwndListColor );
				m_nCurrentColorType = nIndex;		/* ���ݑI������Ă���F�^�C�v */

				{
					// �e��R���g���[���̗L���^������؂�ւ���	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
					unsigned int fAttribute = g_ColorAttributeArr[nIndex].fAttribute;
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ),			(0 == (fAttribute & COLOR_ATTRIB_FORCE_DISP))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BOLD ),			(0 == (fAttribute & COLOR_ATTRIB_NO_BOLD))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ),		(0 == (fAttribute & COLOR_ATTRIB_NO_UNDERLINE))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_MOZI ),			(0 == (fAttribute & COLOR_ATTRIB_NO_TEXT))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ),		(0 == (fAttribute & COLOR_ATTRIB_NO_TEXT))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ),	(0 == (fAttribute & COLOR_ATTRIB_NO_TEXT))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ),			(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ),		(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ),	(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );
				}

				/* �F����/�\�� ������ */
				if( m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp ){
					::CheckDlgButton( hwndDlg, IDC_CHECK_DISP, TRUE );
				}else{
					::CheckDlgButton( hwndDlg, IDC_CHECK_DISP, FALSE );
				}
				/* �����ŕ\�� */
				if( m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bBoldFont ){
					::CheckDlgButton( hwndDlg, IDC_CHECK_BOLD, TRUE );
				}else{
					::CheckDlgButton( hwndDlg, IDC_CHECK_BOLD, FALSE );
				}
				/* ������\�� */
				if( m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine ){
					::CheckDlgButton( hwndDlg, IDC_CHECK_UNDERLINE, TRUE );
				}else{
					::CheckDlgButton( hwndDlg, IDC_CHECK_UNDERLINE, FALSE );
				}


				::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
				::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
				return TRUE;
			}
		}
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_BUTTON_SAMETEXTCOLOR: /* �����F���� */
				{
					// 2006.04.26 ryoji �����F�^�w�i�F����_�C�A���O���g��
					CDlgSameColor cDlgSameColor;
					COLORREF cr = m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colTEXT;
					cDlgSameColor.DoModal( ::GetModuleHandle(NULL), hwndDlg, wID, &m_Types, cr );
				}
				::InvalidateRect( hwndListColor, NULL, TRUE );
				return TRUE;

			case IDC_BUTTON_SAMEBKCOLOR:	/* �w�i�F���� */
				{
					// 2006.04.26 ryoji �����F�^�w�i�F����_�C�A���O���g��
					CDlgSameColor cDlgSameColor;
					COLORREF cr = m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK;
					cDlgSameColor.DoModal( ::GetModuleHandle(NULL), hwndDlg, wID, &m_Types, cr );
				}
				::InvalidateRect( hwndListColor, NULL, TRUE );
				return TRUE;

			case IDC_BUTTON_TEXTCOLOR:	/* �e�L�X�g�F */
				/* �F�I���_�C�A���O */
				if( SelectColor( hwndDlg, &m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colTEXT, m_dwCustColors ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
				}
				/* ���ݑI������Ă���F�^�C�v */
				List_SetCurSel( hwndListColor, m_nCurrentColorType );
				return TRUE;
			case IDC_BUTTON_BACKCOLOR:	/* �w�i�F */
				/* �F�I���_�C�A���O */
				if( SelectColor( hwndDlg, &m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK, m_dwCustColors ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
				}
				/* ���ݑI������Ă���F�^�C�v */
				List_SetCurSel( hwndListColor, m_nCurrentColorType );
				return TRUE;
			case IDC_CHECK_DISP:	/* �F����/�\�� ������ */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp = true;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp = false;
				}
				/* ���ݑI������Ă���F�^�C�v */
				List_SetCurSel( hwndListColor, m_nCurrentColorType );
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK ���K�\���L�[���[�h�̂���
				return TRUE;
			case IDC_CHECK_BOLD:	/* ������ */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BOLD ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bBoldFont = true;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bBoldFont = false;
				}
				/* ���ݑI������Ă���F�^�C�v */
				List_SetCurSel( hwndListColor, m_nCurrentColorType );
				return TRUE;
			case IDC_CHECK_UNDERLINE:	/* ������\�� */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_UNDERLINE ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine = true;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine = false;
				}
				/* ���ݑI������Ă���F�^�C�v */
				List_SetCurSel( hwndListColor, m_nCurrentColorType );
				return TRUE;

			case IDC_BUTTON_IMPORT:	/* �F�̐ݒ���C���|�[�g */
				Import( hwndDlg );
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK ���K�\���L�[���[�h�̂���
				return TRUE;

			case IDC_BUTTON_EXPORT:	/* �F�̐ݒ���G�N�X�|�[�g */
				Export( hwndDlg );
				return TRUE;

			//	From Here Jun. 6, 2001 genta
			//	�s�R�����g�J�n���w���ON/OFF
			case IDC_CHECK_LCPOS:
			case IDC_CHECK_LCPOS2:
			case IDC_CHECK_LCPOS3:
			//	To Here Jun. 6, 2001 genta
				EnableTypesPropInput( hwndDlg );
				return TRUE;
			//	To Here Sept. 10, 2000

			//�����L�[���[�h�̑I��
			case IDC_BUTTON_KEYWORD_SELECT:
				{
					CDlgKeywordSelect cDlgKeywordSelect;
					//�����L�[���[�h1���擾����B
					HWND hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
					int nIdx = Combo_GetCurSel( hwndCombo );
					if( CB_ERR == nIdx || 0 == nIdx ){
						m_nSet[ 0 ] = -1;
					}else{
						m_nSet[ 0 ] = nIdx - 1;
					}
					cDlgKeywordSelect.DoModal( ::GetModuleHandle(NULL), hwndDlg, m_nSet );
					RearrangeKeywordSet( hwndDlg );	//	Jan. 23, 2005 genta �L�[���[�h�Z�b�g�Ĕz�u
					//�����L�[���[�h1�𔽉f����B
					if( -1 == m_nSet[ 0 ] ){
						Combo_SetCurSel( hwndCombo, 0 );
					}else{
						Combo_SetCurSel( hwndCombo, m_nSet[ 0 ] + 1 );
					}
				}
				break;
			//�����L�[���[�h�̑I��
			case IDC_BUTTON_EDITKEYWORD:
				{
					CPropKeyword* pPropKeyword = new CPropKeyword;
					CPropCommon* pCommon = (CPropCommon*)pPropKeyword;
					pCommon->InitData();
					pCommon->m_nKeywordSet1 = m_nSet[0];
					INT_PTR res = ::DialogBoxParam(
						::GetModuleHandle(NULL),
						MAKEINTRESOURCE( IDD_PROP_KEYWORD ),
						hwndDlg,
						CPropKeyword::DlgProc_dialog,
						(LPARAM)pPropKeyword
					);
					if( res == IDOK ){
						pCommon->ApplyData();
						SetData(hwndDlg);
					}
					delete pPropKeyword;
					return TRUE;
				}
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
		//	From Here May 21, 2001 genta activate spin control
		case IDC_SPIN_LCColNum:
			/* �s�R�����g���ʒu */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 1000 ){
				nVal = 1000;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_LCColNum2:
			/* �s�R�����g���ʒu */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS2, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 1000 ){
				nVal = 1000;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS2, nVal, FALSE );
			return TRUE;
		//	To Here May 21, 2001 genta activate spin control

		//	From Here Jun. 01, 2001 JEPRO 3�ڂ�ǉ�
		case IDC_SPIN_LCColNum3:
			/* �s�R�����g���ʒu */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS3, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 1000 ){
				nVal = 1000;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINECOMMENTPOS3, nVal, FALSE );
			return TRUE;
		//	To Here Jun. 01, 2001
		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
//	Sept. 10, 2000 JEPRO ID�������ۂ̖��O�ɕύX���邽�߈ȉ��̍s�̓R�����g�A�E�g
//				OnHelp( hwndDlg, IDD_PROP1P3 );
				OnHelp( hwndDlg, IDD_PROP_COLOR );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( _T("color PSN_KILLACTIVE\n") );
				/* �_�C�A���O�f�[�^�̎擾 color */
				GetData( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = 1;
				return TRUE;
			}
			break;	/* default */
		}
		break;	/* WM_NOTIFY */
	case WM_DRAWITEM:
		idCtrl = (UINT) wParam;				/* �R���g���[����ID */
		pDis = (LPDRAWITEMSTRUCT) lParam;	/* ���ڕ`���� */
		switch( idCtrl ){

		case IDC_BUTTON_TEXTCOLOR:	/* �e�L�X�g�F */
			DrawColorButton( pDis, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colTEXT );
			return TRUE;
		case IDC_BUTTON_BACKCOLOR:	/* �w�i�F */
			DrawColorButton( pDis, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK );
			return TRUE;
		case IDC_LIST_COLORS:		/* �F��ʃ��X�g */
			DrawColorListItem( pDis );
			return TRUE;
		}
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_pszHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids2 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_pszHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids2 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}


/* �_�C�A���O�f�[�^�̐ݒ� color */
void CPropTypesColor::SetData( HWND hwndDlg )
{

	HWND	hwndWork;
	int		i;
	int		nItem;

	m_nCurrentColorType = 0;	/* ���ݑI������Ă���F�^�C�v */

	/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */	//@@@ 2002.09.22 YAZAKI
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT )		, COMMENT_DELIMITER_BUFFERSIZE - 1 );
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT2 )		, COMMENT_DELIMITER_BUFFERSIZE - 1 );
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT3 )		, COMMENT_DELIMITER_BUFFERSIZE - 1 );	//Jun. 01, 2001 JEPRO �ǉ�
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM )	, BLOCKCOMMENT_BUFFERSIZE - 1 );
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO )	, BLOCKCOMMENT_BUFFERSIZE - 1 );
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2 ), BLOCKCOMMENT_BUFFERSIZE - 1 );
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2 )	, BLOCKCOMMENT_BUFFERSIZE - 1 );
//#endif

	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, m_Types.m_cBlockComments[0].getBlockCommentFrom() );	/* �u���b�N�R�����g�f���~�^(From) */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO	, m_Types.m_cBlockComments[0].getBlockCommentTo() );	/* �u���b�N�R�����g�f���~�^(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, m_Types.m_cBlockComments[1].getBlockCommentFrom() );	/* �u���b�N�R�����g�f���~�^2(From) */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, m_Types.m_cBlockComments[1].getBlockCommentTo() );	/* �u���b�N�R�����g�f���~�^2(To) */
//#endif

	/* �s�R�����g�f���~�^ @@@ 2002.09.22 YAZAKI*/
	//	From Here May 12, 2001 genta
	//	�s�R�����g�̊J�n���ʒu�ݒ�
	//	May 21, 2001 genta ���ʒu��1���琔����悤��
	for ( i=0; i<COMMENT_DELIMITER_NUM; i++ ){
		//	�e�L�X�g
		::DlgItem_SetText( hwndDlg, cLineComment[i].nEditID, m_Types.m_cLineComment.getLineComment(i) );	

		//	�����`�F�b�N�ƁA���l
		int nPos = m_Types.m_cLineComment.getLineCommentPos(i);
		if( nPos >= 0 ){
			::CheckDlgButton( hwndDlg, cLineComment[i].nCheckBoxID, TRUE );
			::SetDlgItemInt( hwndDlg, cLineComment[i].nTextID, nPos + 1, FALSE );
		}
		else {
			::CheckDlgButton( hwndDlg, cLineComment[i].nCheckBoxID, FALSE );
			::SetDlgItemInt( hwndDlg, cLineComment[i].nTextID, (~nPos) + 1, FALSE );
		}
	}

	if( 0 == m_Types.m_nStringType ){	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_1, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_2, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_ESCAPETYPE_2, TRUE );
	}

	/* �Z�b�g���R���{�{�b�N�X�̒l�Z�b�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	Combo_ResetContent( hwndWork );  /* �R���{�{�b�N�X����ɂ��� */
	/* ��s�ڂ͋� */
	Combo_AddString( hwndWork, L" " );
	//	Mar. 31, 2003 genta KeyWordSetMgr���|�C���^��
	if( 0 < m_pCKeyWordSetMgr->m_nKeyWordSetNum ){
		for( i = 0; i < m_pCKeyWordSetMgr->m_nKeyWordSetNum; ++i ){
			Combo_AddString( hwndWork, m_pCKeyWordSetMgr->GetTypeName( i ) );
		}
		if( -1 == m_Types.m_nKeyWordSetIdx[0] ){
			/* �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I�� */
			Combo_SetCurSel( hwndWork, 0 );
		}else{
			/* �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I�� */
			Combo_SetCurSel( hwndWork, m_Types.m_nKeyWordSetIdx[0] + 1 );
		}
	}

	//�����L�[���[�h1�`10�̐ݒ�
	for( i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		m_nSet[ i ] = m_Types.m_nKeyWordSetIdx[i];
	}

	/* �F�����镶����ނ̃��X�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_COLORS );
	List_ResetContent( hwndWork );  /* ���X�g����ɂ��� */
	for( i = 0; i < COLORIDX_LAST; ++i ){
		nItem = ::List_AddString( hwndWork, m_Types.m_ColorInfoArr[i].m_szName );
		List_SetItemData( hwndWork, nItem, &m_Types.m_ColorInfoArr[i] );
	}
	/* ���ݑI������Ă���F�^�C�v */
	List_SetCurSel( hwndWork, m_nCurrentColorType );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwndWork );

	// from here 2005.11.30 Moca �w��ʒu�c���̐ݒ�
	WCHAR szVertLine[MAX_VERTLINES * 15] = L"";
	int offset = 0;
	for( i = 0; i < MAX_VERTLINES && m_Types.m_nVertLineIdx[i] != 0; i++ ){
		CLayoutInt nXCol = m_Types.m_nVertLineIdx[i];
		CLayoutInt nXColEnd = nXCol;
		CLayoutInt nXColAdd = CLayoutInt(1);
		if( nXCol < 0 ){
			if( i < MAX_VERTLINES - 2 ){
				nXCol = -nXCol;
				nXColEnd = m_Types.m_nVertLineIdx[++i];
				nXColAdd = m_Types.m_nVertLineIdx[++i];
				if( nXColEnd < nXCol || nXColAdd <= 0 ){
					continue;
				}
				if(offset){
					szVertLine[offset] = ',';
					szVertLine[offset+1] = '\0';
					offset += 1;
				}
				offset += auto_sprintf( &szVertLine[offset], L"%d(%d,%d)", nXColAdd, nXCol, nXColEnd );
			}
		}
		else{
			if(offset){
				szVertLine[offset] = ',';
				szVertLine[offset+1] = '\0';
				offset += 1;
			}
			offset += auto_sprintf( &szVertLine[offset], L"%d", nXCol );
		}
	}
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_VERTLINE ), MAX_VERTLINES * 15 );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_VERTLINE, szVertLine );
	// to here 2005.11.30 Moca �w��ʒu�c���̐ݒ�
	return;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



/* �_�C�A���O�f�[�^�̎擾 color */
int CPropTypesColor::GetData( HWND hwndDlg )
{
	int		nIdx;
	HWND	hwndWork;

//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = 1;

	//	From Here May 12, 2001 genta
	//	�R�����g�̊J�n���ʒu�̎擾
	//	May 21, 2001 genta ���ʒu��1���琔����悤��
	wchar_t buffer[COMMENT_DELIMITER_BUFFERSIZE];	//@@@ 2002.09.22 YAZAKI LineComment���擾���邽�߂̃o�b�t�@
	int pos;
	UINT en;
	BOOL bTranslated;

	int i;
	for( i=0; i<COMMENT_DELIMITER_NUM; i++ ){
		en = ::IsDlgButtonChecked( hwndDlg, cLineComment[i].nCheckBoxID );
		pos = ::GetDlgItemInt( hwndDlg, cLineComment[i].nTextID, &bTranslated, FALSE );
		if( bTranslated != TRUE ){
			en = 0;
			pos = 0;
		}
		//	pos == 0�̂Ƃ��͖�������
		if( pos == 0 )	en = 0;
		else			--pos;
		//	�����̂Ƃ���1�̕␔�Ŋi�[

		::DlgItem_GetText( hwndDlg, cLineComment[i].nEditID		, buffer	, COMMENT_DELIMITER_BUFFERSIZE );		/* �s�R�����g�f���~�^ */
		m_Types.m_cLineComment.CopyTo( i, buffer, en ? pos : ~pos );
	}

	wchar_t szFromBuffer[BLOCKCOMMENT_BUFFERSIZE];	//@@@ 2002.09.22 YAZAKI
	wchar_t szToBuffer[BLOCKCOMMENT_BUFFERSIZE];	//@@@ 2002.09.22 YAZAKI

	::DlgItem_GetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, szFromBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* �u���b�N�R�����g�f���~�^(From) */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, szToBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* �u���b�N�R�����g�f���~�^(To) */
	m_Types.m_cBlockComments[0].SetBlockCommentRule( szFromBuffer, szToBuffer );

	::DlgItem_GetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, szFromBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* �u���b�N�R�����g�f���~�^(From) */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, szToBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* �u���b�N�R�����g�f���~�^(To) */
	m_Types.m_cBlockComments[1].SetBlockCommentRule( szFromBuffer, szToBuffer );

	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_ESCAPETYPE_1 ) ){
		m_Types.m_nStringType = 0;
	}else{
		m_Types.m_nStringType = 1;
	}

	/* �Z�b�g���R���{�{�b�N�X�̒l�Z�b�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	nIdx = Combo_GetCurSel( hwndWork );
	if( CB_ERR == nIdx ||
		0 == nIdx ){
		m_Types.m_nKeyWordSetIdx[0] = -1;
	}else{
		m_Types.m_nKeyWordSetIdx[0] = nIdx - 1;

	}

	//�����L�[���[�h2�`10�̎擾(1�͕�)
	for( nIdx = 1; nIdx < MAX_KEYWORDSET_PER_TYPE; nIdx++ ){
		m_Types.m_nKeyWordSetIdx[nIdx] = m_nSet[nIdx];
	}

	// from here 2005.11.30 Moca �w��ʒu�c���̐ݒ�
	WCHAR szVertLine[MAX_VERTLINES * 15];
	::DlgItem_GetText( hwndDlg, IDC_EDIT_VERTLINE, szVertLine, MAX_VERTLINES * 15 );

	int offset = 0;
	i = 0;
	while( i < MAX_VERTLINES ){
		int value = 0;
		for(; '0' <= szVertLine[offset] && szVertLine[offset] <= '9';  offset++){
			value = szVertLine[offset] - '0' + value * 10;
		}
		if( value <= 0 ){
			break;
		}
		if( szVertLine[offset] == '(' ){
			offset++;
			int valueBegin = 0;
			int valueEnd = 0;
			for(; '0' <= szVertLine[offset] && szVertLine[offset] <= '9';  offset++){
				valueBegin = szVertLine[offset] - '0' + valueBegin * 10;
			}
			if( valueBegin <= 0 ){
				break;
			}
			if( szVertLine[offset] == ',' ){
				offset++;
			}else if( szVertLine[offset] != ')' ){
				break;
			}
			for(; '0' <= szVertLine[offset] && szVertLine[offset] <= '9';  offset++){
				valueEnd = szVertLine[offset] - '0' + valueEnd * 10;
			}
			if( valueEnd <= 0 ){
				valueEnd = MAXLINEKETAS;
			}
			if( szVertLine[offset] != ')' ){
				break;
			}
			offset++;
			if(i + 2 < MAX_VERTLINES){
				m_Types.m_nVertLineIdx[i++] = CLayoutInt(-valueBegin);
				m_Types.m_nVertLineIdx[i++] = CLayoutInt(valueEnd);
				m_Types.m_nVertLineIdx[i++] = CLayoutInt(value);
			}
			else{
				break;
			}
		}
		else{
			m_Types.m_nVertLineIdx[i++] = CLayoutInt(value);
		}
		if( szVertLine[offset] != ',' ){
			break;
		}
		offset++;
	}
	if( i < MAX_VERTLINES ){
		m_Types.m_nVertLineIdx[i] = CLayoutInt(0);
	}
	// to here 2005.11.30 Moca �w��ʒu�c���̐ݒ�
	return TRUE;
}



/* �F�{�^���̕`�� */
void CPropTypesColor::DrawColorButton( DRAWITEMSTRUCT* pDis, COLORREF cColor )
{
//	MYTRACE( _T("pDis->itemAction = ") );

	COLORREF	cBtnHiLight		= (COLORREF)::GetSysColor(COLOR_3DHILIGHT);
	COLORREF	cBtnShadow		= (COLORREF)::GetSysColor(COLOR_3DSHADOW);
	COLORREF	cBtnDkShadow	= (COLORREF)::GetSysColor(COLOR_3DDKSHADOW);
	COLORREF	cBtnFace		= (COLORREF)::GetSysColor(COLOR_3DFACE);
	RECT		rc;
	RECT		rcFocus;

	//�`��Ώ�
	CGraphics gr(pDis->hDC);

	/* �{�^���̕\�ʂ̐F�œh��Ԃ� */
	gr.SetBrushColor( cBtnFace );
	gr.FillMyRect( pDis->rcItem );

	/* �g�̕`�� */
	rcFocus = rc = pDis->rcItem;
	rc.top += 4;
	rc.left += 4;
	rc.right -= 4;
	rc.bottom -= 4;
	rcFocus = rc;
//	rc.right -= 11;

	if( pDis->itemState & ODS_SELECTED ){

		gr.SetPen(cBtnDkShadow);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 2, NULL );
		::LineTo( gr, 0, 0 );
		::LineTo( gr, pDis->rcItem.right - 1, 0 );

		gr.SetPen(cBtnShadow);
		::MoveToEx( gr, 1, pDis->rcItem.bottom - 3, NULL );
		::LineTo( gr, 1, 1 );
		::LineTo( gr, pDis->rcItem.right - 2, 1 );

		gr.SetPen(cBtnHiLight);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 1, NULL );
		::LineTo( gr, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1 );
		::LineTo( gr, pDis->rcItem.right - 1, -1 );

		rc.top += 1;
		rc.left += 1;
		rc.right += 1;
		rc.bottom += 1;

		rcFocus.top += 1;
		rcFocus.left += 1;
		rcFocus.right += 1;
		rcFocus.bottom += 1;

	}
	else{
		gr.SetPen(cBtnHiLight);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 2, NULL );
		::LineTo( gr, 0, 0 );
		::LineTo( gr, pDis->rcItem.right - 1, 0 );

		gr.SetPen(cBtnShadow);
		::MoveToEx( gr, 1, pDis->rcItem.bottom - 2, NULL );
		::LineTo( gr, pDis->rcItem.right - 2, pDis->rcItem.bottom - 2 );
		::LineTo( gr, pDis->rcItem.right - 2, 0 );

		gr.SetPen(cBtnDkShadow);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 1, NULL );
		::LineTo( gr, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1 );
		::LineTo( gr, pDis->rcItem.right - 1, -1 );
	}
	
	if((pDis->itemState & ODS_DISABLED)==0){
		/* �w��F�œh��Ԃ� */
		gr.SetBrushColor(cColor);
		gr.SetPen(cBtnShadow);
		::RoundRect( gr, rc.left, rc.top, rc.right, rc.bottom , 5, 5 );
	}

	/* �t�H�[�J�X�̒����` */
	if( pDis->itemState & ODS_FOCUS ){
		rcFocus.top -= 3;
		rcFocus.left -= 3;
		rcFocus.right += 2;
		rcFocus.bottom += 2;
		::DrawFocusRect( gr, &rcFocus );
	}
}



//	From Here Sept. 10, 2000 JEPRO
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void CPropTypesColor::EnableTypesPropInput( HWND hwndDlg )
{
	//	From Here Jun. 6, 2001 genta
	//	�s�R�����g�J�n���ʒu���̓{�b�N�X��Enable/Disable�ݒ�
	//	1��
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_LCPOS ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum ), FALSE );
	}
	//	2��
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_LCPOS2 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS2 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS2 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum2 ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS2 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS2 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum2 ), FALSE );
	}
	//	3��
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_LCPOS3 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS3 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS3 ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum3 ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENTPOS3 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LCPOS3 ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_SPIN_LCColNum3 ), FALSE );
	}
	//	To Here Jun. 6, 2001 genta
}
//	To Here Sept. 10, 2000



/*!	@brief �L�[���[�h�Z�b�g�̍Ĕz��

	�L�[���[�h�Z�b�g�̐F�����ł͖��w��̃L�[���[�h�Z�b�g�ȍ~�̓`�F�b�N���ȗ�����D
	���̂��߃Z�b�g�̓r���ɖ��w��̂��̂�����ꍇ�͂���ȍ~��O�ɋl�߂邱�Ƃ�
	�w�肳�ꂽ�S�ẴL�[���[�h�Z�b�g���L���ɂȂ�悤�ɂ���D
	���̍ہC�F�����̐ݒ�������Ɉړ�����D

	m_nSet, m_Types.m_ColorInfoArr[]���ύX�����D

	@param hwndDlg [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��

	@author	genta 
	@date	2005.01.23 genta new

*/
void CPropTypesColor::RearrangeKeywordSet( HWND hwndDlg )
{
	int i, j;
	for( i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		if( m_nSet[ i ] != -1 )
			continue;

		//	���ݒ�̏ꍇ
		for( j = i; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			if( m_nSet[ j ] != -1 ){
				//	���ɐݒ�ςݍ��ڂ��������ꍇ
				m_nSet[ i ] = m_nSet[ j ];
				m_nSet[ j ] = -1;

				//	�F�ݒ�����ւ���
				//	�\���̂��Ɠ���ւ���Ɩ��O���ς���Ă��܂��̂Œ���
				ColorInfo colT;
				ColorInfo &col1 = m_Types.m_ColorInfoArr[ COLORIDX_KEYWORD1 + i ];
				ColorInfo &col2   = m_Types.m_ColorInfoArr[ COLORIDX_KEYWORD1 + j ];

				colT.m_bDisp		= col1.m_bDisp;
				colT.m_bBoldFont	= col1.m_bBoldFont;
				colT.m_bUnderLine	= col1.m_bUnderLine;
				colT.m_colTEXT		= col1.m_colTEXT;
				colT.m_colBACK		= col1.m_colBACK;

				col1.m_bDisp		= col2.m_bDisp;
				col1.m_bBoldFont	= col2.m_bBoldFont;
				col1.m_bUnderLine	= col2.m_bUnderLine;
				col1.m_colTEXT		= col2.m_colTEXT;
				col1.m_colBACK		= col2.m_colBACK;

				col2.m_bDisp		= colT.m_bDisp;
				col2.m_bBoldFont	= colT.m_bBoldFont;
				col2.m_bUnderLine	= colT.m_bUnderLine;
				col2.m_colTEXT		= colT.m_colTEXT;
				col2.m_colBACK		= colT.m_colBACK;
				
				break;
			}
		}
		if( j == MAX_KEYWORDSET_PER_TYPE ){
			//	���ɂ͐ݒ�ςݍ��ڂ��Ȃ�����
			break;
		}
	}
	
	//	���X�g�{�b�N�X�y�ѐF�ݒ�{�^�����ĕ`��
	::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
	::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
	::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_LIST_COLORS ), NULL, TRUE );
}



/* �F��ʃ��X�g �I�[�i�[�`�� */
void CPropTypesColor::DrawColorListItem( DRAWITEMSTRUCT* pDis )
{
	ColorInfo*	pColorInfo;
//	RECT		rc0,rc1,rc2;
	RECT		rc1;
	COLORREF	cRim = (COLORREF)::GetSysColor( COLOR_3DSHADOW );

	if( pDis == NULL || pDis->itemData == 0 ) return;

	//�`��Ώ�
	CGraphics gr(pDis->hDC);

//	rc0 = pDis->rcItem;
	rc1 = pDis->rcItem;
//	rc2 = pDis->rcItem;

	/* �A�C�e���f�[�^�̎擾 */
	pColorInfo = (ColorInfo*)pDis->itemData;

	/* �A�C�e����`�h��Ԃ� */
	gr.SetBrushColor( ::GetSysColor( COLOR_WINDOW ) );
	gr.FillMyRect( pDis->rcItem );
	
	/* �A�C�e�����I������Ă��� */
	if( pDis->itemState & ODS_SELECTED ){
		gr.SetBrushColor( ::GetSysColor( COLOR_HIGHLIGHT ) );
		gr.SetTextForeColor( ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );
	}else{
		gr.SetBrushColor( ::GetSysColor( COLOR_WINDOW ) );
		gr.SetTextForeColor( ::GetSysColor( COLOR_WINDOWTEXT ) );
	}

	rc1.left+= (2 + 16);
	rc1.top += 2;
	rc1.right -= ( 2 + 27 );
	rc1.bottom -= 2;
	/* �I���n�C���C�g��` */
	gr.FillMyRect(rc1);
	/* �e�L�X�g */
	::SetBkMode( gr, TRANSPARENT );
	::TextOut( gr, rc1.left, rc1.top, pColorInfo->m_szName, _tcslen( pColorInfo->m_szName ) );
	if( pColorInfo->m_bBoldFont ){	/* ������ */
		::TextOut( gr, rc1.left + 1, rc1.top, pColorInfo->m_szName, _tcslen( pColorInfo->m_szName ) );
	}
	if( pColorInfo->m_bUnderLine ){	/* ������ */
		SIZE	sz;
		::GetTextExtentPoint32( gr, pColorInfo->m_szName, _tcslen( pColorInfo->m_szName ), &sz );
		::MoveToEx( gr, rc1.left,		rc1.bottom - 2, NULL );
		::LineTo( gr, rc1.left + sz.cx,	rc1.bottom - 2 );
		::MoveToEx( gr, rc1.left,		rc1.bottom - 1, NULL );
		::LineTo( gr, rc1.left + sz.cx,	rc1.bottom - 1 );
	}

	/* �A�C�e���Ƀt�H�[�J�X������ */	// 2006.05.01 ryoji �`������̕s�����C��
	if( pDis->itemState & ODS_FOCUS ){
		::DrawFocusRect( gr, &pDis->rcItem );
	}

	/* �u�F����/�\������v�̃`�F�b�N */
	rc1 = pDis->rcItem;
	rc1.left += 2;
	rc1.top += 3;
	rc1.right = rc1.left + 12;
	rc1.bottom = rc1.top + 12;
	if( pColorInfo->m_bDisp ){	/* �F����/�\������ */
		// 2006.04.26 ryoji �e�L�X�g�F���g���i�u�n�C�R���g���X�g���v�̂悤�Ȑݒ�ł�������悤�Ɂj
		gr.SetPen( ::GetSysColor( COLOR_WINDOWTEXT ) );

		::MoveToEx( gr,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( gr,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( gr,	rc1.right - 2, rc1.top + 4 );
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx( gr,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( gr,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( gr,	rc1.right - 2, rc1.top + 4 );
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx( gr,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( gr,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( gr,	rc1.right - 2, rc1.top + 4 );
	}
//	return;


	// 2002/11/02 Moca ��r���@�ύX
//	if( 0 != strcmp( "�J�[�\���s�A���_�[���C��", pColorInfo->m_szName ) )
	if ( 0 == (g_ColorAttributeArr[pColorInfo->m_nColorIdx].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
	{
		/* �w�i�F ���{��` */
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 13;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;

		gr.SetBrushColor( pColorInfo->m_colBACK );
		gr.SetPen( cRim );
		::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );
	}


	if( 0 == (g_ColorAttributeArr[pColorInfo->m_nColorIdx].fAttribute & COLOR_ATTRIB_NO_TEXT) )
	{
		/* �O�i�F ���{��` */
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 27;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;
		gr.SetBrushColor( pColorInfo->m_colTEXT );
		gr.SetPen( cRim );
		::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );
	}
}



/* �F�I���_�C�A���O */
BOOL CPropTypesColor::SelectColor( HWND hwndParent, COLORREF* pColor, DWORD* pCustColors )
{
	CHOOSECOLOR		cc;
	cc.lStructSize = sizeof_raw( cc );
	cc.hwndOwner = hwndParent;
	cc.hInstance = NULL;
	cc.rgbResult = *pColor;
	cc.lpCustColors = pCustColors;
	cc.Flags = /*CC_PREVENTFULLOPEN |*/ CC_RGBINIT;
	cc.lCustData = 0;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;
	if( !::ChooseColor( &cc ) ){
		return FALSE;
	}
	*pColor = cc.rgbResult;
	return TRUE;
}

