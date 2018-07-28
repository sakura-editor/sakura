#include "StdAfx.h"
#include <string>
#include <mbstring.h>
#include "mem/CNativeA.h"
#include "CEol.h"
#include "charset/CShiftJis.h"
#include "charset/charcode.h"
#include "util/string_ex2.h"

CNativeA::CNativeA(const char* szData)
: CNative()
{
	SetString(szData);
}

CNativeA::CNativeA()
: CNative()
{
}

CNativeA::CNativeA(const CNativeA& rhs)
: CNative()
{
	SetString(rhs.GetStringPtr(),rhs.GetStringLength());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ設定インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// バッファの内容を置き換える
void CNativeA::SetString( const char* pszData )
{
	SetString(pszData,strlen(pszData));
}

// バッファの内容を置き換える。nLenは文字単位。
void CNativeA::SetString( const char* pData, int nDataLen )
{
	int nDataLenBytes = nDataLen * sizeof(char);
	CNative::SetRawData(pData, nDataLenBytes);
}

// バッファの内容を置き換える
void CNativeA::SetNativeData( const CNativeA& pcNative )
{
	CNative::SetRawData(pcNative);
}

// バッファの最後にデータを追加する
void CNativeA::AppendString( const char* pszData )
{
	AppendString(pszData, strlen(pszData));
}

//! バッファの最後にデータを追加する。nLengthは文字単位。
void CNativeA::AppendString( const char* pszData, int nLength )
{
	CNative::AppendRawData(pszData, nLength * sizeof(char));
}

//! バッファの最後にデータを追加する (フォーマット機能付き)
void CNativeA::AppendStringF(const char* pszData, ...)
{
	char buf[2048];

	// 整形
	va_list v;
	va_start(v, pszData);
	int len = _vsnprintf(buf, _countof(buf), pszData, v);
	va_end(v);

	if (len == -1) {
		char cbuf[128];
		sprintf_s(cbuf, _countof(cbuf), "AppendStringF error. errno = %d", errno);
		throw std::exception(cbuf);
	}

	// 追加
	this->AppendString(buf, len);
}

const CNativeA& CNativeA::operator = ( char cChar )
{
	char pszChar[2];
	pszChar[0] = cChar;
	pszChar[1] = '\0';
	SetRawData( pszChar, 1 );
	return *this;
}

//! バッファの最後にデータを追加する
void CNativeA::AppendNativeData( const CNativeA& pcNative )
{
	AppendString(pcNative.GetStringPtr(), pcNative.GetStringLength());
}

//! (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。
void CNativeA::AllocStringBuffer( int nDataLen )
{
	CNative::AllocBuffer(nDataLen * sizeof(char));
}

const CNativeA& CNativeA::operator += ( char ch )
{
	char szChar[2]={ch,'\0'};
	AppendString(szChar);
	return *this;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           互換                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CNativeA::SetStringNew(const wchar_t* wszData, int nDataLen)
{
	std::wstring buf(wszData,nDataLen); //切り出し
	char* tmp=wcstombs_new(buf.c_str());
	SetString(tmp);
	delete[] tmp;
}

void CNativeA::SetStringNew(const wchar_t* wszData)
{
	SetStringNew(wszData,wcslen(wszData));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ取得インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int CNativeA::GetStringLength() const
{
	return CNative::GetRawLength() / sizeof(char);
}

const char* CNativeA::GetStringPtr(int* pnLength) const
{
	if(pnLength)*pnLength=GetStringLength();
	return GetStringPtr();
}

// 任意位置の文字取得。nIndexは文字単位。
char CNativeA::operator[](int nIndex) const
{
	if( nIndex < GetStringLength() ){
		return GetStringPtr()[nIndex];
	}else{
		return 0;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              ネイティブ変換インターフェース                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 文字列置換 */
void CNativeA::Replace( const char* pszFrom, const char* pszTo )
{
	CNativeA	cmemWork;
	int			nFromLen = strlen( pszFrom );
	int			nToLen = strlen( pszTo );
	int			nBgnOld = 0;
	int			nBgn = 0;
	while( nBgn <= GetStringLength() - nFromLen ){
		if( 0 == auto_memcmp( &GetStringPtr()[nBgn], pszFrom, nFromLen ) ){
			if( 0  < nBgn - nBgnOld ){
				cmemWork.AppendString( &GetStringPtr()[nBgnOld], nBgn - nBgnOld );
			}
			cmemWork.AppendString( pszTo, nToLen );
			nBgn = nBgn + nFromLen;
			nBgnOld = nBgn;
		}else{
			nBgn++;
		}
	}
	if( 0  < GetStringLength() - nBgnOld ){
		cmemWork.AppendString( &GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld );
	}
	SetNativeData( cmemWork );
	return;
}



/* 文字列置換（日本語考慮版） */
void CNativeA::Replace_j( const char* pszFrom, const char* pszTo )
{
	CNativeA	cmemWork;
	int			nFromLen = strlen( pszFrom );
	int			nToLen = strlen( pszTo );
	int			nBgnOld = 0;
	int			nBgn = 0;
	while( nBgn <= GetStringLength() - nFromLen ){
		if( 0 == memcmp( &GetStringPtr()[nBgn], pszFrom, nFromLen ) ){
			if( 0  < nBgn - nBgnOld ){
				cmemWork.AppendString( &GetStringPtr()[nBgnOld], nBgn - nBgnOld );
			}
			cmemWork.AppendString( pszTo, nToLen );
			nBgn = nBgn + nFromLen;
			nBgnOld = nBgn;
		}else{
			if( _IS_SJIS_1( (unsigned char)GetStringPtr()[nBgn] ) ) nBgn++;
			nBgn++;
		}
	}
	if( 0  < GetStringLength() - nBgnOld ){
		cmemWork.AppendString( &GetStringPtr()[nBgnOld], GetStringLength() - nBgnOld );
	}
	SetNativeData( cmemWork );
	return;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   一般インターフェース                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 小文字 */
void CNativeA::ToLower()
{
	unsigned char*	pBuf = (unsigned char*)GetStringPtr();
	int				nBufLen = GetStringLength();
	int				i;
	int				nCharChars;
	unsigned char	uc;
	for( i = 0; i < nBufLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CShiftJis::GetSizeOfChar( (const char *)pBuf, nBufLen, i );
		if( nCharChars == 1 ){
			uc = (unsigned char)tolower( pBuf[i] );
			pBuf[i] = uc;
		}
		else if( nCharChars == 2 ){
			/* 全角英大文字→全角英小文字 */
			if( pBuf[i] == 0x82 && pBuf[i + 1] >= 0x60 && pBuf[i + 1] <= 0x79 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] + 0x21;
//@@@ 2001.02.03 Start by MIK: ギリシャ文字変換
			//大文字:0x839f～0x83b6
			//小文字:0x83bf～0x83d6
			}else if( pBuf[i] == 0x83 && pBuf[i + 1] >= 0x9f && pBuf[i + 1] <= 0xb6 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] + 0x20;
//@@@ 2001.02.03 End
//@@@ 2001.02.03 Start by MIK: ロシア文字変換
			//大文字:0x8440～0x8460
			//小文字:0x8470～0x8491 0x847fがない！
			}else if( pBuf[i] == 0x84 && pBuf[i + 1] >= 0x40 && pBuf[i + 1] <= 0x60 ){
				pBuf[i] = pBuf[i];
				if( pBuf[i + 1] >= 0x4f ){
					pBuf[i + 1] = pBuf[i + 1] + 0x31;
				}else{
					pBuf[i + 1] = pBuf[i + 1] + 0x30;
				}
//@@@ 2001.02.03 End
			}
		}
		if( nCharChars > 0 ){
			i += nCharChars - 1;
		}
	}
	return;
}





/* 大文字 */
void CNativeA::ToUpper()
{
	unsigned char*	pBuf = (unsigned char*)GetStringPtr();
	int				nBufLen = GetStringLength();
	int				i;
	int				nCharChars;
	unsigned char	uc;
	for( i = 0; i < nBufLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CShiftJis::GetSizeOfChar( (const char *)pBuf, nBufLen, i );
		if( nCharChars == 1 ){
			uc = (unsigned char)toupper( pBuf[i] );
			pBuf[i] = uc;
		}
		else if( nCharChars == 2 ){
			/* 全角英小文字→全角英大文字 */
			if( pBuf[i] == 0x82 && pBuf[i + 1] >= 0x81 && pBuf[i + 1] <= 0x9a ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] - 0x21;
//@@@ 2001.02.03 Start by MIK: ギリシャ文字変換
			//大文字:0x839f～0x83b6
			//小文字:0x83bf～0x83d6
			}else if( pBuf[i] == 0x83 && pBuf[i + 1] >= 0xbf && pBuf[i + 1] <= 0xd6 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] - 0x20;
//@@@ 2001.02.03 End
//@@@ 2001.02.03 Start by MIK: ロシア文字変換
			//大文字:0x8440～0x8460
			//小文字:0x8470～0x8491 0x847fがない！
			}else if( pBuf[i] == 0x84 && pBuf[i + 1] >= 0x70 && pBuf[i + 1] <= 0x91 && pBuf[i + 1] != 0x7f ){
				pBuf[i] = pBuf[i];
				if( pBuf[i + 1] >= 0x7f ){
					pBuf[i + 1] = pBuf[i + 1] - 0x31;
				}else{
					pBuf[i + 1] = pBuf[i + 1] - 0x30;
				}
//@@@ 2001.02.03 End
			}
		}
		if( nCharChars > 0 ){
			i += nCharChars - 1;
		}
	}
	return;
}


/* 半角→全角 */
void CNativeA::ToZenkaku(
		int bHiragana,		/* 1== ひらがな 0==カタカナ //2==英数専用 2001/07/30 Misaka 追加 */
		int bHanKataOnly	/* 1== 半角カタカナにのみ作用する*/
)
{
	unsigned char*			pBuf = (unsigned char*)GetStringPtr();
	int						nBufLen = GetStringLength();
	int						i;
	int						nCharChars;
//	unsigned char			uc;
	unsigned short			usSrc;
	unsigned short			usDes;
	unsigned char*			pBufDes;
	int						nBufDesLen;
	static unsigned char*	pszHanKataSet = (unsigned char*)"｡｢｣､･ｦｧｨｩｪｫｬｭｮｯｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝﾞﾟ";
	static unsigned char*	pszDakuSet = (unsigned char*)"ｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾊﾋﾌﾍﾎ";
	static unsigned char*	pszYouSet = (unsigned char*)"ﾊﾋﾌﾍﾎ";
	BOOL					bHenkanOK;

	pBufDes = new unsigned char[nBufLen * 2 + 1];
	if( NULL ==	pBufDes ){
		return;
	}
	nBufDesLen = 0;
	for( i = 0; i < nBufLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CShiftJis::GetSizeOfChar( (const char *)pBuf, nBufLen, i );
		if( nCharChars == 1){
			bHenkanOK = FALSE;
			if( bHanKataOnly ){	/* 1== 半角カタカナにのみ作用する */
				if( NULL != strchr( (const char *)pszHanKataSet, pBuf[i] ) ){
					bHenkanOK = TRUE;
				}
			}else{
				//! 英数変換用に新たな条件を付加 2001/07/30 Misaka
				if( ( (unsigned char)0x20 <= pBuf[i] && pBuf[i] <= (unsigned char)0x7E ) ||
					( bHiragana != 2 && (unsigned char)0xA1 <= pBuf[i] && pBuf[i] <= (unsigned char)0xDF )
				){
					bHenkanOK = TRUE;
				}
			}
			if( bHenkanOK ){
				usSrc = pBuf[i];
				if( !bHiragana &&
					pBuf[i]		== (unsigned char)'ｳ' &&
					pBuf[i + 1] == (unsigned char)'ﾞ' &&
					bHiragana != 2
				){
					usDes = (unsigned short)0x8394; /* ヴ */
					nCharChars = 2;
				}else {
					usDes = (unsigned short)_mbbtombc( usSrc );
					/* 濁音 */
					if( bHiragana != 2 && pBuf[i + 1] == (unsigned char)'ﾞ' && NULL != strchr( (const char *)pszDakuSet, pBuf[i] ) ){
						usDes++;
						nCharChars = 2;
					}
					/* 拗音 */
					//! 英数変換用に新たな条件を付加 2001/07/30 Misaka
					//! bHiragana != 2 //英数変換フラグがオンではない場合
					if( bHiragana != 2 && pBuf[i + 1] == (unsigned char)'ﾟ' && NULL != strchr( (const char *)pszYouSet, pBuf[i] ) ){
						usDes += 2;
						nCharChars = 2;
					}
				}

				if( bHiragana == 1 ){
					/* ひらがなに変換可能なカタカナならば、ひらがなに変換する */
					if( (unsigned short)0x8340 <= usDes && usDes <= (unsigned short)0x837e ){	/* ァ～ミ */
						usDes-= (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x8380 <= usDes && usDes <= (unsigned short)0x8393 ){	/* ム～ン */
						usDes-= (unsigned short)0x00a2;
					}
				}
				pBufDes[nBufDesLen]		= ( usDes & 0xff00 ) >>  8;
				pBufDes[nBufDesLen + 1] = ( usDes & 0x00ff );
				nBufDesLen += 2;
			}else{
				memcpy( &pBufDes[nBufDesLen], &pBuf[i], nCharChars );
				nBufDesLen += nCharChars;

			}
		}else
		if( nCharChars == 2 ){
			usDes = usSrc = pBuf[i + 1] | ( pBuf[i] << 8 );
			if( bHanKataOnly == 0 ){
				if( bHiragana == 1 ){//英数変換を付加したために数値で指定した　2001/07/30 Misaka
					/* 全角ひらがなに変換可能な全角カタカナならば、ひらがなに変換する */
					if( (unsigned short)0x8340 <= usSrc && usSrc <= (unsigned short)0x837e ){	/* ァ～ミ */
						usDes = usSrc - (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x8380 <= usSrc && usSrc <= (unsigned short)0x8393 ){	/* ム～ン */
						usDes = usSrc - (unsigned short)0x00a2;
					}
				}else if( bHiragana == 0 ){//英数変換を付加したために数値で指定した　2001/07/30 Misaka
					/* 全角カタカナに変換可能な全角ひらがなならば、カタカナに変換する */
					if( (unsigned short)0x829f <= usSrc && usSrc <= (unsigned short)0x82dd ){	/* ぁ～み */
						usDes = usSrc + (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x82de <= usSrc && usSrc <= (unsigned short)0x82f1 ){	/* む～ん */
						usDes = usSrc + (unsigned short)0x00a2;
					}
				}
			}
			pBufDes[nBufDesLen]		= ( usDes & 0xff00 ) >> 8;
			pBufDes[nBufDesLen + 1] = ( usDes & 0x00ff );
			nBufDesLen += 2;
		}else{
			memcpy( &pBufDes[nBufDesLen], &pBuf[i], nCharChars );
			nBufDesLen += nCharChars;

		}
		if( nCharChars > 0 ){
			i += nCharChars - 1;
		}
	}
	pBufDes[nBufDesLen] = '\0';
	SetRawData( pBufDes, nBufDesLen );
	delete [] pBufDes;


	return;
}


/* TAB→空白 */
void CNativeA::TABToSPACE( int nTabSpace	/* TABの文字数 */ )
{
	using namespace ACODE;

	const char*	pLine;
	int			nLineLen;
	char*		pDes;
	int			nBgn;
	int			i;
	int			nPosDes;
//	BOOL		bEOL;
	int			nPosX;
	int			nWork;
	CEol		cEol;
	nBgn = 0;
	nPosDes = 0;
	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
	while( NULL != ( pLine = GetNextLine( GetStringPtr(), GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosX = 0;
			for( i = 0; i < nLineLen; ++i ){
				if( TAB == pLine[i]	){
					nWork = nTabSpace - ( nPosX % nTabSpace );
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
		return;
	}
	pDes = new char[nPosDes + 1];
	nBgn = 0;
	nPosDes = 0;
	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
	while( NULL != ( pLine = GetNextLine( GetStringPtr(), GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosX = 0;
			for( i = 0; i < nLineLen; ++i ){
				if( TAB == pLine[i]	){
					nWork = nTabSpace - ( nPosX % nTabSpace );
					auto_memset( &pDes[nPosDes], ' ', nWork );
					nPosDes += nWork;
					nPosX += nWork;
				}else{
					pDes[nPosDes] = pLine[i];
					nPosDes++;
					nPosX++;
				}
			}
		}
		CMemory cEolMem; CShiftJis::S_GetEol(&cEolMem,cEol.GetType());
		auto_memcpy( &pDes[nPosDes], (const char*)cEolMem.GetRawPtr(), cEolMem.GetRawLength() );
		nPosDes += cEolMem.GetRawLength();
	}
	pDes[nPosDes] = '\0';

	SetRawData( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return;
}


//!空白→TAB変換
/*!
	@param nTabSpace TABの文字数
	単独のスペースは変換しない

	@author Stonee
	@date 2001/5/27
*/
void CNativeA::SPACEToTAB( int nTabSpace )
{
	using namespace ACODE;

	const char*	pLine;
	int			nLineLen;
	char*		pDes;
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
	while( NULL != ( pLine = GetNextLine( GetStringPtr(), GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosDes += nLineLen;
		}
		nPosDes += cEol.GetLen();
	}
	if( 0 >= nPosDes ){
		return;
	}
	pDes = new char[nPosDes + 1];
	nBgn = 0;
	nPosDes = 0;
	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
	while( NULL != ( pLine = GetNextLine( GetStringPtr(), GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosX = 0;	// 処理中のiに対応する表示桁位置
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
						nPosX += nTabSpace - (nPosX % nTabSpace);
					}
				}else{
					if( bSpace ){
						if( (1 == nPosX - nStartPos) && (SPACE == pLine[i - 1]) ){
							pDes[nPosDes] = SPACE;
							nPosDes++;
						} else{
							for( j = nStartPos / nTabSpace; j < (nPosX / nTabSpace); j++ ){
								pDes[nPosDes] = TAB;
								nPosDes++;
								nStartPos += nTabSpace - ( nStartPos % nTabSpace );
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
					//for( j = nStartPos - 1; (j + nTabSpace) <= nPosX + 1; j+=nTabSpace ){
					for( j = nStartPos / nTabSpace; j < (nPosX / nTabSpace); j++ ){
						pDes[nPosDes] = TAB;
						nPosDes++;
						nStartPos += nTabSpace - ( nStartPos % nTabSpace );
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
		CMemory cEolMem; CShiftJis::S_GetEol(&cEolMem,cEol.GetType());
		auto_memcpy( &pDes[nPosDes], (const char*)cEolMem.GetRawPtr(), cEolMem.GetRawLength() );
		nPosDes += cEolMem.GetRawLength();
	}
	pDes[nPosDes] = '\0';

	SetRawData( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  staticインターフェース                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 指定した位置の文字が何バイト文字かを返す
int CNativeA::GetSizeOfChar( const char* pData, int nDataLen, int nIdx )
{
	return CShiftJis::GetSizeOfChar(pData,nDataLen,nIdx);
}

/* ポインタで示した文字の次にある文字の位置を返します */
/* 次にある文字がバッファの最後の位置を越える場合は&pData[nDataLen]を返します */
const char* CNativeA::GetCharNext( const char* pData, int nDataLen, const char* pDataCurrent )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( "CMemory::MemCharNext" );
//#endif

	const char*	pNext;
	if( pDataCurrent[0] == '\0' ){
		pNext = pDataCurrent + 1;
	}else
	{
//		pNext = ::CharNext( pDataCurrent );
		if(
			/* SJIS全角コードの1バイト目か */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
			_IS_SJIS_1( (unsigned char)pDataCurrent[0] )
			&&
			/* SJIS全角コードの2バイト目か */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
			_IS_SJIS_2( (unsigned char)pDataCurrent[1] )
		){
			pNext = pDataCurrent + 2;
		}else{
			pNext = pDataCurrent + 1;
		}
	}

	if( pNext >= &pData[nDataLen] ){
		pNext = &pData[nDataLen];
	}
	return pNext;
}

/* ポインタで示した文字の直前にある文字の位置を返します */
/* 直前にある文字がバッファの先頭の位置を越える場合はpDataを返します */
const char* CNativeA::GetCharPrev( const char* pData, int nDataLen, const char* pDataCurrent )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( "CMemory::MemCharPrev" );
//#endif


	const char*	pPrev;
	pPrev = ::CharPrevA( pData, pDataCurrent );

//===1999.08.12  このやり方だと、ダメだった。===============-
//
//	if( (pDataCurrent - 1)[0] == '\0' ){
//		pPrev = pDataCurrent - 1;
//	}else{
//		if( pDataCurrent - pData >= 2 &&
//			/* SJIS全角コードの1バイト目か */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
//			(
//			( (unsigned char)0x81 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0x9F ) ||
//			( (unsigned char)0xE0 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0xFC )
//			) &&
//			/* SJIS全角コードの2バイト目か */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
//			(
//			( (unsigned char)0x40 <= (unsigned char)pDataCurrent[-1] && (unsigned char)pDataCurrent[-1] <= (unsigned char)0x7E ) ||
//			( (unsigned char)0x80 <= (unsigned char)pDataCurrent[-1] && (unsigned char)pDataCurrent[-1] <= (unsigned char)0xFC )
//			)
//		){
//			pPrev = pDataCurrent - 2;
//		}else{
//			pPrev = pDataCurrent - 1;
//		}
//	}
//	if( pPrev < pData ){
//		pPrev = pData;
//	}
	return pPrev;
}


void CNativeA::AppendStringNew( const wchar_t* pszData )
{
	AppendStringNew(pszData,wcslen(pszData));
}
void CNativeA::AppendStringNew( const wchar_t* pData, int nDataLen )
{
	char* buf=wcstombs_new(pData,nDataLen);
	AppendString(buf);
	delete[] buf;
}


const wchar_t* CNativeA::GetStringW() const
{
	return to_wchar(GetStringPtr(),GetStringLength());
}
