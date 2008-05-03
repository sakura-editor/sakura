#include "stdafx.h"
#include "CConvert_Base64Decode.h"
#include "charset/charcode.h"
#include "charset/CBase64.h"

#if 1 //$$$�ۗ�

/* ������Base64�̃f�[�^�� */
int IsBASE64Char( wchar_t cData )
{
	char buf[4]={0};
	int n=wctomb(buf,cData);

	int nret = -1;
	if( n==1 && (unsigned char)buf[0] < 0x80 ){
		nret = CBase64::BASE64VAL[cData];
		if( nret == 0xff ){
			nret = -1;
		}
	}
	return nret;
}
int IsBASE64Char( char cData )
{
	int nret = -1;
	if( (unsigned char)cData < 0x80 ){
		nret = CBase64::BASE64VAL[cData];
		if( nret == 0xff ){
			nret = -1;
		}
	}
	return nret;
}


/* Base64�f�R�[�h */
void CConvert_Base64Decode::BASE64Decode( const CNativeW* pcSrc, CMemory* pcDst )
{
	//�f�[�^�擾
	int	nSrcLen;
	const wchar_t*	pszSrc = pcSrc->GetStringPtr(&nSrcLen);

	int		nBgn;
	int		nPos;
	CMemory	cmemBuf;

	nBgn = 0;
	nPos = 0;
	while( nBgn < nSrcLen ){
		while( nBgn < nSrcLen && 0 > IsBASE64Char( pszSrc[nBgn] ) ){
			nBgn++;
		}
		nPos = nBgn + 1;
		while( nPos < nSrcLen && 0 <= IsBASE64Char( pszSrc[nPos] ) ){
			nPos++;
		}
		if( nBgn < nSrcLen && nPos - nBgn > 0 ){
			BYTE* pBin = new BYTE[nPos - nBgn];

			// Base64�f�R�[�h
			int nBinLen = MemBASE64_Decode( &pszSrc[nBgn], nPos - nBgn, pBin );
			cmemBuf.AppendRawData( pBin, nBinLen );
			delete [] pBin;
		}
		nBgn = nPos;
	}
	pcDst->SetRawData( cmemBuf.GetRawPtr(), cmemBuf.GetRawLength() );
	return;
}




// BASE64 => �f�R�[�h��
// 4����  => 3����

// Base64�f�R�[�h
template <class CHAR_TYPE>
long CConvert_Base64Decode::MemBASE64_Decode( const CHAR_TYPE* pszSrc, long nSrcLen, BYTE* pDst )
{
	long			nSrcSize;
	long			lData;
	long			nDesLen;
	long			lDesSize;
	int				sMax;

	//Src�̗L�����̎Z�o -> nSrcSize
	{
		int i;
		for( i = 0; i < nSrcLen; i++ ){
			if( sizeof(CHAR_TYPE)==1 && pszSrc[i]=='=' ){
				break;
			}
			else if( sizeof(CHAR_TYPE)==2 && pszSrc[i]==L'=' ){
				break;
			}
		}
		nSrcSize = i;
	}

	//�ϊ���m�� -> pszDes, nDesLen
	nDesLen = (nSrcSize / 4) * 3;
	nDesLen += ((nSrcSize % 4) * 6) / 8;
	BYTE*	pszDes;
	pszDes = new BYTE[nDesLen + 1];
	memset( pszDes, 0, nDesLen + 1 );

	lDesSize = 0;
	for( int i = 0; i < nSrcSize; i++ ){
		if( i < nSrcSize - (nSrcSize % 4) ){
			sMax = 4;
		}else{
			sMax = (nSrcSize % 4);
		}
		lData = 0;
		for( int j = 0; j < sMax; j++ ){
			int k = IsBASE64Char( pszSrc[i + j] );
			if( k >= 0 ){
				lData |= (((long)k) << ((4 - j - 1) * 6));
			}
		}
		for( int j = 0; j < (sMax * 6)/ 8 ; j++ ){
			pszDes[lDesSize] = (unsigned char)((lData >> (8 * (2 - j))) & 0x0000ff);
			lDesSize++;
		}
		i+= 3;
	}
	pszDes[lDesSize] = 0;

	//�o�b�t�@�R�s�[
	memcpy( pDst, pszDes, lDesSize );

	delete [] pszDes;
	return lDesSize;
}



//�C���X�^���X��
template long CConvert_Base64Decode::MemBASE64_Decode( const WCHAR* pszSrc, long nSrcLen, BYTE* pDst );	/* Base64�f�R�[�h */
template long CConvert_Base64Decode::MemBASE64_Decode( const ACHAR* pszSrc, long nSrcLen, BYTE* pDst );	/* Base64�f�R�[�h */

#endif

