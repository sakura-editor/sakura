//	$Id$
//////////////
// etc_uty.cpp
//	Copyright (C) 1998-2000, Norio Nakatani

//#include <stdio.h>
#include <io.h>
#include "etc_uty.h"
#include "debug.h"
#include "CMemory.h"


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
//		MYTRACE( "BFFM_INITIALIZED (char*)lpData = [%s]\n", (char*)lpData);
		::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData );
		break;
	case BFFM_SELCHANGED:
//		MYTRACE( "BFFM_SELCHANGED\n");
		break;
	}
	return 0;

}




/* �t�H���_�I���_�C�A���O */
BOOL SelectDir(HWND hWnd, const char* pszTitle, const char* pszInitFolder, char* strFolderName)
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
	ULONG			chEaten;  //������̃T�C�Y���󂯎��܂��B
	ULONG			dwAttributes;     //�������󂯎��܂��B
	HRESULT			hRes;
	if( '\0' == pszPath[0] ){
		return NULL;
	}
	if( ::SHGetDesktopFolder( &pDesktopFolder ) != NOERROR ){
		return NULL;
	}
	//�@��������Ȃ��ƃC���^�[�t�F�C�X�̓_���Ȃ̂ł��B
	::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszPath, -1, ochPath, MAX_PATH );
	//�@���ۂ�ITEMIDLIST���擾���܂��B
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
	//�@IMalloc�C���^�[�t�F�C�X�ւ̃|�C���^���擾���܂��B
	if( NOERROR != ::SHGetMalloc( &pMalloc ) ){
		return FALSE;
	}
	//�@�A�C�e���h�c���X�g�������[�X���܂��B
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
	else {
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
//-	while(1){
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
//-			if( szPathSrc[1] == ':'	){
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
//-		::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, "��҂ɋ����ė~�����G���[",
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
     �@Win16 �̎��́AGetFreeSystemResources �Ƃ����֐�������܂����B�������AWin32 �ł͂�
     ��܂���B�T���N����邾�� DLL ����邾�͓̂�����܂��B�ȒP�ȕ��@��������܂��B
     �@���g���� Windows95 �� [�A�N�Z�T��]-[�V�X�e���c�[��] �Ƀ��\�[�X���[�^������̂Ȃ�A
     c:\windows\system\rsrc32.dll ������͂��ł��B����́A���\�[�X���[�^�Ƃ��� Win32 �A�v
     �����AWin16 �� GetFreeSystemResources �֐����ĂԈׂ� DLL �ł��B������g���܂��傤�B
*/
BOOL GetSystemResources(
	int*	pnSystemResources,
	int*	pnUserResources,
	int*	pnGDIResources
)
{
	#define GFSR_SYSTEMRESOURCES 0x0000
	#define GFSR_GDIRESOURCES 0x0001
	#define GFSR_USERRESOURCES 0x0002
	HINSTANCE	hlib;
	int (CALLBACK *GetFreeSystemResources)( int );

	hlib = ::LoadLibrary( "RSRC32.dll" );
	if( (int)hlib > 32 ) {
		GetFreeSystemResources = (int (CALLBACK *)( int ))GetProcAddress(
			hlib,
			"_MyGetFreeSystemResources32@4"
		);
		if( GetFreeSystemResources != NULL ) {
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
	if( GetSystemResources(	&nSystemResources, &nUserResources,	&nGDIResources ) ){
//		MYTRACE( "nSystemResources=%d\n", nSystemResources  );
//		MYTRACE( "nUserResources=%d\n", nUserResources  );
//		MYTRACE( "nGDIResources=%d\n", nGDIResources  );
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
    WORD  wLength;
    WORD  wValueLength;
    WORD  bText;
	WCHAR szKey[16];
	VS_FIXEDFILEINFO Value;
};

/* ���\�[�X���琻�i�o�[�W�����̎擾 */
void GetAppVersionInfo( 
	HINSTANCE	hInstance , 
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
	OSVERSIONINFO			osvi;
	memset( (void *)&osvi, 0, sizeof( osvi ) );
	osvi.dwOSVersionInfoSize = sizeof( osvi );
	if( ::GetVersionEx( &osvi ) ){
		if( osvi.dwMajorVersion < 4 ){
			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, pszAppName,
				"���̃A�v���P�[�V���������s����ɂ́A\nWindows95�ȏ� �܂��� WindowsNT4.0�ȏ��OS���K�v�ł��B\n�A�v���P�[�V�������I�����܂��B"
			);
			return FALSE;
		}
	}else{
		::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, pszAppName,
			"OS�̃o�[�W�������擾�ł��܂���B\n�A�v���P�[�V�������I�����܂��B"
		);
		return FALSE;
	}
	return TRUE;
}


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
BOOL IsURL( const char*	pszText, int nTextLen, int* pnUrlLen )
{
	BOOL			bFindURL;
//	int				nTextLen;
	int				nCharChars;
	int				i;
	const char*	ppszURLHEADARR[]={
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
	int	nURLHEADNUM = sizeof( ppszURLHEADARR ) / sizeof( ppszURLHEADARR[0] );
	const char*	pszHeahHead = "mhfgntwp";
	const int nHeadHeadLen = 8;
	int	nURLHeadLen;
//	nTextLen = strlen( pszText );
	bFindURL = FALSE;
	for( i = 0 ;i < nHeadHeadLen; ++i ){
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
	

	while(1){
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







BOOL IsFilePath( const char* pLine, int* pnBgn, int* pnPathLen )
{
	int		i;
	int		j;
	int		nLineLen;
	char	szJumpToFile[1024];
	memset( szJumpToFile, 0, sizeof(szJumpToFile) );

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
			  pLine[i] == '\r'   ||
			  pLine[i] == '\n'   ||
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
	memset( szNumber, 0, sizeof(szNumber) );
	if( i >= nLineLen ){
	}else{
		/* �s�ʒu ���s�P�ʍs�ԍ�(1�N�_)�̒��o */
		j = 0;
		for( ; i < nLineLen && j + 1 < sizeof(szNumber); ){
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
			memset( szNumber, 0, sizeof(szNumber) );
			j = 0;
			++i;
			for( ; i < nLineLen && j + 1 < sizeof(szNumber); ){
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
//	const char* GetNextLine( const char* pText, int nTextLen, int* pnLineLen, int* pnBgn, BOOL* pbEOL, BOOL bLFisOK   )
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


/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� "c:\\"���̃��[�g�͎�菜���Ȃ�*/
void CutLastYenFromDirectoryPath( char*	pszFolder )
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
void AddLastYenFromDirectoryPath( char*	pszFolder )
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
BOOL ResolveShortcutLink(HWND hwnd, LPCSTR lpszLinkFile, LPSTR lpszPath)
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
	if( SUCCEEDED(hRes = ::CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pIShellLink )) ){
		// Get a pointer to the IPersistFile interface.
		if( SUCCEEDED(hRes = pIShellLink->QueryInterface( IID_IPersistFile, (void**)&pIPersistFile )) ){
			// Ensure that the string is Unicode.
			MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH );
			// Load the shortcut.
			if( SUCCEEDED(hRes = pIPersistFile->Load(wsz, STGM_READ)) ){
				// Resolve the link.
				if( SUCCEEDED(hRes = pIShellLink->Resolve(hwnd, SLR_ANY_MATCH)) ){
					// Get the path to the link target.
					if( SUCCEEDED(hRes = pIShellLink->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH )) ){
						// Get the description of the target.
						if( SUCCEEDED(hRes = pIShellLink->GetDescription(szDescription, MAX_PATH)) ){
							if( '\0' != szGotPath[0] ){
								/* ����I�� */
								lstrcpy(lpszPath, szGotPath);
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
||   �������̃��[�U�[������\�ɂ���
||�@�u���b�L���O�t�b�N(?)�i���b�Z�[�W�z���j
*/
BOOL BlockingHook( HWND hwndDlgCancel )
{
        MSG msg;
        BOOL ret;
        ret = (BOOL)::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if(ret){
                if ( msg.message == WM_QUIT ){
                        return FALSE;
                }
                if( NULL != hwndDlgCancel && IsDialogMessage( hwndDlgCancel, &msg) ){
                }else{
                        ::TranslateMessage(&msg);
                        ::DispatchMessage(&msg);
                }
        }
        return TRUE/*ret*/;
}



/* [EOF] */

