//	$Id$
/*!	@file
	CEditView�N���X�̃R�}���h�����n�֐��Q

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "CEditView.h"
#include "CWaitCursor.h"
#include "charcode.h"
#include "CRunningTimer.h"




/* ���݈ʒu�Ƀf�[�^��}�� Ver0  */
void CEditView::InsertData_CEditView(
	int			nX,
	int			nY,
	const char*	pData,
	int			nDataLen,
	int*		pnNewLine,			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int*		pnNewPos,			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	COpe*		pcOpe,				/* �ҏW����v�f COpe */
	BOOL		bRedraw,
	BOOL		bUndo			/* Undo���삩�ǂ��� */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	CRunningTimer*  pCRunningTimer = new CRunningTimer( (const char*)"CEditView::InsertData_CEditView()" );
#endif
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			nIdxFrom;
	int			nModifyLayoutLinesOld;
	int			nInsLineNum;		/* �}���ɂ���đ��������C�A�E�g�s�̐� */
	PAINTSTRUCT ps;
	HDC			hdc;
	int			nLineAllColLen;
	CMemory		cMem;
	int			i;
	const CLayout*	pcLayout;

	*pnNewLine = 0;			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	*pnNewPos = 0;			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */

	/* �e�L�X�g���I������Ă��邩 */
	if( IsTextSelected() ){
		DeleteData( bRedraw );
		nX = m_nCaretPosX;
		nY = m_nCaretPosY;
	}

	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nY, &nLineLen, &pcLayout );

	nIdxFrom = 0;
//	cMem.SetData( "", lstrlen( "" ) );
	cMem.SetDataSz( "" );
	if( NULL != pLine ){
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
		nIdxFrom = LineColmnToIndex2(
			pLine,
			nLineLen/* - pcLayout->m_cEol.GetLen() + (pcLayout->m_cEol.GetLen()?1:0)*/,
			nX, &nLineAllColLen
		);
		/* �s�I�[���E�ɑ}�����悤�Ƃ��� */
		if( nLineAllColLen > 0 ){
			/* �I�[���O����}���ʒu�܂ŋ󔒂𖄂߂�ׂ̏��� */
			/* �s�I�[�����炩�̉��s�R�[�h��? */
			if( EOL_NONE != pcLayout->m_cEol ){
//			if( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ){
				nIdxFrom = nLineLen - 1;
//				nIdxFrom = nLineLen - pcLayout->m_cEol.GetLen();
				for( i = 0; i < nX - nLineAllColLen + 1; ++i ){
					cMem += ' ';
				}
				cMem.Append( pData, nDataLen );
			}else{
				nIdxFrom = nLineLen;
				for( i = 0; i < nX - nLineAllColLen; ++i ){
					cMem += ' ';
				}
				cMem.Append( pData, nDataLen );
			}
		}else{
			cMem.Append( pData, nDataLen );
		}
	}else{
		nLineAllColLen = nX;
		for( i = 0; i < nX - nIdxFrom; ++i ){
			cMem += ' ';
		}
		cMem.Append( pData, nDataLen );
	}
//	MYTRACE( "nY=%d nIdxFrom=%d nLineAllColLen=%d \n", nY, nIdxFrom, nLineAllColLen );


	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* �A���h�D�E���h�D�̎��s���� */
		if( NULL != pLine ){
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
				LineIndexToColmn( pLine, nLineLen, nIdxFrom ),
				nY,
				&pcOpe->m_nCaretPosX_PHY_Before,
				&pcOpe->m_nCaretPosY_PHY_Before
			);
		}else{
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
				0,
				nY,
				&pcOpe->m_nCaretPosX_PHY_Before,
				&pcOpe->m_nCaretPosY_PHY_Before
			);
		}
	}


	/* ������}�� */
	m_pcEditDoc->m_cLayoutMgr.InsertData_CLayoutMgr(
		nY,
		nIdxFrom,
		cMem.GetPtr( NULL ),
		cMem.GetLength(),
		&nModifyLayoutLinesOld,
		&nInsLineNum,
		pnNewLine,			/* �}�����ꂽ�����̎��̈ʒu�̍s */
		pnNewPos,			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* �V���O���N�H�[�e�[�V�����������\������ */
		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp,	/* �_�u���N�H�[�e�[�V�����������\������ */
		bUndo	/* Undo���삩�ǂ��� */
	);


	/* ���������Ċm�ۂ���ăA�h���X�������ɂȂ�̂ŁA�ēx�A�s�f�[�^�����߂� */
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nY, &nLineLen );

	/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
	pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( *pnNewLine, &nLineLen2 );
	if( pLine2 != NULL ){
		*pnNewPos = LineIndexToColmn( pLine2, nLineLen2, *pnNewPos );
	}
	if( *pnNewPos >= m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize ){
		*pnNewPos = 0;
		(*pnNewLine)++;
	}

//	MYTRACE( "nModifyLayoutLinesOld=%d nInsLineNum=%d *pnNewLine=%d *pnNewPos=%d\n", nModifyLayoutLinesOld, nInsLineNum, *pnNewLine, *pnNewPos );


	/* ��ԑJ�� */
	if( FALSE == m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		m_pcEditDoc->m_bIsModified = TRUE;	/* �ύX�t���O */
		if( bRedraw ){
			SetParentCaption();	/* �e�E�B���h�E�̃^�C�g�����X�V */
		}
	}

	/* �ĕ`�� */
	/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	if( DetectWidthOfLineNumberArea( bRedraw ) ){
		::DestroyCaret();
		m_nCaretWidth = 0;
		for( i = 0; i < 4; ++i ){
			if( m_nMyIndex != i ){
				m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
			}
		}
		/* �L�����b�g�̕\���E�X�V */
		ShowEditCaret();
	}else{

		if( bRedraw ){
			if( 0 < nInsLineNum ){
				/* �X�N���[���o�[�̏�Ԃ��X�V���� */
				AdjustScrollBars();
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nY - m_nViewTopLine);
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			}else{
				if( nModifyLayoutLinesOld < 1 ){
					nModifyLayoutLinesOld = 1;
				}
	//			ps.rcPaint.left = m_nViewAlignLeft;
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nY - m_nViewTopLine - 1);
				ps.rcPaint.bottom = ps.rcPaint.top + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * ( nModifyLayoutLinesOld + 1);
				if( m_nViewAlignTop + m_nViewCy < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				}
			}
			hdc = ::GetDC( m_hWnd );
			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
			OnSetFocus();
			::ReleaseDC( m_hWnd, hdc );
		}
	}

	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe->m_nOpe = OPE_INSERT;				/* ������ */
//		pcOpe->m_nCaretPosX_To = *pnNewPos;		/* ����O�̃L�����b�g�ʒu�w To */
//		pcOpe->m_nCaretPosY_To = *pnNewLine;	/* ����O�̃L�����b�g�ʒu�x To */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_To,
//			pcOpe->m_nCaretPosY_To,
//			&pcOpe->m_nCaretPosX_PHY_To,
//			&pcOpe->m_nCaretPosY_PHY_To
//		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			*pnNewPos,
			*pnNewLine,
			&pcOpe->m_nCaretPosX_PHY_To,
			&pcOpe->m_nCaretPosY_PHY_To
		);

//?	2000.1.24 ���[�h���b�v���̕s��̍���
//?
//?		if( NULL != pLine ){
//?			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//?				LineIndexToColmn( pLine, nLineLen, nIdxFrom ),
//?				nY,
//?				&pcOpe->m_nCaretPosX_PHY_Before,
//?				&pcOpe->m_nCaretPosY_PHY_Before
//?			);
//?		}else{
//?			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//?				0,
//?				nY,
//?				&pcOpe->m_nCaretPosX_PHY_Before,
//?				&pcOpe->m_nCaretPosY_PHY_Before
//?			);
//?		}




		pcOpe->m_nDataLen = cMem.GetLength();	/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		pcOpe->m_pcmemData = NULL;				/* ����Ɋ֘A����f�[�^ */
	}
#ifdef _DEBUG
	delete pCRunningTimer;
	pCRunningTimer = NULL;

	gm_ProfileOutput = 0;
