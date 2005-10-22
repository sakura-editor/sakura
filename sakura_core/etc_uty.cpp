//	$Id$
/*!	@file
	@brief ���ʊ֐��Q

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, shoji masami, stonee, MIK, YAZAKI
	Copyright (C) 2002, genta, aroka, hor, MIK, �S
	Copyright (C) 2003, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//	Sep. 10, 2005 genta GetLongPathName�̃G�~�����[�V�����֐��̎��̐����̂���
#define COMPILE_NEWAPIS_STUBS

#include <io.h>
#include <memory.h>		// Apr. 03, 2003 genta
#include "etc_uty.h"
#include "debug.h"
#include "CMemory.h"
#include "funccode.h"	//Stonee, 2001/02/23

#include "WINNETWK.H"	//Stonee, 2001/12/21
#include "sakura.hh"	//YAZAKI, 2001/12/11
#include "CEol.h"// 2002/2/3 aroka
#include "CBregexp.h"// 2002/2/3 aroka
#include "COsVersionInfo.h"
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

#include "CShareData.h"
#include "CMRU.h"
#include "CMRUFolder.h"
#include "CMultiMonitor.h"	//	2004.05.01 genta
#include "Keycode.h"// novice 2004/10/10

//	CShareData�ֈړ�
/* ���t���t�H�[�}�b�g */
//const char* MyGetDateFormat( char* pszDest, int nDestLen, int nDateFormatType, const char* pszDateFormat )

/* �������t�H�[�}�b�g */
//const char* MyGetTimeFormat( char* pszDest, int nDestLen, int nTimeFormatType, const char* pszTimeFormat )

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

/*!
	�󔒂��܂ރt�@�C�������l�������g�[�N���̕���
	
	�擪�ɂ���A��������؂蕶���͖�������D
	
	@param pBuffer [in] ������o�b�t�@(�I�[�����邱��)
	@param nLen [in] ������̒���
	@param pnOffset [in/out] �I�t�Z�b�g
	@param pDelimiter [in] ��؂蕶��
	@return �g�[�N��

	@date 2004.02.15 �݂� �œK��
*/
TCHAR* my_strtok( TCHAR* pBuffer, int nLen, int* pnOffset, const TCHAR* pDelimiter )
{
	int i = *pnOffset;
	TCHAR* p;

	do {
		bool bFlag = false;	//�_�u���R�[�e�[�V�����̒����H
		if( i >= nLen ) return NULL;
		p = &pBuffer[i];
		for( ; i < nLen; i++ )
		{
			if( pBuffer[i] == _T('"') ) bFlag = ! bFlag;
			if( ! bFlag )
			{
				if( _tcschr( pDelimiter, pBuffer[i] ) )
				{
					pBuffer[i++] = _T('\0');
					break;
				}
			}
		}
		*pnOffset = i;
	} while( ! *p );	//��̃g�[�N���Ȃ玟��T��
	return p;
}




/*! �w���v�t�@�C���̃t���p�X��Ԃ�
 
    @param pszHelpFile [in] �p�X���i�[����o�b�t�@
    @param nMaxLen [in] �o�b�t�@�Ɋi�[�\�ȕ������F�f�t�H���g�l��_MAX_PATH
    
    @return �p�X���i�[�����o�b�t�@�̃|�C���^
 
    @note ���s�t�@�C���Ɠ����ʒu�� sakura.hlp �t�@�C����Ԃ��B
        �p�X�� UNC �̂Ƃ��� _MAX_PATH �Ɏ��܂�Ȃ��\��������B
        
 
    @date 2002/01/19 aroka �GnMaxLen �����ǉ�
*/
char* GetHelpFilePath( char* pszHelpFile, unsigned int nMaxLen )
{
//	int		i;
	unsigned long	lPathLen;
	char	szHelpFile[_MAX_PATH + 1];
//	int		nCharChars;
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	/* �w���v�t�@�C���̃t�@�C���p�X */
	lPathLen = ::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szHelpFile, sizeof(szHelpFile)
	);
	if( lPathLen > nMaxLen ){
		*pszHelpFile = '\0';
		return pszHelpFile;
	}

	_splitpath( szHelpFile, szDrive, szDir, NULL, NULL );
	if( strlen(szDrive) + strlen(szDir) + strlen("sakura.hlp") > nMaxLen ){
		*pszHelpFile = '\0';
		return pszHelpFile;
	}
	strcpy( szHelpFile, szDrive );
	strcat( szHelpFile, szDir );
	strcat( szHelpFile, "sakura.hlp" );
	strncpy( pszHelpFile, szHelpFile, nMaxLen );
	return pszHelpFile;
}

