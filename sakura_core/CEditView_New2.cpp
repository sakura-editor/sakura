//	$Id$
/*!	@file
	@brief CEditViewクラス

	@author Norio Nakatani
	@date	1998/12/08 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, mik, asa-o
	Copyright (C) 2001, hor, MIK 
	Copyright (C) 2002, YAZAKI, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditView.h"
#include "debug.h"
//#include "keycode.h"
#include "funccode.h"
//#include "CRunningTimer.h" 2002/2/10 aroka
#include "charcode.h"
#include "mymessage.h"
//#include "CWaitCursor.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
#include "etc_uty.h"
#include "Clayout.h" /// 2002/2/3 aroka
#include "CDocLine.h" /// 2002/2/3 aroka
#include "CMarkMgr.h" /// 2002/2/3 aroka
#include "my_icmp.h"	//@@@ 2002.01.13 add


/* 現在の色を指定 */
void CEditView::SetCurrentColor( HDC hdc, int nCOMMENTMODE )
{
	int				nColorIdx;
	COLORREF		colText;
	COLORREF		colBack;
//	if( NULL != m_hFontOld ){
//		::SelectObject( hdc, m_hFontOld );
//		m_hFontOld = NULL;
//	}
	nColorIdx = -1;
	switch( nCOMMENTMODE ){
// 2002/03/13 novice
	case COLORIDX_TEXT: /* テキスト */
	case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */
	case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */
	case COLORIDX_CTRLCODE:	/* コントロールコードである */
	case COLORIDX_DIGIT:	/* 半角数値である */
	case COLORIDX_KEYWORD1:	/* 強調キーワード1（登録単語）文字列である */
	case COLORIDX_KEYWORD2:	/* 強調キーワード2（登録単語）文字列である */
	case COLORIDX_URL:	    /* URLである */
	case COLORIDX_SEARCH:	/* 検索文字列である */
		nColorIdx = nCOMMENTMODE;
		break;
	case COLORIDX_COMMENT:	/* 行コメントである */
	case COLORIDX_BLOCK1:	/* ブロックコメント1である */
	case COLORIDX_BLOCK2:	/* ブロックコメント2である */	//@@@ 2001.03.10 by MIK
		nColorIdx = COLORIDX_COMMENT;
		break;
#if 0
	case 0:
		nColorIdx = COLORIDX_TEXT;
//		colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colTEXT;	/* テキスト色 */
//		colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK;	/* 背景色 */
//		::SetTextColor( hdc, colText );
//		::SetBkColor( hdc, colBack );
//		if( NULL != m_hFontOld ){
//			::SelectObject( hdc, m_hFontOld );
//			m_hFontOld = NULL;
//		}

		break;
	case 1:	/* 行コメントである */
	case 2:	/* ブロックコメントである */
//#ifdef	COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	case 20:	/* ブロックコメントである */	//@@@ 2001.03.10 by MIK
//#endif
		nColorIdx = COLORIDX_COMMENT;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp ){	/* コメントを表示する */
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_COMMENT].m_colTEXT;	/* コメント色 */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_COMMENT].m_colBACK;	/* コメント背景の色 */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_COMMENT].m_bFatFont ){	/* 太字か */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
	case 3:	/* シングルクォーテーション文字列である */
		nColorIdx = COLORIDX_SSTRING;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp ){	/* シングルクォーテーション文字列を表示する */
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_colTEXT;	/* シングルクォーテーション文字列色 */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_colBACK;	/* シングルクォーテーション文字列背景の色 */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SSTRING].m_bFatFont ){	/* 太字か */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
	case 4:	/* ダブルクォーテーション文字列である */
		nColorIdx = COLORIDX_WSTRING;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp ){	/* ダブルクォーテーション文字列を表示する */
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_colTEXT;	/* ダブルクォーテーション文字列色 */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_colBACK;	/* ダブルクォーテーション文字列背景の色 */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_WSTRING].m_bFatFont ){	/* 太字か */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
	case 5:	/* キーワード（登録単語）文字列である */
		nColorIdx = COLORIDX_KEYWORD;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_KEYWORD].m_bDisp ){
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_KEYWORD].m_colTEXT;	/* 強調キーワードの色 */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_KEYWORD].m_colBACK;	/* 強調キーワード背景の色 */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_KEYWORD].m_bFatFont ){	/* 太字か */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
	case 6:	/* コントロールコードである */
		nColorIdx = COLORIDX_CTRLCODE;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp ){
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CTRLCODE].m_colTEXT;	/* コントロールコードの色 */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CTRLCODE].m_colBACK;	/* コントロールコードの背景色 */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CTRLCODE].m_bFatFont ){	/* 太字か */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
//@@@ 2001.02.17 Start by MIK: 半角数値を強調表示
//#ifdef COMPILE_COLOR_DIGIT
	case 9:	/* 半角数値である */
		nColorIdx = COLORIDX_DIGIT;
		break;
//#endif
//@@@ 2001.02.17 End by MIK: 半角数値を強調表示
	case 50:	/* キーワード2（登録単語）文字列である */	//MIK
		nColorIdx = COLORIDX_KEYWORD2;						//MIK
		break;												//MIK
	case 80:	/* URLである */
		nColorIdx = COLORIDX_URL;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_bDisp ){
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_colTEXT;	/* URL */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_colBACK;	/* URL */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_bFatFont ){	/* URL */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
	case 90:	/* 検索文字列である */
		nColorIdx = COLORIDX_SEARCH;
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
//			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SEARCH].m_colTEXT;	/* URL */
//			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SEARCH].m_colBACK;	/* URL */
//			::SetTextColor( hdc, colText );
//			::SetBkColor( hdc, colBack );
//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_SEARCH].m_bFatFont ){	/* URL */
//				if( NULL != m_hFontOld ){
//					::SelectObject( hdc, m_hFontOld );
//				}
//				m_hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//			}
//		}
		break;
#endif

//@@@ 2001.11.17 add start MIK
	default:	/* 正規表現キーワード */
		if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 )
		{
			nColorIdx = nCOMMENTMODE - 1000;	//下駄を履かせているのをはずす
			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* フォントを選ぶ */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				ChooseFontHandle(
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
			return;
		}
		break;
//@@@ 2001.11.17 add end MIK

	}





	if( -1 != nColorIdx ){
		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bDisp ){
			colText = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colTEXT;
			colBack = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_colBACK;
			::SetTextColor( hdc, colText );
			::SetBkColor( hdc, colBack );
			if( NULL != m_hFontOld ){
				::SelectObject( hdc, m_hFontOld );
			}
			/* フォントを選ぶ */
			m_hFontOld = (HFONT)::SelectObject( hdc,
				ChooseFontHandle(
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bFatFont,
					m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIdx].m_bUnderLine
				)
			);
		}
	}

	return;
}


