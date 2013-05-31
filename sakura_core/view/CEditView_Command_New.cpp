/*!	@file
	@brief CEditView�N���X�̃R�}���h�����n�֐��Q

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, asa-o, hor
	Copyright (C) 2002, YAZAKI, hor, genta. aroka, MIK, minfu, KK, �����
	Copyright (C) 2003, MIK, Moca
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, ryoji, genta, D.S.Koba
	Copyright (C) 2006, genta, Moca, fon
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "view/CEditView.h"
#include "uiparts/CWaitCursor.h"
#include "charset/charcode.h"
#include "COpe.h" ///	2002/2/3 aroka from here
#include "COpeBlk.h" ///
#include "doc/CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "doc/CDocReader.h"
#include "doc/layout/CLayout.h"
#include "doc/logic/CDocLine.h"
#include "window/CEditWnd.h"
#include "dlg/CDlgCtrlCode.h"	//�R���g���[���R�[�h�̓���(�_�C�A���O)
#include "dlg/CDlgFavorite.h"	//�����̊Ǘ�	//@@@ 2003.04.08 MIK
#include "debug/CRunningTimer.h"

using namespace std; // 2002/2/3 aroka



/*!	���݈ʒu�Ƀf�[�^��}�� Ver0

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CEditView::InsertData_CEditView(
	CLayoutPoint	ptInsertPos,	// [in] �}���ʒu
	const wchar_t*	pData,			// [in] �}���e�L�X�g
	int				nDataLen,		// [in] �}���e�L�X�g���B�����P�ʁB
	CLayoutPoint*	pptNewPos,		// [out] �}�����ꂽ�����̎��̈ʒu�̃��C�A�E�g�ʒu
	bool			bRedraw
)
{
#ifdef _DEBUG
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::InsertData_CEditView" );
#endif

	//2007.10.18 kobake COpe�����������ɂ܂Ƃ߂�
	CInsertOpe* pcOpe = NULL;
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new CInsertOpe();
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			ptInsertPos,
			&pcOpe->m_ptCaretPos_PHY_Before
		);
	}


	pptNewPos->y = 0;			// �}�����ꂽ�����̎��̈ʒu�̃��C�A�E�g�s
	pptNewPos->x = 0;			// �}�����ꂽ�����̎��̈ʒu�̃��C�A�E�g�ʒu

	// �e�L�X�g���I������Ă��邩
	if( GetSelectionInfo().IsTextSelected() ){
		DeleteData( bRedraw );
		ptInsertPos = GetCaret().GetCaretLayoutPos();
	}

	//�e�L�X�g�擾 -> pLine, nLineLen, pcLayout
	CLogicInt		nLineLen;
	const CLayout*	pcLayout;
	bool			bHintPrev = false;	// �X�V���O�s����ɂȂ�\�������邱�Ƃ���������
	bool			bHintNext = false;	// �X�V�����s����ɂȂ�\�������邱�Ƃ���������
	bool			bKinsoku;			// �֑��̗L��
	const wchar_t*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( ptInsertPos.GetY2(), &nLineLen, &pcLayout );
	bool			bLineModifiedChange = (pLine)? !CModifyVisitor().IsLineModified(pcLayout->GetDocLineRef()): true;

	//�֑�������ꍇ��1�s�O����ĕ`����s��	@@@ 2002.04.19 MIK
	bKinsoku = ( m_pTypeData->m_bWordWrap
			 || m_pTypeData->m_bKinsokuHead	//@@@ 2002.04.19 MIK
			 || m_pTypeData->m_bKinsokuTail	//@@@ 2002.04.19 MIK
			 || m_pTypeData->m_bKinsokuRet	//@@@ 2002.04.19 MIK
			 || m_pTypeData->m_bKinsokuKuto );	//@@@ 2002.04.19 MIK

	CLayoutInt	nLineAllColLen;
	CLogicInt	nIdxFrom = CLogicInt(0);
	CNativeW	cMem;
	cMem.SetString(L"");
	if( pLine ){
		// �X�V���O�s����ɂȂ�\���𒲂ׂ�	// 2009.02.17 ryoji
		// ���܂�Ԃ��s���ւ̋�Ǔ_���͂őO�̍s�������X�V�����ꍇ������
		// ���}���ʒu�͍s�r���ł���Ǔ_���́{���[�h���b�v�őO�̕����񂩂瑱���đO�s�ɉ�荞�ޏꍇ������
		if( pcLayout->GetLogicOffset() && bKinsoku ){	// �܂�Ԃ����C�A�E�g�s���H
			bHintPrev = true;	// �X�V���O�s����ɂȂ�\��������
		}

		// �X�V�����s����ɂȂ�\���𒲂ׂ�	// 2009.02.17 ryoji
		// ���܂�Ԃ��s���ւ̕������͂╶����\��t���Ō��ݍs�͍X�V���ꂸ���s�Ȍオ�X�V�����ꍇ������
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		nIdxFrom = LineColumnToIndex2( pcLayout, ptInsertPos.GetX2(), &nLineAllColLen );

		// �s�I�[���E�ɑ}�����悤�Ƃ���
		if( nLineAllColLen > 0 ){
			// �I�[���O����}���ʒu�܂ŋ󔒂𖄂߂�ׂ̏���
			// �s�I�[�����炩�̉��s�R�[�h��?
			if( EOL_NONE != pcLayout->GetLayoutEol() ){
				nIdxFrom = nLineLen - CLogicInt(1);
				for( int i = 0; i < ptInsertPos.GetX2() - nLineAllColLen + 1; ++i ){
					cMem += L' ';
				}
				cMem.AppendString( pData, nDataLen );
			}
			else{
				nIdxFrom = nLineLen;
				for( int i = 0; i < ptInsertPos.GetX2() - nLineAllColLen; ++i ){
					cMem += L' ';
				}
				cMem.AppendString( pData, nDataLen );
				if( 0 < nDataLen && WCODE::IsLineDelimiter(pData[nDataLen-1]) ){
					bHintNext = true;	// �X�V�����s����ɂȂ�\��������
				}
			}
		}
		else{
			cMem.AppendString( pData, nDataLen );
		}
	}
	else{
		// �X�V���O�s����ɂȂ�\���𒲂ׂ�	// 2009.02.17 ryoji
		const CLayout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.GetBottomLayout();
		if( pcLayoutWk && pcLayoutWk->GetLayoutEol() == EOL_NONE && bKinsoku ){	// �܂�Ԃ����C�A�E�g�s���H�i�O�s�̏I�[�Œ����j
			bHintPrev = true;	// �X�V���O�s����ɂȂ�\��������
		}

		for( CLayoutInt i = CLayoutInt(0); i < ptInsertPos.GetX2(); ++i ){
			cMem += L' ';
		}
		cMem.AppendString( pData, nDataLen );
	}


	if( !m_bDoing_UndoRedo && pcOpe ){	// �A���h�D�E���h�D�̎��s����
		if( pLine ){
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				CLayoutPoint(LineIndexToColumn( pcLayout, nIdxFrom ), ptInsertPos.y),
				&pcOpe->m_ptCaretPos_PHY_Before
			);
		}
		else{
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
				CLayoutPoint(CLayoutInt(0), ptInsertPos.y),
				&pcOpe->m_ptCaretPos_PHY_Before
			);
		}
	}


	// ������}��
	CLayoutInt	nModifyLayoutLinesOld=CLayoutInt(0);
	CLayoutInt	nInsLineNum;		/* �}���ɂ���đ��������C�A�E�g�s�̐� */
	m_pcEditDoc->m_cLayoutMgr.InsertData_CLayoutMgr(
		ptInsertPos.GetY2(),
		nIdxFrom,
		cMem.GetStringPtr(),
		CLogicInt(cMem.GetStringLength()),
		&nModifyLayoutLinesOld,
		&nInsLineNum,
		pptNewPos
	);

	// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
	const wchar_t*	pLine2;
	CLogicInt		nLineLen2;
	pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( pptNewPos->GetY2(), &nLineLen2, &pcLayout );
	if( pLine2 ){
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		pptNewPos->x = pptNewPos->GetX2(); //LineIndexToColumn( pcLayout, pptNewPos->GetX2() );
	}

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if( pptNewPos->x >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){
		if( m_pTypeData->m_bKinsokuRet
		 || m_pTypeData->m_bKinsokuKuto )	//@@@ 2002.04.16 MIK
		{
			if( m_pcEditDoc->m_cLayoutMgr.IsEndOfLine( *pptNewPos ) )	//@@@ 2002.04.18
			{
				pptNewPos->x = 0;
				pptNewPos->y++;
			}
		}
		else
		{
			// Oct. 7, 2002 YAZAKI
			pptNewPos->x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : CLayoutInt(0);
			pptNewPos->y++;
		}
	}

	// ��ԑJ��
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		m_pcEditDoc->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	// �ĕ`��
	// �s�ԍ��\���ɕK�v�ȕ���ݒ�
	if( m_pcEditWnd->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
		// �L�����b�g�̕\���E�X�V
		GetCaret().ShowEditCaret();
	}
	else{
		PAINTSTRUCT ps;

		if( bRedraw ){
			CLayoutInt nStartLine(ptInsertPos.y);
			// 2013.05.08 �܂�Ԃ��s��EOF���O�ŉ��s�����Ƃ�EOF���ĕ`�悳��Ȃ��o�O�̏C��
			if( nModifyLayoutLinesOld < 1 ){
				nModifyLayoutLinesOld = CLayoutInt(1);
			}
			// 2011.12.26 ���K�\���L�[���[�h�E����������Ȃǂ́A���W�b�N�s���܂ł����̂ڂ��čX�V����K�v������
			{
				const CLayout* pcLayoutLineFirst = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( ptInsertPos.GetY2() );
				while( pcLayoutLineFirst && 0 != pcLayoutLineFirst->GetLogicOffset() ){
					pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
					if( bHintPrev ){
						bHintPrev = false;
					}
					nStartLine--;
					nModifyLayoutLinesOld++;
				}
			}
			if( 0 != nInsLineNum ){
				// �X�N���[���o�[�̏�Ԃ��X�V����
				AdjustScrollBars();

				// �`��J�n�s�ʒu�𒲐�����	// 2009.02.17 ryoji
				if( bHintPrev ){	// �X�V���O�s����ɂȂ�\��������
					nStartLine--;
				}

				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();
				ps.rcPaint.top = GetTextArea().GetAreaTop() + GetTextMetrics().GetHankakuDy() * (Int)(nStartLine - GetTextArea().GetViewTopLine());
				ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
			}
			else{
				// �`��J�n�s�ʒu�ƕ`��s���𒲐�����	// 2009.02.17 ryoji
				if( bHintPrev ){	// �X�V���O�s����ɂȂ�\��������
					nStartLine--;
					nModifyLayoutLinesOld++;
				}
				if( bHintNext ){	// �X�V�����s����ɂȂ�\��������
					nModifyLayoutLinesOld++;
				}

	//			ps.rcPaint.left = GetTextArea().GetAreaLeft();
				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();

				// 2002.02.25 Mod By KK ���s (ptInsertPos.y - GetTextArea().GetViewTopLine() - 1); => (ptInsertPos.y - GetTextArea().GetViewTopLine());
				//ps.rcPaint.top = GetTextArea().GetAreaTop() + GetTextMetrics().GetHankakuDy() * (ptInsertPos.y - GetTextArea().GetViewTopLine() - 1);
				ps.rcPaint.top = GetTextArea().GetAreaTop() + GetTextMetrics().GetHankakuDy() * (Int)(nStartLine - GetTextArea().GetViewTopLine());
				ps.rcPaint.bottom = ps.rcPaint.top + GetTextMetrics().GetHankakuDy() * (Int)(nModifyLayoutLinesOld);
				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}
				if( GetTextArea().GetAreaBottom() < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
				}
			}
			HDC hdc = this->GetDC();
			OnPaint( hdc, &ps, FALSE );
			this->ReleaseDC( hdc );

			// �s�ԍ��i�ύX�s�j�\���͉��s�P�ʂ̍s������X�V����K�v������	// 2009.03.26 ryoji
			if( bLineModifiedChange ){	// ���ύX�������s���ύX���ꂽ
				const CLayout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nStartLine );
				if( pcLayoutWk && pcLayoutWk->GetLogicOffset() ){	// �܂�Ԃ����C�A�E�g�s���H
					Call_OnPaint( PAINT_LINENUMBER, false );
				}
			}
		}
	}

	if( !m_bDoing_UndoRedo && pcOpe ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe->m_pcmemData.Clear();					/* ����Ɋ֘A����f�[�^ */
	}



	//2007.10.18 kobake ������COpe�������܂Ƃ߂�
	if( !m_bDoing_UndoRedo ){
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			*pptNewPos,
			&pcOpe->m_ptCaretPos_PHY_After
		);

		// ����̒ǉ�
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
}


