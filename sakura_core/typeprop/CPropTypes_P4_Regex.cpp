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
#include "sakura_rc.h"
//#include "types/CPropTypes.h"
#include "debug/Debug.h"
//#include <windows.h>
//#include <commctrl.h>
//#include "dlg/CDlgOpenFile.h"
//#include "CProfile.h"
//#include "env/CShareData.h"
//#include "func/Funccode.h"	//Stonee, 2001/05/18
//#include <stdio.h>	//@@@ 2001.11.17 add MIK
//#include "CRegexKeyword.h"	//@@@ 2001.11.17 add MIK
//#include "io/CTextStream.h"
#include "util/shell.h"
//#include "util/file.h"
#include "view/colors/CColorStrategy.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/23 Uchi

using namespace std;


#include "sakura.hh"
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
BOOL CPropTypes::Import_Regex(HWND hwndDlg)
{
//	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
//	CDlgOpenFile	cDlgOpenFile;
//	TCHAR			szPath[_MAX_PATH + 1]=_T("");
//	cDlgOpenFile.Create(
//		m_hInstance,
//		hwndDlg,
//		_T("*.rkw"),					// [R]egex [K]ey[W]ord
//		GetDllShareData().m_sHistory.m_szIMPORTFOLDER	// �C���|�[�g�p�t�H���_
//	);
//	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
//		return FALSE;
//	}
//
//	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
//	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
//	::SplitPath_FolderAndFile( szPath, GetDllShareData().m_sHistory.m_szIMPORTFOLDER, NULL );
//	_tcscat( GetDllShareData().m_sHistory.m_szIMPORTFOLDER, _T("\\") );
//
//
//	CTextInputStream in(szPath);
//	if(!in){
//	/*
//	FILE		*fp;
//	if( (fp = _tfopen(szPath, _T("r"))) == NULL )
//	{
//	*/
//		ErrorMessage( hwndDlg, _T("�t�@�C�����J���܂���ł����B\n\n%ts"), szPath );
//		return FALSE;
//	}
//
//	RegexKeywordInfo	pRegexKey[MAX_REGEX_KEYWORD];
//	TCHAR				buff[1024];
//	int					i, j, k;
//	j = 0;
//	while(in)
//	{
//		//1�s�ǂݍ���
//		wstring line=in.ReadLineW();
//		_wcstotcs(buff,line.c_str(),_countof(buff));
//
//		if(j >= MAX_REGEX_KEYWORD) break;
//
//		//RxKey[999]=ColorName,RegexKeyword
//		if( auto_strlen(buff) < 12 ) continue;
//		if( auto_memcmp(buff, _T("RxKey["), 6) != 0 ) continue;
//		if( auto_memcmp(&buff[9], _T("]="), 2) != 0 ) continue;
//		TCHAR	*p;
//		p = auto_strstr(&buff[11], _T(","));
//		if( p )
//		{
//			*p = _T('\0');
//			p++;
//			if( p[0] && RegexKakomiCheck(to_wchar(p)) )	//�݂͂�����
//			{
//				//�F�w�薼�ɑΉ�����ԍ���T��
//				k = GetColorIndexByName( &buff[11] );	//@@@ 2002.04.30
//				if( k != -1 )	/* 3�����J���[������C���f�b�N�X�ԍ��ɕϊ� */
//				{
//					pRegexKey[j].m_nColorIndex = k;
//					_tcstowcs(pRegexKey[j].m_szKeyword, p, _countof(pRegexKey[j].m_szKeyword));
//					j++;
//				}
//				else
//				{	/* ���{�ꖼ����C���f�b�N�X�ԍ��ɕϊ����� */
//					for(k = 0; k < COLORIDX_LAST; k++)
//					{
//						if( auto_strcmp(m_Types.m_ColorInfoArr[k].m_szName, &buff[11]) == 0 )
//						{
//							pRegexKey[j].m_nColorIndex = k;
//							_tcstowcs(pRegexKey[j].m_szKeyword, p, _countof(pRegexKey[j].m_szKeyword));
//							j++;
//							break;
//						}
//					}
//				}
//			}
//		}
//	}
//
//	in.Close();
//
//	HWND	hwndWork;
//	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_REGEX );
//	ListView_DeleteAllItems(hwndWork);  /* ���X�g����ɂ��� */
//	for(i = 0; i < j; i++)
//	{
//		LV_ITEM	lvi;
//		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
//		lvi.pszText  = const_cast<TCHAR*>(to_tchar(pRegexKey[i].m_szKeyword));
//		lvi.iItem    = i;
//		lvi.iSubItem = 0;
//		lvi.lParam   = 0;
//		ListView_InsertItem( hwndWork, &lvi );
//
//		lvi.mask     = LVIF_TEXT;
//		lvi.iItem    = i;
//		lvi.iSubItem = 1;
//		lvi.pszText  = m_Types.m_ColorInfoArr[pRegexKey[i].m_nColorIndex].m_szName;
//		ListView_SetItem( hwndWork, &lvi );
//	}
//	ListView_SetItemState( hwndWork, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	CImpExpRegex	cImpExpRegex( m_Types, GetDlgItem( hwndDlg, IDC_LIST_REGEX ));

	// �C���|�[�g
	if (!cImpExpRegex.ImportUI(m_hInstance, hwndDlg)) {
		// �C���|�[�g�����Ă��Ȃ�
		return FALSE;
	}

	return TRUE;
}

