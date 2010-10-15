#include "stdafx.h"
#include "module.h"
#include "util/os.h"
#include "util/file.h"
#include <Shlwapi.h>	// 2006.06.17 ryoji

/*! 
	�J�����g�f�B���N�g�������s�t�@�C���̏ꏊ�Ɉړ�
	@date 2010.08.28 Moca �V�K�쐬
*/void ChangeCurrentDirectoryToExeDir()
{
	TCHAR szExeDir[_MAX_PATH];
	szExeDir[0] = _T('\0');
	GetExedir( szExeDir, NULL );
	if( szExeDir[0] ){
		::SetCurrentDirectory( szExeDir );
	}else{
		// �ړ��ł��Ȃ��Ƃ���SYSTEM32(9x�ł�SYSTEM)�Ɉړ�
		szExeDir[0] = _T('\0');
		int n = ::GetSystemDirectory( szExeDir, _MAX_PATH );
		if( n && n < _MAX_PATH ){
			::SetCurrentDirectory( szExeDir );
		}
	}
}

/*! 
	@date 2010.08.28 Moca �V�K�쐬
*/
HMODULE LoadLibraryExedir(LPCTSTR pszDll)
{
	CCurrentDirectoryBackupPoint dirBack;
	// DLL �C���W�F�N�V�����΍�Ƃ���EXE�̃t�H���_�Ɉړ�����
	ChangeCurrentDirectoryToExeDir();
	return ::LoadLibrary( pszDll );
}

/*!	�V�F����R�����R���g���[�� DLL �̃o�[�W�����ԍ����擾

	@param[in] lpszDllName DLL �t�@�C���̃p�X
	@return DLL �̃o�[�W�����ԍ��i���s���� 0�j

	@author ? (from MSDN Library document)
	@date 2006.06.17 ryoji MSDN���C�u����������p
*/
DWORD GetDllVersion(LPCTSTR lpszDllName)
{
	HINSTANCE hinstDll;
	DWORD dwVersion = 0;

	/* For security purposes, LoadLibrary should be provided with a
	   fully-qualified path to the DLL. The lpszDllName variable should be
	   tested to ensure that it is a fully qualified path before it is used. */
	hinstDll = LoadLibraryExedir(lpszDllName);

	if(hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll,
						  "DllGetVersion");

		/* Because some DLLs might not implement this function, you
		must test for it explicitly. Depending on the particular
		DLL, the lack of a DllGetVersion function can be a useful
		indicator of the version. */

		if(pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;

			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			hr = (*pDllGetVersion)(&dvi);

			if(SUCCEEDED(hr))
			{
			   dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
			}
		}

		FreeLibrary(hinstDll);
	}
	return dwVersion;
}



/*!
	@brief �A�v���P�[�V�����A�C�R���̎擾
	
	�A�C�R���t�@�C�������݂���ꍇ�͂�������C�����ꍇ��
	���\�[�X�t�@�C������擾����
	
	@param hInst [in] Instance Handle
	@param nResource [in] �f�t�H���g�A�C�R���pResource ID
	@param szFile [in] �A�C�R���t�@�C����
	@param bSmall [in] true: small icon (16x16) / false: large icon (32x32)
	
	@return �A�C�R���n���h���D���s�����ꍇ��NULL�D
	
	@date 2002.12.02 genta �V�K�쐬
	@data 2007.05.20 ryoji ini�t�@�C���p�X��D��
	@author genta
*/
HICON GetAppIcon( HINSTANCE hInst, int nResource, const TCHAR* szFile, bool bSmall )
{
	// �T�C�Y�̐ݒ�
	int size = ( bSmall ? 16 : 32 );

	TCHAR szPath[_MAX_PATH];
	HICON hIcon;

	// �t�@�C������̓ǂݍ��݂��܂����݂�
	GetInidirOrExedir( szPath, szFile );

	hIcon = (HICON)::LoadImage(
		NULL,
		szPath,
		IMAGE_ICON,
		size,
		size,
		LR_SHARED | LR_LOADFROMFILE
	);
	if( hIcon != NULL ){
		return hIcon;
	}

	//	�t�@�C������̓ǂݍ��݂Ɏ��s�����烊�\�[�X����擾
	hIcon = (HICON)::LoadImage(
		hInst,
		MAKEINTRESOURCE(nResource),
		IMAGE_ICON,
		size,
		size,
		LR_SHARED
	);
	
	return hIcon;
}






struct VS_VERSION_INFO_HEAD {
	WORD	wLength;
	WORD	wValueLength;
	WORD	bText;
	WCHAR	szKey[16];
	VS_FIXEDFILEINFO Value;
};

/*! ���\�[�X���琻�i�o�[�W�����̎擾
	@date 2004.05.13 Moca ��x�擾������L���b�V������
*/
void GetAppVersionInfo(
	HINSTANCE	hInstance,
	int			nVersionResourceID,
	DWORD*		pdwProductVersionMS,
	DWORD*		pdwProductVersionLS
)
{
	HRSRC					hRSRC;
	HGLOBAL					hgRSRC;
	VS_VERSION_INFO_HEAD*	pVVIH;
	/* ���\�[�X���琻�i�o�[�W�����̎擾 */
	*pdwProductVersionMS = 0;
	*pdwProductVersionLS = 0;
	static bool bLoad = false;
	static DWORD dwVersionMS = 0;
	static DWORD dwVersionLS = 0;
	if( hInstance == NULL && bLoad ){
		*pdwProductVersionMS = dwVersionMS;
		*pdwProductVersionLS = dwVersionLS;
		return;
	}
	if( NULL != ( hRSRC = ::FindResource( hInstance, MAKEINTRESOURCE(nVersionResourceID), RT_VERSION ) )
	 && NULL != ( hgRSRC = ::LoadResource( hInstance, hRSRC ) )
	 && NULL != ( pVVIH = (VS_VERSION_INFO_HEAD*)::LockResource( hgRSRC ) )
	){
		*pdwProductVersionMS = pVVIH->Value.dwProductVersionMS;
		*pdwProductVersionLS = pVVIH->Value.dwProductVersionLS;
		dwVersionMS = pVVIH->Value.dwProductVersionMS;
		dwVersionLS = pVVIH->Value.dwProductVersionLS;
	}
	if( hInstance == NULL ){
		bLoad = true;
	}
	return;

}
