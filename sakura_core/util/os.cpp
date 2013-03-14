#include "StdAfx.h"
#include "os.h"
#include "util/module.h"
#include "extmodule/CUxTheme.h"

/*!	Comctl32.dll �̃o�[�W�����ԍ����擾

	@return Comctl32.dll �̃o�[�W�����ԍ��i���s���� 0�j

	@author ryoji
	@date 2006.06.17 ryoji �V�K
*/
static DWORD s_dwComctl32Version = PACKVERSION(0, 0);
DWORD GetComctl32Version()
{
	if( PACKVERSION(0, 0) == s_dwComctl32Version )
		s_dwComctl32Version = GetDllVersion(_T("Comctl32.dll"));
	return s_dwComctl32Version;
}


/*!	���������݃r�W���A���X�^�C���\����Ԃ��ǂ���������
	Win32 API �� IsAppThemed() �͂���Ƃ͈�v���Ȃ��iIsAppThemed() �� IsThemeActive() �Ƃ̍��ق͕s���j

	@return �r�W���A���X�^�C���\�����(TRUE)�^�N���b�V�b�N�\�����(FALSE)

	@author ryoji
	@date 2006.06.17 ryoji �V�K
*/
BOOL IsVisualStyle()
{
	// ���[�h���� Comctl32.dll �� Ver 6 �ȏ�ŉ�ʐݒ肪�r�W���A���X�^�C���w��ɂȂ��Ă���ꍇ����
	// �r�W���A���X�^�C���\���ɂȂ�i�}�j�t�F�X�g�Ŏw�肵�Ȃ��� Comctl32.dll �� 6 �����ɂȂ�j
	return ( (GetComctl32Version() >= PACKVERSION(6, 0)) && CUxTheme::getInstance()->IsThemeActive() );
}



/*!	�w��E�B���h�E�Ńr�W���A���X�^�C�����g��Ȃ��悤�ɂ���

	@param[in] hWnd �E�B���h�E

	@author ryoji
	@date 2006.06.23 ryoji �V�K
*/
void PreventVisualStyle( HWND hWnd )
{
	CUxTheme::getInstance()->SetWindowTheme( hWnd, L"", L"" );
	return;
}




/*!	�R�����R���g���[��������������

	@author ryoji
	@date 2006.06.21 ryoji �V�K
*/
void MyInitCommonControls()
{
	BOOL (WINAPI *pfnInitCommonControlsEx)(LPINITCOMMONCONTROLSEX);

	BOOL bInit = FALSE;
	HINSTANCE hDll = ::GetModuleHandle(_T("COMCTL32"));
	if( NULL != hDll ){
		*(FARPROC*)&pfnInitCommonControlsEx = ::GetProcAddress( hDll, "InitCommonControlsEx" );
		if( NULL != pfnInitCommonControlsEx ){
			INITCOMMONCONTROLSEX icex;
			icex.dwSize = sizeof(icex);
			icex.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES;
			bInit = pfnInitCommonControlsEx( &icex );
		}
	}

	if( !bInit ){
		::InitCommonControls();
	}
}





/*!
	�w�肵���E�B���h�E�^�����`�̈�^�_�^���j�^�ɑΉ����郂�j�^��Ɨ̈���擾����

	���j�^��Ɨ̈�F��ʑS�̂���V�X�e���̃^�X�N�o�[��A�v���P�[�V�����̃c�[���o�[����L����̈���������̈�

	@param hWnd/prc/pt/hMon [in] �ړI�̃E�B���h�E�^�����`�̈�^�_�^���j�^
	@param prcWork [out] ���j�^��Ɨ̈�
	@param prcMonitor [out] ���j�^��ʑS��

	@retval true �Ή����郂�j�^�̓v���C�}�����j�^
	@retval false �Ή����郂�j�^�͔�v���C�}�����j�^

	@note �o�̓p�����[�^�� prcWork �� prcMonior �� NULL ���w�肵���ꍇ�A
	�Y������̈���͏o�͂��Ȃ��B�Ăяo�����͗~�������̂������w�肷��΂悢�B
*/
//	From Here May 01, 2004 genta MutiMonitor
bool GetMonitorWorkRect(HWND hWnd, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	// 2006.04.21 ryoji Windows API �`���̊֐��Ăяo���ɕύX�i�X�^�u�� PSDK �� MultiMon.h �𗘗p�j
	HMONITOR hMon = ::MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}
