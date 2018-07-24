/*!	@file
	@brief BREGEXP Library Handler

	Perl5互換正規表現を扱うDLLであるBREGEXP.DLLを利用するためのインターフェース

	@author genta
	@date Jun. 10, 2001
	@date 2002/2/1 hor		ReleaseCompileBufferを適宜追加
	@date Jul. 25, 2002 genta 行頭条件を考慮した検索を行うように．(置換はまだ)
	@date 2003.05.22 かろと 正規な正規表現に近づける
	@date 2005.03.19 かろと リファクタリング。クラス内部を隠蔽。
*/
/*
	Copyright (C) 2001-2002, genta
	Copyright (C) 2002, novice, hor, Azumaiya
	Copyright (C) 2003, かろと
	Copyright (C) 2005, かろと
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

#include "StdAfx.h"
#include <string>
#include <stdio.h>
#include <string.h>
#include "CBregexp.h"
#include "charset/charcode.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"


// Compile時、行頭置換(len=0)の時にダミー文字列(１つに統一) by かろと
const wchar_t CBregexp::m_tmpBuf[2] = L"\0";





CBregexp::CBregexp()
: m_pRegExp( NULL )
, m_ePatType( PAT_NORMAL )	//	Jul, 25, 2002 genta
{
	m_szMsg[0] = L'\0';
}

CBregexp::~CBregexp()
{
	//コンパイルバッファを解放
	ReleaseCompileBuffer();
}


/*! @brief 検索パターンが特定の検索パターンかチェックする
**
** @param[in] szPattern 検索パターン
**
** @retval 検索パターン文字列長
** 
** @date 2005.03.20 かろと 関数に切り出し
*/
int CBregexp::CheckPattern(const wchar_t* szPattern)
{
	static const wchar_t TOP_MATCH[] = L"/^\\(*\\^/k";							//!< 行頭パターンのチェック用パターン
	static const wchar_t DOL_MATCH[] = L"/\\\\\\$$/k";							//!< \$(行末パターンでない)チェック用パターン
	static const wchar_t BOT_MATCH[] = L"/\\$\\)*$/k";							//!< 行末パターンのチェック用パターン
	static const wchar_t TAB_MATCH[] = L"/^\\(*\\^\\$\\)*$/k";					//!< "^$"パターンかをチェック用パターン
	static const wchar_t LOOKAHEAD[] = L"/\\(\\?[=]/k";							//!< "(?=" 先読み の存在チェックパターン
	BREGEXP_W*	sReg = NULL;					//!< コンパイル構造体
	wchar_t szMsg[80] = L"";					//!< エラーメッセージ
	int nLen;									//!< 検索パターンの長さ
	const wchar_t *szPatternEnd;				//!< 検索パターンの終端

	m_ePatType = PAT_NORMAL;	//!<　ノーマルは確定
	nLen = wcslen( szPattern );
	szPatternEnd = szPattern + nLen;
	// パターン種別の設定
	if( BMatch( TOP_MATCH, szPattern, szPatternEnd, &sReg, szMsg ) > 0 ) {
		// 行頭パターンにマッチした
		m_ePatType |= PAT_TOP;
	}
	BRegfree(sReg);
	sReg = NULL;
	if( BMatch( TAB_MATCH, szPattern, szPatternEnd, &sReg, szMsg ) > 0 ) {
		// 行頭行末パターンにマッチした
		m_ePatType |= PAT_TAB;
	}
	BRegfree(sReg);
	sReg = NULL;
	if( BMatch( DOL_MATCH, szPattern, szPatternEnd, &sReg, szMsg ) > 0 ) {
		// 行末の\$ にマッチした
		// PAT_NORMAL
	} else {
		BRegfree(sReg);
		sReg = NULL;
		if( BMatch( BOT_MATCH, szPattern, szPatternEnd, &sReg, szMsg ) > 0 ) {
			// 行末パターンにマッチした
			m_ePatType |= PAT_BOTTOM;
		} else {
			// その他
			// PAT_NORMAL
		}
	}
	BRegfree(sReg);
	sReg = NULL;
	
	if( BMatch( LOOKAHEAD, szPattern, szPattern + nLen, &sReg, szMsg ) > 0 ) {
		// 先読みパターンにマッチした
		m_ePatType |= PAT_LOOKAHEAD;
	}
	BRegfree(sReg);
	sReg = NULL;
	return (nLen);
}

