//	$Id$
/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�S�ʁv�y�[�W

	@author Norio Nakatani
	@date 1998/12/24 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, genta, MIK, hor, Stonee
	Copyright (C) 2002, YAZAKI, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "sakura_rc.h"
#include "CPropCommon.h"
#include "debug.h"
#include <windows.h>
//#include <shlwapi.h>
//#include <stdio.h>
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "etc_uty.h"
#include "global.h"
#include "CDlgInput1.h"
#include "CDlgDebug.h"
#include "CSplitBoxWnd.h"
#include "CMenuDrawer.h"
#include "funccode.h"	//Stonee, 2001/05/18
#include "CImageListMgr.h" // 2002/2/10 aroka

//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//10900
	IDC_BUTTON_CLEAR_MRU_FILE,		HIDC_BUTTON_CLEAR_MRU_FILE,			//�������N���A�i�t�@�C���j
	IDC_BUTTON_CLEAR_MRU_FOLDER,	HIDC_BUTTON_CLEAR_MRU_FOLDER,		//�������N���A�i�t�H���_�j
	IDC_CHECK_FREECARET,			HIDC_CHECK_FREECARET,				//�t���[�J�[�\��
	IDC_CHECK_INDENT,				HIDC_CHECK_INDENT,					//�����C���f���g
	IDC_CHECK_INDENT_WSPACE,		HIDC_CHECK_INDENT_WSPACE,			//�S�p�󔒂��C���f���g
	IDC_CHECK_USETRAYICON,			HIDC_CHECK_USETRAYICON,				//�^�X�N�g���C���g��
	IDC_CHECK_STAYTASKTRAY,			HIDC_CHECK_STAYTASKTRAY,			//�^�X�N�g���C�ɏ풓
	IDC_CHECK_REPEATEDSCROLLSMOOTH,	HIDC_CHECK_REPEATEDSCROLLSMOOTH,	//�������炩�ɂ���
	IDC_CHECK_EXITCONFIRM,			HIDC_CHECK_EXITCONFIRM,				//�I���̊m�F
	IDC_HOTKEY_TRAYMENU,			HIDC_HOTKEY_TRAYMENU,				//���N���b�N���j���[�̃V���[�g�J�b�g�L�[
	IDC_EDIT_REPEATEDSCROLLLINENUM,	HIDC_EDIT_REPEATEDSCROLLLINENUM,	//�X�N���[���s��
	IDC_EDIT_MAX_MRU_FILE,			HIDC_EDIT_MAX_MRU_FILE,				//�t�@�C�������̍ő吔
	IDC_EDIT_MAX_MRU_FOLDER,		HIDC_EDIT_MAX_MRU_FOLDER,			//�t�H���_�����̍ő吔
	IDC_RADIO_CARETTYPE0,			HIDC_RADIO_CARETTYPE0,				//�J�[�\���`��iWindows���j
	IDC_RADIO_CARETTYPE1,			HIDC_RADIO_CARETTYPE1,				//�J�[�\���`��iMS-DOS���j
	IDC_SPIN_REPEATEDSCROLLLINENUM,	HIDC_EDIT_REPEATEDSCROLLLINENUM,
	IDC_SPIN_MAX_MRU_FILE,			HIDC_EDIT_MAX_MRU_FILE,
	IDC_SPIN_MAX_MRU_FOLDER,		HIDC_EDIT_MAX_MRU_FOLDER,
//	IDC_STATIC,						-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//10900
	IDC_BUTTON_CLEAR_MRU_FILE,		10900,	//�������N���A�i�t�@�C���j
	IDC_BUTTON_CLEAR_MRU_FOLDER,	10901,	//�������N���A�i�t�H���_�j
	IDC_CHECK_FREECARET,			10910,	//�t���[�J�[�\��
	IDC_CHECK_INDENT,				10911,	//�����C���f���g
	IDC_CHECK_INDENT_WSPACE,		10912,	//�S�p�󔒂��C���f���g
	IDC_CHECK_USETRAYICON,			10913,	//�^�X�N�g���C���g��
	IDC_CHECK_STAYTASKTRAY,			10914,	//�^�X�N�g���C�ɏ풓
	IDC_CHECK_REPEATEDSCROLLSMOOTH,	10915,	//�������炩�ɂ���
	IDC_CHECK_EXITCONFIRM,			10916,	//�I���̊m�F
	IDC_HOTKEY_TRAYMENU,			10940,	//���N���b�N���j���[�̃V���[�g�J�b�g�L�[
	IDC_EDIT_REPEATEDSCROLLLINENUM,	10941,	//�X�N���[���s��
	IDC_EDIT_MAX_MRU_FILE,			10942,	//�t�@�C�������̍ő吔
	IDC_EDIT_MAX_MRU_FOLDER,		10943,	//�t�H���_�����̍ő吔
	IDC_RADIO_CARETTYPE0,			10960,	//�J�[�\���`��iWindows���j
	IDC_RADIO_CARETTYPE1,			10961,	//�J�[�\���`��iMS-DOS���j
	IDC_SPIN_REPEATEDSCROLLLINENUM,	-1,
	IDC_SPIN_MAX_MRU_FILE,			-1,
	IDC_SPIN_MAX_MRU_FOLDER,		-1,
//	IDC_STATIC,						-1,
	0, 0
};
#endif
//@@@ 2001.02.04 End






int	CPropCommon::SearchIntArr( int nKey, int* pnArr, int nArrNum )
{
	int i;
	for( i = 0; i < nArrNum; ++i ){
		if( nKey == pnArr[i] ){
			return i;
		}
	}
	return -1;
}

//	From Here Jun. 2, 2001 genta
//	Dialog procedure�̏��������ʉ����A�e�y�[�W��Dialog Procedure�ł�
//	�^�̏������\�b�h���w�肵�ċ��ʊ֐����ĂԂ����ɂ����D
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
BOOL CALLBACK CPropCommon::DlgProc_PROP_GENERAL(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_p1, hwndDlg, uMsg, wParam, lParam );
}

