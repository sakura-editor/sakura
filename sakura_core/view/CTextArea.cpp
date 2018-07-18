#include "StdAfx.h"
#include "CTextArea.h"
#include "CViewFont.h"
#include "CRuler.h"
#include "CEditView.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "doc/CEditDoc.h"

// 2014.07.26 katze
//#define USE_LOG10			// この行のコメントを外すと行番号の最小桁数の計算にlog10()を用いる
#ifdef USE_LOG10
#include <math.h>
#endif

//! テンプレートでべき乗を計算(!=0)
template <int N, int M>
struct power{
    static const int value = N * power<N, M - 1>::value;
};

//! テンプレートでべき乗を計算(==0)
template<int N>
struct power<N, 0>{
    static const int value = 1;
};

CTextArea::CTextArea(CEditView* pEditView)
: m_pEditView(pEditView)
{
	DLLSHAREDATA* pShareData = &GetDllShareData();

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
	m_nViewColNum = CLayoutInt(t_max(1, t_max(0, m_nViewCx - 1) / pView->GetTextMetrics().GetCharPxWidth()));	// 表示域の桁数
	m_nViewRowNum = CLayoutInt(t_max(1, t_max(0, m_nViewCy - 1) / pView->GetTextMetrics().GetHankakuDy()));	// 表示域の行数
}

//!フォント変更の際、各種パラメータを計算し直す
void CTextArea::UpdateAreaMetrics()
{
	//表示域の再計算
	//2010.08.24 Dx/Dyを使うので後で設定
	UpdateViewColRowNums();
}

