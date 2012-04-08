#include "StdAfx.h"
#include "CEditView.h"
#include "parse/CWordParse.h"

const int STRNCMP_MAX = 100;	/* MAXキーワード長：strnicmp文字列比較最大値(CEditView::KeySearchCore) */	// 2006.04.10 fon

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           検索                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! キーワード辞書検索の前提条件チェックと、検索

	@date 2006.04.10 fon OnTimer, CreatePopUpMenu_Rから分離
*/
BOOL CEditView::KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc )
{
	CNativeW	cmemCurText;
	int			i;

	/* キーワードヘルプを使用するか？ */
	if( !m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyWordHelp )	/* キーワードヘルプ機能を使用する */	// 2006.04.10 fon
		goto end_of_search;
	/* フォーカスがあるか？ */
	if( !GetCaret().ExistCaretFocus() ) 
		goto end_of_search;
	/* ウィンドウ内にマウスカーソルがあるか？ */
	GetCursorPos( po );
	GetWindowRect( GetHwnd(), rc );
	if( !PtInRect( rc, *po ) )
		goto end_of_search;
	switch(nID){
	case LID_SKH_ONTIMER:
		/* 右コメントの１～３でない場合 */
		if(!( m_bInMenuLoop == FALSE	&&			/* １．メニュー モーダル ループに入っていない */
			0 != m_dwTipTimer			&&			/* ２．辞書Tipを表示していない */
			300 < ::GetTickCount() - m_dwTipTimer	/* ３．一定時間以上、マウスが固定されている */
		) )	goto end_of_search;
		break;
	case LID_SKH_POPUPMENU_R:
		if(!( m_bInMenuLoop == FALSE	//&&			/* １．メニュー モーダル ループに入っていない */
		//	0 != m_dwTipTimer			&&			/* ２．辞書Tipを表示していない */
		//	1000 < ::GetTickCount() - m_dwTipTimer	/* ３．一定時間以上、マウスが固定されている */
		) )	goto end_of_search;
		break;
	default:
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, _T("作者に教えて欲しいエラー"),
		_T("CEditView::KeyWordHelpSearchDict\nnID=%d"), (int)nID );
	}
	/* 選択範囲のデータを取得(複数行選択の場合は先頭の行のみ) */
	if( GetSelectedData( &cmemCurText, TRUE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		wchar_t* pszWork = cmemCurText.GetStringPtr();
		int nWorkLength	= wcslen( pszWork );
		for( i = 0; i < nWorkLength; ++i ){
			if( pszWork[i] == L'\0' ||
				pszWork[i] == WCODE::CR ||
				pszWork[i] == WCODE::LF ){
				break;
			}
		}
		wchar_t* pszBuf = new wchar_t[i + 1];
		wmemcpy( pszBuf, pszWork, i );
		pszBuf[i] = L'\0';
		cmemCurText.SetString( pszBuf, i );
		delete [] pszBuf;
	}
	/* キャレット位置の単語を取得する処理 */	// 2006.03.24 fon
	else if(GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord){
		if(!GetParser().GetCurrentWord(&cmemCurText))
			goto end_of_search;
	}
	else
		goto end_of_search;

	if( CNativeW::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) &&	/* 既に検索済みか */
		(!m_cTipWnd.m_KeyWasHit) )								/* 該当するキーがなかった */
		goto end_of_search;
	m_cTipWnd.m_cKey = cmemCurText;

	/* 検索実行 */
	if( !KeySearchCore(&m_cTipWnd.m_cKey) )
		goto end_of_search;
	m_dwTipTimer = 0;		/* 辞書Tipを表示している */
	m_poTipCurPos = *po;	/* 現在のマウスカーソル位置 */
	return TRUE;			/* ここまで来ていればヒット・ワード */

	/* キーワードヘルプ表示処理終了 */
	end_of_search:
	return FALSE;
}

