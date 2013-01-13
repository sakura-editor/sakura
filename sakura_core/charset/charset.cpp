//
//	�S�ʓI�ɍ��ς�	2010/6/21 Uchi
//		interface�͂��܂�ς��Ȃ��l�ɂ���
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
	bool			m_bUseBom;		// BOM���g���邩
	bool			m_bIsBomDefOn;	// BOM�̃f�t�H���g��On��
	bool			m_bCanDefault;	// �f�t�H���g�����R�[�h�ɂȂ�邩
};

// �����R�[�h�Z�b�g(�����f�[�^)
static	SCodeSet	ASCodeSet[] = {
	{ CODE_AUTODETECT,	L"Auto",	L"Auto",	L"�����I��",	false,	false,	false },	//!< �����R�[�h��������	//map�ɂ͓���Ȃ�
	{ CODE_SJIS,		L"SJIS",	L"SJIS",	L"SJIS",		false,	false,	true  },	//!< SJIS				(MS-CP932(Windows-31J), �V�t�gJIS(Shift_JIS))
	{ CODE_JIS,			L"JIS",		L"JIS",		L"JIS",			false,	false,	false },	//!< JIS				(MS-CP5022x(ISO-2022-JP-MS))
	{ CODE_EUC,			L"EUC",		L"EUC",		L"EUC-JP",		false,	false,	true  },	//!< EUC				(MS-CP51932)	// eucJP-ms(eucJP-open)�ł͂Ȃ�
	{ CODE_LATIN1,		L"Latin1",	L"Latin1",	L"Latin1",		false,	false,	true  },	//!< Latin1				(����, Windows-932, Windows Codepage 1252 West European)
	{ CODE_UNICODE,		L"Unicode",	L"Uni",		L"Unicode",		true,	true,	true  },	//!< Unicode			(UTF-16 LittleEndian)	// UCS-2
	{ CODE_UNICODEBE,	L"UniBE",	L"UniBE",	L"UnicodeBE",	true,	true,	true  },	//!< Unicode BigEndian	(UTF-16 BigEndian)		// UCS-2
	{ CODE_UTF8,		L"UTF-8",	L"UTF-8",	L"UTF-8",		true,	false,	true  },	//!< UTF-8
	{ CODE_CESU8,		L"CESU-8",	L"CESU-8",	L"CESU-8",		true,	false,	true  },	//!< CESU-8				(UCS-2����UTF-8��)
	{ CODE_UTF7,		L"UTF-7",	L"UTF-7",	L"UTF-7",		true,	false,	false },	//!< UTF-7
};

// �����R�[�h�Z�b�g
typedef	std::map<int, SCodeSet>	MSCodeSet;
static MSCodeSet				msCodeSet;
// �\����
static std::vector<ECodeType>	vDispIdx;


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ������                            //
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
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
extern bool IsValidCodeType(int code)
{
	// ������
	InitCodeSet();
	return (msCodeSet.find( code ) != msCodeSet.end());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���O                              //
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
//                      �R���{�{�b�N�X                         //
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
