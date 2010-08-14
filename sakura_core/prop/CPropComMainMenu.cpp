/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u���C�����j���[�v�y�[�W

	@author Uchi
*/
/*
	Copyright (C) 2010, Uchi

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "env/CShareData.h"
#include "env/cShareData_IO.h"
#include "typeprop/CImpExpManager.h"
#include "dlg/CDlgInput1.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

using std::wstring;

// TreeView �\���Œ菉���l
const WCHAR	DEFAULT_NODE[]	= L"�ҏW���Ă�������";
const WCHAR	DEFAULT_SEPA[]	= L"�\�\�\�\�\�\�\�\�\�\";

static const DWORD p_helpids[] = {
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND,				//�@�\�̎��
	IDC_LIST_FUNC,					HIDC_LIST_FUNC,						//�@�\�ꗗ
	IDC_TREE_RES,					HIDC_TREE_RES,						//���j���[�ꗗ
	IDC_BUTTON_DELETE,				HIDC_BUTTON_TREE_DELETE,			//���j���[����@�\�폜
	IDC_BUTTON_INSERT_NODE,			HIDC_BUTTON_TREE_INSERT_NODE,		//���j���[�փm�[�h�ǉ�
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_TREE_INSERT_SEPARATOR,	//���j���[�֋�ؐ��}��
	IDC_BUTTON_INSERT,				HIDC_BUTTON_TREE_INSERT,			//���j���[�֋@�\�}��(��)
	IDC_BUTTON_INSERT_A,			HIDC_BUTTON_TREE_INSERT_A,			//���j���[�֋@�\�}��(��)
	IDC_BUTTON_ADD,					HIDC_BUTTON_TREE_ADD,				//���j���[�֋@�\�ǉ�
	IDC_BUTTON_UP,					HIDC_BUTTON_TREE_UP,				//���j���[�̋@�\����ֈړ�
	IDC_BUTTON_DOWN,				HIDC_BUTTON_TREE_DOWN,				//���j���[�̋@�\�����ֈړ�
	IDC_BUTTON_RIGHT,				HIDC_BUTTON_TREE_RIGHT,				//���j���[�̋@�\���E�ֈړ�
	IDC_BUTTON_LEFT,				HIDC_BUTTON_TREE_LEFT,				//���j���[�̋@�\�����ֈړ�
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT,					//���j���[�̃C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT,					//���j���[�̃G�N�X�|�[�g
	IDC_BUTTON_CHECK,				HIDC_BUTTON_NENU_CHECK,				//���j���[�̌���
	IDC_BUTTON_CLEAR,				HIDC_BUTTON_TREE_CLEAR,				//���j���[���N���A
	IDC_BUTTON_INITIALIZE,			HIDC_BUTTON_TREE_INITIALIZE,		//���j���[��������Ԃɖ߂�
	0, 0
};

// �����g�p�ϐ�
// �@�\�i�[(Work)
struct SMainMenuWork {
	wstring			m_sName;		// ���O
	EFunctionCode	m_nFunc;		// Function
	WCHAR			m_sKey[2];		// �A�N�Z�X�L�[
	bool			m_bDupErr;		// �A�N�Z�X�L�[�d���G���[
};

static	std::map<int, SMainMenuWork>	msMenu;	// �ꎞ�f�[�^
static	int		nMenuCnt = 0;					// �ꎞ�f�[�^�ԍ�


// ���[�J���֐���`
static HTREEITEM TreeCopy( HWND, HTREEITEM, HTREEITEM, bool, bool );
static void TreeView_ExpandAll( HWND, bool );
static const TCHAR * MakeDispLabel( SMainMenuWork* );

// ���ʋ@�\
struct SSpecialFunc	{
	EFunctionCode	m_nFunc;		// Function
	WCHAR* 			m_sName;		// ���O
};

static	const	TCHAR*	NAME_SPECIAL_TOP	= _T("���ʋ@�\");
static	SSpecialFunc	sSpecialFuncs[] = {
		{F_WINDOW_LIST,				L"�E�B���h�E���X�g",},
		{F_FILE_USED_RECENTLY,		L"�ŋߎg�����t�@�C��",},
		{F_FOLDER_USED_RECENTLY,	L"�ŋߎg�����t�H���_",},
		{F_CUSTMENU_LIST,			L"�J�X�^�����j���[���X�g",},
		{F_USERMACRO_LIST,			L"�o�^�ς݃}�N�����X�g",},
		{F_PLUGIN_LIST,				L"�v���O�C���R�}���h���X�g",},
};

static	int 	nSpecialFuncsNum;		// ���ʋ@�\�̃R���{�{�b�N�X���ł̔ԍ�

//  TreeView�L�[���͎��̃��b�Z�[�W�����p
static WNDPROC	m_wpTreeView = NULL;
static HWND		m_hwndDlg;

// TreeView���x���ҏW���̃��b�Z�[�W�����p
static WNDPROC	m_wpEdit = NULL;


/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropMainMenu::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}



// TreeView�L�[���͎��̃��b�Z�[�W����
static LRESULT CALLBACK TreeViewProc(
	HWND	hwndTree,		// handle to dialog box
	UINT	uMsg,			// message
	WPARAM	wParam,			// first message parameter
	LPARAM	lParam 			// second message parameter
)
{
	HTREEITEM		htiItem;
	TV_ITEM			tvi;		// �擾�p
	WCHAR			cKey;	
	SMainMenuWork*	pFuncWk;	// �@�\
	short	AltKey;

	switch (uMsg) {
	case WM_GETDLGCODE:
		MSG*	pMsg;
		if (lParam == 0) {
			break;
		}
		pMsg = (MSG*)lParam;
		if (pMsg->wParam == wParam && ( wParam == VK_RETURN|| wParam == VK_ESCAPE || wParam == VK_TAB )) {
			break;
		}
		return DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		htiItem = TreeView_GetSelection( hwndTree );
		cKey = MapVirtualKey( wParam, 2 );
		if (cKey > ' ') {
			// �A�N�Z�X�L�[�ݒ�
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = htiItem;
			if (!TreeView_GetItem( hwndTree, &tvi )) {
				break;
			}
			pFuncWk = &msMenu[tvi.lParam];
			if (pFuncWk->m_nFunc == F_SEPARATOR) {
				return 0;
			}
			pFuncWk->m_sKey[0] = cKey;
			pFuncWk->m_sKey[1] = L'\0';
			pFuncWk->m_bDupErr = false;
			tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
			tvi.pszText = const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
			TreeView_SetItem( hwndTree , &tvi );		//	�L�[�ݒ茋�ʂ𔽉f
			return 0;
		}

		AltKey = GetKeyState(VK_MENU);

		switch (wParam) {
		case VK_BACK:
		case VK_DELETE:	//	DEL�L�[�������ꂽ��_�C�A���O�{�b�N�X�Ƀ��b�Z�[�W�𑗐M
			::SendMessage( m_hwndDlg, WM_COMMAND, IDC_BUTTON_DELETE, (LPARAM)::GetDlgItem( m_hwndDlg, IDC_BUTTON_DELETE ) );
			return 0;
		case VK_F2:						// F2�ŕҏW
			if (htiItem != NULL) {
				TreeView_EditLabel( hwndTree, htiItem );
			}
			return 0;
		}
		break;
	case WM_CHAR:
		return 0;
	}
	return  CallWindowProc( m_wpTreeView, hwndTree, uMsg, wParam, lParam);
}

// TreeView���x���ҏW���̃��b�Z�[�W����
static LRESULT CALLBACK WindowProcEdit(
	HWND	hwndEdit,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	LRESULT	reult = 0;
	switch (uMsg) {
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	}
	return CallWindowProc( m_wpEdit, hwndEdit, uMsg, wParam, lParam );
}

/* Menu ���b�Z�[�W���� */
INT_PTR CPropMainMenu::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	int			idCtrl;
	static HWND	hwndComboFunkKind;
	static HWND	hwndListFunk;
	static HWND	hwndTreeRes;

	int			i;

	HTREEITEM	nIdxMenu;
	int			nIdxFIdx;
	int			nIdxFunc;
	WCHAR		szLabel[MAX_MAIN_MENU_NAME_LEN+10];

	EFunctionCode	eFuncCode;
	SMainMenuWork*	pFuncWk;	// �@�\
	TCHAR			szKey[2];

	TV_INSERTSTRUCT	tvis;		// �}���p
	TV_ITEM			tvi;		// �擾�p
	HTREEITEM		htiItem;
	HTREEITEM		htiParent;
	HTREEITEM		htiTemp;
	HTREEITEM		htiTemp2;
	TV_DISPINFO*	ptdi;

	CDlgInput1		cDlgInput1;

	static	bool	bInMove;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� Menu */
		SetData( hwndDlg );
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* �R���g���[���̃n���h�����擾 */
		hwndComboFunkKind = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
		hwndListFunk = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
		hwndTreeRes = ::GetDlgItem( hwndDlg, IDC_TREE_RES );

		/* �L�[�I�����̏��� */
		::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndComboFunkKind );

		// TreeView�̃��b�Z�[�W�����i�A�N�Z�X�L�[���͗p�j
		m_hwndDlg = hwndDlg;
		m_wpTreeView = (WNDPROC)SetWindowLongPtr( hwndTreeRes, GWLP_WNDPROC, (LONG_PTR)TreeViewProc );

		::SetTimer( hwndDlg, 1, 300, NULL );

		bInMove = false;

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		ptdi = (TV_DISPINFO*)lParam;

		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_MAINMENU );
			return TRUE;
		case PSN_KILLACTIVE:
			/* �_�C�A���O�f�[�^�̎擾 Menu */
			GetData( hwndDlg );
			return TRUE;
		case PSN_SETACTIVE:
			m_nPageNum = ID_PAGENUM_MAINMENU;

			// �\�����X�V����i�}�N���ݒ��ʂł̃}�N�����ύX�𔽉f�j
			nIdxFIdx = Combo_GetCurSel( hwndComboFunkKind );
			nIdxFunc = List_GetCurSel( hwndListFunk );
			if( nIdxFIdx != CB_ERR ){
				::SendMessage( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndComboFunkKind );
				if( nIdxFunc != LB_ERR ){
					List_SetCurSel( hwndListFunk, nIdxFunc );
				}
			}
			return TRUE;
		case TVN_BEGINLABELEDIT:	//	�A�C�e���̕ҏW�J�n
			if (pNMHDR->hwndFrom == hwndTreeRes) { 
				HWND hEdit = TreeView_GetEditControl( hwndTreeRes );
				if (msMenu[ptdi->item.lParam].m_nFunc == F_NODE) {
					// �m�[�h�̂ݗL��
					SetWindowText( hEdit, to_tchar( msMenu[ptdi->item.lParam].m_sName.c_str() ) ) ;
					EditCtl_LimitText( hEdit, MAX_MAIN_MENU_NAME_LEN );
					// �ҏW���̃��b�Z�[�W����
					m_wpEdit = (WNDPROC)SetWindowLongPtr( hEdit, GWLP_WNDPROC, (LONG_PTR)WindowProcEdit );
				}
				else {
					// �m�[�h�ȊO�ҏW�s��
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
				}
			}
			return TRUE;
		case TVN_ENDLABELEDIT:		//	�A�C�e���̕ҏW���I��
 			if (pNMHDR->hwndFrom == hwndTreeRes 
			  && (msMenu[ptdi->item.lParam].m_nFunc == F_NODE)) {
				// �m�[�h�L��
				pFuncWk = &msMenu[ptdi->item.lParam];
				if (ptdi->item.pszText == NULL) {
					// Esc
					//	�����ݒ肵�Ȃ��i���̂܂܁j
				}
				else if (auto_strcmp(ptdi->item.pszText, _T("")) == 0) {
					// ��
					pFuncWk->m_sName = DEFAULT_NODE;
				}
				else {
					pFuncWk->m_sName = to_wchar(ptdi->item.pszText);
				}
				ptdi->item.pszText = const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
				TreeView_SetItem( hwndTreeRes , &ptdi->item );	//	�ҏW���ʂ𔽉f

				// �ҏW���̃��b�Z�[�W������߂�
				SetWindowLongPtr( TreeView_GetEditControl( hwndTreeRes ), GWLP_WNDPROC, (LONG_PTR)m_wpEdit );
				m_wpEdit = NULL;
			}
			return TRUE;
		case TVN_DELETEITEM:
			if (!bInMove && !msMenu.empty()
			  && pNMHDR->hwndFrom == hwndTreeRes
			  && (htiItem = TreeView_GetSelection( hwndTreeRes )) != NULL) {
				//�t�������폜
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiItem;
				if (TreeView_GetItem( hwndTreeRes, &tvi )) {
					msMenu.erase( tvi.lParam );
				}
				return 0;
			}
			break;
		case NM_DBLCLK:
			// �_�u���N���b�N���̏���
			if (pNMHDR->hwndFrom == hwndTreeRes) {
				htiItem = TreeView_GetSelection( hwndTreeRes );
				if (htiItem == NULL) {
					break;
				}
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiItem;
				if (!TreeView_GetItem( hwndTreeRes, &tvi )) {
					break;
				}
				pFuncWk = &msMenu[tvi.lParam];
				if (pFuncWk->m_nFunc != F_SEPARATOR) {
					auto_sprintf( szKey, _T("%ls"), pFuncWk->m_sKey);

					if (!cDlgInput1.DoModal(
							G_AppInstance(),
							hwndDlg,
							_T("���j���[�A�C�e���̃A�N�Z�X�L�[�ݒ�"),
							_T("�L�[����͂��Ă��������B"),
							1,
							szKey)) {
						return TRUE;
					}
					auto_sprintf( pFuncWk->m_sKey, L"%ts", szKey);
					pFuncWk->m_bDupErr = false;

					tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
					tvi.pszText = const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
					TreeView_SetItem( hwndTreeRes, &tvi );
				}
			}
			break;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID = LOWORD(wParam);			/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl = (HWND) lParam;		/* �R���g���[���̃n���h�� */

		if (hwndComboFunkKind == hwndCtl) {
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIdxFIdx = Combo_GetCurSel( hwndComboFunkKind );

				if (nIdxFIdx == nSpecialFuncsNum) {
					// �@�\�ꗗ�ɓ���@�\���Z�b�g
					List_ResetContent( hwndListFunk );
					for (i = 0; i <_countof(sSpecialFuncs); i++) {
						List_AddString( hwndListFunk, sSpecialFuncs[i].m_sName );
					}
				}
				else {
					/* �@�\�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
					m_cLookup.SetListItem( hwndListFunk, nIdxFIdx );
				}

				return TRUE;
			}
		}
		else{
			switch( wNotifyCode ){
			/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_IMPORT:	/* �C���|�[�g */
					/* �J�X�^�����j���[�ݒ���C���|�[�g���� */
					Import( hwndDlg );
					return TRUE;
				case IDC_BUTTON_EXPORT:	/* �G�N�X�|�[�g */
					/* �J�X�^�����j���[�ݒ���G�N�X�|�[�g���� */
					Export( hwndDlg );
					return TRUE;

				case IDC_BUTTON_CLEAR:
					if (IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						_T("���j���[�̐ݒ���N���A���܂��B\n��낵���ł����H")) ) {
						return TRUE;
					}
					// �����f�[�^������
					msMenu.clear();
					nMenuCnt = 0;
					// TreeView������
					TreeView_DeleteAllItems( hwndTreeRes );
					return TRUE;

				case IDC_BUTTON_INITIALIZE:
					if (IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						_T("���j���[�̐ݒ��������Ԃɖ߂��܂��B\n��낵���ł����H"))) {
						return TRUE;
					}
					// ������Ԃɖ߂�
					{
						CDataProfile	cProfile;
						cProfile.SetReadingMode();
						cProfile.ReadProfileRes( MAKEINTRESOURCE(IDR_MENU1), MAKEINTRESOURCE(ID_RC_TYPE_INI) );

						CShareData_IO::ShareData_IO_MainMenu( cProfile, m_Common.m_sMainMenu, false );
						
						SetData( hwndDlg ); 
					}
					return TRUE;

				case IDC_BUTTON_DELETE:
					htiItem = TreeView_GetSelection( hwndTreeRes );
					if (htiItem != NULL) {
						if (TreeView_GetChild( hwndTreeRes, htiItem ) != NULL
						  && IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
							_T("�I�����Ă��鍀�ڂ����ʍ��ڂ��ƍ폜���܂��B\n��낵���ł����H"))) {
							return TRUE;
						}
						htiTemp = TreeView_GetNextSibling( hwndTreeRes, htiItem );
						if (htiTemp == NULL) {
							// �����Ȃ�΁A�O�����
							htiTemp = TreeView_GetPrevSibling( hwndTreeRes, htiItem );
						}
						TreeView_DeleteItem( hwndTreeRes, htiItem );
						if (htiTemp != NULL) {
							TreeView_SelectItem( hwndTreeRes, htiTemp );
						}
					}
					break;


				case IDC_BUTTON_INSERT_NODE:			// �m�[�h�}��
				case IDC_BUTTON_INSERTSEPARATOR:		// ��ؐ��}��
				case IDC_BUTTON_INSERT:					// �}��(��)
				case IDC_BUTTON_INSERT_A:				// �}��(��)
				case IDC_BUTTON_ADD:					// �ǉ�
					eFuncCode = F_INVALID;
					switch (wID) {
					case IDC_BUTTON_INSERT_NODE:		// �m�[�h�}��
						eFuncCode = F_NODE;
						auto_strcpy( szLabel , DEFAULT_NODE );
						break;
					case IDC_BUTTON_INSERTSEPARATOR:	// ��ؐ��}��
						eFuncCode = F_SEPARATOR;
						auto_strcpy( szLabel , DEFAULT_SEPA );
						break;
					case IDC_BUTTON_INSERT:				// �}��
					case IDC_BUTTON_INSERT_A:			// �}��
					case IDC_BUTTON_ADD:				// �ǉ�
						// Function �擾
						if (CB_ERR == (nIdxFIdx = Combo_GetCurSel( hwndComboFunkKind ))) {
							break;
						}
						if (LB_ERR == (nIdxFunc = List_GetCurSel( hwndListFunk ))) {
							break;
						}
						if (nIdxFIdx == nSpecialFuncsNum) {
							// ����@�\
							auto_strcpy( szLabel, sSpecialFuncs[nIdxFunc].m_sName );
							eFuncCode = sSpecialFuncs[nIdxFunc].m_nFunc;
						}
						else if (m_cLookup.Pos2FuncCode( nIdxFIdx, nIdxFunc ) != 0) {
							List_GetText( hwndListFunk, nIdxFunc, szLabel );
							eFuncCode = m_cLookup.Pos2FuncCode( nIdxFIdx, nIdxFunc );
						}
						else {
							auto_strcpy( szLabel, L"?" );
							eFuncCode = F_SEPARATOR;
						}
						break;
					}

					// �}���ʒu����
					htiTemp = TreeView_GetSelection( hwndTreeRes );
					if (htiTemp == NULL) {
						// ���Ȃ�������Root�̖���
						htiParent = TVI_ROOT;
						htiTemp = TVI_LAST;
					}
					else {
						if (wID == IDC_BUTTON_ADD) {
							// �ǉ�
							tvi.mask = TVIF_HANDLE | TVIF_PARAM;
							tvi.hItem = htiTemp;
							if (!TreeView_GetItem( hwndTreeRes, &tvi )) {
								// ���Ȃ�������Root�̖���
								htiParent = TVI_ROOT;
								htiTemp = TVI_LAST;
							}
							else {
								if (msMenu[tvi.lParam].m_nFunc == F_NODE) {
									// �m�[�h
									htiParent = htiTemp;
									htiTemp = TVI_LAST;
								}
								else {
									// �q��t�����Ȃ��̂Őe�ɕt����i�I���A�C�e���̉��ɕt���j
									htiParent = TreeView_GetParent( hwndTreeRes, htiTemp );
									htiTemp = TVI_LAST;
									if (htiParent == NULL) {
										// ���Ȃ�������Root�̖���
										htiParent = TVI_ROOT;
									}
								}
							}
						}
						else if (wID == IDC_BUTTON_INSERT_NODE || wID == IDC_BUTTON_INSERT_A) {
							// �m�[�h�}���A�}��(��)
							// �ǉ����T��
							htiTemp = TreeView_GetSelection( hwndTreeRes );
							if (htiTemp == NULL) {
								htiParent = TVI_ROOT;
								htiTemp = TVI_LAST;
							}
							else {
								tvi.mask = TVIF_HANDLE | TVIF_PARAM;
								tvi.hItem = htiTemp;
								if (TreeView_GetItem( hwndTreeRes, &tvi )) {
									if (msMenu[tvi.lParam].m_nFunc == F_NODE) {
										// �m�[�h
										htiParent = htiTemp;
										htiTemp = TVI_FIRST;
									}
									else {
										// �q��t�����Ȃ��̂Őe�ɕt����i�I���A�C�e���̉��ɕt���j
										htiParent = TreeView_GetParent( hwndTreeRes, htiTemp );
										if (htiParent == NULL) {
											// ���Ȃ�������Root
											htiParent = TVI_ROOT;
										}
									}
								}
								else {
									// ���Ȃ�������Root
									htiParent = TVI_ROOT;
									htiTemp = TVI_LAST;
								}
							}
						}
						else {
							// �}��(��)�A��ؐ�
							// �}�����T��
							htiParent = TreeView_GetParent( hwndTreeRes, htiTemp );
							if (htiParent == NULL) {
								// ���Ȃ�������Root�̃g�b�v
								htiParent = TVI_ROOT;
								htiTemp = TVI_FIRST;
							}
							else {
								// ���O
								htiTemp = TreeView_GetPrevSibling( hwndTreeRes, htiTemp );
								if (htiTemp == NULL) {
									// ���Ȃ�������e�̍ŏ�
									htiTemp = TVI_FIRST;
								}
							}
						}
					}

					// TreeView�ɑ}��
					pFuncWk = &msMenu[nMenuCnt];
					pFuncWk->m_nFunc = (EFunctionCode)eFuncCode;
					pFuncWk->m_sName = szLabel;
					pFuncWk->m_bDupErr = false;
					auto_strcpy(pFuncWk->m_sKey, L"");
					tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
					tvis.hParent = htiParent;
					tvis.hInsertAfter = htiTemp;
					tvis.item.pszText = const_cast<TCHAR*>(to_tchar(szLabel));
					tvis.item.lParam = nMenuCnt++;
					tvis.item.cChildren = ( wID == IDC_BUTTON_INSERT_NODE );
					htiItem = TreeView_InsertItem( hwndTreeRes, &tvis );
					// �W�J
					if (htiParent != TVI_ROOT) {
						TreeView_Expand( hwndTreeRes, htiParent, TVE_EXPAND );
					}
					TreeView_SelectItem( hwndTreeRes, htiItem );

					// ���X�g��1�i�߂�
					switch (wID) {
					case IDC_BUTTON_INSERT:				// �}��
					case IDC_BUTTON_INSERT_A:			// �}��
					case IDC_BUTTON_ADD:				// �ǉ�
						List_SetCurSel( hwndListFunk, nIdxFunc+1 );
						break;
					}
					break;


				case IDC_BUTTON_UP:
					htiItem = TreeView_GetSelection( hwndTreeRes );
					if (htiItem == NULL) {
						break;
					}
					htiTemp = TreeView_GetPrevSibling( hwndTreeRes, htiItem );
					if (htiTemp == NULL) {
						// ���̃G���A�ōŏ�
						break;
					}

					// �R�s�[
					bInMove = true;
					TreeCopy(hwndTreeRes, htiItem, htiTemp, false, true);

					// �폜
					TreeView_DeleteItem( hwndTreeRes, htiTemp );
					bInMove = false;
					break;

				case IDC_BUTTON_DOWN:
					htiItem = TreeView_GetSelection( hwndTreeRes );
					if (htiItem == NULL) {
						break;
					}
					htiTemp = TreeView_GetNextSibling( hwndTreeRes, htiItem );
					if (htiTemp == NULL) {
						// ���̃G���A�ōŌ�
						break;
					}

					// �R�s�[
					bInMove = true;
					TreeCopy(hwndTreeRes, htiTemp, htiItem, false, true);

					// �폜
					TreeView_DeleteItem( hwndTreeRes, htiItem );
					bInMove = false;

					// �I��
					htiItem = TreeView_GetNextSibling( hwndTreeRes, htiTemp );
					if (htiItem != NULL) {
						TreeView_SelectItem( hwndTreeRes, htiItem );
					}
					break;

				case IDC_BUTTON_RIGHT:
					htiItem = TreeView_GetSelection( hwndTreeRes );
					if (htiItem == NULL) {
						break;
					}
					htiTemp = TreeView_GetPrevSibling( hwndTreeRes, htiItem );
					if (htiTemp == NULL) {
						// ���̃G���A�ōŏ�
						break;
					}
					// �m�[�h�m�F
					tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
					tvi.hItem = htiTemp;
					i = TreeView_GetItem( hwndTreeRes, &tvi );
					if (TreeView_GetItem( hwndTreeRes, &tvi ) == NULL) {
						// �G���[
						break;
					}
					if (tvi.cChildren) {
						// ���O���m�[�h
						HTREEITEM		htiTemp2;
						// �R�s�[
						bInMove = true;
						htiTemp2 = TreeCopy(hwndTreeRes, htiTemp, htiItem, true, true);

						// �폜
						TreeView_DeleteItem( hwndTreeRes, htiItem );
						bInMove = false;

						// �I��
						TreeView_SelectItem( hwndTreeRes, htiTemp2 );
					}
					else {
						// �m�[�h������
						break;
					}
					break;

				case IDC_BUTTON_LEFT:
					htiItem = TreeView_GetSelection( hwndTreeRes );
					if (htiItem == NULL) {
						break;
					}
					htiParent = TreeView_GetParent( hwndTreeRes, htiItem );
					if (htiParent == NULL) {
						// Root
						break;
					}
					// �R�s�[
					bInMove = true;
					htiTemp2 = TreeCopy(hwndTreeRes, htiParent, htiItem, false, true);

					// �폜
					TreeView_DeleteItem( hwndTreeRes, htiItem );
					bInMove = false;

					// �I��
					TreeView_SelectItem( hwndTreeRes, htiTemp2 );
					break;


				case IDC_BUTTON_CHECK:		// ���j���[�̌���
					{
						wstring sErrMsg;
						if (Check_MainMenu( hwndTreeRes, sErrMsg )) {
							InfoMessage( hwndDlg, _T("��肠��܂���ł����B"));
						}
						else {
							WarningMessage( hwndDlg, to_tchar(sErrMsg.c_str()) );
						}
					}
					break;


				case IDC_BUTTON_EXPAND:		// �c���[�S�J
					TreeView_ExpandAll( hwndTreeRes, true );
					break;

				case IDC_BUTTON_COLLAPSE:	// �c���[�S��
					TreeView_ExpandAll( hwndTreeRes, false );
					break;
				}

				break;
			}
		}
		break;

	case WM_TIMER:
		nIdxMenu = TreeView_GetSelection( hwndTreeRes );
		nIdxFIdx = Combo_GetCurSel( hwndComboFunkKind );
		nIdxFunc = List_GetCurSel( hwndListFunk );
		i = List_GetCount( hwndTreeRes );
		if (nIdxMenu == NULL) {
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ),     FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ),   FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RIGHT ),  FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_LEFT ),   FALSE );
		}
		else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ),     NULL != TreeView_GetPrevSibling( hwndTreeRes, nIdxMenu ) );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ),   NULL != TreeView_GetNextSibling( hwndTreeRes, nIdxMenu ) );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RIGHT ),  NULL != TreeView_GetPrevSibling( hwndTreeRes, nIdxMenu ) );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_LEFT ),   NULL != TreeView_GetParent( hwndTreeRes, nIdxMenu ) );
		}
		if (LB_ERR == nIdxFunc ||
		  ( LB_ERR != nIdxFIdx && LB_ERR != nIdxFunc &&
		    (m_cLookup.Pos2FuncCode( nIdxFIdx, nIdxFunc ) == 0 && nIdxFIdx != nSpecialFuncsNum))) {
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT_A ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}
		else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), NULL != nIdxMenu );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT_A ), NULL != nIdxMenu );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), TRUE );
		}
		break;
	case WM_DESTROY:
		::KillTimer( hwndDlg, 1 );

		// �ҏW���̃��b�Z�[�W������߂�
		SetWindowLongPtr( hwndTreeRes, GWLP_WNDPROC, (LONG_PTR)m_wpTreeView );
		m_wpTreeView = NULL;

		// ���[�N�̃N���A
		msMenu.clear();
		nMenuCnt = 0;
		break;

	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );
		}
		return TRUE;

	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );

		return TRUE;
	}
	return FALSE;
}




