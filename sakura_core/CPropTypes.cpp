/*!	@file
	@brief �^�C�v�ʐݒ�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/12/24  �V�K�쐬
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, MIK, hor, Stonee, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta, ������, Moca
	Copyright (C) 2003, MIK, zenryaku, Moca, naoh, KEITA, genta
	Copyright (C) 2005, MIK, genta, Moca, ryoji
	Copyright (C) 2006, ryoji, fon

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "sakura_rc.h"
#include "CPropTypes.h"
#include "debug.h"
#include <windows.h>
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "CDlgKeywordSelect.h"
#include "etc_uty.h"
#include "global.h"
#include "CProfile.h"
#include "CShareData.h"
#include "funccode.h"	//Stonee, 2001/05/18
#include "CDlgSameColor.h"	// 2006.04.26 ryoji

struct TYPE_NAME {
	int		nMethod;
	char*	pszName;
};
TYPE_NAME OlmArr[] = {
//	{ OUTLINE_C,		"C" },
	{ OUTLINE_CPP,		"C/C++" },
	{ OUTLINE_PLSQL,	"PL/SQL" },
	{ OUTLINE_JAVA,		"Java" },
	{ OUTLINE_COBOL,	"COBOL" },
	{ OUTLINE_PERL,		"Perl" },			//Sep. 8, 2000 genta
	{ OUTLINE_ASM,		"�A�Z���u��" },
	{ OUTLINE_VB,		"Visual Basic" },	// 2001/06/23 N.Nakatani
	{ OUTLINE_WZTXT,	"WZ�K�w�t�e�L�X�g" },	// 2003.05.20 zenryaku, 2003.06.23 Moca ���̕ύX
	{ OUTLINE_HTML,		"HTML" },			// 2003.05.20 zenryaku
	{ OUTLINE_TEX,		"TeX" },			// 2003.07.20 naoh
	{ OUTLINE_TEXT,		"�e�L�X�g" }		//Jul. 08, 2001 JEPRO ��ɍŌ���ɂ���
};
const int	nOlmArrNum = sizeof( OlmArr ) / sizeof( OlmArr[0] );


TYPE_NAME SmartIndentArr[] = {
	{ SMARTINDENT_NONE,	"�Ȃ�" },
	{ SMARTINDENT_CPP,	"C/C++" }
};
const int	nSmartIndentArrNum = sizeof( SmartIndentArr ) / sizeof( SmartIndentArr[0] );

//	Nov. 20, 2000 genta
TYPE_NAME ImeStateArr[] = {
	{ 0, "�W���ݒ�" },
	{ 1, "�S�p" },
	{ 2, "�S�p�Ђ炪��" },
	{ 3, "�S�p�J�^�J�i" },
	{ 4, "���ϊ�" }
};
const int nImeStateArrNum = sizeof( ImeStateArr ) / sizeof( ImeStateArr[0] );

TYPE_NAME ImeSwitchArr[] = {
	{ 0, "���̂܂�" },
	{ 1, "���ON" },
	{ 2, "���OFF" },
};
const int nImeSwitchArrNum = sizeof( ImeSwitchArr ) / sizeof( ImeSwitchArr[0] );

/*!	2�s�ڈȍ~�̃C���f���g���@

	@sa CLayoutMgr::SetLayoutInfo()
	@date Oct. 1, 2002 genta 
*/
TYPE_NAME IndentTypeArr[] = {
	{ 0, "�Ȃ�" },
	{ 1, "tx2x" },
	{ 2, "�_���s�擪" },
};
const int nIndentTypeArrNum = sizeof( IndentTypeArr ) / sizeof( IndentTypeArr[0] );


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

WNDPROC	m_wpColorListProc;

//Sept. 5, 2000 JEPRO ���p�J�^�J�i�̑S�p���ɔ�����������ύX(21��32)
#define STR_COLORDATA_HEAD_LEN	32
#define STR_COLORDATA_HEAD		"�e�L�X�g�G�f�B�^ �F�ݒ�t�@�C��\x1a"

//#define STR_COLORDATA_HEAD2	" �e�L�X�g�G�f�B�^�F�ݒ� Ver2"
//#define STR_COLORDATA_HEAD21	" �e�L�X�g�G�f�B�^�F�ݒ� Ver2.1"	//Nov. 2, 2000 JEPRO �ύX [��]. 0.3.9.0:ur3��10�ȍ~�A�ݒ荀�ڂ̔ԍ������ւ�������
#define STR_COLORDATA_HEAD3		" �e�L�X�g�G�f�B�^�F�ݒ� Ver3"		//Jan. 15, 2001 Stonee  �F�ݒ�Ver3�h���t�g(�ݒ�t�@�C���̃L�[��A�ԁ��������)	//Feb. 11, 2001 JEPRO �L���ɂ���
#define STR_COLORDATA_SECTION	"SakuraColor"

char* MakeRGBStr( DWORD dwRGB, char* pszText )
{
	wsprintf( pszText, "RGB( %d, %d, %d )",
		GetRValue( dwRGB ),
		GetGValue( dwRGB ),
		GetBValue( dwRGB )
	);
	return pszText;

}


