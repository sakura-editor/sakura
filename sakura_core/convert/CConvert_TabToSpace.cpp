#include "StdAfx.h"
#include "CConvert_TabToSpace.h"
#include "charset/charcode.h"
#include "CEol.h"
#include "util/string_ex2.h"

//! TAB→空白
bool CConvert_TabToSpace::DoConvert(CNativeW* pcData)
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
	CEol		cEol;
	nBgn = 0;
	nPosDes = 0;
	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
	while( NULL != ( pLine = GetNextLineW( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol ) ) ){
		if( 0 < nLineLen ){
			// 先頭行については開始桁位置を考慮する（さらに折り返し関連の対策が必要？）
			nPosX = (pcData->GetStringPtr() == pLine)? m_nStartColumn: 0;
			for( i = 0; i < nLineLen; ++i ){
				if( TAB == pLine[i]	){
					nWork = m_nTabWidth - ( nPosX % m_nTabWidth );
					nPosDes += nWork;
					nPosX += nWork;
				}else{
					nPosDes++;
					nPosX++;
					if(WCODE::IsZenkaku(pLine[i])) nPosX++;		//全角文字ずれ対応 2008.10.15 matsumo
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
	while( NULL != ( pLine = GetNextLineW( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol ) ) ){
		if( 0 < nLineLen ){
			// 先頭行については開始桁位置を考慮する（さらに折り返し関連の対策が必要？）
			nPosX = (pcData->GetStringPtr() == pLine)? m_nStartColumn: 0;
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
					if(WCODE::IsZenkaku(pLine[i])) nPosX++;		//全角文字ずれ対応 2008.10.15 matsumo
				}
			}
		}
		auto_memcpy( &pDes[nPosDes], cEol.GetValue2(), cEol.GetLen() );
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pcData->SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return true;
}

