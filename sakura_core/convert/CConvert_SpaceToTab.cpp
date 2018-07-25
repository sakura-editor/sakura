#include "StdAfx.h"
#include "CConvert_SpaceToTab.h"
#include "charset/charcode.h"
#include "CEol.h"
#include "util/string_ex2.h"

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
					if(WCODE::IsZenkaku(pLine[i])) nPosX++;		//全角文字ずれ対応 2008.10.17 matsumo
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
		auto_memcpy( &pDes[nPosDes], cEol.GetValue2(), cEol.GetLen() );
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pcData->SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return true;
}

