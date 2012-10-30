#include "StdAfx.h"
#include "CViewCommander.h"
#include <algorithm>		// 2001.12.11 hor    for VC++
#include <string>///	2002/2/3 aroka 
#include <vector> ///	2002/2/3 aroka
#include "view/CEditView.h"
#include "CWaitCursor.h"
#include "charset/charcode.h"
#include "debug/CRunningTimer.h"
#include "COpe.h" ///	2002/2/3 aroka from here
#include "COpeBlk.h" ///	2002/2/3 aroka 
#include "doc/CLayout.h"///	2002/2/3 aroka 
#include "doc/CDocLine.h"///	2002/2/3 aroka 
#include "debug/Debug.h"///	2002/2/3 aroka 
#include "COsVersionInfo.h"   // 2002.04.09 minfu 
#include "doc/CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "dlg/CDlgCtrlCode.h"	//�R���g���[���R�[�h�̓���(�_�C�A���O)
#include "dlg/CDlgFavorite.h"	//�����̊Ǘ�	//@@@ 2003.04.08 MIK
#include "window/CEditWnd.h"
#include "io/CFileLoad.h"	// 2006.12.09 maru
#include "dlg/CDlgCancel.h"	// 2006.12.09 maru
#include "env/CSakuraEnvironment.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

using namespace std; // 2002/2/3 aroka to here



#ifndef FID_RECONVERT_VERSION  // 2002.04.10 minfu 
#define FID_RECONVERT_VERSION 0x10000000
#endif


/* Undo ���ɖ߂� */
void CViewCommander::Command_UNDO( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	if( !GetDocument()->m_cDocEditor.IsEnableUndo() ){	/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
		return;
	}

	MY_RUNNINGTIMER( cRunningTimer, "CViewCommander::Command_UNDO()" );

	COpe*		pcOpe = NULL;

	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
	bool		bIsModified;
//	int			nNewLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int			nNewPos;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	CLayoutPoint ptCaretPos_Before;

	CLayoutPoint ptCaretPos_After;

	/* �e�탂�[�h�̎����� */
	Command_CANCEL_MODE();

	m_pCommanderView->m_bDoing_UndoRedo = TRUE;	/* �A���h�D�E���h�D�̎��s���� */

	/* ���݂�Undo�Ώۂ̑���u���b�N��Ԃ� */
	if( NULL != ( pcOpeBlk = GetDocument()->m_cDocEditor.m_cOpeBuf.DoUndo( &bIsModified ) ) ){
		m_pCommanderView->SetDrawSwitch(false);	//	hor
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = nOpeBlkNum - 1; i >= 0; i-- ){
			pcOpe = pcOpeBlk->GetOpe( i );
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_After,
				&ptCaretPos_After
			);
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before,
				&ptCaretPos_Before
			);


			/* �J�[�\�����ړ� */
			GetCaret().MoveCursor( ptCaretPos_After, false );

			switch( pcOpe->GetCode() ){
			case OPE_INSERT:
				{
					CInsertOpe* pcInsertOpe = static_cast<CInsertOpe*>(pcOpe);

					/* �I��͈͂̕ύX */
					m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetFrom(ptCaretPos_Before);
					m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetTo(m_pCommanderView->GetSelectionInfo().m_sSelectBgn.GetFrom());
					m_pCommanderView->GetSelectionInfo().m_sSelect.SetFrom(ptCaretPos_Before);
					m_pCommanderView->GetSelectionInfo().m_sSelect.SetTo(ptCaretPos_After);

					/* �f�[�^�u�� �폜&�}���ɂ��g���� */
					m_pCommanderView->ReplaceData_CEditView(
						m_pCommanderView->GetSelectionInfo().m_sSelect,				// �폜�͈�
						&pcInsertOpe->m_pcmemData,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						L"",						// �}������f�[�^
						CLogicInt(0),				// �}������f�[�^�̒���
						false,						// �ĕ`�悷�邩�ۂ�
						m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
					);

					/* �I��͈͂̕ύX */
					m_pCommanderView->GetSelectionInfo().m_sSelectBgn.Clear(-1); //�͈͑I��(���_)
					m_pCommanderView->GetSelectionInfo().m_sSelect.Clear(-1);
				}
				break;
			case OPE_DELETE:
				{
					CDeleteOpe* pcDeleteOpe = static_cast<CDeleteOpe*>(pcOpe);

					//2007.10.17 kobake ���������[�N���Ă܂����B�C���B
					if( 0 < pcDeleteOpe->m_pcmemData.GetStringLength() ){
						/* �f�[�^�u�� �폜&�}���ɂ��g���� */
						CLayoutRange sRange;
						sRange.Set(ptCaretPos_Before);
						m_pCommanderView->ReplaceData_CEditView(
							sRange,
							NULL,										/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
							pcDeleteOpe->m_pcmemData.GetStringPtr(),	/* �}������f�[�^ */
							pcDeleteOpe->m_nDataLen,					/* �}������f�[�^�̒��� */
							false,										/*�ĕ`�悷�邩�ۂ�*/
							m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
						);
					}
					pcDeleteOpe->m_pcmemData.Clear();
				}
				break;
			case OPE_MOVECARET:
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_After, false );
				break;
			}

			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before,
				&ptCaretPos_Before
			);
			if( i == 0 ){
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_Before, true );
			}else{
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_Before, false );
			}
		}
		m_pCommanderView->SetDrawSwitch(true);	//	hor
		m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji

		/* Undo��̕ύX�t���O */
		GetDocument()->m_cDocEditor.SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

		m_pCommanderView->SetBracketPairPos( true );	// 03/03/07 ai

		/* �ĕ`�� */
		// ���[���[�ĕ`��̕K�v������Ƃ��� DispRuler() �ł͂Ȃ����̕����Ɠ����� Call_OnPaint() �ŕ`�悷��	// 2010.08.20 ryoji
		// �EDispRuler() �̓��[���[�ƃe�L�X�g�̌��ԁi�����͍s�ԍ��̕��ɍ��킹���сj��`�悵�Ă���Ȃ�
		// �E�s�ԍ��\���ɕK�v�ȕ��� OPE_INSERT/OPE_DELETE �������ōX�V����Ă���ύX������΃��[���[�ĕ`��t���O�ɔ��f����Ă���
		// �E�����X�N���[�������[���[�ĕ`��t���O�ɔ��f����Ă���
		const bool bRedrawRuler = m_pCommanderView->GetRuler().GetRedrawFlag();
		m_pCommanderView->Call_OnPaint( PAINT_LINENUMBER | PAINT_BODY | (bRedrawRuler? PAINT_RULER: 0), false );
		if( !bRedrawRuler ){
			// ���[���[�̃L�����b�g�݂̂��ĕ`��
			HDC hdc = m_pCommanderView->GetDC();
			m_pCommanderView->GetRuler().DispRuler( hdc );
			m_pCommanderView->ReleaseDC( hdc );
		}

		GetCaret().ShowCaretPosInfo();	// �L�����b�g�̍s���ʒu��\������	// 2007.10.19 ryoji

		if( !GetEditWindow()->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews( m_pCommanderView );	//	���̃y�C���̕\�����X�V

	}

	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji �ǉ�
	m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

	return;
}