/*!
	�v���p�e�B�y�[�W���Ƃ�Window Procedure�������Ɏ�邱�Ƃ�
	�����̋��ʉ���_�����D

	@param DispatchPage �^��Window Procedure�̃����o�֐��|�C���^
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handlw
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
BOOL CPropCommon::DlgProc(
	BOOL (CPropCommon::*DispatchPage)( HWND, UINT, WPARAM, LPARAM ),
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam
)
{
	PROPSHEETPAGE*	pPsp;
	CPropCommon*	pCPropCommon;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropCommon = ( CPropCommon* )(pPsp->lParam);
		if( NULL != pCPropCommon ){
			return (pCPropCommon->*DispatchPage)( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		pCPropCommon = ( CPropCommon* )::GetWindowLong( hwndDlg, DWL_USER );
		if( NULL != pCPropCommon ){
			return (pCPropCommon->*DispatchPage)( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}
//	To Here Jun. 2, 2001 genta


CPropCommon::CPropCommon()
{
//	int		i;
//	long	lPathLen;

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
//	m_cShareData.Init();
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_hInstance = NULL;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = NULL;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_hwndThis  = NULL;		/* ���̃_�C�A���O�̃n���h�� */
	m_nPageNum = 0;

	/* �w���v�t�@�C���̃t���p�X��Ԃ� */
	::GetHelpFilePath( m_szHelpFile );

	return;
}





CPropCommon::~CPropCommon()
{
}





/* ������ */
//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
void CPropCommon::Create( HINSTANCE hInstApp, HWND hwndParent, CImageListMgr* cIcons, CSMacroMgr* pMacro, CMenuDrawer* pMenuDrawer )
{
	m_hInstance = hInstApp;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_pcIcons = cIcons;
	m_pcSMacro = pMacro;
	m_cLookup.Init( m_hInstance, m_pcSMacro, &m_Common );	//	�@�\���E�ԍ�resolve�N���X�D
//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	m_pcMenuDrawer = pMenuDrawer;

	return;
}





/* �F�I���_�C�A���O */
BOOL CPropCommon::SelectColor( HWND hwndParent, COLORREF* pColor )
{
	int			i;
	CHOOSECOLOR	cc;
	DWORD	dwCustColors[16] ;
	for( i = 0; i < 16; i++ ){
		dwCustColors[i] = (DWORD)RGB( 255, 255, 255 );
	}
	cc.lStructSize = sizeof( cc );
	cc.hwndOwner = hwndParent;
	cc.hInstance = NULL;
	cc.rgbResult = *pColor;
	cc.lpCustColors = (LPDWORD) dwCustColors;
	cc.Flags = /*CC_PREVENTFULLOPEN |*/ CC_RGBINIT;
	cc.lCustData = NULL;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;
	if( FALSE == ::ChooseColor( &cc ) ){
		return FALSE;
	}
	*pColor = cc.rgbResult;
	return TRUE;
}





/* �F�{�^���̕`�� */
void CPropCommon::DrawColorButton( DRAWITEMSTRUCT* pDis, COLORREF cColor )
{
#ifdef _DEBUG
	MYTRACE( "pDis->itemAction = " );
#endif
	COLORREF	cBtnHiLight		= (COLORREF)::GetSysColor(COLOR_3DHILIGHT);
	COLORREF	cBtnShadow		= (COLORREF)::GetSysColor(COLOR_3DSHADOW);
	COLORREF	cBtnDkShadow	= (COLORREF)::GetSysColor(COLOR_3DDKSHADOW);
	COLORREF	cBtnFace		= (COLORREF)::GetSysColor(COLOR_3DFACE);
	COLORREF	cRim;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	HPEN		hPen;
	HPEN		hPenOld;
	RECT		rc;
	RECT		rcFocus;

	/* �{�^���̕\�ʂ̐F�œh��Ԃ� */
	hBrush = ::CreateSolidBrush( cBtnFace );
	::FillRect( pDis->hDC, &(pDis->rcItem), hBrush );
	::DeleteObject( hBrush );

	/* �g�̕`�� */
	rcFocus = rc = pDis->rcItem;
	rc.top += 4;
	rc.left += 4;
	rc.right -= 4;
	rc.bottom -= 4;
	rcFocus = rc;
//	rc.right -= 11;

	if( pDis->itemState & ODS_SELECTED ){
		hPen = ::CreatePen( PS_SOLID, 0, cBtnDkShadow );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 0, pDis->rcItem.bottom - 2, NULL );
		::LineTo( pDis->hDC, 0, 0 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, 0 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );

		hPen = ::CreatePen( PS_SOLID, 0, cBtnShadow );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 1, pDis->rcItem.bottom - 3, NULL );
		::LineTo( pDis->hDC, 1, 1 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 2, 1 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );

		hPen = ::CreatePen( PS_SOLID, 0, cBtnHiLight );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 0, pDis->rcItem.bottom - 1, NULL );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, -1 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );

		rc.top += 1;
		rc.left += 1;
		rc.right += 1;
		rc.bottom += 1;

		rcFocus.top += 1;
		rcFocus.left += 1;
		rcFocus.right += 1;
		rcFocus.bottom += 1;

	}else{
		hPen = ::CreatePen( PS_SOLID, 0, cBtnHiLight );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 0, pDis->rcItem.bottom - 2, NULL );
		::LineTo( pDis->hDC, 0, 0 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, 0 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );

		hPen = ::CreatePen( PS_SOLID, 0, cBtnShadow );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 1, pDis->rcItem.bottom - 2, NULL );
		::LineTo( pDis->hDC, pDis->rcItem.right - 2, pDis->rcItem.bottom - 2 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 2, 0 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );

		hPen = ::CreatePen( PS_SOLID, 0, cBtnDkShadow );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::MoveToEx( pDis->hDC, 0, pDis->rcItem.bottom - 1, NULL );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1 );
		::LineTo( pDis->hDC, pDis->rcItem.right - 1, -1 );
		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );
	}
	/* �w��F�œh��Ԃ� */
	hBrush = ::CreateSolidBrush( cColor );
	hBrushOld = (HBRUSH)::SelectObject( pDis->hDC, hBrush );
	cRim = cBtnShadow;
	hPen = ::CreatePen( PS_SOLID, 0, cRim );
	hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
	::RoundRect( pDis->hDC, rc.left, rc.top, rc.right, rc.bottom , 5, 5 );
	::SelectObject( pDis->hDC, hPenOld );
	::SelectObject( pDis->hDC, hBrushOld );
	::DeleteObject( hPen );
	::DeleteObject( hBrush );


