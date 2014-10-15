#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Quote.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �N�H�[�e�[�V����                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CLayoutColorQuoteInfo : public CLayoutColorInfo{
public:
	std::wstring m_tag;
	int m_nColorTypeIndex;
	bool IsEqual(const CLayoutColorInfo* p) const{
		if( !p ){
			return false;
		}
		const CLayoutColorQuoteInfo* info = dynamic_cast<const CLayoutColorQuoteInfo*>(p);
		if( info == NULL ){
			return false;
		}
		return info->m_tag == this->m_tag;
	}
};

void CColor_Quote::Update(void)
{
	const CEditDoc* pCEditDoc = CEditDoc::GetInstance(0);
	m_pTypeData = &pCEditDoc->m_cDocType.GetDocumentAttribute();
	m_nStringType = m_pTypeData->m_nStringType;
	int nEspaceTypeList[] = {
		STRING_LITERAL_CPP,
		STRING_LITERAL_PLSQL,
		STRING_LITERAL_HTML,
		STRING_LITERAL_CPP,
		STRING_LITERAL_CPP,
	};
	m_nEscapeType = nEspaceTypeList[m_nStringType];
	bool* pbEscapeEndList[] = {
		&m_bEscapeEnd,
		NULL,
		NULL,
		NULL,
		&m_bEscapeEnd,
	};
	m_pbEscapeEnd = pbEscapeEndList[m_nStringType];
}

void CColor_Quote::SetStrategyColorInfo(const CLayoutColorInfo* colorInfo)
{
	if( colorInfo ){
		const CLayoutColorQuoteInfo* info = dynamic_cast<const CLayoutColorQuoteInfo*>(colorInfo);
		if( info == NULL ){
			return;
		}
		m_tag = info->m_tag;
		m_nColorTypeIndex = info->m_nColorTypeIndex;
	}else{
		m_nColorTypeIndex = 0;
	}
}

CLayoutColorInfo* CColor_Quote::GetStrategyColorInfo() const
{
	if( 0 < m_nColorTypeIndex ){
		CLayoutColorQuoteInfo* info = new CLayoutColorQuoteInfo();
		info->m_tag = m_tag;
		info->m_nColorTypeIndex = m_nColorTypeIndex;
		return info;
	}
	return NULL;
}

bool CColor_Quote::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	if( cStr.At(nPos) == m_cQuote ){
		m_nCOMMENTEND = -1;
		int nStringType = m_pTypeData->m_nStringType;
		bool bPreString = true;
		/* �N�H�[�e�[�V����������̏I�[�����邩 */
		switch( nStringType ){
		case STRING_LITERAL_CPP:
			if( 0 < nPos && cStr.At(nPos-1) == 'R' && cStr.At(nPos) == '"'
				&& nPos + 1 < cStr.GetLength() ){
				for( int i = nPos + 1; i < cStr.GetLength(); i++ ){
					if( cStr.At(i) == '(' ){
						if( nPos + 1 < i ){
							m_tag = L')';
							m_tag.append( cStr.GetPtr()+nPos+1, i - (nPos + 1) );
							m_tag += L'"';
						}else{
							m_tag.assign(L")\"", 2);
						}
						m_nCOMMENTEND = Match_QuoteStr( m_tag.c_str(), m_tag.size(), i + 1, cStr, false );
						m_nColorTypeIndex = 1;
						return true;
					}
				}
			}
			break;
		case STRING_LITERAL_HTML:
			{
				int i;
				for(i = nPos - 1; 0 <= i; i--){
					if( cStr.At(i) != L' ' && cStr.At(i) != L'\t' ){
						break;
					}
				}
				if( !(0 <= i && cStr.At(i) == L'=') ){
					bPreString = false;
				}
			}
			break;
		case STRING_LITERAL_CSHARP:
			if( 0 < nPos && cStr.At(nPos - 1) == L'@' && m_cQuote == L'"' ){
				m_nCOMMENTEND = Match_Quote( m_cQuote, nPos + 1, cStr, STRING_LITERAL_PLSQL );
				m_nColorTypeIndex = 2;
				return true;
			}
			break;
		case STRING_LITERAL_PYTHON:
			if( nPos + 2 < cStr.GetLength()
			 && cStr.At(nPos+1) == m_cQuote && cStr.At(nPos+2) == m_cQuote ){
				m_nCOMMENTEND = Match_QuoteStr( m_szQuote, 3, nPos + 3, cStr, true );
				m_nColorTypeIndex = 3;
				return true;
			}
			break;
		}
		m_bEscapeEnd = false;
		if( bPreString ){
			m_nCOMMENTEND = Match_Quote( m_cQuote, nPos + 1, cStr, m_nEscapeType, m_pbEscapeEnd );
			m_nColorTypeIndex = 0;
		}

		// �u������͍s���̂݁v(C++ Raw String�APython��long string�A@""�͓���)
		if( m_pTypeData->m_bStringLineOnly && !m_bEscapeEnd
				&& m_nCOMMENTEND == cStr.GetLength() ){
			// �I�������񂪂Ȃ��ꍇ�͍s���܂ł�F����
			if( m_pTypeData->m_bStringEndLine ){
				// ���s�R�[�h������
				if( 0 < cStr.GetLength() && WCODE::IsLineDelimiter(cStr.At(cStr.GetLength()-1), GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
					if( 1 < cStr.GetLength() && cStr.At(cStr.GetLength()-2) == WCODE::CR
							&& cStr.At(cStr.GetLength()-1) == WCODE::LF ){
						m_nCOMMENTEND = cStr.GetLength() - 2;
					}else{
						m_nCOMMENTEND = cStr.GetLength() - 1;
					}
				}
				return true;
			}
			// �I�������񂪂Ȃ��ꍇ�͐F�������Ȃ�
			m_nCOMMENTEND = -1;
			return false;
		}
		if( 0 < m_nCOMMENTEND ){
			return true;
		}
	}
	return false;
}