/* Redo ��蒼�� */
void CViewCommander::Command_REDO( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}


	if( !GetDocument()->m_cDocEditor.IsEnableRedo() ){	/* Redo(��蒼��)�\�ȏ�Ԃ��H */
		return;
	}
	MY_RUNNINGTIMER( cRunningTimer, "CViewCommander::Command_REDO()" );

	COpe*		pcOpe = NULL;
	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
//	int			nNewLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int			nNewPos;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	bool		bIsModified;
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	CLayoutPoint ptCaretPos_Before;
	CLayoutPoint ptCaretPos_To;
	CLayoutPoint ptCaretPos_After;


	/* �e�탂�[�h�̎����� */
	Command_CANCEL_MODE();

	m_pCommanderView->m_bDoing_UndoRedo = TRUE;	/* �A���h�D�E���h�D�̎��s���� */

	/* ���݂�Redo�Ώۂ̑���u���b�N��Ԃ� */
	if( NULL != ( pcOpeBlk = GetDocument()->m_cDocEditor.m_cOpeBuf.DoRedo( &bIsModified ) ) ){
		m_pCommanderView->SetDrawSwitch(false);	// 2007.07.22 ryoji
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = 0; i < nOpeBlkNum; ++i ){
			pcOpe = pcOpeBlk->GetOpe( i );
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before,
				&ptCaretPos_Before
			);

			if( i == 0 ){
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_Before, true );
			}else{
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_Before, false );
			}
			switch( pcOpe->GetCode() ){
			case OPE_INSERT:
				{
					CInsertOpe* pcInsertOpe = static_cast<CInsertOpe*>(pcOpe);

					//2007.10.17 kobake ���������[�N���Ă܂����B�C���B
					if( 0 < pcInsertOpe->m_pcmemData.GetStringLength() ){
						/* �f�[�^�u�� �폜&�}���ɂ��g���� */
						CLayoutRange sRange;
						sRange.Set(ptCaretPos_Before);
						m_pCommanderView->ReplaceData_CEditView(
							sRange,
							NULL,										/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
							pcInsertOpe->m_pcmemData.GetStringPtr(),	/* �}������f�[�^ */
							pcInsertOpe->m_pcmemData.GetStringLength(),	/* �}������f�[�^�̒��� */
							false,										/*�ĕ`�悷�邩�ۂ�*/
							m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
						);

					}
					pcInsertOpe->m_pcmemData.Clear();
				}
				break;
			case OPE_DELETE:
				{
					CDeleteOpe* pcDeleteOpe = static_cast<CDeleteOpe*>(pcOpe);

					GetDocument()->m_cLayoutMgr.LogicToLayout(
						pcDeleteOpe->m_ptCaretPos_PHY_To,
						&ptCaretPos_To
					);

					/* �f�[�^�u�� �폜&�}���ɂ��g���� */
					m_pCommanderView->ReplaceData_CEditView(
						CLayoutRange(ptCaretPos_Before,ptCaretPos_To),
						&pcDeleteOpe->m_pcmemData,	/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
						L"",						/* �}������f�[�^ */
						CLogicInt(0),				/* �}������f�[�^�̒��� */
						false,
						m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
					);
				}
				break;
			case OPE_MOVECARET:
				break;
			}
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_After,
				&ptCaretPos_After
			);

			if( i == nOpeBlkNum - 1	){
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_After, true );
			}else{
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_After, false );
			}
		}
		m_pCommanderView->SetDrawSwitch(true); // 2007.07.22 ryoji
		m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji

		/* Redo��̕ύX�t���O */
		GetDocument()->m_cDocEditor.SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

		m_pCommanderView->SetBracketPairPos( true );	// 03/03/07 ai

		/* �ĕ`�� */
		// ���[���[�ĕ`��̕K�v������Ƃ��� DispRuler() �ł͂Ȃ����̕����Ɠ����� Call_OnPaint() �ŕ`�悷��	// 2010.08.20 ryoji
		// �EDispRuler() �̓��[���[�ƃe�L�X�g�̌��ԁi�����͍s�ԍ��̕��ɍ��킹���сj��`�悵�Ă���Ȃ�
		// �E�s�ԍ��\���ɕK�v�ȕ��� OPE_INSERT/OPE_DELETE �������ōX�V����Ă���ύX������΃��[���[�ĕ`��t���O�ɔ��f����Ă���
		// �E�����X�N���[�������[���[�ĕ`��t���O�ɔ��f����Ă���
		const bool bRedrawRuler = m_pCommanderView->GetRuler().GetRedrawFlag();
		m_pCommanderView->Call_OnPaint( PAINT_LINENUMBER | PAINT_BODY | (bRedrawRuler? PAINT_RULER: 0), false );
		if( !bRedrawRuler ){
			// ���[���[�̃L�����b�g�݂̂��ĕ`��
			HDC hdc = m_pCommanderView->GetDC();
			m_pCommanderView->GetRuler().DispRuler( hdc );
			m_pCommanderView->ReleaseDC( hdc );
		}

		GetCaret().ShowCaretPosInfo();	// �L�����b�g�̍s���ʒu��\������	// 2007.10.19 ryoji

		if( !GetEditWindow()->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews( m_pCommanderView );	//	���̃y�C���̕\�����X�V
	}

	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji �ǉ�
	m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

	return;
}