// & �̕⊮
static wstring	SupplementAmpersand( wstring sLavel)
{
	size_t	nPos =0;
	while ((nPos = sLavel.find( L"&", nPos)) != wstring::npos) {
		if (sLavel[nPos+1] != L'&') {
			// &&�łȂ�
			sLavel.replace( nPos, 1, L"&&");
		}
		nPos +=2;
	}
	return sLavel;
}

// & �̍폜
static wstring	RemoveAmpersand( wstring sLavel)
{
	size_t	nPos =0;
	while ((nPos = sLavel.find( L"&", nPos)) != wstring::npos) {
		if (sLavel[nPos+1] == L'&') {
			// &&
			sLavel.replace( nPos, 1, L"");
		}
		nPos ++;
	}
	return sLavel;
}

/* �_�C�A���O�f�[�^�̐ݒ� MainMenu */
void CPropMainMenu::SetData( HWND hwndDlg )
{
	CMainMenu*		pcMenuTBL = m_Common.m_sMainMenu.m_cMainMenuTbl;
	CMainMenu*		pcFunc;
	HWND			hwndCombo;
	HWND			hwndCheck;
	HWND			hwndTreeRes;
	WCHAR			szLabel[MAX_MAIN_MENU_NAME_LEN+10];
	int				nCurLevel;
	HTREEITEM		htiItem;
	HTREEITEM		htiParent;
	TV_INSERTSTRUCT	tvis;			// �}���p
	SMainMenuWork*	pFuncWk;		// �@�\(work)
	int 			i;
	int 			j;

	/* �@�\��ʈꗗ�ɕ�������Z�b�g�i�R���{�{�b�N�X�j */
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );

	// ���ʋ@�\�ǉ�
	nSpecialFuncsNum = Combo_AddString( hwndCombo, NAME_SPECIAL_TOP );

	/* ��ʂ̐擪�̍��ڂ�I���i�R���{�{�b�N�X�j*/
	Combo_SetCurSel( hwndCombo, 0 );

	// ���[�N�ATreeView�̏�����
	msMenu.clear();
	nMenuCnt = 0;

	hwndTreeRes = ::GetDlgItem( hwndDlg, IDC_TREE_RES );
	TreeView_DeleteAllItems( hwndTreeRes );

	// �A�N�Z�X�L�[��( )�t�ŕ\��
	hwndCheck = ::GetDlgItem( hwndDlg, IDC_CHECK_KEY_PARENTHESES );
	BtnCtl_SetCheck( hwndCheck, m_Common.m_sMainMenu.m_bMainMenuKeyParentheses );

	/* ���j���[���ڈꗗ�Ɠ����f�[�^���Z�b�g�iTreeView�j*/
	nCurLevel = 0;
	htiParent = TVI_ROOT;
	htiItem = TreeView_GetRoot( hwndTreeRes );
	for (i = 0; i < m_Common.m_sMainMenu.m_nMainMenuNum; i++) {
		pcFunc = &pcMenuTBL[i];
		if (pcFunc->m_nLevel < nCurLevel) {
			// Level Up
			for (; pcFunc->m_nLevel < nCurLevel; nCurLevel--) {
				htiParent = (htiParent == TVI_ROOT) ? TVI_ROOT : TreeView_GetParent( hwndTreeRes, htiParent );
				if (htiParent == NULL)		htiParent = TVI_ROOT;
			}
		}
		else if (pcFunc->m_nLevel > nCurLevel) {
			// Level Down
			for ( htiParent = htiItem, nCurLevel++; pcFunc->m_nLevel < nCurLevel; nCurLevel++) {
				// ���s����邱�Ƃ͖����͂��i�f�[�^������Ȃ�΁j
				htiParent = TreeView_GetChild( hwndTreeRes, htiItem );
				if (htiParent == NULL)		htiParent = htiItem;
			}
		}

		// �����f�[�^���쐬
		pFuncWk = &msMenu[nMenuCnt];
		pFuncWk->m_nFunc = pcFunc->m_nFunc;
		switch (pcFunc->m_nType) {
			case T_LEAF:
				m_cLookup.Funccode2Name( pcFunc->m_nFunc, szLabel, MAX_MAIN_MENU_NAME_LEN );
				pFuncWk->m_sName = szLabel;
				break;
			case T_SEPARATOR:
				pFuncWk->m_sName = DEFAULT_SEPA;
				break;
			case T_SPECIAL:
				pFuncWk->m_sName = pcFunc->m_sName;
				if (pFuncWk->m_sName.empty()) {
					for (j = 0; j < _countof(sSpecialFuncs); j++) {
						if (pcFunc->m_nFunc == sSpecialFuncs[j].m_nFunc) {
							pFuncWk->m_sName = RemoveAmpersand( sSpecialFuncs[j].m_sName );
							break;
						}
					}
				}
				break;
			default:
				pFuncWk->m_sName = RemoveAmpersand( pcFunc->m_sName );
				break;
		}
		auto_strcpy(pFuncWk->m_sKey, pcFunc->m_sKey);
		pFuncWk->m_bDupErr = false;
		// TreeView�ɑ}��
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvis.hParent = htiParent;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.pszText = const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
		tvis.item.lParam = nMenuCnt++;								// �����f�[�^�C���f�b�N�X�̃C���N�������g
		tvis.item.cChildren = ( pcFunc->m_nType == T_NODE );
		htiItem = TreeView_InsertItem( hwndTreeRes, &tvis );
	}
}

