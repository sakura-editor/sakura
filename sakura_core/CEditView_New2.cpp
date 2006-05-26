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
	Copyright (C) 2006, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditView.h"
#include "debug.h"
#include "funccode.h"
#include "charcode.h"
#include "mymessage.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
#include "etc_uty.h"
#include "Clayout.h" /// 2002/2/3 aroka
#include "CDocLine.h" /// 2002/2/3 aroka
#include "CMarkMgr.h" /// 2002/2/3 aroka
#include "my_icmp.h"	//@@@ 2002.01.13 add


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
				ChooseFontHandle(
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
				ChooseFontHandle(
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
		}
	}

	return;
}


/* �s�ԍ��\�� */
void CEditView::DispLineNumber(
		HDC						hdc,
		const CLayout*			pcLayout,
		int						nLineNum,
		int						y
)
{
	RECT			rcClip;
	HBRUSH			hBrush;
	COLORREF		colTextColorOld;
	COLORREF		colBkColorOld;
	char			szLineNum[18];
	int				nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	int				nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
	int				nLineCols;
	UINT			fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	HPEN			hPen, hPenOld;
	int				nColorIndex;
	const CDocLine*	pCDocLine;
	//	Sep. 23, 2002 genta ���ʎ��̂����肾��
	int				nLineNumAreaWidth = m_nViewAlignLeftCols * nCharWidth;

	nColorIndex = COLORIDX_GYOU;	/* �s�ԍ� */
	if( NULL != pcLayout ){
//		pCDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( pcLayout->m_nLinePhysical );
		pCDocLine = pcLayout->m_pCDocLine;

		if( m_pcEditDoc->IsModified()	/* �h�L�������g�����ύX�̏�Ԃ� */
		 && pCDocLine->IsModifyed() ){		/* �ύX�t���O */
//			if( 0 == pCDocLine->m_nModifyCount ){	/* �ύX�� */
				nColorIndex = COLORIDX_GYOU_MOD;	/* �s�ԍ��i�ύX�s�j */
//			}
//		}else{
//			if( /* FALSE == m_pcEditDoc->IsModified() && --*/ /* �h�L�������g�����ύX�̏�Ԃ� */
//				0 < pCDocLine->m_nModifyCount	/* �ύX�� */
//			){
//				nColorIndex = COLORIDX_GYOU_MODSAVE;	/* �s�ԍ��i�ύX&�ۑ��ρj */
//			}
		}
	}

	int type = pCDocLine->IsDiffMarked();
	{	//DIFF�����}�[�N�\��	//@@@ 2002.05.25 MIK
		if( type )
		{
			switch( type )
			{
			case MARK_DIFF_APPEND:	//�ǉ�
				if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_DIFF_APPEND].m_bDisp )
					nColorIndex = COLORIDX_DIFF_APPEND;
				break;
			case MARK_DIFF_CHANGE:	//�ύX
				if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_DIFF_CHANGE].m_bDisp )
					nColorIndex = COLORIDX_DIFF_CHANGE;
				break;
			case MARK_DIFF_DELETE:	//�폜
			case MARK_DIFF_DEL_EX:	//�폜
				if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_DIFF_DELETE].m_bDisp )
					nColorIndex = COLORIDX_DIFF_DELETE;
				break;
			}
		}
	}

	/* 02/10/16 ai Start */
	// �u�b�N�}�[�N�̕\��
	if(pCDocLine->IsBookMarked()){
		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_MARK].m_bDisp ) {
			nColorIndex = COLORIDX_MARK;
		}
	}
	/* 02/10/16 ai End */

//	if( m_pcEditDoc->GetDocumentAttribute().m_bDispLINE ){	/* �s�ԍ��\���^��\�� */
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[/*nColorIndex*/COLORIDX_GYOU].m_bDisp ){	/* �s�ԍ��\���^��\�� */
		/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		if( m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){
			/* �_���s�ԍ��\�����[�h */
			if( NULL == pcLayout || 0 != pcLayout->m_nOffset ){
				strcpy( szLineNum, " " );
			}else{
				_itoa( pcLayout->m_nLinePhysical + 1, szLineNum, 10 );	/* �Ή�����_���s�ԍ� */
			}
		}else{
			/* �����s�i���C�A�E�g�s�j�ԍ��\�����[�h */
			_itoa( nLineNum + 1, szLineNum, 10 );
		}
		nLineCols = lstrlen( szLineNum );

		colTextColorOld = ::SetTextColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );	/* �s�ԍ��̐F */
		//	Sep. 23, 2002 �]�����e�L�X�g�̔w�i�F�ɂ���
		colBkColorOld = ::SetBkColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );		/* �e�L�X�g�w�i�̐F */

		HFONT	hFontOld;
		/* �t�H���g��I�� */
		hFontOld = (HFONT)::SelectObject( hdc,
			ChooseFontHandle(
				m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_bFatFont,
				m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_bUnderLine
			)
		);
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_bFatFont ){	/* ������ */
//			hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//		}else{
//			hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//		}

		/* �]���𖄂߂� */
		rcClip.left = nLineNumAreaWidth;
		rcClip.right = m_nViewAlignLeft;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOut( hdc,
			rcClip.left,
			y, fuOptions,
			&rcClip, " ", 1, m_pnDx
		);
		
		//	Sep. 23, 2002 �]�����e�L�X�g�̔w�i�F�ɂ��邽�߁C�w�i�F�̐ݒ���ړ�
		SetBkColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colBACK );		/* �s�ԍ��w�i�̐F */