#endif
	return;
}


/* �w��ʒu�̎w�蒷�f�[�^�폜 */
void CEditView::DeleteData2(
	int			nCaretX,
	int			nCaretY,
	int			nDelLen,
	CMemory*	pcMem,
	COpe*		pcOpe,		/* �ҏW����v�f COpe */
	BOOL		bRedraw,
	BOOL		bRedraw2,
	BOOL		bUndo			/* Undo���삩�ǂ��� */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	CRunningTimer* pCRunningTimer = new CRunningTimer( (const char*)"CEditView::DeleteData(1)" );
#endif
	const char*	pLine;
	int			nLineLen;
	int			nIdxFrom;
	int			nModifyLayoutLinesOld;
	int			nModifyLayoutLinesNew;
	int			nDeleteLayoutLines;
	PAINTSTRUCT ps;
	HDC			hdc;
	int			bLastLine;

	/* �Ō�̍s�ɃJ�[�\�������邩�ǂ��� */
	if( nCaretY == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
		bLastLine = 1;
	}else{
		bLastLine = 0;
	}

	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCaretY, &nLineLen );
	if( NULL == pLine ){
		goto end_of_func;
	}
	nIdxFrom = LineColmnToIndex( pLine, nLineLen, nCaretX );
	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe->m_nOpe = OPE_DELETE;				/* ������ */
//		pcOpe->m_nCaretPosX_To = LineIndexToColmn( pLine, nLineLen, nIdxFrom + nDelLen );/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_To = nCaretY;	/* ����O�̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_To,
//			pcOpe->m_nCaretPosY_To,
//			&pcOpe->m_nCaretPosX_PHY_To,
//			&pcOpe->m_nCaretPosY_PHY_To
//		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			LineIndexToColmn( pLine, nLineLen, nIdxFrom + nDelLen ),
			nCaretY,
			&pcOpe->m_nCaretPosX_PHY_To,
			&pcOpe->m_nCaretPosY_PHY_To
		);
	}


	/* �f�[�^�폜 */
	m_pcEditDoc->m_cLayoutMgr.DeleteData_CLayoutMgr(
		nCaretY, nIdxFrom, nDelLen,
		&nModifyLayoutLinesOld,
		&nModifyLayoutLinesNew,
		&nDeleteLayoutLines,
		*pcMem,
		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* �V���O���N�H�[�e�[�V�����������\������ */
		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp,	/* �_�u���N�H�[�e�[�V�����������\������ */
		bUndo	/* Undo���삩�ǂ��� */
	);

	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe->m_nDataLen = pcMem->GetLength();	/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		pcOpe->m_pcmemData = pcMem;				/* ����Ɋ֘A����f�[�^ */
	}

	if( bRedraw2 ){
		/* �ĕ`�� */
		if( 0 < nDeleteLayoutLines ){
			ps.rcPaint.left = 0;
			ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
			ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nCaretY - m_nViewTopLine - bLastLine );
			ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			hdc = ::GetDC( m_hWnd );
			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
			OnSetFocus();
			::ReleaseDC( m_hWnd, hdc );

			/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
			if( DetectWidthOfLineNumberArea( TRUE ) ){
				int i;
				::DestroyCaret();
				m_nCaretWidth = 0;
				for( i = 0; i < 4; ++i ){
					if( m_nMyIndex != i ){
						m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
					}
				}
				/* �L�����b�g�̕\���E�X�V */
				ShowEditCaret();
			}

			/* �X�N���[���o�[�̏�Ԃ��X�V���� */
			AdjustScrollBars();
		}else{
			if( bRedraw ){
//				ps.rcPaint.left = m_nViewAlignLeft;
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nCaretY - m_nViewTopLine - bLastLine);
				ps.rcPaint.bottom = ps.rcPaint.top + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nModifyLayoutLinesOld + bLastLine );
				hdc = ::GetDC( m_hWnd );
				OnKillFocus();
				OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
				OnSetFocus();
				::ReleaseDC( m_hWnd, hdc );
			}
		}
	}
	/* �I���G���A�̐擪�փJ�[�\�����ړ� */
	MoveCursor( nCaretX, nCaretY, FALSE );
	m_nCaretPosX_Prev = m_nCaretPosX;


end_of_func:;
#ifdef _DEBUG
	delete pCRunningTimer;
	pCRunningTimer = NULL;

	gm_ProfileOutput = 0;
#endif
	return;

}





