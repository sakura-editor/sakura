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
#if (defined(_MSC_VER) && 1400 <= _MSC_VER) || (defined(__GNUC__) && 3 <= __GNUC__ )
#define DBPRINT_A(...)
#define DBPRINT_W(...)
#else
// Not support C99 variable macro
inline void DBPRINT_A( ... ){}
inline void DBPRINT_W( ... ){}
#endif
#endif

#ifdef _UNICODE
#define DBPRINT DBPRINT_W
#else
#define DBPRINT DBPRINT_A
#endif

#define DEBUG_TRACE DBPRINT


//RELEASE�łł��o�͂���� (RELEASE�ł̂ݔ�������o�O���Ď�����ړI)
#define RELPRINT_A DebugOutA
#define RELPRINT_W DebugOutW

#ifdef _UNICODE
#define RELPRINT DebugOutW
#else
#define RELPRINT DebugOutA
#endif

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

//��ʂ̌x����
#define DefaultBeep()   MessageBeep(MB_OK)

//�G���[�F�ԊۂɁu�~�v[OK]
int ErrorMessage  (HWND hwnd, LPCTSTR format, ...);
int ErrorMessage_A(HWND hwnd, LPCSTR  format, ...);
//(TOPMOST)
int TopErrorMessage  (HWND hwnd, LPCTSTR format, ...);
int TopErrorMessage_A(HWND hwnd, LPCSTR format, ...);
#define ErrorBeep()     MessageBeep(MB_ICONSTOP)

//�x���F�O�p�Ɂu�I�v[OK]
int WarningMessage   (HWND hwnd, LPCTSTR format, ...);
int WarningMessage_A (HWND hwnd, LPCSTR  format, ...);
int TopWarningMessage(HWND hwnd, LPCTSTR format, ...);
#define WarningBeep()   MessageBeep(MB_ICONEXCLAMATION)

//���F�ۂɁui�v[OK]
int InfoMessage   (HWND hwnd, LPCTSTR format, ...);
int InfoMessage_A (HWND hwnd, LPCSTR  format, ...);
int TopInfoMessage(HWND hwnd, LPCTSTR format, ...);
#define InfoBeep()      MessageBeep(MB_ICONINFORMATION)

//�m�F�F�����o���́u�H�v [�͂�][������] �߂�l:IDYES,IDNO
int ConfirmMessage   (HWND hwnd, LPCTSTR format, ...);
int ConfirmMessage_A (HWND hwnd, LPCSTR  format, ...);
int TopConfirmMessage(HWND hwnd, LPCTSTR format, ...);
#define ConfirmBeep()   MessageBeep(MB_ICONQUESTION)

//���̑����b�Z�[�W�\���p�{�b�N�X[OK]
int OkMessage  (HWND hwnd, LPCTSTR format, ...);
int OkMessage_A(HWND hwnd, LPCSTR  format, ...);
int TopOkMessage  (HWND hwnd, LPCTSTR format, ...);
int TopOkMessage_A(HWND hwnd, LPCSTR format, ...);

//�^�C�v�w�胁�b�Z�[�W�\���p�{�b�N�X
int CustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...);
int CustomMessage_A(HWND hwnd, UINT uType, LPCSTR format, ...);
//(TOPMOST)
int TopCustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...);
int TopCustomMessage_A(HWND hwnd, UINT uType, LPCSTR format, ...);

//��҂ɋ����ė~�����G���[
int PleaseReportToAuthor(HWND hwnd, LPCTSTR format, ...);



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