//		/* �s�ԍ��̃e�L�X�g��\�� */
//		m_pShareData->m_Types[nIdx].m_nLineTermType = 1;			/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
//		m_pShareData->m_Types[nIdx].m_cLineTermChar = ':';			/* �s�ԍ���؂蕶�� */

		int drawNumTop = (m_nViewAlignLeftCols - nLineCols - 1) * ( nCharWidth );

		/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
		if( 2 == m_pcEditDoc->GetDocumentAttribute().m_nLineTermType ){
			//	Sep. 22, 2002 genta
			szLineNum[ nLineCols ] = m_pcEditDoc->GetDocumentAttribute().m_cLineTermChar;
			szLineNum[ ++nLineCols ] = '\0';
			//char szLineTerm[2];
			//wsprintf( szLineTerm, "%c", m_pcEditDoc->GetDocumentAttribute().m_cLineTermChar );	/* �s�ԍ���؂蕶�� */
			//strcat( szLineNum, szLineTerm );
		}
		rcClip.left = 0;
		//rcClip.right = m_nViewAlignLeft/* - 3*/;
		//	Sep. 23, 2002 genta
		rcClip.right = nLineNumAreaWidth;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOut( hdc,
			drawNumTop,
			y, fuOptions,
			&rcClip, szLineNum, nLineCols, m_pnDx // Sep. 22, 2002 genta
		);


//		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_colorGYOU );


		/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
		if( 1 == m_pcEditDoc->GetDocumentAttribute().m_nLineTermType ){
			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );
			hPenOld = (HPEN)::SelectObject( hdc, hPen );
			::MoveToEx( hdc, nLineNumAreaWidth - 2, y, NULL );
			::LineTo( hdc, nLineNumAreaWidth - 2, y + nLineHeight );
			::SelectObject( hdc, hPenOld );
			::DeleteObject( hPen );
		}
		::SetTextColor( hdc, colTextColorOld );
		::SetBkColor( hdc, colBkColorOld );
		::SelectObject( hdc, hFontOld );
	}else{
		rcClip.left = 0;
		//	Sep. 23 ,2002 genta �]���̓e�L�X�g�F�̂܂܎c��
		rcClip.right = m_nViewAlignLeft - m_pShareData->m_Common.m_nLineNumRightSpace;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
//		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_colorBACK );
		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex/*COLORIDX_TEXT*/].m_colBACK );
		::FillRect( hdc, &rcClip, hBrush );
		::DeleteObject( hBrush );
		
		// From Here Mar. 5, 2003, Moca
		// �s�ԍ��ƃe�L�X�g�̌��Ԃ̕`��R��
		rcClip.left = rcClip.right;
		rcClip.right = m_nViewAlignLeft;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
		::FillRect( hdc, &rcClip, hBrush );
		::DeleteObject( hBrush );
		// To Here Mar. 5, 2003, Moca
	}

// From Here 2001.12.03 hor
	/* �Ƃ肠�����u�b�N�}�[�N�ɏc�� */
	if(pCDocLine->IsBookMarked() &&	// Dec. 24, 2002 ai
		(FALSE == m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_MARK].m_bDisp)){
		hPen = ::CreatePen( PS_SOLID, 2, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, 1, y, NULL );
		::LineTo( hdc, 1, y + nLineHeight );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
	}
// To Here 2001.12.03 hor

	if( type )	//DIFF�����}�[�N�\��	//@@@ 2002.05.25 MIK
	{
		int	cy = y + nLineHeight / 2;

		hPen = ::CreatePen( PS_SOLID, 1, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );

		switch( type )
		{
		case MARK_DIFF_APPEND:	//�ǉ�
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo( hdc, 6, cy );
			::MoveToEx( hdc, 4, cy - 2, NULL );
			::LineTo( hdc, 4, cy + 3 );
			break;

		case MARK_DIFF_CHANGE:	//�ύX
			::MoveToEx( hdc, 3, cy - 4, NULL );
			::LineTo( hdc, 3, cy );
			::MoveToEx( hdc, 3, cy + 2, NULL );
			::LineTo( hdc, 3, cy + 3 );
			break;

		case MARK_DIFF_DELETE:	//�폜
			cy -= 3;
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo( hdc, 5, cy );
			::LineTo( hdc, 3, cy + 2 );
			::LineTo( hdc, 3, cy );
			::LineTo( hdc, 7, cy + 4 );
			break;
		
		case MARK_DIFF_DEL_EX:	//�폜(EOF)
			cy += 3;
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo( hdc, 5, cy );
			::LineTo( hdc, 3, cy - 2 );
			::LineTo( hdc, 3, cy );
			::LineTo( hdc, 7, cy - 4 );
			break;
		}

		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
	}
	
	return;
}