// 2001/06/20 Start by asa-o

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

// From Here 2001.12.03 hor

//! �u�b�N�}�[�N�̐ݒ�E�������s��(�g�O������)
void CViewCommander::Command_BOOKMARK_SET(void)
{
	CDocLine*	pCDocLine;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() && m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y<m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().y ){
		CLogicPoint ptFrom;
		CLogicPoint ptTo;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint(CLayoutInt(0), m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y),
			&ptFrom
		);
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint(CLayoutInt(0), m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().y  ),
			&ptTo
		);
		for(CLogicInt nY=ptFrom.GetY2();nY<=ptTo.y;nY++){
			pCDocLine=GetDocument()->m_cDocLineMgr.GetLine( nY );
			CBookmarkSetter cBookmark(pCDocLine);
			if(pCDocLine)cBookmark.SetBookmark(!cBookmark.IsBookmarked());
		}
	}
	else{
		pCDocLine=GetDocument()->m_cDocLineMgr.GetLine( GetCaret().GetCaretLogicPos().GetY2() );
		CBookmarkSetter cBookmark(pCDocLine);
		if(pCDocLine)cBookmark.SetBookmark(!cBookmark.IsBookmarked());
	}

	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow()->Views_Redraw();
}



//! ���̃u�b�N�}�[�N��T���C����������ړ�����
void CViewCommander::Command_BOOKMARK_NEXT(void)
{
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor

	CLogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	CLogicInt tmp_y;

	nYOld=ptXY.y;					// hor

re_do:;								// hor
	if(CBookmarkManager(&GetDocument()->m_cDocLineMgr).SearchBookMark(ptXY.GetY2(), SEARCH_FORWARD, &tmp_y)){
		ptXY.y = tmp_y;
		bFound = TRUE;
		CLayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
		//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		m_pCommanderView->MoveCursorSelecting( ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&		// ������Ȃ�����
			bRedo			// �ŏ��̌���
		){
			ptXY.y=-1;	//	2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;		// �擪����Č���
		}
	}
	if(bFound){
		if(nYOld >= ptXY.y)m_pCommanderView->SendStatusMessage(_T("���擪����Č������܂���"));
	}else{
		m_pCommanderView->SendStatusMessage(_T("��������܂���ł���"));
		AlertNotFound( m_pCommanderView->GetHwnd(), _T("�O��(��) �Ƀu�b�N�}�[�N��������܂���B"));
	}
	return;
}



