//	$Id$
/*!	@file
	CEditView�N���X

	@author Norio Nakatani
	@date	1998/12/08 �쐬
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

//#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditView.h"
#include "debug.h"
#include "keycode.h"
#include "funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
#include "etc_uty.h"

/*! �t�H���g��I��
	@param bFat TRUE�ő���
	@param bUnderLine TRUE�ŃA���_�[���C��
*/
HFONT CEditView::ChooseFontHandle( BOOL bFat, BOOL bUnderLine )
{
	if( bFat ){	/* ������ */
		if( bUnderLine ){	/* ������ */
			return m_hFont_HAN_FAT_UL;
		}else{
			return m_hFont_HAN_FAT;
		}
	}else{
		if( bUnderLine ){	/* ������ */
			return m_hFont_HAN_UL;
		}else{
			return m_hFont_HAN;
		}
	}
}


/* �ʏ�̕`�揈�� new */
void CEditView::OnPaint( HDC hdc, PAINTSTRUCT *pPs, BOOL bUseMemoryDC )
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CEditView::OnPaint" );
#endif
//	if( m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
//		return;
//	}
	if( !m_bDrawSWITCH ){
		return;
	}
	/* �L�����b�g���B�� */
	::HideCaret( m_hWnd );

	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());


	int				i;
	HFONT			hFontOld;
	HBRUSH			hBrush;
	COLORREF		crBackOld;
	COLORREF		crTextOld;
	HDC				hdcOld;
	int				nTop;
	RECT			rc;
	RECT			rcBack;
	BOOL			bEOF;
	int				nLineHeight = m_nCharHeight + TypeDataPtr->m_nLineSpace;
	int				nCharWidth = m_nCharWidth + TypeDataPtr->m_nColmSpace;
	int				nLineTo = m_nViewTopLine + m_nViewRowNum + 1;
	int				nX = m_nViewAlignLeft - m_nViewLeftCol * nCharWidth;
	int				nY;
	BOOL			bDispBkBitmap = /*TRUE*/FALSE;
	const CLayout*	pcLayout;
	HPEN			hPen;
	HPEN			hPenOld;

	/* �L�����b�g�̍s���ʒu��\������ */
//	DrawCaretPosInfo();

	/* �������c�b�𗘗p�����ĕ`��̏ꍇ�͕`���̂c�b��؂�ւ��� */
	bUseMemoryDC = FALSE;
	if( bUseMemoryDC ){
		hdcOld = hdc;
		hdc = m_hdcCompatDC;
	}

	/* ���[���[�ƃe�L�X�g�̊Ԃ̗]�� */
	hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
	rc.left = 0;
	rc.top = m_nViewAlignTop - m_nTopYohaku;
	rc.right = m_nViewAlignLeft + m_nViewCx;
	rc.bottom = m_nViewAlignTop;
	::FillRect( hdc, &rc, hBrush );
	::DeleteObject( hBrush );
	
	//	From Here Sep. 7, 2001 genta
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_GYOU].m_bDisp ){ 
		rc.left = 0;
		rc.top = m_nViewAlignTop - m_nTopYohaku;
		rc.right = m_nViewAlignLeft;
		rc.bottom = m_nViewAlignTop;
		hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_GYOU].m_colBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}	
	//	To Here Sep. 7, 2001 genta

	::SetBkMode( hdc, TRANSPARENT );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
	m_hFontOld = NULL;


//	crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_colorBACK );
//	crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_colorTEXT );
	crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
	crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colTEXT );

//	::FillRect( hdc, &pPs->rcPaint, ::GetStockObject( WHITE_BRUSH ) );


	nTop = pPs->rcPaint.top;
	bEOF = FALSE;
//	i = m_nViewTopLine;

	if( 0 > nTop - m_nViewAlignTop ){
		i = m_nViewTopLine;
	}else{
		i = m_nViewTopLine + ( ( nTop - m_nViewAlignTop ) / nLineHeight );
	}

	int nMaxRollBackLineNum = 260 / TypeDataPtr->m_nMaxLineSize + 1;
	int nRollBackLineNum;
	nRollBackLineNum = 0;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( i );
	while( nRollBackLineNum < nMaxRollBackLineNum ){
		pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( i );
		if( NULL == pcLayout ){
			break;
		}
		if( 0 == pcLayout->m_nOffset ){	/* �Ή�����_���s�̐擪����̃I�t�Z�b�g */
			break;
		}
		i--;
		nRollBackLineNum++;
	}


