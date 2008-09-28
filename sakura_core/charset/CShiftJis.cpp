#include "stdafx.h"
#include "CShiftJis.h"
#include "charset/charcode.h"

void CShiftJis::S_GetEol(CMemory* pcmemEol, EEolType eEolType)
{
	static const struct{
		const char* szData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		"",			0,	// EOL_NONE
		"\x0d\x0a",	2,	// EOL_CRLF
		"\x0a",		1,	// EOL_LF
		"\x0d",		1,	// EOL_CR
	};
	pcmemEol->SetRawData(aEolTable[eEolType].szData,aEolTable[eEolType].nLen);
}


//! �w�肵���ʒu�̕��������o�C�g��������Ԃ�
/*!
	@param[in] pData �ʒu�����߂���������̐擪
	@param[in] nDataLen ������
	@param[in] nIdx �ʒu(0�I���W��)
	@retval 1  1�o�C�g����
	@retval 2  2�o�C�g����
	@retval 0  �G���[

	@date 2005-09-02 D.S.Koba �쐬

	@note nIdx�͗\�ߕ����̐擪�ʒu�Ƃ킩���Ă��Ȃ���΂Ȃ�Ȃ��D
	2�o�C�g������2�o�C�g�ڂ�nIdx�ɗ^����Ɛ��������ʂ������Ȃ��D
*/
int CShiftJis::GetSizeOfChar( const char* pData, int nDataLen, int nIdx )
{
	if( nIdx >= nDataLen ){
		return 0;
	}else if( nIdx == (nDataLen - 1) ){
		return 1;
	}
	
	if( _IS_SJIS_1( reinterpret_cast<const unsigned char*>(pData)[nIdx] )
			&& _IS_SJIS_2( reinterpret_cast<const unsigned char*>(pData)[nIdx+1] ) ){
		return 2;
	}
	return 1;
}


/* �R�[�h�ϊ� SJIS��Unicode */
EConvertResult CShiftJis::SJISToUnicode( CMemory* pMem )
{
	//�\�[�X�擾
	int nSrcLen;
	const char* pSrc = (const char*)pMem->GetRawPtr(&nSrcLen);

	//�ϊ���o�b�t�@�T�C�Y
	int nDstLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0
	);

	//�ϊ���o�b�t�@�m��
	wchar_t* pDst = new wchar_t[nDstLen+1];

	//�ϊ�
	nDstLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		pDst,
		nDstLen
	);
	pDst[nDstLen]=L'\0';

	//pMem���X�V
	pMem->SetRawData( pDst, nDstLen * sizeof(wchar_t) );

	//��n��
	delete[] pDst;

	return RESULT_COMPLETE; //SJIS��UNICODE�Ńf�[�^�͎���Ȃ� (���߂̎d���͐l�ɂ�邪�A�����ł̓f�[�^���������ƒ�߂�)
}


/* �R�[�h�ϊ� Unicode��SJIS */
EConvertResult CShiftJis::UnicodeToSJIS( CMemory* pMem )
{
	//�\�[�X�擾
	int nSrcLen;
	const wchar_t* pSrc=(const wchar_t*)pMem->GetRawPtr(&nSrcLen);
	nSrcLen/=sizeof(wchar_t); //�����P�ʂɕϊ�

	//�ϊ���o�b�t�@�T�C�Y
	int nDstLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0,
		NULL,
		NULL
	);

	//�ϊ���o�b�t�@�m��
	char* pDst = new char[nDstLen+1];

	//�ϊ�
	BOOL bLost = TRUE;
	nDstLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		pDst,
		nDstLen,
		NULL,
		&bLost
	);
	pDst[nDstLen] = '\0';

	//pMem���X�V
	pMem->SetRawData( pDst, nDstLen );

	//��n��
	delete[] pDst;

	//����
	if(bLost){
		return RESULT_LOSESOME;
	}
	else{
		return RESULT_COMPLETE;
	}
}


// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
EConvertResult CShiftJis::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst)
{
	//�ϊ���o�b�t�@�m��
	unsigned char sCvt[8];

	// 2008/6/21 Uchi
	if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispUniInSjis) {
		// Unicode�ŕ\��
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst);
	}

	//�ϊ�
	BOOL bLost = FALSE;
	int nDstLen = WideCharToMultiByte(
		CP_SJIS,
		0,
		cSrc,
		1,
		(char*)sCvt,
		8,
		NULL,
		&bLost
	);

	//����
	if (bLost) {
		return RESULT_LOSESOME;
	}

	int		i;
	TCHAR*	p; 
	for (i = 0, p = pDst; i < nDstLen; i++, p += 2) {
		auto_sprintf( p, _T("%02x"), sCvt[i]);
	}

	return RESULT_COMPLETE;
}