// Export
// 2010/4/23 Uchi Export�̊O�o��
BOOL CPropTypes::Export_Regex(HWND hwndDlg)
{
//	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
//	CDlgOpenFile	cDlgOpenFile;
//	TCHAR			szPath[_MAX_PATH + 1]=_T("");
//	cDlgOpenFile.Create(
//		m_hInstance,
//		hwndDlg,
//		_T("*.rkw"),					// [R]egex [K]ey[W]ord
//		GetDllShareData().m_sHistory.m_szIMPORTFOLDER	// �C���|�[�g�p�t�H���_
//	);
//	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
//		return FALSE;
//	}
//	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
//	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
//	::SplitPath_FolderAndFile( szPath, GetDllShareData().m_sHistory.m_szIMPORTFOLDER, NULL );
//	_tcscat( GetDllShareData().m_sHistory.m_szIMPORTFOLDER, _T("\\") );
//
//	CTextOutputStream out(szPath);
//	if(!out){
//		ErrorMessage( hwndDlg, _T("�t�@�C�����J���܂���ł����B\n\n%ts"), szPath );
//		return FALSE;
//	}
//
//	out.WriteF(L"// ���K�\���L�[���[�h Ver1\n");
//
//	HWND	hwndList;
//	hwndList = GetDlgItem( hwndDlg, IDC_LIST_REGEX );
//	int j = ListView_GetItemCount(hwndList);
//	for(int i = 0; i < j; i++)
//	{
//		TCHAR	szKeyWord[256];
//		auto_memset(szKeyWord, 0, _countof(szKeyWord));
//		ListView_GetItemText(hwndList, i, 0, szKeyWord, _countof(szKeyWord));
//
//		TCHAR	szColorIndex[256];
//		auto_memset(szColorIndex, 0, _countof(szColorIndex));
//		ListView_GetItemText(hwndList, i, 1, szColorIndex, _countof(szColorIndex));
//
//		const TCHAR* p = szColorIndex;
//		for(int k = 0; k < COLORIDX_LAST; k++)
//		{
//			if( _tcscmp( m_Types.m_ColorInfoArr[k].m_szName, szColorIndex ) == 0 )
//			{
//				p = GetColorNameByIndex(k);
//				break;
//			}
//		}
//		out.WriteF( L"RxKey[%03d]=%ts,%ts\n", i, p, szKeyWord);
//	}
//
//	out.Close();
//
//	InfoMessage( hwndDlg, _T("�t�@�C���փG�N�X�|�[�g���܂����B\n\n%ts"), szPath );

	CImpExpRegex	cImpExpRegex( m_Types, GetDlgItem( hwndDlg, IDC_LIST_REGEX ));

	// �G�N�X�|�[�g
	if (!cImpExpRegex.ExportUI(m_hInstance, hwndDlg)) {
		// �G�N�X�|�[�g�����Ă��Ȃ�
		return FALSE;
	}

	return TRUE;
}

