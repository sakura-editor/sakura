#pragma once

#include "charcode.h"

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

