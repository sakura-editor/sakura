/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_
#define SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_
#pragma once

#include <array>
#include <cassert>
#include <stdexcept>

#include "util/string_ex.h"

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
		return m_aElements[nIndex];
	}
	const ElementType& operator[](int nIndex) const
	{
		assert(nIndex<MAX_SIZE);
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

//! ヒープを用いない文字列クラス
//2007.09.23 kobake 作成。
template <class CHAR_TYPE, int N_BUFFER_COUNT>
class StaticString{
private:
	typedef StaticString<CHAR_TYPE,N_BUFFER_COUNT> Me;
public:
	static const int BUFFER_COUNT = N_BUFFER_COUNT;
public:
	//コンストラクタ・デストラクタ
	StaticString() noexcept = default;
	StaticString(const CHAR_TYPE* rhs) { Assign(rhs); }

	//クラス属性
	size_t GetBufferCount() const{ return N_BUFFER_COUNT; }

	//データアクセス
	CHAR_TYPE*       GetBufferPointer()       { return m_szData.data(); }
	const CHAR_TYPE* GetBufferPointer() const { return m_szData.data(); }
	CHAR_TYPE*       data()                   { return GetBufferPointer(); } //std::string風
	const CHAR_TYPE* c_str()            const { return GetBufferPointer(); } //std::string風

	//簡易データアクセス
	operator       CHAR_TYPE*()       { return GetBufferPointer(); }
	operator const CHAR_TYPE*() const { return GetBufferPointer(); }

	CHAR_TYPE At(int nIndex) const{ return m_szData[nIndex]; }

	//簡易コピー
	void Assign(const CHAR_TYPE* src) {
		if (!src) m_szData[0] = 0;
		else auto_strcpy_s(m_szData.data(), m_szData.size(), src);
	}
	void Append(std::basic_string_view<CHAR_TYPE> src) {
		auto_strcat_s(m_szData.data(), m_szData.size(), src.data());
	}

	Me& operator = (const CHAR_TYPE* src){ Assign(src); return *this; }
	Me& operator += (const CHAR_TYPE* src) { Append(src); return *this; }

	//各種メソッド
	int Length() const { return static_cast<int>(auto_strnlen(m_szData.data(), m_szData.size())); }

private:
	std::array<CHAR_TYPE, N_BUFFER_COUNT> m_szData = {};
};

#define _countof2(s) s.BUFFER_COUNT
#endif /* SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_ */
