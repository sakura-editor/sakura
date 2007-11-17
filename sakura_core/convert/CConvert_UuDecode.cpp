#include "stdafx.h"
#include "CConvert_UuDecode.h"
#include "charcode.h"

inline uchar_t UUDECODE_CHAR( wchar_t c )
{
	char buf[4]={0};
	int ret=wctomb(buf,c);
	if(ret<=0)return 0;
	return static_cast<uchar_t>((buf[0] - 0x20) & 0x3f);
}


/* Uudecode (デコード）*/
void CConvert_UuDecode::UUDECODE(
	const CNativeW&	pcSrc,		//!< [in]
	CMemory*			pcDst,		//!< [out]
	TCHAR*				pszFileName	//!< [out]
)
{
	using namespace WCODE;

	//元データ
	int				nSrcLen;
	const wchar_t*	pszSrc = pcSrc.GetStringPtr(&nSrcLen);

	CMemory			cmemBuf;
	BOOL			bBEGIN;
	unsigned char	uchDecode[64];
	bBEGIN = FALSE;

	int nBgn = 0;
	while( nBgn < nSrcLen ){
		int nPos = nBgn;
		while( nPos < nSrcLen && pszSrc[nPos] != CR && pszSrc[nPos] != LF ){
			nPos++;
		}
		if( nBgn < nSrcLen && nPos - nBgn > 0 ){
			int			nBufLen = nPos - nBgn;
			wchar_t*	pBuf = new wchar_t[ nBufLen + 1];
			memset( pBuf, 0, nBufLen + 1 );
			memcpy( pBuf, &pszSrc[nBgn], nBufLen );
			if( !bBEGIN	){
				if( 0 == auto_memcmp( pBuf, L"begin ", 6 ) ){
					bBEGIN = TRUE;
					char*	pTok;
					/* 最初のトークンを取得します。*/
					pTok = strtok( (char*)pBuf, " " );
					/* 次のトークンを取得します。*/
					pTok = strtok( NULL, " " );
					/* 更に次のトークンを取得します。*/
					pTok = strtok( NULL, "\0" );
					if( NULL != pszFileName ){
						_mbstotcs( pszFileName, pTok, _MAX_PATH );
					}
				}
			}
			else{
				if( 0 == auto_memcmp( pBuf, L"end", 3 ) ){
					break;
				}
				else{
					int		nBytes;
					nBytes = UUDECODE_CHAR( pBuf[0] );
					if( 0 == nBytes ){
						break;
					}
					int		iByteIndex = 0;
					int		iCharIndex = 1;
					while ( iByteIndex < nBytes ) {
						// Note that nBytes may not be a multiple of 3, but the
						// number of characters on the line should be a multiple of 4.
						// If the number of encoded bytes is not a multiple of 3 then
						// extra pad characters should have been added to the text
						// by the encoder to make the character count a multiple of 4.
//						if( pBuf[iCharIndex] == '\0' ||
//							pBuf[iCharIndex + 1] == '\0' ||
//							pBuf[iCharIndex + 2] == '\0' ||
//							pBuf[iCharIndex + 3] == '\0') {
//							//break;
//						}
						// Decode the line in 3-byte (=4-character) jumps.
						if( iByteIndex < nBytes ){
							uchDecode[iByteIndex] =
								(unsigned char)((UUDECODE_CHAR(pBuf[iCharIndex]) << 2) |
								(UUDECODE_CHAR(pBuf[iCharIndex + 1]) >> 4));
							iByteIndex++;
						}
						if( iByteIndex < nBytes ){
							uchDecode[iByteIndex] =
								(unsigned char)((UUDECODE_CHAR(pBuf[iCharIndex + 1]) << 4) |
								(UUDECODE_CHAR(pBuf[iCharIndex + 2]) >> 2));
							iByteIndex++;
						}
						if( iByteIndex < nBytes ){
							uchDecode[iByteIndex] =
								(unsigned char)((UUDECODE_CHAR(pBuf[iCharIndex + 2]) << 6) |
								UUDECODE_CHAR(pBuf[iCharIndex + 3]));
							iByteIndex++;
						}
						iCharIndex += 4;
					}
					cmemBuf.AppendRawData( uchDecode, iByteIndex );
				}
			}
			delete [] pBuf;
		}
		nBgn = nPos;
		while( nBgn < nSrcLen && ( pszSrc[nBgn] == CR || pszSrc[nBgn] == LF ) ){
			nBgn++;
		}
	}
	pcDst->SetRawData( cmemBuf.GetRawPtr(), cmemBuf.GetRawLength() );
	return;
}
