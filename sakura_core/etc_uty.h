//	$Id$
/*!	@file
	@brief ���ʊ֐��Q

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001-2002, genta
	Copyright (C) 2001, shoji masami, Stonee, MIK
	Copyright (C) 2002, aroka, hor, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _ETC_UTY_H_
#define _ETC_UTY_H_

#include <windows.h>
#include "global.h"
//#include "CMemory.h"// 2002/2/3 aroka �w�b�_�y�ʉ�
#include <shlobj.h>
//#include "CEol.h"// 2002/2/3 aroka �w�b�_�y�ʉ�
//#include "CBregexp.h"	//	Jun. 26, 2001 genta  2002/2/3 aroka �w�b�_�y�ʉ�
#include "CHtmlHelp.h"	//	Jul.  6, 2001 genta
class CMemory;// 2002/2/3 aroka �w�b�_�y�ʉ�
class CEOL;// 2002/2/3 aroka �w�b�_�y�ʉ�
class CBregexp;// 2002/2/3 aroka �w�b�_�y�ʉ�

//@@@ 2002.2.9 YAZAKI CShareData�Ɉړ�
//SAKURA_CORE_API const char* MyGetDateFormat( SYSTEMTIME& systime, char* pszDest, int nDestLen, int nDateFormatType, const char* pszDateFormat );/* ���t���t�H�[�}�b�g */
//SAKURA_CORE_API const char* MyGetTimeFormat( SYSTEMTIME &systime, char* pszDest, int nDestLen, int nTimeFormatType, const char* pszTimeFormat );/* �������t�H�[�}�b�g */
SAKURA_CORE_API void CutLastYenFromDirectoryPath( char* );/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� "c:\\"���̃��[�g�͎�菜���Ȃ�*/
SAKURA_CORE_API void AddLastYenFromDirectoryPath( char* );/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
SAKURA_CORE_API int LimitStringLengthB( const char*, int, int, CMemory& );/* �f�[�^���w��o�C�g���ȓ��ɐ؂�l�߂� */
SAKURA_CORE_API const char* GetNextLimitedLengthText( const char*, int, int, int*, int* );/* �w�蒷�ȉ��̃e�L�X�g�ɐ؂蕪���� */
//SAKURA_CORE_API const char* GetNextLine( const char*, int, int*, int*, BOOL*, BOOL );/* CRLF�ŋ�؂���u�s�v��Ԃ��BCRLF�͍s���ɉ����Ȃ� */
SAKURA_CORE_API const char* GetNextLine( const char*, int, int*, int*, CEOL* );/* CR0LF0,CRLF,LFCR,LF,CR�ŋ�؂���u�s�v��Ԃ��B���s�R�[�h�͍s���ɉ����Ȃ� */
SAKURA_CORE_API void GetLineColm( const char*, int*, int* );
SAKURA_CORE_API bool IsFilePath( const char*, int*, int*, bool = true );
SAKURA_CORE_API bool IsFileExists(const char* path, bool bFileOnly = false);
SAKURA_CORE_API BOOL IsURL( const char*, int, int* );/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
SAKURA_CORE_API BOOL IsMailAddress( const char*, int, int* );	/* ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ� */
//#ifdef COMPILE_COLOR_DIGIT
SAKURA_CORE_API int IsNumber( const char*, int, int );/* ���l�Ȃ炻�̒�����Ԃ� */	//@@@ 2001.02.17 by MIK
//#endif
SAKURA_CORE_API void ActivateFrameWindow( HWND );	/* �A�N�e�B�u�ɂ��� */
SAKURA_CORE_API BOOL GetSystemResources( int*, int*, int* );	/* �V�X�e�����\�[�X�𒲂ׂ� */
SAKURA_CORE_API BOOL CheckSystemResources( const char* );	/* �V�X�e�����\�[�X�̃`�F�b�N */
//SAKURA_CORE_API BOOL CheckWindowsVersion( const char* pszAppName );	/* Windows�o�[�W�����̃`�F�b�N */
// Jul. 5, 2001 shoji masami
//SAKURA_CORE_API bool CheckWindowsVersionNT( void );	/* NT�v���b�g�t�H�[�����ǂ��� */
SAKURA_CORE_API void GetAppVersionInfo( HINSTANCE, int, DWORD*, DWORD* );	/* ���\�[�X���琻�i�o�[�W�����̎擾 */
SAKURA_CORE_API void SplitPath_FolderAndFile( const char*, char*, char* );	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
SAKURA_CORE_API BOOL GetAbsolutePath( const char*, char*, BOOL );	/* ���΃p�X����΃p�X */
SAKURA_CORE_API BOOL GetLongFileName( const char*, char* );	/* �����O�t�@�C�������擾���� */
SAKURA_CORE_API char* GetHelpFilePath( char* , unsigned int nMaxLen = _MAX_PATH );	/* �w���v�t�@�C���̃t���p�X��Ԃ� */// 20020119 aroka
SAKURA_CORE_API BOOL CheckEXT( const char*, const char* );	/* �g���q�𒲂ׂ� */
SAKURA_CORE_API char* my_strtok( char*, int, int*, char* );
/* Shell Interface�n(?) */
SAKURA_CORE_API BOOL SelectDir(HWND, const char*, const char*, char* );	/* �t�H���_�I���_�C�A���O */
SAKURA_CORE_API ITEMIDLIST* CreateItemIDList( const char* );	/* �p�X���ɑ΂���A�C�e���h�c���X�g���擾���� */
SAKURA_CORE_API BOOL DeleteItemIDList( ITEMIDLIST* );/* �A�C�e���h�c���X�g���폜���� */
SAKURA_CORE_API BOOL ResolveShortcutLink(HWND hwnd, LPCSTR lpszLinkFile, LPSTR lpszPath);/* �V���[�g�J�b�g(.lnk)�̉��� */

/*
||	�������̃��[�U�[������\�ɂ���
||	�u���b�L���O�t�b�N(?)(���b�Z�[�W�z��)
*/
SAKURA_CORE_API BOOL BlockingHook( HWND hwndDlgCancel );

/*�@�\�ԍ��ɑΉ������w���v�g�s�b�NID��Ԃ�*/
SAKURA_CORE_API int FuncID_To_HelpContextID( int nFuncID );	//Stonee, 2001/02/23

//	Jun. 26, 2001 genta
//!	���K�\�����C�u�����̃o�[�W�����擾
SAKURA_CORE_API bool CheckRegexpVersion( HWND hWnd, int nCmpId, bool bShowMsg = false );
SAKURA_CORE_API bool CheckRegexpSyntax( const char* szPattern, HWND hWnd, bool bShowMessage, int nOption = -1 );// 2002/2/1 hor�ǉ�
SAKURA_CORE_API bool InitRegexp( HWND hWnd, CBregexp& rRegexp, bool bShowMessage );

SAKURA_CORE_API HWND OpenHtmlHelp( HWND hWnd, LPCSTR szFile, UINT uCmd, DWORD data,bool msgflag = true);
SAKURA_CORE_API DWORD NetConnect ( const char strNetWorkPass[] );

SAKURA_CORE_API int cescape(const char* org, char* out, char cesc, char cwith);
SAKURA_CORE_API int cescape_j(const char* org, char* out, char cesc, char cwith);

/* �w���v�̖ڎ���\�� */
SAKURA_CORE_API void ShowWinHelpContents( HWND hwnd, LPCTSTR lpszHelp );

/*!	&�̓�d��
	���j���[�Ɋ܂܂��&��&&�ɒu��������
	@author genta
	@date 2002/01/30 cescape�Ɋg�����C
*/
inline void dupamp(const char* org, char* out)
{	cescape( org, out, '&', '&' ); }
///////////////////////////////////////////////////////////////////////

/* �J���[�������C���f�b�N�X�ԍ��̕ϊ� */	//@@@ 2002.04.30
SAKURA_CORE_API int GetColorIndexByName( const char *name );
SAKURA_CORE_API const char* GetColorNameByIndex( int index );

//	Sep. 10, 2002 genta CWSH.cpp����̈ړ��ɔ����ǉ�
bool ReadRegistry(HKEY Hive, char const *Path, char const *Item, char *Buffer, unsigned BufferSize);

#endif /* _ETC_UTY_H_ */


/*[EOF]*/
