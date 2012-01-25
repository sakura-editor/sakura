#include "StdAfx.h"
#include "CTextArea.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "CViewFont.h"
#include "view/CEditView.h"
#include "view/colors/CColorStrategy.h"
#include "doc/CEditDoc.h"

CTextArea::CTextArea(CEditView* pEditView)
: m_pEditView(pEditView)
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();

	m_nViewAlignLeft = 0;		/* 表示域の左端座標 */
	m_nViewAlignLeftCols = 0;	/* 行番号域の桁数 */
	m_nViewCx = 0;				/* 表示域の幅 */
	m_nViewCy = 0;				/* 表示域の高さ */
	m_nViewColNum = CLayoutInt(0);			/* 表示域の桁数 */
	m_nViewRowNum = CLayoutInt(0);			/* 表示域の行数 */
	m_nViewTopLine = CLayoutInt(0);			/* 表示域の一番上の行 */
	m_nViewLeftCol = CLayoutInt(0);			/* 表示域の一番左の桁 */
	SetTopYohaku( pShareData->m_Common.m_sWindow.m_nRulerBottomSpace ); 	/* ルーラーとテキストの隙間 */
	SetLeftYohaku( pShareData->m_Common.m_sWindow.m_nLineNumRightSpace );
	m_nViewAlignTop = GetTopYohaku();		/* 表示域の上端座標 */
}

CTextArea::~CTextArea()
{
}

void CTextArea::CopyTextAreaStatus(CTextArea* pDst) const
{
	pDst->SetAreaLeft				( this->GetAreaLeft() );		// 表示域の左端座標
	pDst->m_nViewAlignLeftCols		= this->m_nViewAlignLeftCols;	// 行番号域の桁数
	pDst->SetAreaTop				(this->GetAreaTop());			// 表示域の上端座標
//	pDst->m_nViewCx					= m_nViewCx;					// 表示域の幅
//	pDst->m_nViewCy					= m_nViewCy;					// 表示域の高さ
//	pDst->m_nViewColNum				= this->m_nViewColNum;			// 表示域の桁数
//	pDst->m_nViewRowNum				= this->m_nViewRowNum;			// 表示域の行数
	pDst->SetViewTopLine			( this->GetViewTopLine() );		// 表示域の一番上の行(0開始)
	pDst->SetViewLeftCol			( this->GetViewLeftCol() );		// 表示域の一番左の桁(0開始)
}

//!表示域の再計算
void CTextArea::UpdateViewColRowNums()
{
	CEditView* pView=m_pEditView;
	// Note: マイナスの割り算は処理系依存です。
	// 0だとカーソルを設定できない・選択できないなど動作不良になるので1以上にする
	m_nViewColNum = CLayoutInt(std::max(1, std::max(0, m_nViewCx - 1) / pView->GetTextMetrics().GetHankakuDx()));	// 表示域の桁数
	m_nViewRowNum = CLayoutInt(std::max(1, std::max(0, m_nViewCy - 1) / pView->GetTextMetrics().GetHankakuDy()));	// 表示域の行数
}

//!フォント変更の際、各種パラメータを計算し直す
void CTextArea::UpdateAreaMetrics(HDC hdc)
{
	CEditView* pView=m_pEditView;

	// 文字間隔
	pView->GetTextMetrics().SetHankakuDx( pView->GetTextMetrics().GetHankakuWidth() + pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nColmSpace );

	// 行間隔
	pView->GetTextMetrics().SetHankakuDy( pView->GetTextMetrics().GetHankakuHeight() + pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nLineSpace );

	//表示域の再計算
	//2010.08.24 Dx/Dyを使うので後で設定
	UpdateViewColRowNums();
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
	//左はみ出し調整
	if( rc->left < GetAreaLeft() ){
		rc->left = GetAreaLeft();
	}

	if(rc->left >= rc->right)return false; //左と右があべこべ
	if(rc->left >= GetAreaRight())return false; //画面外(右)
	if(rc->right <= GetAreaLeft())return false; //画面外(左)

	//$ 元動作踏襲：画面上下のはみ出し判定は省略

	return true;
}

bool CTextArea::GenerateClipRect(RECT* rc,const DispPos& sPos,int nHankakuNum) const
{
	const CEditView* pView=m_pEditView;

	GenerateCharRect(rc,sPos,nHankakuNum);
	return TrimRectByArea(rc);
}