//	/* ��؂�c�_ */
//	hPen = ::CreatePen( PS_SOLID, 0, cBtnShadow );
//	hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
//	::MoveToEx( pDis->hDC, rc.right + 3, rc.top, NULL );
//	::LineTo( pDis->hDC, rc.right + 3, rc.bottom );
//	::SelectObject( pDis->hDC, hPenOld );
//	::DeleteObject( hPen );
//
//	hPen = ::CreatePen( PS_SOLID, 0, cBtnHiLight );
//	hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
//	::MoveToEx( pDis->hDC, rc.right + 4, rc.top, NULL );
//	::LineTo( pDis->hDC, rc.right + 4, rc.bottom );
//	::SelectObject( pDis->hDC, hPenOld );
//	::DeleteObject( hPen );
//
//	/* ���L�� */
//	hPen = ::CreatePen( PS_SOLID, 0, cBtnDkShadow );
//	hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
//	::MoveToEx( pDis->hDC, rc.right + 6		, rc.top + 6, NULL );
//	::LineTo(	pDis->hDC, rc.right + 6 + 5	, rc.top + 6 );
//	::MoveToEx( pDis->hDC, rc.right + 7		, rc.top + 7, NULL );
//	::LineTo(	pDis->hDC, rc.right + 7 + 3	, rc.top + 7 );
//	::MoveToEx( pDis->hDC, rc.right + 8		, rc.top + 8, NULL );
//	::LineTo(	pDis->hDC, rc.right + 8 + 1	, rc.top + 8 );
//	::SelectObject( pDis->hDC, hPenOld );
//	::DeleteObject( hPen );

	/* �t�H�[�J�X�̒����` */
	if( pDis->itemState & ODS_FOCUS ){
		rcFocus.top -= 3;
		rcFocus.left -= 3;
		rcFocus.right += 2;
		rcFocus.bottom += 2;
		::DrawFocusRect( pDis->hDC, &rcFocus );
	}
	return;
}






/* �c�[���o�[�{�^�����X�g�̃A�C�e���`�� */
void CPropCommon::DrawToolBarItemList( DRAWITEMSTRUCT* pDis )
{
	char		szLabel[256];
//	char		szFuncName[200];
	TBBUTTON	tbb;
	int			nLength;
	HBRUSH		hBrush;
	RECT		rc;
	RECT		rc0;
	RECT		rc1;
	RECT		rc2;
//	SIZE		sizDimension;
//	HDC			hdcCompatDC;
//	HBITMAP		m_hbmpCompatBMPOld;


	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
	::FillRect( pDis->hDC, &pDis->rcItem, hBrush );
	::DeleteObject( hBrush );

	rc  = pDis->rcItem;
	rc0 = pDis->rcItem;
	rc0.left += 18;//20 //Oct. 18, 2000 JEPRO �s�擪�̃A�C�R���Ƃ���ɑ����L���v�V�����Ƃ̊Ԃ������l�߂�(20��18)
	rc1 = rc0;
	rc2 = rc0;

	if( (int)pDis->itemID < 0 ){
	}else{

//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
//		tbb = m_cShareData.m_tbMyButton[pDis->itemData];
		tbb = m_pcMenuDrawer->m_tbMyButton[pDis->itemData];

		if( 0 != tbb.idCommand ){
			/* �r�b�g�}�b�v�̕\�� �D�F�𓧖��`�� */
			m_pcIcons->Draw( tbb.iBitmap, pDis->hDC, rc.left + 2, rc.top  + 2, ILD_NORMAL);
#if 0//////////////////////////////////
			/* �ĕ`��p�R���p�`�u���c�b */
			hdcCompatDC = ::CreateCompatibleDC( pDis->hDC );
			/* �r�b�g�}�b�v�T�C�Y�擾 */
			::GetBitmapDimensionEx( m_hbmpToolButtons, &sizDimension );
			/* �r�b�g�}�b�v�I�� */
			m_hbmpCompatBMPOld = (HBITMAP)::SelectObject( hdcCompatDC, m_hbmpToolButtons );

			/* �r�b�g�}�b�v�`�� */
			::BitBlt(
				pDis->hDC,		// handle to destination device context
				rc.left + 2,	// x-coordinate of destination rectangle's upper-left corner
				rc.top  + 2,	// x-coordinate of destination rectangle's upper-left corner
				16,	// width of destination rectangle
//	Sept. 17, 2000 JEPRO_16thdot �A�C�R����16dot�ڂ��\�������悤�Ɏ��s��ύX
//				15,				// height of destination rectangle
				16,				// height of destination rectangle
				hdcCompatDC,	// handle to source device context
				16 * tbb.iBitmap,// x-coordinate of source rectangle's upper-left corner
				0,				// y-coordinate of source rectangle's upper-left corner
				SRCCOPY 		// raster operation code
			);
//	Sept. 17,2000 JEPRO_16thdot �A�C�R����16dot�ڂ��\�������悤�Ɏ��s��ύX����K�v����H
//	�Ƃ肠�������̂܂܂Ŗ��Ȃ������Ȃ̂ŕύX���Ă��Ȃ�
//	Sept. 21, 2000 JEPRO_16thdot ��͂�O�̂��߂������ύX���Ă݂�
//			CSplitBoxWnd::Draw3dRect( pDis->hDC, rc.left + 0, rc.top  + 0, 16 + 4, 15 + 4,
//				::GetSysColor( COLOR_3DLIGHT ), ::GetSysColor( COLOR_3DDKSHADOW )
//			);
//			CSplitBoxWnd::Draw3dRect( pDis->hDC, rc.left + 1, rc.top  + 1, 16 + 2, 15 + 2,
//				::GetSysColor( COLOR_3DHILIGHT ), ::GetSysColor( COLOR_3DSHADOW )
//			);
			CSplitBoxWnd::Draw3dRect( pDis->hDC, rc.left + 0, rc.top  + 0, 16 + 4, 16 + 4,
				::GetSysColor( COLOR_3DLIGHT ), ::GetSysColor( COLOR_3DDKSHADOW )
			);
			CSplitBoxWnd::Draw3dRect( pDis->hDC, rc.left + 1, rc.top  + 1, 16 + 2, 16 + 2,
				::GetSysColor( COLOR_3DHILIGHT ), ::GetSysColor( COLOR_3DSHADOW )
			);

			::SelectObject( hdcCompatDC, m_hbmpCompatBMPOld );
			::DeleteDC( hdcCompatDC );
#endif //////////////////////////////////
		}

		if( 0 == tbb.idCommand ){
//			nLength = strlen( strcpy( szFuncName, "�Z�p���[�^"	) );
//			nLength = strlen( strcpy( szFuncName, "---------------------" ) );
			nLength = strlen( strcpy( szLabel, "����������������������" ) );	//Oct. 18, 2000 JEPRO �u�c�[���o�[�v�^�u�Ŏg���Ă���Z�p���[�^
		//	From Here Oct. 15, 2001 genta
		}else if( !m_cLookup.Funccode2Name( tbb.idCommand, szLabel, sizeof( szLabel ) )){
			wsprintf( szLabel, "%s", "-- UNKNOWN --" );
		}
		//	To Here Oct. 15, 2001 genta

		/* �A�C�e�����I������Ă��� */
		if( pDis->itemState & ODS_SELECTED ){
			hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_HIGHLIGHT ) );
			::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );
		}else{
			hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
			::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_WINDOWTEXT ) );
		}
		rc1.left++;
		rc1.top++;
		rc1.right--;
		rc1.bottom--;
		::FillRect( pDis->hDC, &rc1, hBrush );
		::DeleteObject( hBrush );

		::SetBkMode( pDis->hDC, TRANSPARENT );
		::TextOut( pDis->hDC, rc1.left + 4, rc1.top + 2, szLabel, strlen( szLabel ) );

	}

	/* �A�C�e���Ƀt�H�[�J�X������ */
	if( pDis->itemState & ODS_FOCUS ){
		::DrawFocusRect( pDis->hDC, &rc2 );
	}
	return;
}

