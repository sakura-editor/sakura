/*!	@file
	@brief BREGEXP Library Handler

	Perl5互換正規表現を扱うDLLであるBREGEXP.DLLを利用するためのインターフェース

	@author genta
	@date Jun. 10, 2001
	@date Jan. 05, 2002 genta コメント追加
	@date 2002/2/1 hor
	@date Jul. 25, 2002 genta 行頭条件の考慮を追加
*/
/*
	Copyright (C) 2001-2002, genta
	Copyright (C) 2001, novice, hor
	Copyright (C) 2002, novice, hor, Azumaiya
	Copyright (C) 2003, かろと
	Copyright (C) 2005, かろと, aroka
	Copyright (C) 2006, かろと
	Copyright (C) 2007, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CBREGEXP_7B193CCD_1EE7_48A7_8004_6E59B0F4F161_H_
#define SAKURA_CBREGEXP_7B193CCD_1EE7_48A7_8004_6E59B0F4F161_H_
#pragma once

#include "basis/SakuraBasis.h"

#include "extmodule/CBregOnig.hpp"

/*!
	@brief Perl互換正規表現 BREGEXP.DLL をサポートするクラス

	DLLの動的ロードを行うため、DllHandlerを継承している。

	CJreに近い動作をさせるため、バッファをクラス内に1つ保持し、
	データの設定と検索の２つのステップに分割するようにしている。
	Jreエミュレーション関数を使うときは入れ子にならないように注意すること。

	本来はこのような部分は別クラスとして分離すべきだが、その場合このクラスが
	破棄される前に全てのクラスを破棄する必要がある。
	その安全性を保証するのが難しいため、現時点では両者を１つのクラスに入れた。

	@note このクラスはThread Safeではない。

	@date 2005.03.19 かろと リファクタリング。クラス内部を隠蔽
	@date 2006.01.22 かろと オプション追加・名称変更(全て行置換用Globalオプション追加のため)
*/
class CBregexp : public CBregOnig
{
public:
	CBregexp();
	~CBregexp() override;

	// 2006.01.22 かろと オプション追加・名称変更
	enum Option {
		optNothing = 0,					//!< オプションなし
		optCaseSensitive = 1,			//!< 大文字小文字区別オプション(/iをつけない)
		optGlobal = 2,					//!< 全域オプション(/g)
		optExtend = 4,					//!< 拡張正規表現(/x)
		optASCII = 8,					//!< ASCII(/a)
		optUnicode = 0x10,				//!< Unicode(/u)
		optDefault = 0x20,				//!< Default(/d)
		optLocale = 0x40,				//!< Locale(/l)
		optR = 0x80,					//!< CRLF(/R)
	};

	struct CPattern {
		const CBregOnig*	m_cDll;						//!< BregOnigクラスへのポインタ
		BREGEXP*			m_pRegExp = nullptr;		//!< コンパイル構造体
		std::wstring_view	m_Target;					//!< 対象文字列へのポインタ
		std::wstring		m_Msg{ 79, '\0' };			//!< bregonig.dllからのメッセージ

		CPattern(
			const CBregOnig& cDll,
			BREGEXP* pRegExp,
			const std::wstring& msg
		) noexcept;
	};

	//! DLLのバージョン情報を取得
	const WCHAR* GetVersionW() noexcept { return IsAvailable() ? BRegexpVersion() : L""; }

	//	CJreエミュレーション関数
	//!	検索パターンのコンパイル
	// 2002/01/19 novice 正規表現による文字列置換
	// 2002.01.26 hor    置換後文字列を別引数に
	// 2002.02.01 hor    大文字小文字を無視するオプション追加
	//>> 2002/03/27 Azumaiya 正規表現置換にコンパイル関数を使う形式を追加
	bool Compile(
		const std::wstring& pattern0,
		int nOption = 0,
		const std::optional<std::wstring>& optPattern1 = std::nullopt,
		bool bKakomi = false
	);

