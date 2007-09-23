/*!	@file
	@brief CEditView�N���X

	@author Norio Nakatani
	@date	1998/12/08 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK
	Copyright (C) 2001, hor, YAZAKI
	Copyright (C) 2002, MIK, Moca, genta, hor, novice, YAZAKI, aroka, KK
	Copyright (C) 2003, MIK, ryoji, �����
	Copyright (C) 2004, genta, Moca, MIK
	Copyright (C) 2005, genta, Moca, MIK, D.S.Koba
	Copyright (C) 2007, ryoji

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
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
#include "etc_uty.h"
#include "CRegexKeyword.h"	//@@@ 2001.11.17 add MIK
#include "my_icmp.h"	//@@@ 2002.01.13 add
#include "Clayout.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/2/10 aroka

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
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::OnPaint" );

	// 2004.01.28 Moca �f�X�N�g�b�v�ɍ�悵�Ȃ��悤��
	if( NULL == hdc ){
		return;
	}
//	if( m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
//		return;
//	}
	if( !m_bDrawSWITCH ){
		return;
	}
	/* �L�����b�g���B�� */
	HideCaret_( m_hWnd ); // 2002/07/22 novice

	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	const int nWrapWidth = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();

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
	int				nLineTo;
	int				nX = m_nViewAlignLeft - m_nViewLeftCol * nCharWidth;
	int				nY;
	BOOL			bDispBkBitmap = /*TRUE*/FALSE;
	const CLayout*	pcLayout;
	HPEN			hPen;
	HPEN			hPenOld;

//@@@ 2001.11.17 add start MIK
	//�ύX������΃^�C�v�ݒ���s���B
	if( TypeDataPtr->m_bUseRegexKeyword
	 || m_cRegexKeyword->m_bUseRegexKeyword )	//OFF�Ȃ̂ɑO��̃f�[�^���c���Ă�
	{
		//�^�C�v�ʐݒ������B�ݒ�ς݂��ǂ����͌Ăѐ�Ń`�F�b�N����B
		m_cRegexKeyword->RegexKeySetTypes(TypeDataPtr);
	}
//@@@ 2001.11.17 add end MIK

	/* �L�����b�g�̍s���ʒu��\������ */
