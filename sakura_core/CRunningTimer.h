//	$Id$
/************************************************************************

	CRunningTimer.h

    �������v���Ԃ̌v���N���X
	windows�p�ł�
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/3/6  �V�K�쐬


************************************************************************/


class CRunningTimer;

#ifndef _CRUNNINGTIMER_H_
#define _CRUNNINGTIMER_H_

#include <windows.h>


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
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
