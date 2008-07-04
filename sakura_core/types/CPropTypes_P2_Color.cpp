/*
	�^�C�v�ʐݒ� - �J���[

	2008.04.12 kobake CPropTypes.cpp���番��
*/
#include "stdafx.h"
#include "CPropTypes.h"
#include "sakura.hh"
#include "io/CTextStream.h"
#include "CDlgSameColor.h"
#include "CDlgKeywordSelect.h"
#include "util/shell.h"
using namespace std;

WNDPROC	m_wpColorListProc;

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
//	IDC_COMBO_SET2,					HIDC_COMBO_SET2_COLOR,			//�����L�[���[�h�Q�Z�b�g��	del 2008/6/3 Uchi
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
void CPropTypes::_Import_Colors( HWND hwndDlg )
{
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	CDlgOpenFile	cDlgOpenFile;
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		_T("*.col"),
		GetDllShareData().m_szIMPORTFOLDER, // �C���|�[�g�p�t�H���_
		std::vector<LPCTSTR>(),
		std::vector<LPCTSTR>()
	);
	TCHAR	szPath[_MAX_PATH + 1] = _T("");
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}

	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, GetDllShareData().m_szIMPORTFOLDER, NULL );
	_tcscat( GetDllShareData().m_szIMPORTFOLDER, _T("\\") );


	/* �F�ݒ�Ver1�� */
	CTextInputStream in(szPath);
	if(!in){
		ErrorMessage(hwndDlg, _T("�t�@�C�����J���܂���ł����B\n\n%ts"), szPath );
		return;
	}

	/* �t�@�C���擪 */
	//�w�b�_�ǎ�
	wstring szHeader = in.ReadLineW().c_str();
	//if(szHeader.length()>=2)szHeader=&szHeader.c_str()[2]; //�R�����g�𔲂�
	if(szHeader.length()>=2) {
		//�R�����g�𔲂�	�R�����g���� �ύX�Ή� 2008/6/28 Uchi
		szHeader = &szHeader.c_str()[ szHeader.c_str()[0] == _T(';') ? 1 : 2];
	}
	//��r
	if(wcscmp(szHeader.c_str(),LTEXT(STR_COLORDATA_HEAD3))==0){
		//OK
	}
	else{
		ErrorMessage_A( hwndDlg,
			"�F�ݒ�t�@�C���̌`�����Ⴂ�܂��B�Â��`���̓T�|�[�g����Ȃ��Ȃ�܂����B\n%ts\n\n"	//Jan. 20, 2001 JEPRO ���s��1�����
			"�F�ݒ�Ver3�ł� CI[�ԍ�] ���� C[���O] �ɕύX����܂����B\n"
			"��L�t�@�C���̐ݒ���e�𗘗p�������ꍇ�́A�ȉ��̏C�����s���Ă���C���|�[�g���Ă��������B\n\n"
			"�EUR1.2.24.0 (00/12/04) �ȍ~�Ŏg���Ă����ꍇ��\n"
			"  (1) ��s�ڂ� Ver3 �Ə��������ACI �����ׂ� C �ɏk�߂�\n"
			"  (2) (1)�̌�A�ԍ���( )���̕�����ɕύX:\n"
			"      00(TXT), 01(RUL), 02(UND), 03(LNO), 04(MOD), 05(TAB), 06(ZEN), 07(CTL), 08(EOL),\n"
			"      09(RAP), 10(EOF), 11(FND), 12(KW1), 13(KW), 14(CMT), 15(SQT), 16(WQT), 17(URL)\n\n"
			"�Eur3��10 (00/09/28)�`UR1.2.23.0 (00/11/29) �Ŏg���Ă����ꍇ��\n"
			"  (3) (1)�̌�A00-12 �܂ł�(2)�Ɠ���  13(CMT), 14(SQT), 15(WQT), 16(URL)\n\n"
			"�Eur3��9 (00/09/26) �ȑO�Ŏg���Ă����ꍇ��\n"
			"  (4) (1)�̌�A(2)�Ɠ��l:\n"
			"      00(TXT), 01(LNO), 02(EOL), 03(TAB), 04(ZEN), 05(EOF), 06(KW1), 07(CMT), 08(SQT),\n"
			"      09(WQT), 10(UND), 11(RAP), 12(CTL), 13(URL), 14(FND), 15(MOD), 16(RUL)\n\n"
			, szPath
		);
		in.Close();
		return;
	}
	in.Close();


	CDataProfile	cProfile;
	cProfile.SetReadingMode();

	int				nColorInfoArrNum;						/* �L�[���蓖�ĕ\�̗L���f�[�^�� */
	ColorInfo		ColorInfoArr[64];

	/* �F�ݒ�Ver2 */
	nColorInfoArrNum = COLORIDX_LAST;
	if( !cProfile.ReadProfile( szPath ) ){
		/* �ݒ�t�@�C�������݂��Ȃ� */
		ErrorMessage( hwndDlg, _T("�t�@�C�����J���܂���ł����B\n\n%ts"), szPath );
		return;
	}
	/* �F�ݒ� I/O */
	for( int i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
		ColorInfoArr[i] = m_Types.m_ColorInfoArr[i];
		_tcscpy( ColorInfoArr[i].m_szName, m_Types.m_ColorInfoArr[i].m_szName );
	}
	CShareData::IO_ColorSet( &cProfile, LTEXT(STR_COLORDATA_SECTION), ColorInfoArr );