//�J�[�\���ʒu�܂��͑I���G���A���폜
void CEditView::DeleteData(
				BOOL	bRedraw,
				BOOL	bUndo	/* Undo���삩�ǂ��� */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	CRunningTimer*  pCRunningTimer = new CRunningTimer( (const char*)"CEditView::DeleteData(2)" );
#endif
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			nLineNum;
	int			nCurIdx;
	int			nNxtIdx;
	int			nNxtPos;
//	int			nModifyLayoutLinesOld;
//	int			nModifyLayoutLinesNew;
//	int			nDeleteLayoutLines;
	PAINTSTRUCT ps;
	HDC			hdc;
	int			nIdxFrom;
	int			nIdxTo;
	int			nDelPos;
	int			nDelLen;
	int			nDelPosNext;
	int			nDelLenNext;
	CMemory		cmemBuf;
	RECT		rcSel;
	int			bLastLine;
	CMemory*	pcMemDeleted;
	COpe*		pcOpe = NULL;
//	int			nPosX;
//	int			nPosY;
//	int			nPosXNext;
//	int			nPosYNext;
	int			nCaretPosXOld;
	int			nCaretPosYOld;
	BOOL		bBoxSelected;
	int			i;
	const CLayout*	pcLayout;
	int			nSelectColmFrom_Old;
	int			nSelectLineFrom_Old;


	nCaretPosXOld = m_nCaretPosX;
	nCaretPosYOld = m_nCaretPosY;
	bBoxSelected = FALSE;

	/* �e�L�X�g���I������Ă��邩 */
	if( IsTextSelected() ){
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
//			pcOpe->m_nCaretPosX_Before = m_nCaretPosX;			/* ����O�̃L�����b�g�ʒu�w */
//			pcOpe->m_nCaretPosY_Before = m_nCaretPosY;			/* ����O�̃L�����b�g�ʒu�x */
//			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//				pcOpe->m_nCaretPosX_Before,
//				pcOpe->m_nCaretPosY_Before,
//				&pcOpe->m_nCaretPosX_PHY_Before,
//				&pcOpe->m_nCaretPosY_PHY_Before
//			);
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */

//			pcOpe->m_nCaretPosX_After = m_nCaretPosX;	/* �����̃L�����b�g�ʒu�w */
//			pcOpe->m_nCaretPosY_After = m_nCaretPosY;	/* �����̃L�����b�g�ʒu�x */
//			pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
//			pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}

		/* ��`�͈͑I�𒆂� */
		if( m_bBeginBoxSelect ){
			bBoxSelected = TRUE;
			nSelectColmFrom_Old = m_nSelectColmFrom;
			nSelectLineFrom_Old = m_nSelectLineFrom;

			/* �I��͈͂̃f�[�^���擾 */
			/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
			/* �Q�_��Ίp�Ƃ����`�����߂� */
			TwoPointToRect(
				&rcSel,
				m_nSelectLineFrom,		/* �͈͑I���J�n�s */
				m_nSelectColmFrom,		/* �͈͑I���J�n�� */
				m_nSelectLineTo,		/* �͈͑I���I���s */
				m_nSelectColmTo			/* �͈͑I���I���� */
			);
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( bRedraw );

			nIdxFrom = 0;
			nIdxTo = 0;
			for( nLineNum = rcSel.bottom; nLineNum >= rcSel.top - 1; nLineNum-- ){
				nDelPosNext = nIdxFrom;
				nDelLenNext	= nIdxTo - nIdxFrom;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
				if( NULL != pLine ){
					/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
					nIdxFrom = LineColmnToIndex( pLine, nLineLen, rcSel.left  );
					nIdxTo	 = LineColmnToIndex( pLine, nLineLen, rcSel.right );

					for( i = nIdxFrom; i <= nIdxTo; ++i ){
						if( pLine[i] == CR || pLine[i] == LF ){
							nIdxTo = i;
							break;
						}
					}
				}else{
					nIdxFrom = 0;
					nIdxTo	 = 0;
				}
				nDelPos = nDelPosNext;
				nDelLen	= nDelLenNext;
				if( nLineNum < rcSel.bottom && 0 < nDelLen ){
					if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
						pcOpe = new COpe;
						pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + 1, &nLineLen2 );
//						pcOpe->m_nCaretPosX_Before = LineIndexToColmn( pLine2, nLineLen2, nDelPos );	/* ����O�̃L�����b�g�ʒu�w */
//						pcOpe->m_nCaretPosY_Before = nLineNum + 1;	/* ����O�̃L�����b�g�ʒu�x */
//						m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//							pcOpe->m_nCaretPosX_Before,
//							pcOpe->m_nCaretPosY_Before,
//							&pcOpe->m_nCaretPosX_PHY_Before,
//							&pcOpe->m_nCaretPosY_PHY_Before
//						);
						m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
							LineIndexToColmn( pLine2, nLineLen2, nDelPos ),
							nLineNum + 1,
							&pcOpe->m_nCaretPosX_PHY_Before,
							&pcOpe->m_nCaretPosY_PHY_Before
						);

					}else{
						pcOpe = NULL;
					}
					pcMemDeleted = new CMemory;
					/* �w��ʒu�̎w�蒷�f�[�^�폜 */
					DeleteData2(
						rcSel.left,
						nLineNum + 1,
						nDelLen,
						pcMemDeleted,
						pcOpe,				/* �ҏW����v�f COpe */
						bRedraw,
						bRedraw,
						bUndo	/* Undo���삩�ǂ��� */
					);

					if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
//						pcOpe->m_nCaretPosX_After = rcSel.left;	/* �����̃L�����b�g�ʒu�w */
//						pcOpe->m_nCaretPosY_After = nLineNum + 1;	/* �����̃L�����b�g�ʒu�x */
//						m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//							pcOpe->m_nCaretPosX_After,
//							pcOpe->m_nCaretPosY_After,
//							&pcOpe->m_nCaretPosX_PHY_After,
//							&pcOpe->m_nCaretPosY_PHY_After
//						);
						m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
							rcSel.left,
							nLineNum + 1,
							&pcOpe->m_nCaretPosX_PHY_After,
							&pcOpe->m_nCaretPosY_PHY_After
						);
						/* ����̒ǉ� */
						m_pcOpeBlk->AppendOpe( pcOpe );
					}else{
						delete pcMemDeleted;
						pcMemDeleted = NULL;
					}
				}
			}

			/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
			if( DetectWidthOfLineNumberArea( TRUE ) ){
				for( i = 0; i < 4; ++i ){
					::DestroyCaret();
					m_nCaretWidth = 0;
					if( m_nMyIndex != i ){
						m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
					}
				}
				/* �L�����b�g�̕\���E�X�V */
				ShowEditCaret();
			}
			if( bRedraw ){
				/* �X�N���[���o�[�̏�Ԃ��X�V���� */
				AdjustScrollBars();
			}
			if( bRedraw  ){
			/* �ĕ`�� */
				hdc = ::GetDC( m_hWnd );
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
	//			ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (m_nCaretPosY - m_nViewTopLine);
				ps.rcPaint.top = m_nViewAlignTop;
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				OnKillFocus();
				OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
				OnSetFocus();
				::ReleaseDC( m_hWnd, hdc );
			}
			/* �I���G���A�̐擪�փJ�[�\�����ړ� */
			::UpdateWindow( m_hWnd );
			MoveCursor( nSelectColmFrom_Old, nSelectLineFrom_Old, bRedraw );
			m_nCaretPosX_Prev = m_nCaretPosX;
			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				pcOpe = new COpe;
				pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
//				pcOpe->m_nCaretPosX_Before = nCaretPosXOld;			/* ����O�̃L�����b�g�ʒu�w */
//				pcOpe->m_nCaretPosY_Before = nCaretPosYOld;			/* ����O�̃L�����b�g�ʒu�x */
//				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//					pcOpe->m_nCaretPosX_Before,
//					pcOpe->m_nCaretPosY_Before,
//					&pcOpe->m_nCaretPosX_PHY_Before,
//					&pcOpe->m_nCaretPosY_PHY_Before
//				);
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
					nCaretPosXOld,
					nCaretPosYOld,
					&pcOpe->m_nCaretPosX_PHY_Before,
					&pcOpe->m_nCaretPosY_PHY_Before
				);

//				pcOpe->m_nCaretPosX_After = m_nCaretPosX;	/* �����̃L�����b�g�ʒu�w */
//				pcOpe->m_nCaretPosY_After = m_nCaretPosY;	/* �����̃L�����b�g�ʒu�x */
//				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//					pcOpe->m_nCaretPosX_After,
//					pcOpe->m_nCaretPosY_After,
//					&pcOpe->m_nCaretPosX_PHY_After,
//					&pcOpe->m_nCaretPosY_PHY_After
//				);
				pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
				pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
				/* ����̒ǉ� */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}
		}else{
			/* �f�[�^�u�� �폜&�}���ɂ��g���� */
			ReplaceData_CEditView(
				m_nSelectLineFrom,		/* �͈͑I���J�n�s */
				m_nSelectColmFrom,		/* �͈͑I���J�n�� */
				m_nSelectLineTo,		/* �͈͑I���I���s */
				m_nSelectColmTo,		/* �͈͑I���I���� */
				NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
				"",						/* �}������f�[�^ */
				0,						/* �}������f�[�^�̒��� */
				bRedraw
			);
		}
	}else{
		/* ���ݍs�̃f�[�^���擾 */
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL == pLine ){
			goto end_of_func;
			return;
		}
		/* �Ō�̍s�ɃJ�[�\�������邩�ǂ��� */
		if( m_nCaretPosY == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
			bLastLine = 1;
		}else{
			bLastLine = 0;
		}

		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
		nCurIdx = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );
//		MYTRACE("nLineLen=%d nCurIdx=%d \n", nLineLen, nCurIdx);
		if( nCurIdx == nLineLen && bLastLine ){	/* �S�e�L�X�g�̍Ō� */
			goto end_of_func;
			return;
		}
		/* �w�肳�ꂽ���̕����̃o�C�g���𒲂ׂ� */
		if( pLine[nCurIdx] == '\r' || pLine[nCurIdx] == '\n' ){
			/* ���s */
			nNxtIdx = nCurIdx + pcLayout->m_cEol.GetLen();
			nNxtPos = m_nCaretPosX + pcLayout->m_cEol.GetLen();
		}else{
			nNxtIdx = CMemory::MemCharNext( pLine, nLineLen, &pLine[nCurIdx] ) - pLine;
			/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
			nNxtPos = LineIndexToColmn( pLine, nLineLen, nNxtIdx );
		}


		/* �f�[�^�u�� �폜&�}���ɂ��g���� */
		ReplaceData_CEditView(
			m_nCaretPosY,		/* �͈͑I���J�n�s */
			m_nCaretPosX,		/* �͈͑I���J�n�� */
			m_nCaretPosY,		/* �͈͑I���I���s */
			nNxtPos,			/* �͈͑I���I���� */
			NULL,				/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
			"",					/* �}������f�[�^ */
			0,					/* �}������f�[�^�̒��� */
			bRedraw
		);
	}

	m_pcEditDoc->m_bIsModified = TRUE;	/* �ύX�t���O */
	if( bRedraw ){
		SetParentCaption();	/* �e�E�B���h�E�̃^�C�g�����X�V */
	}

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() > 0 ){
		if( m_nCaretPosY > m_pcEditDoc->m_cLayoutMgr.GetLineCount()	- 1	){
			/* ���ݍs�̃f�[�^���擾 */
			const CLayout*	pcLayout;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				goto end_of_func;
				return;
			}
			/* ���s�ŏI����Ă��邩 */
			if( ( EOL_NONE != pcLayout->m_cEol.GetLen() ) ){
				goto end_of_func;
				return;
			}
			/*�t�@�C���̍Ō�Ɉړ� */
			Command_GOFILEEND( FALSE );
		}
	}