/*!	�w��ʒu�̎w�蒷�f�[�^�폜

	@date 2002/03/24 YAZAKI bUndo�폜
	@date 2002/05/12 YAZAKI bRedraw, bRedraw2�폜�i���FALSE������j
	@date 2007/10/17 kobake (�d�v)pcMem�̏��L�҂������ɂ��COpe�Ɉڂ�����ڂ�Ȃ������肷��U�镑����
	                        ���ɂ�₱���������̌��ɂȂ邽�߁A��ɁApcMem�̏��L�҂͈ڂ��Ȃ��悤�Ɏd�l�ύX�B
*/
void CEditView::DeleteData2(
	const CLayoutPoint& _ptCaretPos,
	CLogicInt			nDelLen,
	CNativeW*			pcMem		//!< [out]
)
{
#ifdef _DEBUG
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DeleteData(1)" );
#endif
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nIdxFrom;

	const CLayout* pcLayout;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( _ptCaretPos.GetY2(), &nLineLen, &pcLayout );
	if( NULL == pLine ){
		return;
	}
	nIdxFrom = LineColumnToIndex( pcLayout, _ptCaretPos.GetX2() );

	//2007.10.18 kobake COpe�̐����������ɂ܂Ƃ߂�
	CDeleteOpe*	pcOpe = NULL;
	if( !m_bDoing_UndoRedo ){
		pcOpe = new CDeleteOpe();
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint( LineIndexToColumn( pcLayout, nIdxFrom ), _ptCaretPos.GetY2() ),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint( LineIndexToColumn( pcLayout, nIdxFrom + nDelLen ), _ptCaretPos.GetY2() ),
			&pcOpe->m_ptCaretPos_PHY_To
		);
	}

	/* �f�[�^�폜 */
	CLayoutInt	nModifyLayoutLinesOld;
	CLayoutInt	nModifyLayoutLinesNew;
	CLayoutInt	nDeleteLayoutLines;
	m_pcEditDoc->m_cLayoutMgr.DeleteData_CLayoutMgr(
		_ptCaretPos.GetY2(),
		nIdxFrom,
		nDelLen,
		&nModifyLayoutLinesOld,
		&nModifyLayoutLinesNew,
		&nDeleteLayoutLines,
		pcMem
	);

	/* �I���G���A�̐擪�փJ�[�\�����ړ� */
	GetCaret().MoveCursor( _ptCaretPos, false );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();

	//2007.10.18 kobake COpe�̒ǉ��������ɂ܂Ƃ߂�
	if( pcOpe ){
		pcOpe->m_nDataLen = pcMem->GetStringLength();	/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		pcOpe->m_pcmemData = *pcMem;					/* ����Ɋ֘A����f�[�^ */
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
			_ptCaretPos,
			&pcOpe->m_ptCaretPos_PHY_After
		);
		// ����̒ǉ�
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

}