bool CColor_Quote::EndColor(const CStringRef& cStr, int nPos)
{
	if( -1 == m_nCOMMENTEND ){
		// �����ɂ���͍̂s���̂͂�
		assert_warning( 0 == nPos );
		// �N�H�[�e�[�V����������̏I�[�����邩
		switch( m_nColorTypeIndex ){
		case 0:
			m_nCOMMENTEND = Match_Quote( m_cQuote, nPos, cStr, m_nEscapeType );
			break;
		case 1:
			m_nCOMMENTEND = Match_QuoteStr( m_tag.c_str(), m_tag.size(), nPos, cStr, false );
			break;
		case 2:
			m_nCOMMENTEND = Match_Quote( m_cQuote, nPos, cStr, STRING_LITERAL_PLSQL );
			break;
		case 3:
			m_nCOMMENTEND = Match_QuoteStr( m_szQuote, 3, nPos, cStr, true );
			break;
		}
		// -1��EndColor���Ăяo�����͍̂s�𒴂��Ă�������Ȃ̂ōs���`�F�b�N�͕s�v
	}
	else if( nPos == m_nCOMMENTEND ){
		return true;
	}
	return false;
}

int CColor_Quote::Match_Quote( wchar_t wcQuote, int nPos, const CStringRef& cLineStr, int escapeType, bool* pbEscapeEnd )
{
	int nCharChars;
	int i;
	for( i = nPos; i < cLineStr.GetLength(); ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = (Int)t_max(CLogicInt(1), CNativeW::GetSizeOfChar( cLineStr.GetPtr(), cLineStr.GetLength(), i ));
		if( escapeType == STRING_LITERAL_CPP ){
			// �G�X�P�[�v \"
			if( 1 == nCharChars && cLineStr.At(i) == L'\\' ){
				++i;
				if( i < cLineStr.GetLength() && WCODE::IsLineDelimiter(cLineStr.At(i), GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
					if( pbEscapeEnd ){
						*pbEscapeEnd = true;
					}
				}
			}else
			if( 1 == nCharChars && cLineStr.At(i) == wcQuote ){
				return i + 1;
			}
		}else if( escapeType == STRING_LITERAL_PLSQL ){
			// �G�X�P�[�v ""
			if( 1 == nCharChars && cLineStr.At(i) == wcQuote ){
				if( i + 1 < cLineStr.GetLength() && cLineStr.At(i + 1) == wcQuote ){
					++i;
				}else{
					return i + 1;
				}
			}
		}else{
			// �G�X�P�[�v�Ȃ�
			if( 1 == nCharChars && cLineStr.At(i) == wcQuote ){
				return i + 1;
			}
		}
		if( 2 == nCharChars ){
			++i;
		}
	}
	return cLineStr.GetLength();
}

int CColor_Quote::Match_QuoteStr( const wchar_t* pszQuote, int nQuoteLen, int nPos, const CStringRef& cLineStr, bool bEscape )
{
	int nCharChars;
	int i;
	const int nCompLen = cLineStr.GetLength() - nQuoteLen + 1;
	const WCHAR quote1 = pszQuote[0];
	const WCHAR* pLine = cLineStr.GetPtr();
	for( i = nPos; i < nCompLen; i += nCharChars ){
		if( quote1 == pLine[i] && wmemcmp( pszQuote + 1, pLine + i + 1, nQuoteLen - 1 ) == 0 ){
			return i + nQuoteLen;
		}
		nCharChars = (Int)t_max(CLogicInt(1), CNativeW::GetSizeOfChar( pLine, cLineStr.GetLength(), i ));
		if( bEscape && pLine[i] == L'\\' ){
			i += (Int)t_max(CLogicInt(1), CNativeW::GetSizeOfChar( pLine, cLineStr.GetLength(), i + nCharChars ));
		}
	}
	return cLineStr.GetLength();
}
