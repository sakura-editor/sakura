/*! @file
	@brief �����R�[�h�Z�b�g�̊Ǘ�

	@author kobake
	@date 2008
	@date 2010/6/21	Uchi	�S�ʓI�ɍ��ς�(�}�[�W��2013/1/4)
							interface�͂��܂�ς��Ȃ��l�ɂ���
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2009, rastiv
	Copyright (C) 2010, Uchi
	Copyright (C) 2012, novice
	Copyright (C) 2013, Moca, Uchi

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
#include "charset.h"
#include <vector>
#include <map>

struct SCodeSet {
	ECodeType		m_eCodeSet;
	const WCHAR*	m_sNormal;
	const WCHAR*	m_sShort;
	const WCHAR*	m_sLong;		// for Combo
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
	return to_tchar( msCodeSet[m_eCodeType].m_sNormal );
}

LPCTSTR CCodeTypeName::Short() const
{
	if (msCodeSet.find( m_eCodeType ) == msCodeSet.end()) {
		return NULL;
	}
	return to_tchar( msCodeSet[m_eCodeType].m_sShort );
}

LPCTSTR CCodeTypeName::Bracket() const
{
	if (msCodeSet.find( m_eCodeType ) == msCodeSet.end()) {
		return NULL;
	}

//	static	std::wstring	sWork = L"  [" + msCodeSet[m_eCodeType].m_sShort + L"]";
	static	std::wstring	sWork;
	sWork = std::wstring(L"  [") + msCodeSet[m_eCodeType].m_sShort + L"]";	// �ϐ��̒�`�ƒl�̐ݒ���ꏏ�ɂ��ƃo�O��l�Ȃ̂ŕ���	// 2013/4/20 Uchi

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
		return to_tchar( ASCodeSet[0].m_sLong );
	}
	return to_tchar( msCodeSet[vDispIdx[nIndex]].m_sLong );
}
