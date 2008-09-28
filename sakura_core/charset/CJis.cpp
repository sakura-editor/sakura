#include "stdafx.h"
#include "CJis.h"
#include "CShiftJis.h"
#include <mbstring.h>
#include "convert/CConvert_Base64Decode.h"
#include "charset/charcode.h"
#include "codeutil.h"

#define ESC_JIS		"\x01b$B"
#define ESC_ASCII	"\x01b(B"
#define ESC_8BIT	"\x01b(I"

#define MIME_BASE64	1
#define MIME_QUOTED	2

/* ������� */
#define CHAR_ASCII		0	/* ASCII���� */
#define CHAR_8BITCODE	1	/* 8�r�b�g�R�[�h(���p�J�^�J�i�Ȃ�) */
#define CHAR_ZENKAKU	2	/* �S�p���� */
#define CHAR_NULL		3	/* �Ȃɂ��Ȃ� */


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �e�픻��萔                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

const char CJis::JISESCDATA_ASCII[]				= "\x1b" "(B";
const char CJis::JISESCDATA_JISX0201Latin[]		= "\x1b" "(J";
const char CJis::JISESCDATA_JISX0201Latin_OLD[]	= "\x1b" "(H";
const char CJis::JISESCDATA_JISX0201Katakana[]	= "\x1b" "(I";
const char CJis::JISESCDATA_JISX0208_1978[]		= "\x1b" "$@";
const char CJis::JISESCDATA_JISX0208_1983[]		= "\x1b" "$B";
const char CJis::JISESCDATA_JISX0208_1990[]		= "\x1b" "&@""\x1b""$B";

