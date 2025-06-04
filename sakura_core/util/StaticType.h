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

namespace basis {

template<size_t N, typename C>
class SString;

/*!
 * バッファ参照型
 *
 * 読み書き可能な文字バッファとサイズを指定して構築する
 *
 * ms-gslのgsl::span や stdcpp20のstd::span のようなクラス。、
 * 共有メモリ入出力 CShareData_IO で使われていたStringBufferWを拡張したもの。
 */
template <typename C = WCHAR>
class TCharBuffer {
private:
	using Me = TCharBuffer<C>;

	C*		m_Data;		//!< 文字列バッファ。バッファは常にNUL終端されているとは限らない。
	size_t	m_Size;		//!< バッファサイズ(NUL終端の分を含む)。

public:
	using char_type = C;
	using string_type = std::basic_string<char_type>;
	using string_view_type = std::basic_string_view<char_type>;

	/*!
	 * コンストラクタ
	 *
	 * 読み書き可能な文字バッファとサイズを指定して構築する
	 */
	TCharBuffer(
		_In_reads_(count)
		char_type*  buffer,         //!< [in] 文字列バッファ(NUL終端不要)。
		_In_range_(1, UINT_MAX)
		size_t      count           //!< [in] バッファサイズ(NUL終端の分を含む)。
	)
		: m_Data(buffer)
		, m_Size(count)
	{
		if (m_Data == nullptr) {
			throw std::invalid_argument("data can't be null");
		}

		if (m_Size == 0) {
			throw std::invalid_argument("count can't be zero");
		}
	}

	/*!
	 * 暗黙変換用コンストラクタ
	 */
	template <size_t N>
	/* implicit */ TCharBuffer(SString<N, char_type>& buffer)
		: TCharBuffer(buffer, std::size(buffer)) {}

	/*!
	 * 配列ラップ用コンストラクタ
	 */
	template <size_t N>
	/* implicit */ TCharBuffer(char_type (&buffer)[N])
		: TCharBuffer(buffer, std::size(buffer)) {}

	/*!
	 * 末尾に文字列を追加する
	 */
	errno_t append(
		_In_reads_(count)
		const char_type* text,      //!< [in] 文字列(NUL終端不要)
		_In_range_(1, UINT_MAX - 1)
		size_t           count      //!< [in] 文字列長(NUL終端を含まない)
	)
	{
		if (count < 1) return EINVAL;
		const auto currentLength = length();
		return Me(data() + currentLength, size() - currentLength).assign(text, count);
	}

	errno_t append(string_view_type rhs) {
		return append(rhs.data(), rhs.length());
	}

	errno_t append(_In_z_ const char_type* rhs) {
		if (!rhs) return EINVAL;
		return append(string_view_type(rhs));
	}

	errno_t append(const string_type& rhs) {
		return append(rhs.data(), rhs.length());
	}

	/*!
	 * 文字列を割り当てる
	 */
	errno_t assign(
		_In_reads_opt_(count)
		const char_type* text,      //!< [in] 文字列(NUL終端不要、途中にNUL文字があってもOK)
		_In_range_(0, UINT_MAX - 1)
		size_t           count      //!< [in] 文字列長(NUL終端を含まない)
	)
	{
		// 現在の文字数を算出する
		const auto currentLength = length();

		// 設定する文字数を算出する
		const auto setLength = std::min(count, size() - 1);
	
		// コピーできる文字が存在する場合のみ、コピーを行う
		if (text && setLength && data() != text) {
			auto_strncpy_s(data(), size(), text, setLength);
		}
		// NUL終端する
		else if (at(setLength)) {
			auto_memset(data() + setLength, '\0', std::max<ptrdiff_t>(1, currentLength - setLength));
		}
	
		// 戻り値を返却
		return setLength < count ? STRUNCATE : 0;
	}

	errno_t assign(string_view_type rhs) {
		return assign(rhs.data(), rhs.length());
	}

	errno_t assign(_In_opt_z_ const char_type* rhs) {
		if (!rhs) {
			return assign(nullptr, 0);
		} else {
			return assign(string_view_type(rhs));
		}
	}

	errno_t assign(const string_type& rhs) {
		return assign(rhs.data(), rhs.length());
	}

	auto& at(_In_range_(0, INT_MAX) size_t index) const {
		assert(index <= std::numeric_limits<int>::max());
		if (size() <= index) {
			throw std::out_of_range("index is out of range.");
		}
		return operator[](int(index));
	}

	const char_type* c_str() const noexcept { return data(); }
	char_type*       data()        noexcept { return m_Data; }
	const char_type* data()  const noexcept { return m_Data; }

	bool    empty()  const noexcept { return !length(); }

	size_t length() const noexcept {
		const auto len = auto_strnlen(data(), size());
		return len < size() ? len : 0;
	}
	
	size_t size() const noexcept { return m_Size; }