//! �O�̃u�b�N�}�[�N��T���C����������ړ�����D
void CViewCommander::Command_BOOKMARK_PREV(void)
{
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor

	CLogicPoint	ptXY(0,GetCaret().GetCaretLogicPos().y);
	CLogicInt tmp_y;

	nYOld=ptXY.y;						// hor

re_do:;								// hor
	if(CBookmarkManager(&GetDocument()->m_cDocLineMgr).SearchBookMark(ptXY.GetY2(), SEARCH_BACKWARD, &tmp_y)){
		ptXY.y = tmp_y;
		bFound = TRUE;				// hor
		CLayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
		//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		m_pCommanderView->MoveCursorSelecting( ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&	// ������Ȃ�����
			bRedo		// �ŏ��̌���
		){
			// 2011.02.02 m_cLayoutMgr��m_cDocLineMgr
			ptXY.y= GetDocument()->m_cDocLineMgr.GetLineCount();	// 2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;	// ��������Č���
		}
	}
	if(bFound){
		if(nYOld <= ptXY.y)m_pCommanderView->SendStatusMessage(_T("����������Č������܂���"));
	}else{
		m_pCommanderView->SendStatusMessage(_T("��������܂���ł���"));
		AlertNotFound( m_pCommanderView->GetHwnd(), _T("���(��) �Ƀu�b�N�}�[�N��������܂���B") );
	}
	return;
}



//! �u�b�N�}�[�N���N���A����
void CViewCommander::Command_BOOKMARK_RESET(void)
{
	CBookmarkManager(&GetDocument()->m_cDocLineMgr).ResetAllBookMark();
	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow()->Views_Redraw();
}


//�w��p�^�[���Ɉ�v����s���}�[�N 2002.01.16 hor
//�L�[�}�N���ŋL�^�ł���悤��	2002.02.08 hor
void CViewCommander::Command_BOOKMARK_PATTERN( void )
{
	//����or�u���_�C�A���O����Ăяo���ꂽ
	if( !m_pCommanderView->ChangeCurRegexp(false) ) return;
	
	CBookmarkManager(&GetDocument()->m_cDocLineMgr).MarkSearchWord(
		m_pCommanderView->m_strCurSearchKey.c_str(),		// ��������
		m_pCommanderView->m_sCurSearchOption,	// ��������
		&m_pCommanderView->m_CurRegexp							// ���K�\���R���p�C���f�[�^
	);
	// 2002.01.16 hor ���������r���[���X�V
	GetEditWindow()->Views_Redraw();
}



