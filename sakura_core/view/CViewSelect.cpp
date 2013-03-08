#include "StdAfx.h"
#include "CViewSelect.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "mem/CMemoryIterator.h"
#include "doc/CLayout.h"
#include "window/CEditWnd.h"
#include "charset/CCodeBase.h"
#include "charset/CCodeFactory.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "types/CTypeSupport.h"
#include <limits.h>

CViewSelect::CViewSelect(CEditView* pcEditView)
: m_pcEditView(pcEditView)
{
	m_bSelectingLock   = false;	// �I����Ԃ̃��b�N
	m_bBeginSelect     = false;		// �͈͑I��
	m_bBeginBoxSelect  = false;	// ��`�͈͑I��
	m_bBeginLineSelect = false;	// �s�P�ʑI��
	m_bBeginWordSelect = false;	// �P��P�ʑI��

	m_sSelectBgn.Clear(-1); // �͈͑I��(���_)
	m_sSelect   .Clear(-1); // �͈͑I��
	m_sSelectOld.Clear(0);  // �͈͑I��(Old)
	m_bSelectAreaChanging = false;	// �I��͈͕ύX��
	m_nLastSelectedByteLen = 0;	// �O��I�����̑I���o�C�g��
}

void CViewSelect::CopySelectStatus(CViewSelect* pSelect) const
{
	pSelect->m_bSelectingLock		= m_bSelectingLock;		/* �I����Ԃ̃��b�N */
	pSelect->m_bBeginSelect			= m_bBeginSelect;		/* �͈͑I�� */
	pSelect->m_bBeginBoxSelect		= m_bBeginBoxSelect;	/* ��`�͈͑I�� */

	pSelect->m_sSelectBgn			= m_sSelectBgn;			//�͈͑I��(���_)
	pSelect->m_sSelect				= m_sSelect;			//�͈͑I��
	pSelect->m_sSelectOld			= m_sSelectOld;			//�͈͑I��

	pSelect->m_ptMouseRollPosOld	= m_ptMouseRollPosOld;	// �}�E�X�͈͑I��O��ʒu(XY���W)
}

//! ���݂̃J�[�\���ʒu����I�����J�n����
void CViewSelect::BeginSelectArea( const CLayoutPoint* po )
{
	const CEditView* pView=GetEditView();

	if( NULL == po ){
		po = &(pView->GetCaret().GetCaretLayoutPos());
	}
	m_sSelectBgn.Set(*po); //�͈͑I��(���_)
	m_sSelect.   Set(*po); //�͈͑I��
}


// ���݂̑I��͈͂��I����Ԃɖ߂�
void CViewSelect::DisableSelectArea( bool bDraw, bool bDrawBracketCursorLine )
{
	const CEditView* pView=GetEditView();
	CEditView* pView2=GetEditView();

	m_sSelectOld = m_sSelect;		//�͈͑I��(Old)
	m_sSelect.Clear(-1);
	m_bSelectingLock	 = false;	// �I����Ԃ̃��b�N

	if( bDraw ){
		DrawSelectArea( bDrawBracketCursorLine );
	}
	m_bDrawSelectArea = false;	// 02/12/13 ai // 2011.12.24 bDraw���ʓ�����ړ�

	m_sSelectOld.Clear(0);			// �͈͑I��(Old)
	m_bBeginBoxSelect = false;		// ��`�͈͑I��
	m_bBeginLineSelect = false;		// �s�P�ʑI��
	m_bBeginWordSelect = false;		// �P��P�ʑI��
	m_nLastSelectedByteLen = 0;		// �O��I�����̑I���o�C�g��

	// 2002.02.16 hor ���O�̃J�[�\���ʒu�����Z�b�g
	pView2->GetCaret().m_nCaretPosX_Prev=pView->GetCaret().GetCaretLayoutPos().GetX();

}



// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
void CViewSelect::ChangeSelectAreaByCurrentCursor( const CLayoutPoint& ptCaretPos )
{
	m_sSelectOld=m_sSelect; // �͈͑I��(Old)

	//	2002/04/08 YAZAKI �R�[�h�̏d����r��
	ChangeSelectAreaByCurrentCursorTEST(
		ptCaretPos,
		&m_sSelect
	);

	// �I��̈�̕`��
	m_bSelectAreaChanging = true;
	DrawSelectArea(true);
	m_bSelectAreaChanging = false;
}


// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX(�e�X�g�̂�)
void CViewSelect::ChangeSelectAreaByCurrentCursorTEST(
	const CLayoutPoint& ptCaretPos,
	CLayoutRange* pSelect
)
{
	const CEditView* pView=GetEditView();
	CEditView* pView2=GetEditView();

	if(m_sSelectBgn.GetFrom()==m_sSelectBgn.GetTo()){
		if( ptCaretPos==m_sSelectBgn.GetFrom() ){
			// �I������
			pSelect->Clear(-1);
			m_nLastSelectedByteLen = 0;		// �O��I�����̑I���o�C�g��
		}
		else if( PointCompare(ptCaretPos,m_sSelectBgn.GetFrom() ) < 0 ){ //�L�����b�g�ʒu��m_sSelectBgn��from��菬����������
			 pSelect->SetFrom(ptCaretPos);
			 pSelect->SetTo(m_sSelectBgn.GetFrom());
		}
		else{
			pSelect->SetFrom(m_sSelectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}
	else{
		// �펞�I��͈͈͓͂̔�
		// �L�����b�g�ʒu�� m_sSelectBgn �� from�ȏ�ŁAto��菬�����ꍇ
		if( PointCompare(ptCaretPos,m_sSelectBgn.GetFrom()) >= 0 && PointCompare(ptCaretPos,m_sSelectBgn.GetTo()) < 0 ){
			pSelect->SetFrom(m_sSelectBgn.GetFrom());
			if ( ptCaretPos==m_sSelectBgn.GetFrom() ){
				pSelect->SetTo(m_sSelectBgn.GetTo());
			}
			else {
				pSelect->SetTo(ptCaretPos);
			}
		}
		//�L�����b�g�ʒu��m_sSelectBgn��from��菬����������
		else if( PointCompare(ptCaretPos,m_sSelectBgn.GetFrom()) < 0 ){
			// �펞�I��͈͂̑O����
			pSelect->SetFrom(ptCaretPos);
			pSelect->SetTo(m_sSelectBgn.GetTo());
		}
		else{
			// �펞�I��͈͂̌�����
			pSelect->SetFrom(m_sSelectBgn.GetFrom());
			pSelect->SetTo(ptCaretPos);
		}
	}
}



/*! �I��̈�̕`��

	@date 2006.10.01 Moca �d���R�[�h�폜�D��`�����P�D
	@date 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
		��ʃo�b�t�@���L�����A��ʂƌ݊�BMP�̗����̔��]�������s���B
*/
void CViewSelect::DrawSelectArea(bool bDrawBracketCursorLine)
{
	CEditView* pView=GetEditView();

	if( !pView->GetDrawSwitch() ){
		return;
	}
	m_bDrawSelectArea = true;
	
	bool bDispText = CTypeSupport(pView,COLORIDX_SELECT).IsDisp();
	if( bDispText ){
		if( m_sSelect != m_sSelectOld ){
			// �I��F�\���̎��́AWM_PAINT�o�R�ō��
			const int nCharWidth = pView->GetTextMetrics().GetHankakuDx();
			const int nCharHeight = pView->GetTextMetrics().GetHankakuDy();
			const CTextArea& area =  pView->GetTextArea();
			CLayoutRect rcOld; // CLayoutRect
			TwoPointToRect( &rcOld, m_sSelectOld.GetFrom(), m_sSelectOld.GetTo() );
			CLayoutRect rcNew; // CLayoutRect
			TwoPointToRect( &rcNew, m_sSelect.GetFrom(), m_sSelect.GetTo() );
			CLayoutRect rc; // CLayoutRect ������top,bottom�����g��
			CLayoutInt drawLeft = CLayoutInt(0);
			CLayoutInt drawRight = CLayoutInt(-1);
			if( !m_sSelect.IsValid() ){
				rc.top    = rcOld.top;
				rc.bottom = rcOld.bottom;
			}else if( !m_sSelectOld.IsValid() ){
				rc.top    = rcNew.top;
				rc.bottom = rcNew.bottom;
			}else if(IsBoxSelecting() && 
				(m_sSelect.GetTo().x != m_sSelectOld.GetTo().x || m_sSelect.GetFrom().x != m_sSelectOld.GetFrom().x)){
				rc.UnionStrictRect(rcOld, rcNew);
			}else if(!IsBoxSelecting() && rcOld.top == rcNew.top && rcOld.bottom == rcNew.bottom){
				if(m_sSelect.GetFrom() == m_sSelectOld.GetFrom() && m_sSelect.GetTo().x != m_sSelectOld.GetTo().x){
					// GetTo�̍s���Ώ�
					rc.top = rc.bottom = m_sSelect.GetTo().GetY2();
					drawLeft  = t_min(m_sSelect.GetTo().x, m_sSelectOld.GetTo().x);
					drawRight = t_max(m_sSelect.GetTo().x, m_sSelectOld.GetTo().x) + 1;
				}else if(m_sSelect.GetTo() == m_sSelectOld.GetTo() && m_sSelect.GetFrom().x != m_sSelectOld.GetFrom().x){
					// GetFrom�̍s���Ώ�
					rc.top = rc.bottom = m_sSelect.GetFrom().GetY2();
					drawLeft  = t_min(m_sSelectOld.GetFrom().x, m_sSelect.GetFrom().x);
					drawRight = t_max(m_sSelectOld.GetFrom().x, m_sSelect.GetFrom().x) + 1;
				}else{
					rc.UnionStrictRect(rcOld, rcNew);
				}
			}else if(rcOld.top == rcNew.top){
				rc.top    = t_min(rcOld.bottom, rcNew.bottom);
				rc.bottom = t_max(rcOld.bottom, rcNew.bottom);
			}else if(rcOld.bottom == rcNew.bottom){
				rc.top    = t_min(rcOld.top, rcNew.top);
				rc.bottom = t_max(rcOld.top, rcNew.top);
			}else{
				rc.UnionStrictRect(rcOld, rcNew);
			}
			CMyRect rcPx;
			if( pView->IsBkBitmap() ||  drawRight == -1){
				// �w�i�\���̃N���b�s���O���Â��̂ō��E���w�肵�Ȃ�
				rcPx.left   =  0;
				rcPx.right  = SHRT_MAX; 
			}else{
				rcPx.left   =  area.GetAreaLeft() + nCharWidth * (Int)(drawLeft - area.GetViewLeftCol());
				rcPx.right  = area.GetAreaLeft() + nCharWidth * (Int)(drawRight- area.GetViewLeftCol());
			}
			rcPx.top    = area.GetAreaTop() + nCharHeight * (Int)(rc.top -area.GetViewTopLine());
			rcPx.bottom = area.GetAreaTop() + nCharHeight * (Int)(rc.bottom + 1 -area.GetViewTopLine());

			CMyRect rcArea;
			pView->GetTextArea().GenerateTextAreaRect(&rcArea);
			RECT rcUpdate;
			CEditView& view = *pView;
			if( ::IntersectRect(&rcUpdate, &rcPx, &rcArea) ){
				HDC hdc = view.GetDC();
				PAINTSTRUCT ps;
				ps.rcPaint = rcUpdate;
				// DrawSelectAreaLine�ł̉���OFF�̑���
				view.GetCaret().m_cUnderLine.CaretUnderLineOFF(true, false);
				view.GetCaret().m_cUnderLine.Lock();
				view.OnPaint(hdc, &ps, false);
				view.GetCaret().m_cUnderLine.UnLock();
				view.ReleaseDC( hdc );
			}
			// 2010.10.10 0���I��(����)��Ԃł́A�J�[�\���ʒu���C�����A(���[�W�����O)
			if( bDrawBracketCursorLine ){
				view.GetCaret().m_cUnderLine.CaretUnderLineON(true, false);
			}
		}
	}else{
		HDC hdc = pView->GetDC();
		DrawSelectArea2( hdc );
		// 2011.12.02 �I��������Ԃł́A�J�[�\���ʒu���C�����A
		if( bDrawBracketCursorLine ){
			pView->GetCaret().m_cUnderLine.CaretUnderLineON(true, false);
		}
		pView->ReleaseDC( hdc );
	}

	// 2011.12.02 �I��������ԂɂȂ�ƑΊ��ʋ������ł��Ȃ��Ȃ�o�O�΍�
	if( !IsTextSelecting() ){
		// �������I�����b�N���͂����ł͋����\������Ȃ�
		m_bDrawSelectArea = false;
		if( bDrawBracketCursorLine ){
			pView->SetBracketPairPos( true );
			pView->DrawBracketPair( true );
		}
	}

	//	Jul. 9, 2005 genta �I��̈�̏���\��
	PrintSelectionInfoMsg();
}

/*!
	���]�p�č�揈���{��
*/
void CViewSelect::DrawSelectArea2( HDC hdc ) const
{
	CEditView const * const pView = GetEditView();

	// 2006.10.01 Moca �d���R�[�h����
	HBRUSH      hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
	HBRUSH      hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
	int         nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	HBRUSH		hBrushCompatOld;
	int			nROPCompatOld;
	bool bCompatBMP = pView->m_hbmpCompatBMP && hdc != pView->m_hdcCompatDC;
	if( bCompatBMP ){
		hBrushCompatOld = (HBRUSH)::SelectObject( pView->m_hdcCompatDC, hBrush );
		nROPCompatOld = ::SetROP2( pView->m_hdcCompatDC, SELECTEDAREA_ROP2 );
	}
	// To Here 2007.09.09 Moca

//	MYTRACE_A( "DrawSelectArea()  m_bBeginBoxSelect=%hs\n", m_bBeginBoxSelect?"TRUE":"FALSE" );
	if( IsBoxSelecting() ){		// ��`�͈͑I��
		// 2001.12.21 hor ��`�G���A��EOF������ꍇ�ARGN_XOR�Ō��������
		// EOF�ȍ~�̃G���A�����]���Ă��܂��̂ŁA���̏ꍇ��Redraw���g��
		// 2002.02.16 hor �������}�~���邽��EOF�ȍ~�̃G���A�����]�����������x���]���Č��ɖ߂����Ƃɂ���
		//if((GetTextArea().GetViewTopLine()+m_nViewRowNum+1>=m_pcEditDoc->m_cLayoutMgr.GetLineCount()) &&
		//   (m_sSelect.GetTo().y+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ||
		//	m_sSelectOld.GetTo().y+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) ) {
		//	Redraw();
		//	return;
		//}

		const int nCharWidth = pView->GetTextMetrics().GetHankakuDx();
		const int nCharHeight = pView->GetTextMetrics().GetHankakuDy();


		// 2�_��Ίp�Ƃ����`�����߂�
		CLayoutRect  rcOld;
		TwoPointToRect(
			&rcOld,
			m_sSelectOld.GetFrom(),	// �͈͑I���J�n
			m_sSelectOld.GetTo()	// �͈͑I���I��
		);
		rcOld.left   = t_max(rcOld.left  , pView->GetTextArea().GetViewLeftCol()  );
		rcOld.right  = t_max(rcOld.right , pView->GetTextArea().GetViewLeftCol()  );
		rcOld.right  = t_min(rcOld.right , pView->GetTextArea().GetRightCol() + 1 );
		rcOld.top    = t_max(rcOld.top   , pView->GetTextArea().GetViewTopLine()  );
		rcOld.bottom = t_max(rcOld.bottom, pView->GetTextArea().GetViewTopLine() - 1);	// 2010.11.02 ryoji �ǉ��i��ʏ�[������ɂ����`�I������������ƃ��[���[�����]�\���ɂȂ���̏C���j
		rcOld.bottom = t_min(rcOld.bottom, pView->GetTextArea().GetBottomLine()   );

		RECT rcOld2;
		rcOld2.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcOld.left  * nCharWidth;
		rcOld2.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcOld.right * nCharWidth;
		rcOld2.top		= (Int)( rcOld.top - pView->GetTextArea().GetViewTopLine() ) * nCharHeight + pView->GetTextArea().GetAreaTop();
		rcOld2.bottom	= (Int)( rcOld.bottom + 1 - pView->GetTextArea().GetViewTopLine() ) * nCharHeight + pView->GetTextArea().GetAreaTop();
		HRGN hrgnOld = ::CreateRectRgnIndirect( &rcOld2 );

		// 2�_��Ίp�Ƃ����`�����߂�
		CLayoutRect  rcNew;
		TwoPointToRect(
			&rcNew,
			m_sSelect.GetFrom(),	// �͈͑I���J�n
			m_sSelect.GetTo()		// �͈͑I���I��
		);
		rcNew.left   = t_max(rcNew.left  , pView->GetTextArea().GetViewLeftCol() );
		rcNew.right  = t_max(rcNew.right , pView->GetTextArea().GetViewLeftCol() );
		rcNew.right  = t_min(rcNew.right , pView->GetTextArea().GetRightCol() + 1);
		rcNew.top    = t_max(rcNew.top   , pView->GetTextArea().GetViewTopLine() );
		rcNew.bottom = t_max(rcNew.bottom, pView->GetTextArea().GetViewTopLine() - 1);	// 2010.11.02 ryoji �ǉ��i��ʏ�[������ɂ����`�I������������ƃ��[���[�����]�\���ɂȂ���̏C���j
		rcNew.bottom = t_min(rcNew.bottom, pView->GetTextArea().GetBottomLine()  );

		RECT rcNew2;
		rcNew2.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcNew.left  * nCharWidth;
		rcNew2.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)rcNew.right * nCharWidth;
		rcNew2.top		= (Int)(rcNew.top - pView->GetTextArea().GetViewTopLine()) * nCharHeight + pView->GetTextArea().GetAreaTop();
		rcNew2.bottom	= (Int)(rcNew.bottom + 1 - pView->GetTextArea().GetViewTopLine()) * nCharHeight + pView->GetTextArea().GetAreaTop();

		HRGN hrgnNew = ::CreateRectRgnIndirect( &rcNew2 );

		// ��`���B
		// ::CombineRgn()�̌��ʂ��󂯎�邽�߂ɁA�K���ȃ��[�W���������
		HRGN hrgnDraw = ::CreateRectRgnIndirect( &rcNew2 );
		{
			// ���I����`�ƐV�I����`�̃��[�W������������� �d�Ȃ肠�������������������܂�
			if( NULLREGION != ::CombineRgn( hrgnDraw, hrgnOld, hrgnNew, RGN_XOR ) ){

				// 2002.02.16 hor
				// ������̃G���A��EOF���܂܂��ꍇ��EOF�ȍ~�̕������������܂�
				// 2006.10.01 Moca ���[�\�[�X���[�N���C��������A�`�����悤�ɂȂ������߁A
				// �}���邽�߂� EOF�ȍ~�����[�W��������폜����1�x�̍��ɂ���

				// 2006.10.01 Moca Start EOF�ʒu�v�Z��GetEndLayoutPos�ɏ��������B
				CLayoutPoint ptLast;
				pView->m_pcEditDoc->m_cLayoutMgr.GetEndLayoutPos( &ptLast );
				// 2006.10.01 Moca End
				// 2011.12.26 EOF�̂Ԃ牺����s�͔��]���AEOF�݂̂̍s�͔��]���Ȃ�
				const CLayout* pBottom = pView->m_pcEditDoc->m_cLayoutMgr.GetBottomLayout();
				if( pBottom && pBottom->GetLayoutEol() == EOL_NONE ){
					ptLast.x = 0;
					ptLast.y++;
				}
				if(m_sSelect.GetFrom().y>=ptLast.y || m_sSelect.GetTo().y>=ptLast.y ||
					m_sSelectOld.GetFrom().y>=ptLast.y || m_sSelectOld.GetTo().y>=ptLast.y){
					//	Jan. 24, 2004 genta nLastLen�͕������Ȃ̂ŕϊ��K�v
					//	�ŏI�s��TAB�������Ă���Ɣ��]�͈͂��s������D
					//	2006.10.01 Moca GetEndLayoutPos�ŏ������邽��ColumnToIndex�͕s�v�ɁB
					RECT rcNew;
					rcNew.left   = pView->GetTextArea().GetAreaLeft() + (Int)(pView->GetTextArea().GetViewLeftCol() + ptLast.x) * nCharWidth;
					rcNew.right  = pView->GetTextArea().GetAreaRight();
					rcNew.top    = (Int)(ptLast.y - pView->GetTextArea().GetViewTopLine()) * nCharHeight + pView->GetTextArea().GetAreaTop();
					rcNew.bottom = rcNew.top + nCharHeight;
					
					// 2006.10.01 Moca GDI(���[�W����)���\�[�X���[�N�C��
					HRGN hrgnEOFNew = ::CreateRectRgnIndirect( &rcNew );
					::CombineRgn( hrgnDraw, hrgnDraw, hrgnEOFNew, RGN_DIFF );
					::DeleteObject( hrgnEOFNew );
				}
				::PaintRgn( hdc, hrgnDraw );
				// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
				if( bCompatBMP ){
					::PaintRgn( pView->m_hdcCompatDC, hrgnDraw );
				}
				// To Here 2007.09.09 Moca
			}
		}

		//////////////////////////////////////////
		// �f�o�b�O�p ���[�W������`�̃_���v
//@@		TraceRgn( hrgnDraw );


		if( NULL != hrgnDraw ){
			::DeleteObject( hrgnDraw );
		}
		if( NULL != hrgnNew ){
			::DeleteObject( hrgnNew );
		}
		if( NULL != hrgnOld ){
			::DeleteObject( hrgnOld );
		}
	}else{
		CLayoutRange sRangeA;
		CLayoutInt nLineNum;

		// ���ݕ`�悳��Ă���͈͂Ǝn�_������
		if( m_sSelect.GetFrom() == m_sSelectOld.GetFrom() ){
			// �͈͂�����Ɋg�傳�ꂽ
			if( PointCompare(m_sSelect.GetTo(),m_sSelectOld.GetTo()) > 0 ){
				sRangeA.SetFrom(m_sSelectOld.GetTo());
				sRangeA.SetTo  (m_sSelect.GetTo());
			}
			else{
				sRangeA.SetFrom(m_sSelect.GetTo());
				sRangeA.SetTo  (m_sSelectOld.GetTo());
			}
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine(	hdc, nLineNum, sRangeA);
				}
			}
		}
		else if( m_sSelect.GetTo() == m_sSelectOld.GetTo() ){
			// �͈͂��O���Ɋg�傳�ꂽ
			if(PointCompare(m_sSelect.GetFrom(),m_sSelectOld.GetFrom()) < 0){
				sRangeA.SetFrom(m_sSelect.GetFrom());
				sRangeA.SetTo  (m_sSelectOld.GetFrom());
			}
			else{
				sRangeA.SetFrom(m_sSelectOld.GetFrom());
				sRangeA.SetTo  (m_sSelect.GetFrom());
			}
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, sRangeA );
				}
			}
		}
		else{
			sRangeA = m_sSelectOld;
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, sRangeA );
				}
			}
			sRangeA = m_sSelect;
			for( nLineNum = sRangeA.GetFrom().GetY2(); nLineNum <= sRangeA.GetTo().GetY2(); ++nLineNum ){
				if( nLineNum >= pView->GetTextArea().GetViewTopLine() && nLineNum <= pView->GetTextArea().GetBottomLine() + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, sRangeA );
				}
			}
		}
	}

	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if( bCompatBMP ){
		::SetROP2( pView->m_hdcCompatDC, nROPCompatOld );
		::SelectObject( pView->m_hdcCompatDC, hBrushCompatOld );
	}
	// To Here 2007.09.09 Moca

	// 2006.10.01 Moca �d���R�[�h����
	::SetROP2( hdc, nROP_Old );
	::SelectObject( hdc, hBrushOld );
	::DeleteObject( hBrush );
}




