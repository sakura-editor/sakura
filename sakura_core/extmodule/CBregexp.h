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

#ifndef _DLL_BREGEXP_H_
#define _DLL_BREGEXP_H_

#include "CBregexpDll2.h"

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
class CBregexp : public CBregexpDll2{
public:
	CBregexp();
	virtual ~CBregexp();

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
	//! 検索パターン定義
	enum Pattern {
		PAT_UNKNOWN = 0,		//!< 不明（初期値)
		PAT_NORMAL = 1,			//!< 通常
		PAT_TOP = 2,			//!< 行頭"^"
		PAT_BOTTOM = 4,			//!< 行末"$"
		PAT_TAB = 8,			//!< 行頭行末"^$"
		PAT_LOOKAHEAD = 16		//!< 先読み"(?[=]"
	};

	//! DLLのバージョン情報を取得
	const TCHAR* GetVersionT(){ return IsAvailable() ? to_tchar(BRegexpVersion()) : _T(""); }

	//	CJreエミュレーション関数
	//!	検索パターンのコンパイル
	// 2002/01/19 novice 正規表現による文字列置換
	// 2002.01.26 hor    置換後文字列を別引数に
	// 2002.02.01 hor    大文字小文字を無視するオプション追加
	//>> 2002/03/27 Azumaiya 正規表現置換にコンパイル関数を使う形式を追加
	bool Compile(const wchar_t *szPattern, int nOption = 0) {
		return Compile(szPattern, NULL, nOption);
	}
	bool Compile(const wchar_t *szPattern0, const wchar_t *szPattern1, int nOption = 0, bool bKakomi = false);	//!< Replace用
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

		if (m_pRegExp->outp == NULL) {
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
	const TCHAR* GetLastMessage() const;// { return m_szMsg; }

	/*!	先読みパターンが存在するかを返す
		この関数は、コンパイル後であることが前提なので、コンパイル前はfalse
		@retval true 先読みがある
		@retval false 先読みがない 又は コンパイル前
	*/
	bool IsLookAhead(void) {
		return m_ePatType & PAT_LOOKAHEAD ? true : false;
	}
	/*!	検索パターンに先読みが含まれるか？（コンパイル前でも判別可能）
		@param[in] pattern 検索パターン
		@retval true 先読みがある
		@retval false 先読みがない
	*/
	bool IsLookAhead(const wchar_t *pattern) {
		CheckPattern(pattern);
		return IsLookAhead();
	}

protected:


	//!	コンパイルバッファを解放する
	/*!
		m_pcRegをBRegfree()に渡して解放する．解放後はNULLにセットする．
		元々NULLなら何もしない
	*/
	void ReleaseCompileBuffer(void){
		if( m_pRegExp ){
			BRegfree( m_pRegExp );
			m_pRegExp = NULL;
		}
		m_ePatType = PAT_UNKNOWN;
	}

private:
	//	内部関数

	//! 検索パターン作成
	int CheckPattern( const wchar_t* szPattern );
	wchar_t* MakePatternSub( const wchar_t* szPattern, const wchar_t* szPattern2, const wchar_t* szAdd2, int nOption );
	wchar_t* MakePattern( const wchar_t* szPattern, const wchar_t* szPattern2, int nOption );
	wchar_t* MakePatternAlternate( const wchar_t* const szSearch, const wchar_t* const szReplace, int nOption );

	//	メンバ変数
	BREGEXP_W*			m_pRegExp;			//!< コンパイル構造体
	int					m_ePatType;			//!< 検索文字列パターン種別
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


#endif