	Me& operator = (const Me& rhs) = default;
	Me& operator = (string_view_type rhs) { assign(rhs); return *this; }
	Me& operator = (_In_opt_z_ const char_type* rhs) { assign(rhs); return *this; }
	Me& operator = (const string_type& rhs) { assign(rhs); return *this; }
	Me& operator = (const char_type ch) { assign(&ch, 1); return *this; }

	template <size_t N>
	Me& operator = (const SString<N, char_type>& rhs ) { assign(rhs, rhs.length()); return *this; }

	Me& operator += (string_view_type rhs) { append(rhs); return *this; }
	Me& operator += (_In_z_ const char_type* rhs) { append(rhs); return *this; }
	Me& operator += (const string_type& rhs) { append(rhs); return *this; }
	Me& operator += (const char_type ch) { append(&ch, 1); return *this; }

	constexpr       char_type& operator [] (int index)       { return 0 <= index && size_t(index) < size() ? data()[index] : data()[size() - 1]; }
	constexpr const char_type& operator [] (int index) const { return 0 <= index && size_t(index) < size() ? data()[index] : data()[size() - 1]; }

	explicit operator string_view_type() const noexcept { return string_view_type(data(), length()); }
	explicit operator std::filesystem::path() const noexcept { return std::filesystem::path(string_view_type(*this)); }

	/* implicit */ operator char_type*()             noexcept { return data(); }
	/* implicit */ operator const char_type*() const noexcept { return c_str(); }
};

/*!
 * 文字配列拡張型（似非string）クラステンプレート
 *
 * ヒープを用いない文字列クラス StaticString を拡張したもの。
 */
template <size_t N, typename C = WCHAR>
class SString {
private:
	using Me = SString<N, C>;
	std::array<C, N> m_szData{};

public:
	using char_type = C;
	using string_type = std::basic_string<char_type>;
	using string_view_type = std::basic_string_view<char_type>;
	using buffer_type = TCharBuffer<char_type>;

	static constexpr size_t size() noexcept { return N; }

	SString() = default;

	SString(const Me& rhs) = default;

	explicit SString(string_view_type rhs) noexcept { assign(rhs); }

	/* implicit */ SString(_In_opt_z_ const char_type* rhs) noexcept { assign(rhs); }
	/* implicit */ SString(const string_type& rhs) noexcept { assign(rhs); }
	/* implicit */ SString(const std::filesystem::path& rhs) noexcept { assign(rhs); }

	errno_t append(
		_In_reads_(count)
		const char_type* text,
		_In_range_(1, N - 1)
		size_t           count
	)
	{
		if (count < 1) return EINVAL;
		const auto currentLength = length();
		return buffer_type(data() + currentLength, size() - currentLength).assign(text, count);
	}

	errno_t append(string_view_type rhs) {
		return append(rhs.data(), rhs.length());
	}

	errno_t append(_In_z_ const char_type* rhs) {
		if (!rhs) return EINVAL;
		return append(string_view_type(rhs));
	}

	errno_t append(const string_type& rhs) {
		return append(rhs.data(), rhs.length());
	}

	errno_t assign(
		_In_reads_opt_(count)
		const char_type* text,
		_In_range_(0, N - 1)
		size_t           count
	)
	{
		return buffer_type(data(), size()).assign(text, count);
	}

	errno_t assign(string_view_type rhs) {
		return assign(rhs.data(), rhs.length());
	}

	errno_t assign(_In_opt_z_ const char_type* rhs) {
		if (!rhs) {
			return assign(nullptr, 0);
		} else {
			return assign(string_view_type(rhs));
		}
	}

	errno_t assign(const string_type& rhs) {
		return assign(rhs.data(), rhs.length());
	}

	errno_t assign(const std::filesystem::path& rhs) {
		if constexpr (std::is_same_v<char_type, wchar_t>) {
			return assign(rhs.wstring());
		} else {
			return assign(rhs.string());
		}
	}

	auto& at(_In_range_(0, N - 1) size_t index) const {
		if (size() <= index) {
			throw std::out_of_range("index is out of range.");
		}
		return operator[](index);
	}

	const char_type* c_str() const noexcept { return data(); }
	char_type*       data()        noexcept { return m_szData.data(); }
	const char_type* data()  const noexcept { return m_szData.data(); }

	bool    empty()  const noexcept { return !length(); }

	size_t length() const noexcept {
		const auto len = auto_strnlen(data(), size());
		return len < size() ? len : 0;
	}

	/*!
	 * 文字列の長さを取得する
	 *
	 * 旧コードとの互換性のため戻り値をintにキャストする
	 */
	int Length() const { return int(length()); }

	Me& operator = (const Me& rhs) = default;

