//	$Id$
/*!	@file
	@brief CEditView�N���X

	@author Norio Nakatani
	@date	1998/12/08 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, mik, asa-o
	Copyright (C) 2001, hor, MIK 
	Copyright (C) 2002, YAZAKI, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditView.h"
#include "debug.h"
//#include "keycode.h"
#include "funccode.h"
//#include "CRunningTimer.h" 2002/2/10 aroka
#include "charcode.h"
#include "mymessage.h"
//#include "CWaitCursor.h"
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
	case COLORIDX_URL:	    /* URL�ł��� */
	case COLORIDX_SEARCH:	/* ����������ł��� */
		nColorIdx = nCOMMENTMODE;
		break;
	case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */
	case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */
	case COLORIDX_BLOCK2:	/* �u���b�N�R�����g2�ł��� */	//@@@ 2001.03.10 by MIK
		nColorIdx = COLORIDX_COMMENT;
		break;
#if 0
	case 0:
		nColorIdx = COLORIDX_TEXT;
//		colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colTEXT;	/* �e�L�X�g�F */
//		colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK;	/* �w�i�F */
//		::SetTextColor( hdc, colText );
//		::SetBkColor( hdc, colBack );
//		if( NULL != m_hFontOld ){
//			::SelectObject( hdc, m_hFontOld );
//			m_hFontOld = NULL;
//		}

		break;
	case 1:	/* �s�R�����g�ł��� */
	case 2:	/* �u���b�N�R�����g�ł��� */
//#ifdef	COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	case 20:	/* �u���b�N�R�����g�ł��� */	//@@@ 2001.03.10 by MIK
//#endif
		nColorIdx = COLORIDX_COMMENT;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp ){	/* �R�����g��\������ */
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_COMMENT].m_colTEXT;	/* �R�����g�F */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_COMMENT].m_colBACK;	/* �R�����g�w�i�̐F */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_COMMENT].m_bFatFont ){	/* ������ */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
	case 3:	/* �V���O���N�H�[�e�[�V����������ł��� */
		nColorIdx = COLORIDX_SSTRING;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp ){	/* �V���O���N�H�[�e�[�V�����������\������ */
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_colTEXT;	/* �V���O���N�H�[�e�[�V����������F */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_colBACK;	/* �V���O���N�H�[�e�[�V����������w�i�̐F */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bFatFont ){	/* ������ */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
	case 4:	/* �_�u���N�H�[�e�[�V����������ł��� */
		nColorIdx = COLORIDX_WSTRING;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp ){	/* �_�u���N�H�[�e�[�V�����������\������ */
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_colTEXT;	/* �_�u���N�H�[�e�[�V����������F */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_colBACK;	/* �_�u���N�H�[�e�[�V����������w�i�̐F */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bFatFont ){	/* ������ */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
	case 5:	/* �L�[���[�h�i�o�^�P��j������ł��� */
		nColorIdx = COLORIDX_KEYWORD;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_KEYWORD].m_bDisp ){
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_KEYWORD].m_colTEXT;	/* �����L�[���[�h�̐F */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_KEYWORD].m_colBACK;	/* �����L�[���[�h�w�i�̐F */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_KEYWORD].m_bFatFont ){	/* ������ */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
	case 6:	/* �R���g���[���R�[�h�ł��� */
		nColorIdx = COLORIDX_CTRLCODE;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp ){
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CTRLCODE].m_colTEXT;	/* �R���g���[���R�[�h�̐F */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CTRLCODE].m_colBACK;	/* �R���g���[���R�[�h�̔w�i�F */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CTRLCODE].m_bFatFont ){	/* ������ */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
//@@@ 2001.02.17 Start by MIK: ���p���l�������\��
//#ifdef COMPILE_COLOR_DIGIT
	case 9:	/* ���p���l�ł��� */
		nColorIdx = COLORIDX_DIGIT;
		break;
//#endif
//@@@ 2001.02.17 End by MIK: ���p���l�������\��
	case 50:	/* �L�[���[�h2�i�o�^�P��j������ł��� */	//MIK
		nColorIdx = COLORIDX_KEYWORD2;						//MIK
		break;												//MIK
	case 80:	/* URL�ł��� */
		nColorIdx = COLORIDX_URL;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_bDisp ){
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_colTEXT;	/* URL */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_colBACK;	/* URL */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_bFatFont ){	/* URL */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
	case 90:	/* ����������ł��� */
		nColorIdx = COLORIDX_SEARCH;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SEARCH].m_colTEXT;	/* URL */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SEARCH].m_colBACK;	/* URL */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SEARCH].m_bFatFont ){	/* URL */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
#endif

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
		colBkColorOld = ::SetBkColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colBACK );		/* �s�ԍ��w�i�̐F */

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
		rcClip.left = m_nViewAlignLeft - 3;
		rcClip.right = m_nViewAlignLeft;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOut( hdc,
			(m_nViewAlignLeftCols - nLineCols - 1) * ( nCharWidth ),
			y, fuOptions,
			&rcClip, " ", 1, m_pnDx
		);


