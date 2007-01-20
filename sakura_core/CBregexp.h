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

#include "CDllHandler.h"

/*!
	BREGEXP 正規表現のコンパイル結果を保持する構造体
*/
typedef struct bregexp {
	const char *outp;	//!< BSubst 置換データの先頭ポインタ
	const char *outendp;	//!< BSubst 置換データの最終ポインタ+1
	int  splitctr;	//!< BSplit 配列数
	const char **splitp;	//!< BSplit データポインタ
	int	rsv1;		//!< リザーブ 自由に使用可能
	char *parap;		//!< パターンデータポインタ
	char *paraendp;		//!< パターンデータポインタ+1
	char *transtblp;	//!< BTrans 変換テーブルポインタ
	char **startp;		//!< マッチしたデータの先頭ポインタ
	char **endp;		//!< マッチしたデータの最終ポインタ+1
	int nparens;		//!< パターンの中の() の数。 $1,$2, を調べるときに使用
} BREGEXP;

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
class SAKURA_CORE_API CBregexp : public CDllHandler {
public:
	CBregexp();
	virtual ~CBregexp();

	// 2006.01.22 かろと オプション追加・名称変更
	enum Option {
		optNothing = 0,					//!< オプションなし
		optCaseSensitive = 1,			//!< 大文字小文字区別オプション(/iをつけない)
		optGlobal = 2					//!< 全域オプション(/g)
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

	const char* GetVersion(){		//!< DLLのバージョン情報を取得
		return IsAvailable() ? BRegexpVersion() : "";
	}

	//	CJreエミュレーション関数
	//!	検索パターンのコンパイル
	// 2002/01/19 novice 正規表現による文字列置換
	// 2002.01.26 hor    置換後文字列を別引数に
	// 2002.02.01 hor    大文字小文字を無視するオプション追加
	//>> 2002/03/27 Azumaiya 正規表現置換にコンパイル関数を使う形式を追加
	bool Compile(const char *szPattern, int nOption = 0) {
		return Compile(szPattern, NULL, nOption);
	}
	bool Compile(const char *szPattern0, const char *szPattern1, int nOption = 0);	//!< Replace用
	bool Match(const char *szTarget, int nLen, int nStart = 0);						//!< 検索を実行する
	int Replace(const char *szTarget, int nLen, int nStart = 0);					//!< 置換を実行する	// 2007.01.16 ryoji 戻り値を置換個数に変更

	//-----------------------------------------
	// 2005.03.19 かろと クラス内部を隠蔽
	/*! @{
		@name 結果を得るメソッドを追加し、BREGEXPを外部から隠す
	*/
	/*!
	    検索に一致した文字列の先頭位置を返す(文字列先頭なら0)
		@retval 検索に一致した文字列の先頭位置
	*/
	int GetIndex(void) {
		return m_pRegExp->startp[0] - m_szTarget;
	}
	/*!
	    検索に一致した文字列の次の位置を返す
		@retval 検索に一致した文字列の次の位置
	*/
	int GetLastIndex(void) {
		return m_pRegExp->endp[0] - m_szTarget;
	}
	/*!
		検索に一致した文字列の長さを返す
		@retval 検索に一致した文字列の長さ
	*/
	int GetMatchLen(void) {
		return m_pRegExp->endp[0] - m_pRegExp->startp[0];
	}
	/*!
		置換された文字列の長さを返す
		@retval 置換された文字列の長さ
	*/
	int GetStringLen(void) {
		// 置換後文字列が０幅なら outp、outendpもNULLになる
		// NULLポインタの引き算は問題なく０になる。
		// outendpは '\0'なので、文字列長は +1不要

		// Jun. 03, 2005 Karoto
		//	置換後文字列が0幅の場合にoutpがNULLでもoutendpがNULLでない場合があるので，
		//	outpのNULLチェックが必要
		if (m_pRegExp->outp == NULL) {
			return 0;
		} else {
			return m_pRegExp->outendp - m_pRegExp->outp;
		}
	}
	/*!
		置換された文字列を返す
		@retval 置換された文字列へのポインタ
	*/
	const char *GetString(void) {
		return m_pRegExp->outp;
	}
	/*! @} */
	//-----------------------------------------

	/*! BREGEXPメッセージを取得する
		@retval メッセージへのポインタ
	*/
	const char* GetLastMessage(void) const { return m_szMsg; }
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
	bool IsLookAhead(const char *pattern) {
		CheckPattern(pattern);
		return IsLookAhead();
	}

protected:
	//	Jul. 5, 2001 genta インターフェース変更に伴う引数追加
	virtual char* GetDllName(char*);
	virtual int InitDll(void);
	virtual int DeinitDll(void);

	//	DLL Interfaceの受け皿
	//	Aug. 20, 2005 Aroka : 最適化オプションでデフォルトを__fastcallに変更しても
	//	影響を受けないようにする．
	typedef int (__cdecl *BREGEXP_BMatch)(const char*,const char *,const char *,BREGEXP **,char *);
	typedef int (__cdecl *BREGEXP_BSubst)(const char*,const char *,const char *,BREGEXP **,char *);
	typedef int (__cdecl *BREGEXP_BTrans)(const char*,char *,char *,BREGEXP **,char *);
	typedef int (__cdecl *BREGEXP_BSplit)(const char*,char *,char *,int,BREGEXP **,char *);
	typedef void (__cdecl *BREGEXP_BRegfree)(BREGEXP*);
	typedef const char* (__cdecl *BREGEXP_BRegexpVersion)(void);
	// 2005.03.19 かろと 前方一致用新インターフェース
	typedef int (*BREGEXP_BMatchEx)(const char*,const char*, const char *,const char *,BREGEXP **,char *);
	typedef int (*BREGEXP_BSubstEx)(const char*,const char*, const char *,const char *,BREGEXP **,char *);

