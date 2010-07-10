#pragma once

#include "debug/Debug.h"

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

	//要素アクセス
	ElementType&       operator[](int nIndex)      { assert(nIndex<MAX_SIZE); assert_warning(nIndex<m_nCount); return m_aElements[nIndex]; }
	const ElementType& operator[](int nIndex) const{ assert(nIndex<MAX_SIZE); assert_warning(nIndex<m_nCount); return m_aElements[nIndex]; }

	//操作
	void clear(){ m_nCount=0; }
	void push_back(SET_TYPE e)
	{
		assert(m_nCount<MAX_SIZE);
		m_nCount++;
		m_aElements[m_nCount-1]=e;
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
		if( m_nCount < MAX_SIZE ){
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
	StaticString(){ m_szData[0]=0; }
	StaticString(const CHAR_TYPE* rhs){ if(!rhs) m_szData[0]=0; else auto_strcpy(m_szData,rhs); }

	//クラス属性
	size_t GetBufferCount() const{ return N_BUFFER_COUNT; }

	//データアクセス
	CHAR_TYPE*       GetBufferPointer()      { return m_szData; }
	const CHAR_TYPE* GetBufferPointer() const{ return m_szData; }
	const CHAR_TYPE* c_str()            const{ return m_szData; } //std::string風

	//簡易データアクセス
	operator       CHAR_TYPE*()      { return m_szData; }
	operator const CHAR_TYPE*() const{ return m_szData; }
	CHAR_TYPE At(int nIndex) const{ return m_szData[nIndex]; }

	//簡易コピー
	void Assign(const CHAR_TYPE* src){ if(!src) m_szData[0]=0; else auto_strcpy_s(m_szData,_countof(m_szData),src); }
	Me& operator = (const CHAR_TYPE* src){ Assign(src); return *this; }

	//各種メソッド
	int Length() const{ return auto_strlen(m_szData); }

private:
	CHAR_TYPE m_szData[N_BUFFER_COUNT];
};

#define _countof2(s) s.BUFFER_COUNT
