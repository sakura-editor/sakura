#include "stdafx.h"
#include "CDraw_Quote.h"

#define SetNPos(N) pInfo->nPos=(N)
#define GetNPos() (pInfo->nPos+CLogicInt(0))

#define SetNBgn(N) pInfo->nBgn=(N)
#define GetNBgn() (pInfo->nBgn+0)

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 シングルクォーテーション                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDraw_SingleQuote::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( pInfo->pLine[pInfo->nPos] == L'\'' &&
		TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* シングルクォーテーション文字列を表示する */
	){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_SSTRING);	/* シングルクォーテーション文字列である */ // 2002/03/13 novice

		/* シングルクォーテーション文字列の終端があるか */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			int nCharChars_2 = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, i );
			if( 0 == nCharChars_2 ){
				nCharChars_2 = 1;
			}
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\\' ){
					++i;
				}else
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\'' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\'' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'\'' ){
						++i;
					}else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
			if( 2 == nCharChars_2 ){
				++i;
			}
		}
		return true;
	}
	return false;
}


bool CDraw_SingleQuote::EndColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* シングルクォーテーション文字列の終端があるか */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( i = pInfo->nPos/* + 1*/; i <= pInfo->nLineLen - 1; ++i ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			int nCharChars_2 = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, i );
			if( 0 == nCharChars_2 ){
				nCharChars_2 = 1;
			}
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\\' ){
					++i;
				}else
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\'' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\'' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'\'' ){
						++i;
					}else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
			if( 2 == nCharChars_2 ){
				++i;
			}
		}
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_TEXT); // 2002/03/13 novice
		return true;
	}
	return false;
}

bool CDraw_SingleQuote::GetColorIndexImp(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( pInfo->pLine[pInfo->nPos] == L'\'' &&
		TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* シングルクォーテーション文字列を表示する */
	){
		pInfo->nBgn = pInfo->nPos;
		pInfo->nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice

		/* 現在の色を指定 */
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		/* シングルクォーテーション文字列の終端があるか */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			CLogicInt	nCharChars_2 = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, i );
			if( 0 == nCharChars_2 ){
				nCharChars_2 = CLogicInt(1);
			}
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\\' ){
					++i;
				}else
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\'' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\'' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'\'' ){
						++i;
					}else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
			if( 2 == nCharChars_2 ){
				++i;
			}
		}
		return true;
	}
	return false;
}

bool CDraw_SingleQuote::GetColorIndexImpEnd(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* シングルクォーテーション文字列の終端があるか */
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( int i = pInfo->nPos/* + 1*/; i <= pInfo->nLineLen - 1; ++i ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			CLogicInt	nCharChars_2 = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, i );
			if( 0 == nCharChars_2 ){
				nCharChars_2 = CLogicInt(1);
			}
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\\' ){
					++i;
				}else
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\'' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}else
			if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\'' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'\'' ){
						++i;
					}else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
			if( 2 == nCharChars_2 ){
				++i;
			}
		}
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->nBgn = pInfo->nPos;
		pInfo->nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		/* 現在の色を指定 */
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  ダブルクォーテーション                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDraw_DoubleQuote::BeginColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( pInfo->pLine[pInfo->nPos] == L'"' &&
		TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
	){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_WSTRING);	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
		/* ダブルクォーテーション文字列の終端があるか */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			int nCharChars_2 = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, i );
			if( 0 == nCharChars_2 ){
				nCharChars_2 = 1;
			}
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\\' ){
					++i;
				}else
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'"' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'"' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'"' ){
						++i;
					}else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
			if( 2 == nCharChars_2 ){
				++i;
			}
		}
		return true;
	}
	return false;
}



bool CDraw_DoubleQuote::EndColor(SDrawStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* ダブルクォーテーション文字列の終端があるか */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( i = pInfo->nPos/* + 1*/; i <= pInfo->nLineLen - 1; ++i ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			int nCharChars_2 = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, i );
			if( 0 == nCharChars_2 ){
				nCharChars_2 = 1;
			}
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\\' ){
					++i;
				}
				else if( 1 == nCharChars_2 && pInfo->pLine[i] == L'"' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}
			else if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'"' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'"' ){
						++i;
					}else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
			if( 2 == nCharChars_2 ){
				++i;
			}
		}
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->DrawToHere();
		pInfo->ChangeColor(COLORIDX_TEXT); // 2002/03/13 novice
		return true;
	}
	return false;
}

bool CDraw_DoubleQuote::GetColorIndexImp(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( pInfo->pLine[pInfo->nPos] == L'"' &&
		TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
	){
		pInfo->nBgn = pInfo->nPos;
		pInfo->nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
		/* 現在の色を指定 */
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		/* ダブルクォーテーション文字列の終端があるか */
		int i;
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			CLogicInt	nCharChars_2 = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, i );
			if( 0 == nCharChars_2 ){
				nCharChars_2 = CLogicInt(1);
			}
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\\' ){
					++i;
				}else
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'"' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}else
			if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'"' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'"' ){
						++i;
					}else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
			if( 2 == nCharChars_2 ){
				++i;
			}
		}
		return true;
	}
	return false;
}

bool CDraw_DoubleQuote::GetColorIndexImpEnd(SColorInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if( 0 == pInfo->nCOMMENTEND ){
		/* ダブルクォーテーション文字列の終端があるか */
		pInfo->nCOMMENTEND = pInfo->nLineLen;
		for( int i = pInfo->nPos/* + 1*/; i <= pInfo->nLineLen - 1; ++i ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			CLogicInt	nCharChars_2 = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, i );
			if( 0 == nCharChars_2 ){
				nCharChars_2 = CLogicInt(1);
			}
			if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'\\' ){
					++i;
				}else
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'"' ){
					pInfo->nCOMMENTEND = i + 1;
					break;
				}
			}else
			if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
				if( 1 == nCharChars_2 && pInfo->pLine[i] == L'"' ){
					if( i + 1 < pInfo->nLineLen && pInfo->pLine[i + 1] == L'"' ){
						++i;
					}else{
						pInfo->nCOMMENTEND = i + 1;
						break;
					}
				}
			}
			if( 2 == nCharChars_2 ){
				++i;
			}
		}
	}
	else if( pInfo->nPos == pInfo->nCOMMENTEND ){
		pInfo->nBgn = pInfo->nPos;
		pInfo->nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		/* 現在の色を指定 */
		if( !pInfo->bSearchStringMode ){
			//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
			pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
		}
		return true;
	}
	return false;
}