/*! キーワード辞書検索処理メイン

	@date 2006.04.10 fon KeyWordHelpSearchDictから分離
*/
BOOL CEditView::KeySearchCore( const CNativeW* pcmemCurText )
{
	CNativeW*	pcmemRefKey;
	int			nCmpLen = STRNCMP_MAX; // 2006.04.10 fon
	int			nLine; // 2006.04.10 fon


	CTypeConfig nTypeNo = m_pcEditDoc->m_cDocType.GetDocumentType();
	m_cTipWnd.m_cInfo.SetString( _T("") );	/* tooltipバッファ初期化 */
	/* 1行目にキーワード表示の場合 */
	if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpKeyDisp){	/* キーワードも表示する */	// 2006.04.10 fon
		m_cTipWnd.m_cInfo.AppendString( _T("[ ") );
		m_cTipWnd.m_cInfo.AppendString( pcmemCurText->GetStringT() );
		m_cTipWnd.m_cInfo.AppendString( _T(" ]") );
	}
	/* 途中まで一致を使う場合 */
	if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpPrefix)
		nCmpLen = wcslen( pcmemCurText->GetStringPtr() );	// 2006.04.10 fon
	m_cTipWnd.m_KeyWasHit = FALSE;
	for(int i=0;i<CDocTypeManager().GetTypeSetting(nTypeNo).m_nKeyHelpNum;i++){	//最大数：MAX_KEYHELP_FILE
		if( CDocTypeManager().GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_bUse ){
			// 2006.04.10 fon (nCmpLen,pcmemRefKey,nSearchLine)引数を追加
			CNativeW*	pcmemRefText;
			int nSearchResult=m_cDicMgr.CDicMgr::Search(
				pcmemCurText->GetStringPtr(),
				nCmpLen,
				&pcmemRefKey,
				&pcmemRefText,
				CDocTypeManager().GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_szPath,
				&nLine
			);
			if(nSearchResult){
				/* 該当するキーがある */
				LPWSTR		pszWork;
				pszWork = pcmemRefText->GetStringPtr();
				/* 有効になっている辞書を全部なめて、ヒットの都度説明の継ぎ増し */
				if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpAllSearch){	/* ヒットした次の辞書も検索 */	// 2006.04.10 fon
					/* バッファに前のデータが詰まっていたらseparator挿入 */
					if(m_cTipWnd.m_cInfo.GetStringLength() != 0)
						m_cTipWnd.m_cInfo.AppendString( _T("\n--------------------\n■") );
					else
						m_cTipWnd.m_cInfo.AppendString( _T("■") );	/* 先頭の場合 */
					/* 辞書のパス挿入 */
					m_cTipWnd.m_cInfo.AppendString( CDocTypeManager().GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_szPath );
					m_cTipWnd.m_cInfo.AppendString( _T("\n") );
					/* 前方一致でヒットした単語を挿入 */
					if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpPrefix){	/* 選択範囲で前方一致検索 */
						m_cTipWnd.m_cInfo.AppendString( pcmemRefKey->GetStringT() );
						m_cTipWnd.m_cInfo.AppendString( _T(" >>\n") );
					}/* 調査した「意味」を挿入 */
					m_cTipWnd.m_cInfo.AppendStringW( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* タグジャンプ用の情報を残す */
					if(!m_cTipWnd.m_KeyWasHit){
						m_cTipWnd.m_nSearchDict=i;	/* 辞書を開くとき最初にヒットした辞書を開く */
						m_cTipWnd.m_nSearchLine=nLine;
						m_cTipWnd.m_KeyWasHit = TRUE;
					}
				}
				else{	/* 最初のヒット項目のみ返す場合 */
					/* キーワードが入っていたらseparator挿入 */
					if(m_cTipWnd.m_cInfo.GetStringLength() != 0)
						m_cTipWnd.m_cInfo.AppendString( _T("\n--------------------\n") );
					
					/* 前方一致でヒットした単語を挿入 */
					if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpPrefix){	/* 選択範囲で前方一致検索 */
						m_cTipWnd.m_cInfo.AppendString( pcmemRefKey->GetStringT() );
						m_cTipWnd.m_cInfo.AppendString( _T(" >>\n") );
					}
					
					/* 調査した「意味」を挿入 */
					m_cTipWnd.m_cInfo.AppendStringW( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* タグジャンプ用の情報を残す */
					m_cTipWnd.m_nSearchDict=i;
					m_cTipWnd.m_nSearchLine=nLine;
					m_cTipWnd.m_KeyWasHit = TRUE;
					return TRUE;
				}
			}
		}
	}
	if(m_cTipWnd.m_KeyWasHit == TRUE){
			return TRUE;
	}
	/* 該当するキーがなかった場合 */
	return FALSE;
}

