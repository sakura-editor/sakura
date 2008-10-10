#include "stdafx.h"
#include "CFigure_Tab.h"
#include "types/CTypeSupport.h"

//2007.08.28 kobake 追加
void _DispTab( CGraphics& gr, DispPos* pDispPos, const CEditView* pcView );
//タブ矢印描画関数	//@@@ 2003.03.26 MIK
void _DrawTabArrow( CGraphics& gr, int nPosX, int nPosY, int nWidth, int nHeight, bool bBold, COLORREF pColor );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         CFigure_Tab                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_Tab::Match(const wchar_t* pText) const
{
	if( pText[0] == WCODE::TAB ){
		return true;
	}
	return false;
}

//$$ 高速化可能。
//$$ 整理可能。Matchの所でnStartColを取るとよい。CFigure_Tabインスタンスにタブ幅メンバを持たせる？
CLayoutInt CFigure_Tab::GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	return pcDoc->m_cLayoutMgr.GetActualTabSpace( nStartCol );
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//	Sep. 22, 2002 genta 共通式のくくりだし
//	Sep. 23, 2002 genta LayoutMgrの値を使う
//@@@ 2001.03.16 by MIK
//@@@ 2003.03.26 MIK タブ矢印表示
void CFigure_Tab::DispSpace( CGraphics& gr, DispPos* pDispPos, CEditView* pcView ) const
{
	DispPos& sPos=*pDispPos;

	//必要なインターフェース
	const CTextMetrics* pMetrics=&pcView->GetTextMetrics();
	const CTextArea* pArea=&pcView->GetTextArea();
	STypeConfig* TypeDataPtr = &pcView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	int nLineHeight = pMetrics->GetHankakuDy();
	int nCharWidth = pMetrics->GetHankakuDx();

	CTypeSupport cTabType(pcView,COLORIDX_TAB);

	// これから描画するタブ幅
	int tabDispWidth = (Int)pcView->m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace( sPos.GetDrawCol() );

	// タブ記号領域
	RECT rcClip2;
	rcClip2.left = sPos.GetDrawPos().x;
	rcClip2.right = rcClip2.left + nCharWidth * tabDispWidth;
	if( rcClip2.left < pArea->GetAreaLeft() ){
		rcClip2.left = pArea->GetAreaLeft();
	}
	rcClip2.top = sPos.GetDrawPos().y;
	rcClip2.bottom = sPos.GetDrawPos().y + nLineHeight;

	if( pArea->IsRectIntersected(rcClip2) ){
		if( cTabType.IsDisp() && !TypeDataPtr->m_bTabArrow ){	//タブ通常表示	//@@@ 2003.03.26 MIK
			//@@@ 2001.03.16 by MIK
			::ExtTextOutW_AnyBuild(
				gr,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption(),
				&rcClip2,
				TypeDataPtr->m_szTabViewString,
				tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
				pMetrics->GetDxArray_AllHankaku()
			);
		}else{
			//背景
			::ExtTextOutW_AnyBuild(
				gr,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption(),
				&rcClip2,
				L"        ",
				tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
				pMetrics->GetDxArray_AllHankaku()
			);

			//タブ矢印表示
			if( cTabType.IsDisp() && TypeDataPtr->m_bTabArrow && rcClip2.left <= sPos.GetDrawPos().x ){ // Apr. 1, 2003 MIK 行番号と重なる
				_DrawTabArrow(
					gr,
					sPos.GetDrawPos().x,
					sPos.GetDrawPos().y,
					pMetrics->GetHankakuWidth(),
					pMetrics->GetHankakuHeight(),
					cTabType.IsFatFont(),
					::GetTextColor(gr)//cTabType.GetTextColor()
				);
			}
		}
	}

	//Xを進める
	sPos.ForwardDrawCol(tabDispWidth);
}



/*
	タブ矢印描画関数
*/
void _DrawTabArrow(
	CGraphics&	_gr,
	int			nPosX,   //ピクセルX
	int			nPosY,   //ピクセルY
	int			nWidth,  //ピクセルW
	int			nHeight, //ピクセルH
	bool		bBold,
	COLORREF	pColor
)
{
	// 一時的なペンを使用するため、新しく CGraphics オブジェクトを作成。
	CGraphics gr(_gr);

	// ペン設定
	gr.SetPen( pColor );

	// 幅調整
	nWidth--;

	// 矢印の先頭
	int sx = nPosX + nWidth;
	int sy = nPosY + ( nHeight / 2 );

	for(int i = 0; i < (bBold?2:1); i++){
		int y = sy + i;
		gr.DrawLine(sx - nWidth,	y,	sx,					y				);	//「─」左端から右端
		gr.DrawLine(sx,				y,	sx - nHeight / 4,	y + nHeight / 4	);	//「／」右端から斜め左下
		gr.DrawLine(sx,				y,	sx - nHeight / 4,	y - nHeight / 4	);	//「＼」右端から斜め左上
	}
}
