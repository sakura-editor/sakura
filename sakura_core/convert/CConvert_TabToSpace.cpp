#include "stdafx.h"
#include "CConvert_TabToSpace.h"
#include "charcode.h"
#include "CEOL.h"
#include "util/string_ex2.h"

//! TAB→空白
bool CConvert_TabToSpace::DoConvert(CNativeW2* pcData)
{
	using namespace WCODE;

	const wchar_t*	pLine;
	int			nLineLen;
	wchar_t*	pDes;
	int			nBgn;
	int			i;
	int			nPosDes;
	int			nPosX;
	int			nWork;
	CEOL		cEol;
	nBgn = 0;
	nPosDes = 0;
	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
	while( NULL != ( pLine = GetNextLineW2( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosX = 0;
			for( i = 0; i < nLineLen; ++i ){
				if( TAB == pLine[i]	){
					nWork = m_nTabWidth - ( nPosX % m_nTabWidth );
					nPosDes += nWork;
					nPosX += nWork;
				}else{
					nPosDes++;
					nPosX++;
				}
			}
		}
		nPosDes += cEol.GetLen();
	}
	if( 0 >= nPosDes ){
		return false;
	}
	pDes = new wchar_t[nPosDes + 1];
	nBgn = 0;
	nPosDes = 0;
	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
	while( NULL != ( pLine = GetNextLineW2( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosX = 0;
			for( i = 0; i < nLineLen; ++i ){
				if( TAB == pLine[i]	){
					nWork = m_nTabWidth - ( nPosX % m_nTabWidth );
					auto_memset( &pDes[nPosDes], L' ', nWork );
					nPosDes += nWork;
					nPosX += nWork;
				}else{
					pDes[nPosDes] = pLine[i];
					nPosDes++;
					nPosX++;
				}
			}
		}
		auto_memcpy( &pDes[nPosDes], cEol.GetUnicodeValue(), cEol.GetLen() );
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pcData->SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return true;
}

