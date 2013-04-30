#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigure_Eol.h"
#include "types/CTypeSupport.h"
#include "doc/layout/CLayout.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "window/CEditWnd.h"

//折り返し描画
void _DispWrap(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);

//EOF描画関数
//実際には pX と nX が更新される。
//2004.05.29 genta
//2007.08.25 kobake 戻り値を void に変更。引数 x, y を DispPos に変更
//2007.08.25 kobake 引数から nCharWidth, nLineHeight を削除
//2007.08.28 kobake 引数 fuOptions を削除
//void _DispEOF( CGraphics& gr, DispPos* pDispPos, const CEditView* pcView, bool bTrans);

//改行記号描画
//2007.08.30 kobake 追加
void _DispEOL(CGraphics& gr, DispPos* pDispPos, CEol cEol, const CEditView* pcView, bool bTrans);


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CFigure_Eol                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_Eol::Match(const wchar_t* pText) const
{
	if(pText[0]==L'\r' && pText[1]==L'\n' && pText[2]==L'\0')return true;
	if(pText[0]==L'\n' && pText[1]==L'\0')return true;
	if(pText[0]==L'\r' && pText[1]==L'\0')return true;
	return false;
}

// 2006.04.29 Moca 選択処理のため縦線処理を追加
//$$ 高速化可能。
bool CFigure_Eol::DrawImp(SColorStrategyInfo* pInfo)
{
	CEditView* pcView = pInfo->pcView;

	// 改行取得
	const CLayout*	pcLayout2 = m_pCEditDoc->m_cLayoutMgr.SearchLineByLayoutY(pInfo->pDispPos->GetLayoutLineRef());
	CEol cEol = pcLayout2->GetLayoutEol();
	if(cEol.GetLen()){
		m_cEol = cEol;
		// this->CFigureSpace::DrawImp(pInfo);
		{
			// CFigureSpace::DrawImp_StyleSelectもどき。選択・検索色を優先する
			CTypeSupport cCurrentType(pcView, pInfo->GetCurrentColor());	// 周辺の色（現在の指定色/選択色）
			CTypeSupport cCurrentType2(pcView, pInfo->GetCurrentColor2());	// 周辺の色（現在の指定色）
			CTypeSupport cTextType(pcView, COLORIDX_TEXT);				// テキストの指定色
			CTypeSupport cSpaceType(pcView, pcView->GetTextDrawer()._GetColorIdx(GetColorIdx()));	// 空白の指定色
			CTypeSupport cSearchType(pcView, COLORIDX_SEARCH);	// 検索色(EOL固有)
			CTypeSupport cCurrentTypeBg(pcView, pInfo->GetCurrentColorBg());
			CTypeSupport& cCurrentType3 = (cCurrentType2.GetBackColor() == cTextType.GetBackColor() ? cCurrentTypeBg: cCurrentType2);
			COLORREF crText;
			COLORREF crBack;
			bool bSelecting = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2();
			bool blendColor = bSelecting && cCurrentType.GetTextColor() == cCurrentType.GetBackColor(); // 選択混合色
			CTypeSupport& currentStyle = blendColor ? cCurrentType2 : cCurrentType;
			CTypeSupport *pcText, *pcBack;
			if( bSelecting && !blendColor ){
				// 選択文字色固定指定
				pcText = &cCurrentType;
				pcBack = &cCurrentType;
			}else if( pInfo->GetCurrentColor2() == COLORIDX_SEARCH ){
				// 検索色優先
				pcText = &cSearchType;
				pcBack = &cSearchType;
			}else{
				pcText = cSpaceType.GetTextColor() == cTextType.GetTextColor() ? &cCurrentType2 : &cSpaceType;
				pcBack = cSpaceType.GetBackColor() == cTextType.GetBackColor() ? &cCurrentType3 : &cSpaceType;
			}
			if( blendColor ){
				// 混合色(検索色を優先しつつ)
				crText = pcView->GetTextColorByColorInfo2(cCurrentType.GetColorInfo(), pcText->GetColorInfo());
				crBack = pcView->GetBackColorByColorInfo2(cCurrentType.GetColorInfo(), pcBack->GetColorInfo());
			}else{
				crText = pcText->GetTextColor();
				crBack = pcBack->GetBackColor();
			}
			pInfo->gr.PushTextForeColor(crText);
			pInfo->gr.PushTextBackColor(crBack);
			bool bTrans = pcView->IsBkBitmap() && cTextType.GetBackColor() == crBack;
			pInfo->gr.PushMyFont(
				pInfo->pcView->GetFontset().ChooseFontHandle(cSpaceType.IsFatFont() || currentStyle.IsFatFont(), cSpaceType.HasUnderLine())
			);
			
			DispPos sPos(*pInfo->pDispPos);	// 現在位置を覚えておく
			_DispEOL(pInfo->gr, pInfo->pDispPos, cEol, pcView, bTrans);
			DrawImp_StylePop(pInfo);
			DrawImp_DrawUnderline(pInfo, sPos);
		}
		pInfo->nPosInLogic+=cEol.GetLen();
	}

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     折り返し描画実装                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 折り返し描画
void _DispWrap(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView)
{
	RECT rcClip2;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,1))
	{
		//サポートクラス
		CTypeSupport cWrapType(pcView,COLORIDX_WRAP);
		bool bTrans = pcView->IsBkBitmap() && cWrapType.GetBackColor() == CTypeSupport(pcView,COLORIDX_TEXT).GetBackColor();

		//描画文字列と色の決定
		const wchar_t* szText;
		if( cWrapType.IsDisp() )
		{
			szText = L"<";
			cWrapType.SetGraphicsState_WhileThisObj(gr);
		}
		else
		{
			szText = L" ";
		}

		//描画
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip2,
			szText,
			wcslen(szText),
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}
	pDispPos->ForwardDrawCol(1);
}
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       EOF描画実装                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void CFigure_Eol::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	_DispEOL(gr, pDispPos, m_cEol, pcView, bTrans);
}

