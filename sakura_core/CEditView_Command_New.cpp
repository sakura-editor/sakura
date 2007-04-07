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
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CEditView.h"
#include "CWaitCursor.h"
#include "charcode.h"
#include "CRunningTimer.h"
#include <algorithm>		// 2001.12.11 hor    for VC++
#include "COpe.h" ///	2002/2/3 aroka from here
#include "COpeBlk.h" ///
#include "CLayout.h"///
#include "CDocLine.h"///
#include "mymessage.h"///
#include "debug.h"///
#include "etc_uty.h"///
#include <string>///
#include <vector> /// 2002/2/3 aroka to here
#include "COsVersionInfo.h"   // 2002.04.09 minfu 
#include "CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "CEditWnd.h"
#include "CDlgCtrlCode.h"	//�R���g���[���R�[�h�̓���(�_�C�A���O)
#include "CDlgFavorite.h"	//���C�ɓ���	//@@@ 2003.04.08 MIK

using namespace std; // 2002/2/3 aroka to here

#ifndef FID_RECONVERT_VERSION  // 2002.04.10 minfu 
#define FID_RECONVERT_VERSION 0x10000000
#endif
/*!	���݈ʒu�Ƀf�[�^��}�� Ver0

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CEditView::InsertData_CEditView(
	int			nX,
	int			nY,
	const char*	pData,
	int			nDataLen,
	int*		pnNewLine,			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int*		pnNewPos,			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	COpe*		pcOpe,				/* �ҏW����v�f COpe */
	BOOL		bRedraw
//	BOOL		bUndo			/* Undo���삩�ǂ��� */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::InsertData_CEditView" );
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

	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nY, &nLineLen, &pcLayout );

	nIdxFrom = 0;
//	cMem.SetData( "", lstrlen( "" ) );
	cMem.SetDataSz( "" );
	if( NULL != pLine ){
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
		nIdxFrom = LineColmnToIndex2( pcLayout, nX, nLineAllColLen );
		/* �s�I�[���E�ɑ}�����悤�Ƃ��� */
		if( nLineAllColLen > 0 ){
			/* �I�[���O����}���ʒu�܂ŋ󔒂𖄂߂�ׂ̏��� */
			/* �s�I�[�����炩�̉��s�R�[�h��? */
			if( EOL_NONE != pcLayout->m_cEol ){
				nIdxFrom = nLineLen - 1;
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
				LineIndexToColmn( pcLayout, nIdxFrom ),
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
		cMem.GetPtr(),
		cMem.GetLength(),
		&nModifyLayoutLinesOld,
		&nInsLineNum,
		pnNewLine,			/* �}�����ꂽ�����̎��̈ʒu�̍s */
		pnNewPos			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	);


	/* ���������Ċm�ۂ���ăA�h���X�������ɂȂ�̂ŁA�ēx�A�s�f�[�^�����߂� */
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nY, &nLineLen );

	/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
	pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( *pnNewLine, &nLineLen2, &pcLayout );
	if( pLine2 != NULL ){
		*pnNewPos = LineIndexToColmn( pcLayout, *pnNewPos );
	}
	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if( *pnNewPos >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() ){
		if( m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet
		 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.16 MIK
		{
			if( m_pcEditDoc->m_cLayoutMgr.IsEndOfLine( *pnNewLine, *pnNewPos ) )	//@@@ 2002.04.18
			{
				*pnNewPos = 0;
				(*pnNewLine)++;
			}
		}
		else
		{
			// Oct. 7, 2002 YAZAKI
			*pnNewPos = pcLayout->m_pNext ? pcLayout->m_pNext->GetIndent() : 0;
			(*pnNewLine)++;
		}
	}

//	MYTRACE( "nModifyLayoutLinesOld=%d nInsLineNum=%d *pnNewLine=%d *pnNewPos=%d\n", nModifyLayoutLinesOld, nInsLineNum, *pnNewLine, *pnNewPos );


	/* ��ԑJ�� */
	if( FALSE == m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	/* �ĕ`�� */
	/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
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

				// 2002.02.25 Mod By KK ���s (nY - m_nViewTopLine - 1); => (nY - m_nViewTopLine);
				//ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nY - m_nViewTopLine - 1);
				ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (nY - m_nViewTopLine);

				//�֑�������ꍇ��1�s�O����ĕ`����s��	@@@ 2002.04.19 MIK
				if( m_pcEditDoc->GetDocumentAttribute().m_bWordWrap
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuHead	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuTail	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.19 MIK
				{
					ps.rcPaint.top -= (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				}
				if( ps.rcPaint.top < 0 ){
					ps.rcPaint.top = 0;
				}

				ps.rcPaint.bottom = ps.rcPaint.top + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * ( nModifyLayoutLinesOld + 1);
				if( m_nViewAlignTop + m_nViewCy < ps.rcPaint.bottom ){
					ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
				}
			}
			hdc = ::GetDC( m_hWnd );
//			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
//			OnSetFocus();
			::ReleaseDC( m_hWnd, hdc );
		}
	}

	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe->m_nOpe = OPE_INSERT;				/* ������ */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			*pnNewPos,
			*pnNewLine,
			&pcOpe->m_nCaretPosX_PHY_To,
			&pcOpe->m_nCaretPosY_PHY_To
		);

		pcOpe->m_nDataLen = cMem.GetLength();	/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		pcOpe->m_pcmemData = NULL;				/* ����Ɋ֘A����f�[�^ */
	}
#ifdef _DEBUG
	gm_ProfileOutput = 0;
#endif
	return;
}


/*!	�w��ʒu�̎w�蒷�f�[�^�폜

	@date 2002/03/24 YAZAKI bUndo�폜
	@date 2002/05/12 YAZAKI bRedraw, bRedraw2�폜�i���FALSE������j
*/
void CEditView::DeleteData2(
	int			nCaretX,
	int			nCaretY,
	int			nDelLen,
	CMemory*	pcMem,
	COpe*		pcOpe		/* �ҏW����v�f COpe */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DeleteData(1)" );
#endif
	const char*	pLine;
	int			nLineLen;
	int			nIdxFrom;
	int			nModifyLayoutLinesOld;
	int			nModifyLayoutLinesNew;
	int			nDeleteLayoutLines;
	int			bLastLine;

	/* �Ō�̍s�ɃJ�[�\�������邩�ǂ��� */
	if( nCaretY == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
		bLastLine = 1;
	}else{
		bLastLine = 0;
	}

	const CLayout* pcLayout;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCaretY, &nLineLen, &pcLayout );
	if( NULL == pLine ){
		goto end_of_func;
	}
	nIdxFrom = LineColmnToIndex( pcLayout, nCaretX );
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
			LineIndexToColmn( pcLayout, nIdxFrom + nDelLen ),
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
		*pcMem
	);

	if( !m_bDoing_UndoRedo && NULL != pcOpe ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe->m_nDataLen = pcMem->GetLength();	/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		pcOpe->m_pcmemData = pcMem;				/* ����Ɋ֘A����f�[�^ */
	}

	/* �I���G���A�̐擪�փJ�[�\�����ړ� */
	MoveCursor( nCaretX, nCaretY, FALSE );
	m_nCaretPosX_Prev = m_nCaretPosX;


