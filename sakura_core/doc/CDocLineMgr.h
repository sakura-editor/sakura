/*!	@file
	@brief �s�f�[�^�̊Ǘ�

	@author Norio Nakatani
	@date 1998/3/5  �V�K�쐬
	@date 2001/06/23 N.Nakatani WhereCurrentWord_2()�ǉ� static�����o
	@date 2001/12/03 hor ������(bookmark)�@�\�ǉ��ɔ����֐��ǉ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, hor, genta
	Copyright (C) 2002, aroka, MIK, hor
	Copyright (C) 2003, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDocLineMgr;

#ifndef _CDOCLINEMGR_H_
#define _CDOCLINEMGR_H_

#include <windows.h>
#include "_main/global.h" // 2002/2/10 aroka
#include "CEol.h" // 2002/2/10 aroka
class CDocLine; // 2002/2/10 aroka
class CMemory; // 2002/2/10 aroka
class CBregexp; // 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "charset/CCodeBase.h"
#include "CDocFile.h"
#include "doc/CDocLine.h"

struct DocLineReplaceArg {
	CLogicRange		sDelRange;			//!< �폜�͈́B���W�b�N�P�ʁB
	CNativeW*		pcmemDeleted;		//!< �폜���ꂽ�f�[�^��ۑ�
	CLogicInt		nDeletedLineNum;	//!< �폜�����s�̑���
	const wchar_t*	pInsData;			//!< �}������f�[�^
	int				nInsDataLen;		//!< �}������f�[�^�̒���
	CLogicInt		nInsLineNum;		//!< �}���ɂ���đ������s�̐�
	CLogicPoint		ptNewPos;			//!< �}�����ꂽ�����̎��̈ʒu
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
//2007.09.30 kobake WhereCurrentWord_2 �� CWordParse �Ɉړ�
class SAKURA_CORE_API CDocLineMgr{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CDocLineMgr();
	~CDocLineMgr();

	//���
	CLogicInt GetLineCount() const{ return m_nLines; }	//!< �S�s����Ԃ�

	//�s�f�[�^�ւ̃A�N�Z�X
	CDocLine* GetLine( CLogicInt nLine ) const;						//!< �w��s���擾
	CDocLine* GetDocLineTop() const { return m_pDocLineTop; }		//!< �擪�s���擾
	CDocLine* GetDocLineBottom() const { return m_pDocLineBot; }	//!< �ŏI�s���擾

	//�s�f�[�^�̊Ǘ�
	CDocLine* InsertNewLine(CDocLine* pPos);	//!< pPos�̒��O�ɐV�����s��}��
	CDocLine* AddNewLine();						//!< �ŉ����ɐV�����s��}��
	void DeleteAllLine();						//!< �S�Ă̍s���폜����
	void DeleteLine( CDocLine* );				//!< �s�̍폜

	//�f�o�b�O
	void DUMP();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �����⏕                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	void _Init();
	// -- -- �`�F�[���֐� -- -- // 2007.10.11 kobake �쐬
	void _PushBottom(CDocLine* pDocLineNew);             //!< �ŉ����ɑ}��
	void _InsertBeforePos(CDocLine* pDocLineNew, CDocLine* pPos); //!< pPos�̒��O�ɑ}��
	void _InsertAfterPos(CDocLine* pDocLineNew, CDocLine* pPos); //!< pPos�̒���ɑ}��

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	CDocLine*	m_pDocLineTop;		//!< �ŏ��̍s
	CDocLine*	m_pDocLineBot;		//!< �Ō�̍s(��1�s�����Ȃ��ꍇ��m_pDocLineTop�Ɠ������Ȃ�)
	CLogicInt	m_nLines;			//!< �S�s��

public:
	//$$ kobake��: �ȉ��A��΂ɐ؂藣�������i�Œ�؂藣���Ȃ��Ă��A�ϐ��̈Ӗ����R�����g�Ŗ��m�ɋL���ׂ��j�ϐ��Q
	mutable CDocLine*	m_pDocLineCurrent;	//!< ���A�N�Z�X���̌��݈ʒu
	mutable CLogicInt	m_nPrevReferLine;
	mutable CDocLine*	m_pCodePrevRefer;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINEMGR_H_ */



