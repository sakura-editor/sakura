/*!	@file
	@brief 処理所要時間の計測クラス

	デバッグ目的で用いる

	@author Norio Nakatani
	@date 1998/03/06  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef _CRUNNINGTIMER_H_
#define _CRUNNINGTIMER_H_

#include <windows.h>
// RunningTimerで経過時間の測定を行う場合にはコメントを外してください
//#define TIME_MEASURE

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 処理所要時間の計測クラス

	定義の切り替えのみでタイマーのON/OFFを行えるようにするため，
	このクラスを直接使わず，後ろにあるMY_RUNNINGTIMERとMY_TRACETIMEを
	使うこと．

	@date 2002/10/16  genta WriteTrace及びマクロ追加
*/
class CRunningTimer
{
public:
	/*
	||  Constructors
	*/
	CRunningTimer( const char* Text = NULL);
	~CRunningTimer();

	/*
	|| 関数
	*/
	void Reset();
	DWORD Read();
	
	void WriteTrace(const char* msg = "") const;

protected:
	DWORD	m_nStartTime;
	char	m_szText[100];	//!< タイマー名
	int		m_nDeapth;	//!< このオブジェクトのネストの深さ

#ifdef _DEBUG
	static int m_nNestCount;
#endif
};

//	Oct. 16, 2002 genta
//	#ifdef _DEBUG～#endifで逐一囲まなくても簡単にタイマーのON/OFFを行うためのマクロ
#if defined(_DEBUG) && defined(TIME_MEASURE)
  #define MY_TRACETIME(c,m) (c).WriteTrace(m)
  #define MY_RUNNINGTIMER(c,m) CRunningTimer c(m)
#else
  #define MY_TRACETIME(c,m)
  #define MY_RUNNINGTIMER(c,m)
#endif

///////////////////////////////////////////////////////////////////////
#endif /* _CRUNNINGTIMER_H_ */