//complete:;
	/* �f�[�^�̃R�s�[ */
	m_Types.m_nColorInfoArrNum = nColorInfoArrNum;
	for( int i = 0; i < m_Types.m_nColorInfoArrNum; ++i ){
		m_Types.m_ColorInfoArr[i] =  ColorInfoArr[i];
		_tcscpy( m_Types.m_ColorInfoArr[i].m_szName, ColorInfoArr[i].m_szName );
	}
	/* �_�C�A���O�f�[�^�̐ݒ� p5 */
	SetData_Color( hwndDlg );
}


/* �F�̐ݒ���G�N�X�|�[�g */
void CPropTypes::_Export_Colors( HWND hwndDlg )
{
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1] = _T("");
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		_T("*.col"),
		GetDllShareData().m_szIMPORTFOLDER, // �C���|�[�g�p�t�H���_
		std::vector<LPCTSTR>(),
		std::vector<LPCTSTR>()
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}

	/* �t�@�C���̃t���p�X���t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, GetDllShareData().m_szIMPORTFOLDER, NULL );
	_tcscat( GetDllShareData().m_szIMPORTFOLDER, _T("\\") );

	/* �F�ݒ� I/O */
	CDataProfile	cProfile;
	cProfile.SetWritingMode();
	CShareData::IO_ColorSet( &cProfile, LTEXT(STR_COLORDATA_SECTION), m_Types.m_ColorInfoArr );
	cProfile.WriteProfile( szPath, LTEXT(STR_COLORDATA_HEAD3) );	//Jan. 15, 2001 Stonee

	return;
}



