#include "stdafx.h"
#include "CEditView_Paint.h"
#include "view/CEditView.h"
#include <vector>
#include "CTypeSupport.h"
#include "doc/CLayout.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"

/*
	PAINT_LINENUMBER = (1<<0), //!< �s�ԍ�
	PAINT_RULER      = (1<<1), //!< ���[���[
	PAINT_BODY       = (1<<2), //!< �{��
*/

void CEditView_Paint::Call_OnPaint(
	int nPaintFlag,   //!< �`�悷��̈��I������
	bool bUseMemoryDC //!< ������DC���g�p����
)
{
	CEditView* pView = GetEditView();

	//�e�v�f
	CMyRect rcLineNumber(0,pView->GetTextArea().GetAreaTop(),pView->GetTextArea().GetAreaLeft(),pView->GetTextArea().GetAreaBottom());
	CMyRect rcRuler(pView->GetTextArea().GetAreaLeft(),0,pView->GetTextArea().GetAreaRight(),pView->GetTextArea().GetAreaTop());
	CMyRect rcBody(pView->GetTextArea().GetAreaLeft(),pView->GetTextArea().GetAreaTop(),pView->GetTextArea().GetAreaRight(),pView->GetTextArea().GetAreaBottom());

	//�̈���쐬 -> rc
	std::vector<CMyRect> rcs;
	if(nPaintFlag & PAINT_LINENUMBER)rcs.push_back(rcLineNumber);
	if(nPaintFlag & PAINT_RULER)rcs.push_back(rcRuler);
	if(nPaintFlag & PAINT_BODY)rcs.push_back(rcBody);
	if(rcs.size()==0)return;
	CMyRect rc=rcs[0];
	for(int i=1;i<(int)rcs.size();i++)
		rc=MergeRect(rc,rcs[i]);

	//�`��
	PAINTSTRUCT	ps;
	ps.rcPaint = rc;
	HDC hdc = pView->GetDC();
	pView->OnPaint( hdc, &ps, bUseMemoryDC );
	pView->ReleaseDC( hdc );
}



/* �t�H�[�J�X�ړ����̍ĕ`��

	@date 2001/06/21 asa-o �u�X�N���[���o�[�̏�Ԃ��X�V����v�u�J�[�\���ړ��v�폜
*/
void CEditView::RedrawAll()
{
	// �E�B���h�E�S�̂��ĕ`��
	PAINTSTRUCT	ps;
	HDC hdc = ::GetDC( GetHwnd() );
	::GetClientRect( GetHwnd(), &ps.rcPaint );
	OnPaint( hdc, &ps, FALSE );	// �������c�b���g�p���Ă�����̂Ȃ��ĕ`��
	::ReleaseDC( GetHwnd(), hdc );

	// �L�����b�g�̕\��
	GetCaret().ShowEditCaret();

	// �L�����b�g�̍s���ʒu��\������
	GetCaret().ShowCaretPosInfo();

	// �e�E�B���h�E�̃^�C�g�����X�V
	m_pcEditWnd->UpdateCaption();

	//	Jul. 9, 2005 genta	�I��͈͂̏����X�e�[�^�X�o�[�֕\��
	GetSelectionInfo().PrintSelectionInfoMsg();

	// �X�N���[���o�[�̏�Ԃ��X�V����
	AdjustScrollBars();
}

// 2001/06/21 Start by asa-o �ĕ`��
void CEditView::Redraw()
{
	HDC			hdc;
	PAINTSTRUCT	ps;

	hdc = ::GetDC( GetHwnd() );

	::GetClientRect( GetHwnd(), &ps.rcPaint );

	OnPaint( hdc, &ps, FALSE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */

	::ReleaseDC( GetHwnd(), hdc );
}
// 2001/06/21 End



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@date 2003/02/18 ai
	@param flag [in] ���[�h(true:�o�^, false:����)
*/
void CEditView::SetBracketPairPos( bool flag )
{
	int	mode;

	// 03/03/06 ai ���ׂĒu���A���ׂĒu�����Undo&Redo�����Ȃ�x�����ɑΉ�
	if( m_bDoing_UndoRedo || !GetDrawSwitch() ){
		return;
	}

	if( !m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// �Ί��ʂ̌���&�o�^
	/*
	bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
	bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
	bit2(out) : ���������ʒu         0:���      1:�O
	*/
	mode = 2;

	CLayoutPoint ptColLine;

	if( flag && !GetSelectionInfo().IsTextSelected() && !GetSelectionInfo().m_bDrawSelectArea
		&& !GetSelectionInfo().IsBoxSelecting() && SearchBracket( GetCaret().GetCaretLayoutPos(), &ptColLine, &mode ) )
	{
		// �o�^�w��(flag=true)			&&
		// �e�L�X�g���I������Ă��Ȃ�	&&
		// �I��͈͂�`�悵�Ă��Ȃ�		&&
		// ��`�͈͑I�𒆂łȂ�			&&
		// �Ή����銇�ʂ���������		�ꍇ
		if ( ( ptColLine.x >= GetTextArea().GetViewLeftCol() ) && ( ptColLine.x <= GetTextArea().GetRightCol() )
			&& ( ptColLine.y >= GetTextArea().GetViewTopLine() ) && ( ptColLine.y <= GetTextArea().GetBottomLine() ) )
		{
			// �\���̈���̏ꍇ

			// ���C�A�E�g�ʒu���畨���ʒu�֕ϊ�(�����\���ʒu��o�^)
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptColLine, &m_ptBracketPairPos_PHY );
			m_ptBracketCaretPos_PHY.y = GetCaret().GetCaretLogicPos().y;
			if( 0 == ( mode & 4 ) ){
				// �J�[�\���̌�������ʒu
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x;
			}else{
				// �J�[�\���̑O�������ʒu
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x - 1;
			}
			return;
		}
	}

	// ���ʂ̋����\���ʒu��񏉊���
	m_ptBracketPairPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));
	m_ptBracketCaretPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));

	return;
}

/*!
	�Ί��ʂ̋����\��
	@date 2002/09/18 ai
	@date 2003/02/18 ai �ĕ`��Ή��̈ב����
*/
void CEditView::DrawBracketPair( bool bDraw )
{
	// 03/03/06 ai ���ׂĒu���A���ׂĒu�����Undo&Redo�����Ȃ�x�����ɑΉ�
	if( m_bDoing_UndoRedo || !GetDrawSwitch() ){
		return;
	}

	if( !m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// ���ʂ̋����\���ʒu�����o�^�̏ꍇ�͏I��
	if( m_ptBracketPairPos_PHY.HasNegative() || m_ptBracketCaretPos_PHY.HasNegative() ){
		return;
	}

	// �`��w��(bDraw=true)				����
	// ( �e�L�X�g���I������Ă���		����
	//   �I��͈͂�`�悵�Ă���			����
	//   ��`�͈͑I��					����
	//   �t�H�[�J�X�������Ă��Ȃ�		����
	//   �A�N�e�B�u�ȃy�C���ł͂Ȃ� )	�ꍇ�͏I��
	if( bDraw
	 &&( GetSelectionInfo().IsTextSelected() || GetSelectionInfo().m_bDrawSelectArea || GetSelectionInfo().IsBoxSelecting() || !m_bDrawBracketPairFlag
	 || ( m_pcEditDoc->m_pcEditWnd->m_nActivePaneIndex != m_nMyIndex ) ) ){
		return;
	}

	HDC			hdc;
	hdc = ::GetDC( GetHwnd() );
	Types *TypeDataPtr = &( m_pcEditDoc->m_cDocType.GetDocumentAttribute() );

	for( int i = 0; i < 2; i++ )
	{
		// i=0:�J�[�\���ʒu�̊���,i=1:�Ί���

		CLayoutPoint	ptColLine;

		if( i == 0 ){
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketCaretPos_PHY, &ptColLine );
		}else{
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketPairPos_PHY,  &ptColLine );
		}

		if ( ( ptColLine.x >= GetTextArea().GetViewLeftCol() ) && ( ptColLine.x <= GetTextArea().GetRightCol() )
			&& ( ptColLine.y >= GetTextArea().GetViewTopLine() ) && ( ptColLine.y <= GetTextArea().GetBottomLine() ) )
		{	// �\���̈���̏ꍇ
			if( !bDraw && GetSelectionInfo().m_bDrawSelectArea && ( 0 == IsCurrentPositionSelected( ptColLine ) ) )
			{	// �I��͈͕`��ς݂ŏ����Ώۂ̊��ʂ��I��͈͓��̏ꍇ
				continue;
			}
			const CLayout* pcLayout;
			CLogicInt		nLineLen;
			const wchar_t*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( ptColLine.GetY2(), &nLineLen, &pcLayout );
			if( pLine )
			{
				int		nColorIndex;
				CLogicInt	OutputX = LineColmnToIndex( pcLayout, ptColLine.GetX2() );
				if( bDraw )	{
					nColorIndex = COLORIDX_BRACKET_PAIR;
				}
				else{
					if( IsBracket( pLine, OutputX, CLogicInt(1) ) ){
						// 03/10/24 ai �܂�Ԃ��s��ColorIndex���������擾�ł��Ȃ����ɑΉ�
						if( i == 0 ){
							nColorIndex = GetColorIndex( hdc, pcLayout, m_ptBracketCaretPos_PHY.x );
						}else{
							nColorIndex = GetColorIndex( hdc, pcLayout, m_ptBracketPairPos_PHY.x );
						}
					}
					else{
						SetBracketPairPos( false );
						break;
					}
				}


				m_hFontOld = NULL;

				//�F�ݒ�
				CTypeSupport cTextType(this,COLORIDX_TEXT);
				cTextType.SetFont(hdc);
				cTextType.SetColors(hdc);

				SetCurrentColor( hdc, nColorIndex );

				int nHeight = GetTextMetrics().GetHankakuDy();
				int nLeft = (GetTextArea().GetDocumentLeftClientPointX()) + (Int)ptColLine.x * GetTextMetrics().GetHankakuDx();
				int nTop  = (Int)( ptColLine.GetY2() - GetTextArea().GetViewTopLine() ) * nHeight + GetTextArea().GetAreaTop();

				// 03/03/03 ai �J�[�\���̍��Ɋ��ʂ����芇�ʂ������\������Ă����Ԃ�Shift+���őI���J�n�����
				//             �I��͈͓��ɔ��]�\������Ȃ�������������̏C��
				if( ptColLine.x == GetCaret().GetCaretLayoutPos().GetX2() && GetCaret().GetCaretShowFlag() ){
					GetCaret().HideCaret_( GetHwnd() );	// �L�����b�g����u������̂�h�~
					GetTextDrawer().DispText( hdc, nLeft, nTop, &pLine[OutputX], 1 );
					// 2006.04.30 Moca �Ί��ʂ̏c���Ή�
					GetTextDrawer().DispVerticalLines( hdc, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + CLayoutInt(2) ); //�����ʂ��S�p���ł���ꍇ���l��
					GetCaret().ShowCaret_( GetHwnd() );	// �L�����b�g����u������̂�h�~
				}
				else{
					GetTextDrawer().DispText( hdc, nLeft, nTop, &pLine[OutputX], 1 );
					// 2006.04.30 Moca �Ί��ʂ̏c���Ή�
					GetTextDrawer().DispVerticalLines( hdc, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + CLayoutInt(2) ); //�����ʂ��S�p���ł���ꍇ���l��
				}

				if( NULL != m_hFontOld ){
					::SelectObject( hdc, m_hFontOld );
					m_hFontOld = NULL;
				}

				cTextType.RewindFont(hdc);
				cTextType.RewindColors(hdc);

				if( ( m_pcEditDoc->m_pcEditWnd->m_nActivePaneIndex == m_nMyIndex )
					&& ( ( ptColLine.y == GetCaret().GetCaretLayoutPos().GetY() ) || ( ptColLine.y - 1 == GetCaret().GetCaretLayoutPos().GetY() ) ) ){	// 03/02/27 ai �s�̊Ԋu��"0"�̎��ɃA���_�[���C���������鎖������׏C��
					GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
				}
			}
		}
	}

	::ReleaseDC( GetHwnd(), hdc );
}