/* �e�L�X�g�\��
	@@@ 2002.09.22 YAZAKI const unsigned char* pData���Aconst char* pData�ɕύX
*/
int CEditView::DispText( HDC hdc, int x, int y, const char* pData, int nLength )
{
	if( 0 >= nLength ){
		return 0;
	}
	RECT	rcClip;
	int		nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	int		nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
	UINT	fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	rcClip.left = x;
	rcClip.right = rcClip.left + ( nCharWidth ) * nLength;
	rcClip.top = y;
	if( rcClip.left < m_nViewAlignLeft ){
		rcClip.left = m_nViewAlignLeft;
	}
	if( rcClip.left < rcClip.right
	 && rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft
	 && rcClip.top >= m_nViewAlignTop
	){
		rcClip.bottom = y + nLineHeight;

		//@@@	From Here 2002.01.30 YAZAKI ExtTextOut�̐������
		if( rcClip.right - rcClip.left > m_nViewCx ){
			rcClip.right = rcClip.left + m_nViewCx;
		}
		int nBefore = 0;	//	�E�B���h�E�̍��ɂ��ӂꂽ������
		int nAfter = 0;		//	�E�B���h�E�̉E�ɂ��ӂꂽ������
		if ( x < 0 ){
			int nLeft = ( 0 - x ) / nCharWidth - 1;
			while (nBefore < nLeft){
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nBefore += CMemory::GetSizeOfChar( pData, nLength, nBefore );
			}
		}
		if ( rcClip.right < x + nCharWidth * nLength ){
			//	-1���Ă��܂����i������͂�����ˁH�j
			nAfter = (x + nCharWidth * nLength - rcClip.right) / nCharWidth - 1;
		}
		::ExtTextOut( hdc, x + nBefore * nCharWidth, y, fuOptions, &rcClip, &pData[nBefore], nLength - nBefore - nAfter, m_pnDx );
		//@@@	To Here 2002.01.30 YAZAKI ExtTextOut�̐������
	}
	return nLength;

}


/* �e�L�X�g���] */
void CEditView::DispTextSelected( HDC hdc, int nLineNum, int x, int y, int nX  )
{
//	MYTRACE( "CEditView::DispTextSelected()\n" );

	int			nROP_Old;
	int			nSelectFrom;
	int			nSelectTo;
	RECT		rcClip;
//	HPEN		hPen;
//	HPEN		hPenOld;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	int			nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	int			nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
	HRGN		hrgnDraw;
	const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum );

	/* �I��͈͓��̍s���� */
