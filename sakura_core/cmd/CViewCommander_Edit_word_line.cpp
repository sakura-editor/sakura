/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�ҏW�n �P��/�s�P��)�֐��Q

	2012/12/16	CViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, �����
	Copyright (C) 2005, Moca
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
#include "COpeBlk.h"/// 2002/2/3 aroka �ǉ�


//�P��̍��[�܂ō폜
void CViewCommander::Command_WordDeleteToStart( void )
{
	/* ��`�I����Ԃł͎��s�s�\(��������蔲������) */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			ErrorBeep();
			return;
		}
	}

	// �P��̍��[�Ɉړ�
	CViewCommander::Command_WORDLEFT( true );
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		//$$ �����͖�肪�����炵���ł����B
		//   ������ COpe �����A�Ӗ��t��(EOpeCode�w��)������Ă��Ȃ��̂ŁA
		//   ���߂ĉ��炩�̖��O�t�������Ă����Ȃ��ƁA�Ӑ}���ǂݎ��܂���B
		COpe*	pcOpe = new COpe;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			GetSelect().GetTo(),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		pcOpe->m_ptCaretPos_PHY_After = pcOpe->m_ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu

		// ����̒ǉ�
		GetOpeBlk()->AppendOpe( pcOpe );
	}

	// �폜
	m_pCommanderView->DeleteData( true );
}



//�P��̉E�[�܂ō폜
void CViewCommander::Command_WordDeleteToEnd( void )
{

	/* ��`�I����Ԃł͎��s�s�\((��������蔲������)) */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			ErrorBeep();
			return;
		}
	}
	/* �P��̉E�[�Ɉړ� */
	CViewCommander::Command_WORDRIGHT( true );
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}
	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		CMoveCaretOpe*	pcOpe = new CMoveCaretOpe();
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			GetSelect().GetFrom(),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		pcOpe->m_ptCaretPos_PHY_After = pcOpe->m_ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu
		/* ����̒ǉ� */
		GetOpeBlk()->AppendOpe( pcOpe );
	}
	/* �폜 */
	m_pCommanderView->DeleteData( true );
}



//�P��؂���
void CViewCommander::Command_WordCut( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
		Command_CUT();
		return;
	}
	//���݈ʒu�̒P��I��
	Command_SELECTWORD();
	/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
	if ( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		//	�P��I���őI���ł��Ȃ�������A���̕�����I�Ԃ��Ƃɒ���B
		Command_RIGHT( true, false, false );
	}
	Command_CUT();
	return;
}



//�P��폜
void CViewCommander::Command_WordDelete( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* �폜 */
		m_pCommanderView->DeleteData( true );
		return;
	}
	//���݈ʒu�̒P��I��
	Command_SELECTWORD();
	/* �폜 */
	m_pCommanderView->DeleteData( true );
	return;
}



//�s���܂Ő؂���(���s�P��)
void CViewCommander::Command_LineCutToStart( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
		Command_CUT();
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;
	GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, pCLayout->GetLogicLineNo()), &ptPos );
	if( GetCaret().GetCaretLayoutPos() == ptPos ){
		ErrorBeep();
		return;
	}

	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	CLayoutRange sRange(ptPos,GetCaret().GetCaretLayoutPos());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/*�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
	Command_CUT();
}



//�s���܂Ő؂���(���s�P��)
void CViewCommander::Command_LineCutToEnd( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
		Command_CUT();
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	if( EOL_NONE == pCLayout->GetDocLineRef()->GetEol() ){	/* ���s�R�[�h�̎�� */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}
	else{
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL() - pCLayout->GetDocLineRef()->GetEol().GetLen(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if( GetCaret().GetCaretLayoutPos().GetY2() == ptPos.y && GetCaret().GetCaretLayoutPos().GetX2() >= ptPos.x ){
		ErrorBeep();
		return;
	}

	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	CLayoutRange sRange(GetCaret().GetCaretLayoutPos(),ptPos);
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/*�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
	Command_CUT();
}



//�s���܂ō폜(���s�P��)
void CViewCommander::Command_LineDeleteToStart( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		m_pCommanderView->DeleteData( true );
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, pCLayout->GetLogicLineNo()), &ptPos );
	if( GetCaret().GetCaretLayoutPos() == ptPos ){
		ErrorBeep();
		return;
	}

	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	CLayoutRange sRange(ptPos,GetCaret().GetCaretLayoutPos());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* �I��̈�폜 */
	m_pCommanderView->DeleteData( true );
}



//�s���܂ō폜(���s�P��)
void CViewCommander::Command_LineDeleteToEnd( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		m_pCommanderView->DeleteData( true );
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	if( EOL_NONE == pCLayout->GetDocLineRef()->GetEol() ){	/* ���s�R�[�h�̎�� */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else{
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL() - pCLayout->GetDocLineRef()->GetEol().GetLen(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if( GetCaret().GetCaretLayoutPos().GetY2() == ptPos.y && GetCaret().GetCaretLayoutPos().GetX2() >= ptPos.x ){
		ErrorBeep();
		return;
	}

	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	CLayoutRange sRange( GetCaret().GetCaretLayoutPos(), ptPos );
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* �I��̈�폜 */
	m_pCommanderView->DeleteData( true );
}



//�s�؂���(�܂�Ԃ��P��)
void CViewCommander::Command_CUT_LINE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		ErrorBeep();
		return;
	}

	const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().y );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}

	// 2007.10.04 ryoji �����ȑf��
	m_pCommanderView->CopyCurLine(
		GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy,
		EOL_UNKNOWN,
		GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste
	);
	Command_DELETE_LINE();
	return;
}



