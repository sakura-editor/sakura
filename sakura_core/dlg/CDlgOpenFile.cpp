/*!	@file
	@brief �t�@�C���I�[�v���_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/08/10 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, Stonee, genta
	Copyright (C) 2002, MIK, YAZAKI, genta
	Copyright (C) 2003, MIK, KEITA, Moca, ryoji
	Copyright (C) 2004, genta
	Copyright (C) 2005, novice, ryoji
	Copyright (C) 2006, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "dlg/CDlgOpenFile.h"
#include "debug/Debug.h"
#include "sakura_rc.h"
#include "global.h"
#include "func/Funccode.h"	//Stonee, 2001/05/18
#include "CFileExt.h"
#include <dlgs.h>    // stc3,...
#include <cderr.h>   // FNERR...,CDERR...
#include "CEditApp.h"
#include "util/shell.h"
#include "util/file.h"
#include "util/os.h"

// �I�[�v���t�@�C�� CDlgOpenFile.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//13100
//	IDOK,					HIDOK_OPENDLG,		//Win�̃w���v�ŏ���ɏo�Ă���
//	IDCANCEL,				HIDCANCEL_OPENDLG,		//Win�̃w���v�ŏ���ɏo�Ă���
//	IDC_BUTTON_HELP,		HIDC_OPENDLG_BUTTON_HELP,		//�w���v�{�^��
	IDC_COMBO_CODE,			HIDC_OPENDLG_COMBO_CODE,		//�����R�[�h�Z�b�g
	IDC_COMBO_MRU,			HIDC_OPENDLG_COMBO_MRU,			//�ŋ߂̃t�@�C��
	IDC_COMBO_OPENFOLDER,	HIDC_OPENDLG_COMBO_OPENFOLDER,	//�ŋ߂̃t�H���_
	IDC_COMBO_EOL,			HIDC_OPENDLG_COMBO_EOL,			//���s�R�[�h
	IDC_CHECK_BOM,			HIDC_OPENDLG_CHECK_BOM,			//BOM	// 2006.08.06 ryoji
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

#ifndef OFN_ENABLESIZING
	#define OFN_ENABLESIZING	0x00800000
#endif

WNDPROC			m_wpOpenDialogProc;

std::vector<LPCTSTR>	m_vMRU;
std::vector<LPCTSTR>	m_vOPENFOLDER;
SFilePath		m_szHelpFile;
int				m_nHelpTopicID;
bool			m_bViewMode;		/* �r���[���[�h�� */
BOOL			m_bIsSaveDialog;	/* �ۑ��̃_�C�A���O�� */

COsVersionInfo CDlgOpenFile::m_cOsVer;	// 2005.11.02 ryoji




/*
|| 	�J���_�C�A���O�̃T�u�N���X�v���V�[�W��

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
LRESULT APIENTRY OFNHookProcMain( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	WPARAM					wCtlFocus;	/* �t�H�[�J�X�����R���g���[����ID */
//	BOOL					fHandle;	/* wParam����������t���O */
	int						idCtrl;
	OFNOTIFY*				pofn;
//	int						nIdx;
//	char*					pszWork;
	WORD					wNotifyCode;
	WORD					wID;
	HWND					hwndCtl;
//	HWND					hwndFrame;
	static DLLSHAREDATA*	pShareData;
	switch( uMsg ){
	case WM_MOVE:
		/* �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu */
		pShareData = CShareData::getInstance()->GetShareData();
		::GetWindowRect( hwnd, &pShareData->m_Common.m_sOthers.m_rcOpenDialog );
//		MYTRACE_A( "WM_MOVE 1\n" );
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// notification code
		wID = LOWORD(wParam);			// item, control, or accelerator identifier
		hwndCtl = (HWND) lParam;		// handle of control
		switch( wNotifyCode ){
//			break;
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case pshHelp:
				/* �w���v */
				MyWinHelp( hwnd, m_szHelpFile, HELP_CONTEXT, m_nHelpTopicID );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
				break;
			case chx1:	// The read-only check box
				m_bViewMode = ( 0 != ::IsDlgButtonChecked( hwnd , chx1 ) );
				break;
			}
			break;
		}
		break;
	case WM_NOTIFY:
		idCtrl = (int) wParam;
		pofn = (OFNOTIFY*) lParam;
//		MYTRACE_A( "=========WM_NOTIFY=========\n" );
//		MYTRACE_A( "pofn->hdr.hwndFrom=%xh\n", pofn->hdr.hwndFrom );
//		MYTRACE_A( "pofn->hdr.idFrom=%xh(%d)\n", pofn->hdr.idFrom, pofn->hdr.idFrom );
//		MYTRACE_A( "pofn->hdr.code=%xh(%d)\n", pofn->hdr.code, pofn->hdr.code );
		break;
	}
//	return ::CallWindowProc( (int (__stdcall *)( void ))(WNDPROC)m_wpOpenDialogProc, hwnd, uMsg, wParam, lParam );
	return ::CallWindowProc( (WNDPROC)m_wpOpenDialogProc, hwnd, uMsg, wParam, lParam );
}




