#include "stdafx.h"
#include "CShiftJis.h"
#include "charcode.h"

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
		CP_ACP,
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
		CP_ACP,
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
		CP_ACP,
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
	BOOL bLost = FALSE;
	nDstLen = WideCharToMultiByte(
		CP_ACP,
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