/*!	�J�[�\���ʒu�܂��͑I���G���A���폜

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CEditView::DeleteData(
	bool	bRedraw
//	BOOL	bUndo	/* Undo���삩�ǂ��� */
)
{
#ifdef _DEBUG
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DeleteData(2)" );
#endif
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLayoutInt	nLineNum;
	CLogicInt	nCurIdx;
	CLogicInt	nNxtIdx;
	CLogicInt	nIdxFrom;
	CLogicInt	nIdxTo;
	CLogicInt	nDelPos;
	CLogicInt	nDelLen;
	CLogicInt	nDelPosNext;
	CLogicInt	nDelLenNext;
	CLayoutRect		rcSel;
	const CLayout*	pcLayout;

	// �e�L�X�g�̑��݂��Ȃ��G���A�̍폜�́A�I��͈͂̃L�����Z���ƃJ�[�\���ړ��݂̂Ƃ���	// 2008.08.05 ryoji
	if( GetSelectionInfo().IsTextSelected() ){		// �e�L�X�g���I������Ă��邩
		if( IsEmptyArea( GetSelectionInfo().m_sSelect.GetFrom(), GetSelectionInfo().m_sSelect.GetTo(), true, GetSelectionInfo().IsBoxSelecting() ) ){
			// �J�[�\����I��͈͂̍���Ɉړ�
			GetCaret().MoveCursor(
				CLayoutPoint(
					GetSelectionInfo().m_sSelect.GetFrom().GetX2() < GetSelectionInfo().m_sSelect.GetTo().GetX2() ? GetSelectionInfo().m_sSelect.GetFrom().GetX2() : GetSelectionInfo().m_sSelect.GetTo().GetX2(),
					GetSelectionInfo().m_sSelect.GetFrom().GetY2() < GetSelectionInfo().m_sSelect.GetTo().GetY2() ? GetSelectionInfo().m_sSelect.GetFrom().GetY2() : GetSelectionInfo().m_sSelect.GetTo().GetY2()
				), bRedraw
			);
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			GetSelectionInfo().DisableSelectArea( bRedraw );
			return;
		}
	}else{
		if( IsEmptyArea( GetCaret().GetCaretLayoutPos() ) ){
			return;
		}
	}

	CLayoutPoint ptCaretPosOld = GetCaret().GetCaretLayoutPos();

	/* �e�L�X�g���I������Ă��邩 */
	if( GetSelectionInfo().IsTextSelected() ){
		CWaitCursor cWaitCursor( this->GetHwnd() );  // 2002.02.05 hor
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
					GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
				)
			);
		}

		/* ��`�͈͑I�𒆂� */
		if( GetSelectionInfo().IsBoxSelecting() ){
			m_pcEditDoc->m_cDocEditor.SetModified(true,bRedraw);	//	2002/06/04 YAZAKI ��`�I�����폜�����Ƃ��ɕύX�}�[�N�����Ȃ��B

			SetDrawSwitch(false);	// 2002.01.25 hor
			/* �I��͈͂̃f�[�^���擾 */
			/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
			/* �Q�_��Ίp�Ƃ����`�����߂� */
			TwoPointToRect(
				&rcSel,
				GetSelectionInfo().m_sSelect.GetFrom(),	// �͈͑I���J�n
				GetSelectionInfo().m_sSelect.GetTo()		// �͈͑I���I��
			);
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			GetSelectionInfo().DisableSelectArea( bRedraw );

			nIdxFrom = CLogicInt(0);
			nIdxTo = CLogicInt(0);
			for( nLineNum = rcSel.bottom; nLineNum >= rcSel.top - 1; nLineNum-- ){
				nDelPosNext = nIdxFrom;
				nDelLenNext	= nIdxTo - nIdxFrom;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
				if( pLine ){
					using namespace WCODE;

					/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
					nIdxFrom = LineColumnToIndex( pcLayout, rcSel.left  );
					nIdxTo	 = LineColumnToIndex( pcLayout, rcSel.right );

					for( CLogicInt i = nIdxFrom; i <= nIdxTo; ++i ){
						if( WCODE::IsLineDelimiter(pLine[i]) ){
							nIdxTo = i;
							break;
						}
					}
				}else{
					nIdxFrom = CLogicInt(0);
					nIdxTo	 = CLogicInt(0);
				}
				nDelPos = nDelPosNext;
				nDelLen	= nDelLenNext;
				if( nLineNum < rcSel.bottom && 0 < nDelLen ){
					CNativeW	pcMemDeleted;
					// �w��ʒu�̎w�蒷�f�[�^�폜
					DeleteData2(
						CLayoutPoint(rcSel.left, nLineNum + 1),
						nDelLen,
						&pcMemDeleted
					);
				}
			}
			SetDrawSwitch(true);	// 2002.01.25 hor

			/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
			if ( m_pcEditWnd->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
				/* �L�����b�g�̕\���E�X�V */
				GetCaret().ShowEditCaret();
			}
			if( bRedraw ){
				/* �X�N���[���o�[�̏�Ԃ��X�V���� */
				AdjustScrollBars();

				/* �ĕ`�� */
				Call_OnPaint(PAINT_LINENUMBER | PAINT_BODY, false);
			}
			/* �I���G���A�̐擪�փJ�[�\�����ړ� */
			this->UpdateWindow();
			
			CLayoutPoint caretOld = CLayoutPoint(rcSel.left, rcSel.top);
			m_pcEditDoc->m_cLayoutMgr.GetLineStr( rcSel.top, &nLineLen, &pcLayout );
			if( rcSel.left <= pcLayout->CalcLayoutWidth( m_pcEditDoc->m_cLayoutMgr ) ){
				// EOL��荶�Ȃ當���̒P�ʂɂ��낦��
				CLogicInt nIdxCaret = LineColumnToIndex( pcLayout, rcSel.left );
				caretOld.SetX( LineIndexToColumn( pcLayout, nIdxCaret ) );
			}
			GetCaret().MoveCursor( caretOld, bRedraw );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				CMoveCaretOpe*		pcOpe = new CMoveCaretOpe();
				m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
					ptCaretPosOld,
					&pcOpe->m_ptCaretPos_PHY_Before
				);

				pcOpe->m_ptCaretPos_PHY_After = GetCaret().GetCaretLogicPos();	// �����̃L�����b�g�ʒu
				/* ����̒ǉ� */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}
		}else{
			/* �f�[�^�u�� �폜&�}���ɂ��g���� */
			ReplaceData_CEditView(
				GetSelectionInfo().m_sSelect,
				NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
				L"",					/* �}������f�[�^ */
				CLogicInt(0),			/* �}������f�[�^�̒��� */
				bRedraw,
				m_bDoing_UndoRedo?NULL:m_pcOpeBlk
			);
		}
	}else{
		/* ���ݍs�̃f�[�^���擾 */
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );
		if( NULL == pLine ){
			goto end_of_func;
//			return;
		}
		/* �Ō�̍s�ɃJ�[�\�������邩�ǂ��� */
		bool bLastLine = ( GetCaret().GetCaretLayoutPos().GetY() == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 );

		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
		nCurIdx = LineColumnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