end_of_func:;
#ifdef _DEBUG
	gm_ProfileOutput = 0;
#endif
	return;

}





/*!	�J�[�\���ʒu�܂��͑I���G���A���폜

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CEditView::DeleteData(
	BOOL	bRedraw
//	BOOL	bUndo	/* Undo���삩�ǂ��� */
)
{
#ifdef _DEBUG
	gm_ProfileOutput = 1;
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DeleteData(2)" );
#endif
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			nLineNum;
	int			nCurIdx;
	int			nNxtIdx;
	int			nNxtPos;
	PAINTSTRUCT ps;
	HDC			hdc;
	int			nIdxFrom;
	int			nIdxTo;
	int			nDelPos;
	int			nDelLen;
	int			nDelPosNext;
	int			nDelLenNext;
//	CMemory		cmemBuf;
	RECT		rcSel;
	int			bLastLine;
	CMemory*	pcMemDeleted;
	COpe*		pcOpe = NULL;
	int			nCaretPosXOld;
	int			nCaretPosYOld;
	int			i;
	const CLayout*	pcLayout;
	int			nSelectColmFrom_Old;
	int			nSelectLineFrom_Old;

// hor IsTextSelected���Ɉړ�
//	CWaitCursor cWaitCursor( m_hWnd );	// 2002.01.25 hor

	nCaretPosXOld = m_nCaretPosX;
	nCaretPosYOld = m_nCaretPosY;

	/* �e�L�X�g���I������Ă��邩 */
	if( IsTextSelected() ){
		CWaitCursor cWaitCursor( m_hWnd );  // 2002.02.05 hor
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */

			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}

		/* ��`�͈͑I�𒆂� */
		if( m_bBeginBoxSelect ){
			m_pcEditDoc->SetModified(true,bRedraw);	//	2002/06/04 YAZAKI ��`�I�����폜�����Ƃ��ɕύX�}�[�N�����Ȃ��B

			m_bDrawSWITCH=FALSE;	// 2002.01.25 hor
//			bBoxSelected = TRUE;	// 2002/2/3 aroka
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
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
					nIdxFrom = LineColmnToIndex( pcLayout, rcSel.left  );
					nIdxTo	 = LineColmnToIndex( pcLayout, rcSel.right );

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
						pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + 1, &nLineLen2, &pcLayout );
						m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
							LineIndexToColmn( pcLayout, nDelPos ),
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
						pcOpe				/* �ҏW����v�f COpe */
//						FALSE/*bRedraw	2002.01.25 hor*/,
//						FALSE/*bRedraw*	2002.01.25 hor*/
					);

					if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
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
			m_bDrawSWITCH=TRUE;	// 2002.01.25 hor

			/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
			if ( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
				/* �L�����b�g�̕\���E�X�V */
				ShowEditCaret();
			}
			if( bRedraw ){
				/* �X�N���[���o�[�̏�Ԃ��X�V���� */
				AdjustScrollBars();

				/* �ĕ`�� */
				hdc = ::GetDC( m_hWnd );
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
	//			ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (m_nCaretPosY - m_nViewTopLine);
				ps.rcPaint.top = m_nViewAlignTop;
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//				OnKillFocus();
				OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
//				OnSetFocus();
				::ReleaseDC( m_hWnd, hdc );
			}
			/* �I���G���A�̐擪�փJ�[�\�����ړ� */
			::UpdateWindow( m_hWnd );
			MoveCursor( nSelectColmFrom_Old, nSelectLineFrom_Old, bRedraw );
			m_nCaretPosX_Prev = m_nCaretPosX;
			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				pcOpe = new COpe;
				pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
					nCaretPosXOld,
					nCaretPosYOld,
					&pcOpe->m_nCaretPosX_PHY_Before,
					&pcOpe->m_nCaretPosY_PHY_Before
				);

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
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL == pLine ){
			goto end_of_func;
//			return;
		}
		/* �Ō�̍s�ɃJ�[�\�������邩�ǂ��� */
		if( m_nCaretPosY == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
			bLastLine = 1;
		}else{
			bLastLine = 0;
		}

		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
		nCurIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );
//		MYTRACE("nLineLen=%d nCurIdx=%d \n", nLineLen, nCurIdx);
		if( nCurIdx == nLineLen && bLastLine ){	/* �S�e�L�X�g�̍Ō� */
			goto end_of_func;
//			return;
		}
		/* �w�肳�ꂽ���̕����̃o�C�g���𒲂ׂ� */
		if( pLine[nCurIdx] == '\r' || pLine[nCurIdx] == '\n' ){
			/* ���s */
			nNxtIdx = nCurIdx + pcLayout->m_cEol.GetLen();
			nNxtPos = m_nCaretPosX + pcLayout->m_cEol.GetLen();
		}else{
			nNxtIdx = CMemory::MemCharNext( pLine, nLineLen, &pLine[nCurIdx] ) - pLine;
			/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
			nNxtPos = LineIndexToColmn( pcLayout, nNxtIdx );
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

	m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() > 0 ){
		if( m_nCaretPosY > m_pcEditDoc->m_cLayoutMgr.GetLineCount()	- 1	){
			/* ���ݍs�̃f�[�^���擾 */
			const CLayout*	pcLayout;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				goto end_of_func;
			}
			/* ���s�ŏI����Ă��邩 */
			if( ( EOL_NONE != pcLayout->m_cEol ) ){
				goto end_of_func;
			}
			/*�t�@�C���̍Ō�Ɉړ� */
			Command_GOFILEEND( FALSE );
		}
	}
end_of_func:;

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

	MY_RUNNINGTIMER( cRunningTimer, "CEditView::Command_UNDO()" );

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
//@@@ 2002.01.03 YAZAKI �s�g�p�̂���
//	BOOL		bUndo;	/* Undo���삩�ǂ��� */
//	bUndo = TRUE;	/* Undo���삩�ǂ��� */

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
		m_bDrawSWITCH = FALSE;	//	hor
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


		// 2002.02.16 hor
		//	if( i == nOpeBlkNum - 1	){
		//		/* �J�[�\�����ړ� */
		//		MoveCursor( nCaretPosX_After, nCaretPosY_After, TRUE );
		//	}else{
				/* �J�[�\�����ړ� */
				MoveCursor( nCaretPosX_After, nCaretPosY_After, FALSE );
		//	}
			switch( pcOpe->m_nOpe ){
			case OPE_INSERT:
				pcMem = new CMemory;

				/* �I��͈͂̕ύX */
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
						pcOpe->m_pcmemData->GetPtr(),	/* �}������f�[�^ */
						pcOpe->m_nDataLen,					/* �}������f�[�^�̒��� */
						FALSE								/*�ĕ`�悷�邩�ۂ�*/
					);