/*! @brief ライブラリに渡すための検索・置換パターンを作成する
**
** @note szPattern2: == NULL:検索 != NULL:置換
**
** @retval ライブラリに渡す検索パターンへのポインタを返す
** @note 返すポインタは、呼び出し側で delete すること
** 
** @date 2003.05.03 かろと 関数に切り出し
*/
wchar_t* CBregexp::MakePatternSub(
	const wchar_t*	szPattern,	//!< 検索パターン
	const wchar_t*	szPattern2,	//!< 置換パターン(NULLなら検索)
	const wchar_t*	szAdd2,		//!< 置換パターンの後ろに付け加えるパターン($1など) 
	int				nOption		//!< 検索オプション
) 
{
	static const wchar_t DELIMITER = WCODE::BREGEXP_DELIMITER;	//!< デリミタ
	int nLen;									//!< szPatternの長さ
	int nLen2;									//!< szPattern2 + szAdd2 の長さ

	// 検索パターン作成
	wchar_t *szNPattern;		//!< ライブラリ渡し用の検索パターン文字列
	wchar_t *pPat;				//!< パターン文字列操作用のポインタ

	nLen = wcslen(szPattern);
	if (szPattern2 == NULL) {
		// 検索(BMatch)時
		szNPattern = new wchar_t[ nLen + 15 ];	//	15：「s///option」が余裕ではいるように。
		pPat = szNPattern;
		*pPat++ = L'm';
	}
	else {
		// 置換(BSubst)時
		nLen2 = wcslen(szPattern2) + wcslen(szAdd2);
		szNPattern = new wchar_t[ nLen + nLen2 + 15 ];
		pPat = szNPattern;
		*pPat++ = L's';
	}
	*pPat++ = DELIMITER;
	while (*szPattern != L'\0') { *pPat++ = *szPattern++; }
	*pPat++ = DELIMITER;
	if (szPattern2 != NULL) {
		while (*szPattern2 != L'\0') { *pPat++ = *szPattern2++; }
		while (*szAdd2 != L'\0') { *pPat++ = *szAdd2++; }
		*pPat++ = DELIMITER;
	}
	*pPat++ = L'k';			// 漢字対応
	*pPat++ = L'm';			// 複数行対応(但し、呼び出し側が複数行対応でない)
	// 2006.01.22 かろと 論理逆なので bIgnoreCase -> optCaseSensitiveに変更
	if( !(nOption & optCaseSensitive) ) {		// 2002/2/1 hor IgnoreCase オプション追加 マージ：aroka
		*pPat++ = L'i';		// 大文字小文字を同一視(無視)する
	}
	// 2006.01.22 かろと 行単位置換のために、全域オプション追加
	if( (nOption & optGlobal) ) {
		*pPat++ = L'g';			// 全域(global)オプション、行単位の置換をする時に使用する
	}
	if( (nOption & optExtend) ) {
		*pPat++ = L'x';
	}
	if( (nOption & optASCII ) ){
		*pPat++ = L'a';
	}
	if( (nOption & optUnicode ) ){
		*pPat++ = L'u';
	}
	if( (nOption & optDefault ) ){
		*pPat++ = L'd';
	}
	if( (nOption & optLocale ) ){
		*pPat++ = L'l';
	}
	if( (nOption & optR ) ){
		*pPat++ = L'R';
	}

	*pPat = L'\0';
	return szNPattern;
}