// ������ enumJISEscSeqType �Ɉˑ� (charcode.h �ɂĒ�`����Ă���)
const int CJis::TABLE_JISESCLEN[] = {
	0,		// JISESC_UNKNOWN
	3,		// JISESC_ASCII
	3,		// JISESC_JISX0201Latin
	3,		// JISESC_JISX0201Latin_OLD
	3,		// JISESC_JISX0201Katakana
	3,		// JISESC_JISX0208_1978
	3,		// JISESC_JISX0208_1983
	6,		// JISESC_JISX0208_1990
};
const char* CJis::TABLE_JISESCDATA[] = {
	NULL,
	JISESCDATA_ASCII,
	JISESCDATA_JISX0201Latin,
	JISESCDATA_JISX0201Latin_OLD,
	JISESCDATA_JISX0201Katakana,
	JISESCDATA_JISX0208_1978,
	JISESCDATA_JISX0208_1983,
	JISESCDATA_JISX0208_1990,
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* E-Mail(JIS��Unicode)�R�[�h�ϊ� */
//2007.08.13 kobake �ǉ�
EConvertResult CJis::JISToUnicode(CMemory* pMem, bool base64decode)
{
	//$$ SJIS����Ă��邽�߁A���ʂɃf�[�^�������邩������܂���
	JIStoSJIS(pMem,base64decode);
	return CShiftJis::SJISToUnicode(pMem);		//	�G���[��Ԃ��悤�ɂ���B	2008/5/12 Uchi
}

EConvertResult CJis::UnicodeToJIS(CMemory* pMem)
{
	EConvertResult	res;

	//$$ SJIS����Ă��邽�߁A���ʂɃf�[�^�������邩������܂���
	res = CShiftJis::UnicodeToSJIS(pMem);
	if (res != RESULT_COMPLETE) {
		return res;				//	�G���[���������Ȃ�΃G���[��Ԃ��悤�ɂ���B	2008/5/12 Uchi
	}
	SJIStoJIS(pMem);

	return RESULT_COMPLETE;
}


/* JIS��SJIS�ϊ� */
long CJis::MemJIStoSJIS( unsigned char* pszSrc, long nSrcLen )
{
	int				i, j;
	char*			pszDes;
	unsigned short	sCode;

	pszDes = new char [nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	j = 0;
	for( i = 0; i < nSrcLen - 1; i++ ){
		sCode = (unsigned short)_mbcjistojms(
			(unsigned int)
			(((unsigned short)pszSrc[i	  ] << 8) |
			 ((unsigned short)pszSrc[i + 1]))
		);
		if( sCode != 0 ){
			pszDes[j	] = (unsigned char)(sCode >> 8);
			pszDes[j + 1] = (unsigned char)(sCode);
			++i;
			j+=2;
		}else{
			pszDes[j] = pszSrc[i];
			j++;
		}
	}
	pszDes[j] = 0;
	memcpy( pszSrc, pszDes, j );
	delete [] pszDes;
	return j;
}



/* �R�[�h�ϊ� JIS��SJIS */
void CJis::JIStoSJIS( CMemory* pMem, bool bMIMEdecode )
{
	//�f�[�^�擾
	CMemory cSrcMem(pMem->GetRawPtr(),pMem->GetRawLength());
	int						nSrcLen;
	const unsigned char*	pszSrc = (const unsigned char*)cSrcMem.GetRawPtr(&nSrcLen);

	CMemory cTmpMem;

	int				i;
	int				j;
	unsigned char*	pszDes;
	BOOL			bMIME = FALSE;
	int				nMEME_Selected;
	long			nWorkBgn;
	long			nWorkLen;
	unsigned char*	pszWork;

	int				nISOCode = CHAR_ASCII;
	int				nOldISOCode = nISOCode;
	BOOL			bFindESCSeq = FALSE;
	int				nESCSeqLen  = - 1; // �G�X�P�[�v�V�[�P���X�� - 1

	pszDes = new unsigned char [nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	j = 0;
	if( bMIMEdecode ){
		for( i = 0; i < nSrcLen; i++ ){
			if( i <= nSrcLen - 16 && '=' == pszSrc[i] ){
				if( 0 == auto_memicmp( "=?ISO-2022-JP?B?", (char*)&pszSrc[i], 16 ) ){
					nMEME_Selected = MIME_BASE64;
					bMIME = TRUE;
					i += 15;
					nWorkBgn = i + 1;
					continue;
				}
				if( 0 == auto_memicmp( "=?ISO-2022-JP?Q?", (char*)&pszSrc[i], 16 ) ){
					nMEME_Selected = MIME_QUOTED;
					bMIME = TRUE;
					i += 15;
					nWorkBgn = i + 1;
					continue;
				}
			}
			if( bMIME == TRUE ){
				if( i <= nSrcLen - 2  &&
					0 == memcmp( "?=", &pszSrc[i], 2 ) ){
					nWorkLen = i - nWorkBgn;
					pszWork = new unsigned char [nWorkLen + 1];
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[nWorkLen] = '\0';
					switch( nMEME_Selected ){
					case MIME_BASE64:
						// Base64�f�R�[�h
						nWorkLen = CConvert_Base64Decode::MemBASE64_Decode((char*)pszWork, nWorkLen, pszWork);
						break;
					case MIME_QUOTED:
						// Quoted-Printable�f�R�[�h
						nWorkLen = QuotedPrintable_Decode( (char*)pszWork, nWorkLen );
						break;
					}
					memcpy( &pszDes[j], pszWork, nWorkLen );
					bMIME = FALSE;
					j += nWorkLen;
					++i;
					delete [] pszWork;
					continue;
				}else{
					continue;
				}
			}
			pszDes[j] = pszSrc[i];
			j++;
		}
		if( bMIME ){
			nWorkBgn -= 16; // MIME�w�b�_�����̂܂܃R�s�[
			nWorkLen = i - nWorkBgn;
			memcpy( &pszDes[j], &pszSrc[nWorkBgn], nWorkLen );
			j += nWorkLen;
		}

		// ��ASCII�e�L�X�g�Ή����b�Z�[�W�w�b�_��MIME�R�[�h
		cTmpMem.SetRawData(pszDes,j);
		//memcpy( pszSrc, pszDes, j );

		nSrcLen = j;
		pszSrc=(const unsigned char*)cTmpMem.GetRawPtr();
	}

	nWorkBgn = 0;
	j = 0;
	for( i = 0; i < nSrcLen; i++ ){
		if( i <= nSrcLen - 3		&&
			pszSrc[i + 0] == 0x1b	&&
			pszSrc[i + 1] == '$'	&&
		   (pszSrc[i + 2] == 'B' || pszSrc[i + 2] == '@') ){

			bFindESCSeq = TRUE;
			nOldISOCode = nISOCode;
			nISOCode = CHAR_ZENKAKU;
			nESCSeqLen = 2;
		}
		else if( i <= nSrcLen - 3		&&
			pszSrc[i + 0] == 0x1b	&&
			pszSrc[i + 1] == '('	&&
			pszSrc[i + 2] == 'I' ){

			bFindESCSeq = TRUE;
			nOldISOCode = nISOCode;
			nISOCode = CHAR_8BITCODE;
			nESCSeqLen = 2;
		}
		else if( i <= nSrcLen - 3		&&
			pszSrc[i + 0] == 0x1b	&&
			pszSrc[i + 1] == '('	&&
		   (pszSrc[i + 2] == 'B' || pszSrc[i + 2] == 'J') ){
			
			bFindESCSeq = TRUE;
			nOldISOCode = nISOCode;
			nISOCode = CHAR_ASCII;
			nESCSeqLen = 2;
		}

		if( bFindESCSeq ){
			if( 0 < i - nWorkBgn ){
				if( CHAR_ZENKAKU == nOldISOCode ){
					nWorkLen = i - nWorkBgn;
					pszWork = new unsigned char [nWorkLen + 1];
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[nWorkLen] = '\0';
					// JIS��SJIS�ϊ�
					nWorkLen = MemJIStoSJIS( (unsigned char*)pszWork, nWorkLen );
					memcpy( &pszDes[j], pszWork, nWorkLen );
					j += nWorkLen;
					delete [] pszWork;
				}
			}
			i += nESCSeqLen;
			nWorkBgn = i + 1;
			bFindESCSeq = FALSE;
			continue;
		}
		else if( CHAR_ASCII == nISOCode ){
			pszDes[j] = pszSrc[i];
			j++;
			continue;
		}
		else if( CHAR_8BITCODE == nISOCode ){
			pszDes[j] = (unsigned char)0x80 | pszSrc[i];
			j++;
			continue;
		}
	}

	// ESCSeq��ASCII�ɖ߂�Ȃ������Ƃ��ɁC�f�[�^������Ȃ��悤��
	if( CHAR_ZENKAKU == nISOCode ){
		if( 0 < i - nWorkBgn ){
			nWorkBgn -= nESCSeqLen + 1; // ESCSeq���c���Ă�������
			nWorkLen = i - nWorkBgn;
			memcpy( &pszDes[j], &pszSrc[nWorkBgn], nWorkLen );
			j += nWorkLen;
		}
	}

	pMem->SetRawData(pszDes,j);
//	memcpy( pszSrc, pszDes, j );
 //	m_nDataLen = j;
	delete [] pszDes;
	return;
}



/* Quoted-Printable�f�R�[�h */
long CJis::QuotedPrintable_Decode( char* pszSrc, long nSrcLen )
{
	int			i;
	char*		pszDes;
	long		lDesSize;
	char		szHex[3];
	int			nHex;

	memset( szHex, 0, 3 );
	pszDes = new char [nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	lDesSize = 0;
	for( i = 0; i < nSrcLen; i++ ){
		if( pszSrc[i] == '=' ){
			szHex[0] = pszSrc[i + 1];
			szHex[1] = pszSrc[i + 2];
			sscanf( szHex, "%x", &nHex );
			pszDes[lDesSize] = (char)nHex;
			lDesSize++;
			i += 2;
		}else{
			pszDes[lDesSize] = pszSrc[i];
			lDesSize++;
		}
	}
	pszDes[lDesSize] = 0;
	memcpy( pszSrc, pszDes, lDesSize );
	delete [] pszDes;
	return lDesSize;
}






/* �R�[�h�ϊ� SJIS��JIS */
void CJis::SJIStoJIS( CMemory* pMem )
{
	void*	pBufJIS;
	int		nBufJISLen;
	CMemory	cMem;

	/* SJIS��JIS */
	StrSJIStoJIS( &cMem, (unsigned char *)pMem->GetRawPtr(), pMem->GetRawLength() );
	pBufJIS = cMem.GetRawPtr( &nBufJISLen );
	pMem->SetRawData( pBufJIS, nBufJISLen );
	return;
}



/*!	SJIS��JIS

	@date 2003.09.07 genta �s�v�ȃL���X�g����
*/
int CJis::StrSJIStoJIS( CMemory* pcmemDes, unsigned char* pszSrc, int nSrcLen )
{
//	BOOL bSJISKAN	= FALSE;
//	BOOL b8BITCODE	= FALSE;

	long			nWorkBgn;
	long			nWorkLen;
	int				i;
	int				j;
	unsigned char *	pszWork;
	int				nCharKindOld;
	int				nCharKind;
	int				bChange;
	nCharKind = CHAR_ASCII;		/* ASCII���� */
	nCharKindOld = nCharKind;
	bChange = FALSE;
//	/* ������� */
//	#define CHAR_ASCII		0	/* ASCII���� */
//	#define CHAR_8BITCODE	1	/* 8�r�b�g�R�[�h(���p�J�^�J�i�Ȃ�) */
//	#define CHAR_ZENKAKU	2	/* �S�p���� */

	pcmemDes->SetRawData("",0);
	pcmemDes->AllocBuffer( nSrcLen );
//	bSJISKAN  = FALSE;
	nWorkBgn = 0;
	for( i = 0;; i++ ){
		/* �������I������ */
		if( i >= nSrcLen ){
			nCharKind = CHAR_NULL;	/* �Ȃɂ��Ȃ� */
		}else
		// �������H
		if( ( i < nSrcLen - 1) && _IS_SJIS_1(pszSrc[i + 0]) && _IS_SJIS_2(pszSrc[i + 1]) ){
			nCharKind = CHAR_ZENKAKU;	/* �S�p���� */
//			++i;
		}
		else if( pszSrc[i] & (unsigned char)0x80 ){
			nCharKind = CHAR_8BITCODE;	/* 8�r�b�g�R�[�h(���p�J�^�J�i�Ȃ�) */
		}else{
			nCharKind = CHAR_ASCII;		/* ASCII���� */
		}
		/* ������ނ��ω����� */
		if( nCharKindOld != nCharKind ){
			if( CHAR_NULL != nCharKind ){
				bChange = TRUE;
			}

			nWorkLen = i - nWorkBgn;
			/* �ȑO�̕������ */
			switch( nCharKindOld ){
			case CHAR_ASCII:	/* ASCII���� */
				if( 0 < nWorkLen ){
					pcmemDes->AppendRawData( &(pszSrc[nWorkBgn]), nWorkLen );
				}
				break;
			case CHAR_8BITCODE:	/* 8�r�b�g�R�[�h(���p�J�^�J�i�Ȃ�) */
				if( 0 < nWorkLen ){
					pszWork = new unsigned char[nWorkLen + 1];
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[ nWorkLen ] = '\0';
					for( j = 0; j < nWorkLen; ++j ){
						pszWork[j] -= (unsigned char)0x80;
					}
					pcmemDes->AppendRawData( pszWork, nWorkLen );
					delete [] pszWork;
				}
				break;
			case CHAR_ZENKAKU:	/* �S�p���� */
				if( 0 < nWorkLen ){
					pszWork = new unsigned char[nWorkLen + 1];
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[ nWorkLen ] = '\0';
					// SJIS��JIS�ϊ�
					nWorkLen = MemSJIStoJIS( pszWork, nWorkLen );
					pcmemDes->AppendRawData( pszWork, nWorkLen );
					delete [] pszWork;
				}
				break;
			}
			/* �V����������� */
			switch( nCharKind ){
			case CHAR_ASCII:	/* ASCII���� */
				pcmemDes->_AppendSz( ESC_ASCII );
				break;
			case CHAR_NULL:		/* �Ȃɂ��Ȃ� */
				if( bChange &&					/* ���͕����킪�ω����� */
					nCharKindOld != CHAR_ASCII	/* ���O��ASCII�����ł͂Ȃ� */
				){
					pcmemDes->_AppendSz( ESC_ASCII );
				}
				break;
			case CHAR_8BITCODE:	/* 8�r�b�g�R�[�h(���p�J�^�J�i�Ȃ�) */
				pcmemDes->_AppendSz( ESC_8BIT );
				break;
			case CHAR_ZENKAKU:	/* �S�p���� */
				pcmemDes->_AppendSz( ESC_JIS );
				break;
			}
			nCharKindOld = nCharKind;
			nWorkBgn = i;
			if( nCharKind == CHAR_NULL ){	/* �Ȃɂ��Ȃ� */
				break;
			}
		}
		if( nCharKind == CHAR_ZENKAKU ){	/* �S�p���� */
			++i;
		}
	}
	return pcmemDes->GetRawLength();
}



/* SJIS��JIS�ϊ� */
long CJis::MemSJIStoJIS( unsigned char* pszSrc, long nSrcLen )
{
	int				i, j;
	char *			pszDes;
	unsigned short	sCode;

	pszDes = new char[nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	j = 0;
	for( i = 0; i < nSrcLen - 1; i++ ){
		//	Oct. 3, 2002 genta IBM�g�������Ή�
		sCode = _mbcjmstojis_ex( pszSrc + i );
		if( sCode != 0 ){
			pszDes[j	] = (unsigned char)(sCode >> 8);
			pszDes[j + 1] = (unsigned char)(sCode);
			++i;
			j += 2;
		}else{
			pszDes[j] = pszSrc[i];
			j++;
		}
	}
	pszDes[j] = 0;
	memcpy( pszSrc, pszDes, j );
	delete [] pszDes;
	return j;
}



// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
EConvertResult CJis::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst)
{
	static CMemory	cCharBuffer;
	EConvertResult	res;
	int				i;
	TCHAR*			pd; 
	unsigned char*	ps; 

	// 2008/6/21 Uchi
	if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispUniInJis) {
		// Unicode�ŕ\��
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst);
	}

	// 1�����f�[�^�o�b�t�@
	cCharBuffer.SetRawData("",0);
	cCharBuffer.AppendRawData( cSrc, sizeof(wchar_t));

	// EUC-JP �ϊ�
	res = UnicodeToJIS(&cCharBuffer);
	if (res != RESULT_COMPLETE) {
		return res;
	}

	// Hex�ϊ�
	bool	bInEsc;
	bInEsc = false;
	pd = pDst;
	for (i = cCharBuffer.GetRawLength(), ps = (unsigned char*)cCharBuffer.GetRawPtr(); i >0; i--, ps ++) {
		if (*ps == 0x1B) {
			bInEsc = true;
		}
		else if (bInEsc) {
			if (*ps >= 'A' && *ps <='Z') {
				bInEsc = false;
			}
		}
		else {
			auto_sprintf( pd, _T("%02x"), *ps);
			pd += 2;
		}
	}

	return RESULT_COMPLETE;
}