//					InsertData_CEditView(
//						nCaretPosX_Before,
//						nCaretPosY_Before,
//						pcOpe->m_pcmemData->GetPtr(),
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
				MoveCursor( nCaretPosX_After, nCaretPosY_After, FALSE/*TRUE 2002.02.16 hor */ );
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
		m_bDrawSWITCH = TRUE;	//	hor

		/* Undo��̕ύX�t���O */
		m_pcEditDoc->SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

		SetBracketPairPos( true );	// 03/03/07 ai

		/* �ĕ`�� */
		hdc = ::GetDC( m_hWnd );
		ps.rcPaint.left = 0;
		ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
		ps.rcPaint.top = m_nViewAlignTop;
		ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//		OnKillFocus();
		OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
//		OnSetFocus();
		DispRuler( hdc );
		::ReleaseDC( m_hWnd, hdc );
		/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
		if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
			/* �L�����b�g�̕\���E�X�V */
			ShowEditCaret();
		}

		m_pcEditDoc->RedrawInactivePane();/* ���̃y�C���̕\����Ԃ��X�V */
#if 0
	//	2001/06/21 Start by asa-o: ���̃y�C���̕\����Ԃ��X�V
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^1].Redraw();
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^2].Redraw();
		m_pcEditDoc->m_cEditViewArr[(m_nMyIndex^1)^2].Redraw();
	//	2001/06/21 End
#endif

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
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::Command_REDO()" );

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
						pcOpe->m_pcmemData->GetPtr(),	/* �}������f�[�^ */
						pcOpe->m_pcmemData->GetLength(),	/* �}������f�[�^�̒��� */
						FALSE								/*�ĕ`�悷�邩�ۂ�*/
					);

//					InsertData_CEditView(
//						nCaretPosX_Before,
//						nCaretPosY_Before,
//						pcOpe->m_pcmemData->GetPtr(),
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
		/* Redo��̕ύX�t���O */
		m_pcEditDoc->SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

		SetBracketPairPos( true );	// 03/03/07 ai

		/* �ĕ`�� */
		hdc = ::GetDC( m_hWnd );
		ps.rcPaint.left = 0;
		ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
		ps.rcPaint.top = m_nViewAlignTop;
		ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//		OnKillFocus();
		OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
//		OnSetFocus();
		::ReleaseDC( m_hWnd, hdc );

		/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
		if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( TRUE ) ){
			/* �L�����b�g�̕\���E�X�V */
			ShowEditCaret();
		}

		m_pcEditDoc->RedrawInactivePane();/* ���̃y�C���̕\����Ԃ��X�V */
#if 0
	//	2001/06/21 Start by asa-o: ���̃y�C���̕\����Ԃ��X�V
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^1].Redraw();
		m_pcEditDoc->m_cEditViewArr[m_nMyIndex^2].Redraw();
		m_pcEditDoc->m_cEditViewArr[(m_nMyIndex^1)^2].Redraw();
	//	2001/06/21 End
#endif
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
		const CLayout* pcLayout;
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nDelLineFrom, &len, &pcLayout );
		//	Jun. 1, 2000 genta
		//	������NULL�`�F�b�N���܂��傤
		if( line != NULL ){
			pos = LineColmnToIndex( pcLayout, nDelColmFrom );
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
		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nDelLineTo, &len, &pcLayout );
		if( line != NULL ){
			pos = LineIndexToColmn( pcLayout, len );

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
//	LRArg.bUndo = m_bDoing_UndoRedo;					/* Undo���삩�ǂ��� */
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
		m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	}

	/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	DisableSelectArea( bRedraw );

	/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	if( m_pcEditDoc->DetectWidthOfLineNumberAreaAllPane( bRedraw ) ){
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
				//ps.rcPaint.top = m_nViewAlignTop + (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace) * (m_nCaretPosY - m_nViewTopLine); // 2002.02.25 Del By KK ���ŏ㏑������Ă��邽�ߖ��g�p�B
				//ps.rcPaint.top = 0/*m_nViewAlignTop*/;			// 2002.02.25 Del By KK
				ps.rcPaint.top = m_nViewAlignTop - m_nTopYohaku;	// ���[���[��͈͂Ɋ܂߂Ȃ��B2002.02.25 Add By KK
				ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			}else{
				ps.rcPaint.left = 0;
				ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;

				/* �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��) */
				ps.rcPaint.top = m_nViewAlignTop + (LRArg.nModLineFrom - m_nViewTopLine)* (m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace);
				if( m_pcEditDoc->GetDocumentAttribute().m_bWordWrap
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuHead	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuTail	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet	//@@@ 2002.04.19 MIK
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.19 MIK
				{
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
//			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
//			OnSetFocus();
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
	//	Jan. 30, 2001 genta
	//	�t�@�C���S�̂̍X�V�t���O�������Ă��Ȃ��Ɗe�s�̍X�V��Ԃ��\������Ȃ��̂�
	//	�t���O�X�V�������ĕ`����O�Ɉړ�����
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


			//	nWork�ɏ����̊���ʒu��ݒ肷��
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
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
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
					//	Sep. 18, 2002 �����
					|| (( '{' == cChar ) && ( '#' != pLine[i] ))
					|| (( '(' == cChar ) && ( '#' != pLine[i] ))
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
		for( j = m_nCaretPosY_PHY; j >= 0; --j ){
			pLine2 = m_pcEditDoc->m_cDocLineMgr.GetLineStr( j, &nLineLen2 );
			if( j == m_nCaretPosY_PHY ){
				// 2005.10.11 ryoji EOF �݂̂̍s���X�}�[�g�C���f���g�̑Ώۂɂ���
				if( NULL == pLine2 ){
					if( m_nCaretPosY_PHY == m_pcEditDoc->m_cDocLineMgr.GetLineCount() )
						continue;	// EOF �݂̂̍s
					break;
				}
				nCharChars = &pLine2[nWork] - CMemory::MemCharPrev( pLine2, nLineLen2, &pLine2[nWork] );
				k = nWork - nCharChars;
			}else{
				if( NULL == pLine2 )
					break;
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
			nCharChars = (m_pcEditDoc->GetDocumentAttribute().m_bInsSpace)? m_pcEditDoc->m_cLayoutMgr.GetTabSpace(): 1;
			pszData = new char[nDataLen + nCharChars + 1];
			memcpy( pszData, pLine2, nDataLen );
			if( CR  == cChar
			 || '{' == cChar
			 || '(' == cChar
			){
				// 2005.10.11 ryoji TAB�L�[��SPACE�}���̐ݒ�Ȃ�ǉ��C���f���g��SPACE�ɂ���
				//	����������̉E�[�̕\���ʒu�����߂���ő}������X�y�[�X�̐������肷��
				if( m_pcEditDoc->GetDocumentAttribute().m_bInsSpace ){	// SPACE�}���ݒ�
					i = m = 0;
					while( i < nDataLen ){
						nCharChars = CMemory::GetSizeOfChar( pszData, nDataLen, i );
						if( nCharChars == 1 && TAB == pszData[i] )
							m += m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace(m);
						else
							m += nCharChars;
						i += nCharChars;
					}
					nCharChars = m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace(m);
					for( i = 0; i < nCharChars; i++ )
						pszData[nDataLen + i] = SPACE;
					pszData[nDataLen + nCharChars] = '\0';
					nDataLen += nCharChars;
				}else{
					pszData[nDataLen] = TAB;
					pszData[nDataLen + 1] = '\0';
					++nDataLen;
				}
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
			//	Sep. 18, 2002 genta ���������[�N�΍�
			delete [] pszData;
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
		delete [] pszData;
		pszData = NULL;
	}
	return;
}


/* 2005.10.11 ryoji �O�̍s�ɂ��閖���̋󔒂��폜 */
void CEditView::RTrimPrevLine( void )
{
	const char*	pLine;
	int			nLineLen;
	int			i;
	int			j;
	int			nXFm;
	int			nYFm;
	int			nXTo;
	int			nYTo;
	int			nCPX;
	int			nCPY;
	int			nCharChars;
	int			nCaretPosX_PHY;
	int			nCaretPosY_PHY;
	COpe*		pcOpe = NULL;

	nCaretPosX_PHY = m_nCaretPosX_PHY;
	nCaretPosY_PHY = m_nCaretPosY_PHY;

	if( m_nCaretPosY_PHY > 0 ){
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStrWithoutEOL( m_nCaretPosY_PHY - 1, &nLineLen );
		if( NULL != pLine && nLineLen > 0 ){
			i = j = 0;
			while( i < nLineLen ){
				nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
				if( 1 == nCharChars ){
					if( TAB != pLine[i] && SPACE != pLine[i])
						j = i + nCharChars;
				}
				else if( 2 == nCharChars ){
					if( !((unsigned char)pLine[i] == (unsigned char)0x81 && (unsigned char)pLine[i + 1] == (unsigned char)0x40) )
						j = i + nCharChars;
				}
				i += nCharChars;
			}
			if( j < nLineLen ){
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( j, m_nCaretPosY_PHY - 1, &nXFm, &nYFm );
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nLineLen, m_nCaretPosY_PHY - 1, &nXTo, &nYTo );
				if( !( nXFm >= nXTo && nYFm == nYTo) ){
					ReplaceData_CEditView(
						nYFm,		/* �폜�͈͍s  From ���C�A�E�g�s�ԍ� */
						nXFm,		/* �폜�͈͈ʒuFrom ���C�A�E�g�s���ʒu */
						nYTo,		/* �폜�͈͍s  To   ���C�A�E�g�s�ԍ� */
						nXTo,		/* �폜�͈͈ʒuTo   ���C�A�E�g�s���ʒu */
						NULL,		/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
						NULL,		/* �}������f�[�^ */
						0,			/* �}������f�[�^�̒��� */
						TRUE
					);
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nCaretPosX_PHY, nCaretPosY_PHY, &nCPX, &nCPY );
					MoveCursor( nCPX, nCPY, TRUE );

					if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
						pcOpe = new COpe;
						pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
						pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
						pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */
						pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
						pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
						/* ����̒ǉ� */
						m_pcOpeBlk->AppendOpe( pcOpe );
					}
				}
			}
		}
	}
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
		m_cUnderLine.CaretUnderLineOFF( TRUE );
	}

	//	Sep. 11, 2004 genta �����p�ɍs�����L��
	//	Sep. 11, 2004 genta �����X�N���[���̊֐���
	SyncScrollV( ScrollAtV(m_nViewTopLine - 1));

	// �e�L�X�g���I������Ă��Ȃ�
	if( !IsTextSelected() )
	{
		// �J�[�\������ʊO�ɏo��
		if( m_nCaretPosY > m_nViewRowNum + m_nViewTopLine - nCaretMarginY )
		{
// From Here 2001.12.03 hor
//			if( m_nCaretPosY > m_pcEditDoc->m_cDocLineMgr.GetLineCount() - nCaretMarginY )
//				Cursor_UPDOWN( (m_pcEditDoc->m_cDocLineMgr.GetLineCount() - nCaretMarginY) - m_nCaretPosY, FALSE );
// To Here 2001.12.03 hor
			if( m_nCaretPosY > m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nCaretMarginY )
				Cursor_UPDOWN( (m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nCaretMarginY) - m_nCaretPosY, FALSE );
			else
				Cursor_UPDOWN( -1, FALSE);
			DrawCaretPosInfo();
		}
	}

	m_cUnderLine.CaretUnderLineON( TRUE );
}