/*! 
** 行末文字の意味がライブラリでは \n固定なので、
** これをごまかすために、ライブラリに渡すための検索・置換パターンを工夫する
**
** 行末文字($)が検索パターンの最後にあり、その直前が [\r\n] でない場合に、
** 行末文字($)の手前に ([\r\n]+)を補って、置換パターンに $(nParen+1)を補う
** というアルゴリズムを用いて、ごまかす。
**
** @note szPattern2: == NULL:検索 != NULL:置換
** 
** @param[in] szPattern 検索パターン
** @param[in] szPattern2 置換パターン(NULLなら検索)
** @param[in] nOption 検索オプション
**
** @retval ライブラリに渡す検索パターンへのポインタを返す
** @note 返すポインタは、呼び出し側で delete すること
**
** @date 2003.05.03 かろと 関数に切り出し
*/
wchar_t* CBregexp::MakePattern( const wchar_t* szPattern, const wchar_t* szPattern2, int nOption ) 
{
	using namespace WCODE;
	static const wchar_t* szCRLF = CRLF;		//!< 復帰・改行
	static const wchar_t szCR[] = {CR,0};				//!< 復帰
	static const wchar_t szLF[] = {LF,0};				//!< 改行
	static const wchar_t BOT_SUBST[] = L"s/\\$(\\)*)$/([\\\\r\\\\n]+)\\$$1/k";	//!< 行末パターンの置換用パターン
	int nLen;									//!< szPatternの長さ
	BREGEXP_W*	sReg = NULL;					//!< コンパイル構造体
	wchar_t szMsg[80] = L"";						//!< エラーメッセージ
	wchar_t szAdd2[5] = L"";						//!< 行末あり置換の $数字 格納用
	int nParens = 0;							//!< 検索パターン(szPattern)中の括弧の数(行末時に使用)
	wchar_t *szNPattern;							//!< 検索パターン

	nLen = CheckPattern( szPattern );
	if( (m_ePatType & PAT_BOTTOM) != 0 ) {
		bool bJustDollar = false;			// 行末指定の$のみであるフラグ($の前に \r\nが指定されていない)
		szNPattern = MakePatternSub(szPattern, NULL, NULL, nOption);
		int matched = BMatch( szNPattern, szCRLF, szCRLF+wcslen(szCRLF), &sReg, szMsg );
		if( matched >= 0 ) {
			// szNPatternが不正なパターン等のエラーでなかった
			// エラー時には sRegがNULLのままなので、sReg->nparensへのアクセスは不正
			nParens = sReg->nparens;			// オリジナルの検索文字列中の()の数を記憶
			if( matched > 0 ) {
				if( sReg->startp[0] == &szCRLF[1] && sReg->endp[0] == &szCRLF[1] ) {
					if( BMatch( NULL, szCR, szCR+wcslen(szCR), &sReg, szMsg ) > 0 && sReg->startp[0] == &szCR[1] && sReg->endp[0] == &szCR[1] ) {
						if( BMatch( NULL, szLF, szLF+wcslen(szLF), &sReg, szMsg ) > 0 && sReg->startp[0] == &szLF[0] && sReg->endp[0] == &szLF[0] ) {
							// 検索文字列は 行末($)のみだった
							bJustDollar = true;
						}
					}
				}
			} else {
				if( BMatch( NULL, szCR, szCR+wcslen(szCR), &sReg, szMsg ) <= 0 ) {
					if( BMatch( NULL, szLF, szLF+wcslen(szLF), &sReg, szMsg ) <= 0 ) {
						// 検索文字列は、文字＋行末($)だった
						bJustDollar = true;
					}
				}
			}
			BRegfree(sReg);
			sReg = NULL;
		}
		delete [] szNPattern;

		if( bJustDollar == true || (m_ePatType & PAT_TAB) != 0 ) {
			// 行末指定の$ or 行頭行末指定 なので、検索文字列を置換
			if( BSubst( BOT_SUBST, szPattern, szPattern + nLen, &sReg, szMsg ) > 0 ) {
				szPattern = sReg->outp;
				if( szPattern2 != NULL ) {
					// 置換パターンもあるので、置換パターンの最後に $(nParens+1)を追加
					auto_sprintf( szAdd2, L"$%d", nParens + 1 );
				}
			}
			// sReg->outp のポインタを参照しているので、sRegを解放するのは最後に
		}
	}

	szNPattern = MakePatternSub( szPattern, szPattern2, szAdd2, nOption );
	if( sReg != NULL ) {
		BRegfree(sReg);
	}
	return szNPattern;
}


