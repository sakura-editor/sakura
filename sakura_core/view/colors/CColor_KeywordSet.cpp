#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_KeywordSet.h"
#include <limits>
#include "mem/CNativeW.h"
#include "charset/charcode.h"

/** 文字を分類する。
	@retval 0 キーワードに使える文字ではない。
	@retval その他の数 連続する同じ数の文字が一つのキーワードをつくる。
	たとえば、ABCDEFGという文字列があり、それぞれの文字の CharTypeが順番に 1121133だったとすると、AB、C、DE、FGがキーワード候補になる。
*/
static int CharType( const wchar_t wch );

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
	const CEditDoc* const pDoc = CEditDoc::GetInstance(0);
	const STypeConfig& doctype = pDoc->m_cDocType.GetDocumentAttribute();
	
	/*
		Summary:
			現在位置からキーワードを抜き出し、そのキーワードが登録単語ならば、色を変える
	*/

	const int charType = CharType( cStr.At( nPos ) );
	if( ! charType ) {
		return false; // この文字はキーワード対象文字ではない。
	}
	if( 0 < nPos && charType == CharType( cStr.At( nPos - 1 ) ) ) {
		return false; // 語の境界ではなかった。
	}

	const int posNextWordHead = NextWordBreak( cStr, nPos );
	for( int i = 0; i < MAX_KEYWORDSET_PER_TYPE; ++i ) {
		if( ! doctype.m_ColorInfoArr[ COLORIDX_KEYWORD1 + i ].m_bDisp ) {
			continue; // 色設定が非表示なのでスキップ。
		}
		const int iKwdSet = doctype.m_nKeyWordSetIdx[i];
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
		} while( posWordEndCandidate < cStr.GetLength() && (posWordEndCandidate = NextWordBreak( cStr, posWordEndCandidate )) );

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


static inline int CharType( const wchar_t wch )
{
	if( ! wch || WCODE::IsBlank( wch ) || WCODE::IsLineDelimiter( wch ) || WCODE::IsControlCode( wch ) ) {
		return 0;
	}
	const int charType = IS_KEYWORD_CHAR( wch ) ? 1 : 2;
	return charType;
}

static int NextWordBreak( const CStringRef& str, const int start )
{
	const int charType = CharType( str.At( start ) );
	int result = start;
	for( ; result < str.GetLength(); result += CNativeW::GetSizeOfChar( str.GetPtr(), str.GetLength(), result ) ) {
		if( charType != CharType( str.At( result ) ) ) {
			break;
		}
	}
	return result;
}
