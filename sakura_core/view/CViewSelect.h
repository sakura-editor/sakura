#pragma once

class CEditView;

#include "basis/SakuraBasis.h"


class CViewSelect{
public:
	CEditView* GetEditView(){ return m_pcEditView; }
	const CEditView* GetEditView() const{ return m_pcEditView; }

public:
	CViewSelect(CEditView* pcEditView);
	void CopySelectStatus(CViewSelect* pSelect) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �I��͈͂̕ύX                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DisableSelectArea( bool bDraw ); //!< ���݂̑I��͈͂��I����Ԃɖ߂�

	void BeginSelectArea();								// ���݂̃J�[�\���ʒu����I�����J�n����
	void ChangeSelectAreaByCurrentCursor( const CLayoutPoint& ptCaretPos );			// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
	void ChangeSelectAreaByCurrentCursorTEST( const CLayoutPoint& ptCaretPos, CLayoutRange* pSelect );// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX

	//!�I��͈͂��w�肷��(���_���I��)
	// 2005.06.24 Moca
	void SetSelectArea( const CLayoutRange& sRange )
	{
		m_sSelectBgn.Set(sRange.GetFrom());
		m_sSelect = sRange;
	}

	//!�P��I���J�n
	void SelectBeginWord()
	{
		m_bBeginSelect     = true;				/* �͈͑I�� */
		m_bBeginBoxSelect  = false;			/* ��`�͈͑I�𒆂łȂ� */
		m_bBeginLineSelect = false;			/* �s�P�ʑI�� */
		m_bBeginWordSelect = true;			/* �P��P�ʑI�� */
	}

	//!��`�I���J�n
	void SelectBeginBox()
	{
		m_bBeginSelect     = true;			/* �͈͑I�� */
		m_bBeginBoxSelect  = true;		/* ��`�͈͑I�� */
		m_bBeginLineSelect = false;		/* �s�P�ʑI�� */
		m_bBeginWordSelect = false;		/* �P��P�ʑI�� */
	}

	//!��̑I���J�n
	void SelectBeginNazo()
	{
		m_bBeginSelect     = true;			/* �͈͑I�� */
//		m_bBeginBoxSelect  = FALSE;		/* ��`�͈͑I�𒆂łȂ� */
		m_bBeginLineSelect = false;		/* �s�P�ʑI�� */
		m_bBeginWordSelect = false;		/* �P��P�ʑI�� */
	}

	//!�͈͑I���I��
	void SelectEnd()
	{
		m_bBeginSelect = false;
	}

	//!m_bBeginBoxSelect��ݒ�B
	void SetBoxSelect(bool b)
	{
		m_bBeginBoxSelect = b;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �`��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DrawSelectArea() const;		//!< �w��s�̑I��̈�̕`��
	void DrawSelectAreaLine(			//!< �w��s�̑I��̈�̕`��
		HDC					hdc,		//!< [in] �`��̈��Device Context Handle
		CLayoutInt			nLineNum,	//!< [in] �`��Ώۍs(���C�A�E�g�s)
		const CLayoutRange&	sRange		//!< [in] �I��͈�(���C�A�E�g�P��)
	) const;

	//! �I�����f�[�^�̍쐬	2005.07.09 genta
	void PrintSelectionInfoMsg() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ��Ԏ擾                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//! �e�L�X�g���I������Ă��邩
	// 2002/03/29 Azumaiya �C�����C���֐���
	bool IsTextSelected() const
	{
		return m_sSelect.IsValid();
//		return 0!=(
//			~((DWORD)(m_sSelect.nLineFrom | m_sSelect.nLineTo | m_sSelect.nColmFrom | m_sSelect.nColmTo)) >> 31
//			);
	}

	//! �e�L�X�g�̑I�𒆂�
	// 2002/03/29 Azumaiya �C�����C���֐���
	bool IsTextSelecting() const
	{
		// �W�����v�񐔂����炵�āA��C�ɔ���B
		return m_bSelectingLock || IsTextSelected();
	}

	//!�}�E�X�őI�𒆂�
	bool IsMouseSelecting() const
	{
		return m_bBeginSelect;
	}

	//!��`�I�𒆂�
	bool IsBoxSelecting() const
	{
		return m_bBeginBoxSelect;
	}


private:
	//�Q��
	CEditView*	m_pcEditView;

public:


	mutable bool	m_bDrawSelectArea;		// �I��͈͂�`�悵����	// 02/12/13 ai

	// �I�����
	bool	m_bSelectingLock;		// �I����Ԃ̃��b�N
private:
	bool	m_bBeginSelect;			// �͈͑I��
	bool	m_bBeginBoxSelect;		// ��`�͈͑I��
	bool	m_bSelectAreaChanging;	// �I��͈͕ύX��
	int		m_nLastSelectedByteLen;	// �O��I�����̑I���o�C�g��

public:
	bool	m_bBeginLineSelect;		// �s�P�ʑI��
	bool	m_bBeginWordSelect;		// �P��P�ʑI��

	// �I��͈͂�ێ����邽�߂̕ϐ��Q
	// �����͂��ׂĐ܂�Ԃ��s�ƁA�܂�Ԃ�����ێ����Ă���B
	CLayoutRange m_sSelectBgn; //�͈͑I��(���_)
	CLayoutRange m_sSelect;    //�͈͑I��
	CLayoutRange m_sSelectOld; //�͈͑I��Old

	CMyPoint	m_ptMouseRollPosOld;	// �}�E�X�͈͑I��O��ʒu(XY���W)
};

/*
m_sSelectOld�ɂ���
	DrawSelectArea()�Ɍ��݂̑I��͈͂������č����̂ݕ`�悷�邽�߂̂���
	���݂̑I��͈͂�Old�փR�s�[������ŐV�����I��͈͂�Select�ɐݒ肵��
	DrawSelectArea()���Ăт������ƂŐV�����͈͂��`�����D
*/
