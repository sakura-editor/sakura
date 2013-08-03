/*!	@file
	@brief 文書ウィンドウの管理

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date   2005/09/02 D.S.Koba GetSizeOfCharで書き換え
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, かろと, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta, maru
	Copyright (C) 2007, ryoji, じゅうじ, maru, genta, Moca, nasukoji, D.S.Koba
	Copyright (C) 2008, ryoji, nasukoji, bosagami, Moca, genta
	Copyright (C) 2009, nasukoji, ryoji, syat
	Copyright (C) 2010, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CEditView.h"
#include "CEditWnd.h"

/*!	IME ONか

	@date  2006.12.04 ryoji 新規作成（関数化）
*/
bool CEditView::IsImeON( void )
{
	bool bRet;
	HIMC	hIme;
	DWORD	conv, sent;

	//	From here Nov. 26, 2006 genta
	hIme = ImmGetContext( m_hwndParent );
	if( ImmGetOpenStatus( hIme ) != FALSE ){
		ImmGetConversionStatus( hIme, &conv, &sent );
		if(( conv & IME_CMODE_NOCONVERSION ) == 0 ){
			bRet = true;
		}
		else {
			bRet = false;
		}
	}
	else {
		bRet = false;
	}
	ImmReleaseContext( m_hwndParent, hIme );
	//	To here Nov. 26, 2006 genta

	return bRet;
}

/* IME編集エリアの位置を変更 */
void CEditView::SetIMECompFormPos( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	COMPOSITIONFORM	CompForm;
	HIMC			hIMC = ::ImmGetContext( m_hWnd );
	POINT			point;
	HWND			hwndFrame;
	hwndFrame = ::GetParent( m_hwndParent );

	::GetCaretPos( &point );
	CompForm.dwStyle = CFS_POINT;
	CompForm.ptCurrentPos.x = (long) point.x;
	CompForm.ptCurrentPos.y = (long) point.y + m_nCaretHeight - m_nCharHeight;

	if ( hIMC ){
		::ImmSetCompositionWindow( hIMC, &CompForm );
	}
	::ImmReleaseContext( m_hWnd , hIMC );
}


/* IME編集エリアの表示フォントを変更 */
void CEditView::SetIMECompFormFont( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	HIMC	hIMC = ::ImmGetContext( m_hWnd );
	if ( hIMC ){
		::ImmSetCompositionFont( hIMC, &(m_pShareData->m_Common.m_sView.m_lf) );
	}
	::ImmReleaseContext( m_hWnd , hIMC );
}
//  2002.04.09 minfu from here
/*再変換用 カーソル位置から前後200byteを取り出してRECONVERTSTRINGを埋める */
/*  引数  pReconv RECONVERTSTRING構造体へのポインタ。                     */
/*        bUnicode trueならばUNICODEで構造体を埋める                      */
/*  戻り値   RECONVERTSTRINGのサイズ                                      */
LRESULT CEditView::SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode)
{
	const char	*pLine;
	int			nCurrentLine;
	
	//行の中で再変換のAPIにわたすとする文字列の開始位置と長さ（考慮文字列）
	int			nReconvIndex, nReconvLenWithNull;
	
	//行の中で再変換の注目する文節とする文字列の開始位置、終了位置、長さ（対象文字列）
	int			nSelectedIndex, nSelectedEndIndex, nSelectedLen;
	
	int			nSelectColumnFrom;
	int			nSelectLineFrom, nSelectLineTo;
	
	DWORD		dwReconvTextLen;
	DWORD		dwCompStrOffset, dwCompStrLen;
	
	CMemory		cmemBuf1;
	const char*		pszReconv;
	CDocLine*	pcCurDocLine;
	
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;
	
	//矩形選択中は何もしない
	if( m_bBeginBoxSelect )
		return 0;
	
	if( IsTextSelected() ){
		//テキストが選択されているとき
		nSelectColumnFrom = m_sSelect.m_ptFrom.x;
		nSelectLineFrom   = m_sSelect.m_ptFrom.y;
		
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y, &nSelectedIndex, &nCurrentLine);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(m_sSelect.m_ptTo.x, m_sSelect.m_ptTo.y, &nSelectedEndIndex, &nSelectLineTo);
		
		//選択範囲が複数行の時は
		if (nSelectLineTo != nCurrentLine){
			//行末までに制限
			pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(nCurrentLine);
			nSelectedEndIndex = pcCurDocLine->m_cLine.GetStringLength();
		}
		
		nSelectedLen = nSelectedEndIndex - nSelectedIndex;
		
	}else{
		//テキストが選択されていないとき
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(m_ptCaretPos.x ,m_ptCaretPos.y , &nSelectedIndex, &nCurrentLine);
		nSelectedLen = 0;
	}
	
	pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(nCurrentLine);
	if (NULL == pcCurDocLine )
		return 0;
	
	const int nLineLen = pcCurDocLine->m_cLine.GetStringLength() - pcCurDocLine->m_cEol.GetLen() ; //改行コードをのぞいた長さ
	if ( 0 == nLineLen )
		return 0;
	
	pLine = pcCurDocLine->m_cLine.GetStringPtr();

	//再変換考慮文字列開始
	nReconvIndex = 0;
	if ( nSelectedIndex > 200 ) {
		const char* pszWork = pLine;
		while( (nSelectedIndex - nReconvIndex) > 200 ){
			pszWork = ::CharNext( pszWork);
			nReconvIndex = pszWork - pLine ;
		}
	}
	
	//再変換考慮文字列終了
	int nReconvLen = nLineLen - nReconvIndex;
	if ( (nReconvLen + nReconvIndex - nSelectedIndex) > 200 ){
		const char* pszWork = pLine + nSelectedIndex;
		nReconvLen = nSelectedIndex - nReconvIndex;
		while( ( nReconvLen + nReconvIndex - nSelectedIndex) <= 200 ){
			pszWork = ::CharNext( pszWork);
			nReconvLen = pszWork - (pLine + nReconvIndex) ;
		}
	}
	
	//対象文字列の調整
	if ( nSelectedIndex + nSelectedLen > nReconvIndex + nReconvLen ){
		nSelectedLen = nReconvIndex + nReconvLen - nSelectedIndex;
	}
	
	pszReconv =  pLine + nReconvIndex;
	
	if(bUnicode){
		
		//考慮文字列の開始から対象文字列の開始まで
		if( nSelectedIndex - nReconvIndex > 0 ){
			cmemBuf1.SetString(pszReconv, nSelectedIndex - nReconvIndex);
			cmemBuf1.SJISToUnicode();
			dwCompStrOffset = cmemBuf1.GetStringLength();  //Offset はbyte
		}else{
			dwCompStrOffset = 0;
		}
		
		//対象文字列の開始から対象文字列の終了まで
		if (nSelectedLen > 0 ){
			cmemBuf1.SetString(pszReconv + nSelectedIndex, nSelectedLen);  
			cmemBuf1.SJISToUnicode();
			dwCompStrLen = cmemBuf1.GetStringLength() / sizeof(wchar_t);
		}else{
			dwCompStrLen = 0;
		}
		
		//考慮文字列すべて
		cmemBuf1.SetString(pszReconv , nReconvLen );
		cmemBuf1.SJISToUnicode();
		
		dwReconvTextLen =  cmemBuf1.GetStringLength() / sizeof(wchar_t);
		nReconvLenWithNull =  cmemBuf1.GetStringLength()  + sizeof(wchar_t);
		
		pszReconv = cmemBuf1.GetStringPtr();
	}else{
		dwReconvTextLen = nReconvLen;
		nReconvLenWithNull = nReconvLen + 1;
		dwCompStrOffset = nSelectedIndex - nReconvIndex;
		dwCompStrLen    = nSelectedLen;
	}
	
	if ( NULL != pReconv) {
		//再変換構造体の設定
		pReconv->dwSize = sizeof(*pReconv) + nReconvLenWithNull ;
		pReconv->dwVersion = 0;
		pReconv->dwStrLen = dwReconvTextLen ;
		pReconv->dwStrOffset = sizeof(*pReconv) ;
		pReconv->dwCompStrLen = dwCompStrLen;			//文字単位
		pReconv->dwCompStrOffset = dwCompStrOffset;		//バイト単位
		pReconv->dwTargetStrLen = dwCompStrLen;			//文字単位
		pReconv->dwTargetStrOffset = dwCompStrOffset;	//バイト単位
		
		// 2004.01.28 Moca ヌル終端の修正
		if( bUnicode ){
			CopyMemory( (void *)(pReconv + 1), (void *)pszReconv , nReconvLenWithNull - sizeof(wchar_t) );
			*((wchar_t *)(pReconv + 1) + nReconvLenWithNull - sizeof(wchar_t) ) = L'\0';
		}else{
			CopyMemory( (void *)(pReconv + 1), (void *)pszReconv , nReconvLenWithNull - 1 );
			*((char *)(pReconv + 1) + nReconvLenWithNull - 1 ) = '\0';
		}
	}
	
	// 再変換情報の保存
	m_nLastReconvIndex = nReconvIndex;
	m_nLastReconvLine  = nCurrentLine;
	
	return sizeof(RECONVERTSTRING) + nReconvLenWithNull;

}