/*!
	CBregexp::MakePattern()の代替。
	* エスケープされておらず、文字集合と \Q...\Eの中にない . を [^\r\n] に置換する。
	* エスケープされておらず、文字集合と \Q...\Eの中にない $ を (?<![\r\n])(?=\r|$) に置換する。
	これは「改行」の意味を LF のみ(BREGEXP.DLLの仕様)から、CR, LF, CRLF に拡張するための変更である。
	また、$ は改行の後ろ、行文字列末尾にマッチしなくなる。最後の一行の場合をのぞいて、
	正規表現DLLに与えられる文字列の末尾は文書末とはいえず、$ がマッチする必要はないだろう。
	$ が行文字列末尾にマッチしないことは、一括置換での期待しない置換を防ぐために必要である。
*/
wchar_t* CBregexp::MakePatternAlternate( const wchar_t* const szSearch, const wchar_t* const szReplace, int nOption )
{
	this->CheckPattern( szSearch );

	static const wchar_t szDotAlternative[] = L"[^\\r\\n]";
	static const wchar_t szDollarAlternative[] = L"(?<![\\r\\n])(?=\\r|$)";

	// すべての . を [^\r\n] へ、すべての $ を (?<![\r\n])(?=\r|$) へ置換すると仮定して、strModifiedSearchの最大長を決定する。
	std::wstring::size_type modifiedSearchSize = 0;
	for( const wchar_t* p = szSearch; *p; ++p ) {
		if( *p == L'.') {
			modifiedSearchSize += (sizeof szDotAlternative) / (sizeof szDotAlternative[0]) - 1;
		} else if( *p == L'$' ) {
			modifiedSearchSize += (sizeof szDollarAlternative) / (sizeof szDollarAlternative[0]) - 1;
		} else {
			modifiedSearchSize += 1;
		}
	}
	++modifiedSearchSize; // '\0'

	std::wstring strModifiedSearch;
	strModifiedSearch.reserve( modifiedSearchSize );

	// szSearchを strModifiedSearchへ、ところどころ置換しながら順次コピーしていく。
	enum State {
		DEF = 0, /* DEFULT 一番外側 */
		D_E,     /* DEFAULT_ESCAPED 一番外側で \の次 */
		D_C,     /* DEFAULT_SMALL_C 一番外側で \cの次 */
		CHA,     /* CHARSET 文字クラスの中 */
		C_E,     /* CHARSET_ESCAPED 文字クラスの中で \の次 */
		C_C,     /* CHARSET_SMALL_C 文字クラスの中で \cの次 */
		QEE,     /* QEESCAPE \Q...\Eの中 */
		Q_E,     /* QEESCAPE_ESCAPED \Q...\Eの中で \の次 */
		NUMBER_OF_STATE,
		_EC = -1, /* ENTER CHARCLASS charsetLevelをインクリメントして CHAへ */
		_XC = -2, /* EXIT CHARCLASS charsetLevelをデクリメントして CHAか DEFへ */
		_DT = -3, /* DOT (特殊文字としての)ドットを置き換える */
		_DL = -4, /* DOLLAR (特殊文字としての)ドルを置き換える */
	};
	enum CharClass {
		OTHER = 0,
		DOT,    /* . */
		DOLLAR, /* $ */
		SMALLC, /* c */
		LARGEQ, /* Q */
		LARGEE, /* E */
		LBRCKT, /* [ */
		RBRCKT, /* ] */
		ESCAPE, /* \ */
		NUMBER_OF_CHARCLASS
	};
	static const State state_transition_table[NUMBER_OF_STATE][NUMBER_OF_CHARCLASS] = {
	/*        OTHER   DOT  DOLLAR  SMALLC LARGEQ LARGEE LBRCKT RBRCKT ESCAPE*/
	/* DEF */ {DEF,  _DT,   _DL,    DEF,   DEF,   DEF,   _EC,   DEF,   D_E},
	/* D_E */ {DEF,  DEF,   DEF,    D_C,   QEE,   DEF,   DEF,   DEF,   DEF},
	/* D_C */ {DEF,  DEF,   DEF,    DEF,   DEF,   DEF,   DEF,   DEF,   D_E},
	/* CHA */ {CHA,  CHA,   CHA,    CHA,   CHA,   CHA,   _EC,   _XC,   C_E},
	/* C_E */ {CHA,  CHA,   CHA,    C_C,   CHA,   CHA,   CHA,   CHA,   CHA},
	/* C_C */ {CHA,  CHA,   CHA,    CHA,   CHA,   CHA,   CHA,   CHA,   C_E},
	/* QEE */ {QEE,  QEE,   QEE,    QEE,   QEE,   QEE,   QEE,   QEE,   Q_E},
	/* Q_E */ {QEE,  QEE,   QEE,    QEE,   QEE,   DEF,   QEE,   QEE,   Q_E}
	};
	State state = DEF;
	int charsetLevel = 0; // ブラケットの深さ。POSIXブラケット表現など、エスケープされていない [] が入れ子になることがある。
	const wchar_t *left = szSearch, *right = szSearch;
	for( ; *right; ++right ) { // CNativeW::GetSizeOfChar()は使わなくてもいいかな？
		const wchar_t ch = *right;
		const CharClass charClass =
			ch == L'.'  ? DOT:
			ch == L'$'  ? DOLLAR:
			ch == L'c'  ? SMALLC:
			ch == L'Q'  ? LARGEQ:
			ch == L'E'  ? LARGEE:
			ch == L'['  ? LBRCKT:
			ch == L']'  ? RBRCKT:
			ch == L'\\' ? ESCAPE:
			OTHER;
		const State nextState = state_transition_table[state][charClass];
		if(0 <= nextState) {
			state = nextState;
		} else switch(nextState) {
			case _EC: // ENTER CHARSET
				charsetLevel += 1;
				state = CHA;
			break;
			case _XC: // EXIT CHARSET
				charsetLevel -= 1;
				state = 0 < charsetLevel ? CHA : DEF;
			break;
			case _DT: // DOT(match anything)
				strModifiedSearch.append( left, right );
				left = right + 1;
				strModifiedSearch.append( szDotAlternative );
			break;
			case _DL: // DOLLAR(match end of line)
				strModifiedSearch.append( left, right );
				left = right + 1;
				strModifiedSearch.append( szDollarAlternative );
			break;
			default: // バグ。enum Stateに見逃しがある。
			break;
		}
	}
	strModifiedSearch.append( left, right + 1 ); // right + 1 は '\0' の次を指す(明示的に '\0' をコピー)。

	return this->MakePatternSub( strModifiedSearch.data(), szReplace, L"", nOption );
}


