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
	using Me = StaticString<N_BUFFER_COUNT>;
public:
	static constexpr auto BUFFER_COUNT = N_BUFFER_COUNT;

	static constexpr auto size() noexcept { return BUFFER_COUNT; }

public:
	//コンストラクタ・デストラクタ
	StaticString(){ m_szData[0]=0; }
	StaticString(const WCHAR* src) { Assign(src); }

	//クラス属性
	size_t GetBufferCount() const{ return N_BUFFER_COUNT; }

	//データアクセス
	WCHAR*       GetBufferPointer()      { return m_szData; }
	const WCHAR* GetBufferPointer() const{ return m_szData; }
	const WCHAR* c_str()            const{ return m_szData; } //std::string風

	//簡易データアクセス
	operator       WCHAR*()      { return m_szData; }
	operator const WCHAR*() const{ return m_szData; }
	WCHAR At(int nIndex) const{ return m_szData[nIndex]; }

	//簡易コピー
	void Assign(const WCHAR* src){ if(!src) m_szData[0]=0; else wcscpy_s(m_szData, std::size(m_szData),src); }
	Me& operator = (const WCHAR* src){ Assign(src); return *this; }

	//各種メソッド
	int Length() const { return static_cast<int>(auto_strnlen(m_szData, BUFFER_COUNT)); }

private:
	WCHAR m_szData[N_BUFFER_COUNT];
};

#endif /* SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_ */