	bool Compile(
		const std::wstring& pattern0,
		const std::wstring& pattern1,
		int nOption = 0,
		bool bKakomi = false
	)
	{
		return Compile(pattern0, nOption, std::make_optional(pattern1), bKakomi);
	}

	bool Match(const wchar_t *szTarget, int nLen, int nStart = 0);						//!< 検索を実行する
	int Replace(const wchar_t *szTarget, int nLen, int nStart = 0);					//!< 置換を実行する	// 2007.01.16 ryoji 戻り値を置換個数に変更

	//-----------------------------------------
	// 2005.03.19 かろと クラス内部を隠蔽
	/*! @{
		@name 結果を得るメソッドを追加し、BREGEXPを外部から隠す
	*/
	/*!
	    検索に一致した文字列の先頭位置を返す(文字列先頭なら0)
		@retval 検索に一致した文字列の先頭位置
	*/
	CLogicInt GetIndex(void)
	{
		return CLogicInt(m_pRegExp->startp[0] - m_szTarget);
	}
	/*!
	    検索に一致した文字列の次の位置を返す
		@retval 検索に一致した文字列の次の位置
	*/
	CLogicInt GetLastIndex(void)
	{
		return CLogicInt(m_pRegExp->endp[0] - m_szTarget);
	}
	/*!
		検索に一致した文字列の長さを返す
		@retval 検索に一致した文字列の長さ
	*/
	CLogicInt GetMatchLen(void)
	{
		return CLogicInt(m_pRegExp->endp[0] - m_pRegExp->startp[0]);
	}
	/*!
		置換された文字列の長さを返す
		@retval 置換された文字列の長さ
	*/
	CLogicInt GetStringLen(void) {
		// 置換後文字列が０幅なら outp、outendpもNULLになる
		// NULLポインタの引き算は問題なく０になる。
		// outendpは '\0'なので、文字列長は +1不要

		// Jun. 03, 2005 Karoto
		//	置換後文字列が0幅の場合にoutpがNULLでもoutendpがNULLでない場合があるので，
		//	outpのNULLチェックが必要

		if (m_pRegExp->outp == nullptr) {
			return CLogicInt(0);
		} else {
			return CLogicInt(m_pRegExp->outendp - m_pRegExp->outp);
		}
	}
	/*!
		置換された文字列を返す
		@retval 置換された文字列へのポインタ
	*/
	const wchar_t *GetString(void)
	{
		return m_pRegExp->outp;
	}
	/*! @} */
	//-----------------------------------------

	/*! BREGEXPメッセージを取得する
		@retval メッセージへのポインタ
	*/
	const WCHAR* GetLastMessage() const;// { return m_szMsg; }

protected:
	void ReleaseCompileBuffer() noexcept;

private:
	//	内部関数

	//! 検索パターン作成
	wchar_t* MakePatternSub( const wchar_t* szPattern, const wchar_t* szPattern2, const wchar_t* szAdd2, int nOption );
	wchar_t* MakePatternAlternate( const wchar_t* const szSearch, const wchar_t* const szReplace, int nOption );

	//	メンバ変数
	BREGEXP_W*			m_pRegExp;			//!< コンパイル構造体
	const wchar_t*		m_szTarget;			//!< 対象文字列へのポインタ
	wchar_t				m_szMsg[80];		//!< BREGEXP_Wからのメッセージを保持する

	// 静的メンバ変数
	static const wchar_t	m_tmpBuf[2];	//!< ダミー文字列
};

//	Jun. 26, 2001 genta
//!	正規表現ライブラリのバージョン取得
bool CheckRegexpVersion( HWND hWnd, int nCmpId, bool bShowMsg = false );
bool CheckRegexpSyntax( const wchar_t* szPattern, HWND hWnd, bool bShowMessage, int nOption = -1, bool bKakomi = false );// 2002/2/1 hor追加
bool InitRegexp( HWND hWnd, CBregexp& rRegexp, bool bShowMessage );

#endif /* SAKURA_CBREGEXP_7B193CCD_1EE7_48A7_8004_6E59B0F4F161_H_ */