/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
void CEditView::GetCurrentTextForSearch( CNativeW& cmemCurText, bool bTrimSpaceTab /* = false */ )
{

	int				i;
	wchar_t			szTopic[_MAX_PATH];
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nIdx;
	CLayoutRange	sRange;

	cmemCurText.SetString(L"");
	szTopic[0] = L'\0';
	if( GetSelectionInfo().IsTextSelected() ){	/* テキストが選択されているか */
		/* 選択範囲のデータを取得 */
		if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
			/* 検索文字列を現在位置の単語で初期化 */
			wcsncpy( szTopic, cmemCurText.GetStringPtr(), _MAX_PATH - 1 );
			szTopic[_MAX_PATH - 1] = L'\0';
		}
	}else{
		const CLayout*	pcLayout;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdx = LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

			/* 現在位置の単語の範囲を調べる */
			bool bWhere = m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
				GetCaret().GetCaretLayoutPos().GetY2(),
				nIdx,
				&sRange,
				NULL,
				NULL
			);
			if( bWhere ){
				/* 選択範囲の変更 */
				GetSelectionInfo().m_sSelectBgn = sRange;
				GetSelectionInfo().m_sSelect    = sRange;

				/* 選択範囲のデータを取得 */
				if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
					/* 検索文字列を現在位置の単語で初期化 */
					wcsncpy( szTopic, cmemCurText.GetStringPtr(), MAX_PATH - 1 );
					szTopic[MAX_PATH - 1] = L'\0';
				}
				/* 現在の選択範囲を非選択状態に戻す */
				GetSelectionInfo().DisableSelectArea( FALSE );
			}
		}
	}

	wchar_t *pTopic2 = szTopic;
	if( bTrimSpaceTab ){
		// 前のスペース・タブを取り除く
		while( L'\0' != *pTopic2 && ( ' ' == *pTopic2 || '\t' == *pTopic2 ) ){
			pTopic2++;
		}
	}
	int nTopic2Len = (int)wcslen( pTopic2 );
	/* 検索文字列は改行まで */
	for( i = 0; i < nTopic2Len; ++i ){
		if( pTopic2[i] == WCODE::CR || pTopic2[i] == WCODE::LF ){
			pTopic2[i] = L'\0';
			break;
		}
	}
	
	if( bTrimSpaceTab ){
		// 後ろのスペース・タブを取り除く
		int m = (int)wcslen( pTopic2 ) - 1;
		while( 0 <= m &&
		    ( ' ' == pTopic2[m] || '\t' == pTopic2[m] ) ){
			m--;
		}
		if( 0 <= m ){
			pTopic2[m + 1] = L'\0';
		}
	}
	cmemCurText.SetString( pTopic2 );
}