//		/* �s�ԍ��̃e�L�X�g��\�� */
//		m_pShareData->m_Types[nIdx].m_nLineTermType = 1;			/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
//		m_pShareData->m_Types[nIdx].m_cLineTermChar = ':';			/* �s�ԍ���؂蕶�� */

		/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
		if( 2 == m_pcEditDoc->GetDocumentAttribute().m_nLineTermType ){
			char szLineTerm[2];
			wsprintf( szLineTerm, "%c", m_pcEditDoc->GetDocumentAttribute().m_cLineTermChar );	/* �s�ԍ���؂蕶�� */
			strcat( szLineNum, szLineTerm );
		}
		rcClip.left = 0;
		rcClip.right = m_nViewAlignLeft/* - 3*/;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOut( hdc,
			(m_nViewAlignLeftCols - nLineCols - 1) * ( nCharWidth ),
			y, fuOptions,
			&rcClip, szLineNum, lstrlen( szLineNum ), m_pnDx
		);


//		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_colorGYOU );


		/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
		if( 1 == m_pcEditDoc->GetDocumentAttribute().m_nLineTermType ){
			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );
			hPenOld = (HPEN)::SelectObject( hdc, hPen );
			::MoveToEx( hdc, m_nViewAlignLeft - 4, y, NULL );
			::LineTo( hdc, m_nViewAlignLeft - 4, y + nLineHeight );
			::SelectObject( hdc, hPenOld );
			::DeleteObject( hPen );
		}
		::SetTextColor( hdc, colTextColorOld );
		::SetBkColor( hdc, colBkColorOld );

//		colBkColorOld = ::SetBkColor( hdc, RGB( 255, 0, 0 ) );






//		::SetBkColor( hdc, colBkColorOld );


		::SelectObject( hdc, hFontOld );
	}else{
		rcClip.left = 0;
		rcClip.right = m_nViewAlignLeft;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
//		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_colorBACK );
		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex/*COLORIDX_TEXT*/].m_colBACK );
		::FillRect( hdc, &rcClip, hBrush );
		::DeleteObject( hBrush );
	}

// From Here 2001.12.03 hor
	/* �Ƃ肠�����u�b�N�}�[�N�ɏc�� */
	if(pCDocLine->IsBookMarked()){
		hPen = ::CreatePen( PS_SOLID, 2, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, 1, y, NULL );
		::LineTo( hdc, 1, y + nLineHeight );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
	}
// To Here 2001.12.03 hor

	return;
}




