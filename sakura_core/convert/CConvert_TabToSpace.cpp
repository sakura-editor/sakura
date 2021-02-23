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
					wmemset( &pDes[nPosDes], L' ', nWork );
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
		wmemcpy( &pDes[nPosDes], cEol.GetValue2(), cEol.GetLen() );
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pcData->SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return true;
}
