#pragma once

//�萔
enum EConvertResult{
	RESULT_COMPLETE, //!< �f�[�^���������ƂȂ��ϊ������������B
	RESULT_LOSESOME, //!< �ϊ��������������A�ꕔ�̃f�[�^������ꂽ�B
	RESULT_FAILURE,  //!< ���炩�̌����ɂ�莸�s�����B
};

/*!
	�����R�[�h���N���X�B
	
	�����Ō����u����R�[�h�v�Ƃ́A
	CCodeBase���p�������q�N���X����߂�A��ӂ̕����R�[�h�̂��Ƃł��B
*/
class CCodeBase{
public:
	virtual ~CCodeBase(){}
//	virtual bool IsCode(const CMemory* pMem){return false;}  //!< ����R�[�h�ł����true
	virtual EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst)=0; //!< ����R�[�h �� UNICODE    �ϊ�
	virtual EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst)=0; //!< UNICODE    �� ����R�[�h �ϊ�
};