/*再変換用 エディタ上の選択範囲を変更する 2002.04.09 minfu */
LRESULT CEditView::SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode){
	
	CMemory		cmemBuf;
	
	// 再変換情報が保存されているか
	if ( (m_nLastReconvIndex < 0) || (m_nLastReconvLine < 0))
		return 0;

	if ( IsTextSelected()) 
		DisableSelectArea( true );

	DWORD		dwOffset, dwLen;
	
	if(bUnicode){
		
		//考慮文字列の開始から対象文字列の開始まで
		if( pReconv->dwCompStrOffset > 0){
			cmemBuf.SetString((const char *)((const wchar_t *)(pReconv + 1)), 
								pReconv->dwCompStrOffset ); 
			cmemBuf.UnicodeToSJIS();
			dwOffset = cmemBuf.GetStringLength();
			
		}else{
			dwOffset = 0;
		}

		//対象文字列の開始から対象文字列の終了まで
		if( pReconv->dwCompStrLen > 0 ){
			cmemBuf.SetString((const char *)(const wchar_t *)(pReconv + 1) + pReconv->dwCompStrOffset , 
								pReconv->dwCompStrLen * sizeof(wchar_t)); 
			cmemBuf.UnicodeToSJIS();
			dwLen = cmemBuf.GetStringLength();
		}else{
			dwLen = 0;
		}
	}else{
		dwOffset = pReconv->dwCompStrOffset;
		dwLen =  pReconv->dwCompStrLen;
	}
	
	//選択開始の位置を取得
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(m_nLastReconvIndex + dwOffset 
												, m_nLastReconvLine, &m_sSelect.m_ptFrom.x, &m_sSelect.m_ptFrom.y);
	//選択終了の位置を取得
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(m_nLastReconvIndex + dwOffset + dwLen
												, m_nLastReconvLine, &m_sSelect.m_ptTo.x, &m_sSelect.m_ptTo.y);

	// 単語の先頭にカーソルを移動
	MoveCursor( m_sSelect.m_ptFrom.x, m_sSelect.m_ptFrom.y, true );

	//選択範囲再描画 
	DrawSelectArea();

	// 再変換情報の破棄
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;

	return 1;

}

/*[EOF]*/
