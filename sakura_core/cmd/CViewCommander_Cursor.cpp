/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�J�[�\���ړ��n)�֐��Q

	2012/12/17	CViewCommander.cpp,CViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, asa-o, hor
	Copyright (C) 2002, hor, YAZAKI, oak
	Copyright (C) 2003, Moca
	Copyright (C) 2004, Moca, genta, fotomo
	Copyright (C) 2006, genta
	Copyright (C) 2007, kobake, maru
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
#include "CMarkMgr.h"/// 2002/2/3 aroka �ǉ�
#include "mem/CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI


void CViewCommander::Command_MOVECURSOR(CLogicPoint pos, int option)
{
	if( pos.GetX2() < 0 || pos.GetY2() < 0 ){
		ErrorBeep();
		return;
	}
	CLayoutPoint layoutPos;
	GetDocument()->m_cLayoutMgr.LogicToLayout(pos, &layoutPos);
	Command_MOVECURSORLAYOUT(layoutPos, option);
}



void CViewCommander::Command_MOVECURSORLAYOUT(CLayoutPoint pos, int option)
{
	if( pos.GetX2() < 0 || pos.GetY2() < 0 ){
		ErrorBeep();
		return;
	}

	bool bSelect = (option & 0x01) == 0x01;
	bool bBoxSelect = (option & 0x02) == 0x02;

	if( bSelect || bBoxSelect ){
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			if( bBoxSelect ){
				Command_BEGIN_BOXSELECT();
			}else{
				m_pCommanderView->GetSelectionInfo().BeginSelectArea();
			}
		}
	}else{
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}else if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			m_pCommanderView->GetSelectionInfo().SetBoxSelect(false);
		}
	}

	GetCaret().MoveCursor( pos, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
	// �I��
	if( bSelect || bBoxSelect ){
		m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
	}
}


/////////////////////////////////// �ȉ��̓R�}���h�Q (Oct. 17, 2000 jepro note) ///////////////////////////////////////////

/*! �J�[�\����ړ� */
int CViewCommander::Command_UP( bool bSelect, bool bRepeat, int lines )
{
	//	From Here Oct. 24, 2001 genta
	if( lines != 0 ){
		GetCaret().Cursor_UPDOWN( CLayoutInt(lines), FALSE );
		return 1;
	}
	//	To Here Oct. 24, 2001 genta


	int		nRepeat = 0;

	/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	if( !GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScroll_Smooth ){
		CLayoutInt i;
		if( !bRepeat ){
			i = CLayoutInt(-1);
		}else{
			i = -1 * GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum;	/* �L�[���s�[�g���̃X�N���[���s�� */
		}
		GetCaret().Cursor_UPDOWN( i, bSelect );
		nRepeat = -1 * (Int)i;
	}
	else{
		++nRepeat;
		if( GetCaret().Cursor_UPDOWN( CLayoutInt(-1), bSelect )!=0 && bRepeat ){
			for( int i = 0; i < GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum - 1; ++i ){		/* �L�[���s�[�g���̃X�N���[���s�� */
				::UpdateWindow( m_pCommanderView->GetHwnd() );	//	YAZAKI
				GetCaret().Cursor_UPDOWN( CLayoutInt(-1), bSelect );
				++nRepeat;
			}
		}
	}
	return nRepeat;
}



/* �J�[�\�����ړ� */
int CViewCommander::Command_DOWN( bool bSelect, bool bRepeat )
{
	int		nRepeat;
	nRepeat = 0;
	/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	if( !GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScroll_Smooth ){
		CLayoutInt i;
		if( !bRepeat ){
			i = CLayoutInt(1);
		}else{
			i = GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum;	/* �L�[���s�[�g���̃X�N���[���s�� */
		}
		GetCaret().Cursor_UPDOWN( i, bSelect );
		nRepeat = (Int)i;
	}else{
		++nRepeat;
		if( GetCaret().Cursor_UPDOWN(CLayoutInt(1),bSelect)!=0 && bRepeat ){
			for( int i = 0; i < GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum - 1; ++i ){	/* �L�[���s�[�g���̃X�N���[���s�� */
				//	�����ōĕ`��B
				::UpdateWindow( m_pCommanderView->GetHwnd() );	//	YAZAKI
				GetCaret().Cursor_UPDOWN( CLayoutInt(1), bSelect );
				++nRepeat;
			}
		}
	}
	return nRepeat;
}



