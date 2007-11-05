/*!	@file
	@brief CEditView�N���X

	@author Norio Nakatani
	@date	1998/12/08 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, MIK, asa-o, ao
	Copyright (C) 2001, hor, MIK, genta, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, novice, hor, ai, KK
	Copyright (C) 2003, Moca, ai, �����
	Copyright (C) 2004, genta
	Copyright (C) 2005, genta, MIK, Moca, �����, ryoji, zenryaku, D.S.Koba
	Copyright (C) 2006, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include "CEditView.h"
#include "debug.h"
#include "funccode.h"
#include "charcode.h"
#include "mymessage.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
#include "Clayout.h" /// 2002/2/3 aroka
#include "CDocLine.h" /// 2002/2/3 aroka
#include "CMarkMgr.h" /// 2002/2/3 aroka
#include "my_icmp.h"	//@@@ 2002.01.13 add
#include "CTextMetrics.h"
#include "CTypeSupport.h"
#include "parse/CWordParse.h"
#include "util/os.h"

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
			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* �t�H���g��I�� */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				GetFontset().ChooseFontHandle(
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
			return;
		}
		break;
//@@@ 2001.11.17 add end MIK

	}





	if( -1 != nColorIdx ){
		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bDisp ){
			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* �t�H���g��I�� */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				GetFontset().ChooseFontHandle(
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
		}
	}

	return;
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