/* �e�L�X�g�\�� */
int CEditView::DispText( HDC hdc, int x, int y, const unsigned char* pData, int nLength )
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
//		::ExtTextOut( hdc, x, y, fuOptions, &rcClip, (const char *)pData, nLength, m_pnDx );
		//@@@	From Here 2002.01.30 YAZAKI ExtTextOut�̐������
		if( rcClip.right - rcClip.left > m_nViewCx ){
			rcClip.right = rcClip.left + m_nViewCx;
		}
		int nBefore = 0;	//	�E�B���h�E�̍��ɂ��ӂꂽ������
		int nAfter = 0;		//	�E�B���h�E�̉E�ɂ��ӂꂽ������
		if ( x < 0 ){
			int nLeft = ( 0 - x ) / nCharWidth - 1;
			while (nBefore < nLeft){
				nBefore += CMemory::MemCharNext( (const char *)pData, nLength, (const char *)&pData[nBefore] ) - (const char *)&pData[nBefore];
			}
		}
		if ( rcClip.right < x + nCharWidth * nLength ){
			//	-1���Ă��܂����i������͂�����ˁH�j
			nAfter = (x + nCharWidth * nLength - rcClip.right) / nCharWidth - 1;
		}
		::ExtTextOut( hdc, x + nBefore * nCharWidth, y, fuOptions, &rcClip, (const char *)&pData[nBefore], nLength - nBefore - nAfter, m_pnDx );
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
						nSelectFrom = 0;
						nSelectTo   = m_nSelectColmTo;
					}else{
						nSelectFrom = 0;
						nSelectTo   = nX;
					}
				}
			}
			if( nSelectFrom < m_nViewLeftCol ){
				nSelectFrom = m_nViewLeftCol;
			}
			if( nSelectTo < m_nViewLeftCol ){
				nSelectTo = m_nViewLeftCol;
			}
			rcClip.left   = x + nSelectFrom * ( nCharWidth );
			rcClip.right  = x + nSelectTo   * ( nCharWidth );
			rcClip.top    = y;
			rcClip.bottom = y + nLineHeight;
			if( rcClip.right - rcClip.left > 3000 ){
				rcClip.right = rcClip.left + 3000;
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
	BREGEXP* result;
	*pnSearchStart = nPos;	// 2002.02.08 hor

	if( m_bCurSrchRegularExp ){
		/* �s���ł͂Ȃ�? */
//		if( ( ( m_szCurSrchKey[0] == '/' && '^' == m_szCurSrchKey[1] )
//			|| ( m_szCurSrchKey[0] == 'm' && '^' == m_szCurSrchKey[2] ) )
		if( ( m_szCurSrchKey[0] == '^' )
			&& 0 != nPos ){
			return FALSE;
		}

		if( m_CurRegexp.GetMatchInfo( &pszData[nPos], nDataLen - nPos, 0, &result )
	//	 && ( result->startp[0] == &pszData[nPos] )			// 2002.02.08 hor
		){
			*pnSearchStart = result->startp[0] - pszData;	// 2002.02.08 hor
			*pnSearchEnd = result->endp[0] - pszData;
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



/*! ���[���[�`�� */
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

		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_colBACK );
		rc.left = 0;
		rc.top = 0;
		rc.right = m_nViewAlignLeft + m_nViewCx;
		rc.bottom = m_nViewAlignTop - m_nTopYohaku;
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );

		nX = m_nViewAlignLeft;
		nY = m_nViewAlignTop - m_nTopYohaku - 2;

		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		colTextOld = ::SetTextColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_colTEXT );


		nToX = m_nViewAlignLeft + m_nViewCx;

		nToX = m_nViewAlignLeft + (m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - m_nViewLeftCol) * ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		if( nToX > m_nViewAlignLeft + m_nViewCx ){
			nToX = m_nViewAlignLeft + m_nViewCx;
		}
		::MoveToEx( hdc, m_nViewAlignLeft, nY + 1, NULL );
		::LineTo( hdc, nToX/*m_nViewAlignLeft + m_nViewCx*/, nY + 1 );


		for( i = m_nViewLeftCol;
			i <= m_nViewLeftCol + m_nViewColNum + 1
		 && i <= m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize;
			i++
		){
			if( i == m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize ){
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
			nX += ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		}
		::SetTextColor( hdc, colTextOld );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );

		/* �L�����b�g�`��i���݂̈ʒu�ɕ`�悷�邾���B�Â��ʒu�͂��łɏ�����Ă���j */
		if( m_nViewLeftCol <= m_nCaretPosX
		 && m_nViewLeftCol + m_nViewColNum + 2 >= m_nCaretPosX
		){
			//	Aug. 18, 2000 ����
			rc.left = m_nViewAlignLeft + ( m_nCaretPosX - m_nViewLeftCol ) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) + 1;
			rc.right = rc.left + m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace - 1;
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
	@param upChar [in] ���ʂ̎n�܂�̕���
	@param dnChar [in] ���ʂ���镶����

	@retval true ����
	@retval false ���s

	@author genta
	@date Jun. 16, 2000 genta
	@date Feb. 03, 2001 MIK �S�p���ʂɑΉ�

*/
bool CEditView::SearchBracket( int LayoutX, int LayoutY, int* NewX, int* NewY )
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

	nCharSize = CMemory::MemCharNext( cline, len, cline + PosX ) - cline - PosX;

	if( nCharSize == 1 ){	//	1�o�C�g����
//		char buf[] = "Bracket:  Forward";
//		buf[8] = cline[ PosX ];
//		::MessageBox( NULL, buf, "Bracket", MB_OK );

		switch( cline[ PosX ] ){
		case '(':	return SearchBracketForward( PosX, PosY, NewX, NewY, '(', ')' );
		case '[':	return SearchBracketForward( PosX, PosY, NewX, NewY, '[', ']' );
		case '{':	return SearchBracketForward( PosX, PosY, NewX, NewY, '{', '}' );
		case '<':	return SearchBracketForward( PosX, PosY, NewX, NewY, '<', '>' );

		case ')':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '(', ')' );
		case ']':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '[', ']' );
		case '}':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '{', '}' );
		case '>':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '<', '>' );
		}
//@@@ 2001.02.03 Start by MIK: �S�p�����̑Ί���
	}else if( nCharSize == 2 ){	// 2�o�C�g����
		int i;
		const struct ZENKAKKO_T *p;
		p = zenkakkoarr;
		for(i = 0; p->sStr != NULL; i++, p++)
		{
			if(strncmp(p->sStr, &cline[PosX], 2) == 0)
			{
				return SearchBracketForward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr );
			}
			else if(strncmp(p->eStr, &cline[PosX], 2) == 0)
			{
				return SearchBracketBackward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr );
			}
		}
//@@@ 2001.02.03 End: �S�p�����̑Ί���
	}

	//	���ʂ�������Ȃ�������C�J�[�\���̒��O�̕����𒲂ׂ�

	if( PosX <= 0 ){
//		::MessageBox( NULL, "NO DATA", "Bracket", MB_OK );
		return false;	//	�O�̕����͂Ȃ�
	}
	const char *bPos = CMemory::MemCharPrev( cline, PosX, cline + PosX );
	nCharSize = cline + PosX - bPos;
	if( nCharSize == 1 ){	//	1�o�C�g����
		PosX = bPos - cline;

//		char buf[] = "Bracket:  Back";
//		buf[8] = cline[ PosX ];
//		::MessageBox( NULL, buf, "Bracket", MB_OK );

		switch( cline[ PosX ] ){
		case '(':	return SearchBracketForward( PosX, PosY, NewX, NewY, '(', ')' );
		case '[':	return SearchBracketForward( PosX, PosY, NewX, NewY, '[', ']' );
		case '{':	return SearchBracketForward( PosX, PosY, NewX, NewY, '{', '}' );
		case '<':	return SearchBracketForward( PosX, PosY, NewX, NewY, '<', '>' );

		case ')':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '(', ')' );
		case ']':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '[', ']' );
		case '}':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '{', '}' );
		case '>':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '<', '>' );
		}
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
				return SearchBracketForward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr );
			}
			else if( strncmp(p->eStr, &cline[PosX], 2) == 0 )
			{
				return SearchBracketBackward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr );
			}
		}
