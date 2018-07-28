/*!	@file
	@brief OLE Type wrapper

	@author 鬼
	@date 2003.0221
*/
/*
	Copyright (C) 2003, 鬼, Moca
	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/
#ifndef __OLE_TYPES_WRAP__
#define __OLE_TYPES_WRAP__

#include <Windows.h>
#include <OleAuto.h>

/*! BSTRのWrapper class

	データ構造はBSTRと互換性あり
*/
struct SysString
{
	BSTR Data;

	SysString()                         { Data = NULL; }
	SysString(const SysString &Source)  { Data = ::SysAllocStringLen(Source.Data, SysStringLen(Source.Data)); }
	SysString(BSTR &Source)             { Data = ::SysAllocStringLen(Source, SysStringLen(Source)); }
	SysString(const wchar_t *S, int L)  { Data = ::SysAllocStringLen(S, L); }
	SysString(const char *S, int L)
	{ 
		wchar_t *buf = new wchar_t[L + 1];
		int L2 = ::MultiByteToWideChar(CP_ACP, 0, S, L, buf, L);
		Data = ::SysAllocStringLen(buf, L2); 
		delete[] buf;
	}
	~SysString()                        { ::SysFreeString(Data); }
	SysString& operator = (const SysString& Source) { Data = ::SysAllocStringLen(Source.Data, SysStringLen(Source.Data)); return *this; }
	int Length()                        { return ::SysStringLen(Data); }
	void Get(char **S, int *L)
	{
		int Len = ::SysStringLen(Data);
		*S = new char[Len * 2 + 1];
		*L = ::WideCharToMultiByte(CP_ACP, 0, Data, Len, *S, Len * 2, NULL, NULL);
		(*S)[*L] = 0;
	}
	void GetW(wchar_t **S, int *L)
	{
		int Len = ::SysStringLen(Data);
		*S = new wchar_t[Len + 1];
		*L = Len;
		wmemcpy(*S, Data, Len);
		(*S)[Len] = L'\0';
	}
	void Get(std::string* str)
	{
		char* S;
		int Len;
		Get(&S, &Len);
		str->assign(S, Len);
		delete [] S;
	}
	void GetW(std::wstring* str)
	{
		int Len = ::SysStringLen(Data);
		str->assign(Data, Len);
	}
#ifdef _UNICODE
	void GetT(TCHAR **S, int *L){GetW(S, L);}
	void GetT(std::wstring* str){GetW(str);}
#else
	void GetT(TCHAR **S, int *L){Get(S, L);}
	void GetT(std::string* str){Get(str);}
#endif
};

/*! VARIANTのWrapper class

	データ構造はVARIANTと互換性あり
*/
struct Variant
{
	VARIANT Data;
	Variant()                       { ::VariantInit(&Data); }
	Variant(Variant &Source)        { ::VariantCopyInd(&Data, &Source.Data); }
	Variant(VARIANT &Source)        { ::VariantCopyInd(&Data, &Source); }
	~Variant()                      { ::VariantClear(&Data); }
	Variant& operator = (Variant& Source) { ::VariantCopyInd(&Data, &Source.Data); return *this; }
	/*! SysStringをVariantにセットする
	
		セット後、SysStringの方は中身がNULLになる。
	*/
	void Receive(SysString &Source)
	{
		::VariantClear(&Data); 
		Data.vt = VT_BSTR; 
		Data.bstrVal = Source.Data; 
		Source.Data = NULL; 
	}

	// 2003.06.25 Moca
	// intを戻り値として返す場合に対応
	int Receive(int i)
	{
		::VariantClear(&Data); 
		Data.vt = VT_I4;
		return Data.lVal = i;
	}
};
/*
#if sizeof(SysString) != 4
#error "error"
#endif

#if sizeof(Variant) != 16
#error "error"
#endif
*/
inline Variant* Wrap(VARIANT *Value)
{
	return reinterpret_cast<Variant*>(Value);
}

inline SysString* Wrap(BSTR *Value)
{
	return reinterpret_cast<SysString*>(Value);
}
#endif