/*!
	�J���_�C�A���O�̃t�b�N�v���V�[�W��
*/
// Modified by KEITA for WIN64 2003.9.6
// APIENTRY -> CALLBACK Moca 2003.09.09
//UINT APIENTRY OFNHookProc(
UINT_PTR CALLBACK OFNHookProc(
	HWND hdlg,		// handle to child dialog window
	UINT uiMsg,		// message identifier
	WPARAM wParam,	// message parameter
	LPARAM lParam 	// message parameter
)
{
	POINT					po;
//	RECT					rc0;
	RECT					rc;
	static OPENFILENAME*	pOf;
	static HWND				hwndOpenDlg;
	static HWND				hwndComboMRU;
	static HWND				hwndComboOPENFOLDER;
	static HWND				hwndComboCODES;
	static HWND				hwndComboEOL;	//	Feb. 9, 2001 genta
	static HWND				hwndCheckBOM;	//	Jul. 26, 2003 ryoji BOM�`�F�b�N�{�b�N�X
	static CDlgOpenFile*	pcDlgOpenFile;
	int						i;
	OFNOTIFY*				pofn;
	int						idCtrl;
//	HWND					hwndCtrl;
	LRESULT					lRes;
	const int				nExtraSize = 100;
	const int				nControls = 9;
	WORD					wNotifyCode;
	WORD					wID;
	HWND					hwndCtl;
	HWND					hwndFilebox;	// 2005.11.02 ryoji
	int						nIdx;
	int						nIdxSel;
	int						fwSizeType;
	int						nWidth;
	int						nHeight;
	WPARAM					fCheck;	//	Jul. 26, 2003 ryoji BOM��ԗp

	//	From Here	Feb. 9, 2001 genta
	const int			nEolValueArr[] = {
		EOL_NONE,
		EOL_CRLF,
		EOL_LF,
		EOL_CR,
	};
	//	�������Resource���ɓ����
	const TCHAR*	const	pEolNameArr[] = {
		_T("�ϊ��Ȃ�"),
		_T("CR+LF"),
		_T("LF (UNIX)"),
		_T("CR (Mac)"),
	};
	int nEolNameArrNum = (int)_countof(pEolNameArr);

//	To Here	Feb. 9, 2001 genta
	int	Controls[nControls] = {
		stc3, stc2,		// The two label controls
		edt1, cmb1,		// The edit control and the drop-down box
		IDOK, IDCANCEL,
		pshHelp,		// The Help command button (push button)
		lst1,			// The Explorer window
		chx1			// The read-only check box
	};
	int	nRightMargin = 24;
	HWND	hwndFrame;

	switch( uiMsg ){
	case WM_MOVE:
//		MYTRACE_A( "WM_MOVE 2\n" );
		break;
	case WM_SIZE:
		fwSizeType = wParam;		// resizing flag
		nWidth = LOWORD(lParam);	// width of client area
		nHeight = HIWORD(lParam);	// height of client area

		/* �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu */
		hwndFrame = ::GetParent( hdlg );
		::GetWindowRect( hwndFrame, &pcDlgOpenFile->m_pShareData->m_Common.m_sOthers.m_rcOpenDialog );

		// 2005.10.29 ryoji �ŋ߂̃t�@�C���^�t�H���_ �R���{�̉E�[���q�_�C�A���O�̉E�[�ɍ��킹��
		::GetWindowRect( hwndComboMRU, &rc );
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient( hdlg, &po );
		::SetWindowPos( hwndComboMRU, 0, 0, 0, nWidth - po.x - nRightMargin, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		::SetWindowPos( hwndComboOPENFOLDER, 0, 0, 0, nWidth - po.x - nRightMargin, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		return 0;
	case WM_INITDIALOG:
		// Save off the long pointer to the OPENFILENAME structure.
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hdlg, DWLP_USER, lParam);
		pOf = (OPENFILENAME*)lParam;
		pcDlgOpenFile = (CDlgOpenFile*)pOf->lCustData;


		/* Explorer�X�^�C���́u�J���v�_�C�A���O�̃n���h�� */
		hwndOpenDlg = ::GetParent( hdlg );
		/* �R���g���[���̃n���h�� */
		hwndComboCODES = ::GetDlgItem( hdlg, IDC_COMBO_CODE );
		hwndComboMRU = ::GetDlgItem( hdlg, IDC_COMBO_MRU );
		hwndComboOPENFOLDER = ::GetDlgItem( hdlg, IDC_COMBO_OPENFOLDER );
		hwndComboEOL = ::GetDlgItem( hdlg, IDC_COMBO_EOL );
		hwndCheckBOM = ::GetDlgItem( hdlg, IDC_CHECK_BOM );//	Jul. 26, 2003 ryoji BOM�`�F�b�N�{�b�N�X

		// 2005.11.02 ryoji �������C�A�E�g�ݒ�
		CDlgOpenFile::InitLayout( hwndOpenDlg, hdlg, hwndComboCODES );

		/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
		::SendMessageAny( hwndComboCODES,		CB_SETEXTENDEDUI, TRUE, 0 );
		::SendMessageAny( hwndComboMRU,			CB_SETEXTENDEDUI, TRUE, 0 );
		::SendMessageAny( hwndComboOPENFOLDER,	CB_SETEXTENDEDUI, TRUE, 0 );
		::SendMessageAny( hwndComboEOL,			CB_SETEXTENDEDUI, TRUE, 0 );

		//	From Here Feb. 9, 2001 genta
		//	���s�R�[�h�̑I���R���{�{�b�N�X������
		//	�K�v�ȂƂ��̂ݗ��p����
		if( pcDlgOpenFile->m_bUseEol ){
			//	�l�̐ݒ�
			for( i = 0; i < nEolNameArrNum; ++i ){
				nIdx = Combo_AddString( hwndComboEOL, pEolNameArr[i] );
				::SendMessageAny( hwndComboEOL, CB_SETITEMDATA, nIdx, nEolValueArr[i] );
			}
			//	�g���Ƃ��͐擪�̗v�f��I����Ԃɂ���
			::SendMessageAny( hwndComboEOL, CB_SETCURSEL, (WPARAM)0, 0 );
		}
		else {
			//	�g��Ȃ��Ƃ��͉B��
			::ShowWindow( ::GetDlgItem( hdlg, IDC_STATIC_EOL ), SW_HIDE );
			::ShowWindow( hwndComboEOL, SW_HIDE );
		}
		//	To Here Feb. 9, 2001 genta

		//	From Here Jul. 26, 2003 ryoji BOM�`�F�b�N�{�b�N�X�̏�����
		if( pcDlgOpenFile->m_bUseBom ){
			//	�g���Ƃ��͗L���^������؂�ւ��A�`�F�b�N��Ԃ������l�ɐݒ肷��
			switch( pcDlgOpenFile->m_nCharCode ){
			case CODE_UNICODE:
			case CODE_UTF8:
			case CODE_UNICODEBE:
				::EnableWindow( hwndCheckBOM, TRUE );
				fCheck = pcDlgOpenFile->m_bBom? BST_CHECKED: BST_UNCHECKED;
				break;
			default:
				::EnableWindow( hwndCheckBOM, FALSE );
				fCheck = BST_UNCHECKED;
				break;
			}
			::SendMessageAny( hwndCheckBOM, BM_SETCHECK, fCheck, 0 );
		}
		else {
			//	�g��Ȃ��Ƃ��͉B��
			::ShowWindow( hwndCheckBOM, SW_HIDE );
		}
		//	To Here Jul. 26, 2003 ryoji BOM�`�F�b�N�{�b�N�X�̏�����

		/* Explorer�X�^�C���́u�J���v�_�C�A���O���t�b�N */
		// Modified by KEITA for WIN64 2003.9.6
		m_wpOpenDialogProc = (WNDPROC) ::SetWindowLongPtr( hwndOpenDlg, GWLP_WNDPROC, (LONG_PTR) OFNHookProcMain );

		/* �����R�[�h�I���R���{�{�b�N�X������ */
		nIdxSel = 0;
		if( m_bIsSaveDialog ){	/* �ۑ��̃_�C�A���O�� */
			i = 1;
		}else{
			i = 0;
		}
		CCodeTypesForCombobox cCodeTypes;
		for( /*i = 0*/; i < cCodeTypes.GetCount(); ++i ){
			nIdx = Combo_AddString( hwndComboCODES, cCodeTypes.GetName(i) );
			::SendMessageAny( hwndComboCODES, CB_SETITEMDATA, nIdx, cCodeTypes.GetCode(i) );
			if( cCodeTypes.GetCode(i) == pcDlgOpenFile->m_nCharCode ){
				nIdxSel = nIdx;
			}
		}
		::SendMessageAny( hwndComboCODES, CB_SETCURSEL, (WPARAM)nIdxSel, 0 );


		/* �r���[���[�h�̏����l�Z�b�g */
		::CheckDlgButton( hwndOpenDlg, chx1, m_bViewMode );

		/* �ŋߊJ�����t�@�C�� �R���{�{�b�N�X�����l�ݒ� */
		//	2003.06.22 Moca m_vMRU ��NULL�̏ꍇ���l������
		for( i = 0; i < (int)m_vMRU.size(); i++ ){
			Combo_AddString( hwndComboMRU, m_vMRU[i] );
		}

		/* �ŋߊJ�����t�H���_ �R���{�{�b�N�X�����l�ݒ� */
		//	2003.06.22 Moca m_vOPENFOLDER ��NULL�̏ꍇ���l������
		for( i = 0; i < (int)m_vOPENFOLDER.size(); i++ ){
			Combo_AddString( hwndComboOPENFOLDER, m_vOPENFOLDER[i] );
		}

		break;


	case WM_DESTROY:
		/* �t�b�N���� */
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndOpenDlg, GWLP_WNDPROC, (LONG_PTR) m_wpOpenDialogProc );
		return FALSE;

	case WM_NOTIFY:
		idCtrl = (int) wParam;
		pofn = (OFNOTIFY*) lParam;
//		MYTRACE_A( "=========WM_NOTIFY=========\n" );
//		MYTRACE_A( "pofn->hdr.hwndFrom=%xh\n", pofn->hdr.hwndFrom );
//		MYTRACE_A( "pofn->hdr.idFrom=%xh(%d)\n", pofn->hdr.idFrom, pofn->hdr.idFrom );
//		MYTRACE_A( "pofn->hdr.code=%xh(%d)\n", pofn->hdr.code, pofn->hdr.code );

		switch( pofn->hdr.code ){
		case CDN_FILEOK:
			// �g���q�̕⊮�����O�ōs��	// 2006.11.10 ryoji
			if( m_bIsSaveDialog ){
				TCHAR szDefExt[_MAX_EXT];	// �⊮����g���q
				TCHAR szBuf[_MAX_PATH + _MAX_EXT];	// ���[�N
				LPTSTR pszCur, pszNext;
				int i;

				CommDlg_OpenSave_GetSpec(hwndOpenDlg, szBuf, _MAX_PATH);	// �t�@�C�������̓{�b�N�X���̕�����
				pszCur = szBuf;
				while( *pszCur == _T(' ') )	// �󔒂�ǂݔ�΂�
					pszCur = ::CharNext(pszCur);
				if( *pszCur == _T('\"') ){	// ��d���p���Ŏn�܂��Ă���
					::lstrcpyn(pcDlgOpenFile->m_szPath, pOf->lpstrFile, _MAX_PATH);
				}
				else{
					_tsplitpath( pOf->lpstrFile, NULL, NULL, NULL, szDefExt );
					if( szDefExt[0] == _T('.') /* && szDefExt[1] != _T('\0') */ ){	// ���Ɋg���q�����Ă���	2�����ڂ̃`�F�b�N�̍폜	2008/6/14 Uchi
						// .�݂̂̏ꍇ�ɂ��g���q�t���Ƃ݂Ȃ��B
						lstrcpyn(pcDlgOpenFile->m_szPath, pOf->lpstrFile, _MAX_PATH);
					}
					else{
						switch( pOf->nFilterIndex )	// �I������Ă���t�@�C���̎��
						{
						case 1:		// ���[�U�[��`
							pszCur = pcDlgOpenFile->m_szDefaultWildCard;
							while( *pszCur != _T('.') && *pszCur != _T('\0') )	// '.'�܂œǂݔ�΂�
								pszCur = ::CharNext(pszCur);
							i = 0;
							while( *pszCur != _T(';') && *pszCur != _T('\0') ){	// ';'�܂ŃR�s�[����
								pszNext = ::CharNext(pszCur);
								while( pszCur < pszNext )
									szDefExt[i++] = *pszCur++;
							}
							szDefExt[i] = _T('\0');
							if( ::_tcslen(szDefExt) < 2 || szDefExt[1] == _T('*') )	// �����Ȋg���q?
								szDefExt[0] = _T('\0');
							break;
						case 2:		// *.txt
							::_tcscpy(szDefExt, _T(".txt"));
							break;
						case 3:		// *.*
						default:	// �s��
							szDefExt[0] = _T('\0');
							break;
						}
						lstrcpyn(szBuf, pOf->lpstrFile, _MAX_PATH + 1);
						::_tcscat(szBuf, szDefExt);
						lstrcpyn(pcDlgOpenFile->m_szPath, szBuf, _MAX_PATH);
					}
				}

				// �t�@�C���̏㏑���m�F�����O�ōs��	// 2006.11.10 ryoji
				if( IsFileExists(pcDlgOpenFile->m_szPath, true) ){
					TCHAR szText[_MAX_PATH + 100];
					lstrcpyn(szText, pcDlgOpenFile->m_szPath, _MAX_PATH);
					::_tcscat(szText, _T(" �͊��ɑ��݂��܂��B\r\n�㏑�����܂����H"));
					if( IDYES != ::MessageBox( hwndOpenDlg, szText, _T("���O��t���ĕۑ�"), MB_YESNO | MB_ICONEXCLAMATION) ){
						::SetWindowLongPtr( hdlg, DWLP_MSGRESULT, TRUE );
						return TRUE;
					}
				}
			}

			/* �����R�[�h�I���R���{�{�b�N�X �l���擾 */
			nIdx = ::SendMessageAny( hwndComboCODES, CB_GETCURSEL, 0, 0 );
			lRes = ::SendMessageAny( hwndComboCODES, CB_GETITEMDATA, nIdx, 0 );
			pcDlgOpenFile->m_nCharCode = (ECodeType)lRes;	/* �����R�[�h */
			//	Feb. 9, 2001 genta
			if( pcDlgOpenFile->m_bUseEol ){
				nIdx = ::SendMessageAny( hwndComboEOL, CB_GETCURSEL, 0, 0 );
				lRes = ::SendMessageAny( hwndComboEOL, CB_GETITEMDATA, nIdx, 0 );
				pcDlgOpenFile->m_cEol = (enumEOLType)lRes;	/* �����R�[�h */
			}
			//	From Here Jul. 26, 2003 ryoji
			//	BOM�`�F�b�N�{�b�N�X�̏�Ԃ��擾
			if( pcDlgOpenFile->m_bUseBom ){
				lRes = ::SendMessageAny( hwndCheckBOM, BM_GETCHECK, 0, 0 );
				pcDlgOpenFile->m_bBom = (lRes == BST_CHECKED);	/* BOM */
			}
			//	To Here Jul. 26, 2003 ryoji

//			MYTRACE_A( "�����R�[�h  lRes=%d\n", lRes );
//			MYTRACE_A( "pofn->hdr.code=CDN_FILEOK        \n" );break;
			break;	/* CDN_FILEOK */

		case CDN_FOLDERCHANGE  :
//			MYTRACE_A( "pofn->hdr.code=CDN_FOLDERCHANGE  \n" );
			{
				wchar_t szFolder[_MAX_PATH];
				lRes = ::SendMessage( hwndOpenDlg, CDM_GETFOLDERPATH, _countof( szFolder ), (LPARAM)szFolder );
			}
//			MYTRACE_A( "\tlRes=%d\tszFolder=[%ls]\n", lRes, szFolder );

			break;
//		case CDN_HELP			:	MYTRACE_A( "pofn->hdr.code=CDN_HELP          \n" );break;
//		case CDN_INITDONE		:	MYTRACE_A( "pofn->hdr.code=CDN_INITDONE      \n" );break;
//		case CDN_SELCHANGE		:	MYTRACE_A( "pofn->hdr.code=CDN_SELCHANGE     \n" );break;
//		case CDN_SHAREVIOLATION	:	MYTRACE_A( "pofn->hdr.code=CDN_SHAREVIOLATION\n" );break;
//		case CDN_TYPECHANGE		:	MYTRACE_A( "pofn->hdr.code=CDN_TYPECHANGE    \n" );break;
//		default:					MYTRACE_A( "pofn->hdr.code=???\n" );break;

		}

//		MYTRACE_A( "=======================\n" );
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// notification code
		wID = LOWORD(wParam);			// item, control, or accelerator identifier
		hwndCtl = (HWND) lParam;		// handle of control
		switch( wNotifyCode ){
		case CBN_SELCHANGE:
			switch( (int) LOWORD(wParam) ){
			//	From Here Jul. 26, 2003 ryoji
			//	�����R�[�h�̕ύX��BOM�`�F�b�N�{�b�N�X�ɔ��f
			case IDC_COMBO_CODE:
				nIdx = ::SendMessageAny( (HWND) lParam, CB_GETCURSEL, 0, 0 );
				lRes = ::SendMessageAny( (HWND) lParam, CB_GETITEMDATA, nIdx, 0 );
				switch( lRes ){
				case CODE_UNICODE:
				case CODE_UTF8:
				case CODE_UNICODEBE:
					::EnableWindow( hwndCheckBOM, TRUE );
					if (lRes == pcDlgOpenFile->m_nCharCode){
						fCheck = pcDlgOpenFile->m_bBom? BST_CHECKED: BST_UNCHECKED;
					}else{
						fCheck = (lRes == CODE_UTF8)? BST_UNCHECKED: BST_CHECKED;
					}
					break;
				default:
					::EnableWindow( hwndCheckBOM, FALSE );
					fCheck = BST_UNCHECKED;
					break;
				}
				::SendMessageAny( hwndCheckBOM, BM_SETCHECK, fCheck, 0 );
				break;
			//	To Here Jul. 26, 2003 ryoji
			case IDC_COMBO_MRU:
			case IDC_COMBO_OPENFOLDER:
				{
					TCHAR	szWork[_MAX_PATH + 1];
					nIdx = ::SendMessageAny( (HWND) lParam, CB_GETCURSEL, 0, 0 );

					if( CB_ERR != Combo_GetText( (HWND) lParam, nIdx, szWork ) ){
						// 2005.11.02 ryoji �t�@�C�����w��̃R���g���[�����m�F����
						hwndFilebox = ::GetDlgItem( hwndOpenDlg, cmb13 );		// �t�@�C�����R���{�iWindows 2000�^�C�v�j
						if( !::IsWindow( hwndFilebox ) )
							hwndFilebox = ::GetDlgItem( hwndOpenDlg, edt1 );	// �t�@�C�����G�f�B�b�g�i���K�V�[�^�C�v�j
						if( ::IsWindow( hwndFilebox ) ){
							::SetWindowText( hwndFilebox, szWork );
							if( IDC_COMBO_OPENFOLDER == wID )
								::PostMessageAny( hwndFilebox, WM_KEYDOWN, VK_RETURN, (LPARAM)0 );
						}
					}
				}
				break;
			}
			break;	/* CBN_SELCHANGE */
		case CBN_DROPDOWN:
			switch( wID ){
			case IDC_COMBO_MRU:
			case IDC_COMBO_OPENFOLDER:
				CDlgOpenFile::OnCmbDropdown( hwndCtl );
				break;
			}
			break;	/* CBN_DROPDOWN */
		}
		break;	/* WM_COMMAND */

	//@@@ 2002.01.08 add start
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;

	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hdlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	//@@@ 2002.01.08 add end

	default:
		return FALSE;
	}
	return TRUE;
}