/*! �I��̈�̒��̎w��s�̕`��

	�����s�ɓn��I��͈͂̂����CnLineNum�Ŏw�肳�ꂽ1�s��������`�悷��D
	�I��͈͂͌Œ肳�ꂽ�܂�nLineNum�݂̂��K�v�s���ω����Ȃ���Ăт������D

	@date 2006.03.29 Moca 3000��������P�p�D
*/
void CViewSelect::DrawSelectAreaLine(
	HDC					hdc,		//!< [in] �`��̈��Device Context Handle
	CLayoutInt			nLineNum,	//!< [in] �`��Ώۍs(���C�A�E�g�s)
	const CLayoutRange&	sRange		//!< [in] �I��͈�(���C�A�E�g�P��)
) const
{
	CEditView const * const pView = m_pcEditView;
	bool bCompatBMP = pView->m_hbmpCompatBMP && hdc != pView->m_hdcCompatDC;

	const CLayoutMgr& layoutMgr = pView->m_pcEditDoc->m_cLayoutMgr;
	const CLayout* pcLayout = layoutMgr.SearchLineByLayoutY( nLineNum );
	CLayoutRange lineArea;
	GetSelectAreaLineFromRange(lineArea, nLineNum, pcLayout, sRange);
	CLayoutInt nSelectFrom = lineArea.GetFrom().GetX2();
	CLayoutInt nSelectTo = lineArea.GetTo().GetX2();
	if( nSelectFrom == INT_MAX || nSelectTo == INT_MAX ){
		CLayoutInt nPosX = CLayoutInt(0);
		CMemoryIterator it = CMemoryIterator(pcLayout, layoutMgr.GetTabSpace());
		
		while( !it.end() ){
			it.scanNext();
			if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
				nPosX ++;
				break;
			}
			// 2006.03.28 Moca ��ʊO�܂ŋ��߂���ł��؂�
			if( it.getColumn() > pView->GetTextArea().GetRightCol() ){
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();

		if( nSelectFrom == INT_MAX ){
			nSelectFrom = nPosX;
		}
		if( nSelectTo == INT_MAX ){
			nSelectTo = nPosX;
		}
	}
	
	// 2006.03.28 Moca �E�B���h�E�����傫���Ɛ��������]���Ȃ������C��
	if( nSelectFrom < pView->GetTextArea().GetViewLeftCol() ){
		nSelectFrom = pView->GetTextArea().GetViewLeftCol();
	}
	int		nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int		nCharWidth = pView->GetTextMetrics().GetHankakuDx();
	CMyRect	rcClip; // px
	rcClip.left		= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)nSelectFrom * nCharWidth;
	rcClip.right	= (pView->GetTextArea().GetAreaLeft() - (Int)pView->GetTextArea().GetViewLeftCol() * nCharWidth) + (Int)nSelectTo   * nCharWidth;
	rcClip.top		= (Int)(nLineNum - pView->GetTextArea().GetViewTopLine()) * nLineHeight + pView->GetTextArea().GetAreaTop();
	rcClip.bottom	= rcClip.top + nLineHeight;
	if( rcClip.right > pView->GetTextArea().GetAreaRight() ){
		rcClip.right = pView->GetTextArea().GetAreaRight();
	}
	//	�K�v�ȂƂ������B
	if ( rcClip.right != rcClip.left ){
		CLayoutRange selectOld = m_sSelect;
		const_cast<CLayoutRange*>(&m_sSelect)->Clear(-1);
		pView->GetCaret().m_cUnderLine.CaretUnderLineOFF(true, false, true);
		*(const_cast<CLayoutRange*>(&m_sSelect)) = selectOld;
		
		// 2006.03.28 Moca �\������̂ݏ�������
		if( nSelectFrom <=pView->GetTextArea().GetRightCol() && pView->GetTextArea().GetViewLeftCol() < nSelectTo ){
			HRGN hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
			::PaintRgn( hdc, hrgnDraw );
			// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
			if( bCompatBMP ){
				::PaintRgn( pView->m_hdcCompatDC, hrgnDraw );
			}
			// To Here 2007.09.09 Moca
			::DeleteObject( hrgnDraw );
		}
	}
}

