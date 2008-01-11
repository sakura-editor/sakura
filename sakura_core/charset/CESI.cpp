/*!	@file
	@brief 文字コード調査情報保持クラス

	@author Sakura-Editor collaborators
	@date 2006/12/10 新規作成
*/
/*
	Copyright (C) 2006, rastiv

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

#include "stdafx.h"
#include "charset/CESI.h"
#include "charcode.h"

/*!
	テキストの文字コードをスキャンする．
*/
bool CESI::ScanEncoding( const char* pS, const int nLen )
{
	if( NULL == pS ){
		return false;
	}
	Charcode::GetEncdInf_SJis( pS, nLen, &m_pEI[0] );
	Charcode::GetEncdInf_EucJp( pS, nLen, &m_pEI[1] );
	Charcode::GetEncdInf_Jis( pS, nLen, &m_pEI[2] );
	Charcode::GetEncdInf_Utf8( pS, nLen, &m_pEI[3] );
	Charcode::GetEncdInf_Utf7( pS, nLen, &m_pEI[4] );
	Charcode::GetEncdInf_Uni( pS, nLen, &m_WEI );
	
	return true;
}



/*!
	Unicode を判定.
	
	@retval 正の数： Unicode らしいと判断された．
	        負の数： Unicode BE らしいと判断された．
	        ゼロ：   どちらともいえない．
	        *pEI_result： 戻り値がゼロ以外の場合に限り，詳細情報を返す．
*/
int CESI::DetectUnicode( WCCODE_INFO* pEI_result )
{
	int nCRorLF_Uni;	// ワイド文字 Unicode の改行の個数
	int nCRorLF_UniBe;	// ワイド文字 UnicodeBE の改行の個数
	int nCRorLF_mb;		// マルチバイト文字の改行の個数
	int nCRorLF_wc;		// := MAX( nCRorLF_Uni, nCRorLF_UniBe )
	int nUniType;
	int d;
	
	/* マルチバイト文字の改行（0x0d,0x0a）の個数をチェック */
	nCRorLF_mb = m_WEI.nCRorLF_ascii;
	if( nCRorLF_mb < 1 ){
		return 0;
	}
	
	/* ユニコード BOM の種類を判定 */
	// nUniType > 0 : リトルエンディアンらしい．
	// nUniType < 0 : ビッグエンディアンらしい.
	nCRorLF_Uni = m_WEI.Uni.nCRorLF;
	nCRorLF_UniBe = m_WEI.UniBe.nCRorLF;
	d = nUniType = nCRorLF_Uni - nCRorLF_UniBe;
	
	/* ワイド文字の改行（0x0d00,0x0a00 or 0x000d,0x000a）の個数を取得 */
	nCRorLF_wc = nCRorLF_Uni;
	// d := nCRorLF_Uni - nCRorLF_UniBe
	if( nUniType < 0 ){
		nCRorLF_wc += -d;
	}
	
	/*
		検証ステップ
	*/
	
	d = nCRorLF_mb - nCRorLF_wc;
	// ここで，必ず  nCRorLF_mb > 0 && nCRorLF_mb >= nCRorLF_wc.
	// だから，d > 0.
	
	if( d < nCRorLF_wc ){
		/* マルチバイト文字の改行がワイド文字の改行より少ない場合. */
		// BOM の判定結果に従う．
		if( 0 < nUniType ){
			*pEI_result = m_WEI.Uni;
		}else if( nUniType < 0 ){
			*pEI_result = m_WEI.UniBe;
		}
		return nUniType;
	}else{ // 0 <= nCRorLF_wc <= d
		/* マルチバイト文字の改行がワイド文字の改行と同数かより多い場合. */
		// BOM の判定を破棄する.
		return 0;
	}
}



/*!
	マルチバイト系文字コードを判定.
	
	@retval   判定ポイント（特有バイト数 － 不正バイト数）の最も大きいものを返す．
	        *pEI_result: 判定ポイントが最も高かった文字コードの詳細．
*/
int CESI::DetectMultibyte( MBCODE_INFO* pEI_result )
{
	MBCODE_INFO* ppEI_MostDiff[NUM_OF_MBCODE];
	MBCODE_INFO* pei_tmp;
	int i, j;
	
	/*
		「特有バイト数 － 不正バイト数」の数の大きい順にソート（バブルソート）
	*/
	for( i = 0; i < NUM_OF_MBCODE; i++ ){
		ppEI_MostDiff[i] = &m_pEI[i];
	}
	for( i = 1; i < NUM_OF_MBCODE; i++ ){
		for( j = 0; j < NUM_OF_MBCODE - i; j++ ){
			if( ppEI_MostDiff[j]->nDiff < ppEI_MostDiff[j+1]->nDiff ){
				pei_tmp = ppEI_MostDiff[j+1];
				ppEI_MostDiff[j+1] = ppEI_MostDiff[j];
				ppEI_MostDiff[j] = pei_tmp;
			}
		}
	}
	*pEI_result = *ppEI_MostDiff[0];
	return ppEI_MostDiff[0]->nDiff;
}