//======================================================================
//!�Ί��ʂ̑Ή��\
//2007.10.16 kobake
struct KAKKO_T{
	wchar_t *sStr;
	wchar_t *eStr;
};
const KAKKO_T g_aKakkos[] = {
	//���p
	{ L"(", L")", },
	{ L"[", L"]", },
	{ L"{", L"}", },
	{ L"<", L">", },
	{ L"�", L"�", },
	//�S�p
	{ L"�y", L"�z", },
	{ L"�w", L"�x", },
	{ L"�u", L"�v", },
	{ L"��", L"��", },
	{ L"��", L"��", },
	{ L"�s", L"�t", },
	{ L"�i", L"�j", },
	{ L"�q", L"�r", },
	{ L"�o", L"�p", },
	{ L"�k", L"�l", },
	{ L"�m", L"�n", },
	{ L"�g", L"�h", },
	{ L"��", L"��", },
	//�I�[
	{ NULL, NULL, },
};



//	Jun. 16, 2000 genta
/*!
	@brief �Ί��ʂ̌���

	�J�[�\���ʒu�̊��ʂɑΉ����銇�ʂ�T���B�J�[�\���ʒu�����ʂłȂ��ꍇ��
	�J�[�\���̌��̕��������ʂ��ǂ����𒲂ׂ�B

	�J�[�\���̑O�ア����������ʂłȂ��ꍇ�͉������Ȃ��B

	���ʂ����p���S�p���A�y�юn�܂肩�I��肩�ɂ���Ă���ɑ���4�̊֐���
	������ڂ��B

	@param LayoutX [in] �����J�n�_�̕������WX
	@param LayoutY [in] �����J�n�_�̕������WY
	@param NewX [out] �ړ���̃��C�A�E�g���WX
	@param NewY [out] �ړ���̃��C�A�E�g���WY
	@param mode [in/out] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
						 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ� (����bit���Q��)
						 bit2(out) : ���������ʒu         0:���      1:�O     (����bit���X�V)

	@retval true ����
	@retval false ���s

	@author genta
	@date Jun. 16, 2000 genta
	@date Feb. 03, 2001 MIK �S�p���ʂɑΉ�
	@date Sep. 18, 2002 ai mode�̒ǉ�
*/
bool CEditView::SearchBracket(
	const CLayoutPoint&	ptLayout,
	CLayoutPoint*		pptLayoutNew,
	int*				mode
)
{
	CLogicInt len;	//	�s�̒���
	
	CLogicPoint ptPos;
	//int PosX, PosY;	//	�����ʒu

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptLayout, &ptPos );
	const wchar_t *cline = m_pcEditDoc->m_cDocLineMgr.GetLine(ptPos.GetY2())->GetDocLineStrWithEOL(&len);

	//	Jun. 19, 2000 genta
	if( cline == NULL )	//	�Ō�̍s�ɖ{�����Ȃ��ꍇ
		return false;

	// ���ʏ��� 2007.10.16 kobake
	{
		const KAKKO_T* p;
		for( p = g_aKakkos; p->sStr != NULL;  p++ )
		{
			if( wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketForward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
			else if( wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketBackward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
		}
	}

	// 02/09/18 ai Start
	if( 0 == ( *mode & 2 ) ){
		/* �J�[�\���̑O���𒲂ׂȂ��ꍇ */
		return false;
	}
	*mode |= 4;
	// 02/09/18 ai End

	//	���ʂ�������Ȃ�������C�J�[�\���̒��O�̕����𒲂ׂ�

	if( ptPos.x <= 0 ){
//		::MessageBoxA( NULL, "NO DATA", "Bracket", MB_OK );
		return false;	//	�O�̕����͂Ȃ�
	}

	const wchar_t *bPos = CNativeW::GetCharPrev( cline, ptPos.x, cline + ptPos.x );
	int nCharSize = cline + ptPos.x - bPos;
	// ���ʏ��� 2007.10.16 kobake
	if(nCharSize==1){
		const KAKKO_T* p;
		ptPos.x = bPos - cline;
		for( p = g_aKakkos; p->sStr != NULL; p++ )
		{
			if( wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketForward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
			else if( wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketBackward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
		}
	}
	return false;
}

/*!
	@brief ���p�Ί��ʂ̌���:������

	@author genta

	@param PosX   [in] �����J�n�_�̕������WX
	@param PosY   [in] �����J�n�_�̕������WY
	@param NewX   [out] �ړ���̃��C�A�E�g���WX
	@param NewY   [out] �ړ���̃��C�A�E�g���WY
	@param upChar [in] ���ʂ̎n�܂�̕���
	@param dnChar [in] ���ʂ���镶����
	@param mode   [in] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ� (����bit���Q��)
					 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
					 bit2(out) : ���������ʒu         0:���      1:�O

	@retval true ����
	@retval false ���s
*/
// 03/01/08 ai
bool CEditView::SearchBracketForward(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	upChar,
	const wchar_t*	dnChar,
	int*			mode
)
{
	CDocLine* ci;

	int			len;
	const wchar_t* cPos;
	const wchar_t* nPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int			level = 0;

	CLayoutPoint ptColLine;

	CLayoutInt	nSearchNum;	// 02/09/19 ai

	//	�����ʒu�̐ݒ�
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ( GetTextArea().GetBottomLine() ) - ptColLine.y;					// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos < lineend ){
			nPos = CNativeW::GetCharNext( cline, len, cPos );
			if( nPos - cPos > 1 ){
				//	skip
				cPos = nPos;
				continue;
			}
			// 03/01/08 ai Start
			if( wcsncmp(upChar, cPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, cPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	���������I
				ptPos.x = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
				//	Happy Ending
			}
			cPos = nPos;	//	���̕�����
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// �\���̈�O�𒲂ׂȂ����[�h�ŕ\���̈�̏I�[�̏ꍇ
			//SendStatusMessage( "�Ί��ʂ̌����𒆒f���܂���" );
			break;
		}
		// 02/09/19 ai End

		//	���̍s��
		ptPos.y++;
		ci = ci->GetNextLine();	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}

/*!
	@brief ���p�Ί��ʂ̌���:�t����

	@author genta

	@param PosX [in] �����J�n�_�̕������WX
	@param PosY [in] �����J�n�_�̕������WY
	@param NewX [out] �ړ���̃��C�A�E�g���WX
	@param NewY [out] �ړ���̃��C�A�E�g���WY
	@param upChar [in] ���ʂ̎n�܂�̕���
	@param dnChar [in] ���ʂ���镶����
	@param mode [in] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ� (����bit���Q��)
					 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
					 bit2(out) : ���������ʒu         0:���      1:�O

	@retval true ����
	@retval false ���s
*/
bool CEditView::SearchBracketBackward(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	dnChar,
	const wchar_t*	upChar,
	int*			mode
)
{
	CDocLine* ci;

	int			len;
	const wchar_t* cPos;
	const wchar_t* pPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int			level = 1;
	
	CLayoutPoint ptColLine;
	//int			nCol, nLine;

	CLayoutInt		nSearchNum;	// 02/09/19 ai

	//	�����ʒu�̐ݒ�
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ptColLine.y - GetTextArea().GetViewTopLine();										// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos > cline ){
			pPos = CNativeW::GetCharPrev( cline, len, cPos );
			if( cPos - pPos > 1 ){
				//	skip
				cPos = pPos;
				continue;
			}
			// 03/01/08 ai Start
			if( wcsncmp(upChar, pPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, pPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	���������I
				ptPos.x = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
				//	Happy Ending
			}
			cPos = pPos;	//	���̕�����
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// �\���̈�O�𒲂ׂȂ����[�h�ŕ\���̈�̐擪�̏ꍇ
			//SendStatusMessage( "�Ί��ʂ̌����𒆒f���܂���" );
			break;
		}
		// 02/09/19 ai End

		//	���̍s��
		ptPos.y--;
		ci = ci->GetPrevLine();	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}

//@@@ 2001.02.03 Start by MIK:
/*!
	@brief �S�p�Ί��ʂ̌���:������

	@author MIK

	@param PosX [in] �����J�n�_�̕������WX
	@param PosY [in] �����J�n�_�̕������WY
	@param NewX [out] �ړ���̃��C�A�E�g���WX
	@param NewY [out] �ړ���̃��C�A�E�g���WY
	@param upChar [in] ���ʂ̎n�܂�̕����ւ̃|�C���^
	@param dnChar [in] ���ʂ���镶����ւ̃|�C���^
	@param mode [in] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ� (����bit���Q��)
					 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
					 bit2(out) : ���������ʒu         0:���      1:�O

	@retval true ����
	@retval false ���s
*/
bool CEditView::SearchBracketForward2(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	upChar,
	const wchar_t*	dnChar,
	int*			mode
)
{
	CDocLine* ci;

	int len;
	const wchar_t* cPos;
	const wchar_t* nPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int level = 0;
	
	CLayoutPoint ptColLine;
//	int			nCol, nLine;

	CLayoutInt	nSearchNum;	// 02/09/19 ai

	//	�����ʒu�̐ݒ�
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ( GetTextArea().GetBottomLine() ) - ptColLine.y;					// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos < lineend ){
			nPos = CNativeW::GetCharNext( cline, len, cPos );
			if( nPos - cPos != 1 ){
				//	skip
				cPos = nPos;
				continue;
			}
			if( wcsncmp(upChar, cPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, cPos, 1) == 0 ){
				--level;
			}

			if( level == 0 ){	//	���������I
				ptPos.x = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
			}
			cPos = nPos;	//	���̕�����
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// �\���̈�O�𒲂ׂȂ����[�h�ŕ\���̈�̏I�[�̏ꍇ
			//SendStatusMessage( "�Ί��ʂ̌����𒆒f���܂���" );
			break;
		}
		// 02/09/19 ai End

		//	���̍s��
		ptPos.y++;
		ci = ci->GetNextLine();	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}
//@@@ 2001.02.03 End

//@@@ 2001.02.03 Start by MIK:
/*!
	@brief �S�p�Ί��ʂ̌���:�t����

	@author MIK

	@param PosX [in] �����J�n�_�̕������WX
	@param PosY [in] �����J�n�_�̕������WY
	@param NewX [out] �ړ���̃��C�A�E�g���WX
	@param NewY [out] �ړ���̃��C�A�E�g���WY
	@param upChar [in] ���ʂ̎n�܂�̕����ւ̃|�C���^
	@param dnChar [in] ���ʂ���镶����ւ̃|�C���^
	@param mode [in] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ� (����bit���Q��)
					 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
					 bit2(out) : ���������ʒu         0:���      1:�O

	@retval true ����
	@retval false ���s
*/
bool CEditView::SearchBracketBackward2(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	dnChar,
	const wchar_t*	upChar,
	int*			mode
)
{
	CDocLine* ci;

	int len;
	const wchar_t* cPos;
	const wchar_t* pPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int level = 1;

	CLayoutPoint ptColLine;
	//int	nCol, nLine;

	CLayoutInt nSearchNum;	// 02/09/19 ai

	//	�����ʒu�̐ݒ�
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ptColLine.y - GetTextArea().GetViewTopLine();										// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos > cline ){
			pPos = CNativeW::GetCharPrev( cline, len, cPos );
			if( cPos - pPos != 1 ){
				//	skip
				cPos = pPos;
				continue;
			}
			if( wcsncmp(upChar, pPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, pPos, 1) == 0 ){
				--level;
			}

			if( level == 0 ){	//	���������I
				ptPos.x = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
			}
			cPos = pPos;	//	���̕�����
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// �\���̈�O�𒲂ׂȂ����[�h�ŕ\���̈�̐擪�̏ꍇ
			//SendStatusMessage( "�Ί��ʂ̌����𒆒f���܂���" );
			break;
		}
		// 02/09/19 ai End

		//	���̍s��
		ptPos.y--;
		ci = ci->GetPrevLine();	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}
//@@@ 2001.02.03 End

//@@@ 2003.01.09 Start by ai:
/*!
	@brief ���ʔ���

	@author ai

	@param pLine [in] 
	@param x
	@param size

	@retval true ����
	@retval false �񊇌�
*/
bool CEditView::IsBracket( const wchar_t *pLine, CLogicInt x, CLogicInt size )
{
	// ���ʏ��� 2007.10.16 kobake
	if( size == 1 ){
		const struct KAKKO_T *p;
		for( p = g_aKakkos; p->sStr != NULL; p++ )
		{
			if( wcsncmp( p->sStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
			else if( wcsncmp( p->eStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
		}
	}

	return false;
}
//@@@ 2003.01.09 End


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �F�ݒ�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! �w��ʒu��ColorIndex�̎擾
	CEditView::DispLineNew�����ɂ�������CEditView::DispLineNew��
	�C�����������ꍇ�́A�������C�����K�v�B

	@par nCOMMENTMODE
	�֐������ŏ�ԑJ�ڂ̂��߂Ɏg����ϐ�nCOMMENTMODE�Ə�Ԃ̊֌W�B
 - COLORIDX_TEXT     : �e�L�X�g
 - COLORIDX_COMMENT  : �s�R�����g
 - COLORIDX_BLOCK1   : �u���b�N�R�����g1
 - COLORIDX_SSTRING  : �V���O���R�[�e�[�V����
 - COLORIDX_WSTRING  : �_�u���R�[�e�[�V����
 - COLORIDX_KEYWORD1 : �����L�[���[�h1
 - COLORIDX_CTRLCODE : �R���g���[���R�[�h
 - COLORIDX_DIGIT    : ���p���l
 - COLORIDX_BLOCK2   : �u���b�N�R�����g2
 - COLORIDX_KEYWORD2 : �����L�[���[�h2
 - COLORIDX_URL      : URL
 - COLORIDX_SEARCH   : ����
 - 1000: ���K�\���L�[���[�h
 	�F�w��SetCurrentColor���ĂԂƂ���COLORIDX_*�l�����Z����̂ŁA
 	1000�`COLORIDX_LAST�܂ł͐��K�\���Ŏg�p����B
*/
int CEditView::GetColorIndex(
		HDC						hdc,
		const CLayout*			pcLayout,
		int						nCol
)
{
	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->m_cDocType.GetDocumentAttribute());

	const wchar_t*			pLine;	//@@@ 2002.09.22 YAZAKI
	CLogicInt				nLineLen;
	int						nLineBgn;
	int						nCOMMENTMODE;
	int						nCOMMENTMODE_OLD;
	int						nCOMMENTEND;
	int						nCOMMENTEND_OLD;
	const CLayout*			pcLayout2;
	int						i, j;
	int						nIdx;
	int						nUrlLen;
	BOOL					bSearchStringMode;
	bool					bSearchFlg;			// 2002.02.08 hor
	bool					bKeyWordTop = true;	//	Keyword Top
	int						nColorIndex;

//@@@ 2001.11.17 add start MIK
	int		nMatchLen;
	int		nMatchColor;
//@@@ 2001.11.17 add end MIK

	bSearchStringMode = FALSE;
	bSearchFlg	= true;	// 2002.02.08 hor

	CLogicInt	nSearchStart = CLogicInt(-1);
	CLogicInt	nSearchEnd   = CLogicInt(-1);

	/* �_���s�f�[�^�̎擾 */
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory�ύX
		nLineLen = pcLayout->GetDocLineRef()->GetLengthWithEOL()/* - pcLayout->GetLogicOffset()*/;	// 03/10/24 ai �܂�Ԃ��s��ColorIndex���������擾�ł��Ȃ����ɑΉ�
		pLine = pcLayout->GetPtr()/* + pcLayout->GetLogicOffset()*/;			// 03/10/24 ai �܂�Ԃ��s��ColorIndex���������擾�ł��Ȃ����ɑΉ�

		// 2005.11.20 Moca �F���������Ȃ����Ƃ�������ɑΏ�
		const CLayout* pcLayoutLineFirst = pcLayout;
		// �_���s�̍ŏ��̃��C�A�E�g�����擾����
		while( 0 != pcLayoutLineFirst->GetLogicOffset() ){
			pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
		}
		nCOMMENTMODE = pcLayoutLineFirst->GetColorTypePrev();
		nCOMMENTEND = 0;
		pcLayout2 = pcLayout;

	}else{
		pLine = NULL;
		nLineLen = CLogicInt(0);
		nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		nCOMMENTEND = 0;
		pcLayout2 = NULL;
	}

	/* ���݂̐F���w�� */
	//@SetCurrentColor( hdc, nCOMMENTMODE );
	nColorIndex = nCOMMENTMODE;	// 02/12/18 ai

	int						nBgn;
	CLogicInt				nPos;
	nBgn = 0;
	nPos = CLogicInt(0);
	nLineBgn = 0;

	CLogicInt				nCharChars = CLogicInt(0);
	CLogicInt				nCharChars_2;

	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		while( nPos <= nCol ){	// 03/10/24 ai �s����ColorIndex���擾�ł��Ȃ����ɑΉ�

			nBgn = nPos;
			nLineBgn = nBgn;

			while( nPos - nLineBgn <= nCol ){	// 02/12/18 ai
				/* ����������̐F���� */
				if( m_bCurSrchKeyMark	/* ����������̃}�[�N */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
				// 2002.02.08 hor ���K�\���̌���������}�[�N������������
					if(!bSearchStringMode && (!m_sCurSearchOption.bRegularExp || (bSearchFlg && nSearchStart < nPos))){
						bSearchFlg=IsSearchString( pLine, nLineLen, nPos, &nSearchStart, &nSearchEnd );
					}
					if( !bSearchStringMode && bSearchFlg && nSearchStart==nPos ){
						nBgn = nPos;
						bSearchStringMode = TRUE;
						/* ���݂̐F���w�� */
						//@SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
						nColorIndex = COLORIDX_SEARCH;	// 02/12/18 ai
					}
					else if( bSearchStringMode && nSearchEnd == nPos ){
						nBgn = nPos;
						/* ���݂̐F���w�� */
						//@SetCurrentColor( hdc, nCOMMENTMODE );
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						bSearchStringMode = FALSE;
						goto searchnext;
					}
				}

				if( nPos >= nLineLen - pcLayout2->GetLayoutEol().GetLen() ){
					goto end_of_line;
				}
				SEARCH_START:;
				switch( nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
//@@@ 2001.11.17 add start MIK
					//���K�\���L�[���[�h
					if( TypeDataPtr->m_bUseRegexKeyword
					 && m_cRegexKeyword->RegexIsKeyword( pLine, nPos, nLineLen, &nMatchLen, &nMatchColor )
					 /*&& TypeDataPtr->m_ColorInfoArr[nMatchColor].m_bDisp*/ )
					{
						/* ���݂̐F���w�� */
						nBgn = nPos;
						nCOMMENTMODE = MakeColorIndexType_RegularExpression(nMatchColor);	/* �F�w�� */	//@@@ 2002.01.04 upd
						nCOMMENTEND = nPos + nMatchLen;  /* �L�[���[�h������̏I�[���Z�b�g���� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}
					else
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;

						nCOMMENTMODE = COLORIDX_COMMENT;	/* �s�R�����g�ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}else
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI

//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
					}else
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
//#endif
					}else
					if( pLine[nPos] == L'\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* �V���O���N�H�[�e�[�V�����������\������ */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_SSTRING;	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( pLine[nPos] == L'"' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_WSTRING;	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URL��\������ */
					 && ( TRUE == IsURL( &pLine[nPos], nLineLen - nPos, &nUrlLen ) )	/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_URL;	/* URL���[�h */ // 2002/03/13 novice
						nCOMMENTEND = nPos + nUrlLen;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 Start by MIK: ���p���l�������\��
					}else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (i = IsNumber( pLine, nPos, nLineLen )) > 0 )		/* ���p������\������ */
					{
						/* �L�[���[�h������̏I�[���Z�b�g���� */
						i = nPos + i;
						/* ���݂̐F���w�� */
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_DIGIT;	/* ���p���l�ł��� */ // 2002/03/13 novice
						nCOMMENTEND = i;
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 End by MIK: ���p���l�������\��
					}else
					if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* �L�[���[�h�Z�b�g */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* �����L�[���[�h��\������ */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						//	Mar 4, 2001 genta comment out
						/* �L�[���[�h������̏I�[��T�� */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							if( IS_KEYWORD_CHAR( pLine[i] ) ){
							}else{
								break;
							}
						}
						/* �L�[���[�h���o�^�P��Ȃ�΁A�F��ς��� */
						j = i - nPos;
						/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */
						nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
							TypeDataPtr->m_nKeyWordSetIdx[0],
							&pLine[nPos],
							j
						);
						if( nIdx != -1 ){
							/* ���݂̐F���w�� */
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_KEYWORD1;	/* �����L�[���[�h1 */ // 2002/03/13 novice
							nCOMMENTEND = i;
							if( !bSearchStringMode ){
								//@SetCurrentColor( hdc, nCOMMENTMODE );
								nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
							}
						}else{		//MIK START ADD 2000.12.01 second keyword & binary search
							// 2005.01.13 MIK �����L�[���[�h���ǉ��ɔ����z��
							for( int my_i = 1; my_i < 10; my_i++ )
							{
								if(TypeDataPtr->m_nKeyWordSetIdx[my_i] != -1 && /* �L�[���[�h�Z�b�g */							//MIK 2000.12.01 second keyword
									TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + my_i].m_bDisp)									//MIK
								{																							//MIK
									/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */						//MIK
									nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
										TypeDataPtr->m_nKeyWordSetIdx[my_i] ,													//MIK
										&pLine[nPos],																		//MIK
										j																					//MIK
									);																						//MIK
									if( nIdx != -1 ){																		//MIK
										/* ���݂̐F���w�� */																//MIK
										nBgn = nPos;																		//MIK
										nCOMMENTMODE = COLORIDX_KEYWORD1 + my_i;	/* �����L�[���[�h2 */ // 2002/03/13 novice		//MIK
										nCOMMENTEND = i;																	//MIK
										if( !bSearchStringMode ){															//MIK
											//@SetCurrentColor( hdc, nCOMMENTMODE );										//MIK
											nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
										}																					//MIK
										break;
									}																						//MIK
								}																							//MIK
								else
								{
									if(TypeDataPtr->m_nKeyWordSetIdx[my_i] == -1 )
										break;
								}
							}
						}			//MIK END
					}
					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pLine[nPos] ))	bKeyWordTop = false;
					else								bKeyWordTop = true;
					//	To Here
					break;
// 2002/03/13 novice
				case COLORIDX_URL:		/* URL���[�h�ł��� */
				case COLORIDX_KEYWORD1:	/* �����L�[���[�h1 */
				case COLORIDX_DIGIT:	/* ���p���l�ł��� */  //@@@ 2001.02.17 by MIK
				case COLORIDX_KEYWORD2:	/* �����L�[���[�h2 */	//MIK
				case COLORIDX_KEYWORD3:
				case COLORIDX_KEYWORD4:
				case COLORIDX_KEYWORD5:
				case COLORIDX_KEYWORD6:
				case COLORIDX_KEYWORD7:
				case COLORIDX_KEYWORD8:
				case COLORIDX_KEYWORD9:
				case COLORIDX_KEYWORD10:
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* �R���g���[���R�[�h */ // 2002/03/13 novice
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = nCOMMENTMODE_OLD;
						nCOMMENTEND = nCOMMENTEND_OLD;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;

				case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_BLOCK2:	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_SSTRING:	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_WSTRING:	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 ){	//���K�\���L�[���[�h1�`10
						if( nPos == nCOMMENTEND ){
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								//@SetCurrentColor( hdc, nCOMMENTMODE );
								nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}
				if( pLine[nPos] == WCODE::TAB ){
					nBgn = nPos + 1;
					nCharChars = CLogicInt(1);
				}
				else if( WCODE::isZenkakuSpace(pLine[nPos]) && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )	//@@@ 2002.01.04
				{
					nBgn = nPos + 1;
					nCharChars = CLogicInt(1);
				}
				//���p�󔒁i���p�X�y�[�X�j��\�� 2002.04.28 Add by KK 
				else if (pLine[nPos] == L' ' && CTypeSupport(this,COLORIDX_SPACE).IsDisp() && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{
					nBgn = nPos + 1;
					nCharChars = CLogicInt(1);
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = CLogicInt(1);
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* �R���g���[���R�[�h��F���� */
					 && WCODE::isControlCode(pLine[nPos])
					){
						nBgn = nPos;
						nCOMMENTMODE_OLD = nCOMMENTMODE;
						nCOMMENTEND_OLD = nCOMMENTEND;
						nCOMMENTMODE = COLORIDX_CTRLCODE;	/* �R���g���[���R�[�h ���[�h */ // 2002/03/13 novice
						/* �R���g���[���R�[�h��̏I�[��T�� */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if(!WCODE::isControlCode(pLine[i])){
								break;
							}
						}
						nCOMMENTEND = i;
						/* ���݂̐F���w�� */
						//@SetCurrentColor( hdc, nCOMMENTMODE );
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
					}
				}
				nPos+= nCharChars;
			} //end of while( nPos - nLineBgn < pcLayout2->m_nLength ){
			if( nPos > nCol ){	// 03/10/24 ai �s����ColorIndex���擾�ł��Ȃ����ɑΉ�
				break;
			}
		}

end_of_line:;

	}

//@end_of_func:;
	return nColorIndex;
}


/* ���݂̐F���w�� */
void CEditView::SetCurrentColor( HDC hdc, int nCOMMENTMODE )
{
	int				nColorIdx;
	COLORREF		colText;
	COLORREF		colBack;
//	if( NULL != m_hFontOld ){
//		::SelectObject( hdc, m_hFontOld );
//		m_hFontOld = NULL;
//	}
	nColorIdx = -1;
	switch( nCOMMENTMODE ){
// 2002/03/13 novice
	case COLORIDX_TEXT: /* �e�L�X�g */
	case COLORIDX_SSTRING:	/* �V���O���N�H�[�e�[�V����������ł��� */
	case COLORIDX_WSTRING:	/* �_�u���N�H�[�e�[�V����������ł��� */
	case COLORIDX_CTRLCODE:	/* �R���g���[���R�[�h�ł��� */
	case COLORIDX_DIGIT:	/* ���p���l�ł��� */
	case COLORIDX_KEYWORD1:	/* �����L�[���[�h1�i�o�^�P��j������ł��� */
	case COLORIDX_KEYWORD2:	/* �����L�[���[�h2�i�o�^�P��j������ł��� */
	case COLORIDX_KEYWORD3:	// 2005.01.13 MIK �����L�[���[�h3-10
	case COLORIDX_KEYWORD4:
	case COLORIDX_KEYWORD5:
	case COLORIDX_KEYWORD6:
	case COLORIDX_KEYWORD7:
	case COLORIDX_KEYWORD8:
	case COLORIDX_KEYWORD9:
	case COLORIDX_KEYWORD10:
	case COLORIDX_URL:	    /* URL�ł��� */
	case COLORIDX_SEARCH:	/* ����������ł��� */
	case COLORIDX_BRACKET_PAIR: /* �Ί��ʂ̋����\���ł��� */	// 2002/12/18 ai
		nColorIdx = nCOMMENTMODE;
		break;
	case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */
	case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */
	case COLORIDX_BLOCK2:	/* �u���b�N�R�����g2�ł��� */	//@@@ 2001.03.10 by MIK
		nColorIdx = COLORIDX_COMMENT;
		break;

//@@@ 2001.11.17 add start MIK
	default:	/* ���K�\���L�[���[�h */
		if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 )
		{
			nColorIdx = nCOMMENTMODE - 1000;	//���ʂ𗚂����Ă���̂��͂���
			colText = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* �t�H���g��I�� */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				GetFontset().ChooseFontHandle(
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
			return;
		}
		break;
//@@@ 2001.11.17 add end MIK

	}

	if( -1 != nColorIdx ){
		if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bDisp ){
			colText = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* �t�H���g��I�� */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				GetFontset().ChooseFontHandle(
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
		}
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �`��                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �ʏ�̕`�揈�� new
void CEditView::OnPaint( HDC hdc, PAINTSTRUCT *pPs, BOOL bUseMemoryDC )
{
//	MY_RUNNINGTIMER( cRunningTimer, "CEditView::OnPaint" );

	// 2004.01.28 Moca �f�X�N�g�b�v�ɍ�悵�Ȃ��悤��
	if( NULL == hdc )return;

	if( !GetDrawSwitch() )return;

	// �L�����b�g���B��
	GetCaret().HideCaret_( this->GetHwnd() ); // 2002/07/22 novice

	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->m_cDocType.GetDocumentAttribute());

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	RECT			rc;
	int				nLineHeight = GetTextMetrics().GetHankakuDy();
	int				nCharDx = GetTextMetrics().GetHankakuDx();
	const CLayout*	pcLayout;

	//�T�|�[�g
	CTypeSupport cTextType(this,COLORIDX_TEXT);

//@@@ 2001.11.17 add start MIK
	//�ύX������΃^�C�v�ݒ���s���B
	if( TypeDataPtr->m_bUseRegexKeyword || m_cRegexKeyword->m_bUseRegexKeyword ) //OFF�Ȃ̂ɑO��̃f�[�^���c���Ă�
	{
		//�^�C�v�ʐݒ������B�ݒ�ς݂��ǂ����͌Ăѐ�Ń`�F�b�N����B
		m_cRegexKeyword->RegexKeySetTypes(TypeDataPtr);
	}
//@@@ 2001.11.17 add end MIK

	// �������c�b�𗘗p�����ĕ`��̏ꍇ�͕`���̂c�b��؂�ւ���
	HDC hdcOld;
	bUseMemoryDC = FALSE;
	if( bUseMemoryDC ){
		hdcOld = hdc;
		hdc = m_hdcCompatDC;
	}

	/* 03/02/18 �Ί��ʂ̋����\��(����) ai */
	DrawBracketPair( false );

	/* ���[���[�ƃe�L�X�g�̊Ԃ̗]�� */
	//@@@ 2002.01.03 YAZAKI �]����0�̂Ƃ��͖��ʂł����B
	if ( GetTextArea().GetTopYohaku() ){
		rc.left   = 0;
		rc.top    = GetTextArea().GetRulerHeight();
		rc.right  = GetTextArea().GetAreaRight();
		rc.bottom = GetTextArea().GetAreaTop();

		cTextType.FillBack(hdc,rc);
	}
	
	/* �s�ԍ��̕\�� */
	//	From Here Sep. 7, 2001 genta
	//	Sep. 23, 2002 genta �s�ԍ���\���ł��s�ԍ��F�̑т�����̂Ō��Ԃ𖄂߂�
	if( GetTextArea().GetTopYohaku() ){ 
		rc.left   = 0;
		rc.top    = GetTextArea().GetRulerHeight();
		rc.right  = GetTextArea().GetAreaLeft() - m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace; //	Sep. 23 ,2002 genta �]���̓e�L�X�g�F�̂܂܎c��
		rc.bottom = GetTextArea().GetAreaTop();
		HBRUSH hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_GYOU].m_colBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}
	//	To Here Sep. 7, 2001 genta

	::SetBkMode( hdc, TRANSPARENT );
	m_hFontOld = NULL;

	cTextType.SetFont(hdc);
	cTextType.SetColors(hdc);


	int nTop = pPs->rcPaint.top;
	BOOL bEOF = FALSE;



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//           �`��J�n�s -> nLayoutLine             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt nLayoutLine;
	if( 0 > nTop - GetTextArea().GetAreaTop() ){
		nLayoutLine = GetTextArea().GetViewTopLine(); //�r���[�㕔����`��
	}else{
		nLayoutLine = GetTextArea().GetViewTopLine() + CLayoutInt( ( nTop - GetTextArea().GetAreaTop() ) / nLineHeight ); //�r���[�r������`��
	}

	int nMaxRollBackLineNum = 260 / (Int)nWrapKeta + 1;
	int nRollBackLineNum = 0;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLayoutLine );
	while( nRollBackLineNum < nMaxRollBackLineNum ){
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLayoutLine );
		if( NULL == pcLayout ){
			break;
		}
		if( 0 == pcLayout->GetLogicOffset() ){	/* �Ή�����_���s�̐擪����̃I�t�Z�b�g */
			break;
		}
		nLayoutLine--;
		nRollBackLineNum++;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          �`��I���s -> nLogicLineTo            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt nLogicLineTo = GetTextArea().GetViewTopLine() + CLayoutInt( ( pPs->rcPaint.bottom - GetTextArea().GetAreaTop() ) / nLineHeight );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �`����W                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	DispPos sPos(GetTextMetrics().GetHankakuDx(),GetTextMetrics().GetHankakuDy());
	sPos.InitDrawPos(CMyPoint(
		GetTextArea().GetAreaLeft() - (Int)GetTextArea().GetViewLeftCol() * nCharDx,
		GetTextArea().GetAreaTop() + (Int)( nLayoutLine - GetTextArea().GetViewTopLine() ) * nLineHeight
	));
	sPos.SetLayoutLineRef(nLayoutLine);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �S���̍s��`��                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//�K�v�ȍs��`�悷��
	bool bSelected = GetSelectionInfo().IsTextSelected();
	while(sPos.GetLayoutLineRef() <= nLogicLineTo)
	{
		pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( sPos.GetLayoutLineRef() );
	
		sPos.ResetDrawCol();

		bool bDispResult = DispLineNew(
			hdc,
			pcLayout,
			&sPos,
			nLogicLineTo,
			bSelected
		);

		if(bDispResult){
			pPs->rcPaint.bottom += nLineHeight;	// EOF�ĕ`��Ή�
			bEOF = TRUE;
			break;
		}

		if( NULL == pcLayout ){
			bEOF = TRUE;
			break;
		}
	}
	if( NULL != m_hFontOld ){
		::SelectObject( hdc, m_hFontOld );
		m_hFontOld = NULL;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              �e�L�X�g�̖��������̓h��Ԃ�                 //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if( IsBkBitmap() ){
	}else{
		/* �e�L�X�g�̂Ȃ�������w�i�F�œh��Ԃ� */
		if( sPos.GetDrawPos().y < pPs->rcPaint.bottom ){
			RECT rcBack;
			rcBack.left   = pPs->rcPaint.left;
			rcBack.right  = pPs->rcPaint.right;
			rcBack.top    = sPos.GetDrawPos().y;
			rcBack.bottom = pPs->rcPaint.bottom;

			cTextType.FillBack(hdc,rcBack);

			// 2006.04.29 �s�����͍s���Ƃɍ�悵�A�����ł͏c���̎c������
			GetTextDrawer().DispVerticalLines( hdc, sPos.GetDrawPos().y, pPs->rcPaint.bottom, CLayoutInt(0), CLayoutInt(-1) );
		}
	}

	cTextType.RewindColors(hdc);
	cTextType.RewindFont(hdc);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                �܂�Ԃ��ʒu�̕\�� (�c��)                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CTypeSupport cWrapType(this,COLORIDX_WRAP);
	if( cWrapType.IsDisp() ){
		int nXPos = GetTextArea().GetAreaLeft() + (Int)( nWrapKeta - GetTextArea().GetViewLeftCol() ) * nCharDx;
		//	2005.11.08 Moca �������ύX
		if( GetTextArea().GetAreaLeft() < nXPos && nXPos < GetTextArea().GetAreaRight() ){
			/// �܂�Ԃ��L���̐F�̃y����ݒ�
			cWrapType.SetSolidPen(hdc,0);

			::MoveToEx( hdc, nXPos, GetTextArea().GetAreaTop(), NULL );
			::LineTo( hdc, nXPos, GetTextArea().GetAreaBottom() );

			cWrapType.RewindPen(hdc);
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ���[���[�`��                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if ( pPs->rcPaint.top < GetTextArea().GetRulerHeight() ) { // ���[���[���ĕ`��͈͂ɂ���Ƃ��̂ݍĕ`�悷�� 2002.02.25 Add By KK
		GetRuler().SetRedrawFlag(); //2002.02.25 Add By KK ���[���[�S�̂�`��B
		GetRuler().DispRuler( hdc );
	}

	if ( m_pcEditWnd->m_nActivePaneIndex == m_nMyIndex ){
		/* �A�N�e�B�u�y�C���́A�A���_�[���C���`�� */
		GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     ���̑���n���Ȃ�                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	/* �������c�b�𗘗p�����ĕ`��̏ꍇ�̓������c�b�ɕ`�悵�����e����ʂփR�s�[���� */
	if( bUseMemoryDC ){
		::BitBlt(
			hdcOld,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			hdc,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
	}

	/* 03/02/18 �Ί��ʂ̋����\��(�`��) ai */
	DrawBracketPair( true );

	/* �L�����b�g�����݈ʒu�ɕ\�����܂� */
	GetCaret().ShowCaret_( this->GetHwnd() ); // 2002/07/22 novice
	return;
}









/*
2002/03/13 novice
@par nCOMMENTMODE
�֐������ŏ�ԑJ�ڂ̂��߂Ɏg����ϐ�nCOMMENTMODE�Ə�Ԃ̊֌W�B
 - COLORIDX_TEXT     : �e�L�X�g
 - COLORIDX_COMMENT  : �s�R�����g
 - COLORIDX_BLOCK1   : �u���b�N�R�����g1
 - COLORIDX_SSTRING  : �V���O���R�[�e�[�V����
 - COLORIDX_WSTRING  : �_�u���R�[�e�[�V����
 - COLORIDX_KEYWORD1 : �����L�[���[�h1
 - COLORIDX_CTRLCODE : �R���g���[���R�[�h
 - COLORIDX_DIGIT    : ���p���l
 - COLORIDX_BLOCK2   : �u���b�N�R�����g2
 - COLORIDX_KEYWORD2 : �����L�[���[�h2
 - COLORIDX_URL      : URL
 - COLORIDX_SEARCH   : ����
 - 1000: ���K�\���L�[���[�h
 	�F�w��SetCurrentColor���ĂԂƂ���COLORIDX_*�l�����Z����̂ŁA
 	1000�`COLORIDX_LAST�܂ł͐��K�\���Ŏg�p����B
*/

//@@@ 2001.02.17 MIK
//@@@ 2001.12.21 YAZAKI ���s�L���̕`��������ύX
//@@@ 2007.08.31 kobake ���� bDispBkBitmap ���폜
/*!
	�s�̃e�L�X�g�^�I����Ԃ̕`��
	1���1�_���s������悷��B

	@return EOF����悵����true
*/
bool CEditView::DispLineNew(
	HDC				hdc,			//!< [in]     ���Ώ�
	const CLayout*	pcLayout,		//!< [in]     �\�����J�n���郌�C�A�E�g
	DispPos*		pDispPos,		//!< [in/out] �`�悷��ӏ��A�`�挳�\�[�X
	CLayoutInt		nLineTo,		//!< [in]     ���I�����郌�C�A�E�g�s�ԍ�
	bool			bSelected		//!< [in]     �I�𒆂�
)
{
//	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DispLineNew" );

	//DispPos��ۑ����Ă���
	DispPos sDispPosBegin = *pDispPos;

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	int		nCharChars_2;
	EColorIndexType		nCOMMENTMODE;
	EColorIndexType		nCOMMENTMODE_OLD;
	int		nCOMMENTEND;
	int		nCOMMENTEND_OLD;

	bool	bEOF = false;
	int		i, j;
	int		nIdx;
	int		nUrlLen;
	int		nColorIdx;
	bool	bKeyWordTop = true;	//	Keyword Top

	//�T�C�Y
	Types* TypeDataPtr = &m_pcEditDoc->m_cDocType.GetDocumentAttribute();
	int nLineHeight = GetTextMetrics().GetHankakuDy();  //�s�̏c���H
	int nCharDx  = GetTextMetrics().GetHankakuDx();  //���p

	bool bSearchStringMode = false; //�����q�b�g�t���O�H����ŐF�����𔻒肷��
	BOOL bSearchFlg        = TRUE;  //�H 2002.02.08 hor
	CLogicInt  nSearchStart      = CLogicInt(-1);    //�H 2002.02.08 hor
	CLogicInt  nSearchEnd        = CLogicInt(-1);    //�H 2002.02.08 hor

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          �_���s�f�[�^�̎擾 -> pLine, pLineLen              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	const CLayout*	pcLayout2; //���[�N�pCLayout�|�C���^
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory�ύX
		nLineLen = pcLayout->GetDocLineRef()->GetLengthWithEOL() - pcLayout->GetLogicOffset();
		pLine    = pcLayout->GetDocLineRef()->GetPtr() + pcLayout->GetLogicOffset();

		// �^�C�v
		// 0=�ʏ�
		// 1=�s�R�����g
		// 2=�u���b�N�R�����g
		// 3=�V���O���N�H�[�e�[�V����������
		// 4=�_�u���N�H�[�e�[�V����������
		nCOMMENTMODE = pcLayout->GetColorTypePrev();
		nCOMMENTEND = 0;
		pcLayout2 = pcLayout;
	}
	else{
		pLine = NULL;
		nLineLen = CLogicInt(0);
		nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		nCOMMENTEND = 0;

		pcLayout2 = NULL;
	}

	/* ���݂̐F���w�� */
	SetCurrentColor( hdc, nCOMMENTMODE );

	int nLineBgn   = 0; //�H
//	int nX         = 0; //�e�L�X�gX�ʒu
	CLogicInt nCharChars = CLogicInt(0); //��������������

	//�������镶���ʒu
	CLogicInt nPos = CLogicInt(0);
	#define SetNPos(N) nPos=(N)
	#define GetNPos() (nPos+CLogicInt(0))

	//�ʏ�e�L�X�g�J�n�ʒu (�قڌŒ�)
	int nBgn = 0;
	#define SetNBgn(N) nBgn=(N)
	#define GetNBgn() (nBgn+0)

	//�T�|�[�g
	CTypeSupport cTextType(this,COLORIDX_TEXT);
	CTypeSupport cSearchType(this,COLORIDX_SEARCH);

	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		pDispPos->ForwardDrawLine(-1);
		pDispPos->ForwardLayoutLineRef(-1);

		while( GetNPos() < nLineLen ){
			pDispPos->ForwardDrawLine(1);
			pDispPos->ForwardLayoutLineRef(1);
			if( GetTextArea().GetBottomLine() < pDispPos->GetLayoutLineRef() ){
				pDispPos->SetLayoutLineRef(nLineTo + CLayoutInt(1));
				goto end_of_func;
			}
			if( nLineTo < pDispPos->GetLayoutLineRef() ){
				goto end_of_func;
			}

			pcLayout2 = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( pDispPos->GetLayoutLineRef() );

			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                        �s�ԍ��`��                           //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
				// �s�ԍ��\��
				GetTextDrawer().DispLineNumber( hdc, pcLayout2, (Int)pDispPos->GetLayoutLineRef(), pDispPos->GetDrawPos().y );
			}


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                       �{���`��J�n                          //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			SetNBgn(GetNPos());
			nLineBgn = GetNBgn();
			pDispPos->ResetDrawCol();


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                 �s��(�C���f���g)�w�i�`��                    //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			if(pcLayout2 && pcLayout2->GetIndent()!=0)
			{
				RECT rcClip;
				if(GetTextArea().GenerateClipRect(&rcClip,*pDispPos,(Int)pcLayout2->GetIndent())){
					cTextType.FillBack(hdc,rcClip);
				}
				//�`��ʒu�i�߂�
				pDispPos->ForwardDrawCol((Int)pcLayout2->GetIndent());
			}


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                 �ʏ핶����ȊO�`�惋�[�v                    //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//�s�I�[�܂��͐܂�Ԃ��ɒB����܂Ń��[�v
			while( GetNPos() - nLineBgn < pcLayout2->GetLengthWithEOL() ){
				/* ����������̐F���� */
				/* ����������̃}�[�N */
				if( m_bCurSrchKeyMark && cSearchType.IsDisp() ){
searchnext:;
					// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
					//        �����q�b�g�t���O�ݒ� -> bSearchStringMode            //
					// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

					// 2002.02.08 hor ���K�\���̌���������}�[�N������������
					if(!bSearchStringMode && (!m_sCurSearchOption.bRegularExp || (bSearchFlg && nSearchStart < GetNPos()))){
						bSearchFlg=IsSearchString( pLine, nLineLen, GetNPos(), &nSearchStart, &nSearchEnd );
					}
					if( !bSearchStringMode && bSearchFlg && nSearchStart==GetNPos()){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						bSearchStringMode = true;

						// ���݂̐F���w��
						SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
					}
					else if( bSearchStringMode && nSearchEnd <= GetNPos() ){ //+ == �ł͍s�������̏ꍇ�AnSearchEnd���O�ł��邽�߂ɕ����F�̉������ł��Ȃ��o�O���C�� 2003.05.03 �����
						// ��������������̏I���܂ł�����A�����F��W���ɖ߂�����
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							// �e�L�X�g�\��
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						/* ���݂̐F���w�� */
						SetCurrentColor( hdc, nCOMMENTMODE );
						bSearchStringMode = false;
						goto searchnext;
					}


				}

				if( GetNPos() >= nLineLen - pcLayout2->GetLayoutEol().GetLen() ){
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						/* �e�L�X�g�\�� */
						GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						SetNBgn(GetNPos() + 1);

						// �s���w�i�`��
						RECT rcClip;
						if(GetTextArea().GenerateClipRectRight(&rcClip,*pDispPos)){
							cTextType.FillBack(hdc,rcClip);
						}

						// ���s�L���̕\��
						GetTextDrawer().DispEOL(hdc,pDispPos,pcLayout2->GetLayoutEol(),bSearchStringMode);

						// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
						GetTextDrawer().DispVerticalLines( hdc, pDispPos->GetDrawPos().y, pDispPos->GetDrawPos().y + nLineHeight, CLayoutInt(0), CLayoutInt(-1) );
						if( bSelected ){
							/* �e�L�X�g���] */
							DispTextSelected(
								hdc,
								pDispPos->GetLayoutLineRef(),
								CMyPoint(sDispPosBegin.GetDrawPos().x, pDispPos->GetDrawPos().y),
								pDispPos->GetDrawCol()
							);
						}
					}

					goto end_of_line;
				}
SEARCH_START:;
				int		nMatchLen;
				int		nMatchColor;
				switch( nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
//@@@ 2001.11.17 add start MIK
					//���K�\���L�[���[�h
					if( TypeDataPtr->m_bUseRegexKeyword
					 && m_cRegexKeyword->RegexIsKeyword( pLine, GetNPos(), nLineLen, &nMatchLen, &nMatchColor )
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() )
						{
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						/* ���݂̐F���w�� */
						SetNBgn(GetNPos());
						nCOMMENTMODE = MakeColorIndexType_RegularExpression(nMatchColor);	/* �F�w�� */	//@@@ 2002.01.04 upd
						nCOMMENTEND = GetNPos() + nMatchLen;  /* �L�[���[�h������̏I�[���Z�b�g���� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
						}
					}
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					else if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( GetNPos(), nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());

						nCOMMENTMODE = COLORIDX_COMMENT;	/* �s�R�����g�ł��� */ // 2002/03/13 novice

						/* �R�����g��\������ */
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
					}
					//	Mar. 15, 2000 genta
					else if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, GetNPos(), nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}

						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, GetNPos() + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI

					}
					else if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, GetNPos(), nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}

						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, GetNPos() + (int)wcslen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}
					else if( pLine[GetNPos()] == L'\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* �V���O���N�H�[�e�[�V�����������\������ */
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_SSTRING;	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice

//						if( TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp ){	/* �V���O���N�H�[�e�[�V�����������\������ */
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
						/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}
					else if( pLine[GetNPos()] == L'"' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_WSTRING;	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}
					else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URL��\������ */
					 && IsURL( &pLine[GetNPos()], nLineLen - GetNPos(), &nUrlLen )	/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_URL;	/* URL���[�h */ // 2002/03/13 novice
						nCOMMENTEND = GetNPos() + nUrlLen;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//@@@ 2001.02.17 Start by MIK: ���p���l�������\��
//#ifdef COMPILE_COLOR_DIGIT
					}
					else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (i = IsNumber( pLine, GetNPos(), nLineLen )) > 0 )		/* ���p������\������ */
					{
						/* �L�[���[�h������̏I�[���Z�b�g���� */
						i = GetNPos() + i;
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() )
						{
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						/* ���݂̐F���w�� */
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_DIGIT;	/* ���p���l�ł��� */ // 2002/03/13 novice
						nCOMMENTEND = i;
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//#endif
//@@@ 2001.02.17 End by MIK: ���p���l�������\��
					}
					else if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* �L�[���[�h�Z�b�g */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* �����L�[���[�h��\������ */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[GetNPos()] )
					){
						//	Mar 4, 2001 genta comment out
						//	bKeyWordTop = false;
						/* �L�[���[�h������̏I�[��T�� */
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							if( IS_KEYWORD_CHAR( pLine[i] ) ){
							}else{
								break;
							}
						}
						/* �L�[���[�h���o�^�P��Ȃ�΁A�F��ς��� */
						j = i - GetNPos();
						/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */
						nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
							TypeDataPtr->m_nKeyWordSetIdx[0],
							&pLine[GetNPos()],
							j
						);
						if( nIdx != -1 ){
							if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
								/* �e�L�X�g�\�� */
								GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
							}

							/* ���݂̐F���w�� */
							SetNBgn(GetNPos());
							nCOMMENTMODE = COLORIDX_KEYWORD1;	/* �����L�[���[�h1 */ // 2002/03/13 novice
							nCOMMENTEND = i;
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
						}else{		//MIK START ADD 2000.12.01 second keyword & binary search
							// 2005.01.13 MIK �����L�[���[�h���ǉ��ɔ����z��
							for( int my_i = 1; my_i < 10; my_i++ )
							{
								if(TypeDataPtr->m_nKeyWordSetIdx[ my_i ] != -1 && /* �L�[���[�h�Z�b�g */							//MIK 2000.12.01 second keyword
									TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + my_i].m_bDisp)									//MIK
								{																							//MIK
									/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */						//MIK
									nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
										TypeDataPtr->m_nKeyWordSetIdx[ my_i ] ,													//MIK
										&pLine[GetNPos()],																		//MIK
										j																					//MIK
									);																						//MIK
									if( nIdx != -1 ){																		//MIK
										if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){										//MIK
											/* �e�L�X�g�\�� */																//MIK
											GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );	//MIK
										}																					//MIK
										/* ���݂̐F���w�� */																//MIK
										SetNBgn(GetNPos());																		//MIK
										nCOMMENTMODE = (EColorIndexType)(COLORIDX_KEYWORD1 + my_i);	/* �����L�[���[�h2 */ // 2002/03/13 novice		//MIK
										nCOMMENTEND = i;																	//MIK
										if( !bSearchStringMode ){															//MIK
											SetCurrentColor( hdc, nCOMMENTMODE );											//MIK
										}																					//MIK
										break;
									}																						//MIK
								}																							//MIK
								else
								{
									if(TypeDataPtr->m_nKeyWordSetIdx[my_i] == -1 )
										break;
								}
							}
						}			//MIK END
					}
					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pLine[GetNPos()] ))	bKeyWordTop = false;
					else								bKeyWordTop = true;
					//	To Here
					break;
// 2002/03/13 novice
				case COLORIDX_URL:		/* URL���[�h�ł��� */
				case COLORIDX_KEYWORD1:	/* �����L�[���[�h1 */
				case COLORIDX_DIGIT:	/* ���p���l�ł��� */  //@@@ 2001.02.17 by MIK
				case COLORIDX_KEYWORD2:	/* �����L�[���[�h2 */	//MIK
				case COLORIDX_KEYWORD3:	// 2005.01.13 MIK �����L�[���[�h3-10
				case COLORIDX_KEYWORD4:
				case COLORIDX_KEYWORD5:
				case COLORIDX_KEYWORD6:
				case COLORIDX_KEYWORD7:
				case COLORIDX_KEYWORD8:
				case COLORIDX_KEYWORD9:
				case COLORIDX_KEYWORD10:
				//case 1000:	//���K�\���L�[���[�h1�`10	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04 del
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* �R���g���[���R�[�h */ // 2002/03/13 novice
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = nCOMMENTMODE_OLD;
						nCOMMENTEND = nCOMMENTEND_OLD;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;

				case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, GetNPos(), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
//#ifdef	COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
				case COLORIDX_BLOCK2:	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, GetNPos(), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
//#endif
				case COLORIDX_SSTRING:	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = GetNPos()/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_WSTRING:	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = GetNPos()/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == L'"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == L'"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( GetNPos() == nCOMMENTEND ){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 ){	//���K�\���L�[���[�h1�`10
						if( GetNPos() == nCOMMENTEND ){
							if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
								/* �e�L�X�g�\�� */
								GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
							}
							SetNBgn(GetNPos());
							nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}

				//�^�u
				if( pLine[GetNPos()] == WCODE::TAB ){
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						// �e�L�X�g�\��
						GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );

						// �F����
						if( bSearchStringMode ){
							nColorIdx = COLORIDX_SEARCH;
						}else{
							nColorIdx = COLORIDX_TAB;
						}

						// �^�u�\��
						GetTextDrawer().DispTab( hdc, pDispPos, nColorIdx );
					}
					SetNBgn(GetNPos() + 1);
					nCharChars = CLogicInt(1);
				}
				//�S�p�X�y�[�X
				else if( WCODE::isZenkakuSpace(pLine[GetNPos()]) && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						// �S�p�X�y�[�X�ȑO�̃e�L�X�g��\��
						GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );

						// �S�p�󔒂�\������
						GetTextDrawer().DispZenkakuSpace(hdc,pDispPos,bSearchStringMode);
					}
					//�����i�߂�
					SetNBgn(GetNPos() + 1);
					nCharChars = CLogicInt(1);
				}
				//���p�󔒁i���p�X�y�[�X�j��\�� 2002.04.28 Add by KK 
				else if (pLine[GetNPos()] == ' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp 
					 && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{
					GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
					if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
						GetTextDrawer().DispHankakuSpace(hdc,pDispPos,bSearchStringMode);
					}
					SetNBgn(GetNPos() + 1);
					nCharChars = CLogicInt(1);
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, GetNPos() );
					if( 0 == nCharChars ){
						nCharChars = CLogicInt(1);
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* �R���g���[���R�[�h��F���� */
					 &&	WCODE::isControlCode(pLine[GetNPos()])
					){
						if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
							/* �e�L�X�g�\�� */
							GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
						}
						SetNBgn(GetNPos());
						nCOMMENTMODE_OLD = nCOMMENTMODE;
						nCOMMENTEND_OLD = nCOMMENTEND;
						nCOMMENTMODE = COLORIDX_CTRLCODE;	/* �R���g���[���R�[�h ���[�h */ // 2002/03/13 novice
						/* �R���g���[���R�[�h��̏I�[��T�� */
						for( i = GetNPos() + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if(!WCODE::isControlCode(pLine[i])){
								break;
							}
						}
						nCOMMENTEND = i;
						/* ���݂̐F���w�� */
						SetCurrentColor( hdc, nCOMMENTMODE );
					}
				}
				SetNPos( GetNPos() + nCharChars );
			}
			//end of while( GetNPos() - nLineBgn < pcLayout2->GetLength() )


			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			//                         �{���`��                            //
			// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
			if( GetNPos() >= nLineLen ){
				break;
			}
			if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
				/* �e�L�X�g�\�� */
				GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );
				SetNBgn(GetNPos());

				/* �s���w�i�`�� */
				RECT rcClip;
				if(GetTextArea().GenerateClipRectRight(&rcClip,*pDispPos)){
					cTextType.FillBack(hdc,rcClip);
				}

				/* �܂�Ԃ��L����\������ */
				GetTextDrawer().DispWrap(hdc,pDispPos);

				// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
				GetTextDrawer().DispVerticalLines( hdc, pDispPos->GetDrawPos().y, pDispPos->GetDrawPos().y + nLineHeight,  CLayoutInt(0), CLayoutInt(-1) );
				if( bSelected ){
					/* �e�L�X�g���] */
					DispTextSelected(
						hdc,
						pDispPos->GetLayoutLineRef(),
						CMyPoint(sDispPosBegin.GetDrawPos().x, pDispPos->GetDrawPos().y),
						pDispPos->GetDrawCol()
					);
				}
			}
		}
		// end of while( GetNPos() < nLineLen )

		if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){

			/* �e�L�X�g�\�� */
			GetTextDrawer().DispText( hdc, pDispPos, &pLine[GetNBgn()], GetNPos() - GetNBgn() );

			/* EOF�L���̕\�� */
			if( pDispPos->GetLayoutLineRef() + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount()
				&& pDispPos->GetDrawCol() < nWrapKeta )
			{
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					//	May 29, 2004 genta (advised by MIK) ���ʊ֐���
					GetTextDrawer().DispEOF(hdc,pDispPos);
				}
				bEOF = true;
			}

			if( IsBkBitmap() ){
			}else{
				// �s���w�i�`��
				RECT rcClip;
				if(GetTextArea().GenerateClipRectRight(&rcClip,*pDispPos)){
					cTextType.FillBack(hdc,rcClip);
				}
			}

			// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
			GetTextDrawer().DispVerticalLines( hdc, pDispPos->GetDrawPos().y, pDispPos->GetDrawPos().y + nLineHeight,  CLayoutInt(0), CLayoutInt(-1) );

			if( bSelected ){
				/* �e�L�X�g���] */
				DispTextSelected(
					hdc,
					pDispPos->GetLayoutLineRef(),
					CMyPoint(sDispPosBegin.GetDrawPos().x, pDispPos->GetDrawPos().y),
					pDispPos->GetDrawCol()
				);
			}

		}
