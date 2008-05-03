#include "stdafx.h"
#include "mem/CNativeA.h"
#include "charset/CShiftJis.h"
#include <mbstring.h>
#include "CEol.h"
#include "charset/charcode.h"
#include <string>
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
//              �l�C�e�B�u�ݒ�C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �o�b�t�@�̓��e��u��������
void CNativeA::SetString( const char* pszData )
{
	SetString(pszData,strlen(pszData));
}

// �o�b�t�@�̓��e��u��������BnLen�͕����P�ʁB
void CNativeA::SetString( const char* pData, int nDataLen )
{
	int nDataLenBytes = nDataLen * sizeof(char);
	CNative::SetRawData(pData, nDataLenBytes);
}

// �o�b�t�@�̓��e��u��������
void CNativeA::SetNativeData( const CNativeA& pcNative )
{
	CNative::SetRawData(pcNative);
}

// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
void CNativeA::AppendString( const char* pszData )
{
	AppendString(pszData, strlen(pszData));
}

//! �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����BnLength�͕����P�ʁB
void CNativeA::AppendString( const char* pszData, int nLength )
{
	CNative::AppendRawData(pszData, nLength * sizeof(char));
}

const CNativeA& CNativeA::operator = ( char cChar )
{
	char pszChar[2];
	pszChar[0] = cChar;
	pszChar[1] = '\0';
	SetRawData( pszChar, 1 );
	return *this;
}

//! �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
void CNativeA::AppendNativeData( const CNativeA& pcNative )
{
	AppendString(pcNative.GetStringPtr(), pcNative.GetStringLength());
}

//! (�d�v�FnDataLen�͕����P��) �o�b�t�@�T�C�Y�̒����B�K�v�ɉ����Ċg�傷��B
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
//                           �݊�                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CNativeA::SetStringNew(const wchar_t* wszData, int nDataLen)
{
	std::wstring buf(wszData,nDataLen); //�؂�o��
	char* tmp=wcstombs_new(buf.c_str());
	SetString(tmp);
	delete[] tmp;
}