/*! @brief �J�[�\�����ړ�

	@date 2004.03.28 Moca EOF�����̍s�ȍ~�̓r���ɃJ�[�\��������Ɨ�����o�O�C���D
			pcLayout == NULL���L�����b�g�ʒu���s���ȊO�̏ꍇ��
			2��if�̂ǂ���ɂ����Ă͂܂�Ȃ����C���̂��Ƃ�MoveCursor�ɂēK����
			�ʒu�Ɉړ���������D
*/
int CViewCommander::Command_LEFT( bool bSelect, bool bRepeat )
{
	bool	bUnderlineDoNotOFF = true;	// �A���_�[���C�����������Ȃ�
	if( bSelect ){
		bUnderlineDoNotOFF = false;		//�I����ԂȂ�A���_�[���C���������s��
	}
	int		nRepeat = bRepeat ? 2 : 1;
	int		nRes;
	for( int nRepCount = 0; nRepCount < nRepeat; ++nRepCount ) {

		if( bSelect && ! m_pCommanderView->GetSelectionInfo().IsTextSelected() ) {
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			m_pCommanderView->GetSelectionInfo().BeginSelectArea();
		}
		if( ! bSelect ){
			if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ) {
				this->Command_CANCEL_MODE( 1 );
				nRes = 1;
				continue; // �I���̃L�����Z���ō��ړ��� 1����B���̌�̈ړ������̓X�L�b�v����B
			}else if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
				m_pCommanderView->GetSelectionInfo().SetBoxSelect(false);
			}
		}
		// (���ꂩ�狁�߂�)�J�[�\���̈ړ���B
		CLayoutPoint ptPos(CLayoutInt(0), GetCaret().GetCaretLayoutPos().GetY2());

		/* ���ݍs�̃f�[�^���擾 */
		const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
		/* �J�[�\�������[�ɂ��� */
		if( GetCaret().GetCaretLayoutPos().GetX2() == (pcLayout ? pcLayout->GetIndent() : CLayoutInt(0))) {
			if( 0 < GetCaret().GetCaretLayoutPos().GetY2()
			   && ! m_pCommanderView->GetSelectionInfo().IsBoxSelecting()
			) {
				// �O�̃��C�A�E�g�s�́A�܂�Ԃ������O�܂��͉��s�����̎�O�Ɉړ�����B
				pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() - CLayoutInt(1) );
				CMemoryIterator it( pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace() );
				while( !it.end() ){
					it.scanNext();
					if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
						ptPos.x += it.getColumnDelta();
						break;
					}
					it.addDelta();
				}
				ptPos.x += it.getColumn() - it.getColumnDelta();
				ptPos.y --;
			} else {
				nRes = 0;
				break; // ����ȏ㍶�ɓ����ʁB
			}
			bUnderlineDoNotOFF = false;	//�s���ς��̂ŃA���_�[���C������������
		}
		//  2004.03.28 Moca EOF�����̍s�ȍ~�̓r���ɃJ�[�\��������Ɨ�����o�O�C��
		else if( pcLayout ) {
			CMemoryIterator it( pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace() );
			while( !it.end() ){
				it.scanNext();
				if ( it.getColumn() + it.getColumnDelta() > GetCaret().GetCaretLayoutPos().GetX2() - 1 ){
					ptPos.x += it.getColumnDelta();
					break;
				}
				it.addDelta();
			}
			ptPos.x += it.getColumn() - it.getColumnDelta();
			//	Oct. 18, 2002 YAZAKI
			if( it.getIndex() >= pcLayout->GetLengthWithEOL() ){
				ptPos.x = GetCaret().GetCaretLayoutPos().GetX2() - CLayoutInt(1);
			}
		}

		GetCaret().MoveCursor( ptPos, TRUE, _CARETMARGINRATE, bUnderlineDoNotOFF );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ) {
			/*	���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX�D
				2004.04.02 Moca 
				�L�����b�g�ʒu���s���������ꍇ��MoveCursor�̈ړ����ʂ�
				�����ŗ^�������W�Ƃ͈قȂ邱�Ƃ����邽�߁C
				ptPos�̑���Ɏ��ۂ̈ړ����ʂ��g���悤�ɁD
			*/
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
		}
		nRes = 1;
	}
	return nRes;
}



/* �J�[�\���E�ړ� */
void CViewCommander::Command_RIGHT( bool bSelect, bool bIgnoreCurrentSelection, bool bRepeat )
{
	bool	bUnderlineDoNotOFF = true;	// �A���_�[���C�����������Ȃ�
	if( bSelect ){
		bUnderlineDoNotOFF = false;		//�I����ԂȂ�A���_�[���C���������s��
	}
	int nRepeat = bRepeat ? 2 : 1; // �ړ������
	for( int nRepCount = 0; nRepCount < nRepeat; ++nRepCount ) {
		// 2003.06.28 Moca [EOF]�݂̂̍s�ɃJ�[�\��������Ƃ��ɉE�������Ă��I���������ł��Ȃ�����
		// �Ή����邽�߁A���ݍs�̃f�[�^���擾���ړ�
		if( ! bIgnoreCurrentSelection ) {
			if( bSelect && ! m_pCommanderView->GetSelectionInfo().IsTextSelected() ) {
				/* ���݂̃J�[�\���ʒu����I�����J�n���� */
				m_pCommanderView->GetSelectionInfo().BeginSelectArea();
			}
			if( ! bSelect ){
				if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ) {
					this->Command_CANCEL_MODE( 2 );
					continue; // �I���̃L�����Z���ŉE�ړ��� 1����B���̌�̈ړ������̓X�L�b�v����B
				}else if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
					m_pCommanderView->GetSelectionInfo().SetBoxSelect(false);
				}
			}
		}
