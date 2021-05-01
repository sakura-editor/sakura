/*!	@file
	@brief 処理所要時間の計測クラス

	デバッグ目的で用いる

	@author Norio Nakatani
	@date 1998/03/06  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta
	Copyright (C) 2018-2021, Sakura Editor Organization

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

#ifndef SAKURA_CRUNNINGTIMER_B4A1B7C4_EA83_41F2_9132_21DE3A57470D_H_
#define SAKURA_CRUNNINGTIMER_B4A1B7C4_EA83_41F2_9132_21DE3A57470D_H_
#pragma once

#include <windows.h>
#include <string>
#include <string_view>
#include <chrono>

// 本番コードに組み込まれている時間計測の結果を出力ペインに出したい時にコメントを外して下さい
//#define TIME_MEASURE

#if defined(TIME_MEASURE)
  #define MY_TRACETIME(c,m) (c).WriteTrace(m)
  #define MY_RUNNINGTIMER(c,m) CRunningTimer c(m, CRunningTimer::OutputStyle::Conventional)
#else
  #define MY_TRACETIME(c,m)
  #define MY_RUNNINGTIMER(c,m)
#endif

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 処理所要時間の計測クラス

	本番コードに時間計測箇所を組み込む場合には、
	このクラスを直接使わず代わりにMY_RUNNINGTIMER,MY_TRACETIMEマクロを使用して下さい。
	(計測が必要な時以外で出力ペインを計測ログが埋め尽くさないようにするため)

	@date 2002/10/16  genta WriteTrace及びマクロ追加
*/
class CRunningTimer
{
public:
	enum class OutputStyle { Conventional, Markdown };

	/*
	||  Constructors
	*/
	CRunningTimer( std::wstring_view name = L"", OutputStyle style = OutputStyle::Markdown );
	~CRunningTimer();

	/*
	|| 関数
	*/
	void Reset();
	DWORD Read();
	void WriteTrace( std::wstring_view msg = L"" );

protected:
	enum class TraceType { Normal, Enter, ExitScope };
	typedef std::chrono::high_resolution_clock::time_point TimePoint;

	static TimePoint m_initialTime;				// タイムスタンプ基準時間

	const size_t	m_nNameOutputWidthMin;		// タイマー名最低出力幅(文字数)

	TimePoint		m_startTime;				// 計測開始時間
	TimePoint		m_lastTime;					// 最後に出力した時間
	std::wstring	m_timerName;				// タイマー名
	int				m_nDepth;					// このオブジェクトのネストの深さ
	OutputStyle		m_outputStyle;				// ログの出力形式
	size_t			m_nNameOutputWidth;			// タイマー名出力幅(文字数)
	LARGE_INTEGER	m_nPerformanceFrequency;	// 計時用

	enum class OutputTiming { Normal, Enter };

	static double GetElapsedTimeInSeconds( TimePoint from, TimePoint to );
	TimePoint GetTime() const;
	void OutputHeader() const;
	void OutputFooter() const;
	void OutputTrace( TimePoint currentTime, TraceType traceType, std::wstring_view msg = L"" ) const;
	void Output( std::wstring_view fmt, ... ) const;

	static int m_nNestCount;
};
#endif /* SAKURA_CRUNNINGTIMER_B4A1B7C4_EA83_41F2_9132_21DE3A57470D_H_ */