//@@@ 2001.02.04 Start by MIK: Popup Help
#include "sakura.hh"
static const DWORD p_helpids1[] = {	//11300
	IDC_CHECK_WORDWRAP,				HIDC_CHECK_WORDWRAP,		//�p�����[�h���b�v
	IDC_EDIT_TABSPACE,				HIDC_EDIT_TABSPACE,			//TAB�� // Sep. 19, 2002 genta
	IDC_COMBO_IMESWITCH,			HIDC_COMBO_IMESWITCH,		//IME��ON/OFF���
	IDC_COMBO_IMESTATE,				HIDC_COMBO_IMESTATE,		//IME�̓��̓��[�h
	IDC_COMBO_SMARTINDENT,			HIDC_COMBO_SMARTINDENT,		//�X�}�[�g�C���f���g
	IDC_COMBO_OUTLINES,				HIDC_COMBO_OUTLINES,		//�A�E�g���C����͕��@
	IDC_EDIT_TYPENAME,				HIDC_EDIT_TYPENAME,			//�ݒ�̖��O
	IDC_EDIT_TYPEEXTS,				HIDC_EDIT_TYPEEXTS,			//�t�@�C���g���q
	IDC_EDIT_MAXLINELEN,			HIDC_EDIT_MAXLINELEN,		//�܂�Ԃ�����
	IDC_EDIT_CHARSPACE,				HIDC_EDIT_CHARSPACE,		//�����̊Ԋu
	IDC_EDIT_LINESPACE,				HIDC_EDIT_LINESPACE,		//�s�̊Ԋu
	IDC_EDIT_INDENTCHARS,			HIDC_EDIT_INDENTCHARS,		//���̑��̃C���f���g�Ώە���
	IDC_EDIT_TABVIEWSTRING,			HIDC_EDIT_TABVIEWSTRING,	//TAB�\��������
	IDC_CHECK_INS_SPACE,			HIDC_CHECK_INS_SPACE,		//�X�y�[�X�̑}��
	IDC_SPIN_MAXLINELEN,			HIDC_EDIT_MAXLINELEN,
	IDC_SPIN_CHARSPACE,				HIDC_EDIT_CHARSPACE,
	IDC_SPIN_LINESPACE,				HIDC_EDIT_LINESPACE,
	IDC_CHECK_KINSOKUHEAD,			HIDC_CHECK_KINSOKUHEAD,		//�s���֑�	//@@@ 2002.04.08 MIK
	IDC_CHECK_KINSOKUTAIL,			HIDC_CHECK_KINSOKUTAIL,		//�s���֑�	//@@@ 2002.04.08 MIK
	IDC_CHECK_KINSOKURET,			HIDC_CHECK_KINSOKURET,		//���s�������Ԃ牺����	//@@@ 2002.04.14 MIK
	IDC_CHECK_KINSOKUKUTO,			HIDC_CHECK_KINSOKUKUTO,		//��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	IDC_EDIT_KINSOKUHEAD,			HIDC_EDIT_KINSOKUHEAD,		//�s���֑�	//@@@ 2002.04.08 MIK
	IDC_EDIT_KINSOKUTAIL,			HIDC_EDIT_KINSOKUTAIL,		//�s���֑�	//@@@ 2002.04.08 MIK
	IDC_CHECK_TAB_ARROW,			HIDC_CHECK_TAB_ARROW,		//���\��	// 2006.08.06 ryoji
	IDC_CHECK_INDENT,				HIDC_CHECK_INDENT,			//�����C���f���g	// 2006.08.19 ryoji
	IDC_CHECK_INDENT_WSPACE,		HIDC_CHECK_INDENT_WSPACE,	//�S�p�󔒂��C���f���g	// 2006.08.19 ryoji
	IDC_COMBO_INDENTLAYOUT,			HIDC_COMBO_INDENTLAYOUT,	//�܂�Ԃ��s�C���f���g	// 2006.08.06 ryoji
	IDC_CHECK_RTRIM_PREVLINE,		HIDC_CHECK_RTRIM_PREVLINE,	//���s���ɖ����̋󔒂��폜	// 2006.08.06 ryoji
	IDC_RADIO_OUTLINEDEFAULT,		HIDC_RADIO_OUTLINEDEFAULT,	//�W�����[��	// 2006.08.06 ryoji
	IDC_RADIO_OUTLINERULEFILE,		HIDC_RADIO_OUTLINERULEFILE,	//���[���t�@�C��	// 2006.08.06 ryoji
	IDC_EDIT_OUTLINERULEFILE,		HIDC_EDIT_OUTLINERULEFILE,	//���[���t�@�C����	// 2006.08.06 ryoji
	IDC_CHECK_DOCICON,				HIDC_CHECK_DOCICON,			//�����A�C�R�����g��	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};
static const DWORD p_helpids2[] = {	//11400
	IDC_BUTTON_TEXTCOLOR,			HIDC_BUTTON_TEXTCOLOR,			//�����F
	IDC_BUTTON_BACKCOLOR,			HIDC_BUTTON_BACKCOLOR,			//�w�i�F
	IDC_BUTTON_SAMETEXTCOLOR,		HIDC_BUTTON_SAMETEXTCOLOR,		//�����F����
	IDC_BUTTON_SAMEBKCOLOR,			HIDC_BUTTON_SAMEBKCOLOR,		//�w�i�F����
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_COLOR,		//�C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_COLOR,		//�G�N�X�|�[�g
	IDC_CHECK_DISP,					HIDC_CHECK_DISP,				//�F�����\��
	IDC_CHECK_FAT,					HIDC_CHECK_FAT,					//����
	IDC_CHECK_UNDERLINE,			HIDC_CHECK_UNDERLINE,			//����
	IDC_CHECK_LCPOS,				HIDC_CHECK_LCPOS,				//���w��P
	IDC_CHECK_LCPOS2,				HIDC_CHECK_LCPOS2,				//���w��Q
	IDC_COMBO_SET,					HIDC_COMBO_SET_COLOR,			//�����L�[���[�h�P�Z�b�g��
	IDC_COMBO_SET2,					HIDC_COMBO_SET2_COLOR,			//�����L�[���[�h�Q�Z�b�g��
	IDC_EDIT_BLOCKCOMMENT_FROM,		HIDC_EDIT_BLOCKCOMMENT_FROM,	//�u���b�N�R�����g�P�J�n
	IDC_EDIT_BLOCKCOMMENT_TO,		HIDC_EDIT_BLOCKCOMMENT_TO,		//�u���b�N�R�����g�P�I��
	IDC_EDIT_LINECOMMENT,			HIDC_EDIT_LINECOMMENT,			//�s�R�����g�P
	IDC_EDIT_LINECOMMENT2,			HIDC_EDIT_LINECOMMENT2,			//�s�R�����g�Q
	IDC_EDIT_LINECOMMENTPOS,		HIDC_EDIT_LINECOMMENTPOS,		//�����P
	IDC_EDIT_LINECOMMENTPOS2,		HIDC_EDIT_LINECOMMENTPOS2,		//�����Q
	IDC_EDIT_LINETERMCHAR,			HIDC_EDIT_LINETERMCHAR,			//�s�ԍ���؂�
	IDC_EDIT_BLOCKCOMMENT_FROM2,	HIDC_EDIT_BLOCKCOMMENT_FROM2,	//�u���b�N�R�����g�Q�J�n
	IDC_EDIT_BLOCKCOMMENT_TO2,		HIDC_EDIT_BLOCKCOMMENT_TO2,		//�u���b�N�R�����g�Q�I��
	IDC_EDIT_LINECOMMENT3,			HIDC_EDIT_LINECOMMENT3,			//�s�R�����g�R
	IDC_LIST_COLORS,				HIDC_LIST_COLORS,				//�F�w��
	IDC_CHECK_LCPOS3,				HIDC_CHECK_LCPOS3,				//���w��R
	IDC_EDIT_LINECOMMENTPOS3,		HIDC_EDIT_LINECOMMENTPOS3,		//�����R
	IDC_RADIO_ESCAPETYPE_1,			HIDC_RADIO_ESCAPETYPE_1,		//������G�X�P�[�v�iC���ꕗ�j
	IDC_RADIO_ESCAPETYPE_2,			HIDC_RADIO_ESCAPETYPE_2,		//������G�X�P�[�v�iPL/SQL���j
	IDC_RADIO_LINENUM_LAYOUT,		HIDC_RADIO_LINENUM_LAYOUT,		//�s�ԍ��̕\���i�܂�Ԃ��P�ʁj
	IDC_RADIO_LINENUM_CRLF,			HIDC_RADIO_LINENUM_CRLF,		//�s�ԍ��̕\���i���s�P�ʁj
	IDC_RADIO_LINETERMTYPE0,		HIDC_RADIO_LINETERMTYPE0,		//�s�ԍ���؂�i�Ȃ��j
	IDC_RADIO_LINETERMTYPE1,		HIDC_RADIO_LINETERMTYPE1,		//�s�ԍ���؂�i�c���j
	IDC_RADIO_LINETERMTYPE2,		HIDC_RADIO_LINETERMTYPE2,		//�s�ԍ���؂�i�C�Ӂj
	IDC_BUTTON_KEYWORD_SELECT,		HIDC_BUTTON_KEYWORD_SELECT,		//�����L�[���[�h2�`10	// 2006.08.06 ryoji
	IDC_EDIT_VERTLINE,				HIDC_EDIT_VERTLINE,				//�c���̌��w��	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};
static const DWORD p_helpids3[] = {	//11500
	IDC_BUTTON_HOKANFILE_REF,		HIDC_BUTTON_HOKANFILE_REF,			//���͕⊮ �P��t�@�C���Q��
	IDC_CHECK_HOKANLOHICASE,		HIDC_CHECK_HOKANLOHICASE,			//���͕⊮�̉p�啶��������
	IDC_CHECK_HOKANBYFILE,			HIDC_CHECK_HOKANBYFILE,				//���݂̃t�@�C��������͕⊮
	IDC_EDIT_HOKANFILE,				HIDC_EDIT_HOKANFILE,				//�P��t�@�C����
	IDC_EDIT_TYPEEXTHELP,			HIDC_EDIT_TYPEEXTHELP,				//�O���w���v�t�@�C����	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENHELP,		HIDC_BUTTON_TYPEOPENHELP,			//�O���w���v�t�@�C���Q��	// 2006.08.06 ryoji
	IDC_EDIT_TYPEEXTHTMLHELP,		HIDC_EDIT_TYPEEXTHTMLHELP,			//�O��HTML�w���v�t�@�C����	// 2006.08.06 ryoji
	IDC_BUTTON_TYPEOPENEXTHTMLHELP,	HIDC_BUTTON_TYPEOPENEXTHTMLHELP,	//�O��HTML�w���v�t�@�C���Q��	// 2006.08.06 ryoji
	IDC_CHECK_TYPEHTMLHELPISSINGLE,	HIDC_CHECK_TYPEHTMLHELPISSINGLE,	//�r���[�A�𕡐��N�����Ȃ�	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};
//To Here Jul. 05, 2001



/* p1 �_�C�A���O�v���V�[�W�� */
INT_PTR CALLBACK PropTypesP1Proc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypes* pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypes* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p1( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropTypes = ( CPropTypes* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p1( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}

/* p2 �_�C�A���O�v���V�[�W�� */
INT_PTR CALLBACK PropTypesP2Proc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypes* pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypes* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p2( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropTypes = ( CPropTypes* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p2( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}






//	/* p3 �_�C�A���O�v���V�[�W�� */
//	BOOL CALLBACK PropTypesP3Proc(
//		HWND	hwndDlg,	// handle to dialog box
//		UINT	uMsg,		// message
//		WPARAM	wParam,		// first message parameter
//		LPARAM	lParam 		// second message parameter
//	)
//	{
//	PROPSHEETPAGE*	pPsp;
//	CPropTypes*		pCPropTypes;
//	switch( uMsg ){
//	case WM_INITDIALOG:
//		pPsp = (PROPSHEETPAGE*)lParam;
//		pCPropTypes = ( CPropTypes* )(pPsp->lParam);
//		if( NULL != pCPropTypes ){
//			return pCPropTypes->DispatchEvent_p3( hwndDlg, uMsg, wParam, pPsp->lParam );
//		}else{
//			return FALSE;
//		}
//	default:
//		pCPropTypes = ( CPropTypes* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
//		if( NULL != pCPropTypes ){
//			return pCPropTypes->DispatchEvent_p3( hwndDlg, uMsg, wParam, lParam );
//		}else{
//			return FALSE;
//		}
//	}
//	}



/* p3 �_�C�A���O�v���V�[�W�� */
INT_PTR CALLBACK PropTypesP3_newProc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypes*		pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypes* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p3_new( hwndDlg, uMsg, wParam, pPsp->lParam );
		}else{
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropTypes = ( CPropTypes* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_p3_new( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}




CPropTypes::CPropTypes()
{
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	// Mar. 31, 2003 genta �������팸�̂��߃|�C���^�ɕύX
	m_pCKeyWordSetMgr = &(m_pShareData->m_CKeyWordSetMgr);

	m_hInstance = NULL;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = NULL;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_hwndThis  = NULL;		/* ���̃_�C�A���O�̃n���h�� */
	m_nPageNum = 0;

	// 2005.11.30 Moca �J�X�^���F��ݒ�E�ێ�
	int i;
	for( i = 0; i < sizeof(m_dwCustColors)/sizeof(m_dwCustColors[0]); i++ ){
		m_dwCustColors[i] = RGB( 255, 255, 255 );
	}
	
	/* �w���v�t�@�C���̃t���p�X��Ԃ� */
	::GetHelpFilePath( m_szHelpFile );

	return;
}





CPropTypes::~CPropTypes()
{
//	if( NULL != m_hbmpToolButtons ){
//		/* �r�b�g�}�b�v�j�� */
//		::DeleteObject( m_hbmpToolButtons );
//	}
	return;
}





/* ������ */
void CPropTypes::Create( HINSTANCE hInstApp, HWND hwndParent )
{
	m_hInstance = hInstApp;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	return;
}





/* �F�I���_�C�A���O */
BOOL CPropTypes::SelectColor( HWND hwndParent, COLORREF* pColor, DWORD* pCustColors )
{
	CHOOSECOLOR		cc;
	cc.lStructSize = sizeof( cc );
	cc.hwndOwner = hwndParent;
	cc.hInstance = NULL;
	cc.rgbResult = *pColor;
	cc.lpCustColors = pCustColors;
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
void CPropTypes::DrawColorButton( DRAWITEMSTRUCT* pDis, COLORREF cColor )
{
//	MYTRACE( "pDis->itemAction = " );

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
	if( pDis->itemState & ODS_DISABLED ){
	}else{
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
	}


//	/* ��؂肽�Ė_ */
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










//	�L�[���[�h�F�^�C�v�ʐݒ�^�u����(�v���p�e�B�V�[�g)
/* �v���p�e�B�V�[�g�̍쐬 */
int CPropTypes::DoPropertySheet( int nPageNum )
{
	int				nRet;
	PROPSHEETPAGE	psp[16];
	PROPSHEETHEADER	psh;
	int				nIdx;

	m_nMaxLineSize_org = m_Types.m_nMaxLineSize;

	// 2005.11.30 Moca �J�X�^���F�̐擪�Ƀe�L�X�g�F��ݒ肵�Ă���
	m_dwCustColors[0] = m_Types.m_ColorInfoArr[COLORIDX_TEXT].m_colTEXT;
	m_dwCustColors[1] = m_Types.m_ColorInfoArr[COLORIDX_TEXT].m_colBACK;

	nIdx = 0;
	memset( &psp[nIdx], 0, sizeof( PROPSHEETPAGE ) );
	psp[nIdx].dwSize = sizeof( PROPSHEETPAGE );
	psp[nIdx].dwFlags = /*PSP_USEICONID |*/ PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROPTYPESP1 );
	psp[nIdx].pszIcon = NULL/*MAKEINTRESOURCE( IDI_FONT )*/;
	psp[nIdx].pfnDlgProc = (DLGPROC)PropTypesP1Proc;
	psp[nIdx].pszTitle = "�X�N���[��";
	psp[nIdx].lParam = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;

//	memset( &psp[nIdx], 0, sizeof( PROPSHEETPAGE ) );
//	psp[nIdx].dwSize = sizeof( PROPSHEETPAGE );
//	psp[nIdx].dwFlags = /*PSP_USEICONID |*/ PSP_USETITLE | PSP_HASHELP;
//	psp[nIdx].hInstance = m_hInstance;
//	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP1P3 );
//	psp[nIdx].pszIcon = NULL /*MAKEINTRESOURCE( IDI_BORDER )*/;
//	psp[nIdx].pfnDlgProc = (DLGPROC)PropTypesP3Proc;
//	psp[nIdx].pszTitle = "�J���[";
//	psp[nIdx].lParam = (LPARAM)this;
//	psp[nIdx].pfnCallback = NULL;
//	nIdx++;

	memset( &psp[nIdx], 0, sizeof( PROPSHEETPAGE ) );
	psp[nIdx].dwSize = sizeof( PROPSHEETPAGE );
	psp[nIdx].dwFlags = /*PSP_USEICONID |*/ PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP_COLOR );
	psp[nIdx].pszIcon = NULL /*MAKEINTRESOURCE( IDI_BORDER) */;
	psp[nIdx].pfnDlgProc = (DLGPROC)PropTypesP3_newProc;
	psp[nIdx].pszTitle = "�J���[";
	psp[nIdx].lParam = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;

	// 2001/06/14 Start by asa-o: �^�C�v�ʐݒ�Ɏx���^�u�ǉ�
	memset( &psp[nIdx], 0, sizeof( PROPSHEETPAGE ) );
	psp[nIdx].dwSize = sizeof( PROPSHEETPAGE );
	psp[nIdx].dwFlags = PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROPTYPESP2 );
	psp[nIdx].pszIcon = NULL;
	psp[nIdx].pfnDlgProc = (DLGPROC)PropTypesP2Proc;
	psp[nIdx].pszTitle = "�x��";
	psp[nIdx].lParam = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;
	// 2001/06/14 End

	// 2001.11.17 add start MIK �^�C�v�ʐݒ�ɐ��K�\���L�[���[�h�^�u�ǉ�
	memset( &psp[nIdx], 0, sizeof( PROPSHEETPAGE ) );
	psp[nIdx].dwSize = sizeof( PROPSHEETPAGE );
	psp[nIdx].dwFlags = PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP_REGEX );
	psp[nIdx].pszIcon = NULL;
	psp[nIdx].pfnDlgProc = (DLGPROC)PropTypesRegex;
	psp[nIdx].pszTitle = "���K�\���L�[���[�h";
	psp[nIdx].lParam = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;
	// 2001.11.17 add end MIK

	// 2006.04.10 fon ADD-start �^�C�v�ʐݒ�Ɂu�L�[���[�h�w���v�v�^�u��ǉ�
	memset( &psp[nIdx], 0, sizeof( PROPSHEETPAGE ) );
	psp[nIdx].dwSize = sizeof( PROPSHEETPAGE );
	psp[nIdx].dwFlags = PSP_USETITLE | PSP_HASHELP;
	psp[nIdx].hInstance = m_hInstance;
	psp[nIdx].pszTemplate = MAKEINTRESOURCE( IDD_PROP_KEYHELP );
	psp[nIdx].pszIcon = NULL;
	psp[nIdx].pfnDlgProc = (DLGPROC)PropTypesKeyHelp;
	psp[nIdx].pszTitle = "�L�[���[�h�w���v";
	psp[nIdx].lParam = (LPARAM)this;
	psp[nIdx].pfnCallback = NULL;
	nIdx++;
	// 2006.04.10 fon ADD-end

	memset( &psh, 0, sizeof( PROPSHEETHEADER ) );
#ifdef _WIN64
	psh.dwSize = sizeof( psh );
#else
	//	Jun. 29, 2002 ������
	//	Windows 95�΍�DProperty Sheet�̃T�C�Y��Windows95���F���ł��镨�ɌŒ肷��D
	const size_t sizeof_old_PROPSHEETHEADER=40;
	psh.dwSize = sizeof_old_PROPSHEETHEADER;
#endif
// JEPROtest Sept. 30, 2000 �^�C�v�ʐݒ�̉B��[�K�p]�{�^���̐��̂͂����B�s���̃R�����g�A�E�g�����ւ��Ă݂�΂킩��
//  psh.dwFlags = /*PSH_USEICONID |*/ /*PSH_NOAPPLYNOW |*/ PSH_PROPSHEETPAGE/* | PSH_HASHELP*/;
	psh.dwFlags = /*PSH_USEICONID |*/ PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE/* | PSH_HASHELP*/;
	psh.hwndParent = m_hwndParent;
	psh.hInstance = m_hInstance;
	psh.pszIcon = NULL /*MAKEINTRESOURCE( IDI_CELL_PROPERTIES )*/;
	psh.pszCaption = (LPSTR)"�^�C�v�ʐݒ�";	// Sept. 8, 2000 jepro �P�Ȃ�u�ݒ�v����ύX
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

//	m_hbmpToolButtons = ::LoadBitmap( m_hInstance, MAKEINTRESOURCE( IDB_MYTOOL ) );

	nRet = ::PropertySheet( &psh );
	if( -1 == nRet ){
		char*	pszMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			::GetLastError(),
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), // �f�t�H���g����
			(LPTSTR)&pszMsgBuf,
			0,
			NULL
		);
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "��҂ɋ����ė~�����G���[",
			"CPropTypes::DoPropertySheet()���ŃG���[���o�܂����B\npsh.nStartPage=[%d]\n::PropertySheet()���s�B\n\n%s\n", psh.nStartPage, pszMsgBuf
		);
		::LocalFree( pszMsgBuf );
	}

//	::DeleteObject( m_hbmpToolButtons );
//	m_hbmpToolButtons = NULL;
	return nRet;
}





/* p1 ���b�Z�[�W���� */
INT_PTR CPropTypes::DispatchEvent_p1(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
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
		m_hwndThis = hwndDlg;
		/* �_�C�A���O�f�[�^�̐ݒ� p1 */
		SetData_p1( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPENAME ), EM_LIMITTEXT, (WPARAM)( sizeof( m_Types.m_szTypeName ) - 1 ), 0 );
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPEEXTS ), EM_LIMITTEXT, (WPARAM)( sizeof( m_Types.m_szTypeExts ) - 1 ), 0 );
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_INDENTCHARS ), EM_LIMITTEXT, (WPARAM)( sizeof( m_Types.m_szIndentChars ) - 1 ), 0 );
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), EM_LIMITTEXT, (WPARAM)( sizeof( m_Types.m_szTabViewString ) - 1 ), 0 );
//#endif
		//	Oct. 5, 2002 genta ��ʏ�ł����͐���
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ),  EM_LIMITTEXT, (WPARAM)( sizeof( m_Types.m_szOutlineRuleFilename ) - 1 ), 0 );

		if( 0 == m_Types.m_nIdx ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPENAME ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPEEXTS ), FALSE );
		}

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
//		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			/*	2002.04.01 YAZAKI �I�[�g�C���f���g���폜�i���Ƃ��ƕs�v�j
				�A�E�g���C����͂Ƀ��[���t�@�C���֘A��ǉ�
			*/
			case IDC_RADIO_OUTLINEDEFAULT:	/* �A�E�g���C����́��W�����[�� */
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), TRUE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), FALSE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), FALSE );

				::SendMessage( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), CB_SETCURSEL, 0, 0 );

				return TRUE;
			case IDC_RADIO_OUTLINERULEFILE:	/* �A�E�g���C����́����[���t�@�C�� */
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), FALSE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), TRUE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), TRUE );
				return TRUE;

			case IDC_BUTTON_RULEFILE_REF:	/* �A�E�g���C����́����[���t�@�C���́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					char			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
					if( _IS_REL_PATH( m_Types.m_szOutlineRuleFilename ) ){
						GetExecutableDir( szPath, m_Types.m_szOutlineRuleFilename );
					}else{
						strcpy( szPath, m_Types.m_szOutlineRuleFilename );
					}
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						"*.*",
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Types.m_szOutlineRuleFilename, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_Types.m_szOutlineRuleFilename );
					}
				}
				return TRUE;

//			case IDC_CHECK_KINSOKUHEAD:	/* �s���֑� */	//@@@ 2002.04.08 MIK
//				if( IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUHEAD ) ){
//					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUHEAD ), TRUE );
//				}else{
//					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUHEAD ), FALSE );
//				}
//				return TRUE;