//		2003.06.28 Moca [EOF]�݂̂̍s�ɃJ�[�\��������Ƃ��ɉE�������Ă��I���������ł��Ȃ����ɑΉ�

		// (���ꂩ�狁�߂�)�J�[�\���̈ړ���B
		CLayoutPoint ptTo( 0, 0 );
		const CLayoutPoint ptCaret = GetCaret().GetCaretLayoutPos();

		/* ���ݍs�̃f�[�^���擾 */
		const CLayout* const pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptCaret.y );
		//	2004.04.02 EOF�ȍ~�ɃJ�[�\�����������Ƃ��ɉE�������Ă������N���Ȃ������̂��AEOF�Ɉړ�����悤��
		if( pcLayout )
		{
			// �L�����b�g�ʒu�̃��C�A�E�g�s�ɂ��āB
			const CLayoutInt x_wrap = pcLayout->CalcLayoutWidth( GetDocument()->m_cLayoutMgr ); // ���s�����A�܂��͐܂�Ԃ��̈ʒu�B
			const bool wrapped = EOL_NONE == pcLayout->GetLayoutEol(); // �܂�Ԃ��Ă��邩�A���s�����ŏI����Ă��邩�B����ɂ�� x_wrap�̈Ӗ����ς��B
			const bool nextline_exists = pcLayout->GetNextLayout() || pcLayout->GetLayoutEol() != EOL_NONE; // EOF�݂̂̍s���܂߁A�L�����b�g���ړ��\�Ȏ��s�����݂��邩�B

			// ���݂̃L�����b�g�̉E�̈ʒu( to_x )�����߂�B
			CMemoryIterator it( pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace() );
			for( ; ! it.end(); it.scanNext(), it.addDelta() ) {
				if( ptCaret.x < it.getColumn() ) {
					break;
				}
			}
			const CLayoutInt to_x = std::max( it.getColumn(), ptCaret.x + 1 );

			// �L�����b�g�̉E�[( x_max )�ƁA�����ł̈���( on_x_max )�����߂�B
			CLayoutInt x_max;
			enum {
				STOP,
				MOVE_NEXTLINE_IMMEDIATELY, // �E�[�Ɏ~�܂炸���̍s���Ɉړ�����B(�܂�Ԃ��Ȃ�)
				MOVE_NEXTLINE_NEXTTIME, // �E�[�Ɏ~�܂�A���Ɏ��̍s���Ɉړ�����B(���s�𒴂���Ƃ��Ȃ�)
				MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT // �E�[�Ɏ~�܂�A���Ɏ��̍s���̈�E�Ɉړ�����B(�܂�Ԃ��Ȃ�)
			} on_x_max;

			if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ) {
				x_max = std::max( x_wrap, GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
				on_x_max = STOP;
			} else if( GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode ) {
				// �t���[�J�[�\�����[�h�ł͐܂�Ԃ��ʒu�������݂āA���s�����̈ʒu�݂͂Ȃ��B
				if( wrapped ) {
					if( nextline_exists ){
						x_max = x_wrap;
						on_x_max = MOVE_NEXTLINE_IMMEDIATELY;
					}else{
						// �f�[�^�̂���EOF�s�͐܂�Ԃ��ł͂Ȃ�
						x_max = std::max( x_wrap, GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
						on_x_max = STOP;
					}
				} else {
					if( x_wrap < GetDocument()->m_cLayoutMgr.GetMaxLineKetas() ) {
						x_max = GetDocument()->m_cLayoutMgr.GetMaxLineKetas();
						on_x_max = MOVE_NEXTLINE_IMMEDIATELY;
					} else { // ���s�������Ԃ牺�����Ă���Ƃ��͗�O�B
						x_max = x_wrap;
						on_x_max = MOVE_NEXTLINE_NEXTTIME;
					}
				}
			} else {
				x_max = x_wrap;
				on_x_max = wrapped ? MOVE_NEXTLINE_IMMEDIATELY : MOVE_NEXTLINE_NEXTTIME;
			}

			// �L�����b�g�̈ړ�������߂�B
			if( nextline_exists
				&& ( on_x_max == MOVE_NEXTLINE_IMMEDIATELY && x_max <= to_x
					|| on_x_max == MOVE_NEXTLINE_NEXTTIME && x_max < to_x
					|| on_x_max == MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT && x_max < to_x
				)
			) {
				ptTo.y = ptCaret.y + 1;
				ptTo.x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : CLayoutInt(0);
				if( on_x_max == MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT ) {
					++nRepeat;
				}
				bUnderlineDoNotOFF = false;
			} else {
				ptTo.y = ptCaret.y;
				ptTo.x = std::min( to_x, x_max );
			}
		} else {
			// pcLayout��NULL�̏ꍇ��ptPos.x=0�ɒ���
			ptTo.y = ptCaret.y;
			ptTo.x = 0;
		}

		GetCaret().MoveCursor( ptTo, TRUE, _CARETMARGINRATE, bUnderlineDoNotOFF );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
		}
	}
	return;
}



/* �J�[�\����ړ�(�Q�s�Â�) */
void CViewCommander::Command_UP2( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( CLayoutInt(-2), bSelect );
	return;
}



/* �J�[�\�����ړ�(�Q�s�Â�) */
void CViewCommander::Command_DOWN2( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( CLayoutInt(2), bSelect );
	return;
}