/*!
	JRE32のエミュレーション関数．空の文字列に対して検索・置換を行うことにより
	BREGEXP_W構造体の生成のみを行う．

	@param[in] szPattern0	検索or置換パターン
	@param[in] szPattern1	置換後文字列パターン(検索時はNULL)
	@param[in] nOption		検索・置換オプション

	@retval true 成功
	@retval false 失敗
*/
bool CBregexp::Compile( const wchar_t *szPattern0, const wchar_t *szPattern1, int nOption, bool bKakomi )
{

	//	DLLが利用可能でないときはエラー終了
	if( !IsAvailable() )
		return false;

	//	BREGEXP_W構造体の解放
	ReleaseCompileBuffer();

	// ライブラリに渡す検索パターンを作成
	// 別関数で共通処理に変更 2003.05.03 by かろと
	wchar_t *szNPattern = NULL;
	const wchar_t *pszNPattern = NULL;
	if( bKakomi ){
		pszNPattern = szPattern0;
	}else{
		szNPattern = MakePatternAlternate( szPattern0, szPattern1, nOption );
		pszNPattern = szNPattern;
	}
	m_szMsg[0] = L'\0';		//!< エラー解除
	if (szPattern1 == NULL) {
		// 検索実行
		BMatch( pszNPattern, m_tmpBuf, m_tmpBuf+1, &m_pRegExp, m_szMsg );
	} else {
		// 置換実行
		BSubst( pszNPattern, m_tmpBuf, m_tmpBuf+1, &m_pRegExp, m_szMsg );
	}
	delete [] szNPattern;

	//	メッセージが空文字列でなければ何らかのエラー発生。
	//	サンプルソース参照
	if( m_szMsg[0] ){
		ReleaseCompileBuffer();
		return false;
	}
	
	// 行頭条件チェックは、MakePatternに取り込み 2003.05.03 by かろと

	return true;
}

