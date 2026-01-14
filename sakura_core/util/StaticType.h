/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_
#define SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_
#pragma once

#include <stdexcept>

#include "util/string_ex.h"
#include "debug/Debug2.h"

//! ヒープを用いないvector
//2007.09.23 kobake 作成。
template <class ELEMENT_TYPE, int MAX_SIZE, class SET_TYPE = const ELEMENT_TYPE&>
class StaticVector {
public:
	//型
	using ElementType = ELEMENT_TYPE;

private:
	using ArrayType = std::array<ElementType, MAX_SIZE>;

public:
	static int max_size() noexcept { return MAX_SIZE; }

	StaticVector() = default;

	explicit StaticVector(std::span<ELEMENT_TYPE> source) noexcept
		: m_nCount(static_cast<int>(std::size(source)))
		, m_aElements(source)
	{
	}

	//属性
	int size() const noexcept { return m_nCount; }

	auto begin() noexcept { return m_aElements.begin(); }
	auto end() noexcept { return m_aElements.begin() + m_nCount; }
	auto begin() const noexcept { return m_aElements.begin(); }
	auto end() const noexcept { return m_aElements.begin() + m_nCount; }

	//要素アクセス
	ElementType& operator[](size_t nIndex) noexcept
	{
		assert(nIndex<MAX_SIZE);
		assert_warning(nIndex<m_nCount);
		return m_aElements[nIndex];
	}
	const ElementType& operator[](size_t nIndex) const noexcept
	{
		assert(nIndex<MAX_SIZE);
		assert_warning(nIndex<m_nCount);
		return m_aElements[nIndex];
	}

	//操作
	void clear() noexcept { m_nCount=0; }
	template<typename ... Args>
	void emplace_back(Args ...args)
	{
		if (MAX_SIZE <= m_nCount) {
			throw std::out_of_range("m_nCount is out of range.");
		}
		m_aElements[m_nCount++] = ElementType(args...);
	}
	void push_back(SET_TYPE e)
	{
		assert(m_nCount<MAX_SIZE);
		if (MAX_SIZE <= m_nCount) {
			throw std::out_of_range("m_nCount is out of range.");
		}
		m_aElements[m_nCount++] = e;
	}
	void resize(size_t nNewSize)
	{
		if (size_t(MAX_SIZE) <= nNewSize) {
			throw std::out_of_range("nNewSize is out of range.");
		}
		m_nCount = static_cast<int>(nNewSize);
	}
	
	//! 要素数が0でも要素へのポインタを取得
	ElementType* dataPtr() noexcept { return &m_aElements.front();}

	//特殊
	int& _GetSizeRef(){ return m_nCount; }
	void SetSizeLimit(){
		if( MAX_SIZE < m_nCount ){
			m_nCount = MAX_SIZE;
		}else if( m_nCount < 0 ){
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
		return pos ? static_cast<size_t>(pos - data()) : size();
	}

	constexpr bool empty() const noexcept { return 0 == m_szData[0]; }

	constexpr       WCHAR* data()        noexcept { return std::data(m_szData); }
	constexpr const WCHAR* data()  const noexcept { return std::data(m_szData); }
	constexpr const WCHAR* c_str() const noexcept { return data(); }

	constexpr operator std::span<WCHAR, N>()       & noexcept { return std::span<WCHAR, N>{ data(), N }; }
	constexpr operator std::span<WCHAR>()          & noexcept { return std::span<WCHAR, N>{ *this }; }
	constexpr operator std::wstring_view()   const & noexcept { return std::wstring_view{ data(), length() }; }

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