//	From Here Jun. 2, 2001 genta
/*!
	�u���ʐݒ�v�v���p�e�B�V�[�g�̍쐬���ɕK�v�ȏ���
	�ێ�����\����
*/
struct ComPropSheetInfo {
	const char* szTabname;	//!< TAB�̕\����
	unsigned int resId;	//!< Property sheet�ɑΉ�����Dialog resource
	BOOL (CALLBACK *DProc)(HWND, UINT, WPARAM, LPARAM);
		//!<  Dialog Procedure
};
//	To Here Jun. 2, 2001 genta

//	�L�[���[�h�F���ʐݒ�^�u����(�v���p�e�B�V�[�g)
/* �v���p�e�B�V�[�g�̍쐬 */
int CPropCommon::DoPropertySheet( int nPageNum/*, int nActiveItem*/ )
{
//	m_nActiveItem = nActiveItem;

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
//	m_cShareData.Init();
	m_pShareData = CShareData::getInstance()->GetShareData();

	int				nRet;
	PROPSHEETPAGE	psp[32];
	PROPSHEETHEADER	psh;
	int				nIdx;
	int				i;

//	m_Common.m_nMAXLINELEN_org = m_Common.m_nMAXLINELEN;

	//	From Here Jun. 2, 2001 genta
	//!	�u���ʐݒ�v�v���p�e�B�V�[�g�̍쐬���ɕK�v�ȏ��̔z��D
	static ComPropSheetInfo ComPropSheetInfoList[] = {
		{ "�S��", 			IDD_PROP1P1,		DlgProc_PROP_GENERAL },
		{ "�E�B���h�E",		IDD_PROP_WIN,		DlgProc_PROP_WIN },
		{ "�ҏW",			IDD_PROP_EDIT,		DlgProc_PROP_EDIT },
		{ "�t�@�C��",		IDD_PROP_FILE,		DlgProc_PROP_FILE },
		{ "�o�b�N�A�b�v",	IDD_PROP_BACKUP,	DlgProc_PROP_BACKUP },
		{ "����",			IDD_PROP_FORMAT,	DlgProc_PROP_FORMAT },
		{ "�N���b�J�u��URL",IDD_PROP_URL,		DlgProc_PROP_URL },
		{ "Grep",			IDD_PROP_GREP,		DlgProc_PROP_GREP },
		{ "�L�[���蓖��",	IDD_PROP_KEYBIND,	DlgProc_PROP_KEYBIND },
		{ "�J�X�^�����j���[",IDD_PROP_CUSTMENU,	DlgProc_PROP_CUSTMENU },
		{ "�c�[���o�[",		IDD_PROP_TOOLBAR,	DlgProc_PROP_TOOLBAR },
		{ "�����L�[���[�h",	IDD_PROP_KEYWORD,	DlgProc_PROP_KEYWORD },
		{ "�x��",			IDD_PROP_HELPER,	DlgProc_PROP_HELPER },
		{ "�}�N��",			IDD_PROP_MACRO,		DlgProc_PROP_MACRO },
	};

	for( nIdx = 0, i = 0; i < sizeof(ComPropSheetInfoList)/sizeof(ComPropSheetInfoList[0])
			&& nIdx < 32 ; i++ ){
		if( ComPropSheetInfoList[i].szTabname != NULL ){
			PROPSHEETPAGE *p = &psp[nIdx];
			memset( p, 0, sizeof( PROPSHEETPAGE ) );
			p->dwSize = sizeof( PROPSHEETPAGE );
			p->dwFlags = /*PSP_USEICONID |*/ PSP_USETITLE | PSP_HASHELP;
			p->hInstance = m_hInstance;
			p->pszTemplate = MAKEINTRESOURCE( ComPropSheetInfoList[i].resId );
			p->pszIcon = NULL/*MAKEINTRESOURCE( IDI_FONT )*/;
			p->pfnDlgProc = (DLGPROC)(ComPropSheetInfoList[i].DProc);
			p->pszTitle = ComPropSheetInfoList[i].szTabname;
			p->lParam = (LPARAM)this;
			p->pfnCallback = NULL;
			nIdx++;
		}
	}
	//	To Here Jun. 2, 2001 genta

	memset( &psh, 0, sizeof( PROPSHEETHEADER ) );
	psh.dwSize = sizeof( PROPSHEETHEADER );
//	JEPROtest Sept. 30, 2000 ���ʐݒ�̉B��[�K�p]�{�^���̐��̂͂����B�s���̃R�����g�A�E�g�����ւ��Ă݂�΂킩��
//	psh.dwFlags = /*PSH_USEICONID |*/ /*PSH_NOAPPLYNOW |*/ PSH_PROPSHEETPAGE/* | PSH_HASHELP*/;
	psh.dwFlags = /*PSH_USEICONID |*/ PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE/* | PSH_HASHELP*/;
	psh.hwndParent = m_hwndParent;
	psh.hInstance = m_hInstance;
	psh.pszIcon = NULL /*MAKEINTRESOURCE( IDI_CELL_PROPERTIES )*/;
	psh.pszCaption = (LPSTR) "���ʐݒ�";
	psh.nPages = nIdx;

	//- 20020106 aroka # psh.nStartPage �� unsigned �Ȃ̂ŕ��ɂȂ�Ȃ�
	if( -1 == nPageNum ){
		psh.nStartPage = m_nPageNum;
	}else
	if( 0 > nPageNum ){			//- 20020106 aroka
		psh.nStartPage = 0;
	}else{
		psh.nStartPage = nPageNum;
	}
//	if( 0 > psh.nStartPage ){	//- 20020106 aroka
//		psh.nStartPage = 0;
//	}
	if( psh.nPages - 1 < psh.nStartPage ){
		psh.nStartPage = psh.nPages - 1;
	}

	psh.ppsp = (LPCPROPSHEETPAGE)psp;
	psh.pfnCallback = NULL;

	nRet = ::PropertySheet( &psh );
	if( -1 == nRet ){
		char*	pszMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			::GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// �f�t�H���g����
			(LPTSTR) &pszMsgBuf,
			0,
			NULL
		);
		::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "��҂ɋ����ė~�����G���[",
			"CPropCommon::DoPropertySheet()���ŃG���[���o�܂����B\npsh.nStartPage=[%d]\n::PropertySheet()���s\n\n%s\n", psh.nStartPage, pszMsgBuf
		);
		::LocalFree( pszMsgBuf );
	}
