/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_
#define SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_
#pragma once

#include "util/string_ex.h"
#include "debug/Debug2.h"

#include <array>
#include <initializer_list>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

/*!
 * @brief ヒープを用いない可変長配列クラス
 *
 * 有効要素数と固定長バッファをクラス内部に保持する可変長配列クラス。
 * ヒープ領域を使用しないため、共有メモリに配置できる。
 *
 * @code{.cpp}
 * using SMruList = StaticVector<SFilePath, 50>;
 * SMruList mruList{};
 *
 * // 以下とおおむね同等。
 * int nMruFileNum = 0;
 * SFilePath aMruFileArr[50] {};
 * @endcode
 *
 * @tparam ELEMENT_TYPE 要素型。
 * @tparam MAX_SIZE バッファの要素数。
 * @tparam SET_TYPE push_backで追加する型。
 *
 * @author kobake
 * @date 2007.09.23 kobake 作成
 */
template <class ELEMENT_TYPE, int MAX_SIZE, class SET_TYPE = const ELEMENT_TYPE&>
class StaticVector final {
public:
	//型
	using ElementType = ELEMENT_TYPE;

private:
	using ArrayType = std::array<ElementType, MAX_SIZE>;

	using Me = StaticVector<ElementType, MAX_SIZE, SET_TYPE>;

public:
	/*!
	 * @brief バッファサイズを取得する
	 */
	static int max_size() noexcept { return MAX_SIZE; }

	/*!
	 * @brief デフォルトコンストラクタ
	 *
	 * 有効要素数0、全要素デフォルト値で構築する。
	 */
	StaticVector() = default;

	/*!
	 * @brief イニシャライザで構築する
	 */
	constexpr explicit StaticVector(std::initializer_list<const ElementType> source)
		: StaticVector(std::span(source.begin(), source.size()))
	{
	}

	/*!
	 * @brief データを指定して構築する
	 *
	 * @tparam S [in] 要素の型。
	 * @param source [in] 初期データ。
	 * @throws std::out_of_range 初期データの要素数がバッファサイズを越える場合。
	 */
	template<std::ranges::sized_range S>
	constexpr explicit StaticVector(const S& source)
	{
		// 要素数がバッファサイズを越えたら例外を投げる
		const auto sourceSize = std::size(source);
		if (static_cast<size_t>(MAX_SIZE) < sourceSize) {
			throw std::out_of_range(std::format("source has too many elements. (elements: {}, allowed: {})", sourceSize, MAX_SIZE));
		}

		m_nCount = static_cast<int>(sourceSize);

		std::ranges::copy(source, m_aElements.begin());
	}

	//属性
	/*!
	 * @brief 有効要素数を取得する
	 */
	constexpr int size() const noexcept { return m_nCount; }

	constexpr auto begin() noexcept { return m_aElements.begin(); }
	constexpr auto end() noexcept { return m_aElements.begin() + MAX_SIZE; }

	auto begin() const noexcept { return m_aElements.begin(); }
	auto end() const noexcept { return m_aElements.begin() + m_nCount; }

	constexpr       auto* data()        noexcept { return std::data(m_aElements); }
	constexpr const auto* data()  const noexcept { return std::data(m_aElements); }

	constexpr std::span<ElementType, MAX_SIZE> span()       noexcept { return std::span(data(), MAX_SIZE); }
	constexpr std::span<const ElementType>     span() const noexcept { return std::span(data(), m_nCount); }

	//要素アクセス
	/*!
	 * @brief 指定した要素を取得する（読み書き可能。）
	 *
	 * @param nIndex [in] 取得する要素のインデックス。
	 * @returns 指定した要素への参照。（読み書き可能。）
	 * @throws std::out_of_range インデックスがバッファの要素数を超える場合。
	 */
	constexpr ElementType& operator[](size_t nIndex)
	{
		// 有効要素数を越えたら例外を投げる
		if (size_t(m_nCount) <= nIndex) {
			throw std::out_of_range(std::format("nIndex is out of range. (nIndex: {}, allowed: {})", nIndex, m_nCount - 1));
		}

		return m_aElements[nIndex];
	}

