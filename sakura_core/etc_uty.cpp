//	$Id$
/*!	@file
	���ʊ֐��Q

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//#include <stdio.h>
#include <io.h>
#include "etc_uty.h"
#include "debug.h"
#include "CMemory.h"
#include "funccode.h"	//Stonee, 2001/02/23


/* ���t���t�H�[�}�b�g */
const char* MyGetDateFormat( char* pszDest, int nDestLen, int nDateFormatType, const char* pszDateFormat )
{
	SYSTEMTIME systime;
	const char* pszForm;
	DWORD dwFlags;
	::GetLocalTime( &systime );
	if( 0 == nDateFormatType ){
		dwFlags = DATE_LONGDATE;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = pszDateFormat;
	}
	::GetDateFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}




/* �������t�H�[�}�b�g */
const char* MyGetTimeFormat( char* pszDest, int nDestLen, int nTimeFormatType, const char* pszTimeFormat )
{
	SYSTEMTIME systime;
	const char* pszForm;
	DWORD dwFlags;
	::GetLocalTime( &systime );
	if( 0 == nTimeFormatType ){
		dwFlags = 0;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = pszTimeFormat;
	}
	::GetTimeFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}




int CALLBACK MYBrowseCallbackProc(
	HWND hwnd,
	UINT uMsg,
	LPARAM lParam,
	LPARAM lpData
)
{
	switch( uMsg ){
	case BFFM_INITIALIZED:
//		MYTRACE( "BFFM_INITIALIZED (char*)lpData = [%s]\n", (char*)lpData );
		::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData );
		break;
	case BFFM_SELCHANGED:
//		MYTRACE( "BFFM_SELCHANGED\n" );
		break;
	}
	return 0;

}




/* �t�H���_�I���_�C�A���O */
BOOL SelectDir( HWND hWnd, const char* pszTitle, const char* pszInitFolder, char* strFolderName )
{
	BOOL	bRes;
	char	szInitFolder[MAX_PATH];

	strcpy( szInitFolder, pszInitFolder );
	/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� "c:\\"���̃��[�g�͎�菜���Ȃ�*/
	CutLastYenFromDirectoryPath( szInitFolder );

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
	ITEMIDLIST* pList = ::SHBrowseForFolder(&bi);
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




/* �p�X���ɑ΂���A�C�e���h�c���X�g���쐬���� */
ITEMIDLIST* CreateItemIDList( const char* pszPath )
{
	ITEMIDLIST*		pIDL;
	IShellFolder*	pDesktopFolder;
	OLECHAR			ochPath[MAX_PATH + 1];
	ULONG			chEaten;			//������̃T�C�Y���󂯎��܂��B
	ULONG			dwAttributes;		//�������󂯎��܂��B
	HRESULT			hRes;
	if( '\0' == pszPath[0] ){
		return NULL;
	}
	if( ::SHGetDesktopFolder( &pDesktopFolder ) != NOERROR ){
		return NULL;
	}
	//  ��������Ȃ��ƃC���^�[�t�F�C�X�̓_���Ȃ̂ł��B
	::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszPath, -1, ochPath, MAX_PATH );
	//  ���ۂ�ITEMIDLIST���擾���܂��B
	hRes = pDesktopFolder->ParseDisplayName( NULL, NULL, ochPath, &chEaten, &pIDL, &dwAttributes);
	if( hRes != NOERROR ){
		pIDL = NULL;
	}
	pDesktopFolder->Release();
	return pIDL;
}




/* �A�C�e���h�c���X�g���폜���� */
BOOL DeleteItemIDList( ITEMIDLIST* pIDL )
{
	IMalloc*	pMalloc;
	//  IMalloc�C���^�[�t�F�C�X�ւ̃|�C���^���擾���܂��B
	if( NOERROR != ::SHGetMalloc( &pMalloc ) ){
		return FALSE;
	}
	//  �A�C�e���h�c���X�g�������[�X���܂��B
	if( NULL != pIDL ){
		pMalloc->Free( pIDL );
	}
	pMalloc->Release();
	return TRUE;
}




/* �g���q�𒲂ׂ� */
BOOL CheckEXT( const char* pszPath, const char* pszExt )
{
	char	szExt[_MAX_EXT];
	char*	pszWork;
	_splitpath( pszPath, NULL, NULL, NULL, szExt );
	pszWork = szExt;
	if( pszWork[0] == '.' ){
		pszWork++;
	}
	if( 0 == _stricmp( pszExt, pszWork ) ){
		return TRUE;
	}else{
		return FALSE;
	}
}




char* my_strtok( char* pszStr, int nStrLen, int* pnPos, char* pszDelemitor )
{
	int		i;
	int		nBgn = -1;
	int		bComment = FALSE;
	i = *pnPos;
	while( i < nStrLen ){
		if( NULL == strchr( pszDelemitor, pszStr[i] ) ){
			break;
		}
		++i;
	}
	if( i >= nStrLen ){
		return NULL;
	}
	nBgn = i;
	while( i < nStrLen ){
		if( bComment ){
			if( '\"' == pszStr[i] ){
				if( i + 1 < nStrLen ){
					if( '\"' != pszStr[i + 1] ){
						bComment = FALSE;
						goto last_of_loop;
					}else{
						++i;
					}
				}
			}
		}else{
			if( '\"' == pszStr[i] ){
				bComment = TRUE;
				goto last_of_loop;
			}
			if( NULL != strchr( pszDelemitor, pszStr[i] ) ){
				break;
			}
		}
last_of_loop:;
		++i;
	}
	pszStr[i] = '\0';
	*pnPos = i + 1;
	return &pszStr[nBgn];
}




/* �w���v�t�@�C���̃t���p�X��Ԃ� */
char* GetHelpFilePath( char* pszHelpFile )
{
//	int		i;
	long	lPathLen;
	char	szHelpFile[_MAX_PATH + 1];
//	int		nCharChars;
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	/* �w���v�t�@�C���̃t�@�C���p�X */
	lPathLen = ::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szHelpFile, sizeof(szHelpFile)
	);
	_splitpath( szHelpFile, szDrive, szDir, NULL, NULL );
	strcpy( szHelpFile, szDrive );
	strcat( szHelpFile, szDir );
	strcat( szHelpFile, "sakura.hlp" );
	strcpy( pszHelpFile, szHelpFile );
	return pszHelpFile;
}




/* ���΃p�X����΃p�X */
BOOL GetAbsolutePath( const char* pszFilePathSrc, char* pszFilePathDes, BOOL bChangeCurrentFolder )
{
	char			szCurFolderOld[MAX_PATH];
	char			szCurFolder[MAX_PATH];
//	HANDLE			nFind;
//	WIN32_FIND_DATA	wfd;
	char			szPathSrc[MAX_PATH];
//	char			szPath2[MAX_PATH];
//	char			szPath3[MAX_PATH];
//	char			szPath4[MAX_PATH];
//	int				nPathSrcLen;
//	int				i;
	int				nCharChars;
//	int				nDirBgn;

	char			szDrive[_MAX_DRIVE];
	char			szDir[_MAX_DIR];
	char			szFname[_MAX_FNAME];
	char			szExt[_MAX_EXT];
	int				nFolderLen;
//	int				nDriveOrg;
	BOOL			bRet;
	bRet = TRUE;


	/* �J�����g�t�H���_�̑ޔ� */
	if( !bChangeCurrentFolder ){	/* �J�����g�t�H���_��ύX���邩 */
		::GetCurrentDirectory( MAX_PATH, szCurFolderOld );
	}

	_splitpath( pszFilePathSrc, szDrive, szDir, szFname, szExt );
//	MYTRACE( "_splitpath �֐��ɂ��p�X���̕���:\n" );
//	MYTRACE( " pszFilePathSrc    : %s\n", pszFilePathSrc );
//	MYTRACE( "  �h���C�u    : %s\n", szDrive );
//	MYTRACE( "  �f�B���N�g��: %s\n", szDir );
//	MYTRACE( "  �t�@�C����  : %s\n", szFname );
//	MYTRACE( "  �g���q      : %s\n", szExt );
	_makepath( szCurFolder, szDrive, szDir, NULL, NULL );
//	MYTRACE( "  SetCurrentDirectory     : %s\n", szCurFolder );
	if( 0 < strlen( szCurFolder ) && 0 == ::SetCurrentDirectory( szCurFolder ) ){
		bRet =  FALSE;
		goto end_of_func;
	}
	::GetCurrentDirectory( MAX_PATH, szCurFolder );
//	MYTRACE( "  GetCurrentDirectory     : %s\n", szCurFolder );

	/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� */
	nFolderLen = strlen( szCurFolder );
	if( 0 < nFolderLen ){
		nCharChars = &szCurFolder[nFolderLen] - CMemory::MemCharPrev( szCurFolder, nFolderLen, &szCurFolder[nFolderLen] );
		if( 1 == nCharChars && '\\' == szCurFolder[nFolderLen - 1] ){
			szCurFolder[nFolderLen - 1] = '\0';
		}
	}
	wsprintf( szPathSrc, "%s\\%s%s", szCurFolder, szFname, szExt );
//	MYTRACE( "  ��΃p�X      : %s\n", szPathSrc );
	strcpy( pszFilePathDes, szPathSrc );
end_of_func:;
	/* �J�����g�t�H���_�̕��A */
	if( !bChangeCurrentFolder ){	/* �J�����g�t�H���_��ύX���邩 */
		::SetCurrentDirectory( szCurFolderOld );
	}
	return bRet;

}




