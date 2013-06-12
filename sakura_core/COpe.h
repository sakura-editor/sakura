/*!	@file
	@brief �ҏW����v�f

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class COpe;

#ifndef SAKURA_COPE_H_
#define SAKURA_COPE_H_



// �A���h�D�o�b�t�@�p ����R�[�h
enum EOpeCode {
	OPE_UNKNOWN		= 0,
	OPE_INSERT		= 1,
	OPE_DELETE		= 2,
	OPE_MOVECARET	= 3,
};

class CLineData {
public:
	CNativeW cmemLine;
	int nSeq;
	void swap(CLineData& o){
		std::swap(cmemLine, o.cmemLine);
		std::swap(nSeq, o.nSeq);
	}
};

namespace std {
template <>
	inline void swap(CLineData& n1, CLineData& n2)
	{
		n1.swap(n2);
	}
}

typedef std::vector<CLineData> COpeLineData;

/*!
	�ҏW����v�f
	
	Undo�̂��߂ɂɑ���菇���L�^���邽�߂ɗp����B
	1�I�u�W�F�N�g���P�̑����\���B
*/
//2007.10.17 kobake ����R���h�����߁A�f�[�^���|�C���^�ł͂Ȃ��C���X�^���X���̂Ŏ��悤�ɕύX
class COpe {
public:
	COpe(EOpeCode eCode = OPE_UNKNOWN);		/* COpe�N���X�\�z */
	virtual ~COpe();	/* COpe�N���X���� */

	void DUMP( void );	/* �ҏW����v�f�̃_���v */

	EOpeCode	GetCode() const{ return m_nOpe; }

private:
	EOpeCode	m_nOpe;						//!< ������

public:
	CLogicPoint	m_ptCaretPos_PHY_Before;	//!< �L�����b�g�ʒu�B�����P�ʁB			[����]
	CLogicPoint	m_ptCaretPos_PHY_After;		//!< �L�����b�g�ʒu�B�����P�ʁB			[����]
};

//!�폜
class CDeleteOpe : public COpe{
public:
	CDeleteOpe() : COpe(OPE_DELETE)
	{
		m_ptCaretPos_PHY_To.Set(CLogicInt(0),CLogicInt(0));
	}
public:
	CLogicPoint	m_ptCaretPos_PHY_To;		//!< ����O�̃L�����b�g�ʒu�B�����P�ʁB	[DELETE]
	COpeLineData	m_pcmemData;				//!< ����Ɋ֘A����f�[�^				[DELETE/INSERT]
	int				m_nOrgSeq;
};

//!�}��
class CInsertOpe : public COpe{
public:
	CInsertOpe() : COpe(OPE_INSERT) { }
public:
	COpeLineData	m_pcmemData;				//!< ����Ɋ֘A����f�[�^				[DELETE/INSERT]
	int				m_nOrgSeq;
};

//!�L�����b�g�ړ�
class CMoveCaretOpe : public COpe{
public:
	CMoveCaretOpe() : COpe(OPE_MOVECARET) { }
	CMoveCaretOpe(const CLogicPoint& ptBefore, const CLogicPoint& ptAfter)
	: COpe(OPE_MOVECARET)
	{
		m_ptCaretPos_PHY_Before = ptBefore;
		m_ptCaretPos_PHY_After = ptAfter;
	}
};









///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_COPE_H_ */



