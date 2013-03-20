/*
	�^�C�v�ʐݒ� - �E�B���h�E

	2008.04.12 kobake CPropTypes.cpp���番��
	2009.02.22 ryoji
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

using namespace std;

static const DWORD p_helpids2[] = {	//11400
	IDC_CHECK_DOCICON,				HIDC_CHECK_DOCICON,				//�����A�C�R�����g��	// 2006.08.06 ryoji

	IDC_COMBO_IMESWITCH,			HIDC_COMBO_IMESWITCH,		//IME��ON/OFF���
	IDC_COMBO_IMESTATE,				HIDC_COMBO_IMESTATE,		//IME�̓��̓��[�h

	IDC_COMBO_DEFAULT_CODETYPE,		HIDC_COMBO_DEFAULT_CODETYPE,		//�f�t�H���g�����R�[�h
	IDC_COMBO_DEFAULT_EOLTYPE,		HIDC_COMBO_DEFAULT_EOLTYPE,			//�f�t�H���g���s�R�[�h	// 2011.01.24 ryoji
	IDC_CHECK_DEFAULT_BOM,			HIDC_CHECK_DEFAULT_BOM,				//�f�t�H���gBOM	// 2011.01.24 ryoji
	IDC_CHECK_PRIOR_CESU8,			HIDC_CHECK_PRIOR_CESU8,				//�������ʎ���CESU-8��D�悷��

	IDC_EDIT_BACKIMG_PATH,			HIDC_EDIT_BACKIMG_PATH,			//�w�i�摜
	IDC_BUTTON_BACKIMG_PATH_SEL,	HIDC_BUTTON_BACKIMG_PATH_SEL,	//�w�i�摜�{�^��
	IDC_COMBO_BACKIMG_POS,			HIDC_COMBO_BACKIMG_POS,			//�w�i�摜�ʒu
	IDC_CHECK_BACKIMG_SCR_X,		HIDC_CHECK_BACKIMG_SCR_X,		//�w�i�摜ScrollX
	IDC_CHECK_BACKIMG_SCR_Y,		HIDC_CHECK_BACKIMG_SCR_Y,		//�w�i�摜ScrollY
	IDC_CHECK_BACKIMG_REP_X,		HIDC_CHECK_BACKIMG_REP_X,		//�w�i�摜RepeatX
	IDC_CHECK_BACKIMG_REP_Y,		HIDC_CHECK_BACKIMG_REP_Y,		//�w�i�摜RepeatY
	IDC_EDIT_BACKIMG_OFFSET_X,		HIDC_EDIT_BACKIMG_OFFSET_X,		//�w�i�摜OffsetX
	IDC_EDIT_BACKIMG_OFFSET_Y,		HIDC_EDIT_BACKIMG_OFFSET_Y,		//�w�i�摜OffsetY
	IDC_RADIO_LINENUM_LAYOUT,		HIDC_RADIO_LINENUM_LAYOUT,		//�s�ԍ��̕\���i�܂�Ԃ��P�ʁj
	IDC_RADIO_LINENUM_CRLF,			HIDC_RADIO_LINENUM_CRLF,		//�s�ԍ��̕\���i���s�P�ʁj
	IDC_RADIO_LINETERMTYPE0,		HIDC_RADIO_LINETERMTYPE0,		//�s�ԍ���؂�i�Ȃ��j
	IDC_RADIO_LINETERMTYPE1,		HIDC_RADIO_LINETERMTYPE1,		//�s�ԍ���؂�i�c���j
	IDC_RADIO_LINETERMTYPE2,		HIDC_RADIO_LINETERMTYPE2,		//�s�ԍ���؂�i�C�Ӂj
	IDC_EDIT_LINETERMCHAR,			HIDC_EDIT_LINETERMCHAR,			//�s�ԍ���؂�
//	IDC_STATIC,						-1,
	0, 0
};



TYPE_NAME<int> ImeSwitchArr[] = {
	{ 0, _T("���̂܂�") },
	{ 1, _T("���ON") },
	{ 2, _T("���OFF") },
};

//	Nov. 20, 2000 genta
TYPE_NAME<int> ImeStateArr[] = {
	{ 0, _T("�W���ݒ�") },
	{ 1, _T("�S�p") },
	{ 2, _T("�S�p�Ђ炪��") },
	{ 3, _T("�S�p�J�^�J�i") },
	{ 4, _T("���ϊ�") }
};

static const wchar_t* aszEolStr[] = {
	L"CR+LF",
	L"LF (UNIX)",
	L"CR (Mac)",
};
static const EEolType aeEolType[] = {
	EOL_CRLF,
	EOL_LF,
	EOL_CR,
};

/* window ���b�Z�[�W���� */
INT_PTR CPropTypesWindow::DispatchEvent(
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

	switch( uMsg ){
	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* �_�C�A���O�f�[�^�̐ݒ� color */
		SetData( hwndDlg );

		/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), 1 );

		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD( wParam );	/* �ʒm�R�[�h */
		wID			= LOWORD( wParam );	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */

		switch( wNotifyCode ){
		case CBN_SELCHANGE:
			{
				int i;
				switch( wID ){
				case IDC_COMBO_DEFAULT_CODETYPE:
					// �����R�[�h�̕ύX��BOM�`�F�b�N�{�b�N�X�ɔ��f
					i = Combo_GetCurSel( (HWND) lParam );
					if( CB_ERR != i ){
						CCodeTypeName	cCodeTypeName( Combo_GetItemData( (HWND)lParam, i ) );
						::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, (cCodeTypeName.IsBomDefOn() ?  BST_CHECKED : BST_UNCHECKED) );
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), cCodeTypeName.UseBom() );
					}
					break;
				}
			}
			break;

		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_BUTTON_BACKIMG_PATH_SEL:
				{
					CDialog::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_PATH),
						_T("*.bmp;*.jpg;*.jpeg"), true );
				}
				return TRUE;
			//	From Here Sept. 10, 2000 JEPRO
			//	�s�ԍ���؂��C�ӂ̔��p�����ɂ���Ƃ������w�蕶�����͂�Enable�ɐݒ�
			case IDC_RADIO_LINETERMTYPE0: /* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
			case IDC_RADIO_LINETERMTYPE1:
			case IDC_RADIO_LINETERMTYPE2:
				EnableTypesPropInput( hwndDlg );
				return TRUE;
			//	To Here Sept. 10, 2000

			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
//	Sept. 10, 2000 JEPRO ID�������ۂ̖��O�ɕύX���邽�߈ȉ��̍s�̓R�����g�A�E�g
//				OnHelp( hwndDlg, IDD_PROP1P3 );
			OnHelp( hwndDlg, IDD_PROP_WINDOW );
			return TRUE;
		case PSN_KILLACTIVE:
//				MYTRACE_A( "color PSN_KILLACTIVE\n" );
			/* �_�C�A���O�f�[�^�̎擾 window */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
		case PSN_SETACTIVE:
			m_nPageNum = 2;
			return TRUE;
		}
		break;	/* WM_NOTIFY */

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