//			case IDC_CHECK_KINSOKUTAIL:	/* �s���֑� */	//@@@ 2002.04.08 MIK
//				if( IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUTAIL ) ){
//					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUTAIL ), TRUE );
//				}else{
//					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUTAIL ), FALSE );
//				}
//				return TRUE;

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

				case IDC_CHECK_TAB_ARROW:
				// Mar. 31, 2003 genta ���\����ON/OFF��TAB������ݒ�ɘA��������
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TAB_ARROW ) ){
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), FALSE );
				}
				else {
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), TRUE );
				}
				return TRUE;

			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
		case IDC_SPIN_MAXLINELEN:
			/* �܂�Ԃ������� */
//			MYTRACE( "IDC_SPIN_MAXLINELEN\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < MINLINESIZE ){
				nVal = MINLINESIZE;
			}
			if( nVal > MAXLINESIZE ){
				nVal = MAXLINESIZE;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_CHARSPACE:
			/* �����̌��� */
//			MYTRACE( "IDC_SPIN_CHARSPACE\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > COLUMNSPACE_MAX ){ // Feb. 18, 2003 genta �ő�l�̒萔��
				nVal = COLUMNSPACE_MAX;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_LINESPACE:
			/* �s�̌��� */
//			MYTRACE( "IDC_SPIN_LINESPACE\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
//	From Here Oct. 8, 2000 JEPRO �s�Ԃ��ŏ�0�܂Őݒ�ł���悤�ɕύX(�̂ɖ߂�������?)
//			if( nVal < 1 ){
//				nVal = 1;
//			}
			if( nVal < 0 ){
				nVal = 0;
			}
//	To Here  Oct. 8, 2000
			if( nVal > LINESPACE_MAX ){ // Feb. 18, 2003 genta �ő�l�̒萔��
				nVal = LINESPACE_MAX;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, nVal, FALSE );
			return TRUE;
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
		case IDC_SPIN_TABSPACE:
			//	Sep. 22, 2002 genta
			/* TAB�� */
//			MYTRACE( "IDC_SPIN_CHARSPACE\n" );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 64 ){
				nVal = 64;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, nVal, FALSE );
			return TRUE;

		default:
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROPTYPESP1 );
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE( "p1 PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 p1 */
				GetData_p1( hwndDlg );

//				if( m_nMaxLineSize_org != m_Types.m_nMaxLineSize ){
//					if( IDNO == ::MessageBox( hwndDlg, "�܂�Ԃ�������ύX����ƁA�A���h�D�E���h�D�o�b�t�@���N���A����܂��B\n��낵���ł����H", GSTR_APPNAME, MB_YESNO | MB_ICONQUESTION ) ){
//						m_Types.m_nMaxLineSize = m_nMaxLineSize_org;
//						SetData_p1( hwndDlg );
//					}
//				}
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = 0;
				return TRUE;
			}
			break;
		}

//		MYTRACE( "pNMHDR->hwndFrom	=%xh\n",	pNMHDR->hwndFrom );
//		MYTRACE( "pNMHDR->idFrom	=%xh\n",	pNMHDR->idFrom );
//		MYTRACE( "pNMHDR->code		=%xh\n",	pNMHDR->code );
//		MYTRACE( "pMNUD->iPos		=%d\n",		pMNUD->iPos );
//		MYTRACE( "pMNUD->iDelta		=%d\n",		pMNUD->iDelta );
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids1 );
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids1 );
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}





/* �_�C�A���O�f�[�^�̐ݒ� p1 */
void CPropTypes::SetData_p1( HWND hwndDlg )
{
	BOOL	bRet;
	static	int	nTabArr[] = { 2, 3, 4, 6, 8 };
	static	int	nTabArrNum = sizeof( nTabArr ) / sizeof( nTabArr[0] );
	int			i;
	//char		szWork[32];

	/* �^�C�v�����F���� */
	::SetDlgItemText( hwndDlg, IDC_EDIT_TYPENAME, m_Types.m_szTypeName );

	/* �^�C�v�����F�g���q���X�g */
	::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTS, m_Types.m_szTypeExts );

	/* �܂�Ԃ������� */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, m_Types.m_nMaxLineSize, FALSE );

	/* �����̌��� */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, m_Types.m_nColmSpace, FALSE );

	/* �s�̌��� */
	bRet = ::SetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, m_Types.m_nLineSpace, FALSE );

	/* TAB�� */
	//	Sep. 22, 2002 genta
	::SetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, m_Types.m_nTabSpace, FALSE );
	//j = 0;
	//for( i = 0; i < nTabArrNum; ++i ){
	//	wsprintf( szWork, "%d", nTabArr[i] );
	//	::SendDlgItemMessage( hwndDlg, IDC_COMBO_TABSPACE, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)szWork );
	//	if( m_Types.m_nTabSpace == nTabArr[i] ){
	//		j = i;
	//	}
	//}
	//::SendDlgItemMessage( hwndDlg, IDC_COMBO_TABSPACE, CB_SETCURSEL, (WPARAM)j, 0 );
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	/* TAB�\�������� */
	::SetDlgItemText( hwndDlg, IDC_EDIT_TABVIEWSTRING, m_Types.m_szTabViewString );
//#endif

	//�^�u���\��	//@@@ 2003.03.26 MIK
	::CheckDlgButton( hwndDlg, IDC_CHECK_TAB_ARROW, m_Types.m_bTabArrow );
	// Mar. 31, 2003 genta ���\����ON/OFF��TAB������ݒ�ɘA��������
	if( m_Types.m_bTabArrow ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), FALSE );
	}
	else {
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), TRUE );
	}
// From Here 2001.12.03 hor
	/* �X�y�[�X�̑}�� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_INS_SPACE, m_Types.m_bInsSpace );
// To Here 2001.12.03 hor

	/* ���̑��̃C���f���g�Ώە��� */
	::SetDlgItemText( hwndDlg, IDC_EDIT_INDENTCHARS, m_Types.m_szIndentChars );


	/* �A�E�g���C����͕��@
	
		2002.04.01 YAZAKI ���[���t�@�C���֘A�ǉ�
	*/
	HWND	hwndCombo;
	int		nSelPos;
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	nSelPos = 0;
	for( i = 0; i < nOlmArrNum; ++i ){
		::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)OlmArr[i].pszName );
		if( OlmArr[i].nMethod == m_Types.m_nDefaultOutline ){	/* �A�E�g���C����͕��@ */
			nSelPos = i;
		}
	}

	// 2003.06.23 Moca ���[���t�@�C�����͎g��Ȃ��Ă��Z�b�g���Ă���
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), TRUE );
	::SetDlgItemText( hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_Types.m_szOutlineRuleFilename );

	if( m_Types.m_nDefaultOutline == OUTLINE_FILE ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINEDEFAULT, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINERULEFILE, TRUE );

		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), TRUE );

	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINEDEFAULT, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINERULEFILE, FALSE );

		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), FALSE );

		::SendMessage( hwndCombo, CB_SETCURSEL, nSelPos, 0 );
	}

	/* �C���f���g */
	::CheckDlgButton( hwndDlg, IDC_CHECK_INDENT, m_Types.m_bAutoIndent );

	/* ���{��󔒂��C���f���g */
	::CheckDlgButton( hwndDlg, IDC_CHECK_INDENT_WSPACE, m_Types.m_bAutoIndent_ZENSPACE );

	if( !m_Types.m_bAutoIndent ){
		/* ���{��󔒂��C���f���g */
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_INDENT_WSPACE ), FALSE );
	}

	/* 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜 */
	::CheckDlgButton( hwndDlg, IDC_CHECK_RTRIM_PREVLINE, m_Types.m_bRTrimPrevLine );

	/* �X�}�[�g�C���f���g��� */