/* 行番号表示 */
void CEditView::DispLineNumber(
		HDC						hdc,
		const CLayout*			pcLayout,
		int						nLineNum,
		int						y
)
{
	RECT			rcClip;
	HBRUSH			hBrush;
	COLORREF		colTextColorOld;
	COLORREF		colBkColorOld;
	char			szLineNum[18];
	int				nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	int				nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
	int				nLineCols;
	UINT			fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	HPEN			hPen, hPenOld;
	int				nColorIndex;
	const CDocLine*	pCDocLine;

	nColorIndex = COLORIDX_GYOU;	/* 行番号 */
	if( NULL != pcLayout ){
//		pCDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( pcLayout->m_nLinePhysical );
		pCDocLine = pcLayout->m_pCDocLine;

		if( m_pcEditDoc->IsModified()	/* ドキュメントが無変更の状態か */
		 && pCDocLine->IsModifyed() ){		/* 変更フラグ */
//			if( 0 == pCDocLine->m_nModifyCount ){	/* 変更回数 */
				nColorIndex = COLORIDX_GYOU_MOD;	/* 行番号（変更行） */
//			}
//		}else{
//			if( /* FALSE == m_pcEditDoc->IsModified() && --*/ /* ドキュメントが無変更の状態か */
//				0 < pCDocLine->m_nModifyCount	/* 変更回数 */
//			){
//				nColorIndex = COLORIDX_GYOU_MODSAVE;	/* 行番号（変更&保存済） */
//			}
		}
	}

//	if( m_pcEditDoc->GetDocumentAttribute().m_bDispLINE ){	/* 行番号表示／非表示 */
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[/*nColorIndex*/COLORIDX_GYOU].m_bDisp ){	/* 行番号表示／非表示 */
		/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		if( m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){
			/* 論理行番号表示モード */
			if( NULL == pcLayout || 0 != pcLayout->m_nOffset ){
				strcpy( szLineNum, " " );
			}else{
				_itoa( pcLayout->m_nLinePhysical + 1, szLineNum, 10 );	/* 対応する論理行番号 */
			}
		}else{
			/* 物理行（レイアウト行）番号表示モード */
			_itoa( nLineNum + 1, szLineNum, 10 );
		}
		nLineCols = lstrlen( szLineNum );

		colTextColorOld = ::SetTextColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );	/* 行番号の色 */
		colBkColorOld = ::SetBkColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colBACK );		/* 行番号背景の色 */

		HFONT	hFontOld;
		/* フォントを選ぶ */
		hFontOld = (HFONT)::SelectObject( hdc,
			ChooseFontHandle(
				m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_bFatFont,
				m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_bUnderLine
			)
		);
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_bFatFont ){	/* 太字か */
//			hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
//		}else{
//			hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//		}
		/* 余白を埋める */
		rcClip.left = m_nViewAlignLeft - 3;
		rcClip.right = m_nViewAlignLeft;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOut( hdc,
			(m_nViewAlignLeftCols - nLineCols - 1) * ( nCharWidth ),
			y, fuOptions,
			&rcClip, " ", 1, m_pnDx
		);


//		/* 行番号のテキストを表示 */
//		m_pShareData->m_Types[nIdx].m_nLineTermType = 1;			/* 行番号区切り 0=なし 1=縦線 2=任意 */
//		m_pShareData->m_Types[nIdx].m_cLineTermChar = ':';			/* 行番号区切り文字 */

		/* 行番号区切り 0=なし 1=縦線 2=任意 */
		if( 2 == m_pcEditDoc->GetDocumentAttribute().m_nLineTermType ){
			char szLineTerm[2];
			wsprintf( szLineTerm, "%c", m_pcEditDoc->GetDocumentAttribute().m_cLineTermChar );	/* 行番号区切り文字 */
			strcat( szLineNum, szLineTerm );
		}
		rcClip.left = 0;
		rcClip.right = m_nViewAlignLeft/* - 3*/;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOut( hdc,
			(m_nViewAlignLeftCols - nLineCols - 1) * ( nCharWidth ),
			y, fuOptions,
			&rcClip, szLineNum, lstrlen( szLineNum ), m_pnDx
		);


//		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_colorGYOU );


		/* 行番号区切り 0=なし 1=縦線 2=任意 */
		if( 1 == m_pcEditDoc->GetDocumentAttribute().m_nLineTermType ){
			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );
			hPenOld = (HPEN)::SelectObject( hdc, hPen );
			::MoveToEx( hdc, m_nViewAlignLeft - 4, y, NULL );
			::LineTo( hdc, m_nViewAlignLeft - 4, y + nLineHeight );
			::SelectObject( hdc, hPenOld );
			::DeleteObject( hPen );
		}
		::SetTextColor( hdc, colTextColorOld );
		::SetBkColor( hdc, colBkColorOld );

