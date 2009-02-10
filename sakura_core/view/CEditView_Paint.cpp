#include "stdafx.h"
#include "CEditView_Paint.h"
#include "view/CEditView.h"
#include <vector>
#include "types/CTypeSupport.h"
#include "doc/CLayout.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "view/colors/CColorStrategy.h"
#include <memory> //auto_ptr
#include "view/colors/CColor_Found.h"
#include "view/figures/CFigureStrategy.h"

void _DispWrap(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);

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
EColorIndexType CEditView::GetColorIndex(
	const CLayout*			pcLayout,
	int						nIndex
)
{
	EColorIndexType eRet = COLORIDX_TEXT;

	if(!pcLayout){
		return COLORIDX_TEXT;
	}

	//	May 9, 2000 genta
	STypeConfig	*TypeDataPtr = &(m_pcEditDoc->m_cDocType.GetDocumentAttribute());

	/* �_���s�f�[�^�̎擾 */
	DispPos _sPos(0,0);
	SColorStrategyInfo _sInfo;
	SColorStrategyInfo* pInfo = &_sInfo;
	pInfo->pcView = this;
	pInfo->pDispPos=&_sPos;
	{
		// 2002/2/10 aroka CMemory�ύX
		pInfo->pLineOfLogic = pcLayout->GetDocLineRef()->GetPtr();

		// �_���s�̍ŏ��̃��C�A�E�g�����擾 -> pcLayoutLineFirst
		const CLayout* pcLayoutLineFirst = pcLayout;
		while( 0 != pcLayoutLineFirst->GetLogicOffset() ){
			pcLayoutLineFirst = pcLayoutLineFirst->GetPrevLayout();
		}

		// 2005.11.20 Moca �F���������Ȃ����Ƃ�������ɑΏ�
		eRet = pcLayoutLineFirst->GetColorTypePrev();	/* ���݂̐F���w�� */	// 02/12/18 ai
		pInfo->nPosInLogic = pcLayoutLineFirst->GetLogicOffset();


		// 2009.02.07 ryoji ���̊֐��ł� pInfo->DoChangeColor() �ŐF�𒲂ׂ邾���Ȃ̂ňȉ��̏����͕s�v
		//
		////############�����B�{����Visitor���g���ׂ�
		//class TmpVisitor{
		//public:
		//	static int CalcLayoutIndex(const CLayout* pcLayout)
		//	{
		//		int n = -1;
		//		while(pcLayout){
		//			pcLayout = pcLayout->GetPrevLayout(); //prev or null
		//			n++;
		//		}
		//		return n;
		//	}
		//};
		//pInfo->pDispPos->SetLayoutLineRef(CLayoutInt(TmpVisitor::CalcLayoutIndex(pcLayout)));
	}

	//@@@ 2001.11.17 add start MIK
	if( TypeDataPtr->m_bUseRegexKeyword )
	{
		m_cRegexKeyword->RegexKeyLineStart();
	}
	//@@@ 2001.11.17 add end MIK

	//������Q��
	const CDocLine* pcDocLine = pcLayout->GetDocLineRef();
	CStringRef cLineStr(pcDocLine->GetPtr(),pcDocLine->GetLengthWithEOL());

	//color strategy
	CColorStrategyPool* pool = CColorStrategyPool::Instance();
	pInfo->pStrategy = pool->GetStrategyByColor(eRet);
	if(pInfo->pStrategy)pInfo->pStrategy->InitStrategyStatus();

	int nPosTo = pcLayout->GetLogicOffset() + __min(nIndex, pcLayout->GetLengthWithEOL() - 1);
	while(pInfo->nPosInLogic <= nPosTo){
		//�F�ؑ�
		pInfo->DoChangeColor(cLineStr);

		//1�����i��
		pInfo->nPosInLogic += CNativeW::GetSizeOfChar(
									cLineStr.GetPtr(),
									cLineStr.GetLength(),
									pInfo->nPosInLogic
								);
	}
	eRet = pInfo->GetCurrentColor();

	return eRet;
}