/* �P��̍��[�Ɉړ� */
void CViewCommander::Command_WORDLEFT( bool bSelect )
{
	bool	bUnderlineDoNotOFF = true;	// �A���_�[���C�����������Ȃ�
	if( bSelect ){
		bUnderlineDoNotOFF = false;		//�I����ԂȂ�A���_�[���C���������s��
	}
	CLogicInt		nIdx;
	if( bSelect ){
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			m_pCommanderView->GetSelectionInfo().BeginSelectArea();
		}
	}else{
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){		/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}else if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			m_pCommanderView->GetSelectionInfo().SetBoxSelect(false);
		}
	}

	const CLayout* pcLayout;
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		bool bIsFreeCursorModeOld = GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* �t���[�J�[�\�����[�h�� */
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* �J�[�\�����ړ� */
		Command_LEFT( bSelect, false );
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* �t���[�J�[�\�����[�h�� */
		return;
	}

	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
	nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

	/* ���݈ʒu�̍��̒P��̐擪�ʒu�𒲂ׂ� */
	CLayoutPoint ptLayoutNew;
	int nResult=GetDocument()->m_cLayoutMgr.PrevWord(
		GetCaret().GetCaretLayoutPos().GetY2(),
		nIdx,
		&ptLayoutNew,
		GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if( nResult ){
		/* �s���ς���� */
		if( ptLayoutNew.y != GetCaret().GetCaretLayoutPos().GetY2() ){
			pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptLayoutNew.GetY2() );
			if( NULL == pcLayout ){
				return;
			}
			bUnderlineDoNotOFF = false;
		}

		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		ptLayoutNew.x = m_pCommanderView->LineIndexToColmn( pcLayout, ptLayoutNew.x );
		*/

		/* �J�[�\���ړ� */
		GetCaret().MoveCursor( ptLayoutNew, TRUE, _CARETMARGINRATE, bUnderlineDoNotOFF );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptLayoutNew );
		}
	}else{
		bool bIsFreeCursorModeOld = GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* �t���[�J�[�\�����[�h�� */
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* �J�[�\�����ړ� */
		Command_LEFT( bSelect, false );
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* �t���[�J�[�\�����[�h�� */
	}
	return;
}



/* �P��̉E�[�Ɉړ� */
void CViewCommander::Command_WORDRIGHT( bool bSelect )
{
	bool	bUnderlineDoNotOFF = true;	// �A���_�[���C�����������Ȃ�
	if( bSelect ){
		bUnderlineDoNotOFF = false;		//�I����ԂȂ�A���_�[���C���������s��
	}
	CLogicInt	nIdx;
	CLayoutInt	nCurLine;
	if( bSelect ){
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			m_pCommanderView->GetSelectionInfo().BeginSelectArea();
		}
	}else{
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}else if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			m_pCommanderView->GetSelectionInfo().SetBoxSelect(false);
		}
	}
	bool	bTryAgain = false;
try_again:;
	nCurLine = GetCaret().GetCaretLayoutPos().GetY2();
	const CLayout* pcLayout;
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nCurLine );
	if( NULL == pcLayout ){
		return;
	}
	if( bTryAgain ){
		const wchar_t*	pLine = pcLayout->GetPtr();
		if( pLine[0] != L' ' && pLine[0] != WCODE::TAB ){
			return;
		}
	}
	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
	nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

	/* ���݈ʒu�̉E�̒P��̐擪�ʒu�𒲂ׂ� */
	CLayoutPoint ptLayoutNew;
	int nResult = GetDocument()->m_cLayoutMgr.NextWord(
		nCurLine,
		nIdx,
		&ptLayoutNew,
		GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if( nResult ){
		/* �s���ς���� */
		if( ptLayoutNew.y != nCurLine ){
			pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptLayoutNew.GetY2() );
			if( NULL == pcLayout ){
				return;
			}
			bUnderlineDoNotOFF = false;
		}
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		ptLayoutNew.x = m_pCommanderView->LineIndexToColmn( pcLayout, ptLayoutNew.x );
		*/
		// �J�[�\���ړ�
		GetCaret().MoveCursor( ptLayoutNew, TRUE, _CARETMARGINRATE, bUnderlineDoNotOFF );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptLayoutNew );
		}
	}
	else{
		bool	bIsFreeCursorModeOld = GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* �t���[�J�[�\�����[�h�� */
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* �J�[�\���E�ړ� */
		Command_RIGHT( bSelect, false, false );
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* �t���[�J�[�\�����[�h�� */
		if( !bTryAgain ){
			bTryAgain = true;
			goto try_again;
		}
	}
	return;
}



/*! @brief �s���Ɉړ�

	@date Oct. 29, 2001 genta �}�N���p�@�\�g��(�p�����[�^�ǉ�) + goto�r��
	@date May. 15, 2002 oak   ���s�P�ʈړ�
	@date Oct.  7, 2002 YAZAKI �璷�Ȉ��� bLineTopOnly ���폜
	@date Jun. 18, 2007 maru �s������ɑS�p�󔒂̃C���f���g�ݒ���l������
*/
void CViewCommander::Command_GOLINETOP(
	bool	bSelect,	//!< [in] �I���̗L���Btrue: �I�����Ȃ���ړ��Bfalse: �I�����Ȃ��ňړ��B
	int		lparam		/*!< [in] �}�N������g�p����g���t���O
								  @li 0: �L�[����Ɠ���(default)
								  @li 1: �J�[�\���ʒu�Ɋ֌W�Ȃ��s���Ɉړ��B
								  @li 4: �I�����Ĉړ�(������)
								  @li 8: ���s�P�ʂŐ擪�Ɉړ�(������)
						*/
)
{
	using namespace WCODE;

	// lparam�̉���
	bool	bLineTopOnly = ((lparam & 1) != 0);
	if( lparam & 4 ){
		bSelect = true;
	}


	CLayoutPoint ptCaretPos;
	if ( lparam & 8 ){
		/* ���s�P�ʎw��̏ꍇ�́A�����s���ʒu����ړI�_���ʒu�����߂� */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0,GetCaret().GetCaretLogicPos().y),
			&ptCaretPos
		);
	}
	else{
		const CLayout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
		ptCaretPos.x = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
		ptCaretPos.y = GetCaret().GetCaretLayoutPos().GetY2();
	}
	if( !bLineTopOnly ){
		/* �ړI�s�̃f�[�^���擾 */
		/* ���s�P�ʎw��ŁA�擪����󔒂�1�܂�Ԃ��s�ȏ㑱���Ă���ꍇ�͎��̍s�f�[�^���擾 */
		CLayoutInt nPosY_Layout;
		CLogicInt  nPosX_Logic;

		nPosY_Layout = ptCaretPos.y - 1;
		const CLayout*	pcLayout;
		bool			bZenSpace = GetDocument()->m_cDocType.GetDocumentAttribute().m_bAutoIndent_ZENSPACE;
		
		CLogicInt		nLineLen;
		do {
			++nPosY_Layout;
			const wchar_t*	pLine = GetDocument()->m_cLayoutMgr.GetLineStr( nPosY_Layout, &nLineLen, &pcLayout );
			if( !pLine ){
				return;
			}
			for( nPosX_Logic = 0; nPosX_Logic < nLineLen; ++nPosX_Logic ){
				if(WCODE::IsIndentChar(pLine[nPosX_Logic],bZenSpace!=0))continue;
				
				if(WCODE::IsLineDelimiter(pLine[nPosX_Logic]) ){
					nPosX_Logic = 0;	// �󔒂܂��̓^�u����щ��s�����̍s������
				}
				break;
			}
		}
		while (( lparam & 8 ) && (nPosX_Logic >= nLineLen) && (GetDocument()->m_cLayoutMgr.GetLineCount() - 1 > nPosY_Layout) );
		
		if( nPosX_Logic >= nLineLen ){
			/* �܂�Ԃ��P�ʂ̍s����T���ĕ����s���܂œ��B����
			�܂��́A�ŏI�s�̂��߉��s�R�[�h�ɑ��������ɍs���ɓ��B���� */
			nPosX_Logic = 0;
		}
		
		if(0 == nPosX_Logic) nPosY_Layout = ptCaretPos.y;	/* �����s�̈ړ��Ȃ� */
		
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		CLayoutInt nPosX_Layout = m_pCommanderView->LineIndexToColmn( pcLayout, nPosX_Logic );
		CLayoutPoint ptPos(nPosX_Layout, nPosY_Layout);
		if( GetCaret().GetCaretLayoutPos() != ptPos ){
			ptCaretPos = ptPos;
		}
	}

	//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
	m_pCommanderView->MoveCursorSelecting( ptCaretPos, bSelect );
}



// �s���Ɉړ�(�܂�Ԃ��P��)
void CViewCommander::Command_GOLINEEND( bool bSelect, int bIgnoreCurrentSelection )
{
	if( !bIgnoreCurrentSelection ){
		if( bSelect ){
			if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ���݂̃J�[�\���ʒu����I�����J�n���� */
				m_pCommanderView->GetSelectionInfo().BeginSelectArea();
			}
		}else{
			if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
			}else if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
				m_pCommanderView->GetSelectionInfo().SetBoxSelect(false);
			}
		}
	}

	// ���ݍs�̃f�[�^����A���̃��C�A�E�g�����擾
	CLayoutPoint	nPosXY = GetCaret().GetCaretLayoutPos();
	nPosXY.x = CLayoutInt(0);
	const CLayout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nPosXY.y );
	if(pcLayout)
		nPosXY.x = pcLayout->CalcLayoutWidth(GetDocument()->m_cLayoutMgr);

	// �L�����b�g�ړ�
	GetCaret().MoveCursor( nPosXY, true );
	GetCaret().m_nCaretPosX_Prev = nPosXY.x;
	if( bSelect ){
		// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
		m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( nPosXY );
	}
}



/* ���y�[�W�A�b�v */	//Oct. 6, 2000 JEPRO added (���͏]���̃X�N���[���_�E�����̂���)
void CViewCommander::Command_HalfPageUp( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( - ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 ), bSelect );
	return;
}



/* ���y�[�W�_�E�� */	//Oct. 6, 2000 JEPRO added (���͏]���̃X�N���[���A�b�v���̂���)
void CViewCommander::Command_HalfPageDown( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 ), bSelect );
	return;
}



/*! �P�y�[�W�A�b�v

	@date 2000.10.10 JEPRO �쐬
	@date 2001.12.13 hor ��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂�
		�P�y�[�W�A�b�v�ɓ���ύX
*/	//Oct. 10, 2000 JEPRO added
void CViewCommander::Command_1PageUp( bool bSelect )
{
//	GetCaret().Cursor_UPDOWN( - m_pCommanderView->GetTextArea().m_nViewRowNum, bSelect );

// 2001.12.03 hor
//		���������C�N�ɁA��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂łP�y�[�W�A�b�v
	if(m_pCommanderView->GetTextArea().GetViewTopLine()>=m_pCommanderView->GetTextArea().m_nViewRowNum-1){
		m_pCommanderView->SetDrawSwitch(false);
		CLayoutInt nViewTopLine=GetCaret().GetCaretLayoutPos().GetY2()-m_pCommanderView->GetTextArea().GetViewTopLine();
		GetCaret().Cursor_UPDOWN( -m_pCommanderView->GetTextArea().m_nViewRowNum+1, bSelect );
		//	Sep. 11, 2004 genta �����X�N���[�������̂���
		//	m_pCommanderView->RedrawAll�ł͂Ȃ�ScrollAt���g���悤��
		m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV( GetCaret().GetCaretLayoutPos().GetY2()-nViewTopLine ));
		m_pCommanderView->SetDrawSwitch(true);
		m_pCommanderView->RedrawAll();
		
	}else{
		GetCaret().Cursor_UPDOWN( -m_pCommanderView->GetTextArea().m_nViewRowNum+1, bSelect );
	}
	return;
}



/*!	�P�y�[�W�_�E��

	@date 2000.10.10 JEPRO �쐬
	@date 2001.12.13 hor ��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂�
		�P�y�[�W�_�E���ɓ���ύX
*/
void CViewCommander::Command_1PageDown( bool bSelect )
{
//	GetCaret().Cursor_UPDOWN( m_pCommanderView->GetTextArea().m_nViewRowNum, bSelect );

// 2001.12.03 hor
//		���������C�N�ɁA��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂łP�y�[�W�_�E��
	if(m_pCommanderView->GetTextArea().GetViewTopLine()+m_pCommanderView->GetTextArea().m_nViewRowNum <= GetDocument()->m_cLayoutMgr.GetLineCount() ){ //- m_pCommanderView->GetTextArea().m_nViewRowNum){
		m_pCommanderView->SetDrawSwitch(false);
		CLayoutInt nViewTopLine=GetCaret().GetCaretLayoutPos().GetY2()-m_pCommanderView->GetTextArea().GetViewTopLine();
		GetCaret().Cursor_UPDOWN( m_pCommanderView->GetTextArea().m_nViewRowNum-1, bSelect );
		//	Sep. 11, 2004 genta �����X�N���[�������̂���
		//	m_pCommanderView->RedrawAll�ł͂Ȃ�ScrollAt���g���悤��
		m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV( GetCaret().GetCaretLayoutPos().GetY2()-nViewTopLine ));
		m_pCommanderView->SetDrawSwitch(true);
		m_pCommanderView->RedrawAll();
	}else{
		GetCaret().Cursor_UPDOWN( m_pCommanderView->GetTextArea().m_nViewRowNum , bSelect );
		Command_DOWN( bSelect, TRUE );
	}

	return;
}



/* �t�@�C���̐擪�Ɉړ� */
void CViewCommander::Command_GOFILETOP( bool bSelect )
{
	/* �擪�փJ�[�\�����ړ� */
	//	Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();

	//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
	CLayoutPoint pt(
		!m_pCommanderView->GetSelectionInfo().IsBoxSelecting()? CLayoutInt(0): GetCaret().GetCaretLayoutPos().GetX2(),
		CLayoutInt(0)
	);
	m_pCommanderView->MoveCursorSelecting( pt, bSelect );	//	�ʏ�́A(0, 0)�ֈړ��B�{�b�N�X�I�𒆂́A(GetCaret().GetCaretLayoutPos().GetX2(), 0)�ֈړ�
}



/* �t�@�C���̍Ō�Ɉړ� */
void CViewCommander::Command_GOFILEEND( bool bSelect )
{
// 2001.12.13 hor BOX�I�𒆂Ƀt�@�C���̍Ō�ɃW�����v�����[EOF]�̍s�����]�����܂܂ɂȂ�̏C��
	if( !bSelect ){
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			m_pCommanderView->GetSelectionInfo().DisableSelectArea(true);	// 2001.12.21 hor Add
		}else if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			m_pCommanderView->GetSelectionInfo().SetBoxSelect(false);
		}
	}
	m_pCommanderView->AddCurrentLineToHistory();
	GetCaret().Cursor_UPDOWN( GetDocument()->m_cLayoutMgr.GetLineCount() , bSelect );
	Command_DOWN( bSelect, TRUE );
	if ( !m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){							// 2002/04/18 YAZAKI
		/*	2004.04.19 fotomo
			���s�̂Ȃ��ŏI�s�őI�����Ȃ��當�����ֈړ������ꍇ��
			�I��͈͂��������Ȃ��ꍇ��������ɑΉ�
		*/
		Command_GOLINEEND( bSelect, FALSE );				// 2001.12.21 hor Add
	}
	GetCaret().MoveCursor( GetCaret().GetCaretLayoutPos(), TRUE );	// 2001.12.21 hor Add
	// 2002.02.16 hor ��`�I�𒆂��������O�̃J�[�\���ʒu�����Z�b�g
	if( !(m_pCommanderView->GetSelectionInfo().IsTextSelected() && m_pCommanderView->GetSelectionInfo().IsBoxSelecting()) ) GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	// �I��͈͏�񃁃b�Z�[�W��\������	// 2009.05.06 ryoji �ǉ�
	if( bSelect ){
		m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
	}
}



