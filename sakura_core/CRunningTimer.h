//	$Id$
/*!	@file
	@brief �������v���Ԃ̌v���N���X

	�f�o�b�O�ړI�ŗp����

	@author Norio Nakatani
	@date 1998/03/06  �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CRunningTimer;

#ifndef _CRUNNINGTIMER_H_
#define _CRUNNINGTIMER_H_

#include <windows.h>


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �������v���Ԃ̌v���N���X
*/
class CRunningTimer
{
public:
	/*
	||  Constructors
	*/
//	CRunningTimer();
	CRunningTimer( const char* );
	~CRunningTimer();

	/*
	|| �֐�
	*/
	void Reset();
	DWORD Read();

protected:
	DWORD	m_nStartTime;
	char	m_szText[100];
	static int m_nNestCount;
};




///////////////////////////////////////////////////////////////////////
#endif /* _CRUNNINGTIMER_H_ */


/*[EOF]*/