end_of_line:;
		pDispPos->ForwardLayoutLineRef(1);
		pDispPos->ForwardDrawLine(1);
	}
	// NULL == pLine�̏ꍇ
	else{
		if( pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
			if(GetTextDrawer().DispEmptyLine(hdc,pDispPos)){
				bEOF=true;
			}
		}
	}

end_of_func:;
//	2002/05/08 YAZAKI �A���_�[���C���̍ĕ`��͕s�v�ł���
//	MYTRACE_A( "m_nOldUnderLineY=%d\n", m_nOldUnderLineY );
//	if( -1 != m_nOldUnderLineY ){
//		/* �J�[�\���s�A���_�[���C����ON */
//		CaretUnderLineON( TRUE );
//	}


	return bEOF;
}




/* �e�L�X�g���]

	@param hdc      
	@param nLineNum 
	@param x        
	@param y        
	@param nX       

	@note
	CCEditView::DispLineNew() �ł̍��(WM_PAINT)���ɁA1���C�A�E�g�s���܂Ƃ߂Ĕ��]�������邽�߂̊֐��B
	�͈͑I���̐����X�V�́ACEditView::DrawSelectArea() ���I���E���]�������s���B
	
*/
void CEditView::DispTextSelected(
	HDC				hdc,		//!< ���Ώۃr�b�g�}�b�v���܂ރf�o�C�X
	CLayoutInt		nLineNum,	//!< ���]�����Ώۃ��C�A�E�g�s�ԍ�(0�J�n)
	const CMyPoint&	ptXY,		//!< (���΃��C�A�E�g0���ڂ̍��[���W, �Ώۍs�̏�[���W)
	CLayoutInt		nX_Layout	//!< �Ώۍs�̏I�����ʒu�B�@[ABC\n]�Ȃ���s�̌���4
)
{
	CLayoutInt	nSelectFrom;
	CLayoutInt	nSelectTo;
	RECT		rcClip;
	int			nLineHeight = GetTextMetrics().GetHankakuDy();
	int			nCharWidth = GetTextMetrics().GetHankakuDx();
	HRGN		hrgnDraw;
	const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );

	/* �I��͈͓��̍s���� */