/*!	現在カーソル位置単語または選択範囲より検索等のキーを取得（ダイアログ用）
	@date 2006.08.23 ryoji 新規作成
*/
void CEditView::GetCurrentTextForSearchDlg( CNativeW& cmemCurText )
{
	cmemCurText.SetString(L"");

	if( GetSelectionInfo().IsTextSelected() ){	// テキストが選択されている
		GetCurrentTextForSearch( cmemCurText );
	}
	else{	// テキストが選択されていない
		if( GetDllShareData().m_Common.m_sSearch.m_bCaretTextForSearch ){
			GetCurrentTextForSearch( cmemCurText );	// カーソル位置単語を取得
		}
		else{
			cmemCurText.SetString( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0] );	// 履歴からとってくる
		}
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        描画用判定                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 現在位置が検索文字列に該当するか */
//2002.02.08 hor
//正規表現で検索したときの速度改善のため、マッチ先頭位置を引数に追加
//Jun. 26, 2001 genta	正規表現ライブラリの差し替え
/*
	@retval 0
		(パターン検索時) 指定位置以降にマッチはない。
		(それ以外) 指定位置は検索文字列の始まりではない。
	@retval 1,2,3,...
		(パターン検索時) 指定位置以降にマッチが見つかった。
		(単語検索時) 指定位置が検索文字列に含まれる何番目の単語の始まりであるか。
		(それ以外) 指定位置が検索文字列の始まりだった。
*/
int CEditView::IsSearchString(
	const CStringRef&	cStr,
	/*
	const wchar_t*	pszData,
	CLogicInt		nDataLen,
	*/
	CLogicInt		nPos,
	CLogicInt*		pnSearchStart,
	CLogicInt*		pnSearchEnd
) const
{
	CLogicInt		nKeyLength;

	*pnSearchStart = nPos;	// 2002.02.08 hor

	if( m_sCurSearchOption.bRegularExp ){
		/* 行頭ではない? */
		/* 行頭検索チェックは、CBregexpクラス内部で実施するので不要 2003.11.01 かろと */

		/* 位置を0でMatchInfo呼び出すと、行頭文字検索時に、全て true　となり、
		** 画面全体が検索文字列扱いになる不具合修正
		** 対策として、行頭を MacthInfoに教えないといけないので、文字列の長さ・位置情報を与える形に変更
		** 2003.05.04 かろと
		*/
		if( m_CurRegexp.Match( cStr.GetPtr(), cStr.GetLength(), nPos ) ){
			*pnSearchStart = m_CurRegexp.GetIndex();	// 2002.02.08 hor
			*pnSearchEnd = m_CurRegexp.GetLastIndex();
			return 1;
		}
		else{
			return 0;
		}
	}
	else if( m_sCurSearchOption.bWordOnly ) { // 単語検索
		/* 指定位置の単語の範囲を調べる */
		CLogicInt posWordHead, posWordEnd;
		if( ! CWordParse::WhereCurrentWord_2( cStr.GetPtr(), CLogicInt(cStr.GetLength()), nPos, &posWordHead, &posWordEnd, NULL, NULL ) ) {
			return 0; // 指定位置に単語が見つからなかった。
 		}
		if( nPos != posWordHead ) {
			return 0; // 指定位置は単語の始まりではなかった。
		}
		const CLogicInt wordLength = posWordEnd - posWordHead;
		const wchar_t *const pWordHead = cStr.GetPtr() + posWordHead;

		// 比較関数
		int (*const fcmp)( const wchar_t*, const wchar_t*, size_t ) = m_sCurSearchOption.bLoHiCase ? wcsncmp : wcsnicmp;

		// 検索語を単語に分割しながら指定位置の単語と照合する。
		int wordIndex = 0;
		const wchar_t* const searchKeyEnd = m_szCurSrchKey + wcslen( m_szCurSrchKey );
		for( const wchar_t* p = m_szCurSrchKey; p < searchKeyEnd; ) {
			CLogicInt begin, end; // 検索語に含まれる単語?の位置。WhereCurrentWord_2()の仕様では空白文字列も単語に含まれる。
			if( CWordParse::WhereCurrentWord_2( p, CLogicInt(searchKeyEnd - p), CLogicInt(0), &begin, &end, NULL, NULL )
				&& begin == 0 && begin < end
			) {
				if( ! WCODE::IsWordDelimiter( *p ) ) {
					++wordIndex;
					// p...(p + end) が検索語に含まれる wordIndex番目の単語。(wordIndexの最初は 1)
					if( wordLength == end && 0 == fcmp( p, pWordHead, wordLength ) ) {
						*pnSearchStart = posWordHead;
						*pnSearchEnd = posWordEnd;
						return wordIndex;
					}
				}
				p += end;
			} else {
				p += CNativeW::GetSizeOfChar( p, searchKeyEnd - p, 0 );
			}
		}
		return 0; // 指定位置の単語と検索文字列に含まれる単語は一致しなかった。
	}
	else {
		nKeyLength = CLogicInt(wcslen( m_szCurSrchKey ));		/* 検索条件 */

		//検索条件が未定義 または 検索条件の長さより調べるデータが短いときはヒットしない
		if( 0 == nKeyLength || nKeyLength > cStr.GetLength() - nPos ){
			return 0;
		}
		//英大文字小文字の区別をするかどうか
		if( m_sCurSearchOption.bLoHiCase ){	/* 1==英大文字小文字の区別 */
			if( 0 == auto_memcmp( &cStr.GetPtr()[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return 1;
			}
		}else{
			if( 0 == auto_memicmp( &cStr.GetPtr()[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return 1;
			}
		}
	}
	return 0;
}