//	To Here May 01, 2004 genta

//	From Here 2006.04.21 ryoji MutiMonitor
bool GetMonitorWorkRect(LPCRECT prc, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	HMONITOR hMon = ::MonitorFromRect( prc, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}

bool GetMonitorWorkRect(POINT pt, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	HMONITOR hMon = ::MonitorFromPoint( pt, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}

bool GetMonitorWorkRect(HMONITOR hMon, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	MONITORINFO mi;
	::ZeroMemory( &mi, sizeof( mi ));
	mi.cbSize = sizeof( mi );
	::GetMonitorInfo( hMon, &mi );
	if( NULL != prcWork )
		*prcWork = mi.rcWork;		// work area rectangle of the display monitor
	if( NULL != prcMonitor )
		*prcMonitor = mi.rcMonitor;	// display monitor rectangle
	return ( mi.dwFlags == MONITORINFOF_PRIMARY ) ? true : false;
}
//	To Here 2006.04.21 ryoji MutiMonitor




/*!
	@brief ���W�X�g�����當�����ǂݏo���D
	
	@param Hive        [in]  HIVE
	@param Path        [in]  ���W�X�g���L�[�ւ̃p�X
	@param Item        [in]  ���W�X�g���A�C�e�����DNULL�ŕW���̃A�C�e���D
	@param Buffer      [out] �擾��������i�[����ꏊ
	@param BufferCount [in]  Buffer�̎w���̈�̃T�C�Y�B�����P�ʁB
	
	@retval true �l�̎擾�ɐ���
	@retval false �l�̎擾�Ɏ��s
	
	@author �S
	@date 2002.09.10 genta CWSH.cpp����ړ�
*/
bool ReadRegistry(HKEY Hive, const TCHAR* Path, const TCHAR* Item, TCHAR* Buffer, unsigned BufferCount)
{
	bool Result = false;
	
	HKEY Key;
	if(RegOpenKeyEx(Hive, Path, 0, KEY_READ, &Key) == ERROR_SUCCESS)
	{
		auto_memset(Buffer, 0, BufferCount);

		DWORD dwType = REG_SZ;
		DWORD dwDataLen = (BufferCount - 1) * sizeof(TCHAR); //���o�C�g�P�ʁI
		
		Result = (RegQueryValueEx(Key, Item, NULL, &dwType, reinterpret_cast<LPBYTE>(Buffer), &dwDataLen) == ERROR_SUCCESS);
		
		RegCloseKey(Key);
	}
	return Result;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �N���b�v�{�[�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//SetClipboardTextA,SetClipboardTextT �����p�e���v���[�g
//2007.08.14 kobake UNICODE�p�ɉ���
//
/*! �N���[�v�{�[�h��Text�`���ŃR�s�[����
	@param hwnd    [in] �N���b�v�{�[�h�̃I�[�i�[
	@param pszText [in] �ݒ肷��e�L�X�g
	@param nLength [in] �L���ȃe�L�X�g�̒����B�����P�ʁB
	
	@retval true �R�s�[����
	@retval false �R�s�[���s�B�ꍇ�ɂ���Ă̓N���b�v�{�[�h�Ɍ��̓��e���c��
	@date 2004.02.17 Moca �e���̃\�[�X�𓝍�
*/
template <class T>
SAKURA_CORE_API bool SetClipboardTextImp( HWND hwnd, const T* pszText, int nLength )
{
	HGLOBAL	hgClip;
	T*		pszClip;

	hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, (nLength + 1) * sizeof(T) );
	if( NULL == hgClip ){
		return false;
	}
	pszClip = (T*)::GlobalLock( hgClip );
	if( NULL == pszClip ){
		::GlobalFree( hgClip );
		return false;
	}
	auto_memcpy( pszClip, pszText, nLength );
	pszClip[nLength] = 0;
	::GlobalUnlock( hgClip );
	if( !::OpenClipboard( hwnd ) ){
		::GlobalFree( hgClip );
		return false;
	}
	::EmptyClipboard();
	if(sizeof(T)==sizeof(char)){
		::SetClipboardData( CF_OEMTEXT, hgClip );
	}
	else if(sizeof(T)==sizeof(wchar_t)){
		::SetClipboardData( CF_UNICODETEXT, hgClip );
	}
	else{
		assert(0); //�������ɂ͗��Ȃ�
	}
	::CloseClipboard();

	return true;
}

SAKURA_CORE_API bool SetClipboardText( HWND hwnd, const ACHAR* pszText, int nLength )
{
	return SetClipboardTextImp<ACHAR>(hwnd,pszText,nLength);
}

SAKURA_CORE_API bool SetClipboardText( HWND hwnd, const WCHAR* pszText, int nLength )
{
	return SetClipboardTextImp<WCHAR>(hwnd,pszText,nLength);
}

/*
	@date 2006.01.16 Moca ����TYMED�����p�\�ł��A�擾�ł���悤�ɕύX�B
	@note IDataObject::GetData() �� tymed = TYMED_HGLOBAL ���w�肷�邱�ƁB
*/
BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC	fe;

	// 2006.01.16 Moca ����TYMED�����p�\�ł��AIDataObject::GetData()��
	//  tymed = TYMED_HGLOBAL���w�肷��Ζ��Ȃ�
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = TYMED_HGLOBAL;
	// 2006.03.16 Moca S_FALSE�ł��󂯓���Ă��܂��o�O���C��(�t�@�C���̃h���b�v��)
	return S_OK == pDataObject->QueryGetData( &fe );
}

HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC fe;
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	// 2006.01.16 Moca fe.tymed = -1����TYMED_HGLOBAL�ɕύX�B
	fe.tymed = TYMED_HGLOBAL;

	HGLOBAL hDest = NULL;
	STGMEDIUM stgMedium;
	// 2006.03.16 Moca SUCCEEDED�}�N���ł�S_FALSE�̂Ƃ�����̂ŁAS_OK�ɕύX
	if( S_OK == pDataObject->GetData( &fe, &stgMedium ) ){
		if( stgMedium.pUnkForRelease == NULL ){
			if( stgMedium.tymed == TYMED_HGLOBAL )
				hDest = stgMedium.hGlobal;
		}else{
			if( stgMedium.tymed == TYMED_HGLOBAL ){
				DWORD nSize = ::GlobalSize( stgMedium.hGlobal );
				hDest = ::GlobalAlloc( GMEM_SHARE|GMEM_MOVEABLE, nSize );
				if( hDest != NULL ){
					// copy the bits
					LPVOID lpSource = ::GlobalLock( stgMedium.hGlobal );
					LPVOID lpDest = ::GlobalLock( hDest );
					memcpy_raw( lpDest, lpSource, nSize );
					::GlobalUnlock( hDest );
					::GlobalUnlock( stgMedium.hGlobal );
				}
			}
			::ReleaseStgMedium( &stgMedium );
		}
	}
	return hDest;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �V�X�e������                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �V�X�e�����\�[�X�𒲂ׂ�
	Win16 �̎��́AGetFreeSystemResources �Ƃ����֐�������܂����B�������AWin32 �ł͂���܂���B
	�T���N����邾�� DLL ����邾�͓̂�����܂��B�ȒP�ȕ��@��������܂��B
	���g���� Windows95 �� [�A�N�Z�T��]-[�V�X�e���c�[��] �Ƀ��\�[�X���[�^������̂Ȃ�A
	c:\windows\system\rsrc32.dll ������͂��ł��B����́A���\�[�X���[�^�Ƃ��� Win32 �A�v�����A
	Win16 �� GetFreeSystemResources �֐����ĂԈׂ� DLL �ł��B������g���܂��傤�B
*/
BOOL GetSystemResources(
	int*	pnSystemResources,
	int*	pnUserResources,
	int*	pnGDIResources
)
{
	#define GFSR_SYSTEMRESOURCES	0x0000
	#define GFSR_GDIRESOURCES		0x0001
	#define GFSR_USERRESOURCES		0x0002
	HINSTANCE	hlib;
	int (CALLBACK *GetFreeSystemResources)( int );

	hlib = ::LoadLibraryExedir( _T("RSRC32.dll") );
	if( (int)hlib > 32 ){
		GetFreeSystemResources = (int (CALLBACK *)( int ))GetProcAddress(
			hlib,
			"_MyGetFreeSystemResources32@4"
		);
		if( GetFreeSystemResources != NULL ){
			*pnSystemResources = GetFreeSystemResources( GFSR_SYSTEMRESOURCES );
			*pnUserResources = GetFreeSystemResources( GFSR_USERRESOURCES );
			*pnGDIResources = GetFreeSystemResources( GFSR_GDIRESOURCES );
			::FreeLibrary( hlib );
			return TRUE;
		}else{
			::FreeLibrary( hlib );
			return FALSE;
		}
	}else{
		return FALSE;
	}
}


/* �V�X�e�����\�[�X�̃`�F�b�N */
BOOL CheckSystemResources( const TCHAR* pszAppName )
{
	int		nSystemResources;
	int		nUserResources;
	int		nGDIResources;
	const TCHAR*	pszResourceName;
	/* �V�X�e�����\�[�X�̎擾 */
	if( GetSystemResources( &nSystemResources, &nUserResources,	&nGDIResources ) ){
//		MYTRACE_A( "nSystemResources=%d\n", nSystemResources );
//		MYTRACE_A( "nUserResources=%d\n", nUserResources );
//		MYTRACE_A( "nGDIResources=%d\n", nGDIResources );
		pszResourceName = NULL;
		if( nSystemResources <= 5 ){
			pszResourceName = _T("�V�X�e�� ");
		}else
		if( nUserResources <= 5 ){
			pszResourceName = _T("���[�U�[ ");
		}else
		if( nGDIResources <= 5 ){
			pszResourceName = _T("GDI ");
		}
		if( NULL != pszResourceName ){
			ErrorBeep();
			ErrorBeep();
			::MYMESSAGEBOX( NULL, MB_OK | /*MB_YESNO | */ MB_ICONSTOP | MB_APPLMODAL | MB_TOPMOST, pszAppName,
				_T("%ts���\�[�X���ɒ[�ɕs�����Ă��܂��B\n")
				_T("���̂܂�%ts���N������ƁA����ɓ��삵�Ȃ��\��������܂��B\n")
				_T("�V����%ts�̋N���𒆒f���܂��B\n")
				_T("\n")
				_T("�V�X�e�� ���\�[�X\t�c��  %d%%\n")
				_T("User ���\�[�X\t�c��  %d%%\n")
				_T("GDI ���\�[�X\t�c��  %d%%\n\n"),
				pszResourceName,
				pszAppName,
				pszAppName,
				nSystemResources,
				nUserResources,
				nGDIResources
			);
//			) ){
				return FALSE;
//			}
		}
	}
	return TRUE;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �֗��N���X                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�R���X�g���N�^�ŃJ�����g�f�B���N�g����ۑ����A�f�X�g���N�^�ŃJ�����g�f�B���N�g���𕜌����郂�m�B

CCurrentDirectoryBackupPoint::CCurrentDirectoryBackupPoint()
{
	int n = ::GetCurrentDirectory(_countof(m_szCurDir),m_szCurDir);
	if(n>0 && n<_countof(m_szCurDir)){
		//ok
	}
	else{
		//ng
		m_szCurDir[0] = _T('\0');
	}
}

CCurrentDirectoryBackupPoint::~CCurrentDirectoryBackupPoint()
{
	if(m_szCurDir[0]){
		::SetCurrentDirectory(m_szCurDir);
	}
}