LRESULT APIENTRY ColorList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
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

		poMouse.x = xPos;
		poMouse.y = yPos;
		nItemNum = ::SendMessageAny( hwnd, LB_GETCOUNT, 0, 0 );
		nIndex = -1;
		for( i = 0; i < nItemNum; ++i ){
			::SendMessageAny( hwnd, LB_GETITEMRECT, i, (LPARAM)&rcItem );
			if( ::PtInRect( &rcItem, poMouse ) ){
//				MYTRACE_A( "hit at i==%d\n", i );
//				MYTRACE_A( "\n" );
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
		::SendMessageAny( hwnd, LB_SETCURSEL, nIndex, 0 );
		::SendMessageCmd( ::GetParent( hwnd ), WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwnd );
		pColorInfo = (ColorInfo*)::SendMessageAny( hwnd, LB_GETITEMDATA, nIndex, 0 );
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
		break;

	case WM_LBUTTONDBLCLK:
		if( -1 == nIndex ){
			break;
		}
		pColorInfo = (ColorInfo*)::SendMessageAny( hwnd, LB_GETITEMDATA, nIndex, 0 );
		/* �����ŕ\�� */
		if( 0 == (g_ColorAttributeArr[nIndex].fAttribute & COLOR_ATTRIB_NO_BOLD) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
		{
			if( pColorInfo->m_bFatFont ){	/* �����ŕ\�� */
				pColorInfo->m_bFatFont = false;
				::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_FAT, FALSE );
			}else{
				pColorInfo->m_bFatFont = true;
				::CheckDlgButton( ::GetParent( hwnd ), IDC_CHECK_FAT, TRUE );
			}
			::InvalidateRect( hwnd, &rcItem, TRUE );
		}
		break;
	case WM_LBUTTONUP:
		if( -1 == nIndex ){
			break;
		}
		pColorInfo = (ColorInfo*)::SendMessageAny( hwnd, LB_GETITEMDATA, nIndex, 0 );
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
				pColorInfo = (ColorInfo*)::SendMessageAny( hwnd, LB_GETITEMDATA, nIndex, 0 );

			}

			::InvalidateRect( hwnd, &rcItem, TRUE );
		}else
		/* �O�i�F���{ ��` */
		if( rcItem.right - 27 <= xPos && xPos <= rcItem.right - 27 + 12 ){
			/* �F�I���_�C�A���O */
			// 2005.11.30 Moca �J�X�^���F�ێ�
			DWORD* pColors = (DWORD*)::GetProp( hwnd, _T("ptrCustomColors") );
			if( CPropTypes::SelectColor( hwnd, &pColorInfo->m_colTEXT, pColors ) ){
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
			if( CPropTypes::SelectColor( hwnd, &pColorInfo->m_colBACK, pColors ) ){
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
	return CallWindowProc( (WNDPROC)m_wpColorListProc, hwnd, uMsg, wParam, lParam );
}





/* p3 ���b�Z�[�W���� */
INT_PTR CPropTypes::DispatchEvent_Color(
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
		SetData_Color( hwndDlg );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), EM_LIMITTEXT, (WPARAM)1, 0 );


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
				nIndex = ::SendMessageAny( hwndListColor, LB_GETCURSEL, 0, 0 );
				m_nCurrentColorType = nIndex;		/* ���ݑI������Ă���F�^�C�v */

				{
					// �e��R���g���[���̗L���^������؂�ւ���	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
					unsigned int fAttribute = g_ColorAttributeArr[nIndex].fAttribute;
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ),			(0 == (fAttribute & COLOR_ATTRIB_FORCE_DISP))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ),				(0 == (fAttribute & COLOR_ATTRIB_NO_BOLD))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ),		(0 == (fAttribute & COLOR_ATTRIB_NO_UNDERLINE))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ),	TRUE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ),			(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ),		(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ),	(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? TRUE: FALSE );

					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ),				(0 == (fAttribute & COLOR_ATTRIB_FORCE_DISP))? SW_SHOW: SW_HIDE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_FAT ),				(0 == (fAttribute & COLOR_ATTRIB_NO_BOLD))? SW_SHOW: SW_HIDE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ),			(0 == (fAttribute & COLOR_ATTRIB_NO_UNDERLINE))? SW_SHOW: SW_HIDE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ),	TRUE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ),			(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? SW_SHOW: SW_HIDE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ),		(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? SW_SHOW: SW_HIDE );
					//::ShowWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ),		(0 == (fAttribute & COLOR_ATTRIB_NO_BACK))? SW_SHOW: SW_HIDE );
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
				::SendMessageAny( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_BUTTON_BACKCOLOR:	/* �w�i�F */
				/* �F�I���_�C�A���O */
				if( SelectColor( hwndDlg, &m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK, m_dwCustColors ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
				}
				/* ���ݑI������Ă���F�^�C�v */
				::SendMessageAny( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_CHECK_DISP:	/* �F����/�\�� ������ */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp = true;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bDisp = false;
				}
				/* ���ݑI������Ă���F�^�C�v */
				::SendMessageAny( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK ���K�\���L�[���[�h�̂���
				return TRUE;
			case IDC_CHECK_FAT:	/* ������ */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_FAT ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bFatFont = true;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bFatFont = false;
				}
				/* ���ݑI������Ă���F�^�C�v */
				::SendMessageAny( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;
			case IDC_CHECK_UNDERLINE:	/* ������\�� */
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_UNDERLINE ) ){
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine = true;
				}else{
					m_Types.m_ColorInfoArr[m_nCurrentColorType].m_bUnderLine = false;
				}
				/* ���ݑI������Ă���F�^�C�v */
				::SendMessageAny( hwndListColor, LB_SETCURSEL, m_nCurrentColorType, 0 );
				return TRUE;

			case IDC_BUTTON_IMPORT:	/* �F�̐ݒ���C���|�[�g */
				_Import_Colors( hwndDlg );
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK ���K�\���L�[���[�h�̂���
				return TRUE;

			case IDC_BUTTON_EXPORT:	/* �F�̐ݒ���G�N�X�|�[�g */
				_Export_Colors( hwndDlg );
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
					int nIdx = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIdx || 0 == nIdx ){
						m_nSet[ 0 ] = -1;
					}else{
						m_nSet[ 0 ] = nIdx - 1;
					}
					cDlgKeywordSelect.DoModal( ::GetModuleHandle(NULL), hwndDlg, m_nSet );
					RearrangeKeywordSet( hwndDlg );	//	Jan. 23, 2005 genta �L�[���[�h�Z�b�g�Ĕz�u
					//�����L�[���[�h1�𔽉f����B
					if( -1 == m_nSet[ 0 ] ){
						::SendMessageAny( hwndCombo, CB_SETCURSEL, (WPARAM)0, 0 );
					}else{
						::SendMessageAny( hwndCombo, CB_SETCURSEL, (WPARAM)m_nSet[ 0 ] + 1, 0 );
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
//				MYTRACE_A( "p3 PSN_KILLACTIVE\n" );
				/* �_�C�A���O�f�[�^�̎擾 p3 */
				GetData_Color( hwndDlg );
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
			_DrawColorButton( pDis, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colTEXT );
			return TRUE;
		case IDC_BUTTON_BACKCOLOR:	/* �w�i�F */
			_DrawColorButton( pDis, m_Types.m_ColorInfoArr[m_nCurrentColorType].m_colBACK );
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
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids2 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
//		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.17 add start MIK
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids2 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.11.17 add end MIK

	}
	return FALSE;
}





/* �_�C�A���O�f�[�^�̐ݒ� p3 */
void CPropTypes::SetData_Color( HWND hwndDlg )
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
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2 ), EM_LIMITTEXT, (WPARAM)( BLOCKCOMMENT_BUFFERSIZE - 1 ), 0 );
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2 )	, EM_LIMITTEXT, (WPARAM)( BLOCKCOMMENT_BUFFERSIZE - 1 ), 0 );
//#endif

	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM	, m_Types.m_cBlockComment.getBlockCommentFrom(0) );	/* �u���b�N�R�����g�f���~�^(From) */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO		, m_Types.m_cBlockComment.getBlockCommentTo(0) );	/* �u���b�N�R�����g�f���~�^(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, m_Types.m_cBlockComment.getBlockCommentFrom(1) );	/* �u���b�N�R�����g�f���~�^2(From) */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, m_Types.m_cBlockComment.getBlockCommentTo(1) );	/* �u���b�N�R�����g�f���~�^2(To) */
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

	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	if( !m_Types.m_bLineNumIsCRLF ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, TRUE );
	}


	/* �Z�b�g���R���{�{�b�N�X�̒l�Z�b�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	::SendMessageAny( hwndWork, CB_RESETCONTENT, 0, 0 );  /* �R���{�{�b�N�X����ɂ��� */
	/* ��s�ڂ͋� */
	Combo_AddString( hwndWork, L" " );
	//	Mar. 31, 2003 genta KeyWordSetMgr���|�C���^��
	if( 0 < m_pCKeyWordSetMgr->m_nKeyWordSetNum ){
		for( i = 0; i < m_pCKeyWordSetMgr->m_nKeyWordSetNum; ++i ){
			Combo_AddString( hwndWork, m_pCKeyWordSetMgr->GetTypeName( i ) );
		}
		if( -1 == m_Types.m_nKeyWordSetIdx[0] ){
			/* �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I�� */
			::SendMessageAny( hwndWork, CB_SETCURSEL, (WPARAM)0, 0 );
		}else{
			/* �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I�� */
			::SendMessageAny( hwndWork, CB_SETCURSEL, (WPARAM)m_Types.m_nKeyWordSetIdx[0] + 1, 0 );
		}
	}

	//�����L�[���[�h1�`10�̐ݒ�
	for( i = 0; i < MAX_KEYWORDSET_PER_TYPE; i++ ){
		m_nSet[ i ] = m_Types.m_nKeyWordSetIdx[i];
	}

	/* �F�����镶����ނ̃��X�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_COLORS );
	::SendMessageAny( hwndWork, LB_RESETCONTENT, 0, 0 );  /* �R���{�{�b�N�X����ɂ��� */
	for( i = 0; i < COLORIDX_LAST; ++i ){
		nItem = ::List_AddString( hwndWork, m_Types.m_ColorInfoArr[i].m_szName );
		::SendMessageAny( hwndWork, LB_SETITEMDATA, nItem, (LPARAM)&m_Types.m_ColorInfoArr[i] );
	}
	/* ���ݑI������Ă���F�^�C�v */
	::SendMessageAny( hwndWork, LB_SETCURSEL, m_nCurrentColorType, 0 );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwndWork );

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
	wchar_t	szLineTermChar[2];
	auto_sprintf( szLineTermChar, L"%lc", m_Types.m_cLineTermChar );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar );

	//	From Here Sept. 10, 2000 JEPRO
	//	�s�ԍ���؂��C�ӂ̔��p�����ɂ���Ƃ������w�蕶�����͂�Enable�ɐݒ�
	EnableTypesPropInput( hwndDlg );
	//	To Here Sept. 10, 2000


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
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_VERTLINE ), EM_LIMITTEXT, (WPARAM)(MAX_VERTLINES * 15), 0 );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_VERTLINE, szVertLine );
	// to here 2005.11.30 Moca �w��ʒu�c���̐ݒ�
	return;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



