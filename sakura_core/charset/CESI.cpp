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
#include "charset/charcode.h"
#include "CShiftJis.h"
#include "CEuc.h"
#include "CUnicode.h"
#include "CUtf7.h"
#include "CBase64.h"
#include "CJis.h"

/*!
	テキストの文字コードをスキャンする．
*/
bool CESI::ScanEncoding( const char* pS, const int nLen )
{
	if( NULL == pS ){
		return false;
	}
	CESI::_GetEncdInf_SJis( pS, nLen, &m_pEI[0] );
	CESI::_GetEncdInf_EucJp( pS, nLen, &m_pEI[1] );
	CESI::_GetEncdInf_Jis( pS, nLen, &m_pEI[2] );
	CESI::_GetEncdInf_Utf8( pS, nLen, &m_pEI[3] );
	CESI::_GetEncdInf_Utf7( pS, nLen, &m_pEI[4] );
	CESI::_GetEncdInf_Uni( pS, nLen, &m_WEI );
	
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


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         デバッグ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 2006.12.17 rastiv CDlgProperty::SetData内コード
// 2008.04.20 kobake CESIに移動
CNativeT CESI::GetDebugInfo(const char* pBuf, int nBufLen)
{
	CNativeT		cmemProp;
	MBCODE_INFO		mbci_tmp;
	UNICODE_INFO	uci_tmp;
	TCHAR			szWork[500];

	/*
	||ファイルの日本語コードセット判別: Unicodeか？
	*/
	/*
	||ファイルの日本語コードセット判別: UnicodeBEか？
	*/
	CESI::_GetEncdInf_Uni( pBuf, nBufLen, &uci_tmp );
	auto_sprintf( szWork, _T("Unicodeコード調査：改行バイト数=%d  BE改行バイト数=%d ASCII改行バイト数=%d\r\n")
		, uci_tmp.Uni.nCRorLF, uci_tmp.UniBe.nCRorLF, uci_tmp.nCRorLF_ascii );
	cmemProp.AppendString( szWork );
	
	/*
	||ファイルの日本語コードセット判別: EUCか？
	*/
	CESI::_GetEncdInf_EucJp( pBuf, nBufLen, &mbci_tmp );
	auto_sprintf( szWork, _T("EUCJPコード検査：特有バイト数=%d  ポイント数=%d\r\n")
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );
	
	/*
	||ファイルの日本語コードセット判別: SJISか？
	*/
	CESI::_GetEncdInf_SJis( pBuf, nBufLen, &mbci_tmp );
	auto_sprintf( szWork, _T("SJISコード検査：特有バイト数=%d  ポイント数=%d\r\n")
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );
	
	/*
	||ファイルの日本語コードセット判別: JISか？
	*/
	CESI::_GetEncdInf_Jis( pBuf, nBufLen, &mbci_tmp );
	auto_sprintf( szWork, _T("JISコード検査：特有バイト数=%d  ポイント数=%d\r\n")
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );

	/*
	||ファイルの日本語コードセット判別: UTF-8Sか？
	*/
	CESI::_GetEncdInf_Utf8( pBuf, nBufLen, &mbci_tmp );
	auto_sprintf( szWork, _T("UTF-8コード検査：特有バイト数=%d  ポイント数=%d\r\n")
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );

	/*
	||ファイルの日本語コードセット判別: UTF-7Sか？
	*/
	CESI::_GetEncdInf_Utf7( pBuf, nBufLen, &mbci_tmp );
	auto_sprintf( szWork, _T("UTF-7コード検査：特有バイト数=%d  ポイント数=%d\r\n")
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );

	return cmemProp;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    共有実装ヘルパ関数                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
int CESI::_GuessCharLen_utf8(const uchar_t* pC, const int nlen )
{
	uchar_t uc;
	
	if( nlen < 1 ){
		return 0;
	}
	uc = *pC;
	if( 1 < nlen && ( uc & 0xe0 ) == 0xc0 ){
		return 2;
	}
	if( 2 < nlen && ( uc & 0xf0 ) == 0xe0 ){
		return 3;
	}
	if( 3 < nlen && ( uc & 0xf8 ) == 0xf0 ){
		return 4;
	}
	return 1;
}
int CESI::_GuessCharLen_sjis( const uchar_t* pC, const int nLen )
{
	if( nLen < 1 ){
		return 0;
	}
	if( 1 < nLen && CShiftJis::IsSJisKan1(*pC) ){
		return 2;
	}
	return 1;
}
int CESI::_GuessCharLen_eucjp( const uchar_t*pC, const int nlen )
{
	uchar_t uc;

	if( nlen < 1 ){
		return 0;
	}
	uc = *pC;
	if( 2 < nlen && uc == 0x8f ){
		return 3;
	}
	if( 1 < nlen && (uc == 0x8e || CEuc::IsEucKan1(uc)) ){
		return 2;
	}
	return 1;
}


/*
	SJIS のこと.

	第１バイト |  1000 0001(0x81)         |   1110 0000(0xE0)         |   1010 0001(0xA1)       
	           | ～ 1001 1111(0x9F)       |  ～ 1110 1111(0xEF)       |  ～ 1101 1111(0xDF)     
	           | → SJIS 全角漢字かなカナ |  → SJIS 全角漢字カナかな |  → 半角カナ            
	-----------+--------------------------+---------------------------+-------------------------
	第２バイト |        0100 0000(0x40)  ～  1111 1100(0xFC)          |      ----               
	           |         ただし 0111 1111(0x7F) は除く.               |                         

	参考：「■G-PROJECT■ -日本語文字コードの判別」http://www.gprj.net/dev/tips/other/kanji.shtml
	      「ミケネコの文字コードの部屋」http://mikeneko.creator.club.ne.jp/~lab/kcode/index.html
*/

/*!
	SJIS の文字か
*/
int CESI::_CheckSJisChar( const uchar_t* pS, const int nLen )
{
	uchar_t uc;
	
	if( 0 < nLen ){
		uc = *pS;
		if( (uc & 0x80) == 0 || CShiftJis::IsSJisHanKata( uc ) ){
			// ASCII またはローマ字(JIS X 0201 Roman)
			// 半角カナ(JIS X 0201 Kana)
			return 1;
		}
		if( 1 < nLen && CShiftJis::IsSJisKan(pS) ){
			// SJIS 漢字・全角カナかな  (JIS X 0208)
			return 2;
		}
		return -1;
	}
	return 0;
}
int CESI::_CheckSJisCharR( const uchar_t* pS, const int nLen )
{
	uchar_t uc;
	
	if( 0 < nLen ){
		uc = pS[-1];
		if( uc < 0x40 || 0x7f == uc ){
			// ASCII またはローマ字(JIS X 0201 Roman.)
			return 1;
		}
		if( 1 < nLen && CShiftJis::IsSJisKan1(pS[-2]) ){
			// SJIS 漢字・全角カナかな  (JIS X 0208)
			return 2;
		}
		if( CShiftJis::IsSJisHanKata(uc) ){
			// 半角カナ(JIS X 0201 Kana)
			return 1;
		}
		return -1;
	}
	return 0;
}

/*
	EUC-JP のこと.

	第1バイト |   1000 1110(0x8E)   |  1000 1111(0x8F)    |  1010 0001(0xA1) ～ 1111 1110(0xFE) 
	          |   → 半角カナ       |  → 補助漢字        |  → 漢字かなカナ                    
	----------+---------------------+---------------------+-------------------------------------
	第2バイト |  1010 0001(0xA1)    |   1010 0001(0xA1)   |      1010 0001(0xA1)                
	          | ～ 1101 1111(0xDF)  |  ～ 1111 1110(0xFE) |     ～ 1111 1110(0xFE)              
	----------+---------------------+---------------------+-------------------------------------
	第3バイト |        ----         |   1010 0001(0xA1)   |        ----                         
	          |                     |  ～ 1111 1110(0xFE) |                                     

	参考：「■G-PROJECT■ -日本語文字コードの判別」http://www.gprj.net/dev/tips/other/kanji.shtml
	      「ミケネコの文字コードの部屋」http://mikeneko.creator.club.ne.jp/~lab/kcode/index.html
*/

/*!
	EUC-JP の文字か
	
	fix: 2006.09.23 genta  EUCJP 半角カタカナ判別が間違っていたのを修正．
*/
int CESI::_CheckEucJpChar( const uchar_t* pS, const int nLen )
{
	uchar_t uc;
	
	if( 0 < nLen ){
		uc = *pS;
		if( (uc & 0x80) == 0 ){
			// ASCII またはローマ字です.  (JIS X 0201 Roman.)
			return 1;
		}
		if( 1 < nLen ){
			if( CEuc::IsEucKan( pS ) ){
				// EUC-JP 漢字・かなカナ です.  (JIS X 0208.)
				return 2;
			}
		//-	if( uc == 0x8e && CEuc::IsEucKan( pS ) ){
		//-		// 半角カナです.  (JIS X 0201 Kana.)
		//-		return 2;
		//-	}
			if( uc == 0x8e && CEuc::IsEucHanKata2( pS[1] ) ){
				// 半角カナです.  (JIS X 0201 Kana.)
				return 2;
			}
			if( 2 < nLen ){
				if( uc == 0x8f && CEuc::IsEucKan( pS+1 ) ){
					// EUC-JP 補助漢字です.  (JIS X 0212.)
					return 3;
				}
			}
		}
		return -1;
	}
	return 0;
}


/* -------------------------------------------------------------------------------------------------------------- *
UTF-8のコード
ビット列		内容
0xxx xxxx	1バイトコードの先頭
110x xxxx	2バイトコードの先頭
1110 xxxx	3バイトコードの先頭
1111 0xxx	4バイトコードの先頭
10xx xxxx	UTF-8 バイトコードの 2 バイト目以降

UTF-8のエンコーディング

ビット列                  MSB -         UCS ビット列         - LSB     第1バイト  第2バイト  第3バイト  第4バイト
\u0～\u7F         (UCS2)  0000 0000 0000 0000  0000 0000 0aaa bbbb  -> 0aaa bbbb     ---        ---        ---   
\u80～\u7FF       (UCS2)  0000 0000 0000 0000  0000 0aaa bbbb cccc  -> 110a aabb  10bb cccc     ---        ---   
\u800～\uFFFF     (UCS2)  0000 0000 0000 0000  aaaa bbbb cccc dddd  -> 1110 aaaa  10bb bbcc  10cc dddd     ---   
\u10000～\u1FFFFF (UCS4)  0000 0000 000a bbbb  cccc dddd eeee ffff  -> 1111 0abb  10bb cccc  10dd ddee  10ee ffff

参考資料：「UCSとUTF」http://homepage1.nifty.com/nomenclator/unicode/ucs_utf.htm
* --------------------------------------------------------------------------------------------------------------- */

/*!
	UTF-8 の文字か
	
	@retval 正の数 : 正常な UTF-8 バイト列.
	@retval 負の数 : 不正な UTF-8 バイト列. (往復変換不可とか範囲不正とか)
	@retval 0      : 検査データがなくなった.
*/
int CESI::_CheckUtf8Char( const uchar_t* pC, const int nLen )
{
	uchar_t c0, c1, c2, c3, ctemp;
	
	if( 0 < nLen ){
		c0 = *pC;
		if( (c0 & 0x80) == 0 ){
			return 1;
		}
		if( 1 < nLen ){
			c1 = pC[1];
			if( (c0 & 0xe0) == 0xc0 ){
				if( (c1 & 0xc0) == 0x80 ){
					if( (c0 & 0x1e) == 0 ){
						// デコードできません.(往復変換不可領域)
						return -2;
					}
					return 2;
				}
			}
			if( 2 < nLen ){
				c2 = pC[2];
				if( (c0 & 0xf0) == 0xe0 ){
				//	if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 ){
					if( ((c1 & 0xc0) & (c2 & 0xc0)) == 0x80 ){
					//	if( (c0 & 0x0f) == 0 && (c1 & 0x20) == 0 ){
						if( ((c0 & 0x0f) | (c1 & 0x20)) == 0 ){
							// デコードできません.(往復変換不可領域)
							return -3;
						}
					//	if( (c0 & 0x0f) == 0x0d && (c1 & 0x20) != 0 ){
						if( (((c0 & 0x0f) ^ 0x0d) | (c1 & 0x20)) == 0x20 ){
							// U+D800 から U+DFFF のサロゲート領域は不正です.
							return -3;
						}
						return 3;
					}
				}
				if( 3 < nLen ){
					c3 = pC[3];
					if( (c0 & 0xf8) == 0xf0 ){
					//	if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 && (c3 & 0xc0) == 0x80 ){
						if( ((c1 & 0xc0) & (c2 & 0xc0) & (c3 & 0xc0)) == 0x80 ){
						//	if( (c0 & 0x07) == 0 && (c1 & 0x30) == 0 ){
							if( ((c0 & 0x07) | (c1 & 0x30)) == 0 ){
								// デコードできません.(往復変換不可領域)
								return -4;
							}
						//	if( (c0 & 0x04) != 0 && ((c0 & 0x03) != 0 || (c1 & 0x30) != 0) ){
							ctemp = static_cast<uchar_t>(c0 & 0x04);
							if( (ctemp | (c0 & 0x03)) | (ctemp | (c1 & 0x30)) ){
								// 値が大きすぎます.
								return -4;
							}
							return 4;
						}
					}
				} // 3 < nLen
			} // 2 < nLen
		} // 1 < nLen
		return -1;
	} // 0 < nLen
	return 0;
}

/*
	UTF-16 サロゲート処理のこと.
	
	U+10000 から U+10FFFF の文字値 a0 に対しては,

		a0 = HHHHHHHHHHLLLLLLLLLL  U+10000 ～ U+10FFFF
		w1 = 110110HH HHHHHHHH     上位サロゲート：U+D800 ～ U+DBFF
		w2 = 110111LL LLLLLLLL     下位サロゲート：U+DC00 ～ U+DFFF

	1. 0x10000 を引き, 20ビットの文字値 a1 (0x00000 ～ 0xFFFFF) で表現した後,
	     a1 ← a0 - 0x10000
	2. 上位 10ビットを w1, 下位 10ビットを w2 に分け,
	     w1 ← (a1 & 0xFFC0) >> 6
	     w2 ←  a1 & 0x03FF
	3. w1, w2 の上位 6ビット分の空き領域を, それぞれ 110110 と 110111 で埋める.
	     w1 ← w1 | 0xD800
	     w2 ← w2 | 0xDC00
	
	
	U+FFFE, U+FFFF は, 未定義値.
	
	参考資料：「UCSとUTF」http://homepage1.nifty.com/nomenclator/unicode/ucs_utf.htm
*/

/*!
	UTF-16 の文字か
    主にサロゲートペアのチェックをする.
    
    @note 返却値は ２の倍数であること．
*/
int CESI::_imp_CheckUtf16Char( const uchar_t* pC, const int nLen, bool bBigEndian )
{
	const uchar16_t* pwC = reinterpret_cast<const uchar16_t*>(pC);
	uchar16_t wc1, wc2, tmp;
	
	if( 1 < nLen ){
		wc1 = pwC[0];
		if( bBigEndian ){
			tmp = static_cast<uchar16_t>(wc1 >> 8);
			wc1 <<= 8;
			wc1 |= tmp;
		}
		if( (wc1 & 0xfffe) == 0xfffe || CUnicode::IsUtf16SurrogLow(wc1) ){
			/* 未定義文字，または下位サロゲート． */
			// 0xffff とか 0xfffe は将来的に内部で使用するかもしれないのでカット．
			return -2;
		}
		if( !CUnicode::IsUtf16SurrogHi(wc1) ){
			return 2; // 通常の文字．
		}
		if( 3 < nLen ){
			wc2 = pwC[1];
			if( bBigEndian ){
				tmp = static_cast<uchar16_t>(wc2 >> 8);
				wc2 <<= 8;
				wc2 |= tmp;
			}
			if( CUnicode::IsUtf16SurrogLow(wc2) ){
				return 4;  // サロゲートペア．
			}
		}
		return -2;  // サロゲート片 → 不正．
	}
	return 0;
}
int CESI::_CheckUtf16Char( const uchar_t* pC, const int nLen )
{
	return _imp_CheckUtf16Char( pC, nLen, false );
}
int CESI::_CheckUtf16BeChar( const uchar_t* pC, const int nLen )
{
	return _imp_CheckUtf16Char( pC, nLen, true );
}

/*
	UTF-7 のこと.
	
	UTF-7 セットD：　角英数字、'(),-./:?、および、TAB SP CR LF
	UTF-7 セットO：　!"#$%&*;<=>@[]^_`{|}
	UTF-7 セットB：　パッド文字を除く BASE64 文字
	
	1. セットD または セットO にあるユニコード文字は, 等価な ASCII 文字で表現される.
	2. セットD または セットO にないユニコード文字は, BASE64 符号化され, セットB 文字列によって表現される.
	3. セットB の開始は, ASCII 文字 '+' によって, セットB 文字列の終端は, セットB にない文字の出現によって認識される.
	   セットB 文字列の終端部分には, セットB にない ASCII 文字 '-' を入れてもよいことになっていて,
	   その終端文字は見つかり次第削除される.
	   セットB 開始文字である ASCII 文字 '+' 自体は, "+-" という文字列で表される.
	
	参考資料：「UCSとUTF」http://homepage1.nifty.com/nomenclator/unicode/ucs_utf.htm
	          「RFC 2152」http://www.ietf.org/rfc/rfc2152.txt
*/

/*!
	UTF-7 セットＤの文字列か
	
	@param[out] ref_pNext : 最後に読み込んだ次の文字へのポインタを返す．
	@retval 不正バイト数を返す．
*/
int CESI::_CheckUtf7SetDPart( const uchar_t* pS, const int nLen, uchar_t*& ref_pNext )
{
	uchar_t* ptr;
	uchar_t* base_ptr;
	uchar_t* end_ptr;
	int nlostbytes = 0;
	
	base_ptr = const_cast<uchar_t*>(pS);
	end_ptr = base_ptr + nLen;
	for( ptr = base_ptr; ptr < end_ptr; ++ptr ){
		if( CUtf7::IsUtf7SetDChar( *ptr ) ){
			continue;
		}
		if( *ptr == '+' ){
			// UTF-7 Set B 部分の開始記号を発見.
			break;
		}
		nlostbytes++;
	}
	
	ref_pNext = ptr;
	return nlostbytes;
}

/*!
	UTF-7 セットＢの文字列か
	
	@param[out] ref_pNext : 最後に読み込んだ次の文字へのポインタを返す．
	@retval 不正バイト数を返す．
	@note この関数を実行する前に必ず _CheckUtf7SetDPart() を実行すること.
*/
int CESI::_CheckUtf7SetBPart( const uchar_t* pS, const int nLen, uchar_t*& ref_pNext )
{
	uchar_t* ptr;
	uchar_t* base_ptr;
	uchar_t* end_ptr;
	int nlostbytes = 0;
	int nrem;
	
	base_ptr = const_cast<uchar_t*>(pS);
	end_ptr = base_ptr + nLen;
	for( ptr = base_ptr; ptr < end_ptr; ++ptr ){
		if( !CBase64::IsBase64Char( *ptr ) ){
			// UTF-7 Set D 部分の開始を認識.
			break;
		}
	}
	
	/*
	◆ 検証ステップ
	
	デコード後のデータ長をチェックする.
	調査してきたデータ長 (ptr-base_ptr) を８で割ってみる.
	その余りの値から考えられるビット列は…
	
	             |----------------------------- Base64 表現 --------------------------------------------|
	             第1バイト  第2バイト  第3バイト  第4バイト  第5バイト  第6バイト  第7バイト  第8バイト 
	残り１文字   00xx xxxx  00xx xxxx  00xx xx00     ---        ---        ---        ---        ---    
	残り２文字   00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx 0000     ---        ---    
	残り３文字   00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx 
	
	上記３通りのいづれにも当てはまらない場合は全データを落とす（不正バイトとする）.
	*/
	
	nrem = (ptr-base_ptr) % 8;
	switch ( nrem ){
	case 3:
		if( CBase64::Base64_CharToVal( ptr[-1] ) & 0x03 ){
			nlostbytes = ptr - base_ptr;
		}
		break;
	case 6:
		if( CBase64::Base64_CharToVal( ptr[-1] ) & 0x0f ){
			nlostbytes = ptr - base_ptr;
		}
		break;
	case 0:
		break;
	default:
		nlostbytes = ptr - base_ptr;
	}
	
	ref_pNext = ptr;
	return nlostbytes;
}



/*!
	JIS の エスケープ文字列を検出する
	
	@param [in]  pS			検出対象データ
	@param [in]  nLen		検出対象データ長
	@param [out] pnEscType	検出されたエスケープ文字列の種類
	
	@retval
		検出された場合は，検出されたエスケープ文字列長
		検出されなかった場合は， -1
		検出データがない場合は， 0
	
	@note
		戻り値がゼロより大きい場合に限り，*pnEscType が更新される．
		pnEscType は NULL でも良い．
	
	
	符号化文字集合       16進表現            文字列表現
	------------------------------------------------------------
	JIS C 6226-1978      1b 24 40            ESC $ @
	JIS X 0208-1983      1b 24 42            ESC $ B
	JIS X 0208-1990      1b 26 40 1b 24 42   ESC & @ ESC $ B
	JIS X 0212-1990      1b 24 28 44         ESC $ ( D
	JIS X 0213:2000 1面  1b 24 28 4f         ESC $ ( O
	JIS X 0213:2004 1面  1b 24 28 51         ESC $ ( Q
	JIS X 0213:2000 2面  1b 24 28 50         ESC $ ( P
	JIS X 0201 ラテン    1b 28 4a            ESC ( J
	JIS X 0201 ラテン    1b 28 48            ESC ( H         歴史的 [*]
	JIS X 0201 片仮名    1b 28 49            ESC ( I
	ISO/IEC 646 IRV      1b 28 42            ESC ( B
	
	  [*] 歴史的な理由により出現したエスケープシーケンス．
	      JIS X 0201の指示としては使用すべきでない．
	
	出展：http://www.asahi-net.or.jp/~wq6k-yn/code/
	参考：http://homepage2.nifty.com/zaco/code/

*/
int CESI::_DetectJisESCSeq( const uchar_t* pS, const int nLen, EJisESCSeqType* pnEscType )
{
	const uchar_t* end_ptr = pS + nLen;
	uchar_t* p;
	int expected_esc_len;
	EJisESCSeqType nEscType;
	
	if( nLen < 1 ){
		return 0;
	}
	
	nEscType = JISESC_UNKNOWN;
	expected_esc_len = 0;
	
	if( *pS == ACODE::ESC ){
		expected_esc_len++;
		p = const_cast<uchar_t *>(pS)+1;
		if( p+2 <= end_ptr ){
			expected_esc_len += 2;
			if( *p == '(' ){
				if( p[1] == 'B' ){
					nEscType = JISESC_ASCII;			// ESC ( B  -  ASCII
				}else if( p[1] == 'J'){
					nEscType = JISESC_JISX0201Latin;	// ESC ( J  -  JIS X 0201 ラテン
				}else if( p[1] == 'H'){
					nEscType = JISESC_JISX0201Latin_OLD;// ESC ( H  -  JIS X 0201 ラテン
				}else if( p[1] == 'I' ){
					nEscType = JISESC_JISX0201Katakana;	// ESC ( I  -  JIS X 0201 片仮名
				}
			}else if( *p == '$' ){
				if( p[1] == 'B' ){
					nEscType = JISESC_JISX0208_1983;	// ESC $ B  -  JIS X 0208-1983
				}else if( p[1] == '@' ){
					nEscType = JISESC_JISX0208_1978;	// ESC $ @  -  JIS X 0208-1978  (旧JIS)
				}
			}
		}else if( p+5 <= end_ptr ){
			expected_esc_len += 5;
			if( 0 == memcmp( p, &CJis::JISESCDATA_JISX0208_1990[1], 5 ) ){
				nEscType = JISESC_JISX0208_1990;		// ESC & @ ESC $ B  -  JIS X 0208-1990
			}
		}
	}
	
	if( 0 < expected_esc_len ){
		if( pnEscType ){
			*pnEscType = nEscType;
		}
		if( JISESC_UNKNOWN != nEscType ){
			return expected_esc_len;
		}else{
			return 1;
		}
	}else{
		return -1;
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      文字列チェック                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*!
	2007.08.14 kobake 戻り値をECodeTypeに変更

	文字列の先頭にUnicode系BOMが付いているか？
	
	@retval	CODE_NONE		なし,未検出
	@retval	CODE_UNICODE	Unicode
	@retval	CODE_UTF8		UTF-8
	@retval	CODE_UNICODEBE	UnicodeBE
*/
ECodeType CESI::DetectUnicodeBom( const char* pS, int nLen )
{
	const uchar_t* pBuf = reinterpret_cast<const uchar_t *>(pS);

	if( NULL == pS ){
		return CODE_NONE;
	}
	if( 2 <= nLen ){
		if( pBuf[0] == 0xff && pBuf[1] == 0xfe ){
			return CODE_UNICODE;
		}
		if( pBuf[0] == 0xfe && pBuf[1] == 0xff ){
			return CODE_UNICODEBE;
		}
		if( 3 <= nLen ){
			if( pBuf[0] == 0xef && pBuf[1] == 0xbb && pBuf[2] == 0xbf ){
				return CODE_UTF8;
			}
		}
	}
	return CODE_NONE;
}



/*!
	SJIS の文字コード判定情報を取得する
*/
void CESI::_GetEncdInf_SJis( const char* pS, const int nLen, MBCODE_INFO* pEI )
{
	uchar_t* ptr;
	uchar_t* end_ptr;
	int nlostbytes;
	int num_of_sjis_encoded_bytes;
	int nret;
	
	nlostbytes = 0;
	num_of_sjis_encoded_bytes = 0;
	ptr = (uchar_t *)pS;
	end_ptr = ptr + nLen;
	
	while( 0 != (nret = _CheckSJisChar( ptr, end_ptr-ptr )) ){
		if( 0 < nret ){
			ptr += nret;
			if( 1 < nret ){
				num_of_sjis_encoded_bytes += nret;
			}
		}else /* ret < 0 */{
			if( end_ptr-ptr < _GuessCharLen_sjis( ptr ) ){
				// データが残り少なくなりました.
				break;
			}
			ptr++;
			nlostbytes++;
		}
	}
	pEI->eCodeID = CODE_SJIS;
	pEI->nSpecBytes = num_of_sjis_encoded_bytes;
	pEI->nDiff = num_of_sjis_encoded_bytes - nlostbytes;
	
	return;
}

/*!
	JIS の文字コード判定情報を取得する
*/
void CESI::_GetEncdInf_Jis( const char* pS, const int nLen, MBCODE_INFO* pEI )
{
	uchar_t* ptr = (uchar_t *)pS;
	uchar_t* end_ptr = ptr + nLen;
	int nescbytes = 0;
	int nlostbytes = 0;
	EJisESCSeqType nEscType = JISESC_ASCII;
	
	int nret;
	for( ; 0 != (nret = _DetectJisESCSeq(ptr, end_ptr - ptr, &nEscType)); ptr += nret ){
		if( nret < 0 ){
			nret = 1;
		}else{
			nescbytes += nret;
		}
		if( *ptr & 0x80 ){
			nlostbytes++;
		}
	}
	pEI->eCodeID = CODE_JIS;
	pEI->nSpecBytes = nescbytes;
	pEI->nDiff = nescbytes - nlostbytes;
}


/*!
	EUC-JP の文字コード判定情報を取得する
*/
void CESI::_GetEncdInf_EucJp( const char* pS, const int nLen, MBCODE_INFO* pEI )
{
	uchar_t* ptr;
	uchar_t* end_ptr;
	int nlostbytes;
	int num_of_eucjp_encoded_bytes;
	int nret;
	
	nlostbytes = 0;
	num_of_eucjp_encoded_bytes = 0;
	ptr = (uchar_t *)pS;
	end_ptr = ptr + nLen;
	while( 0 != (nret = _CheckEucJpChar( ptr, end_ptr-ptr )) ){
		if( 0 < nret ){
			ptr += nret;
			if( 1 < nret ){
				num_of_eucjp_encoded_bytes += nret;
			}
		}else /* ret < 0 */{
			if( end_ptr-ptr < _GuessCharLen_eucjp( ptr ) ){
				// 残りデータが少なくなりました...
				break;
			}
			ptr++;
			nlostbytes++;
		}
	}
	pEI->eCodeID = CODE_EUC;
	pEI->nSpecBytes = num_of_eucjp_encoded_bytes;
	pEI->nDiff = num_of_eucjp_encoded_bytes - nlostbytes;
	
	return;
}


/*!
	UTF-8 の文字コード判定情報を取得する
*/
void CESI::_GetEncdInf_Utf8( const char* pS, const int nLen, MBCODE_INFO* pEI )
{
	uchar_t* ptr;
	uchar_t* end_ptr;
	int nlostbytes;
	int num_of_utf8_encoded_bytes;
	int nret;
	
	nlostbytes = 0;
	num_of_utf8_encoded_bytes = 0;
	ptr = (uchar_t *)pS;
	end_ptr = ptr + nLen;
	
	while( 0 != (nret = _CheckUtf8Char( ptr, end_ptr-ptr )) ){
		if( 0 < nret ){
			ptr += nret;
			if( 1 < nret ){
				num_of_utf8_encoded_bytes += nret;
			}
		}else{
			if( end_ptr - ptr < _GuessCharLen_utf8( ptr ) ){
				// データが少なくなってきました．
				break;
			}
			ptr += -nret;
			nlostbytes += -nret;
		}
	}
	pEI->eCodeID = CODE_UTF8;
	pEI->nSpecBytes = num_of_utf8_encoded_bytes;
	pEI->nDiff = num_of_utf8_encoded_bytes - nlostbytes;
	
	return;
}

/*!
	UTF-7 の文字コード判定情報を取得する
*/
void CESI::_GetEncdInf_Utf7( const char* pS, const int nLen, MBCODE_INFO* pEI )
{
	uchar_t* ptr;
	uchar_t* base_ptr;
	uchar_t* end_ptr;
	int nlostbytes;
	int num_of_base64_encoded_bytes;
	bool bSetBPart;
	int nret;
	
	nlostbytes = 0;
	num_of_base64_encoded_bytes = 0;
	bSetBPart = false;
	base_ptr = (uchar_t *)pS;
	ptr = base_ptr;
	end_ptr = base_ptr + nLen;
	
	while( 1 ){
		nret = _CheckUtf7SetDPart( base_ptr, end_ptr-base_ptr, ptr );
		nlostbytes += nret;
		
		ptr++;  // '+' をスキップ．
		base_ptr = ptr;
		
		nret = _CheckUtf7SetBPart( base_ptr, end_ptr-base_ptr, ptr );
		// ここで， [特有バイト数] := [実際に調査されたデータ長] - [不正バイト数]
		num_of_base64_encoded_bytes += (ptr-base_ptr) - nret;
		
		// ptr == end_ptr が成り立つ場合, _CheckUtf7SetBPart() 関数の特性により，
		// 単純に不正バイト数 := ptr-base_ptr（読み込んだバイト数）となる恐れがあるので，
		// 条件 ptr == end_ptr が成り立つときはループ脱出．
		if( end_ptr <= ptr ){
			break;
		}
		nlostbytes += nret;
		
		if( *ptr == '-' ){
			ptr++;
			if( end_ptr <= ptr ){
				break;
			}
		}
		base_ptr = ptr;
	}
	pEI->eCodeID = CODE_UTF7;
	pEI->nSpecBytes = num_of_base64_encoded_bytes;
	pEI->nDiff = num_of_base64_encoded_bytes - nlostbytes;
	
	return;
}


/*!
	UNICODE の文字コード判定情報を取得する
*/
void CESI::_GetEncdInf_Uni( const char* pS, const int nLen, UNICODE_INFO* pWEI )
{
	uchar_t* ptr;
	uchar_t* end_ptr;
	int nlostbytes;
	int nCRorLF_uni;	// Unicode 版 CR, LF のバイト数
	int nCRorLF_ascii;	// ASCII 版 CR, LF のバイト数
	int nret;
	uchar16_t wc, wc_tmp1, wc_tmp2;
	
	nlostbytes = 0;
	nCRorLF_ascii = 0;
	nCRorLF_uni = 0;
	ptr = (uchar_t *)pS;
	end_ptr = ptr + nLen;
	
	while( 0 != (nret = _CheckUtf16Char(ptr, end_ptr-ptr)) ){
		if( 0 < nret ){
			if( nret == 2 ){
				//
				// ワイド文字(Unicode)の改行とマルチバイト文字の改行をカウント
				//
				wc = *reinterpret_cast<uchar16_t *>(ptr);
				wc_tmp1 = static_cast<uchar16_t>(0x00ff & wc);
				wc_tmp2 = static_cast<uchar16_t>(0xff00 & wc);
				if( 0x000a == wc_tmp1 ){
					nCRorLF_ascii++;
					if( 0x0000 == wc_tmp2 ){
						nCRorLF_uni++;
					}else if( 0x0a00 == wc_tmp2 || 0x0d00 == wc_tmp2 ){
						nCRorLF_ascii++;
					}
				}else if( 0x000d == wc_tmp1 ){
					nCRorLF_ascii++;
					if( 0x0000 == wc_tmp2 ){
						nCRorLF_uni++;
					}else if( 0x0a00 == wc_tmp2 || 0x0d00 == wc_tmp2 ){
						nCRorLF_ascii++;
					}
				}else{
					if( 0x0a00 == wc_tmp2 || 0x0d00 == wc_tmp2 ){
						nCRorLF_ascii++;
					}
				}
			}/*else{ // nret == 4
			}*/
			ptr += nret;
		}else if( nret == -2 ){
			// サロゲートペアが片一方しかない, または値が未定義.
			ptr += 2;
			nlostbytes += 2;
		}else{
			break;
		}
	}
	pWEI->Uni.eCodeID = CODE_UNICODE;
	pWEI->Uni.nCRorLF = nCRorLF_uni;
	pWEI->Uni.nLostBytes = nlostbytes;
	pWEI->nCRorLF_ascii = nCRorLF_ascii;
	
	nlostbytes = 0;
	nCRorLF_uni = 0;
	ptr = (uchar_t *)pS;
	//end_ptr = ptr + nLen;
	
	while( 0 != (nret = _CheckUtf16BeChar(ptr, end_ptr-ptr)) ){
		if( 0 < nret ){
			if( nret == 2 ){
				//
				// ワイド文字(Unicode BE)の改行をカウント
				//
				wc = *reinterpret_cast<uchar16_t *>(ptr);
				if( 0x0a00 == wc || 0x0d00 == wc ){
					nCRorLF_uni++;
				}
			}/*else{ // nret == 4
			}*/
			ptr += nret;
		}else if( nret == -2 ){
			// サロゲートペアが片一方しかない, または値が未定義.
			ptr += 2;
			nlostbytes += 2;
		}else{
			break;
		}
	}
	pWEI->UniBe.eCodeID = CODE_UNICODEBE;
	pWEI->UniBe.nCRorLF = nCRorLF_uni;
	pWEI->UniBe.nLostBytes = nlostbytes;
	
	return;
}