/*! TRIM Step1
	��I�����̓J�����g�s��I������ m_pCommanderView->ConvSelectedArea �� ConvMemory ��
	@author hor
	@date 2001.12.03 hor �V�K�쐬
*/
void CViewCommander::Command_TRIM(
	BOOL bLeft	//!<  [in] FALSE: �ETRIM / ����ȊO: ��TRIM
)
{
	bool bBeDisableSelectArea = false;
	CViewSelect& cViewSelect = m_pCommanderView->GetSelectionInfo();

	if(!cViewSelect.IsTextSelected()){	//	��I�����͍s�I���ɕύX
		cViewSelect.m_sSelect.SetFrom(
			CLayoutPoint(
				CLayoutInt(0),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		cViewSelect.m_sSelect.SetTo  (
			CLayoutPoint(
				GetDocument()->m_cLayoutMgr.GetMaxLineKetas(),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		bBeDisableSelectArea = true;
	}

	if(bLeft){
		m_pCommanderView->ConvSelectedArea( F_LTRIM );
	}
	else{
		m_pCommanderView->ConvSelectedArea( F_RTRIM );
	}

	if(bBeDisableSelectArea)
		cViewSelect.DisableSelectArea( TRUE );
}

/*!	�����s�̃\�[�g�Ɏg���\����*/
typedef struct _SORTTABLE {
	wstring sKey1;
	wstring sKey2;
} SORTDATA, *SORTTABLE;

/*!	�����s�̃\�[�g�Ɏg���֐�(����) */
bool SortByKeyAsc (SORTTABLE pst1, SORTTABLE pst2) {return (pst1->sKey1<pst2->sKey1);}

/*!	�����s�̃\�[�g�Ɏg���֐�(�~��) */
bool SortByKeyDesc(SORTTABLE pst1, SORTTABLE pst2) {return (pst1->sKey1>pst2->sKey1);}

/*!	@brief �����s�̃\�[�g

	��I�����͉������s���Ȃ��D��`�I�����́A���͈̔͂��L�[�ɂ��ĕ����s���\�[�g�D
	
	@note �Ƃ肠�������s�R�[�h���܂ރf�[�^���\�[�g���Ă���̂ŁA
	�t�@�C���̍ŏI�s�̓\�[�g�ΏۊO�ɂ��Ă��܂�
	@author hor
	@date 2001.12.03 hor �V�K�쐬
	@date 2001.12.21 hor �I��͈͂̒������W�b�N�����
*/
void CViewCommander::Command_SORT(BOOL bAsc)	//bAsc:TRUE=����,FALSE=�~��
{
	CLayoutRange sRangeA;
	CLogicRange sSelectOld;

	int			nColmFrom,nColmTo;
	CLayoutInt	nCF,nCT;
	CLayoutInt	nCaretPosYOLD;
	bool		bBeginBoxSelectOld;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int			j;
	CNativeW	cmemBuf;
	std::vector<SORTTABLE> sta;

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
		return;
	}

	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		sRangeA=m_pCommanderView->GetSelectionInfo().m_sSelect;
		if( m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().x==m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().x ){
			//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
			m_pCommanderView->GetSelectionInfo().m_sSelect.SetToX( GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
		}
		if(m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().x<m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().x){
			nCF=m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().GetX2();
			nCT=m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().GetX2();
		}else{
			nCF=m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().GetX2();
			nCT=m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().GetX2();
		}
	}
	bBeginBoxSelectOld=m_pCommanderView->GetSelectionInfo().IsBoxSelecting();
	nCaretPosYOLD=GetCaret().GetCaretLayoutPos().GetY();
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		m_pCommanderView->GetSelectionInfo().m_sSelect,
		&sSelectOld
	);

	if( bBeginBoxSelectOld ){
		sSelectOld.GetToPointer()->y++;
	}
	else{
		// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
		// ���̍s���I��͈͂ɉ�����
		if ( sSelectOld.GetTo().x > 0 ) {
			// 2006.03.31 Moca nSelectLineToOld�́A�����s�Ȃ̂�Layout�n����DocLine�n�ɏC��
			const CDocLine* pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( sSelectOld.GetTo().GetY2() );
			if( NULL != pcDocLine && EOL_NONE != pcDocLine->GetEol() ){
				sSelectOld.GetToPointer()->y++;
			}
		}
	}
	sSelectOld.SetFromX(CLogicInt(0));
	sSelectOld.SetToX(CLogicInt(0));

	//�s�I������ĂȂ�
	if(sSelectOld.IsLineOne()){
		return;
	}

	for( CLogicInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().y; i++ ){
		const CDocLine* pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( i );
		pLine = GetDocument()->m_cDocLineMgr.GetLine(i)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ) continue;
		SORTTABLE pst = new SORTDATA;
		if( bBeginBoxSelectOld ){
			nColmFrom = m_pCommanderView->LineColmnToIndex( pcDocLine, nCF );
			nColmTo   = m_pCommanderView->LineColmnToIndex( pcDocLine, nCT );
			if(nColmTo<nLineLen){	// BOX�I��͈͂̉E�[���s���Ɏ��܂��Ă���ꍇ
				// 2006.03.31 genta std::string::assign���g���Ĉꎞ�ϐ��폜
				pst->sKey1.assign( &pLine[nColmFrom], nColmTo-nColmFrom );
			}
			else if(nColmFrom<nLineLen){	// BOX�I��͈͂̉E�[���s�����E�ɂ͂ݏo���Ă���ꍇ
				pst->sKey1=&pLine[nColmFrom];
			}
			pst->sKey2=pLine;
		}else{
			pst->sKey1=pLine;
		}
		sta.push_back(pst);
	}
	if(bAsc){
		std::stable_sort(sta.begin(), sta.end(), SortByKeyAsc);
	}else{
		std::stable_sort(sta.begin(), sta.end(), SortByKeyDesc);
	}
	cmemBuf.SetString(L"");
	j=(int)sta.size();
	if( bBeginBoxSelectOld ){
		for (int i=0; i<j; i++) cmemBuf.AppendString( sta[i]->sKey2.c_str() ); 
	}else{
		for (int i=0; i<j; i++) cmemBuf.AppendString( sta[i]->sKey1.c_str() );
	}

	//sta.clear(); �����ꂶ�Ⴞ�߂݂���
	for (int i=0; i<j; i++) delete sta[i];

	CLayoutRange sSelectOld_Layout;
	GetDocument()->m_cLayoutMgr.LogicToLayout(sSelectOld, &sSelectOld_Layout);
	m_pCommanderView->ReplaceData_CEditView(
		sSelectOld_Layout,
		NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		false,
		m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
	);

	//	�I���G���A�̕���
	if(bBeginBoxSelectOld){
		m_pCommanderView->GetSelectionInfo().SetBoxSelect(bBeginBoxSelectOld);
		m_pCommanderView->GetSelectionInfo().m_sSelect=sRangeA;
	}else{
		m_pCommanderView->GetSelectionInfo().m_sSelect=sSelectOld_Layout;
	}
	if(nCaretPosYOLD==m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y || m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ) {
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom(), true );
	}else{
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo(), true );
	}
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		m_pCommanderView->m_pcOpeBlk->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}
	m_pCommanderView->RedrawAll();
}


