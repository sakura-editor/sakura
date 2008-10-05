#include "stdafx.h"
#include "CColor_KeywordSet.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キーワードセット                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CColor_KeywordSet::CColor_KeywordSet(int nKeywordIndex)
: m_nKeywordIndex(nKeywordIndex)
, m_nCOMMENTEND(0)
{
	assert(m_nKeywordIndex>=0 && m_nKeywordIndex<MAX_KEYWORDSET_PER_TYPE);
}


// 2005.01.13 MIK 強調キーワード数追加に伴う配列化
bool CColor_KeywordSet::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	
	/*
		Summary:
			現在位置からキーワードを抜き出し、そのキーワードが登録単語ならば、色を変える
	*/
	if( TypeDataPtr->m_ColorInfoArr[GetStrategyColor()].m_bDisp &&  /* 強調キーワードを表示する */ // 2002/03/13 novice
		_IsPosKeywordHead(cStr,nPos) && IS_KEYWORD_CHAR(cStr.At(nPos))
	){
		// キーワードの開始 -> iKeyBegin
		int iKeyBegin = nPos;

		// キーワードの終端 -> iKeyEnd
		int iKeyEnd;
		for( iKeyEnd = iKeyBegin + 1; iKeyEnd <= cStr.GetLength() - 1; ++iKeyEnd ){
			if( !IS_KEYWORD_CHAR( cStr.At(iKeyEnd) ) ){
				break;
			}
		}

		// キーワードの長さ -> nKeyLen
		int nKeyLen = iKeyEnd - iKeyBegin;

		// キーワードが色変え対象であるか調査
		for( int my_i = 0; my_i < MAX_KEYWORDSET_PER_TYPE; my_i++ )
		{
			if( TypeDataPtr->m_nKeyWordSetIdx[my_i] != -1 && // キーワードセット
				TypeDataPtr->m_ColorInfoArr[GetStrategyColor()].m_bDisp)								//MIK
			{																							//MIK
				/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */						//MIK
				int nIdx = GetDllShareData().m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SearchKeyWord2(							//MIK 2000.12.01 binary search
					TypeDataPtr->m_nKeyWordSetIdx[my_i] ,									//MIK
					&cStr.GetPtr()[iKeyBegin],															//MIK
					nKeyLen																				//MIK
				);																						//MIK
				if( nIdx != -1 ){																		//MIK
					this->m_nCOMMENTEND = iKeyEnd;														//MIK
					return true;
				}																						//MIK
			}
		}
	}
	return false;
}

bool CColor_KeywordSet::EndColor(const CStringRef& cStr, int nPos)
{
	if( nPos == this->m_nCOMMENTEND ){
		return true;
	}
	return false;
}

