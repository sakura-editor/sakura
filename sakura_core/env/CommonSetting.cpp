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

#include "StdAfx.h"
#include "CommonSetting.h"
#include <vector>
using namespace std;

//CommonValue管理
struct CommonValueInfo{
	enum EType{
		TYPE_UNKNOWN,
		TYPE_ASTR,    //char文字列 (終端NULL)
		TYPE_WSTR,    //wchar_t文字列 (終端NULL)
	};

	void* m_pValue;     //!< 値へのポインタ
	int   m_nValueSize; //!< 値のサイズ。バイト単位。
	char  m_szEntryKey[32];
	EType m_eType;

	CommonValueInfo(void* pValue, int nValueSize, const char* szEntryKey, EType eType=TYPE_UNKNOWN)
	: m_pValue(pValue), m_nValueSize(nValueSize), m_eType(eType)
	{
		strcpy_s(m_szEntryKey,_countof(m_szEntryKey),szEntryKey);
	}

	void Save()
	{
		printf("%hs=",m_szEntryKey);

		//intと同じサイズならintとして出力
		if(m_nValueSize==sizeof(int)){
			printf("%d\n",*((int*)m_pValue));
		}
		//それ以外ならバイナリ出力
		else{
			for(int i=0;i<m_nValueSize;i++){
				printf("%%%02X",((BYTE*)m_pValue)[i]);
			}
		}
	}
};
vector<CommonValueInfo> g_commonvalues;
void CommonValue_AllSave()
{
	int nSize = (int)g_commonvalues.size();
	for(int i=0;i<nSize;i++){
		g_commonvalues[i].Save();
	}
}

//CommonValue ※virtual使うの禁止
template <class T>
class CommonValue{
private:
	typedef CommonValue<T> Me;
public:
	CommonValue()
	{
	}
	void Regist(const char* szEntryKey)
	{
		//CommonValueリストに自分を追加
		g_commonvalues.push_back(CommonValueInfo(&m_value,sizeof(m_value),szEntryKey));
	}
	Me& operator = (const T& rhs){ m_value=rhs; return *this; }
	operator T& () { return m_value; }
	operator const T& () const{ return m_value; }
private:
	T m_value;
};


typedef char mystring[10];

void sample()
{
	CommonValue<int>      intvalue;
	CommonValue<mystring> strvalue;

	intvalue.Regist("intvalue");
	strvalue.Regist("strvalue");

	intvalue=3;
	strcpy(strvalue,"hage");

	CommonValue_AllSave();
}