//@@@ 2001.02.03 End: �S�p�����̑Ί���
	}
	return false;
}

/*!
	@brief ���p�Ί��ʂ̌���:������

	@author genta

	@param LayoutX [in] �����J�n�_�̕������WX
	@param LayoutY [in] �����J�n�_�̕������WY
	@param NewX [out] �ړ���̃��C�A�E�g���WX
	@param NewY [out] �ړ���̃��C�A�E�g���WY
	@param upChar [in] ���ʂ̎n�܂�̕���
	@param dnChar [in] ���ʂ���镶����

	@retval true ����
	@retval false ���s
*/
bool CEditView::SearchBracketForward( int PosX, int PosY, int* NewX, int* NewY,
									int upChar, int dnChar )
{
	CDocLine* ci;

	int			len;
	const char	*cPos, *nPos;
	char		*cline, *lineend;
	int			level = 0;

//	char buf[50];	Debug�p

	//	�����ʒu�̐ݒ�
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
			if( *cPos == upChar )		++level;
			else if( *cPos == dnChar )	--level;

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

	@param LayoutX [in] �����J�n�_�̕������WX
	@param LayoutY [in] �����J�n�_�̕������WY
	@param NewX [out] �ړ���̃��C�A�E�g���WX
	@param NewY [out] �ړ���̃��C�A�E�g���WY
	@param upChar [in] ���ʂ̎n�܂�̕���
	@param dnChar [in] ���ʂ���镶����

	@retval true ����
	@retval false ���s
*/
bool CEditView::SearchBracketBackward( int PosX, int PosY, int* NewX, int* NewY,
									int dnChar, int upChar )
{
	CDocLine* ci;

	int			len;
	const char	*cPos, *pPos;
	char		*cline, *lineend;
	int			level = 1;

//	char buf[50];	Debug�p

	//	�����ʒu�̐ݒ�
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
			if( *pPos == upChar )		++level;
			else if( *pPos == dnChar )	--level;

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

	@param LayoutX [in] �����J�n�_�̕������WX
	@param LayoutY [in] �����J�n�_�̕������WY
	@param NewX [out] �ړ���̃��C�A�E�g���WX
	@param NewY [out] �ړ���̃��C�A�E�g���WY
	@param upChar [in] ���ʂ̎n�܂�̕����ւ̃|�C���^
	@param dnChar [in] ���ʂ���镶����ւ̃|�C���^

	@retval true ����
	@retval false ���s
*/
bool CEditView::SearchBracketForward2(  int		PosX,	int		PosY,
										int*	NewX,	int*	NewY,
										char*	upChar,	char*	dnChar )
{
	CDocLine* ci;

	int len;
	const char *cPos, *nPos;
	char *cline, *lineend;
	int level = 0;

	//	�����ʒu�̐ݒ�
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

	@param LayoutX [in] �����J�n�_�̕������WX
	@param LayoutY [in] �����J�n�_�̕������WY
	@param NewX [out] �ړ���̃��C�A�E�g���WX
	@param NewY [out] �ړ���̃��C�A�E�g���WY
	@param upChar [in] ���ʂ̎n�܂�̕����ւ̃|�C���^
	@param dnChar [in] ���ʂ���镶����ւ̃|�C���^

	@retval true ����
	@retval false ���s
*/
bool CEditView::SearchBracketBackward2( int   PosX,   int   PosY,
									    int*  NewX,   int*  NewY,
									    char* dnChar, char* upChar )
{
	CDocLine* ci;

	int len;
	const char *cPos, *pPos;
	char *cline, *lineend;
	int level = 1;

	//	�����ʒu�̐ݒ�
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

			::SystemParametersInfo( SPI_GETWORKAREA, NULL, &rcDesktop, 0 );

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


/*[EOF]*/
