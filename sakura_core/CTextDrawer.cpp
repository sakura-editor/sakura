#include "stdafx.h"
#include "CTextDrawer.h"
#include <vector>
#include "CTextMetrics.h"
#include "CTextArea.h"
#include "CViewFont.h"
#include "CEol.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "CTypeSupport.h"
#include "charcode.h"
#include "CLayout.h"

const CTextArea* CTextDrawer::GetTextArea() const
{
	return &m_pEditView->GetTextArea();
}

using namespace std;

int CTextDrawer::_GetColorIdx(int nColorIdx,bool bSearchStringMode) const
{
	if(bSearchStringMode)return COLORIDX_SEARCH;                        //検索ヒット色
	if(CTypeSupport(m_pEditView,nColorIdx).IsDisp())return nColorIdx; //特殊色
	return 0;                                                           //通常色
}


/*
テキスト表示
@@@ 2002.09.22 YAZAKI    const unsigned char* pDataを、const char* pDataに変更
@@@ 2007.08.25 kobake 戻り値を void に変更。引数 x, y を DispPos に変更
*/
void CTextDrawer::DispText( HDC hdc, DispPos* pDispPos, const wchar_t* pData, int nLength ) const
{
	if( 0 >= nLength ){
		return;
	}
	int x=pDispPos->GetDrawPos().x;
	int y=pDispPos->GetDrawPos().y;

	//必要なインターフェースを取得
	const CTextMetrics* pMetrics=&m_pEditView->GetTextMetrics();
	const CTextArea* pArea=GetTextArea();

	//
	int nDy = m_pEditView->GetTextMetrics().GetHankakuDy();
	int nDx = m_pEditView->GetTextMetrics().GetHankakuDx();

	//文字間隔配列を生成
	vector<int> vDxArray;
	const int* pDxArray=pMetrics->GenerateDxArray(&vDxArray,pData,nLength,this->m_pEditView->GetTextMetrics().GetHankakuDx());

	//文字列のピクセル幅
	int nTextWidth=pMetrics->CalcTextWidth(pData,nLength,pDxArray);

	//テキストの描画範囲の矩形を求める -> rcClip
	CMyRect rcClip;
	rcClip.left   = x;
	rcClip.right  = x + nTextWidth;
	rcClip.top    = y;
	rcClip.bottom = y + nDy;
	if( rcClip.left < pArea->GetAreaLeft() ){
		rcClip.left = pArea->GetAreaLeft();
	}

	if( rcClip.left < rcClip.right
	 && rcClip.left < pArea->GetAreaRight() && rcClip.right > pArea->GetAreaLeft()
	 && rcClip.top >= pArea->GetAreaTop()
	){

		//@@@	From Here 2002.01.30 YAZAKI ExtTextOutW2の制限回避
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
			while (nBeforeLayout < nLeftLayout){
				nBeforeLayout += CNativeW2::GetKetaOfChar( pData, nLength, nBeforeLogic );
				nBeforeLogic  += CNativeW2::GetSizeOfChar( pData, nLength, nBeforeLogic );
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

		//描画
		::ExtTextOutW2(
			hdc,
			nDrawX,					//X
			y,						//Y
			ExtTextOutOption(),
			&rcClip,
			pDrawData,				//文字列
			nDrawLength,			//文字列長
			pDrawDxArray			//文字間隔の入った配列
		);
	}

end:
	//描画位置を進める
	pDispPos->ForwardDrawCol(nTextWidth / nDx);
}

void CTextDrawer::DispText( HDC hdc, int x, int y, const wchar_t* pData, int nLength ) const
{
	DispPos sPos(m_pEditView->GetTextMetrics().GetHankakuDx(),m_pEditView->GetTextMetrics().GetHankakuDy());
	sPos.InitDrawPos(CMyPoint(x,y));
	DispText(hdc,&sPos,pData,nLength);
}


/*!
EOF記号の描画
@date 2004.05.29 genta  MIKさんのアドバイスにより関数にくくりだし
@date 2007.08.28 kobake 引数 nCharWidth 削除
@date 2007.08.28 kobake 引数 fuOptions 削除
@date 2007.08.30 kobake 引数 EofColInfo 削除
*/
void CTextDrawer::DispEOF(
	HDC              hdc,      //!< [in] 描画対象のDevice Context
	DispPos*         pDispPos  //!< [in] 表示座標
) const
{
	// 描画に使う色情報
	CTypeSupport cEofType(m_pEditView,COLORIDX_EOF);

	//必要なインターフェースを取得
	const CTextMetrics* pMetrics=&m_pEditView->GetTextMetrics();
	const CTextArea* pArea=GetTextArea();

	//定数
	const wchar_t	szEof[] = L"[EOF]";
	const int		nEofLen = _countof(szEof) - 1;

	//クリッピング領域を計算
	RECT rcClip;
	if(pArea->GenerateClipRect(&rcClip,*pDispPos,nEofLen))
	{
		//色設定
		cEofType.SetColors(hdc);
		cEofType.SetFont(hdc);

		//描画
		::ExtTextOutW2(
			hdc,
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
//                         改行描画                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//	May 23, 2000 genta
/*!
画面描画補助関数:
行末の改行マークを改行コードによって書き分ける（メイン）

@note bBoldがTRUEの時は横に1ドットずらして重ね書きを行うが、
あまり太く見えない。

@date 2001.12.21 YAZAKI 改行記号の描きかたを変更。ペンはこの関数内で作るようにした。
						矢印の先頭を、sx, syにして描画ルーチン書き直し。
*/
void CTextDrawer::_DrawEOL(
	HDC      hdc,     //!< Device Context Handle
	int      nPosX,   //!< 描画座標X
	int      nPosY,   //!< 描画座標Y
	int      nWidth,  //!< 描画エリアのサイズX
	int      nHeight, //!< 描画エリアのサイズY
	CEOL     cEol,    //!< 行末コード種別
	bool     bBold,   //!< TRUE: 太字
	COLORREF pColor   //!< 色
) const
{
	int sx, sy;	//	矢印の先頭
	HANDLE	hPen;
	HPEN	hPenOld;
	hPen = ::CreatePen( PS_SOLID, 1, pColor );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	switch( cEol.GetType() ){
	case EOL_CRLF:	//	下左矢印
		sx = nPosX;
		sy = nPosY + ( nHeight / 2);
		::MoveToEx( hdc, sx + nWidth, sy - nHeight / 4, NULL );	//	上へ
		::LineTo(   hdc, sx + nWidth, sy );			//	下へ
		::LineTo(   hdc, sx, sy );					//	先頭へ
		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	先頭から下へ
		::MoveToEx( hdc, sx, sy, NULL);				//	先頭へ戻り
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	先頭から上へ
		if ( bBold ) {
			::MoveToEx( hdc, sx + nWidth + 1, sy - nHeight / 4, NULL );	//	上へ（右へずらす）
			++sy;
			::LineTo( hdc, sx + nWidth + 1, sy );	//	右へ（右にひとつずれている）
			::LineTo(   hdc, sx, sy );					//	先頭へ
			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	先頭から下へ
			::MoveToEx( hdc, sx, sy, NULL);				//	先頭へ戻り
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	先頭から上へ
		}
//		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//		::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy - ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL);
//		::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy - ( nHeight * 5 / 20 ) );
//		if( bBold ){
//			++sx;
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//			::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//			::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), nPosY + ( nHeight * 15/ 20) - ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL);
//			::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), nPosY + ( nHeight * 15/ 20) - ( nHeight * 5 / 20 ) );
//		}
		break;
	case EOL_CR:	//	左向き矢印	// 2007.08.17 ryoji EOL_LF -> EOL_CR
		sx = nPosX;
		sy = nPosY + ( nHeight / 2 );
		::MoveToEx( hdc, sx + nWidth, sy, NULL );	//	右へ
		::LineTo(   hdc, sx, sy );					//	先頭へ
		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	先頭から下へ
		::MoveToEx( hdc, sx, sy, NULL);				//	先頭へ戻り
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	先頭から上へ
		if ( bBold ) {
			++sy;
			::MoveToEx( hdc, sx + nWidth, sy, NULL );	//	右へ
			::LineTo(   hdc, sx, sy );					//	先頭へ
			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	先頭から下へ
			::MoveToEx( hdc, sx, sy, NULL);				//	先頭へ戻り
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	先頭から上へ
		}
//		::MoveToEx( hdc, nPosX, sy, NULL );
//		::LineTo(   hdc, nPosX + nWidth, sy );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4);
//		if( bBold ){
//			++sy;
//			::MoveToEx( hdc, nPosX, sy, NULL );
//			::LineTo(   hdc, nPosX + nWidth, sy );
//			::MoveToEx( hdc, sx, sy, NULL );
//			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4);
//		}
		break;
	case EOL_LF:	//	下向き矢印	// 2007.08.17 ryoji EOL_CR -> EOL_LF
		sx = nPosX + ( nWidth / 2 );
		sy = nPosY + ( nHeight * 3 / 4 );
		::MoveToEx( hdc, sx, nPosY + nHeight / 4 + 1, NULL );	//	上へ
		::LineTo(   hdc, sx, sy );								//	上から下へ
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	そのまま左上へ
		::MoveToEx( hdc, sx, sy, NULL);							//	矢印の先端に戻る
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	そして右上へ
		if( bBold ){
			++sx;
			::MoveToEx( hdc, sx, nPosY + nHeight / 4 + 1, NULL );
			::LineTo(   hdc, sx, sy );								//	上から下へ
			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	そのまま左上へ
			::MoveToEx( hdc, sx, sy, NULL);							//	矢印の先端に戻る
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	そして右上へ
		}
//#if 1
//		sx = nPosX + nWidth;
//		sy = nPosY + ( nHeight / 2 );
//		::MoveToEx( hdc, nPosX, sy, NULL );
//		::LineTo(   hdc, nPosX + nWidth, sy );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//		::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4);
//		if( bBold ){
//			++sy;
//			::MoveToEx( hdc, nPosX, sy, NULL );
//			::LineTo(   hdc, nPosX + nWidth, sy );
//			::MoveToEx( hdc, sx, sy, NULL );
//			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );
//			::MoveToEx( hdc, sx, nPosY + ( nHeight / 2 ), NULL );
//			::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4);
//		}
//#else
//		sx = nPosX;
//		::MoveToEx( hdc, sx + nWidth - 3, nPosY + nHeight * 1 / 4, NULL );
//		::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4);
//		::LineTo(   hdc, sx, nPosY + nHeight * 3 / 4 );
//		::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4 - nHeight / 4);
//		if( bBold ){
//			++sx;
//			::MoveToEx( hdc, sx + nWidth - 3, nPosY + nHeight * 1 / 4, NULL );
//			::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4);
//			::LineTo(   hdc, sx, nPosY + nHeight * 3 / 4 );
//			::LineTo(   hdc, sx + nWidth - 3, nPosY + nHeight * 3 / 4 - nHeight / 4);
//		}
//#endif
		break;
	case EOL_LFCR:
		sx = nPosX + ( nWidth / 2 );
		sy = nPosY + ( nHeight * 3 / 4 );
		::MoveToEx( hdc, sx + nWidth / 2, nPosY + nHeight / 4 + 1, NULL );	//	右上へ
		::LineTo(   hdc, sx, nPosY + nHeight / 4 + 1 );			//	右から左へ
		::LineTo(   hdc, sx, sy );								//	上から下へ
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	そのまま左上へ
		::MoveToEx( hdc, sx, sy, NULL);							//	矢印の先端に戻る
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	そして右上へ
		if( bBold ){
			::MoveToEx( hdc, sx + nWidth / 2, nPosY + nHeight / 4 + 2, NULL );	//	右上へ
			++sx;
			::LineTo(   hdc, sx, nPosY + nHeight / 4 + 2 );			//	右から左へ
			::LineTo(   hdc, sx, sy );								//	上から下へ
			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	そのまま左上へ
			::MoveToEx( hdc, sx, sy, NULL);							//	矢印の先端に戻る
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	そして右上へ
		}
//		sx = nPosX + ( nWidth / 2 );
//		sy = nPosY + ( nHeight * 1 / 4 );
//		::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//		::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, sy, NULL );
//		::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//		::MoveToEx( hdc, sx, sy, NULL);
//		::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//		if( bBold ){
//			++sx;
//			::MoveToEx( hdc, sx, nPosY + ( nHeight * 15 / 20 ), NULL );
//			::LineTo(   hdc, sx, nPosY + ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, sy, NULL );
//			::LineTo(   hdc, sx - ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//			::MoveToEx( hdc, sx, sy, NULL);
//			::LineTo(   hdc, sx + ( nHeight * 5 / 20 ), sy + ( nHeight * 5 / 20 ) );
//		}
		break;
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}


//2007.08.30 kobake 追加
void CTextDrawer::DispEOL(HDC hdc, DispPos* pDispPos, CEOL cEol, bool bSearchStringMode) const
{
	const CEditView* pView=m_pEditView;

	RECT rcClip2;
	if(pView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,2)){

		// 色決定
		CTypeSupport cSupport(pView,_GetColorIdx(COLORIDX_CRLF,bSearchStringMode));
		cSupport.SetFont(hdc);
		cSupport.SetColors(hdc);

		// 2003.08.17 ryoji 改行文字が欠けないように
		::ExtTextOutW2(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClip2,
			L"  ",
			2,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		// 改行記号の表示
		if( CTypeSupport(pView,COLORIDX_CRLF).IsDisp() ){
			int nPosX = pDispPos->GetDrawPos().x;
			int nPosY = pDispPos->GetDrawPos().y;
			
			// From Here 2003.08.17 ryoji 改行文字が欠けないように

			// リージョン作成、選択。
			HRGN hRgn;
			hRgn = ::CreateRectRgnIndirect(&rcClip2);
			::SelectClipRgn(hdc, hRgn);
			
			//@@@ 2001.12.21 YAZAKI
			_DrawEOL(
				hdc,
				nPosX + 1,
				nPosY,
				pView->GetTextMetrics().GetHankakuWidth(),
				pView->GetTextMetrics().GetHankakuHeight(),
				cEol,
				cSupport.IsFatFont(),
				cSupport.GetTextColor()
			);

			// リージョン破棄
			::SelectClipRgn(hdc, NULL);
			::DeleteObject(hRgn);
			
			// To Here 2003.08.17 ryoji 改行文字が欠けないように
		}
	}

	//描画位置を進める
	pDispPos->ForwardDrawCol(1);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         タブ描画                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispTab( HDC hdc, DispPos* pDispPos, int nColorIdx ) const
{
	DispPos& sPos=*pDispPos;
	const CEditView* pView=m_pEditView;

	//定数
	static const wchar_t* pszSPACES = L"        ";

	//必要なインターフェース
	const CTextMetrics* pMetrics=&m_pEditView->GetTextMetrics();
	const CTextArea* pArea=GetTextArea();
	Types* TypeDataPtr = &pView->m_pcEditDoc->GetDocumentAttribute();

	int nLineHeight = pMetrics->GetHankakuDy();
	int nCharWidth = pMetrics->GetHankakuDx();


	CTypeSupport cTabType(pView,COLORIDX_TAB);

	//	Sep. 22, 2002 genta 共通式のくくりだし
	//	Sep. 23, 2002 genta LayoutMgrの値を使う
	int tabDispWidth = (Int)pView->m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace( sPos.GetDrawCol() );

	// タブ記号を表示する
	RECT rcClip2;
	rcClip2.left = sPos.GetDrawPos().x;
	rcClip2.right = rcClip2.left + nCharWidth * tabDispWidth;
	if( rcClip2.left < pArea->GetAreaLeft() ){
		rcClip2.left = pArea->GetAreaLeft();
	}
	if( rcClip2.left < rcClip2.right &&
		rcClip2.left < pArea->GetAreaRight() && rcClip2.right > pArea->GetAreaLeft() ){

		rcClip2.top = sPos.GetDrawPos().y;
		rcClip2.bottom = sPos.GetDrawPos().y + nLineHeight;
		// TABを表示するか？
		if( cTabType.IsDisp() && 0 == TypeDataPtr->m_bTabArrow ){	//タブ通常表示	//@@@ 2003.03.26 MIK

			// サポートクラス
			CTypeSupport cSupport(pView,nColorIdx);

			// フォントを選ぶ
			cSupport.SetFont(hdc);
			cSupport.SetColors(hdc);

			//@@@ 2001.03.16 by MIK
			::ExtTextOutW2(
				hdc,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption(),
				&rcClip2,
				TypeDataPtr->m_szTabViewString,
				tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
				pMetrics->GetDxArray_AllHankaku()
			);
		}else{
			CTypeSupport cSearchType(pView,COLORIDX_SEARCH);
			if( nColorIdx == COLORIDX_SEARCH ){
				cSearchType.SetBkColor(hdc);
			}
			::ExtTextOutW2(
				hdc, sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption(),
				&rcClip2,
				pszSPACES,
				tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
				pMetrics->GetDxArray_AllHankaku()
			);
			cSearchType.RewindColors(hdc);
			
			//タブ矢印表示	//@@@ 2003.03.26 MIK
			if( cTabType.IsDisp()
			 && 0 != TypeDataPtr->m_bTabArrow
			 && rcClip2.left <= sPos.GetDrawPos().x ) // Apr. 1, 2003 MIK 行番号と重なる
			{
				_DrawTabArrow(
					hdc,
					sPos.GetDrawPos().x,
					sPos.GetDrawPos().y,
					pMetrics->GetHankakuWidth(),
					pMetrics->GetHankakuHeight(),
					cTabType.IsFatFont(),
					cTabType.GetTextColor()
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
void CTextDrawer::_DrawTabArrow(
	HDC hdc,
	int nPosX,   //ピクセルX
	int nPosY,   //ピクセルY
	int nWidth,  //ピクセルW
	int nHeight, //ピクセルH
	int bBold,
	COLORREF pColor
) const
{
	HPEN hPen    = ::CreatePen( PS_SOLID, 1, pColor );
	HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );

	nWidth--;

	//	矢印の先頭
	int sx = nPosX + nWidth;
	int sy = nPosY + ( nHeight / 2 );

	::MoveToEx( hdc, sx - nWidth, sy, NULL );				//	左へ
	::LineTo(   hdc, sx, sy );								//	最後へ
	::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	最後から下へ
	::MoveToEx( hdc, sx, sy, NULL);							//	最後へ戻り
	::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	最後から上へ
	if ( bBold ) {
		++sy;
		::MoveToEx( hdc, sx - nWidth, sy, NULL );				//	左へ
		::LineTo(   hdc, sx, sy );								//	最後へ
		::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	最後から下へ
		::MoveToEx( hdc, sx, sy, NULL);							//	最後へ戻り
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	最後から上へ
	}

	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       スペース描画                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispZenkakuSpace( HDC hdc, DispPos* pDispPos, bool bSearchStringMode ) const
{
	//クリッピング矩形を計算。画面外なら描画しない
	RECT rcClip2;
	if(GetTextArea()->GenerateClipRect(&rcClip2,*pDispPos,2))
	{
		// 色決定
		CTypeSupport cSupport(m_pEditView,_GetColorIdx(COLORIDX_ZENSPACE,bSearchStringMode));
		cSupport.SetFont(hdc);
		cSupport.SetColors(hdc);

		//描画文字列
		const wchar_t* szZenSpace =
			CTypeSupport(m_pEditView,COLORIDX_ZENSPACE).IsDisp()?L"□":L"　";

		//描画
		::ExtTextOutW2(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClip2,
			szZenSpace,
			wcslen(szZenSpace),
			m_pEditView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
	}

	//位置進める
	pDispPos->ForwardDrawCol(2);
}

void CTextDrawer::DispHankakuSpace( HDC hdc, DispPos* pDispPos, bool bSearchStringMode) const
{
	//クリッピング矩形を計算。画面外なら描画しない
	CMyRect rcClip;
	if(m_pEditView->GetTextArea().GenerateClipRect(&rcClip,*pDispPos,1))
	{
		// 色決定
		CTypeSupport cSupport(m_pEditView,_GetColorIdx(COLORIDX_SPACE,bSearchStringMode));
		cSupport.SetFont(hdc);
		cSupport.SetColors(hdc);
		
		//小文字"o"の下半分を出力
		CMyRect rcClipBottom=rcClip;
		rcClipBottom.top=rcClip.top+rcClip.Height()/2;
		::ExtTextOutW2(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClipBottom,
			L"o",
			1,
			m_pEditView->GetTextMetrics().GetDxArray_AllHankaku()
		);
		
		//上半分は普通の空白で出力（"o"の上半分を消す）
		CMyRect rcClipTop=rcClip;
		rcClipTop.bottom=rcClip.top+rcClip.Height()/2;
		::ExtTextOutW2(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClipTop,
			L" ",
			1,
			m_pEditView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	//位置進める
	pDispPos->ForwardDrawCol(1);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       折り返し描画                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispWrap(HDC hdc, DispPos* pDispPos) const
{
	const CEditView* pView=m_pEditView;

	RECT rcClip2;
	if(pView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,1))
	{
		//サポートクラス
		CTypeSupport cWrapType(pView,COLORIDX_WRAP);

		//描画文字列と色の決定
		const wchar_t* szText;
		if( cWrapType.IsDisp() )
		{
			szText = L"<";
			cWrapType.SetFont(hdc);
			cWrapType.SetColors(hdc);
		}
		else
		{
			szText = L" ";
		}

		//描画
		::ExtTextOutW2(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClip2,
			szText,
			wcslen(szText),
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      空(から)行描画                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 空行を描画。EOFを描画した場合はtrueを返す。
bool CTextDrawer::DispEmptyLine(HDC hdc, DispPos* pDispPos) const
{
	bool bEof=false;

	const CEditView* pView=m_pEditView;
	CTypeSupport cEofType(pView,COLORIDX_EOF);
	CTypeSupport cTextType(pView,COLORIDX_TEXT);

	const CLayoutInt nWrapKetas = pView->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	int nYPrev = pDispPos->GetDrawPos().y;
	
	if( m_pEditView->IsBkBitmap() ){
	}else{
		// 背景描画
		RECT rcClip;
		pView->GetTextArea().GenerateClipRectLine(&rcClip,*pDispPos);
		cTextType.FillBack(hdc,rcClip);
	}

	// EOF記号の表示
	CLayoutInt nCount = pView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();

	// ドキュメントが空(nCount==0)。そして1行目(pDispPos->GetLayoutLineRef() == 0)。表示域も1行目(m_nViewTopLine==0)
	if( nCount == 0 && pView->GetTextArea().GetViewTopLine() == 0 && pDispPos->GetLayoutLineRef() == 0 ){
		// EOF記号の表示
		if( cEofType.IsDisp() ){
			DispEOF(hdc,pDispPos);
		}

		//描画Y位置進める
		pDispPos->ForwardDrawLine(1);
		bEof = true;
	}
	else{
		//最終行の次の行
		if( nCount > 0 && pDispPos->GetLayoutLineRef() == nCount ){
			//最終行の取得
			const wchar_t*	pLine;
			CLogicInt		nLineLen;
			const CLayout*	pcLayout;
			pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCount - CLayoutInt(1), &nLineLen, &pcLayout );
			
			//最終行の桁数
			CLayoutInt nLineCols = pView->LineIndexToColmn( pcLayout, nLineLen );

			if( WCODE::isLineDelimiter(pLine[nLineLen-1]) || nLineCols >= nWrapKetas ){
				// EOF記号の表示
				if( cEofType.IsDisp() ){
					DispEOF(hdc,pDispPos);
				}

				//描画Y位置進める
				pDispPos->ForwardDrawLine(1);
				bEof = true;
			}
		}
	}

	// 2006.04.29 Moca 選択処理のため縦線処理を追加
	DispVerticalLines( hdc, nYPrev, nYPrev + pView->GetTextMetrics().GetHankakuDy(),  CLayoutInt(0), CLayoutInt(-1) );

	return bEof;
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
	HDC			hdc,		//!< 作画するウィンドウのDC
	int			nTop,		//!< 線を引く上端のクライアント座標y
	int			nBottom,	//!< 線を引く下端のクライアント座標y
	CLayoutInt	nLeftCol,	//!< 線を引く範囲の左桁の指定
	CLayoutInt	nRightCol	//!< 線を引く範囲の右桁の指定(-1で未指定)
) const
{
	const CEditView* pView=m_pEditView;

	const Types&	typeData = pView->m_pcEditDoc->GetDocumentAttribute();

	CTypeSupport cVertType(pView,COLORIDX_VERTLINE);
	CTypeSupport cTextType(pView,COLORIDX_TEXT);

	if(!cVertType.IsDisp())return;

	nLeftCol = t_max( pView->GetTextArea().GetViewLeftCol(), nLeftCol );

	const CLayoutInt nWrapKetas  = pView->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
	const int nCharDx  = pView->GetTextMetrics().GetHankakuDx();
	if( nRightCol < 0 ){
		nRightCol = nWrapKetas;
	}
	const int nPosXOffset = pView->m_pShareData->m_Common.m_sWindow.m_nVertLineOffset + pView->GetTextArea().GetAreaLeft();
	const int nPosXLeft   = max( pView->GetTextArea().GetAreaLeft() + (Int)(nLeftCol  - pView->GetTextArea().GetViewLeftCol()) * nCharDx, pView->GetTextArea().GetAreaLeft() );
	const int nPosXRight  = min( pView->GetTextArea().GetAreaLeft() + (Int)(nRightCol - pView->GetTextArea().GetViewLeftCol()) * nCharDx, pView->GetTextArea().GetAreaRight() );
	const int nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	bool bOddLine = ((((nLineHeight % 2) ? (Int)pView->GetTextArea().GetViewTopLine() : 0) + pView->GetTextArea().GetAreaTop() + nTop) % 2 == 1);

	// 太線
	const bool bBold = cVertType.IsFatFont();
	// ドット線(下線属性を転用/テスト用)
	const bool bDot = cVertType.HasUnderLine();
	const bool bExorPen = ( cVertType.GetTextColor() == cTextType.GetBackColor() );
	HPEN hPen;
	int nROP_Old = 0;
	if( bExorPen ){
		hPen = ::CreatePen( PS_SOLID, 0, cVertType.GetBackColor() );
		nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
	}else{
		hPen = ::CreatePen( PS_SOLID, 0, cVertType.GetTextColor() );
	}
	HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );

	int k;
	for( k = 0; k < MAX_VERTLINES && typeData.m_nVertLineIdx[k] != 0; k++ ){
		// nXColは1開始。GetTextArea().GetViewLeftCol()は0開始なので注意。
		CLayoutInt nXCol = typeData.m_nVertLineIdx[k];
		CLayoutInt nXColEnd = nXCol;
		CLayoutInt nXColAdd = CLayoutInt(1);
		// nXColがマイナスだと繰り返し。k+1を終了値、k+2をステップ幅として利用する
		if( nXCol < 0 ){
			if( k < MAX_VERTLINES - 2 ){
				nXCol = -nXCol;
				nXColEnd = typeData.m_nVertLineIdx[++k];
				nXColAdd = typeData.m_nVertLineIdx[++k];
				if( nXColEnd < nXCol || nXColAdd <= 0 ){
					continue;
				}
				// 作画範囲の始めまでスキップ
				if( nXCol < pView->GetTextArea().GetViewLeftCol() ){
					nXCol = pView->GetTextArea().GetViewLeftCol() + nXColAdd - (pView->GetTextArea().GetViewLeftCol() - nXCol) % nXColAdd;
				}
			}else{
				k += 2;
				continue;
			}
		}
		for(; nXCol <= nXColEnd; nXCol += nXColAdd ){
			if( nWrapKetas < nXCol ){
				break;
			}
			int nPosX = nPosXOffset + (Int)( nXCol - 1 - pView->GetTextArea().GetViewLeftCol() ) * nCharDx;
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
							::MoveToEx( hdc, nPosX, y, NULL );
							::LineTo( hdc, nPosX, y + 1 );
						}
						if( bBold && nPosXLeft <= nPosXBold ){
							::MoveToEx( hdc, nPosXBold, y, NULL );
							::LineTo( hdc, nPosXBold, y + 1 );
						}
					}
				}else{
					if( nPosX < nPosXRight ){
						::MoveToEx( hdc, nPosX, nTop, NULL );
						::LineTo( hdc, nPosX, nBottom );
					}
					if( bBold && nPosXLeft <= nPosXBold ){
						::MoveToEx( hdc, nPosXBold, nTop, NULL );
						::LineTo( hdc, nPosXBold, nBottom );
					}
				}
			}
		}
	}
	if( bExorPen ){
		::SetROP2( hdc, nROP_Old );
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          行番号                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispLineNumber(
	HDC						hdc,
	const CLayout*			pcLayout,
	int						nLineNum,
	int						y
) const
{
	const CEditView* pView=m_pEditView;
	const Types* pTypes=&pView->m_pcEditDoc->GetDocumentAttribute();

	int				nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int				nCharWidth = pView->GetTextMetrics().GetHankakuDx();
	const CDocLine*	pCDocLine;
	//	Sep. 23, 2002 genta 共通式のくくりだし
	int				nLineNumAreaWidth = pView->GetTextArea().m_nViewAlignLeftCols * nCharWidth;

	CTypeSupport cTextType(pView,COLORIDX_TEXT);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     nColorIndexを決定                       //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int nColorIndex = COLORIDX_GYOU;	/* 行番号 */
	if( NULL != pcLayout ){
		pCDocLine = pcLayout->m_pCDocLine;

		if( pView->m_pcEditDoc->IsModified()	/* ドキュメントが無変更の状態か */
		 && pCDocLine->IsModifyed() ){		/* 変更フラグ */
			if( CTypeSupport(pView,COLORIDX_GYOU_MOD).IsDisp() )	// 2006.12.12 ryoji
				nColorIndex = COLORIDX_GYOU_MOD;	/* 行番号（変更行） */
		}
	}

	int type = pCDocLine->IsDiffMarked();
	{
		//DIFF差分マーク表示	//@@@ 2002.05.25 MIK
		if( type )
		{
			switch( type )
			{
			case MARK_DIFF_APPEND:	//追加
				if( CTypeSupport(pView,COLORIDX_DIFF_APPEND).IsDisp() )
					nColorIndex = COLORIDX_DIFF_APPEND;
				break;
			case MARK_DIFF_CHANGE:	//変更
				if( CTypeSupport(pView,COLORIDX_DIFF_CHANGE).IsDisp() )
					nColorIndex = COLORIDX_DIFF_CHANGE;
				break;
			case MARK_DIFF_DELETE:	//削除
			case MARK_DIFF_DEL_EX:	//削除
				if( CTypeSupport(pView,COLORIDX_DIFF_DELETE).IsDisp() )
					nColorIndex = COLORIDX_DIFF_DELETE;
				break;
			}
		}
	}

	// 02/10/16 ai
	// ブックマークの表示
	if(pCDocLine->IsBookMarked()){
		if( CTypeSupport(pView,COLORIDX_MARK).IsDisp() ) {
			nColorIndex = COLORIDX_MARK;
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             決定されたnColorIndexを使って描画               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	CTypeSupport cColorType(pView,nColorIndex);
	CTypeSupport cMarkType(pView,COLORIDX_MARK);

	if( CTypeSupport(pView,COLORIDX_GYOU).IsDisp() ){ /* 行番号表示／非表示 */
		wchar_t szLineNum[18];

		/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		if( pTypes->m_bLineNumIsCRLF ){
			/* 論理行番号表示モード */
			if( NULL == pcLayout || 0 != pcLayout->GetLogicOffset() ){
				wcscpy( szLineNum, L" " );
			}else{
				_itow( pcLayout->GetLogicLineNo() + 1, szLineNum, 10 );	/* 対応する論理行番号 */
			}
		}else{
			/* 物理行（レイアウト行）番号表示モード */
			_itow( nLineNum + 1, szLineNum, 10 );
		}

		int nLineCols = wcslen( szLineNum );

		//色、フォント
		cTextType.SetBkColor(hdc);    //背景色：テキストの背景色 //	Sep. 23, 2002 余白をテキストの背景色にする
		cColorType.SetTextColor(hdc); //テキスト：行番号の色
		cColorType.SetFont(hdc);      //フォント：行番号のフォント

		// 余白を埋める
		RECT	rcClip;
		rcClip.left   = nLineNumAreaWidth;
		rcClip.right  = pView->GetTextArea().GetAreaLeft();
		rcClip.top    = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOutW2(
			hdc,
			rcClip.left,
			y,
			ExtTextOutOption(),
			&rcClip,
			L" ",
			1,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
		
		//	Sep. 23, 2002 余白をテキストの背景色にするため，背景色の設定を移動
		SetBkColor( hdc, cColorType.GetBackColor() );		/* 行番号背景の色 */

		int drawNumTop = (pView->GetTextArea().m_nViewAlignLeftCols - nLineCols - 1) * ( nCharWidth );

		/* 行番号区切り 0=なし 1=縦線 2=任意 */
		if( 2 == pTypes->m_nLineTermType ){
			//	Sep. 22, 2002 genta
			szLineNum[ nLineCols ] = pTypes->m_cLineTermChar;
			szLineNum[ ++nLineCols ] = '\0';
		}
		rcClip.left = 0;

		//	Sep. 23, 2002 genta
		rcClip.right = nLineNumAreaWidth;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOutW2( hdc,
			drawNumTop,
			y,
			ExtTextOutOption(),
			&rcClip,
			szLineNum,
			nLineCols,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		/* 行番号区切り 0=なし 1=縦線 2=任意 */
		if( 1 == pTypes->m_nLineTermType ){
			cColorType.SetSolidPen(hdc,0);
			::MoveToEx( hdc, nLineNumAreaWidth - 2, y, NULL );
			::LineTo( hdc, nLineNumAreaWidth - 2, y + nLineHeight );
			cColorType.RewindPen(hdc);
		}
		cColorType.RewindColors(hdc);
		cTextType.RewindColors(hdc);
		cColorType.RewindFont(hdc);
	}
	else{
		RECT	rcClip;
		// 行番号エリアの背景描画
		rcClip.left   = 0;
		rcClip.right  = pView->GetTextArea().GetAreaLeft() - pView->m_pShareData->m_Common.m_sWindow.m_nLineNumRightSpace; //	Sep. 23 ,2002 genta 余白はテキスト色のまま残す
		rcClip.top    = y;
		rcClip.bottom = y + nLineHeight;
		cColorType.FillBack(hdc,rcClip);
		
		// Mar. 5, 2003, Moca
		// 行番号とテキストの隙間の描画
		rcClip.left   = rcClip.right;
		rcClip.right  = pView->GetTextArea().GetAreaLeft();
		rcClip.top    = y;
		rcClip.bottom = y + nLineHeight;
		cTextType.FillBack(hdc,rcClip);
	}

	// 2001.12.03 hor
	/* とりあえずブックマークに縦線 */
	if(pCDocLine->IsBookMarked() && !cMarkType.IsDisp() )
	{
		cColorType.SetSolidPen(hdc,2);
		::MoveToEx( hdc, 1, y, NULL );
		::LineTo( hdc, 1, y + nLineHeight );
		cColorType.RewindPen(hdc);
	}

	if( type )	//DIFF差分マーク表示	//@@@ 2002.05.25 MIK
	{
		int	cy = y + nLineHeight / 2;

		cColorType.SetSolidPen(hdc,1);

		switch( type )
		{
		case MARK_DIFF_APPEND:	//追加
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo  ( hdc, 6, cy );
			::MoveToEx( hdc, 4, cy - 2, NULL );
			::LineTo  ( hdc, 4, cy + 3 );
			break;

		case MARK_DIFF_CHANGE:	//変更
			::MoveToEx( hdc, 3, cy - 4, NULL );
			::LineTo  ( hdc, 3, cy );
			::MoveToEx( hdc, 3, cy + 2, NULL );
			::LineTo  ( hdc, 3, cy + 3 );
			break;

		case MARK_DIFF_DELETE:	//削除
			cy -= 3;
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo  ( hdc, 5, cy );
			::LineTo  ( hdc, 3, cy + 2 );
			::LineTo  ( hdc, 3, cy );
			::LineTo  ( hdc, 7, cy + 4 );
			break;
		
		case MARK_DIFF_DEL_EX:	//削除(EOF)
			cy += 3;
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo  ( hdc, 5, cy );
			::LineTo  ( hdc, 3, cy - 2 );
			::LineTo  ( hdc, 3, cy );
			::LineTo  ( hdc, 7, cy - 4 );
			break;
		}

		cColorType.RewindPen(hdc);
	}
	
	return;
}

