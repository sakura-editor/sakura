//	$Id$
/************************************************************************

	CRunningTimer.h

    処理所要時間の計測クラス
	windows用です
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/3/6  新規作成


************************************************************************/


class CRunningTimer;

#ifndef _CRUNNINGTIMER_H_
#define _CRUNNINGTIMER_H_

#include <windows.h>


/*-----------------------------------------------------------------------
クラスの宣言
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
    || 関数
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