void CViewSelect::GetSelectAreaLineFromRange(
	CLayoutRange& ret,
	CLayoutInt nLineNum,
	const CLayout* pcLayout,
	const CLayoutRange&	sRange
) const
{
	const CEditView& view = *GetEditView();
	if( nLineNum >= sRange.GetFrom().y && nLineNum <= sRange.GetTo().y ||
		nLineNum >= sRange.GetTo().y && nLineNum <= sRange.GetFrom().y ){
		CLayoutInt	nSelectFrom = sRange.GetFrom().GetX2();
		CLayoutInt	nSelectTo   = sRange.GetTo().GetX2();
		if( IsBoxSelecting() ){		/* ��`�͈͑I�� */
			nSelectFrom = sRange.GetFrom().GetX2();
			nSelectTo   = sRange.GetTo().GetX2();
			// 2006.09.30 Moca From ��`�I����[EOF]�Ƃ��̉E���͔��]���Ȃ��悤�ɏC���B������ǉ�
			// 2011.12.26 [EOF]�P�ƍs�ȊO�Ȃ甽�]����
			if( view.m_pcEditDoc->m_cLayoutMgr.GetLineCount() <= nLineNum ){
				nSelectFrom = -1;
				nSelectTo = -1;
			}
			// 2006.09.30 Moca To
		}
		else{
			if( sRange.IsLineOne() ){
				nSelectFrom = sRange.GetFrom().GetX2();
				nSelectTo   = sRange.GetTo().GetX2();
			}
			else{
				CLayoutInt nX_Layout = CLayoutInt(INT_MAX);
				if( nLineNum == sRange.GetFrom().y ){
					nSelectFrom = sRange.GetFrom().GetX2();
					nSelectTo   = nX_Layout;
				}
				else if( nLineNum == sRange.GetTo().GetY2() ){
					nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
					nSelectTo   = sRange.GetTo().GetX2();
				}
				else{
					nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
					nSelectTo   = nX_Layout;
				}
			}
		}
		// 2006.05.24 Moca ��`�I��/�t���[�J�[�\���I��(�I���J�n/�I���s)��
		// To < From �ɂȂ邱�Ƃ�����B�K�� From < To �ɂȂ�悤�ɓ���ւ���B
		if( nSelectTo < nSelectFrom ){
			t_swap(nSelectFrom, nSelectTo);
		}
		ret.SetFrom(CLayoutPoint(nSelectFrom, nLineNum));
		ret.SetTo(CLayoutPoint(nSelectTo, nLineNum));
	}else{
		ret.SetFrom(CLayoutPoint(-1, -1));
		ret.SetTo(CLayoutPoint(-1, -1));
	}
}

