#include "stdafx.h"
#include "CEuc.h"
#include "CShiftJis.h"
#include "charcode.h"
#include <mbstring.h>
#include "codeutil.h"

/* EUC→Unicodeコード変換 */
//2007.08.13 kobake 追加
EConvertResult CEuc::EUCToUnicode(CMemory* pMem)
{
	//$$ SJISを介しているので無駄にデータを失うかも？
	EUCToSJIS(pMem);
	CShiftJis::SJISToUnicode(pMem);

	return RESULT_COMPLETE;
}

EConvertResult CEuc::UnicodeToEUC(CMemory* pMem)
{
	//$$ SJISを介しているので無駄にデータを失うかも？
	CShiftJis::UnicodeToSJIS(pMem);
	SJISToEUC(pMem);

	return RESULT_COMPLETE;
}


/************************************************************************
*
*【関数名】
*	EUCToSJIS
*
*【機能】
*	指定範囲のバッファ内にEUC漢字コード
*	があればSJIS全角コードに変換する。	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
*	半角文字は変換せずにそのまま残す。
*
*	制御文字CRLF以外のバイナリコードが混入している場合には結果が不定と
*	なることがあるので注意。
*	バッファの最後に漢字コードの1バイト目だけがあると困る
*
*【入力】	なし
*
*【戻り値】	なし
*
************************************************************************/
/* EUC→SJISコード変換 */
void CEuc::EUCToSJIS( CMemory* pMem )
{
	//データ取得
	int				nBufLen;
	const char*		pBuf = (const char*)pMem->GetRawPtr(&nBufLen);

	CMemory cmemTmp;

	int				nPtr = 0L;
	int				nPtrDes = 0L;
	char*			pszDes = new char[nBufLen];
	unsigned int	sCode;

	while( nPtr < nBufLen ){
		if( (unsigned char)pBuf[nPtr] == (unsigned char)0x8e && nPtr < nBufLen - 1 ){
			/* 半角カタカナ */
			pszDes[nPtrDes] = pBuf[nPtr + 1];
			nPtrDes++;
			nPtr += 2;
		}
		/* EUC漢字コードか? */
		else if( nPtr < nBufLen - 1 && Charcode::IsEucKan1(pBuf[nPtr]) && Charcode::IsEucKan2(pBuf[nPtr + 1L]) ){
			/* 通常のJISコードに変換 */
			char jis[2];
			jis[0] = pBuf[nPtr	  ] & 0x7f;
			jis[1] = pBuf[nPtr + 1L] & 0x7f;

			/* SJISコードに変換 */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
			sCode = (unsigned short)_mbcjistojms(
				(unsigned int)
				((unsigned short)jis[0] << 8) |
				 ((unsigned short)jis[1])
			);
			if( sCode != 0 ){
				pszDes[nPtrDes	  ] = (unsigned char)(sCode >> 8);
				pszDes[nPtrDes + 1] = (unsigned char)(sCode);
				nPtrDes += 2;;
				nPtr += 2;
			}else{
				pszDes[nPtrDes] = jis[0];
				nPtrDes++;
				nPtr++;
			}
		}
		else{
			pszDes[nPtrDes] = pBuf[nPtr];
			nPtrDes++;
			nPtr++;
		}
	}
	pMem->SetRawData( pszDes, nPtrDes );
	delete [] pszDes;
	return;
}


/* SJIS→EUCコード変換 */
void CEuc::SJISToEUC( CMemory* pMem )
{
	//データ取得
	int				nBufLen;
	unsigned char*	pBuf = (unsigned char*)pMem->GetRawPtr(&nBufLen);

	int				i;
	int				nCharChars;
	unsigned char*	pDes;
	int				nDesIdx;
	unsigned short	sCode;

	pDes = new unsigned char[nBufLen * 2];
	nDesIdx = 0;
	for( i = 0; i < nBufLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CShiftJis::GetSizeOfChar( reinterpret_cast<char*>(pBuf), nBufLen, i );
		if( nCharChars == 1 ){
			if( pBuf[i] >= (unsigned char)0x80 ){
				/* 半角カタカナ */
				pDes[nDesIdx	] = (unsigned char)0x8e;
				pDes[nDesIdx + 1] = pBuf[i];
				nDesIdx += 2;
			}else{
				pDes[nDesIdx] = pBuf[i];
				nDesIdx++;
			}
		}else
		if( nCharChars == 2 ){
			/* 全角文字 */
			//	Oct. 3, 2002 genta IBM拡張文字対応
			sCode =	(unsigned short)_mbcjmstojis_ex( pBuf + i );
			if(sCode != 0){
				pDes[nDesIdx	] = (unsigned char)0x80 | (unsigned char)(sCode >> 8);
				pDes[nDesIdx + 1] = (unsigned char)0x80 | (unsigned char)(sCode);
				nDesIdx += 2;
				++i;
			}else{
				pDes[nDesIdx	] = pBuf[i];
				pDes[nDesIdx + 1] = pBuf[i + 1];
				nDesIdx += 2;
				++i;
			}
		}else
		if( nCharChars > 0 ){
			i += nCharChars - 1;
		}
	}
	pMem->SetRawData( pDes, nDesIdx );
	delete [] pDes;
	return;
}
