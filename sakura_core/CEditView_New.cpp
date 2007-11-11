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
#include "CRegexKeyword.h"	//@@@ 2001.11.17 add MIK
#include "my_icmp.h"	//@@@ 2002.01.13 add
#include "Clayout.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/2/10 aroka
#include "CTypeSupport.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"

// �ʏ�̕`�揈�� new
void CEditView::OnPaint( HDC hdc, PAINTSTRUCT *pPs, BOOL bUseMemoryDC )
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::OnPaint" );

	// 2004.01.28 Moca �f�X�N�g�b�v�ɍ�悵�Ȃ��悤��
	if( NULL == hdc )return;

	if( !GetDrawSwitch() )return;

	// �L�����b�g���B��
	GetCaret().HideCaret_( m_hWnd ); // 2002/07/22 novice

	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

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
	GetCaret().ShowCaret_( m_hWnd ); // 2002/07/22 novice
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
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DispLineNew" );

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
	Types* TypeDataPtr = &m_pcEditDoc->GetDocumentAttribute();
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
		nLineLen = pcLayout->m_pCDocLine->m_cLine.GetStringLength() - pcLayout->GetLogicOffset();
		pLine    = pcLayout->m_pCDocLine->m_cLine.GetStringPtr()    + pcLayout->GetLogicOffset();

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
			while( GetNPos() - nLineBgn < pcLayout2->GetLength() ){
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

				if( GetNPos() >= nLineLen - pcLayout2->m_cEol.GetLen() ){
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
						GetTextDrawer().DispEOL(hdc,pDispPos,pcLayout2->m_cEol,bSearchStringMode);

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



// �@�@�@


/*[EOF]*/