/*!
EOF記号の描画
@date 2004.05.29 genta  MIKさんのアドバイスにより関数にくくりだし
@date 2007.08.28 kobake 引数 nCharWidth 削除
@date 2007.08.28 kobake 引数 fuOptions 削除
@date 2007.08.30 kobake 引数 EofColInfo 削除
*/
void _DispEOF(
	CGraphics&			gr,			//!< [in] 描画対象のDevice Context
	DispPos*			pDispPos,	//!< [in] 表示座標
	const CEditView*	pcView
)
{
	// 描画に使う色情報
	CTypeSupport cEofType(pcView,COLORIDX_EOF);
	if(!cEofType.IsDisp())
		return;
	CTypeSupport cTextType(pcView,COLORIDX_TEXT);
	bool bTrans = pcView->IsBkBitmap() && cEofType.GetBackColor() == cTextType.GetBackColor();

	//必要なインターフェースを取得
	const CTextMetrics* pMetrics=&pcView->GetTextMetrics();
	const CTextArea* pArea=&pcView->GetTextArea();

	//定数
	const wchar_t	szEof[] = L"[EOF]";
	const int		nEofLen = _countof(szEof) - 1;

	//クリッピング領域を計算
	RECT rcClip;
	if(pArea->GenerateClipRect(&rcClip,*pDispPos,nEofLen))
	{
		//色設定
		cEofType.SetGraphicsState_WhileThisObj(gr);

		//描画
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip,
			szEof,
			nEofLen,
			pMetrics->GetDxArray_AllHankaku()
		);
	}

	//描画位置を進める
	pDispPos->ForwardDrawCol(nEofLen);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       改行描画実装                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//画面描画補助関数
//May 23, 2000 genta
//@@@ 2001.12.21 YAZAKI 改行記号の書きかたが変だったので修正
void _DrawEOL(
	CGraphics&		gr,
	const CMyRect&	rcEol,
	CEol			cEol,
	bool			bBold,
	COLORREF		pColor
);

//2007.08.30 kobake 追加
void _DispEOL(CGraphics& gr, DispPos* pDispPos, CEol cEol, const CEditView* pcView, bool bTrans)
{
	RECT rcClip2;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,2)){
		// 2003.08.17 ryoji 改行文字が欠けないように
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip2,
			L"  ",
			2,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		// 改行記号の表示
		if( CTypeSupport(pcView,COLORIDX_EOL).IsDisp() ){
			// From Here 2003.08.17 ryoji 改行文字が欠けないように

			// リージョン作成、選択。
			gr.SetClipping(rcClip2);
			
			// 描画領域
			CMyRect rcEol;
			rcEol.SetPos(pDispPos->GetDrawPos().x + 1, pDispPos->GetDrawPos().y);
			rcEol.SetSize(pcView->GetTextMetrics().GetHankakuWidth(), pcView->GetTextMetrics().GetHankakuHeight());

			// 描画
			// 文字色や太字かどうかを現在の DC から調べる	// 2009.05.29 ryoji 
			// （検索マッチ等の状況に柔軟に対応するため、ここは記号の色指定には決め打ちしない）
			TEXTMETRIC tm;
			::GetTextMetrics(gr, &tm);
			LONG lfWeightNormal = pcView->m_pcEditWnd->GetLogfont().lfWeight;
			_DrawEOL(gr, rcEol, cEol, tm.tmWeight > lfWeightNormal, GetTextColor(gr));

			// リージョン破棄
			gr.ClearClipping();
			
			// To Here 2003.08.17 ryoji 改行文字が欠けないように
		}
	}

	//描画位置を進める(2桁)
	pDispPos->ForwardDrawCol(2);
}