	/*!
	 * @brief 指定した要素を取得する（読み取り専用。）
	 *
	 * @param nIndex [in] 取得する要素のインデックス。
	 * @returns 指定した要素への参照。（読み取り専用。）
	 * @throws std::out_of_range インデックスがバッファの要素数を超える場合。
	 */
	constexpr const ElementType& operator[](size_t nIndex) const
	{
		// バッファサイズを越えたら例外を投げる
		if (size_t(MAX_SIZE) <= nIndex) {
			throw std::out_of_range(std::format("nIndex is out of range. (nIndex: {}, allowed: {})", nIndex, MAX_SIZE - 1));
		}

		return m_aElements[nIndex];
	}

	//操作
	void clear() noexcept { m_nCount=0; }

	/*!
	 * @brief 配列末尾に要素を追加する
	 *
	 * @tparam Args... 要素の構築に必要な引数群。
	 * @param args [in] 追加するデータ。
	 * @throws std::out_of_range 有効要素数が既にバッファの要素数に達していた場合。
	 */
	template<typename ... Args>
	void emplace_back(Args&& ...args)
	{
		// 変更前の有効要素数を取得する
		const auto countOld = m_nCount;

		// 有効要素数を1増やす
		resize(countOld + 1);

		// 末尾要素に代入する
		m_aElements[countOld] = ElementType(std::forward<Args>(args)...);
	}

	/*!
	 * @brief 配列末尾に要素を追加する
	 *
	 * @param e [in] 追加するデータ。
	 * @throws std::out_of_range 有効要素数が既にバッファの要素数に達していた場合。
	 */
	void push_back(SET_TYPE e)
	{
		// 変更前の有効要素数を取得する
		const auto countOld = m_nCount;

		// 有効要素数を1増やす
		resize(countOld + 1);

		// 末尾要素に代入する
		m_aElements[countOld] = e;
	}

	/*!
	 * @brief 有効要素数を更新する
	 *
	 * @param nNewSize [in] 新しい有効要素数。
	 * @throws std::out_of_range 有効要素数がバッファの要素数を超える場合。
	 */
	constexpr void resize(size_t nNewSize)
	{
		// バッファサイズを越えたら例外を投げる
		if (size_t(MAX_SIZE) < nNewSize) {
			throw std::out_of_range(std::format("nNewSize is out of range. (nNewSize: {}, allowed: {})", nNewSize, MAX_SIZE - 1));
		}
		m_nCount = static_cast<int>(nNewSize);
	}
	
	//! 要素数が0でも要素へのポインタを取得
	ElementType* dataPtr() noexcept { return &m_aElements.front();}

	//特殊
	/*!
	 * @brief 有効要素数への参照を取得する
	 *
	 * カプセル化を無効化する効果がある。
	 *
	 * 存在自体が「問題あり」寄り。
	 *
	 * CRecent派生クラスで使ってるので削除できない。
	 */
	 // TODO: いつか廃止する
	int& _GetSizeRef(){ return m_nCount; }

	/*!
	 * @brief の有効要素数を妥当な値に更新する
	 *
	 * 有効要素数に不正な値を入れてしまったあとに補正するためのもの。
	 *
	 * 存在自体が「問題あり」寄り。
	 */
	 // TODO: いつか廃止する
	void SetSizeLimit(){
		if( MAX_SIZE < m_nCount ){
			m_nCount = MAX_SIZE;
		}else if( m_nCount < 0 ){
			m_nCount = 0;
		}
	}

	constexpr explicit operator std::span<ElementType, MAX_SIZE>()   & noexcept { return span(); }
	constexpr explicit operator std::span<ElementType>()             & noexcept { return span(); }
	constexpr explicit operator std::span<const ElementType>() const & noexcept { return span(); }

private:
	int         m_nCount = 0;
	ArrayType	m_aElements{};
};

/*!
 * @brief ヒープを用いない文字列クラス
 *
 * 固定長の文字バッファをクラス内部に保持する文字列クラス。
 * ヒープ領域を使用しないため、共有メモリに配置できる。
 *
 * 既存コードにある生配列を最小の変更で置き換えるために、
 * C++の作法に照らして不適切な演算子を多く定義している。
 *
 * Windows APIには指定できる文字列に長さ制限があるものも多いため、
 * LPCWSTRへの暗黙変換機能を外したとしてもクラスの存在価値はなくならない。
 *
 * @code{.cpp}
 * using SFilePath = StaticString<_MAX_PATH>;
 * SFilePath filePath{};
 *
 * // 以下と同等。
 * WCHAR szFilePath[_MAX_PATH] {};
 * @endcode
 *
 * @tparam N バッファサイズ。最大文字列長 + 1（NUL終端）を指定すること。
 *
 * @author kobake
 * @date 2007.09.23 kobake 作成
 */
