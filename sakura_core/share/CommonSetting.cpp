#include "stdafx.h"
#include "CommonSetting.h"
#include <vector>
using namespace std;

//CommonValue�Ǘ�
struct CommonValueInfo{
	enum EType{
		TYPE_UNKNOWN,
		TYPE_ASTR,    //char������ (�I�[NULL)
		TYPE_WSTR,    //wchar_t������ (�I�[NULL)
	};

	void* m_pValue;     //!< �l�ւ̃|�C���^
	int   m_nValueSize; //!< �l�̃T�C�Y�B�o�C�g�P�ʁB
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

		//int�Ɠ����T�C�Y�Ȃ�int�Ƃ��ďo��
		if(m_nValueSize==sizeof(int)){
			printf("%d\n",*((int*)m_pValue));
		}
		//����ȊO�Ȃ�o�C�i���o��
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
	for(int i=0;i<(int)g_commonvalues.size();i++){
		g_commonvalues[i].Save();
	}
}

//CommonValue ��virtual�g���̋֎~
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
		//CommonValue���X�g�Ɏ�����ǉ�
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