/* �_�C�A���O�f�[�^�̎擾 MainMenu */
int CPropMainMenu::GetData( HWND hwndDlg )
{
	HWND			hwndTreeRes;
	HWND			hwndCheck;
	HTREEITEM		htiItem;

	// �A�N�Z�X�L�[��( )�t�ŕ\��
	hwndCheck = ::GetDlgItem( hwndDlg, IDC_CHECK_KEY_PARENTHESES );
	m_Common.m_sMainMenu.m_bMainMenuKeyParentheses = (BtnCtl_GetCheck( hwndCheck ) != 0);

	// ���j���[�g�b�v���ڂ��Z�b�g
	m_Common.m_sMainMenu.m_nMainMenuNum = 0;
	memset( m_Common.m_sMainMenu.m_nMenuTopIdx, -1, sizeof(m_Common.m_sMainMenu.m_nMenuTopIdx) );

	hwndTreeRes = ::GetDlgItem( hwndDlg, IDC_TREE_RES );

	htiItem = TreeView_GetRoot( hwndTreeRes );
	GetDataTree( hwndTreeRes, htiItem, 0);

	return TRUE;
}

/* �_�C�A���O�f�[�^�̎擾 TreeView�� 1 level */
bool CPropMainMenu::GetDataTree( HWND hwndTree, HTREEITEM htiTrg, int nLevel )
{
	static	bool	bOptionOk;
	CMainMenu*		pcMenuTBL = m_Common.m_sMainMenu.m_cMainMenuTbl;
	CMainMenu*		pcFunc;
	HTREEITEM		s;
	HTREEITEM		ts;
	TV_ITEM			tvi;			// �擾�p
	SMainMenuWork*	pFuncWk;		// �@�\(work)
	int 			nTopCount = 0;

	if (nLevel == 0) {
		// ���ʐݒ�t���O
		bOptionOk = false;
	}

	for (s = htiTrg; s != NULL; s = TreeView_GetNextSibling( hwndTree, s )) {
		if (m_Common.m_sMainMenu.m_nMainMenuNum >= MAX_MAINMENU) {
			// �o�^�� over
			return false;
		}
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		if (TreeView_GetItem( hwndTree, &tvi ) == NULL) {
			// Error
			return false;
		}
		pFuncWk = &msMenu[tvi.lParam];

		if (nLevel == 0) {
			if (nTopCount >= MAX_MAINMENU_TOP) {
				continue;
			}
			// Top Level�̋L�^
			m_Common.m_sMainMenu.m_nMenuTopIdx[nTopCount++] = m_Common.m_sMainMenu.m_nMainMenuNum;
		}
		pcFunc = &pcMenuTBL[m_Common.m_sMainMenu.m_nMainMenuNum++];

		switch(pFuncWk->m_nFunc) {
		case F_NODE:
			pcFunc->m_nType = T_NODE;
			auto_strcpy_s( pcFunc->m_sName, MAX_MAIN_MENU_NAME_LEN+1, SupplementAmpersand( pFuncWk->m_sName ).c_str() );
			break;
		case F_SEPARATOR:
			pcFunc->m_nType = T_SEPARATOR;
			pcFunc->m_sName[0] = L'\0';
			break;
		default:
			if (pFuncWk->m_nFunc >= F_SPECIAL_FIRST && pFuncWk->m_nFunc <= F_SPECIAL_LAST) {
				pcFunc->m_nType = T_SPECIAL;
				if (nLevel == 0) {
					auto_strcpy_s( pcFunc->m_sName, MAX_MAIN_MENU_NAME_LEN+1, SupplementAmpersand( pFuncWk->m_sName ).c_str() );
				}
				else {
					pcFunc->m_sName[0] = L'\0';
				}
			}
			else {
				if (pFuncWk->m_nFunc == F_OPTION) {
					bOptionOk = true;
				}
				pcFunc->m_nType = T_LEAF;
				pcFunc->m_sName[0] = L'\0';
			}
			break;
		}
		pcFunc->m_nFunc = pFuncWk->m_nFunc;
		auto_strcpy( pcFunc->m_sKey, pFuncWk->m_sKey );
		pcFunc->m_nLevel = nLevel;

		if (tvi.cChildren) {
			ts = TreeView_GetChild( hwndTree, s );	//	�q�̎擾
			if (ts != NULL) {
				if (!GetDataTree( hwndTree, ts, nLevel+1 )) {
					return false;
				}
			}
		}
	}

	if (nLevel == 0 && !bOptionOk) {
		// ���ʐݒ肪����
		if (nTopCount < MAX_MAINMENU_TOP && m_Common.m_sMainMenu.m_nMainMenuNum+1 < MAX_MAINMENU) {
			// Top Level�̋L�^
			m_Common.m_sMainMenu.m_nMenuTopIdx[nTopCount++] = m_Common.m_sMainMenu.m_nMainMenuNum;
			// Top Level�̒ǉ��i�_�~�[�j
			pcFunc = &pcMenuTBL[m_Common.m_sMainMenu.m_nMainMenuNum++];
			pcFunc->m_nType = T_NODE;
			pcFunc->m_nFunc = F_NODE;
			auto_strcpy( pcFunc->m_sName, L"auto_add" );
			pcFunc->m_sKey[0] = L'\0';
			pcFunc->m_nLevel = nLevel++;
		}
		else {
			// �����ɒǉ����w��
			nLevel = 1;
		}
		if (m_Common.m_sMainMenu.m_nMainMenuNum < MAX_MAINMENU) {
			// ���ʐݒ�
			pcFunc = &pcMenuTBL[m_Common.m_sMainMenu.m_nMainMenuNum++];
			pcFunc->m_nType = T_LEAF;
			pcFunc->m_nFunc = F_OPTION;
			pcFunc->m_sName[0] = L'\0';
			pcFunc->m_sKey[0] = L'\0';
			pcFunc->m_nLevel = nLevel;
		}
		else {
			// �o�^�� over
			return false;
		}
	}

	return true;
}



