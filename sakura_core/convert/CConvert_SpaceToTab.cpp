﻿/*! @file */
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
#include "CConvert_SpaceToTab.h"
#include "charset/charcode.h"
#include "CEol.h"
#include "util/string_ex2.h"
#include "mem/CNativeW.h"

//! 空白→TAB変換。単独のスペースは変換しない
bool CConvert_SpaceToTab::DoConvert(CNativeW* pcData)
{
	using namespace WCODE;

	const wchar_t*	pLine;
	int			nLineLen;
	wchar_t*	pDes;
	int			nBgn;
	int			i;
	int			nPosDes;
	int			nPosX;
	CEol		cEol;

	BOOL		bSpace = FALSE;	//スペースの処理中かどうか
	int		j;
	int		nStartPos;

	nBgn = 0;
	nPosDes = 0;
	/* 変換後に必要なバイト数を調べる */
	while( NULL != ( pLine = GetNextLineW( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol ) ) ){
		if( 0 < nLineLen ){
			nPosDes += nLineLen;
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
			nPosX = (pcData->GetStringPtr() == pLine)? m_nStartColumn: 0;	// 処理中のiに対応する表示桁位置
			bSpace = FALSE;	//直前がスペースか
			nStartPos = 0;	// スペースの先頭
			for( i = 0; i < nLineLen; ++i ){
				if( SPACE == pLine[i] || TAB == pLine[i] ){
					if( bSpace == FALSE ){
						nStartPos = nPosX;
					}
					bSpace = TRUE;
					if( SPACE == pLine[i] ){
						nPosX++;
					}else if( TAB == pLine[i] ){
						nPosX += m_nTabWidth - (nPosX % m_nTabWidth);
					}
				}else{
					if( bSpace ){
						if( (1 == nPosX - nStartPos) && (SPACE == pLine[i - 1]) ){
							pDes[nPosDes] = SPACE;
							nPosDes++;
						} else{
							for( j = nStartPos / m_nTabWidth; j < (nPosX / m_nTabWidth); j++ ){
								pDes[nPosDes] = TAB;
								nPosDes++;
								nStartPos += m_nTabWidth - ( nStartPos % m_nTabWidth );
							}
							//	2003.08.05 Moca
							//	変換後にTABが1つも入らない場合にスペースを詰めすぎて
							//	バッファをはみ出すのを修正
							for( j = nStartPos; j < nPosX; j++ ){
								pDes[nPosDes] = SPACE;
								nPosDes++;
							}
						}
					}
					nPosX++;
					if(WCODE::IsZenkaku(pLine[i], m_cCache)) nPosX++;	//全角文字ずれ対応 2008.10.17 matsumo
					pDes[nPosDes] = pLine[i];
					nPosDes++;
					bSpace = FALSE;
				}
			}
			//for( ; i < nLineLen; ++i ){
			//	pDes[nPosDes] = pLine[i];
			//	nPosDes++;
			//}
			if( bSpace ){
				if( (1 == nPosX - nStartPos) && (SPACE == pLine[i - 1]) ){
					pDes[nPosDes] = SPACE;
					nPosDes++;
				} else{
					//for( j = nStartPos - 1; (j + m_nTabWidth) <= nPosX + 1; j+=m_nTabWidth ){
					for( j = nStartPos / m_nTabWidth; j < (nPosX / m_nTabWidth); j++ ){
						pDes[nPosDes] = TAB;
						nPosDes++;
						nStartPos += m_nTabWidth - ( nStartPos % m_nTabWidth );
					}
					//	2003.08.05 Moca
					//	変換後にTABが1つも入らない場合にスペースを詰めすぎて
					//	バッファをはみ出すのを修正
					for( j = nStartPos; j < nPosX; j++ ){
						pDes[nPosDes] = SPACE;
						nPosDes++;
					}
				}
			}
		}

		/* 行末の処理 */
		wmemcpy( &pDes[nPosDes], cEol.GetValue2(), cEol.GetLen() );
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pcData->SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return true;
}