/*! @brief �����s�̃}�[�W

	�A�����镨���s�œ��e������̕���1�s�ɂ܂Ƃ߂܂��D
	
	��`�I�����͂Ȃɂ����s���܂���D
	
	@note ���s�R�[�h���܂ރf�[�^���r���Ă���̂ŁA
	�t�@�C���̍ŏI�s�̓\�[�g�ΏۊO�ɂ��Ă��܂�
	
	@author hor
	@date 2001.12.03 hor �V�K�쐬
	@date 2001.12.21 hor �I��͈͂̒������W�b�N�����
*/
void CViewCommander::Command_MERGE(void)
{
	CLayoutInt		nCaretPosYOLD;
	const wchar_t*	pLinew;
	CLogicInt		nLineLen;
	int			j;
	CNativeW	cmemBuf;

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
		return;
	}
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		return;
	}

	nCaretPosYOLD=GetCaret().GetCaretLayoutPos().GetY();
	CLogicRange sSelectOld; //�͈͑I��
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		m_pCommanderView->GetSelectionInfo().m_sSelect,
		&sSelectOld
	);

	// 2001.12.21 hor
	// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
	// ���̍s���I��͈͂ɉ�����
	if ( sSelectOld.GetTo().x > 0 ) {
		const CLayout* pcLayout=GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().GetY2()); //2007.10.09 kobake �P�ʍ��݃o�O�C��
		if( NULL != pcLayout && EOL_NONE != pcLayout->GetLayoutEol() ){
			sSelectOld.GetToPointer()->y++;
			//sSelectOld.GetTo().y++;
		}
	}

	sSelectOld.SetFromX(CLogicInt(0));
	sSelectOld.SetToX(CLogicInt(0));

	//�s�I������ĂȂ�
	if(sSelectOld.IsLineOne()){
		return;
	}

	pLinew=NULL;
	cmemBuf.SetString(L"");
	for( CLogicInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().y; i++ ){
		const wchar_t*	pLine = GetDocument()->m_cDocLineMgr.GetLine(i)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ) continue;
		if( NULL == pLinew || wcscmp(pLine,pLinew) ){
			cmemBuf.AppendString( pLine );
		}
		pLinew=pLine;
	}
	j=GetDocument()->m_cDocLineMgr.GetLineCount();

	CLayoutRange sSelectOld_Layout;
	GetDocument()->m_cLayoutMgr.LogicToLayout(sSelectOld, &sSelectOld_Layout);

	m_pCommanderView->ReplaceData_CEditView(
		sSelectOld_Layout,
		NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		false,
		m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
	);
	j-=GetDocument()->m_cDocLineMgr.GetLineCount();

	//	�I���G���A�̕���
	m_pCommanderView->GetSelectionInfo().m_sSelect=sSelectOld_Layout;
	m_pCommanderView->GetSelectionInfo().m_sSelect.GetToPointer()->y -= j;

	if(nCaretPosYOLD==m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y){
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom(), true );
	}else{
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo(), true );
	}
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		m_pCommanderView->m_pcOpeBlk->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}
	m_pCommanderView->RedrawAll();

	if(j){
		TopOkMessage( m_pCommanderView->GetHwnd(), _T("%d�s���}�[�W���܂����B"), j);
	}else{
		InfoMessage( m_pCommanderView->GetHwnd(), _T("�}�[�W�\�ȍs���݂���܂���ł����B") );
	}
}



