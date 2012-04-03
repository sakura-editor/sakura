/*!	@file
	�^�C�v�ʐݒ� - ���K�\���L�[���[�h �_�C�A���O�{�b�N�X

	@author MIK
	@date 2001/11/17  �V�K�쐬
*/
/*
	Copyright (C) 2001, MIK, Stonee
	Copyright (C) 2002, MIK
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#include "StdAfx.h"
#include "CPropTypes.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/23 Uchi
#include "util/shell.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "sakura_rc.h"
#include "sakura.hh"

using namespace std;


static const DWORD p_helpids[] = {	//11600
	IDC_BUTTON_REGEX_IMPORT,	HIDC_BUTTON_REGEX_IMPORT,	//�C���|�[�g
	IDC_BUTTON_REGEX_EXPORT,	HIDC_BUTTON_REGEX_EXPORT,	//�G�N�X�|�[�g
	IDC_BUTTON_REGEX_INS,		HIDC_BUTTON_REGEX_INS,		//�}��
	IDC_BUTTON_REGEX_ADD,		HIDC_BUTTON_REGEX_ADD,		//�ǉ�
	IDC_BUTTON_REGEX_UPD,		HIDC_BUTTON_REGEX_UPD,		//�X�V
	IDC_BUTTON_REGEX_DEL,		HIDC_BUTTON_REGEX_DEL,		//�폜
	IDC_BUTTON_REGEX_TOP,		HIDC_BUTTON_REGEX_TOP,		//�擪
	IDC_BUTTON_REGEX_LAST,		HIDC_BUTTON_REGEX_LAST,		//�ŏI
	IDC_BUTTON_REGEX_UP,		HIDC_BUTTON_REGEX_UP,		//���
	IDC_BUTTON_REGEX_DOWN,		HIDC_BUTTON_REGEX_DOWN,		//����
	IDC_CHECK_REGEX,			HIDC_CHECK_REGEX,			//���K�\���L�[���[�h���g�p����
	IDC_COMBO_REGEX_COLOR,		HIDC_COMBO_REGEX_COLOR,		//�F
	IDC_EDIT_REGEX,				HIDC_EDIT_REGEX,			//���K�\���L�[���[�h
	IDC_LIST_REGEX,				HIDC_LIST_REGEX,			//���X�g
	IDC_LABEL_REGEX_KEYWORD,	HIDC_EDIT_REGEX,			
	IDC_LABEL_REGEX_COLOR,		HIDC_COMBO_REGEX_COLOR,		
	IDC_FRAME_REGEX,			HIDC_LIST_REGEX,			
	IDC_LABEL_REGEX_VERSION,	HIDC_LABEL_REGEX_VERSION,	//�o�[�W����
//	IDC_STATIC,						-1,
	0, 0
};



// Import
// 2010/4/23 Uchi Import�̊O�o��
bool CPropRegex::Import(HWND hwndDlg)
{
	CImpExpRegex	cImpExpRegex(m_Types);

	// �C���|�[�g
	bool bImport = cImpExpRegex.ImportUI(m_hInstance, hwndDlg);
	if( bImport ){
		SetDataKeywordList(hwndDlg);
	}
	return bImport;
}

// Export
// 2010/4/23 Uchi Export�̊O�o��
bool CPropRegex::Export(HWND hwndDlg)
{
	GetData(hwndDlg);
	CImpExpRegex	cImpExpRegex(m_Types);

	// �G�N�X�|�[�g
	return cImpExpRegex.ExportUI(m_hInstance, hwndDlg);
}

/* ���K�\���L�[���[�h ���b�Z�[�W���� */
INT_PTR CPropRegex::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl, hwndList;
	int	idCtrl;
	NMHDR*	pNMHDR;
	int	nIndex, nIndex2, i, j, nRet;
	LV_ITEM	lvi;
	LV_COLUMN	col;
	RECT		rc;
	static int nPrevIndex = -1;	//�X�V���ɂ��������Ȃ�o�O�C�� @@@ 2003.03.26 MIK


	hwndList = GetDlgItem( hwndDlg, IDC_LIST_REGEX );

	// ANSI�r���h�ł�CP932����2�{���x�K�v
	TCHAR	szKeyWord[ _countof( m_Types.m_RegexKeywordArr[0].m_szKeyword ) * 2 + 1 ];
	TCHAR	szColorIndex[256];

	switch( uMsg ){
	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* �J�����ǉ� */
		//ListView_DeleteColumn( hwndList, 1 );
		//ListView_DeleteColumn( hwndList, 0 );
		::GetWindowRect( hwndList, &rc );
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 54 / 100;
		col.pszText  = _T("�L�[���[�h");
		col.iSubItem = 0;
		ListView_InsertColumn( hwndList, 0, &col );
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 38 / 100;
		col.pszText  = _T("�F�w��");
		col.iSubItem = 1;
		ListView_InsertColumn( hwndList, 1, &col );

		nPrevIndex = -1;	//@@@ 2003.05.12 MIK
		SetData( hwndDlg );	/* �_�C�A���O�f�[�^�̐ݒ� ���K�\���L�[���[�h */
		if( CheckRegexpVersion( hwndDlg, IDC_LABEL_REGEX_VERSION, false ) == false )	//@@@ 2001.11.17 add MIK
		{
			::DlgItem_SetText( hwndDlg, IDC_LABEL_REGEX_VERSION, _T("���K�\���L�[���[�h�͎g���܂���B") );
			//���C�u�������Ȃ��āA�g�p���Ȃ��ɂȂ��Ă���ꍇ�́A�����ɂ���B
			if( ! IsDlgButtonChecked( hwndDlg, IDC_CHECK_REGEX ) )
			{
				//Disable�ɂ���B
				EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_REGEX ), FALSE );
			}
			else
			{
				//�g�p����ɂȂ��Ă�񂾂���Disable�ɂ���B�������[�U�͕ύX�ł��Ȃ��B
				EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_REGEX ), FALSE );
			}
		}
		return TRUE;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID	= LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl	= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_REGEX:	/* ���K�\���L�[���[�h���g�� */
				if( IsDlgButtonChecked( hwndDlg, IDC_CHECK_REGEX ) )
				{
					if( CheckRegexpVersion( NULL, 0, false ) == false )
					{
						nRet = ::MYMESSAGEBOX(
								hwndDlg,
								MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
								GSTR_APPNAME,
								_T("���K�\�����C�u������������܂���B\n\n���K�\���L�[���[�h�͋@�\���܂��񂪁A����ł��L���ɂ��܂����H"),
								_T("���K�\���L�[���[�h���g�p����") );
						if( nRet != IDYES )
						{
							CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_UNCHECKED );
							//Disable�ɂ���B
							EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_REGEX ), FALSE );
							return TRUE;
						}
					}
				}
				else
				{
					if( CheckRegexpVersion( NULL, 0, false ) == false )
					{
						//Disable�ɂ���B
						EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_REGEX ), FALSE );
					}
				}
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile
				return TRUE;

			case IDC_BUTTON_REGEX_INS:	/* �}�� */
				//�}������L�[�����擾����B
				auto_memset(szKeyWord, 0, _countof(szKeyWord));
				::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEX, szKeyWord, _countof(szKeyWord) );
				if( szKeyWord[0] == L'\0' ) return FALSE;
				//�����L�[���Ȃ������ׂ�B
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 >= MAX_REGEX_KEYWORD )
				{
					ErrorMessage( hwndDlg, _T("����ȏ�o�^�ł��܂���B"));
					return FALSE;
				}
				for(i = 0; i < nIndex2; i++)
				{
					auto_memset(szColorIndex, 0, _countof(szColorIndex));
					ListView_GetItemText(hwndList, i, 0, szColorIndex, _countof(szColorIndex));
					if( _tcscmp(szKeyWord, szColorIndex) == 0 ) 
					{
						ErrorMessage( hwndDlg, _T("�����L�[���[�h�œo�^�ς݂ł��B"));
						return FALSE;
					}
				}
				//�I�𒆂̃L�[��T���B
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex )
				{
					//�I�𒆂łȂ���΍Ō�ɂ���B
					nIndex = nIndex2;
				}
				//�������`�F�b�N����B
				if( !RegexKakomiCheck(to_wchar(szKeyWord)) )	//�݂͂��`�F�b�N����B
				{
					TopErrorMessage(
							hwndDlg,
							_T("���K�\���L�[���[�h�� / �� /k �ň͂��Ă��������B\n�L�[���[�h�� / ������ꍇ�� m# �� #k �ň͂��Ă��������B"),
							_T("���K�\���L�[���[�h") );
					return FALSE;
				}
				if( !CheckRegexpSyntax( to_wchar(szKeyWord), hwndDlg, false ) )
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							_T("�������������Ȃ����A���K�\�����C�u������������܂���B\n\n�o�^���܂����H"),
							_T("���K�\���L�[���[�h") );
					if( nRet != IDYES ) return FALSE;
				}
				//�}������L�[�����擾����B
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex) );
				//�L�[����}������B
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//�}�������L�[��I������B
				ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_ADD:	/* �ǉ� */
				//�Ō�̃L�[�ԍ����擾����B
				nIndex = ListView_GetItemCount( hwndList );
				//�ǉ�����L�[�����擾����B
				auto_memset(szKeyWord, 0, _countof(szKeyWord));
				::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEX, szKeyWord, _countof(szKeyWord) );
				if( szKeyWord[0] == L'\0' ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 >= MAX_REGEX_KEYWORD )
				{
					ErrorMessage( hwndDlg, _T("����ȏ�o�^�ł��܂���B"));
					return FALSE;
				}
				for(i = 0; i < nIndex2; i++)
				{
					auto_memset(szColorIndex, 0, _countof(szColorIndex));
					ListView_GetItemText(hwndList, i, 0, szColorIndex, _countof(szColorIndex));
					if( _tcscmp(szKeyWord, szColorIndex) == 0 ) 
					{
						ErrorMessage( hwndDlg, _T("�����L�[���[�h�œo�^�ς݂ł��B"));
						return FALSE;
					}
				}
				//�������`�F�b�N����B
				if( !RegexKakomiCheck(to_wchar(szKeyWord)) )	//�݂͂��`�F�b�N����B
				{
					TopErrorMessage(
						hwndDlg,
						_T("���K�\���L�[���[�h�� / �� /k �ň͂��Ă��������B\n�L�[���[�h�� / ������ꍇ�� m# �� #k �ň͂��Ă��������B"),
						_T("���K�\���L�[���[�h")
					);
					return FALSE;
				}
				if( !CheckRegexpSyntax( to_wchar(szKeyWord), hwndDlg, false ) )
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							_T("�������������Ȃ����A���K�\�����C�u������������܂���B\n\n�o�^���܂����H"),
							_T("���K�\���L�[���[�h") );
					if( nRet != IDYES ) return FALSE;
				}
				//�ǉ�����L�[�����擾����B
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex) );
				//�L�[��ǉ�����B
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//�ǉ������L�[��I������B
				ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_UPD:	/* �X�V */
				//�I�𒆂̃L�[��T���B
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex )
				{
					ErrorMessage( hwndDlg, _T("�L�[���[�h���I������Ă��܂���B"));
					return FALSE;
				}
				//�X�V����L�[�����擾����B
				auto_memset(szKeyWord, 0, _countof(szKeyWord));
				::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEX, szKeyWord, _countof(szKeyWord) );
				if( szKeyWord[0] == L'\0' ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				for(i = 0; i < nIndex2; i++)
				{
					if( i != nIndex )
					{
						auto_memset(szColorIndex, 0, _countof(szColorIndex));
						ListView_GetItemText(hwndList, i, 0, szColorIndex, _countof(szColorIndex));
						if( _tcscmp(szKeyWord, szColorIndex) == 0 ) 
						{
							ErrorMessage( hwndDlg, _T("�����L�[���[�h�œo�^�ς݂ł��B"));
							return FALSE;
						}
					}
				}
				//�������`�F�b�N����B
				if( !RegexKakomiCheck(to_wchar(szKeyWord)) )	//�݂͂��`�F�b�N����B
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_OK | MB_ICONSTOP | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							_T("���K�\���L�[���[�h�� / �� /k �ň͂��Ă��������B\n�L�[���[�h�� / ������ꍇ�� m# �� #k �ň͂��Ă��������B"),
							_T("���K�\���L�[���[�h") );
					return FALSE;
				}
				if( !CheckRegexpSyntax( to_wchar(szKeyWord), hwndDlg, false ) )
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							_T("�������������Ȃ����A���K�\�����C�u������������܂���B\n\n�o�^���܂����H"),
							_T("���K�\���L�[���[�h") );
					if( nRet != IDYES ) return FALSE;
				}
				//�ǉ�����L�[�����擾����B
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex) );
				//�L�[���X�V����B
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_SetItem( hwndList, &lvi );

				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );

				//�X�V�����L�[��I������B
				ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_DEL:	/* �폜 */
				//�I�𒆂̃L�[�ԍ���T���B
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				//�폜����B
				ListView_DeleteItem( hwndList, nIndex );
				//�����ʒu�̃L�[��I����Ԃɂ���B
				ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_TOP:	/* �擪 */
				//�I�𒆂̃L�[��T���B
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				if( 0 == nIndex ) return TRUE;	//���łɐ擪�ɂ���B
				nIndex2 = 0;
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, _countof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				ListView_DeleteItem(hwndList, nIndex);	//�Â��L�[���폜
				//�L�[��ǉ�����B
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//�ړ������L�[��I����Ԃɂ���B
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_LAST:	/* �ŏI */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 - 1 == nIndex ) return TRUE;	//���łɍŏI�ɂ���B
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, _countof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				//�L�[��ǉ�����B
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//�ړ������L�[��I����Ԃɂ���B
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				ListView_DeleteItem(hwndList, nIndex);	//�Â��L�[���폜
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_UP:	/* ��� */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				if( 0 == nIndex ) return TRUE;	//���łɐ擪�ɂ���B
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 <= 1 ) return TRUE;
				nIndex2 = nIndex - 1;
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, _countof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				ListView_DeleteItem(hwndList, nIndex);	//�Â��L�[���폜
				//�L�[��ǉ�����B
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//�ړ������L�[��I����Ԃɂ���B
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_DOWN:	/* ���� */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 - 1 == nIndex ) return TRUE;	//���łɍŏI�ɂ���B
				if( nIndex2 <= 1 ) return TRUE;
				nIndex2 = nIndex + 2;
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, _countof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				//�L�[��ǉ�����B
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//�ړ������L�[��I����Ԃɂ���B
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				ListView_DeleteItem(hwndList, nIndex);	//�Â��L�[���폜
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_IMPORT:	/* �C���|�[�g */
				Import(hwndDlg);
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK ���K�\���L�[���[�h�̂���
				return TRUE;

			case IDC_BUTTON_REGEX_EXPORT:	/* �G�N�X�|�[�g */
				Export(hwndDlg);
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_REGEX );
			return TRUE;
		case PSN_KILLACTIVE:
			/* �_�C�A���O�f�[�^�̎擾 ���K�\���L�[���[�h */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
		case PSN_SETACTIVE:
			m_nPageNum = 3;
			return TRUE;
		case LVN_ITEMCHANGED:
			if( pNMHDR->hwndFrom == hwndList )
			{
				HWND	hwndCombo;
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex )	//�폜�A�͈͊O�ŃN���b�N�����f����Ȃ��o�O�C��	//@@@ 2003.06.17 MIK
				{
					nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_FOCUSED );
				}
				if( -1 == nIndex )
				{
					/* �����l��ݒ肷�� */
					::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEX, _T("//k") );	/* ���K�\�� */
					hwndCombo = GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
					for( i = 0, j = 0; i < COLORIDX_LAST; i++ )
					{
						if ( 0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
						{
							if( m_Types.m_ColorInfoArr[i].m_nColorIdx == COLORIDX_REGEX1 )
							{
								Combo_SetCurSel( hwndCombo, j );	/* �R���{�{�b�N�X�̃f�t�H���g�I�� */
								break;
							}
							j++;
						}
					}
					return FALSE;
				}
				if( nPrevIndex != nIndex )	//@@@ 2003.03.26 MIK
				{	//�X�V����ListView��SubItem�𐳂����擾�ł��Ȃ��̂ŁA���̑΍�
					ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, _countof(szKeyWord));
					ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
					::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEX, szKeyWord );	/* ���K�\�� */
					hwndCombo = GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
					for(i = 0, j = 0; i < COLORIDX_LAST; i++)
					{
						if ( 0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
						{
							if(_tcscmp(m_Types.m_ColorInfoArr[i].m_szName, szColorIndex) == 0)
							{
								Combo_SetCurSel(hwndCombo, j);
								break;
							}
							j++;
						}
					}
				}
				nPrevIndex = nIndex;
			}
			break;
		}
		break;

	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;

	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;

	}
	return FALSE;
}