//		colBkColorOld = ::SetBkColor( hdc, RGB( 255, 0, 0 ) );






//		::SetBkColor( hdc, colBkColorOld );


		::SelectObject( hdc, hFontOld );
	}else{
		rcClip.left = 0;
		rcClip.right = m_nViewAlignLeft;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
//		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_colorBACK );
		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex/*COLORIDX_TEXT*/].m_colBACK );
		::FillRect( hdc, &rcClip, hBrush );
		::DeleteObject( hBrush );
	}

// From Here 2001.12.03 hor
	/* とりあえずブックマークに縦線 */
	if(pCDocLine->IsBookMarked()){
		hPen = ::CreatePen( PS_SOLID, 2, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[nColorIndex].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, 1, y, NULL );
		::LineTo( hdc, 1, y + nLineHeight );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
	}
// To Here 2001.12.03 hor

	return;
}




/* テキスト表示 */
int CEditView::DispText( HDC hdc, int x, int y, const unsigned char* pData, int nLength )
{
	if( 0 >= nLength ){
		return 0;
	}
	RECT	rcClip;
	int		nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	int		nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
	UINT	fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	rcClip.left = x;
	rcClip.right = rcClip.left + ( nCharWidth ) * nLength;
	rcClip.top = y;
	if( rcClip.left < m_nViewAlignLeft ){
		rcClip.left = m_nViewAlignLeft;
	}
	if( rcClip.left < rcClip.right
	 && rcClip.left < m_nViewAlignLeft + m_nViewCx && rcClip.right > m_nViewAlignLeft
	 && rcClip.top >= m_nViewAlignTop
	){
		rcClip.bottom = y + nLineHeight;
//		::ExtTextOut( hdc, x, y, fuOptions, &rcClip, (const char *)pData, nLength, m_pnDx );
		//@@@	From Here 2002.01.30 YAZAKI ExtTextOutの制限回避
		if( rcClip.right - rcClip.left > m_nViewCx ){
			rcClip.right = rcClip.left + m_nViewCx;
		}
		int nBefore = 0;	//	ウィンドウの左にあふれた文字数
		int nAfter = 0;		//	ウィンドウの右にあふれた文字数
		if ( x < 0 ){
			int nLeft = ( 0 - x ) / nCharWidth - 1;
			while (nBefore < nLeft){
				nBefore += CMemory::MemCharNext( (const char *)pData, nLength, (const char *)&pData[nBefore] ) - (const char *)&pData[nBefore];
			}
		}
		if ( rcClip.right < x + nCharWidth * nLength ){
			//	-1してごまかす（うしろはいいよね？）
			nAfter = (x + nCharWidth * nLength - rcClip.right) / nCharWidth - 1;
		}
		::ExtTextOut( hdc, x + nBefore * nCharWidth, y, fuOptions, &rcClip, (const char *)&pData[nBefore], nLength - nBefore - nAfter, m_pnDx );
		//@@@	To Here 2002.01.30 YAZAKI ExtTextOutの制限回避
	}
	return nLength;

}