// To Here 2001.12.03 hor
	
/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09

	@date 2002.04.11 YAZAKI COsVersionInfo�̃J�v�Z���������܂��傤�B
	@date 2010.03.17 ATOK�p��SCS_SETRECONVERTSTRING => ATRECONVERTSTRING_SET�ɕύX
		2002.11.20 Stonee����̏��
*/
void CViewCommander::Command_Reconvert(void)
{
	const int ATRECONVERTSTRING_SET = 1;

	//�T�C�Y���擾
	int nSize = m_pCommanderView->SetReconvertStruct(NULL,UNICODE_BOOL);
	if( 0 == nSize )  // �T�C�Y�O�̎��͉������Ȃ�
		return ;
	
	bool bUseUnicodeATOK = false;
	//�o�[�W�����`�F�b�N
	COsVersionInfo cOs;
	if( cOs.OsDoesNOTSupportReconvert() ){
		
		// MSIME���ǂ���
		HWND hWnd = ImmGetDefaultIMEWnd(m_pCommanderView->GetHwnd());
		if (SendMessage(hWnd, m_pCommanderView->m_uWM_MSIME_RECONVERTREQUEST, FID_RECONVERT_VERSION, 0)){
			SendMessage(hWnd, m_pCommanderView->m_uWM_MSIME_RECONVERTREQUEST, 0, (LPARAM)m_pCommanderView->GetHwnd());
			return ;
		}

		// ATOK���g���邩�ǂ���
		TCHAR sz[256];
		ImmGetDescription(GetKeyboardLayout(0),sz,_countof(sz)); //�����̎擾
		if ( (_tcsncmp(sz,_T("ATOK"),4) == 0) && (NULL != m_pCommanderView->m_AT_ImmSetReconvertString) ){
			bUseUnicodeATOK = true;
		}else{
			//�Ή�IME�Ȃ�
			return;
		}
	}else{
		//���݂�IME���Ή����Ă��邩�ǂ���
		//IME�̃v���p�e�B
		if ( !(ImmGetProperty(GetKeyboardLayout(0),IGP_SETCOMPSTR) & SCS_CAP_SETRECONVERTSTRING) ){
			//�Ή�IME�Ȃ�			
			return ;
		}
#ifdef _UNICODE
#endif
	}
	
	//�T�C�Y�擾������
	if((UNICODE_BOOL || bUseUnicodeATOK) != UNICODE_BOOL){
		nSize = m_pCommanderView->SetReconvertStruct(NULL,UNICODE_BOOL || bUseUnicodeATOK);
		if( 0 == nSize )  // �T�C�Y�O�̎��͉������Ȃ�
			return ;
	}

	//IME�̃R���e�L�X�g�擾
	HIMC hIMC = ::ImmGetContext( m_pCommanderView->GetHwnd() );
	
	//�̈�m��
	PRECONVERTSTRING pReconv = (PRECONVERTSTRING)::HeapAlloc(
		GetProcessHeap(),
		HEAP_GENERATE_EXCEPTIONS,
		nSize
	);
	
	//�\���̐ݒ�
	// Size�̓o�b�t�@�m�ۑ����ݒ�
	pReconv->dwSize = nSize;
	pReconv->dwVersion = 0;
	m_pCommanderView->SetReconvertStruct( pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	//�ϊ��͈͂̒���
	if(bUseUnicodeATOK){
		(*m_pCommanderView->m_AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize, NULL,0);
	}

	//���������ϊ��͈͂�I������
	m_pCommanderView->SetSelectionFromReonvert(pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	//�ĕϊ����s
	if(bUseUnicodeATOK){
		(*m_pCommanderView->m_AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize, NULL, 0);
	}

	//�̈���
	::HeapFree(GetProcessHeap(),0,(LPVOID)pReconv);
	::ImmReleaseContext( m_pCommanderView->GetHwnd(), hIMC);
}

/*!	�R���g���[���R�[�h�̓���(�_�C�A���O)
	@author	MIK
	@date	2002/06/02
*/
void CViewCommander::Command_CtrlCode_Dialog( void )
{
	CDlgCtrlCode	cDlgCtrlCode;

	//�R���g���[���R�[�h���̓_�C�A���O��\������
	if( cDlgCtrlCode.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() ) )
	{
		//�R���g���[���R�[�h����͂���
		Command_WCHAR( cDlgCtrlCode.GetCharCode() );
	}
}

/*!	�����J�n�ʒu�֖߂�
	@author	ai
	@date	02/06/26
*/
void CViewCommander::Command_JUMP_SRCHSTARTPOS(void)
{
	if( m_pCommanderView->m_ptSrchStartPos_PHY.BothNatural() )
	{
		CLayoutPoint pt;
		/* �͈͑I�𒆂� */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			m_pCommanderView->m_ptSrchStartPos_PHY,
			&pt
		);
		//	2006.07.09 genta �I����Ԃ�ۂ�
		m_pCommanderView->MoveCursorSelecting( pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
	else
	{
		ErrorBeep();
	}
	return;
}

/*!	�����̊Ǘ�(�_�C�A���O)
	@author	MIK
	@date	2003/04/07
*/
void CViewCommander::Command_Favorite( void )
{
	CDlgFavorite	cDlgFavorite;

	//�_�C�A���O��\������
	if( !cDlgFavorite.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() ) )
	{
		return;
	}

	return;
}

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

/*! ��Ɏ�O�ɕ\��
	@date 2004.09.21 Moca
*/
void CViewCommander::Command_WINTOPMOST( LPARAM lparam )
{
	GetDocument()->m_pcEditWnd->WindowTopMost( int(lparam) );
}

/*!	@brief ���p���̐ݒ�
	@date Jan. 29, 2005 genta �V�K�쐬
*/
void CViewCommander::Command_SET_QUOTESTRING( const wchar_t* quotestr )
{
	if( quotestr == NULL )
		return;

	wcsncpy( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou, quotestr,
		_countof( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou ));
	
	GetDllShareData().m_Common.m_sFormat.m_szInyouKigou[ _countof( GetDllShareData().m_Common.m_sFormat.m_szInyouKigou ) - 1 ] = L'\0';
}

/*!	@brief �E�B���h�E�ꗗ�|�b�v�A�b�v�\�������i�t�@�C�����̂݁j
	@date  2006.03.23 fon �V�K�쐬
	@date  2006.05.19 genta �R�}���h���s�v����\�������ǉ�
	@date  2007.07.07 genta �R�}���h���s�v���̒l��ύX
*/
void CViewCommander::Command_WINLIST( int nCommandFrom )
{
	CEditWnd	*pCEditWnd;
	pCEditWnd = GetDocument()->m_pcEditWnd;

	//�E�B���h�E�ꗗ���|�b�v�A�b�v�\������
	pCEditWnd->PopupWinList(( nCommandFrom & FA_FROMKEYBOARD ) != FA_FROMKEYBOARD );
	// 2007.02.27 ryoji �A�N�Z�����[�^�L�[����łȂ���΃}�E�X�ʒu��

}


/*!	@brief �}�N���p�A�E�g�v�b�g�E�C���h�E�ɕ\��
	@date 2006.04.26 maru �V�K�쐬
*/
void CViewCommander::Command_TRACEOUT( const wchar_t* outputstr, int nFlgOpt )
{
	if( outputstr == NULL )
		return;

	// 0x01 ExpandParameter�ɂ�镶����W�J�L��
	if (nFlgOpt & 0x01) {
		wchar_t Buffer[2048];
		CSakuraEnvironment::ExpandParameter(outputstr, Buffer, 2047);
		CShareData::getInstance()->TraceOutString( Buffer );
	} else {
		CShareData::getInstance()->TraceOutString(outputstr );
	}

	// 0x02 ���s�R�[�h�̗L��
	if ((nFlgOpt & 0x02) == 0) CShareData::getInstance()->TraceOutString( L"\r\n" );

}