//	HWND	hwndCombo;
//	int		nSelPos;
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_SMARTINDENT );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	nSelPos = 0;
	for( i = 0; i < nSmartIndentArrNum; ++i ){
		::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)SmartIndentArr[i].pszName );
		if( SmartIndentArr[i].nMethod == m_Types.m_nSmartIndent ){	/* �X�}�[�g�C���f���g��� */
			nSelPos = i;
		}
	}
	::SendMessage( hwndCombo, CB_SETCURSEL, nSelPos, 0 );

	/* �܂�Ԃ���2�s�ڈȍ~���������\�� */
	//	Oct. 1, 2002 genta �R���{�{�b�N�X�ɕύX
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_INDENTLAYOUT );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	nSelPos = 0;
	for( i = 0; i < nIndentTypeArrNum; ++i ){
		::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)IndentTypeArr[i].pszName );
		if( IndentTypeArr[i].nMethod == m_Types.m_nIndentLayout ){	/* �܂�Ԃ��C���f���g��� */
			nSelPos = i;
		}
	}
	::SendMessage( hwndCombo, CB_SETCURSEL, nSelPos, 0 );

	//	From Here Nov. 20, 2000 genta
	//	IME���̓��[�h
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	int ime = m_Types.m_nImeState >> 2;
	nSelPos = 0;
	for( i = 0; i < nImeStateArrNum; ++i ){
		::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)ImeStateArr[i].pszName );
		if( ImeStateArr[i].nMethod == ime ){	/* IME��� */
			nSelPos = i;
		}
	}
	::SendMessage( hwndCombo, CB_SETCURSEL, nSelPos, 0 );
	//	IME ON/OFF����
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	ime = m_Types.m_nImeState & 3;
	nSelPos = 0;
	for( i = 0; i < nImeSwitchArrNum; ++i ){
		::SendMessage( hwndCombo, CB_INSERTSTRING, i, (LPARAM)ImeSwitchArr[i].pszName );
		if( ImeSwitchArr[i].nMethod == ime ){	/* IME��� */
			nSelPos = i;
		}
	}
	::SendMessage( hwndCombo, CB_SETCURSEL, nSelPos, 0 );
	//	To Here Nov. 20, 2000 genta

	/* �p�����[�h���b�v������ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_WORDWRAP, m_Types.m_bWordWrap );

	/* �֑����� */
	{	//@@@ 2002.04.08 MIK start
		::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKUHEAD, m_Types.m_bKinsokuHead ? TRUE : FALSE );
		::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKUTAIL, m_Types.m_bKinsokuTail ? TRUE : FALSE );
		::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKURET,  m_Types.m_bKinsokuRet  ? TRUE : FALSE );	/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
		::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKUKUTO, m_Types.m_bKinsokuKuto ? TRUE : FALSE );	/* ��Ǔ_���Ԃ牺���� */	//@@@ 2002.04.17 MIK
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUHEAD ), EM_LIMITTEXT, (WPARAM)(sizeof(m_Types.m_szKinsokuHead) - 1 ), 0 );
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUTAIL ), EM_LIMITTEXT, (WPARAM)(sizeof(m_Types.m_szKinsokuTail) - 1 ), 0 );
		::SetDlgItemText( hwndDlg, IDC_EDIT_KINSOKUHEAD, m_Types.m_szKinsokuHead );
		::SetDlgItemText( hwndDlg, IDC_EDIT_KINSOKUTAIL, m_Types.m_szKinsokuTail );
//		if( m_Types.m_bKinsokuHead ){
//			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUHEAD ), TRUE );
//		}else{
//			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUHEAD ), FALSE );
//		}
//		if( m_Types.m_bKinsokuTail ){
//			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUTAIL ), TRUE );
//		}else{
//			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUTAIL ), FALSE );
//		}
	}	//@@@ 2002.04.08 MIK end
	
	//	Sep. 10, 2002 genta
	::CheckDlgButton( hwndDlg, IDC_CHECK_DOCICON, m_Types.m_bUseDocumentIcon  ? TRUE : FALSE );

	return;
}





/* �_�C�A���O�f�[�^�̎擾 p1 */
int CPropTypes::GetData_p1( HWND hwndDlg )
{
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	char szTab[8+1+1]; /* +1. happy */
	int  i;
//#endif

//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = 0;
	/* �^�C�v�����F���� */
	::GetDlgItemText( hwndDlg, IDC_EDIT_TYPENAME, m_Types.m_szTypeName, sizeof( m_Types.m_szTypeName ) );
	/* �^�C�v�����F�g���q���X�g */
	::GetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTS, m_Types.m_szTypeExts, sizeof( m_Types.m_szTypeExts ) );


	/* �܂�Ԃ������� */
	m_Types.m_nMaxLineSize = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE );
	if( m_Types.m_nMaxLineSize < MINLINESIZE ){
		m_Types.m_nMaxLineSize = MINLINESIZE;
	}
	if( m_Types.m_nMaxLineSize > MAXLINESIZE ){
		m_Types.m_nMaxLineSize = MAXLINESIZE;
	}

	/* �����̌��� */
	m_Types.m_nColmSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE );
	if( m_Types.m_nColmSpace < 0 ){
		m_Types.m_nColmSpace = 0;
	}
	if( m_Types.m_nColmSpace > COLUMNSPACE_MAX ){ // Feb. 18, 2003 genta �ő�l�̒萔��
		m_Types.m_nColmSpace = COLUMNSPACE_MAX;
	}

	/* �s�̌��� */
	m_Types.m_nLineSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE );
//	From Here Oct. 8, 2000 JEPRO �s�Ԃ��ŏ�0�܂Őݒ�ł���悤�ɕύX(�̂ɖ߂�������?)
//	if( m_Types.m_nLineSpace < 1 ){
//		m_Types.m_nLineSpace = 1;
//	}
	if( m_Types.m_nLineSpace < 0 ){
		m_Types.m_nLineSpace = 0;
	}
//	To Here  Oct. 8, 2000
	if( m_Types.m_nLineSpace > LINESPACE_MAX ){	// Feb. 18, 2003 genta �ő�l�̒萔��
		m_Types.m_nLineSpace = LINESPACE_MAX;
	}

	/* ���̑��̃C���f���g�Ώە��� */
	::GetDlgItemText( hwndDlg, IDC_EDIT_INDENTCHARS, m_Types.m_szIndentChars, sizeof( m_Types.m_szIndentChars ) - 1 );

	/* TAB�� */
	m_Types.m_nTabSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, NULL, FALSE );
	if( m_Types.m_nTabSpace < 1 ){
		m_Types.m_nTabSpace = 1;
	}
	if( m_Types.m_nTabSpace > 64 ){
		m_Types.m_nTabSpace = 64;
	}

//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
	/* TAB�\�������� */
	::GetDlgItemText( hwndDlg, IDC_EDIT_TABVIEWSTRING, szTab, sizeof( szTab ) - 1 );
	strcpy( m_Types.m_szTabViewString, "^       " );
	for( i = 0; i < 8; i++ ){
		if( (szTab[i] == '\0') || (szTab[i] < 0x20 || szTab[i] >= 0x7f) ) break;
		m_Types.m_szTabViewString[i] = szTab[i];
	}
//#endif

	//�^�u���\��	//@@@ 2003.03.26 MIK
	m_Types.m_bTabArrow = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TAB_ARROW );
// 2001.12.03 hor
	/* �X�y�[�X�̑}�� */
	m_Types.m_bInsSpace = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_INS_SPACE );
// From Here 2001.12.03 hor

	/* �A�E�g���C����͕��@
	
		2002.04.01 YAZAKI ���[���t�@�C���֘A�ǉ�
	*/
	HWND	hwndCombo;
	int		nSelPos;
	
	// 2003.06.23 Moca ���[�����g���Ă��Ȃ��Ă��t�@�C������ێ�
	::GetDlgItemText( hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_Types.m_szOutlineRuleFilename, sizeof( m_Types.m_szOutlineRuleFilename ));

	if ( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_OUTLINERULEFILE) ){
		m_Types.m_nDefaultOutline = OUTLINE_FILE;
	}
	else {
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES );
		nSelPos = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
		m_Types.m_nDefaultOutline = OlmArr[nSelPos].nMethod;	/* �A�E�g���C����͕��@ */
	}

	/* �C���f���g */
	m_Types.m_bAutoIndent = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_INDENT );

	/* ���{��󔒂��C���f���g */
	m_Types.m_bAutoIndent_ZENSPACE = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_INDENT_WSPACE );

	/* 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜 */
	m_Types.m_bRTrimPrevLine = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_RTRIM_PREVLINE );

	/* �X�}�[�g�C���f���g��� */
//	HWND	hwndCombo;
//	int		nSelPos;
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_SMARTINDENT );
	nSelPos = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
	m_Types.m_nSmartIndent = SmartIndentArr[nSelPos].nMethod;	/* �X�}�[�g�C���f���g��� */

	/* �܂�Ԃ���2�s�ڈȍ~���������\�� */
	//	Oct. 1, 2002 genta �R���{�{�b�N�X�ɕύX
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_INDENTLAYOUT );
	nSelPos = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
	m_Types.m_nIndentLayout = IndentTypeArr[nSelPos].nMethod;	/* �܂�Ԃ����C���f���g��� */

	//	From Here Nov. 20, 2000 genta	IME���
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
	nSelPos = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
	m_Types.m_nImeState = ImeStateArr[nSelPos].nMethod << 2;	//	IME���̓��[�h

	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
	nSelPos = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
	m_Types.m_nImeState |= ImeSwitchArr[nSelPos].nMethod;	//	IME ON/OFF
	//	To Here Nov. 20, 2000 genta

	/* �p�����[�h���b�v������ */
	m_Types.m_bWordWrap = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_WORDWRAP );

	/* �֑����� */
	{	//@@@ 2002.04.08 MIK start
		m_Types.m_bKinsokuHead = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUHEAD ) ? TRUE : FALSE;
		m_Types.m_bKinsokuTail = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUTAIL ) ? TRUE : FALSE;
		m_Types.m_bKinsokuRet  = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKURET  ) ? TRUE : FALSE;	/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
		m_Types.m_bKinsokuKuto = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUKUTO ) ? TRUE : FALSE;	/* ��Ǔ_���Ԃ牺���� */	//@@@ 2002.04.17 MIK
		::GetDlgItemText( hwndDlg, IDC_EDIT_KINSOKUHEAD, m_Types.m_szKinsokuHead, sizeof( m_Types.m_szKinsokuHead ) );
		::GetDlgItemText( hwndDlg, IDC_EDIT_KINSOKUTAIL, m_Types.m_szKinsokuTail, sizeof( m_Types.m_szKinsokuTail ) );
	}	//@@@ 2002.04.08 MIK end

	//	Sep. 10, 2002 genta
	m_Types.m_bUseDocumentIcon = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DOCICON ) ? TRUE : FALSE;

	return TRUE;
}



// 2001/06/13 Start By asa-o: �^�C�v�ʐݒ�̎x���^�u�Ɋւ��鏈��

/* p2 ���b�Z�[�W���� */
INT_PTR CPropTypes::DispatchEvent_p2(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p2 */
		SetData_p2( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		/* ���͕⊮ �P��t�@�C�� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_HOKANFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );
		/* �L�[���[�h�w���v �����t�@�C�� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_KEYWORDHELPFILE ), EM_LIMITTEXT, (WPARAM)(_MAX_PATH - 1 ), 0 );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
//		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			/* �_�C�A���O�f�[�^�̎擾 p2 */
			GetData_p2( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_HOKANFILE_REF:	/* ���͕⊮ �P��t�@�C���́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					char			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
					if( _IS_REL_PATH( m_Types.m_szHokanFile ) ){
						GetExecutableDir( szPath, m_Types.m_szHokanFile );
					}else{
						strcpy( szPath, m_Types.m_szHokanFile );
					}
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						"*.*",
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Types.m_szHokanFile, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENHELP:	/* �O���w���v�P�́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					char			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
					if( _IS_REL_PATH( m_Types.m_szExtHelp ) ){
						GetExecutableDir( szPath, m_Types.m_szExtHelp );
					}else{
						strcpy( szPath, m_Types.m_szExtHelp );
					}
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						"*.hlp",
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Types.m_szExtHelp, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp );
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENEXTHTMLHELP:	/* �O��HTML�w���v�́u�Q��...�v�{�^�� */
				{
					CDlgOpenFile	cDlgOpenFile;
					char			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
					if( _IS_REL_PATH( m_Types.m_szExtHtmlHelp ) ){
						GetExecutableDir( szPath, m_Types.m_szExtHtmlHelp );
					}else{
						strcpy( szPath, m_Types.m_szExtHtmlHelp );
					}
					/* �t�@�C���I�[�v���_�C�A���O�̏����� */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						"*.chm;*.col",
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						strcpy( m_Types.m_szExtHtmlHelp, szPath );
						::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp );
					}
				}
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
//		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
//		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:	//Jul. 03, 2001 JEPRO �x���^�u�̃w���v��L����
			OnHelp( hwndDlg, IDD_PROPTYPESP2 );
			return TRUE;
		case PSN_KILLACTIVE:
			/* �_�C�A���O�f�[�^�̎擾 p2 */
			GetData_p2( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
		case PSN_SETACTIVE:
			m_nPageNum = 2;
			return TRUE;
		}
		break;

//From Here Jul. 05, 2001 JEPRO: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids3 );
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//To Here  Jul. 05, 2001

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids3 );
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}

/* �_�C�A���O�f�[�^�̐ݒ� p2 */
void CPropTypes::SetData_p2( HWND hwndDlg )
{
	/* ���͕⊮ �P��t�@�C�� */
	::SetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );

//	2001/06/19 asa-o
	/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANLOHICASE, m_Types.m_bHokanLoHiCase );

	// 2003.06.25 Moca �t�@�C������̕⊮�@�\
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANBYFILE, m_Types.m_bUseHokanByFile );

	//@@@ 2002.2.2 YAZAKI
	::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp );
	::SetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp );
	::CheckDlgButton( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE, m_Types.m_bHtmlHelpIsSingle );
	return;
}

/* �_�C�A���O�f�[�^�̎擾 p2 */
int CPropTypes::GetData_p2( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = 2;

//	2001/06/19	asa-o
	/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
	m_Types.m_bHokanLoHiCase = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANLOHICASE );

	m_Types.m_bUseHokanByFile = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANBYFILE );

	/* ���͕⊮ �P��t�@�C�� */
	::GetDlgItemText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile, sizeof( m_Types.m_szHokanFile ));

	//@@@ 2002.2.2 YAZAKI
	::GetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHELP, m_Types.m_szExtHelp, sizeof( m_Types.m_szExtHelp ));
	::GetDlgItemText( hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, m_Types.m_szExtHtmlHelp, sizeof( m_Types.m_szExtHtmlHelp ));
	m_Types.m_bHtmlHelpIsSingle = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE );
	return TRUE;
}

// 2001/06/13 End




/* �F�̐ݒ���C���|�[�g */
void CPropTypes::p3_Import_Colors( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char*			pszMRU = NULL;;
	char*			pszOPENFOLDER = NULL;;
	char			szPath[_MAX_PATH + 1];
	HFILE			hFile;
//	char			szLine[1024];
//	int				i;
	int				i;

	char			pHeader[1024];
	int				nColorInfoArrNum;						/* �L�[���蓖�ĕ\�̗L���f�[�^�� */
	ColorInfo		ColorInfoArr[64];
	char			szInitDir[_MAX_PATH + 1];
//	HWND			hwndCtrl;
	CProfile		cProfile;

	cProfile.SetReadingMode();

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* �C���|�[�g�p�t�H���_ */

	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.col",
		szInitDir,
		(const char **)&pszMRU,
		(const char **)&pszOPENFOLDER
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );


	/* �F�ݒ�Ver1�� */
	hFile = _lopen( szPath, OF_READ );
	if( HFILE_ERROR == hFile ){
		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "�t�@�C�����J���܂���ł����B\n\n%s", szPath );
		return;
	}
//	if( STR_COLORDATA_HEAD_LEN		== _lread( hFile, pHeader, STR_COLORDATA_HEAD_LEN )
//	 && sizeof( nColorInfoArrNum )	== _lread( hFile, &nColorInfoArrNum, sizeof( nColorInfoArrNum ) )
//	 && 0 == memcmp( pHeader, STR_COLORDATA_HEAD, STR_COLORDATA_HEAD_LEN )
//	){
//		int nWorkWork = sizeof( ColorInfoArr[0] );
//
//		for( i = 0; i < nColorInfoArrNum && i < m_Types.m_nColorInfoArrNum; ++i ){
//			if( sizeof( ColorInfoArr[i] ) != _lread( hFile,  &ColorInfoArr[i], sizeof( ColorInfoArr[i] ) ) ){
//				::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//					"�F�ݒ�t�@�C���̌`�����Ⴂ�܂��B\n\n%s", szPath
//				);
//				_lclose( hFile );
//				return;
//			}
//		}
//		_lclose( hFile );
//		goto complete;
//	}else{
		/* �t�@�C���擪 */
		_llseek( hFile, 0, FILE_BEGIN );
		char	szWork[256];
		int		nWorkLen;
//		wsprintf( szWork, "//%s\r\n", STR_COLORDATA_HEAD2 );
#ifndef STR_COLORDATA_HEAD3
		wsprintf( szWork, "//%s\r\n", STR_COLORDATA_HEAD21 );	//Nov. 2, 2000 JEPRO �ύX [��]. 0.3.9.0:ur3��10�ȍ~�A�ݒ荀�ڂ̔ԍ������ւ�������
#else
		wsprintf( szWork, "//%s\r\n", STR_COLORDATA_HEAD3 );	//Jan. 15, 2001 Stonee
#endif
		nWorkLen = strlen( szWork );
		if( nWorkLen == (int)_lread( hFile, pHeader, nWorkLen ) &&
			0 == memcmp( pHeader, szWork, nWorkLen )
		){
		}else{
			::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//				"�F�ݒ�t�@�C���̌`�����Ⴂ�܂��B\n�Â��`���̓T�|�[�g����Ȃ��Ȃ�܂����B\n%s", szPath
//				Nov. 2, 2000 JEPRO �ύX [��]. 0.3.9.0:ur3��10�ȍ~�A�ݒ荀�ڂ̔ԍ������ւ�������
//				Dec. 26, 2000 JEPRO UR1.2.24.0�ŋ����L�[���[�h2�������Ă�������CI[13]���ǉ����ꂽ. ����ɔ���13�Ԉȍ~��1�Â炵��
//				"�F�ݒ�t�@�C���̌`�����Ⴂ�܂��B\n�Â��`���̓T�|�[�g����Ȃ��Ȃ�܂����B\n%s\n\n"
				"�F�ݒ�t�@�C���̌`�����Ⴂ�܂��B�Â��`���̓T�|�[�g����Ȃ��Ȃ�܂����B\n%s\n\n"	//Jan. 20, 2001 JEPRO ���s��1�����
#ifdef STR_COLORDATA_HEAD3
//				"�F�ݒ�t�@�C���̕ύX���e�̓w���v���������������B"	//Jan. 15, 2001 Stonee added	//Jan. 20, 2001 JEPRO killed
// From Here Jan. 20, 2001 JEPRO �������I�[�o�[�̂��߃R�����g�A�E�g�I
//				"���݂̐F�ݒ�Ver3�ł� CI[�C���f�b�N�X�ԍ�] ���� C[�C���f�b�N�X��] �Ɏd�l���ύX����܂��� (CI��C �ɒ���)�B\n"
//				"��L�̐F�ݒ�t�@�C���̐ݒ���e�𗘗p�������ꍇ�́A���̃t�@�C�����R�s�[���G�f�B�^��\n"
//				"�ȉ��̏C�����s���Ă���C���|�[�g���Ă��������B\n\n"
//				"�EUR1.2.24.0 (2000/12/04) �ȍ~�Ŏg���Ă����ꍇ��\n"
//				"  (1) ��s�ڂɏ����Ă��� Ver2 (or 2.1) �� Ver3 �Ə��������ACI �����ׂ� C �ɏk�߂�\n"
//				"  (2) (1)�ɉ����āA�C���f�b�N�X�ԍ���( )���̕�����ɕύX:\n"
//				"      00(TXT), 01(RUL), 02(UND), 03(LNO), 04(MOD), 05(TAB), 06(ZEN), 07(CTL), 08(EOL),\n"
//				"      09(RAP), 10(EOF), 11(FND), 12(KW1), 13(KW2), 14(CMT), 15(SQT), 16(WQT), 17(URL)\n\n"
//				"�Eur3��10 (2000/09/28)�`UR1.2.23.0 (2000/11/29) �Ŏg���Ă����ꍇ��\n"
//				"  (3) (1)�ɉ����āA�C���f�b�N�X�ԍ���( )���̕�����ɕύX:\n"
//				"      00(TXT), 01(RUL), 02(UND), 03(LNO), 04(MOD), 05(TAB), 06(ZEN), 07(CTL), 08(EOL),\n"
//				"      09(RAP), 10(EOF), 11(FND), 12(KW1), 13(CMT), 14(SQT), 15(WQT), 16(URL)\n\n"
//				"  (4) (1)�ɉ����āA�ԍ���( )���̕�����ɕύX:\n"
//				"      00(TXT), 01(LNO), 02(EOL), 03(TAB), 04(ZEN), 05(EOF), 06(KW1), 07(CMT), 08(SQT),\n"
//				"      09(WQT), 10(UND), 11(RAP), 12(CTL), 13(URL), 14(FND), 15(MOD), 16(RUL)\n\n"
// To Here Jan. 20, 2001
// From Here Jan. 21, 2001 JEPRO
				"�F�ݒ�Ver3�ł� CI[�ԍ�] ���� C[���O] �ɕύX����܂����B\n"
				"��L�t�@�C���̐ݒ���e�𗘗p�������ꍇ�́A�ȉ��̏C�����s���Ă���C���|�[�g���Ă��������B\n\n"
				"�EUR1.2.24.0 (00/12/04) �ȍ~�Ŏg���Ă����ꍇ��\n"
				"  (1) ��s�ڂ� Ver3 �Ə��������ACI �����ׂ� C �ɏk�߂�\n"
				"  (2) (1)�̌�A�ԍ���( )���̕�����ɕύX:\n"
				"      00(TXT), 01(RUL), 02(UND), 03(LNO), 04(MOD), 05(TAB), 06(ZEN), 07(CTL), 08(EOL),\n"
				"      09(RAP), 10(EOF), 11(FND), 12(KW1), 13(KW2), 14(CMT), 15(SQT), 16(WQT), 17(URL)\n\n"
				"�Eur3��10 (00/09/28)�`UR1.2.23.0 (00/11/29) �Ŏg���Ă����ꍇ��\n"
				"  (3) (1)�̌�A00-12 �܂ł�(2)�Ɠ���  13(CMT), 14(SQT), 15(WQT), 16(URL)\n\n"
				"�Eur3��9 (00/09/26) �ȑO�Ŏg���Ă����ꍇ��\n"
				"  (4) (1)�̌�A(2)�Ɠ��l:\n"
				"      00(TXT), 01(LNO), 02(EOL), 03(TAB), 04(ZEN), 05(EOF), 06(KW1), 07(CMT), 08(SQT),\n"
				"      09(WQT), 10(UND), 11(RAP), 12(CTL), 13(URL), 14(FND), 15(MOD), 16(RUL)\n\n"
// To Here Jan. 21, 2001
#else
// From Here Nov. 2, Dec. 26, 2000, Dec. 26, 2000 �ǉ�, Jan. 21, 2001 �C�� JEPRO
				"���݂̐F�ݒ�Ver2.1�ł�Ver2�ł̎d�l���ꕔ�ύX����܂����B\n"
				"��L�̃t�@�C���̐ݒ���e�𗘗p�������ꍇ�́A���̃t�@�C�����R�s�[���G�f�B�^��\n"
				"�ȉ��̏C�����s���Ă���C���|�[�g���Ă��������B\n\n"
				"�E0.3.9.0 UR1.2.24.0 (2000/12/04) �ȍ~�Ŏg���Ă����ꍇ��\n"
				"  (1) ��s�ڂɏ�����Ă��� Ver2 �� Ver2.1 �Ə���������\n\n"
				"�E0.3.9.0:ur3��10 (2000/09/28)�`UR1.2.23.0 (2000/11/29) �Ŏg���Ă����ꍇ��\n"
				"  (2) (1)�ɉ����� CI[12] �̎��s�� CI[12] �̐ݒ���R�s�[���� CI[13] �Ƃ�\n"
				"      ���� CI[13] �ȍ~�̔ԍ���1�Â�17�܂ł��炷\n\n"
				"�E0.3.9.0:ur3��9 (2000/09/26) �ȑO�Ŏg���Ă����ꍇ��\n"
				"  (3) (1)�ɉ����� CI ��[ ]��������\n"
				"      00, 17, 10, 01, 16, 03, 04, 12, 02, 11, 05, 15, 06, 07, 08, 09, 14\n"
				"      �Ə�����������ACI[12] �̐ݒ���R�s�[���� CI[13] �Ƃ���\n\n"
// To Here Nov. 2, Dec. 26, 2000, Jan. 21, 2001 JEPRO
#endif
				, szPath
			);
			_lclose( hFile );
			return;
		}
