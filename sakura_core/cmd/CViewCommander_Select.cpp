/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�I���n/��`�I���n)�֐��Q

	2012/12/20	CViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2005, Moca
	Copyright (C) 2007, kobake, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"


/* ���݈ʒu�̒P��I�� */
bool CViewCommander::Command_SELECTWORD( void )
{
	CLayoutRange sRange;
	CLogicInt	nIdx;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}
	const CLayout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		return false;	//	�P��I���Ɏ��s
	}
	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
	nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	if( GetDocument()->m_cLayoutMgr.WhereCurrentWord(	GetCaret().GetCaretLayoutPos().GetY2(), nIdx, &sRange, NULL, NULL ) ){

		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( sRange.GetFrom().GetY2() );
		sRange.SetFromX( m_pCommanderView->LineIndexToColmn( pcLayout, sRange.GetFrom().x ) );
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( sRange.GetTo().GetY2() );
		sRange.SetToX( m_pCommanderView->LineIndexToColmn( pcLayout, sRange.GetTo().x ) );
		*/

		/* �I��͈͂̕ύX */
		//	2005.06.24 Moca
		m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

		/* �P��̐擪�ɃJ�[�\�����ړ� */
		GetCaret().MoveCursor( sRange.GetTo(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		/* �I��̈�`�� */
		m_pCommanderView->GetSelectionInfo().DrawSelectArea();
		return true;	//	�P��I���ɐ����B
	}
	else {
		return false;	//	�P��I���Ɏ��s
	}
}



/* ���ׂđI�� */
void CViewCommander::Command_SELECTALL( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}

	/* �擪�փJ�[�\�����ړ� */
	//	Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	//	Jul. 29, 2006 genta �I���ʒu�̖����𐳊m�Ɏ擾����
	//	�}�N������擾�����ꍇ�ɐ������͈͂��擾�ł��Ȃ�����
	//int nX, nY;
	CLayoutRange sRange;
	sRange.SetFrom(CLayoutPoint(0,0));
	GetDocument()->m_cLayoutMgr.GetEndLayoutPos(sRange.GetToPointer());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* �I��̈�`�� */
	m_pCommanderView->GetSelectionInfo().DrawSelectArea();
}



/*!	1�s�I��
	@brief �J�[�\���ʒu��1�s�I������
	@param lparam [in] �}�N������g�p����g���t���O�i�g���p�ɗ\��j

	note ���s�P�ʂőI�����s���B

	@date 2007.11.15 nasukoji	�V�K�쐬
*/
void CViewCommander::Command_SELECTLINE( int lparam )
{
	// ���s�P�ʂ�1�s�I������
	Command_GOLINETOP( FALSE, 0x9 );	// �����s���Ɉړ�

	m_pCommanderView->GetSelectionInfo().m_bBeginLineSelect = TRUE;		// �s�P�ʑI��

	CLayoutPoint ptCaret;

	// �ŉ��s�i�����s�j�łȂ�
	if(GetCaret().GetCaretLogicPos().y < GetDocument()->m_cDocLineMgr.GetLineCount() ){
		// 1�s��̕����s���烌�C�A�E�g�s�����߂�
		GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, GetCaret().GetCaretLogicPos().y + 1), &ptCaret );

		// �J�[�\�������̕����s���ֈړ�����
		m_pCommanderView->MoveCursorSelecting( ptCaret, TRUE );

		// �ړ���̃J�[�\���ʒu���擾����
		ptCaret = GetCaret().GetCaretLayoutPos().Get();
	}else{
		// �J�[�\�����ŉ��s�i���C�A�E�g�s�j�ֈړ�����
		m_pCommanderView->MoveCursorSelecting( CLayoutPoint(CLayoutInt(0), GetDocument()->m_cLayoutMgr.GetLineCount()), TRUE );
		Command_GOLINEEND( TRUE, FALSE );	// �s���Ɉړ�

		// �I��������̂������i[EOF]�݂̂̍s�j���͑I����ԂƂ��Ȃ�
		if(( ! m_pCommanderView->GetSelectionInfo().IsTextSelected() )&&
		   ( GetCaret().GetCaretLogicPos().y >= GetDocument()->m_cDocLineMgr.GetLineCount() ))
		{
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}

	if( m_pCommanderView->GetSelectionInfo().m_bBeginLineSelect ){
		// �͈͑I���J�n�s�E�J�������L��
		m_pCommanderView->GetSelectionInfo().m_sSelect.SetTo( ptCaret );
		m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetTo( ptCaret );
	}

	return;
}



/* �͈͑I���J�n */
void CViewCommander::Command_BEGIN_SELECT( void )
{
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̃J�[�\���ʒu����I�����J�n���� */
		m_pCommanderView->GetSelectionInfo().BeginSelectArea();
	}

	//	���b�N�̉����؂�ւ�
	if ( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ) {
		m_pCommanderView->GetSelectionInfo().m_bSelectingLock = FALSE;	/* �I����Ԃ̃��b�N���� */
	}
	else {
		m_pCommanderView->GetSelectionInfo().m_bSelectingLock = TRUE;	/* �I����Ԃ̃��b�N */
	}
	if( GetSelect().IsOne() ){
		GetCaret().m_cUnderLine.CaretUnderLineOFF(true);
	}
	m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
	return;
}



/* ��`�͈͑I���J�n */
void CViewCommander::Command_BEGIN_BOXSELECT( void )
{
	if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH ){	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		return;
	}

//@@@ 2002.01.03 YAZAKI �͈͑I�𒆂�Shift+F6�����s����ƑI��͈͂��N���A����Ȃ����ɑΏ�
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}

	/* ���݂̃J�[�\���ʒu����I�����J�n���� */
	m_pCommanderView->GetSelectionInfo().BeginSelectArea();

	m_pCommanderView->GetSelectionInfo().m_bSelectingLock = true;	/* �I����Ԃ̃��b�N */
	m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);	/* ��`�͈͑I�� */

	m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
	GetCaret().m_cUnderLine.CaretUnderLineOFF(true);
	return;
}
