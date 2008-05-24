#pragma once

#include "debug/Debug.h"

//! �q�[�v��p���Ȃ�vector
//2007.09.23 kobake �쐬�B
template <class ELEMENT_TYPE, int MAX_SIZE, class SET_TYPE = const ELEMENT_TYPE&>
class StaticVector{
public:
	//�^
	typedef ELEMENT_TYPE ElementType;

public:
	//����
	int size() const{ return m_nCount; }

	//�v�f�A�N�Z�X
	ElementType&       operator[](int nIndex)      { assert(nIndex<MAX_SIZE); assert_warning(nIndex<m_nCount); return m_aElements[nIndex]; }
	const ElementType& operator[](int nIndex) const{ assert(nIndex<MAX_SIZE); assert_warning(nIndex<m_nCount); return m_aElements[nIndex]; }

	//����
	void clear(){ m_nCount=0; }
	void push_back(SET_TYPE e)
	{
		assert(m_nCount<MAX_SIZE);
		m_nCount++;
		m_aElements[m_nCount-1]=e;
	}
	void resize(int nNewSize)
	{
		assert(nNewSize < MAX_SIZE);
		m_nCount = nNewSize;
	}

	//����
	int& _GetSizeRef(){ return m_nCount; }

private:
	int         m_nCount;
	ElementType m_aElements[MAX_SIZE];
};

//! �q�[�v��p���Ȃ�������N���X
//2007.09.23 kobake �쐬�B
template <class CHAR_TYPE, int N_BUFFER_COUNT>
class StaticString{
private:
	typedef StaticString<CHAR_TYPE,N_BUFFER_COUNT> Me;
public:
	static const int BUFFER_COUNT = N_BUFFER_COUNT;
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	StaticString(){ m_szData[0]=0; }
	StaticString(const CHAR_TYPE* rhs){ if(!rhs) m_szData[0]=0; else auto_strcpy(m_szData,rhs); }

	//�N���X����
	size_t GetBufferCount() const{ return N_BUFFER_COUNT; }

	//�f�[�^�A�N�Z�X
	CHAR_TYPE*       GetBufferPointer()      { return m_szData; }
	const CHAR_TYPE* GetBufferPointer() const{ return m_szData; }
	const CHAR_TYPE* c_str()            const{ return m_szData; } //std::string��

	//�ȈՃf�[�^�A�N�Z�X
	operator       CHAR_TYPE*()      { return m_szData; }
	operator const CHAR_TYPE*() const{ return m_szData; }
	CHAR_TYPE At(int nIndex) const{ return m_szData[nIndex]; }

	//�ȈՃR�s�[
	void Assign(const CHAR_TYPE* src){ if(!src) m_szData[0]=0; else auto_strcpy_s(m_szData,_countof(m_szData),src); }
	Me& operator = (const CHAR_TYPE* src){ Assign(src); return *this; }

	//�e�탁�\�b�h
	int Length() const{ return auto_strlen(m_szData); }

private:
	CHAR_TYPE m_szData[N_BUFFER_COUNT];
};

#define _countof2(s) s.BUFFER_COUNT
