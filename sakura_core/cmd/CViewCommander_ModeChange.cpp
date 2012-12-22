/*!	@file
@brief CViewCommander�N���X�̃R�}���h(���[�h�؂�ւ��n)�֐��Q

	2012/12/15	CViewCommander.cpp,CViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2007, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <htmlhelp.h>
#include "CViewCommander.h"

#include "view/CEditView.h"


/*! �}���^�㏑�����[�h�؂�ւ�

	@date 2005.10.02 genta InsMode�֐���
*/
void CViewCommander::Command_CHGMOD_INS( void )
{
	/* �}�����[�h���H */
	if( m_pCommanderView->IsInsMode() ){
		m_pCommanderView->SetInsMode( false );
	}else{
		m_pCommanderView->SetInsMode( true );
	}
	/* �L�����b�g�̕\���E�X�V */
	GetCaret().ShowEditCaret();
	/* �L�����b�g�̍s���ʒu��\������ */
	GetCaret().ShowCaretPosInfo();
}



//	from CViewCommander_New.cpp
/*! ���͂�����s�R�[�h��ݒ�

	@author moca
	@date 2003.06.23 �V�K�쐬
*/
void CViewCommander::Command_CHGMOD_EOL( EEolType e ){
	if( EOL_NONE < e && e < EOL_CODEMAX  ){
		GetDocument()->m_cDocEditor.SetNewLineCode( e );
		// �X�e�[�^�X�o�[���X�V���邽��
		// �L�����b�g�̍s���ʒu��\������֐����Ăяo��
		GetCaret().ShowCaretPosInfo();
	}
}



/** �e�탂�[�h�̎�����
	@param whereCursorIs �I�����L�����Z��������A�L�����b�g���ǂ��ɒu�����B0=�������Ȃ��B1=����B2=�E���B
*/
void CViewCommander::Command_CANCEL_MODE( int whereCursorIs )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ) {
		// �I��������̃J�[�\���ʒu�����߂�B
		CLayoutPoint ptTo ;
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ) { // ��`�I���ł̓L�����b�g�����s�̌��Ɏ��c����Ȃ��悤�ɁA����B
			/* 2�_��Ίp�Ƃ����`�����߂� */
			CLayoutRange rcSel;
			TwoPointToRange(
				&rcSel,
				GetSelect().GetFrom(),	// �͈͑I���J�n
				GetSelect().GetTo()		// �͈͑I���I��
			);
			ptTo = rcSel.GetFrom();
		} else if( 1 == whereCursorIs ) { // ����
			ptTo = GetSelect().GetFrom();
		} else if( 2 == whereCursorIs ) { // �E��
			ptTo = GetSelect().GetTo();
		} else {
			ptTo = GetCaret().GetCaretLayoutPos();
		}

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );

		/* �J�[�\�����ړ� */
		if( ptTo.y >= GetDocument()->m_cLayoutMgr.GetLineCount() ){
			/* �t�@�C���̍Ō�Ɉړ� */
			Command_GOFILEEND(FALSE);
		} else {
			GetCaret().MoveCursor( ptTo, TRUE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
	}else{
		// 2011.12.05 Moca �I�𒆂̖��I����Ԃł�Lock�̉����ƕ`�悪�K�v
		if( m_pCommanderView->GetSelectionInfo().IsTextSelecting()
				|| m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			m_pCommanderView->GetSelectionInfo().DisableSelectArea(true);
			GetCaret().m_cUnderLine.CaretUnderLineON(true);
			m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
		}
	}
}