template <int N>
// TODO: final指定したいが継承クラスがあるのでコメントアウトしている
class StaticString /* final */ {
private:
	// 文字バッファの要素数は 1以上 を想定する
	static_assert(1 <= N, "BUFFER_COUNT must be greater than 0.");

	using ArrayType = std::array<WCHAR, N>;
	using Traits = std::char_traits<WCHAR>;

	using Me = StaticString<N>;

#pragma push_macro("DISABLE_IMPLICIT_OPERATORS")

// C++の作法に照らして不適切な演算子を無効にするマクロ（これを有効にするとクラスの存在価値が半減することに注意。
#define DISABLE_IMPLICIT_OPERATORS 0

public:
	static constexpr auto BUFFER_COUNT = N;

	static constexpr auto size() noexcept { return BUFFER_COUNT; }

	//コンストラクタ・デストラクタ
	StaticString() = default;

	/*!
	 * @brief 文字列をコピーして構築する
	 *
	 * @param source [in] コピーする文字列
	 */
	constexpr explicit StaticString(
		std::wstring_view source
	)
	{
		assign(source);
	}

	/*!
	 * @brief 文字列を末尾に追加する
	 *
	 * @retval 0 成功
	 * @retval STRUNCATE 切り詰め発生
	 */
	constexpr errno_t append(std::wstring_view src) noexcept
	{
		const auto len = length();
		const auto count = std::min<size_t>(std::size(src), size() - len - 1);
		Traits::move(data() + len, std::data(src), count);
		Traits::assign(data()[len + count], L'\0');
		return count < std::size(src) ? STRUNCATE : 0;
	}

	/*!
	 * @brief 文字列を代入する
	 *
	 * @retval 0 成功
	 * @retval STRUNCATE 切り詰め発生
	 */
	constexpr errno_t assign(std::wstring_view src) noexcept
	{
		const auto count = std::min<size_t>(std::size(src), size() - 1);
		Traits::move(data(), std::data(src), count);
		Traits::assign(data()[count], L'\0');
		return count < std::size(src) ? STRUNCATE : 0;
	}

	constexpr auto begin() noexcept { return m_szData.begin(); }
	constexpr auto end() noexcept { return m_szData.end() - 1; }

	auto begin() const noexcept { return m_szData.begin(); }
	auto end() const noexcept { return m_szData.begin() + length(); }

	constexpr auto c_str() const noexcept { return data(); }

	constexpr auto data()        noexcept { return std::data(m_szData); }
	constexpr auto data()  const noexcept { return std::data(m_szData); }

	/*!
	 * @brief 文字列が空かどうか調べる
	 */
	constexpr bool empty() const noexcept
	{
		 return 0 == m_szData[0];
	}

	/*!
	 * @brief 文字列長を取得する
	 *
	 * @note 毎回再計算するので効率は良くない。
	 * @note 長さを頻繁に確認する用途ではstd::wstringへの移行を検討すること。
	 */
	constexpr size_t length() const noexcept
	{
		const auto len = ::wcsnlen(data(), size());
		return len < size() ? len : size() - 1;
	}

	constexpr auto span()       & noexcept { return std::span<WCHAR, size()>(data(), size()); }
	constexpr auto str()  const & noexcept { return std::wstring_view(data(), length()); }

	//クラス属性
	size_t GetBufferCount() const noexcept { return size(); }

	//データアクセス
	WCHAR*       GetBufferPointer()      { return data(); }
	const WCHAR* GetBufferPointer() const{ return data(); }

#if !DISABLE_IMPLICIT_OPERATORS

	//簡易データアクセス
	constexpr operator       WCHAR*()       & noexcept { return data(); }
	constexpr operator const WCHAR*() const & noexcept { return data(); }

#endif // #if !DISABLE_IMPLICIT_OPERATORS

	WCHAR At(int nIndex) const{ return m_szData[nIndex]; }

	//簡易コピー
	/*!
	 * @brief ポインタを指定して文字列を割り当てる
	 *
	 * @param pszData [in, opt] 割り当てる文字列を指すポインタ。
	 *
	 * @note 引数にはNULLを指定できる
	 */
	constexpr void Assign(_In_opt_z_ LPCWSTR pszData) noexcept
	{
		assign(std::wstring_view{ pszData ? pszData : L"" });
	}

	//各種メソッド
	constexpr int Length() const noexcept { return static_cast<int>(length()); }

