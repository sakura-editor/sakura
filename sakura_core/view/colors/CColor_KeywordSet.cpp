#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_KeywordSet.h"
#include <limits>
#include "mem/CNativeW.h"
#include "charset/charcode.h"

/** start�����̌�̋��E�̈ʒu��Ԃ��B
	start���O�̕����͓ǂ܂Ȃ��B��ԑ傫���߂�l�� str.GetLength()�Ɠ������Ȃ�B
*/
static int NextWordBreak( const CStringRef& str, const int start );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�Z�b�g                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CColor_KeywordSet::CColor_KeywordSet()
: m_nKeywordIndex(0)
, m_nCOMMENTEND(0)
{
}


// 2005.01.13 MIK �����L�[���[�h���ǉ��ɔ����z��
bool CColor_KeywordSet::BeginColor(const CStringRef& cStr, int nPos)
{
	if( ! cStr.IsValid() ) {
		return false; // �ǂ��ɂ��ł��Ȃ��B
	}
	const CEditDoc* const pDoc = CEditDoc::GetInstance(0);
	const STypeConfig& doctype = pDoc->m_cDocType.GetDocumentAttribute();
	
	/*
		Summary:
			���݈ʒu����L�[���[�h�𔲂��o���A���̃L�[���[�h���o�^�P��Ȃ�΁A�F��ς���
	*/

	const ECharKind charKind = CWordParse::WhatKindOfChar( cStr.GetPtr(), cStr.GetLength() , nPos );
	if( charKind <= CK_SPACE ){
		return false; // ���̕����̓L�[���[�h�Ώە����ł͂Ȃ��B
	}
	if( 0 < nPos ){
		const ECharKind charKindPrev = CWordParse::WhatKindOfChar( cStr.GetPtr(), cStr.GetLength() , nPos-1 );
		const ECharKind charKindTwo = CWordParse::WhatKindOfTwoChars4KW( charKindPrev, charKind );
		if( charKindTwo != CK_NULL ){
			return false;
		}
	}

	const int posNextWordHead = NextWordBreak( cStr, nPos );
	for( int i = 0; i < MAX_KEYWORDSET_PER_TYPE; ++i ) {
		if( ! doctype.m_ColorInfoArr[ COLORIDX_KEYWORD1 + i ].m_bDisp ) {
			continue; // �F�ݒ肪��\���Ȃ̂ŃX�L�b�v�B
		}
		const int iKwdSet = doctype.m_nKeyWordSetIdx[i];
		if( iKwdSet == -1 ) {
			continue; // �L�[���[�h�Z�b�g���ݒ肳��Ă��Ȃ��̂ŃX�L�b�v�B
		}
		int posWordEnd = nPos; ///< nPos...posWordEnd���L�[���[�h�B
		int posWordEndCandidate = posNextWordHead; ///< nPos...posWordEndCandidate�̓L�[���[�h���B
		do {
			const int ret = GetDllShareData().m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SearchKeyWord2( iKwdSet, cStr.GetPtr() + nPos, posWordEndCandidate - nPos );
			if( 0 <= ret ) {
				// �o�^���ꂽ�L�[���[�h�������B
				posWordEnd = posWordEndCandidate;
				if( ret == std::numeric_limits<int>::max() ) {
					// ��蒷���L�[���[�h�����݂���̂ŉ������ă��g���C�B
					continue;
				}
				break;
			} else if( ret == -1 ) {
				// �o�^���ꂽ�L�[���[�h�ł͂Ȃ������B
				break;
			} else if( ret == -2 ) {
				// ����������Ȃ������̂ŉ������ă��g���C�B
				continue;
			} else {
				// �o�^���ꂽ�L�[���[�h�ł͂Ȃ������H
				// CKeyWordSetMgr::SearchKeyWord2()����z��O�̖߂�l�B
				break;
			}
		} while( posWordEndCandidate < cStr.GetLength() && (posWordEndCandidate = NextWordBreak( cStr, posWordEndCandidate )) != NULL );

		// nPos...posWordEnd ���L�[���[�h�B
		if( nPos < posWordEnd ) {
			this->m_nCOMMENTEND = posWordEnd;
			this->m_nKeywordIndex = i;
			return true;
		}
	}
	return false;
}

bool CColor_KeywordSet::EndColor(const CStringRef& cStr, int nPos)
{
	return nPos == this->m_nCOMMENTEND;
}


static inline int NextWordBreak( const CStringRef& str, const int start )
{
	CLogicInt nColmNew;
	if( CWordParse::SearchNextWordPosition4KW( str.GetPtr(), CLogicInt(str.GetLength()), CLogicInt(start), &nColmNew, true ) ){
		return nColmNew;
	}
	return start;
}