end_of_func:;
//#ifdef _DEBUG
//	delete pCRunningTimer;
//	pCRunningTimer = NULL;
//
//	gm_ProfileOutput = 0;
//#endif

	return;
}











/* Undo ���ɖ߂� */
void CEditView::Command_UNDO( void )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		::MessageBeep( MB_ICONHAND );
		return;
	}

	if( !m_pcEditDoc->IsEnableUndo() ){	/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
		return;
	}

#ifdef _DEBUG
//	MYTRACE( "\n\n======================================\n" );
	CRunningTimer cRunningTimer( (const char*)"CEditView::Command_UNDO()" );
#endif
	COpe*		pcOpe = NULL;
	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
	CMemory*	pcMem;
	int			bIsModified;
//	int			nNewLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int			nNewPos;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	HDC			hdc;
	PAINTSTRUCT	ps;
	CWaitCursor cWaitCursor( m_hWnd );
	BOOL		bUndo;	/* Undo���삩�ǂ��� */
	bUndo = TRUE;	/* Undo���삩�ǂ��� */

	int			nCaretPosX_Before;
	int			nCaretPosY_Before;
//	int			nCaretPosX_To;
//	int			nCaretPosY_To;
	int			nCaretPosX_After;
	int			nCaretPosY_After;

	/* �e�탂�[�h�̎����� */
	Command_CANCEL_MODE();

	m_bDoing_UndoRedo = TRUE;	/* �A���h�D�E���h�D�̎��s���� */

	/* ���݂�Undo�Ώۂ̑���u���b�N��Ԃ� */
	if( NULL != ( pcOpeBlk = m_pcEditDoc->m_cOpeBuf.DoUndo( &bIsModified ) ) ){
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = nOpeBlkNum - 1; i >= 0; i-- ){
			pcOpe = pcOpeBlk->GetOpe( i );
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				pcOpe->m_nCaretPosX_PHY_After,
				pcOpe->m_nCaretPosY_PHY_After,
				&nCaretPosX_After,
				&nCaretPosY_After
			);
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				pcOpe->m_nCaretPosX_PHY_Before,
				pcOpe->m_nCaretPosY_PHY_Before,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);


			if( i == nOpeBlkNum - 1	){
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, TRUE );
			}else{
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, FALSE );
			}
			switch( pcOpe->m_nOpe ){
			case OPE_INSERT:
				pcMem = new CMemory;

				/* �I��͈͂̕ύX */
//				m_nSelectLineBgn = pcOpe->m_nCaretPosY_Before;	/* �͈͑I���J�n�s(���_) */
//				m_nSelectColmBgn = pcOpe->m_nCaretPosX_Before;	/* �͈͑I���J�n��(���_) */

				m_nSelectLineBgnFrom = nCaretPosY_Before;	/* �͈͑I���J�n�s(���_) */
				m_nSelectColmBgnFrom = nCaretPosX_Before;	/* �͈͑I���J�n��(���_) */
				m_nSelectLineBgnTo = m_nSelectLineBgnFrom;		/* �͈͑I���J�n�s(���_) */
				m_nSelectColmBgnTo = m_nSelectColmBgnFrom;		/* �͈͑I���J�n��(���_) */
				m_nSelectLineFrom = nCaretPosY_Before;
				m_nSelectColmFrom = nCaretPosX_Before;
				m_nSelectLineTo = nCaretPosY_After;
				m_nSelectColmTo = nCaretPosX_After;

//				GetSelectedData( *pcMem, FALSE, NULL, FALSE );
//				DeleteData(
//					FALSE,
//					bUndo	/* Undo���삩�ǂ��� */
//				);


				/* �f�[�^�u�� �폜&�}���ɂ��g���� */
				ReplaceData_CEditView(
					m_nSelectLineFrom,		/* �͈͑I���J�n�s */
					m_nSelectColmFrom,		/* �͈͑I���J�n�� */
					m_nSelectLineTo,		/* �͈͑I���I���s */
					m_nSelectColmTo,		/* �͈͑I���I���� */
					pcMem,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
					"",						/* �}������f�[�^ */
					0,						/* �}������f�[�^�̒��� */
					FALSE					/*�ĕ`�悷�邩�ۂ�*/
				);




				/* �I��͈͂̕ύX */
//				m_nSelectLineBgn = -1;	/* �͈͑I���J�n�s(���_) */
//				m_nSelectColmBgn = -1;	/* �͈͑I���J�n��(���_) */
				m_nSelectLineBgnFrom = -1;	/* �͈͑I���J�n�s(���_) */
				m_nSelectColmBgnFrom = -1;	/* �͈͑I���J�n��(���_) */
				m_nSelectLineBgnTo = -1;	/* �͈͑I���J�n�s(���_) */
				m_nSelectColmBgnTo = -1;	/* �͈͑I���J�n��(���_) */
				m_nSelectLineFrom = -1;
				m_nSelectColmFrom = -1;
				m_nSelectLineTo = -1;
				m_nSelectColmTo = -1;

				pcOpe->m_pcmemData = pcMem;
//				if( 0 == pcMem->GetLength() ){
//					MYTRACE( "?? ERROR\n" );
//				}

				break;
			case OPE_DELETE:
				pcMem = new CMemory;
				if( 0 < pcOpe->m_pcmemData->GetLength() ){
					/* �f�[�^�u�� �폜&�}���ɂ��g���� */
					ReplaceData_CEditView(
						nCaretPosY_Before,					/* �͈͑I���J�n�s */
						nCaretPosX_Before,					/* �͈͑I���J�n�� */
						nCaretPosY_Before,					/* �͈͑I���I���s */
						nCaretPosX_Before,					/* �͈͑I���I���� */
						pcMem,								/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
						pcOpe->m_pcmemData->GetPtr( NULL ),	/* �}������f�[�^ */
						pcOpe->m_nDataLen,					/* �}������f�[�^�̒��� */
						FALSE								/*�ĕ`�悷�邩�ۂ�*/
					);

//					InsertData_CEditView(
//						nCaretPosX_Before,
//						nCaretPosY_Before,
//						pcOpe->m_pcmemData->GetPtr( NULL ),
//						pcOpe->m_nDataLen,
//						&nNewLine,
//						&nNewPos,
//						NULL,
//						FALSE,
//						bUndo	/* Undo���삩�ǂ��� */
//					);
				}
				delete pcOpe->m_pcmemData;
				pcOpe->m_pcmemData = NULL;
				break;
			case OPE_MOVECARET:
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, TRUE );
				break;
			}

			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				pcOpe->m_nCaretPosX_PHY_Before,
				pcOpe->m_nCaretPosY_PHY_Before,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);
			if( i == 0 ){
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, TRUE );
			}else{
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, FALSE );
			}
		}
		m_pcEditDoc->m_bIsModified = bIsModified;	/* Undo��̕ύX�t���O */
		SetParentCaption();	/* �e�E�B���h�E�̃^�C�g�����X�V */

		m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

		/* �ĕ`�� */
		hdc = ::GetDC( m_hWnd );
		ps.rcPaint.left = 0;
		ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
		ps.rcPaint.top = m_nViewAlignTop;
		ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
		OnKillFocus();
		OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
		OnSetFocus();
		::ReleaseDC( m_hWnd, hdc );
		/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
		if( DetectWidthOfLineNumberArea( TRUE ) ){
			::DestroyCaret();
			m_nCaretWidth = 0;
			for( i = 0; i < 4; ++i ){
				if( m_nMyIndex != i ){
					m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
				}
			}
			/* �L�����b�g�̕\���E�X�V */
			ShowEditCaret();
		}

	//	2001/06/21 Start by asa-o: ���̃y�C���̕\����Ԃ��X�V
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^1].Redraw();
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^2].Redraw();
		m_pcEditDoc->m_cEditViewArr[(m_nMyIndex^1)^2].Redraw();
	//	2001/06/21 End

	}
	m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */
	return;
}





