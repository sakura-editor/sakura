#include "stdafx.h"
#include "CUtf8.h"

//! UTF-8��Unicode�R�[�h�ϊ�
// 2007.08.13 kobake �쐬
EConvertResult CUtf8::UTF8ToUnicode( const CMemory* pSrcMem, CNativeW* pDstMem )
{
	//�f�[�^�擾
	int nSrcLen;
	const char* pData = (char*)pSrcMem->GetRawPtr(&nSrcLen);

	//�K�v�ȃo�b�t�@�T�C�Y�𒲂ׂ�
	size_t nDstLen=MultiByteToWideChar(
		CP_UTF8,
		0,
		pData,
		nSrcLen,
		NULL,
		0
	);

	//�o�b�t�@�m��
	pDstMem->Clear();
	pDstMem->AllocStringBuffer(nDstLen+1);
	wchar_t* pDst = pDstMem->GetStringPtr();

	//�ϊ�
	nDstLen=MultiByteToWideChar(
		CP_UTF8,
		0,
		pData,
		nSrcLen,
		pDst,
		nDstLen
	);
	pDst[nDstLen]=L'\0';

	//CNativeW�X�V
	pDstMem->_SetStringLength(nDstLen);

	return RESULT_COMPLETE; //���v���Ǝv��
}


//! �R�[�h�ϊ� Unicode��UTF-8
EConvertResult CUtf8::UnicodeToUTF8( const CNativeW* pSrcMem, CMemory* pDstMem )
{
	//�f�[�^�擾
	int nSrcLen;
	const wchar_t* pSrc = pSrcMem->GetStringPtr(&nSrcLen);

	//�K�v�ȃo�b�t�@�T�C�Y�𒲂ׂ�
	size_t nDstLen=WideCharToMultiByte(
		CP_UTF8,
		0,
		pSrc,
		nSrcLen,
		NULL,
		0,
		NULL,
		NULL
	);

	//�o�b�t�@�m��
	pDstMem->Clean();
	pDstMem->AllocBuffer( (nDstLen+1) * sizeof(char) );
	char* pDst = reinterpret_cast<char*>(pDstMem->GetRawPtr());

	//�ϊ�
	nDstLen=WideCharToMultiByte(
		CP_UTF8,
		0,
		pSrc,
		nSrcLen,
		pDst,
		nDstLen,
		NULL,
		NULL
	);
	pDst[nDstLen]='\0';

	//CMemory�X�V
	pDstMem->_SetRawLength(nDstLen);

	return RESULT_COMPLETE; //���v���Ǝv��
}

