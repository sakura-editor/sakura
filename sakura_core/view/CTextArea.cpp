#include "stdafx.h"
#include "CTextArea.h"
#include "env/CShareData.h"
#include "CViewFont.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "view/colors/CColorStrategy.h"

CTextArea::CTextArea(CEditView* pEditView)
: m_pEditView(pEditView)
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();

	m_nViewAlignLeft = 0;		/* �\����̍��[���W */
	m_nViewAlignLeftCols = 0;	/* �s�ԍ���̌��� */
	m_nViewCx = 0;				/* �\����̕� */
	m_nViewCy = 0;				/* �\����̍��� */
	m_nViewColNum = CLayoutInt(0);			/* �\����̌��� */
	m_nViewRowNum = CLayoutInt(0);			/* �\����̍s�� */
	m_nViewTopLine = CLayoutInt(0);			/* �\����̈�ԏ�̍s */
	m_nViewLeftCol = CLayoutInt(0);			/* �\����̈�ԍ��̌� */
	SetTopYohaku( pShareData->m_Common.m_sWindow.m_nRulerBottomSpace ); 	/* ���[���[�ƃe�L�X�g�̌��� */
	m_nViewAlignTop = GetTopYohaku();		/* �\����̏�[���W */
}

CTextArea::~CTextArea()
{
}

void CTextArea::CopyTextAreaStatus(CTextArea* pDst) const
{
	pDst->SetAreaLeft				( this->GetAreaLeft() );		// �\����̍��[���W
	pDst->m_nViewAlignLeftCols		= this->m_nViewAlignLeftCols;	// �s�ԍ���̌���
	pDst->SetAreaTop				(this->GetAreaTop());			// �\����̏�[���W
//	pDst->m_nViewCx					= m_nViewCx;					// �\����̕�
//	pDst->m_nViewCy					= m_nViewCy;					// �\����̍���
//	pDst->m_nViewColNum				= this->m_nViewColNum;			// �\����̌���
//	pDst->m_nViewRowNum				= this->m_nViewRowNum;			// �\����̍s��
	pDst->SetViewTopLine			( this->GetViewTopLine() );		// �\����̈�ԏ�̍s(0�J�n)
	pDst->SetViewLeftCol			( this->GetViewLeftCol() );		// �\����̈�ԍ��̌�(0�J�n)
}


//!�t�H���g�ύX�̍ہA�e��p�����[�^���v�Z������
void CTextArea::UpdateAreaMetrics(HDC hdc)
{
	CEditView* pView=m_pEditView;

	//�\����̍Čv�Z
	m_nViewColNum = CLayoutInt((m_nViewCx - 1) / pView->GetTextMetrics().GetHankakuDx());	// �\����̌���
	m_nViewRowNum = CLayoutInt((m_nViewCy - 1) / pView->GetTextMetrics().GetHankakuDy());	// �\����̍s��

	// �����Ԋu
	pView->GetTextMetrics().SetHankakuDx( pView->GetTextMetrics().GetHankakuWidth() + pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nColmSpace );

	// �s�Ԋu
	pView->GetTextMetrics().SetHankakuDy( pView->GetTextMetrics().GetHankakuHeight() + pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nLineSpace );
}

void CTextArea::GenerateCharRect(RECT* rc,const DispPos& sPos,int nHankakuNum) const
{
	const CEditView* pView=m_pEditView;

	rc->left   = sPos.GetDrawPos().x;
	rc->right  = sPos.GetDrawPos().x + pView->GetTextMetrics().GetHankakuDx() * nHankakuNum;
	rc->top    = sPos.GetDrawPos().y;
	rc->bottom = sPos.GetDrawPos().y + pView->GetTextMetrics().GetHankakuDy();
}

bool CTextArea::TrimRectByArea(RECT* rc) const
{
	//���͂ݏo������
	if( rc->left < GetAreaLeft() ){
		rc->left = GetAreaLeft();
	}

	if(rc->left >= rc->right)return false; //���ƉE�����ׂ���
	if(rc->left >= GetAreaRight())return false; //��ʊO(�E)
	if(rc->right <= GetAreaLeft())return false; //��ʊO(��)

	//$ �����쓥�P�F��ʏ㉺�̂͂ݏo������͏ȗ�

	return true;
}

bool CTextArea::GenerateClipRect(RECT* rc,const DispPos& sPos,int nHankakuNum) const
{
	const CEditView* pView=m_pEditView;

	GenerateCharRect(rc,sPos,nHankakuNum);
	return TrimRectByArea(rc);
}

