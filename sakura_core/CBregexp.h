//	$Id$
/*!	@file
	BREGEXP Library Handler

	Perl5互換正規表現を扱うDLLであるBREGEXP.DLLを利用するためのインターフェース

	@author genta
	@date Jun. 10, 2001
	$Revision$
*/
/*
	Copyright (C) 2001, genta

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
*/
class SAKURA_CORE_API CBregexp : public CDllHandler {
public:
	CBregexp();
	virtual ~CBregexp();

	char* GetVersion(){		//!< DLLのバージョン情報を取得
		return IsAvailable() ? BRegexpVersion() : NULL;
	}

	//	CJreエミュレーション関数
	//!	検索パターンのコンパイル
	bool Compile(const char *szPattern);
	bool GetMatchInfo(const char*target, int len, int nStart, BREGEXP**rep);

	//! BREGEXPメッセージを取得する
	const char* GetLastMessage(void) const { return m_szMsg; }

protected:
	//	Jul. 5, 2001 genta インターフェース変更に伴う引数追加
	virtual char* GetDllName(char*);
	virtual int InitDll(void);
	virtual int DeinitDll(void);

	//	DLL Interfaceの受け皿
	typedef int (*BREGEXP_BMatch)(const char*,const char *,const char *,BREGEXP **,char *);
	typedef int (*BREGEXP_BSubst)(const char*,char *,char *,BREGEXP **,char *);
	typedef int (*BREGEXP_BTrans)(const char*,char *,char *,BREGEXP **,char *);
	typedef int (*BREGEXP_BSplit)(const char*,char *,char *,int,BREGEXP **,char *);
	typedef void (*BREGEXP_BRegfree)(BREGEXP*);
	typedef char* (*BREGEXP_BRegexpVersion)(void);

	BREGEXP_BMatch BMatch;
	BREGEXP_BSubst BSubst;
	BREGEXP_BTrans BTrans;
	BREGEXP_BSplit BSplit;
	BREGEXP_BRegfree BRegfree;
	BREGEXP_BRegexpVersion BRegexpVersion;

	//!	コンパイルバッファを解放する
	/*!
		m_sRepをBRegfree()に渡して解放する．解放後はNULLにセットする．
		元々NULLなら何もしない
	*/
	void ReleaseCompileBuffer(void){
		if( m_sRep ){
			BRegfree( m_sRep );
			m_sRep = NULL;
		}
	}

private:
	//	内部関数

	//!	境界選択
	//int ChooseBoundary(const char* str1, const char* str2 = NULL );

	//	メンバ変数
	BREGEXP*	m_sRep;	//!< コンパイル構造体
	char		m_szMsg[80];		//!< BREGEXPからのメッセージを保持する
};

//	以下は関数ポインタに読み込まれる関数の解説
/*!	@fn int CBregexp::BMatch(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg)

	m/pattern/option 形式のPerl互換パターンマッチングを行う。

	@param str [in] 検索するパターン
	@param target [in] 検索対象領域先頭
	@param targetendp [in] 検索対象領域末尾
	@param rxp [out] BREGEXP構造体。結果はここから取得する。
	@pararm msg [out] エラーメッセージ

	target <= p < targetendp の範囲が検索対象になる。
*/

/*!	@fn int CBregexp::BSubst(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg);

	s/pattern/replace/option 形式のPerl互換文字列置換を行う

	@param str [in] 検索・置換パターン
	@param target [in] 検索対象領域先頭
	@param targetendp [in] 検索対象領域末尾
	@param rxp [out] BREGEXP構造体。結果はここから取得する。
	@pararm msg [out] エラーメッセージ

*/

/*!	@fn	int CBregexp::BTrans(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg)

	tr/pattern/replace/option 形式のPerl互換文字置換を行う

	@param str [in] 検索・置換パターン
	@param target [in] 検索対象領域先頭
	@param targetendp [in] 検索対象領域末尾
	@param rxp [out] BREGEXP構造体。結果はここから取得する。
	@pararm msg [out] エラーメッセージ

*/

/*!	@fn int CBregexp::BSplit(char* str,char *target,char *targetendp, int limit,BREGEXP **rxp,char *msg)

	split( /patttern/, string ) 相当の文字列分割を行う

	@param str [in] 検索パターン
	@param target [in] 検索対象領域先頭
	@param targetendp [in] 検索対象領域末尾
	@param rxp [out] BREGEXP構造体。結果はここから取得する。
	@pararm msg [out] エラーメッセージ
	@param limit [in] 最大分割数。これを越えた分については分割は行われずに最終要素に入る。

*/

/*!	@fn void CBregexp::BRegfree(BREGEXP* rx)

	検索関数によって渡されたBREGEXP構造体の解放

	@param rx [in] 解放する構造体

*/

/*!	@fn char* CBregexp::BRegexpVersion(void)

	BREGEXP.DLLのバージョン番号を返す。

	@par Sample
	Version: Bregexp.dll V1.1 Build 22 Apr 29 2000 21:13:19
*/

#endif


/*[EOF]*/
