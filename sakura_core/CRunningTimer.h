//	$Id$
/*!	@file
	処理所要時間の計測クラス
	
	デバッグ目的で用いる
	
	@author Norio Nakatani
	@date 1998/03/06  新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


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
