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
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "extmodule/CBregexp.h"

#include "charset/charcode.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "apiwrap/StdControl.h"

// Compile時、行頭置換(len=0)の時にダミー文字列(１つに統一) by かろと
const wchar_t CBregexp::m_tmpBuf[2] = L"\0";

CBregexp::CBregexp()
: m_pRegExp( nullptr )
{
	m_szMsg[0] = L'\0';
}

CBregexp::~CBregexp() = default;

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

	nLen = (int)wcslen(szPattern);
	if (szPattern2 == nullptr) {
		// 検索(BMatch)時
		szNPattern = new wchar_t[ nLen + 15 ];	//	15：「s///option」が余裕ではいるように。
		pPat = szNPattern;
		*pPat++ = L'm';
	}
	else {
		// 置換(BSubst)時
		nLen2 = int(wcslen(szPattern2) + wcslen(szAdd2));
		szNPattern = new wchar_t[ nLen + nLen2 + 15 ];
		pPat = szNPattern;
		*pPat++ = L's';
	}
	*pPat++ = DELIMITER;
	while (*szPattern != L'\0') { *pPat++ = *szPattern++; }
	*pPat++ = DELIMITER;
	if (szPattern2 != nullptr) {
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
	@param[in] optPattern1	置換後文字列パターン(検索時はstd::nullopt)
	@param[in] nOption		検索・置換オプション

	@retval true 成功
	@retval false 失敗
*/
bool CBregexp::Compile(
	const std::wstring& szPattern0,
	int					nOption,
	const std::optional<std::wstring>& optPattern1,
	bool				bKakomi
)
{
	//	DLLが利用可能でないときはエラー終了
	if( !IsAvailable() )
		return false;

	//	前回のコンパイル情報を破棄
	m_Pattern = nullptr;

	// ライブラリに渡す検索パターンを作成
	// 別関数で共通処理に変更 2003.05.03 by かろと
	wchar_t *szNPattern = nullptr;
	const wchar_t *pszNPattern = nullptr;
	if( bKakomi ){
		pszNPattern = std::data(szPattern0);
	}else{
		szNPattern = MakePatternAlternate(std::data(szPattern0), optPattern1.has_value() ? std::data(optPattern1.value()) : nullptr, nOption );
		pszNPattern = szNPattern;
	}
	auto targetbegp = LPWSTR(std::data(m_tmpBuf));
	auto targetp = targetbegp + 0;
	auto targetendp = targetbegp + std::size(m_tmpBuf) - 1;

	m_szMsg[0] = L'\0';		//!< エラー解除

	if (!optPattern1.has_value()) {
		// 検索実行
		BMatchExW(pszNPattern, targetbegp, targetp, targetendp, &m_pRegExp, m_szMsg);
	} else {
		// 置換実行
		BSubstExW(pszNPattern, targetbegp, targetp, targetendp, &m_pRegExp, m_szMsg);
	}
	delete [] szNPattern;

	m_Pattern = std::make_unique<CPattern>(*this, m_pRegExp, m_szMsg);

	//	メッセージが空文字列でなければ何らかのエラー発生。
	//	サンプルソース参照
	if( m_szMsg[0] ){
		m_Pattern = nullptr;
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
	if( (!IsAvailable()) || m_pRegExp == nullptr ){
		return false;
	}

	m_szMsg[0] = '\0';		//!< エラー解除

	//	検索文字列＝NULLを指定すると前回と同一の文字列と見なされる
	matched = BMatchExW(nullptr, target, target + nStart, target + len, &m_pRegExp, m_szMsg);

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
	if( !IsAvailable() || m_pRegExp == nullptr )
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

	result = BSubstExW(nullptr, szTarget, szTarget + nStart, szTarget + nLen, &m_pRegExp, m_szMsg);

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

const WCHAR* CBregexp::GetLastMessage() const
{
	return m_szMsg;
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

	LPCWSTR RegexpDll = pShareData->m_Common.m_sSearch.m_szRegexpLib;
	//	To Here 2007.08.12 genta

	EDllResult eDllResult = rRegexp.InitDll(RegexpDll);
	if( DLL_SUCCESS != eDllResult ){
		if( bShowMessage ){
			LPCWSTR pszMsg = L"";
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
		if( hWnd != nullptr ){
			ApiWrap::DlgItem_SetText( hWnd, nCmpId, L" ");
		}
		return false;
	}
	if( hWnd != nullptr ){
		ApiWrap::DlgItem_SetText( hWnd, nCmpId, cRegexp.GetVersionW() );
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
	if( !cRegexp.Compile( szPattern, nullptr, nOption, bKakomi ) ){	// 2002/2/1 hor追加
		if( bShowMessage ){
			std::wstring message(LS(STR_REGEX_COMPILE_ERR_PREAMBLE));
			message += cRegexp.GetLastMessage();
			::MessageBox( hWnd, message.c_str(),
				LS(STR_BREGONIG_TITLE), MB_OK | MB_ICONEXCLAMATION );
		}
		return false;
	}
	return true;
}
//	To Here Jun. 26, 2001 genta

CBregexp::CPattern::CPattern(
	const CBregOnig& cDll,
	BREGEXP* pRegExp,
	const std::wstring& msg
) noexcept
	: m_cDll(cDll)
	, m_pRegExp(pRegExp)
	, m_Msg(msg)
{
}

CBregexp::CPattern::~CPattern() noexcept
{
	if (m_pRegExp) {
		m_cDll.BRegfreeW(m_pRegExp);
		m_pRegExp = nullptr;
	}
}
