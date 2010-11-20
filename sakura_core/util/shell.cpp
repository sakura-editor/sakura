#include "stdafx.h"
#include "shell.h"
#include <HtmlHelp.h>
#include <Shlobj.h>
#include <shellapi.h>
#include <Cderr.h> // Nov. 3, 2005 genta	//CDERR_FINDRESFAILURE��
#include "util/string_ex2.h"
#include "util/file.h"
#include "util/os.h"
#include "util/module.h"
#include "COsVersionInfo.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "CHtmlHelp.h"

int CALLBACK MYBrowseCallbackProc(
	HWND hwnd,
	UINT uMsg,
	LPARAM lParam,
	LPARAM lpData
)
{
	switch( uMsg ){
	case BFFM_INITIALIZED:
		::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData );
		break;
	case BFFM_SELCHANGED:
		break;
	}
	return 0;

}


/* �t�H���_�I���_�C�A���O */
BOOL SelectDir( HWND hWnd, const TCHAR* pszTitle, const TCHAR* pszInitFolder, TCHAR* strFolderName )
{
	BOOL	bRes;
	TCHAR	szInitFolder[MAX_PATH];

	_tcscpy( szInitFolder, pszInitFolder );
	/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� "c:\\"���̃��[�g�͎�菜���Ȃ�*/
	CutLastYenFromDirectoryPath( szInitFolder );

	// 2010.08.28 �t�H���_���J���ƃt�b�N���܂߂ĐF�XDLL���ǂݍ��܂��̂ňړ�
	CCurrentDirectoryBackupPoint dirBack;
	ChangeCurrentDirectoryToExeDir();

	// SHBrowseForFolder()�֐��ɓn���\����
	BROWSEINFO bi;
	bi.hwndOwner = hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = strFolderName;
	bi.lpszTitle = pszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS/* | BIF_EDITBOX*//* | BIF_STATUSTEXT*/;
	bi.lpfn = MYBrowseCallbackProc;
	bi.lParam = (LPARAM)szInitFolder;
	bi.iImage = 0;
	// �A�C�e���h�c���X�g��Ԃ�
	// ITEMIDLIST�̓A�C�e���̈�ӂ�\���\����
	LPITEMIDLIST pList = ::SHBrowseForFolder(&bi);
	if( NULL != pList ){
		// SHGetPathFromIDList()�֐��̓A�C�e���h�c���X�g�̕����p�X��T���Ă����
		bRes = ::SHGetPathFromIDList( pList, strFolderName );
		// :SHBrowseForFolder()�Ŏ擾�����A�C�e���h�c���X�g���폜
		::CoTaskMemFree( pList );
		if( bRes ){
			return TRUE;
		}else{
			return FALSE;
		}
	}
	return FALSE;
}



/*!	����t�H���_�̃p�X���擾����
	SHGetSpecialFolderPath API�ishell32.dll version 4.71�ȏオ�K�v�j�Ɠ����̏���������

	@author ryoji
	@date 2007.05.19 �V�K
*/
BOOL GetSpecialFolderPath( int nFolder, LPTSTR pszPath )
{
	BOOL bRet = FALSE;
	HRESULT hres;
	LPMALLOC pMalloc;
	LPITEMIDLIST pidl;

	hres = ::SHGetMalloc( &pMalloc );
	if( FAILED( hres ) )
		return FALSE;

	hres = ::SHGetSpecialFolderLocation( NULL, nFolder, &pidl );
	if( SUCCEEDED( hres ) ){
		bRet = ::SHGetPathFromIDList( pidl, pszPath );
		pMalloc->Free( (void*)pidl );
	}

	pMalloc->Release();

	return bRet;
}



///////////////////////////////////////////////////////////////////////
// From Here 2007.05.25 ryoji �Ǝ��g���̃v���p�e�B�V�[�g�֐��Q

static WNDPROC s_pOldPropSheetWndProc;	// �v���p�e�B�V�[�g�̌��̃E�B���h�E�v���V�[�W��

/*!	�Ǝ��g���v���p�e�B�V�[�g�̃E�B���h�E�v���V�[�W��
	@author ryoji
	@date 2007.05.25 �V�K
*/
static LRESULT CALLBACK PropSheetWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ){
	case WM_SHOWWINDOW:
		// �ǉ��{�^���̈ʒu�𒲐�����
		if( wParam ){
			HWND hwndBtn;
			RECT rcOk;
			RECT rcTab;
			POINT pt;

			hwndBtn = ::GetDlgItem( hwnd, 0x02000 );
			::GetWindowRect( ::GetDlgItem( hwnd, IDOK ), &rcOk );
			::GetWindowRect( PropSheet_GetTabControl( hwnd ), &rcTab );
			pt.x = rcTab.left;
			pt.y = rcOk.top;
			::ScreenToClient( hwnd, &pt );
			::MoveWindow( hwndBtn, pt.x, pt.y, 140, rcOk.bottom - rcOk.top, FALSE );
		}
		break;

	case WM_COMMAND:
		// �ǉ��{�^���������ꂽ���͂��̏������s��
		if( HIWORD( wParam ) == BN_CLICKED && LOWORD( wParam ) == 0x02000 ){
			HWND hwndBtn = ::GetDlgItem( hwnd, 0x2000 );
			RECT rc;
			POINT pt;

			// ���j���[��\������
			::GetWindowRect( hwndBtn, &rc );
			pt.x = rc.left;
			pt.y = rc.bottom;
			GetMonitorWorkRect( pt, &rc );	// ���j�^�̃��[�N�G���A

			HMENU hMenu = ::CreatePopupMenu();
			::InsertMenu( hMenu, 0, MF_BYPOSITION | MF_STRING, 100, _T("�J��(&O)...") );
			::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_STRING, 101, _T("�C���|�[�g�^�G�N�X�|�[�g�̋N�_���Z�b�g(&R)") );

			int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
										( pt.x > rc.left )? pt.x: rc.left,
										( pt.y < rc.bottom )? pt.y: rc.bottom,
										0, hwnd, NULL );
			::DestroyMenu( hMenu );

			// �I�����ꂽ���j���[�̏���
			switch( nId ){
			case 100:	// �ݒ�t�H���_���J��
				TCHAR szPath[_MAX_PATH];
				GetInidir( szPath );

				// �t�H���_�� ITEMIDLIST ���擾���� ShellExecuteEx() �ŊJ��
				// Note. MSDN �� ShellExecute() �̉���ɂ�����@�Ńt�H���_���J�����Ƃ����ꍇ�A
				//       �t�H���_�Ɠ����ꏊ�� <�t�H���_��>.exe ������Ƃ��܂������Ȃ��B
				//       verb��"open"��NULL�ł�exe�̂ق������s����"explore"�ł͎��s����
				//       �i�t�H���_���̖�����'\\'��t�����Ă�Windows 2000�ł͕t�����Ȃ��̂Ɠ�������ɂȂ��Ă��܂��j
				LPSHELLFOLDER pDesktopFolder;
				if( SUCCEEDED(::SHGetDesktopFolder(&pDesktopFolder)) ){
					LPMALLOC pMalloc;
					if( SUCCEEDED(::SHGetMalloc(&pMalloc)) ){
						LPITEMIDLIST pIDL;
						WCHAR pwszDisplayName[_MAX_PATH];
						_tcstowcs(pwszDisplayName, szPath, _countof(pwszDisplayName));
//#ifdef _UNICODE
//						pwszDisplayName = szPath;
//#else
//						WCHAR wszPath[_MAX_PATH];
//						::MultiByteToWideChar( CP_ACP, 0, szPath, -1, wszPath, _MAX_PATH );
//						pwszDisplayName = wszPath;
//#endif
						if( SUCCEEDED(pDesktopFolder->ParseDisplayName(NULL, NULL, pwszDisplayName, NULL, &pIDL, NULL)) ){
							SHELLEXECUTEINFO si;
							::ZeroMemory( &si, sizeof(si) );
							si.cbSize   = sizeof(si);
							si.fMask    = SEE_MASK_IDLIST;
							si.lpVerb   = _T("open");
							si.lpIDList = pIDL;
							si.nShow    = SW_SHOWNORMAL;
							::ShellExecuteEx( &si );	// �t�H���_���J��
							pMalloc->Free( (void*)pIDL );
						}
						pMalloc->Release();
					}
					pDesktopFolder->Release();
				}
				break;
			case 101:	// �C���|�[�g�^�G�N�X�|�[�g�̋N�_���Z�b�g�i�N�_��ݒ�t�H���_�ɂ���j
				int nMsgResult = MYMESSAGEBOX(
					hwnd,
					MB_OKCANCEL | MB_ICONINFORMATION,
					GSTR_APPNAME,
					_T("�e��ݒ�̃C���|�[�g�^�G�N�X�|�[�g�p�t�@�C���I����ʂ�\n")
					_T("�����\���t�H���_��ݒ�t�H���_�ɖ߂��܂��B")
				);
				if( IDOK == nMsgResult )
				{
					DLLSHAREDATA *pShareData = CShareData::getInstance()->GetShareData();
					GetInidir( pShareData->m_sHistory.m_szIMPORTFOLDER );
					AddLastChar( pShareData->m_sHistory.m_szIMPORTFOLDER, _countof2(pShareData->m_sHistory.m_szIMPORTFOLDER), _T('\\') );
				}
				break;
			}
		}
		break;

	case WM_DESTROY:
		::SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)s_pOldPropSheetWndProc );
		break;
	}

	return ::CallWindowProc( (WNDPROC)s_pOldPropSheetWndProc, hwnd, uMsg, wParam, lParam );
}

