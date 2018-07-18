/*!	@file
	@brief IMEの処理

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date   2008/04/13 CEditView.cppから分離
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, かろと, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta
	Copyright (C) 2007, ryoji, じゅうじ, maru

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CEditView.h"
#include <algorithm>
#include "charset/CShiftJis.h"
#include "doc/CEditDoc.h"
#include "env/DLLSHAREDATA.h"
#include "_main/CAppMode.h"
#include "window/CEditWnd.h"

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
	hIme = ImmGetContext( GetHwnd() );
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
	ImmReleaseContext( GetHwnd(), hIme );
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
		::ImmSetCompositionFont( hIMC, const_cast<LOGFONT *>(&(m_pcEditWnd->GetLogfont())) );
	}
	::ImmReleaseContext( GetHwnd() , hIMC );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          再変換・変換補助
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*!
	@brief IMEの再変換/前後参照で、カーソル位置から前後200chars位を取り出してRECONVERTSTRINGを埋める
	@param  pReconv  [out]  RECONVERTSTRING構造体へのポインタ。NULLあり
	@param  bUnicode        trueならばUNICODEで構造体を埋める
	@param  bDocumentFeed   trueならばIMR_DOCUMENTFEEDとして処理する
	@return   RECONVERTSTRINGのサイズ。0ならIMEは何もしない(はず)
	@date 2002.04.09 minfu
	@date 2010.03.16 Moca IMR_DOCUMENTFEED対応
*/
LRESULT CEditView::SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode, bool bDocumentFeed)
{
	if( false == bDocumentFeed ){
		m_nLastReconvIndex = -1;
		m_nLastReconvLine  = -1;
	}
	
	//矩形選択中は何もしない
	if( GetSelectionInfo().IsBoxSelecting() )
		return 0;

	// 2010.04.06 ビューモードでは何もしない
	if( CAppMode::getInstance()->IsViewMode() ){
		return 0;
	}
	
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
		
		// 選択範囲が複数行の時、１ロジック行以内に制限
		if (ptSelectTo.y != ptSelect.y){
			if( bDocumentFeed ){
				// 暫定：未選択として振舞う
				// 改善案：選択範囲は置換されるので、選択範囲の前後をIMEに渡す
				// ptSelectTo.y = ptSelectTo.y;
				ptSelectTo.x = ptSelect.x;
			}else{
				// 2010.04.06 対象をptSelect.yの行からカーソル行に変更
				const CDocLine* pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(GetCaret().GetCaretLogicPos().y);
				CLogicInt targetY = GetCaret().GetCaretLogicPos().y;
				// カーソル行が実質無選択なら、直前・直後の行を選択
				if( ptSelect.y == GetCaret().GetCaretLogicPos().y
						&& pDocLine && pDocLine->GetLengthWithoutEOL() == GetCaret().GetCaretLogicPos().x ){
					// カーソルが上側行末 => 次の行。行末カーソルでのShift+Upなど
					targetY = t_min(m_pcEditDoc->m_cDocLineMgr.GetLineCount(),
						GetCaret().GetCaretLogicPos().y + 1);
					pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(targetY);
				}else
				if( ptSelectTo.y == GetCaret().GetCaretLogicPos().y
						&& 0 == GetCaret().GetCaretLogicPos().x ){
					// カーソルが下側行頭 => 前の行。 行頭でShift+Down/Shift+End→Rightなど
					targetY = GetCaret().GetCaretLogicPos().y - 1;
					pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(targetY);
				}
				// 選択範囲をxで指定：こちらはカーソルではなく選択範囲基準
				if(targetY == ptSelect.y){
					// ptSelect.x; 未変更
					ptSelectTo.x = pDocLine ? pDocLine->GetLengthWithoutEOL() : 0;
				}else
				if(targetY == ptSelectTo.y){
					ptSelect.x = 0;
					// ptSelectTo.x; 未変更
				}else{
					ptSelect.x = 0;
					ptSelectTo.x = pDocLine ? pDocLine->GetLengthWithoutEOL() : 0;
				}
				ptSelect.y = targetY;
				// ptSelectTo.y = targetY; 以下未使用
			}
		}
	}
	else{
		//テキストが選択されていないとき
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos(), &ptSelect);
		ptSelectTo = ptSelect;
	}
	nSelectedLen = ptSelectTo.x - ptSelect.x;
	// 以下 ptSelect.y ptSelect.x, nSelectedLen を使用

	//ドキュメント行取得 -> pcCurDocLine
	const CDocLine* pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(ptSelect.GetY2());
	if (NULL == pcCurDocLine )
		return 0;

	//テキスト取得 -> pLine, nLineLen
	const int nLineLen = pcCurDocLine->GetLengthWithoutEOL();
	if ( 0 == nLineLen )
		return 0;
	const wchar_t* pLine = pcCurDocLine->GetPtr();

	// 2010.04.17 行頭から←選択だと「SelectToが改行の後ろの位置」にあるため範囲を調整する
	// フリーカーソル選択でも行末より後ろにカーソルがある
	if( nLineLen < ptSelect.x ){
		// 改行直前をIMEに渡すカーソル位置ということにする
		ptSelect.x = CLogicInt(nLineLen);
		nSelectedLen = 0;
	}
	if( nLineLen <  ptSelect.x + nSelectedLen ){
		nSelectedLen = nLineLen - ptSelect.x;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              再変換範囲・考慮文字を修正                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//再変換考慮文字列開始  //行の中で再変換のAPIにわたすとする文字列の開始位置
	int nReconvIndex = 0;
	int nInsertCompLen = 0; // DOCUMENTFEED用。変換中の文字列をdwStrに混ぜる
	// Iはカーソル　[]が選択範囲=dwTargetStrLenだとして
	// 行：日本語をIします。
	// IME：にゅうｒ
	// APIに渡す文字列：日本語を[にゅうｒ]Iします。

	// 選択開始位置より前後200(or 50)文字ずつを考慮文字列にする
	const int nReconvMaxLen = (bDocumentFeed ? 50 : 200); //$$マジックナンバー注意
	while (ptSelect.x - nReconvIndex > nReconvMaxLen) {
		nReconvIndex = t_max<int>(nReconvIndex+1, ::CharNextW_AnyBuild(pLine+nReconvIndex)-pLine);
	}
	
	//再変換考慮文字列終了  //行の中で再変換のAPIにわたすとする文字列の長さ
	int nReconvLen = nLineLen - nReconvIndex;
	if ( (nReconvLen + nReconvIndex - ptSelect.x) > nReconvMaxLen ){
		const wchar_t*       p = pLine + ptSelect.x;
		const wchar_t* const q = pLine + ptSelect.x + nReconvMaxLen;
		while (p <= q) {
			p = t_max(p+1, const_cast<LPCWSTR>(::CharNextW_AnyBuild(p)));
		}
		nReconvLen = p - pLine - nReconvIndex;
	}
	
	//対象文字列の調整
	if ( ptSelect.x + nSelectedLen > nReconvIndex + nReconvLen ){
		// 考慮分しかAPIに渡さないので、選択範囲を縮小
		nSelectedLen = nReconvLen + nReconvIndex - ptSelect.x;
	}
	
	if( bDocumentFeed ){
		// IMR_DOCUMENTFEEDでは、再変換対象はIMEから取得した入力中文字列
		nInsertCompLen = auto_strlen(m_szComposition);
		if( 0 == nInsertCompLen ){
			// 2回呼ばれるので、m_szCompositionに覚えておく
			HWND hwnd = GetHwnd();
			HIMC hIMC = ::ImmGetContext( hwnd );
			if( !hIMC ){
				return 0;
			}
			auto_memset(m_szComposition, _T('\0'), _countof(m_szComposition));
			LONG immRet = ::ImmGetCompositionString(hIMC, GCS_COMPSTR, m_szComposition, _countof(m_szComposition));
			if( immRet == IMM_ERROR_NODATA || immRet == IMM_ERROR_GENERAL ){
				m_szComposition[0] = _T('\0');
			}
			::ImmReleaseContext( hwnd, hIMC );
			nInsertCompLen = auto_strlen(m_szComposition);
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      構造体設定要素                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//行の中で再変換のAPIにわたすとする文字列の長さ
	int         cbReconvLenWithNull; // byte
	DWORD       dwReconvTextLen;    // CHARs
	DWORD       dwReconvTextInsLen; // CHARs
	DWORD       dwCompStrOffset;    // byte
	DWORD       dwCompStrLen;       // CHARs
	DWORD       dwInsByteCount = 0; // byte
	CNativeW    cmemBuf1;
	CNativeA    cmemBuf2;
	const void* pszReconv; 
	const void* pszInsBuffer;

	//UNICODE→UNICODE
	if(bUnicode){
		const WCHAR* pszCompInsStr = L"";
		int nCompInsStr   = 0;
		if( nInsertCompLen ){
			pszCompInsStr = to_wchar( m_szComposition );
			nCompInsStr   = wcslen( pszCompInsStr );
		}
		dwInsByteCount      = nCompInsStr * sizeof(wchar_t);
		dwReconvTextLen     = nReconvLen;
		dwReconvTextInsLen  = dwReconvTextLen + nCompInsStr;                 //reconv文字列長。文字単位。
		cbReconvLenWithNull = (dwReconvTextInsLen + 1) * sizeof(wchar_t);    //reconvデータ長。バイト単位。
		dwCompStrOffset     = (Int)(ptSelect.x - nReconvIndex) * sizeof(wchar_t);    //compオフセット。バイト単位。
		dwCompStrLen        = nSelectedLen + nCompInsStr;                            //comp文字列長。文字単位。
		pszReconv           = reinterpret_cast<const void*>(pLine + nReconvIndex);   //reconv文字列へのポインタ。
		pszInsBuffer        = pszCompInsStr;
	}
	//UNICODE→ANSI
	else{
		const wchar_t* pszReconvSrc =  pLine + nReconvIndex;

		//考慮文字列の開始から対象文字列の開始まで -> dwCompStrOffset
		if( ptSelect.x - nReconvIndex > 0 ){
			cmemBuf1.SetString(pszReconvSrc, ptSelect.x - nReconvIndex);
			CShiftJis::UnicodeToSJIS(cmemBuf1, cmemBuf2._GetMemory());
			dwCompStrOffset = cmemBuf2._GetMemory()->GetRawLength();				//compオフセット。バイト単位。
		}else{
			dwCompStrOffset = 0;
		}
		
		pszInsBuffer = "";
		//対象文字列の開始から対象文字列の終了まで -> dwCompStrLen
		if (nSelectedLen > 0 ){
			cmemBuf1.SetString(pszReconvSrc + ptSelect.x, nSelectedLen);
			CShiftJis::UnicodeToSJIS(cmemBuf1, cmemBuf2._GetMemory());
			dwCompStrLen = cmemBuf2._GetMemory()->GetRawLength();					//comp文字列長。文字単位。
		}else if(nInsertCompLen > 0){
			// nSelectedLen と nInsertCompLen が両方指定されることはないはず
			const ACHAR* pComp = to_achar(m_szComposition);
			pszInsBuffer = pComp;
			dwInsByteCount = strlen( pComp );
			dwCompStrLen = dwInsByteCount;
		}else{
			dwCompStrLen = 0;
		}
		
		//考慮文字列すべて
		cmemBuf1.SetString(pszReconvSrc , nReconvLen );
		CShiftJis::UnicodeToSJIS(cmemBuf1, cmemBuf2._GetMemory());
		
		dwReconvTextLen    = cmemBuf2._GetMemory()->GetRawLength();				//reconv文字列長。文字単位。
		dwReconvTextInsLen = dwReconvTextLen + dwInsByteCount;						//reconv文字列長。文字単位。
		cbReconvLenWithNull = cmemBuf2._GetMemory()->GetRawLength() + dwInsByteCount + sizeof(char);		//reconvデータ長。バイト単位。
		
		pszReconv = reinterpret_cast<const void*>(cmemBuf2._GetMemory()->GetRawPtr());	//reconv文字列へのポインタ
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        構造体設定                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if ( NULL != pReconv) {
		//再変換構造体の設定
		DWORD dwOrgSize = pReconv->dwSize;
		// 2010.03.17 Moca dwSizeはpReconvを用意する側(IME等)が設定
		//     のはずなのに Win XP+IME2002+TSF では dwSizeが0で送られてくる
		if( dwOrgSize != 0 && dwOrgSize < sizeof(*pReconv) + cbReconvLenWithNull ){
			// バッファ不足
			m_szComposition[0] = _T('\0');
			return 0;
		}
		else if( 0 == dwOrgSize ){
			pReconv->dwSize = sizeof(*pReconv) + cbReconvLenWithNull;
		}
		pReconv->dwVersion         = 0;
		pReconv->dwStrLen          = dwReconvTextInsLen;	//文字単位
		pReconv->dwStrOffset       = sizeof(*pReconv) ;
		pReconv->dwCompStrLen      = dwCompStrLen;		//文字単位
		pReconv->dwCompStrOffset   = dwCompStrOffset;	//バイト単位
		pReconv->dwTargetStrLen    = dwCompStrLen;		//文字単位
		pReconv->dwTargetStrOffset = dwCompStrOffset;	//バイト単位
		
		// 2004.01.28 Moca ヌル終端の修正
		if( bUnicode ){
			WCHAR* p = (WCHAR*)(pReconv + 1);
			if( dwInsByteCount ){
				// カーソル位置に、入力中IMEデータを挿入
				CHAR* pb = (CHAR*)p;
				CopyMemory(pb, pszReconv, dwCompStrOffset);
				pb += dwCompStrOffset;
				CopyMemory(pb, pszInsBuffer, dwInsByteCount);
				pb += dwInsByteCount;
				CopyMemory(pb, ((char*)pszReconv) + dwCompStrOffset,
					dwReconvTextLen*sizeof(wchar_t) - dwCompStrOffset);
			}else{
				CopyMemory(p, pszReconv, cbReconvLenWithNull - sizeof(wchar_t));
			}
			// \0があると応答なしになることがある
			for( DWORD i = 0; i < dwReconvTextInsLen; i++ ){
				if( p[i] == 0 ){
					p[i] = L' ';
				}
			}
			p[dwReconvTextInsLen] = L'\0';
		}else{
			ACHAR* p = (ACHAR*)(pReconv + 1);
			if( dwInsByteCount ){
				CHAR* pb = p;
				CopyMemory(p, pszReconv, dwCompStrOffset);
				pb += dwCompStrOffset;
				CopyMemory(pb, pszInsBuffer, dwInsByteCount);
				pb += dwInsByteCount;
				CopyMemory(pb, ((char*)pszReconv) + dwCompStrOffset,
					dwReconvTextLen - dwCompStrOffset);
			}else{
				CopyMemory(p, pszReconv, cbReconvLenWithNull - sizeof(char));
			}
			// \0があると応答なしになることがある
			for( DWORD i = 0; i < dwReconvTextInsLen; i++ ){
				if( p[i] == 0 ){
					p[i] = ' ';
				}
			}
			p[dwReconvTextInsLen]='\0';
		}
	}
	
	if( false == bDocumentFeed ){
		// 再変換情報の保存
		m_nLastReconvIndex = nReconvIndex;
		m_nLastReconvLine  = ptSelect.y;
	}
	if( bDocumentFeed && pReconv ){
		m_szComposition[0] = _T('\0');
	}
	return sizeof(RECONVERTSTRING) + cbReconvLenWithNull;
}

/*再変換用 エディタ上の選択範囲を変更する 2002.04.09 minfu */
LRESULT CEditView::SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode){
	
	// 再変換情報が保存されているか
	if ( (m_nLastReconvIndex < 0) || (m_nLastReconvLine < 0))
		return 0;

	if ( GetSelectionInfo().IsTextSelected()) 
		GetSelectionInfo().DisableSelectArea( true );

	if( 0 != pReconv->dwVersion ){
		return 0;
	}
	
	DWORD dwOffset, dwLen;

	//UNICODE→UNICODE
	if(bUnicode){
		dwOffset = pReconv->dwCompStrOffset/sizeof(WCHAR);	//0またはデータ長。バイト単位。→文字単位
		dwLen    = pReconv->dwCompStrLen;					//0または文字列長。文字単位。
	}
	//ANSI→UNICODE
	else{
		CNativeW	cmemBuf;

		//考慮文字列の開始から対象文字列の開始まで
		if( pReconv->dwCompStrOffset > 0){
			if( pReconv->dwSize < (pReconv->dwStrOffset + pReconv->dwCompStrOffset) ){
				return 0;
			}
			// 2010.03.17 sizeof(pReconv)+1ではなくdwStrOffsetを利用するように
			const char* p=((const char*)(pReconv)) + pReconv->dwStrOffset;
			cmemBuf._GetMemory()->SetRawData(p, pReconv->dwCompStrOffset );
			CShiftJis::SJISToUnicode(*(cmemBuf._GetMemory()), &cmemBuf);
			dwOffset = cmemBuf.GetStringLength();
		}else{
			dwOffset = 0;
		}

		//対象文字列の開始から対象文字列の終了まで
		if( pReconv->dwCompStrLen > 0 ){
			if( pReconv->dwSize <
					pReconv->dwStrOffset + pReconv->dwCompStrOffset + pReconv->dwCompStrLen*sizeof(char) ){
				return 0;
			}
			// 2010.03.17 sizeof(pReconv)+1ではなくdwStrOffsetを利用するように
			const char* p= ((const char*)pReconv) + pReconv->dwStrOffset;
			cmemBuf._GetMemory()->SetRawData(p + pReconv->dwCompStrOffset, pReconv->dwCompStrLen);
			CShiftJis::SJISToUnicode(*(cmemBuf._GetMemory()), &cmemBuf);
			dwLen = cmemBuf.GetStringLength();
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
	GetCaret().MoveCursor( GetSelectionInfo().m_sSelect.GetFrom(), true );

	//選択範囲再描画 
	GetSelectionInfo().DrawSelectArea();

	// 再変換情報の破棄
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;

	return 1;

}
