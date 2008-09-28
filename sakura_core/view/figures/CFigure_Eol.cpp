#include "stdafx.h"
#include "CFigure_Eol.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"

//折り返し描画
void _DispWrap(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);

//EOF描画関数
//実際には pX と nX が更新される。
//2004.05.29 genta
//2007.08.25 kobake 戻り値を void に変更。引数 x, y を DispPos に変更
//2007.08.25 kobake 引数から nCharWidth, nLineHeight を削除
//2007.08.28 kobake 引数 fuOptions を削除
void _DispEOF( CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);

//空(から)行描画
bool _DispEmptyLine(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);

//改行記号描画
//2007.08.30 kobake 追加
void _DispEOL(CGraphics& gr, DispPos* pDispPos, CEol cEol, const CEditView* pcView);


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

CLayoutInt CFigure_Eol::GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const
{
	return CLayoutInt(2);
}

// 2006.04.29 Moca 選択処理のため縦線処理を追加
//$$ 高速化可能。
bool CFigure_Eol::DrawImp(SColorStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	CEditView* pcView = &CEditWnd::Instance()->GetActiveView();

	//コンフィグ
	CTypeSupport		cTextType	(pcView,COLORIDX_TEXT);
	int					nLineHeight	= pcView->GetTextMetrics().GetHankakuDy();
	const CLayoutInt	nWrapKeta	= pcDoc->m_cLayoutMgr.GetMaxLineKetas();	// 折り返し幅

	// 改行取得
	const CLayout*	pcLayout2 = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );
	CEol cEol = pcLayout2->GetLayoutEol();

	// 行末記号を描画
	{
		// 改行が存在した場合は、改行記号を表示
		if(cEol.GetLen()){
			_DispEOL(pInfo->gr,pInfo->pDispPos,pcLayout2->GetLayoutEol(),pInfo->pcView);
			pInfo->nPosInLogic+=cEol.GetLen();
		}
		// 最終行の場合は、EOFを表示
		else if(pInfo->pDispPos->GetLayoutLineRef()+1==CEditDoc::GetInstance(0)->m_cLayoutMgr.GetLineCount() && pInfo->pDispPos->GetDrawCol() < nWrapKeta){
			if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
				_DispEOF(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
			}
			pInfo->nPosInLogic+=CLogicInt(1);
		}
		// それ以外では、折り返し記号を表示
		else{
			_DispWrap(pInfo->gr,pInfo->pDispPos,pcView);
			pInfo->nPosInLogic+=CLogicInt(1);
		}
	}

	// 行末背景描画
	RECT rcClip;
	if(pInfo->pcView->GetTextArea().GenerateClipRectRight(&rcClip,*pInfo->pDispPos)){
		cTextType.FillBack(pInfo->gr,rcClip);
	}

	// 縦線描画
	pInfo->pcView->GetTextDrawer().DispVerticalLines(
		pInfo->gr,
		pInfo->pDispPos->GetDrawPos().y,
		pInfo->pDispPos->GetDrawPos().y + nLineHeight,
		CLayoutInt(0),
		CLayoutInt(-1)
	);



	return true;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    空(から)行描画実装                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 空行を描画。EOFを描画した場合はtrueを返す。
