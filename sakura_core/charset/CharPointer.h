/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_CHARPOINTER_B649A129_0BBB_4ABE_BE02_0389CF537319_H_
#define SAKURA_CHARPOINTER_B649A129_0BBB_4ABE_BE02_0389CF537319_H_
#pragma once

#include "charset/charcode.h"
#include "charset/codechecker.h"

class CharPointerW{
public:
	CharPointerW(){ }
	CharPointerW(const wchar_t* p) : m_p(p) { }
	CharPointerW(const CharPointerW& rhs) : m_p(rhs.m_p) { }

	//進む
	const wchar_t* operator ++ ()   { _forward(); return this->m_p; }                   //!< ++p;
	const wchar_t* operator ++ (int){ CharPointerW tmp; _forward(); return tmp.m_p;   } //!< p++;
	const wchar_t* operator += (size_t n){ while(n-->0)_forward(); return this->m_p;  } //!< p+=n;
	
	//進んだ値
	const wchar_t* operator + (size_t n) const{ CharPointerW tmp=*this; return tmp+=n; }
	WORD operator[](size_t n) const{ CharPointerW tmp=*this; tmp+=n; return *tmp; }

	//代入
	const wchar_t* operator = (const wchar_t* p){ m_p=p; return this->m_p; }

	//文字取得
	WORD operator * () const{ return _get(); }

	//ポインタ取得
//	operator const wchar_t*() const{ return m_p; } //※operator + と競合するので、このキャスト演算子は提供しない
	const wchar_t* GetPointer() const{ return m_p; }

protected:
	void _forward() //!< 1文字進む
	{
		++m_p;
	}
	WORD _get() const //!< 1文字取得する
	{
		return *m_p;
	}

private:
	const wchar_t* m_p;
};

#endif /* SAKURA_CHARPOINTER_B649A129_0BBB_4ABE_BE02_0389CF537319_H_ */