//!右の残りを表す矩形を生成する
bool CTextArea::GenerateClipRectRight(RECT* rc,const DispPos& sPos) const
{
	const CEditView* pView=m_pEditView;

	rc->left   = sPos.GetDrawPos().x;
	rc->right  = GetAreaRight();
	rc->top    = sPos.GetDrawPos().y;
	rc->bottom = sPos.GetDrawPos().y + pView->GetTextMetrics().GetHankakuDy();

	//左はみ出し調整
	if( rc->left < GetAreaLeft() ){
		rc->left = GetAreaLeft();
	}

	if(rc->left >= rc->right)return false; //左と右があべこべ
	if(rc->left >= GetAreaRight())return false; //画面外(右)
	if(rc->right <= GetAreaLeft())return false; //画面外(左)

	//$ 元動作踏襲：画面上下のはみ出し判定は省略

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
行番号表示に必要な幅を設定。幅が変更された場合はTRUEを返す
*/
bool CTextArea::DetectWidthOfLineNumberArea( bool bRedraw )
{
	const CEditView* pView=m_pEditView;
	CEditView* pView2=m_pEditView;

	int				nViewAlignLeftNew;

	if( pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr[COLORIDX_GYOU].m_bDisp ){
		/* 行番号表示に必要な桁数を計算 */
		int i = DetectWidthOfLineNumberArea_calculate();
		nViewAlignLeftNew = pView->GetTextMetrics().GetHankakuDx() * (i + 1);	/* 表示域の左端座標 */
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
		int nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL ); // 垂直スクロールバーの横幅
		m_nViewCx = rc.Width() - nCxVScroll - GetAreaLeft(); // 表示域の幅
		// 2008.05.27 nasukoji	表示域の桁数も算出する（右端カーソル移動時の表示場所ずれへの対処）
		// m_nViewColNum = CLayoutInt(std::max(0, m_nViewCx - 1) / pView->GetTextMetrics().GetHankakuDx());	// 表示域の桁数
		UpdateViewColRowNums();

		if( bRedraw ){
			/* 再描画 */
			pView2->GetCaret().m_cUnderLine.Lock();
			// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
			pView2->Call_OnPaint(PAINT_LINENUMBER | PAINT_RULER | PAINT_BODY, false); /* メモリＤＣを使用してちらつきのない再描画 */
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


/* 行番号表示に必要な桁数を計算 */
int CTextArea::DetectWidthOfLineNumberArea_calculate() const
{
	const CEditView* pView=m_pEditView;

	int nAllLines; //$$ 単位混在

	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
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
			if( nWork > nAllLines ){	// Oct. 18, 2003 genta 式を整理
				break;
			}
			nWork *= 10;
		}
		return i;
	}else{
		//	2003.09.11 wmlhq 行番号が1桁のときと幅を合わせる
		return 3;
	}
}

void CTextArea::TextArea_OnSize(
	const CMySize& sizeClient, //!< ウィンドウのクライアントサイズ
	int nCxVScroll,            //!< 垂直スクロールバーの横幅
	int nCyHScroll             //!< 水平スクロールバーの縦幅
)
{
	const CEditView* pView = m_pEditView;

	m_nViewCx = sizeClient.cx - nCxVScroll - GetAreaLeft(); // 表示域の幅
	m_nViewCy = sizeClient.cy - nCyHScroll - GetAreaTop();  // 表示域の高さ
	UpdateViewColRowNums();
}



int CTextArea::GetDocumentLeftClientPointX() const
{
	return GetAreaLeft() - (Int)GetViewLeftCol() * m_pEditView->GetTextMetrics().GetHankakuDx();
}

//! クライアント座標からレイアウト位置に変換する
void CTextArea::ClientToLayout(CMyPoint ptClient, CLayoutPoint* pptLayout) const
{
	const CEditView* pView=m_pEditView;
	pptLayout->Set(
		GetViewLeftCol() + CLayoutInt( (ptClient.x - GetAreaLeft()) / pView->GetTextMetrics().GetHankakuDx() ),
		GetViewTopLine() + CLayoutInt( (ptClient.y - GetAreaTop()) / pView->GetTextMetrics().GetHankakuDy() )
	);
}


//! 行番号エリアも含む範囲
void CTextArea::GenerateTopRect   (RECT* rc, CLayoutInt nLineCount) const
{
	rc->left   = 0; //m_nViewAlignLeft;
	rc->right  = m_nViewAlignLeft + m_nViewCx;
	rc->top    = m_nViewAlignTop;
	rc->bottom = m_nViewAlignTop + (Int)nLineCount * m_pEditView->GetTextMetrics().GetHankakuDy();
}

//! 行番号エリアも含む範囲
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
	rc->left   = m_nViewAlignLeft + m_nViewCx - (Int)nColCount * m_pEditView->GetTextMetrics().GetHankakuDx(); //2008.01.26 kobake 符号が逆になってたのを修正
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



