#include "stdafx.h"
#include "CRuler.h"
#include "CTextArea.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "types/CTypeSupport.h"
#include "view/colors/CColorStrategy.h"

CRuler::CRuler(const CEditView* pEditView, const CEditDoc* pEditDoc)
: m_pEditView(pEditView)
, m_pEditDoc(pEditDoc)
{
	m_nOldRulerDrawX = 0;	// �O��`�悵�����[���[�̃L�����b�g�ʒu 2002.02.25 Add By KK
	m_nOldRulerWidth = 0;	// �O��`�悵�����[���[�̃L�����b�g��   2002.02.25 Add By KK
}

CRuler::~CRuler()
{
}

//2007.08.26 kobake UNICODE�p��X�ʒu��ύX
void CRuler::_DrawRulerCaret( CGraphics& gr, int nCaretDrawPosX, int nCaretWidth )
{
	//�`��̈� -> hRgn
	RECT rc;
	rc.left = nCaretDrawPosX; //m_pEditView->GetTextArea().GetAreaLeft() + ( cCaretPosTmp.GetX() - m_pEditView->GetTextArea().GetViewLeftCol() ) * m_pEditView->GetTextMetrics().GetHankakuDx() + 1;
	rc.right = rc.left + m_pEditView->GetTextMetrics().GetHankakuDx() - 1;
	rc.top = 0;
	rc.bottom = m_pEditView->GetTextArea().GetAreaTop() - m_pEditView->GetTextArea().GetTopYohaku() - 1;
	HRGN hRgn = ::CreateRectRgnIndirect( &rc );

	//�u���V�쐬 -> hBrush
	HBRUSH hBrush;
	if( 0 == nCaretWidth ){
		hBrush = ::CreateSolidBrush( RGB( 128, 128, 128 ) );
	}else{
		hBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
	}

	//�̈��`�� (�F�𔽓]������)
	int    nROP_Old  = ::SetROP2( gr, R2_NOTXORPEN );
	HBRUSH hBrushOld = (HBRUSH)::SelectObject( gr, hBrush );
	::SelectObject( gr, hBrush );
	::PaintRgn( gr, hRgn );
	::SelectObject( gr, hBrushOld );
	::SetROP2( gr, nROP_Old );

	//�`��I�u�W�F�N�g�j��
	::DeleteObject( hRgn );
	::DeleteObject( hBrush );
}

/*! 
	���[���[�̃L�����b�g���ĕ`��	2002.02.25 Add By KK
	@param hdc [in] �f�o�C�X�R���e�L�X�g
	DispRuler�̓��e�����ɍ쐬
*/
void CRuler::DrawRulerCaret( CGraphics& gr )
{
	if( m_pEditView->GetTextArea().GetViewLeftCol() <= m_pEditView->GetCaret().GetCaretLayoutPos().GetX()
	 && m_pEditView->GetTextArea().GetRightCol() + 2 >= m_pEditView->GetCaret().GetCaretLayoutPos().GetX()
	){
		if (m_pEditView->GetRuler().m_nOldRulerDrawX == m_pEditView->GetCaret().CalcCaretDrawPos(m_pEditView->GetCaret().GetCaretLayoutPos()).x
			&& m_pEditView->GetCaret().GetCaretSize().cx == m_pEditView->GetRuler().m_nOldRulerWidth) {
			//�O�`�悵���ʒu�擯�� ���� ���[���[�̃L�����b�g�������� 
			return;
		}

		//���ʒu���N���A m_nOldRulerWidth
		this->_DrawRulerCaret( gr, m_nOldRulerDrawX, m_nOldRulerWidth );

		//�V�����ʒu�ŕ`��   2007.08.26 kobake UNICODE�p��X�ʒu��ύX
		this->_DrawRulerCaret(
			gr,
			m_pEditView->GetCaret().CalcCaretDrawPos(m_pEditView->GetCaret().GetCaretLayoutPos()).x,
			m_pEditView->GetCaret().GetCaretSize().cx
		);
	}
}