//	{
//		CDlgDebug	cDlgDebug;
//		CMemory		cmemDebugInfo;
//		char		szText[1024];
//
//		sprintf( szText, "aaaaaaaaa\r\nbbbbbbbbbbb\r\nccccccccccc\r\n" );
//		cmemDebugInfo.Append( szText, strlen( szText ) );
//		cDlgDebug.DoModal( m_hInstance, m_hwndParent, cmemDebugInfo );
//	}

	return nRet;
}





/* p1 ���b�Z�[�W���� */
BOOL CPropCommon::DispatchEvent_p1(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
//	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;
//	LPDRAWITEMSTRUCT pDis;

	switch( uMsg ){

	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p1 */
		SetData_p1( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
//		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
//	/* �^�X�N�g���C���g�� */
//	m_Common.m_bUseTaskTray = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USETRAYICON );
//	/* �^�X�N�g���C�ɏ풓 */
//	m_Common.m_bStayTaskTray = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_STAYTASKTRAY );

			case IDC_CHECK_USETRAYICON:	/* �^�X�N�g���C���g�� */
			// From Here 2001.12.03 hor
			//		���삵�ɂ������ĕ]���������̂Ń^�X�N�g���C�֌W��Enable�������߂܂���
			//@@@ YAZAKI 2001.12.31 IDC_CHECKSTAYTASKTRAY�̃A�N�e�B�u�A��A�N�e�B�u�̂ݐ���B
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USETRAYICON ) ){
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), TRUE );
				}else{
			//		::CheckDlgButton( hwndDlg, IDC_CHECK_STAYTASKTRAY, FALSE );	/* �^�X�N�g���C�ɏ풓 */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), FALSE );
				}
			//	if(!::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USETRAYICON ) ){
			//		::CheckDlgButton( hwndDlg, IDC_CHECK_STAYTASKTRAY, FALSE );	/* �^�X�N�g���C�ɏ풓 */
			//	}
			// To Here 2001.12.03 hor
				return TRUE;

			case IDC_CHECK_STAYTASKTRAY:	/* �^�X�N�g���C�ɏ풓 */
			//@@@ YAZAKI 2001.12.31 ���䂵�Ȃ��B
			//	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_STAYTASKTRAY ) ){
			//		::CheckDlgButton( hwndDlg, IDC_CHECK_USETRAYICON, TRUE );	/* �^�X�N�g���C���g�� */
			//	}else{
			//	}
				return TRUE;

			case IDC_CHECK_INDENT:	/* �I�[�g�C���f���g */