/* ���݂̐F���w�� */
void CEditView::SetCurrentColor( CGraphics& gr, EColorIndexType eColorIndex )
{
	//�C���f�b�N�X����
	int		nColorIdx = ToColorInfoArrIndex(eColorIndex);

	//���ۂɐF��ݒ�
	if( -1 != nColorIdx ){
		const ColorInfo& info = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx];
		if( info.m_bDisp ){
			gr.SetForegroundColor(info.m_colTEXT);
			gr.SetBackgroundColor(info.m_colBACK);
			gr.SetMyFont(
				GetFontset().ChooseFontHandle(
					info.m_bFatFont,
					info.m_bUnderLine
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
void CEditView::OnPaint( HDC _hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp )
{
//	MY_RUNNINGTIMER( cRunningTimer, "CEditView::OnPaint" );
	CGraphics gr(_hdc);

	// 2004.01.28 Moca �f�X�N�g�b�v�ɍ�悵�Ȃ��悤��
	if( NULL == gr )return;

	if( !GetDrawSwitch() )return;
	//@@@
#if 0
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
			gr,
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
		hdcOld = gr;
		gr = m_hdcCompatDC;
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

		cTextType.FillBack(gr,rc);
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
		::FillRect( gr, &rc, hBrush );
		::DeleteObject( hBrush );
	}
	//	To Here Sep. 7, 2001 genta

	::SetBkMode( gr, TRANSPARENT );

	cTextType.SetGraphicsState_WhileThisObj(gr);


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
	// �_���s�̐擪�܂Ŗ߂�Ȃ��Ɗm���ɂ͐��m�ȐF�͓����Ȃ�
	// �i���K�\���L�[���[�h�Ƀ}�b�`�������������\�������̈ʒu�̃��C�A�E�g�s�����܂����ł���ꍇ�Ȃǁj
	//while( nRollBackLineNum < nMaxRollBackLineNum ){
	while(1){
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
			gr,
			&sPos,
			nLayoutLineTo
		);

		if(bDispResult){
			pPs->rcPaint.bottom += nLineHeight;	// EOF�ĕ`��Ή�
			break;
		}

	}

	gr.ClearMyFont();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              �e�L�X�g�̖��������̓h��Ԃ�                 //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if( IsBkBitmap() ){
	}
	else{
		/* �e�L�X�g�̂Ȃ�������w�i�F�œh��Ԃ� */
		if( sPos.GetDrawPos().y < pPs->rcPaint.bottom ){
			RECT rcBack;
			rcBack.left   = pPs->rcPaint.left;
			rcBack.right  = pPs->rcPaint.right;
			rcBack.top    = sPos.GetDrawPos().y;
			rcBack.bottom = pPs->rcPaint.bottom;

			cTextType.FillBack(gr,rcBack);

			// 2006.04.29 �s�����͍s���Ƃɍ�悵�A�����ł͏c���̎c������
			GetTextDrawer().DispVerticalLines( gr, sPos.GetDrawPos().y, pPs->rcPaint.bottom, CLayoutInt(0), CLayoutInt(-1) );
		}
	}

	cTextType.RewindGraphicsState(gr);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                �܂�Ԃ��ʒu�̕\�� (�c��)                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CTypeSupport cWrapType(this,COLORIDX_WRAP);
	if( cWrapType.IsDisp() ){
		int nXPos = GetTextArea().GetAreaLeft() + (Int)( nWrapKeta - GetTextArea().GetViewLeftCol() ) * nCharDx;
		//	2005.11.08 Moca �������ύX
		if( GetTextArea().GetAreaLeft() < nXPos && nXPos < GetTextArea().GetAreaRight() ){
			/// �܂�Ԃ��L���̐F�̃y����ݒ�
			gr.PushPen(cWrapType.GetTextColor(),0);

			::MoveToEx( gr, nXPos, GetTextArea().GetAreaTop(), NULL );
			::LineTo( gr, nXPos, GetTextArea().GetAreaBottom() );

			gr.PopPen();
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ���[���[�`��                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if ( pPs->rcPaint.top < GetTextArea().GetRulerHeight() ) { // ���[���[���ĕ`��͈͂ɂ���Ƃ��̂ݍĕ`�悷�� 2002.02.25 Add By KK
		GetRuler().SetRedrawFlag(); //2002.02.25 Add By KK ���[���[�S�̂�`��B
		GetRuler().DispRuler( gr );
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
			gr,
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
	SColorStrategyInfo _sInfo;
	SColorStrategyInfo* pInfo = &_sInfo;
	pInfo->gr.Init(_hdc);
	pInfo->pDispPos = _pDispPos;
	pInfo->pcView = this;

//	if( !pcLayout ){
//		return true;
//	}

	//CColorStrategyPool������
	CColorStrategyPool* pool = CColorStrategyPool::Instance();
	pool->NotifyOnStartScanLogic();

	//DispPos��ۑ����Ă���
	pInfo->sDispPosBegin = *pInfo->pDispPos;

	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	const CLayoutInt nWrapKeta = m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();


	//�T�C�Y
	STypeConfig* TypeDataPtr = &m_pcEditDoc->m_cDocType.GetDocumentAttribute();
	int nLineHeight = GetTextMetrics().GetHankakuDy();  //�s�̏c���H
	int nCharDx  = GetTextMetrics().GetHankakuDx();  //���p

	//�������镶���ʒu
	pInfo->nPosInLogic = CLogicInt(0); //���J�n

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          �_���s�f�[�^�̎擾 -> pLine, pLineLen              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �O�s�̍ŏI�ݒ�F
	{
		const CLayout* pcLayout = pInfo->pDispPos->GetLayoutRef();
		EColorIndexType eType = pcLayout? pcLayout->GetColorTypePrev(): COLORIDX_TEXT;
		CColorStrategyPool* pool = CColorStrategyPool::Instance();
		pInfo->pStrategy = pool->GetStrategyByColor(eType);
		if(pInfo->pStrategy)pInfo->pStrategy->InitStrategyStatus();
		pInfo->ChangeColor(eType);
	}

	//�J�n���W�b�N�ʒu���Z�o
	{
		const CLayout* pcLayout = pInfo->pDispPos->GetLayoutRef();
		pInfo->nPosInLogic = pcLayout?pcLayout->GetLogicOffset():CLogicInt(0);
	}

	//�T�|�[�g
	CTypeSupport cTextType(this,COLORIDX_TEXT);
	CTypeSupport cSearchType(this,COLORIDX_SEARCH);

	//���K�\���L�[���[�h���g����	//@@@ 2001.11.17 add MIK
	if( TypeDataPtr->m_bUseRegexKeyword ){
		m_cRegexKeyword->RegexKeyLineStart();
	}

	while(1){
		//�Ώۍs���`��͈͊O��������I��
		if( GetTextArea().GetBottomLine() < pInfo->pDispPos->GetLayoutLineRef() ){
			pInfo->pDispPos->SetLayoutLineRef(nLineTo + CLayoutInt(1));
			break;
		}
		if( nLineTo < pInfo->pDispPos->GetLayoutLineRef() ){
			break;
		}

		//���C�A�E�g�s��1�s�`��
		bool bDrawLayoutLine = DrawLayoutLine(pInfo);

		//�s��i�߂�
		CLogicInt nOldLogicLineNo = pInfo->pDispPos->GetLayoutRef()->GetLogicLineNo();
		pInfo->pDispPos->ForwardDrawLine(1);		//�`��Y���W�{�{
		pInfo->pDispPos->ForwardLayoutLineRef(1);	//���C�A�E�g�s�{�{

		// ���W�b�N�s��`�悵�I������甲����
		if(pInfo->pDispPos->GetLayoutRef()->GetLogicLineNo()!=nOldLogicLineNo){
			break;
		}

		// nLineTo�𒴂����甲����
		if(pInfo->pDispPos->GetLayoutLineRef() >= nLineTo + CLayoutInt(1)){
			break;
		}
	}

	return pInfo->pLineOfLogic==NULL;
}

/*!
	���C�A�E�g�s��1�s�`��
*/
//���s�L����`�悵���ꍇ��true��Ԃ��H
bool CEditView::DrawLayoutLine(SColorStrategyInfo* pInfo)
{
	CTypeSupport cTextType(this,COLORIDX_TEXT);
	CTypeSupport cSearchType(this,COLORIDX_SEARCH);

	const CLayout* pcLayout = pInfo->pDispPos->GetLayoutRef(); //m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );

	// ���C�A�E�g���
	if( pcLayout ){
		pInfo->pLineOfLogic					= pcLayout->GetDocLineRef()->GetPtr();
	}
	else{
		pInfo->pLineOfLogic					= NULL;
	}

	//������Q��
	const CDocLine* pcDocLine = pInfo->GetDocLine();
	CStringRef cLineStr = pcDocLine->GetStringRefWithEOL();

	//color strategy
	if(pcLayout && pcLayout->GetLogicOffset()==0){
		CColorStrategyPool* pool = CColorStrategyPool::Instance();
		pInfo->pStrategy = pool->GetStrategyByColor(pcLayout->GetColorTypePrev());
		if(pInfo->pStrategy)pInfo->pStrategy->InitStrategyStatus();
		pInfo->ChangeColor(pcLayout->GetColorTypePrev());
	}

	// �`��͈͊O�̏ꍇ�͐F�ؑւ����Ŕ�����
	if(pInfo->pDispPos->GetDrawPos().y < pInfo->pcView->GetTextArea().GetAreaTop()){
		if(pcLayout){
			while(pInfo->nPosInLogic < pcLayout->GetLogicOffset() + pcLayout->GetLengthWithEOL()){
				//�F�ؑ�
				pInfo->DoChangeColor(cLineStr);

				//1�����i��
				pInfo->nPosInLogic += CNativeW::GetSizeOfChar(
											cLineStr.GetPtr(),
											cLineStr.GetLength(),
											pInfo->nPosInLogic
										);
			}
		}
		return false;
	}

	// �R���t�B�O
	int nLineHeight = GetTextMetrics().GetHankakuDy();  //�s�̏c���H
	STypeConfig* TypeDataPtr = &m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �s�ԍ��`��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pInfo->pcView->GetTextDrawer().DispLineNumber(
		pInfo->gr,
		pInfo->pDispPos->GetLayoutLineRef(),
		pInfo->pDispPos->GetDrawPos().y
	);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �{���`��J�n                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pInfo->pDispPos->ResetDrawCol();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                 �s��(�C���f���g)�w�i�`��                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if(pcLayout && pcLayout->GetIndent()!=0)
	{
		RECT rcClip;
		if(GetTextArea().GenerateClipRect(&rcClip,*pInfo->pDispPos,(Int)pcLayout->GetIndent())){
			cTextType.FillBack(pInfo->gr,rcClip);
		}
		//�`��ʒu�i�߂�
		pInfo->pDispPos->ForwardDrawCol((Int)pcLayout->GetIndent());
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �{���`��                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//�s�I�[�܂��͐܂�Ԃ��ɒB����܂Ń��[�v
	if(pcLayout){
		while(pInfo->nPosInLogic < pcLayout->GetLogicOffset() + pcLayout->GetLengthWithEOL()){
			//�F�ؑ�
			pInfo->DoChangeColor(cLineStr);

			//1�������擾 $$�������\
			CFigure& cFigure = CFigureManager::Instance()->GetFigure(&cLineStr.GetPtr()[pInfo->GetPosInLogic()]);

			//1�����`��
			CLogicInt nPosOld = pInfo->nPosInLogic;
			cFigure.DrawImp(pInfo);
			if(pInfo->nPosInLogic == nPosOld){
				pInfo->nPosInLogic += CNativeW::GetSizeOfChar(
											cLineStr.GetPtr(),
											cLineStr.GetLength(),
											pInfo->nPosInLogic
										);
			}
		}
	}

	// �K�v�Ȃ�EOF�`��
	void _DispEOF( CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);
	if(pcLayout && pcLayout->GetNextLayout()==NULL && pcLayout->GetLayoutEol().GetLen()==0){
		// �L�����s��EOF
		_DispEOF(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
	}
	else if(!pcLayout && pInfo->pDispPos->GetLayoutLineRef()==m_pcEditDoc->m_cLayoutMgr.GetLineCount()){
		// ��s��EOF
		CLayout* pBottom = m_pcEditDoc->m_cLayoutMgr.GetBottomLayout();
		if(pBottom==NULL || (pBottom && pBottom->GetLayoutEol().GetLen())){
			_DispEOF(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
		}
	}

	// �K�v�Ȃ�܂�Ԃ��L���`��
	if(pcLayout && pcLayout->GetLayoutEol().GetLen()==0 && pcLayout->GetNextLayout()!=NULL){
		_DispWrap(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
	}

	// �s���w�i�`��
	RECT rcClip;
	if(pInfo->pcView->GetTextArea().GenerateClipRectRight(&rcClip,*pInfo->pDispPos)){
		cTextType.FillBack(pInfo->gr,rcClip);
	}

	// ���]�`��
	if( pcLayout && pInfo->pcView->GetSelectionInfo().IsTextSelected() ){
		pInfo->pcView->DispTextSelected(
			pInfo->gr,
			pInfo->pDispPos->GetLayoutLineRef(),
			CMyPoint(pInfo->sDispPosBegin.GetDrawPos().x, pInfo->pDispPos->GetDrawPos().y),
			pcLayout->CalcLayoutWidth(CEditDoc::GetInstance(0)->m_cLayoutMgr) + CLayoutInt(pcLayout->GetLayoutEol().GetLen()?1:0)
		);
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