bool _DispEmptyLine(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView)
{
	bool bEof=false;

	CTypeSupport cEofType(pcView,COLORIDX_EOF);
	CTypeSupport cTextType(pcView,COLORIDX_TEXT);

	const CLayoutInt nWrapKetas = pcView->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	int nYPrev = pDispPos->GetDrawPos().y;
	
	if( !pcView->IsBkBitmap() ){
		// 背景描画
		RECT rcClip;
		pcView->GetTextArea().GenerateClipRectLine(&rcClip,*pDispPos);
		cTextType.FillBack(gr,rcClip);
	}

	// EOF記号の表示
	CLayoutInt nCount = pcView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();

	// ドキュメントが空(nCount==0)。そして1行目(pDispPos->GetLayoutLineRef() == 0)。表示域も1行目(m_nViewTopLine==0)
	if( nCount == 0 && pcView->GetTextArea().GetViewTopLine() == 0 && pDispPos->GetLayoutLineRef() == 0 ){
		// EOF記号の表示
		if( cEofType.IsDisp() ){
			_DispEOF(gr,pDispPos,pcView);
		}

		bEof = true;
	}
	else{
		//最終行の次の行
		if( nCount > 0 && pDispPos->GetLayoutLineRef() == nCount ){
			//最終行の取得
			const wchar_t*	pLine;
			CLogicInt		nLineLen;
			const CLayout*	pcLayout;
			pLine = pcView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCount - CLayoutInt(1), &nLineLen, &pcLayout );
			
			//最終行の桁数
			CLayoutInt nLineCols = pcView->LineIndexToColmn( pcLayout, nLineLen );

			if( WCODE::IsLineDelimiter(pLine[nLineLen-1]) || nLineCols >= nWrapKetas ){
				// EOF記号の表示
				if( cEofType.IsDisp() ){
					_DispEOF(gr,pDispPos,pcView);
				}

				bEof = true;
			}
		}
	}

	// 2006.04.29 Moca 選択処理のため縦線処理を追加
	pcView->GetTextDrawer().DispVerticalLines(
		gr,
		nYPrev,
		nYPrev + pcView->GetTextMetrics().GetHankakuDy(),
		CLayoutInt(0),
		CLayoutInt(-1)
	);

	return bEof;
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
			ExtTextOutOption(),
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
			ExtTextOutOption(),
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
void _DispEOL(CGraphics& gr, DispPos* pDispPos, CEol cEol, const CEditView* pcView)
{
	RECT rcClip2;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,2)){

		// 色決定
		CTypeSupport cSupport(pcView,pcView->GetTextDrawer()._GetColorIdx(COLORIDX_EOL));
		cSupport.SetGraphicsState_WhileThisObj(gr);

		// 2003.08.17 ryoji 改行文字が欠けないように
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
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
			_DrawEOL(gr, rcEol, cEol, cSupport.IsFatFont(), cSupport.GetTextColor());

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
		sx = rcEol.left;						//X左端
		sy = rcEol.top + ( rcEol.Height() / 2);	//Y中心
		::MoveToEx( gr, sx + rcEol.Width(), sy - rcEol.Height() / 4, NULL );	//	上へ
		::LineTo(   gr, sx + rcEol.Width(), sy );			//	下へ
		::LineTo(   gr, sx, sy );					//	先頭へ
		::LineTo(   gr, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	先頭から下へ
		::MoveToEx( gr, sx, sy, NULL);				//	先頭へ戻り
		::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	先頭から上へ
		if ( bBold ) {
			::MoveToEx( gr, sx + rcEol.Width() + 1, sy - rcEol.Height() / 4, NULL );	//	上へ（右へずらす）
			++sy;
			::LineTo( gr, sx + rcEol.Width() + 1, sy );	//	右へ（右にひとつずれている）
			::LineTo(   gr, sx, sy );					//	先頭へ
			::LineTo(   gr, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	先頭から下へ
			::MoveToEx( gr, sx, sy, NULL);				//	先頭へ戻り
			::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	先頭から上へ
		}
		break;
	case EOL_CR:	//	左向き矢印	// 2007.08.17 ryoji EOL_LF -> EOL_CR
		sx = rcEol.left;
		sy = rcEol.top + ( rcEol.Height() / 2 );
		::MoveToEx( gr, sx + rcEol.Width(), sy, NULL );	//	右へ
		::LineTo(   gr, sx, sy );					//	先頭へ
		::LineTo(   gr, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	先頭から下へ
		::MoveToEx( gr, sx, sy, NULL);				//	先頭へ戻り
		::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	先頭から上へ
		if ( bBold ) {
			++sy;
			::MoveToEx( gr, sx + rcEol.Width(), sy, NULL );	//	右へ
			::LineTo(   gr, sx, sy );					//	先頭へ
			::LineTo(   gr, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	先頭から下へ
			::MoveToEx( gr, sx, sy, NULL);				//	先頭へ戻り
			::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	先頭から上へ
		}
		break;
	case EOL_LF:	//	下向き矢印	// 2007.08.17 ryoji EOL_CR -> EOL_LF
		sx = rcEol.left + ( rcEol.Width() / 2 );
		sy = rcEol.top + ( rcEol.Height() * 3 / 4 );
		::MoveToEx( gr, sx, rcEol.top + rcEol.Height() / 4 + 1, NULL );	//	上へ
		::LineTo(   gr, sx, sy );								//	上から下へ
		::LineTo(   gr, sx - rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	そのまま左上へ
		::MoveToEx( gr, sx, sy, NULL);							//	矢印の先端に戻る
		::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	そして右上へ
		if( bBold ){
			++sx;
			::MoveToEx( gr, sx, rcEol.top + rcEol.Height() / 4 + 1, NULL );
			::LineTo(   gr, sx, sy );								//	上から下へ
			::LineTo(   gr, sx - rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	そのまま左上へ
			::MoveToEx( gr, sx, sy, NULL);							//	矢印の先端に戻る
			::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	そして右上へ
		}
		break;
	}
}