//				MYTRACE( "IDC_CHECK_INDENT\n" );
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_INDENT ) ){
					/* ���{��󔒂��C���f���g */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_INDENT_WSPACE ), TRUE );
				}else{
					/* ���{��󔒂��C���f���g */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_INDENT_WSPACE ), FALSE );
				}
				return TRUE;

			case IDC_BUTTON_CLEAR_MRU_FILE:
				/* �t�@�C���̗������N���A */
				if( IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
					"�ŋߎg�����t�@�C���̗������폜���܂��B\n��낵���ł����H\n" ) ){
					return TRUE;
				}
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
//				m_pShareData->m_nMRUArrNum = 0;
				{
					CMRU cMRU;
					cMRU.ClearAll();
				}
				::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
					"�ŋߎg�����t�@�C���̗������폜���܂����B\n"
				);
				return TRUE;
			case IDC_BUTTON_CLEAR_MRU_FOLDER:
				/* �t�H���_�̗������N���A */
				if( IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
					"�ŋߎg�����t�H���_�̗������폜���܂��B\n��낵���ł����H\n" ) ){
					return TRUE;
				}
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
//				m_pShareData->m_nOPENFOLDERArrNum = 0;
				{
					CMRUFolder cMRUFolder;	//	MRU���X�g�̏������B���x�������Ɩ�肠��H
					cMRUFolder.ClearAll();
				}
				::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
					"�ŋߎg�����t�H���_�̗������폜���܂����B\n"
				);
				return TRUE;

			}
		}
		break;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
//		case IDC_SPIN_MAXLINELEN:
//			/* �܂�Ԃ������� */
//			MYTRACE( "IDC_SPIN_MAXLINELEN\n" );
//			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE );
//			if( pMNUD->iDelta < 0 ){
//				++nVal;
//			}else
//			if( pMNUD->iDelta > 0 ){
//				--nVal;
//			}
//			if( nVal < 10 ){
//				nVal = 10;
//			}
//			if( nVal > 10240 ){
//				nVal = 10240;
//			}
//			::SetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, nVal, FALSE );
//			return TRUE;
//		case IDC_SPIN_CHARSPACE:
//			/* �����̌��� */
//			MYTRACE( "IDC_SPIN_CHARSPACE\n" );
//			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE );
//			if( pMNUD->iDelta < 0 ){
//				++nVal;
//			}else
//			if( pMNUD->iDelta > 0 ){
//				--nVal;
//			}
//			if( nVal < 0 ){
//				nVal = 0;
//			}
//			if( nVal > 16 ){
//				nVal = 16;
//			}
//			::SetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, nVal, FALSE );
//			return TRUE;
//		case IDC_SPIN_LINESPACE:
//			/* �s�̌��� */
//			MYTRACE( "IDC_SPIN_LINESPACE\n" );
//			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE );
//			if( pMNUD->iDelta < 0 ){
//				++nVal;
//			}else
//			if( pMNUD->iDelta > 0 ){
//				--nVal;
//			}
//			if( nVal < 0 ){
//				nVal = 0;
//			}
//			if( nVal > 16 ){
//				nVal = 16;
//			}
//			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, nVal, FALSE );
//			return TRUE;
		case IDC_SPIN_REPEATEDSCROLLLINENUM:
			/* �L�[���s�[�g���̃X�N���[���s�� */
//			MYTRACE( "IDC_SPIN_REPEATEDSCROLLLINENUM\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 10 ){
				nVal = 10;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_MAX_MRU_FILE:
			/* �t�@�C���̗���MAX */
//			MYTRACE( "IDC_SPIN_MAX_MRU_FILE\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > MAX_MRU ){
				nVal = MAX_MRU;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_MAX_MRU_FOLDER:
			/* �t�H���_�̗���MAX */
//			MYTRACE( "IDC_SPIN_MAX_MRU_FOLDER\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > MAX_OPENFOLDER ){
				nVal = MAX_OPENFOLDER;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, nVal, FALSE );
			return TRUE;
		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP1P1 );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( "p1 PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 p1 */
				GetData_p1( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_ZENPAN;	//Oct. 25, 2000 JEPRO ZENPAN1��ZENPAN �ɕύX(�Q�Ƃ��Ă���̂�CPropCommon.cpp�݂̂�1�ӏ�)
				return TRUE;
			}
			break;
		}

//		MYTRACE( "pNMHDR->hwndFrom=%xh\n", pNMHDR->hwndFrom );
//		MYTRACE( "pNMHDR->idFrom  =%xh\n", pNMHDR->idFrom );
//		MYTRACE( "pNMHDR->code    =%xh\n", pNMHDR->code );
//		MYTRACE( "pMNUD->iPos    =%d\n", pMNUD->iPos );
//		MYTRACE( "pMNUD->iDelta  =%d\n", pMNUD->iDelta );
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)p_helpids );
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}





/* �_�C�A���O�f�[�^�̐ݒ� p1 */
void CPropCommon::SetData_p1( HWND hwndDlg )
{
	BOOL	bRet;
//	static	int	nTabArr[] = { 2, 4, 8 };							//Nov. 3, 2000 JEPRO ����͎E���Y��ł��傤�B(TAB�ݒ�̓^�C�v�ʐݒ�Ɉړ�����Ă���)
//	static	int	nTabArrNum = sizeof(nTabArr) / sizeof(nTabArr[0]);	//Nov. 3, 2000 JEPRO ������E���Y��ł��傤�B(TAB�ݒ�̓^�C�v�ʐݒ�Ɉړ�����Ă���)
//	int		i, j;
//	char	szWork[32];


//	/* �܂�Ԃ������� */
//	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, m_Common.m_nMAXLINELEN, FALSE );

//	/* �����̌��� */
//	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, m_Common.m_nCHARSPACE, FALSE );

//	/* �s�̌��� */
//	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, m_Common.m_nLINESPACE, FALSE );


	/* �J�[�\���̃^�C�v 0=win 1=dos  */
	if( 0 == m_Common.GetCaretType() ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE0, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE1, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_CARETTYPE1, TRUE );
	}


	/* �t���[�J�[�\�����[�h */
	::CheckDlgButton( hwndDlg, IDC_CHECK_FREECARET, m_Common.m_bIsFreeCursorMode );

	/* �C���f���g */
	::CheckDlgButton( hwndDlg, IDC_CHECK_INDENT, m_Common.m_bAutoIndent );

	/* ���{��󔒂��C���f���g */
	::CheckDlgButton( hwndDlg, IDC_CHECK_INDENT_WSPACE, m_Common.m_bAutoIndent_ZENSPACE );

	if( !m_Common.m_bAutoIndent ){
		/* ���{��󔒂��C���f���g */
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_INDENT_WSPACE ), FALSE );
	}