/* テキスト反転 */
void CEditView::DispTextSelected( HDC hdc, int nLineNum, int x, int y, int nX  )
{
//	MYTRACE( "CEditView::DispTextSelected()\n" );

	int			nROP_Old;
	int			nSelectFrom;
	int			nSelectTo;
	RECT		rcClip;
//	HPEN		hPen;
//	HPEN		hPenOld;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	int			nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	int			nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
	HRGN		hrgnDraw;

	/* 選択範囲内の行かな */
//	if( IsTextSelected() ){
		if( nLineNum >= m_nSelectLineFrom && nLineNum <= m_nSelectLineTo ){
			if( m_bBeginBoxSelect){		/* 矩形範囲選択中 */
				nSelectFrom = m_nSelectColmFrom;
				nSelectTo   = m_nSelectColmTo;
			}else{
				if( m_nSelectLineFrom == m_nSelectLineTo ){
						nSelectFrom = m_nSelectColmFrom;
						nSelectTo   = m_nSelectColmTo;
				}else{
					if( nLineNum == m_nSelectLineFrom ){
						nSelectFrom = m_nSelectColmFrom;
						nSelectTo   = nX;
					}else
					if( nLineNum == m_nSelectLineTo ){
						nSelectFrom = 0;
						nSelectTo   = m_nSelectColmTo;
					}else{
						nSelectFrom = 0;
						nSelectTo   = nX;
					}
				}
			}
			if( nSelectFrom < m_nViewLeftCol ){
				nSelectFrom = m_nViewLeftCol;
			}
			if( nSelectTo < m_nViewLeftCol ){
				nSelectTo = m_nViewLeftCol;
			}
			rcClip.left   = x + nSelectFrom * ( nCharWidth );
			rcClip.right  = x + nSelectTo   * ( nCharWidth );
			rcClip.top    = y;
			rcClip.bottom = y + nLineHeight;
			if( rcClip.right - rcClip.left > 3000 ){
				rcClip.right = rcClip.left + 3000;
			}
			hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
			nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );
			hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
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




/* 現在位置が検索文字列に該当するか */
//2002.02.08 hor
//正規表現で検索したときの速度改善のため、マッチ先頭位置を引数に追加
BOOL CEditView::IsSearchString( const char* pszData, int nDataLen, int nPos, int* pnSearchStart, int* pnSearchEnd )
{
	int		nKeyLength;

	//	From Here Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	BREGEXP* result;
	*pnSearchStart = nPos;	// 2002.02.08 hor

	if( m_bCurSrchRegularExp ){
		/* 行頭ではない? */
//		if( ( ( m_szCurSrchKey[0] == '/' && '^' == m_szCurSrchKey[1] )
//			|| ( m_szCurSrchKey[0] == 'm' && '^' == m_szCurSrchKey[2] ) )
		if( ( m_szCurSrchKey[0] == '^' )
			&& 0 != nPos ){
			return FALSE;
		}

		if( m_CurRegexp.GetMatchInfo( &pszData[nPos], nDataLen - nPos, 0, &result )
	//	 && ( result->startp[0] == &pszData[nPos] )			// 2002.02.08 hor
		){
			*pnSearchStart = result->startp[0] - pszData;	// 2002.02.08 hor
			*pnSearchEnd = result->endp[0] - pszData;
	//	To Here Jun. 26, 2001 genta
			return TRUE;

		}else{
			return FALSE;
		}
	}else{
		nKeyLength = lstrlen( m_szCurSrchKey );		/* 検索条件 */

		// 2001/06/23 単語単位の検索のために追加
		if( m_pShareData->m_Common.m_bWordOnly ){	/* 検索／置換  1==単語のみ検索 */
			/* 現在位置の単語の範囲を調べる */
			/* 現在位置の単語の範囲を調べる */
			int nIdxFrom, nIdxTo;
			if( false == CDocLineMgr::WhereCurrentWord_2( pszData, nDataLen, nPos, &nIdxFrom, &nIdxTo, NULL, NULL ) ){
				return FALSE;
			}
			if( nPos != nIdxFrom || nKeyLength != nIdxTo - nIdxFrom ){
				return FALSE;
			}
		}

		//検索条件が未定義 または 検索条件の長さより調べるデータが短いときはヒットしない
		if( 0 == nKeyLength || nKeyLength > nDataLen - nPos ){
			return FALSE;
		}
		//英大文字小文字の区別をするかどうか
		if( m_bCurSrchLoHiCase ){	/* 1==英大文字小文字の区別 */
			if( 0 == memcmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return TRUE;
			}
		}else{
			if( 0 == memicmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*! 
	ルーラーのキャレットを再描画	2002.02.25 Add By KK
	@param hdc [in] デバイスコンテキスト
	DispRulerの内容を元に作成
*/
inline void CEditView::DrawRulerCaret( HDC hdc )
{
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	HRGN		hRgn;
	RECT		rc;
	int			nROP_Old;

	if( m_nViewLeftCol <= m_nCaretPosX
	 && m_nViewLeftCol + m_nViewColNum + 2 >= m_nCaretPosX
	){
		if (m_nOldCaretPosX == m_nCaretPosX && m_nCaretWidth == m_nOldCaretWidth) {
			//前描画した位置画同じ かつ ルーラーのキャレット幅が同じ 
			return;
		}

		rc.left = m_nViewAlignLeft + ( m_nOldCaretPosX - m_nViewLeftCol ) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) + 1;
		rc.right = rc.left + m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace - 1;
		rc.top = 0;
		rc.bottom = m_nViewAlignTop - m_nTopYohaku - 1;

		//元位置をクリア m_nOldCaretWidth
		if( 0 == m_nOldCaretWidth ){
			hBrush = ::CreateSolidBrush( RGB( 128, 128, 128 ) );
		}else{
			hBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
		}
		nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
		hRgn = ::CreateRectRgnIndirect( &rc );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
		::PaintRgn( hdc, hRgn );
		::DeleteObject( hRgn );
		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );

		if( 0 == m_nCaretWidth ){
			hBrush = ::CreateSolidBrush( RGB( 128, 128, 128 ) );
		}else{
			hBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
		}

		//新しい位置で描画
		rc.left = m_nViewAlignLeft + ( m_nCaretPosX - m_nViewLeftCol ) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) + 1;
		rc.right = rc.left + m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace - 1;

		hRgn = ::CreateRectRgnIndirect( &rc );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
		::SelectObject( hdc, hBrush );
		::PaintRgn( hdc, hRgn );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hRgn );
		::DeleteObject( hBrush );
		::SetROP2( hdc, nROP_Old );
	}
}



/*! ルーラー描画 */
void CEditView::DispRuler( HDC hdc )
{

#ifdef _DEBUG
//	if( 0 != m_pShareData->m_Common.m_nRulerType ){	/* ルーラーのタイプ */
//		DispRulerEx( hdc );
//		return;
//	}
#endif


	if( !m_bDrawSWITCH ){
		return;
	}
	if( !m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		return;
	}

	// 2002.02.25 Add By KK ルーラー全体を描き直す必要がない場合は、ルーラ上のキャレットのみ描きなおす 
	if ( !m_bRedrawRuler ) {
		DrawRulerCaret( hdc );
	}
	else {
		/* 描画処理 */
		HBRUSH		hBrush;
		HBRUSH		hBrushOld;
		HRGN		hRgn;
		RECT		rc;
		int			i;
		int			nX;
		int			nY;
		LOGFONT		lf;
		HFONT		hFont;
 		HFONT		hFontOld;
		char		szColm[32];

		HPEN		hPen;
		HPEN		hPenOld;
		int			nROP_Old;
		COLORREF	colTextOld;
		int			nToX;

		/* LOGFONTの初期化 */
		memset( &lf, 0, sizeof(LOGFONT) );
		lf.lfHeight			= 1 - m_pShareData->m_Common.m_nRulerHeight;	//	2002/05/13 ai
		lf.lfWidth			= 5/*0*/;
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
		strcpy( lf.lfFaceName, "Arial" );
		hFont = ::CreateFontIndirect( &lf );
		hFontOld = (HFONT)::SelectObject( hdc, hFont );
		::SetBkMode( hdc, TRANSPARENT );

		hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_colBACK );
		rc.left = 0;
		rc.top = 0;
		rc.right = m_nViewAlignLeft + m_nViewCx;
		rc.bottom = m_nViewAlignTop - m_nTopYohaku;
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );

		nX = m_nViewAlignLeft;
		nY = m_nViewAlignTop - m_nTopYohaku - 2;

		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		colTextOld = ::SetTextColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_colTEXT );


		nToX = m_nViewAlignLeft + m_nViewCx;

		nToX = m_nViewAlignLeft + (m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - m_nViewLeftCol) * ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		if( nToX > m_nViewAlignLeft + m_nViewCx ){
			nToX = m_nViewAlignLeft + m_nViewCx;
		}
		::MoveToEx( hdc, m_nViewAlignLeft, nY + 1, NULL );
		::LineTo( hdc, nToX/*m_nViewAlignLeft + m_nViewCx*/, nY + 1 );


		for( i = m_nViewLeftCol;
			i <= m_nViewLeftCol + m_nViewColNum + 1
		 && i <= m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize;
			i++
		){
			if( i == m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize ){
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, 0/*nY - 8*/ );
			}
			if( 0 == ( (i) % 10 ) ){
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, 0/*nY - 8*/ );
				itoa( (i) / 10, szColm, 10 );
				::TextOut( hdc, nX + 2 + 0, -1 + 0, szColm, lstrlen( szColm ) );
			}else
			if( 0 == ( (i) % 5 ) ){
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, nY - 6 );
			}else{
				::MoveToEx( hdc, nX, nY, NULL );
				::LineTo( hdc, nX, nY - 3 );
			}
			nX += ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		}
		::SetTextColor( hdc, colTextOld );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );

		/* キャレット描画（現在の位置に描画するだけ。古い位置はすでに消されている） */
		if( m_nViewLeftCol <= m_nCaretPosX
		 && m_nViewLeftCol + m_nViewColNum + 2 >= m_nCaretPosX
		){
			//	Aug. 18, 2000 あお
			rc.left = m_nViewAlignLeft + ( m_nCaretPosX - m_nViewLeftCol ) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) + 1;
			rc.right = rc.left + m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace - 1;
			rc.top = 0;
			rc.bottom = m_nViewAlignTop - m_nTopYohaku - 1;

			if( 0 == m_nCaretWidth ){
				hBrush = ::CreateSolidBrush( RGB( 128, 128, 128 ) );
			}else{
				hBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
			}
			nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
			hRgn = ::CreateRectRgnIndirect( &rc );
			hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
			::PaintRgn( hdc, hRgn );

			::SelectObject( hdc, hBrushOld );
			::DeleteObject( hRgn );
			::DeleteObject( hBrush );
			::SetROP2( hdc, nROP_Old );
		}

		::SelectObject( hdc, hFontOld );
		::DeleteObject( hFont );

		m_bRedrawRuler = false;	//m_bRedrawRuler = true で指定されるまで、ルーラのキャレットのみを再描画 2002.02.25 Add By KK
	}

	//描画したルーラーのキャレット位置・幅を保存 2002.02.25 Add By KK
	m_nOldCaretPosX = m_nCaretPosX;
	m_nOldCaretWidth = m_nCaretWidth ;

	return;
}