/* �s�폜(�܂�Ԃ��P��) */
void CViewCommander::Command_DELETE_LINE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	const CLayout*	pcLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		ErrorBeep();
		return;
	}
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}
	GetSelect().SetFrom(CLayoutPoint(CLayoutInt(0),GetCaret().GetCaretLayoutPos().GetY2()    ));	//�͈͑I���J�n�ʒu
	GetSelect().SetTo  (CLayoutPoint(CLayoutInt(0),GetCaret().GetCaretLayoutPos().GetY2() + 1));	//�͈͑I���I���ʒu

	CLayoutPoint ptCaretPos_OLD = GetCaret().GetCaretLayoutPos();

	Command_DELETE();
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL != pcLayout ){
		// 2003-04-30 �����
		// �s�폜������A�t���[�J�[�\���łȂ��̂ɃJ�[�\���ʒu���s�[���E�ɂȂ�s��Ή�
		// �t���[�J�[�\�����[�h�łȂ��ꍇ�́A�J�[�\���ʒu�𒲐�����
		if( !GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode ) {
			CLogicInt nIndex;

			CLayoutInt tmp;
			nIndex = m_pCommanderView->LineColmnToIndex2( pcLayout, ptCaretPos_OLD.GetX2(), &tmp );
			ptCaretPos_OLD.x=tmp;

			if (ptCaretPos_OLD.x > 0) {
				ptCaretPos_OLD.x--;
			} else {
				ptCaretPos_OLD.x = m_pCommanderView->LineIndexToColmn( pcLayout, nIndex );
			}
		}
		/* ����O�̈ʒu�փJ�[�\�����ړ� */
		GetCaret().MoveCursor( ptCaretPos_OLD, true );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			/* ����̒ǉ� */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),
					GetCaret().GetCaretLogicPos()
				)
			);
		}
	}
	return;
}



/* �s�̓�d��(�܂�Ԃ��P��) */
void CViewCommander::Command_DUPLICATELINE( void )
{
	int				bCRLF;
	int				bAddCRLF;
	CNativeW		cmemBuf;
	const CLayout*	pcLayout;

	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );
	}

	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		/* ����̒ǉ� */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}

	CLayoutPoint ptCaretPosOld = GetCaret().GetCaretLayoutPos() + CLayoutPoint(0,1);

	//�s���Ɉړ�(�܂�Ԃ��P��)
	Command_GOLINETOP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, 0x1 /* �J�[�\���ʒu�Ɋ֌W�Ȃ��s���Ɉړ� */ );

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		/* ����̒ǉ� */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}



	/* ��d���������s�𒲂ׂ�
	||	�E���s�ŏI����Ă���
	||	�E���s�ŏI����Ă��Ȃ�
	||	�E�ŏI�s�ł���
	||	���܂�Ԃ��łȂ�
	||	�E�ŏI�s�łȂ�
	||	���܂�Ԃ��ł���
	*/
	bCRLF = ( EOL_NONE == pcLayout->GetLayoutEol() ) ? FALSE : TRUE;

	bAddCRLF = FALSE;
	if( !bCRLF ){
		if( GetCaret().GetCaretLayoutPos().GetY2() == GetDocument()->m_cLayoutMgr.GetLineCount() - 1 ){
			bAddCRLF = TRUE;
		}
	}

	cmemBuf.SetString( pcLayout->GetPtr(), pcLayout->GetLengthWithoutEOL() + pcLayout->GetLayoutEol().GetLen() );	//	��pcLayout->GetLengthWithEOL()�́AEOL�̒�����K��1�ɂ���̂Ŏg���Ȃ��B
	if( bAddCRLF ){
		/* ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾 */
		CEol cWork = GetDocument()->m_cDocEditor.GetNewLineCode();
		cmemBuf.AppendString( cWork.GetValue2(), cWork.GetLen() );
	}

	/* ���݈ʒu�Ƀf�[�^��}�� */
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		&ptLayoutNew,
		true
	);

	/* �J�[�\�����ړ� */
	GetCaret().MoveCursor( ptCaretPosOld, true );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();


	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		/* ����̒ǉ� */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}
	return;
}