//		MYTRACE( _T("nLineLen=%d nCurIdx=%d \n"), nLineLen, nCurIdx);
		if( nCurIdx == nLineLen && bLastLine ){	/* �S�e�L�X�g�̍Ō� */
			goto end_of_func;
//			return;
		}

		/* �w�肳�ꂽ���̕����̃o�C�g���𒲂ׂ� */
		CLayoutInt	nNxtPos;
		if( WCODE::IsLineDelimiter(pLine[nCurIdx]) ){
			/* ���s */
			nNxtIdx = nCurIdx + pcLayout->GetLayoutEol().GetLen();
			nNxtPos = GetCaret().GetCaretLayoutPos().GetX() + CLayoutInt((Int)pcLayout->GetLayoutEol().GetLen()); //�����s�R�[�h�̕������𕶎����ƌ��Ȃ�
		}
		else{
			nNxtIdx = CLogicInt(CNativeW::GetCharNext( pLine, nLineLen, &pLine[nCurIdx] ) - pLine);
			// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
			nNxtPos = LineIndexToColumn( pcLayout, nNxtIdx );
		}


		/* �f�[�^�u�� �폜&�}���ɂ��g���� */
		CLayoutRange sDelRange;
		sDelRange.SetFrom(GetCaret().GetCaretLayoutPos());
		sDelRange.SetTo(CLayoutPoint(nNxtPos,GetCaret().GetCaretLayoutPos().GetY()));
		ReplaceData_CEditView(
			sDelRange,
			NULL,				/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
			L"",				/* �}������f�[�^ */
			CLogicInt(0),		/* �}������f�[�^�̒��� */
			bRedraw,
			m_bDoing_UndoRedo?NULL:m_pcOpeBlk
		);
	}

	m_pcEditDoc->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() > 0 ){
		if( GetCaret().GetCaretLayoutPos().GetY() > m_pcEditDoc->m_cLayoutMgr.GetLineCount()	- 1	){
			/* ���ݍs�̃f�[�^���擾 */
			const CLayout*	pcLayout;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_pcEditDoc->m_cLayoutMgr.GetLineCount() - CLayoutInt(1), &nLineLen, &pcLayout );
			if( NULL == pLine ){
				goto end_of_func;
			}
			/* ���s�ŏI����Ă��邩 */
			if( ( EOL_NONE != pcLayout->GetLayoutEol() ) ){
				goto end_of_func;
			}
			/*�t�@�C���̍Ō�Ɉړ� */
			GetCommander().Command_GOFILEEND( false );
		}
	}