//	May 23, 2000 genta
/*!
画面描画補助関数:
行末の改行マークを改行コードによって書き分ける（メイン）

@note bBoldがTRUEの時は横に1ドットずらして重ね書きを行うが、
あまり太く見えない。

@date 2001.12.21 YAZAKI 改行記号の描きかたを変更。ペンはこの関数内で作るようにした。
						矢印の先頭を、sx, syにして描画ルーチン書き直し。
*/
void _DrawEOL(
	CGraphics&		gr,		//!< Device Context Handle
	const CMyRect&	rcEol,		//!< 描画領域
	CEol			cEol,		//!< 行末コード種別
	bool			bBold,		//!< TRUE: 太字
	COLORREF		pColor		//!< 色
)
{
	int sx, sy;	//	矢印の先頭
	gr.SetPen( pColor );

	switch( cEol.GetType() ){
	case EOL_CRLF:	//	下左矢印
		{
			sx = rcEol.left;						//X左端
			sy = rcEol.top + ( rcEol.Height() / 2);	//Y中心
			DWORD pp[] = { 3, 3 };
			POINT pt[6];
			pt[0].x = sx + rcEol.Width();	//	上へ
			pt[0].y = sy - rcEol.Height() / 4;
			pt[1].x = sx + rcEol.Width();	//	下へ
			pt[1].y = sy;
			pt[2].x = sx;	//	先頭へ
			pt[2].y = sy;
			pt[3].x = sx + rcEol.Height() / 4;	//	先頭から下へ
			pt[3].y = sy + rcEol.Height() / 4;
			pt[4].x = sx;	//	先頭へ戻り
			pt[4].y = sy;
			pt[5].x = sx + rcEol.Height() / 4;	//	先頭から上へ
			pt[5].y = sy - rcEol.Height() / 4;
			::PolyPolyline( gr, pt, pp, _countof(pp));

			if ( bBold ) {
				pt[0].x += 1;	//	上へ（右へずらす）
				pt[0].y += 0;
				pt[1].x += 1;	//	右へ（右にひとつずれている）
				pt[1].y += 1;
				pt[2].x += 0;	//	先頭へ
				pt[2].y += 1;
				pt[3].x += 0;	//	先頭から下へ
				pt[3].y += 1;
				pt[4].x += 0;	//	先頭へ戻り
				pt[4].y += 1;
				pt[5].x += 0;	//	先頭から上へ
				pt[5].y += 1;
				::PolyPolyline( gr, pt, pp, _countof(pp));
			}
		}
		break;
	case EOL_CR:	//	左向き矢印	// 2007.08.17 ryoji EOL_LF -> EOL_CR
		{
			sx = rcEol.left;
			sy = rcEol.top + ( rcEol.Height() / 2 );
			DWORD pp[] = { 3, 2 };
			POINT pt[5];
			pt[0].x = sx + rcEol.Width();	//	右へ
			pt[0].y = sy;
			pt[1].x = sx;	//	先頭へ
			pt[1].y = sy;
			pt[2].x = sx + rcEol.Height() / 4;	//	先頭から下へ
			pt[2].y = sy + rcEol.Height() / 4;
			pt[3].x = sx;	//	先頭へ戻り
			pt[3].y = sy;
			pt[4].x = sx + rcEol.Height() / 4;	//	先頭から上へ
			pt[4].y = sy - rcEol.Height() / 4;
			::PolyPolyline( gr, pt, pp, _countof(pp));

			if ( bBold ) {
				pt[0].x += 0;	//	右へ
				pt[0].y += 1;
				pt[1].x += 0;	//	先頭へ
				pt[1].y += 1;
				pt[2].x += 0;	//	先頭から下へ
				pt[2].y += 1;
				pt[3].x += 0;	//	先頭へ戻り
				pt[3].y += 1;
				pt[4].x += 0;	//	先頭から上へ
				pt[4].y += 1;
				::PolyPolyline( gr, pt, pp, _countof(pp));
			}
		}
		break;
	case EOL_LF:	//	下向き矢印	// 2007.08.17 ryoji EOL_CR -> EOL_LF
		{
			sx = rcEol.left + ( rcEol.Width() / 2 );
			sy = rcEol.top + ( rcEol.Height() * 3 / 4 );
			DWORD pp[] = { 3, 2 };
			POINT pt[5];
			pt[0].x = sx;	//	上へ
			pt[0].y = rcEol.top + rcEol.Height() / 4 + 1;
			pt[1].x = sx;	//	上から下へ
			pt[1].y = sy;
			pt[2].x = sx - rcEol.Height() / 4;	//	そのまま左上へ
			pt[2].y = sy - rcEol.Height() / 4;
			pt[3].x = sx;	//	矢印の先端に戻る
			pt[3].y = sy;
			pt[4].x = sx + rcEol.Height() / 4;	//	そして右上へ
			pt[4].y = sy - rcEol.Height() / 4;
			::PolyPolyline( gr, pt, pp, _countof(pp));

			if( bBold ){
				pt[0].x += 1;	//	上へ
				pt[0].y += 0;
				pt[1].x += 1;	//	上から下へ
				pt[1].y += 0;
				pt[2].x += 1;	//	そのまま左上へ
				pt[2].y += 0;
				pt[3].x += 1;	//	矢印の先端に戻る
				pt[3].y += 0;
				pt[4].x += 1;	//	そして右上へ
				pt[4].y += 0;
				::PolyPolyline( gr, pt, pp, _countof(pp));
			}
		}
		break;
	}
}
