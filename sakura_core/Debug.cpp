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
#include "debug.h"
#include <tchar.h>

#ifdef _DEBUG
	int gm_ProfileOutput = 0;
#endif


//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif


/*! @brief �����t���f�o�b�K�o��

	@param[in] lpFmt printf�̏����t��������

	�����ŗ^����ꂽ����DebugString�Ƃ��ďo�͂���D
*/
void DebugOut( LPCTSTR lpFmt, ...)
{
	static TCHAR szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
	::wvsprintf( szText, lpFmt, argList );
	::OutputDebugString( szText );

	::Sleep(1);	// Norio Nakatani, 2001/06/23 ��ʂɃg���[�X����Ƃ��̂��߂�

	va_end(argList);
	return;
}



/*! @brief �����t�����b�Z�[�W�{�b�N�X

	@param[in] hWndParent �e�E�B���h�E�̃n���h��
	@param[in] nStyle ���b�Z�[�W�{�b�N�X�̌`��(�A�C�R���C�{�^���Ȃǂ�MB_xx�萔�̘_���a�ŗ^����)
	@param[in] pszTitle �_�C�A���O�{�b�N�X�̃^�C�g��
	@param[in] lpFmt printf�̏����w�蕶����

	�����ŗ^����ꂽ�����_�C�A���O�{�b�N�X�ŕ\������D
	�f�o�b�O�ړI�ȊO�ł��g�p�ł���D
*/
int DebugOutDialog(
	HWND	hWndParent,
	UINT	nStyle,
	LPCTSTR	pszTitle,
	LPCTSTR lpFmt,
	...
)
{
	static TCHAR szText[16000];
	va_list	argList;
	int		nRet;
	va_start(argList, lpFmt);
	::wvsprintf( szText, lpFmt, argList );
	nRet = ::MessageBox( hWndParent,  szText, pszTitle, nStyle );
	va_end(argList);
	return nRet;
}


//void MYASSERT( LPCTSTR pszFile, long nLine, BOOL bIsError )
//{
//	AssertError( bIsError, pszFile, nLine );
//	return;
//}


void AssertError( LPCTSTR pszFile, long nLine, BOOL bIsError )
{
	if( !bIsError ){
		TCHAR psz[1000];
		::wsprintf(psz, _T("%s\n�s %d ��ASSERT�������`�F�b�N�G���["), pszFile, nLine );
		::MessageBox( NULL, psz, _T("MYASSERT"), MB_OK );
	}
	return;
}


/*[EOF]*/
