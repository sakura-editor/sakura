#include "stdafx.h"
#include "CCaret.h"
#include "CTextArea.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "CTextMetrics.h"
#include "mem/CMemoryIterator.h"
#include "doc/CLayout.h"
#include "charset/charcode.h"
#include "charset/CCodeFactory.h"
#include "window/CEditWnd.h"
#include <vector>
#include "view/colors/CColorStrategy.h"
using namespace std;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �O���ˑ�                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


int CCaret::GetHankakuDx() const
{
	return m_pEditView->GetTextMetrics().GetHankakuDx();
}

int CCaret::GetHankakuHeight() const
{
	return m_pEditView->GetTextMetrics().GetHankakuHeight();
}

int CCaret::GetHankakuDy() const
{
	return m_pEditView->GetTextMetrics().GetHankakuDy();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CCaretUnderLine                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �J�[�\���s�A���_�[���C����ON */
void CCaretUnderLine::CaretUnderLineON( bool bDraw )
{
	if( m_nLockCounter ) return;	//	���b�N����Ă����牽���ł��Ȃ��B
	m_pcEditView->CaretUnderLineON( bDraw );
}

/* �J�[�\���s�A���_�[���C����OFF */
void CCaretUnderLine::CaretUnderLineOFF( bool bDraw )
{
	if( m_nLockCounter ) return;	//	���b�N����Ă����牽���ł��Ȃ��B
	m_pcEditView->CaretUnderLineOFF( bDraw );
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CCaret::CCaret(CEditView* pEditView, const CEditDoc* pEditDoc)
: m_pEditView(pEditView)
, m_pEditDoc(pEditDoc)
, m_ptCaretPos_Layout(0,0)
, m_sizeCaret(0,0)				// �L�����b�g�̃T�C�Y
, m_ptCaretPos_Logic(0,0)			// �J�[�\���ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
, m_cUnderLine(pEditView)
{
	m_nCaretPosX_Prev = CLayoutInt(0);		/* �r���[���[����̃J�[�\�������O�̈ʒu(�O�I���W��) */

	m_crCaret = -1;				/* �L�����b�g�̐F */			// 2006.12.16 ryoji
	m_hbmpCaret = NULL;			/* �L�����b�g�p�r�b�g�}�b�v */	// 2006.11.28 ryoji
}

CCaret::~CCaret()
{
	// �L�����b�g�p�r�b�g�}�b�v	// 2006.11.28 ryoji
	if( m_hbmpCaret != NULL )
		DeleteObject( m_hbmpCaret );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief �s���w��ɂ��J�[�\���ړ�

	�K�v�ɉ����ďc/���X�N���[��������D
	�����X�N���[���������ꍇ�͂��̍s����Ԃ��i���^���j�D
	
	@return �c�X�N���[���s��(��:��X�N���[��/��:���X�N���[��)

	@note �s���Ȉʒu���w�肳�ꂽ�ꍇ�ɂ͓K�؂ȍ��W�l��
		�ړ����邽�߁C�����ŗ^�������W�ƈړ���̍��W��
		�K��������v���Ȃ��D
	
	@note bScroll��false�̏ꍇ�ɂ̓J�[�\���ʒu�݈̂ړ�����D
		true�̏ꍇ�ɂ̓X�N���[���ʒu�����킹�ĕύX�����

	@date 2001/10/20 deleted by novice AdjustScrollBar()���ĂԈʒu��ύX
	@date 2004.04.02 Moca �s�����L���ȍ��W�ɏC������̂������ɏ�������
	@date 2004.09.11 genta bDraw�X�C�b�`�͓���Ɩ��̂���v���Ă��Ȃ��̂�
		�ĕ`��X�C�b�`����ʈʒu�����X�C�b�`�Ɩ��̕ύX
*/
CLayoutInt CCaret::MoveCursor(
	CLayoutPoint	ptWk_CaretPos,		//!< [in] �ړ��惌�C�A�E�g�ʒu
	bool			bScroll,			//!< [in] true: ��ʈʒu�����L��  false: ��ʈʒu��������
	int				nCaretMarginRate	//!< [in] �c�X�N���[���J�n�ʒu�����߂�l
)
{
	//�K�v�ȃC���^�[�t�F�[�X
	const CLayoutMgr* pLayoutMgr=&m_pEditDoc->m_cLayoutMgr;
	const STypeConfig* pTypes=&m_pEditDoc->m_cDocType.GetDocumentAttribute();

	// �X�N���[������
	CLayoutInt	nScrollRowNum = CLayoutInt(0);
	CLayoutInt	nScrollColNum = CLayoutInt(0);
	int		nCaretMarginY;
	CLayoutInt		nScrollMarginRight;
	CLayoutInt		nScrollMarginLeft;

	if( 0 >= m_pEditView->GetTextArea().m_nViewColNum ){
		return CLayoutInt(0);
	}

	// �J�[�\���s�A���_�[���C����OFF
	m_cUnderLine.CaretUnderLineOFF( bScroll );	//	YAZAKI

	if( m_pEditView->GetSelectionInfo().IsMouseSelecting() ){	// �͈͑I��
		nCaretMarginY = 0;
	}
	else{
		//	2001/10/20 novice
		nCaretMarginY = (Int)m_pEditView->GetTextArea().m_nViewRowNum / nCaretMarginRate;
		if( 1 > nCaretMarginY ){
			nCaretMarginY = 1;
		}
	}
	// 2004.04.02 Moca �s�����L���ȍ��W�ɏC������̂������ɏ�������
	GetAdjustCursorPos( &ptWk_CaretPos );
	
	
	// �����X�N���[���ʁi�������j�̎Z�o
	nScrollColNum = CLayoutInt(0);
	nScrollMarginRight = CLayoutInt(SCROLLMARGIN_RIGHT);
	nScrollMarginLeft = CLayoutInt(SCROLLMARGIN_LEFT);

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if( m_pEditDoc->m_cLayoutMgr.GetMaxLineKetas() > m_pEditView->GetTextArea().m_nViewColNum &&
		ptWk_CaretPos.GetX() > m_pEditView->GetTextArea().GetViewLeftCol() + m_pEditView->GetTextArea().m_nViewColNum - nScrollMarginRight ){
		nScrollColNum =
			( m_pEditView->GetTextArea().GetViewLeftCol() + m_pEditView->GetTextArea().m_nViewColNum - nScrollMarginRight ) - ptWk_CaretPos.GetX2();
	}
	else if( 0 < m_pEditView->GetTextArea().GetViewLeftCol() &&
		ptWk_CaretPos.GetX() < m_pEditView->GetTextArea().GetViewLeftCol() + nScrollMarginLeft
	){
		nScrollColNum = m_pEditView->GetTextArea().GetViewLeftCol() + nScrollMarginLeft - ptWk_CaretPos.GetX2();
		if( 0 > m_pEditView->GetTextArea().GetViewLeftCol() - nScrollColNum ){
			nScrollColNum = m_pEditView->GetTextArea().GetViewLeftCol();
		}

	}

	m_pEditView->GetTextArea().SetViewLeftCol(m_pEditView->GetTextArea().GetViewLeftCol() - nScrollColNum);

	//	From Here 2007.07.28 ���イ�� : �\���s����3�s�ȉ��̏ꍇ�̓�����P
	/* �����X�N���[���ʁi�s���j�̎Z�o */
										// ��ʂ��R�s�ȉ�
	if( m_pEditView->GetTextArea().m_nViewRowNum <= 3 ){
							// �ړ���́A��ʂ̃X�N���[�����C�����ォ�H�iup �L�[�j
		if( ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine() < nCaretMarginY ){
			if( ptWk_CaretPos.y < nCaretMarginY ){	//�P�s�ڂɈړ�
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine();
			}
			else if( m_pEditView->GetTextArea().m_nViewRowNum <= 1 ){	// ��ʂ��P�s
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine() - ptWk_CaretPos.y;
			}
#if !(0)	// COMMENT�ɂ���ƁA�㉺�̋󂫂����炵�Ȃ��ׁA�c�ړ���good�����A���ړ��̏ꍇ�㉺�ɂԂ��
			else if( m_pEditView->GetTextArea().m_nViewRowNum <= 2 ){	// ��ʂ��Q�s
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine() - ptWk_CaretPos.y;
			}
#endif
			else
			{						// ��ʂ��R�s
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine() - ptWk_CaretPos.y + 1;
			}
		}else
							// �ړ���́A��ʂ̍ő�s���|�Q��艺���H�idown �L�[�j
		if( ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine() >= (m_pEditView->GetTextArea().m_nViewRowNum - nCaretMarginY - 2) ){
			CLayoutInt ii = m_pEditDoc->m_cLayoutMgr.GetLineCount();
			if( ii - ptWk_CaretPos.y < nCaretMarginY + 1 &&
				ii - m_pEditView->GetTextArea().GetViewTopLine() < m_pEditView->GetTextArea().m_nViewRowNum ) {
			}
			else if( m_pEditView->GetTextArea().m_nViewRowNum <= 2 ){	// ��ʂ��Q�s�A�P�s
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine() - ptWk_CaretPos.y;
			}else{						// ��ʂ��R�s
				nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine() - ptWk_CaretPos.y + 1;
			}
		}
	}
	// �ړ���́A��ʂ̃X�N���[�����C�����ォ�H�iup �L�[�j
	else if( ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine() < nCaretMarginY ){
		if( ptWk_CaretPos.y < nCaretMarginY ){	//�P�s�ڂɈړ�
			nScrollRowNum = m_pEditView->GetTextArea().GetViewTopLine();
		}else{
			nScrollRowNum = -(ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine()) + nCaretMarginY;
		}
	}
	// �ړ���́A��ʂ̍ő�s���|�Q��艺���H�idown �L�[�j
	else if( ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine() >= m_pEditView->GetTextArea().m_nViewRowNum - nCaretMarginY - 2 ){
		CLayoutInt ii = m_pEditDoc->m_cLayoutMgr.GetLineCount();
		if( ii - ptWk_CaretPos.y < nCaretMarginY + 1 &&
			ii - m_pEditView->GetTextArea().GetViewTopLine() < m_pEditView->GetTextArea().m_nViewRowNum ) {
		}
		else{
			nScrollRowNum =
				-(ptWk_CaretPos.y - m_pEditView->GetTextArea().GetViewTopLine()) + (m_pEditView->GetTextArea().m_nViewRowNum - nCaretMarginY - 2);
		}
	}
	//	To Here 2007.07.28 ���イ��
	if( bScroll ){
		/* �X�N���[�� */
		if( t_abs( nScrollColNum ) >= m_pEditView->GetTextArea().m_nViewColNum ||
			t_abs( nScrollRowNum ) >= m_pEditView->GetTextArea().m_nViewRowNum ){
			m_pEditView->GetTextArea().OffsetViewTopLine(-nScrollRowNum);
			m_pEditView->InvalidateRect( NULL );
		}
		else if( nScrollRowNum != 0 || nScrollColNum != 0 ){
			RECT	rcClip;
			RECT	rcClip2;
			RECT	rcScroll;

			m_pEditView->GetTextArea().GenerateTextAreaRect(&rcScroll);
			if( nScrollRowNum > 0 ){
				rcScroll.bottom = m_pEditView->GetTextArea().GetAreaBottom() - (Int)nScrollRowNum * m_pEditView->GetTextMetrics().GetHankakuDy();
				m_pEditView->GetTextArea().OffsetViewTopLine(-nScrollRowNum);
				m_pEditView->GetTextArea().GenerateTopRect(&rcClip,nScrollRowNum);
			}
			else if( nScrollRowNum < 0 ){
				rcScroll.top = m_pEditView->GetTextArea().GetAreaTop() - (Int)nScrollRowNum * m_pEditView->GetTextMetrics().GetHankakuDy();
				m_pEditView->GetTextArea().OffsetViewTopLine(-nScrollRowNum);
				m_pEditView->GetTextArea().GenerateBottomRect(&rcClip,-nScrollRowNum);
			}

			if( nScrollColNum > 0 ){
				rcScroll.left = m_pEditView->GetTextArea().GetAreaLeft();
				rcScroll.right = m_pEditView->GetTextArea().GetAreaRight() - (Int)nScrollColNum * GetHankakuDx();
				m_pEditView->GetTextArea().GenerateLeftRect(&rcClip2, nScrollColNum);
			}
			else if( nScrollColNum < 0 ){
				rcScroll.left = m_pEditView->GetTextArea().GetAreaLeft() - (Int)nScrollColNum * GetHankakuDx();
				m_pEditView->GetTextArea().GenerateRightRect(&rcClip2, -nScrollColNum);
			}

			if( m_pEditView->GetDrawSwitch() ){
				m_pEditView->ScrollWindowEx(
					(Int)nScrollColNum * GetHankakuDx(),	/* �����X�N���[���� */
					(Int)nScrollRowNum * m_pEditView->GetTextMetrics().GetHankakuDy(),	/* �����X�N���[���� */
					&rcScroll,	/* �X�N���[�������`�̍\���̂̃A�h���X */
					NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
				);

				// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
				if( m_pEditView->m_hbmpCompatBMP ){
					// �݊�BMP���X�N���[�������̂��߂�BitBlt�ňړ�������
					::BitBlt(
						m_pEditView->m_hdcCompatDC,
						rcScroll.left + (Int)nScrollColNum * ( m_pEditView->GetTextMetrics().GetHankakuWidth() +  m_pEditView->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nColmSpace ),
						rcScroll.top  + (Int)nScrollRowNum * ( m_pEditView->GetTextMetrics().GetHankakuHeight() + m_pEditView->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nLineSpace ),
						rcScroll.right - rcScroll.left, rcScroll.bottom - rcScroll.top,
						m_pEditView->m_hdcCompatDC, rcScroll.left, rcScroll.top, SRCCOPY
					);
				}
				// �J�[�\���̏c�����e�L�X�g�ƍs�ԍ��̌��Ԃɂ���Ƃ��A�X�N���[�����ɏc���̈���X�V
				if( nScrollColNum != 0 && m_pEditView->m_nOldCursorLineX == m_pEditView->GetTextArea().GetAreaLeft() - 1 ){
					RECT rcClip3;
					rcClip3.left  = m_pEditView->m_nOldCursorLineX;
					rcClip3.right = m_pEditView->m_nOldCursorLineX + 1;
					rcClip3.top   = m_pEditView->GetTextArea().GetAreaTop();
					rcClip3.bottom = m_pEditView->GetTextArea().GetAreaBottom();
					::InvalidateRect( m_pEditView->GetHwnd(), &rcClip3, TRUE );
				}
				// To Here 2007.09.09 Moca

				if( nScrollRowNum != 0 ){
					m_pEditView->InvalidateRect( &rcClip );
					if( nScrollColNum != 0 ){
						m_pEditView->GetTextArea().GenerateLineNumberRect(&rcClip);
						m_pEditView->InvalidateRect( &rcClip );
					}
				}
				if( nScrollColNum != 0 ){
					m_pEditView->InvalidateRect( &rcClip2 );
				}
			}
		}

		/* �X�N���[���o�[�̏�Ԃ��X�V���� */
		m_pEditView->AdjustScrollBars(); // 2001/10/20 novice
	}

	/* �L�����b�g�ړ� */
	SetCaretLayoutPos(ptWk_CaretPos);

	/* �J�[�\���ʒu�ϊ�
	||  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	||  �������ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	m_pEditDoc->m_cLayoutMgr.LayoutToLogic(
		m_ptCaretPos_Layout,
		&m_ptCaretPos_Logic	//�J�[�\���ʒu�B���W�b�N�P�ʁB
	);

	// ���X�N���[��������������A���[���[�S�̂��ĕ`�� 2002.02.25 Add By KK
	if (nScrollColNum != 0 ){
		//����DispRuler�Ăяo�����ɍĕ`��B�ibDraw=false�̃P�[�X���l�������B�j
		m_pEditView->GetRuler().SetRedrawFlag();
	}

	/* �J�[�\���s�A���_�[���C����ON */
	//CaretUnderLineON( bDraw ); //2002.02.27 Del By KK �A���_�[���C���̂������ጸ
	HDC		hdc = m_pEditView->GetDC();
	if( bScroll ){
		/* �L�����b�g�̕\���E�X�V */
		ShowEditCaret();

		/* ���[���̍ĕ`�� */
		m_pEditView->GetRuler().DispRuler( hdc );

		/* �A���_�[���C���̍ĕ`�� */
		m_cUnderLine.CaretUnderLineON(TRUE);

		/* �L�����b�g�̍s���ʒu��\������ */
		ShowCaretPosInfo();

		//	Sep. 11, 2004 genta �����X�N���[���̊֐���
		//	bScroll == FALSE�̎��ɂ̓X�N���[�����Ȃ��̂ŁC���s���Ȃ�
		m_pEditView->SyncScrollV( -nScrollRowNum );	//	�������t�Ȃ̂ŕ������]���K�v
		m_pEditView->SyncScrollH( -nScrollColNum );	//	�������t�Ȃ̂ŕ������]���K�v

	}
	m_pEditView->ReleaseDC( hdc );

// 02/09/18 �Ί��ʂ̋����\�� ai Start	03/02/18 ai mod S
	m_pEditView->DrawBracketPair( false );
	m_pEditView->SetBracketPairPos( true );
	m_pEditView->DrawBracketPair( true );
// 02/09/18 �Ί��ʂ̋����\�� ai End		03/02/18 ai mod E

	return nScrollRowNum;

}

/* �}�E�X���ɂ����W�w��ɂ��J�[�\���ړ�
|| �K�v�ɉ����ďc/���X�N���[��������
|| �����X�N���[���������ꍇ�͂��̍s����Ԃ�(���^��)
*/
//2007.09.11 kobake �֐����ύX: MoveCursorToPoint��MoveCursorToClientPoint
CLayoutInt CCaret::MoveCursorToClientPoint( const POINT& ptClientPos )
{
	CLayoutInt		nScrollRowNum;
	CLayoutPoint	ptLayoutPos;
	m_pEditView->GetTextArea().ClientToLayout(ptClientPos, &ptLayoutPos);

	int	dx = (ptClientPos.x - m_pEditView->GetTextArea().GetAreaLeft()) % ( m_pEditView->GetTextMetrics().GetHankakuDx() );

	nScrollRowNum = MoveCursorProperly( ptLayoutPos, TRUE, 1000, dx );
	m_nCaretPosX_Prev = GetCaretLayoutPos().GetX2();
	return nScrollRowNum;
}
//_CARETMARGINRATE_CARETMARGINRATE_CARETMARGINRATE



/*! �������J�[�\���ʒu���Z�o����(EOF�ȍ~�̂�)
	@param pnPosX [in/out] �J�[�\���̃��C�A�E�g���WX
	@param pnPosY [in/out] �J�[�\���̃��C�A�E�g���WY
	@retval	TRUE ���W���C������
	@retval	FALSE ���W�͏C������Ȃ�����
	@note	EOF�̒��O�����s�łȂ��ꍇ�́A���̍s�Ɍ���EOF�ȍ~�ɂ��ړ��\
			EOF�����̍s�́A�擪�ʒu�̂ݐ������B
	@date 2004.04.02 Moca �֐���
*/
BOOL CCaret::GetAdjustCursorPos(
	CLayoutPoint* pptPosXY
)
{
	//�K�v�ȃC���^�[�t�F�[�X
	const CLayoutMgr* pLayoutMgr=&m_pEditDoc->m_cLayoutMgr;

	// 2004.03.28 Moca EOF�݂̂̃��C�A�E�g�s�́A0���ڂ̂ݗL��.EOF��艺�̍s�̂���ꍇ�́AEOF�ʒu�ɂ���
	CLayoutInt nLayoutLineCount = m_pEditDoc->m_cLayoutMgr.GetLineCount();

	CLayoutPoint ptPosXY2 = *pptPosXY;
	/*
	int nPosX2 = *pnPosX;
	int nPosY2 = *pnPosY;
	*/
	BOOL ret = FALSE;
	if( ptPosXY2.y >= nLayoutLineCount ){
		if( 0 < nLayoutLineCount ){
			ptPosXY2.y = nLayoutLineCount - 1;
			const CLayout* pcLayout = m_pEditDoc->m_cLayoutMgr.SearchLineByLayoutY( ptPosXY2.GetY2() );
			if( pcLayout->GetLayoutEol() == EOL_NONE ){
				ptPosXY2.x = m_pEditView->LineIndexToColmn( pcLayout, (CLogicInt)pcLayout->GetLengthWithEOL() );
				// [EOF]�̂ݐ܂�Ԃ��̂͂�߂�	// 2009.02.17 ryoji
				// ��������Ȃ� ptPosXY2.x �ɐ܂�Ԃ��s�C���f���g��K�p����̂��悢

				// EOF�����܂�Ԃ���Ă��邩
				//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
				//if( ptPosXY2.x >= m_pEditDoc->m_cLayoutMgr.GetMaxLineKetas() ){
				//	ptPosXY2.y++;
				//	ptPosXY2.x = CLayoutInt(0);
				//}
			}
			else{
				// EOF�����̍s
				ptPosXY2.y++;
				ptPosXY2.x = CLayoutInt(0);
			}
		}else{
			// ��̃t�@�C��
			ptPosXY2.Set(CLayoutInt(0), CLayoutInt(0));
		}
		if( *pptPosXY != ptPosXY2 ){//*pnPosX != nPosX2 || *pnPosY != nPosY2 ){
			*pptPosXY = ptPosXY2;
			//*pnPosX = nPosX2;
			//*pnPosY = nPosY2;
			ret = TRUE;
		}
	}
	return ret;
}

/* �L�����b�g�̕\���E�X�V */
void CCaret::ShowEditCaret()
{
	//�K�v�ȃC���^�[�t�F�[�X
	const CLayoutMgr* pLayoutMgr=&m_pEditDoc->m_cLayoutMgr;
	CommonSetting* pCommon=&GetDllShareData().m_Common;
	const STypeConfig* pTypes=&m_pEditDoc->m_cDocType.GetDocumentAttribute();


	using namespace WCODE;

	int				nIdxFrom;

/*
	�t�H�[�J�X�������Ƃ��ɓ����I�ɃL�����b�g�쐬����ƈÖٓI�ɃL�����b�g�j���i���j����Ă�
	�L�����b�g������im_nCaretWidth != 0�j�Ƃ������ƂɂȂ��Ă��܂��A�t�H�[�J�X���擾���Ă�
	�L�����b�g���o�Ă��Ȃ��Ȃ�ꍇ������
	�t�H�[�J�X�������Ƃ��̓L�����b�g���쐬�^�\�����Ȃ��悤�ɂ���

	���L�����b�g�̓X���b�h�ɂЂƂ����Ȃ̂ŗႦ�΃G�f�B�b�g�{�b�N�X���t�H�[�J�X�擾�����
	�@�ʌ`��̃L�����b�g�ɈÖٓI�ɍ����ւ����邵�t�H�[�J�X�������ΈÖٓI�ɔj�������

	2007.12.11 ryoji
	�h���b�O�A���h�h���b�v�ҏW���̓L�����b�g���K�v�ňÖٔj���̗v���������̂ŗ�O�I�ɕ\������
*/
	if( ::GetFocus() != m_pEditView->GetHwnd() && !m_pEditView->m_bDragMode ){
		m_sizeCaret.cx = 0;
		return;
	}

	/* �L�����b�g�̕��A���������� */
	int				nCaretWidth = 0;
	int				nCaretHeight = 0;
	// �J�[�\���̃^�C�v = win
	if( 0 == pCommon->m_sGeneral.GetCaretType() ){
		nCaretHeight = GetHankakuHeight();					/* �L�����b�g�̍��� */
		if( m_pEditView->IsInsMode() /* Oct. 2, 2005 genta */ ){
			nCaretWidth = 2;
		}
		else{
			nCaretWidth = GetHankakuDx();

			const wchar_t*	pLine;
			CLogicInt		nLineLen;
			const CLayout*	pcLayout;
			pLine = pLayoutMgr->GetLineStr( GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );

			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom = m_pEditView->LineColmnToIndex( pcLayout, GetCaretLayoutPos().GetX2() );
				if( nIdxFrom >= nLineLen ||
					pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
					pLine[nIdxFrom] == TAB ){
					nCaretWidth = GetHankakuDx();
				}
				else{
					CLayoutInt nKeta = CNativeW::GetKetaOfChar( pLine, nLineLen, nIdxFrom );
					if( 0 < nKeta ){
						nCaretWidth = GetHankakuDx() * (Int)nKeta;
					}
				}
			}
		}
	}
	// �J�[�\���̃^�C�v = dos
	else if( 1 == pCommon->m_sGeneral.GetCaretType() ){
		if( m_pEditView->IsInsMode() /* Oct. 2, 2005 genta */ ){
			nCaretHeight = GetHankakuHeight() / 2;			/* �L�����b�g�̍��� */
		}
		else{
			nCaretHeight = GetHankakuHeight();				/* �L�����b�g�̍��� */
		}
		nCaretWidth = GetHankakuDx();

		const wchar_t*	pLine;
		CLogicInt		nLineLen;
		const CLayout*	pcLayout;
		pLine= pLayoutMgr->GetLineStr( GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );

		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdxFrom = m_pEditView->LineColmnToIndex( pcLayout, GetCaretLayoutPos().GetX2() );
			if( nIdxFrom >= nLineLen ||
				pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
				pLine[nIdxFrom] == TAB ){
				nCaretWidth = GetHankakuDx();
			}else{
				CLayoutInt nKeta = CNativeW::GetKetaOfChar( pLine, nLineLen, nIdxFrom );
				if( 0 < nKeta ){
					nCaretWidth = GetHankakuDx() * (Int)nKeta;
				}
			}
		}
	}

	//	�L�����b�g�F�̎擾
	const ColorInfo* ColorInfoArr = pTypes->m_ColorInfoArr;
	int nCaretColor = ( ColorInfoArr[COLORIDX_CARET_IME].m_bDisp && m_pEditView->IsImeON() )? COLORIDX_CARET_IME: COLORIDX_CARET;
	COLORREF crCaret = ColorInfoArr[nCaretColor].m_colTEXT;
	COLORREF crBack = ColorInfoArr[COLORIDX_TEXT].m_colBACK;

	if( !ExistCaretFocus() ){
		/* �L�����b�g���Ȃ������ꍇ */
		/* �L�����b�g�̍쐬 */
		CreateEditCaret( crCaret, crBack, nCaretWidth, nCaretHeight );	// 2006.12.07 ryoji
		m_bCaretShowFlag = false; // 2002/07/22 novice
	}
	else{
		if( GetCaretSize() != CMySize(nCaretWidth,nCaretHeight) || m_crCaret != crCaret || m_pEditView->m_crBack != crBack ){
			/* �L�����b�g�͂��邪�A�傫����F���ς�����ꍇ */
			/* ���݂̃L�����b�g���폜 */
			::DestroyCaret();

			/* �L�����b�g�̍쐬 */
			CreateEditCaret( crCaret, crBack, nCaretWidth, nCaretHeight );	// 2006.12.07 ryoji
			m_bCaretShowFlag = false; // 2002/07/22 novice
		}
		else{
			/* �L�����b�g�͂��邵�A�傫�����ς���Ă��Ȃ��ꍇ */
			/* �L�����b�g���B�� */
			HideCaret_( m_pEditView->GetHwnd() ); // 2002/07/22 novice
		}
	}

	// �L�����b�g�T�C�Y
	SetCaretSize(nCaretWidth,nCaretHeight);

	/* �L�����b�g�̈ʒu�𒲐� */
	//2007.08.26 kobake �L�����b�gX���W�̌v�Z��UNICODE�d�l�ɂ����B
	POINT ptDrawPos=CalcCaretDrawPos(GetCaretLayoutPos());
	::SetCaretPos( ptDrawPos.x, ptDrawPos.y );
	if ( m_pEditView->GetTextArea().GetAreaLeft() <= ptDrawPos.x && m_pEditView->GetTextArea().GetAreaTop() <= ptDrawPos.y ){
		/* �L�����b�g�̕\�� */
		ShowCaret_( m_pEditView->GetHwnd() ); // 2002/07/22 novice
	}

	m_crCaret = crCaret;	//	2006.12.07 ryoji
	m_pEditView->m_crBack = crBack;		//	2006.12.07 ryoji
	m_pEditView->SetIMECompFormPos();
}




/*! �L�����b�g�̍s���ʒu����уX�e�[�^�X�o�[�̏�ԕ\���̍X�V

	@note �X�e�[�^�X�o�[�̏�Ԃ̕��ѕ��̕ύX�̓��b�Z�[�W����M����
		CEditWnd::DispatchEvent()��WM_NOTIFY�ɂ��e�������邱�Ƃɒ���
	
	@note �X�e�[�^�X�o�[�̏o�͓��e�̕ύX��CEditWnd::OnSize()��
		�J�������v�Z�ɉe�������邱�Ƃɒ���
*/
//2007.10.17 kobake �d������R�[�h�𐮗�
void CCaret::ShowCaretPosInfo()
{
	//�K�v�ȃC���^�[�t�F�[�X
	const CLayoutMgr* pLayoutMgr=&m_pEditDoc->m_cLayoutMgr;
	const STypeConfig* pTypes=&m_pEditDoc->m_cDocType.GetDocumentAttribute();


	if( !m_pEditView->GetDrawSwitch() ){
		return;
	}

	// �X�e�[�^�X�o�[�n���h�����擾
	HWND hwndStatusBar = m_pEditDoc->m_pcEditWnd->m_cStatusBar.GetStatusHwnd();


	// �J�[�\���ʒu�̕�������擾
	const CLayout*	pcLayout;
	CLogicInt		nLineLen;
	const wchar_t*	pLine = pLayoutMgr->GetLineStr( GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );


	// -- -- -- -- �����R�[�h��� -> pszCodeName -- -- -- -- //
	const TCHAR* pszCodeName;
	if(!hwndStatusBar){
		pszCodeName = CCodeTypeName(m_pEditDoc->GetDocumentEncoding()).Short();
	}
	else{
		pszCodeName = CCodeTypeName(m_pEditDoc->GetDocumentEncoding()).Normal();
	}


	// -- -- -- -- ���s���[�h -> szEolMode -- -- -- -- //
	//	May 12, 2000 genta
	//	���s�R�[�h�̕\����ǉ�
	CEol cNlType = m_pEditDoc->m_cDocEditor.GetNewLineCode();
	const TCHAR* szEolMode = cNlType.GetName();


	// -- -- -- -- �L�����b�g�ʒu -> ptCaret -- -- -- -- //
	//
	CMyPoint ptCaret;
	//�s�ԍ������W�b�N�P�ʂŕ\��
	if(pTypes->m_bLineNumIsCRLF){
		ptCaret.x = (Int)GetCaretLayoutPos().GetX();
		ptCaret.y = (Int)GetCaretLogicPos().y;
		if(pcLayout){
			ptCaret.x -= (Int)pcLayout->GetIndent();
			ptCaret.x += (Int)pcLayout->CalcLayoutOffset(*pLayoutMgr);
		}
	}
	//�s�ԍ������C�A�E�g�P�ʂŕ\��
	else {
		ptCaret.x = (Int)GetCaretLayoutPos().GetX();
		ptCaret.y = (Int)GetCaretLayoutPos().GetY();
	}
	//�\���l��1����n�܂�悤�ɕ␳
	ptCaret.x++;
	ptCaret.y++;


	// -- -- -- -- �L�����b�g�ʒu�̕������ -> szCaretChar -- -- -- -- //
	//
	TCHAR szCaretChar[32]=_T("");
	if( pLine ){
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		CLogicInt nIdx = m_pEditView->LineColmnToIndex( pcLayout, GetCaretLayoutPos().GetX2() );
		if( nIdx < nLineLen ){
			if( nIdx < nLineLen - (pcLayout->GetLayoutEol().GetLen()?1:0) ){
				//auto_sprintf( szCaretChar, _T("%04x"), );
				//�C�ӂ̕����R�[�h����Unicode�֕ϊ�����		2008/6/9 Uchi
				CCodeBase* pCode = CCodeFactory::CreateCodeBase(m_pEditDoc->GetDocumentEncoding(), false);
				EConvertResult ret = pCode->UnicodeToHex(&pLine[nIdx], nLineLen - nIdx, szCaretChar);
				delete pCode;
				if (ret != RESULT_COMPLETE) {
					// ���܂��R�[�h�����Ȃ�����(Unicode�ŕ\��)
					pCode = CCodeFactory::CreateCodeBase(CODE_UNICODE, false);
					EConvertResult ret = pCode->UnicodeToHex(&pLine[nIdx], nLineLen - nIdx, szCaretChar);
					delete pCode;
				}
			}
			else{
				_tcscpy_s(szCaretChar, _countof(szCaretChar), pcLayout->GetLayoutEol().GetName());
			}
		}
	}


	// -- -- -- --  �X�e�[�^�X���������o�� -- -- -- -- //
	//
	// �E�B���h�E�E��ɏ����o��
	if( !hwndStatusBar ){
		TCHAR	szText[64];
		TCHAR	szFormat[64];
		TCHAR	szLeft[64];
		TCHAR	szRight[64];
		int		nLen;
		{	// ���b�Z�[�W�̍���������i�u�s:��v���������\���j
			nLen = _tcslen(pszCodeName) + _tcslen(szEolMode) + _tcslen(szCaretChar);
			auto_sprintf(
				szFormat,
				_T("%%s(%%s)%%%ds%%s%%s"),	// �u�L�����b�g�ʒu�̕������v���E�l�Ŕz�u�i����Ȃ��Ƃ��͍��l�ɂȂ��ĉE�ɐL�т�j
				(nLen < 15)? 15 - nLen: 1
			);
			auto_sprintf(
				szLeft,
				szFormat,
				pszCodeName,
				szEolMode,
				szCaretChar[0]? _T("["): _T(" "),	// ������񖳂��Ȃ犇�ʂ��ȗ��iEOF��t���[�J�[�\���ʒu�j
				szCaretChar,
				szCaretChar[0]? _T("]"): _T(" ")	// ������񖳂��Ȃ犇�ʂ��ȗ��iEOF��t���[�J�[�\���ʒu�j
			);
		}
		szRight[0] = _T('\0');
		nLen = MENUBAR_MESSAGE_MAX_LEN - _tcslen(szLeft);	// �E���Ɏc���Ă��镶����
		if( nLen > 0 ){	// ���b�Z�[�W�̉E��������i�u�s:��v�\���j
			TCHAR szRowCol[32];
			auto_sprintf(
				szRowCol,
				_T("%d:%-4d"),	// �u��v�͍ŏ������w�肵�č��񂹁i����Ȃ��Ƃ��͉E�ɐL�т�j
				ptCaret.y,
				ptCaret.x
			);
			auto_sprintf(
				szFormat,
				_T("%%%ds"),	// �u�s:��v���E�l�Ŕz�u�i����Ȃ��Ƃ��͍��l�ɂȂ��ĉE�ɐL�т�j
				nLen
			);
			auto_sprintf(
				szRight,
				szFormat,
				szRowCol
			);
		}
		auto_sprintf(
			szText,
			_T("%s%s"),
			szLeft,
			szRight
		);
		m_pEditDoc->m_pcEditWnd->PrintMenubarMessage( szText );
	}
	// �X�e�[�^�X�o�[�ɏ�Ԃ������o��
	else{
		TCHAR	szText_1[64];
		auto_sprintf( szText_1, _T("%5d �s %4d ��"), ptCaret.y, ptCaret.x );	//Oct. 30, 2000 JEPRO �疜�s���v���

		TCHAR	szText_6[16];
		if( m_pEditView->IsInsMode() /* Oct. 2, 2005 genta */ ){
			_tcscpy( szText_6, _T("�}��") );
		}else{
			_tcscpy( szText_6, _T("�㏑") );
		}
		::StatusBar_SetText( hwndStatusBar, 0 | SBT_NOBORDERS, _T("") );
		::StatusBar_SetText( hwndStatusBar, 1 | 0,             szText_1 );
		//	May 12, 2000 genta
		//	���s�R�[�h�̕\����ǉ��D���̔ԍ���1�����炷
		//	From Here
		::StatusBar_SetText( hwndStatusBar, 2 | 0,             szEolMode );
		//	To Here
		::StatusBar_SetText( hwndStatusBar, 3 | 0,             szCaretChar );
		::StatusBar_SetText( hwndStatusBar, 4 | 0,             pszCodeName );
		::StatusBar_SetText( hwndStatusBar, 5 | SBT_OWNERDRAW, _T("") );
		::StatusBar_SetText( hwndStatusBar, 6 | 0,             szText_6 );
	}

}

/* �J�[�\���㉺�ړ����� */
CLayoutInt CCaret::Cursor_UPDOWN( CLayoutInt nMoveLines, bool bSelect )
{
	//�K�v�ȃC���^�[�t�F�[�X
	const CLayoutMgr* pLayoutMgr=&m_pEditDoc->m_cLayoutMgr;
//	const STypeConfig* pTypes=&m_pEditDoc->m_cDocType.GetDocumentAttribute();
	CommonSetting* pCommon=&GetDllShareData().m_Common;


	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	CLayoutInt		nLineCols;
	CLayoutInt		nScrollLines;
	const CLayout*	pcLayout;
	nScrollLines = CLayoutInt(0);

	CLayoutPoint		ptPosXY(CLayoutInt(0), GetCaretLayoutPos().GetY());

	if( nMoveLines > 0 ){
		/* �J�[�\�����e�L�X�g�ŉ��[�s�ɂ��邩 */
		if( GetCaretLayoutPos().GetY() + nMoveLines >= pLayoutMgr->GetLineCount() ){
			nMoveLines = pLayoutMgr->GetLineCount() - GetCaretLayoutPos().GetY()  - 1;
		}
		if( nMoveLines <= 0 ){
			pLine = pLayoutMgr->GetLineStr( GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );
			if( NULL != pLine ){
				nLineCols = m_pEditView->LineIndexToColmn( pcLayout, nLineLen );
				/* ���s�ŏI����Ă��邩 */
				//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
				if( ( EOL_NONE != pcLayout->GetLayoutEol() )
//				if( ( pLine[ nLineLen - 1 ] == L'\n' || pLine[ nLineLen - 1 ] == L'\r' )
				 // [EOF]�̂ݐ܂�Ԃ��̂͂�߂�	// 2009.02.17 ryoji
				 //|| nLineCols >= pLayoutMgr->GetMaxLineKetas()
				){
					if( bSelect ){
						if( !m_pEditView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
							/* ���݂̃J�[�\���ʒu����I�����J�n���� */
							m_pEditView->GetSelectionInfo().BeginSelectArea();
						}
					}else{
						if( m_pEditView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
							/* ���݂̑I��͈͂��I����Ԃɖ߂� */
							m_pEditView->GetSelectionInfo().DisableSelectArea( TRUE );
						}
					}
					ptPosXY.x = CLayoutInt(0);
					++ptPosXY.y;
					nScrollLines = MoveCursor( ptPosXY, m_pEditView->GetDrawSwitch() /* TRUE */ ); // YAZAKI.
					if( bSelect ){
						/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						m_pEditView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptPosXY );
					}
				}
			}
			//	Sep. 11, 2004 genta �����X�N���[���̊֐���
			//	MoveCursor�ŃX�N���[���ʒu�����ς�
			//SyncScrollV( nScrollLines );
			return nScrollLines;
		}
	}else{
		/* �J�[�\�����e�L�X�g�ŏ�[�s�ɂ��邩 */
		if( GetCaretLayoutPos().GetY() + nMoveLines < 0 ){
			nMoveLines = - GetCaretLayoutPos().GetY();
		}
		if( nMoveLines >= 0 ){
			//	Sep. 11, 2004 genta �����X�N���[���̊֐���
			m_pEditView->SyncScrollV( nScrollLines );
			return nScrollLines;
		}
	}
	if( bSelect ){
		if( !m_pEditView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			m_pEditView->GetSelectionInfo().BeginSelectArea();
		}
	}else{
		if( m_pEditView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pEditView->GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}
	/* ���̍s�̃f�[�^���擾 */
	pLine = pLayoutMgr->GetLineStr( GetCaretLayoutPos().GetY2() + CLayoutInt(nMoveLines), &nLineLen, &pcLayout );
	CMemoryIterator it( pcLayout, pLayoutMgr->GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
			i = nLineLen;
			break;
		}
		if( it.getColumn() + it.getColumnDelta() > m_nCaretPosX_Prev ){
			i = it.getIndex();
			break;
		}
		it.addDelta();
	}
	ptPosXY.x += it.getColumn();
	if ( it.end() ){
		i = it.getIndex();
	}

	if( i >= nLineLen ){
		/* �t���[�J�[�\�����[�h�� */
		if( pCommon->m_sGeneral.m_bIsFreeCursorMode
		 || m_pEditView->GetSelectionInfo().IsTextSelected() && m_pEditView->GetSelectionInfo().IsBoxSelecting()	/* ��`�͈͑I�� */
		){
			if( GetCaretLayoutPos().GetY() + nMoveLines + 1 == pLayoutMgr->GetLineCount()  ){
				if( NULL != pLine ){
					if( pLine[nLineLen - 1] == WCODE::CR || pLine[nLineLen - 1] == WCODE::LF ){
						ptPosXY.x = m_nCaretPosX_Prev;
					}
				}
			}else{
				ptPosXY.x = m_nCaretPosX_Prev;
			}
		}
	}
	nScrollLines = MoveCursor( CLayoutPoint(ptPosXY.x, GetCaretLayoutPos().GetY() + nMoveLines), m_pEditView->GetDrawSwitch() /* TRUE */ ); // YAZAKI.
	if( bSelect ){
		/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
		m_pEditView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( CLayoutPoint(ptPosXY.x, GetCaretLayoutPos().GetY()) );
	}

	return nScrollLines;
}


/*!	�L�����b�g�̍쐬

	@param nCaretColor [in]	�L�����b�g�̐F��� (0:�ʏ�, 1:IME ON)
	@param nWidth [in]		�L�����b�g��
	@param nHeight [in]		�L�����b�g��

	@date 2006.12.07 ryoji �V�K�쐬
*/
void CCaret::CreateEditCaret( COLORREF crCaret, COLORREF crBack, int nWidth, int nHeight )
{
	//
	// �L�����b�g�p�̃r�b�g�}�b�v���쐬����
	//
	// Note: �E�B���h�E�݊��̃����� DC ��� PatBlt ��p���ăL�����b�g�F�Ɣw�i�F�� XOR ����
	//       ���邱�ƂŁC�ړI�̃r�b�g�}�b�v�𓾂�D
	//       �� 256 �F���ł� RGB �l��P���ɒ��ډ��Z���Ă��L�����b�g�F���o�����߂̐�����
	//          �r�b�g�}�b�v�F�͓����Ȃ��D
	//       �Q�l: [HOWTO] �L�����b�g�̐F�𐧌䂷����@
	//             http://support.microsoft.com/kb/84054/ja
	//

	HBITMAP hbmpCaret;	// �L�����b�g�p�̃r�b�g�}�b�v

	HDC hdc = m_pEditView->GetDC();

	hbmpCaret = ::CreateCompatibleBitmap( hdc, nWidth, nHeight );
	HDC hdcMem = ::CreateCompatibleDC( hdc );
	HBITMAP hbmpOld = (HBITMAP)::SelectObject( hdcMem, hbmpCaret );
	HBRUSH hbrCaret = ::CreateSolidBrush( crCaret );
	HBRUSH hbrBack = ::CreateSolidBrush( crBack );
	HBRUSH hbrOld = (HBRUSH)::SelectObject( hdcMem, hbrCaret );
	::PatBlt( hdcMem, 0, 0, nWidth, nHeight, PATCOPY );
	::SelectObject( hdcMem, hbrBack );
	::PatBlt( hdcMem, 0, 0, nWidth, nHeight, PATINVERT );
	::SelectObject( hdcMem, hbrOld );
	::SelectObject( hdcMem, hbmpOld );
	::DeleteObject( hbrCaret );
	::DeleteObject( hbrBack );
	::DeleteDC( hdcMem );

	m_pEditView->ReleaseDC( hdc );

	// �ȑO�̃r�b�g�}�b�v��j������
	if( m_hbmpCaret != NULL )
		::DeleteObject( m_hbmpCaret );
	m_hbmpCaret = hbmpCaret;

	// �L�����b�g���쐬����
	m_pEditView->CreateCaret( hbmpCaret, nWidth, nHeight );
	return;
}


// 2002/07/22 novice
/*!
	�L�����b�g�̕\��
*/
void CCaret::ShowCaret_( HWND hwnd )
{
	if ( m_bCaretShowFlag == false ){
		::ShowCaret( hwnd );
		m_bCaretShowFlag = true;
	}
}


/*!
	�L�����b�g�̔�\��
*/
void CCaret::HideCaret_( HWND hwnd )
{
	if ( m_bCaretShowFlag == true ){
		::HideCaret( hwnd );
		m_bCaretShowFlag = false;
	}
}

//! �����̏�Ԃ𑼂�CCaret�ɃR�s�[
void CCaret::CopyCaretStatus(CCaret* pCaret) const
{
	pCaret->SetCaretLayoutPos(GetCaretLayoutPos());
	pCaret->m_nCaretPosX_Prev = m_nCaretPosX_Prev;	/* �r���[���[����̃J�[�\�����ʒu�i�O�I���W���j*/

	//�� �L�����b�g�̃T�C�Y�̓R�s�[���Ȃ��B2002/05/12 YAZAKI
}


POINT CCaret::CalcCaretDrawPos(const CLayoutPoint& ptCaretPos) const
{
	int nPosX = m_pEditView->GetTextArea().GetAreaLeft()
		+ (Int)(ptCaretPos.x - m_pEditView->GetTextArea().GetViewLeftCol()) * GetHankakuDx();

	int nPosY = m_pEditView->GetTextArea().GetAreaTop()
		+ (Int)(ptCaretPos.y - m_pEditView->GetTextArea().GetViewTopLine()) * m_pEditView->GetTextMetrics().GetHankakuDy()
		+ m_pEditView->GetTextMetrics().GetHankakuHeight() - GetCaretSize().cy; //����

	return CMyPoint(nPosX,nPosY);
}




/*!
	�s���w��ɂ��J�[�\���ړ��i���W�����t���j

	@return �c�X�N���[���s��(��:��X�N���[��/��:���X�N���[��)

	@note �}�E�X���ɂ��ړ��ŕs�K�؂Ȉʒu�ɍs���Ȃ��悤���W�������ăJ�[�\���ړ�����

	@date 2007.08.23 ryoji �֐����iMoveCursorToPoint()���珈���𔲂��o���j
	@date 2007.09.26 ryoji ���p�����ł������ō��E�ɃJ�[�\����U�蕪����
	@date 2007.10.23 kobake ���������̌����C�� ([in/out]��[in])
	@date 2009.02.17 ryoji ���C�A�E�g�s���Ȍ�̃J�����ʒu�w��Ȃ疖�������̑O�ł͂Ȃ����������̌�Ɉړ�����
*/
CLayoutInt CCaret::MoveCursorProperly(
	CLayoutPoint	ptNewXY,			//!< [in] �J�[�\���̃��C�A�E�g���WX
	bool			bScroll,			//!< [in] TRUE: ��ʈʒu�����L��/ FALSE: ��ʈʒu�����L�薳��
	int				nCaretMarginRate,	//!< [in] �c�X�N���[���J�n�ʒu�����߂�l
	int				dx					//!< [in] nNewX�ƃ}�E�X�J�[�\���ʒu�Ƃ̌덷(�J�����������̃h�b�g��)
)
{
	const wchar_t*		pLine;
	CLogicInt		nLineLen;
	const CLayout*	pcLayout;

	if( 0 > ptNewXY.y ){
		ptNewXY.y = CLayoutInt(0);
	}
	
	/* �J�[�\�����e�L�X�g�ŉ��[�s�ɂ��邩 */
	if( ptNewXY.y >= m_pEditDoc->m_cLayoutMgr.GetLineCount() ){
		// 2004.04.03 Moca EOF�����̍��W�����́AMoveCursor���ł���Ă��炤�̂ŁA�폜
	}
	/* �J�[�\�����e�L�X�g�ŏ�[�s�ɂ��邩 */
	else if( ptNewXY.y < 0 ){
		ptNewXY.Set(CLayoutInt(0), CLayoutInt(0));
	}
	else{
		/* �ړ���̍s�̃f�[�^���擾 */
		pLine = m_pEditDoc->m_cLayoutMgr.GetLineStr( ptNewXY.GetY2(), &nLineLen, &pcLayout );

		int nColWidth = m_pEditView->GetTextMetrics().GetHankakuDx();
		CLayoutInt nPosX = CLayoutInt(0);
		int i = 0;
		CMemoryIterator it( pcLayout, m_pEditDoc->m_cLayoutMgr.GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			if ( it.getIndex() + it.getIndexDelta() > CLogicInt(pcLayout->GetLengthWithoutEOL()) ){
				i = nLineLen;
				break;
			}
			if( it.getColumn() + it.getColumnDelta() > ptNewXY.GetX2() ){
				if (ptNewXY.GetX2() >= (pcLayout ? pcLayout->GetIndent() : CLayoutInt(0)) && ((ptNewXY.GetX2() - it.getColumn()) * nColWidth + dx) * 2 >= it.getColumnDelta() * nColWidth){
				//if (ptNewXY.GetX2() >= (pcLayout ? pcLayout->GetIndent() : CLayoutInt(0)) && (it.getColumnDelta() > CLayoutInt(1)) && ((it.getColumn() + it.getColumnDelta() - ptNewXY.GetX2()) <= it.getColumnDelta() / 2)){
					nPosX += it.getColumnDelta();
				}
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();
		if ( it.end() ){
			i = it.getIndex();
			//nPosX -= it.getColumnDelta();	// 2009.02.17 ryoji �R�����g�A�E�g�i���������̌�Ɉړ�����j
		}

		if( i >= nLineLen ){
			if( ptNewXY.y +1 == m_pEditDoc->m_cLayoutMgr.GetLineCount() &&
				EOL_NONE == pcLayout->GetLayoutEol().GetLen() ){
				nPosX = m_pEditView->LineIndexToColmn( pcLayout, nLineLen );
			}
			/* �t���[�J�[�\�����[�h�� */
			else if( GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode
			  || ( m_pEditView->GetSelectionInfo().IsMouseSelecting() && m_pEditView->GetSelectionInfo().IsBoxSelecting() )	/* �}�E�X�͈͑I�� && ��`�͈͑I�� */
			  || ( m_pEditView->m_bDragMode && m_pEditView->m_bDragBoxData ) /* OLE DropTarget && ��`�f�[�^ */
			){
				// �܂�Ԃ����ƃ��C�A�E�g�s�����i�Ԃ牺�����܂ށj�̂ǂ��炩�傫���ق��܂ŃJ�[�\���ړ��\
				//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
				CLayoutInt nMaxX = __max(nPosX, m_pEditDoc->m_cLayoutMgr.GetMaxLineKetas());
				nPosX = ptNewXY.GetX2();
				if( nPosX < CLayoutInt(0) ){
					nPosX = CLayoutInt(0);
				}
				else if( nPosX > nMaxX ){
					nPosX = nMaxX;
				}
			}
		}
		ptNewXY.SetX( nPosX );
	}

	return MoveCursor( ptNewXY, bScroll, nCaretMarginRate );
}