//	}
	_lclose( hFile );


	/* �F�ݒ�Ver2 */
	nColorInfoArrNum = COLORIDX_LAST;
	if( false == cProfile.ReadProfile( szPath ) ){
		/* �ݒ�t�@�C�������݂��Ȃ� */
		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�t�@�C�����J���܂���ł����B\n\n%s", szPath
		);
		return;
	}
	/* �F�ݒ� I/O */
	for( i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
		ColorInfoArr[i] = m_Types.m_ColorInfoArr[i];
		strcpy( ColorInfoArr[i].m_szName, m_Types.m_ColorInfoArr[i].m_szName );
	}
	CShareData::IO_ColorSet( &cProfile, STR_COLORDATA_SECTION, ColorInfoArr );


//complete:;
	/* �f�[�^�̃R�s�[ */
	m_Types.m_nColorInfoArrNum = nColorInfoArrNum;
	for( i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
		m_Types.m_ColorInfoArr[i] =  ColorInfoArr[i];
		strcpy( m_Types.m_ColorInfoArr[i].m_szName, ColorInfoArr[i].m_szName );
	}
	/* �_�C�A���O�f�[�^�̐ݒ� p5 */
	SetData_p3_new( hwndDlg );
	return;
}


/* �F�̐ݒ���G�N�X�|�[�g */
void CPropTypes::p3_Export_Colors( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char*			pszMRU = NULL;;
	char*			pszOPENFOLDER = NULL;;
	char			szPath[_MAX_PATH + 1];
//	HFILE			hFile;
//	int				i;
	char			szInitDir[_MAX_PATH + 1];
//	char			szLine[1024];
//	int				i;
//	char			pHeader[STR_KEYDATA_HEAD_LEN + 1];
//	short			m_nColorInfoArrNum;				/* �L�[���蓖�ĕ\�̗L���f�[�^�� */
//	KEYDATA			pKeyNameArr[100];				/* �L�[���蓖�ĕ\ */
	CProfile		cProfile;

	cProfile.SetWritingMode();

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* �C���|�[�g�p�t�H���_ */

	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.col",
		szInitDir,
		(const char **)&pszMRU,
		(const char **)&pszOPENFOLDER
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );

//	hFile = _lcreat( szPath, 0 );
//	if( HFILE_ERROR == hFile ){
//		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//			"�t�@�C�����J���܂���ł����B\n\n%s", szPath
//		);
//		return;
//	}
//	if( STR_COLORDATA_HEAD_LEN					!= _lwrite( hFile, (LPCSTR)STR_COLORDATA_HEAD, STR_COLORDATA_HEAD_LEN )
//	 || sizeof( m_Types.m_nColorInfoArrNum )	!= _lwrite( hFile, (LPCSTR)&m_Types.m_nColorInfoArrNum, sizeof( m_Types.m_nColorInfoArrNum ) )
//	){
//		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//			"�t�@�C���̏������݂Ɏ��s���܂����B\n\n%s", szPath
//		);
//		return;
//	}
//	for( i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
//		if( sizeof( m_Types.m_ColorInfoArr[i] ) != _lwrite( hFile, (LPCSTR)&m_Types.m_ColorInfoArr[i], sizeof( m_Types.m_ColorInfoArr[i] ) ) ){
//			::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//				"�t�@�C���̏������݂Ɏ��s���܂����B\n\n%s", szPath
//			);
//			return;
//		}
//	}
//	_lclose( hFile );

	/* �F�ݒ� I/O */
	CShareData::IO_ColorSet( &cProfile, STR_COLORDATA_SECTION, m_Types.m_ColorInfoArr );
//	cProfile.WriteProfile( szPath, STR_COLORDATA_HEAD2 );
#ifndef STR_COLORDATA_HEAD3
	cProfile.WriteProfile( szPath, STR_COLORDATA_HEAD21 );	//Nov. 2, 2000 JEPRO �ύX [��]. 0.3.9.0:ur3��10�ȍ~�A�ݒ荀�ڂ̔ԍ������ւ�������
#else
	cProfile.WriteProfile( szPath, STR_COLORDATA_HEAD3 );	//Jan. 15, 2001 Stonee
#endif
	return;
}


LRESULT APIENTRY ColorList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	WPARAM		fwKeys;	// key flags
//	int			xPos;	// horizontal position of cursor
//	int			yPos;	// vertical position of cursor
//	HWND		hwndLoseFocus;
//	HWND		hwndGetFocus;
//	int			fwKeys;
	int			xPos;
	int			yPos;
	int			nIndex;
	int			nItemNum;
	RECT		rcItem;
	int			i;
	POINT		poMouse;
	ColorInfo*	pColorInfo;

	switch( uMsg ){
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONUP:
//		fwKeys = wParam;		// key flags
		xPos = LOWORD(lParam);	// horizontal position of cursor
		yPos = HIWORD(lParam);	// vertical position of cursor
//		nIndex = ::SendMessage( hwnd, LB_GETCURSEL, 0, 0 );
//		MYTRACE( "fwKeys=%d\n", fwKeys );
//		MYTRACE( "xPos  =%d\n", xPos );
//		MYTRACE( "yPos  =%d\n", yPos );
//		MYTRACE( "nIndex=%d\n", nIndex );
//		MYTRACE( "\n" );
		poMouse.x = xPos;
		poMouse.y = yPos;
		nItemNum = ::SendMessage( hwnd, LB_GETCOUNT, 0, 0 );
		nIndex = -1;
		for( i = 0; i < nItemNum; ++i ){
			::SendMessage( hwnd, LB_GETITEMRECT, i, (LPARAM)&rcItem );
			if( ::PtInRect( &rcItem, poMouse ) ){
//				MYTRACE( "hit at i==%d\n", i );
//				MYTRACE( "\n" );
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
		::SendMessage( hwnd, LB_SETCURSEL, nIndex, 0 );
		::SendMessage( ::GetParent( hwnd ), WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwnd );
		pColorInfo = (ColorInfo*)::SendMessage( hwnd, LB_GETITEMDATA, nIndex, 0 );
		/* ���� */
		if( nIndex != COLORIDX_UNDERLINE	/* �J�[�\���s�A���_�[���C�� */
		 && nIndex != COLORIDX_TEXT			/* �e�L�X�g */
		 && nIndex != COLORIDX_RULER		/* ���[���[ */
		){
			if( pColorInfo->m_bUnderLine ){	/* �����ŕ\�� */
				pColorInfo->m_bUnderLine = FALSE;
				::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_UNDERLINE, FALSE );
			}else{
				pColorInfo->m_bUnderLine = TRUE;
				::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_UNDERLINE, TRUE );
			}
			::InvalidateRect( hwnd, &rcItem, TRUE );
		}
		break;

	case WM_LBUTTONDBLCLK:
		if( -1 == nIndex ){
			break;
		}
		pColorInfo = (ColorInfo*)::SendMessage( hwnd, LB_GETITEMDATA, nIndex, 0 );
		/* �����ŕ\�� */
		if( nIndex != COLORIDX_UNDERLINE	/* �J�[�\���s�A���_�[���C�� */
		 && nIndex != COLORIDX_TEXT			/* �e�L�X�g */
		 && nIndex != COLORIDX_RULER		/* ���[���[ */
		){
			if( pColorInfo->m_bFatFont ){	/* �����ŕ\�� */
				pColorInfo->m_bFatFont = FALSE;
				::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_FAT, FALSE );
			}else{
				pColorInfo->m_bFatFont = TRUE;
				::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_FAT, TRUE );
			}
			::InvalidateRect( hwnd, &rcItem, TRUE );
		}
		break;
	case WM_LBUTTONUP:
		if( -1 == nIndex ){
			break;
		}
		pColorInfo = (ColorInfo*)::SendMessage( hwnd, LB_GETITEMDATA, nIndex, 0 );
		/* �F����/�\�� ���� */
		if( 2 <= xPos && xPos <= 16 && nIndex != COLORIDX_TEXT ){	/* �e�L�X�g */
			if( pColorInfo->m_bDisp ){	/* �F����/�\������ */
				pColorInfo->m_bDisp = FALSE;
			}else{
				pColorInfo->m_bDisp = TRUE;
			}
			if( COLORIDX_GYOU == nIndex ){
				pColorInfo = (ColorInfo*)::SendMessage( hwnd, LB_GETITEMDATA, nIndex, 0 );

			}

			::InvalidateRect( hwnd, &rcItem, TRUE );
		}else
		/* �O�i�F���{ ��` */
		if( rcItem.right - 27 <= xPos && xPos <= rcItem.right - 27 + 12 ){
			/* �F�I���_�C�A���O */
			// 2005.11.30 Moca �J�X�^���F�ێ�
			DWORD* pColors = (DWORD*)::GetProp( hwnd, "ptrCustomColors" );
			if( CPropTypes::SelectColor( hwnd, &pColorInfo->m_colTEXT, pColors ) ){
				::InvalidateRect( hwnd, &rcItem, TRUE );
				::InvalidateRect( ::GetDlgItem( ::GetParent( hwnd ), IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
			}
		}else
		/* �O�i�F���{ ��` */
		if( rcItem.right - 13 <= xPos && xPos <= rcItem.right - 13 + 12 && nIndex != COLORIDX_UNDERLINE ){	/* �J�[�\���s�A���_�[���C�� */
			/* �F�I���_�C�A���O */
			// 2005.11.30 Moca �J�X�^���F�ێ�
			DWORD* pColors = (DWORD*)::GetProp( hwnd, "ptrCustomColors" );
			if( CPropTypes::SelectColor( hwnd, &pColorInfo->m_colBACK, pColors ) ){
				::InvalidateRect( hwnd, &rcItem, TRUE );
				::InvalidateRect( ::GetDlgItem( ::GetParent( hwnd ), IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
			}
		}
		break;
	// 2005.11.30 Moca �J�X�^���F�ێ�
	case WM_DESTROY:
		if( ::GetProp( hwnd, "ptrCustomColors" ) ){
			::RemoveProp( hwnd, "ptrCustomColors" );
		}
		break;
	}
//	return CallWindowProc( (int (__stdcall *)(void))(WNDPROC)m_wpColorListProc, hwnd, uMsg, wParam, lParam );
	return CallWindowProc( (WNDPROC)m_wpColorListProc, hwnd, uMsg, wParam, lParam );
}





/* p3 ���b�Z�[�W���� */
INT_PTR CPropTypes::DispatchEvent_p3_new(
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

		/* �_�C�A���O�f�[�^�̐ݒ� p3 */
		SetData_p3_new( hwndDlg );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), EM_LIMITTEXT, (WPARAM)1, 0 );


		/* �F���X�g���t�b�N */
		// Modified by KEITA for WIN64 2003.9.6
		m_wpColorListProc = (WNDPROC) ::SetWindowLongPtr( hwndListColor, GWLP_WNDPROC, (LONG_PTR)ColorList_SubclassProc );
		// 2005.11.30 Moca �J�X�^���F��ێ�
		::SetProp( hwndListColor, "ptrCustomColors", m_dwCustColors );
		
		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD( wParam );	/* �ʒm�R�[�h */
		wID			= LOWORD( wParam );	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		if( hwndListColor == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = ::SendMessage( hwndListColor, LB_GETCURSEL, 0, 0 );
				m_nCurrentColorType = nIndex;		/* ���ݑI������Ă���F�^�C�v */
				switch( m_nCurrentColorType ){
				case COLORIDX_UNDERLINE:	/* �J�[�\���s�A���_�[���C�� */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ), FALSE );

					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), FALSE );


