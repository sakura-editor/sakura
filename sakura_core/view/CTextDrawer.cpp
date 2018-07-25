/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "CTextDrawer.h"
#include <vector>
#include "CTextMetrics.h"
#include "CTextArea.h"
#include "CViewFont.h"
#include "CEol.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "types/CTypeSupport.h"
#include "charset/charcode.h"
#include "doc/layout/CLayout.h"


const CTextArea* CTextDrawer::GetTextArea() const
{
	return &m_pEditView->GetTextArea();
}

using namespace std;


/*
テキスト表示
@@@ 2002.09.22 YAZAKI    const unsigned char* pDataを、const char* pDataに変更
@@@ 2007.08.25 kobake 戻り値を void に変更。引数 x, y を DispPos に変更
*/
void CTextDrawer::DispText( HDC hdc, DispPos* pDispPos, int marginy, const wchar_t* pData, int nLength, bool bTransparent ) const
{
	if( 0 >= nLength ){
		return;
	}
	int x=pDispPos->GetDrawPos().x;
	int y=pDispPos->GetDrawPos().y;

	//必要なインターフェースを取得
	const CTextMetrics* pMetrics=&m_pEditView->GetTextMetrics();
	const CTextArea* pArea=GetTextArea();

	//文字間隔配列を生成
	static vector<int> vDxArray(1);
	const int* pDxArray = pMetrics->GenerateDxArray2(&vDxArray, pData, nLength);

	//文字列のピクセル幅
	int nTextWidth=pMetrics->CalcTextWidth(pData,nLength,pDxArray);

	//テキストの描画範囲の矩形を求める -> rcClip
	CMyRect rcClip;
	rcClip.left   = x;
	rcClip.right  = x + nTextWidth;
	rcClip.top    = y;
	rcClip.bottom = y + pMetrics->GetHankakuDy();
	if( rcClip.left < pArea->GetAreaLeft() ){
		rcClip.left = pArea->GetAreaLeft();
	}

	//文字間隔
	int nDx = pMetrics->GetCharPxWidth();

	if( pArea->IsRectIntersected(rcClip) && rcClip.top >= pArea->GetAreaTop() ){

		//@@@	From Here 2002.01.30 YAZAKI ExtTextOutW_AnyBuildの制限回避
		if( rcClip.Width() > pArea->GetAreaWidth() ){
			rcClip.right = rcClip.left + pArea->GetAreaWidth();
		}

		// ウィンドウの左にあふれた文字数 -> nBefore
		// 2007.09.08 kobake注 「ウィンドウの左」ではなく「クリップの左」を元に計算したほうが描画領域を節約できるが、
		//                        バグが出るのが怖いのでとりあえずこのまま。
		int nBeforeLogic = 0;
		CLayoutInt nBeforeLayout = CLayoutInt(0);
		if ( x < 0 ){
			int nLeftLayout = ( 0 - x ) / nDx - 1;
			CLayoutMgr& layoutMgr = m_pEditView->m_pcEditDoc->m_cLayoutMgr;
			while (nBeforeLayout < nLeftLayout){
				nBeforeLayout += layoutMgr.GetLayoutXOfChar(pData, nLength, nBeforeLogic);
				nBeforeLogic  += CNativeW::GetSizeOfChar( pData, nLength, nBeforeLogic );
			}
		}

		/*
		// ウィンドウの右にあふれた文字数 -> nAfter
		int nAfterLayout = 0;
		if ( rcClip.right < x + nTextWidth ){
			//	-1してごまかす（うしろはいいよね？）
			nAfterLayout = (x + nTextWidth - rcClip.right) / nDx - 1;
		}
		*/

		// 描画開始位置
		int nDrawX = x + (Int)nBeforeLayout * nDx;

		// 実際の描画文字列ポインタ
		const wchar_t* pDrawData          = &pData[nBeforeLogic];
		int            nDrawDataMaxLength = nLength - nBeforeLogic;

		// 実際の文字間隔配列
		const int* pDrawDxArray = &pDxArray[nBeforeLogic];

		// 描画する文字列長を求める -> nDrawLength
		int nRequiredWidth = rcClip.right - nDrawX; //埋めるべきピクセル幅
		if(nRequiredWidth <= 0)goto end;
		int nWorkWidth = 0;
		int nDrawLength = 0;
		while(nWorkWidth < nRequiredWidth)
		{
			if(nDrawLength >= nDrawDataMaxLength)break;
			nWorkWidth += pDrawDxArray[nDrawLength++];
		}
		// サロゲートペア対策	2008/7/5 Uchi	Update 7/8 Uchi
		if (nDrawLength < nDrawDataMaxLength && pDrawDxArray[nDrawLength] == 0) {
			nDrawLength++;
		}

		//描画
		::ExtTextOutW_AnyBuild(
			hdc,
			nDrawX,					//X
			y + marginy,			//Y
			ExtTextOutOption() & ~(bTransparent? ETO_OPAQUE: 0),
			&rcClip,
			pDrawData,				//文字列
			nDrawLength,			//文字列長
			pDrawDxArray			//文字間隔の入った配列
		);
	}

end:
	//描画位置を進める
	pDispPos->ForwardDrawCol(CLayoutXInt(nTextWidth / nDx));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        指定桁縦線                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	指定桁縦線の描画
	@date 2005.11.08 Moca 新規作成
	@date 2006.04.29 Moca 太線・点線のサポート。選択中の反転対策に行ごとに作画するように変更
	    縦線の色がテキストの背景色と同じ場合は、縦線の背景色をEXORで作画する
	@note Common::m_nVertLineOffsetにより、指定桁の前の文字の上に作画されることがある。
*/
void CTextDrawer::DispVerticalLines(
	CGraphics&	gr,			//!< 作画するウィンドウのDC
	int			nTop,		//!< 線を引く上端のクライアント座標y
	int			nBottom,	//!< 線を引く下端のクライアント座標y
	CLayoutInt	nLeftCol,	//!< 線を引く範囲の左桁の指定
	CLayoutInt	nRightCol	//!< 線を引く範囲の右桁の指定(-1で未指定)
) const
{
	const CEditView* pView=m_pEditView;

	const STypeConfig&	typeData = pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	CTypeSupport cVertType(pView,COLORIDX_VERTLINE);
	CTypeSupport cTextType(pView,COLORIDX_TEXT);

	if(!cVertType.IsDisp())return;

	const CLayoutXInt nViewLeftCol = pView->GetTextArea().GetViewLeftCol();

	nLeftCol = t_max( nViewLeftCol, nLeftCol );

	const CLayoutInt nWrapLayout  = pView->m_pcEditDoc->m_cLayoutMgr.GetMaxLineLayout();
	if( nRightCol < 0 ){
		nRightCol = nWrapLayout;
	}
	const int nPosXOffset = GetDllShareData().m_Common.m_sWindow.m_nVertLineOffset + pView->GetTextArea().GetAreaLeft();
	const int nPosXLeft   = t_max(pView->GetTextMetrics().GetCharPxWidth(pView->GetTextArea().GetAreaLeft() + (nLeftCol  - nViewLeftCol)), pView->GetTextArea().GetAreaLeft() );
	const int nPosXRight  = t_min(pView->GetTextMetrics().GetCharPxWidth(pView->GetTextArea().GetAreaLeft() + (nRightCol - nViewLeftCol)), pView->GetTextArea().GetAreaRight() );
	const int nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	bool bOddLine = ((((nLineHeight % 2) ? (Int)pView->GetTextArea().GetViewTopLine() : 0) + pView->GetTextArea().GetAreaTop() + nTop) % 2 == 1);

	// 太線
	const bool bBold = cVertType.IsBoldFont();
	// ドット線(下線属性を転用/テスト用)
	const bool bDot = cVertType.HasUnderLine();
	const bool bExorPen = ( cVertType.GetTextColor() == cTextType.GetBackColor() );
	int nROP_Old = 0;
	if( bExorPen ){
		gr.SetPen( cVertType.GetBackColor() );
		nROP_Old = ::SetROP2( gr, R2_NOTXORPEN );
	}
	else{
		gr.SetPen( cVertType.GetTextColor() );
	}

	int k;
	for( k = 0; k < MAX_VERTLINES && typeData.m_nVertLineIdx[k] != 0; k++ ){
		// nXColは1開始。GetTextArea().GetViewLeftCol()は0開始なので注意。
		CLayoutXInt nXCol = pView->GetTextMetrics().GetLayoutXDefault(typeData.m_nVertLineIdx[k]);
		CLayoutXInt nXColEnd = nXCol;
		CLayoutXInt nXColAdd = pView->GetTextMetrics().GetLayoutXDefault();
		// nXColがマイナスだと繰り返し。k+1を終了値、k+2をステップ幅として利用する
		if( nXCol < 0 ){
			if( k < MAX_VERTLINES - 2 ){
				nXCol = -nXCol;
				nXColEnd = pView->GetTextMetrics().GetLayoutXDefault(typeData.m_nVertLineIdx[++k]);
				nXColAdd = pView->GetTextMetrics().GetLayoutXDefault(typeData.m_nVertLineIdx[++k]);
				if( nXColEnd < nXCol || nXColAdd <= 0 ){
					continue;
				}
				// 作画範囲の始めまでスキップ
				if( nXCol < nViewLeftCol ){
					nXCol = nViewLeftCol + nXColAdd - (nViewLeftCol - nXCol) % nXColAdd;
				}
			}else{
				k += 2;
				continue;
			}
		}
		for(; nXCol <= nXColEnd; nXCol += nXColAdd ){
			if( nWrapLayout < nXCol ){
				break;
			}
			int nPosX = nPosXOffset + pView->GetTextMetrics().GetCharPxWidth(nXCol - pView->GetTextMetrics().GetLayoutXDefault() - nViewLeftCol);
			// 2006.04.30 Moca 線の引く範囲・方法を変更
			// 太線の場合、半分だけ作画する可能性がある。
			int nPosXBold = nPosX;
			if( bBold ){
				nPosXBold -= 1;
			}
			if( nPosXRight <= nPosXBold ){
				break;
			}
			if( nPosXLeft <= nPosX ){
				if( bDot ){
					// 点線で作画。1ドットの線を作成
					int y = nTop;
					// スクロールしても線が切れないように座標を調整
					if( bOddLine ){
						y++;
					}
					for( ; y < nBottom; y += 2 ){
						if( nPosX < nPosXRight ){
							::MoveToEx( gr, nPosX, y, NULL );
							::LineTo( gr, nPosX, y + 1 );
						}
						if( bBold && nPosXLeft <= nPosXBold ){
							::MoveToEx( gr, nPosXBold, y, NULL );
							::LineTo( gr, nPosXBold, y + 1 );
						}
					}
				}else{
					if( nPosX < nPosXRight ){
						::MoveToEx( gr, nPosX, nTop, NULL );
						::LineTo( gr, nPosX, nBottom );
					}
					if( bBold && nPosXLeft <= nPosXBold ){
						::MoveToEx( gr, nPosXBold, nTop, NULL );
						::LineTo( gr, nPosXBold, nBottom );
					}
				}
			}
		}
	}
	if( bExorPen ){
		::SetROP2( gr, nROP_Old );
	}
}

void CTextDrawer::DispNoteLine(
	CGraphics&	gr,			//!< 作画するウィンドウのDC
	int			nTop,		//!< 線を引く上端のクライアント座標y
	int			nBottom,	//!< 線を引く下端のクライアント座標y
	int			nLeft,		//!< 線を引く左端
	int			nRight		//!< 線を引く右端
) const
{
	const CEditView* pView=m_pEditView;

	CTypeSupport cNoteLine(pView, COLORIDX_NOTELINE);
	if( cNoteLine.IsDisp() ){
		gr.SetPen( cNoteLine.GetTextColor() );
		const int nLineHeight = pView->GetTextMetrics().GetHankakuDy();
		const int left = nLeft;
		const int right = nRight;
		int userOffset = pView->m_pTypeData->m_nNoteLineOffset;
		int offset = pView->GetTextArea().GetAreaTop() + userOffset - 1;
		while( offset < 0 ){
			offset += nLineHeight;
		}
		int offsetMod = offset % nLineHeight;
		int y = ((nTop - offset) / nLineHeight * nLineHeight) + offsetMod;
		for( ; y < nBottom; y += nLineHeight ){
			if( nTop <= y ){
				::MoveToEx( gr, left, y, NULL );
				::LineTo( gr, right, y );
			}
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        折り返し桁縦線                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	折り返し桁縦線の描画
	@date 2009.10.24 ryoji 新規作成
*/
void CTextDrawer::DispWrapLine(
	CGraphics&	gr,			//!< 作画するウィンドウのDC
	int			nTop,		//!< 線を引く上端のクライアント座標y
	int			nBottom		//!< 線を引く下端のクライアント座標y
) const
{
	const CEditView* pView = m_pEditView;
	CTypeSupport cWrapType(pView, COLORIDX_WRAP);
	if( !cWrapType.IsDisp() ) return;

	const CTextArea& rArea = *GetTextArea();
	const CLayoutInt nWrapLayout = pView->m_pcEditDoc->m_cLayoutMgr.GetMaxLineLayout();
	int nXPos = rArea.GetAreaLeft() + pView->GetTextMetrics().GetCharPxWidth(nWrapLayout - rArea.GetViewLeftCol());
	//	2005.11.08 Moca 作画条件変更
	if( rArea.GetAreaLeft() < nXPos && nXPos < rArea.GetAreaRight() ){
		/// 折り返し記号の色のペンを設定
		gr.PushPen(cWrapType.GetTextColor(), 0);

		::MoveToEx( gr, nXPos, nTop, NULL );
		::LineTo( gr, nXPos, nBottom );

		gr.PopPen();
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          行番号                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispLineNumber(
	CGraphics&		gr,
	CLayoutInt		nLineNum,
	int				y
) const
{
	//$$ 高速化：SearchLineByLayoutYにキャッシュを持たせる
	const CLayout*	pcLayout = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );

	const CEditView* pView=m_pEditView;
	const STypeConfig* pTypes=&pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	int				nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int				nCharWidth = pView->GetTextMetrics().GetHankakuDx();
	// 行番号表示部分X幅	Sep. 23, 2002 genta 共通式のくくりだし
	//int				nLineNumAreaWidth = pView->GetTextArea().m_nViewAlignLeftCols * nCharWidth;
	int				nLineNumAreaWidth = pView->GetTextArea().GetLineNumberWidth();	// 2009.03.26 ryoji
	CTypeSupport cTextType(pView,COLORIDX_TEXT);
	CTypeSupport cCaretLineBg(pView, COLORIDX_CARETLINEBG);
	CTypeSupport cEvenLineBg(pView, COLORIDX_EVENLINEBG);
	// 行がないとき・行の背景が透明のときの色
	CTypeSupport &cBackType = (cCaretLineBg.IsDisp() &&
		pView->GetCaret().GetCaretLayoutPos().GetY() == nLineNum
			? cCaretLineBg
			: cEvenLineBg.IsDisp() && nLineNum % 2 == 1
				? cEvenLineBg
				: cTextType);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     nColorIndexを決定                       //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	EColorIndexType nColorIndex = COLORIDX_GYOU;	/* 行番号 */
	const CDocLine*	pCDocLine = NULL;
	bool bGyouMod = false;
	if( pcLayout ){
		pCDocLine = pcLayout->GetDocLineRef();

		if( pView->GetDocument()->m_cDocEditor.IsModified() && CModifyVisitor().IsLineModified(pCDocLine, pView->GetDocument()->m_cDocEditor.m_cOpeBuf.GetNoModifiedSeq()) ){		/* 変更フラグ */
			if( CTypeSupport(pView,COLORIDX_GYOU_MOD).IsDisp() ){	// 2006.12.12 ryoji
				nColorIndex = COLORIDX_GYOU_MOD;	/* 行番号（変更行） */
				bGyouMod = true;
			}
		}
	}

	if(pCDocLine){
		//DIFF色設定
		CDiffLineGetter(pCDocLine).GetDiffColor(&nColorIndex);

		// 02/10/16 ai
		// ブックマークの表示
		if(CBookmarkGetter(pCDocLine).IsBookmarked()){
			if( CTypeSupport(pView,COLORIDX_MARK).IsDisp() ) {
				nColorIndex = COLORIDX_MARK;
			}
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             決定されたnColorIndexを使って描画               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	CTypeSupport cColorType(pView,nColorIndex);
	CTypeSupport cMarkType(pView,COLORIDX_MARK);

	//該当行の行番号エリア矩形
	RECT	rcLineNum;
	rcLineNum.left = 0;
	rcLineNum.right = nLineNumAreaWidth;
	rcLineNum.top = y;
	rcLineNum.bottom = y + nLineHeight;
	
	bool bTrans = pView->IsBkBitmap() && cTextType.GetBackColor() == cColorType.GetBackColor();
	bool bTransText = pView->IsBkBitmap() && cTextType.GetBackColor() == cBackType.GetBackColor();
	bool bDispLineNumTrans = false;

	COLORREF fgcolor = cColorType.GetTextColor();
	COLORREF bgcolor = cColorType.GetBackColor();
	CTypeSupport cGyouType(pView,COLORIDX_GYOU);
	CTypeSupport cGyouModType(pView,COLORIDX_GYOU_MOD);
	if( bGyouMod && nColorIndex != COLORIDX_GYOU_MOD ){
		if( cGyouType.GetTextColor() == cColorType.GetTextColor() ){
			fgcolor = cGyouModType.GetTextColor();
		}
		if( cGyouType.GetBackColor() == cColorType.GetBackColor() ){
			bgcolor = cGyouModType.GetBackColor();
			bTrans = pView->IsBkBitmap() && cTextType.GetBackColor() == cGyouModType.GetBackColor();
		}
	}
	// 2014.01.29 Moca 背景色がテキストと同じなら、透過色として行背景色を適用
	if( bgcolor == cTextType.GetBackColor() ){
		bgcolor = cBackType.GetBackColor();
		bTrans = pView->IsBkBitmap() && cTextType.GetBackColor() == bgcolor;
		bDispLineNumTrans = true;
	}
	if(!pcLayout){
		//行が存在しない場合は、テキスト描画色で塗りつぶし
		if( !bTransText ){
			cBackType.FillBack(gr,rcLineNum);
		}
		bDispLineNumTrans = true;
	}
	else if( CTypeSupport(pView,COLORIDX_GYOU).IsDisp() ){ /* 行番号表示／非表示 */
		SFONT sFont = cColorType.GetTypeFont();
	 	// 2013.12.30 変更行の色・フォント属性をDIFFブックマーク行に継承するように
		if( bGyouMod && nColorIndex != COLORIDX_GYOU_MOD ){
			bool bChange = true;
			if( cGyouType.IsBoldFont() == cColorType.IsBoldFont() ){
		 		sFont.m_sFontAttr.m_bBoldFont = cGyouModType.IsBoldFont();
				bChange = true;
			}
			if( cGyouType.HasUnderLine() == cColorType.HasUnderLine() ){
				sFont.m_sFontAttr.m_bUnderLine = cGyouModType.HasUnderLine();
				bChange = true;
			}
			if( bChange ){
				sFont.m_hFont = pView->GetFontset().ChooseFontHandle( 0, sFont.m_sFontAttr );
			}
		}
		gr.PushTextForeColor(fgcolor);	//テキスト：行番号の色
		gr.PushTextBackColor(bgcolor);	//テキスト：行番号背景の色
		gr.PushMyFont(sFont);	//フォント：行番号のフォント

		//描画文字列
		wchar_t szLineNum[18];
		int nLineCols;
		int nLineNumCols;
		{
			/* 行番号の表示 false=折り返し単位／true=改行単位 */
			if( pTypes->m_bLineNumIsCRLF ){
				/* 論理行番号表示モード */
				if( NULL == pcLayout || 0 != pcLayout->GetLogicOffset() ){ //折り返しレイアウト行
					wcscpy( szLineNum, L" " );
				}else{
					_itow( pcLayout->GetLogicLineNo() + 1, szLineNum, 10 );	/* 対応する論理行番号 */
//###デバッグ用
//					_itow( CModifyVisitor().GetLineModifiedSeq(pCDocLine), szLineNum, 10 );	// 行の変更番号
				}
			}else{
				/* 物理行（レイアウト行）番号表示モード */
				_itow( (Int)nLineNum + 1, szLineNum, 10 );
			}
			nLineCols = wcslen( szLineNum );
			nLineNumCols = nLineCols; // 2010.08.17 Moca 位置決定に行番号区切りは含めない

			/* 行番号区切り 0=なし 1=縦線 2=任意 */
			if( 2 == pTypes->m_nLineTermType ){
				//	Sep. 22, 2002 genta
				szLineNum[ nLineCols ] = pTypes->m_cLineTermChar;
				szLineNum[ ++nLineCols ] = '\0';
			}
		}

		//	Sep. 23, 2002 genta
		int drawNumTop = (pView->GetTextArea().m_nViewAlignLeftCols - nLineNumCols - 1) * ( nCharWidth );
		int fontNo = WCODE::GetFontNo('0');
		int nHeightMargin = pView->GetTextMetrics().GetCharHeightMarginByFontNo(fontNo);
		::ExtTextOutW_AnyBuild( gr,
			drawNumTop,
			y + nHeightMargin,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcLineNum,
			szLineNum,
			nLineCols,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		/* 行番号区切り 0=なし 1=縦線 2=任意 */
		if( 1 == pTypes->m_nLineTermType ){
			RECT rc;
			rc.left = nLineNumAreaWidth - 2;
			rc.top = y;
			rc.right = nLineNumAreaWidth - 1;
			rc.bottom = y + nLineHeight;
			gr.FillSolidMyRect(rc, fgcolor);
		}

		gr.PopTextForeColor();
		gr.PopTextBackColor();
		gr.PopMyFont();
	}
	else{
		// 行番号エリアの背景描画
		if( !bTrans ){
			gr.FillSolidMyRect(rcLineNum, bgcolor);
		}
		bDispLineNumTrans = true;
	}

	//行属性描画 ($$$分離予定)
	if(pCDocLine)
	{
		// 2001.12.03 hor
		/* とりあえずブックマークに縦線 */
		if(CBookmarkGetter(pCDocLine).IsBookmarked() && !cMarkType.IsDisp() )
		{
			gr.PushPen(cColorType.GetTextColor(),2);
			::MoveToEx( gr, 1, y, NULL );
			::LineTo( gr, 1, y + nLineHeight );
			gr.PopPen();
		}

		//DIFFマーク描画
		if( !pView->m_bMiniMap ){
			CDiffLineGetter(pCDocLine).DrawDiffMark(gr,y,nLineHeight,fgcolor);
		}
	}

	// 行番号とテキストの隙間の描画
	if( !bTransText ){
		RECT rcRest;
		rcRest.left   = rcLineNum.right;
		rcRest.right  = pView->GetTextArea().GetAreaLeft();
		rcRest.top    = y;
		rcRest.bottom = y + nLineHeight;
		cBackType.FillBack(gr,rcRest);
	}

	// 行番号部分のノート線描画
	if( !pView->m_bMiniMap ){
		int left   = bDispLineNumTrans ? 0 : rcLineNum.right;
		int right  = pView->GetTextArea().GetAreaLeft();
		int top    = y;
		int bottom = y + nLineHeight;
		DispNoteLine( gr, top, bottom, left, right );
	}
}