/* ���C�����j���[�ݒ���C���|�[�g���� */
void CPropMainMenu::Import( HWND hwndDlg )
{
	CImpExpMainMenu	cImpExp( m_Common );

	// �C���|�[�g
	if (!cImpExp.ImportUI( G_AppInstance(), hwndDlg )) {
		// �C���|�[�g�����Ă��Ȃ�
		return;
	}
	SetData( hwndDlg );
}

/* ���C�����j���[�ݒ���G�N�X�|�[�g���� */
void CPropMainMenu::Export( HWND hwndDlg )
{
	CImpExpMainMenu	cImpExp( m_Common );

	GetData( hwndDlg );

	// �G�N�X�|�[�g
	if (!cImpExp.ExportUI( G_AppInstance(), hwndDlg )) {
		// �G�N�X�|�[�g�����Ă��Ȃ�
		return;
	}
}



// �c���[�̃R�s�[
//		fChild��true�̎���dst�̎q�Ƃ��ăR�s�[, �����łȂ����dst�̌Z��Ƃ���dst�̌��ɃR�s�[
//		fOnryOne��true�̎���1�����R�s�[�i�q����������R�s�[�j
static HTREEITEM TreeCopy( HWND hwndTree, HTREEITEM dst, HTREEITEM src, bool fChild, bool fOnryOne )
{
	HTREEITEM		s;
	HTREEITEM		ts;
	HTREEITEM		td = NULL;
	TV_INSERTSTRUCT	tvis;		// �}���p
	TV_ITEM			tvi;		// �擾�p
	int				n = 0;
	TCHAR			szLabel[MAX_MAIN_MENU_NAME_LEN + 10];

	for (s = src; s != NULL; s = fOnryOne ? NULL:TreeView_GetNextSibling( hwndTree, s )) {
		tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		tvi.pszText = szLabel;
		tvi.cchTextMax = MAX_MAIN_MENU_NAME_LEN;
		if (TreeView_GetItem( hwndTree, &tvi ) == NULL) {
			// Error
			break;
		}
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		if (fChild || n != 0) {
			// dst�̎q���Ƃ��č쐬
			tvis.hParent = dst;
			tvis.hInsertAfter = TVI_LAST;
		}
		else {
			//	dst�̌Z��Ƃ��č쐬
			tvis.hParent = TreeView_GetParent( hwndTree, dst );
			tvis.hInsertAfter = dst;
		}
		tvis.item.pszText = szLabel;
		tvis.item.lParam = tvi.lParam;
		tvis.item.cChildren = tvi.cChildren;
		td = TreeView_InsertItem( hwndTree, &tvis );	//	Item�̍쐬

		if (tvi.cChildren) {
			ts = TreeView_GetChild( hwndTree, s );	//	�q�̎擾
			if (ts != NULL) {
				TreeCopy( hwndTree, td, ts, true, false );
			}
			// �W�J
			if (tvi.state & TVIS_EXPANDEDONCE) {
				TreeView_Expand( hwndTree, td, TVE_EXPAND );
			}
		}
		n++;
	}

	return td;
}

