#include "StdAfx.h"
#include "CConvert_Trim.h"
#include "convert_util.h"
#include "CEol.h"
#include "charset/charcode.h"
#include "util/string_ex2.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! TRIM Step2
	ConvMemory から 戻ってきた後の処理．
	CMemory.cppのなかに置かないほうが良いかなと思ってこちらに置きました．
	
	@author hor
	@date 2001.12.03 hor    新規作成
	@date 2007.10.18 kobake CConvert_Trimに移動
*/
bool CConvert_Trim::DoConvert(CNativeW* pcData)
{
	const wchar_t*	pLine;
	int			nLineLen;
	wchar_t*	pDes;
	int			nBgn;
	int			i,j;
	int			nPosDes;
	CEol		cEol;
	int			nCharChars;

	nBgn = 0;
	nPosDes = 0;
	/* 変換後に必要なバイト数を調べる */
	while( NULL != ( pLine = GetNextLineW( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol ) ) ){ // 2002/2/10 aroka CMemory変更
		if( 0 < nLineLen ){
			nPosDes += nLineLen;
		}
		nPosDes += cEol.GetLen();
	}
	if( 0 >= nPosDes ){
		return true;
	}
	pDes = new wchar_t[nPosDes + 1];
	nBgn = 0;
	nPosDes = 0;
	// LTRIM
	if( m_bLeft ){
		while( NULL != ( pLine = GetNextLineW( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol ) ) ){ // 2002/2/10 aroka CMemory変更
			if( 0 < nLineLen ){
				for( i = 0; i <= nLineLen; ++i ){
					if( WCODE::IsBlank(pLine[i]) ){
						continue;
					}else{
						break;
					}
				}
				if(nLineLen-i>0){
					wmemcpy( &pDes[nPosDes], &pLine[i], nLineLen );
					nPosDes+=nLineLen-i;
				}
			}
			wmemcpy( &pDes[nPosDes], cEol.GetValue2(), cEol.GetLen() );
			nPosDes += cEol.GetLen();
		}
	}
	// RTRIM
	else{
		while( NULL != ( pLine = GetNextLineW( pcData->GetStringPtr(), pcData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol ) ) ){ // 2002/2/10 aroka CMemory変更
			if( 0 < nLineLen ){
				// 2005.10.11 ryoji 右から遡るのではなく左から探すように修正（"ａ@" の右２バイトが全角空白と判定される問題の対処）
				i = j = 0;
				while( i < nLineLen ){
					nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
					if( !WCODE::IsBlank(pLine[i]) ){
						j = i + nCharChars;
					}
					i += nCharChars;
				}
				if(j>0){
					wmemcpy( &pDes[nPosDes], &pLine[0], j );
					nPosDes+=j;
				}
			}
			wmemcpy( &pDes[nPosDes], cEol.GetValue2(), cEol.GetLen() );
			nPosDes += cEol.GetLen();
		}
	}
	pDes[nPosDes] = L'\0';

	pcData->SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return true;
}