/* �J�[�\���s���E�B���h�E������ */
void CViewCommander::Command_CURLINECENTER( void )
{
	CLayoutInt		nViewTopLine;
	nViewTopLine = GetCaret().GetCaretLayoutPos().GetY2() - ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 );

	// sui 02/08/09
	if( 0 > nViewTopLine )	nViewTopLine = CLayoutInt(0);
	
	CLayoutInt nScrollLines = nViewTopLine - m_pCommanderView->GetTextArea().GetViewTopLine();	//Sep. 11, 2004 genta �����p�ɍs�����L��
	m_pCommanderView->GetTextArea().SetViewTopLine( nViewTopLine );
	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	m_pCommanderView->RedrawAll();
	// sui 02/08/09

	//	Sep. 11, 2004 genta �����X�N���[���̊֐���
	m_pCommanderView->SyncScrollV( nScrollLines );
}



//	�ړ�������O�ւ��ǂ�
//
void CViewCommander::Command_JUMPHIST_PREV( void )
{
	// 2001.12.13 hor
	// �ړ������̍Ō�Ɍ��݂̈ʒu���L������
	// ( ���̗������擾�ł��Ȃ��Ƃ��͒ǉ����Ė߂� )
	if( !m_pCommanderView->m_cHistory->CheckNext() ){
		m_pCommanderView->AddCurrentLineToHistory();
		m_pCommanderView->m_cHistory->PrevValid();
	}

	if( m_pCommanderView->m_cHistory->CheckPrev() ){
		if( ! m_pCommanderView->m_cHistory->PrevValid() ){
			::MessageBox( NULL, _T("Inconsistent Implementation"), _T("PrevValid"), MB_OK );
		}
		CLayoutPoint pt;
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(m_pCommanderView->m_cHistory->GetCurrent().GetPos(), m_pCommanderView->m_cHistory->GetCurrent().GetLineNo()),
			&pt
		);
		//	2006.07.09 genta �I�����l��
		m_pCommanderView->MoveCursorSelecting( pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
}



//	�ړ����������ւ��ǂ�
void CViewCommander::Command_JUMPHIST_NEXT( void )
{
	if( m_pCommanderView->m_cHistory->CheckNext() ){
		if( ! m_pCommanderView->m_cHistory->NextValid() ){
			::MessageBox( NULL, _T("Inconsistent Implementation"), _T("NextValid"), MB_OK );
		}
		CLayoutPoint pt;
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(m_pCommanderView->m_cHistory->GetCurrent().GetPos(), m_pCommanderView->m_cHistory->GetCurrent().GetLineNo()),
			&pt
		);
		//	2006.07.09 genta �I�����l��
		m_pCommanderView->MoveCursorSelecting( pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
}



//	���݈ʒu���ړ������ɓo�^����
void CViewCommander::Command_JUMPHIST_SET( void )
{
	m_pCommanderView->AddCurrentLineToHistory();
}



// 2001/06/20 Start by asa-o

//	from CViewCommander_New.cpp
// �e�L�X�g���P�s���փX�N���[��
void CViewCommander::Command_WndScrollDown( void )
{
	CLayoutInt	nCaretMarginY;

	nCaretMarginY = m_pCommanderView->GetTextArea().m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = CLayoutInt(1);

	nCaretMarginY += 2;

	if( GetCaret().GetCaretLayoutPos().GetY() > m_pCommanderView->GetTextArea().m_nViewRowNum + m_pCommanderView->GetTextArea().GetViewTopLine() - (nCaretMarginY + 1) ){
		GetCaret().m_cUnderLine.CaretUnderLineOFF( TRUE );
	}

	//	Sep. 11, 2004 genta �����p�ɍs�����L��
	//	Sep. 11, 2004 genta �����X�N���[���̊֐���
	m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV(m_pCommanderView->GetTextArea().GetViewTopLine() - CLayoutInt(1)));

	// �e�L�X�g���I������Ă��Ȃ�
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() )
	{
		// �J�[�\������ʊO�ɏo��
		if( GetCaret().GetCaretLayoutPos().GetY() > m_pCommanderView->GetTextArea().m_nViewRowNum + m_pCommanderView->GetTextArea().GetViewTopLine() - nCaretMarginY )
		{
			if( GetCaret().GetCaretLayoutPos().GetY() > GetDocument()->m_cLayoutMgr.GetLineCount() - nCaretMarginY )
				GetCaret().Cursor_UPDOWN( (GetDocument()->m_cLayoutMgr.GetLineCount() - nCaretMarginY) - GetCaret().GetCaretLayoutPos().GetY2(), FALSE );
			else
				GetCaret().Cursor_UPDOWN( CLayoutInt(-1), FALSE);
			GetCaret().ShowCaretPosInfo();
		}
	}

	GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
}