/* �����O�t�@�C�������擾���� */
BOOL GetLongFileName( const char* pszFilePathSrc, char* pszFilePathDes )
{
//	HANDLE			nFind;
//	WIN32_FIND_DATA	wfd;
//	char			szPathSrc[MAX_PATH];
//	char			szPath2[MAX_PATH];
//	char			szPath3[MAX_PATH];
//	int				nPathSrcLen;
//	int				i;
//	int				j;
//	int				nCharChars;
//	int				nDriveOrg;
	/* ���΃p�X����΃p�X */
	if( FALSE == GetAbsolutePath( pszFilePathSrc, pszFilePathDes, TRUE ) ){
		return FALSE;
	}

	ITEMIDLIST* pIDL;
	/* �p�X���ɑ΂���A�C�e���h�c���X�g���쐬���� */
	if( NULL == ( pIDL = CreateItemIDList( pszFilePathSrc ) ) ){
		//	May 9, 2000 genta
		char * name;
		int length;
		length = ::GetFullPathName( pszFilePathSrc, MAX_PATH, pszFilePathDes, &name );
//		::MessageBox( NULL, pszFilePathDes, "GetFullPathName", MB_OK );
		if( length <= 0 || MAX_PATH <= length ){
			::MessageBox( NULL, pszFilePathDes, "Too Long Path", MB_OK );
			return FALSE;
		}
	}
	else{
		// �A�C�e���h�c���X�g�̕����p�X���擾
		::SHGetPathFromIDList( pIDL, pszFilePathDes );

		/* �A�C�e���h�c���X�g���폜���� */
		DeleteItemIDList( pIDL );
	}
//	::MessageBox( NULL, pszFilePathDes, pszFilePathSrc, MB_OK );
	return TRUE;

//-2000.1.6 ���O�̏�������߂�
//-
//-	nPathSrcLen = strlen( szPathSrc );
//-	if( 2 < nPathSrcLen
//-	 && '\\' == szPathSrc[0]
//-	 && '\\' == szPathSrc[1]
//-	){
//-		/* \\�T�[�o��\�V�F�A��\... */
//-		i = 2 - 1;
//-		for( j = 0; j < 2; ++j ){
//-			++i;
//-			for( ; i < nPathSrcLen; ++i ){
//-				nCharChars = CMemory::MemCharNext( szPathSrc, nPathSrcLen, &szPathSrc[i] ) - &szPathSrc[i];
//-				if( 1 == nCharChars && '\\' == szPathSrc[i] ){
//-					break;
//-				}
//-				if( 2 == nCharChars ){
//-					++i;
//-				}
//-			}
//-		}
//-		nDriveOrg = i;
//-	}else{
//-		/* �h���C�u��\... */
//-		nDriveOrg = 2;
//-	}
//-	strcpy( szPath2, "" );
//-	nFind = ::FindFirstFile( szPathSrc, (WIN32_FIND_DATA*)&wfd );
//-	if( INVALID_HANDLE_VALUE == nFind ){
//-		return FALSE;
//-	}
//-	::FindClose( nFind );
//-	while( 1 ){
//-			strcpy( szPath3, "\\" );
//-			strcat( szPath3, wfd.cFileName );
//-			strcat( szPath3, szPath2 );
//-			strcpy( szPath2, szPath3 );
//-		nPathSrcLen = strlen( szPathSrc );
//-		nCharChars = &szPathSrc[nPathSrcLen] - CMemory::MemCharPrev( szPathSrc, nPathSrcLen, &szPathSrc[nPathSrcLen] );
//-		for( i = nPathSrcLen - nCharChars; i >= 0; i-- ){
//-			if( 1 == nCharChars && '\\' == szPathSrc[i] ){
//-				if( 0 == strcmp( ".", wfd.cFileName ) ){
//-					strcpy( szPath3, &szPathSrc[i] );
//-					strcat( szPath3, szPath2 );
//-					strcpy( szPath2, szPath3 );
//-				}
//-				szPathSrc[i] = '\0';
//-				break;
//-			}
//-			nCharChars = &szPathSrc[i] - CMemory::MemCharPrev( szPathSrc, nPathSrcLen, &szPathSrc[i] );
//-			if( 2 == nCharChars ){
//-				i--;
//-			}
//-		}
//-		if( 0 > i ){
//-			break;
//-		}
//-		if( nDriveOrg >= i ){
//-			strcpy( szPath3, szPathSrc );
//-			strcat( szPath3, szPath2 );
//-			strcpy( szPath2, szPath3 );
//-			break;
//-		}
//-		if( 2 == strlen( szPathSrc ) ){
//-			if( szPathSrc[1] == ':' ){
//-				strcpy( szPath3, szPathSrc );
//-				strcat( szPath3, szPath2 );
//-				strcpy( szPath2, szPath3 );
//-				break;
//-			}
//-		}
//-		nFind = ::FindFirstFile( szPathSrc, (WIN32_FIND_DATA*)&wfd );
//-		if( INVALID_HANDLE_VALUE == nFind ){
//-			strcpy( szPath3, szPathSrc );
//-			strcat( szPath3, szPath2 );
//-			strcpy( szPath2, szPath3 );
//-			break;
//-		}
//-		::FindClose( nFind );
//-	}
//-	strcpy( pszFilePathDes, szPath2 );
//-	/* �ň��̏ꍇ���l���āA���݂��Ȃ������O�p�X���𐶐������ꍇ�́A���Ƃ̃p�X���ɂ��� */
//-	if( -1 == _access( pszFilePathDes, 0 ) ){
//-		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, "��҂ɋ����ė~�����G���[",
//-			"�����O�t�@�C���l�[���̐����ŃG���[���o�܂����̂ŁA\n���̃t�@�C�������g���܂��B\n\n���Ƃ̃t�@�C����=[%s]\n���s���������O�t�@�C����=[%s]",
//-			pszFilePathSrc, pszFilePathDes
//-		);
//-		strcpy( pszFilePathDes, pszFilePathSrc );
//-	}
//-	return TRUE;
}




