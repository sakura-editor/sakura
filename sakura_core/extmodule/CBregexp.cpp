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

CBregexp::CBregexp() = default;

CBregexp::~CBregexp() = default;

/*!
 * @brief ライブラリに渡すための検索・置換パターンを作成する
 *
 * @retval ライブラリに渡す検索パターン文字列
 * 
 * @date 2003.05.03 かろと 関数に切り出し
 */
std::wstring CBregexp::_QuoteRegex(
	std::wstring_view   szPattern0,					//!< 検索パターン
	int					nOption,					//!< 検索オプション
	const std::optional<std::wstring>& optPattern1	//!< 置換パターン(検索時はstd::nullopt)
) const
{
	constexpr auto DELIMITER = WCODE::BREGEXP_DELIMITER;
	constexpr auto OPT_SIZE = 15;	//	15：「s///option」が余裕ではいるように。

	// 検索パターン作成
	std::wstring outPattern;		//!< ライブラリ渡し用の検索パターン文字列

	if (!optPattern1.has_value()) {
		// 検索(BMatch)時
		outPattern.reserve(szPattern0.size() + OPT_SIZE);
		outPattern = L'm';
	}
	else {
		// 置換(BSubst)時
		outPattern.reserve(szPattern0.size() + OPT_SIZE + optPattern1->size());
		outPattern = L's';
	}

	outPattern += DELIMITER;

	outPattern += std::data(szPattern0);

	if (optPattern1.has_value()) {
		outPattern += DELIMITER;
		outPattern += std::data(*optPattern1);
	}

	outPattern += DELIMITER;

	outPattern += L'k';			// 漢字対応
	outPattern += L'm';			// 複数行対応(但し、呼び出し側が複数行対応でない)

	// 2006.01.22 かろと 論理逆なので bIgnoreCase -> optCaseSensitiveに変更
	if( !(nOption & optCaseSensitive) ) {		// 2002/2/1 hor IgnoreCase オプション追加 マージ：aroka
		outPattern += L'i';		// 大文字小文字を同一視(無視)する
	}
	// 2006.01.22 かろと 行単位置換のために、全域オプション追加
	if (nOption & optGlobal) {
		outPattern += L'g';			// 全域(global)オプション、行単位の置換をする時に使用する
	}
	if (nOption & optExtend) {
		outPattern += L'x';
	}
	if (nOption & optASCII) {
		outPattern += L'a';
	}
	if (nOption & optUnicode) {
		outPattern += L'u';
	}
	if (nOption & optDefault) {
		outPattern += L'd';
	}
	if (nOption & optLocale) {
		outPattern += L'l';
	}
	if (nOption & optR) {
		outPattern += L'R';
	}

	return outPattern;
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
std::wstring CBregexp::_MakePattern(
	const std::wstring& szSearch,
	int					nOption,
	const std::optional<std::wstring>& optReplace
) const
{
	// 代替パターンの文字列長を概算する
	size_t approximateSize = std::size(szSearch);

	using namespace std::string_view_literals;

	const auto alternateDotPattern = LR"([^\r\n])"sv;
	const auto alternateDollarPattern = LR"((?<![\r\n])(?=\r|$))"sv;

	// すべての . を [^\r\n] へ、すべての $ を (?<![\r\n])(?=\r|$) へ置換すると仮定して、最大長を概算する
	for (const auto ch : szSearch) {
		switch (ch) {
		case L'.': approximateSize += std::size(alternateDotPattern);    break;
		case L'$': approximateSize += std::size(alternateDollarPattern); break;
		default: break;
		}
	}

	// 代替パターンは動的に構築する
	std::wstring alternateSearchPattern;
	alternateSearchPattern.reserve(approximateSize);

	// szSearch を alternateSearchPattern へ、ところどころ置換しながら順次コピーしていく。
	enum class State {
		_DL = -4, // DOLLAR (特殊文字としての)ドルを置き換える
		_DT,      // DOT (特殊文字としての)ドットを置き換える
		_XC,      // EXIT CHARCLASS charsetLevelをデクリメントして CHAか DEFへ
		_EC,      // ENTER CHARCLASS charsetLevelをインクリメントして CHAへ
		DEF,      // DEFULT 一番外側
		D_E,      // DEFAULT_ESCAPED 一番外側で \の次
		CHA,      // CHARSET 文字クラスの中
		C_E,      // CHARSET_ESCAPED 文字クラスの中で \の次
		QEE,      // QEESCAPE \Q...\Eの中
		Q_E,      // QEESCAPE_ESCAPED \Q...\Eの中で \の次
		NUMBER_OF_STATE,
	};
	enum class CharClass {
		OTHER = 0,
		DOT,    /* . */
		DOLLAR, /* $ */
		LARGEQ, /* Q */
		LARGEE, /* E */
		LBRCKT, /* [ */
		RBRCKT, /* ] */
		ESCAPE, /* \ */
		NUMBER_OF_CHARCLASS
	};

	using enum State;
	using enum CharClass;

	using States = std::array<State, (int)NUMBER_OF_CHARCLASS>;
	const std::array<States, (int)NUMBER_OF_STATE> state_transition_table = {
	/*                OTHER  DOT  DOLLAR  LARGEQ LARGEE LBRCKT RBRCKT ESCAPE*/
	/* DEF */ States{ DEF,  _DT,   _DL,    DEF,   DEF,   _EC,   DEF,   D_E},
	/* D_E */ States{ DEF,  DEF,   DEF,    QEE,   DEF,   DEF,   DEF,   DEF},
	/* CHA */ States{ CHA,  CHA,   CHA,    CHA,   CHA,   _EC,   _XC,   C_E},
	/* C_E */ States{ CHA,  CHA,   CHA,    CHA,   CHA,   CHA,   CHA,   CHA},
	/* QEE */ States{ QEE,  QEE,   QEE,    QEE,   QEE,   QEE,   QEE,   Q_E},
	/* Q_E */ States{ QEE,  QEE,   QEE,    QEE,   DEF,   QEE,   QEE,   Q_E}
	};
	const std::unordered_map<WCHAR, CharClass> wcharToClassMap = {
		{ L'.',  DOT    },
		{ L'$',  DOLLAR },
		{ L'Q',  LARGEQ },
		{ L'E',  LARGEE },
		{ L'[',  LBRCKT },
		{ L']',  RBRCKT },
		{ L'\\', ESCAPE },
	};

	State state = DEF;
	int charsetLevel = 0; // ブラケットの深さ。POSIXブラケット表現など、エスケープされていない [] が入れ子になることがある。
	auto left = std::data(szSearch);
	auto right = std::data(szSearch);
	for (; *right; ++right) {
		const auto ch = *right;
		const CharClass charClass = wcharToClassMap.contains(ch) ? wcharToClassMap.at(ch) : OTHER;
		auto nextState = state_transition_table[(int)state][(int)charClass];
		if (int(DEF) <= int(nextState)) {
			state = nextState;
			continue;
		}

		switch (nextState) {
		case _EC: // ENTER CHARSET
			++charsetLevel;
			state = CHA;
			break;

		case _XC: // EXIT CHARSET
			--charsetLevel;
			state = 0 < charsetLevel ? CHA : DEF;
			break;

		case _DT: // DOT(match anything)
			alternateSearchPattern.append(left, right);
			left = right + 1;
			alternateSearchPattern += alternateDotPattern;
			break;

		case _DL: // DOLLAR(match end of line)
			alternateSearchPattern.append(left, right);
			left = right + 1;
			alternateSearchPattern += alternateDollarPattern;
			break;

		default: // ここには絶対こない
			break;
		}
	}
	alternateSearchPattern.append(left, right + 1); // right + 1 は '\0' の次を指す(明示的に '\0' をコピー)。

	return _QuoteRegex(alternateSearchPattern, nOption, optReplace);
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
	const std::optional<std::wstring>& optPattern1
)
{
	//	DLLが利用可能でないときはエラー終了
	if( !IsAvailable() )
		return false;

	//	前回のコンパイル情報を破棄
	m_Pattern = nullptr;

	// ライブラリに渡す検索パターンを作成
	// 別関数で共通処理に変更 2003.05.03 by かろと
	const auto quotedRegexPattern = _MakePattern(szPattern0, nOption, optPattern1);

	auto targetbegp = LPWSTR(std::data(m_tmpBuf));
	auto targetp = targetbegp + 0;
	auto targetendp = targetbegp + std::size(m_tmpBuf) - 1;

	BREGEXP* pRegExp = nullptr;
	std::wstring msg(80, L'\0');

	if (!optPattern1.has_value()) {
		// 検索実行
		BMatchExW(quotedRegexPattern.c_str(), targetbegp, targetp, targetendp, &pRegExp, msg);
	} else {
		// 置換実行
		BSubstExW(quotedRegexPattern.c_str(), targetbegp, targetp, targetendp, &pRegExp, msg);
	}

	m_Pattern = std::make_unique<CPattern>(*this, pRegExp, msg);

	//	メッセージが空文字列でなければ何らかのエラー発生。
	//	サンプルソース参照
	if (msg[0]) {
		return false;
	}
	
	// 行頭条件チェックは、MakePatternに取り込み 2003.05.03 by かろと

	return true;
}

/*!
	JRE32のエミュレーション関数．既にあるコンパイル構造体を利用して検索（1行）を
	行う．

	@param[in] target 検索対象データ
	@param[in] offset 検索開始位置．(先頭は0)

	@retval true Match
	@retval false No Match または エラー。エラーは GetLastMessage()により判定可能。

*/
bool CBregexp::Match(std::wstring_view target, size_t offset) const noexcept
{
	//	DLLが利用可能でないとき、または構造体が未設定の時はエラー終了
	if (!IsAvailable() || !m_Pattern) {
		return false;
	}

	return m_Pattern->Match(target, offset);
}

bool CBregexp::CPattern::Match(std::wstring_view target, size_t offset)
{
	// 構造体が未設定の時は即終了
	if (!m_pRegExp) {
		return false;
	}

	auto targetbegp = LPWSTR(std::data(target));
	auto targetp = targetbegp + offset;
	auto targetendp = targetbegp + std::size(target);

	m_Msg.clear();		//!< エラー解除

	//	検索文字列＝NULLを指定すると前回と同一の文字列と見なされる
	const auto matched = m_cDll.BMatchExW(nullptr, targetbegp, targetp, targetendp, &m_pRegExp, m_Msg);

	m_Target = target;

	if (matched < 0 || m_Msg[0]) {
		// BMatchエラー
		// エラー処理をしていなかったので、nStart>=lenのような場合に、マッチ扱いになり
		// 無限置換等の不具合になっていた 2003.05.03 by かろと
		return false;
	}

	return 0 < matched;
}

/*!
	正規表現による文字列置換
	既にあるコンパイル構造体を利用して置換（1行）を
	行う．

	@param[in] target 置換対象データ
	@param[in] offset 置換開始位置(0からnLen未満)

	@retval 置換個数

	@date 2002/03/27 Azumaiya 追加
	@date	2007.01.16 ryoji 戻り値を置換個数に変更
*/
int CBregexp::Replace(std::wstring_view target, size_t offset) const noexcept
{
	//	DLLが利用可能でないとき、または構造体が未設定の時はエラー終了
	if (!IsAvailable() || !m_Pattern) {
		return 0;
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

	return m_Pattern->Replace(target, offset);
}

int CBregexp::CPattern::Replace(std::wstring_view target, size_t offset)
{
	// 構造体が未設定の時は即終了
	if (!m_pRegExp) {
		return false;
	}

	auto targetbegp = LPWSTR(std::data(target));
	auto targetp = targetbegp + offset;
	auto targetendp = targetbegp + std::size(target);

	m_Msg.clear();		//!< エラー解除

	// 検索文字列＝NULLを指定すると前回と同一の文字列と見なされる
	const auto result = m_cDll.BSubstExW(nullptr, targetbegp, targetp, targetendp, &m_pRegExp, m_Msg);

	m_Target = target;

	//	メッセージが空文字列でなければ何らかのエラー発生。
	//	サンプルソース参照
	if (result < 0 || m_Msg[0]) {
		// 置換するものがなかった、または、なんかエラー
		return 0;
	}

	return result;
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
	if (const auto eDllResult = rRegexp.InitDll(); DLL_SUCCESS != eDllResult) {
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
	std::wstring pattern;
	if (bKakomi) {
		if (std::wcmatch m; std::regex_match(szPattern, m, std::wregex(LR"(^m(.)(.+)\1[kmigxaudlR]*$)"))) {
			pattern = m[2];
		} else {
			return false;
		}
	} else {
		pattern = szPattern;
	}
	if (!cRegexp.Compile(pattern, nOption)) {
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