/* �_�C�A���O�f�[�^�̐ݒ� ���K�\���L�[���[�h */
void CPropRegex::SetData( HWND hwndDlg )
{
	HWND		hwndWork;
	int			i, j;

	/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_REGEX ), _countof( m_Types.m_RegexKeywordArr[0].m_szKeyword ) - 1 );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEX, _T("//k") );	/* ���K�\�� */

	/* �F��ނ̃��X�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
	Combo_ResetContent( hwndWork );  /* �R���{�{�b�N�X����ɂ��� */
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		if ( 0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
		{
			j = Combo_AddString( hwndWork, m_Types.m_ColorInfoArr[i].m_szName );
			if( m_Types.m_ColorInfoArr[i].m_nColorIdx == COLORIDX_REGEX1 )
				Combo_SetCurSel( hwndWork, j );	/* �R���{�{�b�N�X�̃f�t�H���g�I�� */
		}
	}

	if( m_Types.m_bUseRegexKeyword )
		CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_CHECKED );
	else
		CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_UNCHECKED );

	/* �s�I�� */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	DWORD		dwStyle;
	dwStyle = ListView_GetExtendedListViewStyle( hwndWork );
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( hwndWork, dwStyle );

	SetDataKeywordList( hwndDlg );
}

/* �_�C�A���O�f�[�^�̐ݒ� ���K�\���L�[���[�h�̈ꗗ���� */
void CPropRegex::SetDataKeywordList( HWND hwndDlg )
{
	LV_ITEM		lvi;

	/* ���X�g */
	HWND hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	ListView_DeleteAllItems(hwndWork);  /* ���X�g����ɂ��� */

	/* �f�[�^�\�� */
	for(int i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( m_Types.m_RegexKeywordArr[i].m_szKeyword[0] == L'\0' ) break;
		
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = const_cast<TCHAR*>(to_tchar(m_Types.m_RegexKeywordArr[i].m_szKeyword));
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = 0; //m_Types.m_RegexKeywordArr[i].m_nColorIndex;
		ListView_InsertItem( hwndWork, &lvi );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = m_Types.m_ColorInfoArr[m_Types.m_RegexKeywordArr[i].m_nColorIndex].m_szName;
		ListView_SetItem( hwndWork, &lvi );
	}
	ListView_SetItemState( hwndWork, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	return;
}

/* �_�C�A���O�f�[�^�̎擾 ���K�\���L�[���[�h */
int CPropRegex::GetData( HWND hwndDlg )
{
	HWND	hwndList;
	int	nIndex, i, j;
	TCHAR	szKeyWord[ _countof( m_Types.m_RegexKeywordArr[0].m_szKeyword ) * 2 + 1 ];
	TCHAR	szColorIndex[256];

//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	//�����̃y�[�W�ԍ�
//	m_nPageNum = 3;

	//�g�p����E�g�p���Ȃ�
	if( IsDlgButtonChecked( hwndDlg, IDC_CHECK_REGEX ) )
		m_Types.m_bUseRegexKeyword = true;
	else
		m_Types.m_bUseRegexKeyword = false;

	//���X�g�ɓo�^����Ă������z��Ɏ�荞��
	hwndList = GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	nIndex = ListView_GetItemCount(hwndList);
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( i < nIndex )
		{
			szKeyWord[0]    = _T('\0');
			szColorIndex[0] = _T('\0');
			ListView_GetItemText(hwndList, i, 0, szKeyWord,    _countof(szKeyWord)   );
			ListView_GetItemText(hwndList, i, 1, szColorIndex, _countof(szColorIndex));
			_tcstowcs(m_Types.m_RegexKeywordArr[i].m_szKeyword, szKeyWord, _countof(m_Types.m_RegexKeywordArr[i].m_szKeyword));
			//�F�w�蕶�����ԍ��ɕϊ�����
			m_Types.m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
			for(j = 0; j < COLORIDX_LAST; j++)
			{
				if(_tcscmp(m_Types.m_ColorInfoArr[j].m_szName, szColorIndex) == 0)
				{
					m_Types.m_RegexKeywordArr[i].m_nColorIndex = j;
					break;
				}
			}
		}
		else	//���o�^�����̓N���A����
		{
			m_Types.m_RegexKeywordArr[i].m_szKeyword[0] = L'\0';
			m_Types.m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
		}
	}

	//�^�C�v�ݒ�̕ύX��������
	m_Types.m_nRegexKeyMagicNumber++;
//	m_Types.m_nRegexKeyMagicNumber = 0;	//Not Compiled.

	return TRUE;
}

/*!
	@date 2010.07.11 Moca ���̂Ƃ���CRegexKeyword::RegexKeyCheckSyntax�Ɠ���Ȃ̂ŁA���g���폜���ē]���֐��ɕύX
*/
BOOL CPropRegex::RegexKakomiCheck(const wchar_t *s)
{
	return CRegexKeyword::RegexKeyCheckSyntax( s );
}
//@@@ 2001.11.17 add end MIK

