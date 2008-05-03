#include "stdafx.h"
#include "CEditView.h"
#include "charset/CShiftJis.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           IME                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


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
	HIMC			hIMC = ::ImmGetContext( GetHwnd() );
	POINT			point;
	HWND			hwndFrame;
	hwndFrame = ::GetParent( m_hwndParent );

	::GetCaretPos( &point );
	CompForm.dwStyle = CFS_POINT;
	CompForm.ptCurrentPos.x = (long) point.x;
	CompForm.ptCurrentPos.y = (long) point.y + GetCaret().GetCaretSize().cy - GetTextMetrics().GetHankakuHeight();

	if ( hIMC ){
		::ImmSetCompositionWindow( hIMC, &CompForm );
	}
	::ImmReleaseContext( GetHwnd() , hIMC );
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
	HIMC	hIMC = ::ImmGetContext( GetHwnd() );
	if ( hIMC ){
		::ImmSetCompositionFont( hIMC, &(GetDllShareData().m_Common.m_sView.m_lf) );
	}
	::ImmReleaseContext( GetHwnd() , hIMC );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          再変換                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//  2002.04.09 minfu from here
/*再変換用 カーソル位置から前後200byteを取り出してRECONVERTSTRINGを埋める */
/*  引数  pReconv RECONVERTSTRING構造体へのポインタ。                     */
/*        bUnicode trueならばUNICODEで構造体を埋める                      */
/*  戻り値   RECONVERTSTRINGのサイズ                                      */
LRESULT CEditView::SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode)
{
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;
	
	//矩形選択中は何もしない
	if( GetSelectionInfo().IsBoxSelecting() )
		return 0;
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      選択範囲を取得                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//選択範囲を取得 -> ptSelect, ptSelectTo, nSelectedLen
	CLogicPoint	ptSelect;
	CLogicPoint	ptSelectTo;
	int			nSelectedLen;
	if( GetSelectionInfo().IsTextSelected() ){
		//テキストが選択されているとき
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(GetSelectionInfo().m_sSelect.GetFrom(), &ptSelect);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(GetSelectionInfo().m_sSelect.GetTo(), &ptSelectTo);
		
		//選択範囲が複数行の時は
		if (ptSelectTo.y != ptSelect.y){
			//行末までに制限
			CDocLine* pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(ptSelect.GetY2());
			ptSelectTo.x = pDocLine->GetLengthWithEOL();
		}
	}
	else{
		//テキストが選択されていないとき
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos(), &ptSelect);
		ptSelectTo = ptSelect;
	}
	nSelectedLen = ptSelectTo.x - ptSelect.x;

	//ドキュメント行取得 -> pcCurDocLine
	CDocLine* pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(ptSelect.GetY2());
	if (NULL == pcCurDocLine )
		return 0;

	//テキスト取得 -> pLine, nLineLen
	int nLineLen = pcCurDocLine->GetLengthWithEOL() - pcCurDocLine->GetEol().GetLen() ; //改行コードをのぞいた長さ
	if ( 0 == nLineLen )
		return 0;
	const wchar_t* pLine = pcCurDocLine->GetPtr();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      選択範囲を修正                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//再変換考慮文字列開始  //行の中で再変換のAPIにわたすとする文字列の開始位置
	int nReconvIndex = 0;
	if ( ptSelect.x > 200 ) { //$$マジックナンバー注意
		const wchar_t* pszWork = pLine;
		while( (ptSelect.x - nReconvIndex) > 200 ){
			pszWork = ::CharNextW_AnyBuild( pszWork);
			nReconvIndex = pszWork - pLine ;
		}
	}
	
	//再変換考慮文字列終了  //行の中で再変換のAPIにわたすとする文字列の長さ
	int nReconvLen = nLineLen - nReconvIndex;
	if ( (nReconvLen + nReconvIndex - ptSelect.x) > 200 ){
		const wchar_t* pszWork = pLine + ptSelect.x;
		nReconvLen = ptSelect.x - nReconvIndex;
		while( ( nReconvLen + nReconvIndex - ptSelect.x) <= 200 ){
			pszWork = ::CharNextW_AnyBuild( pszWork);
			nReconvLen = pszWork - (pLine + nReconvIndex) ;
		}
	}
	
	//対象文字列の調整
	if ( ptSelect.x + nSelectedLen > nReconvIndex + nReconvLen ){
		nSelectedLen = nReconvIndex + nReconvLen - ptSelect.x;
	}
	

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      構造体設定要素                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//行の中で再変換のAPIにわたすとする文字列の長さ
	int			nReconvLenWithNull;
	DWORD		dwReconvTextLen;
	DWORD		dwCompStrOffset, dwCompStrLen;
	CNativeW	cmemBuf1;
	const void*	pszReconv;

	//UNICODE→UNICODE
	if(bUnicode){
		dwReconvTextLen    = nReconvLen;											//reconv文字列長。文字単位。
		nReconvLenWithNull = (nReconvLen + 1) * sizeof(wchar_t);					//reconvデータ長。バイト単位。
		dwCompStrOffset    = (Int)(ptSelect.x - nReconvIndex) * sizeof(wchar_t);	//compオフセット。バイト単位。
		dwCompStrLen       = nSelectedLen;											//comp文字列長。文字単位。
		pszReconv          = reinterpret_cast<const void*>(pLine + nReconvIndex);	//reconv文字列へのポインタ。
	}
	//UNICODE→ANSI
	else{
		const wchar_t* pszReconvSrc =  pLine + nReconvIndex;

		//考慮文字列の開始から対象文字列の開始まで -> dwCompStrOffset
		if( ptSelect.x - nReconvIndex > 0 ){
			cmemBuf1.SetString(pszReconvSrc, ptSelect.x - nReconvIndex);
			CShiftJis::UnicodeToSJIS(cmemBuf1._GetMemory());
			dwCompStrOffset = cmemBuf1._GetMemory()->GetRawLength();				//compオフセット。バイト単位。
		}else{
			dwCompStrOffset = 0;
		}
		
		//対象文字列の開始から対象文字列の終了まで -> dwCompStrLen
		if (nSelectedLen > 0 ){
			cmemBuf1.SetString(pszReconvSrc + ptSelect.x, nSelectedLen);  
			CShiftJis::UnicodeToSJIS(cmemBuf1._GetMemory());
			dwCompStrLen = cmemBuf1._GetMemory()->GetRawLength();					//comp文字列長。文字単位。
		}else{
			dwCompStrLen = 0;
		}
		
		//考慮文字列すべて
		cmemBuf1.SetString(pszReconvSrc , nReconvLen );
		CShiftJis::UnicodeToSJIS(cmemBuf1._GetMemory());
		
		dwReconvTextLen =  cmemBuf1._GetMemory()->GetRawLength();						//reconv文字列長。文字単位。
		nReconvLenWithNull =  cmemBuf1._GetMemory()->GetRawLength() + sizeof(char);		//reconvデータ長。バイト単位。
		
		pszReconv = reinterpret_cast<const void*>(cmemBuf1._GetMemory()->GetRawPtr());	//reconv文字列へのポインタ
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        構造体設定                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if ( NULL != pReconv) {
		//再変換構造体の設定
		pReconv->dwSize            = sizeof(*pReconv) + nReconvLenWithNull ;
		pReconv->dwVersion         = 0;
		pReconv->dwStrLen          = dwReconvTextLen ;	//文字単位
		pReconv->dwStrOffset       = sizeof(*pReconv) ;
		pReconv->dwCompStrLen      = dwCompStrLen;		//文字単位
		pReconv->dwCompStrOffset   = dwCompStrOffset;	//バイト単位
		pReconv->dwTargetStrLen    = dwCompStrLen;		//文字単位
		pReconv->dwTargetStrOffset = dwCompStrOffset;	//バイト単位
		
		// 2004.01.28 Moca ヌル終端の修正
		if( bUnicode ){
			WCHAR* p = (WCHAR*)(pReconv + 1);
			CopyMemory(p, pszReconv, nReconvLenWithNull - sizeof(wchar_t));
			p[dwReconvTextLen] = L'\0';
		}else{
			ACHAR* p = (ACHAR*)(pReconv + 1);
			CopyMemory(p, pszReconv, nReconvLenWithNull - sizeof(char));
			p[dwReconvTextLen]='\0';
		}
	}
	
	// 再変換情報の保存
	m_nLastReconvIndex = nReconvIndex;
	m_nLastReconvLine  = ptSelect.y;
	
	return sizeof(RECONVERTSTRING) + nReconvLenWithNull;

}