//	from CViewCommander_New.cpp
// �e�L�X�g���P�s��փX�N���[��
void CViewCommander::Command_WndScrollUp(void)
{
	CLayoutInt	nCaretMarginY;

	nCaretMarginY = m_pCommanderView->GetTextArea().m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	if( GetCaret().GetCaretLayoutPos().GetY2() < m_pCommanderView->GetTextArea().GetViewTopLine() + (nCaretMarginY + 1) ){
		GetCaret().m_cUnderLine.CaretUnderLineOFF( TRUE );
	}

	//	Sep. 11, 2004 genta �����p�ɍs�����L��
	//	Sep. 11, 2004 genta �����X�N���[���̊֐���
	m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV( m_pCommanderView->GetTextArea().GetViewTopLine() + CLayoutInt(1) ));

	// �e�L�X�g���I������Ă��Ȃ�
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() )
	{
		// �J�[�\������ʊO�ɏo��
		if( GetCaret().GetCaretLayoutPos().GetY() < m_pCommanderView->GetTextArea().GetViewTopLine() + nCaretMarginY )
		{
			if( m_pCommanderView->GetTextArea().GetViewTopLine() == 1 )
				GetCaret().Cursor_UPDOWN( nCaretMarginY + 1, FALSE );
			else
				GetCaret().Cursor_UPDOWN( CLayoutInt(1), FALSE );
			GetCaret().ShowCaretPosInfo();
		}
	}

	GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
}