//	if( NULL != pcLayout ){
//		if( 0 != pcLayout->m_nOffset ){	/* �Ή�����_���s�̐擪����̃I�t�Z�b�g */
//			pcLayout = pcLayout->m_pPrev;
//			i--;
//			if( NULL != pcLayout ){
//				if( 0 != pcLayout->m_nOffset ){	/* �Ή�����_���s�̐擪����̃I�t�Z�b�g */
//					pcLayout = pcLayout->m_pPrev;
//					i--;
//				}
//			}
//		}
//	}


	nY = ( i - m_nViewTopLine ) * nLineHeight + m_nViewAlignTop;
	nLineTo = m_nViewTopLine + ( ( pPs->rcPaint.bottom - m_nViewAlignTop ) / nLineHeight );

	BOOL bSelected;
	bSelected = IsTextSelected();
	for( ; i <= nLineTo; ){
//		pcLayout = m_pcEditDoc->m_cLayoutMgr.GetLineData( i );
		pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( i );
#ifdef _DEBUG
		{
			if( NULL != pcLayout ){
				if( (void*)0xdddddddd == (void*)pcLayout->m_pCDocLine->m_pLine ){
					m_pcEditDoc->m_cDocLineMgr.DUMP();
					m_pcEditDoc->m_cLayoutMgr.DUMP();

					pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( i );
				}

				int nLineLen = pcLayout->m_pCDocLine->m_pLine->m_nDataLen - pcLayout->m_nOffset;
				const char * pLine = (const char *)pcLayout->m_pCDocLine->m_pLine->m_pData + pcLayout->m_nOffset;
			}
		}
#endif

		if( DispLineNew(
			hdc,
			pcLayout,
			i,
			nX,
			nY,
			bDispBkBitmap,
			nLineTo,
			bSelected
		) ){
			pPs->rcPaint.bottom += nLineHeight;	/* EOF�ĕ`��Ή� */
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

//	if( bEOF ){
//		nTop = ( i + 1 - m_nViewTopLine ) * nLineHeight + m_nViewAlignTop;
//	}else{
//		nTop = ( i	   - m_nViewTopLine ) * nLineHeight + m_nViewAlignTop;
//	}
	if( bDispBkBitmap ){
	}else{
		/* �e�L�X�g�̂Ȃ�������w�i�F�œh��Ԃ� */
		if( nY/*nTop*/ < pPs->rcPaint.bottom ){
			rcBack.left = pPs->rcPaint.left;
			rcBack.right = pPs->rcPaint.right;
			rcBack.top = nY/*nTop*/;
			rcBack.bottom = pPs->rcPaint.bottom;
//#ifdef _DEBUG
//			hBrush = ::CreateSolidBrush( RGB( 128, 128,  128 ) );
//#else
			hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
//#endif
			::FillRect( hdc, &rcBack, hBrush );
			::DeleteObject( hBrush );
		}
	}

	::SetTextColor( hdc, crTextOld );
	::SetBkColor( hdc, crBackOld );
	::SelectObject( hdc, hFontOld );



	/* �܂�Ԃ��ʒu�̕\�� */
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_bDisp ){
		nX = m_nViewAlignLeft + ( TypeDataPtr->m_nMaxLineSize - m_nViewLeftCol ) * nCharWidth;
		/* �܂�Ԃ��L���̐F�̃y�����쐬 */
		hPen = ::CreatePen( PS_SOLID, 0, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, nX, m_nViewAlignTop, NULL );
		::LineTo( hdc, nX, m_nViewAlignTop + m_nViewCy );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
	}


	/* ���[���[�`�� */
	DispRuler( hdc );

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
	/* �L�����b�g�����݈ʒu�ɕ\�����܂� */
	::ShowCaret( m_hWnd );
	return;
}









//@@@ 2001.02.17 Start by MIK
/*! �s�̃e�L�X�g�^�I����Ԃ̕`��
	@par nCOMMENTMODE
	�֐������ŏ�ԑJ�ڂ̂��߂Ɏg����ϐ�nCOMMENTMODE�Ə�Ԃ̊֌W�B

 - 0: �ʏ�
 - 1: �s�R�����g
 - 2: �u���b�N�R�����g
 - 3: �V���O���R�[�e�[�V����
 - 4: �_�u���R�[�e�[�V����
 - 5: �����L�[���[�h�P
 - 6: �R���g���[���R�[�h
 - 9: ���p���l
 - 20: �u���b�N�R�����g�Q
 - 50: �����L�[���[�h�Q
 - 80: URL
 - 90: ����

 */