//	if( IsTextSelected() ){
		if( nLineNum >= m_nSelectLineFrom && nLineNum <= m_nSelectLineTo ){
			if( m_bBeginBoxSelect){		/* ��`�͈͑I�� */
				nSelectFrom = m_nSelectColmFrom;
				nSelectTo   = m_nSelectColmTo;
			}else{
				if( m_nSelectLineFrom == m_nSelectLineTo ){
						nSelectFrom = m_nSelectColmFrom;
						nSelectTo   = m_nSelectColmTo;
				}else{
					if( nLineNum == m_nSelectLineFrom ){
						nSelectFrom = m_nSelectColmFrom;
						nSelectTo   = nX;
					}else
					if( nLineNum == m_nSelectLineTo ){
						nSelectFrom = pcLayout ? pcLayout->GetIndent() : 0;
						nSelectTo   = m_nSelectColmTo;
					}else{
						nSelectFrom = pcLayout ? pcLayout->GetIndent() : 0;
						nSelectTo   = nX;
					}
				}
			}
			// 2006.05.24 Moca ��`�I��/�t���[�J�[�\���I��(�I���J�n/�I���s)��
			// To < From �ɂȂ邱�Ƃ�����B�K�� From < To �ɂȂ�悤�ɓ���ւ���B
			if( nSelectTo < nSelectFrom ){
				int t = nSelectFrom;
				nSelectFrom = nSelectTo;
				nSelectTo = t;
			}
			// 2006.03.28 Moca �\����O�Ȃ牽�����Ȃ�
			if( m_nViewLeftCol + m_nViewColNum < nSelectFrom ){
				return;
			}
			if( nSelectTo <= m_nViewLeftCol ){
				return;
			}

			if( nSelectFrom < m_nViewLeftCol ){
				nSelectFrom = m_nViewLeftCol;
			}
			rcClip.left   = x + nSelectFrom * ( nCharWidth );
			rcClip.right  = x + nSelectTo   * ( nCharWidth );
			rcClip.top    = y;
			rcClip.bottom = y + nLineHeight;
			// 2005/04/02 ����� �O�����}�b�`���Ɣ��]�����O�ƂȂ蔽�]����Ȃ��̂ŁA1/3�������������]������
			// 2005/06/26 zenryaku �I�������ŃL�����b�g�̎c�[���c������C��
			// 2005/09/29 ryoji �X�N���[�����ɃL�����b�g�̂悤�ȃS�~���\�����������C��
			if (IsTextSelected() && rcClip.right == rcClip.left &&
				m_nSelectLineFrom == m_nSelectLineTo &&
				m_nSelectColmFrom >= m_nViewLeftCol)
			{
				rcClip.right = rcClip.left + (nCharWidth/3 == 0 ? 1 : nCharWidth/3);
			}
			// 2006.03.28 Moca �E�B���h�E�����傫���Ɛ��������]���Ȃ������C��
			if( rcClip.right > m_nViewAlignLeft + m_nViewCx ){
				rcClip.right = m_nViewAlignLeft + m_nViewCx;
			}
			
			hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
			nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );
			hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
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
BOOL CEditView::IsSearchString( const char* pszData, int nDataLen, int nPos, int* pnSearchStart, int* pnSearchEnd )
{
	int		nKeyLength;

	//	From Here Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	*pnSearchStart = nPos;	// 2002.02.08 hor

	if( m_bCurSrchRegularExp ){
		/* �s���ł͂Ȃ�? */
		/* �s�������`�F�b�N�́ACBregexp�N���X�����Ŏ��{����̂ŕs�v 2003.11.01 ����� */

		/* �ʒu��0��MatchInfo�Ăяo���ƁA�s�������������ɁA�S�� true�@�ƂȂ�A
		** ��ʑS�̂����������񈵂��ɂȂ�s��C��
		** �΍�Ƃ��āA�s���� MacthInfo�ɋ����Ȃ��Ƃ����Ȃ��̂ŁA������̒����E�ʒu����^����`�ɕύX
		** 2003.05.04 �����
		*/
		if( m_CurRegexp.Match( pszData, nDataLen, nPos )
		){
			*pnSearchStart = m_CurRegexp.GetIndex();	// 2002.02.08 hor
			*pnSearchEnd = m_CurRegexp.GetLastIndex();
	//	To Here Jun. 26, 2001 genta
			return TRUE;

		}else{
			return FALSE;
		}
	}else{
		nKeyLength = lstrlen( m_szCurSrchKey );		/* �������� */

		// 2001/06/23 �P��P�ʂ̌����̂��߂ɒǉ�
		if( m_pShareData->m_Common.m_bWordOnly ){	/* �����^�u��  1==�P��̂݌��� */
			/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
			/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
			int nIdxFrom, nIdxTo;
			if( false == CDocLineMgr::WhereCurrentWord_2( pszData, nDataLen, nPos, &nIdxFrom, &nIdxTo, NULL, NULL ) ){
				return FALSE;
			}
			if( nPos != nIdxFrom || nKeyLength != nIdxTo - nIdxFrom ){
				return FALSE;
			}
		}

		//��������������` �܂��� ���������̒�����蒲�ׂ�f�[�^���Z���Ƃ��̓q�b�g���Ȃ�
		if( 0 == nKeyLength || nKeyLength > nDataLen - nPos ){
			return FALSE;
		}
		//�p�啶���������̋�ʂ����邩�ǂ���
		if( m_bCurSrchLoHiCase ){	/* 1==�p�啶���������̋�� */
			if( 0 == memcmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return TRUE;
			}
		}else{
			if( 0 == memicmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*! 
	���[���[�̃L�����b�g���ĕ`��	2002.02.25 Add By KK
	@param hdc [in] �f�o�C�X�R���e�L�X�g
	DispRuler�̓��e�����ɍ쐬
*/
inline void CEditView::DrawRulerCaret( HDC hdc )
{
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	HRGN		hRgn;
	RECT		rc;
	int			nROP_Old;

	if( m_nViewLeftCol <= m_nCaretPosX
	 && m_nViewLeftCol + m_nViewColNum + 2 >= m_nCaretPosX
	){
		if (m_nOldCaretPosX == m_nCaretPosX && m_nCaretWidth == m_nOldCaretWidth) {
			//�O�`�悵���ʒu�擯�� ���� ���[���[�̃L�����b�g�������� 
			return;
		}

		rc.left = m_nViewAlignLeft + ( m_nOldCaretPosX - m_nViewLeftCol ) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) + 1;
		rc.right = rc.left + m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace - 1;
		rc.top = 0;
		rc.bottom = m_nViewAlignTop - m_nTopYohaku - 1;

		//���ʒu���N���A m_nOldCaretWidth
		if( 0 == m_nOldCaretWidth ){
			hBrush = ::CreateSolidBrush( RGB( 128, 128, 128 ) );
		}else{
			hBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
		}
		nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
		hRgn = ::CreateRectRgnIndirect( &rc );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
		::PaintRgn( hdc, hRgn );
		::DeleteObject( hRgn );
		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );

		if( 0 == m_nCaretWidth ){
			hBrush = ::CreateSolidBrush( RGB( 128, 128, 128 ) );
		}else{
			hBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
		}

		//�V�����ʒu�ŕ`��
		rc.left = m_nViewAlignLeft + ( m_nCaretPosX - m_nViewLeftCol ) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) + 1;
		rc.right = rc.left + m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace - 1;

		hRgn = ::CreateRectRgnIndirect( &rc );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
		::SelectObject( hdc, hBrush );
		::PaintRgn( hdc, hRgn );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hRgn );
		::DeleteObject( hBrush );
		::SetROP2( hdc, nROP_Old );
	}
}



/*! ���[���[�`��

	@date 2005.08.14 genta �܂�Ԃ�����LayoutMgr����擾����悤��
*/
void CEditView::DispRuler( HDC hdc )
{

#ifdef _DEBUG
//	if( 0 != m_pShareData->m_Common.m_nRulerType ){	/* ���[���[�̃^�C�v */
//		DispRulerEx( hdc );
//		return;
//	}
#endif


	if( !m_bDrawSWITCH ){
		return;
	}
	if( !m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		return;
	}

	// 2002.02.25 Add By KK ���[���[�S�̂�`�������K�v���Ȃ��ꍇ�́A���[����̃L�����b�g�̂ݕ`���Ȃ��� 
	if ( !m_bRedrawRuler ) {
		DrawRulerCaret( hdc );
	}
	else {
		/* �`�揈�� */
		HBRUSH		hBrush;
		HBRUSH		hBrushOld;
		HRGN		hRgn;
		RECT		rc;
		int			i;
		int			nX;
		int			nY;
		LOGFONT		lf;
		HFONT		hFont;
 		HFONT		hFontOld;
		char		szColm[32];

		HPEN		hPen;
		HPEN		hPenOld;
		int			nROP_Old;
		COLORREF	colTextOld;
		int			nToX;
		const Types&	typeData = m_pcEditDoc->GetDocumentAttribute();
		const int	nMaxLineSize = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();

		/* LOGFONT�̏����� */
		memset( &lf, 0, sizeof(LOGFONT) );
		lf.lfHeight			= 1 - m_pShareData->m_Common.m_nRulerHeight;	//	2002/05/13 ai
		lf.lfWidth			= 5/*0*/;
		lf.lfEscapement		= 0;
		lf.lfOrientation	= 0;
		lf.lfWeight			= 400;
		lf.lfItalic			= 0;
		lf.lfUnderline		= 0;
		lf.lfStrikeOut		= 0;
		lf.lfCharSet		= 0;
		lf.lfOutPrecision	= 3;
		lf.lfClipPrecision	= 2;
		lf.lfQuality		= 1;
		lf.lfPitchAndFamily	= 34;
		strcpy( lf.lfFaceName, "Arial" );
		hFont = ::CreateFontIndirect( &lf );
		hFontOld = (HFONT)::SelectObject( hdc, hFont );
		::SetBkMode( hdc, TRANSPARENT );

		hBrush = ::CreateSolidBrush( typeData.m_ColorInfoArr[COLORIDX_RULER].m_colBACK );
		rc.left = 0;
		rc.top = 0;
		rc.right = m_nViewAlignLeft + m_nViewCx;
		rc.bottom = m_nViewAlignTop - m_nTopYohaku;
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );

		nX = m_nViewAlignLeft;
		nY = m_nViewAlignTop - m_nTopYohaku - 2;

		hPen = ::CreatePen( PS_SOLID, 0, typeData.m_ColorInfoArr[COLORIDX_RULER].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		colTextOld = ::SetTextColor( hdc, typeData.m_ColorInfoArr[COLORIDX_RULER].m_colTEXT );


		//nToX = m_nViewAlignLeft + m_nViewCx;
		//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
		//	2005.11.10 Moca 1dot����Ȃ�
		nToX = m_nViewAlignLeft + (nMaxLineSize - m_nViewLeftCol) * ( m_nCharWidth + typeData.m_nColmSpace ) + 1;
		if( nToX > m_nViewAlignLeft + m_nViewCx ){
			nToX = m_nViewAlignLeft + m_nViewCx;
		}
		::MoveToEx( hdc, m_nViewAlignLeft, nY + 1, NULL );
		::LineTo( hdc, nToX/*m_nViewAlignLeft + m_nViewCx*/, nY + 1 );


		for( i = m_nViewLeftCol;
			i <= m_nViewLeftCol + m_nViewColNum + 1
		 && i <= nMaxLineSize;
			i++
		){
			if( i == nMaxLineSize ){
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, 0/*nY - 8*/ );
			}
			if( 0 == ( (i) % 10 ) ){
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, 0/*nY - 8*/ );
				itoa( (i) / 10, szColm, 10 );
				::TextOut( hdc, nX + 2 + 0, -1 + 0, szColm, lstrlen( szColm ) );
			}else
			if( 0 == ( (i) % 5 ) ){
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, nY - 6 );
			}else{
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, nY - 3 );
			}
			nX += ( m_nCharWidth + typeData.m_nColmSpace );
		}
		::SetTextColor( hdc, colTextOld );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );

		/* �L�����b�g�`��i���݂̈ʒu�ɕ`�悷�邾���B�Â��ʒu�͂��łɏ�����Ă���j */
		if( m_nViewLeftCol <= m_nCaretPosX
		 && m_nViewLeftCol + m_nViewColNum + 2 >= m_nCaretPosX
		){
			//	Aug. 18, 2000 ����
			rc.left = m_nViewAlignLeft + ( m_nCaretPosX - m_nViewLeftCol ) * ( m_nCharWidth + typeData.m_nColmSpace ) + 1;
			rc.right = rc.left + m_nCharWidth + typeData.m_nColmSpace - 1;
			rc.top = 0;
			rc.bottom = m_nViewAlignTop - m_nTopYohaku - 1;

			if( 0 == m_nCaretWidth ){
				hBrush = ::CreateSolidBrush( RGB( 128, 128, 128 ) );
			}else{
				hBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
			}
			nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
			hRgn = ::CreateRectRgnIndirect( &rc );
			hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
			::PaintRgn( hdc, hRgn );

			::SelectObject( hdc, hBrushOld );
			::DeleteObject( hRgn );
			::DeleteObject( hBrush );
			::SetROP2( hdc, nROP_Old );
		}

		::SelectObject( hdc, hFontOld );
		::DeleteObject( hFont );

		m_bRedrawRuler = false;	//m_bRedrawRuler = true �Ŏw�肳���܂ŁA���[���̃L�����b�g�݂̂��ĕ`�� 2002.02.25 Add By KK
	}

	//�`�悵�����[���[�̃L�����b�g�ʒu�E����ۑ� 2002.02.25 Add By KK
	m_nOldCaretPosX = m_nCaretPosX;
	m_nOldCaretWidth = m_nCaretWidth ;

	return;
}