// �e�L�X�g���P�s��փX�N���[��
void CEditView::Command_WndScrollUp(void)
{
	int	nCaretMarginY;

	nCaretMarginY = m_nViewRowNum / _CARETMARGINRATE;
	if( nCaretMarginY < 1 )
		nCaretMarginY = 1;

	if( m_nCaretPosY < m_nViewTopLine + (nCaretMarginY + 1) ){
		m_cUnderLine.CaretUnderLineOFF( TRUE );
	}

	//	Sep. 11, 2004 genta �����p�ɍs�����L��
	//	Sep. 11, 2004 genta �����X�N���[���̊֐���
	SyncScrollV( ScrollAtV( m_nViewTopLine + 1 ));

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

	m_cUnderLine.CaretUnderLineON( TRUE );
}

// 2001/06/20 End



/* ���̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void CEditView::Command_GONEXTPARAGRAPH( int bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = 0;
	
	bool nFirstLineIsEmptyLine = false;
	/* �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ������� */
	if ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer++;
	}
	else {
		// EOF�s�ł����B
		return;
	}

	/* ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂� */
	while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer++;
		}
		else {
			break;
		}
	};

	/*	nFirstLineIsEmptyLine����s��������A�����Ă���Ƃ���͔��s�B���Ȃ킿�����܂��B
		nFirstLineIsEmptyLine�����s��������A�����Ă���Ƃ���͋�s�B
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	�����܂��B
	}
	else {
		//	���܌��Ă���Ƃ���͋�s��1�s��
		if ( m_pShareData->m_Common.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
		}
		else {
			/* �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői�� */
			while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					nCaretPointer++;
				}
				else {
					break;
				}
			};
		}
	}

	//	EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	/* �ړ��������v�Z */
	int nCaretPosX_Layo;
	int nCaretPosY_Layo;

	/* �ړ��O�̕����ʒu */
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		m_nCaretPosX_PHY, m_nCaretPosY_PHY,
		&nCaretPosX_Layo, &nCaretPosY_Layo
	);

	/* �ړ���̕����ʒu */
	int nCaretPosY_Layo_CaretPointer;
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		m_nCaretPosX_PHY, m_nCaretPosY_PHY + nCaretPointer,
		&nCaretPosX_Layo, &nCaretPosY_Layo_CaretPointer
	);

	Cursor_UPDOWN( nCaretPosY_Layo_CaretPointer - nCaretPosY_Layo, bSelect );
	return;
}