end_of_func:;

	return;
}


void CEditView::ReplaceData_CEditView2(
	const CLogicRange&	sDelRange,			// �폜�͈́B���W�b�N�P�ʁB
	CNativeW*			pcmemCopyOfDeleted,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
	const wchar_t*		pInsData,			// �}������f�[�^
	CLogicInt			nInsDataLen,		// �}������f�[�^�̒���
	bool				bRedraw,
	COpeBlk*			pcOpeBlk
)
{
	CLayoutRange sDelRangeLayout;
	this->m_pcEditDoc->m_cLayoutMgr.LogicToLayout(sDelRange,&sDelRangeLayout);
	ReplaceData_CEditView(sDelRangeLayout,pcmemCopyOfDeleted,pInsData,nInsDataLen,bRedraw,pcOpeBlk);
}




/* �f�[�^�u�� �폜&�}���ɂ��g���� */
// Jun 23, 2000 genta �ϐ��������������Y��Ă����̂��C��
// Jun. 1, 2000 genta DeleteData����ړ�����
void CEditView::ReplaceData_CEditView(
	CLayoutRange	sDelRange,			//!< [in]  �폜�͈̓��C�A�E�g�P��
	CNativeW*		pcmemCopyOfDeleted,	//!< [out] �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
	const wchar_t*	pInsData,			//!< [in]  �}������f�[�^
	CLogicInt		nInsDataLen,		//!< [in]  �}������f�[�^�̒���
	bool			bRedraw,
	COpeBlk*		pcOpeBlk
)
{
	bool bLineModifiedChange;

	{
		//	May. 29, 2000 genta
		//	From Here
		//	�s�̌�낪�I������Ă����Ƃ��̕s���������邽�߁C
		//	�I��̈悩��s���ȍ~�̕�������菜���D

		//	�擪
		const CLayout*	pcLayout;
		CLogicInt		len;
		const wchar_t*	line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sDelRange.GetFrom().GetY2(), &len, &pcLayout );
		bLineModifiedChange = (line)? !CModifyVisitor().IsLineModified(pcLayout->GetDocLineRef()): true;
		if( line ){
			CLogicInt pos = LineColumnToIndex( pcLayout, sDelRange.GetFrom().GetX2() );
			//	Jun. 1, 2000 genta
			//	����s�̍s���ȍ~�݂̂��I������Ă���ꍇ���l������

			//	Aug. 22, 2000 genta
			//	�J�n�ʒu��EOF�̌��̂Ƃ��͎��s�ɑ��鏈�����s��Ȃ�
			//	���������Ă��܂��Ƒ��݂��Ȃ��s��Point���ė�����D
			if( sDelRange.GetFrom().y < m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 && pos >= len){
				if( sDelRange.GetFrom().y == sDelRange.GetTo().y  ){
					//	GetSelectionInfo().m_sSelect.GetFrom().y <= GetSelectionInfo().m_sSelect.GetTo().y �̓`�F�b�N���Ȃ�
					CLayoutPoint tmp = sDelRange.GetFrom();
					tmp.y++;
					tmp.x = CLayoutInt(0);
					sDelRange.Set(tmp);
				}
				else {
					sDelRange.GetFromPointer()->y++;
					sDelRange.SetFromX(CLayoutInt(0));
				}
			}
		}

		//	����
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( sDelRange.GetTo().GetY2(), &len, &pcLayout );
		if( line ){
			CLayoutInt p = LineIndexToColumn( pcLayout, len );

			if( sDelRange.GetTo().x > p ){
				sDelRange.SetToX( p );
			}
		}
		//	To Here
	}

	//�폜�͈̓��W�b�N�P�� sDelRange -> sDelRangeLogic
	CLogicRange sDelRangeLogic;
	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(
		sDelRange,
		&sDelRangeLogic
	);


	CLogicPoint		ptCaretPos_PHY_Old;

	ptCaretPos_PHY_Old = GetCaret().GetCaretLogicPos();
	if( pcOpeBlk ){	/* �A���h�D�E���h�D�̎��s���� */
		/* ����̒ǉ� */
		pcOpeBlk->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}

	CDeleteOpe* pcDeleteOpe = NULL;	// �ҏW����v�f COpe
	if( pcOpeBlk ){
		pcDeleteOpe = new CDeleteOpe();
		pcDeleteOpe->m_ptCaretPos_PHY_Before = sDelRangeLogic.GetFrom();
		pcDeleteOpe->m_ptCaretPos_PHY_To = sDelRangeLogic.GetTo();
		pcDeleteOpe->m_ptCaretPos_PHY_After = pcDeleteOpe->m_ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu
	}

	CNativeW pcMemDeleted;
	/*
	|| �o�b�t�@�T�C�Y�̒���
	*/