//======================================================================
//@@@ 2001.02.03 Start by MIK: �S�p�����̑Ί���
//! �S�p���ʂ̑Ή��\
const struct ZENKAKKO_T{
	char *sStr;
	char *eStr;
} zenkakkoarr[] = {
	"�y", "�z",
	"�w", "�x",
	"�u", "�v",
	"��", "��",
	"��", "��",
	"�s", "�t",
	"�i", "�j",
	"�q", "�r",
	"�o", "�p",
	"�k", "�l",
	"�m", "�n",
	"�g", "�h",
	"��", "��",
	NULL, NULL	//�I�[����
};
//@@@ �S�p�����̑Ί���: End
//@@@ 2003.01.06 Start by ai: ���p�����̑Ί���
//! ���p���ʂ̑Ή��\
const struct HANKAKKO_T{
	char *sStr;
	char *eStr;
} hankakkoarr[] = {
	"(", ")",
	"[", "]",
	"{", "}",
	"<", ">",
	"�", "�",
	NULL, NULL	//�I�[����
};
//@@@ ���p�����̑Ί���: End
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
bool CEditView::SearchBracket( int LayoutX, int LayoutY, int* NewX, int* NewY, int* mode )
{
	int len;	//	�s�̒���
	int nCharSize;	//	�i��������́j������
	int PosX, PosY;	//	�����ʒu

	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys( LayoutX, LayoutY, &PosX, &PosY );
	const char *cline = m_pcEditDoc->m_cDocLineMgr.GetLineStr( PosY, &len );

	//	Jun. 19, 2000 genta
	if( cline == NULL )	//	�Ō�̍s�ɖ{�����Ȃ��ꍇ
		return false;
//	PosX = LineColmnToIndex( cline, len, PosX );	�s�v

	// 2005-09-02 D.S.Koba GetSizeOfChar
	nCharSize = CMemory::GetSizeOfChar( cline, len, PosX );
	m_nCharSize = nCharSize;	// 02/09/18 �Ί��ʂ̕����T�C�Y�ݒ� ai

	if( nCharSize == 1 ){	//	1�o�C�g����
		// 03/01/06 ai Start
		int i;
		const struct HANKAKKO_T *p;
		p = hankakkoarr;
		for( i = 0; p->sStr != NULL; i++, p++ )
		{
			if( strncmp(p->sStr, &cline[PosX], 1) == 0 )
			{
				return SearchBracketForward( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );
			}
			else if( strncmp(p->eStr, &cline[PosX], 1) == 0 )
			{
				return SearchBracketBackward( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );
			}
		}
		// 03/01/06 ai End
//@@@ 2001.02.03 Start by MIK: �S�p�����̑Ί���
	}else if( nCharSize == 2 ){	// 2�o�C�g����
		int i;
		const struct ZENKAKKO_T *p;
		p = zenkakkoarr;
		for(i = 0; p->sStr != NULL; i++, p++)
		{
			if(strncmp(p->sStr, &cline[PosX], 2) == 0)
			{
				return SearchBracketForward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );		// mode�̒ǉ� 02/09/19 ai
			}
			else if(strncmp(p->eStr, &cline[PosX], 2) == 0)
			{
				return SearchBracketBackward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );	// mode�̒ǉ� 02/09/19 ai
			}
		}