//	if( IsTextSelected() ){
		if( nLineNum >= GetSelectionInfo().m_sSelect.GetFrom().y && nLineNum <= GetSelectionInfo().m_sSelect.GetTo().y ){
			if( GetSelectionInfo().IsBoxSelecting() ){		/* ��`�͈͑I�� */
				nSelectFrom = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
				nSelectTo   = GetSelectionInfo().m_sSelect.GetTo().GetX2();
				// 2006.09.30 Moca From ��`�I����[EOF]�Ƃ��̉E���͔��]���Ȃ��悤�ɏC���B������ǉ�
				if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 <= nLineNum ){
					CLayoutPoint ptEnd(0, 0);
					m_pcEditDoc->m_cLayoutMgr.GetEndLayoutPos( &ptEnd );
					if( ptEnd.y == nLineNum ){
						if( ptEnd.GetX2() < nSelectFrom ){
							nSelectFrom = ptEnd.GetX2();
						}
						if( ptEnd.GetX2() < nSelectTo ){
							nSelectTo = ptEnd.GetX2();
						}
					}
				}
				// 2006.09.30 Moca To
			}
			else{
				if( GetSelectionInfo().m_sSelect.IsLineOne() ){
					nSelectFrom = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
					nSelectTo   = GetSelectionInfo().m_sSelect.GetTo().GetX2();
				}
				else{
					if( nLineNum == GetSelectionInfo().m_sSelect.GetFrom().y ){
						nSelectFrom = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
						nSelectTo   = nX_Layout;
					}
					else if( nLineNum == GetSelectionInfo().m_sSelect.GetTo().GetY2() ){
						nSelectFrom = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
						nSelectTo   = GetSelectionInfo().m_sSelect.GetTo().GetX2();
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
				CLayoutInt t = nSelectFrom;
				nSelectFrom = nSelectTo;
				nSelectTo = t;
			}

			// 2006.03.28 Moca �\����O�Ȃ牽�����Ȃ�
			if( GetTextArea().GetRightCol() < nSelectFrom ){
				return;
			}
			if( nSelectTo <= GetTextArea().GetViewLeftCol() ){
				return;
			}

			if( nSelectFrom < GetTextArea().GetViewLeftCol() ){
				nSelectFrom = GetTextArea().GetViewLeftCol();
			}
			rcClip.left   = ptXY.x + (Int)nSelectFrom * ( nCharWidth );
			rcClip.right  = ptXY.x + (Int)nSelectTo   * ( nCharWidth );
			rcClip.top    = ptXY.y;
			rcClip.bottom = ptXY.y + nLineHeight;

			// 2005/04/02 ����� �O�����}�b�`���Ɣ��]�����O�ƂȂ蔽�]����Ȃ��̂ŁA1/3�������������]������
			// 2005/06/26 zenryaku �I�������ŃL�����b�g�̎c�[���c������C��
			// 2005/09/29 ryoji �X�N���[�����ɃL�����b�g�̂悤�ȃS�~���\�����������C��
			if (GetSelectionInfo().IsTextSelected() && rcClip.right == rcClip.left &&
				GetSelectionInfo().m_sSelect.IsLineOne() &&
				GetSelectionInfo().m_sSelect.GetFrom().x >= GetTextArea().GetViewLeftCol())
			{
				rcClip.right = rcClip.left + (nCharWidth/3 == 0 ? 1 : nCharWidth/3);
			}

			// 2006.03.28 Moca �E�B���h�E�����傫���Ɛ��������]���Ȃ������C��
			if( rcClip.right > GetTextArea().GetAreaRight() ){
				rcClip.right = GetTextArea().GetAreaRight();
			}
			
			HBRUSH hBrush    = ::CreateSolidBrush( SELECTEDAREA_RGB );

			int    nROP_Old  = ::SetROP2( hdc, SELECTEDAREA_ROP2 );
			HBRUSH hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
			hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
			::PaintRgn( hdc, hrgnDraw );
			::DeleteObject( hrgnDraw );

			SetROP2( hdc, nROP_Old );
			SelectObject( hdc, hBrushOld );
			DeleteObject( hBrush );
		}
//	}
	return;
}
