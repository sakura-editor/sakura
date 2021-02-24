/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_KeywordSet.h"
#include <limits>
#include "mem/CNativeW.h"
#include "charset/charcode.h"

/** startより後ろの語の境界の位置を返す。
	startより前の文字は読まない。一番大きい戻り値は str.GetLength()と等しくなる。
*/
static int NextWordBreak( const CStringRef& str, const int start );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キーワードセット                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CColor_KeywordSet::CColor_KeywordSet()
: m_nKeywordIndex(0)
, m_nCOMMENTEND(0)
{
}

// 2005.01.13 MIK 強調キーワード数追加に伴う配列化
bool CColor_KeywordSet::BeginColor(const CStringRef& cStr, int nPos)
{
	if( ! cStr.IsValid() ) {
		return false; // どうにもできない。
	}

	/*
		Summary:
			現在位置からキーワードを抜き出し、そのキーワードが登録単語ならば、色を変える
	*/

	const ECharKind charKind = CWordParse::WhatKindOfChar( cStr.GetPtr(), cStr.GetLength() , nPos );
	if( charKind <= CK_SPACE ){
		return false; // この文字はキーワード対象文字ではない。
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
		if( ! m_pTypeData->m_ColorInfoArr[ COLORIDX_KEYWORD1 + i ].m_bDisp ) {
			continue; // 色設定が非表示なのでスキップ。
		}
		const int iKwdSet = m_pTypeData->m_nKeyWordSetIdx[i];
		if( iKwdSet == -1 ) {
			continue; // キーワードセットが設定されていないのでスキップ。
		}
		int posWordEnd = nPos; ///< nPos...posWordEndがキーワード。
		int posWordEndCandidate = posNextWordHead; ///< nPos...posWordEndCandidateはキーワード候補。
		do {
			const int ret = GetDllShareData().m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SearchKeyWord2( iKwdSet, cStr.GetPtr() + nPos, posWordEndCandidate - nPos );
			if( 0 <= ret ) {
				// 登録されたキーワードだった。
				posWordEnd = posWordEndCandidate;
				if( ret == std::numeric_limits<int>::max() ) {
					// より長いキーワードも存在するので延長してリトライ。
					continue;
				}
				break;
			} else if( ret == -1 ) {
				// 登録されたキーワードではなかった。
				break;
			} else if( ret == -2 ) {
				// 長さが足りなかったので延長してリトライ。
				continue;
			} else {
				// 登録されたキーワードではなかった？
				// CKeyWordSetMgr::SearchKeyWord2()から想定外の戻り値。
				break;
			}
		} while( posWordEndCandidate < cStr.GetLength() && ((posWordEndCandidate = NextWordBreak( cStr, posWordEndCandidate )) != 0) );

		// nPos...posWordEnd がキーワード。
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
	CLogicInt nColumnNew;
	if( CWordParse::SearchNextWordPosition4KW( str.GetPtr(), CLogicInt(str.GetLength()), CLogicInt(start), &nColumnNew, true ) ){
		return nColumnNew;
	}
	return start;
}
