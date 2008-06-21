#include "stdafx.h"
#include "CEuc.h"
#include "CShiftJis.h"
#include "charset/charcode.h"
#include <mbstring.h>
#include "codeutil.h"
//#include <MLang.h>

/* EUC��Unicode�R�[�h�ϊ� */
//2007.08.13 kobake �ǉ�
EConvertResult CEuc::EUCToUnicode(CMemory* pMem)
{
	//$$ SJIS����Ă���̂Ŗ��ʂɃf�[�^�����������H
	EUCToSJIS(pMem);
	return CShiftJis::SJISToUnicode(pMem);		//	�G���[��Ԃ��悤�ɂ���B	2008/5/12 Uchi
}

EConvertResult CEuc::UnicodeToEUC(CMemory* pMem)
{
	EConvertResult	res;

	//$$ SJIS����Ă���̂Ŗ��ʂɃf�[�^�����������H
	res = CShiftJis::UnicodeToSJIS(pMem);
	if (res != RESULT_COMPLETE) {
		return res;				//	�G���[���������Ȃ�΃G���[��Ԃ��悤�ɂ���B	2008/5/12 Uchi
	}
	SJISToEUC(pMem);

	return RESULT_COMPLETE;
}


/************************************************************************
*
*�y�֐����z
*	EUCToSJIS
*
*�y�@�\�z
*	�w��͈͂̃o�b�t�@����EUC�����R�[�h
*	�������SJIS�S�p�R�[�h�ɕϊ�����B	//Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
*	���p�����͕ϊ������ɂ��̂܂܎c���B
*
*	���䕶��CRLF�ȊO�̃o�C�i���R�[�h���������Ă���ꍇ�ɂ͌��ʂ��s���
*	�Ȃ邱�Ƃ�����̂Œ��ӁB
*	�o�b�t�@�̍Ō�Ɋ����R�[�h��1�o�C�g�ڂ���������ƍ���
*
*�y���́z	�Ȃ�
*
*�y�߂�l�z	�Ȃ�
*
************************************************************************/
/* EUC��SJIS�R�[�h�ϊ� */
void CEuc::EUCToSJIS( CMemory* pMem )
{
	//�f�[�^�擾
	int				nBufLen;
	const char*		pBuf = (const char*)pMem->GetRawPtr(&nBufLen);

	CMemory cmemTmp;

	int				nPtr = 0L;
	int				nPtrDes = 0L;
	char*			pszDes = new char[nBufLen];
	unsigned int	sCode;

	while( nPtr < nBufLen ){
		if( (unsigned char)pBuf[nPtr] == (unsigned char)0x8e && nPtr < nBufLen - 1 ){
			/* ���p�J�^�J�i */
			pszDes[nPtrDes] = pBuf[nPtr + 1];
			nPtrDes++;
			nPtr += 2;
		}
		/* EUC�����R�[�h��? */
		else if( nPtr < nBufLen - 1 && CEuc::IsEucKan1(pBuf[nPtr]) && CEuc::IsEucKan2(pBuf[nPtr + 1L]) ){
			/* �ʏ��JIS�R�[�h�ɕϊ� */
			char jis[2];
			jis[0] = pBuf[nPtr	  ] & 0x7f;
			jis[1] = pBuf[nPtr + 1L] & 0x7f;

			/* SJIS�R�[�h�ɕϊ� */	//Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
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


/* SJIS��EUC�R�[�h�ϊ� */
void CEuc::SJISToEUC( CMemory* pMem )
{
	//�f�[�^�擾
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
				/* ���p�J�^�J�i */
				pDes[nDesIdx	] = (unsigned char)0x8e;
				pDes[nDesIdx + 1] = pBuf[i];
				nDesIdx += 2;
			}else{
				pDes[nDesIdx] = pBuf[i];
				nDesIdx++;
			}
		}else
		if( nCharChars == 2 ){
			/* �S�p���� */
			//	Oct. 3, 2002 genta IBM�g�������Ή�
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


// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
EConvertResult CEuc::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst)
{
	static CMemory	cCharBuffer;
	EConvertResult	res;
	int				i;
	TCHAR*			pd; 
	unsigned char*	ps; 

	// 2008/6/21 Uchi
	if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispUniInEuc) {
		// Unicode�ŕ\��
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst);
	}

	// 1�����f�[�^�o�b�t�@
	cCharBuffer.SetRawData("",0);
	cCharBuffer.AppendRawData( cSrc, sizeof(wchar_t));

	// EUC-JP �ϊ�
	res = UnicodeToEUC(&cCharBuffer);
	if (res != RESULT_COMPLETE) {
		return res;
	}

	// Hex�ϊ�
	for (i = cCharBuffer.GetRawLength(), ps = (unsigned char*)cCharBuffer.GetRawPtr(), pd = pDst; i >0; i--, ps ++, pd += 2) {
		auto_sprintf( pd, _T("%02x"), *ps);
	}

	return RESULT_COMPLETE;
}