/* �O�̒i���֐i��
	2002/04/26 �i���̗��[�Ŏ~�܂�I�v�V������ǉ�
	2002/04/19 �V�K
*/
void CEditView::Command_GOPREVPARAGRAPH( int bSelect )
{
	CDocLine* pcDocLine;
	int nCaretPointer = -1;

	bool nFirstLineIsEmptyLine = false;
	/* �܂��́A���݈ʒu����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ������� */
	if ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ){
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		nCaretPointer--;
	}
	else {
		nFirstLineIsEmptyLine = true;
		nCaretPointer--;
	}

	/* ���ɁAnFirstLineIsEmptyLine�ƈقȂ�Ƃ���܂œǂݔ�΂� */
	while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ) {
		if ( pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine ){
			nCaretPointer--;
		}
		else {
			break;
		}
	};

	/*	nFirstLineIsEmptyLine����s��������A�����Ă���Ƃ���͔��s�B���Ȃ킿�����܂��B
		nFirstLineIsEmptyLine�����s��������A�����Ă���Ƃ���͋�s�B
	*/
	if ( nFirstLineIsEmptyLine == true ){
		//	�����܂��B
		if ( m_pShareData->m_Common.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
			nCaretPointer++;	//	��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}
		else {
			/* �d�グ�ɁA��s����Ȃ��Ƃ���܂Ői�� */
			while ( pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY + nCaretPointer ) ) {
				if ( pcDocLine->IsEmptyLine() ){
					break;
				}
				else {
					nCaretPointer--;
				}
			};
			nCaretPointer++;	//	��s�̍ŏ�s�i�i���̖��[�̎��̍s�j�Ŏ~�܂�B
		}
	}
	else {
		//	���܌��Ă���Ƃ���͋�s��1�s��
		if ( m_pShareData->m_Common.m_bStopsBothEndsWhenSearchParagraph ){	//	�i���̗��[�Ŏ~�܂�
			nCaretPointer++;
		}
		else {
			nCaretPointer++;
		}
	}

	//	EOF�܂ŗ�����A�ړI�̏ꏊ�܂ł����̂ňړ��I���B

	/* �ړ��������v�Z */
	int nCaretPosX_Layo;
	int nCaretPosY_Layo;

	/* �ړ��O�̕����ʒu */
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		m_nCaretPosX_PHY, m_nCaretPosY_PHY,
		&nCaretPosX_Layo, &nCaretPosY_Layo
	);

	/* �ړ���̕����ʒu */
	int nCaretPosY_Layo_CaretPointer;
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		m_nCaretPosX_PHY, m_nCaretPosY_PHY + nCaretPointer,
		&nCaretPosX_Layo, &nCaretPosY_Layo_CaretPointer
	);

	Cursor_UPDOWN( nCaretPosY_Layo_CaretPointer - nCaretPosY_Layo, bSelect );
	return;
}

// From Here 2001.12.03 hor

//! �u�b�N�}�[�N�̐ݒ�E�������s��(�g�O������)
void CEditView::Command_BOOKMARK_SET(void)
{
	CDocLine*	pCDocLine;
	int			nX=0;
	int			nY;
	int			nYfrom,nYto;
	if( IsTextSelected() && m_nSelectLineFrom<m_nSelectLineTo ){
		nYfrom=m_nSelectLineFrom;
		nYto  =m_nSelectLineTo;
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(nX,nYfrom,&nX,&nYfrom);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(nX,nYto,&nX,&nYto);
		for(nY=nYfrom;nY<=nYto;nY++){
			pCDocLine=m_pcEditDoc->m_cDocLineMgr.GetLineInfo( nY );
			if(NULL!=pCDocLine)pCDocLine->SetBookMark(!pCDocLine->IsBookMarked());
		}
	}else{
		pCDocLine=m_pcEditDoc->m_cDocLineMgr.GetLineInfo( m_nCaretPosY_PHY );
		if(NULL!=pCDocLine)pCDocLine->SetBookMark(!pCDocLine->IsBookMarked());
	}
	// 2002.01.16 hor ���������r���[���X�V
	for( int v = 0; v < 4; ++v ) if( m_pcEditDoc->m_nActivePaneIndex != v )m_pcEditDoc->m_cEditViewArr[v].Redraw();
	Redraw();
	return;
}



//! ���̃u�b�N�}�[�N��T���C����������ړ�����
void CEditView::Command_BOOKMARK_NEXT(void)
{
//	CDocLine*	pCDocLine;
	int			nX=0;
	int			nY;
//	int			nRet;
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor
	nY=m_nCaretPosY_PHY;
	nYOld=nY;						// hor
re_do:;								// hor
	if(m_pcEditDoc->m_cDocLineMgr.SearchBookMark(nY, 1 /* ������� */, &nY)){
		bFound = TRUE;				// hor
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(nX,nY,&nX,&nY);
		//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		MoveCursorSelecting( nX, nY, m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(m_pShareData->m_Common.m_bSearchAll){
		if(!bFound	&&		// ������Ȃ�����
			bRedo			// �ŏ��̌���
		){
			nY=-1;	//	2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;		// �擪����Č���
		}
	}
	if(bFound){
		if(nYOld >= nY)SendStatusMessage("���擪����Č������܂���");
	}else{
		SendStatusMessage("��������܂���ł���");
		if(m_pShareData->m_Common.m_bNOTIFYNOTFOUND)	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
			::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
				"���(��) �Ƀu�b�N�}�[�N��������܂���B" );
	}
	return;
}



//! �O�̃u�b�N�}�[�N��T���C����������ړ�����D
void CEditView::Command_BOOKMARK_PREV(void)
{
//	CDocLine*	pCDocLine;
	int			nX=0;
	int			nY;
//	int			nRet;
	int			nYOld;				// hor
	BOOL		bFound	=	FALSE;	// hor
	BOOL		bRedo	=	TRUE;	// hor
	nY=m_nCaretPosY_PHY;
	nYOld=nY;						// hor
re_do:;								// hor
	if(m_pcEditDoc->m_cDocLineMgr.SearchBookMark(nY, 0 /* �O������ */, &nY)){
		bFound = TRUE;				// hor
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(nX,nY,&nX,&nY);
		//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
		MoveCursorSelecting( nX, nY, m_bSelectingLock );
	}
    // 2002.01.26 hor
	if(m_pShareData->m_Common.m_bSearchAll){
		if(!bFound	&&	// ������Ȃ�����
			bRedo		// �ŏ��̌���
		){
			nY=m_pcEditDoc->m_cLayoutMgr.GetLineCount();	//	2002/06/01 MIK
			bRedo=FALSE;
			goto re_do;	// ��������Č���
		}
	}
	if(bFound){
		if(nYOld <= nY)SendStatusMessage("����������Č������܂���");
	}else{
		SendStatusMessage("��������܂���ł���");
		if(m_pShareData->m_Common.m_bNOTIFYNOTFOUND)	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
			::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
				"�O��(��) �Ƀu�b�N�}�[�N��������܂���B" );
	}
	return;
}



//! �u�b�N�}�[�N���N���A����
void CEditView::Command_BOOKMARK_RESET(void)
{
	m_pcEditDoc->m_cDocLineMgr.ResetAllBookMark();
	// 2002.01.16 hor ���������r���[���X�V
	for( int v = 0; v < 4; ++v ) if( m_pcEditDoc->m_nActivePaneIndex != v )m_pcEditDoc->m_cEditViewArr[v].Redraw();
	Redraw();
	return;
}