/* Redo ��蒼�� */
void CEditView::Command_REDO( void )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		::MessageBeep( MB_ICONHAND );
		return;
	}


	if( !m_pcEditDoc->IsEnableRedo() ){	/* Redo(��蒼��)�\�ȏ�Ԃ��H */
		return;
	}
#ifdef _DEBUG
//	MYTRACE( "\n\n======================================\n" );
	CRunningTimer cRunningTimer( (const char*)"CEditView::Command_REDO()" );
#endif
	COpe*		pcOpe = NULL;
	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
	CMemory*	pcMem;
//	int			nNewLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int			nNewPos;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	int			bIsModified;
	HDC			hdc;
	PAINTSTRUCT	ps;
	CWaitCursor cWaitCursor( m_hWnd );

	int			nCaretPosX_Before;
	int			nCaretPosY_Before;
	int			nCaretPosX_To;
	int			nCaretPosY_To;
	int			nCaretPosX_After;
	int			nCaretPosY_After;


	/* �e�탂�[�h�̎����� */
	Command_CANCEL_MODE();

	m_bDoing_UndoRedo = TRUE;	/* �A���h�D�E���h�D�̎��s���� */

	/* ���݂�Redo�Ώۂ̑���u���b�N��Ԃ� */
	if( NULL != ( pcOpeBlk = m_pcEditDoc->m_cOpeBuf.DoRedo( &bIsModified ) ) ){
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = 0; i < nOpeBlkNum; ++i ){
			pcOpe = pcOpeBlk->GetOpe( i );
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
//			int		nPosX2;
//			int		nPosY2;
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				pcOpe->m_nCaretPosX_PHY_Before,
				pcOpe->m_nCaretPosY_PHY_Before,
				&nCaretPosX_Before,
				&nCaretPosY_Before
			);

			if( i == 0 ){
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, TRUE );
			}else{
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_Before, nCaretPosY_Before, FALSE );
			}
			switch( pcOpe->m_nOpe ){
			case OPE_INSERT:
				pcMem = new CMemory;
				if( 0 < pcOpe->m_pcmemData->GetLength() ){
					/* �f�[�^�u�� �폜&�}���ɂ��g���� */
					ReplaceData_CEditView(
						nCaretPosY_Before,					/* �͈͑I���J�n�s */
						nCaretPosX_Before,					/* �͈͑I���J�n�� */
						nCaretPosY_Before,					/* �͈͑I���I���s */
						nCaretPosX_Before,					/* �͈͑I���I���� */
						NULL,								/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
						pcOpe->m_pcmemData->GetPtr( NULL ),	/* �}������f�[�^ */
						pcOpe->m_pcmemData->GetLength(),	/* �}������f�[�^�̒��� */
						FALSE								/*�ĕ`�悷�邩�ۂ�*/
					);

//					InsertData_CEditView(
//						nCaretPosX_Before,
//						nCaretPosY_Before,
//						pcOpe->m_pcmemData->GetPtr( NULL ),
//						pcOpe->m_pcmemData->GetLength(),//*pcOpe->m_nDataLen,
//						&nNewLine,
//						&nNewPos,
//						NULL,
//						FALSE
//					);
				}
				delete pcOpe->m_pcmemData;
				pcOpe->m_pcmemData = NULL;
				break;
			case OPE_DELETE:
				pcMem = new CMemory;

				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
					pcOpe->m_nCaretPosX_PHY_To,
					pcOpe->m_nCaretPosY_PHY_To,
					&nCaretPosX_To,
					&nCaretPosY_To
				);

				/* �f�[�^�u�� �폜&�}���ɂ��g���� */
				ReplaceData_CEditView(
					nCaretPosY_Before,	/* �͈͑I���J�n�s */
					nCaretPosX_Before,	/* �͈͑I���J�n�� */
					nCaretPosY_To,		/* �͈͑I���I���s */
					nCaretPosX_To,		/* �͈͑I���I���� */
					pcMem,				/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
					"",					/* �}������f�[�^ */
					0,					/* �}������f�[�^�̒��� */
					FALSE
				);

// 199.12.20
//				DeleteData2(
//					pcOpe->m_nCaretPosX_Before,
//					pcOpe->m_nCaretPosY_Before,
//					pcOpe->m_nDataLen,
//					pcMem,
//					NULL,
//					FALSE,
//					FALSE
//				);
				pcOpe->m_pcmemData = pcMem;
				break;
			case OPE_MOVECARET:
				break;
			}
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				pcOpe->m_nCaretPosX_PHY_After,
				pcOpe->m_nCaretPosY_PHY_After,
				&nCaretPosX_After,
				&nCaretPosY_After
			);

			if( i == nOpeBlkNum - 1	){
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, TRUE );
			}else{
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, FALSE );
			}
		}
		m_pcEditDoc->m_bIsModified = bIsModified;	/* Redo��̕ύX�t���O */
		SetParentCaption();	/* �e�E�B���h�E�̃^�C�g�����X�V */

		m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

		/* �ĕ`�� */
		hdc = ::GetDC( m_hWnd );
		ps.rcPaint.left = 0;
		ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
		ps.rcPaint.top = m_nViewAlignTop;
		ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
		OnKillFocus();
		OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
		OnSetFocus();
		::ReleaseDC( m_hWnd, hdc );

		/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
		if( DetectWidthOfLineNumberArea( TRUE ) ){
			::DestroyCaret();
			m_nCaretWidth = 0;
			for( i = 0; i < 4; ++i ){
				if( m_nMyIndex != i ){
					m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
				}
			}
			/* �L�����b�g�̕\���E�X�V */
			ShowEditCaret();
		}

	//	2001/06/21 Start by asa-o: ���̃y�C���̕\����Ԃ��X�V
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^1].Redraw();
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^2].Redraw();
		m_pcEditDoc->m_cEditViewArr[(m_nMyIndex^1)^2].Redraw();
	//	2001/06/21 End

	}
	m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

	return;
}












/* �f�[�^�u�� �폜&�}���ɂ��g���� */
void CEditView::ReplaceData_CEditView(
	int			nDelLineFrom,			/* �폜�͈͍s  From ���C�A�E�g�s�ԍ� */
	int			nDelColmFrom,			/* �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu */
	int			nDelLineTo,				/* �폜�͈͍s  To   ���C�A�E�g�s�ԍ� */
	int			nDelColmTo,				/* �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu */
	CMemory*	pcmemCopyOfDeleted,		/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
	const char*	pInsData,				/* �}������f�[�^ */
	int			nInsDataLen,			/* �}������f�[�^�̒��� */
	BOOL		bRedraw
//	BOOL		bUndo					/* Undo���삩�ǂ��� */
)
{
//#ifdef _DEBUG
//	gm_ProfileOutput = 1;
//	CRunningTimer*  pCRunningTimer = new CRunningTimer( (const char*)"CEditView::ReplaceData_CEditView()" );
//#endif
	{
		//	Jun 23, 2000 genta
		//	�ϐ��������������Y��Ă����̂��C��

		//	Jun. 1, 2000 genta
		//	DeleteData����ړ�����

		//	May. 29, 2000 genta
		//	From Here
		//	�s�̌�낪�I������Ă����Ƃ��̕s���������邽�߁C
		//	�I��̈悩��s���ȍ~�̕�������菜���D

		//	�擪
		int len, pos;
		const char *line;
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nDelLineFrom, &len );
		//	Jun. 1, 2000 genta
		//	������NULL�`�F�b�N���܂��傤
		if( line != NULL ){
			pos = LineColmnToIndex( line, len, nDelColmFrom );
			//	Jun. 1, 2000 genta
			//	����s�̍s���ȍ~�݂̂��I������Ă���ꍇ���l������

			//	Aug. 22, 2000 genta
			//	�J�n�ʒu��EOF�̌��̂Ƃ��͎��s�ɑ��鏈�����s��Ȃ�
			//	���������Ă��܂��Ƒ��݂��Ȃ��s��Point���ė�����D
			if( nDelLineFrom < m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 && pos >= len){
				if( nDelLineFrom == nDelLineTo  ){
					//	m_nSelectLineFrom <= m_nSelectLineTo �̓`�F�b�N���Ȃ�
					++nDelLineFrom;
					nDelLineTo = nDelLineFrom;
					nDelColmTo = nDelColmFrom = 0;
				}
				else {
					nDelLineFrom++;
					nDelColmFrom = 0;
				}
			}
		}

		//	����
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nDelLineTo, &len );
		if( line != NULL ){
			pos = LineIndexToColmn( line, len, len );

			if( nDelColmTo > pos ){
				nDelColmTo = pos;
			}
		}
		//	To Here
	}

	COpe* pcOpe = NULL;		/* �ҏW����v�f COpe */
	CMemory* pcMemDeleted;
	int	nCaretPosXOld;
	int	nCaretPosYOld;
	int	nCaretPosX_PHY_Old;
	int	nCaretPosY_PHY_Old;

	nCaretPosXOld = m_nCaretPosX;
	nCaretPosYOld = m_nCaretPosY;
	nCaretPosX_PHY_Old = m_nCaretPosX_PHY;
	nCaretPosY_PHY_Old = m_nCaretPosY_PHY;
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;						/* ������ */
//		pcOpe->m_nCaretPosX_Before = m_nCaretPosX;			/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_Before = m_nCaretPosY;			/* ����O�̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */


//		pcOpe->m_nCaretPosX_After = m_nCaretPosX;			/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_After = m_nCaretPosY;			/* �����̃L�����b�g�ʒu�x */
//		pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_DELETE;				/* ������ */

//		pcOpe->m_nCaretPosX_Before = nDelColmFrom/*m_nCaretPosX*/;	/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_Before = nDelLineFrom/*m_nCaretPosY*/;	/* ����O�̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_Before,
//			pcOpe->m_nCaretPosY_Before,
//			&pcOpe->m_nCaretPosX_PHY_Before,
//			&pcOpe->m_nCaretPosY_PHY_Before
//		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			nDelColmFrom/*m_nCaretPosX*/,
			nDelLineFrom/*m_nCaretPosY*/,
			&pcOpe->m_nCaretPosX_PHY_Before,
			&pcOpe->m_nCaretPosY_PHY_Before
		);

//		pcOpe->m_nCaretPosX_To = nDelColmTo;	/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_To = nDelLineTo;	/* ����O�̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_To,
//			pcOpe->m_nCaretPosY_To,
//			&pcOpe->m_nCaretPosX_PHY_To,
//			&pcOpe->m_nCaretPosY_PHY_To
//		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			nDelColmTo,
			nDelLineTo,
			&pcOpe->m_nCaretPosX_PHY_To,
			&pcOpe->m_nCaretPosY_PHY_To
		);



//		pcOpe->m_nCaretPosX_After = pcOpe->m_nCaretPosX_Before;	/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_After = pcOpe->m_nCaretPosY_Before;	/* �����̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_After,
//			pcOpe->m_nCaretPosY_After,
//			&pcOpe->m_nCaretPosX_PHY_After,
//			&pcOpe->m_nCaretPosY_PHY_After
//		);
	}else{
		pcOpe = NULL;
	}
	pcMemDeleted = new CMemory;
	/*
	|| �o�b�t�@�T�C�Y�̒���
	*/
	if( 2000 < nDelLineTo - nDelLineFrom ){
		pcMemDeleted->AllocBuffer( 1024000 );
	}


	/* ������u�� */
//	int		nAddLineNum;	/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
//	int		nModLineFrom;	/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
//	int		nModLineTo;		/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
//	int		nNewLine;		/* �}�����ꂽ�����̎��̈ʒu�̍s(���C�A�E�g�s) */
//	int		nNewPos;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu(���C�A�E�g���ʒu) */
	LayoutReplaceArg	LRArg;
	LRArg.nDelLineFrom = nDelLineFrom;	/* �폜�͈͍s  From ���C�A�E�g�s�ԍ� */
	LRArg.nDelColmFrom = nDelColmFrom;	/* �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu */
	LRArg.nDelLineTo = nDelLineTo;		/* �폜�͈͍s  To   ���C�A�E�g�s�ԍ� */
	LRArg.nDelColmTo = nDelColmTo;		/* �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu */
	LRArg.pcmemDeleted = pcMemDeleted;	/* �폜���ꂽ�f�[�^ */
	LRArg.pInsData = pInsData;			/* �}������f�[�^ */
	LRArg.nInsDataLen = nInsDataLen;	/* �}������f�[�^�̒��� */