// TreeView �S�J��S��
static void TreeView_ExpandAll( HWND hwndTree, bool bExpand )
{
	std::map<int, HTREEITEM>	htiStack;
	HTREEITEM	htiCur;
	HTREEITEM	htiItem;
	HTREEITEM	htiNext;
	int			nLevel;

	nLevel = 0;
	htiCur = TreeView_GetSelection( hwndTree );
	if (!bExpand && htiCur != NULL) {
		// ���鎞�̓g�b�v�ɕύX
		for (htiNext = htiCur; htiNext !=  NULL; ) {
			htiItem = htiNext;
			htiNext = TreeView_GetParent( hwndTree, htiItem );
		}
		if (htiCur != htiItem) {
			htiCur = htiItem;
			TreeView_SelectItem( hwndTree, htiCur );
		}
	}


	for (htiItem = TreeView_GetRoot( hwndTree ); htiItem != NULL; htiItem = htiNext) {
		htiNext = TreeView_GetChild( hwndTree, htiItem);
		if (htiNext != NULL) {
			TreeView_Expand( hwndTree, htiItem, bExpand ? TVE_EXPAND : TVE_COLLAPSE);
			// �q�̊J��
			htiStack[nLevel++] = htiItem;
		}
		else {
			htiNext = TreeView_GetNextSibling( hwndTree, htiItem);
			while (htiNext == NULL && nLevel > 0) {
				htiItem = htiStack[--nLevel];
				htiNext = TreeView_GetNextSibling( hwndTree, htiItem);
			}
		}
	}
	// �I���ʒu��߂�
	if (htiCur == NULL) {
		if (bExpand ) {
			htiItem = TreeView_GetRoot( hwndTree );
			TreeView_SelectSetFirstVisible( hwndTree, htiItem );
		}
		TreeView_SelectItem( hwndTree, NULL );
	}
	else {
		TreeView_SelectSetFirstVisible( hwndTree, htiCur );
	}
}