//@@@ 2001.02.17 End by MIK
int CEditView::DispLineNew(
		HDC						hdc,
		const CLayout*			pcLayout,
		int&					nLineNum,
		int						x,
		int&					y,
		BOOL					bDispBkBitmap,
		int						nLineTo,
		BOOL					bSelected
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CEditView::DispLineNew" );
#endif
	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	int						nLineNumOrg = nLineNum;
	const unsigned char*	pLine;
	int						nLineLen;
	int						nX;
	int						nLineBgn;
	int						nBgn;
	int						nPos;
	int						nCharChars;
	int						nCharChars_2;
	int						nCOMMENTMODE;
	int						nCOMMENTMODE_OLD;
	int						nCOMMENTEND;
	int						nCOMMENTEND_OLD;
	RECT					rcClip;
	RECT					rcClip2;
	int						nLineHeight = m_nCharHeight + TypeDataPtr->m_nLineSpace;
	int						nCharWidth = m_nCharWidth + TypeDataPtr->m_nColmSpace;
	UINT					fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	HPEN					hPen;
	HPEN					hPenOld;
	HBRUSH					hBrush;
	COLORREF				colTextColorOld;
	COLORREF				colBkColorOld;
	static char*			pszEOF = "[EOF]";
//#ifndef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
// //	static char*			pszTAB = ">       ";
// //	static char*			pszTAB = ">�������";
//	static char*			pszTAB = "^       ";
//#endif
	static char*			pszSPACES = "        ";
	static char*			pszZENSPACE	= "��";
	static char*			pszWRAP	= "<";
	int						nPosX;
	int						nPosY;
	int						bEOF = FALSE;
	int						nCount;
	int						nLineCols;
	const CLayout*			pcLayout2;
	int						i, j;
	int						nIdx;
	int						nUrlLen;
	BOOL					bSearchStringMode;
	int						nSearchEnd;
	int						nColorIdx;
	bool					bKeyWordTop = true;	//	Keyword Top
//	const CDocLine*			pCDocLine;
	bSearchStringMode = FALSE;

	/* �e�L�X�g�`�惂�[�h */
	fuOptions = ETO_CLIPPED | ETO_OPAQUE;

	/* �_���s�f�[�^�̎擾 */
	if( NULL != pcLayout ){
		nLineLen = pcLayout->m_pCDocLine->m_pLine->m_nDataLen - pcLayout->m_nOffset;
		pLine = (const unsigned char *)pcLayout->m_pCDocLine->m_pLine->m_pData + pcLayout->m_nOffset;

//		pCDocLine = pcLayout->m_pCDocLine;
//		if( NULL == pCDocLine ){
//			nLineLen = 0;
//			pLine = NULL;
//		}else{
//			nLineLen = pCDocLine->m_pLine->m_nDataLen;
//			pLine = (const unsigned char *)pCDocLine->m_pLine->m_pData;
//		}

		nCOMMENTMODE = pcLayout->m_nTypePrev;	/* �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
		nCOMMENTEND = 0;
//		pcLayout2 = m_pcEditDoc->m_cLayoutMgr.GetLineData( nLineNum );
//		pcLayout2 = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum );
//		if( pcLayout2 != pcLayout ){
//			MYTRACE( "������������\n" );
//		}
		pcLayout2 = pcLayout;

	}else{
		pLine = NULL;
		nLineLen = 0;
		nCOMMENTMODE = 0;	/* �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
		nCOMMENTEND = 0;

		pcLayout2 = NULL;
	}
	/* ���݂̐F���w�� */
//	SetCurrentColor( hdc, 0 );
	SetCurrentColor( hdc, nCOMMENTMODE );

	nBgn = 0;
	nPos = 0;
	nLineBgn = 0;
	nX = 0;
	nCharChars = 0;
	setlocale ( LC_ALL, "C" );	//	Oct. 29, 2001 genta ����������̃n�C���C�g�Ɋ֌W����
	if( NULL != pLine ){
		y -= nLineHeight;
		nLineNum--;
//		MYTRACE( "\n\n=======================================" );
		while( nPos < nLineLen ){
//			MYTRACE( "nLineNum = %d\n", nLineNum );

			y += nLineHeight;
			nLineNum++;
			if( m_nViewTopLine + m_nViewRowNum < nLineNum ){
				nLineNum = nLineTo + 1;
				goto end_of_func;
			}
			if( nLineTo < nLineNum ){
				goto end_of_func;
			}
//			pcLayout2 = m_pcEditDoc->m_cLayoutMgr.GetLineData( nLineNum );

//MYTRACE( "pcLayout2 = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum == %d);\n", nLineNum );
			pcLayout2 = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum );
			if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
				/* �s�ԍ��\�� */
				DispLineNumber( hdc, pcLayout2, nLineNum, y );
			}
			nBgn = nPos;
			nLineBgn = nBgn;
			nX = 0;
			while( nPos - nLineBgn < pcLayout2->m_nLength ){
				/* ����������̐F���� */
				if( TRUE == m_bCurSrchKeyMark	/* ����������̃}�[�N */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
					if( !bSearchStringMode
					 && IsSeaechString( (const char*)pLine, nLineLen, nPos, &nSearchEnd )
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						bSearchStringMode = TRUE;
//						nCOMMENTMODE = 90;
//						nCOMMENTMODE_OLD_2 = nCOMMENTMODE;
//						nCOMMENTEND_OLD_2 = nCOMMENTEND;
						/* ���݂̐F���w�� */
//						SetCurrentColor( hdc, nCOMMENTMODE );
						SetCurrentColor( hdc, 90 );
					}else
					if( bSearchStringMode
					 && nSearchEnd == nPos
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
//							SetCurrentColor( hdc, 90 );
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						/* ���݂̐F���w�� */
//						nCOMMENTMODE = nCOMMENTMODE_OLD_2;
//						nCOMMENTEND = nCOMMENTEND_OLD_2;
						SetCurrentColor( hdc, nCOMMENTMODE );
						bSearchStringMode = FALSE;
						goto searchnext;
					}
				}

				if( nPos >= nLineLen - pcLayout2->m_cEol.GetLen() ){
					if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
						/* �e�L�X�g�\�� */
						nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						nBgn = nPos + 1;
						/* �s���w�i�`�� */
						rcClip.left = x + nX * ( nCharWidth );
						rcClip.right = m_nViewAlignLeft + m_nViewCx;
						rcClip.top = y;
						rcClip.bottom = y + nLineHeight;
						if( rcClip.left < m_nViewAlignLeft ){
							rcClip.left = m_nViewAlignLeft;
						}
						if( rcClip.left < rcClip.right &&
							rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
							hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
							::FillRect( hdc, &rcClip, hBrush );
							::DeleteObject( hBrush );
						}

						/* ���s�L���̕\�� */
						if( bSearchStringMode ){
							nColorIdx = COLORIDX_SEARCH;
						}else{
							nColorIdx = COLORIDX_CRLF;
						}
						HFONT	hFontOld;
						/* �t�H���g��I�� */
						hFontOld = (HFONT)::SelectObject( hdc,
							ChooseFontHandle(
								TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont,
								TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bUnderLine
							)
						);
						colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT );	/* TAB�����̐F */
						colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );		/* TAB�����w�i�̐F */

						rcClip2.left = x + nX * ( nCharWidth );
						rcClip2.right = rcClip2.left + ( nCharWidth ) * ( 1 );
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
							rcClip2.top = y;
							rcClip2.bottom = y + nLineHeight;
							colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );	/* CRLF�w�i�̐F */
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, (const char *)" ", 1, m_pnDx );
							::SetBkColor( hdc, colBkColorOld );
							/* ���s�L���̕\�� */
							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_CRLF].m_bDisp ){
								nPosX = x + nX * ( nCharWidth );
								nPosY = y;
								hPen = ::CreatePen( PS_SOLID, 0, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT );
								hPenOld = (HPEN)::SelectObject( hdc, hPen );
								//	May 23, 2000 genta
								DrawEOL(hdc, hPen, nPosX + 1, nPosY, m_nCharWidth, m_nCharHeight,
									pcLayout2->m_cEol, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont );
								::SelectObject( hdc, hPenOld );
								::DeleteObject( hPen );
							}
						}
						::SelectObject( hdc, hFontOld );
						::SetTextColor( hdc, colTextColorOld );
						::SetBkColor( hdc, colBkColorOld );

						nX++;


						if( bSelected ){
							/* �e�L�X�g���] */
							DispTextSelected( hdc, nLineNum, x, y, nX );
						}
					}

					goto end_of_line;
				}
				SEARCH_START:;
				switch( nCOMMENTMODE ){
				case 0:
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp && (
						//	�s�R�����g1
						(
							NULL != TypeDataPtr->m_szLineComment &&									/* �s�R�����g�f���~�^ */
							( TypeDataPtr->m_nLineCommentPos < 0 || nPos == TypeDataPtr->m_nLineCommentPos ) &&
							0 < lstrlen( TypeDataPtr->m_szLineComment ) &&
							nPos <= nLineLen - (int)lstrlen( TypeDataPtr->m_szLineComment ) &&		/* �s�R�����g�f���~�^ */
							0 == memicmp( &pLine[nPos], TypeDataPtr->m_szLineComment, (int)lstrlen( TypeDataPtr->m_szLineComment ) )
						) ||
						(
						//	�s�R�����g2
							NULL != TypeDataPtr->m_szLineComment2 &&								/* �s�R�����g�f���~�^2 */
							( TypeDataPtr->m_nLineCommentPos2 < 0 || nPos == TypeDataPtr->m_nLineCommentPos2 ) &&							0 < lstrlen( TypeDataPtr->m_szLineComment2 ) &&
							//	Mar. 15, 2000 genta for Fortran
							(
							//	Jun. 6, 2001 genta
							//	�s�R�����g��3�J�����ʒu�w��t���łł���悤�ɂȂ����̂�
							//	Fortran��p�@�\�͖������I����
							//	TypeDataPtr->m_szLineComment2[0] == 'C' ?
							//	nPos == 0 ? ( pLine[0] == 'C' || pLine[0] == 'c' || pLine[0] == '*' ) : false
							//	:
								nPos <= nLineLen - (int)lstrlen( TypeDataPtr->m_szLineComment2 ) &&	/* �s�R�����g�f���~�^2 */
								0 == memicmp( &pLine[nPos], TypeDataPtr->m_szLineComment2, (int)lstrlen( TypeDataPtr->m_szLineComment2 ) )
							)
						//	From Here Jun. 6, 2001 genta 3�ڂ̍s�R�����g�F�����ɑΉ�
						) ||
						(
							NULL != TypeDataPtr->m_szLineComment &&									/* �s�R�����g�f���~�^ */
							( TypeDataPtr->m_nLineCommentPos3 < 0 || nPos == TypeDataPtr->m_nLineCommentPos3 ) &&
							0 < lstrlen( TypeDataPtr->m_szLineComment3 ) &&
							nPos <= nLineLen - (int)lstrlen( TypeDataPtr->m_szLineComment3 ) &&		/* �s�R�����g�f���~�^ */
							0 == memicmp( &pLine[nPos], TypeDataPtr->m_szLineComment3, (int)lstrlen( TypeDataPtr->m_szLineComment3 ) )
						//	To Here Jun. 6, 2001 genta
						)
					) ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;

						nCOMMENTMODE = 1;	/* �s�R�����g�ł��� */

//						if( TypeDataPtr->m_bDispCOMMENT ){	/* �R�����g��\������ */
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
					}else
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp && (
						NULL != TypeDataPtr->m_szBlockCommentFrom &&	/* �u���b�N�R�����g�f���~�^(From) */
//						0 < lstrlen( TypeDataPtr->m_szBlockCommentFrom ) &&
						'\0' != TypeDataPtr->m_szBlockCommentFrom[0] &&
						NULL != TypeDataPtr->m_szBlockCommentTo &&		/* �u���b�N�R�����g�f���~�^(To) */
//						0 < lstrlen( TypeDataPtr->m_szBlockCommentTo ) &&
						'\0' != TypeDataPtr->m_szBlockCommentTo[0]  &&
						nPos <= nLineLen - (int)lstrlen( TypeDataPtr->m_szBlockCommentFrom ) &&	/* �u���b�N�R�����g�f���~�^(From) */
						0 == memicmp( &pLine[nPos], TypeDataPtr->m_szBlockCommentFrom, (int)lstrlen( TypeDataPtr->m_szBlockCommentFrom ) )
					) ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = 2;	/* �u���b�N�R�����g�ł��� */

