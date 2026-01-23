/*! @file */
/*
	Copyright (C) 2011, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Heredoc.h"
#include "doc/layout/CLayout.h"

class CLayoutColorHeredocInfo : public CLayoutColorInfo{
public:
	std::wstring m_id;
	bool IsEqual(const CLayoutColorInfo* p) const override{
		if( !p ){
			return false;
		}
		const CLayoutColorHeredocInfo* info = dynamic_cast<const CLayoutColorHeredocInfo*>(p);
		if( info == nullptr ){
			return false;
		}
		return info->m_id == this->m_id;
	}
};

void CColor_Heredoc::SetStrategyColorInfo(const CLayoutColorInfo* colorInfo)
{
	if( colorInfo ){
		const CLayoutColorHeredocInfo* info = dynamic_cast<const CLayoutColorHeredocInfo*>(colorInfo);
		if( info == nullptr ){
			return;
		}
		m_pszId = info->m_id.c_str();
		m_nSize = info->m_id.size();
	}
}

CLayoutColorInfo* CColor_Heredoc::GetStrategyColorInfo() const
{
	CLayoutColorHeredocInfo* info = new CLayoutColorHeredocInfo();
	info->m_id.assign(m_pszId, m_nSize);
	return info;
}

bool CColor_Heredoc::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	// ヒアドキュメント
	// <<<HEREDOC_ID
	// ...
	// HEREDOC_ID
	if( m_pTypeData->m_nHeredocType == HEREDOC_PHP
	 && cStr[nPos] == '<' && nPos + 3 < cStr.GetLength()
	 && wmemcmp(cStr.GetPtr() + nPos + 1, L"<<", 2) == 0
	){
		// <<<[ \t]*((['"][_A-Za-z0-9]+['"])|[_A-Za-z0-9]+)[\r\n]+
		const int length = cStr.GetLength();
		int nPosIdStart = nPos + 3;
		for(; nPosIdStart < length; nPosIdStart++ ){
			if(cStr[nPosIdStart] != L'\t' && cStr[nPosIdStart] != L' '){
				break;
			}
		}
		wchar_t quote = L'\0';
		if( !(nPosIdStart < length) ){
			return false;
		}
		if (cStr[nPosIdStart] == L'\'' || cStr[nPosIdStart] == L'"') {
			quote = cStr[nPosIdStart];
			nPosIdStart++;
		}
		int i = nPosIdStart;
		for(; i < length; i++ ){
			if( !(WCODE::IsAZ(cStr[i]) || WCODE::Is09(cStr[i]) || cStr[i] == L'_') ){
				break;
			}
		}
		if( nPosIdStart == i ){
			return false;
		}
		const int k = i;
		if( quote != L'\0' ){
			if( i < length && cStr[i] == quote ){
				i++;
			}else{
				return false;
			}
		}
		if( i < length && WCODE::IsLineDelimiter(cStr[i], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
			m_id = std::wstring(cStr.GetPtr()+nPosIdStart, k - nPosIdStart);
			m_pszId = m_id.c_str();
			m_nSize = m_id.size();
			this->m_nCOMMENTEND = length;
			return true;
		}
	}
	return false;
}

bool CColor_Heredoc::EndColor(const CStringRef& cStr, int nPos)
{
	if (!m_nCOMMENTEND) {
		if( m_pTypeData->m_nHeredocType == HEREDOC_PHP
		 && nPos == 0 && m_nSize <= size_t(cStr.GetLength())
		 && wmemcmp(cStr.GetPtr(), m_pszId, m_nSize) == 0 ){
			if (m_nSize == size_t(cStr.GetLength())) {
				m_nCOMMENTEND = m_nSize;
				return false;
			}else{
				size_t i = m_nSize;
				if( i + 1 < size_t(cStr.GetLength()) && cStr[i] == L';' && WCODE::IsLineDelimiter(cStr[i+1], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
					// ID;
					m_nCOMMENTEND = i;
					return false;
				}else if( m_nSize < size_t(cStr.GetLength()) && WCODE::IsLineDelimiter(cStr[m_nSize], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
					// ID
					m_nCOMMENTEND = m_nSize;
					return false;
				}
			}
			m_nCOMMENTEND = cStr.GetLength();
		}else{
			m_nCOMMENTEND = cStr.GetLength();
		}
	}
	else if (nPos == int(m_nCOMMENTEND)) {
		return true;
	}
	return false;
}
