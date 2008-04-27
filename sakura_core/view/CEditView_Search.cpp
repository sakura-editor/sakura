#include "stdafx.h"
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
		_T("CEditView::KeyWordHelpSearchDict\nnID=%d") );
	}
	/* 選択範囲のデータを取得(複数行選択の場合は先頭の行のみ) */
	if( GetSelectedData( &cmemCurText, TRUE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
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
	}/* キャレット位置の単語を取得する処理 */	// 2006.03.24 fon
	else if(m_pShareData->m_Common.m_sSearch.m_bUseCaretKeyWord){
		if(!GetParser().GetCurrentWord(&cmemCurText))
			goto end_of_search;
	}else
		goto end_of_search;

	if( CNativeW::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) &&	/* 既に検索済みか */
		(!m_cTipWnd.m_KeyWasHit) )								/* 該当するキーがなかった */
		goto end_of_search;
	m_cTipWnd.m_cKey = cmemCurText;

	/* 検索実行 */
	if( FALSE == KeySearchCore(&m_cTipWnd.m_cKey) )
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
	for(int i=0;i<m_pShareData->GetTypeSetting(nTypeNo).m_nKeyHelpNum;i++){	//最大数：MAX_KEYHELP_FILE
		if( 1 == m_pShareData->GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_nUse ){
			// 2006.04.10 fon (nCmpLen,pcmemRefKey,nSearchLine)引数を追加
			CNativeW*	pcmemRefText;
			int nSearchResult=m_cDicMgr.CDicMgr::Search(
				pcmemCurText->GetStringPtr(),
				nCmpLen,
				&pcmemRefKey,
				&pcmemRefText,
				m_pShareData->GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_szPath,
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
					m_cTipWnd.m_cInfo.AppendString( m_pShareData->GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_szPath );
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
					if(FALSE == m_cTipWnd.m_KeyWasHit){
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
void CEditView::GetCurrentTextForSearch( CNativeW& cmemCurText )
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
		if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
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
				if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
					/* 検索文字列を現在位置の単語で初期化 */
					wcsncpy( szTopic, cmemCurText.GetStringPtr(), MAX_PATH - 1 );
					szTopic[MAX_PATH - 1] = L'\0';
				}
				/* 現在の選択範囲を非選択状態に戻す */
				GetSelectionInfo().DisableSelectArea( FALSE );
			}
		}
	}

	/* 検索文字列は改行まで */
	for( i = 0; i < (int)wcslen( szTopic ); ++i ){
		if( szTopic[i] == WCODE::CR || szTopic[i] == WCODE::LF ){
			szTopic[i] = L'\0';
			break;
		}
	}
	cmemCurText.SetString( szTopic );
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
		if( m_pShareData->m_Common.m_sSearch.m_bCaretTextForSearch ){
			GetCurrentTextForSearch( cmemCurText );	// カーソル位置単語を取得
		}
		else{
			cmemCurText.SetString( m_pShareData->m_aSearchKeys[0] );	// 履歴からとってくる
		}
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        描画用判定                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 現在位置が検索文字列に該当するか */
//2002.02.08 hor
//正規表現で検索したときの速度改善のため、マッチ先頭位置を引数に追加
bool CEditView::IsSearchString(
	const wchar_t*	pszData,
	CLogicInt		nDataLen,
	CLogicInt		nPos,
	CLogicInt*		pnSearchStart,
	CLogicInt*		pnSearchEnd
)
{
	CLogicInt		nKeyLength;

	//	From Here Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	*pnSearchStart = nPos;	// 2002.02.08 hor

	if( m_sCurSearchOption.bRegularExp ){
		/* 行頭ではない? */
		/* 行頭検索チェックは、CBregexpクラス内部で実施するので不要 2003.11.01 かろと */

		/* 位置を0でMatchInfo呼び出すと、行頭文字検索時に、全て true　となり、
		** 画面全体が検索文字列扱いになる不具合修正
		** 対策として、行頭を MacthInfoに教えないといけないので、文字列の長さ・位置情報を与える形に変更
		** 2003.05.04 かろと
		*/
		if( m_CurRegexp.Match( pszData, nDataLen, nPos ) ){
			*pnSearchStart = m_CurRegexp.GetIndex();	// 2002.02.08 hor
			*pnSearchEnd = m_CurRegexp.GetLastIndex();
	//	To Here Jun. 26, 2001 genta
			return true;

		}else{
			return false;
		}
	}else{
		nKeyLength = CLogicInt(wcslen( m_szCurSrchKey ));		/* 検索条件 */

		// 2001/06/23 単語単位の検索のために追加
		if( m_pShareData->m_Common.m_sSearch.m_sSearchOption.bWordOnly ){	/* 検索／置換  1==単語のみ検索 */
			/* 現在位置の単語の範囲を調べる */
			/* 現在位置の単語の範囲を調べる */
			CLogicInt nIdxFrom, nIdxTo;
			if( !CWordParse::WhereCurrentWord_2( pszData, nDataLen, nPos, &nIdxFrom, &nIdxTo, NULL, NULL ) ){
				return false;
			}
			if( nPos != nIdxFrom || nKeyLength != nIdxTo - nIdxFrom ){
				return false;
			}
		}

		//検索条件が未定義 または 検索条件の長さより調べるデータが短いときはヒットしない
		if( 0 == nKeyLength || nKeyLength > nDataLen - nPos ){
			return false;
		}
		//英大文字小文字の区別をするかどうか
		if( m_sCurSearchOption.bLoHiCase ){	/* 1==英大文字小文字の区別 */
			if( 0 == wmemcmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return true;
			}
		}else{
			if( 0 == auto_memicmp( &pszData[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return true;
			}
		}
	}
	return false;
}