void CNativeA::SetStringNew(const wchar_t* wszData)
{
	SetStringNew(wszData,wcslen(wszData));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�擾�C���^�[�t�F�[�X                 //
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

// �C�ӈʒu�̕����擾�BnIndex�͕����P�ʁB
char CNativeA::operator[](int nIndex) const
{
	if( nIndex < GetStringLength() ){
		return GetStringPtr()[nIndex];
	}else{
		return 0;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//              �l�C�e�B�u�ϊ��C���^�[�t�F�[�X                 //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* ������u�� */
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



/* ������u���i���{��l���Łj */
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
//                   ��ʃC���^�[�t�F�[�X                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* ������ */
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
			/* �S�p�p�啶�����S�p�p������ */
			if( pBuf[i] == 0x82 && pBuf[i + 1] >= 0x60 && pBuf[i + 1] <= 0x79 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] + 0x21;
//@@@ 2001.02.03 Start by MIK: �M���V�������ϊ�
			//�啶��:0x839f�`0x83b6
			//������:0x83bf�`0x83d6
			}else if( pBuf[i] == 0x83 && pBuf[i + 1] >= 0x9f && pBuf[i + 1] <= 0xb6 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] + 0x20;
//@@@ 2001.02.03 End
//@@@ 2001.02.03 Start by MIK: ���V�A�����ϊ�
			//�啶��:0x8440�`0x8460
			//������:0x8470�`0x8491 0x847f���Ȃ��I
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





/* �啶�� */
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
			/* �S�p�p���������S�p�p�啶�� */
			if( pBuf[i] == 0x82 && pBuf[i + 1] >= 0x81 && pBuf[i + 1] <= 0x9a ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] - 0x21;
//@@@ 2001.02.03 Start by MIK: �M���V�������ϊ�
			//�啶��:0x839f�`0x83b6
			//������:0x83bf�`0x83d6
			}else if( pBuf[i] == 0x83 && pBuf[i + 1] >= 0xbf && pBuf[i + 1] <= 0xd6 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] - 0x20;
//@@@ 2001.02.03 End
//@@@ 2001.02.03 Start by MIK: ���V�A�����ϊ�
			//�啶��:0x8440�`0x8460
			//������:0x8470�`0x8491 0x847f���Ȃ��I
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


/* ���p���S�p */
void CNativeA::ToZenkaku(
		int bHiragana,		/* 1== �Ђ炪�� 0==�J�^�J�i //2==�p����p 2001/07/30 Misaka �ǉ� */
		int bHanKataOnly	/* 1== ���p�J�^�J�i�ɂ̂ݍ�p����*/
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
	static unsigned char*	pszHanKataSet = (unsigned char*)"���������������������������������������������������������������";
	static unsigned char*	pszDakuSet = (unsigned char*)"��������������������";
	static unsigned char*	pszYouSet = (unsigned char*)"�����";
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
			if( bHanKataOnly ){	/* 1== ���p�J�^�J�i�ɂ̂ݍ�p���� */
				if( NULL != strchr( (const char *)pszHanKataSet, pBuf[i] ) ){
					bHenkanOK = TRUE;
				}
			}else{
				//! �p���ϊ��p�ɐV���ȏ�����t�� 2001/07/30 Misaka
				if( ( (unsigned char)0x20 <= pBuf[i] && pBuf[i] <= (unsigned char)0x7E ) ||
					( bHiragana != 2 && (unsigned char)0xA1 <= pBuf[i] && pBuf[i] <= (unsigned char)0xDF )
				){
					bHenkanOK = TRUE;
				}
			}
			if( bHenkanOK ){
				usSrc = pBuf[i];
				if( !bHiragana &&
					pBuf[i]		== (unsigned char)'�' &&
					pBuf[i + 1] == (unsigned char)'�' &&
					bHiragana != 2
				){
					usDes = (unsigned short)0x8394; /* �� */
					nCharChars = 2;
				}else {
					usDes = _mbbtombc( usSrc );
					/* ���� */
					if( bHiragana != 2 && pBuf[i + 1] == (unsigned char)'�' && NULL != strchr( (const char *)pszDakuSet, pBuf[i] ) ){
						usDes++;
						nCharChars = 2;
					}
					/* �X�� */
					//! �p���ϊ��p�ɐV���ȏ�����t�� 2001/07/30 Misaka
					//! bHiragana != 2 //�p���ϊ��t���O���I���ł͂Ȃ��ꍇ
					if( bHiragana != 2 && pBuf[i + 1] == (unsigned char)'�' && NULL != strchr( (const char *)pszYouSet, pBuf[i] ) ){
						usDes += 2;
						nCharChars = 2;
					}
				}

				if( bHiragana == 1 ){
					/* �Ђ炪�Ȃɕϊ��\�ȃJ�^�J�i�Ȃ�΁A�Ђ炪�Ȃɕϊ����� */
					if( (unsigned short)0x8340 <= usDes && usDes <= (unsigned short)0x837e ){	/* �@�`�~ */
						usDes-= (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x8380 <= usDes && usDes <= (unsigned short)0x8393 ){	/* ���`�� */
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
				if( bHiragana == 1 ){//�p���ϊ���t���������߂ɐ��l�Ŏw�肵���@2001/07/30 Misaka
					/* �S�p�Ђ炪�Ȃɕϊ��\�ȑS�p�J�^�J�i�Ȃ�΁A�Ђ炪�Ȃɕϊ����� */
					if( (unsigned short)0x8340 <= usSrc && usSrc <= (unsigned short)0x837e ){	/* �@�`�~ */
						usDes = usSrc - (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x8380 <= usSrc && usSrc <= (unsigned short)0x8393 ){	/* ���`�� */
						usDes = usSrc - (unsigned short)0x00a2;
					}
				}else if( bHiragana == 0 ){//�p���ϊ���t���������߂ɐ��l�Ŏw�肵���@2001/07/30 Misaka
					/* �S�p�J�^�J�i�ɕϊ��\�ȑS�p�Ђ炪�ȂȂ�΁A�J�^�J�i�ɕϊ����� */
					if( (unsigned short)0x829f <= usSrc && usSrc <= (unsigned short)0x82dd ){	/* ���`�� */
						usDes = usSrc + (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x82de <= usSrc && usSrc <= (unsigned short)0x82f1 ){	/* �ށ`�� */
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


/* TAB���� */
void CNativeA::TABToSPACE( int nTabSpace	/* TAB�̕����� */ )
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
	/* CRLF�ŋ�؂���u�s�v��Ԃ��BCRLF�͍s���ɉ����Ȃ� */
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
	/* CRLF�ŋ�؂���u�s�v��Ԃ��BCRLF�͍s���ɉ����Ȃ� */
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


//!�󔒁�TAB�ϊ�
/*!
	@param nTabSpace TAB�̕�����
	�P�Ƃ̃X�y�[�X�͕ϊ����Ȃ�

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

	BOOL		bSpace = FALSE;	//�X�y�[�X�̏��������ǂ���
	int		j;
	int		nStartPos;

	nBgn = 0;
	nPosDes = 0;
	/* �ϊ���ɕK�v�ȃo�C�g���𒲂ׂ� */
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
	/* CRLF�ŋ�؂���u�s�v��Ԃ��BCRLF�͍s���ɉ����Ȃ� */
	while( NULL != ( pLine = GetNextLine( GetStringPtr(), GetStringLength(), &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosX = 0;	// ��������i�ɑΉ�����\�����ʒu
			bSpace = FALSE;	//���O���X�y�[�X��
			nStartPos = 0;	// �X�y�[�X�̐擪
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
							//	�ϊ����TAB��1������Ȃ��ꍇ�ɃX�y�[�X���l�߂�����
							//	�o�b�t�@���͂ݏo���̂��C��
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
					//	�ϊ����TAB��1������Ȃ��ꍇ�ɃX�y�[�X���l�߂�����
					//	�o�b�t�@���͂ݏo���̂��C��
					for( j = nStartPos; j < nPosX; j++ ){
						pDes[nPosDes] = SPACE;
						nPosDes++;
					}
				}
			}
		}

		/* �s���̏��� */
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
//                  static�C���^�[�t�F�[�X                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �w�肵���ʒu�̕��������o�C�g��������Ԃ�
int CNativeA::GetSizeOfChar( const char* pData, int nDataLen, int nIdx )
{
	return CShiftJis::GetSizeOfChar(pData,nDataLen,nIdx);
}

/* �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂� */
/* ���ɂ��镶�����o�b�t�@�̍Ō�̈ʒu���z����ꍇ��&pData[nDataLen]��Ԃ��܂� */
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
			/* SJIS�S�p�R�[�h��1�o�C�g�ڂ� */	//Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
			_IS_SJIS_1( (unsigned char)pDataCurrent[0] )
			&&
			/* SJIS�S�p�R�[�h��2�o�C�g�ڂ� */	//Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
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

/* �|�C���^�Ŏ����������̒��O�ɂ��镶���̈ʒu��Ԃ��܂� */
/* ���O�ɂ��镶�����o�b�t�@�̐擪�̈ʒu���z����ꍇ��pData��Ԃ��܂� */
const char* CNativeA::GetCharPrev( const char* pData, int nDataLen, const char* pDataCurrent )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( "CMemory::MemCharPrev" );
//#endif


	const char*	pPrev;
	pPrev = ::CharPrevA( pData, pDataCurrent );

//===1999.08.12  ���̂������ƁA�_���������B===============-
//
//	if( (pDataCurrent - 1)[0] == '\0' ){
//		pPrev = pDataCurrent - 1;
//	}else{
//		if( pDataCurrent - pData >= 2 &&
//			/* SJIS�S�p�R�[�h��1�o�C�g�ڂ� */	//Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
//			(
//			( (unsigned char)0x81 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0x9F ) ||
//			( (unsigned char)0xE0 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0xFC )
//			) &&
//			/* SJIS�S�p�R�[�h��2�o�C�g�ڂ� */	//Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
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