//	if( 2000 < sDelRange.GetTo().y - sDelRange.GetFrom().y ){
//		pcMemDeleted.AllocStringBuffer( 1024000 );
//	}


	/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	// 2009.07.18 ryoji �u���と�u���O�Ɉʒu��ύX�i�u���ゾ�Ɣ��]���s���ɂȂ��ĉ��� Wiki BugReport/43�j
	GetSelectionInfo().DisableSelectArea( bRedraw );

	/* ������u�� */
	LayoutReplaceArg LRArg;
	LRArg.sDelRange    = sDelRange;		//!< �폜�͈̓��C�A�E�g
	LRArg.pcmemDeleted = &pcMemDeleted;	//!< [out] �폜���ꂽ�f�[�^
	LRArg.pInsData     = pInsData;		//!< �}������f�[�^
	LRArg.nInsDataLen  = nInsDataLen;	//!< �}������f�[�^�̒���
	m_pcEditDoc->m_cLayoutMgr.ReplaceData_CLayoutMgr(
		&LRArg
	);

	//	Jan. 30, 2001 genta
	//	�ĕ`��̎��_�Ńt�@�C���X�V�t���O���K�؂ɂȂ��Ă��Ȃ��Ƃ����Ȃ��̂�
	//	�֐��̖������炱���ֈړ�
	/* ��ԑJ�� */
	if( pcOpeBlk ){	/* �A���h�D�E���h�D�̎��s���� */
		m_pcEditDoc->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	if( m_pcEditWnd->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
		/* �L�����b�g�̕\���E�X�V */
		GetCaret().ShowEditCaret();
	}
	else{
		/* �ĕ`�� */
		if( bRedraw ){
			/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
			//	Jan. 30, 2001 genta	�\��t���ōs��������ꍇ�̍l���������Ă���
			if( 0 != LRArg.nAddLineNum ){
				Call_OnPaint( PAINT_LINENUMBER | PAINT_BODY, false);
			}
			else{
				// �����������s�Ȃ�������ɕω�������				// 2009.11.11 ryoji
				// EOF�̂ݍs���ǉ��ɂȂ�̂ŁA1�s�]���ɕ`�悷��B
				// �i�����������s���聨�Ȃ��ɕω�����ꍇ�̖���EOF�����͕`��֐����ōs����j
				int nAddLine = ( LRArg.ptLayoutNew.GetY2() > LRArg.sDelRange.GetTo().GetY2() )? 1: 0;

				PAINTSTRUCT ps;

				ps.rcPaint.left = 0;
				ps.rcPaint.right = GetTextArea().GetAreaRight();

				/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
				ps.rcPaint.top = GetTextArea().GetAreaTop() + (Int)(LRArg.nModLineFrom - GetTextArea().GetViewTopLine())* GetTextMetrics().GetHankakuDy();
				// 2011.12.26 ���K�\���L�[���[�h�E����������Ȃǂ́A���W�b�N�s���܂ł����̂ڂ��čX�V����K�v������
				{
					const CLayout* pcLayoutLineFirst = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( LRArg.nModLineFrom );
					while( pcLayoutLineFirst && 0 != pcLayoutLineFirst->GetLogicOffset() ){
						pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
						ps.rcPaint.top -= GetTextMetrics().GetHankakuDy();
					}
				}
				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}
				ps.rcPaint.bottom = GetTextArea().GetAreaTop() + (Int)(LRArg.nModLineTo - GetTextArea().GetViewTopLine() + 1 + nAddLine)* GetTextMetrics().GetHankakuDy();
				if( GetTextArea().GetAreaBottom() < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
				}

				HDC hdc = this->GetDC();
				OnPaint( hdc, &ps, FALSE );
				this->ReleaseDC( hdc );

				// �s�ԍ��i�ύX�s�j�\���͉��s�P�ʂ̍s������X�V����K�v������	// 2009.03.26 ryoji
				if( bLineModifiedChange ){	// ���ύX�������s���ύX���ꂽ
					const CLayout* pcLayoutWk = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( LRArg.nModLineFrom );
					if( pcLayoutWk && pcLayoutWk->GetLogicOffset() ){	// �܂�Ԃ����C�A�E�g�s���H
						Call_OnPaint( PAINT_LINENUMBER, false );
					}
				}
			}
		}
	}

	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
	if( pcmemCopyOfDeleted && 0 < pcMemDeleted.GetStringLength() ){
		*pcmemCopyOfDeleted = pcMemDeleted;
	}

	if( pcOpeBlk && 0 < pcMemDeleted.GetStringLength() ){
		pcDeleteOpe->m_nDataLen = pcMemDeleted.GetStringLength();	/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		pcDeleteOpe->m_pcmemData = pcMemDeleted;					/* ����Ɋ֘A����f�[�^ */
		/* ����̒ǉ� */
		pcOpeBlk->AppendOpe( pcDeleteOpe );
	}
	pcMemDeleted.Clear();


	if( pcOpeBlk && 0 < nInsDataLen ){
		CInsertOpe* pcInsertOpe = new CInsertOpe();
		pcInsertOpe->m_ptCaretPos_PHY_Before = sDelRangeLogic.GetFrom();	// 2009.07.18 ryoji ���C�A�E�g�͕ω�����̂ɈȑO��sDelRange����LayoutToLogic�Ōv�Z���Ă����o�O���C��
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(LRArg.ptLayoutNew,   &pcInsertOpe->m_ptCaretPos_PHY_After);

		/* ����̒ǉ� */
		pcOpeBlk->AppendOpe( pcInsertOpe );
	}


	// �}������ʒu�փJ�[�\�����ړ�
	GetCaret().MoveCursor(
		LRArg.ptLayoutNew,	// �}�����ꂽ�����̎��̈ʒu
		bRedraw
	);
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();


	if( pcOpeBlk ){	/* �A���h�D�E���h�D�̎��s���� */
		/* ����̒ǉ� */
		pcOpeBlk->AppendOpe(
			new CMoveCaretOpe(
				ptCaretPos_PHY_Old,				// �����̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}

	//	Jan. 30, 2001 genta
	//	�t�@�C���S�̂̍X�V�t���O�������Ă��Ȃ��Ɗe�s�̍X�V��Ԃ��\������Ȃ��̂�
	//	�t���O�X�V�������ĕ`����O�Ɉړ�����
}




// 2005.10.11 ryoji �O�̍s�ɂ��閖���̋󔒂��폜
void CEditView::RTrimPrevLine( void )
{
	int			nCharChars;

	CLogicPoint ptCaretPos_PHY = GetCaret().GetCaretLogicPos();

	if( GetCaret().GetCaretLogicPos().y > 0 ){
		int				nLineLen;
		const wchar_t*	pLine = CDocReader(m_pcEditDoc->m_cDocLineMgr).GetLineStrWithoutEOL( GetCaret().GetCaretLogicPos().GetY2() - CLogicInt(1), &nLineLen );
		if( NULL != pLine && nLineLen > 0 ){
			int i=0;
			int j=0;
			while( i < nLineLen ){
				nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
				if( !WCODE::IsBlank(pLine[i]) ){
					j = i + nCharChars;
				}
				i += nCharChars;
			}
			if( j < nLineLen ){
				CLayoutRange sRangeA;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( CLogicPoint(j, GetCaret().GetCaretLogicPos().y - 1), sRangeA.GetFromPointer() );
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( CLogicPoint(nLineLen, GetCaret().GetCaretLogicPos().y - 1), sRangeA.GetToPointer() );
				if( !( sRangeA.GetFrom().x >= sRangeA.GetTo().x && sRangeA.GetFrom().y == sRangeA.GetTo().y) ){
					ReplaceData_CEditView(
						sRangeA,
						NULL,		/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
						NULL,		/* �}������f�[�^ */
						CLogicInt(0),			/* �}������f�[�^�̒��� */
						true,
						m_bDoing_UndoRedo?NULL:m_pcOpeBlk
					);
					CLayoutPoint ptCP;
					m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptCaretPos_PHY, &ptCP );
					GetCaret().MoveCursor( ptCP, true );

					if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
						/* ����̒ǉ� */
						m_pcOpeBlk->AppendOpe(
							new CMoveCaretOpe(
								GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
								GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
							)
						);
					}
				}
			}
		}
	}
}