//======================================================================
//@@@ 2001.02.03 Start by MIK: 全角文字の対括弧
//! 全角括弧の対応表
const struct ZENKAKKO_T{
	char *sStr;
	char *eStr;
} zenkakkoarr[] = {
	"【", "】",
	"『", "』",
	"「", "」",
	"＜", "＞",
	"≪", "≫",
	"《", "》",
	"（", "）",
	"〈", "〉",
	"｛", "｝",
	"〔", "〕",
	"［", "］",
	"“", "”",
	"〝", "〟",
	NULL, NULL	//終端識別
};
//@@@ 全角文字の対括弧: End
//	Jun. 16, 2000 genta
/*!
	@brief 対括弧の検索

	カーソル位置の括弧に対応する括弧を探す。カーソル位置が括弧でない場合は
	カーソルの後ろの文字が括弧かどうかを調べる。

	カーソルの前後いずれもが括弧でない場合は何もしない。

	括弧が半角か全角か、及び始まりか終わりかによってこれに続く4つの関数に
	制御を移す。

	@param LayoutX [in] 検索開始点の物理座標X
	@param LayoutY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字
	@param dnChar [in] 括弧を閉じる文字列

	@retval true 成功
	@retval false 失敗

	@author genta
	@date Jun. 16, 2000 genta
	@date Feb. 03, 2001 MIK 全角括弧に対応

*/
bool CEditView::SearchBracket( int LayoutX, int LayoutY, int* NewX, int* NewY )
{
	int len;	//	行の長さ
	int nCharSize;	//	（メモリ上の）文字幅
	int PosX, PosY;	//	物理位置

	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys( LayoutX, LayoutY, &PosX, &PosY );
	const char *cline = m_pcEditDoc->m_cDocLineMgr.GetLineStr( PosY, &len );

	//	Jun. 19, 2000 genta
	if( cline == NULL )	//	最後の行に本文がない場合
		return false;
//	PosX = LineColmnToIndex( cline, len, PosX );	不要

	nCharSize = CMemory::MemCharNext( cline, len, cline + PosX ) - cline - PosX;

	if( nCharSize == 1 ){	//	1バイト文字
//		char buf[] = "Bracket:  Forward";
//		buf[8] = cline[ PosX ];
//		::MessageBox( NULL, buf, "Bracket", MB_OK );

		switch( cline[ PosX ] ){
		case '(':	return SearchBracketForward( PosX, PosY, NewX, NewY, '(', ')' );
		case '[':	return SearchBracketForward( PosX, PosY, NewX, NewY, '[', ']' );
		case '{':	return SearchBracketForward( PosX, PosY, NewX, NewY, '{', '}' );
		case '<':	return SearchBracketForward( PosX, PosY, NewX, NewY, '<', '>' );

		case ')':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '(', ')' );
		case ']':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '[', ']' );
		case '}':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '{', '}' );
		case '>':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '<', '>' );
		}