//�w��p�^�[���Ɉ�v����s���}�[�N 2002.01.16 hor
//�L�[�}�N���ŋL�^�ł���悤��	2002.02.08 hor
void CEditView::Command_BOOKMARK_PATTERN( void )
{
	//����or�u���_�C�A���O����Ăяo���ꂽ
	if(!ChangeCurRegexp())return;
	m_pcEditDoc->m_cDocLineMgr.MarkSearchWord(
		m_pShareData->m_szSEARCHKEYArr[0],		/* �������� */
		m_pShareData->m_Common.m_bRegularExp,	/* 1==���K�\�� */
		m_pShareData->m_Common.m_bLoHiCase,		/* 1==�p�啶���������̋�� */
		m_pShareData->m_Common.m_bWordOnly,		/* 1==�P��̂݌��� */
		&m_CurRegexp							/* ���K�\���R���p�C���f�[�^ */
	);
	// 2002.01.16 hor ���������r���[���X�V
	for( int v = 0; v < 4; ++v ) if( m_pcEditDoc->m_nActivePaneIndex != v )m_pcEditDoc->m_cEditViewArr[v].Redraw();
	Redraw();
	return;
}



/*! TRIM Step1

	��I�����̓J�����g�s��I������ ConvSelectedArea �� ConvMemory ��
	
	@param bLeft [in] FALSE: �ETRIM / ����ȊO: ��TRIM
	@author hor
	@date 2001.12.03 hor �V�K�쐬
*/
void CEditView::Command_TRIM( BOOL bLeft )
{
	bool bBeDisableSelectArea = false;
	if(!IsTextSelected()){	//	��I�����͍s�I���ɕύX
		m_nSelectLineFrom = m_nCaretPosY;
		m_nSelectColmFrom = 0;
		m_nSelectLineTo   = m_nCaretPosY; 
		//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
		m_nSelectColmTo   = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();
		bBeDisableSelectArea = true;
	}
	if(bLeft){
		ConvSelectedArea( F_LTRIM );
	}else{
		ConvSelectedArea( F_RTRIM );
	}
	if (bBeDisableSelectArea) DisableSelectArea( TRUE );
	return;
}