/*! �R���X�g���N�^
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
CDlgOpenFile::CDlgOpenFile()
{
	/* �����o�̏����� */
	long	lPathLen;
//	int		nCharChars;

	m_nCharCode = CODE_AUTODETECT;	/* �����R�[�h *//* �����R�[�h�������� */


	m_hInstance = NULL;		/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = NULL;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_hWnd = NULL;			/* ���̃_�C�A���O�̃n���h�� */

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	/* OPENFILENAME�̏����� */
	InitOfn( &m_ofn );		// 2005.10.29 ryoji
	m_ofn.nFilterIndex = 1;	//Jul. 09, 2001 JEPRO		/* �u�J���v�ł̍ŏ��̃��C���h�J�[�h */

	TCHAR	szFile[_MAX_PATH + 1];
	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	lPathLen = ::GetModuleFileName(
		NULL,
		szFile, _countof( szFile )
	);
	_tsplitpath( szFile, szDrive, szDir, NULL, NULL );
	_tcscpy( m_szInitialDir, szDrive );
	_tcscat( m_szInitialDir, szDir );



	_tcscpy( m_szDefaultWildCard, _T("*.*") );	/*�u�J���v�ł̍ŏ��̃��C���h�J�[�h�i�ۑ����̊g���q�⊮�ł��g�p�����j */

	/* �w���v�t�@�C���̃t���p�X��Ԃ� */
	m_szHelpFile = CEditApp::Instance()->GetHelpFilePath();
	m_nHelpTopicID = 0;

	return;
}





