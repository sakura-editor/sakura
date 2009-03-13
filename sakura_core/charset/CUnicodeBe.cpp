//	���̍s�͕��������΍��p�ł��D�����Ȃ��ł�������

#include "stdafx.h"
#include "CUnicodeBe.h"
#include "CEol.h"

#include "codechecker.h"


void CUnicodeBe::GetBom(CMemory* pcmemBom)
{
	static const BYTE UTF16BE_BOM[]={0xFE,0xFF};
	pcmemBom->SetRawData(UTF16BE_BOM, sizeof(UTF16BE_BOM));
}

void CUnicodeBe::GetEol(CMemory* pcmemEol, EEolType eEolType)
{
	static const struct{
		const void* pData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		"",					0 * sizeof(wchar_t),	// EOL_NONE
		"\x00\x0d\x00\x0a",	2 * sizeof(wchar_t),	// EOL_CRLF
		"\x00\x0a",			1 * sizeof(wchar_t),	// EOL_LF
		"\x00\x0d",			1 * sizeof(wchar_t),	// EOL_CR
	};
	pcmemEol->SetRawData(aEolTable[eEolType].pData,aEolTable[eEolType].nLen);
}


#if 0 // util/string_ex2.cpp �ֈړ�

/*
	�s�[�q�̎�ނ𒲂ׂ�UnicodeBE��
	@param pszData �����Ώە�����ւ̃|�C���^
	@param nDataLen �����Ώە�����̒���(wchar_t�̒���)
	@return ���s�R�[�h�̎�ށB�I�[�q��������Ȃ������Ƃ���EOL_NONE��Ԃ��B
*/
static EEolType GetEOLTypeUniBE( const wchar_t* pszData, int nDataLen )
{
	/*! �s�I�[�q�̃f�[�^�̔z��(UnicodeBE��) 2000.05.30 Moca */
	static const wchar_t* aEolTable[EOL_TYPE_NUM] = {
		L"",									// EOL_NONE
		(const wchar_t*)"\x00\x0d\x00\x0a\x00",	// EOL_CRLF
		(const wchar_t*)"\x00\x0a\x00",			// EOL_LF
		(const wchar_t*)"\x00\x0d\x00"			// EOL_CR
	};

	/* ���s�R�[�h�̒����𒲂ׂ� */

	for( int i = 1; i < EOL_TYPE_NUM; ++i ){
		CEol cEol((EEolType)i);
		if( cEol.GetLen()<=nDataLen && 0==auto_memcmp(pszData,aEolTable[i],cEol.GetLen()) ){
			return gm_pnEolTypeArr[i];
		}
	}
	return EOL_NONE;
}


/*!
	GetNextLine��wchar_t��(�r�b�N�G���f�B�A���p)
	GetNextLine���쐬
	static �����o�֐�
*/
const wchar_t* GetNextLineWB(
	const wchar_t*	pData,	//!< [in]	����������
	int			nDataLen,	//!< [in]	����������̕�����
	int*		pnLineLen,	//!< [out]	1�s�̕�������Ԃ�������EOL�͊܂܂Ȃ�
	int*		pnBgn,		//!< [i/o]	����������̃I�t�Z�b�g�ʒu
	CEol*		pcEol		//!< [i/o]	EOL
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		// ���s�R�[�h��������
		if( pData[i] == (wchar_t)0x0a00 || pData[i] == (wchar_t)0x0d00 ){
			// �s�I�[�q�̎�ނ𒲂ׂ�
			pcEol->SetType( GetEOLTypeUniBE( &pData[i], nDataLen - i ) );
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}

#endif
