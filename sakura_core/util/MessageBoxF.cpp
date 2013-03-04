/*!	@file
	@brief MessageBox�p�֐�

	@author Norio Nakatani

	@date 2002/01/17 aroka �^�̏C��
	@date 2013/03/03 Uchi Debug1.cpp���番��
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
#include "MessageBoxF.h"
#include "window/CEditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 ���b�Z�[�W�{�b�N�X�F����                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
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
SAKURA_CORE_API int VMessageBoxF(
	HWND		hwndOwner,	//!< [in] �I�[�i�[�E�B���h�E�̃n���h��
	UINT		uType,		//!< [in] ���b�Z�[�W�{�b�N�X�̃X�^�C�� (MessageBox�Ɠ����`��)
	LPCTSTR		lpCaption,	//!< [in] ���b�Z�[�W�{�b�N�X�̃^�C�g��
	LPCTSTR		lpText,		//!< [in] �\������e�L�X�g�Bprintf�d�l�̏����w�肪�\�B
	va_list&	v			//!< [in/out] �������X�g
)
{
	hwndOwner=GetMessageBoxOwner(hwndOwner);
	//���`
	static TCHAR szBuf[16000];
#ifdef _UNICODE
	tchar_vsnwprintf_s(szBuf,_countof(szBuf),lpText,v);
#else
	tchar_vsnprintf_s(szBuf,_countof(szBuf),lpText,v);
#endif
	//API�Ăяo��
	return ::MessageBox( hwndOwner, szBuf, lpCaption, uType);
}

SAKURA_CORE_API int MessageBoxF( HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, ... )
{
	va_list v;
	va_start(v,lpText);
	int nRet = VMessageBoxF(hwndOwner, uType, lpCaption, lpText, v);
	va_end(v);
	return nRet;
}


//�G���[�F�ԊۂɁu�~�v[OK]
int ErrorMessage  (HWND hwnd, LPCTSTR format, ...){       va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP                     , GSTR_APPNAME,   format, p);va_end(p);return n;}
//(TOPMOST)
int TopErrorMessage  (HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST        , GSTR_APPNAME,   format, p);va_end(p);return n;}

//�x���F�O�p�Ɂui�v
int WarningMessage   (HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONEXCLAMATION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopWarningMessage(HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//���F�ۂɁui�v
int InfoMessage   (HWND hwnd, LPCTSTR format, ...){       va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONINFORMATION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopInfoMessage(HWND hwnd, LPCTSTR format, ...){       va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONINFORMATION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//�m�F�F�����o���́u�H�v �߂�l:ID_YES,ID_NO
int ConfirmMessage   (HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNO | MB_ICONQUESTION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopConfirmMessage(HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNO | MB_ICONQUESTION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//���̑����b�Z�[�W�\���p�{�b�N�X
int OkMessage(HWND hwnd, LPCTSTR format, ...){            va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK                                   , GSTR_APPNAME,   format, p);va_end(p);return n;}
//(TOPMOST)
int TopOkMessage(HWND hwnd, LPCTSTR format, ...){         va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_TOPMOST                      , GSTR_APPNAME,   format, p);va_end(p);return n;}

//�^�C�v�w�胁�b�Z�[�W�\���p�{�b�N�X
int CustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...){            va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, uType                   , GSTR_APPNAME,   format, p);va_end(p);return n;}
//(TOPMOST)
int TopCustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...){         va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, uType | MB_TOPMOST      , GSTR_APPNAME,   format, p);va_end(p);return n;}

//��҂ɋ����ė~�����G���[
int PleaseReportToAuthor(HWND hwnd, LPCTSTR format, ...){ va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST, _T("sakura: ��҂ɋ����ė~�����G���["), format, p);va_end(p);return n;}