// �\���p�f�[�^�̍쐬�i�A�N�Z�X�L�[�t���j
static const TCHAR* MakeDispLabel( SMainMenuWork* pFunc )
{
	static	WCHAR	szLabel[MAX_MAIN_MENU_NAME_LEN + 10];

	if (pFunc->m_sKey[0]) {
		auto_sprintf_s( szLabel, MAX_MAIN_MENU_NAME_LEN + 10, L"%ls%ls(%ls)",
			pFunc->m_bDupErr ? L">" : L"",
			pFunc->m_sName.c_str() , pFunc->m_sKey );
	}
	else {
		auto_sprintf_s( szLabel, MAX_MAIN_MENU_NAME_LEN + 10, L"%ls%ls",
			pFunc->m_bDupErr ? L">" : L"",
			pFunc->m_sName.c_str() );
	}

	return to_tchar( szLabel );
}



// ���j���[�̌���
bool CPropMainMenu::Check_MainMenu( 
	HWND	hwndTree,		// handle to TreeView
	wstring&	sErrMsg			// �G���[���b�Z�[�W
)
{
	bool			bRet = false;
	HTREEITEM		htiItem;
	
	sErrMsg = L"";
	
	htiItem = TreeView_GetRoot( hwndTree );

	bRet = Check_MainMenu_Sub( hwndTree, htiItem, 0, sErrMsg );
	return bRet;
}