//!�E�̎c���\����`�𐶐�����
bool CTextArea::GenerateClipRectRight(RECT* rc,const DispPos& sPos) const
{
	const CEditView* pView=m_pEditView;

	rc->left   = sPos.GetDrawPos().x;
	rc->right  = GetAreaRight();
	rc->top    = sPos.GetDrawPos().y;
	rc->bottom = sPos.GetDrawPos().y + pView->GetTextMetrics().GetHankakuDy();

	//���͂ݏo������
	if( rc->left < GetAreaLeft() ){
		rc->left = GetAreaLeft();
	}

	if(rc->left >= rc->right)return false; //���ƉE�����ׂ���
	if(rc->left >= GetAreaRight())return false; //��ʊO(�E)
	if(rc->right <= GetAreaLeft())return false; //��ʊO(��)

	//$ �����쓥�P�F��ʏ㉺�̂͂ݏo������͏ȗ�

	return true;
}

bool CTextArea::GenerateClipRectLine(RECT* rc,const DispPos& sPos) const
{
	rc->left   = 0;
	rc->right  = GetAreaRight();
	rc->top    = sPos.GetDrawPos().y;
	rc->bottom = sPos.GetDrawPos().y + m_pEditView->GetTextMetrics().GetHankakuDy();
	return true;
}


/*
�s�ԍ��\���ɕK�v�ȕ���ݒ�B�����ύX���ꂽ�ꍇ��TRUE��Ԃ�
*/
bool CTextArea::DetectWidthOfLineNumberArea( bool bRedraw )
{
	const CEditView* pView=m_pEditView;
	CEditView* pView2=m_pEditView;

	int				nViewAlignLeftNew;

	if( pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_GYOU].m_bDisp ){
		/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
		int i = DetectWidthOfLineNumberArea_calculate();
		nViewAlignLeftNew = pView->GetTextMetrics().GetHankakuDx() * (i + 1);	/* �\����̍��[���W */
		m_nViewAlignLeftCols = i + 1;
	}
	else{
		nViewAlignLeftNew = 8;
		m_nViewAlignLeftCols = 0;
	}

	//	Sep 18, 2002 genta
	nViewAlignLeftNew += GetDllShareData().m_Common.m_sWindow.m_nLineNumRightSpace;
	if( nViewAlignLeftNew != GetAreaLeft() ){
		CMyRect			rc;
		SetAreaLeft(nViewAlignLeftNew);
		pView->GetClientRect( &rc );
		int nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL ); // �����X�N���[���o�[�̉���
		m_nViewCx = rc.Width() - nCxVScroll - GetAreaLeft(); // �\����̕�
		// 2008.05.27 nasukoji	�\����̌������Z�o����i�E�[�J�[�\���ړ����̕\���ꏊ����ւ̑Ώ��j
		m_nViewColNum = CLayoutInt((m_nViewCx - 1) / pView->GetTextMetrics().GetHankakuDx());	// �\����̌���

		if( bRedraw ){
			/* �ĕ`�� */
			pView2->GetCaret().m_cUnderLine.Lock();
			// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
			pView2->Call_OnPaint(PAINT_LINENUMBER | PAINT_RULER | PAINT_BODY, false); /* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
			// To Here 2007.09.09 Moca
			pView2->GetCaret().m_cUnderLine.UnLock();
			pView2->GetCaret().ShowEditCaret();
			/*
			PAINTSTRUCT		ps;
			HDC hdc = ::GetDC( pView->m_hWnd );
			ps.rcPaint.left   = 0;
			ps.rcPaint.right  = GetAreaRight();
			ps.rcPaint.top    = 0;
			ps.rcPaint.bottom = GetAreaBottom();
			pView2->GetCaret().m_cUnderLine.Lock();
			pView2->OnPaint( hdc, &ps, TRUE );	
			GetCaret().m_cUnderLine.UnLock();
			pView2->GetCaret().ShowEditCaret();
			::ReleaseDC( m_hWnd, hdc );
			*/
		}
		pView2->GetRuler().SetRedrawFlag();
		return true;
	}else{
		return false;
	}
}


/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
int CTextArea::DetectWidthOfLineNumberArea_calculate() const
{
	const CEditView* pView=m_pEditView;

	int nAllLines; //$$ �P�ʍ���

	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	if( pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bLineNumIsCRLF ){
		nAllLines = pView->m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	}
	else{
		nAllLines = (Int)pView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	}
	
	if( 0 < nAllLines ){
		int nWork = 100;
		int i;
		for( i = 3; i < 12; ++i ){
			if( nWork > nAllLines ){	// Oct. 18, 2003 genta ���𐮗�
				break;
			}
			nWork *= 10;
		}
		return i;
	}else{
		//	2003.09.11 wmlhq �s�ԍ���1���̂Ƃ��ƕ������킹��
		return 3;
	}
}