/*!	�I��͈͏�񃁃b�Z�[�W�̕\��

	@author genta
	@date 2005.07.09 genta �V�K�쐬
	@date 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍��ǉ�
	@date 2006.06.28 syat �o�C�g���J�E���g��ǉ�
*/
void CViewSelect::PrintSelectionInfoMsg() const
{
	const CEditView* pView=GetEditView();

	//	�o�͂���Ȃ��Ȃ�v�Z���ȗ�
	if( ! pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2IsEffective() )
		return;

	CLayoutInt nLineCount = pView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( ! IsTextSelected() || m_sSelect.GetFrom().y >= nLineCount ){ // �擪�s�����݂��Ȃ�
		const_cast<CEditView*>(pView)->GetCaret().m_bClearStatus = false;
		if( IsBoxSelecting() ){
			pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( _T("box selecting") );
		}else if( m_bSelectingLock ){
			pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( _T("selecting") );
		}else{
			pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( _T("") );
		}
		return;
	}

	TCHAR msg[128];
	//	From here 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍�

	CLayoutInt select_line;
	if( m_sSelect.GetTo().y >= nLineCount ){	// �ŏI�s�����݂��Ȃ�
		select_line = nLineCount - m_sSelect.GetFrom().y + 1;
	}
	else {
		select_line = m_sSelect.GetTo().y - m_sSelect.GetFrom().y + 1;
	}
	
	//	To here 2006.06.06 ryoji �I��͈͂̍s�����݂��Ȃ��ꍇ�̑΍�
	if( IsBoxSelecting() ){
		//	��`�̏ꍇ�͕��ƍ��������ł��܂���
		CLayoutInt select_col = m_sSelect.GetFrom().x - m_sSelect.GetTo().x;
		if( select_col < 0 ){
			select_col = -select_col;
		}
		auto_sprintf( msg, _T("%d Columns * %d lines selected."),
			select_col, select_line );
			
	}
	else {
		//	�ʏ�̑I���ł͑I��͈͂̒��g�𐔂���
		int select_sum = 0;	//	�o�C�g�����v
		const wchar_t *pLine;	//	�f�[�^���󂯎��
		CLogicInt	nLineLen;		//	�s�̒���
		const CLayout*	pcLayout;
		CViewSelect* thiz = const_cast<CViewSelect*>( this );	// const�O��this

		// ���ʐݒ�E�I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
		BOOL bCountByByteCommon = CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispSelCountByByte;
		BOOL bCountByByte = ( pView->m_pcEditWnd->m_nSelectCountMode == SELECT_COUNT_TOGGLE ?
								bCountByByteCommon :
								pView->m_pcEditWnd->m_nSelectCountMode == SELECT_COUNT_BY_BYTE );

		//	1�s��
		pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.GetFrom().GetY2(), &nLineLen, &pcLayout );
		if( pLine ){
			if( bCountByByte ){
				//  �o�C�g���ŃJ�E���g
				//  ���������R�[�h���猻�݂̕����R�[�h�ɕϊ����A�o�C�g�����擾����B
				//  �R�[�h�ϊ��͕��ׂ������邽�߁A�I��͈͂̑������݂̂�ΏۂƂ���B

				CNativeW* cmemW = new CNativeW();
				CMemory* cmemCode = new CMemory();

				// ������������̎擾��CEditView::GetSelectedData���g���������Am_sSelect����̂��߁A
				// �Ăяo���O��m_sSelect������������B�ďo����Ɍ��ɖ߂��̂ŁAconst�ƌ����Ȃ����Ƃ��Ȃ��B
				CLayoutRange rngSelect = m_sSelect;		// �I��̈�̑ޔ�
				bool bSelExtend;						// �I��̈�g��t���O

				// �ŏI�s�̏���
				pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_sSelect.GetTo().y, &nLineLen, &pcLayout );
				if( pLine ){
					if( pView->LineColmnToIndex( pcLayout, m_sSelect.GetTo().GetX2() ) == 0 ){
						//	�ŏI�s�̐擪�ɃL�����b�g������ꍇ��
						//	���̍s���s���Ɋ܂߂Ȃ�
						--select_line;
					}
				}else{
					//	�ŏI�s����s�Ȃ�
					//	���̍s���s���Ɋ܂߂Ȃ�
					--select_line;
				}

				//2009.07.07 syat m_nLastSelectedByteLen��0�̏ꍇ�́A�����ł͂Ȃ��S�̂�ϊ�����i���[�h�ؑ֎��ɃL���b�V���N���A���邽�߁j

				if( m_bSelectAreaChanging && m_nLastSelectedByteLen && m_sSelect.GetFrom() == m_sSelectOld.GetFrom() ){
					// �͈͂�����Ɋg�傳�ꂽ
					if( PointCompare( m_sSelect.GetTo(), m_sSelectOld.GetTo() ) < 0 ){
						bSelExtend = false;				// �k��
						thiz->m_sSelect = CLayoutRange( m_sSelect.GetTo(), m_sSelectOld.GetTo() );
					}else{
						bSelExtend = true;				// �g��
						thiz->m_sSelect = CLayoutRange( m_sSelectOld.GetTo(), m_sSelect.GetTo() );
					}

					const_cast<CEditView*>( pView )->GetSelectedData( cmemW, FALSE, NULL, FALSE, FALSE );
					thiz->m_sSelect = rngSelect;		// m_sSelect�����ɖ߂�
				}
				else if( m_bSelectAreaChanging && m_nLastSelectedByteLen && m_sSelect.GetTo() == m_sSelectOld.GetTo() ){
					// �͈͂��O���Ɋg�傳�ꂽ
					if( PointCompare( m_sSelect.GetFrom(), m_sSelectOld.GetFrom() ) < 0 ){
						bSelExtend = true;				// �g��
						thiz->m_sSelect = CLayoutRange( m_sSelect.GetFrom(), m_sSelectOld.GetFrom() );
					}else{
						bSelExtend = false;				// �k��
						thiz->m_sSelect = CLayoutRange( m_sSelectOld.GetFrom(), m_sSelect.GetFrom() );
					}

					const_cast<CEditView*>( pView )->GetSelectedData( cmemW, FALSE, NULL, FALSE, FALSE );
					thiz->m_sSelect = rngSelect;		// m_sSelect�����ɖ߂�
				}
				else{
					// �I��̈�S�̂��R�[�h�ϊ��Ώۂɂ���
					const_cast<CEditView*>( pView )->GetSelectedData( cmemW, FALSE, NULL, FALSE, FALSE );
					bSelExtend = true;
					thiz->m_nLastSelectedByteLen = 0;
				}
				//  ���݂̕����R�[�h�ɕϊ����A�o�C�g�����擾����
				CCodeBase* pCode = CCodeFactory::CreateCodeBase(pView->m_pcEditDoc->GetDocumentEncoding(), false);
				pCode->UnicodeToCode( *cmemW, cmemCode );
				delete pCode;

				if( bSelExtend ){
					select_sum = m_nLastSelectedByteLen + cmemCode->GetRawLength();
				}else{
					select_sum = m_nLastSelectedByteLen - cmemCode->GetRawLength();
				}
				thiz->m_nLastSelectedByteLen = select_sum;

				delete cmemW;
				delete cmemCode;
			}
			else{
				//  �������ŃJ�E���g

				//2009.07.07 syat �J�E���g���@��؂�ւ��Ȃ���I��͈͂��g��E�k������Ɛ�������
				//                �Ƃ�Ȃ��Ȃ邽�߁A���[�h�ؑ֎��ɃL���b�V�����N���A����B
				thiz->m_nLastSelectedByteLen = 0;

				//	1�s�����I������Ă���ꍇ
				if( m_sSelect.IsLineOne() ){
					select_sum =
						pView->LineColmnToIndex( pcLayout, m_sSelect.GetTo().GetX2() )
						- pView->LineColmnToIndex( pcLayout, m_sSelect.GetFrom().GetX2() );
				} else {	//	2�s�ȏ�I������Ă���ꍇ
					select_sum =
						pcLayout->GetLengthWithoutEOL()
						+ pcLayout->GetLayoutEol().GetLen()
						- pView->LineColmnToIndex( pcLayout, m_sSelect.GetFrom().GetX2() );

					//	GetSelectedData�Ǝ��Ă��邪�C�擪�s�ƍŏI�s�͔r�����Ă���
					//	Aug. 16, 2005 aroka nLineNum��for�ȍ~�ł��g����̂�for�̑O�Ő錾����
					//	VC .NET�ȍ~�ł�Microsoft�g����L���ɂ����W�������VC6�Ɠ������Ƃɒ���
					CLayoutInt nLineNum;
					for( nLineNum = m_sSelect.GetFrom().GetY2() + CLayoutInt(1);
						nLineNum < m_sSelect.GetTo().GetY2(); ++nLineNum ){
						pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
						//	2006.06.06 ryoji �w��s�̃f�[�^�����݂��Ȃ��ꍇ�̑΍�
						if( NULL == pLine )
							break;
						select_sum += pcLayout->GetLengthWithoutEOL() + pcLayout->GetLayoutEol().GetLen();
					}

					//	�ŏI�s�̏���
					pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
					if( pLine ){
						int last_line_chars = pView->LineColmnToIndex( pcLayout, m_sSelect.GetTo().GetX2() );
						select_sum += last_line_chars;
						if( last_line_chars == 0 ){
							//	�ŏI�s�̐擪�ɃL�����b�g������ꍇ��
							//	���̍s���s���Ɋ܂߂Ȃ�
							--select_line;
						}
					}
					else
					{
						//	�ŏI�s����s�Ȃ�
						//	���̍s���s���Ɋ܂߂Ȃ�
						--select_line;
					}
				}
			}
		}

#ifdef _DEBUG
		auto_sprintf( msg, _T("%d %ts (%d lines) selected. [%d:%d]-[%d:%d]"),
			select_sum,
			( bCountByByte ? _T("bytes") : _T("chars") ),
			select_line,
			m_sSelect.GetFrom().x, m_sSelect.GetFrom().y,
			m_sSelect.GetTo().x, m_sSelect.GetTo().y );
#else
		auto_sprintf( msg, _T("%d %ts (%d lines) selected."), select_sum, ( bCountByByte ? _T("bytes") : _T("chars") ), select_line );
#endif
	}
	const_cast<CEditView*>(pView)->GetCaret().m_bClearStatus = false;
	pView->m_pcEditDoc->m_pcEditWnd->m_cStatusBar.SendStatusMessage2( msg );
}