//						if( TypeDataPtr->m_bDispCOMMENT ){	/* �R�����g��\������ */
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + (int)lstrlen( TypeDataPtr->m_szBlockCommentFrom ); i <= nLineLen - (int)lstrlen( TypeDataPtr->m_szBlockCommentTo ); ++i ){
							nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( 0 == memicmp( &pLine[i], TypeDataPtr->m_szBlockCommentTo, (int)lstrlen( TypeDataPtr->m_szBlockCommentTo ) ) ){
								nCOMMENTEND = i + (int)lstrlen( TypeDataPtr->m_szBlockCommentTo );
								break;
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
						i = i;
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
					}else
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp && (
						NULL != TypeDataPtr->m_szBlockCommentFrom2 &&	/* �u���b�N�R�����g�f���~�^2(From) */
						'\0' != TypeDataPtr->m_szBlockCommentFrom2[0] &&
						NULL != TypeDataPtr->m_szBlockCommentTo2 &&		/* �u���b�N�R�����g�f���~�^2(To) */
						'\0' != TypeDataPtr->m_szBlockCommentTo2[0]  &&
						nPos <= nLineLen - (int)lstrlen( TypeDataPtr->m_szBlockCommentFrom2 ) &&	/* �u���b�N�R�����g�f���~�^2(From) */
						0 == memicmp( &pLine[nPos], TypeDataPtr->m_szBlockCommentFrom2, (int)lstrlen( TypeDataPtr->m_szBlockCommentFrom2 ) )
					) ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = 20;	/* �u���b�N�R�����g�ł��� */
//						if( TypeDataPtr->m_bDispCOMMENT ){	/* �R�����g��\������ */
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + (int)lstrlen( TypeDataPtr->m_szBlockCommentFrom2 ); i <= nLineLen - (int)lstrlen( TypeDataPtr->m_szBlockCommentTo2 ); ++i ){
							nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( 0 == memicmp( &pLine[i], TypeDataPtr->m_szBlockCommentTo2, (int)lstrlen( TypeDataPtr->m_szBlockCommentTo2 ) ) ){
								nCOMMENTEND = i + (int)lstrlen( TypeDataPtr->m_szBlockCommentTo2 );
								break;
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
						i = i;
//#endif
					}else
					if( pLine[nPos] == '\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* �V���O���N�H�[�e�[�V�����������\������ */
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = 3;	/* �V���O���N�H�[�e�[�V����������ł��� */

//						if( TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp ){	/* �V���O���N�H�[�e�[�V�����������\������ */
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
						/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
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
					if( pLine[nPos] == '"' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = 4;	/* �_�u���N�H�[�e�[�V����������ł��� */
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
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
					 && ( TRUE == IsURL( (const char *)&pLine[nPos], nLineLen - nPos, &nUrlLen ) )	/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = 80;	/* URL���[�h */
						nCOMMENTEND = nPos + nUrlLen;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//@@@ 2001.02.17 Start by MIK: ���p���l�������\��
//#ifdef COMPILE_COLOR_DIGIT
					}else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (i = IsNumber( (const char*)pLine, nPos, nLineLen )) > 0 )		/* ���p������\������ */
					{
						/* �L�[���[�h������̏I�[���Z�b�g���� */
						i = nPos + i;
						if( y/* + nLineHeight*/ >= m_nViewAlignTop )
						{
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						/* ���݂̐F���w�� */
						nBgn = nPos;
						nCOMMENTMODE = 9;	/* ���p���l�ł��� */
						nCOMMENTEND = i;
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//#endif
//@@@ 2001.02.17 End by MIK: ���p���l�������\��
					}else
					if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx != -1 && /* �L�[���[�h�Z�b�g */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD].m_bDisp &&  /* �����L�[���[�h��\������ */
//						( pLine[nPos] == '#' || pLine[nPos] == '$' || __iscsym( pLine[nPos] ) )
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						//	Mar 4, 2001 genta comment out
						//	bKeyWordTop = false;
						/* �L�[���[�h������̏I�[��T�� */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
//							if( pLine[i] == '#' || pLine[i] == '$' || __iscsym( pLine[i] ) ){
							if( IS_KEYWORD_CHAR( pLine[i] ) ){
							}else{
								break;
							}
						}
						/* �L�[���[�h���o�^�P��Ȃ�΁A�F��ς��� */
						j = i - nPos;
						/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */
						nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
							TypeDataPtr->m_nKeyWordSetIdx,
							(const char *)&pLine[nPos],
							j
						);
						if( nIdx != -1 ){
							if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
								/* �e�L�X�g�\�� */
								nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
							}

							/* ���݂̐F���w�� */
							nBgn = nPos;
							nCOMMENTMODE = 5;	/* �L�[���[�h���[�h */
							nCOMMENTEND = i;
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
						}else{		//MIK START ADD 2000.12.01 second keyword & binary search
							if(TypeDataPtr->m_nKeyWordSetIdx2 != -1 && /* �L�[���[�h�Z�b�g */							//MIK 2000.12.01 second keyword
								TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD2].m_bDisp)									//MIK
							{																							//MIK
								/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */						//MIK
								nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
									TypeDataPtr->m_nKeyWordSetIdx2 ,													//MIK
									(const char *)&pLine[nPos],															//MIK
									j																					//MIK
								);																						//MIK
								if( nIdx != -1 ){																		//MIK
									if( y/* + nLineHeight*/ >= m_nViewAlignTop ){										//MIK
										/* �e�L�X�g�\�� */																//MIK
										nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );	//MIK
									}																					//MIK
									/* ���݂̐F���w�� */																//MIK
									nBgn = nPos;																		//MIK
									nCOMMENTMODE = 50;	/* �L�[���[�h2���[�h */											//MIK
									nCOMMENTEND = i;																	//MIK
									if( !bSearchStringMode ){															//MIK
										SetCurrentColor( hdc, nCOMMENTMODE );											//MIK
									}																					//MIK
								}																						//MIK
							}																							//MIK
						}			//MIK END
					}
					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pLine[nPos] ))	bKeyWordTop = false;
					else								bKeyWordTop = true;
					//	To Here
					break;
				case 80:	/* URL���[�h�ł��� */
				case 5:		/* �L�[���[�h���[�h�ł��� */
				case 9:		/* ���p���l�ł��� */  //@@@ 2001.02.17 by MIK
				case 50:	/* �L�[���[�h2���[�h�ł��� */	//MIK
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = 0;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case 6:	/* �R���g���[���R�[�h */
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = nCOMMENTMODE_OLD;
						nCOMMENTEND = nCOMMENTEND_OLD;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;

				case 1:	/* �s�R�����g�ł��� */
					break;
				case 2:	/* �u���b�N�R�����g�ł��� */
					if( 0 == nCOMMENTEND ){
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + (int)lstrlen( TypeDataPtr->m_szBlockCommentFrom )*/; i <= nLineLen - (int)lstrlen( TypeDataPtr->m_szBlockCommentTo ); ++i ){
							nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( 0 == memicmp( &pLine[i], TypeDataPtr->m_szBlockCommentTo, (int)lstrlen( TypeDataPtr->m_szBlockCommentTo ) ) ){
								nCOMMENTEND = i + (int)lstrlen( TypeDataPtr->m_szBlockCommentTo );
								break;
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = 0;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
//#ifdef	COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
				case 20:	/* �u���b�N�R�����g�ł��� */
					if( 0 == nCOMMENTEND ){
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + (int)lstrlen( TypeDataPtr->m_szBlockCommentFrom2 )*/; i <= nLineLen - (int)lstrlen( TypeDataPtr->m_szBlockCommentTo2 ); ++i ){
							nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( 0 == memicmp( &pLine[i], TypeDataPtr->m_szBlockCommentTo2, (int)lstrlen( TypeDataPtr->m_szBlockCommentTo2 ) ) ){
								nCOMMENTEND = i + (int)lstrlen( TypeDataPtr->m_szBlockCommentTo2 );
								break;
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = 0;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
//#endif
				case 3:	/* �V���O���N�H�[�e�[�V����������ł��� */
					if( 0 == nCOMMENTEND ){
						/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
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
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = 0;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case 4:	/* �_�u���N�H�[�e�[�V����������ł��� */
					if( 0 == nCOMMENTEND ){
						/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
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
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = 0;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				}
				if( pLine[nPos] == TAB ){
					if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
						/* �e�L�X�g�\�� */
						nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						/* �^�u�L����\������ */
						rcClip2.left = x + nX * ( nCharWidth );
						rcClip2.right = rcClip2.left + ( nCharWidth ) * ( TypeDataPtr->m_nTabSpace - ( nX % TypeDataPtr->m_nTabSpace ) );
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
							rcClip2.top = y;
							rcClip2.bottom = y + nLineHeight;
							/* TAB��\�����邩�H */
							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_bDisp ){
								if( bSearchStringMode ){
									nColorIdx = COLORIDX_SEARCH;
								}else{
									nColorIdx = COLORIDX_TAB;
								}
								colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT );	/* TAB�����̐F */
								colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );		/* TAB�����w�i�̐F */

								HFONT	hFontOld;
								/* �t�H���g��I�� */
								hFontOld = (HFONT)::SelectObject( hdc,
									ChooseFontHandle(
										TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont,
										TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bUnderLine
									)
								);
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, /*pszTAB*/ TypeDataPtr->m_szTabViewString,
									( TypeDataPtr->m_nTabSpace - ( nX % TypeDataPtr->m_nTabSpace ) ),
									 m_pnDx );
//#else
//								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
//									&rcClip2, pszTAB,
//									( TypeDataPtr->m_nTabSpace - ( nX % TypeDataPtr->m_nTabSpace ) ),
//									 m_pnDx );
//#endif

								::SelectObject( hdc, hFontOld );
								::SetTextColor( hdc, colTextColorOld );
								::SetBkColor( hdc, colBkColorOld );
							}else{
								if( bSearchStringMode ){
									nColorIdx = COLORIDX_SEARCH;
									colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );	/* TAB�����w�i�̐F */
								}
								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, pszSPACES,
									( TypeDataPtr->m_nTabSpace - ( nX % TypeDataPtr->m_nTabSpace ) ),
									 m_pnDx );
								if( bSearchStringMode ){
									::SetBkColor( hdc, colBkColorOld );
								}
							}
						}
						nX += ( TypeDataPtr->m_nTabSpace - ( nX % TypeDataPtr->m_nTabSpace ) ) ;
					}
					nBgn = nPos + 1;
					nCharChars = 1;
				}else
				if( pLine[nPos] == 0x81 && pLine[nPos + 1] == 0x40 ){
					if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
						/* �e�L�X�g�\�� */
						nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );

						/* �S�p�󔒂�\������ */
						rcClip2.left = x + nX * ( nCharWidth );
						rcClip2.right = rcClip2.left + ( nCharWidth ) * 2;
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
							rcClip2.top = y;
							rcClip2.bottom = y + nLineHeight;

							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_ZENSPACE].m_bDisp ){	/* ���{��󔒂�\�����邩 */
								if( bSearchStringMode ){
									nColorIdx = COLORIDX_SEARCH;
								}else{
									nColorIdx = COLORIDX_ZENSPACE;
								}
								colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT );	/* �S�p�X�y�[�X�����̐F */
								colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );		/* �S�p�X�y�[�X�����w�i�̐F */


								HFONT	hFontOld;
								/* �t�H���g��I�� */
								hFontOld = (HFONT)::SelectObject( hdc,
									ChooseFontHandle(
										TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont,
										TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bUnderLine
									)
								);
