#include "stdafx.h"
#include "CEditView_Paint.h"
#include "types/CTypeSupport.h"
#include "view/colors/CColorStrategy.h"

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

	CGraphics gr;
	gr.Init(::GetDC(GetHwnd()));
	STypeConfig *TypeDataPtr = &( m_pcEditDoc->m_cDocType.GetDocumentAttribute() );

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
				EColorIndexType		nColorIndex;
				CLogicInt	OutputX = LineColmnToIndex( pcLayout, ptColLine.GetX2() );
				if( bDraw )	{
					nColorIndex = COLORIDX_BRACKET_PAIR;
				}
				else{
					if( IsBracket( pLine, OutputX, CLogicInt(1) ) ){
						// 03/10/24 ai �܂�Ԃ��s��ColorIndex���������擾�ł��Ȃ����ɑΉ�
						// 2009.02.07 ryoji GetColorIndex �ɓn���C���f�b�N�X�̎d�l�ύX�i���͂������̎d�l�������͗l�j
						nColorIndex = GetColorIndex( pcLayout, OutputX + 1 );
					}
					else{
						SetBracketPairPos( false );
						break;
					}
				}

				//�F�ݒ�
				CTypeSupport cTextType(this,COLORIDX_TEXT);
				cTextType.SetGraphicsState_WhileThisObj(gr);

				SetCurrentColor( gr, nColorIndex );

				int nHeight = GetTextMetrics().GetHankakuDy();
				int nLeft = (GetTextArea().GetDocumentLeftClientPointX()) + (Int)ptColLine.x * GetTextMetrics().GetHankakuDx();
				int nTop  = (Int)( ptColLine.GetY2() - GetTextArea().GetViewTopLine() ) * nHeight + GetTextArea().GetAreaTop();

				// 03/03/03 ai �J�[�\���̍��Ɋ��ʂ����芇�ʂ������\������Ă����Ԃ�Shift+���őI���J�n�����
				//             �I��͈͓��ɔ��]�\������Ȃ�������������̏C��
				if( ptColLine.x == GetCaret().GetCaretLayoutPos().GetX2() && GetCaret().GetCaretShowFlag() ){
					GetCaret().HideCaret_( GetHwnd() );	// �L�����b�g����u������̂�h�~
					GetTextDrawer().DispText( gr, nLeft, nTop, &pLine[OutputX], 1 );
					// 2006.04.30 Moca �Ί��ʂ̏c���Ή�
					GetTextDrawer().DispVerticalLines( gr, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + CLayoutInt(2) ); //�����ʂ��S�p���ł���ꍇ���l��
					GetCaret().ShowCaret_( GetHwnd() );	// �L�����b�g����u������̂�h�~
				}
				else{
					GetTextDrawer().DispText( gr, nLeft, nTop, &pLine[OutputX], 1 );
					// 2006.04.30 Moca �Ί��ʂ̏c���Ή�
					GetTextDrawer().DispVerticalLines( gr, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + CLayoutInt(2) ); //�����ʂ��S�p���ł���ꍇ���l��
				}

				cTextType.RewindGraphicsState(gr);

				if( ( m_pcEditDoc->m_pcEditWnd->m_nActivePaneIndex == m_nMyIndex )
					&& ( ( ptColLine.y == GetCaret().GetCaretLayoutPos().GetY() ) || ( ptColLine.y - 1 == GetCaret().GetCaretLayoutPos().GetY() ) ) ){	// 03/02/27 ai �s�̊Ԋu��"0"�̎��ɃA���_�[���C���������鎖������׏C��
					GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
				}
			}
		}
	}

	::ReleaseDC( GetHwnd(), gr );
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