CDlgOpenFile::~CDlgOpenFile()
{
	return;
}


#if 0
/*! ������
	DoModal_GetSaveFileName/DoModal_GetOpenFileName�p
	
	@author Moca
	@date 2003.06.23
*/
void CDlgOpenFile::Create(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	const char*		pszUserWildCard,
	const char*		pszDefaultPath
)
{
	Create(
		hInstance,
		hwndParent,
		pszUserWildCard,
		pszDefaultPath,
		NULL,
		NULL
	);
}
#endif

/* ������ */
void CDlgOpenFile::Create(
	HINSTANCE					hInstance,
	HWND						hwndParent,
	const TCHAR*				pszUserWildCard,
	const TCHAR*				pszDefaultPath,
	const std::vector<LPCTSTR>& vMRU,
	const std::vector<LPCTSTR>& vOPENFOLDER
)
{
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;

	/* ���[�U�[��`���C���h�J�[�h�i�ۑ����̊g���q�⊮�ł��g�p�����j */
	if( NULL != pszUserWildCard ){
		_tcscpy( m_szDefaultWildCard, pszUserWildCard );
	}

	/* �u�J���v�ł̏����t�H���_ */
	if( pszDefaultPath && 0 < _tcslen( pszDefaultPath ) ){	//���ݕҏW���̃t�@�C���̃p�X	//@@@ 2002.04.18
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		//	Jun. 23, 2002 genta
		my_splitpath_t( pszDefaultPath, szDrive, szDir, NULL, NULL );
		auto_sprintf( m_szInitialDir, _T("%ts%ts"), szDrive, szDir );
	}
	m_vMRU = vMRU;
	m_vOPENFOLDER = vOPENFOLDER;
	return;
}




