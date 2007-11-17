//2007.09.30 kobake CDocLineMgr ���番��

#pragma once

//! ������ގ��ʎq
enum ECharKind{
	CK_NULL,			//!< NULL
	CK_TAB,				//!< �^�u 0x9<=c<=0x9
	CK_CR,				//!< CR = 0x0d 
	CK_LF,				//!< LF = 0x0a 

	CK_SPACE,			//!< ���p�̃X�y�[�X 0x20<=c<=0x20
	CK_CSYM,			//!< ���ʎq�Ɏg�p�\�ȕ��� (�p�����A�A���_�[�X�R�A)
	CK_KATA,			//!< ���p�̃J�^�J�i 0xA1<=c<=0xFD
	CK_ETC,				//!< ���p�̂��̑�

	CK_ZEN_SPACE,		//!< �S�p�X�y�[�X
	CK_ZEN_NOBASU,		//!< �L�΂��L�� 0x815B<=c<=0x815B '�['
	CK_ZEN_CSYM,		//!< �S�p�ŁA���ʎq�Ɏg�p�\�ȕ��� (�p�����A�A���_�[�X�R�A)

	CK_ZEN_KIGO,		//!< �S�p�̋L��
	CK_HIRA,			//!< �Ђ炪��
	CK_ZEN_KATA,		//!< �S�p�J�^�J�i
	CK_GREEK,			//!< �M���V������
	CK_ZEN_ROS,			//!< ���V�A����:
	CK_ZEN_SKIGO,		//!< �S�p�̓���L��
	CK_ZEN_ETC,			//!< �S�p�̂��̑��i�����Ȃǁj
};

class CWordParse{
public:
	//2001.06.23 N.Nakatani
	//2007.09.30 kobake     CDocLineMgr����ړ�
	/*!
		@brief ���݈ʒu�̒P��͈̔͂𒲂ׂ� static�����o
		@author N.Nakatani
		@retval true	���� ���݈ʒu�̃f�[�^�́u�P��v�ƔF������B
		@retval false	���s ���݈ʒu�̃f�[�^�́u�P��v�Ƃ͌�������Ȃ��C������B
	*/
	static bool WhereCurrentWord_2(
		const wchar_t*	pLine,			//[in]  ���ׂ郁�����S�̂̐擪�A�h���X
		CLogicInt		nLineLen,		//[in]  ���ׂ郁�����S�̗̂L����
		CLogicInt		nIdx,			//[out] �����J�n�n�_:pLine����̑��ΓI�Ȉʒu
		CLogicInt*		pnIdxFrom,		//[out] �P�ꂪ���������ꍇ�́A�P��̐擪�C���f�b�N�X��Ԃ��B
		CLogicInt*		pnIdxTo,		//[out] �P�ꂪ���������ꍇ�́A�P��̏I�[�̎��̃o�C�g�̐擪�C���f�b�N�X��Ԃ��B
		CNativeW*		pcmcmWord,		//[out] �P�ꂪ���������ꍇ�́A���ݒP���؂�o���Ďw�肳�ꂽCMemory�I�u�W�F�N�g�Ɋi�[����B��񂪕s�v�ȏꍇ��NULL���w�肷��B
		CNativeW*		pcmcmWordLeft	//[out] �P�ꂪ���������ꍇ�́A���ݒP��̍��Ɉʒu����P���؂�o���Ďw�肳�ꂽCMemory�I�u�W�F�N�g�Ɋi�[����B��񂪕s�v�ȏꍇ��NULL���w�肷��B
	);

	//! ���݈ʒu�̕����̎�ނ𒲂ׂ�
	static ECharKind WhatKindOfChar(
		const wchar_t*	pData,
		int				pDataLen,
		int				nIdx
	);

	//	pLine�i�����FnLineLen�j�̕����񂩂玟�̒P���T���B�T���n�߂�ʒu��nIdx�Ŏw��B
	static bool SearchNextWordPosition(
		const wchar_t*	pLine,
		CLogicInt		nLineLen,
		CLogicInt		nIdx,		//	����
		CLogicInt*		pnColmNew,	//	���������ʒu
		BOOL			bStopsBothEnds	//	�P��̗��[�Ŏ~�܂�
	);
};

SAKURA_CORE_API BOOL IsURL( const wchar_t*, int, int* );/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
SAKURA_CORE_API BOOL IsMailAddress( const wchar_t*, int, int* );	/* ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ� */
