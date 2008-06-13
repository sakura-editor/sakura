#include "stdafx.h"
#include "CEditView_Paint.h"
#include "view/CEditView.h"
#include <vector>
#include "types/CTypeSupport.h"
#include "doc/CLayout.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "CDrawStrategy.h"
#include "CDraw_Comment.h"
#include "CDraw_Quote.h"
#include "CDraw_RegexKeyword.h"
#include "CDraw_Space.h"
#include "CDraw_Found.h"

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
	OnPaint( hdc, &ps, FALSE );
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

	OnPaint( hdc, &ps, FALSE );

	::ReleaseDC( GetHwnd(), hdc );
}
// 2001/06/21 End



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �F�ݒ�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! �w��ʒu��ColorIndex�̎擾
	CEditView::DrawLogicLine�����ɂ�������CEditView::DrawLogicLine��
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
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->m_cDocType.GetDocumentAttribute());

	int						nLineBgn;
	const CLayout*			pcLayout2;
	bool					bSearchFlg;			// 2002.02.08 hor
	int						nLineOffset;		// 2008/5/29 Uchi
	SColorInfo sInfo;
	SColorInfo* pInfo = &sInfo;
	sInfo.pcView = this;
	sInfo.bKeyWordTop = true;	//	Keyword Top


	pInfo->bSearchStringMode = FALSE;
	bSearchFlg	= true;	// 2002.02.08 hor

	CLogicInt	nSearchStart = CLogicInt(-1);
	CLogicInt	nSearchEnd   = CLogicInt(-1);

	/* �_���s�f�[�^�̎擾 */
	if( pcLayout ){
		// 2002/2/10 aroka CMemory�ύX
		pInfo->nLineLen = pcLayout->GetDocLineRef()->GetLengthWithEOL()/* - pcLayout->GetLogicOffset()*/;	// 03/10/24 ai �܂�Ԃ��s��ColorIndex���������擾�ł��Ȃ����ɑΉ�
		pInfo->pLine = pcLayout->GetPtr()/* + pcLayout->GetLogicOffset()*/;			// 03/10/24 ai �܂�Ԃ��s��ColorIndex���������擾�ł��Ȃ����ɑΉ�

		// 2005.11.20 Moca �F���������Ȃ����Ƃ�������ɑΏ�
		const CLayout* pcLayoutLineFirst = pcLayout;
		// �_���s�̍ŏ��̃��C�A�E�g�����擾����
		while( 0 != pcLayoutLineFirst->GetLogicOffset() ){
			pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
		}
		pInfo->nCOMMENTMODE = pcLayoutLineFirst->GetColorTypePrev();
		pInfo->nCOMMENTEND = 0;
		pcLayout2 = pcLayout;

		nLineOffset = pcLayout->GetLogicOffset();	// 2008/5/29 Uchi
	}
	else{
		pInfo->pLine = NULL;
		pInfo->nLineLen = CLogicInt(0);
		pInfo->nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		pInfo->nCOMMENTEND = 0;
		pcLayout2 = NULL;
		nLineOffset = 0;							// 2008/5/29 Uchi
	}

	/* ���݂̐F���w�� */
	//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
	pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai

	pInfo->nBgn = 0;
	pInfo->nPos = CLogicInt(0);
	nLineBgn = 0;

	CLogicInt				nCharChars = CLogicInt(0);
	CLogicInt				nCharChars_2; //##############################��ŏ���

	if( NULL != pInfo->pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		while( pInfo->nPos <= nCol ){	// 03/10/24 ai �s����ColorIndex���擾�ł��Ȃ����ɑΉ�

			pInfo->nBgn = pInfo->nPos;
			nLineBgn = pInfo->nBgn;

			while( pInfo->nPos - nLineBgn <= nCol ){	// 02/12/18 ai
				/* ����������̐F���� */
				if( m_bCurSrchKeyMark	/* ����������̃}�[�N */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
				// 2002.02.08 hor ���K�\���̌���������}�[�N������������
					if(!pInfo->bSearchStringMode && (!m_sCurSearchOption.bRegularExp || (bSearchFlg && nSearchStart < pInfo->nPos))){
						bSearchFlg=IsSearchString( pInfo->pLine, pInfo->nLineLen, pInfo->nPos, &nSearchStart, &nSearchEnd );
					}
					if( !pInfo->bSearchStringMode && bSearchFlg && nSearchStart==pInfo->nPos ){
						pInfo->nBgn = pInfo->nPos;
						pInfo->bSearchStringMode = TRUE;
						/* ���݂̐F���w�� */
						//@SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
						pInfo->nColorIndex = COLORIDX_SEARCH;	// 02/12/18 ai
					}
					else if( pInfo->bSearchStringMode && nSearchEnd == pInfo->nPos ){
						pInfo->nBgn = pInfo->nPos;
						/* ���݂̐F���w�� */
						//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
						pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
						pInfo->bSearchStringMode = FALSE;
						goto searchnext;
					}
				}

				if( pInfo->nPos >= pInfo->nLineLen - pcLayout2->GetLayoutEol().GetLen() ){
					goto end_of_line;
				}
SEARCH_START:;
				switch( pInfo->nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
					if( CDraw_RegexKeyword().GetColorIndexImp(&sInfo) ) { }	//���K�\���L�[���[�h
					else if( CDraw_LineComment().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_BlockComment().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_BlockComment2().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_SingleQuote().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_DoubleQuote().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_URL().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_Numeric().GetColorIndexImp(&sInfo) ) { }
					else if( CDraw_KeywordSet().GetColorIndexImp(&sInfo) ) { }

					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pInfo->pLine[pInfo->nPos - nLineOffset] ))	pInfo->bKeyWordTop = false;		// 2008/5/29 Uchi
					else								pInfo->bKeyWordTop = true;
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
					if( pInfo->nPos == pInfo->nCOMMENTEND ){
						pInfo->nBgn = pInfo->nPos;
						pInfo->nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !pInfo->bSearchStringMode ){
							//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
							pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* �R���g���[���R�[�h */ // 2002/03/13 novice
					if( pInfo->nPos == pInfo->nCOMMENTEND ){
						pInfo->nBgn = pInfo->nPos;
						pInfo->nCOMMENTMODE = pInfo->nCOMMENTMODE_OLD;
						pInfo->nCOMMENTEND = pInfo->nCOMMENTEND_OLD;
						/* ���݂̐F���w�� */
						if( !pInfo->bSearchStringMode ){
							//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
							pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;

				case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
					if( CDraw_BlockComment().GetColorIndexImpEnd(pInfo) ){
						goto SEARCH_START;
					}
					break;
				case COLORIDX_BLOCK2:	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
					if( CDraw_BlockComment2().GetColorIndexImpEnd(pInfo) ){
						goto SEARCH_START;
					}
					break;
				case COLORIDX_SSTRING:	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
					if( CDraw_SingleQuote().GetColorIndexImpEnd(pInfo) ){
						goto SEARCH_START;
					}
					break;
				case COLORIDX_WSTRING:	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
					if( CDraw_DoubleQuote().GetColorIndexImpEnd(pInfo) ){
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( pInfo->nCOMMENTMODE >= 1000 && pInfo->nCOMMENTMODE <= 1099 ){	//���K�\���L�[���[�h1�`10
						if( pInfo->nPos == pInfo->nCOMMENTEND ){
							pInfo->nBgn = pInfo->nPos;
							pInfo->nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* ���݂̐F���w�� */
							if( !pInfo->bSearchStringMode ){
								//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
								pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}
				if( pInfo->pLine[pInfo->nPos - nLineOffset] == WCODE::TAB ){		// 2008/5/29 Uchi
					pInfo->nBgn = pInfo->nPos + 1;
					nCharChars = CLogicInt(1);
				}
				else if( WCODE::IsZenkakuSpace(pInfo->pLine[pInfo->nPos - nLineOffset]) && (pInfo->nCOMMENTMODE < 1000 || pInfo->nCOMMENTMODE > 1099) )	//@@@ 2002.01.04	// 2008/5/29 Uchi
				{
					pInfo->nBgn = pInfo->nPos + 1;
					nCharChars = CLogicInt(1);
				}
				//���p�󔒁i���p�X�y�[�X�j��\�� 2002.04.28 Add by KK 
				else if (pInfo->pLine[pInfo->nPos - nLineOffset] == L' ' && CTypeSupport(this,COLORIDX_SPACE).IsDisp() && (pInfo->nCOMMENTMODE < 1000 || pInfo->nCOMMENTMODE > 1099) )		// 2008/5/29 Uchi
				{
					pInfo->nBgn = pInfo->nPos + 1;
					nCharChars = CLogicInt(1);
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, pInfo->nPos );
					if( 0 == nCharChars ){
						nCharChars = CLogicInt(1);
					}
					if( !pInfo->bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != pInfo->nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* �R���g���[���R�[�h��F���� */
					 && WCODE::IsControlCode(pInfo->pLine[pInfo->nPos - nLineOffset])		// 2008/5/29 Uchi
					){
						pInfo->nBgn = pInfo->nPos;
						pInfo->nCOMMENTMODE_OLD = pInfo->nCOMMENTMODE;
						pInfo->nCOMMENTEND_OLD = pInfo->nCOMMENTEND;
						pInfo->nCOMMENTMODE = COLORIDX_CTRLCODE;	/* �R���g���[���R�[�h ���[�h */ // 2002/03/13 novice
						/* �R���g���[���R�[�h��̏I�[��T�� */
						int i;
						for( i = pInfo->nPos + 1; i <= pInfo->nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CNativeW::GetSizeOfChar( pInfo->pLine, pInfo->nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = CLogicInt(1);
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if(!WCODE::IsControlCode(pInfo->pLine[i - nLineOffset])){		// 2008/5/29 Uchi
								break;
							}
						}
						pInfo->nCOMMENTEND = i;
						/* ���݂̐F���w�� */
						//@SetCurrentColor( hdc, pInfo->nCOMMENTMODE );
						pInfo->nColorIndex = pInfo->nCOMMENTMODE;	// 02/12/18 ai
					}
				}
				pInfo->nPos+= nCharChars;
			} //end of while( pInfo->nPos - nLineBgn < pcLayout2->m_nLength ){
			if( pInfo->nPos > nCol ){	// 03/10/24 ai �s����ColorIndex���擾�ł��Ȃ����ɑΉ�
				break;
			}
		}

end_of_line:;

	}

//@end_of_func:;
	return pInfo->nColorIndex;
}


/* ���݂̐F���w�� */
void CEditView::SetCurrentColor( HDC hdc, int nCOMMENTMODE )
{
	int				nColorIdx;
	COLORREF		colText;
	COLORREF		colBack;

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

/*! �ʏ�̕`�揈�� new 
	@param pPs  pPs.rcPaint �͐������K�v������
	@param bDrawFromComptibleBmp  TRUE ��ʃo�b�t�@����hdc�ɍ�悷��(�R�s�[���邾��)�B
			TRUE�̏ꍇ�ApPs.rcPaint�̈�O�͍�悳��Ȃ����AFALSE�̏ꍇ�͍�悳��鎖������B
			�݊�DC/BMP�������ꍇ�́A���ʂ̍�揈��������B
@date 2007.09.09 Moca ���X����������Ă�����O�p�����[�^��bUseMemoryDC��bDrawFromComptibleBmp�ɕύX�B
*/
void CEditView::OnPaint( HDC hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp )
{
//	MY_RUNNINGTIMER( cRunningTimer, "CEditView::OnPaint" );

	// 2004.01.28 Moca �f�X�N�g�b�v�ɍ�悵�Ȃ��悤��
	if( NULL == hdc )return;

	if( !GetDrawSwitch() )return;

	//@@@
#ifdef _DEBUG
	::MYTRACE( _T("OnPaint(%d,%d)-(%d,%d) : %d\n"),
		pPs->rcPaint.left,
		pPs->rcPaint.top,
		pPs->rcPaint.right,
		pPs->rcPaint.bottom,
		bDrawFromComptibleBmp
		);
#endif
	
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	// �݊�BMP����̓]���݂̂ɂ����
	if( bDrawFromComptibleBmp
		&& m_hdcCompatDC && m_hbmpCompatBMP ){
		::BitBlt(
			hdc,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			m_hdcCompatDC,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
		if ( m_pcEditWnd->m_nActivePaneIndex == m_nMyIndex ){
			/* �A�N�e�B�u�y�C���́A�A���_�[���C���`�� */
			GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
		}
		return;
	}
	if( m_hdcCompatDC && NULL == m_hbmpCompatBMP
		 || (pPs->rcPaint.right - pPs->rcPaint.left) < m_nCompatBMPWidth
		 || (pPs->rcPaint.bottom - pPs->rcPaint.top) < m_nCompatBMPHeight ){
		RECT rect;
		::GetWindowRect( this->GetHwnd(), &rect );
		CreateOrUpdateCompatibleBitmap( rect.right - rect.left, rect.bottom - rect.top );
	}
	// To Here 2007.09.09 Moca

	// �L�����b�g���B��
	bool bCaretShowFlag_Old = GetCaret().GetCaretShowFlag();	// 2008.06.09 ryoji
	GetCaret().HideCaret_( this->GetHwnd() ); // 2002/07/22 novice

	//	May 9, 2000 genta
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->m_cDocType.GetDocumentAttribute());

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	RECT			rc;
	int				nLineHeight = GetTextMetrics().GetHankakuDy();
	int				nCharDx = GetTextMetrics().GetHankakuDx();

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
	// 2007.09.09 Moca bUseMemoryDC��L�����B
	// bUseMemoryDC = FALSE;
	BOOL bUseMemoryDC = (m_hdcCompatDC != NULL);
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
		rc.right  = GetTextArea().GetAreaLeft() - GetDllShareData().m_Common.m_sWindow.m_nLineNumRightSpace; //	Sep. 23 ,2002 genta �]���̓e�L�X�g�F�̂܂܎c��
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



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//           �`��J�n���C�A�E�g��΍s -> nLayoutLine             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt nLayoutLine;
	if( 0 > nTop - GetTextArea().GetAreaTop() ){
		nLayoutLine = GetTextArea().GetViewTopLine(); //�r���[�㕔����`��
	}else{
		nLayoutLine = GetTextArea().GetViewTopLine() + CLayoutInt( ( nTop - GetTextArea().GetAreaTop() ) / nLineHeight ); //�r���[�r������`��
	}

	int nMaxRollBackLineNum = 260 / (Int)nWrapKeta + 1;
	int nRollBackLineNum = 0;
	const CLayout*	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLayoutLine );
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
	//          �`��I�����C�A�E�g��΍s -> nLayoutLineTo            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt nLayoutLineTo = GetTextArea().GetViewTopLine()
		+ CLayoutInt( ( pPs->rcPaint.bottom - GetTextArea().GetAreaTop() ) / nLineHeight );


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
	while(sPos.GetLayoutLineRef() <= nLayoutLineTo)
	{
		//�`��X�ʒu���Z�b�g
		sPos.ResetDrawCol();

		//1�s�`��
		bool bDispResult = DrawLogicLine(
			hdc,
			&sPos,
			nLayoutLineTo
		);

		if(bDispResult){
			pPs->rcPaint.bottom += nLineHeight;	// EOF�ĕ`��Ή�
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

	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	//     �A���_�[���C���`���������DC����̃R�s�[�O���������Ɉړ�
	if ( m_pcEditWnd->m_nActivePaneIndex == m_nMyIndex ){
		/* �A�N�e�B�u�y�C���́A�A���_�[���C���`�� */
		GetCaret().m_cUnderLine.CaretUnderLineON( TRUE );
	}
	// To Here 2007.09.09 Moca

	/* 03/02/18 �Ί��ʂ̋����\��(�`��) ai */
	DrawBracketPair( true );

	/* �L�����b�g�����݈ʒu�ɕ\�����܂� */
	if( bCaretShowFlag_Old )	// 2008.06.09 ryoji
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
	1���1���W�b�N�s������悷��B

	@return EOF����悵����true
*/
bool CEditView::DrawLogicLine(
	HDC				_hdc,			//!< [in]     ���Ώ�
	DispPos*		_pDispPos,		//!< [in/out] �`�悷��ӏ��A�`�挳�\�[�X
	CLayoutInt		nLineTo			//!< [in]     ���I�����郌�C�A�E�g�s�ԍ�
)
{
//	MY_RUNNINGTIMER( cRunningTimer, "CEditView::DrawLogicLine" );
	SDrawStrategyInfo sInfo;
	SDrawStrategyInfo* pInfo = &sInfo;
	pInfo->hdc = _hdc;
	pInfo->pDispPos = _pDispPos;
	pInfo->pcView = this;

	// �\�����J�n���郌�C�A�E�g
	const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );
//	if( !pcLayout ){
//		return true;
//	}


	//DispPos��ۑ����Ă���
	pInfo->sDispPosBegin = *pInfo->pDispPos;

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();


	//�T�C�Y
	STypeConfig* TypeDataPtr = &m_pcEditDoc->m_cDocType.GetDocumentAttribute();
	int nLineHeight = GetTextMetrics().GetHankakuDy();  //�s�̏c���H
	int nCharDx  = GetTextMetrics().GetHankakuDx();  //���p

	pInfo->bSearchStringMode = false; //���J�n //�����q�b�g�t���O�H����ŐF�����𔻒肷��
	pInfo->bSearchFlg        = true;  //���J�n //�H 2002.02.08 hor
	pInfo->nSearchStart      = CLogicInt(-1); //���J�n   //�H 2002.02.08 hor
	pInfo->nSearchEnd        = CLogicInt(-1); //���J�n   //�H 2002.02.08 hor

	//�������镶���ʒu
	pInfo->nPos = CLogicInt(0); //���J�n
	#define SetNPos(N) pInfo->nPos=(N)
	#define GetNPos() (pInfo->nPos+CLogicInt(0))

	//�ʏ�e�L�X�g�J�n�ʒu (�قڌŒ�)
	pInfo->nBgn = 0; //���J�n
	#define SetNBgn(N) pInfo->nBgn=(N)
	#define GetNBgn() (pInfo->nBgn+0)

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          �_���s�f�[�^�̎擾 -> pLine, pLineLen              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if( pcLayout ){
		// 2002/2/10 aroka CMemory�ύX
		pInfo->nLineLen = pcLayout->GetDocLineRef()->GetLengthWithEOL() - pcLayout->GetLogicOffset();
		pInfo->pLine    = pcLayout->GetDocLineRef()->GetPtr() + pcLayout->GetLogicOffset();
	}
	else{
		pInfo->pLine = NULL;
		pInfo->nLineLen = CLogicInt(0);
	}

	// �O�s�̍ŏI�ݒ�F
	pInfo->ChangeColor(pcLayout?pcLayout->GetColorTypePrev():COLORIDX_TEXT);
	pInfo->nCOMMENTEND = 0; //���J�n

	//�T�|�[�g
	CTypeSupport cTextType(this,COLORIDX_TEXT);
	CTypeSupport cSearchType(this,COLORIDX_SEARCH);

	//��ԕϐ�
	bool bEOF = false;
	pInfo->bKeyWordTop = true;	//���J�n	//	Keyword Top
	pInfo->nCharChars = CLogicInt(0); //���J�n	//��������������

	if( pInfo->pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		pInfo->pDispPos->ForwardDrawLine(-1);
		pInfo->pDispPos->ForwardLayoutLineRef(-1);

		while( GetNPos() < pInfo->nLineLen ){
			pInfo->pDispPos->ForwardDrawLine(1);
			pInfo->pDispPos->ForwardLayoutLineRef(1);
			if( GetTextArea().GetBottomLine() < pInfo->pDispPos->GetLayoutLineRef() ){
				pInfo->pDispPos->SetLayoutLineRef(nLineTo + CLayoutInt(1));
				goto end_of_func;
			}
			if( nLineTo < pInfo->pDispPos->GetLayoutLineRef() ){
				goto end_of_func;
			}

			//���C�A�E�g�s��1�s�`��
			bool bDrawLayoutLine = DrawLayoutLine(pInfo);
			if(bDrawLayoutLine)goto end_of_line;
		}
		// end of while( GetNPos() < nLineLen )

		if( pInfo->pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){

			/* �e�L�X�g�\�� */
			GetTextDrawer().DispText(
				pInfo->hdc, pInfo->pDispPos, &pInfo->pLine[GetNBgn()], GetNPos() - GetNBgn() );

			/* EOF�L���̕\�� */
			if( pInfo->pDispPos->GetLayoutLineRef() + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount()
				&& pInfo->pDispPos->GetDrawCol() < nWrapKeta )
			{
				if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
					//	May 29, 2004 genta (advised by MIK) ���ʊ֐���
					GetTextDrawer().DispEOF(pInfo->hdc,pInfo->pDispPos);
				}
				bEOF = true;
			}

			if( !IsBkBitmap() ){
				// �s���w�i�`��
				RECT rcClip;
				if(GetTextArea().GenerateClipRectRight(&rcClip,*pInfo->pDispPos)){
					cTextType.FillBack(pInfo->hdc,rcClip);
				}
			}

			// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
			GetTextDrawer().DispVerticalLines(
				pInfo->hdc,
				pInfo->pDispPos->GetDrawPos().y,
				pInfo->pDispPos->GetDrawPos().y + nLineHeight,
				CLayoutInt(0),
				CLayoutInt(-1)
			);

			if( GetSelectionInfo().IsTextSelected() ){
				/* �e�L�X�g���] */
				DispTextSelected(
					pInfo->hdc,
					pInfo->pDispPos->GetLayoutLineRef(),
					CMyPoint(pInfo->sDispPosBegin.GetDrawPos().x, pInfo->pDispPos->GetDrawPos().y),
					pInfo->pDispPos->GetDrawCol()
				);
			}

		}
end_of_line:;
		pInfo->pDispPos->ForwardLayoutLineRef(1);
		pInfo->pDispPos->ForwardDrawLine(1);
	}
	// NULL == pLine�̏ꍇ
	else{
		if( pInfo->pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
			if(GetTextDrawer().DispEmptyLine(pInfo->hdc,pInfo->pDispPos)){
				bEOF=true;
			}
		}
		return true;
	}

end_of_func:;


	return bEOF;
}

/*!
	���C�A�E�g�s��1�s�`��
*/
bool CEditView::DrawLayoutLine(SDrawStrategyInfo* pInfo)
{
	CTypeSupport cTextType(this,COLORIDX_TEXT);
	CTypeSupport cSearchType(this,COLORIDX_SEARCH);

	// �R���t�B�O
	int nLineHeight = GetTextMetrics().GetHankakuDy();  //�s�̏c���H
	STypeConfig* TypeDataPtr = &m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	const CLayout*	pcLayout2; //���[�N�pCLayout�|�C���^
	pcLayout2 = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �s�ԍ��`��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if( pInfo->pDispPos->GetDrawPos().y >= GetTextArea().GetAreaTop() ){
		// �s�ԍ��\��
		pInfo->pcView->GetTextDrawer().DispLineNumber(
			pInfo->hdc,
			pcLayout2,
			(Int)pInfo->pDispPos->GetLayoutLineRef(),
			pInfo->pDispPos->GetDrawPos().y
		);
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �{���`��J�n                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pInfo->nBgn = pInfo->nPos;
	int nLineBgn = pInfo->nBgn;
	pInfo->pDispPos->ResetDrawCol();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                 �s��(�C���f���g)�w�i�`��                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if(pcLayout2 && pcLayout2->GetIndent()!=0)
	{
		RECT rcClip;
		if(GetTextArea().GenerateClipRect(&rcClip,*pInfo->pDispPos,(Int)pcLayout2->GetIndent())){
			cTextType.FillBack(pInfo->hdc,rcClip);
		}
		//�`��ʒu�i�߂�
		pInfo->pDispPos->ForwardDrawCol((Int)pcLayout2->GetIndent());
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                 �ʏ핶����ȊO�`�惋�[�v                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//�s�I�[�܂��͐܂�Ԃ��ɒB����܂Ń��[�v
	while( pInfo->nPos - nLineBgn < pcLayout2->GetLengthWithEOL() ){
		// �}�b�`������
		CDraw_Found().BeginColor(pInfo);

		//�s��
		if(CDraw_Line().EndColor(pInfo)){
			return true;
		}

SEARCH_START:;
		switch( pInfo->nCOMMENTMODE ){
		case COLORIDX_TEXT:
			//�F�̊J�n����
			if( CDraw_RegexKeyword().BeginColor(pInfo) ){ }			// ���K�\���L�[���[�h
			else if( CDraw_LineComment().BeginColor(pInfo) ){ }		// �s�R�����g
			else if( CDraw_BlockComment().BeginColor(pInfo) ){ }	// �u���b�N�R�����g
			else if( CDraw_BlockComment2().BeginColor(pInfo) ){ }	// �u���b�N�R�����g2
			else if( CDraw_SingleQuote().BeginColor(pInfo) ){ }		// �V���O���N�H�[�e�[�V����������
			else if( CDraw_DoubleQuote().BeginColor(pInfo) ){ }		// �_�u���N�H�[�e�[�V����������
			else if( CDraw_URL().BeginColor(pInfo) ){ }				// URL
			else if( CDraw_Numeric().BeginColor(pInfo) ){ }			// ���p����
			else if( CDraw_KeywordSet().BeginColor(pInfo) ){ }		// �L�[���[�h�Z�b�g
			
			//�P�ꔻ��
			if( IS_KEYWORD_CHAR( pInfo->pLine[pInfo->nPos] ))	pInfo->bKeyWordTop = false;
			else												pInfo->bKeyWordTop = true;

			break;

		//�F�̏I������
		case COLORIDX_URL:		// URL
		case COLORIDX_DIGIT:	// ���p���l
		case COLORIDX_KEYWORD1:	// �����L�[���[�h1-10
		case COLORIDX_KEYWORD2:
		case COLORIDX_KEYWORD3:
		case COLORIDX_KEYWORD4:
		case COLORIDX_KEYWORD5:
		case COLORIDX_KEYWORD6:
		case COLORIDX_KEYWORD7:
		case COLORIDX_KEYWORD8:
		case COLORIDX_KEYWORD9:
		case COLORIDX_KEYWORD10:
			if( CDraw_ColorEnd().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;
		case COLORIDX_CTRLCODE:	/* �R���g���[���R�[�h */ // 2002/03/13 novice
			if( CDraw_CtrlCode().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;

		case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */ // 2002/03/13 novice
			break;
		case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
			if( CDraw_BlockComment().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;
		case COLORIDX_BLOCK2:	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
			if( CDraw_BlockComment2().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;
		case COLORIDX_SSTRING:	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
			if( CDraw_SingleQuote().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;
		case COLORIDX_WSTRING:	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
			if( CDraw_DoubleQuote().EndColor(pInfo) ){
				goto SEARCH_START;
			}
			break;
		default:	//@@@ 2002.01.04 add start
			if( pInfo->nCOMMENTMODE >= 1000 && pInfo->nCOMMENTMODE <= 1099 ){	//���K�\���L�[���[�h1�`10
				if( CDraw_RegexKeyword().EndColor(pInfo) ){
					goto SEARCH_START;
				}
			}
			break;	//@@@ 2002.01.04 add end
		}

		//�^�u
		if( CDraw_Tab().BeginColor(pInfo) ){ }
		//�S�p�X�y�[�X
		else if( CDraw_ZenSpace().BeginColor(pInfo) ){ }
		//���p�󔒁i���p�X�y�[�X�j��\�� 2002.04.28 Add by KK 
		else if( CDraw_HanSpace().BeginColor(pInfo) ){ }
		//�R���g���[���R�[�h 2008.04.20 kobake
		else if( CDraw_CtrlCode().BeginColor(pInfo) ){ }
		//���̑�
		else{
			pInfo->nCharChars = CLogicInt(1);
		}
		pInfo->nPos += pInfo->nCharChars;
	}
	//end of while( pInfo->nPos - nLineBgn < pcLayout2->GetLength() )


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �{���`��                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if( pInfo->nPos >= pInfo->nLineLen ){
		return false; //����ŗǂ�
	}
	if( pInfo->DrawToHere() ){
		/* �s���w�i�`�� */
		RECT rcClip;
		if(pInfo->pcView->GetTextArea().GenerateClipRectRight(&rcClip,*pInfo->pDispPos)){
			cTextType.FillBack(pInfo->hdc,rcClip);
		}

		/* �܂�Ԃ��L����\������ */
		pInfo->pcView->GetTextDrawer().DispWrap(pInfo->hdc,pInfo->pDispPos);

		// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
		pInfo->pcView->GetTextDrawer().DispVerticalLines( pInfo->hdc, pInfo->pDispPos->GetDrawPos().y, pInfo->pDispPos->GetDrawPos().y + nLineHeight,  CLayoutInt(0), CLayoutInt(-1) );
		if( GetSelectionInfo().IsTextSelected() ){
			/* �e�L�X�g���] */
			DispTextSelected(
				pInfo->hdc,
				pInfo->pDispPos->GetLayoutLineRef(),
				CMyPoint(pInfo->sDispPosBegin.GetDrawPos().x, pInfo->pDispPos->GetDrawPos().y),
				pInfo->pDispPos->GetDrawCol()
			);
		}
	}
	return false;
}








/* �e�L�X�g���]

	@param hdc      
	@param nLineNum 
	@param x        
	@param y        
	@param nX       

	@note
	CCEditView::DrawLogicLine() �ł̍��(WM_PAINT)���ɁA1���C�A�E�g�s���܂Ƃ߂Ĕ��]�������邽�߂̊֐��B
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







// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ��ʃo�b�t�@                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*!
	��ʂ̌݊��r�b�g�}�b�v���쐬�܂��͍X�V����B
		�K�v�̖����Ƃ��͉������Ȃ��B
	
	@param cx �E�B���h�E�̍���
	@param cy �E�B���h�E�̕�
	@return true: �r�b�g�}�b�v�𗘗p�\ / false: �r�b�g�}�b�v�̍쐬�E�X�V�Ɏ��s

	@date 2007.09.09 Moca CEditView::OnSize���番���B
		�P���ɐ������邾�����������̂��A�d�l�ύX�ɏ]�����e�R�s�[��ǉ��B
		�T�C�Y�������Ƃ��͉������Ȃ��悤�ɕύX

	@par �݊�BMP�ɂ̓L�����b�g�E�J�[�\���ʒu���c���E�Ί��ʈȊO�̏���S�ď������ށB
		�I��͈͕ύX���̔��]�����́A��ʂƌ݊�BMP�̗�����ʁX�ɕύX����B
		�J�[�\���ʒu���c���ύX���ɂ́A�݊�BMP�����ʂɌ��̏��𕜋A�����Ă���B

*/
bool CEditView::CreateOrUpdateCompatibleBitmap( int cx, int cy )
{
	if( NULL == m_hdcCompatDC ){
		return false;
	}
	// �T�C�Y��64�̔{���Ő���
	int nBmpWidthNew  = ((cx + 63) & (0x7fffffff - 63));
	int nBmpHeightNew = ((cy + 63) & (0x7fffffff - 63));
	if( nBmpWidthNew != m_nCompatBMPWidth || nBmpHeightNew != m_nCompatBMPHeight ){
#ifdef _DEBUG
	MYTRACE( _T("CEditView::CreateOrUpdateCompatibleBitmap( %d, %d ): resized\n"), cx, cy );
#endif
		HDC	hdc = ::GetDC( GetHwnd() );
		HBITMAP hBitmapNew = NULL;
		if( m_hbmpCompatBMP ){
			// BMP�̍X�V
			HDC hdcTemp = ::CreateCompatibleDC( hdc );
			hBitmapNew = ::CreateCompatibleBitmap( hdc, nBmpWidthNew, nBmpHeightNew );
			if( hBitmapNew ){
				HBITMAP hBitmapOld = (HBITMAP)::SelectObject( hdcTemp, hBitmapNew );
				// �O�̉�ʓ��e���R�s�[����
				::BitBlt( hdcTemp, 0, 0,
					__min( nBmpWidthNew,m_nCompatBMPWidth ),
					__min( nBmpHeightNew, m_nCompatBMPHeight ),
					m_hdcCompatDC, 0, 0, SRCCOPY );
				::SelectObject( hdcTemp, hBitmapOld );
				::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
				::DeleteObject( m_hbmpCompatBMP );
			}
			::DeleteDC( hdcTemp );
		}else{
			// BMP�̐V�K�쐬
			hBitmapNew = ::CreateCompatibleBitmap( hdc, nBmpWidthNew, nBmpHeightNew );
		}
		if( hBitmapNew ){
			m_hbmpCompatBMP = hBitmapNew;
			m_nCompatBMPWidth = nBmpWidthNew;
			m_nCompatBMPHeight = nBmpHeightNew;
			m_hbmpCompatBMPOld = (HBITMAP)::SelectObject( m_hdcCompatDC, m_hbmpCompatBMP );
		}else{
			// �݊�BMP�̍쐬�Ɏ��s
			// ��������s���J��Ԃ��\���������̂�
			// m_hdcCompatDC��NULL�ɂ��邱�Ƃŉ�ʃo�b�t�@�@�\�����̃E�B���h�E�̂ݖ����ɂ���B
			//	2007.09.29 genta �֐����D������BMP�����
			UseCompatibleDC(FALSE);
		}
		::ReleaseDC( GetHwnd(), hdc );
	}
	return NULL != m_hbmpCompatBMP;
}


/*!
	�݊�������BMP���폜

	@note �����r���[����\���ɂȂ����ꍇ��
		�e�E�B���h�E����\���E�ŏ������ꂽ�ꍇ�ɍ폜�����B
	@date 2007.09.09 Moca �V�K�쐬 
*/
void CEditView::DeleteCompatibleBitmap()
{
	if( m_hbmpCompatBMP ){
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
		::DeleteObject( m_hbmpCompatBMP );
		m_hbmpCompatBMP = NULL;
		m_hbmpCompatBMPOld = NULL;
		m_nCompatBMPWidth = -1;
		m_nCompatBMPHeight = -1;
	}
}



/** ��ʃL���b�V���pCompatibleDC��p�ӂ���

	@param[in] TRUE: ��ʃL���b�V��ON

	@date 2007.09.30 genta �֐���
*/
void CEditView::UseCompatibleDC(BOOL fCache)
{
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if( fCache ){
		if( m_hdcCompatDC == NULL ){
			HDC			hdc;
			hdc = ::GetDC( GetHwnd() );
			m_hdcCompatDC = ::CreateCompatibleDC( hdc );
			::ReleaseDC( GetHwnd(), hdc );
			DEBUG_TRACE(_T("CEditView::UseCompatibleDC: Created\n"), fCache);
		}
		else {
			DEBUG_TRACE(_T("CEditView::UseCompatibleDC: Reused\n"), fCache);
		}
	}
	else {
		//	CompatibleBitmap���c���Ă��邩������Ȃ��̂ōŏ��ɍ폜
		DeleteCompatibleBitmap();
		if( m_hdcCompatDC != NULL ){
			::DeleteDC( m_hdcCompatDC );
			DEBUG_TRACE(_T("CEditView::UseCompatibleDC: Deleted.\n"));
			m_hdcCompatDC = NULL;
		}
	}
}