// ���j���[�̌��� TreeView�� 1 level
bool CPropMainMenu::Check_MainMenu_Sub(
	HWND		hwndTree,		// handle to dialog box
	HTREEITEM 	htiTrg,			// �^�[�Q�b�g
	int 		nLevel,
	wstring&	sErrMsg )
{
	// �����p
	static	bool		bOptionOk;		// �u���ʐݒ�v
	static	int 		nMenuNum;		// ���j���[���ڐ�		�ő� MAX_MAINMENU
	static	int 		nTopNum;		// �g�b�v���x�����ڐ�	�ő� MAX_MAINMENU_TOP
	static	int 		nDupErrNum;		// �d���G���[��
	static	int 		nNoSetErrNum;	// ���ݒ�G���[��
	static	HTREEITEM	htiErr;
	//
	bool			bRet = true;
	EMainMenuType	nType;
	HTREEITEM		s;
	HTREEITEM		ts;
	TV_ITEM			tvi;							// �擾�p
	SMainMenuWork*	pFuncWk;						// �@�\(work)
	std::map< WCHAR, HTREEITEM >	mKey;			// �d���G���[���o�p
	std::map< WCHAR, HTREEITEM >::iterator itKey;	// ����

	if (nLevel == 0) {
		bOptionOk = false;
		nMenuNum = nTopNum = nDupErrNum = nNoSetErrNum = 0;
		htiErr = NULL;
	}
	mKey.clear();

	for (s = htiTrg; s != NULL; s = TreeView_GetNextSibling( hwndTree, s )) {
		// ���j���[���̃J�E���g
		nMenuNum++;
		if (nLevel == 0) {
			nTopNum++;
		}
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		if (TreeView_GetItem( hwndTree, &tvi ) == NULL) {
			// Error
			sErrMsg = L"���j���[�ݒ�̎擾�Ɏ��s���܂���";
			return false;
		}
		pFuncWk = &msMenu[tvi.lParam];
		switch (pFuncWk->m_nFunc) {
		case F_NODE:
			nType = T_NODE;
			break;
		case F_SEPARATOR:
			nType = T_SEPARATOR;
			break;
		default:
			if (pFuncWk->m_nFunc >= F_SPECIAL_FIRST && pFuncWk->m_nFunc <= F_SPECIAL_LAST) {
				nType = T_SPECIAL;
			}
			else {
				if (pFuncWk->m_nFunc == F_OPTION) {
					bOptionOk = true;
				}
				nType = T_LEAF;
			}
			break;
		}
		if (pFuncWk->m_sKey[0] == '\0') {
			if (nType == T_NODE || nType == T_LEAF) {
				// ���ݒ�
				if (nNoSetErrNum == 0) {
					if (htiErr == NULL) {
						htiErr = s;
					}
				}
				TreeView_SelectItem( hwndTree, s );
				nNoSetErrNum++;
			}
		}
		else {
			itKey = mKey.find( pFuncWk->m_sKey[0] );
			if (itKey == mKey.end()) {
				mKey[pFuncWk->m_sKey[0]] = s;

				if (pFuncWk->m_bDupErr) {
					// �ڈ�N���A
					pFuncWk->m_bDupErr = false;
					tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
					tvi.pszText =  const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
					TreeView_SetItem( hwndTree , &tvi );		//	�L�[�ݒ茋�ʂ𔽉f
				}
			}
			else {
				// �d���G���[
				if (nDupErrNum == 0) {
					if (htiErr == NULL) {
						htiErr = mKey[pFuncWk->m_sKey[0]];
					}
				}
				TreeView_SelectItem( hwndTree, mKey[pFuncWk->m_sKey[0]] );

				nDupErrNum++;

				// �ڈ�ݒ�
				pFuncWk->m_bDupErr = true;
				tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
				tvi.pszText = const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
				TreeView_SetItem( hwndTree , &tvi );		//	�L�[�ݒ茋�ʂ𔽉f

				// �ڈ�ݒ�i�����j
				tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
				tvi.hItem = mKey[pFuncWk->m_sKey[0]];
				if (TreeView_GetItem( hwndTree, &tvi ) == NULL) {
					// Error
					sErrMsg = L"���j���[�ݒ�̎擾�Ɏ��s���܂���";
					return false;
				}
				if (!msMenu[tvi.lParam].m_bDupErr) {
					msMenu[tvi.lParam].m_bDupErr = true;
					tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
					tvi.pszText = const_cast<TCHAR*>( MakeDispLabel( &msMenu[tvi.lParam] ) );
					TreeView_SetItem( hwndTree , &tvi );		//	�L�[�ݒ茋�ʂ𔽉f
				}
			}
		}
		if (tvi.cChildren) {
			ts = TreeView_GetChild( hwndTree, s );	//	�q�̎擾
			if (ts != NULL) {
				if (!Check_MainMenu_Sub( hwndTree, ts, nLevel+1, sErrMsg )) {
					// �����G���[
					return false;
				}
			}
		}
	}

	if (nLevel == 0) {
		sErrMsg = L"";
		if (!bOptionOk) {
			sErrMsg += L"�w���ʐݒ�x���L��܂���B\n";
			bRet = false;
		}
		if (nTopNum > MAX_MAINMENU_TOP) {
			sErrMsg += L"�g�b�v���x���̍��ڐ����������܂��B\n";
			bRet = false;
		}
		if (nMenuNum > MAX_MAINMENU) {
			sErrMsg += L"�o�^���ڐ����������܂��B\n";
			bRet = false;
		}
		if (nDupErrNum > 0) {
			sErrMsg += L"�d�������A�N�Z�X�L�[������܂��B\n";
			bRet = false;
		}
		if (nNoSetErrNum > 0) {
			sErrMsg += L"���ݒ�̃A�N�Z�X�L�[������܂��B\n";
			bRet = false;
		}
		if (htiErr != NULL) {
			TreeView_SelectItem( hwndTree, htiErr );
			TreeView_SelectSetFirstVisible( hwndTree, htiErr );
		}
	}
	return bRet;
}