/* ���݈ʒu������������ɊY�����邩 */
//2002.02.08 hor
//���K�\���Ō��������Ƃ��̑��x���P�̂��߁A�}�b�`�擪�ʒu�������ɒǉ�
bool CEditView::IsSearchString(
	const wchar_t*	pszData,
	CLogicInt		nDataLen,
	CLogicInt		nPos,
	CLogicInt*		pnSearchStart,
	CLogicInt*		pnSearchEnd
)
{
	CLogicInt		nKeyLength;

	//	From Here Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	*pnSearchStart = nPos;	// 2002.02.08 hor

	if( m_sCurSearchOption.bRegularExp ){
		/* �s���ł͂Ȃ�? */
		/* �s�������`�F�b�N�́ACBregexp�N���X�����Ŏ��{����̂ŕs�v 2003.11.01 ����� */

		/* �ʒu��0��MatchInfo�Ăяo���ƁA�s�������������ɁA�S�� true�@�ƂȂ�A
		** ��ʑS�̂����������񈵂��ɂȂ�s��C��
		** �΍�Ƃ��āA�s���� MacthInfo�ɋ����Ȃ��Ƃ����Ȃ��̂ŁA������̒����E�ʒu����^����`�ɕύX
		** 2003.05.04 �����
		*/
		if( m_CurRegexp.Match( pszData, nDataLen, nPos ) ){
			*pnSearchStart = m_CurRegexp.GetIndex();	// 2002.02.08 hor
			*pnSearchEnd = m_CurRegexp.GetLastIndex();
	//	To Here Jun. 26, 2001 genta
			return true;

		}else{
			return false;
		}
	}else{
		nKeyLength = CLogicInt(wcslen( m_szCurSrchKey ));		/* �������� */

		// 2001/06/23 �P��P�ʂ̌����̂��߂ɒǉ�
		if( m_pShareData->m_Common.m_sSearch.m_sSearchOption.bWordOnly ){	/* �����^�u��  1==�P��̂݌��� */
			/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
			/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
			CLogicInt nIdxFrom, nIdxTo;
			if( !CWordParse::WhereCurrentWord_2( pszData, nDataLen, nPos, &nIdxFrom, &nIdxTo, NULL, NULL ) ){
				return false;
			}
			if( nPos != nIdxFrom || nKeyLength != nIdxTo - nIdxFrom ){
				return false;
			}
		}

		//��������������` �܂��� ���������̒�����蒲�ׂ�f�[�^���Z���Ƃ��̓q�b�g���Ȃ�
		if( 0 == nKeyLength || nKeyLength > nDataLen - nPos ){
			return false;
		}
		//�p�啶���������̋�ʂ����邩�ǂ���
		if( m_sCurSearchOption.bLoHiCase ){	/* 1==�p�啶���������̋�� */
			if( 0 == wmemcmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return true;
			}
		}else{
			if( 0 == auto_memicmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return true;
			}
		}
	}
	return false;
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
	const wchar_t *cline = m_pcEditDoc->m_cDocLineMgr.GetLineStr( ptPos.GetY2(), &len );

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

	const wchar_t *bPos = CNativeW2::GetCharPrev( cline, ptPos.x, cline + ptPos.x );
	int nCharSize = cline + ptPos.x - bPos;
//	m_nCharSize = nCharSize;	// 02/10/01 �Ί��ʂ̕����T�C�Y�ݒ� ai
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
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( ptPos.GetY2() );
	cline = ci->m_cLine.GetStringPtr( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos < lineend ){
			nPos = CNativeW2::GetCharNext( cline, len, cPos );
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
		ci = ci->m_pNext;	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->m_cLine.GetStringPtr( &len );
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
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( ptPos.GetY2() );
	cline = ci->m_cLine.GetStringPtr( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos > cline ){
			pPos = CNativeW2::GetCharPrev( cline, len, cPos );
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
		ci = ci->m_pPrev;	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->m_cLine.GetStringPtr( &len );
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
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( ptPos.GetY2() );
	cline = ci->m_cLine.GetStringPtr( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos < lineend ){
			nPos = CNativeW2::GetCharNext( cline, len, cPos );
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
		ci = ci->m_pNext;	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->m_cLine.GetStringPtr( &len );
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
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( ptPos.GetY2() );
	cline = ci->m_cLine.GetStringPtr( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos > cline ){
			pPos = CNativeW2::GetCharPrev( cline, len, cPos );
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
		ci = ci->m_pPrev;	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->m_cLine.GetStringPtr( &len );
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

//!	���݂̃J�[�\���s�ʒu�𗚗��ɓo�^����
void CEditView::AddCurrentLineToHistory( void )
{
	CLogicPoint ptPos;
	//int PosX, PosY;	//	�����ʒu�i���s�P�ʂ̌v�Z�j

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( GetCaret().GetCaretLayoutPos(), &ptPos );

	CMarkMgr::CMark m( ptPos );
	m_cHistory->Add( m );

}


//	2001/06/18 Start by asa-o: �⊮�E�B���h�E�p�̃L�[���[�h�w���v�\��
bool  CEditView::ShowKeywordHelp( POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin)
{
	CNativeW2	cmemCurText;
	RECT		rcTipWin,
				rcDesktop;

	if( m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp ){ /* �L�[���[�h�w���v���g�p���� */
		if( m_bInMenuLoop == FALSE	&&	/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ� */
			0 != m_dwTipTimer			/* ����Tip��\�����Ă��Ȃ� */
		){
			cmemCurText.SetString( pszHelp );

			/* ���Ɍ����ς݂� */
			if( CNativeW2::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) ){
				/* �Y������L�[���Ȃ����� */
				if( !m_cTipWnd.m_KeyWasHit ){
					return false;
				}
			}else{
				m_cTipWnd.m_cKey = cmemCurText;
				/* �������s */
				if(FALSE == KeySearchCore(&m_cTipWnd.m_cKey))	// 2006.04.10 fon
					return FALSE;
			}
			m_dwTipTimer = 0;	/* ����Tip��\�����Ă��� */

		// 2001/06/19 Start by asa-o: ����Tip�̕\���ʒu����
			// ����Tip�̃T�C�Y���擾
			m_cTipWnd.GetWindowSize(&rcTipWin);

			//	May 01, 2004 genta �}���`���j�^�Ή�
			::GetMonitorWorkRect( m_cTipWnd.GetHwnd(), &rcDesktop );

			// �E�ɓ���
			if(prcHokanWin->right + rcTipWin.right < rcDesktop.right){
				// ���̂܂�
			}else
			// ���ɓ���
			if(rcDesktop.left < prcHokanWin->left - rcTipWin.right ){
				// ���ɕ\��
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}else
			// �ǂ�����X�y�[�X�������Ƃ��L���ق��ɕ\��
			if(rcDesktop.right - prcHokanWin->right > prcHokanWin->left ){
				// �E�ɕ\�� ���̂܂�
			}else{
				// ���ɕ\��
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}
		// 2001/06/19 End

			/* ����Tip��\�� */
			m_cTipWnd.Show( po.x, po.y , NULL , &rcTipWin);
			return true;
		}
	}
	return false;
}
//	2001/06/18 End

#if 0
/*!	�R���g���[���R�[�h�� "^@" �\������B
	0x00 - 0x1f �� "^@" - "^_" �ŕ\������B
	0x7f �� "^?" �ŕ\������B
	���̑��� "�E" �ŕ\������B
*/
int CEditView::DispCtrlCode( HDC hdc, int x, int y, const unsigned char* pData, int nLength )
{
	int		i, x1, y1;
	unsigned char	c;
	HPEN	hPen, hPenOld;

	x1 = GetTextMetrics().GetHankakuWidth() / 3;
	y1 = GetTextMetrics().GetHankakuHeight() / 5;

	hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CTRLCODE].m_colTEXT );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	for( i = 0; i < nLength; i++, pData++ )
	{
		if     ( (*pData) <= 0x1f ) c = '@' + (*pData);
		else if( (*pData) == 0x7f ) c = '?';
		else                        c = '�';

		//������\������
		DispText( hdc, x, y, &c, 1 );

		//���䕶���������L����`�悷��
		::MoveToEx( hdc, x, y + y1, NULL );
		::LineTo( hdc, x + x1, y );
		::LineTo( hdc, x + x1 * 2, y + y1 );

		x += GetTextMetrics().GetHankakuWidth();
	}

	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );

	return nLength;
}
#endif


/*[EOF]*/
 
