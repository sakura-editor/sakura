#pragma once

#include "util/StaticType.h"
#include "CProfile.h"
#include "types/CType.h"

//������o�b�t�@�̌^
struct StringBufferW{
	WCHAR*    pData;
	const int nDataCount;

	StringBufferW() : pData(L""), nDataCount(0) { }
	StringBufferW(WCHAR* _pData, int _nDataCount) : pData(_pData), nDataCount(_nDataCount) { }

	StringBufferW& operator = (const StringBufferW& rhs)
	{
		auto_strcpy_s(pData,nDataCount,rhs.pData);
		return *this;
	}
};
struct StringBufferA{
	ACHAR* pData;
	int    nDataCount;

	StringBufferA() : pData(""), nDataCount(0) { }
	StringBufferA(ACHAR* _pData, int _nDataCount) : pData(_pData), nDataCount(_nDataCount) { }

	StringBufferA& operator = (const StringBufferA& rhs)
	{
		auto_strcpy_s(pData,nDataCount,rhs.pData);
		return *this;
	}
};
#ifdef _UNICODE
	typedef StringBufferW StringBufferT;
#else
	typedef StringBufferA StringBufferT;
#endif

//������o�b�t�@�^�C���X�^���X�̐����}�N��
#define MakeStringBufferW(S) StringBufferW(S,_countof(S))
#define MakeStringBufferA(S) StringBufferA(S,_countof(S))
#define MakeStringBufferT(S) StringBufferT(S,_countof(S))
#define MakeStringBufferW0(S) StringBufferW(S,0)
#define MakeStringBufferT0(S) StringBufferT(S,0)


//2007.09.24 kobake �f�[�^�ϊ������q�N���X�ɕ���
//!�e��f�[�^�ϊ��t��CProfile
class CDataProfile : public CProfile{
private:
	//��p�^
	typedef std::wstring wstring;
	typedef std::tstring tstring;

protected:
	static const wchar_t* _work_itow(int n)
	{
		static wchar_t buf[32];
		_itow(n,buf,10);
		return buf;
	}
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �f�[�^�ϊ���                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	//bool
	void profile_to_value(const wstring& profile, bool* value)
	{
		if(profile != L"0")*value=true;
		else *value=false;
	}
	void value_to_profile(const bool& value, wstring* profile)
	{
		*profile = _work_itow(value?1:0);
	}
	//int
	void profile_to_value(const wstring& profile, int* value)
	{
		*value = _wtoi(profile.c_str());
	}
	void value_to_profile(const int& value, wstring* profile)
	{
		*profile = _work_itow(value);
	}

	//int�����o�͎����}�N��
	#define AS_INT(TYPE) \
		void profile_to_value(const wstring& profile, TYPE* value){ *value = (TYPE)_wtoi(profile.c_str()); } \
		void value_to_profile(const TYPE& value, wstring* profile){ *profile = _work_itow(value);    }

	//int��
	AS_INT(EOutlineType)
	AS_INT(WORD)

#ifdef USE_STRICT_INT
	//CLayoutInt
	void profile_to_value(const wstring& profile, CLayoutInt* value){ *value = (CLayoutInt)_wtoi(profile.c_str()); }
	void value_to_profile(const CLayoutInt& value, wstring* profile){ *profile = _work_itow((Int)value);    }
	//CLogicInt
	void profile_to_value(const wstring& profile, CLogicInt* value){ *value = (CLogicInt)_wtoi(profile.c_str()); }
	void value_to_profile(const CLogicInt& value, wstring* profile){ *profile = _work_itow((Int)value);    }
#endif
	//ACHAR
	void profile_to_value(const wstring& profile, ACHAR* value)
	{
		if(profile.length()>0){
			ACHAR buf[2]={0};
			int ret=wctomb(buf,profile[0]);
			assert_warning(ret==1);
			*value = buf[0];
		}
		else{
			*value = '\0';
		}
	}
	void value_to_profile(const ACHAR& value, wstring* profile)
	{
		WCHAR buf[2]={0};
		mbtowc(buf,&value,1);
		profile->assign(1,buf[0]);
	}
	//WCHAR
	void profile_to_value(const wstring& profile, WCHAR* value)
	{
		*value = profile[0];
	}
	void value_to_profile(const WCHAR& value, wstring* profile)
	{
		profile->assign(1,value);
	}
	//StringBufferW
	void profile_to_value(const wstring& profile, StringBufferW* value)
	{
		wcscpy_s(value->pData,value->nDataCount,profile.c_str());
	}
	void value_to_profile(const StringBufferW& value, wstring* profile)
	{
		*profile = value.pData;
	}
	//StringBufferA
	void profile_to_value(const wstring& profile, StringBufferA* value)
	{
		strcpy_s(value->pData,value->nDataCount,to_achar(profile.c_str()));
	}
	void value_to_profile(const StringBufferA& value, wstring* profile)
	{
		*profile = to_wchar(value.pData);
	}
	//StaticString<WCHAR,N>
	template <int N>
	void profile_to_value(const wstring& profile, StaticString<WCHAR, N>* value)
	{
		wcscpy_s(value->GetBufferPointer(),value->GetBufferCount(),profile.c_str());
	}
	template <int N>
	void value_to_profile(const StaticString<WCHAR, N>& value, wstring* profile)
	{
		*profile = value.GetBufferPointer();
	}
	//StaticString<ACHAR,N>
	template <int N>
	void profile_to_value(const wstring& profile, StaticString<ACHAR, N>* value)
	{
		strcpy_s(value->GetBufferPointer(),value->GetBufferCount(),to_achar(profile.c_str()));
	}
	template <int N>
	void value_to_profile(const StaticString<ACHAR, N>& value, wstring* profile)
	{
		*profile = to_wchar(value.GetBufferPointer());
	}
	//wstring
	void profile_to_value(const wstring& profile, wstring* value)
	{
		*value = profile;
	}
	void value_to_profile(const wstring& value, wstring* profile)
	{
		*profile = value;
	}



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ���o�͕�                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// ���ӁFStringBuffer�n�̓o�b�t�@������Ȃ���abort���܂�
	template <class T> //T=={bool, int, WORD, wchar_t, char, wstring, StringBufferA, StringBufferW, StaticString}
	bool IOProfileData( const WCHAR* pszSectionName, const WCHAR* pszEntryKey, T& tEntryValue )
	{
		//�ǂݍ���
		if(m_bRead){
			//������ǂݍ���
			wstring buf;
			bool ret=GetProfileDataImp( pszSectionName, pszEntryKey, buf);
			if(ret){
				//T�ɕϊ�
				profile_to_value(buf, &tEntryValue);
			}
			return ret;
		}
		//��������
		else{
			//������ɕϊ�
			wstring buf;
			value_to_profile(tEntryValue, &buf);
			//�����񏑂�����
			return SetProfileDataImp( pszSectionName, pszEntryKey, buf);
		}
	}

	//2007.08.14 kobake �ǉ�
	//! int����ĔC�ӌ^�̓��o�͂��s��
	template <class T>
	bool IOProfileData_WrapInt( const WCHAR* pszSectionName, const WCHAR* pszEntryKey, T& nEntryValue)
	{
		int n=nEntryValue;
		bool ret=this->IOProfileData( pszSectionName, pszEntryKey, n );
		nEntryValue=(T)n;
		return ret;
	}
};