/* ���K�\���L�[���[�h ���b�Z�[�W���� */
INT_PTR CPropTypes::DispatchEvent_Regex(
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

	TCHAR	szKeyWord[256];
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
		SetData_Regex( hwndDlg );	/* �_�C�A���O�f�[�^�̐ݒ� ���K�\���L�[���[�h */
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
				GetData_Regex( hwndDlg );
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
				GetData_Regex( hwndDlg );
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
				GetData_Regex( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_DEL:	/* �폜 */
				//�I�𒆂̃L�[�ԍ���T���B
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				//�폜����B
				ListView_DeleteItem( hwndList, nIndex );
				//�����ʒu�̃L�[��I����Ԃɂ���B
				ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData_Regex( hwndDlg );
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
				GetData_Regex( hwndDlg );
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
				GetData_Regex( hwndDlg );
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
				GetData_Regex( hwndDlg );
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
				GetData_Regex( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_IMPORT:	/* �C���|�[�g */
				Import_Regex(hwndDlg);
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK ���K�\���L�[���[�h�̂���
				return TRUE;

			case IDC_BUTTON_REGEX_EXPORT:	/* �G�N�X�|�[�g */
				Export_Regex(hwndDlg);
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
			GetData_Regex( hwndDlg );
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
								::SendMessageAny( hwndCombo, CB_SETCURSEL, (WPARAM)j, (LPARAM)0 );	/* �R���{�{�b�N�X�̃f�t�H���g�I�� */
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
								::SendMessageAny(hwndCombo, CB_SETCURSEL, j, 0);
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
void CPropTypes::SetData_Regex( HWND hwndDlg )
{
	HWND		hwndWork;
	int			i, j;
	LV_ITEM		lvi;
	DWORD		dwStyle;

	/* ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_REGEX ), EM_LIMITTEXT, _countof( m_Types.m_RegexKeywordArr[0].m_szKeyword ) - 1, (LPARAM)0 );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEX, _T("//k") );	/* ���K�\�� */

	/* �F��ނ̃��X�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
	::SendMessageAny( hwndWork, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0 );  /* �R���{�{�b�N�X����ɂ��� */
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		if ( 0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
		{
			j = ::SendMessage( hwndWork, CB_ADDSTRING, (WPARAM)0, (LPARAM)m_Types.m_ColorInfoArr[i].m_szName );
			if( m_Types.m_ColorInfoArr[i].m_nColorIdx == COLORIDX_REGEX1 )
				::SendMessageAny( hwndWork, CB_SETCURSEL, (WPARAM)j, (LPARAM)0 );	/* �R���{�{�b�N�X�̃f�t�H���g�I�� */
		}
	}

	if( m_Types.m_bUseRegexKeyword )
		CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_CHECKED );
	else
		CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_UNCHECKED );

	/* ���X�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	ListView_DeleteAllItems(hwndWork);  /* ���X�g����ɂ��� */

	/* �s�I�� */
	dwStyle = (DWORD)::SendMessageAny( hwndWork, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
	dwStyle |= LVS_EX_FULLROWSELECT;
	::SendMessageAny( hwndWork, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle );

	/* �f�[�^�\�� */
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
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
int CPropTypes::GetData_Regex( HWND hwndDlg )
{
	HWND	hwndList;
	int	nIndex, i, j;
	TCHAR	szKeyWord[256], szColorIndex[256];

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

BOOL CPropTypes::RegexKakomiCheck(const wchar_t *s)
{
	const wchar_t	*p;
	int	length, i;
	static const wchar_t *kakomi[7 * 2] = {
		L"/",  L"/k",
		L"m/", L"/k",
		L"m#", L"#k",
		L"/",  L"/ki",
		L"m/", L"/ki",
		L"m#", L"#ki",
		NULL, NULL,
	};

	length = wcslen(s);

	for(i = 0; kakomi[i] != NULL; i += 2)
	{
		//���������m���߂�
		if( length > (int)wcslen(kakomi[i]) + (int)wcslen(kakomi[i+1]) )
		{
			//�n�܂���m���߂�
			if( wcsncmp(kakomi[i], s, wcslen(kakomi[i])) == 0 )
			{
				//�I�����m���߂�
				p = &s[length - wcslen(kakomi[i+1])];
				if( wcscmp(p, kakomi[i+1]) == 0 )
				{
					//����
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

//@@@ 2001.11.17 add end MIK