/*! �u�J���v�_�C�A���O ���[�_���_�C�A���O�̕\��

	@param[in,out] pszPath �����t�@�C�����D�I�����ꂽ�t�@�C�����̊i�[�ꏊ
	@param[in] bSetCurDir �J�����g�f�B���N�g����ύX���邩 �f�t�H���g: false
	@date 2002/08/21 �J�����g�f�B���N�g����ύX���邩�ǂ����̃I�v�V������ǉ�
	@date 2003.05.12 MIK �g���q�t�B���^�Ń^�C�v�ʐݒ�̊g���q���g���悤�ɁB
		�g���q�t�B���^�̊Ǘ���CFileExt�N���X�ōs���B
	@date 2005/02/20 novice �g���q���ȗ�������⊮����
*/
bool CDlgOpenFile::DoModal_GetOpenFileName( TCHAR* pszPath , bool bSetCurDir )
{
	//�J�����g�f�B���N�g����ۑ��B�֐����甲����Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕��������B
	CCurrentDirectoryBackupPoint cCurDirBackup;

	//	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( _T("���[�U�[�w��"),     m_szDefaultWildCard );
	cFileExt.AppendExtRaw( _T("�e�L�X�g�t�@�C��"), _T("*.txt") );
	cFileExt.AppendExtRaw( _T("���ׂẴt�@�C��"), _T("*.*") );

	/* �\���̂̏����� */
	InitOfn( &m_ofn );		// 2005.10.29 ryoji
	m_ofn.hwndOwner = m_hwndParent;
	m_ofn.hInstance = m_hInstance;
	m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	// From Here Jun. 23, 2002 genta
	// �u�J���v�ł̏����t�H���_�`�F�b�N����
// 2005/02/20 novice �f�t�H���g�̃t�@�C�����͉����ݒ肵�Ȃ�
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szName[_MAX_FNAME];
	TCHAR szExt  [_MAX_EXT];

	//	Jun. 23, 2002 Thanks to sui
	my_splitpath_t( pszPath, szDrive, szDir, szName, szExt );
	
	//	�w�肳�ꂽ�t�@�C�������݂��Ȃ��Ƃ� szName == NULL
	//	�t�@�C���̏ꏊ�Ƀf�B���N�g�����w�肷��ƃG���[�ɂȂ�̂�
	//	�t�@�C���������ꍇ�͑S���w�肵�Ȃ����Ƃɂ���D
	if( szName[0] == _T('\0') ){
		pszPath[0] = _T('\0');
	}
	else {
		auto_sprintf( pszPath, _T("%ls%ls%ls%ls"), szDrive, szDir, szName, szExt );
	}
	m_ofn.lpstrFile = pszPath;
	// To Here Jun. 23, 2002 genta
	m_ofn.nMaxFile = _MAX_PATH;
	m_ofn.lpstrInitialDir = m_szInitialDir;
	m_ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	m_ofn.lpstrDefExt = _T(""); // 2005/02/20 novice �g���q���ȗ�������⊮����
	if( _GetOpenFileNameRecover( &m_ofn ) ){
		return true;
	}
	else{
		//	May 29, 2004 genta �֐��ɂ܂Ƃ߂�
		DlgOpenFail();
		return false;
	}
}


/*! �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\��
	@param pszPath [i/o] �����t�@�C�����D�I�����ꂽ�t�@�C�����̊i�[�ꏊ
	@param bSetCurDir [in] �J�����g�f�B���N�g����ύX���邩 �f�t�H���g: false
	@date 2002/08/21 �J�����g�f�B���N�g����ύX���邩�ǂ����̃I�v�V������ǉ�
	@date 2003.05.12 MIK �g���q�t�B���^�Ń^�C�v�ʐݒ�̊g���q���g���悤�ɁB
		�g���q�t�B���^�̊Ǘ���CFileExt�N���X�ōs���B
	@date 2005/02/20 novice �g���q���ȗ�������⊮����
*/
bool CDlgOpenFile::DoModal_GetSaveFileName( TCHAR* pszPath, bool bSetCurDir )
{
	//�J�����g�f�B���N�g����ۑ��B�֐����甲����Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕��������B
	CCurrentDirectoryBackupPoint cCurDirBackup;

	//	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( _T("���[�U�[�w��"),     m_szDefaultWildCard );
	cFileExt.AppendExtRaw( _T("�e�L�X�g�t�@�C��"), _T("*.txt") );
	cFileExt.AppendExtRaw( _T("���ׂẴt�@�C��"), _T("*.*") );

	/* �\���̂̏����� */
	InitOfn( &m_ofn );		// 2005.10.29 ryoji
	m_ofn.hwndOwner = m_hwndParent;
	m_ofn.hInstance = m_hInstance;
	m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	m_ofn.lpstrFile = pszPath; // 2005/02/20 novice �f�t�H���g�̃t�@�C�����͉����ݒ肵�Ȃ�
	m_ofn.nMaxFile = _MAX_PATH;
	m_ofn.lpstrInitialDir = m_szInitialDir;
	m_ofn.Flags = OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	m_ofn.lpstrDefExt = _T("");	// 2005/02/20 novice �g���q���ȗ�������⊮����
	if( GetSaveFileNameRecover( &m_ofn ) ){
		return true;
	}
	else{
		//	May 29, 2004 genta �֐��ɂ܂Ƃ߂�
		DlgOpenFail();
		return false;
	}
}





