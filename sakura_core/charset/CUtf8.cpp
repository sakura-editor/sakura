#include "stdafx.h"
#include "CUtf8.h"

//! BOM�f�[�^�擾
void CUtf8::GetBom(CMemory* pcmemBom)
{
	static const BYTE UTF8_BOM[]={0xEF,0xBB,0xBF};
	pcmemBom->SetRawData(UTF8_BOM, sizeof(UTF8_BOM));
}

//! UTF-8��Unicode�R�[�h�ϊ�
// 2007.08.13 kobake �쐬
EConvertResult CUtf8::UTF8ToUnicode( const CMemory& cSrcMem, CNativeW* pDstMem )
{
	//�f�[�^�擾
	int nSrcLen;
	const char* pData = (char*)cSrcMem.GetRawPtr(&nSrcLen);

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
EConvertResult CUtf8::UnicodeToUTF8( const CNativeW& cSrcMem, CMemory* pDstMem )
{
	//�f�[�^�擾
	int nSrcLen;
	const wchar_t* pSrc = cSrcMem.GetStringPtr(&nSrcLen);

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

// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/21 Uchi
EConvertResult CUtf8::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst)
{
	static CNativeW	cBuffSrc;
	static CMemory	cBuffDst;
	EConvertResult	res;
	int				i;
	TCHAR*			pd; 
	unsigned char*	ps; 

	if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispUtf8Codepoint) {
		// Unicode�ŕ\��
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst);
	}

	// 1�����f�[�^�o�b�t�@
	if (WCODE::IsUTF16High(cSrc[0]) && iSLen >= 2 && WCODE::IsUTF16Low(cSrc[1])) {
		cBuffSrc.SetStringW(cSrc,2);
	}
	else {
		cBuffSrc.SetStringW(cSrc,1);
	}
	cBuffDst.SetRawData("",0);

	// RTF-8 �ϊ�
	res = UnicodeToUTF8(cBuffSrc, &cBuffDst);
	if (res != RESULT_COMPLETE) {
		return res;
	}

	// Hex�ϊ�
	for (i = cBuffDst.GetRawLength(), ps = (unsigned char*)cBuffDst.GetRawPtr(), pd = pDst; i >0; i--, ps ++, pd += 2) {
		auto_sprintf( pd, _T("%02x"), *ps);
	}

	return RESULT_COMPLETE;
}