//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), SW_HIDE );
					break;
				case COLORIDX_TEXT:	/* �e�L�X�g */
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ), FALSE );

					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), TRUE );

//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), SW_SHOW );
					break;
//				case COLORIDX_CRLF:	/* CRLF */
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), SW_HIDE );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), SW_SHOW );
//					break;
				default:
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), TRUE );

//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ), SW_SHOW );
//					::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ), SW_SHOW );
					break;
				}


				/* �F����/�\�� ������ */
				if( m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp ){
					::CheckDlgButton( hwndDlg, IDC_CHECK_DISP, TRUE );
				}else{
					::CheckDlgButton( hwndDlg, IDC_CHECK_DISP, FALSE );
				}
				/* �����ŕ\�� */
				if( m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bFatFont ){
					::CheckDlgButton( hwndDlg, IDC_CHECK_FAT, TRUE );
				}else{
					::CheckDlgButton( hwndDlg, IDC_CHECK_FAT, FALSE );
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
				::SendMessage( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_BUTTON_BACKCOLOR:	/* �w�i�F */
				/* �F�I���_�C�A���O */
				if( SelectColor( hwndDlg, &m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK, m_dwCustColors ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
				}
				/* ���ݑI������Ă���F�^�C�v */
				::SendMessage( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_CHECK_DISP:	/* �F����/�\�� ������ */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp = TRUE;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp = FALSE;
				}
				/* ���ݑI������Ă���F�^�C�v */
				::SendMessage( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK ���K�\���L�[���[�h�̂���
				return TRUE;
			case IDC_CHECK_FAT:	/* ������ */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_FAT ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bFatFont = TRUE;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bFatFont = FALSE;
				}
				/* ���ݑI������Ă���F�^�C�v */
				::SendMessage( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_CHECK_UNDERLINE:	/* ������\�� */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_UNDERLINE ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine = TRUE;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine = FALSE;
				}
				/* ���ݑI������Ă���F�^�C�v */
				::SendMessage( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;

			case IDC_BUTTON_IMPORT:	/* �F�̐ݒ���C���|�[�g */
				p3_Import_Colors( hwndDlg );
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK ���K�\���L�[���[�h�̂���
				return TRUE;

			case IDC_BUTTON_EXPORT:	/* �F�̐ݒ���G�N�X�|�[�g */
				p3_Export_Colors( hwndDlg );
				return TRUE;

			//	From Here Sept. 10, 2000 JEPRO
			//	�s�ԍ���؂��C�ӂ̔��p�����ɂ���Ƃ������w�蕶�����͂�Enable�ɐݒ�
			case IDC_RADIO_LINETERMTYPE0: /* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
			case IDC_RADIO_LINETERMTYPE1:
			case IDC_RADIO_LINETERMTYPE2:
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
					int nIdx = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIdx || 0 == nIdx ){
						m_nSet[ 0 ] = -1;
					}else{
						m_nSet[ 0 ] = nIdx - 1;
					}
					cDlgKeywordSelect.DoModal( ::GetModuleHandle(NULL), hwndDlg, m_nSet );
					RearrangeKeywordSet( hwndDlg );	//	Jan. 23, 2005 genta �L�[���[�h�Z�b�g�Ĕz�u
					//�����L�[���[�h1�𔽉f����B
					if( -1 == m_nSet[ 0 ] ){
						::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)0, 0 );
					}else{
						::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)m_nSet[ 0 ] + 1, 0 );
					}
				}
				break;
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
//				MYTRACE( "p3 PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 p3 */
				GetData_p3_new( hwndDlg );
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
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids2 );
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids2 );
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}





/* �_�C�A���O�f�[�^�̐ݒ� p3 */
void CPropTypes::SetData_p3_new( HWND hwndDlg )
{

	HWND	hwndWork;
	int		i;
	int		nItem;

	m_nCurrentColorType = 0;	/* ���ݑI������Ă���F�^�C�v */

	/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */	//@@@ 2002.09.22 YAZAKI
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT )		, EM_LIMITTEXT, (WPARAM)( COMMENT_DELIMITER_BUFFERSIZE - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT2 )		, EM_LIMITTEXT, (WPARAM)( COMMENT_DELIMITER_BUFFERSIZE - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINECOMMENT3 )		, EM_LIMITTEXT, (WPARAM)( COMMENT_DELIMITER_BUFFERSIZE - 1 ), 0 );	//Jun. 01, 2001 JEPRO �ǉ�
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM )	, EM_LIMITTEXT, (WPARAM)( BLOCKCOMMENT_BUFFERSIZE - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO )	, EM_LIMITTEXT, (WPARAM)( BLOCKCOMMENT_BUFFERSIZE - 1 ), 0 );
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2 )	, EM_LIMITTEXT, (WPARAM)( BLOCKCOMMENT_BUFFERSIZE - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2 )	, EM_LIMITTEXT, (WPARAM)( BLOCKCOMMENT_BUFFERSIZE - 1 ), 0 );
//#endif

	::SetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, m_Types.m_cBlockComment.getBlockCommentFrom(0) );	/* �u���b�N�R�����g�f���~�^(From) */
	::SetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, m_Types.m_cBlockComment.getBlockCommentTo(0) );	/* �u���b�N�R�����g�f���~�^(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	::SetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, m_Types.m_cBlockComment.getBlockCommentFrom(1) );	/* �u���b�N�R�����g�f���~�^2(From) */
	::SetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, m_Types.m_cBlockComment.getBlockCommentTo(1) );	/* �u���b�N�R�����g�f���~�^2(To) */
//#endif

	/* �s�R�����g�f���~�^ @@@ 2002.09.22 YAZAKI*/
	//	From Here May 12, 2001 genta
	//	�s�R�����g�̊J�n���ʒu�ݒ�
	//	May 21, 2001 genta ���ʒu��1���琔����悤��
	for ( i=0; i<COMMENT_DELIMITER_NUM; i++ ){
		//	�e�L�X�g
		::SetDlgItemText( hwndDlg, cLineComment[i].nEditID, m_Types.m_cLineComment.getLineComment(i) );	

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

	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	if( FALSE == m_Types.m_bLineNumIsCRLF ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, TRUE );
	}


	/* �Z�b�g���R���{�{�b�N�X�̒l�Z�b�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	::SendMessage( hwndWork, CB_RESETCONTENT, 0, 0 );  /* �R���{�{�b�N�X����ɂ��� */
	/* ��s�ڂ͋� */
	::SendMessage( hwndWork, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)" " );
	//	Mar. 31, 2003 genta KeyWordSetMgr���|�C���^��
	if( 0 < m_pCKeyWordSetMgr->m_nKeyWordSetNum ){
		for( i = 0; i < m_pCKeyWordSetMgr->m_nKeyWordSetNum; ++i ){
			::SendMessage( hwndWork, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)m_pCKeyWordSetMgr->GetTypeName( i ) );
		}
		if( -1 == m_Types.m_nKeyWordSetIdx[0] ){
			/* �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I�� */
			::SendMessage( hwndWork, CB_SETCURSEL, (WPARAM)0, 0 );
		}else{
			/* �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I�� */
			::SendMessage( hwndWork, CB_SETCURSEL, (WPARAM)m_Types.m_nKeyWordSetIdx[0] + 1, 0 );
		}
	}

	//�����L�[���[�h1�`10�̐ݒ�
	for( i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		m_nSet[ i ] = m_Types.m_nKeyWordSetIdx[i];
	}

	/* �F�����镶����ނ̃��X�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_COLORS );
	::SendMessage( hwndWork, LB_RESETCONTENT, 0, 0 );  /* �R���{�{�b�N�X����ɂ��� */
	for( i = 0; i < COLORIDX_LAST; ++i ){
		nItem = ::SendMessage( hwndWork, LB_ADDSTRING, 0, (LPARAM)(char*)m_Types.m_ColorInfoArr[i].m_szName );
		::SendMessage( hwndWork, LB_SETITEMDATA, nItem, (LPARAM)(void*)&m_Types.m_ColorInfoArr[i] );
	}
	/* ���ݑI������Ă���F�^�C�v */
	::SendMessage( hwndWork, LB_SETCURSEL, m_nCurrentColorType, 0 );
	::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwndWork );

	/* �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C�� */
	if( 0 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE );
	}else
	if( 1 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE );
	}else
	if( 2 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, TRUE );
	}

	/* �s�ԍ���؂蕶�� */
	char	szLineTermChar[2];
	wsprintf( szLineTermChar, "%c", m_Types.m_cLineTermChar );
	::SetDlgItemText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar );

	//	From Here Sept. 10, 2000 JEPRO
	//	�s�ԍ���؂��C�ӂ̔��p�����ɂ���Ƃ������w�蕶�����͂�Enable�ɐݒ�
	EnableTypesPropInput( hwndDlg );
	//	To Here Sept. 10, 2000


	// from here 2005.11.30 Moca �w��ʒu�c���̐ݒ�
	TCHAR szVertLine[MAX_VERTLINES * 15] = "";
	int offset = 0;
	for( i = 0; i < MAX_VERTLINES && m_Types.m_nVertLineIdx[i] != 0; i++ ){
		int nXCol = m_Types.m_nVertLineIdx[i];
		int nXColEnd = nXCol;
		int nXColAdd = 1;
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
				offset += wsprintf( &szVertLine[offset], "%d(%d,%d)", nXColAdd, nXCol, nXColEnd );
			}
		}else{
			if(offset){
				szVertLine[offset] = ',';
				szVertLine[offset+1] = '\0';
				offset += 1;
			}
			offset += wsprintf( &szVertLine[offset], "%d", nXCol );
		}
	}
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_VERTLINE ), EM_LIMITTEXT, (WPARAM)(MAX_VERTLINES * 15), 0 );
	::SetDlgItemText( hwndDlg, IDC_EDIT_VERTLINE, szVertLine );
	// to here 2005.11.30 Moca �w��ʒu�c���̐ݒ�
	return;
}