/*! �u�J���v�_�C�A���O ���[�_���_�C�A���O�̕\��
	@date 2003.05.12 MIK �g���q�t�B���^�Ń^�C�v�ʐݒ�̊g���q���g���悤�ɁB
		�g���q�t�B���^�̊Ǘ���CFileExt�N���X�ōs���B
	@date 2005/02/20 novice �g���q���ȗ�������⊮����
*/
bool CDlgOpenFile::DoModalOpenDlg( SLoadInfo* pLoadInfo )
{
	m_bIsSaveDialog = FALSE;	/* �ۑ��̃_�C�A���O�� */

	// �t�@�C���̎��	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( _T("���ׂẴt�@�C��"), _T("*.*") );
	cFileExt.AppendExtRaw( _T("�e�L�X�g�t�@�C��"), _T("*.txt") );
	for( int i = 0; i < MAX_TYPES; i++ ){
		const STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(i));
		cFileExt.AppendExt( types.m_szTypeName, types.m_szTypeExts );
	}

	//�����o�̏�����
	m_bViewMode = pLoadInfo->bViewMode;
	m_nCharCode = pLoadInfo->eCharCode;	/* �����R�[�h�������� */
	m_nHelpTopicID = ::FuncID_To_HelpContextID(F_FILEOPEN);	//Stonee, 2001/05/18 �@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
	m_bUseEol = false;	//	Feb. 9, 2001 genta
	m_bUseBom = false;	//	Jul. 26, 2003 ryoji

	//�t�@�C���p�X�󂯎��o�b�t�@
	TCHAR szPathBuf[_MAX_PATH] = _T("");

	//OPENFILENAME�\���̂̏�����
	InitOfn( &m_ofn );		// 2005.10.29 ryoji
	m_ofn.hwndOwner = m_hwndParent;
	m_ofn.hInstance = m_hInstance;
	m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	m_ofn.lpstrFile = szPathBuf;	// 2005/02/20 novice �f�t�H���g�̃t�@�C�����͉����ݒ肵�Ȃ�
	m_ofn.nMaxFile = _countof(szPathBuf);
	m_ofn.lpstrInitialDir = m_szInitialDir;
	m_ofn.Flags = OFN_EXPLORER | OFN_CREATEPROMPT | OFN_FILEMUSTEXIST | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_SHOWHELP | OFN_ENABLESIZING;
	if( m_bViewMode )m_ofn.Flags |= OFN_READONLY;
	m_ofn.lpstrDefExt = _T("");	// 2005/02/20 novice �g���q���ȗ�������⊮����

	//�J�����g�f�B���N�g����ۑ��B�֐��𔲂���Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕�������܂��B
	CCurrentDirectoryBackupPoint cCurDirBackup;

	//�_�C�A���O�\��
	bool bDlgResult = _GetOpenFileNameRecover( &m_ofn );
	if( bDlgResult ){
		pLoadInfo->cFilePath = m_ofn.lpstrFile;
		pLoadInfo->eCharCode = m_nCharCode;
		pLoadInfo->bViewMode = m_bViewMode;
	}
	else{
		DlgOpenFail();
	}
	return bDlgResult;
}

/*! �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\��

	@date 2001.02.09 genta	�����ǉ�
	@date 2003.05.12 MIK �g���q�t�B���^�Ń^�C�v�ʐݒ�̊g���q���g���悤�ɁB
		�g���q�t�B���^�̊Ǘ���CFileExt�N���X�ōs���B
	@date 2003.07.26 ryoji BOM�p�����[�^�ǉ�
	@date 2005/02/20 novice �g���q���ȗ�������⊮����
	@date 2006.11.10 ryoji �t�b�N���g���ꍇ�͊g���q�̕⊮�����O�ōs��
		Windows�Ŋ֘A�t���������悤�Ȋg���q���w�肵�ĕۑ�����ƁA�����I��
		�g���q���͂��Ă���̂Ƀf�t�H���g�g���q���⊮����Ă��܂����Ƃ�����B
			��jhoge.abc -> hoge.abc.txt
		���O�ŕ⊮���邱�Ƃł�����������B�i���ۂ̏����̓t�b�N�v���V�[�W���̒��j
*/
bool CDlgOpenFile::DoModalSaveDlg(SSaveInfo* pSaveInfo, bool bSimpleMode)
{
	m_bIsSaveDialog = TRUE;	/* �ۑ��̃_�C�A���O�� */

	//	2003.05.12 MIK
	CFileExt	cFileExt;
	cFileExt.AppendExtRaw( _T("���[�U�[�w��"),     m_szDefaultWildCard );
	cFileExt.AppendExtRaw( _T("�e�L�X�g�t�@�C��"), _T("*.txt") );
	cFileExt.AppendExtRaw( _T("���ׂẴt�@�C��"), _T("*.*") );

	// �t�@�C�����̏����ݒ�	// 2006.11.10 ryoji
	if( pSaveInfo->cFilePath[0] == _T('\0') )
		lstrcpyn(pSaveInfo->cFilePath, _T("����"), _MAX_PATH);

	//OPENFILENAME�\���̂̏�����
	InitOfn( &m_ofn );		// 2005.10.29 ryoji
	m_ofn.hwndOwner = m_hwndParent;
	m_ofn.hInstance = m_hInstance;
	m_ofn.lpstrFilter = cFileExt.GetExtFilter();
	m_ofn.lpstrFile = pSaveInfo->cFilePath;	// 2005/02/20 novice �f�t�H���g�̃t�@�C�����͉����ݒ肵�Ȃ�
	m_ofn.nMaxFile = _MAX_PATH;
	m_ofn.lpstrInitialDir = m_szInitialDir;
	m_ofn.Flags = OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_SHOWHELP | OFN_ENABLESIZING;
	if( !bSimpleMode ){
		m_ofn.Flags = m_ofn.Flags | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
		m_ofn.Flags &= ~OFN_OVERWRITEPROMPT;	// 2006.11.10 ryoji �㏑���m�F���t�b�N�̒��Ŏ��O�ŏ�������
	}


// 2005/02/20 novice �g���q���ȗ�������⊮����
//	m_ofn.lpstrDefExt = _T("");
	m_ofn.lpstrDefExt = (m_ofn.Flags & OFN_ENABLEHOOK)? NULL: _T("");	// 2006.11.10 ryoji �t�b�N���g���Ƃ��͎��O�Ŋg���q��⊮����

	//�J�����g�f�B���N�g����ۑ��B�֐����甲����Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕��������B
	CCurrentDirectoryBackupPoint cCurDirBackup;

	m_nCharCode = pSaveInfo->eCharCode;

	//	From Here Feb. 9, 2001 genta
	if(!bSimpleMode){
		m_cEol = EOL_NONE;	//	�����l�́u���s�R�[�h��ۑ��v�ɌŒ�
		m_bUseEol = true;
	}
	else{
		m_bUseEol = false;
	}

	//	To Here Feb. 9, 2001 genta
	//	Jul. 26, 2003 ryoji BOM�ݒ�
	if(!bSimpleMode){
		m_bBom = pSaveInfo->bBomExist;
		m_bUseBom = true;
	}
	else{
		m_bUseBom = false;
	}

	m_nHelpTopicID = ::FuncID_To_HelpContextID(F_FILESAVEAS_DIALOG);	//Stonee, 2001/05/18 �@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
	if( GetSaveFileNameRecover( &m_ofn ) ){
		pSaveInfo->cFilePath = m_ofn.lpstrFile;
		if( m_ofn.Flags & OFN_ENABLEHOOK )
			lstrcpyn(pSaveInfo->cFilePath, m_szPath, _MAX_PATH);	// ���O�Ŋg���q�̕⊮���s�����Ƃ��̃t�@�C���p�X	// 2006.11.10 ryoji

		pSaveInfo->eCharCode = m_nCharCode;

		//	Feb. 9, 2001 genta
		if( m_bUseEol ){
			pSaveInfo->cEol = m_cEol;
		}
		//	Jul. 26, 2003 ryoji BOM�ݒ�
		if( m_bUseBom ){
			pSaveInfo->bBomExist = m_bBom;
		}
		return true;
	}
	else{
		//	May 29, 2004 genta �֐��ɂ܂Ƃ߂�
		DlgOpenFail();
		return false;
	}
}