//								if( TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont ){	/* ������ */
//									hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//								}else{
//									hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//								}

								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, pszZENSPACE, lstrlen( pszZENSPACE ), m_pnDx );

								::SelectObject( hdc, hFontOld );
								::SetTextColor( hdc, colTextColorOld );
								::SetBkColor( hdc, colBkColorOld );

							}else{
							if( bSearchStringMode ){
								nColorIdx = COLORIDX_SEARCH;
								colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );	/* �����w�i�̐F */
							}
								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, pszSPACES, 2, m_pnDx );
							}
							if( bSearchStringMode ){
								::SetBkColor( hdc, colBkColorOld );
							}
						}
						nX += 2;
					}
					nBgn = nPos + 2;
					nCharChars = 2;
				}else{
					nCharChars = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[nPos] ) - (const char *)&pLine[nPos];
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && 6 != nCOMMENTMODE
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* �R���g���[���R�[�h��F���� */
					 &&	(
							( (unsigned char)0x0 <= pLine[nPos] && pLine[nPos] <= (unsigned char)0x1F ) ||
							( (unsigned char)'~' < pLine[nPos] && pLine[nPos] < (unsigned char)'�' ) ||
							( (unsigned char)'�' < pLine[nPos] && pLine[nPos] <= (unsigned char)0xff )
						)
					 && pLine[nPos] != TAB && pLine[nPos] != CR && pLine[nPos] != LF
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE_OLD = nCOMMENTMODE;
						nCOMMENTEND_OLD = nCOMMENTEND;
						nCOMMENTMODE = 6;	/* �R���g���[���R�[�h ���[�h */
						/* �R���g���[���R�[�h��̏I�[��T�� */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							nCharChars_2 = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if( (
								( (unsigned char)0x0 <= pLine[i] && pLine[i] <= (unsigned char)0x1F ) ||
									( (unsigned char)'~' < pLine[i] && pLine[i] < (unsigned char)'�' ) ||
									( (unsigned char)'�' < pLine[i] && pLine[i] <= (unsigned char)0xff )
								) &&
								pLine[i] != TAB && pLine[i] != CR && pLine[i] != LF
							){
							}else{
								break;
							}
						}
						nCOMMENTEND = i;
						/* ���݂̐F���w�� */
//						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
//						}
					}
				}
				nPos+= nCharChars;
			} //end of while( nPos - nLineBgn < pcLayout2->m_nLength ){
			if( nPos >= nLineLen ){
				break;
			}
			if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
				/* �e�L�X�g�\�� */
				nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
				nBgn = nPos;

				/* �s���w�i�`�� */
				rcClip.left = x + nX * ( nCharWidth );
				rcClip.right = m_nViewAlignLeft + m_nViewCx;
				rcClip.top = y;
				rcClip.bottom = y + nLineHeight;
				if( rcClip.left < m_nViewAlignLeft ){
					rcClip.left = m_nViewAlignLeft;
				}
				if( rcClip.left < rcClip.right &&
					rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
					hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
					::FillRect( hdc, &rcClip, hBrush );
					::DeleteObject( hBrush );
				}

				/* �܂�Ԃ��L�� */
				/* �܂�Ԃ��L����\������ */
				rcClip2.left = x + nX * ( nCharWidth );
				rcClip2.right = rcClip2.left + ( nCharWidth ) * 1;
				if( rcClip2.left < m_nViewAlignLeft ){
					rcClip2.left = m_nViewAlignLeft;
				}
				if( rcClip2.left < rcClip2.right &&
					rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
					rcClip2.top = y;
					rcClip2.bottom = y + nLineHeight;

					/* �܂�Ԃ��L����\������ */
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_bDisp ){
						HFONT	hFontOld;
						colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_colTEXT );	/* �܂�Ԃ��L���̐F */
						colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_colBACK );		/* �܂�Ԃ��L���w�i�̐F */
						/* �t�H���g��I�� */
						hFontOld = (HFONT)::SelectObject( hdc,
							ChooseFontHandle(
								TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_bFatFont,
								TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_bUnderLine
							)
						);
						::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
							&rcClip2, pszWRAP, lstrlen( pszWRAP ), m_pnDx );

						::SelectObject( hdc, hFontOld );
						::SetTextColor( hdc, colTextColorOld );
						::SetBkColor( hdc, colBkColorOld );
					}else{
						::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
							&rcClip2, " ", 1, m_pnDx );
					}
				}
				if( bSelected ){
					/* �e�L�X�g���] */
					DispTextSelected( hdc, nLineNum, x, y, nX );
				}
			}