void CPropTypesWindow::SetCombobox(HWND hwndWork, const TCHAR** pszLabels, int nCount, int select)
{
	Combo_ResetContent(hwndWork);
	for(int i = 0; i < nCount; ++i ){
		Combo_AddString( hwndWork, pszLabels[i] );
	}
	Combo_SetCurSel(hwndWork, select);
}



/* �_�C�A���O�f�[�^�̐ݒ� window */
void CPropTypesWindow::SetData( HWND hwndDlg )
{
	{
		// �����A�C�R�����g��	//Sep. 10, 2002 genta
		::CheckDlgButton( hwndDlg, IDC_CHECK_DOCICON, m_Types.m_bUseDocumentIcon  ? TRUE : FALSE );
	}

	//�N������IME(���{����͕ϊ�)	//Nov. 20, 2000 genta
	{
		int ime;
		// ON/OFF���
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
		Combo_ResetContent( hwndCombo );
		ime = m_Types.m_nImeState & 3;
		int		nSelPos = 0;
		for( int i = 0; i < _countof( ImeSwitchArr ); ++i ){
			Combo_InsertString( hwndCombo, i, ImeSwitchArr[i].pszName );
			if( ImeSwitchArr[i].nMethod == ime ){	/* IME��� */
				nSelPos = i;
			}
		}
		Combo_SetCurSel( hwndCombo, nSelPos );

		// ���̓��[�h
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
		Combo_ResetContent( hwndCombo );
		ime = m_Types.m_nImeState >> 2;
		nSelPos = 0;
		for( int i = 0; i < _countof( ImeStateArr ); ++i ){
			Combo_InsertString( hwndCombo, i, ImeStateArr[i].pszName );
			if( ImeStateArr[i].nMethod == ime ){	/* IME��� */
				nSelPos = i;
			}
		}
		Combo_SetCurSel( hwndCombo, nSelPos );
	}

	/* �u�����R�[�h�v�O���[�v�̐ݒ� */
	{
		int i;
		HWND hCombo;

		// �u�����F������CESU-8��D��vm_Types.m_encoding.m_bPriorCesu8 ���`�F�b�N
		::CheckDlgButton( hwndDlg, IDC_CHECK_PRIOR_CESU8, m_Types.m_encoding.m_bPriorCesu8 );

		// �f�t�H���g�R�[�h�^�C�v�̃R���{�{�b�N�X�ݒ�
		int		nSel= 0;
		int		j = 0;
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		CCodeTypesForCombobox cCodeTypes;
		for (i = 0; i < cCodeTypes.GetCount(); i++) {
			if (CCodeTypeName( cCodeTypes.GetCode(i) ).CanDefault()) {
				int idx = Combo_AddString( hCombo, cCodeTypes.GetName(i) );
				Combo_SetItemData( hCombo, idx, cCodeTypes.GetCode(i) );
				if (m_Types.m_encoding.m_eDefaultCodetype == cCodeTypes.GetCode(i)) {
					nSel = j;
				}
				j++;
			}
		}
		Combo_SetCurSel( hCombo, nSel );

		// BOM �`�F�b�N�{�b�N�X�ݒ�
		CCodeTypeName	cCodeTypeName(m_Types.m_encoding.m_eDefaultCodetype);
		if( !cCodeTypeName.UseBom() )
			m_Types.m_encoding.m_bDefaultBom = false;
		::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, (m_Types.m_encoding.m_bDefaultBom ? BST_CHECKED : BST_UNCHECKED) );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), (int)cCodeTypeName.UseBom() );

		// �f�t�H���g���s�^�C�v�̃R���{�{�b�N�X�ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		for( i = 0; i < _countof(aszEolStr); ++i ){
			ApiWrap::Combo_AddString( hCombo, aszEolStr[i] );
		}
		for( i = 0; i < _countof(aeEolType); ++i ){
			if( m_Types.m_encoding.m_eDefaultEoltype == aeEolType[i] ){
				break;
			}
		}
		if( i == _countof(aeEolType) ){
			i = 0;
		}
		Combo_SetCurSel( hCombo, i );
	}

	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	if( !m_Types.m_bLineNumIsCRLF ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, TRUE );
	}

	// �w�i�摜
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_PATH), _countof2(m_Types.m_szBackImgPath));
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X), 5);
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y), 5);

	DlgItem_SetText( hwndDlg, IDC_EDIT_BACKIMG_PATH, m_Types.m_szBackImgPath );
	{
		const TCHAR* posNames[] ={
			_T("����"),
			_T("�E��"),
			_T("����"),
			_T("�E��"),
			_T("����"),
			_T("������"),
			_T("������"),
			_T("������"),
			_T("�����E"),
		};
		/*BGIMAGE_TOP_LEFT .. */
		int nCount = _countof(posNames);
		SetCombobox( ::GetDlgItem(hwndDlg, IDC_COMBO_BACKIMG_POS), posNames, nCount, m_Types.m_backImgPos);
	}
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_REP_X, m_Types.m_backImgRepeatX);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_REP_Y, m_Types.m_backImgRepeatY);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_X, m_Types.m_backImgScrollX);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_Y, m_Types.m_backImgScrollY);
	SetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X, m_Types.m_backImgPosOffset.x, TRUE);
	SetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y, m_Types.m_backImgPosOffset.y, TRUE);

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


	return;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