void CTextArea::GenerateCharRect(RECT* rc,const DispPos& sPos,CLayoutXInt nColumns) const
{
	const CEditView* pView=m_pEditView;

	rc->left   = sPos.GetDrawPos().x;
	rc->right  = sPos.GetDrawPos().x + pView->GetTextMetrics().GetCharPxWidth(nColumns);
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

bool CTextArea::GenerateClipRect(RECT* rc, const DispPos& sPos, CLayoutXInt nColumns) const
{
	GenerateCharRect(rc, sPos, nColumns);
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

	if( pView->m_pTypeData->m_ColorInfoArr[COLORIDX_GYOU].m_bDisp && !pView->m_bMiniMap ){
		/* 行番号表示に必要な桁数を計算 */
		int i = DetectWidthOfLineNumberArea_calculate(&pView->m_pcEditDoc->m_cLayoutMgr);
		nViewAlignLeftNew = pView->GetTextMetrics().GetHankakuDx() * (i + 1);	/* 表示域の左端座標 */
		m_nViewAlignLeftCols = i + 1;
	}else if( pView->m_bMiniMap ){
		nViewAlignLeftNew = 4;
		m_nViewAlignLeftCols = 0;
	}else{
		nViewAlignLeftNew = 8;
		m_nViewAlignLeftCols = 0;
	}

	//	Sep 18, 2002 genta
	nViewAlignLeftNew += GetLeftYohaku();
	if( nViewAlignLeftNew != GetAreaLeft() ){
		CMyRect			rc;
		SetAreaLeft(nViewAlignLeftNew);
		pView->GetClientRect( &rc );
		int nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL ); // 垂直スクロールバーの横幅
		m_nViewCx = rc.Width() - nCxVScroll - GetAreaLeft(); // 表示域の幅
		// 2008.05.27 nasukoji	表示域の桁数も算出する（右端カーソル移動時の表示場所ずれへの対処）
		// m_nViewColNum = CLayoutInt(t_max(0, m_nViewCx - 1) / pView->GetTextMetrics().GetHankakuDx());	// 表示域の桁数
		UpdateViewColRowNums();

		if( bRedraw && pView2->GetDrawSwitch() ){
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


/*!
	行番号表示に必要な桁数を計算

	@param [in] pLayoutMgr
	@param [in] bLayout true:レイアウト行単位 / false:物理行単位

	@return 行番号表示に必要な桁数
*/
int CTextArea::DetectWidthOfLineNumberArea_calculate(const CLayoutMgr* pLayoutMgr, bool bLayout) const
{
	const CEditView* pView=m_pEditView;

	int nAllLines; //$$ 単位混在

	/* 行番号の表示 false=折り返し単位／true=改行単位 */
	if( pView->m_pTypeData->m_bLineNumIsCRLF && !bLayout){
		nAllLines = pView->m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	}
	else{
		nAllLines = (Int)pLayoutMgr->GetLineCount();
	}
	
	if( 0 < nAllLines ){
		int nWork;
		int i;

		// 行番号の桁数を決める 2014.07.26 katze
		/* m_nLineNumWidthは純粋に数字の桁数を示し、先頭の空白を含まない（仕様変更） 2014.08.02 katze */
#ifdef USE_LOG10
		/* 表示している行数の桁数を求める */
		nWork = (int)(log10( (double)nAllLines) +1);	// 10を底とする対数(小数点以下切り捨て)+1で桁数
		/* 設定値と比較し、大きい方を取る */
		i = std::max( nWork, pView->m_pTypeData->m_nLineNumWidth );
		// 先頭の空白分を加算する
		return (i +1);
#else
		/* 設定から行数を求める */
		nWork = power<10, LINENUMWIDTH_MIN>::value;
		for( i = LINENUMWIDTH_MIN; i < pView->m_pTypeData->m_nLineNumWidth; ++i ){
			nWork *= 10;
		}
		/* 表示している行数と比較し、大きい方の値を取る */
		for( /*i = pView->m_pTypeData->m_nLineNumWidth*/; i < LINENUMWIDTH_MAX; ++i ){
			if( nWork > nAllLines ){	// Oct. 18, 2003 genta 式を整理
				break;
			}
			nWork *= 10;
		}
		// 先頭の空白分を加算する
		return (i +1);
#endif
	}else{
		//	2003.09.11 wmlhq 行番号が1桁のときと幅を合わせる
		// 最小桁数を可変に変更 2014.07.26 katze	// 先頭の空白分を加算する 2014.07.31 katze
		return pView->m_pTypeData->m_nLineNumWidth +1;
	}
}

void CTextArea::TextArea_OnSize(
	const CMySize& sizeClient, //!< ウィンドウのクライアントサイズ
	int nCxVScroll,            //!< 垂直スクロールバーの横幅
	int nCyHScroll             //!< 水平スクロールバーの縦幅
)
{
	m_nViewCx = sizeClient.cx - nCxVScroll - GetAreaLeft(); // 表示域の幅
	m_nViewCy = sizeClient.cy - nCyHScroll - GetAreaTop();  // 表示域の高さ
	UpdateViewColRowNums();
}



int CTextArea::GetDocumentLeftClientPointX() const
{
	return GetAreaLeft() - m_pEditView->GetTextMetrics().GetCharPxWidth(GetViewLeftCol());
}

//! クライアント座標からレイアウト位置に変換する
void CTextArea::ClientToLayout(CMyPoint ptClient, CLayoutPoint* pptLayout) const
{
	const CEditView* pView=m_pEditView;
	pptLayout->Set(
		GetViewLeftCol() + CLayoutInt( (ptClient.x - GetAreaLeft()) / pView->GetTextMetrics().GetCharPxWidth() ),
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
	rc->right  = m_nViewAlignLeft + m_pEditView->GetTextMetrics().GetCharPxWidth(nColCount);
	rc->top    = m_nViewAlignTop;
	rc->bottom = m_nViewAlignTop + m_nViewCy;
}

void CTextArea::GenerateRightRect (RECT* rc, CLayoutInt nColCount ) const
{
	rc->left   = m_nViewAlignLeft + m_nViewCx - m_pEditView->GetTextMetrics().GetCharPxWidth(nColCount); //2008.01.26 kobake 符号が逆になってたのを修正
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


int CTextArea::GenerateYPx(CLayoutYInt nLineNum) const
{
	CLayoutYInt nY = nLineNum - GetViewTopLine();
	int ret;
	if( nY < 0 ){
		ret = GetAreaTop();
	}else if( m_nViewRowNum < nY ){
		ret = GetAreaBottom();
	}else{
		ret = GetAreaTop() + m_pEditView->GetTextMetrics().GetHankakuDy() * (Int)(nY);
	}
	return ret;
}

