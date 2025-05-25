/*!	@file
	@brief 処理所要時間の計測クラス

	デバッグ目的で用いる

	@author Norio Nakatani
	@date 1998/03/06  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CRUNNINGTIMER_B4A1B7C4_EA83_41F2_9132_21DE3A57470D_H_
#define SAKURA_CRUNNINGTIMER_B4A1B7C4_EA83_41F2_9132_21DE3A57470D_H_
#pragma once

#include "util/string_ex.h"
#include <string>
#include <string_view>
#include <chrono>
#include <vector>

// 本番コードに組み込まれている時間計測の結果を出力ペインに出したい時にコメントを外して下さい
//#define TIME_MEASURE

#if defined(TIME_MEASURE)
  #define MY_TRACETIME(c,m) (c).WriteTrace(m)
  #define MY_RUNNINGTIMER(c,m) CRunningTimer c(m, CRunningTimer::OutputMode::OnWriteTrace, CRunningTimer::OutputStyle::Conventional)
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
	/*! @brief ログ出力モード */
	enum class OutputMode {
		// WriteTraceを呼び出す度にログ出力します。
		// 計測オーバーヘッドは大きめです。(Release:200us程度 Debug:400us程度)
		OnWriteTrace,

		// 計測終了時(インスタンス破棄時)にまとめてログ出力します。
		// 計測オーバーヘッドは小さめです。(Release:5us程度 Debug:50us程度)
		// 入れ子で計測する場合においてログの出力順がWriteTraceの呼び出し順とならないため注意して下さい。
		OnExitScope
	};

	/*! @brief ログ出力形式 */
	enum class OutputStyle { Conventional, Markdown };

	/*!
		@param[in]	name	タイマー名称
		@param[in]	mode	ログの出力モード
		@param[in]	name	ログの出力形式
	*/
	CRunningTimer( std::wstring_view name = L"", OutputMode mode = OutputMode::OnWriteTrace, OutputStyle style = OutputStyle::Markdown );

	~CRunningTimer();

	/*!
		経過時間を0に戻す
	*/
	void Reset();

	/*!
		現在の経過時間を取得
		@return 経過時間(ms)
	*/
	uint32_t Read() const;

	/*!
		現在の経過時間でログを書き込む
		@param[in]	msg		ログのメッセージ欄に出力する文字列
	*/
	void WriteTrace( std::wstring_view msg = L"" );

	/*!
		現在の経過時間でログを書き込む
		@param[in]	n		ログのメッセージ欄に出力する数値
	*/
	void WriteTrace( int32_t n );

	/*!
		現在の経過時間でログを書き込む
		@param[in]	fmt		書式文字列
		@param[in]	...		書式文字列に対応する引数
	*/
	template <typename... T>
	void WriteTraceFormat( std::wstring_view fmt, T... args )
	{
		auto currentTime = GetTime();
		std::wstring msg;
		strprintf( msg, fmt.data(), args... );
		WriteTraceInternal( currentTime, TraceType::Normal, msg );
	}

protected:
	enum class TraceType { Normal, Enter, ExitScope };
	using TimePoint = std::chrono::high_resolution_clock::time_point;
	struct TraceEntry {
		TraceEntry( TimePoint timePoint, TraceType traceType, std::wstring_view msg ) :
			m_timePoint( timePoint ),
			m_traceType( traceType ),
			m_msg( msg )
		{}
		TimePoint		m_timePoint;
		TraceType		m_traceType;
		std::wstring	m_msg;
	};

	static int m_nNestCount;
	static TimePoint m_initialTime;				// タイムスタンプ基準時間

	static TimePoint GetTime();
	static double GetElapsedTimeInSeconds( TimePoint from, TimePoint to );

	void WriteTraceInternal( TimePoint currentTime, TraceType traceType, std::wstring_view msg = L"" );
	void FlushPendingTraces();
	void OutputHeader() const;
	void OutputFooter() const;
	void OutputTrace( TimePoint currentTime, TraceType traceType, std::wstring_view msg ) const;
	void Output( std::wstring_view fmt, ... ) const;

private:
	TimePoint		m_startTime;				// 計測開始時間
	TimePoint		m_lastTime;					// 最後に出力した時間
	std::wstring	m_timerName;				// タイマー名
	int				m_nDepth;					// このオブジェクトのネストの深さ
	OutputMode		m_outputMode;				// ログの出力モード
	OutputStyle		m_outputStyle;				// ログの出力形式
	size_t			m_nNameOutputWidth = 40;	// タイマー名出力幅(文字数)(初期値は最小幅)
	std::vector<TraceEntry> m_pendingTraces;	// 出力保留中の情報
};
#endif /* SAKURA_CRUNNINGTIMER_B4A1B7C4_EA83_41F2_9132_21DE3A57470D_H_ */
