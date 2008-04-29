/*!	@file
	@brief �f�o�b�O�p�֐�

	@author Norio Nakatani

	@date 2001/06/23 N.Nakatani DebugOut()�ɔ����`�ȏC��
	@date 2002/01/17 aroka �^�̏C��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "debug/Debug.h"
#include <tchar.h>
#include "global.h"

#ifdef _DEBUG
	int gm_ProfileOutput = 0;
#endif

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

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   ���b�Z�[�W�o�́F����                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include "debug/Debug3.h"

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
	auto_vsprintf( szText, lpFmt, argList );

	//�o��
	::OutputDebugStringW( szText );
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
	::tchar_vsprintf( szText, lpFmt, argList );

	//�o��
	::OutputDebugStringA( szText );
#ifdef USE_DEBUGMON
	DebugMonitor_Output(NULL, to_wchar(szText));
#endif

	//�E�F�C�g
	::Sleep(1);	// Norio Nakatani, 2001/06/23 ��ʂɃg���[�X����Ƃ��̂��߂�

	va_end(argList);
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 ���b�Z�[�W�{�b�N�X�F����                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "global.h"
#include "window/CEditWnd.h"
HWND GetMessageBoxOwner(HWND hwndOwner)
{
	if(hwndOwner==NULL && g_pcEditWnd){
		return g_pcEditWnd->GetHwnd();
	}
	else{
		return hwndOwner;
	}
}

/*!
	�����t�����b�Z�[�W�{�b�N�X

	�����ŗ^����ꂽ�����_�C�A���O�{�b�N�X�ŕ\������D
	�f�o�b�O�ړI�ȊO�ł��g�p�ł���D
*/
SAKURA_CORE_API int VMessageBoxF_W(
	HWND		hwndOwner,	//!< [in] �I�[�i�[�E�B���h�E�̃n���h��
	UINT		uType,		//!< [in] ���b�Z�[�W�{�b�N�X�̃X�^�C�� (MessageBox�Ɠ����`��)
	LPCWSTR		lpCaption,	//!< [in] ���b�Z�[�W�{�b�N�X�̃^�C�g��
	LPCWSTR		lpText,		//!< [in] �\������e�L�X�g�Bprintf�d�l�̏����w�肪�\�B
	va_list&	v			//!< [in/out] �������X�g
)
{
	hwndOwner=GetMessageBoxOwner(hwndOwner);
	//���`
	static WCHAR szBuf[16000];
	auto_vsprintf(szBuf,lpText,v);
	//API�Ăяo��
#ifdef _UNICODE
	return ::MessageBoxW( hwndOwner, szBuf, lpCaption, uType);
#else
	return ::MessageBoxA( hwndOwner, to_achar(szBuf), to_achar(lpCaption), uType);
#endif
}

SAKURA_CORE_API int VMessageBoxF_A(
	HWND		hwndOwner,	//!< [in] �I�[�i�[�E�B���h�E�̃n���h��
	UINT		uType,		//!< [in] ���b�Z�[�W�{�b�N�X�̃X�^�C�� (MessageBox�Ɠ����`��)
	LPCSTR		lpCaption,	//!< [in] ���b�Z�[�W�{�b�N�X�̃^�C�g��
	LPCSTR		lpText,		//!< [in] �\������e�L�X�g�Bprintf�d�l�̏����w�肪�\�B
	va_list&	v			//!< [in/out] �������X�g
)
{
	hwndOwner=GetMessageBoxOwner(hwndOwner);
	//���`
	static ACHAR szBuf[16000];
	tchar_vsprintf(szBuf,lpText,v);
	//API�Ăяo��
	return ::MessageBoxA( hwndOwner, szBuf, lpCaption, uType);
}

SAKURA_CORE_API int MessageBoxF_W( HWND hwndOwner, UINT uType, LPCWSTR lpCaption, LPCWSTR lpText, ... )
{
	va_list v;
	va_start(v,lpText);
	int nRet = VMessageBoxF_W(hwndOwner, uType, lpCaption, lpText, v);
	va_end(v);
	return nRet;
}
SAKURA_CORE_API int MessageBoxF_A( HWND hwndOwner, UINT uType, LPCSTR lpCaption, LPCSTR lpText, ... )
{
	va_list v;
	va_start(v,lpText);
	int nRet = VMessageBoxF_A(hwndOwner, uType, lpCaption, lpText, v);
	va_end(v);
	return nRet;
}






void AssertError( LPCTSTR pszFile, long nLine, BOOL bIsError )
{
	if( !bIsError ){
		TCHAR psz[1000];
		::auto_sprintf(psz, _T("%ts\n�s %d ��ASSERT�������`�F�b�N�G���["), pszFile, nLine );
		::MessageBox( NULL, psz, _T("MYASSERT"), MB_OK );
	}
	return;
}


//! ���b�Z�[�W�{�b�N�X��\���B�L���v�V�����ɂ�exe���B
void DBMSG_IMP(const ACHAR* msg)
{
	//EXE�����擾�B(����Ăяo�����̂݌v�Z)
	static const char* exe=NULL;
	if(exe==NULL){
		static char exepath[_MAX_PATH];
		GetModuleFileNameA(NULL,exepath,_countof(exepath));
		const char* p=strrchr(exepath,'\\');
		if(p)exe=p+1;
		else exe=exepath;
	}

	//���b�Z�[�W
	MessageBoxA(NULL,msg,exe,MB_OK);
}