/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
void SplitPath_FolderAndFile( const char* pszFilePath, char* pszFolder, char* pszFile )
{
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	char	szFname[_MAX_FNAME];
	char	szExt[_MAX_EXT];
	int		nFolderLen;
	int		nCharChars;
	_splitpath( pszFilePath, szDrive, szDir, szFname, szExt );
	if( NULL != pszFolder ){
		strcpy( pszFolder, szDrive );
		strcat( pszFolder, szDir );
		/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� */
		nFolderLen = strlen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CMemory::MemCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && '\\' == pszFolder[nFolderLen - 1] ){
				pszFolder[nFolderLen - 1] = '\0';
			}
		}
	}
	if( NULL != pszFile ){
		strcpy( pszFile, szFname );
		strcat( pszFile, szExt );
	}
	return;
}




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

	hlib = ::LoadLibrary( "RSRC32.dll" );
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
BOOL CheckSystemResources( const char* pszAppName )
{
	int		nSystemResources;
	int		nUserResources;
	int		nGDIResources;
	char*	pszResourceName;
	/* �V�X�e�����\�[�X�̎擾 */
	if( GetSystemResources( &nSystemResources, &nUserResources,	&nGDIResources ) ){
//		MYTRACE( "nSystemResources=%d\n", nSystemResources );
//		MYTRACE( "nUserResources=%d\n", nUserResources );
//		MYTRACE( "nGDIResources=%d\n", nGDIResources );
		pszResourceName = NULL;
		if( nSystemResources <= 5 ){
			pszResourceName = "�V�X�e�� ";
		}else
		if( nUserResources <= 5 ){
			pszResourceName = "���[�U�[ ";
		}else
		if( nGDIResources <= 5 ){
			pszResourceName = "GDI ";
		}
		if( NULL != pszResourceName ){
			::MessageBeep( MB_ICONHAND );
			::MessageBeep( MB_ICONHAND );
//			if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNO | MB_ICONSTOP | MB_APPLMODAL | MB_TOPMOST, pszAppName,
			::MYMESSAGEBOX( NULL, MB_OK | /*MB_YESNO | */ MB_ICONSTOP | MB_APPLMODAL | MB_TOPMOST, pszAppName,
				"%s���\�[�X���ɒ[�ɕs�����Ă��܂��B\n\
���̂܂�%s���N������ƁA����ɓ��삵�Ȃ��\��������܂��B\n\
�V����%s�̋N���𒆒f���܂��B\n\
\n\
�V�X�e�� ���\�[�X\t�c��  %d%%\n\
User ���\�[�X\t�c��  %d%%\n\
GDI ���\�[�X\t�c��  %d%%\n\n",
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




struct VS_VERSION_INFO_HEAD {
	WORD	wLength;
	WORD	wValueLength;
	WORD	bText;
	WCHAR	szKey[16];
	VS_FIXEDFILEINFO Value;
};

/* ���\�[�X���琻�i�o�[�W�����̎擾 */
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
	if( NULL != ( hRSRC = ::FindResource( hInstance, MAKEINTRESOURCE(nVersionResourceID), RT_VERSION ) )
	 && NULL != ( hgRSRC = ::LoadResource( hInstance, hRSRC ) )
	 && NULL != ( pVVIH = (VS_VERSION_INFO_HEAD*)::LockResource( hgRSRC ) )
	){
		*pdwProductVersionMS = pVVIH->Value.dwProductVersionMS;
		*pdwProductVersionLS = pVVIH->Value.dwProductVersionLS;
	}
	return;

}




/* Windows�o�[�W�����̃`�F�b�N */
BOOL CheckWindowsVersion( const char* pszAppName )
{
	OSVERSIONINFO	osvi;
	memset( (void *)&osvi, 0, sizeof( osvi ) );
	osvi.dwOSVersionInfoSize = sizeof( osvi );
	if( ::GetVersionEx( &osvi ) ){
		if( osvi.dwMajorVersion < 4 ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, pszAppName,
				"���̃A�v���P�[�V���������s����ɂ́A\nWindows95�ȏ� �܂��� WindowsNT4.0�ȏ��OS���K�v�ł��B\n�A�v���P�[�V�������I�����܂��B"
			);
			return FALSE;
		}
	}else{
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, pszAppName,
			"OS�̃o�[�W�������擾�ł��܂���B\n�A�v���P�[�V�������I�����܂��B"
		);
		return FALSE;
	}
	return TRUE;
}

// From Here Jul. 5, 2001 shoji masami
/*! NT�v���b�g�t�H�[�����ǂ������ׂ�

	@retval TRUE NT platform
	@retval FALSE non-NT platform
*/
bool CheckWindowsVersionNT( void )
{
	OSVERSIONINFO osVer;
	osVer.dwOSVersionInfoSize = sizeof(osVer);
	GetVersionEx(&osVer);

	if (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
		return true;	// NT�n

	return false;		// ����ȊO
}
// To Here Jul. 5, 2001 shoji masami



/* �A�N�e�B�u�ɂ��� */
void ActivateFrameWindow( HWND hwnd )
{
	if( ::IsIconic( hwnd ) ){
		::ShowWindow( hwnd, SW_RESTORE );
	}
	::ShowWindow( hwnd, SW_SHOW );
	::SetForegroundWindow( hwnd );
	return;
}




/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
BOOL IsURL( const char* pszText, int nTextLen, int* pnUrlLen )
{
	BOOL			bFindURL;
//	int				nTextLen;
	int				nCharChars;
	int				i;
	const char*		ppszURLHEADARR[]={
		"mailto:",
		"http://",
		"https://",
		"ftp://",
		"file://",
		"gopher://",
		"news:",
		"nntp://",
		"telnet://",
		"wais://",
		"prospero://",
	};
	const int pnURLHEADLENARR[]={
		7,
		7,
		8,
		6,
		7,
		9,
		5,
		7,
		9,
		7,
		11
	};
	int			nURLHEADNUM = sizeof( ppszURLHEADARR ) / sizeof( ppszURLHEADARR[0] );
	const char*	pszHeahHead = "mhfgntwp";
	const int	nHeadHeadLen = 8;
	int			nURLHeadLen;
//	nTextLen = strlen( pszText );
	bFindURL = FALSE;
	for( i = 0 ; i < nHeadHeadLen; ++i ){
		if( pszText[0] == pszHeahHead[i] ){
			break;
		}
	}
	if( i < nHeadHeadLen ){
		for( i = 0; i < nURLHEADNUM; ++i ){
//			nURLHeadLen = strlen( ppszURLHEADARR[i] );
			nURLHeadLen = pnURLHEADLENARR[i];

			if( nTextLen >= nURLHeadLen
			 && 0 == memcmp( pszText, ppszURLHEADARR[i], nURLHeadLen ) ){
				bFindURL = TRUE;
				break;
			}
		}
	}
	if( bFindURL ){
		if( 0 == i ){
			/* ���[���A�h���X��? */
			/* ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ� */
			if( TRUE == IsMailAddress( &pszText[nURLHeadLen], strlen( &pszText[nURLHeadLen] ), pnUrlLen ) ){
				*pnUrlLen  = *pnUrlLen + nURLHeadLen;
				return TRUE;
			}else{
				return FALSE;
			}
		}

		for( i = nURLHeadLen; i < nTextLen; ++i ){
			nCharChars = CMemory::MemCharNext( pszText, nTextLen, &pszText[i] ) - &pszText[i];
			/* URL������̏I���ɒB������ */
			if( 0 == nCharChars ){
				break;
			}
			if( 2 == nCharChars ){
				break;
//				++i;
//				continue;
			}
			if( ' '  == pszText[i]
			 || '"'  == pszText[i]
			 || '\'' == pszText[i]
			 || '<'  == pszText[i]
			 || '>'  == pszText[i]
			 || '\r' == pszText[i]
			 || '\n' == pszText[i]
			 || '\t' == pszText[i]
			 || '\0' == pszText[i]
			 || ')'  == pszText[i]
			 || '}'  == pszText[i]		//Stonee, 2001/06/24  URL�̋�؂蔻����C��
			 || '{'  == pszText[i]		//Stonee, 2001/06/24
			){
				break;
			}
		}
		if( 0 == i - nURLHeadLen ){
			return FALSE;
		}
		*pnUrlLen = i;
		return TRUE;
	}
	/* ���[���A�h���X��? */
	/* ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ� */
	if( TRUE == IsMailAddress( pszText, strlen( pszText ), pnUrlLen ) ){
		return TRUE;
	}
	return FALSE;
}




/* ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ� */
BOOL IsMailAddress( const char* pszBuf, int nBufLen, int* pnAddressLenfth )
{
//	int		i;
	int		j;
//	int		wk_nBegin;
//	int		wk_nEnd;
//	int		nAtPos;
	int		nDotCount;
//	int		nAlphaCount;
	int		nBgn;
//	int		nLoop;
//	BOOL bDot = FALSE;


	j = 0;
	if( (pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
	 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
	 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
//	 || (pszBuf[j] == '.')
//	 || (pszBuf[j] == '-')
//	 || (pszBuf[j] == '_')
//	 || (pszBuf[j] == '=')
//	 || (pszBuf[j] == '%')
//	 || (pszBuf[j] == '$')
//	   )
	){
		j++;
	}else{
		return FALSE;
	}
	while( j < nBufLen - 2 &&
		(
		(pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
	 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
	 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
	 || (pszBuf[j] == '.')
	 || (pszBuf[j] == '-')
	 || (pszBuf[j] == '_')
//	 || (pszBuf[j] == '=')
//	 || (pszBuf[j] == '%')
//	 || (pszBuf[j] == '$')
		)
	){
		j++;
	}
	if( j == 0 || j >= nBufLen - 2  ){
		return FALSE;
	}
	if( '@' != pszBuf[j] ){
		return FALSE;
	}
//	nAtPos = j;
	j++;
	nDotCount = 0;
//	nAlphaCount = 0;


	while( 1 ){
		nBgn = j;
		while( j < nBufLen &&
			(
			(pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
		 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
		 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
//		 || (pszBuf[j] == '.')
		 || (pszBuf[j] == '-')
		 || (pszBuf[j] == '_')
		//	 || (pszBuf[j] == '=')
		//	 || (pszBuf[j] == '%')
		//	 || (pszBuf[j] == '$')
			)
		){
			j++;
		}
		if( 0 == j - nBgn ){
			return FALSE;
		}
		if( '.' != pszBuf[j] ){
			if( 0 == nDotCount ){
				return FALSE;
			}else{
				break;
			}
		}else{
			nDotCount++;
			j++;
		}
	}
	if( NULL != pnAddressLenfth ){
		*pnAddressLenfth = j;
	}
	return TRUE;
}




//@@@ 2001.02.17 Start by MIK
//#ifdef COMPILE_COLOR_DIGIT
/*
 * ���l�Ȃ璷����Ԃ��B
 * 10�i���̐����܂��͏����B16�i��(����)�B
 * ������   ���l(�F����)
 * ---------------------
 * 123      123
 * 0123     0123
 * 0xfedc   0xfedc
 * -123     -123
 * &H9a     &H9a     (�������\�[�X����#if��L���ɂ����Ƃ�)
 * -0x89a   0x89a
 * 0.5      0.5
 * 0.56.1   0.56 , 1 (�������\�[�X����#if��L���ɂ�����"0.56.1"�ɂȂ�)
 * .5       5        (�������\�[�X����#if��L���ɂ�����".5"�ɂȂ�)
 * -.5      5        (�������\�[�X����#if��L���ɂ�����"-.5"�ɂȂ�)
 * 123.     123
 * 0x567.8  0x567 , 8
 */
int IsNumber(const char* buf, int offset, int length)
{
	const char *p, *q;
	int i = 0;
	int d;

	/* �������̂��߃|�C���^���g��! */
	p = &buf[offset];
	//q = &buf[length];	/* �����ȊO�̂Ƃ��̍������̂��߂����ł͂��Ȃ� */

	if( *p >= '0' && *p <= '9' ){
		q = &buf[length];
		/* 8,10,16�i�� */
		p++;
		if( p < q ){
			if( *(p - 1) == '0' && *p == 'x' ){
				/* 16�i�� */
				p++;
				for( i = 2; p < q; p++, i++ ){
					if( (*p < '0' || *p > '9')
					&& (*p < 'A' || *p > 'F')
					&& (*p < 'a' || *p > 'f') ){
						break;
					}
				}
				/* "0x" �̂Ƃ��� "0" �����l�ɂȂ� */
				if( i == 2 ){
					return 1;
				}
			}else{
				/* 10�i���܂���8�i�� */
				for( i = 1, d = 0; p < q; p++, i++ ){
					if( *p < '0' || *p > '9' ){
						if( *p == '.' ){
							d++;
							if( d > 1 ) break;
							//if( d > 1 )	return 0;
						}else{
							break;
						}
					}
				}
				/* "." �ŏI���Ƃ��� "." �͓���Ȃ� */
				if( *(p - 1) == '.' ){
					i--;
				}
			}
		}else{
			return 1;
		}
	}else if( *p == '-' /* || *p == '+' */ ){
		q = &buf[length];
		/* �}�C�i�X��10�i�� */
		p++;
		if( p < q ){
			if( *p == '0' ){
				p++;
				if( p < q ){
					if( *p == 'x' ){
						p++;
						if( p < q ){						//***16�i���Ƀ}�C�i�X���Ȃ��Ƃ��͂�����L���ɁB
						//for( i = 3; p < q; p++, i++ ){	//***16�i���Ƀ}�C�i�X������Ƃ��͂�����L���ɁB
							if( (*p >= '0' && *p <= '9')
							||  (*p >= 'A' && *p <= 'F')
							||  (*p >= 'a' && *p <= 'f') ){
								//break;					//***16�i���Ƀ}�C�i�X������Ƃ��͂�����L���ɁB
								/* 16�i���Ȃ̂Ń}�C�i�X�����ł͂Ȃ� */
								return 0;					//***16�i���Ƀ}�C�i�X���Ȃ��Ƃ��͂�����L���ɁB
							}
						}
						//if( i == 3 ){						//***16�i���Ƀ}�C�i�X������Ƃ��͂�����L���ɁB
							/* "-0x" �̂��� "-0" �����l */
							return 2;
						//}									//***16�i���Ƀ}�C�i�X������Ƃ��͂�����L���ɁB
					}else{
						for( i = 2, d = 0; p < q; p++, i++ ){
							if( *p < '0' || *p > '9' ){
								if( *p == '.' ){
									d++;
									if( d > 1 ) break;
									//if( d > 1 ) return 0;
								}else{
									break;
								}
							}
						}
						/* "." �ŏI���Ƃ��� "." �͓���Ȃ� */
						if( *(p - 1) == '.' ){
							i--;
						}
					}
				}else{
					return 2;
				}
			}else{
				for( i = 1, d = 0; p < q; p++, i++ ){
					if( *p < '0' || *p > '9' ){
						if( *p == '.' ){
							d++;
							if( d > 1 || i == 1 ) break;	/* �������� -.5 �𐔒l�Ƃ��Ȃ� */
							//if( d > 1 ) break;			/* �������� -.5 �𐔒l�Ƃ��� */
							//if( i == 1 ) break;			/* -.5 �𐔒l�Ƃ��Ȃ� */
						}else{
							break;
						}
					}
				}
				if( i == 1 ){
					/* �}�C�i�X�����ł͂Ȃ����� */
					return 0;
				}else if( *(p - 1) == '.' ){
					if( i == 2 ){
						i = 0;
					}else{
						i--;
					}
				}
			}
		}else{
			return 0;
		}
//#if 0  /* �����_�n�܂�̐��� : ���ꂾ�� "0.5.1.6" ���S�����l�ɂȂ��Ă��܂��܂� */
	}else if( *p == '.' ){
		/* �����_�n�܂�̐��l */
		q = &buf[length];
		p++;
		for( i = 1, d = 1; p < q; p++, i++ ){
			if( *p < '0' || *p > '9' ){
				d++;
				if( d > 1 ) break;
				//if( d > 1 ) return 0;
				break;
			}
		}
		if( i == 1 ){
			return 0;
		}
//#endif
#if 0  /* VB��16�i�� */
	}else if( *p == '&' ){
		q = &buf[length];
		/* VB��16�i�� */
		p++;
		if( p < q ){
			if( *p == 'H' || *p == 'h' ){
				p++;
				for( i = 2; p < q; p++, i++ ){
					if( (*p < '0' || *p > '9')
					&&  (*p < 'A' || *p > 'F')
					&&  (*p < 'a' || *p > 'f') ){
						break;
					}
				}
				/* "&H" �̂Ƃ��͐��l�ł͂Ȃ� */
				if( i == 2 ){
					return 0;
				}
			}else{
				return 0;
			}
		}else{
			return 0;
		}
#endif
	}else{
		/* ���l�ł͂Ȃ� */
		return 0;
	}

	return i;
}
//#endif
//@@@ 2001.02.17 End by MIK




BOOL IsFilePath( const char* pLine, int* pnBgn, int* pnPathLen )
{
	int		i;
	int		j;
	int		nLineLen;
	char	szJumpToFile[1024];
	memset( szJumpToFile, 0, sizeof( szJumpToFile ) );

	nLineLen = strlen( pLine );

	for( i = 0; i < nLineLen; ++i ){
		if( ' ' != pLine[i] &&
			'\t' != pLine[i] &&
			'\"' != pLine[i]
		){
			break;
		}
	}
	if( i < nLineLen &&
		'#' == pLine[i]
	){
		for( ; i < nLineLen; ++i ){
			if( '<'  == pLine[i] ||
				'\"' == pLine[i]
			){
				++i;
				break;
			}
		}
	}
	if( i >= nLineLen ){
		goto can_not_tagjump;
	}
	*pnBgn = i;
	j = 0;
	for( ; i <= nLineLen && j + 1 < sizeof(szJumpToFile); ++i ){
		if( ( i == nLineLen    ||
			  pLine[i] == ' '  ||
			  pLine[i] == '('  ||
			  pLine[i] == '\r' ||
			  pLine[i] == '\n' ||
			  pLine[i] == '\0' ||
			  pLine[i] == '>'  ||
			  pLine[i] == '"'
			) &&
			0 < strlen( szJumpToFile )
		){
			if( -1 != _access( (const char *)szJumpToFile, 0 ) ){
				i--;
				break;
			}
		}
		if( pLine[i] == '\r'  ||
			pLine[i] == '\n' ){
			break;
		}
//	From Here Sept. 27, 2000 JEPRO �^�O�W�����v�ł��Ȃ��͈̂ȉ��̕�����1�o�C�g�R�[�h�Ō����Ƃ��݂̂Ƃ����B
//	(SJIS2�o�C�g�R�[�h��2�o�C�g�ڂɌ����ꍇ�̓p�X���g�p�֎~�����Ƃ͔F�����Ȃ��Ŗ�������悤�ɕύX)
//		if( /*pLine[i] == '/' ||*/
//			pLine[i] == '<' ||
//			pLine[i] == '>' ||
//			pLine[i] == '?' ||
//			pLine[i] == '"' ||
//			pLine[i] == '|' ||
//			pLine[i] == '*'
//		){
//			goto can_not_tagjump;
//		}
//
//		szJumpToFile[j] = pLine[i];
//		j++;
//	}
//  To Here comment out
//	From Here Sept. 27, 2000 JEPRO added
//			  Oct. 3, 2000 JEPRO corrected
		if( ( /*pLine[i] == '/' ||*/
			 pLine[i] == '<' ||	//	0x3C
			 pLine[i] == '>' ||	//	0x3E
			 pLine[i] == '?' ||	//	0x3F
			 pLine[i] == '"' ||	//	0x22
			 pLine[i] == '|' ||	//	0x7C
			 pLine[i] == '*'	//	0x2A
			) &&
			/* ��̕�����SJIS2�o�C�g�R�[�h��2�o�C�g�ڂłȂ����Ƃ��A1�O�̕�����SJIS2�o�C�g�R�[�h��1�o�C�g�ڂłȂ����ƂŔ��f���� */
			( i > 0 &&
				(
				 (unsigned char)pLine[i - 1] < (unsigned char)0x81 ||
				 ( (unsigned char)0x9F < (unsigned char)pLine[i - 1] && (unsigned char)pLine[i - 1] < (unsigned char)0xE0 ) ||
				   (unsigned char)0xEF < (unsigned char)pLine[i - 1]
				)
			) ){
			goto can_not_tagjump;
		}else{
		szJumpToFile[j] = pLine[i];
		j++;
		}
	}
//	To Here Sept. 27, 2000
//	if( i >= nLineLen ){
//		return FALSE;
//	}
	if( 0 < strlen( szJumpToFile ) &&
		-1 != _access( (const char *)szJumpToFile, 0 ) )
	{
	}else{
//#ifdef _DEBUG
//		m_cShareData.TraceOut( "�t�@�C�������݂��Ȃ��BszJumpToFile=[%s]\n", szJumpToFile );
//#endif
		goto can_not_tagjump;
	}


	*pnPathLen = strlen( szJumpToFile );
	return TRUE;
	can_not_tagjump:;
	return FALSE;

}




void GetLineColm( const char* pLine, int* pnJumpToLine, int* pnJumpToColm )
{
	int		i;
	int		j;
	int		nLineLen;
	char	szNumber[32];
	nLineLen = strlen( pLine );
	i = 0;
	for( ; i < nLineLen; ++i ){
		if( pLine[i] >= '0' &&
			pLine[i] <= '9' ){
			break;
		}
	}
	memset( szNumber, 0, sizeof( szNumber ) );
	if( i >= nLineLen ){
	}else{
		/* �s�ʒu ���s�P�ʍs�ԍ�(1�N�_)�̒��o */
		j = 0;
		for( ; i < nLineLen && j + 1 < sizeof( szNumber ); ){
			szNumber[j] = pLine[i];
			j++;
			++i;
			if( pLine[i] >= '0' &&
				pLine[i] <= '9' ){
				continue;
			}
			break;
		}
		*pnJumpToLine = atoi( szNumber );

		/* ���ʒu ���s�P�ʍs�擪����̃o�C�g��(1�N�_)�̒��o */
		if( i < nLineLen && pLine[i] == ',' ){
			memset( szNumber, 0, sizeof( szNumber ) );
			j = 0;
			++i;
			for( ; i < nLineLen && j + 1 < sizeof( szNumber ); ){
				szNumber[j] = pLine[i];
				j++;
				++i;
				if( pLine[i] >= '0' &&
					pLine[i] <= '9' ){
					continue;
				}
				break;
			}
			*pnJumpToColm = atoi( szNumber );
		}
	}
	return;
}




//	/* CRLF�ŋ�؂���u�s�v��Ԃ��BCRLF�͍s���ɉ����Ȃ� */
//	/* bLFisOK��LF�����ł����s�Ƃ݂Ȃ� */
//	const char* GetNextLine( const char* pText, int nTextLen, int* pnLineLen, int* pnBgn, BOOL* pbEOL, BOOL bLFisOK )
//	{
//		int		i;
//		int		nBgn;
//		nBgn = *pnBgn;
//		if( nBgn >= nTextLen ){
//			return NULL;
//		}
//		if( NULL != pbEOL ){
//			*pbEOL = TRUE;
//		}
//		for( i = *pnBgn; i < nTextLen; ++i ){
//			if( bLFisOK &&
//				'\n' == pText[i]
//			){
//				*pnBgn = i + 1;
//				break;
//			}
//			if( '\r' == pText[i]
//			 && i + 1 < nTextLen
//			 && '\n' == pText[i + 1]
//			){
//				*pnBgn = i + 2;
//				break;
//			}
//		}
//		if( i >= nTextLen ){
//			*pnBgn = i;
//			if( NULL != pbEOL ){
//				*pbEOL = FALSE;
//			}
//		}
//		*pnLineLen = i - nBgn;
//		return &pText[nBgn];
//	}
//




/* CR0LF0,CRLF,LFCR,LF,CR�ŋ�؂���u�s�v��Ԃ��B���s�R�[�h�͍s���ɉ����Ȃ� */
const char* GetNextLine(
	const char*		pData,
	int				nDataLen,
	int*			pnLineLen,
	int*			pnBgn,
	CEOL*			pcEol
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	//	May 15, 2000 genta
	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		/* ���s�R�[�h�������� */
		if( pData[i] == '\n' || pData[i] == '\r' ){
			/* �s�I�[�q�̎�ނ𒲂ׂ� */
			 pcEol->GetTypeFromString( &pData[i], nDataLen - i );
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}




/* �w�蒷�ȉ��̃e�L�X�g�ɐ؂蕪���� */
const char* GetNextLimitedLengthText( const char* pText, int nTextLen, int nLimitLen, int* pnLineLen, int* pnBgn )
{
	int		i;
	int		nBgn;
	int		nCharChars;
	nBgn = *pnBgn;
	if( nBgn >= nTextLen ){
		return NULL;
	}
	for( i = nBgn; i + 1 < nTextLen; ++i ){
		nCharChars = CMemory::MemCharNext( pText, nTextLen, &pText[i] ) - &pText[i];
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if( i + nCharChars - nBgn >= nLimitLen ){
			break;
		}
		i += ( nCharChars - 1 );
	}
	*pnBgn = i;
	*pnLineLen = i - nBgn;
	return &pText[nBgn];
}




/* �f�[�^���w��o�C�g���ȓ��ɐ؂�l�߂� */
int LimitStringLengthB( const char* pszData, int nDataLength, int nLimitLengthB, CMemory& cmemDes )
{
	int	i;
	int	nCharChars;
	int	nDesLen;
	nDesLen = 0;
	for( i = 0; i < nDataLength; ){
		nCharChars = CMemory::MemCharNext( pszData, nDataLength, &pszData[i] ) - &pszData[i];
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if( nDesLen + nCharChars > nLimitLengthB ){
			break;
		}
		nDesLen += nCharChars;
		i += nCharChars;
	}
	cmemDes.SetData( pszData, nDesLen );
	return nDesLen;
}




/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� "c:\\"���̃��[�g�͎�菜���Ȃ� */
void CutLastYenFromDirectoryPath( char* pszFolder )
{
	if( 3 == strlen( pszFolder )
	 && pszFolder[1] == ':'
	 && pszFolder[2] == '\\'
	){
		/* �h���C�u��:\ */
	}else{
		/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� */
		int	nFolderLen;
		int	nCharChars;
		nFolderLen = strlen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CMemory::MemCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && '\\' == pszFolder[nFolderLen - 1] ){
				pszFolder[nFolderLen - 1] = '\0';
			}
		}
	}
	return;
}




/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
void AddLastYenFromDirectoryPath( char* pszFolder )
{
	if( 3 == strlen( pszFolder )
	 && pszFolder[1] == ':'
	 && pszFolder[2] == '\\'
	){
		/* �h���C�u��:\ */
	}else{
		/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
		int	nFolderLen;
		int	nCharChars;
		nFolderLen = strlen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CMemory::MemCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && '\\' == pszFolder[nFolderLen - 1] ){
			}else{
				pszFolder[nFolderLen] = '\\';
				pszFolder[nFolderLen + 1] = '\0';
			}
		}
	}
	return;
}




/* �V���[�g�J�b�g(.lnk)�̉��� */
BOOL ResolveShortcutLink( HWND hwnd, LPCSTR lpszLinkFile, LPSTR lpszPath )
{
	BOOL			bRes;
	HRESULT			hRes;
	IShellLink*		pIShellLink;
	IPersistFile*	pIPersistFile;
	char			szGotPath[MAX_PATH];
	char			szDescription[MAX_PATH];
	WIN32_FIND_DATA	wfd;
	WCHAR			wsz[MAX_PATH];
	/* ������ */
	pIShellLink = NULL;
	pIPersistFile = NULL;
	*lpszPath = 0; // assume failure
	bRes = FALSE;
	szGotPath[0] = '\0';

	// COM���C�u�����̏�����
	hRes = ::CoInitialize( NULL );
	if( hRes == E_OUTOFMEMORY
	 || hRes == E_INVALIDARG
	 || hRes == E_UNEXPECTED
	){
		return FALSE;
	}

	// Get a pointer to the IShellLink interface.
	hRes = 0;
	if( SUCCEEDED( hRes = ::CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pIShellLink ) ) ){
		// Get a pointer to the IPersistFile interface.
		if( SUCCEEDED(hRes = pIShellLink->QueryInterface( IID_IPersistFile, (void**)&pIPersistFile ) ) ){
			// Ensure that the string is Unicode.
			MultiByteToWideChar( CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH );
			// Load the shortcut.
			if( SUCCEEDED(hRes = pIPersistFile->Load( wsz, STGM_READ ) ) ){
				// Resolve the link.
				if( SUCCEEDED( hRes = pIShellLink->Resolve(hwnd, SLR_ANY_MATCH ) ) ){
					// Get the path to the link target.
					if( SUCCEEDED( hRes = pIShellLink->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH ) ) ){
						// Get the description of the target.
						if( SUCCEEDED(hRes = pIShellLink->GetDescription(szDescription, MAX_PATH ) ) ){
							if( '\0' != szGotPath[0] ){
								/* ����I�� */
								lstrcpy( lpszPath, szGotPath );
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
	// COM���C�u�����̌�n������
	::CoUninitialize();
	return bRes;
}




/*
||	�������̃��[�U�[������\�ɂ���
||	�u���b�L���O�t�b�N(?)�i���b�Z�[�W�z��
*/
BOOL BlockingHook( HWND hwndDlgCancel )
{
		MSG		msg;
		BOOL	ret;
		ret = (BOOL)::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
		if( ret ){
			if ( msg.message == WM_QUIT ){
				return FALSE;
			}
			if( NULL != hwndDlgCancel && IsDialogMessage( hwndDlgCancel, &msg ) ){
			}else{
				::TranslateMessage( &msg );
				::DispatchMessage( &msg );
			}
		}
		return TRUE/*ret*/;
}




//! �@�\�ԍ��ɉ����ăw���v�g�s�b�N�ԍ���Ԃ�
/*!
	@author Stonee
	@date	2001/02/23
	@param nFuncID �@�\�ԍ�
	@return �w���v�g�s�b�N�ԍ��B�Y��ID�������ꍇ�ɂ�0��Ԃ��B

	���e��case���̗���B

*/
int FuncID_To_HelpContextID( int nFuncID )
{
	switch( nFuncID ){

	/* �t�@�C������n */
	case F_FILENEW:				return 25;			//�V�K�쐬
	case F_FILEOPEN:			return 15;			//�J��
	case F_FILESAVE:			return 20;			//�㏑���ۑ�
	case F_FILESAVEAS:			return 21;			//���O��t���ĕۑ�
	case F_FILECLOSE:			return 17;			//����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
	case F_FILECLOSE_OPEN:		return 119;			//���ĊJ��
	case F_FILE_REOPEN_SJIS:	return 156;			//SJIS�ŊJ������
	case F_FILE_REOPEN_JIS:		return 157;			//JIS�ŊJ������
	case F_FILE_REOPEN_EUC:		return 158;			//EUC�ŊJ������
	case F_FILE_REOPEN_UNICODE:	return 159;			//Unicode�ŊJ������
	case F_FILE_REOPEN_UTF8:	return 160;			//UTF-8�ŊJ������
	case F_FILE_REOPEN_UTF7:	return 161;			//UTF-7�ŊJ������
	case F_PRINT:				return 162;			//���				//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	case F_PRINT_PREVIEW:		return 120;			//����v���r���[
	case F_PRINT_PAGESETUP:		return 122;			//����y�[�W�ݒ�	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	case F_OPEN_HfromtoC:		return 192;			//������C/C++�w�b�_(�\�[�X)���J��	//Feb. 7, 2001 JEPRO �ǉ�
	case F_OPEN_HHPP:			return 24;			//������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	case F_OPEN_CCPP:			return 26;			//������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	case F_ACTIVATE_SQLPLUS:	return 132;			/* Oracle SQL*Plus���A�N�e�B�u�\�� */
	case F_PLSQL_COMPILE_ON_SQLPLUS:	return 27;	/* Oracle SQL*Plus�Ŏ��s */
	case F_BROWSE:				return 121;			//�u���E�Y
	case F_PROPERTY_FILE:		return 22;			/* �t�@�C���̃v���p�e�B */

//	case IDM_EXITALL:			return 28;			//�e�L�X�g�G�f�B�^�̑S�I��
	case F_EXITALL:			return 28;				//�T�N���G�f�B�^�̑S�I��	//Dec. 26, 2000 JEPRO F_�ɕύX


	/* �ҏW�n */
	case F_UNDO:						return 32;	//���ɖ߂�(Undo)
	case F_REDO:						return 33;	//��蒼��(Redo)
	case F_DELETE:						return 41;	//�폜
	case F_DELETE_BACK:					return 42;	//�J�[�\���O���폜
	case F_WordDeleteToStart:			return 166;	//�P��̍��[�܂ō폜
	case F_WordDeleteToEnd:				return 167;	//�P��̉E�[�܂ō폜
	case F_WordCut:						return 169;	//�P��؂���
	case F_WordDelete:					return 168;	//�P��폜
	case F_LineCutToStart:				return 172;	//�s���܂Ő؂���(���s�P��)
	case F_LineCutToEnd:				return 173;	//�s���܂Ő؂���(���s�P��)
	case F_LineDeleteToStart:			return 170;	//�s���܂ō폜(���s�P��)
	case F_LineDeleteToEnd:				return 171;	//�s���܂ō폜(���s�P��)
	case F_CUT_LINE:					return 174;	//�s�؂���(�܂�Ԃ��P��)
	case F_DELETE_LINE:					return 137;	//�s�폜(�܂�Ԃ��P��)
	case F_DUPLICATELINE:				return 43;	//�s�̓�d��(�܂�Ԃ��P��)
	case F_INDENT_TAB:					return 113;	//TAB�C���f���g
	case F_UNINDENT_TAB:				return 113;	//�tTAB�C���f���g
	case F_INDENT_SPACE:				return 114;	//SPACE�C���f���g
	case F_UNINDENT_SPACE:				return 114;	//�tSPACE�C���f���g
//	case ORDSREFERENCE:					return ;	//�P�ꃊ�t�@�����X


	/* �J�[�\���ړ��n */
//	case F_UP:				return ;	//�J�[�\����ړ�
//	case F_DOWN:			return ;	//�J�[�\�����ړ�
//	case F_LEFT:			return ;	//�J�[�\�����ړ�
//	case F_RIGHT:			return ;	//�J�[�\���E�ړ�
//	case F_UP2:				return ;	//�J�[�\����ړ�(�Q�s����)
//	case F_DOWN2:			return ;	//�J�[�\�����ړ�(�Q�s����)
//	case F_WORDLEFT:		return ;	//�P��̍��[�Ɉړ�
//	case F_WORDRIGHT:		return ;	//�P��̉E�[�Ɉړ�
//	case F_GOLINETOP:		return ;	//�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_GOLINEEND:		return ;	//�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_ROLLDOWN:		return ;	//�X�N���[���_�E��
//	case F_ROLLUP:			return ;	//�X�N���[���A�b�v
//	case F_HalfPageUp:		return ;	//���y�[�W�A�b�v	//Oct. 17, 2000 JEPRO �ȉ��u�P�y�[�W�_�E���v�܂Œǉ�
//	case F_HalfPageDown:	return ;	//���y�[�W�_�E��
//	case F_1PageUp:			return ;	//�P�y�[�W�A�b�v
//	case F_1PageDown:		return ;	//�P�y�[�W�_�E��
//	case F_GOFILETOP:		return ;	//�t�@�C���̐擪�Ɉړ�
//	case F_GOFILEEND:		return ;	//�t�@�C���̍Ō�Ɉړ�
//	case F_CURLINECENTER:	return ;	//�J�[�\���s���E�B���h�E������
//	case F_JUMPPREV:		return ;	//�ړ�����: �O��	//Oct. 17, 2000 JEPRO �ȉ��u�ړ�����:���ցv�܂Œǉ�
//	case F_JUMPNEXT:		return ;	//�ړ�����: ����
	case F_WndScrollDown:	return 198;	//�e�L�X�g���P�s���փX�N���[��	//Jul. 05, 2001 JEPRO �ǉ�
	case F_WndScrollUp:		return 199;	//�e�L�X�g���P�s��փX�N���[��	//Jul. 05, 2001 JEPRO �ǉ�


	/* �I���n */	//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v����(�I��)���ړ�
	case F_SELECTWORD:		return 45;	//���݈ʒu�̒P��I��
	case F_SELECTALL:		return 44;	//���ׂđI��
//	case F_BEGIN_SEL:		return ;	//�͈͑I���J�n
//	case F_UP_SEL:			return ;	//(�͈͑I��)�J�[�\����ړ�
//	case F_DOWN_SEL:		return ;	//(�͈͑I��)�J�[�\�����ړ�
//	case F_LEFT_SEL:		return ;	//(�͈͑I��)�J�[�\�����ړ�
//	case F_RIGHT_SEL:		return ;	//(�͈͑I��)�J�[�\���E�ړ�
//	case F_UP2_SEL:			return ;	//(�͈͑I��)�J�[�\����ړ�(�Q�s����)
//	case F_DOWN2_SEL:		return ;	//(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
//	case F_WORDLEFT_SEL:	return ;	//(�͈͑I��)�P��̍��[�Ɉړ�
//	case F_WORDRIGHT_SEL:	return ;	//(�͈͑I��)�P��̉E�[�Ɉړ�
//	case F_GOLINETOP_SEL:	return ;	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_GOLINEEND_SEL:	return ;	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_ROLLDOWN_SEL:	return ;	//(�͈͑I��)�X�N���[���_�E��
//	case F_ROLLUP_SEL:		return ;	//(�͈͑I��)�X�N���[���A�b�v
//	case F_HalfPageUp_Sel:	return ;	//(�͈͑I��)���y�[�W�A�b�v		//Oct. 17, 2000 JEPRO �ȉ��u�P�y�[�W�_�E���v�܂Œǉ�
//	case F_HalfPageDown_Sel:return ;	//(�͈͑I��)���y�[�W�_�E��
//	case F_1PageUp_Sel:		return ;	//(�͈͑I��)�P�y�[�W�A�b�v
//	case F_1PageDown_Sel:	return ;	//(�͈͑I��)�P�y�[�W�_�E��
//	case F_GOFILETOP_SEL:	return ;	//(�͈͑I��)�t�@�C���̐擪�Ɉړ�
//	case F_GOFILEEND_SEL:	return ;	//(�͈͑I��)�t�@�C���̍Ō�Ɉړ�


	/* ��`�I���n */	//Oct. 17, 2000 JEPRO (��`�I��)���V�݂��ꎟ�悱���ɂ���
//	case F_BOXSELALL:		return ;	//��`�ł��ׂđI��
//	case F_BEGIN_BOX:		return ;	//��`�͈͑I���J�n
/*
	case F_UP_BOX:			return ;	//(��`�I��)�J�[�\����ړ�	//Oct. 17, 2000 JEPRO �ȉ��u�t�@�C���̍Ō�Ɉړ��v�܂Œǉ�
	case F_DOWN_BOX:		return ;	//(��`�I��)�J�[�\�����ړ�
	case F_LEFT_BOX:		return ;	/(��`�I��)�J�[�\�����ړ�
	case F_RIGHT_BOX:		return ;	//(��`�I��)�J�[�\���E�ړ�
	case F_UP2_BOX:			return ;	//(��`�I��)�J�[�\����ړ�(�Q�s����)
	case F_DOWN2_BOX:		return ;	//(��`�I��)�J�[�\�����ړ�(�Q�s����)
	case F_WORDLEFT_BOX:	return ;	//(��`�I��)�P��̍��[�Ɉړ�
	case F_WORDRIGHT_BOX:	return ;	//(��`�I��)�P��̉E�[�Ɉړ�
	case F_GOLINETOP_BOX:	return ;	//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
	case F_GOLINEEND_BOX:	return ;	//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
	case F_HalfPageUp_Box:	return ;	//(��`�I��)���y�[�W�A�b�v
	case F_HalfPageDown_Box:return ;	//(��`�I��)���y�[�W�_�E��
	case F_1PageUp_Box:		return ;	//(��`�I��)�P�y�[�W�A�b�v
	case F_1PageDown_Box:	return ;	//(��`�I��)�P�y�[�W�_�E��
	case F_GOFILETOP_BOX:	return ;	//(��`�I��)�t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND_BOX:	return ;	//(��`�I��)�t�@�C���̍Ō�Ɉړ�
*/


	/* �N���b�v�{�[�h�n */
	case F_CUT:				return 34;			//�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	case F_COPY:			return 35;			//�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_COPY_CRLF:		return 163;			//CRLF���s�ŃR�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)	//Feb. 23, 2001 JEPRO �����Ă����̂Œǉ�
	case F_PASTE:			return 39;			//�\��t��(�N���b�v�{�[�h����\��t��)
	case F_PASTEBOX:		return 40;			//��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
//	case F_INSTEXT:			return ;			// �e�L�X�g��\��t��
	case F_COPYLINES:				return 36;	//�I��͈͓��S�s�R�s�[
	case F_COPYLINESASPASSAGE:		return 37;	//�I��͈͓��S�s���p���t���R�s�[
	case F_COPYLINESWITHLINENUMBER:	return 38;	//�I��͈͓��S�s�s�ԍ��t���R�s�[
	case F_COPYPATH:		return 56;			//���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	case F_COPYTAG:			return 175;			//���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[	//Oct. 17, 2000 JEPRO �ǉ�
//	case IDM_TEST_CREATEKEYBINDLIST:	return 57;	//�L�[���蓖�Ĉꗗ���N���b�v�{�[�h�փR�s�[	//Sept. 15, 2000 jepro�u���X�g�v���u�ꗗ�v�ɕύX
	case F_CREATEKEYBINDLIST:		return 57;	//�L�[���蓖�Ĉꗗ���N���b�v�{�[�h�փR�s�[	//Sept. 15, 2000 JEPRO �u���X�g�v���u�ꗗ�v�ɕύX�AIDM�QTEST��F�ɕύX���������܂������Ȃ��̂ŎE���Ă���	//Dec. 25, 2000 ����


	/* �}���n */
	case F_INS_DATE:				return 164;	// ���t�}��
	case F_INS_TIME:				return 165;	// �����}��


	/* �ϊ��n */
	case F_TOLOWER:					return 47;	//�p�啶�����p������
	case F_TOUPPER:					return 48;	//�p���������p�啶��
	case F_TOHANKAKU:				return 49;	/* �S�p�����p */
	case F_TOZENKAKUKATA:			return 50;	/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	case F_TOZENKAKUHIRA:			return 51;	/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	case F_HANKATATOZENKAKUKATA:	return 123;	/* ���p�J�^�J�i���S�p�J�^�J�i */
	case F_HANKATATOZENKAKUHIRA:	return 124;	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	case F_TOZENEI:					return 125;	/* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	case F_TABTOSPACE:				return 182;	/* TAB���� */
	case F_SPACETOTAB:				return 196;	/* �󔒁�TAB */ //#### Stonee, 2001/05/27	//Jul. 03, 2001 JEPRO �ԍ��C��
	case F_CODECNV_AUTO2SJIS:		return 178;	/* �������ʁ�SJIS�R�[�h�ϊ� */
	case F_CODECNV_EMAIL:			return 52;	//E-Mail(JIS��SJIS)�R�[�h�ϊ�
	case F_CODECNV_EUC2SJIS:		return 53;	//EUC��SJIS�R�[�h�ϊ�
	case F_CODECNV_UNICODE2SJIS:	return 179;	//Unicode��SJIS�R�[�h�ϊ�
	case F_CODECNV_UTF82SJIS:		return 142;	/* UTF-8��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UTF72SJIS:		return 143; /* UTF-7��SJIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2JIS:		return 117;	/* SJIS��JIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2EUC:		return 118;	/* SJIS��EUC�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF8:		return 180;	/* SJIS��UTF-8�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF7:		return 181;	/* SJIS��UTF-7�R�[�h�ϊ� */
	case F_BASE64DECODE:			return 54;	//Base64�f�R�[�h���ĕۑ�
	case F_UUDECODE:				return 55;	//uudecode���ĕۑ�	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX


	/* �����n */
	case F_SEARCH_DIALOG:		return 59;	//����(�P�ꌟ���_�C�A���O)
	case F_SEARCH_NEXT:			return 61;	//��������
	case F_SEARCH_PREV:			return 60;	//�O������
	case F_REPLACE:				return 62;	//�u��(�u���_�C�A���O)
	case F_SEARCH_CLEARMARK:	return 136;	//�����}�[�N�̃N���A
	case F_GREP:				return 67;	//Grep
	case F_JUMP:				return 63;	//�w��s�w�W�����v
	case F_OUTLINE:				return 64;	//�A�E�g���C�����
	case F_TAGJUMP:				return 65;	//�^�O�W�����v�@�\
	case F_TAGJUMPBACK:			return 66;	//�^�O�W�����v�o�b�N�@�\
	case F_COMPARE:				return 116;	//�t�@�C�����e��r
	case F_BRACKETPAIR:			return 183;	//�Ί��ʂ̌���	//Oct. 17, 2000 JEPRO �ǉ�


	/* ���[�h�؂�ւ��n */
	case F_CHGMOD_INS:		return 46;	//�}���^�㏑�����[�h�؂�ւ�
	case F_CANCEL_MODE:		return 194;	//�e�탂�[�h�̎�����


	/* �ݒ�n */
	case F_SHOWTOOLBAR:		return 69;	/* �c�[���o�[�̕\�� */
	case F_SHOWFUNCKEY:		return 70;	/* �t�@���N�V�����L�[�̕\�� */
	case F_SHOWSTATUSBAR:	return 134;	/* �X�e�[�^�X�o�[�̕\�� */
	case F_TYPE_LIST:		return 72;	/* �^�C�v�ʐݒ�ꗗ */
	case F_OPTION_TYPE:		return 73;	/* �^�C�v�ʐݒ� */
	case F_OPTION:			return 76;	/* ���ʐݒ� */
//From here �ݒ�_�C�A���O�p��help�g�s�b�NID��ǉ�  Stonee, 2001/05/18
	case F_TYPE_SCREEN:		return 74;	/* �^�C�v�ʐݒ�w�X�N���[���x */
	case F_TYPE_COLOR:		return 75;	/* �^�C�v�ʐݒ�w�J���[�x */
	case F_TYPE_HELPER:		return 197;	/* �^�C�v�ʐݒ�w�x���x */	//Jul. 03, 2001 JEPRO �ǉ�
	case F_OPTION_GENERAL:	return 81;	/* ���ʐݒ�w�S�ʁx */
	case F_OPTION_WINDOW:	return 146;	/* ���ʐݒ�w�E�B���h�E�x */
	case F_OPTION_EDIT:		return 144;	/* ���ʐݒ�w�ҏW�x */
	case F_OPTION_FILE:		return 83;	/* ���ʐݒ�w�t�@�C���x */
	case F_OPTION_BACKUP:	return 145;	/* ���ʐݒ�w�o�b�N�A�b�v�x */
	case F_OPTION_FORMAT:	return 82;	/* ���ʐݒ�w�����x */
	case F_OPTION_URL:		return 147;	/* ���ʐݒ�w�N���b�J�u��URL�x */
	case F_OPTION_GREP:		return 148;	/* ���ʐݒ�wGrep�x */
	case F_OPTION_KEYBIND:	return 84;	/* ���ʐݒ�w�L�[���蓖�āx */
	case F_OPTION_CUSTMENU:	return 87;	/* ���ʐݒ�w�J�X�^�����j���[�x */
	case F_OPTION_TOOLBAR:	return 85;	/* ���ʐݒ�w�c�[���o�[�x */
	case F_OPTION_KEYWORD:	return 86;	/* ���ʐݒ�w�����L�[���[�h�x */
	case F_OPTION_HELPER:	return 88;	/* ���ʐݒ�w�x���x */
//To here  Stonee, 2001/05/18
	case F_FONT:			return 71;	/* �t�H���g�ݒ� */
	case F_WRAPWINDOWWIDTH:	return 184;	/* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX	//Jul. 03, 2001 JEPRO �ԍ��C��


	/* �}�N�� */
	case F_RECKEYMACRO:		return 125;	/* �L�[�}�N���L�^�J�n�^�I�� */
	case F_SAVEKEYMACRO:	return 127;	/* �L�[�}�N���ۑ� */
	case F_LOADKEYMACRO:	return 128;	/* �L�[�}�N���ǂݍ��� */
	case F_EXECKEYMACRO:	return 126;	/* �L�[�}�N�����s */
//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//	case F_EXECCMMAND:		return 103; /* �O���R�}���h���s */
	case F_EXECCOMMAND:		return 103; /* �O���R�}���h���s */
//	To Here Sept. 20, 2000


	/* �J�X�^�����j���[ */
	case F_MENU_RBUTTON:	return 195	;	/* �E�N���b�N���j���[ */
	case F_CUSTMENU_1:	return 186;	/* �J�X�^�����j���[1 */
	case F_CUSTMENU_2:	return 186;	/* �J�X�^�����j���[2 */
	case F_CUSTMENU_3:	return 186;	/* �J�X�^�����j���[3 */
	case F_CUSTMENU_4:	return 186;	/* �J�X�^�����j���[4 */
	case F_CUSTMENU_5:	return 186;	/* �J�X�^�����j���[5 */
	case F_CUSTMENU_6:	return 186;	/* �J�X�^�����j���[6 */
	case F_CUSTMENU_7:	return 186;	/* �J�X�^�����j���[7 */
	case F_CUSTMENU_8:	return 186;	/* �J�X�^�����j���[8 */
	case F_CUSTMENU_9:	return 186;	/* �J�X�^�����j���[9 */
	case F_CUSTMENU_10:	return 186;	/* �J�X�^�����j���[10 */
	case F_CUSTMENU_11:	return 186;	/* �J�X�^�����j���[11 */
	case F_CUSTMENU_12:	return 186;	/* �J�X�^�����j���[12 */
	case F_CUSTMENU_13:	return 186;	/* �J�X�^�����j���[13 */
	case F_CUSTMENU_14:	return 186;	/* �J�X�^�����j���[14 */
	case F_CUSTMENU_15:	return 186;	/* �J�X�^�����j���[15 */
	case F_CUSTMENU_16:	return 186;	/* �J�X�^�����j���[16 */
	case F_CUSTMENU_17:	return 186;	/* �J�X�^�����j���[17 */
	case F_CUSTMENU_18:	return 186;	/* �J�X�^�����j���[18 */
	case F_CUSTMENU_19:	return 186;	/* �J�X�^�����j���[19 */
	case F_CUSTMENU_20:	return 186;	/* �J�X�^�����j���[20 */
	case F_CUSTMENU_21:	return 186;	/* �J�X�^�����j���[21 */
	case F_CUSTMENU_22:	return 186;	/* �J�X�^�����j���[22 */
	case F_CUSTMENU_23:	return 186;	/* �J�X�^�����j���[23 */
	case F_CUSTMENU_24:	return 186;	/* �J�X�^�����j���[24 */


	/* �E�B���h�E�n */
	case F_SPLIT_V:			return 93;	//�㉺�ɕ���	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	case F_SPLIT_H:			return 94;	//���E�ɕ���	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	case F_SPLIT_VH:		return 95;	//�c���ɕ���	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	case F_WINCLOSE:		return 18;	//�E�B���h�E�����
	case F_WIN_CLOSEALL:	return 19;	//���ׂẴE�B���h�E�����	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
	case F_NEXTWINDOW:		return 92;	//���̃E�B���h�E
	case F_PREVWINDOW:		return 91;	//�O�̃E�B���h�E
	case F_CASCADE:			return 138;	//�d�˂ĕ\��
	case F_TILE_V:			return 140;	//�㉺�ɕ��ׂĕ\��
	case F_TILE_H:			return 139;	//���E�ɕ��ׂĕ\��
	case F_MAXIMIZE_V:		return 141;	//�c�����ɍő剻
	case F_MAXIMIZE_H:		return 98;	//�������ɍő剻	//2001.02.10 by MIK
	case F_MINIMIZE_ALL:	return 96;	//���ׂčŏ���	//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
	case F_REDRAW:			return 187;	//�ĕ`��
	case F_WIN_OUTPUT:		return 188;	//�A�E�g�v�b�g�E�B���h�E�\��


	/* �x�� */
	case F_HOKAN:			return 111;	/* ���͕⊮�@�\ */
//Sept. 15, 2000��Nov. 25, 2000 JEPRO	//�V���[�g�J�b�g�L�[�����܂������Ȃ��̂ŎE���Ă���������2�s���C���E����
//	case IDM_HELP_CONTENTS:	return 100;	//�w���v�ڎ�
	case F_HELP_CONTENTS:	return 100;	//�w���v�ڎ�			//Nov. 25, 2000 JEPRO
//	case IDM_HELP_SEARCH:	return 101;	//�w���v�g�s�b�N�̌���
	case F_HELP_SEARCH:		return 101;	//�w���v�L�[���[�h����	//Nov. 25, 2000 JEPRO�u�g�s�b�N�́v���u�L�[���[�h�v�ɕύX
	case F_MENU_ALLFUNC:	return 189;	/* �R�}���h�ꗗ */
	case F_EXTHELP1:		return 190;	/* �O���w���v�P */
	case F_EXTHTMLHELP:		return 191;	/* �O��HTML�w���v */
//	case IDM_ABOUT:			return 102;	//�o�[�W�������
	case F_ABOUT:			return 102;	//�o�[�W�������	//Dec. 24, 2000 JEPRO F_�ɕύX


	/* ���̑� */
//	case F_SENDMAIL:		return ;	/* ���[�����M */

	default:
		return 0;
	}
}

//	From Here Jun. 26, 2001 genta
/*!
	�^����ꂽ���K�\�����C�u�����̏��������s���D
	���b�Z�[�W�t���O��ON�ŏ������Ɏ��s�����Ƃ��̓��b�Z�[�W��\������D

	@param hWnd [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h���B
			�o�[�W�����ԍ��̐ݒ肪�s�v�ł����NULL�B
	@param pRegexp [in] �`�F�b�N�ɗ��p����CBregexp�N���X�ւ̎Q��
	@param bShowMessage [in] ���������s���ɃG���[���b�Z�[�W���o���t���O

	@retval true ����������
	@retval false �������Ɏ��s
*/
bool InitRegexp( HWND hWnd, CBregexp& rRegexp, bool bShowMessage )
{
	if( !rRegexp.Init() ){
		if( bShowMessage ){
			::MessageBeep( MB_ICONEXCLAMATION );
			::MessageBox( hWnd, "BREGEXP.DLL��������܂���B\r\n"
				"���K�\���𗘗p����ɂ�BREGEXP.DLL���K�v�ł��B\r\n"
				"������@�̓w���v���Q�Ƃ��Ă��������B",
				"���", MB_OK | MB_ICONEXCLAMATION );
		}
		return false;
	}
	return true;
}

/*!
	���K�\�����C�u�����̑��݂��m�F���A����΃o�[�W���������w��R���|�[�l���g�ɃZ�b�g����B
	���s�����ꍇ�ɂ͋󕶎�����Z�b�g����B

	@param hWnd [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h���B
			�o�[�W�����ԍ��̐ݒ肪�s�v�ł����NULL�B
	@param nCmpId [in] �o�[�W�����������ݒ肷��R���|�[�l���gID
	@param bShowMessage [in] ���������s���ɃG���[���b�Z�[�W���o���t���O

	@retval true �o�[�W�����ԍ��̐ݒ�ɐ���
	@retval false ���K�\�����C�u�����̏������Ɏ��s
*/
bool CheckRegexpVersion( HWND hWnd, int nCmpId, bool bShowMessage )
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		if( hWnd != NULL ){
			::SetDlgItemText( hWnd, nCmpId, " ");
		}
		return false;
	}
	if( hWnd != NULL ){
		::SetDlgItemText( hWnd, nCmpId, cRegexp.GetVersion() );
	}
	return true;
}

/*!
	���K�\�����K���ɏ]���Ă��邩���`�F�b�N����B

	@param szPattern [in] �`�F�b�N���鐳�K�\��
	@param hWnd [in] ���b�Z�[�W�{�b�N�X�̐e�E�B���h�E
	@param bShowMessage [in] ���������s���ɃG���[���b�Z�[�W���o���t���O

	@retval true ���K�\���͋K���ʂ�
	@retval false ���@�Ɍ�肪����B�܂��́A���C�u�������g�p�ł��Ȃ��B
*/
bool CheckRegexpSyntax( const char* szPattern, HWND hWnd, bool bShowMessage )
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		return false;
	}
	if( !cRegexp.Compile( szPattern ) ){
		if( bShowMessage ){
			::MessageBox( hWnd, cRegexp.GetLastMessage(),
				"���K�\���G���[", MB_OK | MB_ICONEXCLAMATION );
		}
		return false;
	}
	return true;
}
//	To Here Jun. 26, 2001 genta


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

	@param hWnd [in] �Ăяo�����E�B���h�E�̃E�B���h�E�n���h��
	@param szFile [in] HTML Help�̃t�@�C�����B
				�s�����ɑ����ăE�B���h�E�^�C�v�����w��\�B
	@param uCmd [in] HTML Help �ɓn���R�}���h
	@param data [in] �R�}���h�ɉ������f�[�^
	@param msgflag [in] �G���[���b�Z�[�W��\�����邩�B�ȗ�����true�B

	@return �J�����w���v�E�B���h�E�̃E�B���h�E�n���h���B�J���Ȃ������Ƃ���NULL�B
*/

HWND OpenHtmlHelp( HWND hWnd, LPCSTR szFile, UINT uCmd, DWORD data, bool msgflag )
{
	if( g_cHtmlHelp.Init() ){
		return g_cHtmlHelp.HtmlHelp( hWnd, szFile, uCmd, data );
	}
	if( msgflag ){
		::MessageBox( hWnd, "HHCTRL.OCX��������܂���B\r\n"
			"HTML�w���v�𗘗p����ɂ�HHCTRL.OCX���K�v�ł��B\r\n",
			"���", MB_OK | MB_ICONEXCLAMATION );
	}
	return NULL;
}


//	To Here Jun. 26, 2001 genta


/*[EOF]*/
