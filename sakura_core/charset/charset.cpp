//
//	全面的に作り変え	2010/6/21 Uchi
//		interfaceはあまり変えない様にした
//
#include "StdAfx.h"
#include "charset.h"
#include <vector>
#include <map>

struct SCodeSet {
	ECodeType		m_eCodeSet;
	std::wstring	m_sNormal;
	std::wstring	m_sShort;
	std::wstring	m_sLong;		// for Combo
	bool			m_bUseBom;		// BOMが使えるか
	bool			m_bIsBomDefOn;	// BOMのデフォルトがOnか
	bool			m_bCanDefault;	// デフォルト文字コードになれるか
};

// 文字コードセット(初期データ)
static	SCodeSet	ASCodeSet[] = {
	{ CODE_AUTODETECT,	L"Auto",	L"Auto",	L"自動選択",	false,	false,	false },	//!< 文字コード自動判別	//mapには入れない
	{ CODE_SJIS,		L"SJIS",	L"SJIS",	L"SJIS",		false,	false,	true  },	//!< SJIS				(MS-CP932(Windows-31J), シフトJIS(Shift_JIS))
	{ CODE_JIS,			L"JIS",		L"JIS",		L"JIS",			false,	false,	false },	//!< JIS				(MS-CP5022x(ISO-2022-JP-MS))
	{ CODE_EUC,			L"EUC",		L"EUC",		L"EUC-JP",		false,	false,	true  },	//!< EUC				(MS-CP51932)	// eucJP-ms(eucJP-open)ではない
	{ CODE_LATIN1,		L"Latin1",	L"Latin1",	L"Latin1",		false,	false,	true  },	//!< Latin1				(欧文, Windows-932, Windows Codepage 1252 West European)
	{ CODE_UNICODE,		L"Unicode",	L"Uni",		L"Unicode",		true,	true,	true  },	//!< Unicode			(UTF-16 LittleEndian)	// UCS-2
	{ CODE_UNICODEBE,	L"UniBE",	L"UniBE",	L"UnicodeBE",	true,	true,	true  },	//!< Unicode BigEndian	(UTF-16 BigEndian)		// UCS-2
	{ CODE_UTF8,		L"UTF-8",	L"UTF-8",	L"UTF-8",		true,	false,	true  },	//!< UTF-8
	{ CODE_CESU8,		L"CESU-8",	L"CESU-8",	L"CESU-8",		true,	false,	true  },	//!< CESU-8				(UCS-2からUTF-8化)
	{ CODE_UTF7,		L"UTF-7",	L"UTF-7",	L"UTF-7",		true,	false,	false },	//!< UTF-7
};

// 文字コードセット
typedef	std::map<int, SCodeSet>	MSCodeSet;
static MSCodeSet				msCodeSet;
// 表示順
static std::vector<ECodeType>	vDispIdx;


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           初期化                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void InitCodeSet()
{
	if (msCodeSet.empty()) {
		int 	i;
		for (i = 0; i < _countof(ASCodeSet); i++) {
			vDispIdx.push_back( ASCodeSet[i].m_eCodeSet );
			if (i > 0) {
				msCodeSet[ASCodeSet[i].m_eCodeSet] = ASCodeSet[i];
			}
		}
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           判定                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
extern bool IsValidCodeType(int code)
{
	// 初期化
	InitCodeSet();
	return (msCodeSet.find( code ) != msCodeSet.end());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           名前                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LPCTSTR CCodeTypeName::Normal() const
{
	if (msCodeSet.find( m_eCodeType ) == msCodeSet.end()) {
		return NULL;
	}
	return to_tchar( msCodeSet[m_eCodeType].m_sNormal.c_str() );
}

LPCTSTR CCodeTypeName::Short() const
{
	if (msCodeSet.find( m_eCodeType ) == msCodeSet.end()) {
		return NULL;
	}
	return to_tchar( msCodeSet[m_eCodeType].m_sShort.c_str() );
}

LPCTSTR CCodeTypeName::Bracket() const
{
	if (msCodeSet.find( m_eCodeType ) == msCodeSet.end()) {
		return NULL;
	}

	static	std::wstring	sWork = L"  [" + msCodeSet[m_eCodeType].m_sShort + L"]";

	return to_tchar( sWork.c_str() );
}


bool CCodeTypeName::UseBom()
{
	if (msCodeSet.find( m_eCodeType ) == msCodeSet.end()) {
		return false;
	}

	return msCodeSet[m_eCodeType].m_bUseBom;
}

bool CCodeTypeName::IsBomDefOn()
{
	if (msCodeSet.find( m_eCodeType ) == msCodeSet.end()) {
		return false;
	}

	return msCodeSet[m_eCodeType].m_bIsBomDefOn;
}

bool CCodeTypeName::CanDefault()
{
	if (msCodeSet.find( m_eCodeType ) == msCodeSet.end()) {
		return false;
	}

	return msCodeSet[m_eCodeType].m_bCanDefault;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      コンボボックス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int CCodeTypesForCombobox::GetCount() const
{
	return vDispIdx.size();
}

ECodeType CCodeTypesForCombobox::GetCode(int nIndex) const
{
	return vDispIdx[nIndex];
}

LPCTSTR CCodeTypesForCombobox::GetName(int nIndex) const
{
	if (nIndex == 0) {
		return to_tchar( ASCodeSet[0].m_sLong.c_str() );
	}
	return to_tchar( msCodeSet[vDispIdx[nIndex]].m_sLong.c_str() );
}