//			pcLayout2 = pcLayout2->m_pNext;

		}
		if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
			/* �e�L�X�g�\�� */
			nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
			if( bSelected ){
				/* �e�L�X�g���] */
				DispTextSelected( hdc, nLineNum, x, y, nX );
			}
		}

		/* EOF�L���̕\�� */
//		if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
		if( nLineNum + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
			nX < TypeDataPtr->m_nMaxLineSize
		){
			if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){

				colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_colTEXT );	/* EOF�̐F */
				colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_colBACK );		/* EOF�w�i�̐F */
				rcClip.left = x + nX * ( nCharWidth );
				rcClip.right = rcClip.left + ( nCharWidth ) * ( lstrlen( pszEOF ) );
				if( rcClip.left < m_nViewAlignLeft ){
					rcClip.left = m_nViewAlignLeft;
				}
				if( rcClip.left < rcClip.right &&
					rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
					rcClip.top = y;
					rcClip.bottom = y + nLineHeight;

					HFONT	hFontOld;
					/* �t�H���g��I�� */
					hFontOld = (HFONT)::SelectObject( hdc,
						ChooseFontHandle(
							TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bFatFont,
							TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bUnderLine
						)
					);
//					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bFatFont ){	/* ������ */
//						hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//					}else{
//						hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//					}

					::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
						&rcClip, pszEOF, lstrlen( pszEOF ), m_pnDx );

					::SelectObject( hdc, hFontOld );

				}
				nX += lstrlen( pszEOF );
				::SetTextColor( hdc, colTextColorOld );
				::SetBkColor( hdc, colBkColorOld );
			}
			bEOF = TRUE;
		}
		if( bDispBkBitmap ){
		}else{
			/* �s���w�i�`�� */
			rcClip.left = x + nX * ( nCharWidth );
			rcClip.right = m_nViewAlignLeft + m_nViewCx;
			rcClip.top = y;
			rcClip.bottom = y + nLineHeight;
			if( rcClip.left < m_nViewAlignLeft ){
				rcClip.left = m_nViewAlignLeft;
			}
			if( rcClip.left < rcClip.right &&
				rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
				hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
				::FillRect( hdc, &rcClip, hBrush );
				::DeleteObject( hBrush );
			}
		}