//	DrawCaretPosInfo();

	/* �������c�b�𗘗p�����ĕ`��̏ꍇ�͕`���̂c�b��؂�ւ��� */
	bUseMemoryDC = FALSE;
	if( bUseMemoryDC ){
		hdcOld = hdc;
		hdc = m_hdcCompatDC;
	}

	/* 03/02/18 �Ί��ʂ̋����\��(����) ai */
	DrawBracketPair( false );

	/* ���[���[�ƃe�L�X�g�̊Ԃ̗]�� */
	//@@@ 2002.01.03 YAZAKI �]����0�̂Ƃ��͖��ʂł����B
	if ( m_nTopYohaku ){
		rc.left = 0;
		rc.top = m_nViewAlignTop - m_nTopYohaku;
		rc.right = m_nViewAlignLeft + m_nViewCx;
		rc.bottom = m_nViewAlignTop;
		hBrush = ::CreateSolidBrush( TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}
	
	/* �s�ԍ��̕\�� */
	//	From Here Sep. 7, 2001 genta
	//	Sep. 23, 2002 genta �s�ԍ���\���ł��s�ԍ��F�̑т�����̂Ō��Ԃ𖄂߂�
	if( m_nTopYohaku ){ 
		rc.left = 0;
		rc.top = m_nViewAlignTop - m_nTopYohaku;
		//	Sep. 23 ,2002 genta �]���̓e�L�X�g�F�̂܂܎c��
		rc.right = m_nViewAlignLeft - m_pShareData->m_Common.m_nLineNumRightSpace;
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

	int nMaxRollBackLineNum = 260 / nWrapWidth + 1;
	int nRollBackLineNum = 0;
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

	nY = ( i - m_nViewTopLine ) * nLineHeight + m_nViewAlignTop;
	nLineTo = m_nViewTopLine + ( ( pPs->rcPaint.bottom - m_nViewAlignTop ) / nLineHeight );

	BOOL bSelected;
	bSelected = IsTextSelected();
	for( ; i <= nLineTo; ){
		pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( i );
#ifdef _DEBUG
		{
			if( NULL != pcLayout ){
				if( (void*)0xdddddddd == (void*)pcLayout->m_pCDocLine->m_pLine ){
					m_pcEditDoc->m_cDocLineMgr.DUMP();
					m_pcEditDoc->m_cLayoutMgr.DUMP();

					pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( i );
				}

				int nLineLen = pcLayout->m_pCDocLine->m_pLine->GetLength() - pcLayout->m_nOffset;
				const char * pLine = (const char *)pcLayout->m_pCDocLine->m_pLine->GetPtr() + pcLayout->m_nOffset;
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

			// 2006.04.29 �s�����͍s���Ƃɍ�悵�A�����ł͏c���̎c������
			DispVerticalLines( hdc, nY, pPs->rcPaint.bottom, 0, -1 );
		}
	}

	::SetTextColor( hdc, crTextOld );
	::SetBkColor( hdc, crBackOld );
	::SelectObject( hdc, hFontOld );

	/* �܂�Ԃ��ʒu�̕\�� */
	if( TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_bDisp ){
		int nXPos = m_nViewAlignLeft + ( nWrapWidth - m_nViewLeftCol ) * nCharWidth;
		//	2005.11.08 Moca �������ύX
		if( m_nViewAlignLeft < nXPos && nXPos < m_nViewCx + m_nViewAlignLeft ){
			/* �܂�Ԃ��L���̐F�̃y�����쐬 */
			hPen = ::CreatePen( PS_SOLID, 0, TypeDataPtr->m_ColorInfoArr[COLORIDX_WRAP].m_colTEXT );
			hPenOld = (HPEN)::SelectObject( hdc, hPen );
			::MoveToEx( hdc, nXPos, m_nViewAlignTop, NULL );
			::LineTo( hdc, nXPos, m_nViewAlignTop + m_nViewCy );
			::SelectObject( hdc, hPenOld );
			::DeleteObject( hPen );
		}
	}


	/* ���[���[�`�� */
	if ( pPs->rcPaint.top < m_nViewAlignTop - m_nTopYohaku ) { // ���[���[���ĕ`��͈͂ɂ���Ƃ��̂ݍĕ`�悷�� 2002.02.25 Add By KK
		m_bRedrawRuler = true; //2002.02.25 Add By KK ���[���[�S�̂�`��B
		DispRuler( hdc );
	}

	if ( m_pcEditDoc->m_nActivePaneIndex == m_nMyIndex ){
		/* �A�N�e�B�u�y�C���́A�A���_�[���C���`�� */
		m_cUnderLine.CaretUnderLineON( TRUE );
	}

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
	ShowCaret_( m_hWnd ); // 2002/07/22 novice
	return;
}









//@@@ 2001.02.17 Start by MIK
/*! �s�̃e�L�X�g�^�I����Ԃ̕`��
	1���1�_���s������悷��B
	@param          hdc            ���Ώ�
	@param          pcLayout       �\�����J�n���郌�C�A�E�g
	@param[in,out]  nLineNum       ��悷�郌�C�A�E�g�s�ԍ�(0�J�n), ���̕����s�ɑΉ����郌�C�A�E�g�s�ԍ�
	@param          x              ���C�A�E�g0���ڂ̍����Wx
	@param[in,out]  y              �����Wy, ���̍����Wy
	@param          bDispBkBitmap  �w�i�Ƀr�b�g�}�b�v���g�p����(������͏��FALSE�BTRUE�̏ꍇ�w�i��悵�Ȃ�)
	@param          nLineTo        ���I�����郌�C�A�E�g�s�ԍ�
	@param          bSelected      �I�𒆂�
	@return EOF����悵����true

	@par nCOMMENTMODE
	�֐������ŏ�ԑJ�ڂ̂��߂Ɏg����ϐ�nCOMMENTMODE�Ə�Ԃ̊֌W�B

// 2002/03/13 novice
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

	@date 2001.12.21 YAZAKI ���s�L���̕`��������ύX
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
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DispLineNew" );

	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	const int nWrapWidth = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();

	int						nLineNumOrg = nLineNum;
	const char*				pLine;	//@@@ 2002.09.22 YAZAKI
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
	//	�e�L�X�g�`�惂�[�h
	UINT					fuOptions = ETO_CLIPPED | ETO_OPAQUE;
//@@@ 2001.12.21 YAZAKI
//	HPEN					hPen;
//	HPEN					hPenOld;
	HBRUSH					hBrush;
	COLORREF				colTextColorOld;
	COLORREF				colBkColorOld;
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
	BOOL					bSearchFlg;			// 2002.02.08 hor
	int						nSearchStart;		// 2002.02.08 hor
	int						nSearchEnd;
	int						nColorIdx;
	bool					bKeyWordTop = true;	//	Keyword Top
//	const CDocLine*			pCDocLine;

//@@@ 2001.11.17 add start MIK
	int		nMatchLen;
	int		nMatchColor;
//@@@ 2001.11.17 add end MIK

	bSearchStringMode = FALSE;
	bSearchFlg	= TRUE;	// 2002.02.08 hor
	nSearchStart= -1;	// 2002.02.08 hor
	nSearchEnd	= -1;	// 2002.02.08 hor

	/* �_���s�f�[�^�̎擾 */
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory�ύX
		nLineLen = pcLayout->m_pCDocLine->m_pLine->GetLength() - pcLayout->m_nOffset;
		pLine = pcLayout->m_pCDocLine->m_pLine->GetPtr() + pcLayout->m_nOffset;

		nCOMMENTMODE = pcLayout->m_nTypePrev;	/* �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
		nCOMMENTEND = 0;
		pcLayout2 = pcLayout;

	}else{
		pLine = NULL;
		nLineLen = 0;
		nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
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
	// my_icmp�̗��p�ɂ��locale�Ɉˑ����Ȃ� 2002/11/30 Moca
	// setlocale ( LC_ALL, "C" );	//	Oct. 29, 2001 genta ����������̃n�C���C�g�Ɋ֌W����

	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

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
			pcLayout2 = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum );
			if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
				/* �s�ԍ��\�� */
				DispLineNumber( hdc, pcLayout2, nLineNum, y );
			}
			nBgn = nPos;
			nLineBgn = nBgn;
			nX = 0;

			//	�s���w�i
			if (pcLayout2 && pcLayout2->GetIndent()){
				rcClip.left = x;
				rcClip.right = x + pcLayout2->GetIndent() * ( nCharWidth );
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
				nX += pcLayout2->GetIndent();
			}

			while( nPos - nLineBgn < pcLayout2->m_nLength ){
				/* ����������̐F���� */
				if( TRUE == m_bCurSrchKeyMark	/* ����������̃}�[�N */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
				// 2002.02.08 hor ���K�\���̌���������}�[�N������������
					if(!bSearchStringMode && (!m_bCurSrchRegularExp || (bSearchFlg && nSearchStart < nPos))){
						bSearchFlg=IsSearchString( pLine, nLineLen, nPos, &nSearchStart, &nSearchEnd );
					}
					if( !bSearchStringMode && bSearchFlg && nSearchStart==nPos
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
						SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
					}else
					if( bSearchStringMode
					 && nSearchEnd <= nPos		//+ == �ł͍s�������̏ꍇ�AnSearchEnd���O�ł��邽�߂ɕ����F�̉������ł��Ȃ��o�O���C�� 2003.05.03 �����
					){
						// ��������������̏I���܂ł�����A�����F��W���ɖ߂�����
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
						rcClip2.left = x + nX * ( nCharWidth );
						// Jul. 20, 2003 ryoji ���X�N���[�����ɉ��s�R�[�h�������Ȃ��悤��
						rcClip2.right = rcClip2.left + ( nCharWidth ) * ( 2 );
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
							rcClip2.top = y;
							rcClip2.bottom = y + nLineHeight;
							// 2006.04.30 Moca �F�I�������ʓ��Ɉړ�
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
							colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT ); /* CRLF�̐F */
							colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );	/* CRLF�w�i�̐F */
							//	2003.08.17 ryoji ���s�����������Ȃ��悤��
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, (const char *)"  ", 2, m_pnDx );
							/* ���s�L���̕\�� */
							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_CRLF].m_bDisp ){
								nPosX = x + nX * ( nCharWidth );
								nPosY = y;
								//	From Here 2003.08.17 ryoji ���s�����������Ȃ��悤��
								HRGN hRgn;
								hRgn = ::CreateRectRgnIndirect(&rcClip2);
								::SelectClipRgn(hdc, hRgn);
								//@@@ 2001.12.21 YAZAKI
								DrawEOL(hdc, nPosX + 1, nPosY, m_nCharWidth, m_nCharHeight,
									pcLayout2->m_cEol, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont,
										TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT );
								::SelectClipRgn(hdc, NULL);
								::DeleteObject(hRgn);
								//	To Here 2003.08.17 ryoji ���s�����������Ȃ��悤��
							}
							::SelectObject( hdc, hFontOld );
							::SetTextColor( hdc, colTextColorOld );
							::SetBkColor( hdc, colBkColorOld );
						}

						nX++;


						// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
						DispVerticalLines( hdc, y, y + nLineHeight, 0, -1 );
						if( bSelected ){
							/* �e�L�X�g���] */
							DispTextSelected( hdc, nLineNum, x, y, nX );
						}
					}

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
						if( y/* + nLineHeight*/ >= m_nViewAlignTop )
						{
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						/* ���݂̐F���w�� */
						nBgn = nPos;
						//nCOMMENTMODE = 1000;	/* �F�w�� */
						nCOMMENTMODE = 1000 + nMatchColor;	/* �F�w�� */	//@@@ 2002.01.04 upd
						nCOMMENTEND = nPos + nMatchLen;  /* �L�[���[�h������̏I�[���Z�b�g���� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
							//SetCurrentColor( hdc, nCOMMENTMODE + nMatchColor );	//@@@ 2002.01.04
						}
					}
					else
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;

						nCOMMENTMODE = COLORIDX_COMMENT;	/* �s�R�����g�ł��� */ // 2002/03/13 novice

//						if( TypeDataPtr->m_bDispCOMMENT ){	/* �R�����g��\������ */
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
					}else
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice

//						if( TypeDataPtr->m_bDispCOMMENT ){	/* �R�����g��\������ */
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos + (int)lstrlen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI

//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
					}else
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
//						if( TypeDataPtr->m_bDispCOMMENT ){	/* �R�����g��\������ */
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								SetCurrentColor( hdc, nCOMMENTMODE );
							}
//						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos + (int)lstrlen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
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
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
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
						nCOMMENTMODE = COLORIDX_WSTRING;	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
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
					 && ( TRUE == IsURL( &pLine[nPos], nLineLen - nPos, &nUrlLen ) )	/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
					){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_URL;	/* URL���[�h */ // 2002/03/13 novice
						nCOMMENTEND = nPos + nUrlLen;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//@@@ 2001.02.17 Start by MIK: ���p���l�������\��
//#ifdef COMPILE_COLOR_DIGIT
					}else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (i = IsNumber( pLine, nPos, nLineLen )) > 0 )		/* ���p������\������ */
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
						nCOMMENTMODE = COLORIDX_DIGIT;	/* ���p���l�ł��� */ // 2002/03/13 novice
						nCOMMENTEND = i;
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
//#endif
//@@@ 2001.02.17 End by MIK: ���p���l�������\��
					}else
					if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* �L�[���[�h�Z�b�g */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* �����L�[���[�h��\������ */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						//	Mar 4, 2001 genta comment out
						//	bKeyWordTop = false;
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
							if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
								/* �e�L�X�g�\�� */
								nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
							}

							/* ���݂̐F���w�� */
							nBgn = nPos;
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
										&pLine[nPos],																		//MIK
										j																					//MIK
									);																						//MIK
									if( nIdx != -1 ){																		//MIK
										if( y/* + nLineHeight*/ >= m_nViewAlignTop ){										//MIK
											/* �e�L�X�g�\�� */																//MIK
											nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );	//MIK
										}																					//MIK
										/* ���݂̐F���w�� */																//MIK
										nBgn = nPos;																		//MIK
										nCOMMENTMODE = COLORIDX_KEYWORD1 + my_i;	/* �����L�[���[�h2 */ // 2002/03/13 novice		//MIK
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
					if( IS_KEYWORD_CHAR( pLine[nPos] ))	bKeyWordTop = false;
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
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							SetCurrentColor( hdc, nCOMMENTMODE );
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* �R���g���[���R�[�h */ // 2002/03/13 novice
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

				case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
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
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
							/* �e�L�X�g�\�� */
							nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						}
						nBgn = nPos;
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
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
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
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
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
						if( nPos == nCOMMENTEND ){
							if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
								/* �e�L�X�g�\�� */
								nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
							}
							nBgn = nPos;
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
				if( pLine[nPos] == TAB ){
					if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
						/* �e�L�X�g�\�� */
						nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
						//	Sep. 22, 2002 genta ���ʎ��̂����肾��
						//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
						int tabDispWidth = m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace( nX );
						/* �^�u�L����\������ */
						rcClip2.left = x + nX * ( nCharWidth );
						rcClip2.right = rcClip2.left + ( nCharWidth ) * tabDispWidth;
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){
							rcClip2.top = y;
							rcClip2.bottom = y + nLineHeight;
							/* TAB��\�����邩�H */
							if( TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_bDisp
							 && 0 == TypeDataPtr->m_bTabArrow ){	//�^�u�ʏ�\��	//@@@ 2003.03.26 MIK
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
								
								//@@@ 2001.03.16 by MIK
								::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
									&rcClip2, /*pszTAB*/ TypeDataPtr->m_szTabViewString,
									tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
									 m_pnDx );
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
									tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
									 m_pnDx );
								if( bSearchStringMode ){
									::SetBkColor( hdc, colBkColorOld );
								}
								
								//�^�u���\��	//@@@ 2003.03.26 MIK
								if( TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_bDisp
								 && 0 != TypeDataPtr->m_bTabArrow
								 && rcClip2.left <= x + nX * nCharWidth ) // Apr. 1, 2003 MIK �s�ԍ��Əd�Ȃ�
								{
									DrawTabArrow( hdc, x + nX * ( nCharWidth ), y, m_nCharWidth, m_nCharHeight,
										TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_bFatFont,
										TypeDataPtr->m_ColorInfoArr[COLORIDX_TAB].m_colTEXT );
								}
							}
						}
						nX += tabDispWidth ;//	Sep. 22, 2002 genta
					}
					nBgn = nPos + 1;
					nCharChars = 1;
				}else
				if( (unsigned char)pLine[nPos] == 0x81 && (unsigned char)pLine[nPos + 1] == 0x40	//@@@ 2001.11.17 upd MIK
				// && nCOMMENTMODE != 1000 )	//@@@ 2002.01.04
				 && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )	//@@@ 2002.01.04
				{	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04
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
				}
				//���p�󔒁i���p�X�y�[�X�j��\�� 2002.04.28 Add by KK 
				else if (pLine[nPos] == ' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp 
					 && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{
					nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
					if( y >= m_nViewAlignTop ){
						rcClip2.left = x + nX * ( nCharWidth );
						rcClip2.right = rcClip2.left + ( nCharWidth );
						if( rcClip2.left < m_nViewAlignLeft ){
							rcClip2.left = m_nViewAlignLeft;
						}
						if( rcClip2.left < rcClip2.right &&
							rcClip2.left < m_nViewAlignLeft + m_nViewCx && rcClip2.right > m_nViewAlignLeft ){

							if( bSearchStringMode ){
								nColorIdx = COLORIDX_SEARCH;
							}else{
								nColorIdx = COLORIDX_SPACE;
							}

							colTextColorOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colTEXT );	/* ���p�X�y�[�X�����̐F */
							colBkColorOld = ::SetBkColor( hdc, TypeDataPtr->m_ColorInfoArr[nColorIdx].m_colBACK );		/* ���p�X�y�[�X�����w�i�̐F */
							HFONT	hFontOld = (HFONT)::SelectObject( hdc,
								ChooseFontHandle(
									TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bFatFont,
									TypeDataPtr->m_ColorInfoArr[nColorIdx].m_bUnderLine
								)
							);
							
							//������"o"�̉��������o��
							rcClip2.top = y + (int)((nLineHeight/2) ) ;
							rcClip2.bottom = y + nLineHeight;
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, "o", 1, m_pnDx );
							
							//�㔼���͕��ʂ̋󔒂ŏo�́i"o"�̏㔼���������j
							rcClip2.top = y ;
							rcClip2.bottom = y + (int)((nLineHeight/2) ) ;
							::ExtTextOut( hdc, x + nX * ( nCharWidth ), y, fuOptions,
								&rcClip2, " ", 1, m_pnDx );

							::SelectObject( hdc, hFontOld );
							::SetTextColor( hdc, colTextColorOld );
							::SetBkColor( hdc, colBkColorOld );
						}
						nX++;
					}
					nBgn = nPos + 1;
					nCharChars = 1;
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* �R���g���[���R�[�h��F���� */
					 &&	(
								//	Jan. 23, 2002 genta �x���}��
							( (unsigned char)pLine[nPos] <= (unsigned char)0x1F ) ||
							( (unsigned char)'~' < (unsigned char)pLine[nPos] && (unsigned char)pLine[nPos] < (unsigned char)'�' ) ||
							( (unsigned char)'�' < (unsigned char)pLine[nPos] )
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
						nCOMMENTMODE = COLORIDX_CTRLCODE;	/* �R���g���[���R�[�h ���[�h */ // 2002/03/13 novice
						/* �R���g���[���R�[�h��̏I�[��T�� */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if( (
								//	Jan. 23, 2002 genta �x���}��
								( (unsigned char)pLine[i] <= (unsigned char)0x1F ) ||
									( (unsigned char)'~' < (unsigned char)pLine[i] && (unsigned char)pLine[i] < (unsigned char)'�' ) ||
									( (unsigned char)'�' < (unsigned char)pLine[i] )
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
			} //end of while( nPos - nLineBgn < pcLayout2->m_nLength )
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
				// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
				DispVerticalLines( hdc, y, y + nLineHeight,  0, -1 );
				if( bSelected ){
					/* �e�L�X�g���] */
					DispTextSelected( hdc, nLineNum, x, y, nX );
				}
			}
		}
		
		if( y >= m_nViewAlignTop ){
			/* �e�L�X�g�\�� */
			nX += DispText( hdc, x + nX * ( nCharWidth ), y, &pLine[nBgn], nPos - nBgn );
			/* EOF�L���̕\�� */
			if( nLineNum + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
				nX < nWrapWidth
			){
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					//	May 29, 2004 genta (advised by MIK) ���ʊ֐���
					nX += DispEOF( hdc, x + nX * ( nCharWidth ), y, nCharWidth, nLineHeight, fuOptions,
						TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF] );
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
			// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
			DispVerticalLines( hdc, y, y + nLineHeight,  0, -1 );
			if( bSelected ){
				/* �e�L�X�g���] */
				DispTextSelected( hdc, nLineNum, x, y, nX );
			}
		}
end_of_line:;
		nLineNum++;
		y += nLineHeight;
	}else{ // NULL == pLine�̏ꍇ
		if( y/* + nLineHeight*/ >= m_nViewAlignTop ){
			int nYPrev = y;
			
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
					//	May 29, 2004 genta (advised by MIK) ���ʊ֐���
					nX += DispEOF( hdc, x, y, nCharWidth, nLineHeight, fuOptions,
						TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF] );
				}
				y += nLineHeight;
				bEOF = TRUE;
			}else{
				if( nCount > 0 && nLineNum == nCount ){
					const char*	pLine;
					int			nLineLen;
					const CLayout* pcLayout;
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCount - 1, &nLineLen, &pcLayout );
					nLineCols = LineIndexToColmn( pcLayout, nLineLen );
					if( ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ) ||
						nLineCols >= nWrapWidth
					 ){
						/* EOF�L���̕\�� */
						if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
							//	May 29, 2004 genta (advised by MIK) ���ʊ֐���
							nX += DispEOF( hdc, x, y, nCharWidth, nLineHeight, fuOptions,
								TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF] );
						}
						y += nLineHeight;
						bEOF = TRUE;
					}
				}
			}
			// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
			DispVerticalLines( hdc, nYPrev, nYPrev + nLineHeight,  0, -1 );
		}
	}

