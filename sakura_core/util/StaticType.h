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
 * 既存コードにある生配列を最小の変更で置き換えるために、
 * C++の作法に照らして不適切な演算子を多く定義している。
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
	static constexpr size_t size() noexcept { return MAX_SIZE; }

	StaticVector() = default;

	/*!
	 * @brief データを指定して構築する
	 */
	template<std::ranges::sized_range T>
	constexpr explicit StaticVector(const T& source)
	{
		// 要素数がバッファサイズを越えたら例外を投げる
		const auto sourceSize = std::size(source);
		if (size() < sourceSize) {
			throw std::out_of_range(std::format("source has too many elements. (elements: {}, allowed: {})", sourceSize, size()));
		}

		m_nCount = static_cast<int>(sourceSize);

		std::ranges::copy(source, m_aElements.begin());
	}

	/*!
	 * @brief イニシャライザで構築する
	 */
	constexpr explicit StaticVector(std::initializer_list<const ElementType> source)
		: StaticVector(std::span(source.begin(), source.size()))
	{
	}

	//属性
	constexpr size_t count() const noexcept { return m_nCount; }
	constexpr bool empty() const noexcept { return 0 == m_nCount; }

	constexpr auto begin() noexcept { return m_aElements.begin(); }
	constexpr auto end() noexcept { return m_aElements.begin() + size(); }

	auto begin() const noexcept { return m_aElements.begin(); }
	auto end() const noexcept { return m_aElements.begin() + count(); }

	constexpr       auto* data()        noexcept { return std::data(m_aElements); }
	constexpr const auto* data()  const noexcept { return std::data(m_aElements); }

	constexpr explicit		 operator std::span<ElementType, size()>() & noexcept { return std::span<ElementType, size()>(data(), size()); }
	constexpr /* implicit */ operator std::span<ElementType>() & noexcept { return operator std::span<ElementType, size()>(); }
	constexpr /* implicit */ operator std::span<const ElementType>() const & noexcept { return std::span(data(), count()); }
	constexpr /* implicit */ operator const ElementType*() const & noexcept { return data(); }

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
		if (count() <= nIndex) {
			throw std::out_of_range(std::format("nIndex is out of range. (nIndex: {}, allowed: {})", nIndex, count() - 1));
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
		if (size() <= nIndex) {
			throw std::out_of_range(std::format("nIndex is out of range. (nIndex: {}, allowed: {})", nIndex, size() - 1));
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
		if (size() < nNewSize) {
			throw std::out_of_range(std::format("nNewSize is out of range. (nNewSize: {}, allowed: {})", nNewSize, size() - 1));
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
		if (const auto maxSize = static_cast<int>(size());
			maxSize < m_nCount)
		{
			m_nCount = maxSize;
		}
		else if (m_nCount < 0)
		{
			m_nCount = 0;
		}
	}

private:
	int         m_nCount = 0;
	ArrayType	m_aElements{};
};

//! ヒープを用いない文字列クラス
//2007.09.23 kobake 作成。
template <int N_BUFFER_COUNT>
class StaticString{
private:
	//テンプレート定数名が長過ぎて不便なので、エイリアスを切る
	static constexpr auto N = N_BUFFER_COUNT;

	using ArrayType = std::array<WCHAR, N>;
	using Traits = std::char_traits<WCHAR>;

	using Me = StaticString<N>;

public:
	static constexpr auto BUFFER_COUNT = N_BUFFER_COUNT;

	static constexpr auto size() noexcept { return BUFFER_COUNT; }

	//コンストラクタ・デストラクタ
	StaticString() = default;
	constexpr explicit StaticString(std::wstring_view src) { assign(src); }

	/*!
	 * 文字列を末尾に追加する
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
	 * 文字列を代入する
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

	/*!
	 * 文字列長を取得する
	 */
	constexpr size_t length() const noexcept
	{
		const auto pos = Traits::find(data(), size(), L'\0');
		return pos ? static_cast<size_t>(pos - data()) : size() - 1;
	}

	constexpr bool empty() const noexcept { return 0 == m_szData[0]; }

	constexpr auto begin() noexcept { return m_szData.begin(); }
	constexpr auto end() noexcept { return m_szData.end() - 1; }

	auto begin() const noexcept { return m_szData.begin(); }
	auto end() const noexcept { return m_szData.begin() + length(); }

	constexpr       WCHAR* data()        noexcept { return std::data(m_szData); }
	constexpr const WCHAR* data()  const noexcept { return std::data(m_szData); }
	constexpr const WCHAR* c_str() const noexcept { return data(); }

	constexpr operator std::span<WCHAR, N>()       & noexcept { return std::span<WCHAR, N>{ data(), N }; }
	constexpr operator std::wstring_view()   const & noexcept { return std::wstring_view{ data(), length() }; }
	constexpr operator std::span<WCHAR>()          & noexcept { return operator std::span<WCHAR, N>(); }

	explicit operator std::filesystem::path() const & noexcept { return static_cast<std::wstring_view>(*this); }

	constexpr Me& operator = (std::wstring_view rhs) noexcept { assign(rhs); return *this; }
	constexpr Me& operator = (const std::wstring& rhs) noexcept { assign(rhs); return *this; }
	constexpr Me& operator = (const std::filesystem::path& path) noexcept { assign(path.wstring()); return *this; }

	constexpr Me& operator += (std::wstring_view rhs) noexcept { append(rhs); return *this; }
	constexpr Me& operator += (const std::wstring& rhs) noexcept { append(rhs); return *this; }

	//クラス属性
	size_t GetBufferCount() const{ return N_BUFFER_COUNT; }

	//データアクセス
	WCHAR*       GetBufferPointer()      { return data(); }
	const WCHAR* GetBufferPointer() const{ return data(); }

	//簡易データアクセス
	constexpr operator       WCHAR*()       & noexcept { return data(); }
	constexpr operator const WCHAR*() const & noexcept { return data(); }

	WCHAR At(int nIndex) const{ return m_szData[nIndex]; }

	//簡易コピー
	void Assign(const WCHAR* src) noexcept { assign(std::wstring_view{ src ? src : L"" }); }
	Me& operator = (const WCHAR* src){ Assign(src); return *this; }

	//各種メソッド
	int Length() const noexcept { return static_cast<int>(length()); }

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