/*! TRIM Step2
	ConvMemory ���� �߂��Ă�����̏����D
	CMemory.cpp�̂Ȃ��ɒu���Ȃ��ق����ǂ����ȂƎv���Ă�����ɒu���܂����D
	
	@author hor
	@date 2001.12.03 hor �V�K�쐬
*/
void CEditView::Command_TRIM2( CMemory* pCMemory , BOOL bLeft )
{
	const char*	pLine;
	int			nLineLen;
	char*		pDes;
	int			nBgn;
	int			i,j;
	int			nPosDes;
	CEOL		cEol;
	int			nCharChars;

	nBgn = 0;
	nPosDes = 0;
	/* �ϊ���ɕK�v�ȃo�C�g���𒲂ׂ� */
	while( NULL != ( pLine = GetNextLine( pCMemory->GetPtr(), pCMemory->GetLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory�ύX
		if( 0 < nLineLen ){
			nPosDes += nLineLen;
		}
		nPosDes += cEol.GetLen();
	}
	if( 0 >= nPosDes ){
		return;
	}
	pDes = new char[nPosDes + 1];
	nBgn = 0;
	nPosDes = 0;
	if( bLeft ){
	// LTRIM
		while( NULL != ( pLine = GetNextLine( pCMemory->GetPtr(), pCMemory->GetLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory�ύX
			if( 0 < nLineLen ){
				for( i = 0; i <= nLineLen; ++i ){
					if( pLine[i] ==' ' ||
						pLine[i] =='\t'){
						continue;
					}else if( (unsigned char)pLine[i] == (unsigned char)0x81 && (unsigned char)pLine[i + 1] == (unsigned char)0x40 ){
						++i;
						continue;
					}else{
						break;
					}
				}
				if(nLineLen-i>0){
					memcpy( &pDes[nPosDes], (const char *)&pLine[i], nLineLen );
					nPosDes+=nLineLen-i;
				}
			}
			memcpy( &pDes[nPosDes], cEol.GetValue(), cEol.GetLen() );
			nPosDes += cEol.GetLen();
		}
	}else{
	// RTRIM
		while( NULL != ( pLine = GetNextLine( pCMemory->GetPtr(), pCMemory->GetLength(), &nLineLen, &nBgn, &cEol ) ) ){ // 2002/2/10 aroka CMemory�ύX
			if( 0 < nLineLen ){
				// 2005.10.11 ryoji �E����k��̂ł͂Ȃ�������T���悤�ɏC���i"��@" �̉E�Q�o�C�g���S�p�󔒂Ɣ��肳�����̑Ώ��j
				i = j = 0;
				while( i < nLineLen ){
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
					if( 1 == nCharChars ){
						if( TAB != pLine[i] && SPACE != pLine[i])
							j = i + nCharChars;
					}
					else if( 2 == nCharChars ){
						if( !((unsigned char)pLine[i] == (unsigned char)0x81 && (unsigned char)pLine[i + 1] == (unsigned char)0x40) )
							j = i + nCharChars;
					}
					i += nCharChars;
				}
				if(j>0){
					memcpy( &pDes[nPosDes], (const char *)&pLine[0], j );
					nPosDes+=j;
				}
			}
			memcpy( &pDes[nPosDes], cEol.GetValue(), cEol.GetLen() );
			nPosDes += cEol.GetLen();
		}
	}
	pDes[nPosDes] = '\0';

	pCMemory->SetData( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return;
}

/*!	�����s�̃\�[�g�Ɏg���\����*/
typedef struct _SORTTABLE {
	string sKey1;
	string sKey2;
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
void CEditView::Command_SORT(BOOL bAsc)	//bAsc:TRUE=����,FALSE=�~��
{
	int			nLFO , nSelectLineFromOld;	/* �͈͑I���J�n�s */
	int			nCFO , nSelectColFromOld ; 	/* �͈͑I���J�n�� */
	int			nLTO , nSelectLineToOld  ;	/* �͈͑I���I���s */
	int			nCTO , nSelectColToOld   ;	/* �͈͑I���I���� */
	int			nColmFrom,nColmTo;
	int			nCF,nCT;
	int			nCaretPosYOLD;
	BOOL		bBeginBoxSelectOld;
	const char*	pLine;
	int			nLineLen;
	int			i,j;
	CMemory		cmemBuf;
	std::vector<SORTTABLE> sta;
	COpe*		pcOpe = NULL;

	if( !IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
		return;
	}

	if( m_bBeginBoxSelect ){
	// 2001.12.21 hor �ԈႢ
	//	if( m_nSelectLineTo >= m_pcEditDoc->m_cLayoutMgr.GetLineCount()-1 ) {
	//		--m_nSelectLineTo;
	//	}
		nLFO = m_nSelectLineFrom;
		nCFO = m_nSelectColmFrom;
		nLTO = m_nSelectLineTo;
		nCTO = m_nSelectColmTo;
		if( m_nSelectColmFrom==m_nSelectColmTo ){
			//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
			m_nSelectColmTo=m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();
		}
		if(m_nSelectColmFrom<m_nSelectColmTo){
			nCF=m_nSelectColmFrom;
			nCT=m_nSelectColmTo;
		}else{
			nCF=m_nSelectColmTo;
			nCT=m_nSelectColmFrom;
		}
	}
	bBeginBoxSelectOld=m_bBeginBoxSelect;
	nCaretPosYOLD=m_nCaretPosY;
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nSelectColmFrom,m_nSelectLineFrom,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nSelectColmTo,m_nSelectLineTo,
		&nSelectColToOld,&nSelectLineToOld
	);
	if( bBeginBoxSelectOld ){
		++nSelectLineToOld;
	}else{
		// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
		// ���̍s���I��͈͂ɉ�����
		if ( nSelectColToOld > 0 ) {
			// 2006.03.31 Moca nSelectLineToOld�́A�����s�Ȃ̂�Layout�n����DocLine�n�ɏC��
			const CDocLine* pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( nSelectLineToOld );
			if( NULL != pcDocLine && EOL_NONE != pcDocLine->m_cEol ){
				++nSelectLineToOld;
			}
		}
	}
	nSelectColFromOld = 0;
	nSelectColToOld = 0;

	//�s�I������ĂȂ�
	if(nSelectLineFromOld==nSelectLineToOld){
		return;
	}

	for( i = nSelectLineFromOld; i < nSelectLineToOld; i++ ){
		const CDocLine* pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( i );
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( i, &nLineLen );
		if( NULL == pLine ) continue;
		SORTTABLE pst = new SORTDATA;
		if( bBeginBoxSelectOld ){
			nColmFrom = LineColmnToIndex( pcDocLine, nCF );
			nColmTo   = LineColmnToIndex( pcDocLine, nCT );
			if(nColmTo<nLineLen){	// BOX�I��͈͂̉E�[���s���Ɏ��܂��Ă���ꍇ
				// 2006.03.31 genta std::string::assign���g���Ĉꎞ�ϐ��폜
				pst->sKey1.assign( &pLine[nColmFrom], nColmTo-nColmFrom );
			}else
			if(nColmFrom<nLineLen){	// BOX�I��͈͂̉E�[���s�����E�ɂ͂ݏo���Ă���ꍇ
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
	cmemBuf.SetDataSz( "" );
	j=(int)sta.size();
	if( bBeginBoxSelectOld ){
		for (i=0; i<j; i++) cmemBuf.AppendSz( sta[i]->sKey2.c_str() ); 
	}else{
		for (i=0; i<j; i++) cmemBuf.AppendSz( sta[i]->sKey1.c_str() );
	}
	//sta.clear(); �����ꂶ�Ⴞ�߂݂���
	for (i=0; i<j; i++) delete sta[i];
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		nSelectColFromOld,nSelectLineFromOld,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		nSelectColToOld,nSelectLineToOld,
		&nSelectColToOld,&nSelectLineToOld
	);
	ReplaceData_CEditView(
		nSelectLineFromOld,
		nSelectColFromOld,
		nSelectLineToOld,
		nSelectColToOld,
		NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
		cmemBuf.GetPtr(),
		cmemBuf.GetLength(),
		FALSE
	);
	//	�I���G���A�̕���
	if(bBeginBoxSelectOld){
		m_bBeginBoxSelect=bBeginBoxSelectOld;
		m_nSelectLineFrom=nLFO;
		m_nSelectColmFrom=nCFO;
		m_nSelectLineTo  =nLTO;
		m_nSelectColmTo  =nCTO;
	}else{
		m_nSelectLineFrom=nSelectLineFromOld;
		m_nSelectColmFrom=nSelectColFromOld;
		m_nSelectLineTo  =nSelectLineToOld;
		m_nSelectColmTo  =nSelectColToOld;
	}
	if(nCaretPosYOLD==m_nSelectLineFrom || m_bBeginBoxSelect ) {
		MoveCursor( m_nSelectColmFrom, m_nSelectLineFrom, TRUE );
	}else{
		MoveCursor( m_nSelectColmTo, m_nSelectLineTo, TRUE );
	}
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* ����O�̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	RedrawAll();
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
void CEditView::Command_MERGE(void)
{
	int			nSelectLineFromOld;	/* �͈͑I���J�n�s */
	int			nSelectColFromOld ; /* �͈͑I���J�n�� */
	int			nSelectLineToOld  ;	/* �͈͑I���I���s */
	int			nSelectColToOld   ;	/* �͈͑I���I���� */
	int			nCaretPosYOLD;
	const char*	pLine;
	const char*	pLinew;
	int			nLineLen;
	int			i,j;
	CMemory		cmemBuf;
	COpe*		pcOpe = NULL;

	if( !IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
		return;
	}
	if( m_bBeginBoxSelect ){
		return;
	}

	nCaretPosYOLD=m_nCaretPosY;
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nSelectColmFrom,m_nSelectLineFrom,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nSelectColmTo,m_nSelectLineTo,
		&nSelectColToOld,&nSelectLineToOld
	);

	// 2001.12.21 hor
	// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
	// ���̍s���I��͈͂ɉ�����
	if ( nSelectColToOld > 0 ) {
		const CLayout* pcLayout=m_pcEditDoc->m_cLayoutMgr.Search(nSelectLineToOld);
		if( NULL != pcLayout && EOL_NONE != pcLayout->m_cEol ){
			++nSelectLineToOld;
		}
	}

	nSelectColFromOld = 0;
	nSelectColToOld = 0;

	//�s�I������ĂȂ�
	if(nSelectLineFromOld==nSelectLineToOld){
		return;
	}

	pLinew=NULL;
	cmemBuf.SetDataSz( "" );
	for( i = nSelectLineFromOld; i < nSelectLineToOld; i++ ){
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( i, &nLineLen );
		if( NULL == pLine ) continue;
		if( NULL == pLinew || strcmp(pLine,pLinew) ){
			cmemBuf.AppendSz( pLine );
		}
		pLinew=pLine;
	}
	j=m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		nSelectColFromOld,nSelectLineFromOld,
		&nSelectColFromOld,&nSelectLineFromOld
	);
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		nSelectColToOld,nSelectLineToOld,
		&nSelectColToOld,&nSelectLineToOld
	);
	ReplaceData_CEditView(
		nSelectLineFromOld,
		nSelectColFromOld,
		nSelectLineToOld,
		nSelectColToOld,
		NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
		cmemBuf.GetPtr(),
		cmemBuf.GetLength(),
		FALSE
	);
	j-=m_pcEditDoc->m_cDocLineMgr.GetLineCount();

	//	�I���G���A�̕���
	m_nSelectLineFrom=nSelectLineFromOld;
	m_nSelectColmFrom=nSelectColFromOld;
	m_nSelectLineTo  =nSelectLineToOld-j;
	m_nSelectColmTo  =nSelectColToOld;
	if(nCaretPosYOLD==m_nSelectLineFrom){
		MoveCursor( m_nSelectColmFrom, m_nSelectLineFrom, TRUE );
	}else{
		MoveCursor( m_nSelectColmTo, m_nSelectLineTo, TRUE );
	}
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* ����O�̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	RedrawAll();

	if(j){
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_TOPMOST, GSTR_APPNAME,
			"%d�s���}�[�W���܂����B", j
		);
	}else{
		::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
			"�}�[�W�\�ȍs���݂���܂���ł����B" );
	}
}



// To Here 2001.12.03 hor
	
/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09

	@date 2002.04.11 YAZAKI COsVersionInfo�̃J�v�Z���������܂��傤�B
*/
void CEditView::Command_Reconvert(void)
{
	int		nSize;
	PRECONVERTSTRING	pReconv;
	COsVersionInfo cOs;
//	POSVERSIONINFO	pOsVer;
	bool	bUseUnicodeATOK;
	HIMC hIMC ;
	
	
	//�T�C�Y���擾
	nSize = SetReconvertStruct(NULL,false);
	
	if( 0 == nSize )  // �T�C�Y�O�̎��͉������Ȃ�
		return ;
	
	bUseUnicodeATOK = false;
	//�o�[�W�����`�F�b�N
//	pOsVer  = cOs.GetOsVersionInfo();
	if( cOs.OsDoesNOTSupportReconvert() ){
		
		// MSIME���ǂ���
		HWND hWnd = ImmGetDefaultIMEWnd(m_hWnd);
		if (SendMessage(hWnd, m_uWM_MSIME_RECONVERTREQUEST, FID_RECONVERT_VERSION, 0)){
			SendMessage(hWnd, m_uWM_MSIME_RECONVERTREQUEST, 0, (LPARAM)m_hWnd);
			return ;
		}
		// ATOK���g���邩�ǂ���
		
		//�����̎擾
		char sz[256];
		
		ImmGetDescription(GetKeyboardLayout(0),sz,256);
		if ( (strncmp(sz,"ATOK",4) == 0) && (NULL != AT_ImmSetReconvertString) ){
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
	}
	
	//IME�̃R���e�L�X�g�擾
	hIMC = ::ImmGetContext( m_hWnd );
	
	//�̈�m��
	pReconv = (PRECONVERTSTRING)::HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, nSize);
	
	//�\���̐ݒ�
	SetReconvertStruct( pReconv, bUseUnicodeATOK);
	
	//�ϊ��͈͂̒���
	if(bUseUnicodeATOK){
		(*AT_ImmSetReconvertString)(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize, NULL,0);
	}
	//���������ϊ��͈͂�I������
	SetSelectionFromReonvert(pReconv, bUseUnicodeATOK);
	
	//�ĕϊ����s
	if(bUseUnicodeATOK){
		(*AT_ImmSetReconvertString)(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize, NULL,0);
	}
	//�̈���
	::HeapFree(GetProcessHeap(),0,(LPVOID)pReconv);
	::ImmReleaseContext( m_hWnd, hIMC);
}

/*!	�R���g���[���R�[�h�̓���(�_�C�A���O)
	@author	MIK
	@date	2002/06/02
*/
void CEditView::Command_CtrlCode_Dialog( void )
{
	CDlgCtrlCode	cDlgCtrlCode;

	//�R���g���[���R�[�h���̓_�C�A���O��\������
	if( FALSE == cDlgCtrlCode.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc ) )
	{
		return;
	}

	//�R���g���[���R�[�h����͂���
	Command_CHAR( cDlgCtrlCode.m_nCode );

	return;
}

/*!	�����J�n�ʒu�֖߂�
	@author	ai
	@date	02/06/26
*/
void CEditView::Command_JUMP_SRCHSTARTPOS(void)
{
	if( 0 <= m_nSrchStartPosX_PHY && 0 <= m_nSrchStartPosY_PHY )
	{
		int x, y;
		/* �͈͑I�𒆂� */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
			m_nSrchStartPosX_PHY,
			m_nSrchStartPosY_PHY,
			&x, &y );
		//	2006.07.09 genta �I����Ԃ�ۂ�
		MoveCursorSelecting( x, y, m_bSelectingLock );
	}
	else
	{
//		SendStatusMessage( "�����J�n�ʒu������܂���" );
		::MessageBeep( MB_ICONHAND );
	}
	return;
}

/*!	���C�ɓ���̐ݒ�(�_�C�A���O)
	@author	MIK
	@date	2003/04/07
*/
void CEditView::Command_Favorite( void )
{
	CDlgFavorite	cDlgFavorite;

	//�_�C�A���O��\������
	if( FALSE == cDlgFavorite.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc ) )
	{
		return;
	}

	return;
}

