/*!	@file
	@brief �f�o�b�O�p�֐�

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <windows.h>
#include "global.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   ���b�Z�[�W�o�́F����                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
SAKURA_CORE_API void AssertError( LPCTSTR pszFile, long nLine, BOOL bIsError );
SAKURA_CORE_API void DebugOutW( LPCWSTR lpFmt, ...);
SAKURA_CORE_API void DebugOutA( LPCSTR lpFmt, ...);
#ifdef _UNICODE
#define DebugOut DebugOutW
#else
#define DebugOut DebugOutA
#endif

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 ���b�Z�[�W�{�b�N�X�F����                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//2007.10.02 kobake ���b�Z�[�W�{�b�N�X�̎g�p�̓f�o�b�O���Ɍ���Ȃ��̂ŁA�uDebug�`�v�Ƃ������O��p�~

//�e�L�X�g���`�@�\�t��MessageBox
SAKURA_CORE_API int VMessageBoxF_W( HWND hwndOwner, UINT uType, LPCWSTR lpCaption, LPCWSTR lpText, va_list& v );
SAKURA_CORE_API int VMessageBoxF_A( HWND hwndOwner, UINT uType, LPCSTR  lpCaption, LPCSTR  lpText, va_list& v );
SAKURA_CORE_API int MessageBoxF_W ( HWND hwndOwner, UINT uType, LPCWSTR lpCaption, LPCWSTR lpText, ... );
SAKURA_CORE_API int MessageBoxF_A ( HWND hwndOwner, UINT uType, LPCSTR  lpCaption, LPCSTR  lpText, ... );

//TCHAR
#ifdef _UNICODE
	#define VMessageBoxF VMessageBoxF_W
	#define MessageBoxF  MessageBoxF_W
#else
	#define VMessageBoxF VMessageBoxF_A
	#define MessageBoxF  MessageBoxF_A
#endif

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 �f�o�b�O�p���b�Z�[�W�o��                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*
	MYTRACE�̓����[�X���[�h�ł̓R���p�C���G���[�ƂȂ�悤�ɂ��Ă���̂ŁC
	MYTRACE���g���ꍇ�ɂ͕K��#ifdef _DEBUG �` #endif �ň͂ޕK�v������D
*/
#ifdef _DEBUG
	#define MYTRACE DebugOut
	#define MYTRACE_A DebugOutA
	#define MYTRACE_W DebugOutW
#endif
#ifndef _DEBUG
	#define MYTRACE   Do_not_use_the_MYTRACE_function_if_release_mode
	#define MYTRACE_A Do_not_use_the_MYTRACE_A_function_if_release_mode
	#define MYTRACE_W Do_not_use_the_MYTRACE_A_function_if_release_mode
#endif

//#ifdef _DEBUG�`#endif�ň͂܂Ȃ��Ă��ǂ���
#ifdef _DEBUG
#define DBPRINT_A DebugOutA
#define DBPRINT_W DebugOutW
#else
#define DBPRINT_A(...)
#define DBPRINT_W(...)
#endif

#ifdef _UNICODE
#define DBPRINT DBPRINT_W
#else
#define DBPRINT DBPRINT_A
#endif

#define DEBUG_TRACE DBPRINT


//RELEASE�łł��o�͂���� (RELEASE�ł̂ݔ�������o�O���Ď�����ړI)
#define RELPRINT_A DebugOutA