//	LRArg.nAddLineNum = 0;			/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
//	LRArg.nModLineFrom = 0;			/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
//	LRArg.nModLineTo = 0;			/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
//	LRArg.nNewLine = 0;				/* �}�����ꂽ�����̎��̈ʒu�̍s(���C�A�E�g�s) */
//	LRArg.nNewPos = 0;				/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu(���C�A�E�g���ʒu) */
	LRArg.bDispSSTRING = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp;			/* �V���O���N�H�[�e�[�V�����������\������ */
	LRArg.bDispWSTRING = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp;			/* �_�u���N�H�[�e�[�V�����������\������ */
	LRArg.bUndo = m_bDoing_UndoRedo;					/* Undo���삩�ǂ��� */
	m_pcEditDoc->m_cLayoutMgr.ReplaceData_CLayoutMgr(
		&LRArg
#if 0
		nDelLineFrom,			/* �폜�͈͍s  From ���C�A�E�g�s�ԍ� */
		nDelColmFrom,			/* �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu */
		nDelLineTo,				/* �폜�͈͍s  To   ���C�A�E�g�s�ԍ� */
		nDelColmTo,				/* �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu */
		pcMemDeleted,			/* �폜���ꂽ�f�[�^ */
		pInsData,				/* �}������f�[�^ */
		nInsDataLen,			/* �}������f�[�^�̒��� */
		&nAddLineNum,			/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
		&nModLineFrom,			/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
		&nModLineTo,			/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
		&nNewLine,				/* �}�����ꂽ�����̎��̈ʒu�̍s(���C�A�E�g�s) */
		&nNewPos,				/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu(���C�A�E�g���ʒu) */

		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* �V���O���N�H�[�e�[�V�����������\������ */
		m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp,	/* �_�u���N�H�[�e�[�V�����������\������ */
		m_bDoing_UndoRedo	/* Undo���삩�ǂ��� */
#endif
	);

	//	Jan. 30, 2001 genta
	//	�ĕ`��̎��_�Ńt�@�C���X�V�t���O���K�؂ɂȂ��Ă��Ȃ��Ƃ����Ȃ��̂�
	//	�֐��̖������炱���ֈړ�
	/* ��ԑJ�� */
	if( FALSE == m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		m_pcEditDoc->m_bIsModified = TRUE;	/* �ύX�t���O */
		if( bRedraw ){
			SetParentCaption();	/* �e�E�B���h�E�̃^�C�g�����X�V */
		}
	}

	/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	DisableSelectArea( bRedraw );

	/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	if( DetectWidthOfLineNumberArea( bRedraw ) ){
		::DestroyCaret();
		m_nCaretWidth = 0;
		int i;
		for( i = 0; i < 4; ++i ){
			if( m_nMyIndex != i ){
				m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
			}
		}
		/* �L�����b�g�̕\���E�X�V */
		ShowEditCaret();
	}else{

		if( bRedraw ){
		/* �ĕ`�� */
			HDC	hdc;
			PAINTSTRUCT ps;
			hdc = ::GetDC( m_hWnd );
			/* �ĕ`��q���g ���C�A�E�g�s�̑��� */
			//	Jan. 30, 2001 genta	�\��t���ōs��������ꍇ�̍l���������Ă���
			if( 0 != LRArg.nAddLineNum ){
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (m_nCaretPosY - m_nViewTopLine);
				ps.rcPaint.top = 0/*m_nViewAlignTop*/;
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			}else{
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;

				/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
				ps.rcPaint.top = m_nViewAlignTop + (LRArg.nModLineFrom - m_nViewTopLine)* (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				if( m_pcEditDoc->GetDocumentAttribute().m_bWordWrap ){
					ps.rcPaint.top -= (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				}
				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}
				ps.rcPaint.bottom = m_nViewAlignTop + (LRArg.nModLineTo - m_nViewTopLine + 1)* (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				if( m_nViewAlignTop + m_nViewCy < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				}

			}
			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
			OnSetFocus();
			::ReleaseDC( m_hWnd, hdc );
		}
	}

	/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
	if( NULL != pcmemCopyOfDeleted 		/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
	 && 0 < pcMemDeleted->GetLength()	/* �폜�����f�[�^�̒��� */
	){
		*pcmemCopyOfDeleted = *pcMemDeleted;
	}

	if( !m_bDoing_UndoRedo /* �A���h�D�E���h�D�̎��s���� */
	 && 0 < pcMemDeleted->GetLength()	/* �폜�����f�[�^�̒��� */
	){
		pcOpe->m_nDataLen = pcMemDeleted->GetLength();	/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		pcOpe->m_pcmemData = pcMemDeleted;				/* ����Ɋ֘A����f�[�^ */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}else{
		delete pcMemDeleted;
		pcMemDeleted = NULL;
	}


	if( !m_bDoing_UndoRedo		/* �A���h�D�E���h�D�̎��s���� */
	 && 0 < nInsDataLen			/* �}������f�[�^�̒��� */
	){
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_INSERT;				/* ������ */

//		pcOpe->m_nCaretPosX_Before = nDelColmFrom/*m_nCaretPosX*/;	/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_Before = nDelLineFrom/*m_nCaretPosY*/;	/* ����O�̃L�����b�g�ʒu�x */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			nDelColmFrom,
			nDelLineFrom,
			&pcOpe->m_nCaretPosX_PHY_Before,
			&pcOpe->m_nCaretPosY_PHY_Before
		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			LRArg.nNewPos,
			LRArg.nNewLine,
			&pcOpe->m_nCaretPosX_PHY_To,
			&pcOpe->m_nCaretPosY_PHY_To
		);
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_To;
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_To;


		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}else{
		pcOpe = NULL;
	}


	/* �}������ʒu�փJ�[�\�����ړ� */
	MoveCursor(
		LRArg.nNewPos,		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu(���C�A�E�g���ʒu) */
		LRArg.nNewLine,		/* �}�����ꂽ�����̎��̈ʒu�̍s(���C�A�E�g�s) */
		bRedraw
	);
	m_nCaretPosX_Prev = m_nCaretPosX;


	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
//		pcOpe->m_nCaretPosX_Before = nCaretPosXOld;			/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_Before = nCaretPosYOld;			/* ����O�̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_Before = nCaretPosX_PHY_Old;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = nCaretPosY_PHY_Old;	/* �����̃L�����b�g�ʒu�x */

//		pcOpe->m_nCaretPosX_After = m_nCaretPosX;	/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_After = m_nCaretPosY;	/* �����̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_After,
//			pcOpe->m_nCaretPosY_After,
//			&pcOpe->m_nCaretPosX_PHY_After,
//			&pcOpe->m_nCaretPosY_PHY_After
//		);
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
#if 0
	/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	if( DetectWidthOfLineNumberArea( TRUE ) ){
		for( int i = 0; i < 4; ++i ){
			::DestroyCaret();
			m_nCaretWidth = 0;
			if( m_nMyIndex != i ){
				m_pcEditDoc->m_cEditViewArr[i].DetectWidthOfLineNumberArea( TRUE );
			}
		}
		/* �L�����b�g�̕\���E�X�V */
		ShowEditCaret();
	}
#endif
	//	Jan. 30, 2001 genta
	//	�t�@�C���S�̂̍X�V�t���O�������Ă��Ȃ��Ɗe�s�̍X�V��Ԃ��\������Ȃ��̂�
	//	�t���O�X�V�������ĕ`����O�Ɉړ�����

//#ifdef _DEBUG
//	gm_ProfileOutput = 1;
//	delete pCRunningTimer;
//	pCRunningTimer = NULL;
//
//	gm_ProfileOutput = 0;
//#endif
	return;

}











/* C/C++�X�}�[�g�C���f���g���� */
void CEditView::SmartIndent_CPP( char cChar )
{

//! 2000.1.12 �ҏW����ƃX�}�[�g�C���f���g��Undo/Redo�o�b�t�@�𕪂���ꍇ
//!
//!	/* �A���h�D�o�b�t�@�̏��� */
//!	if( NULL != m_pcOpeBlk ){
//!		if( 0 < m_pcOpeBlk->GetNum() ){	/* ����̐���Ԃ� */
//!			/* ����̒ǉ� */
//!			m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
//!		}else{
//!			delete m_pcOpeBlk;
//!		}
//!		m_pcOpeBlk = NULL;
//!	}
//!	/* �A���h�D�o�b�t�@�̏��� */
//!	m_pcOpeBlk = new COpeBlk;


	const char*	pLine;
	int			nLineLen;
	int			i;
	int			j;
	int			k;
	int			m;
	const char*	pLine2;
	int			nLineLen2;
	int			nLevel;

	/* �����ɂ���Ēu�������ӏ� */
	int			nXFm = -1;
	int			nYFm = -1;
	int			nXTo = -1;
	int			nYTo = -1;

	char*		pszData = NULL;
	int			nDataLen;

	int			nCPX;
	int			nCPY;
	COpe*		pcOpe = NULL;
	int			nWork;
	int			nCaretPosX_PHY;
	CDocLine*	pCDocLine = NULL;
	int			nCharChars;
	int			nSrcLen;
	char		pszSrc[1024];
	BOOL		bChange;


	switch( cChar ){
	case CR:
	case ':':
	case '}':
	case ')':
	case '{':
	case '(':
		break;
	default:
		return;
	}
	switch( cChar ){
	case CR:
	case ':':
	case '}':
	case ')':
	case '{':
	case '(':
		/* �C���f���g�����\�� */
//		if( 0 >= m_nCaretPosY_PHY ){
//			return;
//		}
		nCaretPosX_PHY = m_nCaretPosX_PHY;

		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( m_nCaretPosY_PHY, &nLineLen );
		if( NULL == pLine ){
			if( CR != cChar ){
				return;
			}
			/* �����ɂ���Ēu�������ӏ� */
			nXFm = 0;
			nYFm = m_nCaretPosY_PHY;
			nXTo = 0;
			nYTo = m_nCaretPosY_PHY;
		}else{
			pCDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY );


			if( CR != cChar ){
				nWork = nCaretPosX_PHY - 1;
			}else{
				/*
				|| CR�����͂��ꂽ���A�J�[�\������̎��ʎq���C���f���g����B
				|| �J�[�\������̎��ʎq��'}'��')'�Ȃ��
				|| '}'��')'�����͂��ꂽ���Ɠ�������������
				*/

				for( i = nCaretPosX_PHY; i < nLineLen; i++ ){
					if( TAB != pLine[i] && SPACE != pLine[i] ){
						break;
					}
				}
				if( i < nLineLen ){
					nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
					if( 1 == nCharChars && ( pLine[i] == ')' || pLine[i] == '}' ) ){
						cChar = pLine[i];
					}
					nCaretPosX_PHY = i;
					nWork = nCaretPosX_PHY;
				}else{
					nWork = nCaretPosX_PHY;
				}
			}
			for( i = 0; i < nWork; i++ ){
				if( TAB != pLine[i] && SPACE != pLine[i] ){
					break;
				}
			}
			if( i < nWork ){
				if( ( ':' == cChar
					 && (
							0 == strncmp( &pLine[i], "case", 4 )
						 || 0 == strncmp( &pLine[i], "default:", 8 )
						 || 0 == strncmp( &pLine[i], "public:", 7 )
						 || 0 == strncmp( &pLine[i], "private:", 8 )
						 || 0 == strncmp( &pLine[i], "protected:", 10 )
						)
					)
				 || ( '{' == cChar )
				 || ( '(' == cChar )
				){

				}else{
					return;
				}
			}else{
				if( ':' == cChar ){
					return;
				}
			}
			/* �����ɂ���Ēu�������ӏ� */
			nXFm = 0;
			nYFm = m_nCaretPosY_PHY;
			nXTo = i;
			nYTo = m_nCaretPosY_PHY;
		}


		/* �Ή����銇�ʂ������� */
		nLevel = 0;	/* {}�̓���q���x�� */
//		bString = FALSE;


		nDataLen = 0;
		for( j = m_nCaretPosY_PHY; j >= 0 && NULL != ( pLine2 = m_pcEditDoc->m_cDocLineMgr.GetLineStr( j, &nLineLen2 ) ); --j ){
			if( j == m_nCaretPosY_PHY ){
				nCharChars = &pLine2[nWork] - CMemory::MemCharPrev( pLine2, nLineLen2, &pLine2[nWork] );
				k = nWork - nCharChars;
			}else{
				nCharChars = &pLine2[nLineLen2] - CMemory::MemCharPrev( pLine2, nLineLen2, &pLine2[nLineLen2] );
				k = nLineLen2 - nCharChars;
			}

			for( ; k >= 0; /*k--*/ ){
				if( 1 == nCharChars && ( '}' == pLine2[k] || ')' == pLine2[k] )
				){
					if( 0 < k && '\'' == pLine2[k - 1]
					 && nLineLen2 - 1 > k && '\'' == pLine2[k + 1]
					){
//						MYTRACE( "��[%s]\n", pLine2 );
					}else{
						//�����s�̏ꍇ
						if( j == m_nCaretPosY_PHY ){
							if( '{' == cChar && '}' == pLine2[k] ){
								cChar = '}';
								nLevel--;	/* {}�̓���q���x�� */
//								return;
							}
							if( '(' == cChar && ')' == pLine2[k] ){
								cChar = ')';
								nLevel--;	/* {}�̓���q���x�� */
//								return;
							}
						}

						nLevel++;	/* {}�̓���q���x�� */
					}
				}
				if( 1 == nCharChars && ( '{' == pLine2[k] || '(' == pLine2[k] )
				){
					if( 0 < k && '\'' == pLine2[k - 1]
					 && nLineLen2 - 1 > k && '\'' == pLine2[k + 1]
					){
//						MYTRACE( "��[%s]\n", pLine2 );
					}else{
						//�����s�̏ꍇ
						if( j == m_nCaretPosY_PHY ){
							if( '{' == cChar && '{' == pLine2[k] ){
								return;
							}
							if( '(' == cChar && '(' == pLine2[k] ){
								return;
							}
						}
						if( 0 == nLevel ){
							break;
						}else{
							nLevel--;	/* {}�̓���q���x�� */
						}

					}
				}
				nCharChars = &pLine2[k] - CMemory::MemCharPrev( pLine2, nLineLen2, &pLine2[k] );
				if( 0 == nCharChars ){
					nCharChars = 1;
				}
				k -= nCharChars;
			}
			if( k < 0 ){
				/* ���̍s�ɂ͂Ȃ� */
				continue;
			}

			for( m = 0; m < nLineLen2; m++ ){
				if( TAB != pLine2[m] && SPACE != pLine2[m] ){
					break;
				}
			}


			nDataLen = m;
			pszData = new char[nDataLen + 2];
			memcpy( pszData, pLine2, nDataLen );
			if( CR  == cChar
			 || '{' == cChar
			 || '(' == cChar
			){
				pszData[nDataLen] = '\t';
				pszData[nDataLen + 1] = '\0';
				++nDataLen;
			}else{
				pszData[nDataLen] = '\0';

			}
			break;
		}
		if( j < 0 ){
			/* �Ή����銇�ʂ�������Ȃ����� */
			if( CR == cChar ){
				return;
			}else{
				nDataLen = 0;
				pszData = new char[nDataLen + 1];
				pszData[nDataLen] = '\0';
			}
		}

		/* ������̃J�[�\���ʒu���v�Z���Ă��� */
		nCPX = nCaretPosX_PHY - nXTo + nDataLen;
		nCPY = m_nCaretPosY_PHY;

		nSrcLen = nXTo - nXFm;
		if( nSrcLen >= sizeof( pszSrc ) - 1 ){
			return;
		}
		if( NULL == pLine ){
			pszSrc[0] = '\0';
		}else{
			memcpy( pszSrc, &pLine[nXFm], nSrcLen );
			pszSrc[nSrcLen] = '\0';
		}


		/* �����ɂ���Ēu�������ӏ� */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nXFm, nYFm, &nXFm, &nYFm );
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nXTo, nYTo, &nXTo, &nYTo );

		if( ( 0 == nDataLen && nYFm == nYTo && nXFm == nXTo )
		 || ( nDataLen == nSrcLen && 0 == memcmp( pszSrc, pszData, nDataLen ) )
		 ){
			bChange = FALSE;
		}else{
			bChange = TRUE;

			/* �f�[�^�u�� �폜&�}���ɂ��g���� */
			ReplaceData_CEditView(
				nYFm,		/* �폜�͈͍s  From ���C�A�E�g�s�ԍ� */
				nXFm,		/* �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu */
				nYTo,		/* �폜�͈͍s  To   ���C�A�E�g�s�ԍ� */
				nXTo,		/* �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu */
				NULL,		/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
				pszData,	/* �}������f�[�^ */
				nDataLen,	/* �}������f�[�^�̒��� */
				TRUE
			//	BOOL		bUndo			/* Undo���삩�ǂ��� */
			);
		}


		/* �J�[�\���ʒu���� */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nCPX, nCPY, &nCPX, &nCPY );
		/* �I���G���A�̐擪�փJ�[�\�����ړ� */
		MoveCursor( nCPX, nCPY, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;


		if( bChange && !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */
			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		break;
	}
	if( NULL != pszData ){
		delete pszData;
		pszData = NULL;
	}
	return;
}