/* �_�C�A���O�f�[�^�̎擾 p3 */
int CPropTypes::GetData_p3_new( HWND hwndDlg )
{
	int		nIdx;
	HWND	hwndWork;

//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = 1;

	//	From Here May 12, 2001 genta
	//	�R�����g�̊J�n���ʒu�̎擾
	//	May 21, 2001 genta ���ʒu��1���琔����悤��
	char buffer[COMMENT_DELIMITER_BUFFERSIZE];	//@@@ 2002.09.22 YAZAKI LineComment���擾���邽�߂̃o�b�t�@
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

		::GetDlgItemText( hwndDlg, cLineComment[i].nEditID		, buffer	, COMMENT_DELIMITER_BUFFERSIZE );		/* �s�R�����g�f���~�^ */
		m_Types.m_cLineComment.CopyTo( i, buffer, en ? pos : ~pos );
	}

	char szFromBuffer[BLOCKCOMMENT_BUFFERSIZE];	//@@@ 2002.09.22 YAZAKI
	char szToBuffer[BLOCKCOMMENT_BUFFERSIZE];	//@@@ 2002.09.22 YAZAKI

	::GetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, szFromBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* �u���b�N�R�����g�f���~�^(From) */
	::GetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, szToBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* �u���b�N�R�����g�f���~�^(To) */
	m_Types.m_cBlockComment.CopyTo( 0, szFromBuffer, szToBuffer );

	::GetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, szFromBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* �u���b�N�R�����g�f���~�^(From) */
	::GetDlgItemText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, szToBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* �u���b�N�R�����g�f���~�^(To) */
	m_Types.m_cBlockComment.CopyTo( 1, szFromBuffer, szToBuffer );

	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_ESCAPETYPE_1 ) ){
		m_Types.m_nStringType = 0;
	}else{
		m_Types.m_nStringType = 1;
	}
	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINENUM_LAYOUT ) ){
		m_Types.m_bLineNumIsCRLF = FALSE;
	}else{
		m_Types.m_bLineNumIsCRLF = TRUE;
	}

	/* �Z�b�g���R���{�{�b�N�X�̒l�Z�b�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	nIdx = ::SendMessage( hwndWork, CB_GETCURSEL, 0, 0 );
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

	/* �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C�� */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE0 ) ){
		m_Types.m_nLineTermType = 0;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE1 ) ){
		m_Types.m_nLineTermType = 1;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE2 ) ){
		m_Types.m_nLineTermType = 2;
	}


	/* �s�ԍ���؂蕶�� */
	char	szLineTermChar[2];
//	sprintf( szLineTermChar, "%c", m_Types.m_cLineTermChar );
	::GetDlgItemText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar, 2 );
	m_Types.m_cLineTermChar = szLineTermChar[0];


	// from here 2005.11.30 Moca �w��ʒu�c���̐ݒ�
	TCHAR szVertLine[MAX_VERTLINES * 15];
	::GetDlgItemText( hwndDlg, IDC_EDIT_VERTLINE, szVertLine, MAX_VERTLINES * 15 );

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
				valueEnd = MAXLINESIZE;
			}
			if( szVertLine[offset] != ')' ){
				break;
			}
			offset++;
			if(i + 2 < MAX_VERTLINES){
				m_Types.m_nVertLineIdx[i++] = -valueBegin;
				m_Types.m_nVertLineIdx[i++] = valueEnd;
				m_Types.m_nVertLineIdx[i++] = value;
			}else{
				break;
			}
		}else{
			m_Types.m_nVertLineIdx[i++] = value;
		}
		if( szVertLine[offset] != ',' ){
			break;
		}
		offset++;
	}
	if( i < MAX_VERTLINES ){
		m_Types.m_nVertLineIdx[i] = 0;
	}
	// to here 2005.11.30 Moca �w��ʒu�c���̐ݒ�
	return TRUE;
}





/* �F��ʃ��X�g �I�[�i�[�`�� */
void CPropTypes::DrawColorListItem( DRAWITEMSTRUCT* pDis )
{
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	HPEN		hPen;
	HPEN		hPenOld;
	ColorInfo*	pColorInfo;
//	RECT		rc0,rc1,rc2;
	RECT		rc1;
	COLORREF	cRim = (COLORREF)::GetSysColor( COLOR_3DSHADOW );

	if( pDis == NULL || pDis->itemData == NULL ) return;

//	rc0 = pDis->rcItem;
	rc1 = pDis->rcItem;
//	rc2 = pDis->rcItem;

	/* �A�C�e���f�[�^�̎擾 */
	pColorInfo = (ColorInfo*)pDis->itemData;

	/* �A�C�e����`�h��Ԃ� */
	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
	::FillRect( pDis->hDC, &pDis->rcItem, hBrush );
	::DeleteObject( hBrush );


	/* �A�C�e�����I������Ă��� */
	if( pDis->itemState & ODS_SELECTED ){
		hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_HIGHLIGHT ) );
		::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );
	}else{
		hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
		::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_WINDOWTEXT ) );
	}

//	/* �I���n�C���C�g��` */
//	::FillRect( pDis->hDC, &rc1, hBrush );
//	::DeleteObject( hBrush );
//	/* �e�L�X�g */
//	::SetBkMode( pDis->hDC, TRANSPARENT );
//	::TextOut( pDis->hDC, rc1.left, rc1.top, pColorInfo->m_szName, strlen( pColorInfo->m_szName ) );
//	if( pColorInfo->m_bFatFont ){	/* ������ */
//		::TextOut( pDis->hDC, rc1.left + 1, rc1.top, pColorInfo->m_szName, strlen( pColorInfo->m_szName ) );
//	}
//	return;


	rc1.left+= (2 + 16);
	rc1.top += 2;
	rc1.right -= ( 2 + 27 );
	rc1.bottom -= 2;
	/* �I���n�C���C�g��` */
	::FillRect( pDis->hDC, &rc1, hBrush );
	::DeleteObject( hBrush );
	/* �e�L�X�g */
	::SetBkMode( pDis->hDC, TRANSPARENT );
	::TextOut( pDis->hDC, rc1.left, rc1.top, pColorInfo->m_szName, strlen( pColorInfo->m_szName ) );
	if( pColorInfo->m_bFatFont ){	/* ������ */
		::TextOut( pDis->hDC, rc1.left + 1, rc1.top, pColorInfo->m_szName, strlen( pColorInfo->m_szName ) );
	}
	if( pColorInfo->m_bUnderLine ){	/* ������ */
		SIZE	sz;
		::GetTextExtentPoint32( pDis->hDC, pColorInfo->m_szName, strlen( pColorInfo->m_szName ), &sz );
		::MoveToEx( pDis->hDC, rc1.left,		rc1.bottom - 2, NULL );
		::LineTo( pDis->hDC, rc1.left + sz.cx,	rc1.bottom - 2 );
		::MoveToEx( pDis->hDC, rc1.left,		rc1.bottom - 1, NULL );
		::LineTo( pDis->hDC, rc1.left + sz.cx,	rc1.bottom - 1 );
	}

	/* �A�C�e���Ƀt�H�[�J�X������ */	// 2006.05.01 ryoji �`������̕s�����C��
	if( pDis->itemState & ODS_FOCUS ){
		::DrawFocusRect( pDis->hDC, &pDis->rcItem );
	}

	/* �u�F����/�\������v�̃`�F�b�N */
	rc1 = pDis->rcItem;
	rc1.left += 2;
	rc1.top += 3;
	rc1.right = rc1.left + 12;
	rc1.bottom = rc1.top + 12;
	if( pColorInfo->m_bDisp ){	/* �F����/�\������ */
		// 2006.04.26 ryoji �e�L�X�g�F���g���i�u�n�C�R���g���X�g���v�̂悤�Ȑݒ�ł�������悤�Ɂj
		hPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor( COLOR_WINDOWTEXT ) );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );

		::MoveToEx( pDis->hDC,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( pDis->hDC,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( pDis->hDC,	rc1.right - 2, rc1.top + 4 );
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx( pDis->hDC,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( pDis->hDC,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( pDis->hDC,	rc1.right - 2, rc1.top + 4 );
		rc1.top -= 1;
		rc1.bottom -= 1;
		::MoveToEx( pDis->hDC,	rc1.left + 2, rc1.top + 6, NULL );
		::LineTo( pDis->hDC,	rc1.left + 5, rc1.bottom - 3 );
		::LineTo( pDis->hDC,	rc1.right - 2, rc1.top + 4 );

		::SelectObject( pDis->hDC, hPenOld );
		::DeleteObject( hPen );
	}
//	return;


	// 2002/11/02 Moca ��r���@�ύX
//	if( 0 != strcmp( "�J�[�\���s�A���_�[���C��", pColorInfo->m_szName ) )
	if( COLORIDX_UNDERLINE != pColorInfo->m_nColorIdx )
	{
		/* �w�i�F ���{��` */
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 13;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;

		hBrush = ::CreateSolidBrush( pColorInfo->m_colBACK );
		hBrushOld = (HBRUSH)::SelectObject( pDis->hDC, hBrush );
		hPen = ::CreatePen( PS_SOLID, 1, cRim );
		hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
		::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );
		::SelectObject( pDis->hDC, hPenOld );
		::SelectObject( pDis->hDC, hBrushOld );
		::DeleteObject( hPen );
		::DeleteObject( hBrush );
	}


	/* �O�i�F ���{��` */
	rc1 = pDis->rcItem;
	rc1.left = rc1.right - 27;
	rc1.top += 2;
	rc1.right = rc1.left + 12;
	rc1.bottom -= 2;
	hBrush = ::CreateSolidBrush( pColorInfo->m_colTEXT );
	hBrushOld = (HBRUSH)::SelectObject( pDis->hDC, hBrush );
	hPen = ::CreatePen( PS_SOLID, 1, cRim );
	hPenOld = (HPEN)::SelectObject( pDis->hDC, hPen );
	::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );
	::SelectObject( pDis->hDC, hPenOld );
	::SelectObject( pDis->hDC, hBrushOld );
	::DeleteObject( hPen );
	::DeleteObject( hBrush );





//	::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_WINDOWTEXT ) );
//	::SetBkColor( pDis->hDC, ::GetSysColor( COLOR_WINDOW ) );
//	::TextOut( pDis->hDC, pDis->rcItem.left + 3, pDis->rcItem.top + 3, pColorInfo->m_szName, strlen( pColorInfo->m_szName ) );

//	::SetTextColor( pDis->hDC, pColorInfo->m_colTEXT );
//	::SetBkColor( pDis->hDC, pColorInfo->m_colBACK );
//	::TextOut( pDis->hDC, pDis->rcItem.left + 3 + 128, pDis->rcItem.top + 3, gpColorInfo->m_szName, strlen( pColorInfo->m_szName ) );
	return;
}


/* �w���v */
//Stonee, 2001/05/18 �@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
void CPropTypes::OnHelp( HWND hwndParent, int nPageID )
{
	int		nContextID;
	switch( nPageID ){
	case IDD_PROPTYPESP1:
		nContextID = ::FuncID_To_HelpContextID(F_TYPE_SCREEN);
		break;
//	Sept. 10, 2000 JEPRO ID�������ۂ̖��O�ɕύX���邽�߈ȉ��̍s�̓R�����g�A�E�g
//	case IDD_PROP1P3:
	case IDD_PROP_COLOR:
		nContextID = ::FuncID_To_HelpContextID(F_TYPE_COLOR);
		break;
//	Jul. 03, 2001 JEPRO �x���^�u�̃w���v��L����
		case IDD_PROPTYPESP2:
		nContextID = ::FuncID_To_HelpContextID(F_TYPE_HELPER);
		break;
//@@@ 2001.11.17 add start MIK
	case IDD_PROP_REGEX:
		nContextID = ::FuncID_To_HelpContextID(F_TYPE_REGEX_KEYWORD);
		break;
//@@@ 2001.11.17 add end MIK
	default:
		nContextID = -1;
		break;
	}
	if( -1 != nContextID ){
		::WinHelp( hwndParent, m_szHelpFile, HELP_CONTEXT, nContextID );
	}
	return;
}





//	From Here Sept. 10, 2000 JEPRO
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void CPropTypes::EnableTypesPropInput( HWND hwndDlg )
{
	//	�s�ԍ���؂��C�ӂ̔��p�����ɂ��邩�ǂ���
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE2 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LINETERMCHAR ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LINETERMCHAR ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), FALSE );
	}

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
void CPropTypes::RearrangeKeywordSet( HWND hwndDlg )
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
				colT.m_bFatFont		= col1.m_bFatFont;
				colT.m_bUnderLine	= col1.m_bUnderLine;
				colT.m_colTEXT		= col1.m_colTEXT;
				colT.m_colBACK		= col1.m_colBACK;

				col1.m_bDisp		= col2.m_bDisp;
				col1.m_bFatFont		= col2.m_bFatFont;
				col1.m_bUnderLine	= col2.m_bUnderLine;
				col1.m_colTEXT		= col2.m_colTEXT;
				col1.m_colBACK		= col2.m_colBACK;

				col2.m_bDisp		= colT.m_bDisp;
				col2.m_bFatFont		= colT.m_bFatFont;
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


/*[EOF]*/
