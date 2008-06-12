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
};