//@@@ 2001.02.03 End: �S�p�����̑Ί���
	}

	// 02/09/18 ai Start
	if( 0 == ( *mode & 2 ) ){
		/* �J�[�\���̑O���𒲂ׂȂ��ꍇ */
		return false;
	}
	*mode |= 4;
	// 02/09/18 ai End

	//	���ʂ�������Ȃ�������C�J�[�\���̒��O�̕����𒲂ׂ�

	if( PosX <= 0 ){
//		::MessageBox( NULL, "NO DATA", "Bracket", MB_OK );
		return false;	//	�O�̕����͂Ȃ�
	}
	const char *bPos = CMemory::MemCharPrev( cline, PosX, cline + PosX );
	nCharSize = cline + PosX - bPos;
	m_nCharSize = nCharSize;	// 02/10/01 �Ί��ʂ̕����T�C�Y�ݒ� ai
	if( nCharSize == 1 ){	//	1�o�C�g����
		// 03/01/06 ai Start
		int i;
		const struct HANKAKKO_T *p;
		PosX = bPos - cline;
		p = hankakkoarr;
		for( i = 0; p->sStr != NULL; i++, p++ )
		{
			if( strncmp(p->sStr, &cline[PosX], 1) == 0 )
			{
				return SearchBracketForward( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );
			}
			else if( strncmp(p->eStr, &cline[PosX], 1) == 0 )
			{
				return SearchBracketBackward( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );
			}
		}
		// 03/01/06 ai End
//@@@ 2001.02.03 Start by MIK: �S�p�����̑Ί���
	}else if( nCharSize == 2 ){	// 2�o�C�g����
		int i;
		const struct ZENKAKKO_T *p;
		PosX = bPos - cline;
		p = zenkakkoarr;
		for( i = 0; p->sStr != NULL; i++, p++ )
		{
			if( strncmp(p->sStr, &cline[PosX], 2) == 0 )
			{
				return SearchBracketForward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );		// mode�̒ǉ� 02/09/19 ai
			}
			else if( strncmp(p->eStr, &cline[PosX], 2) == 0 )
			{
				return SearchBracketBackward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr, mode );	// mode�̒ǉ� 02/09/19 ai
			}
		}