/*! @brief �R�����_�C�A���O�{�b�N�X���s����

	�R�����_�C�A���O�{�b�N�X����FALSE���Ԃ��ꂽ�ꍇ��
	�G���[�����𒲂ׂăG���[�Ȃ烁�b�Z�[�W���o���D
	
	@author genta
	@date 2004.05.29 genta ���X�������������܂Ƃ߂�
*/
void CDlgOpenFile::DlgOpenFail(void)
{
	const TCHAR*	pszError;
	DWORD dwError = ::CommDlgExtendedError();
	if( dwError == 0 ){
		//	���[�U�L�����Z���ɂ��
		return;
	}
	
	switch( dwError ){
	case CDERR_DIALOGFAILURE  : pszError = _T("CDERR_DIALOGFAILURE  "); break;
	case CDERR_FINDRESFAILURE : pszError = _T("CDERR_FINDRESFAILURE "); break;
	case CDERR_NOHINSTANCE    : pszError = _T("CDERR_NOHINSTANCE    "); break;
	case CDERR_INITIALIZATION : pszError = _T("CDERR_INITIALIZATION "); break;
	case CDERR_NOHOOK         : pszError = _T("CDERR_NOHOOK         "); break;
	case CDERR_LOCKRESFAILURE : pszError = _T("CDERR_LOCKRESFAILURE "); break;
	case CDERR_NOTEMPLATE     : pszError = _T("CDERR_NOTEMPLATE     "); break;
	case CDERR_LOADRESFAILURE : pszError = _T("CDERR_LOADRESFAILURE "); break;
	case CDERR_STRUCTSIZE     : pszError = _T("CDERR_STRUCTSIZE     "); break;
	case CDERR_LOADSTRFAILURE : pszError = _T("CDERR_LOADSTRFAILURE "); break;
	case FNERR_BUFFERTOOSMALL : pszError = _T("FNERR_BUFFERTOOSMALL "); break;
	case CDERR_MEMALLOCFAILURE: pszError = _T("CDERR_MEMALLOCFAILURE"); break;
	case FNERR_INVALIDFILENAME: pszError = _T("FNERR_INVALIDFILENAME"); break;
	case CDERR_MEMLOCKFAILURE : pszError = _T("CDERR_MEMLOCKFAILURE "); break;
	case FNERR_SUBCLASSFAILURE: pszError = _T("FNERR_SUBCLASSFAILURE"); break;
	default: pszError = _T("UNKNOWN_ERRORCODE"); break;
	}

	ErrorBeep();
	TopErrorMessage( m_hwndParent,
		_T("�_�C�A���O���J���܂���B\n")
		_T("\n")
		_T("�G���[:%ls"),
		pszError
	);
}

/*! OPENFILENAME ������

	OPENFILENAME �� CDlgOpenFile �N���X�p�̏����K��l��ݒ肷��

	@author ryoji
	@date 2005.10.29
*/
void CDlgOpenFile::InitOfn( OPENFILENAMEZ* ofn )
{
	memset_raw(ofn, 0, sizeof(m_ofn));

	ofn->lStructSize = IsOfnV5()? sizeof(OPENFILENAMEZ): OPENFILENAME_SIZE_VERSION_400;
	ofn->lCustData = (LPARAM)this;
	ofn->lpfnHook = OFNHookProc;
	ofn->lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPEN);	// <-_T("IDD_FILEOPEN"); 2008/7/26 Uchi
}