	/*!
	 * @brief バッファの内容を置き換える
	 *
	 * @param rhs [in] 代入する文字列
	 * @return 自分自身への参照
	 */
	constexpr Me& operator = (std::wstring_view rhs)
	{
		assign(rhs);
		
		return *this;
	}

	/*!
	 * @brief バッファの内容を置き換える
	 *
	 * @param rhs [in] 代入する文字列
	 * @return 自分自身への参照
	 */
	constexpr Me& operator = (const std::wstring& rhs)
	{
		assign(rhs);
		
		return *this;
	}

	/*!
	 * @brief バッファの内容を置き換える
	 *
	 * @param src [in, opt] 代入する文字列を指すポインタ。
	 *
	 * @note 引数にはNULLを指定できる
	 */
	constexpr Me& operator = (_In_opt_z_ LPCWSTR rhs)
	{
		Assign(rhs);
		
		return *this;
	}

	/*!
	 * @brief バッファの最後に文字列を追加する
	 *
	 * @param rhs [in] 追加する文字列
	 * @return 自分自身への参照
	 */
	constexpr Me& operator += (std::wstring_view rhs) noexcept
	{
		append(rhs);

		return *this;
	}

	/*!
	 * @brief バッファの最後に文字列を追加する
	 *
	 * @param rhs [in] 追加する文字列
	 * @return 自分自身への参照
	 */
	constexpr Me& operator += (const std::wstring& rhs) noexcept
	{
		append(rhs);

		return *this;
	}

	/*!
	 * @brief 文字列バッファに変換する
	 *
	 * @return 文字列バッファ
	 */
	constexpr explicit operator std::span<WCHAR, size()>() & noexcept
	{
		return span();
	}

	/*!
	 * @brief 文字列バッファに変換する
	 *
	 * explicitを付けないのはC++の作法に照らして適切でない。
	 * C++への移行を加速させるために仮置き。
	 *
	 * @return 文字列バッファ
	 */
	// TODO: いつか explicit を付ける
	constexpr /* implicit */ operator std::span<WCHAR>() & noexcept
	{
		return span();
	}

#if DISABLE_IMPLICIT_OPERATORS

	/*!
	 * @brief 文字列に変換する演算子
	 *
	 * @return 文字列
	 */
	constexpr explicit operator std::wstring() const
	{
		return std::wstring{ str() };
	}

#endif // #if DISABLE_IMPLICIT_OPERATORS

#pragma pop_macro("DISABLE_IMPLICIT_OPERATORS")

	/*!
	 * @brief 文字列参照に変換する
	 *
	 * explicitを付けないのはC++の作法に照らして適切でない。
	 * C++への移行を加速させるために仮置き。
	 *
	 * @return 文字列参照
	 */
	// TODO: いつか explicit を付ける
	constexpr /* implicit */ operator std::wstring_view() const & noexcept
	{
		return str();
	}

	/*!
	 * @brief ファイルパスに変換する
	 *
	 * @return ファイルパス
	 */
	explicit operator std::filesystem::path() const & noexcept
	{
		return str();
	}

private:
	ArrayType	m_szData{};
};

template<int N> inline errno_t wcscpy_s(StaticString<N>& dst, std::wstring_view src)        noexcept { return dst.assign(src); }
template<int N> inline errno_t wcscat_s(StaticString<N>& dst, std::wstring_view src)        noexcept { return dst.append(src); }

template<int N> inline errno_t wcsncpy_s(StaticString<N>& dst, std::wstring_view src, size_t count) noexcept { if (_TRUNCATE != count && count < std::size(src)) src = src.substr(0, count); return wcscpy_s(dst, src); }
template<int N> inline errno_t wcsncat_s(StaticString<N>& dst, std::wstring_view src, size_t count) noexcept { if (_TRUNCATE != count && count < std::size(src)) src = src.substr(0, count); return wcscat_s(dst, src); }

template<int N>
inline int vswprintf_s(StaticString<N>& buf, const WCHAR* format, va_list& v) noexcept {
	return ::_vsnwprintf_s(std::data(buf), std::size(buf), _TRUNCATE, format, v);
}

template<int N, typename... Params>
inline int swprintf_s(StaticString<N>& buf, const WCHAR* format, Params&&... params) noexcept {
	return ::_snwprintf_s(std::data(buf), _TRUNCATE, std::size(buf), format, std::forward<Params>(params)...);
}

#endif /* SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_ */