/*!
	JRE32のエミュレーション関数．既にあるコンパイル構造体を利用して検索（1行）を
	行う．

	@param[in] target 検索対象領域先頭アドレス
	@param[in] len 検索対象領域サイズ
	@param[in] nStart 検索開始位置．(先頭は0)

	@retval true Match
	@retval false No Match または エラー。エラーは GetLastMessage()により判定可能。

*/
bool CBregexp::Match( const wchar_t* target, int len, int nStart )
{
	int matched;		//!< 検索一致したか? >0:Match, 0:NoMatch, <0:Error

	//	DLLが利用可能でないとき、または構造体が未設定の時はエラー終了
	if( (!IsAvailable()) || m_pRegExp == NULL ){
		return false;
	}

	m_szMsg[0] = '\0';		//!< エラー解除
	// 拡張関数がない場合は、行の先頭("^")の検索時の特別処理 by かろと
	if (!ExistBMatchEx()) {
		/*
		** 行頭(^)とマッチするのは、nStart=0の時だけなので、それ以外は false
		*/
		if( (m_ePatType & PAT_TOP) != 0 && nStart != 0 ) {
			// nStart!=0でも、BMatch()にとっては行頭になるので、ここでfalseにする必要がある
			return false;
		}
		//	検索文字列＝NULLを指定すると前回と同一の文字列と見なされる
		matched = BMatch( NULL, target + nStart, target + len, &m_pRegExp, m_szMsg );
	} else {
		//	検索文字列＝NULLを指定すると前回と同一の文字列と見なされる
		matched = BMatchEx( NULL, target, target + nStart, target + len, &m_pRegExp, m_szMsg );
	}
	m_szTarget = target;
			
	if ( matched < 0 || m_szMsg[0] ) {
		// BMatchエラー
		// エラー処理をしていなかったので、nStart>=lenのような場合に、マッチ扱いになり
		// 無限置換等の不具合になっていた 2003.05.03 by かろと
		return false;
	} else if ( matched == 0 ) {
		// 一致しなかった
		return false;
	}

	return true;
}


//<< 2002/03/27 Azumaiya
/*!
	正規表現による文字列置換
	既にあるコンパイル構造体を利用して置換（1行）を
	行う．

	@param[in] szTarget 置換対象データ
	@param[in] nLen 置換対象データ長
	@param[in] nStart 置換開始位置(0からnLen未満)

	@retval 置換個数

	@date	2007.01.16 ryoji 戻り値を置換個数に変更
*/
int CBregexp::Replace(const wchar_t *szTarget, int nLen, int nStart)
{
	int result;
	//	DLLが利用可能でないとき、または構造体が未設定の時はエラー終了
	if( !IsAvailable() || m_pRegExp == NULL )
	{
		return false;
	}

	//	From Here 2003.05.03 かろと
	// nLenが０だと、BSubst()が置換に失敗してしまうので、代用データ(m_tmpBuf)を使う
	//
	// 2007.01.19 ryoji 代用データ使用をコメントアウト
	// 使用すると現状では結果に１バイト余分なゴミが付加される
	// 置換に失敗するのはnLenが０に限らず nLen = nStart のとき（行頭マッチだけ対策しても．．．）
	//
	//if( nLen == 0 ) {
	//	szTarget = m_tmpBuf;
	//	nLen = 1;
	//}
	//	To Here 2003.05.03 かろと

	m_szMsg[0] = '\0';		//!< エラー解除
	if (!ExistBSubstEx()) {
		result = BSubst( NULL, szTarget + nStart, szTarget + nLen, &m_pRegExp, m_szMsg );
	} else {
		result = BSubstEx( NULL, szTarget, szTarget + nStart, szTarget + nLen, &m_pRegExp, m_szMsg );
	}
	m_szTarget = szTarget;

	//	メッセージが空文字列でなければ何らかのエラー発生。
	//	サンプルソース参照
	if( m_szMsg[0] ) {
		return 0;
	}

	if( result < 0 ) {
		// 置換するものがなかった
		return 0;
	}
	return result;
}
//>> 2002/03/27 Azumaiya