//	/* TAB�� */
//	j = 0;
//	for( i = 0; i < nTabArrNum; ++i ){
//		sprintf( szWork, "%d", nTabArr[i] );
//		::SendDlgItemMessage( hwndDlg, IDC_COMBO_TABSPACE, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)szWork );
///		if( m_Common.m_nTABSPACE == nTabArr[i] ){
//			j = i;
//		}
//	}
//	::SendDlgItemMessage( hwndDlg, IDC_COMBO_TABSPACE, CB_SETCURSEL, (WPARAM)j, 0 );


	/* �I�����̊m�F������ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_EXITCONFIRM, m_Common.m_bExitConfirm );

	/* �L�[���s�[�g���̃X�N���[���s�� */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, m_Common.m_nRepeatedScrollLineNum, FALSE );

	/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_REPEATEDSCROLLSMOOTH, m_Common.m_nRepeatedScroll_Smooth );


//	/* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
//	::CheckDlgButton( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY, m_Common.m_bAddCRLFWhenCopy );


//	/* Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v */
//	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_RETURN, m_Common.m_bGTJW_RETURN );
//
//	/* Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v  */
//	::CheckDlgButton( hwndDlg, IDC_CHECK_GTJW_LDBLCLK, m_Common.m_bGTJW_LDBLCLK );

	/* �t�@�C���̗���MAX */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, m_Common.m_nMRUArrNum_MAX, FALSE );

	/* �t�H���_�̗���MAX */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, m_Common.m_nOPENFOLDERArrNum_MAX, FALSE );



//	/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
//	::CheckDlgButton( hwndDlg, IDC_CHECK_DispTOOLBAR, m_Common.m_bDispTOOLBAR );
//
//	/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
//	::CheckDlgButton( hwndDlg, IDC_CHECK_DispFUNCKEYWND, m_Common.m_bDispFUNCKEYWND );
//
//	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
//	if( 0 == m_Common.m_nFUNCKEYWND_Place ){
//		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, TRUE );
//		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, FALSE );
//	}else{
//		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, FALSE );
//		::CheckDlgButton( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, TRUE );
//	}
//
//	/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */
//	::CheckDlgButton( hwndDlg, IDC_CHECK_DispSTATUSBAR, m_Common.m_bDispSTATUSBAR );
//
//	/* �E�B���h�E�T�C�Y�p�� */
//	::CheckDlgButton( hwndDlg, IDC_CHECK_WINSIZE, m_Common.m_bSaveWindowSize );



	/* �^�X�N�g���C���g�� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_USETRAYICON, m_Common.m_bUseTaskTray );
// From Here 2001.12.03 hor
//@@@ YAZAKI 2001.12.31 �����͐��䂷��B
	if( m_Common.m_bUseTaskTray ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), FALSE );
	}
// To Here 2001.12.03 hor
	/* �^�X�N�g���C�ɏ풓 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_STAYTASKTRAY, m_Common.m_bStayTaskTray );

	/* �^�X�N�g���C���N���b�N���j���[�̃V���[�g�J�b�g */
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_HOTKEY_TRAYMENU ), HKM_SETHOTKEY, MAKEWORD( m_Common.m_wTrayMenuHotKeyCode, m_Common.m_wTrayMenuHotKeyMods ), 0 );


	return;
}





/* �_�C�A���O�f�[�^�̎擾 p1 */
int CPropCommon::GetData_p1( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_ZENPAN;	//Oct. 25, 2000 JEPRO ZENPAN1��ZENPAN �ɕύX(�Q�Ƃ��Ă���̂�CPropCommon.cpp�݂̂�1�ӏ�)

//	/* �܂�Ԃ������� */
//	m_Common.m_nMAXLINELEN = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE );
//	if( m_Common.m_nMAXLINELEN < 10 ){
//		m_Common.m_nMAXLINELEN = 10;
//	}
//	if( m_Common.m_nMAXLINELEN > 10240 ){
//		m_Common.m_nMAXLINELEN = 10240;
//	}

//	/* �����̌��� */
//	m_Common.m_nCHARSPACE = ::GetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE );
//	if( m_Common.m_nCHARSPACE < 0 ){
//		m_Common.m_nCHARSPACE = 0;
///	}
//	if( m_Common.m_nCHARSPACE > 16 ){
//		m_Common.m_nCHARSPACE = 16;
//	}

	/* �s�̌��� */
//	m_Common.m_nLINESPACE = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE );
//	if( m_Common.m_nLINESPACE < 0 ){
//		m_Common.m_nLINESPACE = 0;
//	}
//	if( m_Common.m_nLINESPACE > 16 ){
//		m_Common.m_nLINESPACE = 16;
//	}

	/* �J�[�\���̃^�C�v 0=win 1=dos  */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_CARETTYPE0 ) ){
		m_Common.SetCaretType(0);
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_CARETTYPE1 ) ){
		m_Common.SetCaretType(1);
	}

	/* �t���[�J�[�\�����[�h */
	m_Common.m_bIsFreeCursorMode = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_FREECARET );

	/* �C���f���g */
	m_Common.m_bAutoIndent = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_INDENT );

	/* ���{��󔒂��C���f���g */
	m_Common.m_bAutoIndent_ZENSPACE = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_INDENT_WSPACE );