//@@@ 2001.02.03 Start by MIK: 全角文字の対括弧
	}else if( nCharSize == 2 ){	// 2バイト文字
		int i;
		const struct ZENKAKKO_T *p;
		p = zenkakkoarr;
		for(i = 0; p->sStr != NULL; i++, p++)
		{
			if(strncmp(p->sStr, &cline[PosX], 2) == 0)
			{
				return SearchBracketForward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr );
			}
			else if(strncmp(p->eStr, &cline[PosX], 2) == 0)
			{
				return SearchBracketBackward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr );
			}
		}
//@@@ 2001.02.03 End: 全角文字の対括弧
	}

	//	括弧が見つからなかったら，カーソルの直前の文字を調べる

	if( PosX <= 0 ){
//		::MessageBox( NULL, "NO DATA", "Bracket", MB_OK );
		return false;	//	前の文字はない
	}
	const char *bPos = CMemory::MemCharPrev( cline, PosX, cline + PosX );
	nCharSize = cline + PosX - bPos;
	if( nCharSize == 1 ){	//	1バイト文字
		PosX = bPos - cline;

//		char buf[] = "Bracket:  Back";
//		buf[8] = cline[ PosX ];
//		::MessageBox( NULL, buf, "Bracket", MB_OK );

		switch( cline[ PosX ] ){
		case '(':	return SearchBracketForward( PosX, PosY, NewX, NewY, '(', ')' );
		case '[':	return SearchBracketForward( PosX, PosY, NewX, NewY, '[', ']' );
		case '{':	return SearchBracketForward( PosX, PosY, NewX, NewY, '{', '}' );
		case '<':	return SearchBracketForward( PosX, PosY, NewX, NewY, '<', '>' );

		case ')':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '(', ')' );
		case ']':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '[', ']' );
		case '}':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '{', '}' );
		case '>':	return SearchBracketBackward( PosX, PosY, NewX, NewY, '<', '>' );
		}