// 2001/06/20 End



//	from CViewCommander_New.cpp
/* ���̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void CViewCommander::Command_GONEXTPARAGRAPH( bool bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = 0;
	
	bool nFirstLineIsEmptyLine = false;
	/* �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ������� */
	if ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer++;
	}
	else {
		// EOF�s�ł����B
		return;
	}

	/* ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂� */
	while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer++;
		}
		else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLine����s��������A�����Ă���Ƃ���͔��s�B���Ȃ킿�����܂��B
		nFirstLineIsEmptyLine�����s��������A�����Ă���Ƃ���͋�s�B
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	�����܂��B
	}
	else {
		//	���܌��Ă���Ƃ���͋�s��1�s��
		if ( GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
		}
		else {
			/* �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői�� */
			while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					nCaretPointer++;
				}
				else {
					break;
				}
			}
		}
	}

	//	EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	/* �ړ��������v�Z */
	CLayoutPoint ptCaretPos_Layo;

	/* �ړ��O�̕����ʒu */
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos(),
		&ptCaretPos_Layo
	);

	/* �ړ���̕����ʒu */
	CLayoutPoint ptCaretPos_Layo_CaretPointer;
	//int nCaretPosY_Layo_CaretPointer;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos() + CLogicPoint(0,nCaretPointer),
		&ptCaretPos_Layo_CaretPointer
	);

	GetCaret().Cursor_UPDOWN( ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect );
}