/*! ���͂�����s�R�[�h��ݒ�

	@author moca
	@date 2003.06.23 �V�K�쐬
*/
void CEditView::Command_CHGMOD_EOL( enumEOLType e ){
	if( EOL_NONE < e && e < EOL_CODEMAX  ){
		m_pcEditDoc->SetNewLineCode( e );
		// �X�e�[�^�X�o�[���X�V���邽��
		// �L�����b�g�̍s���ʒu��\������֐����Ăяo��
		DrawCaretPosInfo();
	}
}

/*! ��Ɏ�O�ɕ\��
	@date 2004.09.21 Moca
*/
void CEditView::Command_WINTOPMOST( LPARAM lparam )
{
	m_pcEditDoc->m_pcEditWnd->WindowTopMost( int(lparam) );
}

/*!	@brief ���p���̐ݒ�
	@date Jan. 29, 2005 genta �V�K�쐬
*/
void CEditView::Command_SET_QUOTESTRING( const char* quotestr )
{
	if( quotestr == NULL )
		return;

	strncpy( m_pShareData->m_Common.m_szInyouKigou, quotestr,
		sizeof( m_pShareData->m_Common.m_szInyouKigou ));
	
	m_pShareData->m_Common.m_szInyouKigou[ sizeof( m_pShareData->m_Common.m_szInyouKigou ) - 1 ] = '\0';
}

/*!	@brief �E�B���h�E�ꗗ�|�b�v�A�b�v�\�������i�t�@�C�����̂݁j
	@date  2006.03.23 fon �V�K�쐬
	@date  2006.05.19 genta �R�}���h���s�v����\�������ǉ�
*/
void CEditView::Command_WINLIST( int nCommandFrom )
{
	CEditWnd	*pCEditWnd;
	pCEditWnd = m_pcEditDoc->m_pcEditWnd;

	//�E�B���h�E�ꗗ���|�b�v�A�b�v�\������
	pCEditWnd->PopupWinList( nCommandFrom != 1 );	// 2007.02.27 ryoji �A�N�Z�����[�^�L�[����łȂ���΃}�E�X�ʒu��

}


/*!	@brief �}�N���p�A�E�g�v�b�g�E�C���h�E�ɕ\��
	@date 2006.04.26 maru �V�K�쐬
*/
void CEditView::Command_TRACEOUT( const char* outputstr, int nFlgOpt )
{
	if( outputstr == NULL )
		return;

	// 0x01 ExpandParameter�ɂ�镶����W�J�L��
	if (nFlgOpt & 0x01) {
		char Buffer[2048];
		m_pcEditDoc->ExpandParameter(outputstr, Buffer, 2047);
		CShareData::getInstance()->TraceOut( "%s", Buffer );
	} else {
		CShareData::getInstance()->TraceOut( "%s", outputstr );
	}

	// 0x02 ���s�R�[�h�̗L��
	if ((nFlgOpt & 0x02) == 0) CShareData::getInstance()->TraceOut( "\r\n" );

}

/*[EOF]*/