/* �_�C�A���O�f�[�^�̎擾 p3 */
int CPropTypes::GetData_Color( HWND hwndDlg )
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
	m_Types.m_cBlockComment.SetBlockCommentRule( 0, szFromBuffer, szToBuffer );

	::DlgItem_GetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_FROM2	, szFromBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* �u���b�N�R�����g�f���~�^(From) */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_BLOCKCOMMENT_TO2	, szToBuffer	, BLOCKCOMMENT_BUFFERSIZE );	/* �u���b�N�R�����g�f���~�^(To) */
	m_Types.m_cBlockComment.SetBlockCommentRule( 1, szFromBuffer, szToBuffer );

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
	nIdx = ::SendMessageAny( hwndWork, CB_GETCURSEL, 0, 0 );
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
	wchar_t	szLineTermChar[2];
	::DlgItem_GetText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar, 2 );
	m_Types.m_cLineTermChar = szLineTermChar[0];


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
void CPropTypes::_DrawColorButton( DRAWITEMSTRUCT* pDis, COLORREF cColor )
{
//	MYTRACE_A( "pDis->itemAction = " );

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

	/* �t�H�[�J�X�̒����` */
	if( pDis->itemState & ODS_FOCUS ){
		rcFocus.top -= 3;
		rcFocus.left -= 3;
		rcFocus.right += 2;
		rcFocus.bottom += 2;
		::DrawFocusRect( pDis->hDC, &rcFocus );
	}
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
//	::TextOutW_AnyBuild( pDis->hDC, rc1.left, rc1.top, pColorInfo->m_szName, wcslen( pColorInfo->m_szName ) );
//	if( pColorInfo->m_bFatFont ){	/* ������ */
//		::TextOutW_AnyBuild( pDis->hDC, rc1.left + 1, rc1.top, pColorInfo->m_szName, wcslen( pColorInfo->m_szName ) );
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
	::TextOut( pDis->hDC, rc1.left, rc1.top, pColorInfo->m_szName, _tcslen( pColorInfo->m_szName ) );
	if( pColorInfo->m_bFatFont ){	/* ������ */
		::TextOut( pDis->hDC, rc1.left + 1, rc1.top, pColorInfo->m_szName, _tcslen( pColorInfo->m_szName ) );
	}
	if( pColorInfo->m_bUnderLine ){	/* ������ */
		SIZE	sz;
		::GetTextExtentPoint32( pDis->hDC, pColorInfo->m_szName, _tcslen( pColorInfo->m_szName ), &sz );
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
	if ( 0 == (g_ColorAttributeArr[pColorInfo->m_nColorIdx].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
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

}



/* �F�I���_�C�A���O */
BOOL CPropTypes::SelectColor( HWND hwndParent, COLORREF* pColor, DWORD* pCustColors )
{
	CHOOSECOLOR		cc;
	cc.lStructSize = sizeof_raw( cc );
	cc.hwndOwner = hwndParent;
	cc.hInstance = NULL;
	cc.rgbResult = *pColor;
	cc.lpCustColors = pCustColors;
	cc.Flags = /*CC_PREVENTFULLOPEN |*/ CC_RGBINIT;
	cc.lCustData = NULL;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;
	if( !::ChooseColor( &cc ) ){
		return FALSE;
	}
	*pColor = cc.rgbResult;
	return TRUE;
}