end_of_func:;
//	2002/05/08 YAZAKI �A���_�[���C���̍ĕ`��͕s�v�ł���
//	MYTRACE( "m_nOldUnderLineY=%d\n", m_nOldUnderLineY );
//	if( -1 != m_nOldUnderLineY ){
//		/* �J�[�\���s�A���_�[���C����ON */
//		CaretUnderLineON( TRUE );
//	}
	return bEOF;
}

//	May 23, 2000 genta
/*! ��ʕ`��⏕�֐�:
	�s���̉��s�}�[�N�����s�R�[�h�ɂ���ď���������i���C���j

	@param hdc Device Context Handle
	@param nPosX �`����WX
	@param nPosY �`����WY
	@param nWidth  �`��G���A�̃T�C�YX
	@param nHeight �`��G���A�̃T�C�YY
	@param cEol �s���R�[�h���
	@param bBold TRUE: ����
	@param pColor �F

	@note bBold��TRUE�̎��͉���1�h�b�g���炵�ďd�ˏ������s�����A
	���܂葾�������Ȃ��B
	
	@date 2001.12.21 YAZAKI ���s�L���̕`��������ύX�B�y���͂��̊֐����ō��悤�ɂ����B
							���̐擪���Asx, sy�ɂ��ĕ`�惋�[�`�����������B
*/
void CEditView::DrawEOL( HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight, CEOL cEol, int bBold, COLORREF pColor )
{
	int sx, sy;	//	���̐擪
	HANDLE	hPen;
	HPEN	hPenOld;
	hPen = ::CreatePen( PS_SOLID, 1, pColor );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	switch( cEol.GetType() ){
	case EOL_CRLF:	//	�������
		sx = nPosX;
		sy = nPosY + ( nHeight / 2);
		::MoveToEx( hdc, sx + nWidth, sy - nHeight / 4, NULL );	//	���
		::LineTo(   hdc, sx + nWidth, sy );			//	����
		::LineTo(   hdc, sx, sy );					//	�擪��
		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	�擪���牺��
		::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	�擪������
		if ( bBold ) {
			::MoveToEx( hdc, sx + nWidth + 1, sy - nHeight / 4, NULL );	//	��ցi�E�ւ��炷�j
			++sy;
			::LineTo( hdc, sx + nWidth + 1, sy );	//	�E�ցi�E�ɂЂƂ���Ă���j
			::LineTo(   hdc, sx, sy );					//	�擪��
			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	�擪���牺��
			::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	�擪������
		}
//		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//		::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy - ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL);
//		::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy - ( nHeight * 5 / 20 ) );
//		if( bBold ){
//			++sx;
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//			::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//			::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), nPosY + ( nHeight * 15/ 20) - ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL);
//			::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), nPosY + ( nHeight * 15/ 20) - ( nHeight * 5 / 20 ) );
//		}
		break;
	case EOL_CR:	//	���������	// 2007.08.17 ryoji EOL_LF -> EOL_CR
		sx = nPosX;
		sy = nPosY + ( nHeight / 2 );
		::MoveToEx( hdc, sx + nWidth, sy, NULL );	//	�E��
		::LineTo(   hdc, sx, sy );					//	�擪��
		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	�擪���牺��
		::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	�擪������
		if ( bBold ) {
			++sy;
			::MoveToEx( hdc, sx + nWidth, sy, NULL );	//	�E��
			::LineTo(   hdc, sx, sy );					//	�擪��
			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	�擪���牺��
			::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	�擪������
		}
//		::MoveToEx( hdc, nPosX, sy, NULL );
//		::LineTo(   hdc, nPosX + nWidth, sy );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4);
//		if( bBold ){
//			++sy;
//			::MoveToEx( hdc, nPosX, sy, NULL );
//			::LineTo(   hdc, nPosX + nWidth, sy );
//			::MoveToEx( hdc, sx, sy, NULL );
//			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4);
//		}
		break;
	case EOL_LF:	//	���������	// 2007.08.17 ryoji EOL_CR -> EOL_LF
		sx = nPosX + ( nWidth / 2 );
		sy = nPosY + ( nHeight * 3 / 4 );
		::MoveToEx( hdc, sx, nPosY + nHeight / 4 + 1, NULL );	//	���
		::LineTo(   hdc, sx, sy );								//	�ォ�牺��
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	���̂܂܍����
		::MoveToEx( hdc, sx, sy, NULL);							//	���̐�[�ɖ߂�
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	�����ĉE���
		if( bBold ){
			++sx;
			::MoveToEx( hdc, sx, nPosY + nHeight / 4 + 1, NULL );
			::LineTo(   hdc, sx, sy );								//	�ォ�牺��
			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	���̂܂܍����
			::MoveToEx( hdc, sx, sy, NULL);							//	���̐�[�ɖ߂�
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	�����ĉE���
		}
//#if 1
//		sx = nPosX + nWidth;
//		sy = nPosY + ( nHeight / 2 );
//		::MoveToEx( hdc, nPosX, sy, NULL );
//		::LineTo(   hdc, nPosX + nWidth, sy );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//		::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4);
//		if( bBold ){
//			++sy;
//			::MoveToEx( hdc, nPosX, sy, NULL );
//			::LineTo(   hdc, nPosX + nWidth, sy );
//			::MoveToEx( hdc, sx, sy, NULL );
//			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//			::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4);
//		}
//#else
//		sx = nPosX;
//		::MoveToEx( hdc, sx + nWidth - 3, nPosY + nHeight * 1 / 4, NULL );
//		::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4);
//		::LineTo(   hdc, sx, nPosY + nHeight * 3 / 4 );
//		::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4 - nHeight / 4);
//		if( bBold ){
//			++sx;
//			::MoveToEx( hdc, sx + nWidth - 3, nPosY + nHeight * 1 / 4, NULL );
//			::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4);
//			::LineTo(   hdc, sx, nPosY + nHeight * 3 / 4 );
//			::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4 - nHeight / 4);
//		}
//#endif
		break;
	case EOL_LFCR:
		sx = nPosX + ( nWidth / 2 );
		sy = nPosY + ( nHeight * 3 / 4 );
		::MoveToEx( hdc, sx + nWidth / 2, nPosY + nHeight / 4 + 1, NULL );	//	�E���
		::LineTo(   hdc, sx, nPosY + nHeight / 4 + 1 );			//	�E���獶��
		::LineTo(   hdc, sx, sy );								//	�ォ�牺��
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	���̂܂܍����
		::MoveToEx( hdc, sx, sy, NULL);							//	���̐�[�ɖ߂�
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	�����ĉE���
		if( bBold ){
			::MoveToEx( hdc, sx + nWidth / 2, nPosY + nHeight / 4 + 2, NULL );	//	�E���
			++sx;
			::LineTo(   hdc, sx, nPosY + nHeight / 4 + 2 );			//	�E���獶��
			::LineTo(   hdc, sx, sy );								//	�ォ�牺��
			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	���̂܂܍����
			::MoveToEx( hdc, sx, sy, NULL);							//	���̐�[�ɖ߂�
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	�����ĉE���
		}
//		sx = nPosX + ( nWidth / 2 );
//		sy = nPosY + ( nHeight * 1 / 4 );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//		::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, sy, NULL);
//		::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//		if( bBold ){
//			++sx;
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//			::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, sy, NULL );
//			::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, sy, NULL);
//			::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//		}
		break;
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}

/*
	�^�u���`��֐�
*/
void CEditView::DrawTabArrow( HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight, int bBold, COLORREF pColor )
{
	int sx, sy;	//	���̐擪
	HANDLE	hPen;
	HPEN	hPenOld;

	hPen = ::CreatePen( PS_SOLID, 1, pColor );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	nWidth--;
	sx = nPosX + nWidth;
	sy = nPosY + ( nHeight / 2 );
	::MoveToEx( hdc, sx - nWidth, sy, NULL );	//	����
	::LineTo(   hdc, sx, sy );					//	�Ō��
	::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	�Ōォ�牺��
	::MoveToEx( hdc, sx, sy, NULL);				//	�Ō�֖߂�
	::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	�Ōォ����
	if ( bBold ) {
		++sy;
		::MoveToEx( hdc, sx - nWidth, sy, NULL );	//	����
		::LineTo(   hdc, sx, sy );					//	�Ō��
		::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	�Ōォ�牺��
		::MoveToEx( hdc, sx, sy, NULL);				//	�Ō�֖߂�
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	�Ōォ����
	}

	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}

/*! EOF�L���̕`��

	@param hdc	[in] �`��Ώۂ�Device Context
	@param x	[in] �\�����Wx
	@param y	[in] �\�����Wy
	@param nCharWidth	[in] 1�����̉���(pixel)
	@param fuOptions	[in] ExtTextOut()�ɓn���`��I�v�V����
	@param EofColInfo	[in] �`��Ɏg���F���

  @date 2004.05.29 genta MIK����̃A�h�o�C�X�ɂ��֐��ɂ����肾��
*/
int CEditView::DispEOF( HDC hdc, int x, int y, int nCharWidth, int nLineHeight, UINT fuOptions, const ColorInfo& EofColInfo )
{
	//	���̐F��ۑ����邽��
	COLORREF	colTextColorOld;
	COLORREF	colBkColorOld;
	RECT		rcClip;
	const char	pszEOF[] = "[EOF]";
	const int	szEOFlen = sizeof( pszEOF) - 1;

	colTextColorOld = ::SetTextColor( hdc, EofColInfo.m_colTEXT );	/* EOF�̐F */
	colBkColorOld = ::SetBkColor( hdc, EofColInfo.m_colBACK );		/* EOF�w�i�̐F */
	rcClip.left = /*m_nViewAlignLeft*/x;
	rcClip.right = rcClip.left + ( nCharWidth ) * szEOFlen;
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
				EofColInfo.m_bFatFont,
				EofColInfo.m_bUnderLine
			)
		);
