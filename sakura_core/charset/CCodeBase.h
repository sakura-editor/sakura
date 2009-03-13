#pragma once

//�萔
enum EConvertResult{
	RESULT_COMPLETE, //!< �f�[�^���������ƂȂ��ϊ������������B
	RESULT_LOSESOME, //!< �ϊ��������������A�ꕔ�̃f�[�^������ꂽ�B
	RESULT_FAILURE,  //!< ���炩�̌����ɂ�莸�s�����B
};

#include "CEol.h"
#include "mem/CNativeW.h"
class CMemory;
class CNativeW;

/*!
	�����R�[�h���N���X�B
	
	�����Ō����u����R�[�h�v�Ƃ́A
	CCodeBase���p�������q�N���X����߂�A��ӂ̕����R�[�h�̂��Ƃł��B
*/
class CCodeBase{
public:
	virtual ~CCodeBase(){}
//	virtual bool IsCode(const CMemory* pMem){return false;}  //!< ����R�[�h�ł����true

	//�����R�[�h�ϊ�
	virtual EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst)=0;	//!< ����R�[�h �� UNICODE    �ϊ�
	virtual EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst)=0;	//!< UNICODE    �� ����R�[�h �ϊ�

	//�t�@�C���`��
	virtual void GetBom(CMemory* pcmemBom);											//!< BOM�f�[�^�擾
	virtual void GetEol(CMemory* pcmemEol, EEolType eEolType)=0;					//!< ���s�f�[�^�擾

	// �����R�[�h�\���p		2008/6/9 Uchi
	virtual EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst);			//!< UNICODE �� Hex �ϊ�

	// �ϊ��G���[�����i�P�o�C�g <-> U+D800 ���� U+D8FF�j
	static int BinToText( const unsigned char*, const int, unsigned short* );
	static int TextToBin( const unsigned short );

	// MIME Header �f�R�[�_
	static bool MIMEHeaderDecode( const char*, const int, CMemory*, const ECodeType );
};

/*!
	�o�C�i���P�o�C�g�� U+DC00 ���� U+DCFF �܂łɑΉ��t����
*/
inline int CCodeBase::BinToText( const unsigned char *pSrc, const int nLen, unsigned short *pDst )
{
	int i;

	for( i = 0; i < nLen; ++i ){
		pDst[i] = static_cast<unsigned short>(pSrc[i]) + 0xdc00;
	}

	return i;
}


/*!
	U+DC00 ���� U+DCFF ����o�C�i��1�o�C�g�𕜌�
*/
inline int CCodeBase::TextToBin( const unsigned short cSrc )
{
	return static_cast<int>((cSrc - 0xdc00) & 0x00ff);
}