//@@@ 2001.02.03 Start by MIK: 全角文字の対括弧
	}else if( nCharSize == 2 ){	// 2バイト文字
		int i;
		const struct ZENKAKKO_T *p;
		PosX = bPos - cline;
		p = zenkakkoarr;
		for( i = 0; p->sStr != NULL; i++, p++ )
		{
			if( strncmp(p->sStr, &cline[PosX], 2) == 0 )
			{
				return SearchBracketForward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr );
			}
			else if( strncmp(p->eStr, &cline[PosX], 2) == 0 )
			{
				return SearchBracketBackward2( PosX, PosY, NewX, NewY, p->sStr, p->eStr );
			}
		}
//@@@ 2001.02.03 End: 全角文字の対括弧
	}
	return false;
}

/*!
	@brief 半角対括弧の検索:順方向

	@author genta

	@param LayoutX [in] 検索開始点の物理座標X
	@param LayoutY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字
	@param dnChar [in] 括弧を閉じる文字列

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchBracketForward( int PosX, int PosY, int* NewX, int* NewY,
									int upChar, int dnChar )
{
	CDocLine* ci;

	int			len;
	const char	*cPos, *nPos;
	char		*cline, *lineend;
	int			level = 0;

//	char buf[50];	Debug用

	//	初期位置の設定
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos < lineend ){
			nPos = CMemory::MemCharNext( cline, len, cPos );
			if( nPos - cPos > 1 ){
				//	skip
				cPos = nPos;
				continue;
			}
			if( *cPos == upChar )		++level;
			else if( *cPos == dnChar )	--level;

			if( level == 0 ){	//	見つかった！
				PosX = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
//				wsprintf( buf, "Layout: %d, %d\nPhys: %d, %d", *NewX, *NewY, PosX, PosY );
//				::MessageBox( NULL, buf, "Bracket", MB_OK );
				return true;
				//	Happy Ending
			}
			cPos = nPos;	//	次の文字へ
		}

		//	次の行へ
		++PosY;
		ci = ci->m_pNext;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_pLine->GetPtr( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}

/*!
	@brief 半角対括弧の検索:逆方向

	@author genta

	@param LayoutX [in] 検索開始点の物理座標X
	@param LayoutY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字
	@param dnChar [in] 括弧を閉じる文字列

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchBracketBackward( int PosX, int PosY, int* NewX, int* NewY,
									int dnChar, int upChar )
{
	CDocLine* ci;

	int			len;
	const char	*cPos, *pPos;
	char		*cline, *lineend;
	int			level = 1;

//	char buf[50];	Debug用

	//	初期位置の設定
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos > cline ){
			pPos = CMemory::MemCharPrev( cline, len, cPos );
			if( cPos - pPos > 1 ){
				//	skip
				cPos = pPos;
				continue;
			}
			if( *pPos == upChar )		++level;
			else if( *pPos == dnChar )	--level;

			if( level == 0 ){	//	見つかった！
				PosX = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
//				wsprintf( buf, "Layout: %d, %d\nPhys: %d, %d", *NewX, *NewY, PosX, PosY );
//				::MessageBox( NULL, buf, "Bracket", MB_OK );
				return true;
				//	Happy Ending
			}
			cPos = pPos;	//	次の文字へ
		}

		//	次の行へ
		--PosY;
		ci = ci->m_pPrev;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_pLine->GetPtr( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}

//@@@ 2001.02.03 Start by MIK:
/*!
	@brief 全角対括弧の検索:順方向

	@author MIK

	@param LayoutX [in] 検索開始点の物理座標X
	@param LayoutY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字へのポインタ
	@param dnChar [in] 括弧を閉じる文字列へのポインタ

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchBracketForward2(  int		PosX,	int		PosY,
										int*	NewX,	int*	NewY,
										char*	upChar,	char*	dnChar )
{
	CDocLine* ci;

	int len;
	const char *cPos, *nPos;
	char *cline, *lineend;
	int level = 0;

	//	初期位置の設定
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos < lineend ){
			nPos = CMemory::MemCharNext( cline, len, cPos );
			if( nPos - cPos != 2 ){
				//	skip
				cPos = nPos;
				continue;
			}
			if( strncmp(upChar, cPos, 2) == 0 ){
				++level;
			}
			else if( strncmp(dnChar, cPos, 2) == 0 ){
				--level;
			}

			if( level == 0 ){	//	見つかった！
				PosX = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
				return true;
			}
			cPos = nPos;	//	次の文字へ
		}

		//	次の行へ
		++PosY;
		ci = ci->m_pNext;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_pLine->GetPtr( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}
//@@@ 2001.02.03 End

//@@@ 2001.02.03 Start by MIK:
/*!
	@brief 全角対括弧の検索:逆方向

	@author MIK

	@param LayoutX [in] 検索開始点の物理座標X
	@param LayoutY [in] 検索開始点の物理座標Y
	@param NewX [out] 移動先のレイアウト座標X
	@param NewY [out] 移動先のレイアウト座標Y
	@param upChar [in] 括弧の始まりの文字へのポインタ
	@param dnChar [in] 括弧を閉じる文字列へのポインタ

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchBracketBackward2( int   PosX,   int   PosY,
									    int*  NewX,   int*  NewY,
									    char* dnChar, char* upChar )
{
	CDocLine* ci;

	int len;
	const char *cPos, *pPos;
	char *cline, *lineend;
	int level = 1;

	//	初期位置の設定
	ci = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( PosY );
	cline = ci->m_pLine->GetPtr( &len );
	lineend = cline + len;
	cPos = cline + PosX;

	do {
		while( cPos > cline ){
			pPos = CMemory::MemCharPrev( cline, len, cPos );
			if( cPos - pPos != 2 ){
				//	skip
				cPos = pPos;
				continue;
			}
			if( strncmp(upChar, pPos, 2) == 0 ){
				++level;
			}
			else if( strncmp(dnChar, pPos, 2) == 0 ){
				--level;
			}

			if( level == 0 ){	//	見つかった！
				PosX = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( PosX, PosY, NewX, NewY );
				return true;
			}
			cPos = pPos;	//	次の文字へ
		}

		//	次の行へ
		--PosY;
		ci = ci->m_pPrev;	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->m_pLine->GetPtr( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}
//@@@ 2001.02.03 End

//!	現在のカーソル行位置を履歴に登録する
void CEditView::AddCurrentLineToHistory( void )
{
	int PosX, PosY;	//	物理位置（改行単位の計算）

	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys( m_nCaretPosX, m_nCaretPosY, &PosX, &PosY );

	CMarkMgr::CMark m( PosX, PosY );
	m_cHistory->Add( m );

}


//	2001/06/18 Start by asa-o: 補完ウィンドウ用のキーワードヘルプ表示
bool  CEditView::ShowKeywordHelp( POINT po, LPCTSTR pszHelp, LPRECT prcHokanWin)
{
	CMemory		cmemCurText;
	CMemory*	pcmemRefText;
	LPSTR		pszWork;
	RECT		rcTipWin,
				rcDesktop;

	if( m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp ){ /* キーワードヘルプを使用する */
		if( m_bInMenuLoop == FALSE	&&	/* メニュー モーダル ループに入っていない */
			0 != m_dwTipTimer			/* 辞書Tipを表示していない */
		){
			cmemCurText.SetDataSz( pszHelp );

			/* 既に検索済みか */
			if( CMemory::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) ){
				/* 該当するキーがなかった */
				if( !m_cTipWnd.m_KeyWasHit ){
					return false;
				}
			}else{
				m_cTipWnd.m_cKey = cmemCurText;
				/* 検索実行 */
				if( m_cDicMgr.Search( cmemCurText.GetPtr(), &pcmemRefText, m_pcEditDoc->GetDocumentAttribute().m_szKeyWordHelpFile ) ){
					/* 該当するキーがある */
					m_cTipWnd.m_KeyWasHit = TRUE;
					pszWork = pcmemRefText->GetPtr();
//								m_cTipWnd.m_cInfo.SetData( pszWork, lstrlen( pszWork ) );
					m_cTipWnd.m_cInfo.SetDataSz( pszWork );
					delete pcmemRefText;
				}else{
					/* 該当するキーがなかった */
					m_cTipWnd.m_KeyWasHit = FALSE;
					return false;
				}
			}
			m_dwTipTimer = 0;	/* 辞書Tipを表示している */

		// 2001/06/19 Start by asa-o: 辞書Tipの表示位置調整
			// 辞書Tipのサイズを取得
			m_cTipWnd.GetWindowSize(&rcTipWin);

			::SystemParametersInfo( SPI_GETWORKAREA, NULL, &rcDesktop, 0 );

			// 右に入る
			if(prcHokanWin->right + rcTipWin.right < rcDesktop.right){
				// そのまま
			}else
			// 左に入る
			if(rcDesktop.left < prcHokanWin->left - rcTipWin.right ){
				// 左に表示
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}else
			// どちらもスペースが無いとき広いほうに表示
			if(rcDesktop.right - prcHokanWin->right > prcHokanWin->left ){
				// 右に表示 そのまま
			}else{
				// 左に表示
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}
		// 2001/06/19 End

			/* 辞書Tipを表示 */
			m_cTipWnd.Show( po.x, po.y , NULL , &rcTipWin);
			return true;
		}
	}
	return false;
}
//	2001/06/18 End


/*[EOF]*/