/*! �������C�A�E�g�ݒ菈��

	�ǉ��R���g���[���̃��C�A�E�g��ύX����

	@param hwndOpenDlg [in]		�t�@�C���_�C�A���O�̃E�B���h�E�n���h��
	@param hwndDlg [in]			�q�_�C�A���O�̃E�B���h�E�n���h��
	@param hwndBaseCtrl [in]	�ړ���R���g���[���i�t�@�C�����{�b�N�X�ƍ��[�����킹��R���g���[���j�̃E�B���h�E�n���h��

	@author ryoji
	@date 2005.11.02
*/
void CDlgOpenFile::InitLayout( HWND hwndOpenDlg, HWND hwndDlg, HWND hwndBaseCtrl )
{
	HWND hwndFilelabel;
	HWND hwndFilebox;
	HWND hwndCtrl;
	RECT rcBase;
	RECT rc;
	POINT po;
	int nLeft;
	int nShift;
	int nWidth;

	// �t�@�C�������x���ƃt�@�C�����{�b�N�X���擾����
	if( !::IsWindow( hwndFilelabel = ::GetDlgItem( hwndOpenDlg, stc3 ) ) )		// �t�@�C�������x��
		return;
	if( !::IsWindow( hwndFilebox = ::GetDlgItem( hwndOpenDlg, cmb13 ) ) ){		// �t�@�C�����R���{�iWindows 2000�^�C�v�j
		if( !::IsWindow( hwndFilebox = ::GetDlgItem( hwndOpenDlg, edt1 ) ) )	// �t�@�C�����G�f�B�b�g�i���K�V�[�^�C�v�j
			return;
	}

	// �R���g���[���̊�ʒu�A�ړ��ʂ����肷��
	::GetWindowRect( hwndFilelabel, &rc );
	nLeft = rc.left;						// ���[�ɑ�����R���g���[���̈ʒu
	::GetWindowRect( hwndFilebox, &rc );
	::GetWindowRect( hwndBaseCtrl, &rcBase );
	nShift = rc.left - rcBase.left;			// ���[�ȊO�̃R���g���[���̉E�����ւ̑��Έړ���

	// �ǉ��R���g���[�������ׂĈړ�����
	// �E��R���g���[���������ɂ�����̂̓t�@�C�������x���ɍ��킹�č��[�Ɉړ�
	// �E���̑��͈ړ���R���g���[���i�t�@�C�����{�b�N�X�ƍ��[�����킹��R���g���[���j�Ɠ��������E�����֑��Έړ�
	hwndCtrl = ::GetWindow( hwndDlg, GW_CHILD );
	while( hwndCtrl ){
		if( ::GetDlgCtrlID(hwndCtrl) != stc32 ){
			::GetWindowRect( hwndCtrl, &rc );
			po.x = ( rc.right < rcBase.left )? nLeft: rc.left + nShift;
			po.y = rc.top;
			::ScreenToClient( hwndDlg, &po );
			::SetWindowPos( hwndCtrl, 0, po.x, po.y, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER );
		}
		hwndCtrl = ::GetWindow( hwndCtrl, GW_HWNDNEXT );
	}


	// �W���R���g���[���̃v���[�X�t�H���_�istc32�j�Ǝq�_�C�A���O�̕����I�[�v���_�C�A���O�̕��ɂ��킹��
	//     WM_INITDIALOG �𔲂���Ƃ���ɃI�[�v���_�C�A���O���Ō��݂̈ʒu�֌W���烌�C�A�E�g�������s����
	//     �����ňȉ��̏���������Ă����Ȃ��ƃR���g���[�����Ӑ}���Ȃ��ꏊ�ɓ����Ă��܂����Ƃ�����
	//     �i�Ⴆ�΁ABOM �̃`�F�b�N�{�b�N�X����ʊO�ɔ��ł��܂��Ȃǁj

	// �I�[�v���_�C�A���O�̃N���C�A���g�̈�̕����擾����
	::GetClientRect( hwndOpenDlg, &rc );
	nWidth = rc.right - rc.left;

	// �W���R���g���[���v���[�X�t�H���_�̕���ύX����
	hwndCtrl = ::GetDlgItem( hwndDlg, stc32 );
	::GetWindowRect( hwndCtrl, &rc );
	::SetWindowPos( hwndCtrl, 0, 0, 0, nWidth, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );

	// �q�_�C�A���O�̕���ύX����
	// ������ SetWindowPos() �̒��� WM_SIZE ����������
	::GetWindowRect( hwndDlg, &rc );
	::SetWindowPos( hwndDlg, 0, 0, 0, nWidth, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
}

/*! �R���{�{�b�N�X�̃h���b�v�_�E��������

	�R���{�{�b�N�X���h���b�v�_�E������鎞��
	�h���b�v�_�E�����X�g�̕����A�C�e��������̍ő�\�����ɍ��킹��

	@param hwnd [in]		�R���{�{�b�N�X�̃E�B���h�E�n���h��

	@author ryoji
	@date 2005.10.29
*/
void CDlgOpenFile::OnCmbDropdown( HWND hwnd )
{
	HDC hDC;
	HFONT hFont;
	LONG nWidth;
	RECT rc;
	SIZE sizeText;
	int nTextLen;
	int iItem;
	int nItem;

	hDC = ::GetDC( hwnd );
	if( NULL == hDC )
		return;
	hFont = (HFONT)::SendMessageAny( hwnd, WM_GETFONT, 0, NULL );
	hFont = (HFONT)::SelectObject( hDC, hFont );
	nItem = ::SendMessageAny( hwnd, CB_GETCOUNT, 0, NULL );
	::GetWindowRect( hwnd, &rc );
	nWidth = rc.right - rc.left - 8;
	for( iItem = 0; iItem < nItem; iItem++ ){
		nTextLen = ::SendMessage( hwnd, CB_GETLBTEXTLEN, (WPARAM)iItem, NULL );
		if( 0 < nTextLen ) {
			TCHAR* pszText = new TCHAR[nTextLen + 1];
			Combo_GetText( hwnd, iItem, pszText );
			if( ::GetTextExtentPoint32( hDC, pszText, nTextLen, &sizeText ) ){
				if ( nWidth < sizeText.cx )
					nWidth = sizeText.cx;
			}
			delete []pszText;
		}
	}
	::SendMessageAny( hwnd, CB_SETDROPPEDWIDTH, (WPARAM)(nWidth + 8), NULL );
	::SelectObject( hDC, hFont );
	::ReleaseDC( hwnd, hDC );
}

/*! ���g���C�@�\�t�� GetOpenFileName
	@author Moca
	@date 2006.09.03 �V�K�쐬
*/
bool CDlgOpenFile::_GetOpenFileNameRecover( OPENFILENAMEZ* ofn )
{
	BOOL bRet = ::GetOpenFileName( ofn );
	if( !bRet  ){
		if( FNERR_INVALIDFILENAME == ::CommDlgExtendedError() ){
			_tcscpy( ofn->lpstrFile, _T("") );
			ofn->lpstrInitialDir = _T("");
			bRet = ::GetOpenFileName( ofn );
		}
	}
	return bRet!=FALSE;
}

/*! ���g���C�@�\�t�� GetSaveFileName
	@author Moca
	@date 2006.09.03 �V�K�쐬
*/
bool CDlgOpenFile::GetSaveFileNameRecover( OPENFILENAMEZ* ofn )
{
	BOOL bRet = ::GetSaveFileName( ofn );
	if( !bRet  ){
		if( FNERR_INVALIDFILENAME == ::CommDlgExtendedError() ){
			_tcscpy( ofn->lpstrFile, _T("") );
			ofn->lpstrInitialDir = _T("");
			bRet = ::GetSaveFileName( ofn );
		}
	}
	return bRet!=FALSE;
}