//		if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bFatFont ){	/* ������ */
//			hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//		}else{
//			hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//								}

		::ExtTextOut( hdc, x , y, fuOptions,
			&rcClip, pszEOF, szEOFlen, m_pnDx );

		::SelectObject( hdc, hFontOld );
	}

	::SetTextColor( hdc, colTextColorOld );
	::SetBkColor( hdc, colBkColorOld );
	
	return szEOFlen;
}
// �@�@�@

/*!	�w�茅�c���̕`��
	@param hdc     ��悷��E�B���h�E��DC
	@param nTop    ����������[�̃N���C�A���g���Wy
	@param nButtom �����������[�̃N���C�A���g���Wy
	@param nColLeft  ���������͈͂̍����̎w��
	@param nColRight ���������͈͂̉E���̎w��(-1�Ŗ��w��)

	@date 2005.11.08 Moca �V�K�쐬
	@date 2006.04.29 Moca �����E�_���̃T�|�[�g�B�I�𒆂̔��]�΍�ɍs���Ƃɍ�悷��悤�ɕύX
	    �c���̐F���e�L�X�g�̔w�i�F�Ɠ����ꍇ�́A�c���̔w�i�F��EXOR�ō�悷��
	@note Common::m_nVertLineOffset�ɂ��A�w�茅�̑O�̕����̏�ɍ�悳��邱�Ƃ�����B
*/
void CEditView::DispVerticalLines( HDC hdc, int nTop, int nBottom, int nLeftCol, int nRightCol )
{
	const Types&	typeData = m_pcEditDoc->GetDocumentAttribute();
	if( typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_bDisp == FALSE ){
		return;
	}
	nLeftCol = __max( m_nViewLeftCol, nLeftCol );
	const int nWrapWidth  = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();
	const int nCharWidth  = m_nCharWidth + typeData.m_nColmSpace;
	if( nRightCol < 0 ){
		nRightCol = nWrapWidth;
	}
	const int nPosXOffset = m_pShareData->m_Common.m_nVertLineOffset + m_nViewAlignLeft;
	const int nPosXLeft   = __max( m_nViewAlignLeft + (nLeftCol  - m_nViewLeftCol) * nCharWidth, m_nViewAlignLeft );
	const int nPosXRight  = __min( m_nViewAlignLeft + (nRightCol - m_nViewLeftCol) * nCharWidth, m_nViewCx + m_nViewAlignLeft );
	const int nLineHeight = m_nCharHeight + typeData.m_nLineSpace;
	bool bOddLine = ((((nLineHeight % 2) ? m_nViewTopLine : 0) + m_nViewAlignTop + nTop) % 2 == 1);

	// ����
	const BOOL bBold = typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_bFatFont;
	// �h�b�g��(����������]�p/�e�X�g�p)
	const BOOL bDot = typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_bUnderLine;
	const bool bExorPen = ( typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_colTEXT 
		== typeData.m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
	HPEN hPen;
	int nROP_Old = 0;
	if( bExorPen ){
		hPen = ::CreatePen( PS_SOLID, 0, typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_colBACK );
		nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
	}else{
		hPen = ::CreatePen( PS_SOLID, 0, typeData.m_ColorInfoArr[COLORIDX_VERTLINE].m_colTEXT );
	}
	HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );

	int k;
	for( k = 0; k < MAX_VERTLINES && typeData.m_nVertLineIdx[k] != 0; k++ ){
		// nXCol��1�J�n�Bm_nViewLeftCol��0�J�n�Ȃ̂Œ��ӁB
		int nXCol = typeData.m_nVertLineIdx[k];
		int nXColEnd = nXCol;
		int nXColAdd = 1;
		// nXCol���}�C�i�X���ƌJ��Ԃ��Bk+1���I���l�Ak+2���X�e�b�v���Ƃ��ė��p����
		if( nXCol < 0 ){
			if( k < MAX_VERTLINES - 2 ){
				nXCol = -nXCol;
				nXColEnd = typeData.m_nVertLineIdx[++k];
				nXColAdd = typeData.m_nVertLineIdx[++k];
				if( nXColEnd < nXCol || nXColAdd <= 0 ){
					continue;
				}
				// ���͈͂̎n�߂܂ŃX�L�b�v
				if( nXCol < m_nViewLeftCol ){
					nXCol = m_nViewLeftCol + nXColAdd - (m_nViewLeftCol - nXCol) % nXColAdd;
				}
			}else{
				k += 2;
				continue;
			}
		}
		for(; nXCol <= nXColEnd; nXCol += nXColAdd ){
			if( nWrapWidth < nXCol ){
				break;
			}
			int nPosX = nPosXOffset + ( nXCol - 1 - m_nViewLeftCol ) * nCharWidth;
			// 2006.04.30 Moca ���̈����͈́E���@��ύX
			// �����̏ꍇ�A����������悷��\��������B
			int nPosXBold = nPosX;
			if( bBold ){
				nPosXBold -= 1;
			}
			if( nPosXRight <= nPosXBold ){
				break;
			}
			if( nPosXLeft <= nPosX ){
				if( bDot ){
					// �_���ō��B1�h�b�g�̐����쐬
					int y = nTop;
					// �X�N���[�����Ă������؂�Ȃ��悤�ɍ��W�𒲐�
					if( bOddLine ){
						y++;
					}
					for( ; y < nBottom; y += 2 ){
						if( nPosX < nPosXRight ){
							::MoveToEx( hdc, nPosX, y, NULL );
							::LineTo( hdc, nPosX, y + 1 );
						}
						if( bBold && nPosXLeft <= nPosXBold ){
							::MoveToEx( hdc, nPosXBold, y, NULL );
							::LineTo( hdc, nPosXBold, y + 1 );
						}
					}
				}else{
					if( nPosX < nPosXRight ){
						::MoveToEx( hdc, nPosX, nTop, NULL );
						::LineTo( hdc, nPosX, nBottom );
					}
					if( bBold && nPosXLeft <= nPosXBold ){
						::MoveToEx( hdc, nPosXBold, nTop, NULL );
						::LineTo( hdc, nPosXBold, nBottom );
					}
				}
			}
		}
	}
	if( bExorPen ){
		::SetROP2( hdc, nROP_Old );
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}


/*[EOF]*/
