/*!	@file
	@brief CEolクラスの実装

	@author genta
	@date 2000/05/15 新規作成 genta
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2000, Frozen, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "basis/CEol.h"

#include "charset/CCodeFactory.h"
#include "charset/charset.h"
#include "CSelectLang.h"

#include "sakura_rc.h"

//-----------------------------------------------
//	固定データ
//-----------------------------------------------

const std::array g_EolDefinitions = {
	SEolDefinition{ EEolType::none,					IDS_EOLTYPENAME_0, {}                 },
	SEolDefinition{ EEolType::cr_and_lf,			IDS_EOLTYPENAME_1, WCODE::CRLF        },
	SEolDefinition{ EEolType::line_feed,			IDS_EOLTYPENAME_2, { &WCODE::LF, 1 }  },
	SEolDefinition{ EEolType::carriage_return,		IDS_EOLTYPENAME_3, { &WCODE::CR, 1 }  },
	SEolDefinition{ EEolType::next_line,			IDS_EOLTYPENAME_4, { &WCODE::NEL, 1 } },
	SEolDefinition{ EEolType::line_separator,		IDS_EOLTYPENAME_5, { &WCODE::PS_, 1 } },
	SEolDefinition{ EEolType::paragraph_separator,	IDS_EOLTYPENAME_6, { &WCODE::LS_, 1 } },
};

//-----------------------------------------------
//	実装補助
//-----------------------------------------------

constexpr SEolDefinition::SEolDefinition(
	EEolType eEolType,
	int wName,
	std::wstring_view data
)
	: m_eEolType(eEolType)
	, m_wName(static_cast<WORD>(wName))
	, m_Data(data)
{
}

const std::wstring&	SEolDefinition::GetVal() const noexcept
{
	return m_Data;
}

std::string SEolDefinition::GetVal(ECodeType eCodeType) const
{
	auto result = CCodeFactory::ConvertToCode(eCodeType, m_Data);
	return std::move(result.destination);
}

bool SEolDefinition::StartsWith(LPCWSTR pData, size_t nLen) const noexcept
{
	const std::wstring_view text{ pData, nLen };
	return !m_Data.empty() && text.starts_with(m_Data);
}

bool SEolDefinition::StartsWith(LPCSTR pData, size_t nLen, ECodeType eCodeType) const
{
	const auto eol = GetVal(eCodeType);
	return nLen && !eol.empty() && std::size(eol) <= nLen && 0 == ::memcmp(pData, std::data(eol), std::size(eol));
}

//-----------------------------------------------
//	実装部
//-----------------------------------------------

//! 現在のEOLの名称取得
LPCWSTR CEol::GetName() const
{
	return LS(GetData().m_wName);
}

//! 現在のEOL文字列先頭へのポインタを取得
LPCWSTR CEol::GetValue2() const noexcept
{
	return std::data(GetData().GetVal());
}

//! 現在のEOL文字列長を取得。文字単位。
CLogicInt CEol::GetLen() const noexcept
{
	return CLogicInt(int(std::size(GetData().GetVal())));
}

const SEolDefinition& CEol::GetData() const noexcept
{
	return *std::ranges::find_if(g_EolDefinitions, [this] (const SEolDefinition& eolDef) { return m_eEolType == eolDef.m_eEolType; });
}

void CEol::SetTypeByString( const wchar_t* pszData, size_t nDataLen )
{
	auto eEolType = EEolType::none;
	for (const auto& eolDef : g_EolDefinitions) {
		if (eolDef.StartsWith(pszData, nDataLen)) {
			eEolType = eolDef.m_eEolType;
			break;
		}
	}
	SetType(eEolType);
}

void CEol::SetTypeByString( const char* pszData, size_t nDataLen)
{
	auto eEolType = EEolType::none;
	for (const auto& eolDef : g_EolDefinitions) {
		if (eolDef.StartsWith(pszData, nDataLen, CODE_SJIS)) {
			eEolType = eolDef.m_eEolType;
			break;
		}
	}
	SetType(eEolType);
}

void CEol::SetTypeByStringForFile_uni( const char* pszData, size_t nDataLen )
{
	auto eEolType = EEolType::none;
	for (const auto& eolDef : g_EolDefinitions) {
		if (eolDef.StartsWith(pszData, nDataLen, CODE_UTF16LE)) {
			eEolType = eolDef.m_eEolType;
			break;
		}
	}
	SetType(eEolType);
}

void CEol::SetTypeByStringForFile_unibe( const char* pszData, size_t nDataLen )
{
	auto eEolType = EEolType::none;
	for (const auto& eolDef : g_EolDefinitions) {
		if (eolDef.StartsWith(pszData, nDataLen, CODE_UTF16BE)) {
			eEolType = eolDef.m_eEolType;
			break;
		}
	}
	SetType(eEolType);
}