/*! �����O�t�@�C�������擾���� 

	@param[in] pszFilePathSrc �ϊ����p�X��
	@param[out] pszFilePathDes ���ʏ������ݐ� (����MAX_PATH�̗̈悪�K�v)

	@date Oct. 2, 2005 genta GetFilePath API���g���ď�������
	@date Oct. 4, 2005 genta ���΃p�X����΃p�X�ɒ�����Ȃ�����
	@date Oct. 5, 2005 Moca  ���΃p�X���΃p�X�ɕϊ�����悤��
*/
BOOL GetLongFileName( const char* pszFilePathSrc, char* pszFilePathDes )
{
	TCHAR* name;
	TCHAR szBuf[_MAX_PATH + 1];
	int len = ::GetFullPathName( pszFilePathSrc, _MAX_PATH, szBuf, &name );
	if( len <= 0 || _MAX_PATH <= len ){
		len = ::GetLongPathName( pszFilePathSrc, pszFilePathDes, _MAX_PATH );
		if( len <= 0 || _MAX_PATH < len ){
			return FALSE;
		}
		return TRUE;
	}
	len = ::GetLongPathName( szBuf, pszFilePathDes, _MAX_PATH );
	if( len <= 0 || _MAX_PATH < len ){
		lstrcpy( pszFilePathDes, szBuf );
	}
	return TRUE;
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




/* �A�N�e�B�u�ɂ��� */
void ActivateFrameWindow( HWND hwnd )
{
	if( ::IsIconic( hwnd ) ){
		::ShowWindow( hwnd, SW_RESTORE );
	}
	else if ( ::IsZoomed( hwnd ) ){
		::ShowWindow( hwnd, SW_MAXIMIZE );
	}
	else {
		::ShowWindow( hwnd, SW_SHOW );
	}
	::SetForegroundWindow( hwnd );
	::BringWindowToTop( hwnd );
	return;
}




//@@@ 2002.01.24 Start by MIK
/*!
	������URL���ǂ�������������B
	
	@param pszLine [in] ������
	@param nLineLen [in] ������̒���
	@param pnMatchLen [out] URL�̒���
	
	@retval TRUE URL�ł���
	@retval FALSE URL�łȂ�
	
	@note �֐����ɒ�`�����e�[�u���͕K�� static const �錾�ɂ��邱��(���\�ɉe�����܂�)�B
		url_char �̒l�� url_table �̔z��ԍ�+1 �ɂȂ��Ă��܂��B
		�V���� URL ��ǉ�����ꍇ�� #define �l���C�����Ă��������B
		url_table �͓��������A���t�@�x�b�g���ɂȂ�悤�ɕ��ׂĂ��������B
*/
BOOL IsURL( const char *pszLine, int nLineLen, int *pnMatchLen )
{
	struct _url_table_t {
		char	name[12];
		int		length;
		bool	is_mail;
	};
	static const struct _url_table_t	url_table[] = {
		/* �A���t�@�x�b�g�� */
		"file://",		7,	false, /* 1 */
		"ftp://",		6,	false, /* 2 */
		"gopher://",	9,	false, /* 3 */
		"http://",		7,	false, /* 4 */
		"https://",		8,	false, /* 5 */
		"mailto:",		7,	true,  /* 6 */
		"news:",		5,	false, /* 7 */
		"nntp://",		7,	false, /* 8 */
		"prospero://",	11,	false, /* 9 */
		"telnet://",	9,	false, /* 10 */
		"tp://",		5,	false, /* 11 */	//2004.02.02
		"ttp://",		6,	false, /* 12 */	//2004.02.02
		"wais://",		7,	false, /* 13 */
		"{",			0,	false  /* 14 */  /* '{' is 'z'+1 : terminate */
	};

/* �e�[�u���̕ێ琫�����߂邽�߂̒�` */
	const char urF = 1;
	const char urG = 3;
	const char urH = 4;
	const char urM = 6;
	const char urN = 7;
	const char urP = 9;
	const char urT = 10;
	const char urW = 13;	//2004.02.02

	static const char	url_char[] = {
	  /* +0  +1  +2  +3  +4  +5  +6  +7  +8  +9  +A  +B  +C  +D  +E  +F */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +00: */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +10: */
		  0, -1,  0, -1, -1, -1, -1,  0,  0,  0,  0, -1, -1, -1, -1, -1,	/* +20: " !"#$%&'()*+,-./" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0, -1,	/* +30: "0123456789:;<=>?" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* +40: "@ABCDEFGHIJKLMNO" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0,  0, -1,	/* +50: "PQRSTUVWXYZ[\]^_" */
		  0, -1, -1, -1, -1, -1,urF,urG,urH, -1, -1, -1, -1,urM,urN, -1,	/* +60: "`abcdefghijklmno" */
		urP, -1, -1, -1,urT, -1, -1,urW, -1, -1, -1,  0,  0,  0, -1,  0,	/* +70: "pqrstuvwxyz{|}~ " */
		/* ����128�o�C�g�]���ɂ����if����2�ӏ��폜�ł��� */
		/* 0    : not url char
		 * -1   : url char
		 * other: url head char --> url_table array number + 1
		 */
	};

	const unsigned char	*p = (const unsigned char*)pszLine;
	const struct _url_table_t	*urlp;
	int	i;

	if( *p & 0x80 ) return FALSE;	/* 2�o�C�g���� */
	if( 0 < url_char[*p] ){	/* URL�J�n���� */
		for(urlp = &url_table[url_char[*p]-1]; urlp->name[0] == *p; urlp++){	/* URL�e�[�u����T�� */
			if( (urlp->length <= nLineLen) && (memcmp(urlp->name, pszLine, urlp->length) == 0) ){	/* URL�w�b�_�͈�v���� */
				p += urlp->length;	/* URL�w�b�_�����X�L�b�v���� */
				if( urlp->is_mail ){	/* ���[����p�̉�͂� */
					if( IsMailAddress((const char*)p, nLineLen - urlp->length, pnMatchLen) ){
						*pnMatchLen = *pnMatchLen + urlp->length;
						return TRUE;
					}
					return FALSE;
				}
				for(i = urlp->length; i < nLineLen; i++, p++){	/* �ʏ�̉�͂� */
					if( (*p & 0x80) || (!(url_char[*p])) ) break;	/* �I�[�ɒB���� */
				}
				if( i == urlp->length ) return FALSE;	/* URL�w�b�_���� */
				*pnMatchLen = i;
				return TRUE;
			}
		}
	}
	return IsMailAddress(pszLine, nLineLen, pnMatchLen);
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




//@@@ 2001.11.07 Start by MIK
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
/*
 * ���p���l
 *   1, 1.2, 1.2.3, .1, 0xabc, 1L, 1F, 1.2f, 0x1L, 0x2F, -.1, -1, 1e2, 1.2e+3, 1.2e-3, -1e0
 *   10�i��, 16�i��, LF�ڔ���, ���������_��, ������
 *   IP�A�h���X�̃h�b�g�A��(�{���͐��l����Ȃ��񂾂��)
 */
int IsNumber(const char *buf, int offset, int length)
{
	register const char *p, *q;
	register int i = 0;
	register int d = 0;
	register int f = 0;

	p = &buf[offset];
	q = &buf[length];

	if( *p == '0' )  /* 10�i��,C��16�i�� */
	{
		p++; i++;
		if( ( p < q ) && ( *p == 'x' ) )  /* C��16�i�� */
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= '0' && *p <= '9' )
				 || ( *p >= 'A' && *p <= 'F' )
				 || ( *p >= 'a' && *p <= 'f' ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "0x" �Ȃ� "0" ���������l */
			if( i == 2 ) return 1;
			
			/* �ڔ��� */
			if( p < q )
			{
				if( *p == 'L' || *p == 'l' || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p >= '0' && *p <= '9' )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( *p == '.' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == '.' ) break;  /* "." ���A���Ȃ璆�f */
						}
					}
					else if( *p == 'E' || *p == 'e' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == '+' || *(p + 1) == '-' )
							 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == '.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
			/* �ڔ��� */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == '.' )
		{
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( *p == '.' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == '.' ) break;  /* "." ���A���Ȃ璆�f */
						}
					}
					else if( *p == 'E' || *p == 'e' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == '+' || *(p + 1) == '-' )
							 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == '.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
			/* �ڔ��� */
			if( p < q )
			{
				if( *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == 'E' || *p == 'e' )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( ( *p == '+' || *p == '-' ) && ( *(p - 1) == 'E' || *(p - 1) == 'e' ) )
					{
						if( p + 1 < q )
						{
							if( *(p + 1) < '0' || *(p + 1) > '9' )
							{
								/* "0E+", "0E-" */
								break;
							}
						}
						else
						{
							/* "0E-", "0E+" */
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( i == 2 ) return 1;  /* "0E", 0e" �Ȃ� "0" �����l */
			/* �ڔ��� */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else
		{
			/* "0" ���������l */
			/*if( *p == '.' ) return i - 1;*/  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
	}

	else if( *p >= '1' && *p <= '9' )  /* 10�i�� */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." ���A���Ȃ璆�f */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		if( *(p - 1) == '.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
		/* �ڔ��� */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
			 || *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == '-' )  /* �}�C�i�X */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." ���A���Ȃ璆�f */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "-", "-." �����Ȃ琔�l�łȂ� */
		//@@@ 2001.11.09 start MIK
		//if( i <= 2 ) return 0;
		//if( *(p - 1)  == '.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
		if( i == 1 ) return 0;
		if( *(p - 1) == '.' )
		{
			i--;
			if( i == 1 ) return 0;
			return i;
		}  //@@@ 2001.11.09 end MIK
		/* �ڔ��� */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
			 || *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == '.' )  /* �����_ */
	{
		d++;
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." ���A���Ȃ璆�f */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "." �����Ȃ琔�l�łȂ� */
		if( i == 1 ) return 0;
		if( *(p - 1)  == '.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
		/* �ڔ��� */
		if( p < q )
		{
			if( *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

#if 0
	else if( *p == '&' )  /* VB��16�i�� */
	{
		p++; i++;
		if( ( p < q ) && ( *p == 'H' ) )
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= '0' && *p <= '9' )
				 || ( *p >= 'A' && *p <= 'F' )
				 || ( *p >= 'a' && *p <= 'f' ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "&H" �����Ȃ琔�l�łȂ� */
			if( i == 2 ) i = 0;
			return i;
		}

		/* "&" �����Ȃ琔�l�łȂ� */
		return 0;
	}
#endif

	/* ���l�ł͂Ȃ� */
	return 0;
}
//@@@ 2001.11.07 End by MIK




/**	�t�@�C���̑��݃`�F�b�N

	�w�肳�ꂽ�p�X�̃t�@�C�������݂��邩�ǂ������m�F����B
	
	@param path [in] ���ׂ�p�X��
	@param bFileOnly [in] true: �t�@�C���̂ݑΏ� / false: �f�B���N�g�����Ώ�
	
	@retval true  �t�@�C���͑��݂���
	@retval false �t�@�C���͑��݂��Ȃ�
	
	@author genta
	@date 2002.01.04 �V�K�쐬
*/
bool IsFileExists(const char* path, bool bFileOnly)
{
	WIN32_FIND_DATA fd;
	::ZeroMemory( &fd, sizeof(fd));

	HANDLE hFind = ::FindFirstFile( path, &fd );
	if( hFind != INVALID_HANDLE_VALUE ){
		::FindClose( hFind );
		if( bFileOnly == false ||
			( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
			return true;
	}
	return false;
}
/**	�t�@�C�����̐؂�o��

	�w�蕶���񂩂�t�@�C�����ƔF������镶��������o���A
	�擪Offset�y�ђ�����Ԃ��B
	
	@param pLine [in] �T���Ώە�����
	@param pnBgn [out] �擪offset�BpLine + *pnBgn���t�@�C�����擪�ւ̃|�C���^�B
	@param pnPathLen [out] �t�@�C�����̒���
	@param bFileOnly [in] true: �t�@�C���̂ݑΏ� / false: �f�B���N�g�����Ώ�
	
	@retval true �t�@�C��������
	@retval false �t�@�C�����͌�����Ȃ�����
	
	@date 2002.01.04 genta �f�B���N�g���������ΏۊO�ɂ���@�\��ǉ�
	@date 2003.01.15 matsumo gcc�̃G���[���b�Z�[�W(:��؂�)�ł��t�@�C�������o�\��
	@date 2004.05.29 genta C:\����t�@�C��C���؂�o�����̂�h�~
	@date 2004.11.13 genta/Moca �t�@�C�����擪��*?���l��
	@date 2005.01.10 genta �ϐ����ύX j -> cur_pos
	@date 2005.01.23 genta �x���}���̂��߁Cgoto��return�ɕύX
	
*/
bool IsFilePath( const char* pLine, int* pnBgn, int* pnPathLen, bool bFileOnly )
{
	int		i;
	int		nLineLen;
	char	szJumpToFile[1024];
	memset( szJumpToFile, 0, sizeof( szJumpToFile ) );

	nLineLen = strlen( pLine );

	//�擪�̋󔒂�ǂݔ�΂�
	for( i = 0; i < nLineLen; ++i ){
		if( ' ' != pLine[i] &&
			'\t' != pLine[i] &&
			'\"' != pLine[i]
		){
			break;
		}
	}

	//	#include <�t�@�C����>�̍l��
	//	#�Ŏn�܂�Ƃ���"�܂���<�܂œǂݔ�΂�
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

	//	���̎��_�Ŋ��ɍs���ɒB���Ă�����t�@�C�����͌�����Ȃ�
	if( i >= nLineLen ){
		return false;
	}
	*pnBgn = i;
	int cur_pos = 0;
	for( ; i <= nLineLen && cur_pos + 1 < sizeof(szJumpToFile); ++i ){
		if( ( i == nLineLen    ||
			  pLine[i] == ' '  ||
			  pLine[i] == '\t' ||	//@@@ 2002.01.08 YAZAKI �^�u�������B
			  pLine[i] == '('  ||
			  pLine[i] == '\r' ||
			  pLine[i] == '\n' ||
			  pLine[i] == '\0' ||
			  pLine[i] == '>'  ||
			  // May 29, 2004 genta C:\��:�̓t�@�C����؂�ƌ��Ȃ��ė~�����Ȃ�
			  ( cur_pos > 1 && pLine[i] == ':' ) ||   //@@@ 2003/1/15/ matsumo (for gcc)
			  pLine[i] == '"'
			) &&
			0 < strlen( szJumpToFile )
		){
			//	Jan. 04, 2002 genta
			//	�t�@�C�����݊m�F���@�ύX
			//if( -1 != _access( (const char *)szJumpToFile, 0 ) )
			if( IsFileExists(szJumpToFile, bFileOnly))
			{
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
//			return false;
//		}
//
//		szJumpToFile[cur_pos] = pLine[i];
//		cur_pos++;
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
			//	Oct. 5, 2002 genta
			//	2004.11.13 Moca/genta �擪�ɏ�̕���������ꍇ�̍l����ǉ�
			( i == 0 || ( i > 0 && ! _IS_SJIS_1( (unsigned char)pLine[i - 1] ))) ){
			return false;
		}else{
		szJumpToFile[cur_pos] = pLine[i];
		cur_pos++;
		}
	}
//	To Here Sept. 27, 2000
//	if( i >= nLineLen ){
//		return FALSE;
//	}
	//	Jan. 04, 2002 genta
	//	�t�@�C�����݊m�F���@�ύX
	if( 0 < strlen( szJumpToFile ) &&
		IsFileExists(szJumpToFile, bFileOnly))
	//	-1 != _access( (const char *)szJumpToFile, 0 ) )
	{
		//	Jan. 04, 2002 genta
		//	���܂�ɕςȃR�[�f�B���O�Ȃ̂ňړ�
		*pnPathLen = strlen( szJumpToFile );
		return true;
	}else{
//#ifdef _DEBUG
//		m_cShareData.TraceOut( "�t�@�C�������݂��Ȃ��BszJumpToFile=[%s]\n", szJumpToFile );
//#endif
	}

	return false;

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




/*! �w�蒷�ȉ��̃e�L�X�g�ɐ؂蕪����

	@param pText [in] �؂蕪���ΏۂƂȂ镶����ւ̃|�C���^
	@param nTextLen [in] �؂蕪���ΏۂƂȂ镶����S�̂̒���
	@param nLimitLen [in] �؂蕪���钷��
	@param pnLineLen [out] ���ۂɎ��o���ꂽ������̒���
	@param pnBgn [i/o] ����: �؂蕪���J�n�ʒu, �o��: ���o���ꂽ������̎��̈ʒu

	@note 2003.05.25 ���g�p�̂悤��
*/
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
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( pText, nTextLen, i );
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
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( pszData, nDataLength, i );
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


/*!	�����񂪎w�肳�ꂽ�����ŏI����Ă��Ȃ������ꍇ�ɂ�
	�����ɂ��̕�����t������D

	@param pszPath [i/o]���삷�镶����
	@param nMaxLen [in]�o�b�t�@��
	@param c [in]�ǉ�����������
	@retval  0 \��������t���Ă���
	@retval  1 \��t������
	@retval -1 �o�b�t�@�����肸�A\��t���ł��Ȃ�����
	@date 2003.06.24 Moca �V�K�쐬
*/
int AddLastChar( char* pszPath, int nMaxLen, char c ){
	int pos = strlen( pszPath );
	// �����Ȃ��Ƃ���\��t��
	if( 0 == pos ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[0] = c;
		pszPath[1] = '\0';
		return 1;
	}
	// �Ōオ\�łȂ��Ƃ���\��t��(���{����l��)
	else if( *::CharPrev( pszPath, &pszPath[pos] ) != c ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[pos] = c;
		pszPath[pos + 1] = '\0';
		return 1;
	}
	return 0;
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
//	hRes = 0;
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




/*!
	�������̃��[�U�[������\�ɂ���
	�u���b�L���O�t�b�N(?)�i���b�Z�[�W�z��

	@date 2003.07.04 genta ���̌Ăяo���ŕ������b�Z�[�W����������悤��
*/
BOOL BlockingHook( HWND hwndDlgCancel )
{
		MSG		msg;
		BOOL	ret;
		//	Jun. 04, 2003 genta ���b�Z�[�W�����邾����������悤��
		while(( ret = (BOOL)::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) != 0 ){
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

	@par history
	2001.12.22 YAZAKI sakura.hh���Q�Ƃ���悤�ɕύX
*/
int FuncID_To_HelpContextID( int nFuncID )
{
	switch( nFuncID ){

	/* �t�@�C������n */
	case F_FILENEW:				return HLP000025;			//�V�K�쐬
	case F_FILEOPEN:			return HLP000015;			//�J��
	case F_FILEOPEN_DROPDOWN:	return HLP000015;			//�J��(�h���b�v�_�E��)	//@@@ 2002.06.15 MIK
	case F_FILESAVE:			return HLP000020;			//�㏑���ۑ�
	case F_FILESAVEAS_DIALOG:	return HLP000021;			//���O��t���ĕۑ�
	case F_FILECLOSE:			return HLP000017;			//����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
	case F_FILECLOSE_OPEN:		return HLP000119;			//���ĊJ��
	case F_FILE_REOPEN:			return HLP000283;			//�J������	//@@@ 2003.06.15 MIK
	case F_FILE_REOPEN_SJIS:	return HLP000156;			//SJIS�ŊJ������
	case F_FILE_REOPEN_JIS:		return HLP000157;			//JIS�ŊJ������
	case F_FILE_REOPEN_EUC:		return HLP000158;			//EUC�ŊJ������
	case F_FILE_REOPEN_UNICODE:	return HLP000159;			//Unicode�ŊJ������
	case F_FILE_REOPEN_UNICODEBE:	return HLP000256;		//UnicodeBE�ŊJ������
	case F_FILE_REOPEN_UTF8:	return HLP000160;			//UTF-8�ŊJ������
	case F_FILE_REOPEN_UTF7:	return HLP000161;			//UTF-7�ŊJ������
	case F_PRINT:				return HLP000162;			//���				//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	case F_PRINT_PREVIEW:		return HLP000120;			//����v���r���[
	case F_PRINT_PAGESETUP:		return HLP000122;			//����y�[�W�ݒ�	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	case F_OPEN_HfromtoC:		return HLP000192;			//������C/C++�w�b�_(�\�[�X)���J��	//Feb. 7, 2001 JEPRO �ǉ�
	case F_OPEN_HHPP:			return HLP000024;			//������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	case F_OPEN_CCPP:			return HLP000026;			//������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	case F_ACTIVATE_SQLPLUS:	return HLP000132;			/* Oracle SQL*Plus���A�N�e�B�u�\�� */
	case F_PLSQL_COMPILE_ON_SQLPLUS:	return HLP000027;	/* Oracle SQL*Plus�Ŏ��s */
	case F_BROWSE:				return HLP000121;			//�u���E�Y
	case F_READONLY:			return HLP000249;			//�ǂݎ���p
	case F_PROPERTY_FILE:		return HLP000022;			/* �t�@�C���̃v���p�e�B */

//	case IDM_EXITALL:			return 28;			//�e�L�X�g�G�f�B�^�̑S�I��
	case F_EXITALL:			return HLP000028;				//�T�N���G�f�B�^�̑S�I��	//Dec. 26, 2000 JEPRO F_�ɕύX


	/* �ҏW�n */
	case F_UNDO:						return HLP000032;	//���ɖ߂�(Undo)
	case F_REDO:						return HLP000033;	//��蒼��(Redo)
	case F_DELETE:						return HLP000041;	//�폜
	case F_DELETE_BACK:					return HLP000042;	//�J�[�\���O���폜
	case F_WordDeleteToStart:			return HLP000166;	//�P��̍��[�܂ō폜
	case F_WordDeleteToEnd:				return HLP000167;	//�P��̉E�[�܂ō폜
	case F_WordCut:						return HLP000169;	//�P��؂���
	case F_WordDelete:					return HLP000168;	//�P��폜
	case F_LineCutToStart:				return HLP000172;	//�s���܂Ő؂���(���s�P��)
	case F_LineCutToEnd:				return HLP000173;	//�s���܂Ő؂���(���s�P��)
	case F_LineDeleteToStart:			return HLP000170;	//�s���܂ō폜(���s�P��)
	case F_LineDeleteToEnd:				return HLP000171;	//�s���܂ō폜(���s�P��)
	case F_CUT_LINE:					return HLP000174;	//�s�؂���(�܂�Ԃ��P��)
	case F_DELETE_LINE:					return HLP000137;	//�s�폜(�܂�Ԃ��P��)
	case F_DUPLICATELINE:				return HLP000043;	//�s�̓�d��(�܂�Ԃ��P��)
	case F_INDENT_TAB:					return HLP000113;	//TAB�C���f���g
	case F_UNINDENT_TAB:				return HLP000113;	//�tTAB�C���f���g
	case F_INDENT_SPACE:				return HLP000114;	//SPACE�C���f���g
	case F_UNINDENT_SPACE:				return HLP000114;	//�tSPACE�C���f���g
	case F_RECONVERT:					return HLP000218;	//�ĕϊ�
//	case ORDSREFERENCE:					return ;	//�P�ꃊ�t�@�����X


	/* �J�[�\���ړ��n */
//	case F_UP:				return ;	//�J�[�\����ړ�
//	case F_DOWN:			return ;	//�J�[�\�����ړ�
//	case F_LEFT:			return ;	//�J�[�\�����ړ�
//	case F_RIGHT:			return ;	//�J�[�\���E�ړ�
	case F_UP2:				return HLP000220;	//�J�[�\����ړ�(�Q�s����)
	case F_DOWN2:			return HLP000221;	//�J�[�\�����ړ�(�Q�s����)
	case F_WORDLEFT:		return HLP000222;	//�P��̍��[�Ɉړ�
	case F_WORDRIGHT:		return HLP000223;	//�P��̉E�[�Ɉړ�
	case F_GOLINETOP:		return HLP000224;	//�s���Ɉړ�(�܂�Ԃ��P��)
	case F_GOLINEEND:		return HLP000225;	//�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_ROLLDOWN:		return ;	//�X�N���[���_�E��
//	case F_ROLLUP:			return ;	//�X�N���[���A�b�v
	case F_HalfPageUp:		return HLP000245;	//���y�[�W�A�b�v	//Oct. 17, 2000 JEPRO �ȉ��u�P�y�[�W�_�E���v�܂Œǉ�
	case F_HalfPageDown:	return HLP000246;	//���y�[�W�_�E��
	case F_1PageUp:			return HLP000226;	//�P�y�[�W�A�b�v
	case F_1PageDown:		return HLP000227;	//�P�y�[�W�_�E��
	case F_GOFILETOP:		return HLP000228;	//�t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND:		return HLP000229;	//�t�@�C���̍Ō�Ɉړ�
	case F_CURLINECENTER:	return HLP000230;	//�J�[�\���s���E�B���h�E������
	case F_JUMP_SRCHSTARTPOS:	return HLP000264; //�����J�n�ʒu�֖߂�
	case F_JUMPHIST_PREV:		return HLP000231;	//�ړ�����: �O��	//Oct. 17, 2000 JEPRO �ȉ��u�ړ�����:���ցv�܂Œǉ�
	case F_JUMPHIST_NEXT:		return HLP000232;	//�ړ�����: ����
	case F_JUMPHIST_SET:	return HLP000265;	//���݈ʒu���ړ������ɓo�^
	case F_WndScrollDown:	return HLP000198;	//�e�L�X�g���P�s���փX�N���[��	//Jul. 05, 2001 JEPRO �ǉ�
	case F_WndScrollUp:		return HLP000199;	//�e�L�X�g���P�s��փX�N���[��	//Jul. 05, 2001 JEPRO �ǉ�
	case F_GONEXTPARAGRAPH:	return HLP000262;	//�O�̒i���Ɉړ�
	case F_GOPREVPARAGRAPH:	return HLP000263;	//�O�̒i���Ɉړ�

	/* �I���n */	//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v����(�I��)���ړ�
	case F_SELECTWORD:		return HLP000045;	//���݈ʒu�̒P��I��
	case F_SELECTALL:		return HLP000044;	//���ׂđI��
	case F_BEGIN_SEL:		return HLP000233;	//�͈͑I���J�n
//	case F_UP_SEL:			return ;	//(�͈͑I��)�J�[�\����ړ�
//	case F_DOWN_SEL:		return ;	//(�͈͑I��)�J�[�\�����ړ�
//	case F_LEFT_SEL:		return ;	//(�͈͑I��)�J�[�\�����ړ�
//	case F_RIGHT_SEL:		return ;	//(�͈͑I��)�J�[�\���E�ړ�
	case F_UP2_SEL:			return HLP000234;	//(�͈͑I��)�J�[�\����ړ�(�Q�s����)
	case F_DOWN2_SEL:		return HLP000235;	//(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
	case F_WORDLEFT_SEL:	return HLP000236;	//(�͈͑I��)�P��̍��[�Ɉړ�
	case F_WORDRIGHT_SEL:	return HLP000237;	//(�͈͑I��)�P��̉E�[�Ɉړ�
	case F_GONEXTPARAGRAPH_SEL:	return HLP000273;	//(�͈͑I��)�O�̒i���Ɉړ�	//@@@ 2003.06.15 MIK
	case F_GOPREVPARAGRAPH_SEL:	return HLP000274;	//(�͈͑I��)�O�̒i���Ɉړ�	//@@@ 2003.06.15 MIK
	case F_GOLINETOP_SEL:	return HLP000238;	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	case F_GOLINEEND_SEL:	return HLP000239;	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_ROLLDOWN_SEL:	return ;	//(�͈͑I��)�X�N���[���_�E��
//	case F_ROLLUP_SEL:		return ;	//(�͈͑I��)�X�N���[���A�b�v
	case F_HalfPageUp_Sel:	return HLP000247;	//(�͈͑I��)���y�[�W�A�b�v		//Oct. 17, 2000 JEPRO �ȉ��u�P�y�[�W�_�E���v�܂Œǉ�
	case F_HalfPageDown_Sel:return HLP000248;	//(�͈͑I��)���y�[�W�_�E��
	case F_1PageUp_Sel:		return HLP000240;	//(�͈͑I��)�P�y�[�W�A�b�v
	case F_1PageDown_Sel:	return HLP000241;	//(�͈͑I��)�P�y�[�W�_�E��
	case F_GOFILETOP_SEL:	return HLP000242;	//(�͈͑I��)�t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND_SEL:	return HLP000243;	//(�͈͑I��)�t�@�C���̍Ō�Ɉړ�
//	case F_GONEXTPARAGRAPH_SEL:	return HLP000262;	//(�͈͑I��)���̒i����
//	case F_GOPREVPARAGRAPH_SEL:	return HLP000263;	//(�͈͑I��)�O�̒i����


	/* ��`�I���n */	//Oct. 17, 2000 JEPRO (��`�I��)���V�݂��ꎟ�悱���ɂ���
//	case F_BOXSELALL:		return ;	//��`�ł��ׂđI��
	case F_BEGIN_BOX:		return HLP000244;	//��`�͈͑I���J�n
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
	/* ���`�n 2002/04/17 YAZAKI */
	case F_LTRIM:		return HLP000210;	//��(�擪)�̋󔒂��폜
	case F_RTRIM:		return HLP000211;	//�E(����)�̋󔒂��폜
	case F_SORT_ASC:	return HLP000212;	//�I���s�̏����\�[�g
	case F_SORT_DESC:	return HLP000213;	//�I���s�̍~���\�[�g
	case F_MERGE:		return HLP000214;	//�I���s�̃}�[�W

	/* �N���b�v�{�[�h�n */
	case F_CUT:				return HLP000034;			//�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	case F_COPY:			return HLP000035;			//�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_COPY_ADDCRLF:	return HLP000219;			//�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_COPY_CRLF:		return HLP000163;			//CRLF���s�ŃR�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)	//Feb. 23, 2001 JEPRO �����Ă����̂Œǉ�
	case F_PASTE:			return HLP000039;			//�\��t��(�N���b�v�{�[�h����\��t��)
	case F_PASTEBOX:		return HLP000040;			//��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
//	case F_INSTEXT:			return ;			// �e�L�X�g��\��t��
	case F_COPYLINES:				return HLP000036;	//�I��͈͓��S�s�R�s�[
	case F_COPYLINESASPASSAGE:		return HLP000037;	//�I��͈͓��S�s���p���t���R�s�[
	case F_COPYLINESWITHLINENUMBER:	return HLP000038;	//�I��͈͓��S�s�s�ԍ��t���R�s�[
	case F_COPYPATH:		return HLP000056;			//���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	case F_COPYTAG:			return HLP000175;			//���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[	//Oct. 17, 2000 JEPRO �ǉ�
	case F_COPYFNAME:		return HLP000303;			//���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ // 2002/2/3 aroka
//	case IDM_TEST_CREATEKEYBINDLIST:	return 57;	//�L�[���蓖�Ĉꗗ���N���b�v�{�[�h�փR�s�[	//Sept. 15, 2000 jepro�u���X�g�v���u�ꗗ�v�ɕύX
	case F_CREATEKEYBINDLIST:		return HLP000057;	//�L�[���蓖�Ĉꗗ���N���b�v�{�[�h�փR�s�[	//Sept. 15, 2000 JEPRO �u���X�g�v���u�ꗗ�v�ɕύX�AIDM�QTEST��F�ɕύX���������܂������Ȃ��̂ŎE���Ă���	//Dec. 25, 2000 ����


	/* �}���n */
	case F_INS_DATE:				return HLP000164;	// ���t�}��
	case F_INS_TIME:				return HLP000165;	// �����}��
	case F_CTRL_CODE_DIALOG:		return HLP000255;	/* �R���g���[���R�[�h���� */


	/* �ϊ��n */
	case F_TOLOWER:					return HLP000047;	//�p�啶�����p������
	case F_TOUPPER:					return HLP000048;	//�p���������p�啶��
	case F_TOHANKAKU:				return HLP000049;	/* �S�p�����p */
	case F_TOHANKATA:				return HLP000258;	//�S�p�J�^�J�i�����p�J�^�J�i
	case F_TOZENKAKUKATA:			return HLP000050;	/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	case F_TOZENKAKUHIRA:			return HLP000051;	/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	case F_HANKATATOZENKAKUKATA:	return HLP000123;	/* ���p�J�^�J�i���S�p�J�^�J�i */
	case F_HANKATATOZENKAKUHIRA:	return HLP000124;	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	case F_TOZENEI:					return HLP000200;	/* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka //Stonee, 2001/09/26 �ԍ��C��
	case F_TOHANEI:					return HLP000215;	/* �S�p�p�������p�p�� */			//@@@ 2002.2.11 YAZAKI
	case F_TABTOSPACE:				return HLP000182;	/* TAB���� */
	case F_SPACETOTAB:				return HLP000196;	/* �󔒁�TAB */	//#### Stonee, 2001/05/27	//Jul. 03, 2001 JEPRO �ԍ��C��
	case F_CODECNV_AUTO2SJIS:		return HLP000178;	/* �������ʁ�SJIS�R�[�h�ϊ� */
	case F_CODECNV_EMAIL:			return HLP000052;	//E-Mail(JIS��SJIS)�R�[�h�ϊ�
	case F_CODECNV_EUC2SJIS:		return HLP000053;	//EUC��SJIS�R�[�h�ϊ�
	case F_CODECNV_UNICODE2SJIS:	return HLP000179;	//Unicode��SJIS�R�[�h�ϊ�
	case F_CODECNV_UNICODEBE2SJIS:	return HLP000257;	//UnicodeBE��SJIS�R�[�h�ϊ�
	case F_CODECNV_UTF82SJIS:		return HLP000142;	/* UTF-8��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UTF72SJIS:		return HLP000143; /* UTF-7��SJIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2JIS:		return HLP000117;	/* SJIS��JIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2EUC:		return HLP000118;	/* SJIS��EUC�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF8:		return HLP000180;	/* SJIS��UTF-8�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF7:		return HLP000181;	/* SJIS��UTF-7�R�[�h�ϊ� */
	case F_BASE64DECODE:			return HLP000054;	//Base64�f�R�[�h���ĕۑ�
	case F_UUDECODE:				return HLP000055;	//uudecode���ĕۑ�	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX


	/* �����n */
	case F_SEARCH_DIALOG:		return HLP000059;	//����(�P�ꌟ���_�C�A���O)
	case F_SEARCH_BOX:			return HLP000059;	//����(�{�b�N�X) Jan. 13, 2003 MIK
	case F_SEARCH_NEXT:			return HLP000061;	//��������
	case F_SEARCH_PREV:			return HLP000060;	//�O������
	case F_REPLACE_DIALOG:		return HLP000062;	//�u��(�u���_�C�A���O)
	case F_SEARCH_CLEARMARK:	return HLP000136;	//�����}�[�N�̃N���A
	case F_GREP_DIALOG:			return HLP000067;	//Grep
	case F_JUMP_DIALOG:			return HLP000063;	//�w��s�փW�����v
	case F_OUTLINE:				return HLP000064;	//�A�E�g���C�����
	case F_TAGJUMP:				return HLP000065;	//�^�O�W�����v�@�\
	case F_TAGJUMPBACK:			return HLP000066;	//�^�O�W�����v�o�b�N�@�\
	case F_TAGS_MAKE:			return HLP000280;	//�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
	case F_TAGJUMP_LIST:		return HLP000281;	//�^�O�W�����v�ꗗ	//@@@ 2003.04.17 MIK
	case F_DIRECT_TAGJUMP:		return HLP000281;	//�_�C���N�g�^�O�W�����v	//@@@ 2003.04.17 MIK
	case F_COMPARE:				return HLP000116;	//�t�@�C�����e��r
	case F_DIFF_DIALOG:			return HLP000251;	//DIFF�����\��(�_�C�A���O)	//@@@ 2002.05.25 MIK
//	case F_DIFF:				return HLP000251;	//DIFF�����\��	//@@@ 2002.05.25 MIK
	case F_DIFF_NEXT:			return HLP000252;	//���̍�����	//@@@ 2002.05.25 MIK
	case F_DIFF_PREV:			return HLP000253;	//�O�̍�����	//@@@ 2002.05.25 MIK
	case F_DIFF_RESET:			return HLP000254;	//�����̑S����	//@@@ 2002.05.25 MIK
	case F_BRACKETPAIR:			return HLP000183;	//�Ί��ʂ̌���	//Oct. 17, 2000 JEPRO �ǉ�
	case F_BOOKMARK_SET:		return HLP000205;	//�u�b�N�}�[�N�ݒ�E����
	case F_BOOKMARK_NEXT:		return HLP000206;	//���̃u�b�N�}�[�N��
	case F_BOOKMARK_PREV:		return HLP000207;	//�O�̃u�b�N�}�[�N��
	case F_BOOKMARK_RESET:		return HLP000208;	//�u�b�N�}�[�N�̑S����
	case F_BOOKMARK_VIEW:		return HLP000209;	//�u�b�N�}�[�N�̈ꗗ

	/* ���[�h�؂�ւ��n */
	case F_CHGMOD_INS:		return HLP000046;	//�}���^�㏑�����[�h�؂�ւ�
	case F_CHGMOD_EOL_CRLF:	return HLP000285;	//���͉��s�R�[�h�w��	// 2003.09.23 Moca
	case F_CHGMOD_EOL_CR:	return HLP000285;	//���͉��s�R�[�h�w��	// 2003.09.23 Moca
	case F_CHGMOD_EOL_LF:	return HLP000285;	//���͉��s�R�[�h�w��	// 2003.09.23 Moca
	case F_CANCEL_MODE:		return HLP000194;	//�e�탂�[�h�̎�����


	/* �ݒ�n */
	case F_SHOWTOOLBAR:		return HLP000069;	/* �c�[���o�[�̕\�� */
	case F_SHOWFUNCKEY:		return HLP000070;	/* �t�@���N�V�����L�[�̕\�� */
	case F_SHOWTAB:			return HLP000282;	/* �^�u�̕\�� */	//@@@ 2003.06.10 MIK
	case F_SHOWSTATUSBAR:	return HLP000134;	/* �X�e�[�^�X�o�[�̕\�� */
	case F_TYPE_LIST:		return HLP000072;	/* �^�C�v�ʐݒ�ꗗ */
	case F_OPTION_TYPE:		return HLP000073;	/* �^�C�v�ʐݒ� */
	case F_OPTION:			return HLP000076;	/* ���ʐݒ� */
//From here �ݒ�_�C�A���O�p��help�g�s�b�NID��ǉ�  Stonee, 2001/05/18
	case F_TYPE_SCREEN:		return HLP000074;	/* �^�C�v�ʐݒ�w�X�N���[���x */
	case F_TYPE_COLOR:		return HLP000075;	/* �^�C�v�ʐݒ�w�J���[�x */
	case F_TYPE_HELPER:		return HLP000197;	/* �^�C�v�ʐݒ�w�x���x */	//Jul. 03, 2001 JEPRO �ǉ�
	case F_TYPE_REGEX_KEYWORD:	return HLP000203;	/* �^�C�v�ʐݒ�w���K�\���L�[���[�h�x */	//@@@ 2001.11.17 add MIK
	case F_OPTION_GENERAL:	return HLP000081;	/* ���ʐݒ�w�S�ʁx */
	case F_OPTION_WINDOW:	return HLP000146;	/* ���ʐݒ�w�E�B���h�E�x */
	case F_OPTION_EDIT:		return HLP000144;	/* ���ʐݒ�w�ҏW�x */
	case F_OPTION_FILE:		return HLP000083;	/* ���ʐݒ�w�t�@�C���x */
	case F_OPTION_BACKUP:	return HLP000145;	/* ���ʐݒ�w�o�b�N�A�b�v�x */
	case F_OPTION_FORMAT:	return HLP000082;	/* ���ʐݒ�w�����x */
	case F_OPTION_URL:		return HLP000147;	/* ���ʐݒ�w�N���b�J�u��URL�x */
	case F_OPTION_GREP:		return HLP000148;	/* ���ʐݒ�wGrep�x */
	case F_OPTION_KEYBIND:	return HLP000084;	/* ���ʐݒ�w�L�[���蓖�āx */
	case F_OPTION_CUSTMENU:	return HLP000087;	/* ���ʐݒ�w�J�X�^�����j���[�x */
	case F_OPTION_TOOLBAR:	return HLP000085;	/* ���ʐݒ�w�c�[���o�[�x */
	case F_OPTION_KEYWORD:	return HLP000086;	/* ���ʐݒ�w�����L�[���[�h�x */
	case F_OPTION_HELPER:	return HLP000088;	/* ���ʐݒ�w�x���x */
//To here  Stonee, 2001/05/18
	case F_OPTION_MACRO:	return HLP000201;	/* ���ʐݒ�w�}�N���x */	//@@@ 2002.01.02
	case F_OPTION_FNAME:	return HLP000277;	/* ���ʐݒ� �w�S�ʁx�v���p�e�B */	// 2002.12.09 Moca Add	//�d�����
	case F_FONT:			return HLP000071;	/* �t�H���g�ݒ� */
	case F_WRAPWINDOWWIDTH:	return HLP000184;	/* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX	//Jul. 03, 2001 JEPRO �ԍ��C��
	case F_FAVORITE:		return HLP000279;	/* ���C�ɓ���̐ݒ� */	//@@@ 2003.04.08 MIK

	/* �}�N�� */
	case F_RECKEYMACRO:		return HLP000125;	/* �L�[�}�N���L�^�J�n�^�I�� */
	case F_SAVEKEYMACRO:	return HLP000127;	/* �L�[�}�N���ۑ� */
	case F_LOADKEYMACRO:	return HLP000128;	/* �L�[�}�N���ǂݍ��� */
	case F_EXECKEYMACRO:	return HLP000126;	/* �L�[�}�N�����s */
//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//	case F_EXECCMMAND:		return 103; /* �O���R�}���h���s */
	case F_EXECCOMMAND_DIALOG:	return HLP000103; /* �O���R�}���h���s */
//	To Here Sept. 20, 2000


	/* �J�X�^�����j���[ */
	case F_MENU_RBUTTON:	return HLP000195;	/* �E�N���b�N���j���[ */
	case F_CUSTMENU_1:	return HLP000186;	/* �J�X�^�����j���[1 */
	case F_CUSTMENU_2:	return HLP000186;	/* �J�X�^�����j���[2 */
	case F_CUSTMENU_3:	return HLP000186;	/* �J�X�^�����j���[3 */
	case F_CUSTMENU_4:	return HLP000186;	/* �J�X�^�����j���[4 */
	case F_CUSTMENU_5:	return HLP000186;	/* �J�X�^�����j���[5 */
	case F_CUSTMENU_6:	return HLP000186;	/* �J�X�^�����j���[6 */
	case F_CUSTMENU_7:	return HLP000186;	/* �J�X�^�����j���[7 */
	case F_CUSTMENU_8:	return HLP000186;	/* �J�X�^�����j���[8 */
	case F_CUSTMENU_9:	return HLP000186;	/* �J�X�^�����j���[9 */
	case F_CUSTMENU_10:	return HLP000186;	/* �J�X�^�����j���[10 */
	case F_CUSTMENU_11:	return HLP000186;	/* �J�X�^�����j���[11 */
	case F_CUSTMENU_12:	return HLP000186;	/* �J�X�^�����j���[12 */
	case F_CUSTMENU_13:	return HLP000186;	/* �J�X�^�����j���[13 */
	case F_CUSTMENU_14:	return HLP000186;	/* �J�X�^�����j���[14 */
	case F_CUSTMENU_15:	return HLP000186;	/* �J�X�^�����j���[15 */
	case F_CUSTMENU_16:	return HLP000186;	/* �J�X�^�����j���[16 */
	case F_CUSTMENU_17:	return HLP000186;	/* �J�X�^�����j���[17 */
	case F_CUSTMENU_18:	return HLP000186;	/* �J�X�^�����j���[18 */
	case F_CUSTMENU_19:	return HLP000186;	/* �J�X�^�����j���[19 */
	case F_CUSTMENU_20:	return HLP000186;	/* �J�X�^�����j���[20 */
	case F_CUSTMENU_21:	return HLP000186;	/* �J�X�^�����j���[21 */
	case F_CUSTMENU_22:	return HLP000186;	/* �J�X�^�����j���[22 */
	case F_CUSTMENU_23:	return HLP000186;	/* �J�X�^�����j���[23 */
	case F_CUSTMENU_24:	return HLP000186;	/* �J�X�^�����j���[24 */


	/* �E�B���h�E�n */
	case F_SPLIT_V:			return HLP000093;	//�㉺�ɕ���	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	case F_SPLIT_H:			return HLP000094;	//���E�ɕ���	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	case F_SPLIT_VH:		return HLP000095;	//�c���ɕ���	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	case F_WINCLOSE:		return HLP000018;	//�E�B���h�E�����
	case F_WIN_CLOSEALL:	return HLP000019;	//���ׂẴE�B���h�E�����	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
	case F_NEXTWINDOW:		return HLP000092;	//���̃E�B���h�E
	case F_PREVWINDOW:		return HLP000091;	//�O�̃E�B���h�E
	case F_CASCADE:			return HLP000138;	//�d�˂ĕ\��
	case F_TILE_V:			return HLP000140;	//�㉺�ɕ��ׂĕ\��
	case F_TILE_H:			return HLP000139;	//���E�ɕ��ׂĕ\��
	case F_MAXIMIZE_V:		return HLP000141;	//�c�����ɍő剻
	case F_MAXIMIZE_H:		return HLP000098;	//�������ɍő剻	//2001.02.10 by MIK
	case F_MINIMIZE_ALL:	return HLP000096;	//���ׂčŏ���	//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
	case F_REDRAW:			return HLP000187;	//�ĕ`��
	case F_WIN_OUTPUT:		return HLP000188;	//�A�E�g�v�b�g�E�B���h�E�\��


	/* �x�� */
	case F_HOKAN:			return HLP000111;	/* ���͕⊮�@�\ */
//Sept. 15, 2000��Nov. 25, 2000 JEPRO	//�V���[�g�J�b�g�L�[�����܂������Ȃ��̂ŎE���Ă���������2�s���C���E����
	case F_HELP_CONTENTS:	return HLP000100;	//�w���v�ڎ�			//Nov. 25, 2000 JEPRO
	case F_HELP_SEARCH:		return HLP000101;	//�w���v�L�[���[�h����	//Nov. 25, 2000 JEPRO�u�g�s�b�N�́v���u�L�[���[�h�v�ɕύX
	case F_MENU_ALLFUNC:	return HLP000189;	/* �R�}���h�ꗗ */
	case F_EXTHELP1:		return HLP000190;	/* �O���w���v�P */
	case F_EXTHTMLHELP:		return HLP000191;	/* �O��HTML�w���v */
	case F_ABOUT:			return HLP000102;	//�o�[�W�������	//Dec. 24, 2000 JEPRO F_�ɕύX


	/* ���̑� */
//	case F_SENDMAIL:		return ;	/* ���[�����M */

	default:
		// From Here 2003.09.23 Moca
		if( IDM_SELMRU <= nFuncID && nFuncID < IDM_SELMRU + MAX_MRU ){
			return HLP000029;	//�ŋߎg�����t�@�C��
		}else if( IDM_SELOPENFOLDER <= nFuncID && nFuncID < IDM_SELOPENFOLDER + MAX_OPENFOLDER ){
			return HLP000023;	//�ŋߎg�����t�H���_
		}else if( IDM_SELWINDOW <= nFuncID && nFuncID < IDM_SELWINDOW + MAX_EDITWINDOWS ){
			return HLP000097;	//�E�B���h�E���X�g
		}
		// To Here 2003.09.23 Moca
		return 0;
	}
}

/*!�N���[�v�{�[�h��Text�`���ŃR�s�[����
	@param hwnd [in] �N���b�v�{�[�h�̃I�[�i�[
	@param pszText [in] �ݒ肷��e�L�X�g
	@param length [in] �L���ȃe�L�X�g�̒���
	
	@retval true �R�s�[����
	@retval false �R�s�[���s�B�ꍇ�ɂ���Ă̓N���b�v�{�[�h�Ɍ��̓��e���c��
	@date 2004.02.17 Moca �e���̃\�[�X�𓝍�
*/
SAKURA_CORE_API bool SetClipboardText( HWND hwnd, const char* pszText, int length )
{
	HGLOBAL		hgClip;
	char*		pszClip;

	hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, length + 1 );
	if( NULL == hgClip ){
		return false;
	}
	pszClip = (char*)::GlobalLock( hgClip );
	if( NULL == pszClip ){
		::GlobalFree( hgClip );
		return false;
	}
	memcpy( pszClip, pszText, length );
	pszClip[length] = 0;
	::GlobalUnlock( hgClip );
	if( FALSE == ::OpenClipboard( hwnd ) ){
		::GlobalFree( hgClip );
		return false;
	}
	::EmptyClipboard();
	::SetClipboardData( CF_OEMTEXT, hgClip );
	::CloseClipboard();

	return true;
}

//	From Here Jun. 26, 2001 genta
/*!
	�^����ꂽ���K�\�����C�u�����̏��������s���D
	���b�Z�[�W�t���O��ON�ŏ������Ɏ��s�����Ƃ��̓��b�Z�[�W��\������D

	@param hWnd [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h���B
			�o�[�W�����ԍ��̐ݒ肪�s�v�ł����NULL�B
	@param rRegexp [in] �`�F�b�N�ɗ��p����CBregexp�N���X�ւ̎Q��
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
	@param nOption [in] �啶���Ə������𖳎����Ĕ�r����t���O // 2002/2/1 hor�ǉ�

	@retval true ���K�\���͋K���ʂ�
	@retval false ���@�Ɍ�肪����B�܂��́A���C�u�������g�p�ł��Ȃ��B
*/
bool CheckRegexpSyntax( const char* szPattern, HWND hWnd, bool bShowMessage, int nOption )
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		return false;
	}
	if( !cRegexp.Compile( szPattern, nOption ) ){	// 2002/2/1 hor�ǉ�
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

HWND OpenHtmlHelp( HWND hWnd, LPCSTR szFile, UINT uCmd, DWORD_PTR data, bool msgflag )
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



// Stonee, 2001/12/21
// NetWork��̃��\�[�X�ɐڑ����邽�߂̃_�C�A���O���o��������
// NO_ERROR:���� ERROR_CANCELLED:�L�����Z�� ����ȊO:���s
// �v���W�F�N�g�̐ݒ�Ń����N���W���[����Mpr.lib��ǉ��̂���
DWORD NetConnect ( const char strNetWorkPass[] )
{
	NETRESOURCE nr;
	//char sPassWord[] = "\0";	//�p�X���[�h
	//char sUser[] = "\0";		//���[�U�[��
	DWORD dwRet;				//�߂�l�@�G���[�R�[�h��WINERROR.H���Q��
	char sTemp[256];
	char sDrive[] = "";
    int i;

	if (strlen(strNetWorkPass) < 3)	return ERROR_BAD_NET_NAME;  //UNC�ł͂Ȃ��B
	if (strNetWorkPass[0] != '\\' && strNetWorkPass[1] != '\\')	return ERROR_BAD_NET_NAME;  //UNC�ł͂Ȃ��B

	//3�����ڂ��琔���čŏ���\�̒��O�܂ł�؂�o��
	sTemp[0] = '\\';
	sTemp[1] = '\\';
	for (i = 2; strNetWorkPass[i] != '\0'; i++) {
		if (strNetWorkPass[i] == '\\') break;
		sTemp[i] = strNetWorkPass[i];
	}
	sTemp[i] = '\0';	//�I�[

/*
	MYMESSAGEBOX(
		0,
		MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
		"�I�I",
		sTemp
	);
*/
	ZeroMemory( &nr, sizeof( nr ) );
	nr.dwScope = RESOURCE_GLOBALNET;
	nr.dwType = RESOURCETYPE_DISK;
	nr.dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
	nr.dwUsage = RESOURCEUSAGE_CONNECTABLE;
	nr.lpLocalName = sDrive;
	nr.lpRemoteName = sTemp;

	//���[�U�[�F�؃_�C�A���O��\��
	dwRet = WNetAddConnection3(0, &nr, NULL, NULL, CONNECT_UPDATE_PROFILE | CONNECT_INTERACTIVE);

	return dwRet;
}

/*! �����̃G�X�P�[�v

	@param org [in] �ϊ�������������
	@param buf [out] �ԊҌ�̕����������o�b�t�@
	@param cesc  [in] �G�X�P�[�v���Ȃ��Ƃ����Ȃ�����
	@param cwith [in] �G�X�P�[�v�Ɏg������
	
	@retval �o�͂����o�C�g�� (Unicode�̏ꍇ�͕�����)

	�����񒆂ɂ��̂܂܎g���Ƃ܂�������������ꍇ�ɂ��̕����̑O��
	�G�X�P�[�v�L�����N�^��}�����邽�߂Ɏg���D

	@note �ϊ���̃f�[�^�͍ő�Ō��̕������2�{�ɂȂ�
	@note ���̊֐���2�o�C�g�����̍l�����s���Ă��Ȃ�

	@author genta
	@date 2002/01/04 �V�K�쐬
	@date 2002/01/30 genta &��p(dupamp)�����ʂ̕�����������悤�Ɋg���D
		dupamp��inline�֐��ɂ����D
	@date 2002/02/01 genta bugfix �G�X�P�[�v���镶���Ƃ���镶���̏o�͏������t������
	@date 2004/06/19 genta Generic mapping�Ή�
*/
int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith)
{
	TCHAR *out = buf;
	for( ; *org != _T('\0'); ++org, ++out ){
		if( *org == cesc ){
			*out = cwith;
			++out;
		}
		*out = *org;
	}
	*out = _T('\0');
	return out - buf;
}

/*! �����̃G�X�P�[�v

	@param org [in] �ϊ�������������
	@param buf [out] �ԊҌ�̕����������o�b�t�@
	@param cesc  [in] �G�X�P�[�v���Ȃ��Ƃ����Ȃ�����
	@param cwith [in] �G�X�P�[�v�Ɏg������
	
	@retval �o�͂����o�C�g��

	�����񒆂ɂ��̂܂܎g���Ƃ܂�������������ꍇ�ɂ��̕����̑O��
	�G�X�P�[�v�L�����N�^��}�����邽�߂Ɏg���D

	@note �ϊ���̃f�[�^�͍ő�Ō��̕������2�{�ɂȂ�
	@note ���̊֐���2�o�C�g�����̍l�����s���Ă���
	
	@note 2003.05.25 ���g�p�̂悤��
*/
int cescape_j(const char* org, char* buf, char cesc, char cwith)
{
	char *out = buf;
	for( ; *org != '\0'; ++org, ++out ){
		if( _IS_SJIS_1( (unsigned char)*org ) ){
			*out = *org;
			++out; ++org;
		}
		else if( *org == cesc ){
			*out = cwith;
			++out;
		}
		*out = *org;
	}
	*out = '\0';
	return out - buf;
}

/*	�w���v�̖ڎ���\��
	�ڎ��^�u��\���B��肪����o�[�W�����ł́A�ڎ��y�[�W��\���B
*/
void ShowWinHelpContents( HWND hwnd, LPCTSTR lpszHelp )
{
	COsVersionInfo cOsVer;
	if ( cOsVer.HasWinHelpContentsProblem() ){
		/* �ڎ��y�[�W��\������ */
		::WinHelp( hwnd, lpszHelp, HELP_CONTENTS , 0 );
		return;
	}
	/* �ڎ��^�u��\������ */
	::WinHelp( hwnd, lpszHelp, HELP_COMMAND, (ULONG_PTR)"CONTENTS()" );
	return;
}


/*
 * �J���[������C���f�b�N�X�ԍ��ɕϊ�����
 */
SAKURA_CORE_API int GetColorIndexByName( const char *name )
{
	int	i;
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		if( strcmp( name, (const char*)colorIDXKeyName[i] ) == 0 ) return i;
	}
	return -1;
}

/*
 * �C���f�b�N�X�ԍ�����J���[���ɕϊ�����
 */
SAKURA_CORE_API const char* GetColorNameByIndex( int index )
{
	return colorIDXKeyName[index];
}

/*!
	@brief ���W�X�g�����當�����ǂݏo���D
	
	@param Hive [in] HIVE
	@param Path [in] ���W�X�g���L�[�ւ̃p�X
	@param Item [in] ���W�X�g���A�C�e�����DNULL�ŕW���̃A�C�e���D
	@param Buffer [out] �擾��������i�[����ꏊ
	@param BufferSize [in] Buffer�̎w���̈�̃T�C�Y
	
	@retval true �l�̎擾�ɐ���
	@retval false �l�̎擾�Ɏ��s
	
	@author �S
	@date 2002.09.10 genta CWSH.cpp����ړ�
*/
bool ReadRegistry(HKEY Hive, char const *Path, char const *Item, char *Buffer, unsigned BufferSize)
{
	bool Result = false;
	
	HKEY Key;
	if(RegOpenKeyEx(Hive, Path, 0, KEY_READ, &Key) == ERROR_SUCCESS)
	{
		ZeroMemory(Buffer, BufferSize);

		DWORD dwType = REG_SZ;
		DWORD dwDataLen = BufferSize - 1;
		
		Result = (RegQueryValueEx(Key, Item, NULL, &dwType, reinterpret_cast<unsigned char*>(Buffer), &dwDataLen) == ERROR_SUCCESS);
		
		RegCloseKey(Key);
	}
	return Result;
}

/*!
	@brief exe�t�@�C���̂���f�B���N�g���C�܂��͎w�肳�ꂽ�t�@�C�����̃t���p�X��Ԃ��D
	
	@param pDir [out] EXE�t�@�C���̂���f�B���N�g����Ԃ��ꏊ�D
		�\��_MAX_PATH�̃o�b�t�@��p�ӂ��Ă������ƁD
	@param szFile [in] �f�B���N�g�����Ɍ�������t�@�C�����D
	
	@author genta
	@date 2002.12.02 genta
*/
void GetExecutableDir( char* pDir, const char* szFile )
{
	if( pDir == NULL )
		return;
	
	char	szPath[_MAX_PATH];
	// sakura.exe �̃p�X���擾
	::GetModuleFileName( ::GetModuleHandle(NULL), szPath, sizeof(szPath) );
	if( szFile == NULL ){
		SplitPath_FolderAndFile( szPath, pDir, NULL );
	}
	else {
		char	szDir[_MAX_PATH];
		SplitPath_FolderAndFile( szPath, szDir, NULL );
		wsprintf( pDir, "%s\\%s", szDir, szFile );
	}
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
	@author genta
*/
HICON GetAppIcon( HINSTANCE hInst, int nResource, const char* szFile, bool bSmall )
{
	// �T�C�Y�̐ݒ�
	int size = ( bSmall ? 16 : 32 );

	char szPath[_MAX_PATH];
	
	GetExecutableDir( szPath, szFile );
	
	HICON hIcon;
	// �t�@�C������̓ǂݍ��݂��܂����݂�
	hIcon = (HICON)::LoadImage( NULL, szPath, IMAGE_ICON, size, size,
			LR_SHARED | LR_LOADFROMFILE );
	if( hIcon != NULL ){
		return hIcon;
	}
	
	//	�t�@�C������̓ǂݍ��݂Ɏ��s�����烊�\�[�X����擾
	hIcon = (HICON)::LoadImage( hInst, MAKEINTRESOURCE(nResource),
		IMAGE_ICON, size, size, LR_SHARED );
	
	return hIcon;
}

/*! fname�����΃p�X�̏ꍇ�́A���s�t�@�C���̃p�X����̑��΃p�X�Ƃ��ĊJ��
	@author Moca
	@date 2003.06.23
*/
FILE* fopen_absexe(const char* fname, const char* mode)
{
	if( _IS_REL_PATH( fname ) ){
		char path[_MAX_PATH];
		GetExecutableDir( path, fname );
		return fopen( path, mode );
	}
	return fopen( fname, mode );
	
}

/*! fname�����΃p�X�̏ꍇ�́A���s�t�@�C���̃p�X����̑��΃p�X�Ƃ��ĊJ��
	@author Moca
	@date 2003.06.23
*/
HFILE _lopen_absexe(LPCSTR fname, int mode)
{
	// fname�����΃p�X
	if( _IS_REL_PATH( fname ) ){
		char path[_MAX_PATH];
		GetExecutableDir( path, fname );
		return _lopen( path, mode );
	}
	return _lopen( fname, mode );
}


/*! �����������@�\�t��strncpy

	�R�s�[��̃o�b�t�@�T�C�Y������Ȃ��悤��strncpy����B
	�o�b�t�@���s������ꍇ�ɂ�2�o�C�g�����̐ؒf�����蓾��B
	������\0�͕t�^����Ȃ����A�R�s�[�̓R�s�[��o�b�t�@�T�C�Y-1�܂łɂ��Ă����B

	@param dst [in] �R�s�[��̈�ւ̃|�C���^
	@param dst_count [in] �R�s�[��̈�̃T�C�Y
	@param src [in] �R�s�[��
	@param src_count [in] �R�s�[���镶����̖���

	@retval ���ۂɃR�s�[���ꂽ�R�s�[��̈��1����w���|�C���^

	@author genta
	@date 2003.04.03 genta
*/
char *strncpy_ex(char *dst, size_t dst_count, const char* src, size_t src_count)
{
	if( src_count >= dst_count ){
		src_count = dst_count - 1;
	}
	memcpy( dst, src, src_count );
	return dst + src_count;
}

/*! @brief �f�B���N�g���̐[�����v�Z����

	�^����ꂽ�p�X������f�B���N�g���̐[�����v�Z����D
	�p�X�̋�؂��\�D���[�g�f�B���N�g�����[��0�ŁC�T�u�f�B���N�g������
	�[����1���オ���Ă����D
 
	@param path [in] �[���𒲂ׂ����t�@�C��/�f�B���N�g���̃t���p�X

	@date 2003.04.30 genta �V�K�쐬
*/
int CalcDirectoryDepth(const char* path)
{
	int depth = 0;
 
	//	�Ƃ肠����\�̐��𐔂���
	for( const char *p = path; *p != '\0'; ++p ){
		//	2�o�C�g�����͋�؂�ł͂Ȃ�
		if( _IS_SJIS_1(*(unsigned const char*)p)){ // unsigned��cast���Ȃ��Ɣ�������
			++p;
			if( *p == '\0' )
				break;
		}
		else if( *p == '\\' ){
			++depth;
			//	�t���p�X�ɂ͓����Ă��Ȃ��͂������O�̂���
			//	.\�̓J�����g�f�B���N�g���Ȃ̂ŁC�[���Ɋ֌W�Ȃ��D
			while( p[1] == '.' && p[2] == '\\' ){
				p += 2;
			}
		}
	}
 
	//	�␳
	//	�h���C�u���̓p�X�̐[���ɐ����Ȃ�
	if(( 'A' <= (path[0] & ~0x20)) && ((path[0] & ~0x20) <= 'Z' ) &&
		path[1] == ':' && path[2] == '\\' ){
		//�t���p�X
		--depth; // C:\ �� \ �̓��[�g�̋L���Ȃ̂ŊK�w�[���ł͂Ȃ�
	}
	else if( path[0] == '\\' ){
		if( path[1] == '\\' ){
			//	�l�b�g���[�N�p�X
			//	�擪��2�̓l�b�g���[�N��\���C���̎��̓z�X�g���Ȃ̂�
			//	�f�B���N�g���K�w�Ƃ͖��֌W
			depth -= 3;
		}
		else {
			//	�h���C�u�������̃t���p�X
			//	�擪��\�͑ΏۊO
			--depth;
		}
	}
	return depth;
}

//	From Here May 01, 2004 genta MutiMonitor
CMultiMonitor	g_MultiMonitor;

bool GetMonitorWorkRect(HWND hWnd, LPRECT rcDesktop)
{
	return g_MultiMonitor.GetMonitorWorkRect( hWnd, rcDesktop );
}
//	To Here May 01, 2004 genta


// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
/*!
	Shift,Ctrl,Alt�L�[��Ԃ̎擾

	@retval nIdx Shift,Ctrl,Alt�L�[���
	@date 2004.10.10 �֐���
*/
int getCtrlKeyState(void)
{
	int nIdx = 0;

	/* Shift�L�[��������Ă���Ȃ� */
	if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
		nIdx |= _SHIFT;
	}
	/* Ctrl�L�[��������Ă���Ȃ� */
	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
		nIdx |= _CTRL;
	}
	/* Alt�L�[��������Ă���Ȃ� */
	if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
		nIdx |= _ALT;
	}

	return nIdx;
}


/*[EOF]*/