const TCHAR* CBregexp::GetLastMessage() const
{
	return to_tchar(m_szMsg);
}












//	From Here Jun. 26, 2001 genta
/*!
	与えられた正規表現ライブラリの初期化を行う．
	メッセージフラグがONで初期化に失敗したときはメッセージを表示する．

	@retval true 初期化成功
	@retval false 初期化に失敗

	@date 2007.08.12 genta 共通設定からDLL名を取得する
*/
bool InitRegexp(
	HWND		hWnd,			//!< [in] ダイアログボックスのウィンドウハンドル。バージョン番号の設定が不要であればNULL。
	CBregexp&	rRegexp,		//!< [in] チェックに利用するCBregexpクラスへの参照
	bool		bShowMessage	//!< [in] 初期化失敗時にエラーメッセージを出すフラグ
)
{
	//	From Here 2007.08.12 genta
	DLLSHAREDATA* pShareData = &GetDllShareData();

	LPCTSTR RegexpDll = pShareData->m_Common.m_sSearch.m_szRegexpLib;
	//	To Here 2007.08.12 genta

	EDllResult eDllResult = rRegexp.InitDll(RegexpDll);
	if( DLL_SUCCESS != eDllResult ){
		if( bShowMessage ){
			LPCTSTR pszMsg = _T("");
			if(eDllResult==DLL_LOADFAILURE){
				pszMsg = LS(STR_BREGONIG_LOAD);
			}
			else if(eDllResult==DLL_INITFAILURE){
				pszMsg = LS(STR_BREGONIG_INIT);
			}
			else{
				pszMsg = LS(STR_BREGONIG_ERROR);
				assert(0);
			}
			::MessageBox( hWnd, pszMsg, LS(STR_BREGONIG_TITLE), MB_OK | MB_ICONEXCLAMATION );
		}
		return false;
	}
	return true;
}

/*!
	正規表現ライブラリの存在を確認し、あればバージョン情報を指定コンポーネントにセットする。
	失敗した場合には空文字列をセットする。

	@retval true バージョン番号の設定に成功
	@retval false 正規表現ライブラリの初期化に失敗
*/
bool CheckRegexpVersion(
	HWND	hWnd,			//!< [in] ダイアログボックスのウィンドウハンドル。バージョン番号の設定が不要であればNULL。
	int		nCmpId,			//!< [in] バージョン文字列を設定するコンポーネントID
	bool	bShowMessage	//!< [in] 初期化失敗時にエラーメッセージを出すフラグ
)
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		if( hWnd != NULL ){
			::DlgItem_SetText( hWnd, nCmpId, _T(" "));
		}
		return false;
	}
	if( hWnd != NULL ){
		::DlgItem_SetText( hWnd, nCmpId, cRegexp.GetVersionT() );
	}
	return true;
}

/*!
	正規表現が規則に従っているかをチェックする。

	@param szPattern [in] チェックする正規表現
	@param hWnd [in] メッセージボックスの親ウィンドウ
	@param bShowMessage [in] 初期化失敗時にエラーメッセージを出すフラグ
	@param nOption [in] 大文字と小文字を無視して比較するフラグ // 2002/2/1 hor追加

	@retval true 正規表現は規則通り
	@retval false 文法に誤りがある。または、ライブラリが使用できない。
*/
bool CheckRegexpSyntax(
	const wchar_t*	szPattern,
	HWND			hWnd,
	bool			bShowMessage,
	int				nOption,
	bool			bKakomi
)
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		return false;
	}
	if( nOption == -1 ){
		nOption = CBregexp::optCaseSensitive;
	}
	if( !cRegexp.Compile( szPattern, NULL, nOption, bKakomi ) ){	// 2002/2/1 hor追加
		if( bShowMessage ){
			::MessageBox( hWnd, cRegexp.GetLastMessage(),
				LS(STR_BREGONIG_TITLE), MB_OK | MB_ICONEXCLAMATION );
		}
		return false;
	}
	return true;
}
//	To Here Jun. 26, 2001 genta