	BREGEXP_BMatch BMatch;
	BREGEXP_BSubst BSubst;
	BREGEXP_BTrans BTrans;
	BREGEXP_BSplit BSplit;
	BREGEXP_BRegfree BRegfree;
	BREGEXP_BRegexpVersion BRegexpVersion;
	BREGEXP_BMatchEx BMatchEx;
	BREGEXP_BSubstEx BSubstEx;

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
	int CheckPattern( const char *szPattern );
	char* MakePatternSub( const char* szPattern, const char* szPattern2, const char* szAdd2, int nOption );
	char* MakePattern( const char* szPattern, const char* szPattern2, int nOption );
	//	メンバ変数
	BREGEXP*			m_pRegExp;			//!< コンパイル構造体
	int					m_ePatType;			//!< 検索文字列パターン種別
	const char			*m_szTarget;		//!< 対象文字列へのポインタ
	char				m_szMsg[80];		//!< BREGEXPからのメッセージを保持する
	// 静的メンバ変数
	static const char	m_tmpBuf[2];		//!< ダミー文字列
};

//	以下は関数ポインタに読み込まれる関数の解説
/*!	@fn int CBregexp::BMatch(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg)

	m/pattern/option 形式のPerl互換パターンマッチングを行う。

	@param str [in] 検索するパターン
	@param target [in] 検索対象領域先頭
	@param targetendp [in] 検索対象領域末尾
	@param rxp [out] BREGEXP構造体。結果はここから取得する。
	@param msg [out] エラーメッセージ

	target <= p < targetendp の範囲が検索対象になる。
*/
/*!	@fn int CBregexp::BMatchEx(char* str,char *targetbeg, char *target,char *targetendp, BREGEXP **rxp,char *msg)

	m/pattern/option 形式のPerl互換パターンマッチングを行う。

	@param str [in] 検索するパターン(コンパイル済みならNULL)
	@param targetbeg [in] 検索対象文字列(行頭から)
	@param target [in] 検索対象領域先頭
	@param targetendp [in] 検索対象領域末尾
	@param rxp [out] BREGEXP構造体。結果はここから取得する。
	@param msg [out] エラーメッセージ

	target <= p < targetendp の範囲が検索対象になる。
*/

/*!	@fn int CBregexp::BSubst(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg);

	s/pattern/replace/option 形式のPerl互換文字列置換を行う

	@param str [in] 検索・置換パターン
	@param target [in] 検索対象領域先頭
	@param targetendp [in] 検索対象領域末尾
	@param rxp [out] BREGEXP構造体。結果はここから取得する。
	@param msg [out] エラーメッセージ

	@return 置換した文字列の数
	
	rxp->outpからrxp->outendpに置換後の文字列が格納される。

*/
/*!	@fn int CBregexp::BSubstEx(char* str,char *targetbeg char *target, ,char *targetendp, BREGEXP **rxp,char *msg);

	s/pattern/replace/option 形式のPerl互換文字列置換を行う

	@param str [in] 検索・置換パターン（コンパイル済みならNULL)
	@param target [in] 検索対象文字列（行頭から）
	@param target [in] 検索対象領域先頭
	@param targetendp [in] 検索対象領域末尾
	@param rxp [out] BREGEXP構造体。結果はここから取得する。
	@param msg [out] エラーメッセージ

	@return 置換した文字列の数
	
	rxp->outpからrxp->outendpに置換後の文字列が格納される。

*/

/*!	@fn	int CBregexp::BTrans(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg)

	tr/pattern/replace/option 形式のPerl互換文字置換を行う

	@param str [in] 検索・置換パターン
	@param target [in] 検索対象領域先頭
	@param targetendp [in] 検索対象領域末尾
	@param rxp [out] BREGEXP構造体。結果はここから取得する。
	@param msg [out] エラーメッセージ

	@return 変換した文字数

	rxp->outpからrxp->outendpに変換後の文字列が格納される。

*/

/*!	@fn int CBregexp::BSplit(char* str,char *target,char *targetendp, int limit,BREGEXP **rxp,char *msg)

	split( /patttern/, string ) 相当の文字列分割を行う

	@param str [in] 検索パターン
	@param target [in] 検索対象領域先頭
	@param targetendp [in] 検索対象領域末尾
	@param rxp [out] BREGEXP構造体。結果はここから取得する。
	@param msg [out] エラーメッセージ
	@param limit [in] 最大分割数。これを越えた分については分割は行われずに最終要素に入る。

	@return 分割数

*/

/*!	@fn void CBregexp::BRegfree(BREGEXP* rx)

	検索関数によって渡されたBREGEXP構造体の解放

	@param rx [in] 解放する構造体
*/


/*!	@fn const char* CBregexp::BRegexpVersion(void)

	BREGEXP.DLLのバージョン番号を返す。
	@return バージョン文字列へのポインタ。

	@par Sample
	Version: Bregexp.dll V1.1 Build 22 Apr 29 2000 21:13:19
*/

#endif
/*[EOF]*/