/*再変換用 エディタ上の選択範囲を変更する 2002.04.09 minfu */
LRESULT CEditView::SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode){
	
	// 再変換情報が保存されているか
	if ( (m_nLastReconvIndex < 0) || (m_nLastReconvLine < 0))
		return 0;

	if ( GetSelectionInfo().IsTextSelected()) 
		GetSelectionInfo().DisableSelectArea( TRUE );

	DWORD dwOffset, dwLen;

	//UNICODE→UNICODE
	if(bUnicode){
		dwOffset = pReconv->dwCompStrOffset/sizeof(WCHAR);	//0またはデータ長。バイト単位。→文字単位
		dwLen    = pReconv->dwCompStrLen;					//0または文字列長。文字単位。
	}
	//ANSI→UNICODE
	else{
		CNativeA	cmemBuf;

		//考慮文字列の開始から対象文字列の開始まで
		if( pReconv->dwCompStrOffset > 0){
			const char* p=(const char*)(pReconv+1);
			cmemBuf.SetString(p, pReconv->dwCompStrOffset ); 
			CShiftJis::SJISToUnicode(cmemBuf._GetMemory());
			dwOffset = cmemBuf._GetMemory()->GetRawLength()/sizeof(WCHAR);
		}else{
			dwOffset = 0;
		}

		//対象文字列の開始から対象文字列の終了まで
		if( pReconv->dwCompStrLen > 0 ){
			const char* p=(const char*)(pReconv+1);
			cmemBuf.SetString(p + pReconv->dwCompStrOffset, pReconv->dwCompStrLen); 
			CShiftJis::SJISToUnicode(cmemBuf._GetMemory());
			dwLen = cmemBuf._GetMemory()->GetRawLength()/sizeof(WCHAR);
		}else{
			dwLen = 0;
		}
	}
	
	//選択開始の位置を取得
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		CLogicPoint(m_nLastReconvIndex + dwOffset, m_nLastReconvLine),
		GetSelectionInfo().m_sSelect.GetFromPointer()
	);

	//選択終了の位置を取得
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		CLogicPoint(m_nLastReconvIndex + dwOffset + dwLen, m_nLastReconvLine),
		GetSelectionInfo().m_sSelect.GetToPointer()
	);

	// 単語の先頭にカーソルを移動
	GetCaret().MoveCursor( GetSelectionInfo().m_sSelect.GetFrom(), TRUE );

	//選択範囲再描画 
	GetSelectionInfo().DrawSelectArea();

	// 再変換情報の破棄
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;

	return 1;

}