#define MYASSERT AssertError


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �f�o�b�O�p���b�Z�[�W�{�b�N�X                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//���ȈՔ�  2007.09.29 kobake �쐬
void DBMSG_IMP(const ACHAR* msg); //!< ���b�Z�[�W�{�b�N�X��\���B�L���v�V�����ɂ�exe���B

#ifdef _DEBUG
#define DBMSG DBMSG_IMP
#else
#define DBMSG(S)
#endif

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                ���[�U�p���b�Z�[�W�{�b�N�X                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//$$�����FDebug.h�ȊO�̒u���ꏊ���l����

//�f�o�b�O�p���b�Z�[�W�{�b�N�X
#define MYMESSAGEBOX MessageBoxF
#define MYMESSAGEBOX_A MessageBoxF_A

//�G���[�F�ԊۂɁu�~�v
#define ErrorMessage(hwnd, format, ...)			MessageBoxF		(hwnd, MB_OK | MB_ICONSTOP						, GSTR_APPNAME,   format, __VA_ARGS__)
#define ErrorMessage_A(hwnd, format, ...)		MessageBoxF_A	(hwnd, MB_OK | MB_ICONSTOP						, GSTR_APPNAME_A, format, __VA_ARGS__)
//(TOPMOST)
#define TopErrorMessage(hwnd, format, ...)		MessageBoxF		(hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST			, GSTR_APPNAME,   format, __VA_ARGS__)
#define TopErrorMessage_A(hwnd, format, ...)	MessageBoxF_A	(hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST			, GSTR_APPNAME_A, format, __VA_ARGS__)
#define ErrorBeep()								MessageBeep(MB_ICONSTOP)

//�x���F�O�p�Ɂu�I�v
#define WarningMessage(hwnd, format, ...)		MessageBoxF		(hwnd, MB_OK | MB_ICONEXCLAMATION				, GSTR_APPNAME,   format, __VA_ARGS__)
#define WarningMessage_A(hwnd, format, ...)		MessageBoxF_A	(hwnd, MB_OK | MB_ICONEXCLAMATION				, GSTR_APPNAME_A, format, __VA_ARGS__)
#define TopWarningMessage(hwnd, format, ...)	MessageBoxF		(hwnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST	, GSTR_APPNAME,   format, __VA_ARGS__)
#define WarningBeep()							MessageBeep(MB_ICONEXCLAMATION)

//���F�ۂɁui�v
#define InfoMessage(hwnd, format, ...)			MessageBoxF		(hwnd, MB_OK | MB_ICONINFORMATION				, GSTR_APPNAME,   format, __VA_ARGS__)
#define InfoMessage_A(hwnd, format, ...)		MessageBoxF_A	(hwnd, MB_OK | MB_ICONINFORMATION				, GSTR_APPNAME_A, format, __VA_ARGS__)
#define TopInfoMessage(hwnd, format, ...)		MessageBoxF		(hwnd, MB_OK | MB_ICONINFORMATION | MB_TOPMOST	, GSTR_APPNAME,   format, __VA_ARGS__)
#define InfoBeep()								MessageBeep(MB_ICONINFORMATION)

//���̑����b�Z�[�W�\���p�{�b�N�X
#define OkMessage(hwnd, format, ...)			MessageBoxF		(hwnd, MB_OK									, GSTR_APPNAME,   format, __VA_ARGS__)
#define OkMessage_A(hwnd, format, ...)			MessageBoxF_A	(hwnd, MB_OK									, GSTR_APPNAME_A, format, __VA_ARGS__)
//(TOPMOST)
#define TopOkMessage(hwnd, format, ...)			MessageBoxF		(hwnd, MB_OK | MB_TOPMOST						, GSTR_APPNAME,   format, __VA_ARGS__)
#define TopOkMessage_A(hwnd, format, ...)		MessageBoxF_A	(hwnd, MB_OK | MB_TOPMOST						, GSTR_APPNAME_A, format, __VA_ARGS__)

//��҂ɋ����ė~�����G���[
#define PleaseReportToAuthor(hwnd, format, ...)	MessageBoxF		(hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST	, _T("��҂ɋ����ė~�����G���["), format, __VA_ARGS__)


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �t���O��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#ifdef _DEBUG
	//!	�ݒ肵�Ă���ꏊ�͂��邪�C�Q�Ƃ��Ă���ꏊ���Ȃ��ϐ�
	SAKURA_CORE_API extern int gm_ProfileOutput;
#endif



#include "debug/Debug2.h"
#include "debug/Debug3.h"

///////////////////////////////////////////////////////////////////////
#endif /* _DEBUG_H_ */