//	/* TAB�� */
//	m_Common.m_nTABSPACE = ::GetDlgItemInt( hwndDlg, IDC_COMBO_TABSPACE, NULL, FALSE );


	/* �I�����̊m�F������ */
	m_Common.m_bExitConfirm = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_EXITCONFIRM );

	/* �L�[���s�[�g���̃X�N���[���s�� */
	m_Common.m_nRepeatedScrollLineNum = ::GetDlgItemInt( hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, NULL, FALSE );
	if( m_Common.m_nRepeatedScrollLineNum < 1 ){
		m_Common.m_nRepeatedScrollLineNum = 1;
	}
	if( m_Common.m_nRepeatedScrollLineNum > 10 ){
		m_Common.m_nRepeatedScrollLineNum = 10;
	}

	/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	m_Common.m_nRepeatedScroll_Smooth = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_REPEATEDSCROLLSMOOTH );


//	/* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
//	m_Common.m_bAddCRLFWhenCopy = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ADDCRLFWHENCOPY );

//	/* Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v */
//	m_Common.m_bGTJW_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_RETURN );
//
//	/* Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v  */
//	m_Common.m_bGTJW_LDBLCLK = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_GTJW_LDBLCLK );

	/* �t�@�C���̗���MAX */
	m_Common.m_nMRUArrNum_MAX = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FILE, NULL, FALSE );
	if( m_Common.m_nMRUArrNum_MAX < 0 ){
		m_Common.m_nMRUArrNum_MAX = 0;
	}
	if( m_Common.m_nMRUArrNum_MAX > MAX_MRU ){
		m_Common.m_nMRUArrNum_MAX = MAX_MRU;
	}


	/* �t�H���_�̗���MAX */
	m_Common.m_nOPENFOLDERArrNum_MAX = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, NULL, FALSE );
	if( m_Common.m_nOPENFOLDERArrNum_MAX < 0 ){
		m_Common.m_nOPENFOLDERArrNum_MAX = 0;
	}
	if( m_Common.m_nOPENFOLDERArrNum_MAX > MAX_OPENFOLDER ){
		m_Common.m_nOPENFOLDERArrNum_MAX = MAX_OPENFOLDER;
	}

//	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
//	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1 ) ){
//		m_Common.m_nFUNCKEYWND_Place = 0;
//	}
//	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2) ){
//		m_Common.m_nFUNCKEYWND_Place = 1;
//	}
//
//	/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
//	m_Common.m_bDispTOOLBAR = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispTOOLBAR );
//
//	/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
//	m_Common.m_bDispFUNCKEYWND = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispFUNCKEYWND );
//
//	/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */
//	m_Common.m_bDispSTATUSBAR = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DispSTATUSBAR );
//
//	/* �E�B���h�E�T�C�Y�p�� */
//	m_Common.m_bSaveWindowSize = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_WINSIZE );
//
	/* �^�X�N�g���C���g�� */
	m_Common.m_bUseTaskTray = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_USETRAYICON );
//@@@ YAZAKI 2001.12.31 m_bUseTaskTray�Ɉ����Â���悤�ɁB
	if( m_Common.m_bUseTaskTray ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_STAYTASKTRAY ), FALSE );
	}
	/* �^�X�N�g���C�ɏ풓 */
	m_Common.m_bStayTaskTray = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_STAYTASKTRAY );

	/* �^�X�N�g���C���N���b�N���j���[�̃V���[�g�J�b�g */
	LRESULT	lResult;
	lResult = ::SendMessage( ::GetDlgItem( hwndDlg, IDC_HOTKEY_TRAYMENU ), HKM_GETHOTKEY, 0, 0 );
	m_Common.m_wTrayMenuHotKeyCode = LOBYTE( lResult );
	m_Common.m_wTrayMenuHotKeyMods = HIBYTE( lResult );

	return TRUE;
}



/* �w���v */
//Stonee, 2001/05/18 �@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
void CPropCommon::OnHelp( HWND hwndParent, int nPageID )
{
	int		nContextID;
	switch( nPageID ){
	case IDD_PROP1P1:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GENERAL);
		break;
	case IDD_PROP_FORMAT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FORMAT);
		break;
	case IDD_PROP_FILE:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_FILE);
		break;
//	Sept. 10, 2000 JEPRO ID�������ۂ̖��O�ɕύX���邽�߈ȉ��̍s�̓R�����g�A�E�g
//	�ύX�͏�����̍s(Sept. 9, 2000)�ōs���Ă���
//	case IDD_PROP1P5:
//		nContextID = 84;
//		break;
	case IDD_PROP_TOOLBAR:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_TOOLBAR);
		break;
	case IDD_PROP_KEYWORD:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYWORD);
		break;
	case IDD_PROP_CUSTMENU:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_CUSTMENU);
		break;
	case IDD_PROP_HELPER:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_HELPER);
		break;

	// From Here Sept. 9, 2000 JEPRO ���ʐݒ�̃w���v�{�^���������Ȃ��Ȃ��Ă����������ȉ��̒ǉ��ɂ���ďC��
	case IDD_PROP_EDIT:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_EDIT);
		break;
	case IDD_PROP_BACKUP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_BACKUP);
		break;
	case IDD_PROP_WIN:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_WINDOW);
		break;
	case IDD_PROP_URL:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_URL);
		break;
	case IDD_PROP_GREP:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_GREP);
		break;
	case IDD_PROP_KEYBIND:
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_KEYBIND);
		break;
	// To Here Sept. 9, 2000
	case IDD_PROP_MACRO:	//@@@ 2002.01.02
		nContextID = ::FuncID_To_HelpContextID(F_OPTION_MACRO);
		break;

	default:
		nContextID = -1;
		break;
	}
	if( -1 != nContextID ){
		::WinHelp( hwndParent, m_szHelpFile, HELP_CONTEXT, nContextID );
	}
	return;
}


/*[EOF]*/