//! ���[���[�̔w�i�̂ݕ`�� 2007.08.29 kobake �ǉ�
void CRuler::DrawRulerBg(CGraphics& gr)
{
	//�K�v�ȃC���^�[�t�F�[�X
	CommonSetting* pCommon=&GetDllShareData().m_Common;

	//�T�|�[�g
	CTypeSupport cRulerType(m_pEditView,COLORIDX_RULER);

	// �t�H���g�ݒ� (���[���[��̐����p)
	LOGFONT	lf;
	HFONT		hFont;
	HFONT		hFontOld;
	memset_raw( &lf, 0, sizeof(lf) );
	lf.lfHeight			= 1 - pCommon->m_sWindow.m_nRulerHeight;	//	2002/05/13 ai
	lf.lfWidth			= 5;
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
	_tcscpy( lf.lfFaceName, _T("Arial") );
	hFont = ::CreateFontIndirect( &lf );
	hFontOld = (HFONT)::SelectObject( gr, hFont );
	::SetBkMode( gr, TRANSPARENT );

	//�w�i�h��Ԃ�
	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = m_pEditView->GetTextArea().GetAreaRight();
	rc.bottom = m_pEditView->GetTextArea().GetAreaTop() - m_pEditView->GetTextArea().GetTopYohaku();
	cRulerType.FillBack(gr,rc);

	//���[���[�F�ݒ�
	gr.PushPen(cRulerType.GetTextColor(),0);
	gr.PushTextForeColor(cRulerType.GetTextColor());


	//�`��J�n�ʒu
	int nX = m_pEditView->GetTextArea().GetAreaLeft();
	int nY = m_pEditView->GetTextArea().GetRulerHeight() - 2;


	// ���� (���[���[�Ɩ{���̋��E)
	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	//	2005.11.10 Moca 1dot����Ȃ�
	CLayoutInt	nMaxLineKetas = m_pEditDoc->m_cLayoutMgr.GetMaxLineKetas();
	int nToX = m_pEditView->GetTextArea().GetAreaLeft() + (Int)(nMaxLineKetas - m_pEditView->GetTextArea().GetViewLeftCol()) * m_pEditView->GetTextMetrics().GetHankakuDx() + 1;
	if( nToX > m_pEditView->GetTextArea().GetAreaRight() ){
		nToX = m_pEditView->GetTextArea().GetAreaRight();
	}
	::MoveToEx( gr, m_pEditView->GetTextArea().GetAreaLeft(), nY + 1, NULL );
	::LineTo( gr, nToX, nY + 1 );


	//�ڐ���`��
	CLayoutInt i = m_pEditView->GetTextArea().GetViewLeftCol();
	while(i <= m_pEditView->GetTextArea().GetRightCol() + 1 && i <= nMaxLineKetas)
	{
		//���[���[�I�[�̋�؂�(��)
		if( i == nMaxLineKetas ){
			::MoveToEx( gr, nX, nY, NULL );
			::LineTo( gr, nX, 0 );
		}
		//10�ڐ������̋�؂�(��)�Ɛ���
		else if( 0 == i % 10 ){
			wchar_t szColm[32];
			::MoveToEx( gr, nX, nY, NULL );
			::LineTo( gr, nX, 0 );
			_itow( ((Int)i) / 10, szColm, 10 );
			::TextOutW_AnyBuild( gr, nX + 2 + 0, -1 + 0, szColm, wcslen( szColm ) );
		}
		//5�ڐ������̋�؂�(��)
		else if( 0 == i % 5 ){
			::MoveToEx( gr, nX, nY, NULL );
			::LineTo( gr, nX, nY - 6 );
		}
		//���ڐ��̋�؂�(��)
		else{
			::MoveToEx( gr, nX, nY, NULL );
			::LineTo( gr, nX, nY - 3 );
		}

		nX += m_pEditView->GetTextMetrics().GetHankakuDx();
		i++;
	}

	//�F�߂�
	gr.PopTextForeColor();
	gr.PopPen();

	//�t�H���g�߂�
	::SelectObject( gr, hFontOld );
	::DeleteObject( hFont );
}

/*! ���[���[�`��

	@date 2005.08.14 genta �܂�Ԃ�����LayoutMgr����擾����悤��
*/
void CRuler::DispRuler( HDC hdc )
{
	//�K�v�ȃC���^�[�t�F�[�X
	const CommonSetting* pCommon=&GetDllShareData().m_Common;

	//�T�|�[�g
	CTypeSupport cRulerType(m_pEditView,COLORIDX_RULER);

	if( !m_pEditView->GetDrawSwitch() ){
		return;
	}
	if( !cRulerType.IsDisp() ){
		return;
	}

	// �`��Ώ�
	CGraphics gr(hdc);

	// 2002.02.25 Add By KK ���[���[�S�̂�`�������K�v���Ȃ��ꍇ�́A���[����̃L�����b�g�̂ݕ`���Ȃ��� 
	if ( !m_bRedrawRuler ) {
		DrawRulerCaret( gr );
	}
	else {
		// �w�i�`��
		DrawRulerBg(gr);

		// �L�����b�g�`��
		if( m_pEditView->GetTextArea().GetViewLeftCol() <= m_pEditView->GetCaret().GetCaretLayoutPos().GetX()
		 && m_pEditView->GetTextArea().GetRightCol() + 2 >= m_pEditView->GetCaret().GetCaretLayoutPos().GetX()
		){
			_DrawRulerCaret(gr,m_pEditView->GetCaret().CalcCaretDrawPos(m_pEditView->GetCaret().GetCaretLayoutPos()).x,m_pEditView->GetCaret().GetCaretSize().cx);
		}

		m_bRedrawRuler = false;	//m_bRedrawRuler = true �Ŏw�肳���܂ŁA���[���̃L�����b�g�݂̂��ĕ`�� 2002.02.25 Add By KK
	}

	//�`�悵�����[���[�̃L�����b�g�ʒu�E����ۑ� 2002.02.25 Add By KK
	m_nOldRulerDrawX = m_pEditView->GetCaret().CalcCaretDrawPos(m_pEditView->GetCaret().GetCaretLayoutPos()).x;
	m_nOldRulerWidth = m_pEditView->GetCaret().GetCaretSize().cx ;
}