end_of_line:;
		nLineNum++;
		y += nLineHeight;
	}else{
		if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
			if( bDispBkBitmap ){
			}else{
				/* �w�i�`�� */
				rcClip.left = 0;
				rcClip.right = m_nViewAlignLeft + m_nViewCx;
				rcClip.top = y;
				rcClip.bottom = y + nLineHeight;
				hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
				::FillRect( hdc, &rcClip, hBrush );
				::DeleteObject( hBrush );
			}
			/* EOF�L���̕\�� */
			nCount = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			if( nCount == 0 && m_nViewTopLine == 0 && nLineNum == 0 ){
				/* EOF�L���̕\�� */
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_colTEXT );	/* EOF�̐F */
					colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_colBACK );		/* EOF�w�i�̐F */
					rcClip.left = /*m_nViewAlignLeft*/x;
					rcClip.right = rcClip.left + ( nCharWidth ) * ( lstrlen( pszEOF ) );
					if( rcClip.left < m_nViewAlignLeft ){
						rcClip.left = m_nViewAlignLeft;
					}
					if( rcClip.left < rcClip.right &&
						rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
						rcClip.top = y;
						rcClip.bottom = y + nLineHeight;

						HFONT	hFontOld;
						/* �t�H���g��I�� */
						hFontOld = (HFONT)::SelectObject( hdc,
							ChooseFontHandle(
								TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bFatFont,
								TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bUnderLine
							)
						);
//						if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bFatFont ){	/* ������ */
//							hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//						}else{
//							hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//						}

						::ExtTextOut( hdc, x , y, fuOptions,
							&rcClip, pszEOF, lstrlen( pszEOF ), m_pnDx );

						::SelectObject( hdc, hFontOld );
					}
					nX += lstrlen( pszEOF );

					::SetTextColor( hdc, colTextColorOld );
					::SetBkColor( hdc, colBkColorOld );

				}
				y += nLineHeight;
				bEOF = TRUE;
			}else{
				if( nCount > 0 && nLineNum == nCount ){
					const char*	pLine;
					int			nLineLen;
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCount - 1, &nLineLen );
					nLineCols = LineIndexToColmn( pLine, nLineLen, nLineLen );
					if( ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ) ||
						nLineCols >= TypeDataPtr->m_nMaxLineSize
					 ){
						/* EOF�L���̕\�� */
						if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
							colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_colTEXT );	/* EOF�̐F */
							colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_colBACK );		/* EOF�w�i�̐F */
							rcClip.left = /*m_nViewAlignLeft*/x;
							rcClip.right = rcClip.left + ( nCharWidth ) * ( lstrlen( pszEOF ) );
							if( rcClip.left < m_nViewAlignLeft ){
								rcClip.left = m_nViewAlignLeft;
							}
							if( rcClip.left < rcClip.right &&
								rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft ){
								rcClip.top = y;
								rcClip.bottom = y + nLineHeight;

								HFONT	hFontOld;
								/* �t�H���g��I�� */
								hFontOld = (HFONT)::SelectObject( hdc,
									ChooseFontHandle(
										TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bFatFont,
										TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bUnderLine
									)
								);
//								if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bFatFont ){	/* ������ */
//									hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//								}else{
//									hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//								}

								::ExtTextOut( hdc, x , y, fuOptions,
									&rcClip, pszEOF, lstrlen( pszEOF ), m_pnDx );

								::SelectObject( hdc, hFontOld );
							}
							nX += lstrlen( pszEOF );

							::SetTextColor( hdc, colTextColorOld );
							::SetBkColor( hdc, colBkColorOld );
						}
						y += nLineHeight;
						bEOF = TRUE;
					}
				}
			}
		}
	}