// 2001/06/20 Start by asa-o

// �e�L�X�g���P�s���փX�N���[��
void CEditView::Command_WndScrollDown( void )
{
	int	nCaretMarginY;

	nCaretMarginY = m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	nCaretMarginY += 2;

	if( m_nCaretPosY > m_nViewRowNum + m_nViewTopLine - (nCaretMarginY + 1) ){
		CaretUnderLineOFF(TRUE);
	}

	ScrollAtV(m_nViewTopLine - 1);

	// �e�L�X�g���I������Ă��Ȃ�
	if( !IsTextSelected() )
	{
		// �J�[�\������ʊO�ɏo��
		if( m_nCaretPosY > m_nViewRowNum + m_nViewTopLine - nCaretMarginY )
		{
			if( m_nCaretPosY > m_pcEditDoc->m_cDocLineMgr.GetLineCount() - nCaretMarginY )
				Cursor_UPDOWN( (m_pcEditDoc->m_cDocLineMgr.GetLineCount() - nCaretMarginY) - m_nCaretPosY, FALSE );
			else
				Cursor_UPDOWN( -1, FALSE);
			DrawCaretPosInfo();
		}
	}
	if( m_pShareData->m_Common.m_bSplitterWndVScroll )	// �����X�N���[���̓������Ƃ�
	{
		CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x01];
		pcEditView -> ScrollAtV( m_nViewTopLine );
	}

	CaretUnderLineON(TRUE);
}

// �e�L�X�g���P�s��փX�N���[��
void CEditView::Command_WndScrollUp(void)
{
	int	nCaretMarginY;

	nCaretMarginY = m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	if( m_nCaretPosY < m_nViewTopLine + (nCaretMarginY + 1) ){
		CaretUnderLineOFF( TRUE );
	}

	ScrollAtV( m_nViewTopLine + 1 );

	// �e�L�X�g���I������Ă��Ȃ�
	if( !IsTextSelected() )
	{
		// �J�[�\������ʊO�ɏo��
		if( m_nCaretPosY < m_nViewTopLine + nCaretMarginY )
		{
			if( m_nViewTopLine == 1 )
				Cursor_UPDOWN( nCaretMarginY + 1, FALSE );
			else
				Cursor_UPDOWN( 1, FALSE );
			DrawCaretPosInfo();
		}
	}
	if( m_pShareData->m_Common.m_bSplitterWndVScroll )	// �����X�N���[���̓������Ƃ�
	{
		CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x01];
		pcEditView -> ScrollAtV( m_nViewTopLine );
	}

	CaretUnderLineON( TRUE );
}

// 2001/06/20 End


/*[EOF]*/