//@@@ 2001.02.03 End: �S�p�����̑Ί���
	}
	return false;
}

/*!
	@brief ���p�Ί��ʂ̌���:������

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
bool CEditView::SearchBracketForward( int PosX, int PosY, int* NewX, int* NewY,
									char* upChar, char* dnChar, int* mode )	// 03/01/08 ai
{
	CDocLine* ci;

	int			len;
	const char	*cPos, *nPos;
	char		*cline, *lineend;
	int			level = 0;
	int			nCol, nLine, nSearchNum;	// 02/09/19 ai

//	char buf[50];	Debug�p

	//	�����ʒu�̐ݒ�
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, &nCol, &nLine );	// 02/09/19 ai
	nSearchNum = ( m_nViewTopLine + m_nViewRowNum ) - nLine;					// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos < lineend ){
			nPos = CMemory::MemCharNext( cline, len, cPos );
			if( nPos - cPos > 1 ){
				//	skip
				cPos = nPos;
				continue;
			}
			// 03/01/08 ai Start
			if( strncmp(upChar, cPos, 1) == 0 ){
				++level;
			}
			else if( strncmp(dnChar, cPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	���������I
				PosX = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
//				wsprintf( buf, "Layout: %d, %d\nPhys: %d, %d", *NewX, *NewY, PosX, PosY );
//				::MessageBox( NULL, buf, "Bracket", MB_OK );
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
		++PosY;
		ci = ci->m_pNext;	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->m_pLine->GetPtr( &len );
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
bool CEditView::SearchBracketBackward( int PosX, int PosY, int* NewX, int* NewY,
									char* dnChar, char* upChar, int* mode )
{
	CDocLine* ci;

	int			len;
	const char	*cPos, *pPos;
	char		*cline, *lineend;
	int			level = 1;
	int			nCol, nLine, nSearchNum;	// 02/09/19 ai

//	char buf[50];	Debug�p

	//	�����ʒu�̐ݒ�
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, &nCol, &nLine );	// 02/09/19 ai
	nSearchNum = nLine - m_nViewTopLine;										// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos > cline ){
			pPos = CMemory::MemCharPrev( cline, len, cPos );
			if( cPos - pPos > 1 ){
				//	skip
				cPos = pPos;
				continue;
			}
			// 03/01/08 ai Start
			if( strncmp(upChar, pPos, 1) == 0 ){
				++level;
			}
			else if( strncmp(dnChar, pPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	���������I
				PosX = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
//				wsprintf( buf, "Layout: %d, %d\nPhys: %d, %d", *NewX, *NewY, PosX, PosY );
//				::MessageBox( NULL, buf, "Bracket", MB_OK );
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
		--PosY;
		ci = ci->m_pPrev;	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->m_pLine->GetPtr( &len );
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
bool CEditView::SearchBracketForward2(  int		PosX,	int		PosY,
										int*	NewX,	int*	NewY,
										char*	upChar,	char*	dnChar,
										int*	mode )
{
	CDocLine* ci;

	int len;
	const char *cPos, *nPos;
	char *cline, *lineend;
	int level = 0;
	int			nCol, nLine, nSearchNum;	// 02/09/19 ai

	//	�����ʒu�̐ݒ�
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, &nCol, &nLine );	// 02/09/19 ai
	nSearchNum = ( m_nViewTopLine + m_nViewRowNum ) - nLine;					// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos < lineend ){
			nPos = CMemory::MemCharNext( cline, len, cPos );
			if( nPos - cPos != 2 ){
				//	skip
				cPos = nPos;
				continue;
			}
			if( strncmp(upChar, cPos, 2) == 0 ){
				++level;
			}
			else if( strncmp(dnChar, cPos, 2) == 0 ){
				--level;
			}

			if( level == 0 ){	//	���������I
				PosX = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
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
		++PosY;
		ci = ci->m_pNext;	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->m_pLine->GetPtr( &len );
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
bool CEditView::SearchBracketBackward2( int   PosX,   int   PosY,
									    int*  NewX,   int*  NewY,
									    char* dnChar, char* upChar,
										int*  mode )
{
	CDocLine* ci;

	int len;
	const char *cPos, *pPos;
	char *cline, *lineend;
	int level = 1;
	int	nCol, nLine, nSearchNum;	// 02/09/19 ai

	//	�����ʒu�̐ݒ�
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, &nCol, &nLine );	// 02/09/19 ai
	nSearchNum = nLine - m_nViewTopLine;										// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos > cline ){
			pPos = CMemory::MemCharPrev( cline, len, cPos );
			if( cPos - pPos != 2 ){
				//	skip
				cPos = pPos;
				continue;
			}
			if( strncmp(upChar, pPos, 2) == 0 ){
				++level;
			}
			else if( strncmp(dnChar, pPos, 2) == 0 ){
				--level;
			}

			if( level == 0 ){	//	���������I
				PosX = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
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
		--PosY;
		ci = ci->m_pPrev;	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->m_pLine->GetPtr( &len );
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
bool CEditView::IsBracket( const char *pLine, int x, int size )
{
	int	i;
	if( size == 1 ){
		const struct HANKAKKO_T *p;
		p = hankakkoarr;
		for( i = 0; p->sStr != NULL; i++, p++ )
		{
			if( strncmp( p->sStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
			else if( strncmp( p->eStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
		}
	}
	else if( size == 2 ) {
		const struct ZENKAKKO_T *p;
		p = zenkakkoarr;
		for( i = 0; p->sStr != NULL; i++, p++ )
		{
			if( strncmp( p->sStr, &pLine[x], 2 ) == 0 )
			{
				return true;
			}
			else if( strncmp( p->eStr, &pLine[x], 2 ) == 0 )
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
	int PosX, PosY;	//	�����ʒu�i���s�P�ʂ̌v�Z�j

	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys( m_nCaretPosX, m_nCaretPosY, &PosX, &PosY );

	CMarkMgr::CMark m( PosX, PosY );
	m_cHistory->Add( m );

}


//	2001/06/18 Start by asa-o: �⊮�E�B���h�E�p�̃L�[���[�h�w���v�\��
bool  CEditView::ShowKeywordHelp( POINT po, LPCTSTR pszHelp, LPRECT prcHokanWin)
{
	CMemory		cmemCurText;
	CMemory*	pcmemRefText;
	LPSTR		pszWork;
	RECT		rcTipWin,
				rcDesktop;

	if( m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp ){ /* �L�[���[�h�w���v���g�p���� */
		if( m_bInMenuLoop == FALSE	&&	/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ� */
			0 != m_dwTipTimer			/* ����Tip��\�����Ă��Ȃ� */
		){
			cmemCurText.SetDataSz( pszHelp );

			/* ���Ɍ����ς݂� */
			if( CMemory::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) ){
				/* �Y������L�[���Ȃ����� */
				if( !m_cTipWnd.m_KeyWasHit ){
					return false;
				}
			}else{
				m_cTipWnd.m_cKey = cmemCurText;
				/* �������s */
				if( m_cDicMgr.Search( cmemCurText.GetPtr(), &pcmemRefText, m_pcEditDoc->GetDocumentAttribute().m_szKeyWordHelpFile ) ){
					/* �Y������L�[������ */
					m_cTipWnd.m_KeyWasHit = TRUE;
					pszWork = pcmemRefText->GetPtr();
//								m_cTipWnd.m_cInfo.SetData( pszWork, lstrlen( pszWork ) );
					m_cTipWnd.m_cInfo.SetDataSz( pszWork );
					delete pcmemRefText;
				}else{
					/* �Y������L�[���Ȃ����� */
					m_cTipWnd.m_KeyWasHit = FALSE;
					return false;
				}
			}
			m_dwTipTimer = 0;	/* ����Tip��\�����Ă��� */

		// 2001/06/19 Start by asa-o: ����Tip�̕\���ʒu����
			// ����Tip�̃T�C�Y���擾
			m_cTipWnd.GetWindowSize(&rcTipWin);

			//	May 01, 2004 genta �}���`���j�^�Ή�
			::GetMonitorWorkRect( m_cTipWnd.m_hWnd, &rcDesktop );

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

	x1 = m_nCharWidth / 3;
	y1 = m_nCharHeight / 5;

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

		x += m_nCharWidth;
	}

	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );

	return nLength;
}
#endif


/*[EOF]*/