//	from CViewCommander_New.cpp
/* �O�̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void CViewCommander::Command_GOPREVPARAGRAPH( bool bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = -1;

	bool nFirstLineIsEmptyLine = false;
	/* �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ������� */
	if ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer--;
	}
	else {
		nFirstLineIsEmptyLine = true;
		nCaretPointer--;
	}

	/* ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂� */
	while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer--;
		}
		else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLine����s��������A�����Ă���Ƃ���͔��s�B���Ȃ킿�����܂��B
		nFirstLineIsEmptyLine�����s��������A�����Ă���Ƃ���͋�s�B
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	�����܂��B
		if ( GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
			nCaretPointer++;	//	��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}
		else {
			/* �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői�� */
			while ( pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() + CLogicInt(nCaretPointer) ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					break;
				}
				else {
					nCaretPointer--;
				}
			}
			nCaretPointer++;	//	��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}
	}
	else {
		//	���܌��Ă���Ƃ���͋�s��1�s��
		if ( GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
			nCaretPointer++;
		}
		else {
			nCaretPointer++;
		}
	}

	//	EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	/* �ړ��������v�Z */
	CLayoutPoint ptCaretPos_Layo;

	/* �ړ��O�̕����ʒu */
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos(),
		&ptCaretPos_Layo
	);

	/* �ړ���̕����ʒu */
	CLayoutPoint ptCaretPos_Layo_CaretPointer;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos() + CLogicPoint(0, nCaretPointer),
		&ptCaretPos_Layo_CaretPointer
	);

	GetCaret().Cursor_UPDOWN( ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect );
}

void CViewCommander::Command_AUTOSCROLL()
{
	if( 0 == m_pCommanderView->m_nAutoScrollMode ){
		GetCursorPos( &m_pCommanderView->m_cAutoScrollMousePos );
		ScreenToClient( m_pCommanderView->GetHwnd(), &m_pCommanderView->m_cAutoScrollMousePos );
		m_pCommanderView->m_bAutoScrollDragMode = false;
		m_pCommanderView->AutoScrollEnter();
	}else{
		m_pCommanderView->AutoScrollExit();
	}
}