/* �_�C�A���O�f�[�^�̎擾 color */
int CPropTypesWindow::GetData( HWND hwndDlg )
{
	{
		// �����A�C�R�����g��	//Sep. 10, 2002 genta
		m_Types.m_bUseDocumentIcon = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DOCICON ) ? true : false;
	}

	//�N������IME(���{����͕ϊ�)	Nov. 20, 2000 genta
	{
		//���̓��[�h
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
		int		nSelPos = Combo_GetCurSel( hwndCombo );
		m_Types.m_nImeState = ImeStateArr[nSelPos].nMethod << 2;	//	IME���̓��[�h

		//ON/OFF���
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
		nSelPos = Combo_GetCurSel( hwndCombo );
		m_Types.m_nImeState |= ImeSwitchArr[nSelPos].nMethod;	//	IME ON/OFF
	}

	/* �u�����R�[�h�v�O���[�v�̐ݒ� */
	{
		int i;
		HWND hCombo;

		// m_Types.m_bPriorCesu8 ��ݒ�
		m_Types.m_encoding.m_bPriorCesu8 = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_PRIOR_CESU8 ) != 0;

		// m_Types.eDefaultCodetype ��ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		i = Combo_GetCurSel( hCombo );
		if( CB_ERR != i ){
			m_Types.m_encoding.m_eDefaultCodetype = ECodeType( Combo_GetItemData( hCombo, i ) );
		}

		// m_Types.m_bDefaultBom ��ݒ�
		m_Types.m_encoding.m_bDefaultBom = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DEFAULT_BOM ) != 0;

		// m_Types.eDefaultEoltype ��ݒ�
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		i = Combo_GetCurSel( hCombo );
		if( CB_ERR != i ){
			m_Types.m_encoding.m_eDefaultEoltype = aeEolType[i];
		}
	}

	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINENUM_LAYOUT ) ){
		m_Types.m_bLineNumIsCRLF = false;
	}else{
		m_Types.m_bLineNumIsCRLF = true;
	}

	DlgItem_GetText(hwndDlg, IDC_EDIT_BACKIMG_PATH, m_Types.m_szBackImgPath, _countof2(m_Types.m_szBackImgPath));
	m_Types.m_backImgPos = static_cast<EBackgroundImagePos>(Combo_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBO_BACKIMG_POS)));
	m_Types.m_backImgRepeatX = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_REP_X);
	m_Types.m_backImgRepeatY = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_REP_Y);
	m_Types.m_backImgScrollX = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_X);
	m_Types.m_backImgScrollY = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_Y);
	m_Types.m_backImgPosOffset.x = GetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X, NULL, TRUE);
	m_Types.m_backImgPosOffset.y = GetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y, NULL, TRUE);

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

	return TRUE;
}




//	From Here Sept. 10, 2000 JEPRO
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void CPropTypesWindow::EnableTypesPropInput( HWND hwndDlg )
{
	//	�s�ԍ���؂��C�ӂ̔��p�����ɂ��邩�ǂ���
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE2 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LINETERMCHAR ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LINETERMCHAR ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), FALSE );
	}
}
//	To Here Sept. 10, 2000