	Me& operator = (string_view_type rhs) { assign(rhs); return *this; }
	Me& operator = (_In_opt_z_ const char_type* rhs) { assign(rhs); return *this; }
	Me& operator = (const string_type& rhs) { assign(rhs); return *this; }
	Me& operator = (const char_type ch) { assign(&ch, 1); return *this; }
	Me& operator = (const std::filesystem::path& rhs) { assign(rhs); return *this; }

	Me& operator += (string_view_type rhs) { append(rhs); return *this; }
	Me& operator += (_In_z_ const char_type* rhs) { append(rhs); return *this; }
	Me& operator += (const string_type& rhs) { append(rhs); return *this; }
	Me& operator += (const char_type ch) { append(&ch, 1); return *this; }

	constexpr       char_type& operator [] (int index)       { return 0 <= index && size_t(index) < size() ? m_szData[index] : m_szData.back(); }
	constexpr const char_type& operator [] (int index) const { return 0 <= index && size_t(index) < size() ? m_szData[index] : m_szData.back(); }

	explicit operator buffer_type() noexcept { return buffer_type(data(), size()); }
	explicit operator string_view_type() const noexcept { return string_view_type(data(), length()); }
	explicit operator std::filesystem::path() const noexcept { return std::filesystem::path(string_view_type(*this)); }

	/* implicit */ operator char_type*()             noexcept { return data(); }
	/* implicit */ operator const char_type*() const noexcept { return c_str(); }
};

} // namespace basis

template <size_t N, typename C = WCHAR>
using SString = basis::SString<N, C>;

//! ヒープを用いないvector
//2007.09.23 kobake 作成。
template <class ELEMENT_TYPE, int MAX_SIZE, class SET_TYPE = const ELEMENT_TYPE&>
class StaticVector{
public:
	//型
	typedef ELEMENT_TYPE ElementType;

public:
	//属性
	int size() const{ return m_nCount; }
	int max_size() const{ return MAX_SIZE; }

	//要素アクセス
	ElementType&       operator[](int nIndex)
	{
		assert(nIndex<MAX_SIZE);
		assert_warning(nIndex<m_nCount);
		return m_aElements[nIndex];
	}
	const ElementType& operator[](int nIndex) const
	{
		assert(nIndex<MAX_SIZE);
		assert_warning(nIndex<m_nCount);
		return m_aElements[nIndex];
	}

	//操作
	void clear(){ m_nCount=0; }
	void push_back(SET_TYPE e)
	{
		assert(m_nCount<MAX_SIZE);
		if (MAX_SIZE <= m_nCount) {
			throw std::out_of_range("m_nCount is out of range.");
		}
		m_aElements[m_nCount++] = e;
	}
	void resize(int nNewSize)
	{
		assert(0 <= nNewSize);
		assert(nNewSize <= MAX_SIZE);
		m_nCount = nNewSize;
	}
	
	//! 要素数が0でも要素へのポインタを取得
	ElementType*  dataPtr(){ return m_aElements;}

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
	int         m_nCount;
	ElementType m_aElements[MAX_SIZE];
};

/*!
 * ヒープを用いない文字列クラス
 *
 * @date 2007/09/23 kobake 作成。
 */
template <int N_BUFFER_COUNT, typename C = WCHAR>
class StaticString : public SString<N_BUFFER_COUNT, C> {
private:
	using Me = StaticString<N_BUFFER_COUNT, C>;
	using Base = SString<N_BUFFER_COUNT, C>;

public:
	using char_type = typename Base::char_type;

	static constexpr int BUFFER_COUNT = N_BUFFER_COUNT;

	/*!
	 * バッファの個数を取得
	 *
	 * 旧コードとの互換性のため 長い名前のバージョンを残しておく
	 */
	static size_t GetBufferCount() noexcept { return BUFFER_COUNT; }

	/*
	 * コンストラクタは基底クラスのものを流用する。
	 */
	using Base::Base;

	/*
	 * 基底クラスの代入演算子を流用する。
	 */
	using Base::operator =;

	/*!
	 * データアクセス
	 *
	 * 旧コードとの互換性のため 長い名前のバージョンを残しておく
	 */
	char_type*       GetBufferPointer()      { return Base::data(); }

	/*!
	 * データアクセス
	 *
	 * 旧コードとの互換性のため 長い名前のバージョンを残しておく
	 */
	const char_type* GetBufferPointer() const{ return Base::data(); }

	/*!
	 * 指定位置の文字を取得
	 *
	 * 旧コードとの互換性のため stdcpp と振る舞いの異なるバージョンを残しておく
	 */
	char_type At(int nIndex) const { return Base::operator[] (nIndex); }

	/*!
	 * 簡易コピー
	 *
	 * 旧コードとの互換性のため std::wstring と異なる名前のバージョンを残しておく
	 */
	void Assign(_In_opt_z_ const char_type* src) { Base::assign(src); }
};

#define _countof2(s) s.BUFFER_COUNT

#endif /* SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_ */
