/*!	@file
	@brief �f�o�b�O�p�֐�

	@author Norio Nakatani

	@date 2001/06/23 N.Nakatani DebugOut()�ɔ����`�ȏC��
	@date 2002/01/17 aroka �^�̏C��
	@date 2013/03/03 Uchi MessageBox�p�֐��𕪗�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdarg.h>
#include <tchar.h>
#include "debug/Debug1.h"
#include "debug/Debug3.h"

#if 0
//�f�o�b�O�E�H�b�`�p�̌^
struct TestArrayA{ char    a[100]; };
struct TestArrayW{ wchar_t a[100]; };
struct TestArrayI{ int     a[100]; };
void Test()
{
	TestArrayA a; a.a[0]=0;
	TestArrayW w; w.a[0]=0;
	TestArrayI i; i.a[0]=0;
}
#endif

#if defined(_DEBUG) || defined(USE_RELPRINT)

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   ���b�Z�[�W�o�́F����                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! @brief �����t���f�o�b�K�o��

	@param[in] lpFmt printf�̏����t��������

	�����ŗ^����ꂽ����DebugString�Ƃ��ďo�͂���D
*/
#ifdef _UNICODE
void DebugOutW( LPCWSTR lpFmt, ...)
{
	//���`
	static WCHAR szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
	int ret = tchar_vsnwprintf_s( szText, _countof(szText), lpFmt, argList );

	//�o��
	::OutputDebugStringW( szText );
	if( -1 == ret ){
		::OutputDebugStringW( L"(�؂�̂Ă܂���...)\n" );
	}
#ifdef USE_DEBUGMON
	DebugMonitor_Output(NULL, to_wchar(szText));
#endif

	//�E�F�C�g
	::Sleep(1);	// Norio Nakatani, 2001/06/23 ��ʂɃg���[�X����Ƃ��̂��߂�

	va_end(argList);
	return;
}
#endif

void DebugOutA( LPCSTR lpFmt, ...)
{
	//���`
	static CHAR szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
	int ret = tchar_vsnprintf_s( szText, _countof(szText), lpFmt, argList );

	//�o��
	::OutputDebugStringA( szText );
	if( -1 == ret ){
		::OutputDebugStringA( "(�؂�̂Ă܂���...)\n" );
	}
#ifdef USE_DEBUGMON
	DebugMonitor_Output(NULL, to_wchar(szText));
#endif

	//�E�F�C�g
	::Sleep(1);	// Norio Nakatani, 2001/06/23 ��ʂɃg���[�X����Ƃ��̂��߂�

	va_end(argList);
	return;
}

#endif	// _DEBUG || USE_RELPRINT