end_of_func:;
//	MYTRACE( "m_nOldUnderLineY=%d\n", m_nOldUnderLineY );
	if( -1 != m_nOldUnderLineY ){
		/* �J�[�\���s�A���_�[���C����ON */
		CaretUnderLineON( TRUE );
	}
//	if( nLineNumOrg == m_nCaretPosY
//	 && -1 != m_nOldUnderLineY
//	 && m_bDoing_UndoRedo == FALSE	/* �A���h�D�E���h�D�̎��s���� */
//	 && TypeDataPtr->m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp
//	 && !IsTextSelected()
//	){
//		HDC			hdc;
//		HPEN		hPen, hPenOld;
//		hdc = ::GetDC( m_hWnd );
//
//		MYTRACE( "���J�[�\���s�A���_�[���C���̕`��\n" );
//		/* �J�[�\���s�A���_�[���C���̕`�� */
//		hPen = ::CreatePen( PS_SOLID, 0, TypeDataPtr->m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
//		hPenOld = (HPEN)::SelectObject( hdc, hPen );
//		m_nOldUnderLineY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( TypeDataPtr->m_nLineSpace + m_nCharHeight ) + m_nCharHeight;
//		::MoveToEx(
//			hdc,
//			m_nViewAlignLeft,
//			m_nOldUnderLineY,
//			NULL
//		);
//		::LineTo(
//			hdc,
//			m_nViewCx + m_nViewAlignLeft,
//			m_nOldUnderLineY
//		);
//		::SelectObject( hdc, hPenOld );
//		::DeleteObject( hPen );
//		::ReleaseDC( m_hWnd, hdc );
//	}
	return bEOF;
}

//	May 23, 2000 genta
/*! ��ʕ`��⏕�֐�:
	�s���̉��s�}�[�N�����s�R�[�h�ɂ���ď���������i���C���j

	@param hdc Device Context Handle
	@param hPen �`��Ɏg���y��
	@param nPosX �`����WX
	@param nPosY �`����WY
	@param nWidth  �`��G���A�̃T�C�YX
	@param nHeight �`��G���A�̃T�C�YY
	@param cEol �s���R�[�h���
	@param bBold TRUE: ����

	@note bBold��TRUE�̎��͉���1�h�b�g���炵�ďd�ˏ������s�����A
	���܂葾�������Ȃ��B
*/
void CEditView::DrawEOL( HDC hdc, HANDLE hPen, int nPosX, int nPosY, int nWidth, int nHeight, CEOL cEol, int bBold )
{
	int sx, sy;
	switch( cEol.GetType() ){
	case EOL_CRLF:
		sx = nPosX + ( nWidth / 2 );
		sy = nPosY + ( nHeight * 3 / 4 );
		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
		::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
		::MoveToEx( hdc, sx, sy, NULL );
		::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy - ( nHeight * 5 / 20 ) );
		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL);
		::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy - ( nHeight * 5 / 20 ) );
		if( bBold ){
			++sx;
			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
			::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
			::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), nPosY + ( nHeight * 15/ 20) - ( nHeight * 5 / 20 ) );
			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL);
			::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), nPosY + ( nHeight * 15/ 20) - ( nHeight * 5 / 20 ) );
		}
		break;
	case EOL_LF:
		sx = nPosX;
		sy = nPosY + ( nHeight / 2 );
		::MoveToEx( hdc, nPosX, sy, NULL );
		::LineTo(   hdc, nPosX + nWidth, sy );
		::MoveToEx( hdc, sx, sy, NULL );
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );
		::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4);
		if( bBold ){
			++sy;
			::MoveToEx( hdc, nPosX, sy, NULL );
			::LineTo(   hdc, nPosX + nWidth, sy );
			::MoveToEx( hdc, sx, sy, NULL );
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );
			::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4);
		}
		break;
	case EOL_CR:
#if 1
		sx = nPosX + nWidth;
		sy = nPosY + ( nHeight / 2 );
		::MoveToEx( hdc, nPosX, sy, NULL );
		::LineTo(   hdc, nPosX + nWidth, sy );
		::MoveToEx( hdc, sx, sy, NULL );
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );
		::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
		::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4);
		if( bBold ){
			++sy;
			::MoveToEx( hdc, nPosX, sy, NULL );
			::LineTo(   hdc, nPosX + nWidth, sy );
			::MoveToEx( hdc, sx, sy, NULL );
			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );
			::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
			::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4);
		}
#else
		sx = nPosX;
		::MoveToEx( hdc, sx + nWidth - 3, nPosY + nHeight * 1 / 4, NULL );
		::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4);
		::LineTo(   hdc, sx, nPosY + nHeight * 3 / 4 );
		::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4 - nHeight / 4);
		if( bBold ){
			++sx;
			::MoveToEx( hdc, sx + nWidth - 3, nPosY + nHeight * 1 / 4, NULL );
			::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4);
			::LineTo(   hdc, sx, nPosY + nHeight * 3 / 4 );
			::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4 - nHeight / 4);
		}
#endif
		break;
	case EOL_LFCR:
		sx = nPosX + ( nWidth / 2 );
		sy = nPosY + ( nHeight * 1 / 4 );
		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
		::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
		::MoveToEx( hdc, sx, sy, NULL );
		::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
		::MoveToEx( hdc, sx, sy, NULL);
		::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
		if( bBold ){
			++sx;
			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
			::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
			::MoveToEx( hdc, sx, sy, NULL );
			::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
			::MoveToEx( hdc, sx, sy, NULL);
			::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
		}
		break;
	}
}


/*[EOF]*/
