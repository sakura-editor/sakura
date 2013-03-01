/*!	@file
	@brief OLE Type wrapper

	@author �S
	@date 2003.0221
*/
/*
	Copyright (C) 2003, �S, Moca
	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/
#ifndef __OLE_TYPES_WRAP__
#define __OLE_TYPES_WRAP__

#include "Windows.h"
#include "OleAuto.h"

/*! BSTR��Wrapper class

	�f�[�^�\����BSTR�ƌ݊�������
*/
struct SysString
{
	BSTR Data;

	SysString()                         { Data = NULL; }
	SysString(SysString &Source)        { Data = ::SysAllocStringLen(Source.Data, SysStringLen(Source.Data)); }
	SysString(BSTR &Source)             { Data = ::SysAllocStringLen(Source, SysStringLen(Source)); }
	SysString(wchar_t const *S, int L)  { Data = ::SysAllocStringLen(S, L); }
	SysString(char const *S, int L)
	{ 
		wchar_t *buf = new wchar_t[L + 1];
		int L2 = ::MultiByteToWideChar(CP_ACP, 0, S, L, buf, L);
		Data = ::SysAllocStringLen(buf, L2); 
		delete[] buf;
	}
	~SysString()                        { ::SysFreeString(Data); }
	SysString& operator = (SysString& Source) { Data = ::SysAllocStringLen(Source.Data, SysStringLen(Source.Data)); return *this; }
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
		wcscpy(*S, Data);
	}
#ifdef _UNICODE
	void GetT(TCHAR **S, int *L){GetW(S, L);}
#else
	void GetT(TCHAR **S, int *L){Get(S, L);}
#endif
};

/*! VARIANT��Wrapper class

	�f�[�^�\����VARIANT�ƌ݊�������
*/
struct Variant
{
	VARIANT Data;
	Variant()                       { ::VariantInit(&Data); }
	Variant(Variant &Source)        { ::VariantCopyInd(&Data, &Source.Data); }
	Variant(VARIANT &Source)        { ::VariantCopyInd(&Data, &Source); }
	~Variant()                      { ::VariantClear(&Data); }
	Variant& operator = (Variant& Source) { ::VariantCopyInd(&Data, &Source.Data); return *this; }
	/*! SysString��Variant�ɃZ�b�g����
	
		�Z�b�g��ASysString�̕��͒��g��NULL�ɂȂ�B
	*/
	void Receive(SysString &Source)
	{
		::VariantClear(&Data); 
		Data.vt = VT_BSTR; 
		Data.bstrVal = Source.Data; 
		Source.Data = NULL; 
	}

	// 2003.06.25 Moca
	// int��߂�l�Ƃ��ĕԂ��ꍇ�ɑΉ�
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