/*!	�Ǝ��g���v���p�e�B�V�[�g�̃R�[���o�b�N�֐�
	@author ryoji
	@date 2007.05.25 �V�K
*/
static int CALLBACK PropSheetProc( HWND hwndDlg, UINT uMsg, LPARAM lParam )
{
	// �v���p�e�B�V�[�g�̏��������Ƀ{�^���ǉ��A�v���p�e�B�V�[�g�̃T�u�N���X�����s��
	if( uMsg == PSCB_INITIALIZED ){
		s_pOldPropSheetWndProc = (WNDPROC)::SetWindowLongPtr( hwndDlg, GWLP_WNDPROC, (LONG_PTR)PropSheetWndProc );
		HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle( NULL );
		HWND hwndBtn = ::CreateWindowEx( 0, _T("BUTTON"), _T("�ݒ�t�H���_(&/) >>"), BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 140, 20, hwndDlg, (HMENU)0x02000, hInstance, NULL );
		::SendMessage( hwndBtn, WM_SETFONT, (WPARAM)::SendMessage( hwndDlg, WM_GETFONT, 0, 0 ), MAKELPARAM( FALSE, 0 ) );
		::SetWindowPos( hwndBtn, ::GetDlgItem( hwndDlg, IDHELP), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	}
	return 0;
}


/*!	�Ǝ��g���v���p�e�B�V�[�g�i���ʐݒ�^�^�C�v�ʐݒ��ʗp�j
	@author ryoji
	@date 2007.05.25 �V�K
*/
int MyPropertySheet( LPPROPSHEETHEADER lppsph )
{
	// �l�ݒ�t�H���_���g�p����Ƃ��́u�ݒ�t�H���_�v�{�^����ǉ�����
	if( CShareData::getInstance()->IsPrivateSettings() ){
		lppsph->dwFlags |= PSH_USECALLBACK;
		lppsph->pfnCallback = PropSheetProc;
	}
	return ::PropertySheet( lppsph );
}


// To Here 2007.05.25 ryoji �Ǝ��g���̃v���p�e�B�V�[�g�֐��Q
///////////////////////////////////////////////////////////////////////




/*	�w���v�̖ڎ���\��
	�ڎ��^�u��\���B��肪����o�[�W�����ł́A�ڎ��y�[�W��\���B
*/
void ShowWinHelpContents( HWND hwnd, LPCTSTR lpszHelp )
{
	COsVersionInfo cOsVer;
	if ( cOsVer.HasWinHelpContentsProblem() ){
		/* �ڎ��y�[�W��\������ */
		MyWinHelp( hwnd, lpszHelp, HELP_CONTENTS , 0 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return;
	}
	/* �ڎ��^�u��\������ */
	MyWinHelp( hwnd, lpszHelp, HELP_COMMAND, (ULONG_PTR)"CONTENTS()" );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	return;
}





// Stonee, 2001/12/21
// NetWork��̃��\�[�X�ɐڑ����邽�߂̃_�C�A���O���o��������
// NO_ERROR:���� ERROR_CANCELLED:�L�����Z�� ����ȊO:���s
// �v���W�F�N�g�̐ݒ�Ń����N���W���[����Mpr.lib��ǉ��̂���
DWORD NetConnect ( const TCHAR strNetWorkPass[] )
{
	//char sPassWord[] = "\0";	//�p�X���[�h
	//char sUser[] = "\0";		//���[�U�[��
	DWORD dwRet;				//�߂�l�@�G���[�R�[�h��WINERROR.H���Q��
	TCHAR sTemp[256];
	TCHAR sDrive[] = _T("");
    int i;

	if (_tcslen(strNetWorkPass) < 3)	return ERROR_BAD_NET_NAME;  //UNC�ł͂Ȃ��B
	if (strNetWorkPass[0] != _T('\\') && strNetWorkPass[1] != _T('\\'))	return ERROR_BAD_NET_NAME;  //UNC�ł͂Ȃ��B

	//3�����ڂ��琔���čŏ���\�̒��O�܂ł�؂�o��
	sTemp[0] = _T('\\');
	sTemp[1] = _T('\\');
	for (i = 2; strNetWorkPass[i] != _T('\0'); i++) {
		if (strNetWorkPass[i] == _T('\\')) break;
		sTemp[i] = strNetWorkPass[i];
	}
	sTemp[i] = _T('\0');	//�I�[

	//NETRESOURCE�쐬
	NETRESOURCE nr;
	ZeroMemory( &nr, sizeof( nr ) );
	nr.dwScope       = RESOURCE_GLOBALNET;
	nr.dwType        = RESOURCETYPE_DISK;
	nr.dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
	nr.dwUsage       = RESOURCEUSAGE_CONNECTABLE;
	nr.lpLocalName   = sDrive;
	nr.lpRemoteName  = sTemp;

	//���[�U�[�F�؃_�C�A���O��\��
	dwRet = WNetAddConnection3(0, &nr, NULL, NULL, CONNECT_UPDATE_PROFILE | CONNECT_INTERACTIVE);

	return dwRet;
}




//	From Here Jun. 26, 2001 genta
/*!
	HTML Help�R���|�[�l���g�̃A�N�Z�X��񋟂���B
	�����ŕێ����ׂ��f�[�^�͓��ɂȂ��A���鏊����g����̂�Global�ϐ��ɂ��邪�A
	���ڂ̃A�N�Z�X��OpenHtmlHelp()�֐��݂̂���s���B
	���̃t�@�C�������CHtmlHelp�N���X�͉B����Ă���B
*/
CHtmlHelp g_cHtmlHelp;

/*!
	HTML Help���J��
	HTML Help�����p�\�ł���Έ��������̂܂ܓn���A�����łȂ���΃��b�Z�[�W��\������B

	@return �J�����w���v�E�B���h�E�̃E�B���h�E�n���h���B�J���Ȃ������Ƃ���NULL�B
*/

HWND OpenHtmlHelp(
	HWND		hWnd,	//!< [in] �Ăяo�����E�B���h�E�̃E�B���h�E�n���h��
	LPCTSTR		szFile,	//!< [in] HTML Help�̃t�@�C�����B�s�����ɑ����ăE�B���h�E�^�C�v�����w��\�B
	UINT		uCmd,	//!< [in] HTML Help �ɓn���R�}���h
	DWORD_PTR	data,	//!< [in] �R�}���h�ɉ������f�[�^
	bool		msgflag	//!< [in] �G���[���b�Z�[�W��\�����邩�B�ȗ�����true�B
)
{
	if( DLL_SUCCESS == g_cHtmlHelp.InitDll() ){
		return g_cHtmlHelp.HtmlHelp( hWnd, szFile, uCmd, data );
	}
	if( msgflag ){
		::MessageBox(
			hWnd,
			_T("HHCTRL.OCX��������܂���B\r\n")
			_T("HTML�w���v�𗘗p����ɂ�HHCTRL.OCX���K�v�ł��B\r\n"),
			_T("���"),
			MB_OK | MB_ICONEXCLAMATION
		);
	}
	return NULL;
}

//	To Here Jun. 26, 2001 genta




/*! �V���[�g�J�b�g(.lnk)�̉���
	@date 2009.01.08 ryoji CoInitialize/CoUninitialize���폜�iWinMain��OleInitialize/OleUninitialize��ǉ��j
*/
BOOL ResolveShortcutLink( HWND hwnd, LPCTSTR lpszLinkFile, LPTSTR lpszPath )
{
	BOOL			bRes;
	HRESULT			hRes;
	IShellLink*		pIShellLink;
	IPersistFile*	pIPersistFile;
	WIN32_FIND_DATA	wfd;
	/* ������ */
	pIShellLink = NULL;
	pIPersistFile = NULL;
	*lpszPath = 0; // assume failure
	bRes = FALSE;

// 2009.01.08 ryoji CoInitialize���폜�iWinMain��OleInitialize�ǉ��j

	// Get a pointer to the IShellLink interface.
//	hRes = 0;
	TCHAR szAbsLongPath[_MAX_PATH];
	if( ! ::GetLongFileName( lpszLinkFile, szAbsLongPath ) ){
		return FALSE;
	}

	// 2010.08.28 DLL �C���W�F�N�V�����΍�Ƃ���EXE�̃t�H���_�Ɉړ�����
	CCurrentDirectoryBackupPoint dirBack;
	ChangeCurrentDirectoryToExeDir();

	if( SUCCEEDED( hRes = ::CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pIShellLink ) ) ){
		// Get a pointer to the IPersistFile interface.
		if( SUCCEEDED(hRes = pIShellLink->QueryInterface( IID_IPersistFile, (void**)&pIPersistFile ) ) ){
			// Ensure that the string is Unicode.
			WCHAR wsz[MAX_PATH];
			_tcstowcs(wsz, szAbsLongPath, _countof(wsz));
//			MultiByteToWideChar( CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH );
			// Load the shortcut.
			if( SUCCEEDED(hRes = pIPersistFile->Load( wsz, STGM_READ ) ) ){
				// Resolve the link.
				if( SUCCEEDED( hRes = pIShellLink->Resolve(hwnd, SLR_ANY_MATCH ) ) ){
					// Get the path to the link target.
					TCHAR szGotPath[MAX_PATH];
					szGotPath[0] = _T('\0');
					if( SUCCEEDED( hRes = pIShellLink->GetPath(szGotPath, MAX_PATH, &wfd, SLGP_SHORTPATH ) ) ){
						// Get the description of the target.
						TCHAR szDescription[MAX_PATH];
						if( SUCCEEDED(hRes = pIShellLink->GetDescription(szDescription, MAX_PATH ) ) ){
							if( _T('\0') != szGotPath[0] ){
								/* ����I�� */
								_tcscpy_s( lpszPath, _MAX_PATH, szGotPath );
								bRes = TRUE;
							}
						}
					}
				}
			}
		}
	}
	// Release the pointer to the IPersistFile interface.
	if( NULL != pIPersistFile ){
		pIPersistFile->Release();
		pIPersistFile = NULL;
	}
	// Release the pointer to the IShellLink interface.
	if( NULL != pIShellLink ){
		pIShellLink->Release();
		pIShellLink = NULL;
	}
// 2009.01.08 ryoji CoUninitialize���폜�iWinMain��OleUninitialize�ǉ��j
	return bRes;
}





/*!	WinHelp �̂����� HtmlHelp ���Ăяo��

	@author ryoji
	@date 2006.07.22 ryoji �V�K
*/
BOOL MyWinHelp(HWND hwndCaller, LPCTSTR lpszHelp, UINT uCommand, DWORD_PTR dwData)
{
	UINT uCommandOrg = uCommand;	// WinHelp �̃R�}���h
	bool bDesktop = false;	// �f�X�N�g�b�v��e�ɂ��ăw���v��ʂ��o�����ǂ���
	HH_POPUP hp;	// �|�b�v�A�b�v�w���v�p�̍\����

	// Note: HH_TP_HELP_CONTEXTMENU �� HELP_WM_HELP �ł̓w���v�R���p�C������
	// �g�s�b�N�t�@�C���� Cshelp.txt �ȊO�ɂ��Ă���ꍇ�A
	// ���̃t�@�C������ .chm �p�X���ɒǉ��w�肷��K�v������B
	//     ��jsakura.chm::/xxx.txt

	switch( uCommandOrg )
	{
	case HELP_COMMAND:	// [�w���v]-[�ڎ�]
	case HELP_CONTENTS:
		uCommand = HH_DISPLAY_TOC;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_KEY:	// [�w���v]-[�L�[���[�h����]
		uCommand = HH_DISPLAY_INDEX;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_CONTEXT:	// ���j���[��ł�[F1]�L�[�^�_�C�A���O��[�w���v]�{�^��
		uCommand = HH_HELP_CONTEXT;
		hwndCaller = ::GetDesktopWindow();
		bDesktop = true;
		break;
	case HELP_CONTEXTMENU:	// �R���g���[����ł̉E�N���b�N
	case HELP_WM_HELP:		// [�H]�{�^���������ăR���g���[�����N���b�N�^�R���g���[���Ƀt�H�[�J�X��u����[F1]�L�[
		uCommand = HH_DISPLAY_TEXT_POPUP;
		{
			// �|�b�v�A�b�v�w���v�p�̍\���̂ɒl���Z�b�g����
			HWND hwndCtrl;	// �w���v�\���Ώۂ̃R���g���[��
			int nCtrlID;	// �ΏۃR���g���[���� ID
			DWORD* pHelpIDs;	// �R���g���[�� ID �ƃw���v ID �̑Ή��\�ւ̃|�C���^

			memset(&hp, 0, sizeof(hp));	// �\���̂��[���N���A
			hp.cbStruct = sizeof(hp);
			hp.pszFont = _T("�l�r �o�S�V�b�N, 9");
			hp.clrForeground = hp.clrBackground = -1;
			hp.rcMargins.left = hp.rcMargins.top = hp.rcMargins.right = hp.rcMargins.bottom = -1;
			if( uCommandOrg == HELP_CONTEXTMENU ){
				// �}�E�X�J�[�\���ʒu����ΏۃR���g���[���ƕ\���ʒu�����߂�
				if( !::GetCursorPos(&hp.pt) )
					return FALSE;
				hwndCtrl = ::WindowFromPoint(hp.pt);
			}
			else{
				// �ΏۃR���g���[���� hwndCaller
				// [F1]�L�[�̏ꍇ������̂őΏۃR���g���[���̈ʒu����\���ʒu�����߂�
				RECT rc;
				hwndCtrl = hwndCaller;
				if( !::GetWindowRect( hwndCtrl, &rc ) )
					return FALSE;
				hp.pt.x = (rc.left + rc.right) / 2;
				hp.pt.y = rc.top;
			}
			// �ΏۃR���g���[���ɑΉ�����w���v ID ��T��
			nCtrlID = ::GetDlgCtrlID( hwndCtrl );
			if( nCtrlID <= 0 )
				return FALSE;
			pHelpIDs = (DWORD*)dwData;
			while( true ){
				if( *pHelpIDs == 0 )
					return FALSE;	// ������Ȃ�����
				if( *pHelpIDs == nCtrlID )
					break;			// ��������
				pHelpIDs += 2;
			}
			hp.idString = *(pHelpIDs + 1);	// �������w���v ID ��ݒ肷��
			dwData = (DWORD_PTR)&hp;	// �������|�b�v�A�b�v�w���v�p�̍\���̂ɍ����ւ���
		}
		break;
	default:
		return FALSE;
	}

	if( IsFileExists( lpszHelp, true ) ){
		// HTML �w���v���Ăяo��
		HWND hWnd = OpenHtmlHelp( hwndCaller, lpszHelp, uCommand, dwData );
		if (bDesktop && hWnd != NULL){
			::SetForegroundWindow( hWnd );	// �w���v��ʂ���O�ɏo��
		}
	}
	else {
		if( uCommandOrg == HELP_CONTEXTMENU)
			return FALSE;	// �E�N���b�N�ł͉������Ȃ��ł���

		// �I�����C���w���v���Ăяo��
		if( uCommandOrg != HELP_CONTEXT )
			dwData = 1;	// �ڎ��y�[�W

		TCHAR buf[256];
		_stprintf( buf, _T("http://sakura-editor.sourceforge.net/cgi-bin/hid.cgi?%d"), dwData );
		ShellExecute( ::GetActiveWindow(), NULL, buf, NULL, NULL, SW_SHOWNORMAL );
	}

	return TRUE;
}




//�t�H���g�I���_�C�A���O
//2008.04.27 kobake CEditDoc::SelectFont ���番��
// 2009.10.01 ryoji �|�C���g�T�C�Y�i1/10�|�C���g�P�ʁj�����ǉ�
BOOL MySelectFont( LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner )
{
	// 2004.02.16 Moca CHOOSEFONT�������o����O��
	CHOOSEFONT cf;
	/* CHOOSEFONT�̏����� */
	::ZeroMemory( &cf, sizeof( cf ) );
	cf.lStructSize = sizeof( cf );
	cf.hwndOwner = hwndDlgOwner;
	cf.hDC = NULL;
//	cf.lpLogFont = &(GetDllShareData().m_Common.m_lf);
	cf.Flags = CF_FIXEDPITCHONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;

	//FIXED�t�H���g�ȊO
	#ifdef USE_UNFIXED_FONT
		cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
	#endif

	cf.lpLogFont = plf;
	if( !ChooseFont( &cf ) ){
#ifdef _DEBUG
		DWORD nErr;
		nErr = CommDlgExtendedError();
		switch( nErr ){
		case CDERR_FINDRESFAILURE:	MYTRACE_A( "CDERR_FINDRESFAILURE \n" );	break;
		case CDERR_INITIALIZATION:	MYTRACE_A( "CDERR_INITIALIZATION \n" );	break;
		case CDERR_LOCKRESFAILURE:	MYTRACE_A( "CDERR_LOCKRESFAILURE \n" );	break;
		case CDERR_LOADRESFAILURE:	MYTRACE_A( "CDERR_LOADRESFAILURE \n" );	break;
		case CDERR_LOADSTRFAILURE:	MYTRACE_A( "CDERR_LOADSTRFAILURE \n" );	break;
		case CDERR_MEMALLOCFAILURE:	MYTRACE_A( "CDERR_MEMALLOCFAILURE\n" );	break;
		case CDERR_MEMLOCKFAILURE:	MYTRACE_A( "CDERR_MEMLOCKFAILURE \n" );	break;
		case CDERR_NOHINSTANCE:		MYTRACE_A( "CDERR_NOHINSTANCE \n" );		break;
		case CDERR_NOHOOK:			MYTRACE_A( "CDERR_NOHOOK \n" );			break;
		case CDERR_NOTEMPLATE:		MYTRACE_A( "CDERR_NOTEMPLATE \n" );		break;
		case CDERR_STRUCTSIZE:		MYTRACE_A( "CDERR_STRUCTSIZE \n" );		break;
		case CFERR_MAXLESSTHANMIN:	MYTRACE_A( "CFERR_MAXLESSTHANMIN \n" );	break;
		case CFERR_NOFONTS:			MYTRACE_A( "CFERR_NOFONTS \n" );			break;
		}
#endif
		return FALSE;
	}
	*piPointSize = cf.iPointSize;

	return TRUE;
}
