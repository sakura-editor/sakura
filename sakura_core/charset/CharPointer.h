/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CHARPOINTER_6EAF62A9_67F6_4CEF_80C2_B7C34014C253_H_
#define SAKURA_CHARPOINTER_6EAF62A9_67F6_4CEF_80C2_B7C34014C253_H_

#include "charset/charcode.h"

//!< ディレクトリを除いた、ファイル名だけを取得する
class CharPointerA{
public:
	CharPointerA(){ }
	CharPointerA(const char* p) : m_p(p) { }
	CharPointerA(const CharPointerA& rhs) : m_p(rhs.m_p) { }

	//進む
	const char* operator ++ ()   { _forward(); return this->m_p; }                  //!< ++p;
	const char* operator ++ (int){ CharPointerA tmp; _forward(); return tmp.m_p;  } //!< p++;
	const char* operator += (size_t n){ while(n-->0)_forward(); return this->m_p; } //!< p+=n;
	
	//進んだ値
	const char* operator + (size_t n) const{ CharPointerA tmp=*this; return tmp+=n; }
	WORD operator[](size_t n) const{ CharPointerA tmp=*this; tmp+=n; return *tmp; }

	//代入
	const char* operator = (const char* p){ m_p=p; return this->m_p; }

	//文字取得
	WORD operator * () const{ return _get(); }

	//ポインタ取得
//	operator const char*() const{ return m_p; } //※operator + と競合するので、このキャスト演算子は提供しない
	const char* GetPointer() const{ return m_p; }

protected:
	void _forward() //!< 1文字進む
	{
		if(_IS_SJIS_1(m_p[0]) && _IS_SJIS_2(m_p[1]))m_p+=2;
		else m_p+=1;
	}
	WORD _get() const //!< 1文字取得する
	{
		if(_IS_SJIS_1(m_p[0]) && _IS_SJIS_2(m_p[1]))return *((WORD*)m_p);
		else return *m_p;
	}

private:
	const char* m_p;
};


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



#ifdef _UNICODE
typedef CharPointerW CharPointerT;
#else
typedef CharPointerA CharPointerT;
#endif

#endif /* SAKURA_CHARPOINTER_6EAF62A9_67F6_4CEF_80C2_B7C34014C253_H_ */
/*[EOF]*/