void CTextArea::TextArea_OnSize(
	const CMySize& sizeClient, //!< �E�B���h�E�̃N���C�A���g�T�C�Y
	int nCxVScroll,            //!< �����X�N���[���o�[�̉���
	int nCyHScroll             //!< �����X�N���[���o�[�̏c��
)
{
	const CEditView* pView = m_pEditView;

	m_nViewCx = sizeClient.cx - nCxVScroll - GetAreaLeft(); // �\����̕�
	m_nViewCy = sizeClient.cy - nCyHScroll - GetAreaTop();  // �\����̍���
	m_nViewColNum = CLayoutInt((m_nViewCx - 1) / pView->GetTextMetrics().GetHankakuDx());	// �\����̌���
	m_nViewRowNum = CLayoutInt((m_nViewCy - 1) / pView->GetTextMetrics().GetHankakuDy());	// �\����̍s��
}



int CTextArea::GetDocumentLeftClientPointX() const
{
	return GetAreaLeft() - (Int)GetViewLeftCol() * m_pEditView->GetTextMetrics().GetHankakuDx();
}

//! �N���C�A���g���W���烌�C�A�E�g�ʒu�ɕϊ�����
void CTextArea::ClientToLayout(CMyPoint ptClient, CLayoutPoint* pptLayout) const
{
	const CEditView* pView=m_pEditView;
	pptLayout->Set(
		GetViewLeftCol() + CLayoutInt( (ptClient.x - GetAreaLeft()) / pView->GetTextMetrics().GetHankakuDx() ),
		GetViewTopLine() + CLayoutInt( (ptClient.y - GetAreaTop()) / pView->GetTextMetrics().GetHankakuDy() )
	);
}


//! �s�ԍ��G���A���܂ޔ͈�
void CTextArea::GenerateTopRect   (RECT* rc, CLayoutInt nLineCount) const
{
	rc->left   = 0; //m_nViewAlignLeft;
	rc->right  = m_nViewAlignLeft + m_nViewCx;
	rc->top    = m_nViewAlignTop;
	rc->bottom = m_nViewAlignTop + (Int)nLineCount * m_pEditView->GetTextMetrics().GetHankakuDy();
}

//! �s�ԍ��G���A���܂ޔ͈�
void CTextArea::GenerateBottomRect(RECT* rc, CLayoutInt nLineCount) const
{
	rc->left   = 0; //m_nViewAlignLeft;
	rc->right  = m_nViewAlignLeft + m_nViewCx;
	rc->top    = m_nViewAlignTop  + m_nViewCy - (Int)nLineCount * m_pEditView->GetTextMetrics().GetHankakuDy();
	rc->bottom = m_nViewAlignTop  + m_nViewCy;
}

void CTextArea::GenerateLeftRect  (RECT* rc, CLayoutInt nColCount ) const
{
	rc->left   = m_nViewAlignLeft;
	rc->right  = m_nViewAlignLeft + (Int)nColCount * m_pEditView->GetTextMetrics().GetHankakuDx();
	rc->top    = m_nViewAlignTop;
	rc->bottom = m_nViewAlignTop + m_nViewCy;
}

void CTextArea::GenerateRightRect (RECT* rc, CLayoutInt nColCount ) const
{
	rc->left   = m_nViewAlignLeft + m_nViewCx - (Int)nColCount * m_pEditView->GetTextMetrics().GetHankakuDx(); //2008.01.26 kobake �������t�ɂȂ��Ă��̂��C��
	rc->right  = m_nViewAlignLeft + m_nViewCx;
	rc->top    = m_nViewAlignTop;
	rc->bottom = m_nViewAlignTop  + m_nViewCy;
}

void CTextArea::GenerateLineNumberRect(RECT* rc) const
{
	rc->left   = 0;
	rc->right  = m_nViewAlignLeft;
	rc->top    = 0;
	rc->bottom = m_nViewAlignTop + m_nViewCy;
}

void CTextArea::GenerateTextAreaRect(RECT* rc) const
{
	rc->left   = 0;
	rc->right  = m_nViewAlignLeft + m_nViewCx;
	rc->top    = m_nViewAlignTop;
	rc->bottom = m_nViewAlignTop + m_nViewCy;
}



