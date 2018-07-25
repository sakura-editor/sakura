/*!	@file
	@brief CEditViewクラスのインクリメンタルサーチ関連コマンド処理系関数群

	@author genta
	@date	2005/01/10 作成
*/
/*
	Copyright (C) 2004, isearch
	Copyright (C) 2005, genta, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h"
#include "extmodule/CMigemo.h"
#include "sakura_rc.h"

/*!
	コマンドコードの変換(ISearch時)及び
	インクリメンタルサーチモードを抜ける判定

	@return true: コマンド処理済み / false: コマンド処理継続

	@date 2004.09.14 isearch 新規作成
	@date 2005.01.10 genta 関数化, UNINDENT追加

	@note UNINDENTを通常文字として扱うのは，
		SHIFT+文字の後でSPACEを入力するようなケースで
		SHIFTの解放が遅れても文字が入らなくなることを防ぐため．
*/
void CEditView::TranslateCommand_isearch(
	EFunctionCode&	nCommand,
	bool&			bRedraw,
	LPARAM&			lparam1,
	LPARAM&			lparam2,
	LPARAM&			lparam3,
	LPARAM&			lparam4
)
{
	if (m_nISearchMode <= SEARCH_NONE )
		return;

	switch (nCommand){
		//これらの機能のとき、インクリメンタルサーチに入る
		case F_ISEARCH_NEXT:
		case F_ISEARCH_PREV:
		case F_ISEARCH_REGEXP_NEXT:
		case F_ISEARCH_REGEXP_PREV:
		case F_ISEARCH_MIGEMO_NEXT:
		case F_ISEARCH_MIGEMO_PREV:
			break;

		//以下の機能のとき、インクリメンタルサーチ中は検索文字入力として処理
		case F_WCHAR:
		case F_IME_CHAR:
			nCommand = F_ISEARCH_ADD_CHAR;
			break;
		case F_INSTEXT_W:
			nCommand = F_ISEARCH_ADD_STR;
			break;

		case F_INDENT_TAB:	// TABはインデントではなく単なるTAB文字と見なす
		case F_UNINDENT_TAB:	// genta追加
			nCommand = F_ISEARCH_ADD_CHAR;
			lparam1 = '\t';
			break;
		case F_INDENT_SPACE:	// スペースはインデントではなく単なるTAB文字と見なす
		case F_UNINDENT_SPACE:	// genta追加
			nCommand = F_ISEARCH_ADD_CHAR;
			lparam1 = ' ';
			break;
		case F_DELETE_BACK:
			nCommand = F_ISEARCH_DEL_BACK;
			break;

		default:
			//上記以外のコマンドの場合はインクリメンタルサーチを抜ける
			ISearchExit();
	}
}

/*!
	ISearch コマンド処理

	@date 2005.01.10 genta 各コマンドに入っていた処理を1カ所に移動
*/
bool CEditView::ProcessCommand_isearch(
	int	nCommand,
	bool	bRedraw,
	LPARAM	lparam1,
	LPARAM	lparam2,
	LPARAM	lparam3,
	LPARAM	lparam4
)
{
	switch( nCommand ){
		//	検索文字列の変更操作
		case F_ISEARCH_ADD_CHAR:
			ISearchExec((DWORD)lparam1);
			return true;
		
		case F_ISEARCH_DEL_BACK:
			ISearchBack();
			return true;

		case F_ISEARCH_ADD_STR:
			ISearchExec((LPCWSTR)lparam1);
			return true;

		//	検索モードへの移行
		case F_ISEARCH_NEXT:
			ISearchEnter(SEARCH_NORMAL, SEARCH_FORWARD);	//前方インクリメンタルサーチ //2004.10.13 isearch
			return true;
		case F_ISEARCH_PREV:
			ISearchEnter(SEARCH_NORMAL, SEARCH_BACKWARD);	//後方インクリメンタルサーチ //2004.10.13 isearch
			return true;
		case F_ISEARCH_REGEXP_NEXT:
			ISearchEnter(SEARCH_REGEXP, SEARCH_FORWARD);	//前方正規表現インクリメンタルサーチ  //2004.10.13 isearch
			return true;
		case F_ISEARCH_REGEXP_PREV:
			ISearchEnter(SEARCH_REGEXP, SEARCH_BACKWARD);	//後方正規表現インクリメンタルサーチ  //2004.10.13 isearch
			return true;
		case F_ISEARCH_MIGEMO_NEXT:
			ISearchEnter(SEARCH_MIGEMO, SEARCH_FORWARD);	//前方MIGEMOインクリメンタルサーチ    //2004.10.13 isearch
			return true;
		case F_ISEARCH_MIGEMO_PREV:
			ISearchEnter(SEARCH_MIGEMO, SEARCH_BACKWARD);	//後方MIGEMOインクリメンタルサーチ    //2004.10.13 isearch
			return true;
	}
	return false;
}

/*!
	インクリメンタルサーチモードに入る

	@param mode [in] 検索方法 1:通常, 2:正規表現, 3:MIGEMO
	@param direction [in] 検索方向 0:後方(上方), 1:前方(下方)

	@author isearch
	@date 2011.12.15 Moca m_sCurSearchOption/m_sSearchOptionと同期をとる
	@date 2012.10.11 novice m_sCurSearchOption/m_sSearchOptionの同期をswitchの前に変更
	@date 2012.10.11 novice MIGEMOの処理をcase内に移動
*/
void CEditView::ISearchEnter( ESearchMode mode, ESearchDirection direction)
{

	if (m_nISearchMode == mode ) {
		//再実行
		m_nISearchDirection =  direction;
		
		if ( m_bISearchFirst ){
			m_bISearchFirst = false;
		}
		//ちょっと修正
		ISearchExec(true);

	}else{
		//インクリメンタルサーチモードに入るだけ.		
		//選択範囲の解除
		if(GetSelectionInfo().IsTextSelected())	
			GetSelectionInfo().DisableSelectArea( true );

		m_sCurSearchOption = GetDllShareData().m_Common.m_sSearch.m_sSearchOption;
		switch( mode ) {
			case SEARCH_NORMAL: // 通常インクリメンタルサーチ
				m_sCurSearchOption.bRegularExp = false;
				m_sCurSearchOption.bLoHiCase = false;
				m_sCurSearchOption.bWordOnly = false;
				//SendStatusMessage(_T("I-Search: "));
				break;
			case SEARCH_REGEXP: // 正規表現インクリメンタルサーチ
				if (!m_CurRegexp.IsAvailable()){
					WarningBeep();
					SendStatusMessage(LS(STR_EDITVWISRCH_REGEX));
					return;
				}
				m_sCurSearchOption.bRegularExp = true;
				m_sCurSearchOption.bLoHiCase = false;
				//SendStatusMessage(_T("[RegExp] I-Search: "));
				break;
			case SEARCH_MIGEMO: // MIGEMOインクリメンタルサーチ
				if (!m_CurRegexp.IsAvailable()){
					WarningBeep();
					SendStatusMessage(LS(STR_EDITVWISRCH_REGEX));
					return;
				}
				if(m_pcmigemo==NULL){
					m_pcmigemo = CMigemo::getInstance();
					m_pcmigemo->InitDll();
				}
				//migemo dll チェック
				//	Jan. 10, 2005 genta 設定変更で使えるようになっている
				//	可能性があるので，使用可能でなければ一応初期化を試みる
				if ( !m_pcmigemo->IsAvailable() && DLL_SUCCESS != m_pcmigemo->InitDll() ){
					WarningBeep();
					SendStatusMessage(LS(STR_EDITVWISRCH_MIGEGO1));
					return;
				}
				m_pcmigemo->migemo_load_all();
				if (m_pcmigemo->migemo_is_enable()) {
					m_sCurSearchOption.bRegularExp = true;
					m_sCurSearchOption.bLoHiCase = false;
					//SendStatusMessage(_T("[MIGEMO] I-Search: "));
				}else{
					WarningBeep();
					SendStatusMessage(LS(STR_EDITVWISRCH_MIGEGO2));
					return;
				}
				break;
		}
		
		//	Feb. 04, 2005 genta	検索開始位置を記録
		//	インクリメンタルサーチ間でモードを切り替える場合には開始と見なさない
		if( m_nISearchMode == SEARCH_NONE ){
			m_ptSrchStartPos_PHY = GetCaret().GetCaretLogicPos();
		}
		
		m_bCurSrchKeyMark = false;
		m_nISearchDirection = direction;
		m_nISearchMode = mode;
		
		m_nISearchHistoryCount = 0;
		m_sISearchHistory[m_nISearchHistoryCount].Set(GetCaret().GetCaretLayoutPos());

		Redraw();
		
		CNativeT msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetStringPtr());
		
		m_bISearchWrap = false;
		m_bISearchFirst = true;
	}

	//マウスカーソル変更
	if (direction == 1){
		::SetCursor( ::LoadCursor( G_AppInstance(),MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
	}else{
		::SetCursor( ::LoadCursor( G_AppInstance(),MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
	}
}

//!	インクリメンタルサーチモードから抜ける
void CEditView::ISearchExit()
{
	// シーケンスを上書きして現在の検索キーを維持する
	if( m_strCurSearchKey.size() < _MAX_PATH ){
		CSearchKeywordManager().AddToSearchKeyArr( m_strCurSearchKey.c_str() );
	}
	m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;
	GetDllShareData().m_Common.m_sSearch.m_sSearchOption = m_sCurSearchOption;
	m_pcEditWnd->m_cToolbar.AcceptSharedSearchKey();
	m_nISearchDirection = SEARCH_BACKWARD;
	m_nISearchMode = SEARCH_NONE;

	if (m_nISearchHistoryCount == 0){
		m_strCurSearchKey.clear();
	}

	//マウスカーソルを元に戻す
	POINT point1;
	GetCursorPos(&point1);
	OnMOUSEMOVE(0,point1.x,point1.y);

	//ステータス表示エリアをクリア
	SendStatusMessage(_T(""));

}

/*!
	@brief インクリメンタルサーチの実行(1文字追加)
	
	@param wChar [in] 追加する文字 (1byte or 2byte)
*/
void CEditView::ISearchExec(DWORD wChar)
{
	//特殊文字は処理しない
	switch ( wChar){
		case L'\r':
		case L'\n':
			ISearchExit();
			return;
		//case '\t':
		//	break;
	}
	
	if (m_bISearchFirst){
		m_bISearchFirst = false;
		m_strCurSearchKey.clear();
	}

	if( wChar <= 0xffff ){
		m_strCurSearchKey.append(1, (WCHAR)wChar);
	}else{
		m_strCurSearchKey.append(1, (WCHAR)(wChar>>16));
		m_strCurSearchKey.append(1, (WCHAR)wChar);
	}

	ISearchExec(false);
	return ;
}

/*!
	@brief インクリメンタルサーチの実行(文字列追加)
	
	@param pszText [in] 追加する文字列
*/
void CEditView::ISearchExec(LPCWSTR pszText)
{
	//一文字ずつ分解して実行

	const WCHAR* p;
	DWORD c;
	p = pszText;
	
	while(*p!=L'\0'){
		if( IsUtf16SurrogHi(*p) && IsUtf16SurrogLow(*(p+1)) ){
			c = ( ((WORD)*p)<<16 ) | ( (WORD)*(p+1) );
			p++;
		}else{
			c = *p;
		}
		ISearchExec(c);
		p++;
	}
	return ;
}

/*!
	@brief インクリメンタルサーチの実行

	@param bNext [in] true:次の候補を検索, false:現在のカーソル位置のまま検索
*/
void CEditView::ISearchExec(bool bNext) 
{
	//検索を実行する.

	if ( (m_strCurSearchKey.size() == 0) || (m_nISearchMode == SEARCH_NONE)){
		//ステータスの表示
		CNativeT msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetStringPtr());
		return ;
	}
	
	ISearchWordMake();
	
	CLayoutInt	nLine(0);
	CLayoutInt	nIdx1(0);
	
	if ( bNext && m_bISearchWrap ) {
		switch (m_nISearchDirection)
		{
		case SEARCH_FORWARD:
			nLine = CLayoutInt(0);
			nIdx1 = CLayoutInt(0);
			break;
		case SEARCH_BACKWARD:
			//最後から検索
			CLogicInt nLineP;
			int nIdxP;
			nLineP =  m_pcEditDoc->m_cDocLineMgr.GetLineCount() - CLogicInt(1);
			const CDocLine* pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( nLineP );
			nIdxP = pDocLine->GetLengthWithEOL() -1;
			CLayoutPoint ptTmp;
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(CLogicPoint(nIdxP,nLineP),&ptTmp);
			nIdx1=ptTmp.GetX2();
			nLine=ptTmp.GetY2();
		}
	}else if (GetSelectionInfo().IsTextSelected()){
		switch( m_nISearchDirection * 2 + (bNext ? 1: 0)){
			case (SEARCH_FORWARD * 2): //前方検索で現在位置から検索のとき
			case (SEARCH_BACKWARD * 2 + 1): //後方検索で次を検索のとき
				//選択範囲の先頭を検索開始位置に
				nLine = GetSelectionInfo().m_sSelect.GetFrom().GetY2();
				nIdx1 = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
				break;
			case (SEARCH_BACKWARD * 2): //後方検索で現在位置から検索
			case (SEARCH_FORWARD * 2 + 1): //前方検索で次を検索
				//選択範囲の後ろから
				nLine = GetSelectionInfo().m_sSelect.GetTo().GetY2();
				nIdx1 = GetSelectionInfo().m_sSelect.GetTo().GetX2();
				break;
		}
	}else{
		nLine = GetCaret().GetCaretLayoutPos().GetY2();
		nIdx1  = GetCaret().GetCaretLayoutPos().GetX2();
	}

	//桁位置からindexに変換
	const CLayout* pCLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLine );
	CLogicInt nIdx = LineColumnToIndex( pCLayout, nIdx1 );

	m_nISearchHistoryCount ++ ;

	CNativeT msg;
	ISearchSetStatusMsg(&msg);

	if (m_nISearchHistoryCount >= 256) {
		m_nISearchHistoryCount = 156;
		for(int i = 100 ; i<= 255 ; i++){
			m_bISearchFlagHistory[i-100] = m_bISearchFlagHistory[i];
			m_sISearchHistory[i-100] = m_sISearchHistory[i];
		}
	}
	m_bISearchFlagHistory[m_nISearchHistoryCount] = bNext;

	CLayoutRange sMatchRange;

	int nSearchResult = m_pcEditDoc->m_cLayoutMgr.SearchWord(
		nLine,						// 検索開始レイアウト行
		nIdx,						// 検索開始データ位置
		m_nISearchDirection,		// 検索方向
		&sMatchRange,				// マッチレイアウト範囲
		m_sSearchPattern
	);
	if( nSearchResult == 0 ){
		/*検索結果がない*/
		msg.AppendString(LS(STR_EDITVWISRCH_NOMATCH));
		SendStatusMessage(msg.GetStringPtr());
		
		if (bNext) 	m_bISearchWrap = true;
		if (GetSelectionInfo().IsTextSelected()){
			m_sISearchHistory[m_nISearchHistoryCount] = GetSelectionInfo().m_sSelect;
		}else{
			m_sISearchHistory[m_nISearchHistoryCount].Set(GetCaret().GetCaretLayoutPos());
		}
	}else{
		//検索結果あり
		//キャレット移動
		GetCaret().MoveCursor( sMatchRange.GetFrom(), true, _CARETMARGINRATE / 3 );
		
		//	2005.06.24 Moca
		GetSelectionInfo().SetSelectArea( sMatchRange );

		m_bISearchWrap = false;
		m_sISearchHistory[m_nISearchHistoryCount] = sMatchRange;
	}

	m_bCurSrchKeyMark = true;

	Redraw();	
	SendStatusMessage(msg.GetStringPtr());
	return ;
}

//!	バックスペースを押されたときの処理
void CEditView::ISearchBack(void) {
	if(m_nISearchHistoryCount==0) return;
	
	if(m_nISearchHistoryCount==1){
		m_bCurSrchKeyMark = false;
		m_bISearchFirst = true;
	}else if( m_bISearchFlagHistory[m_nISearchHistoryCount] == false){
		//検索文字をへらす
		size_t l = m_strCurSearchKey.size();
		if (l > 0 ){
			//最後の文字の一つ前
			wchar_t* p = (wchar_t*)CNativeW::GetCharPrev( m_strCurSearchKey.c_str(), l, &m_strCurSearchKey.c_str()[l] );
			size_t new_len = p - m_strCurSearchKey.c_str();
			m_strCurSearchKey.resize( new_len );
			//m_szCurSrchKey[l-1] = '\0';

			if ( new_len > 0 ) 
				ISearchWordMake();
			else
				m_bCurSrchKeyMark = false;

		}else{
			WarningBeep();
		}
	}
	m_nISearchHistoryCount --;

	CLayoutRange sRange = m_sISearchHistory[m_nISearchHistoryCount];

	if(m_nISearchHistoryCount == 0){
		GetSelectionInfo().DisableSelectArea( true );
		sRange.SetToX( sRange.GetFrom().x );
	}

	GetCaret().MoveCursor( sRange.GetFrom(), true, _CARETMARGINRATE / 3 );
	if(m_nISearchHistoryCount != 0){
		//	2005.06.24 Moca
		GetSelectionInfo().SetSelectArea( sRange );
	}

	Redraw();

	//ステータス表示
	CNativeT msg;
	ISearchSetStatusMsg(&msg);
	SendStatusMessage(msg.GetStringPtr());
	
}

//!	入力文字から、検索文字を生成する。
void CEditView::ISearchWordMake(void)
{
	switch ( m_nISearchMode ) {
	case SEARCH_NORMAL: // 通常インクリメンタルサーチ
	case SEARCH_REGEXP: // 正規表現インクリメンタルサーチ
		m_sSearchPattern.SetPattern(this->GetHwnd(), m_strCurSearchKey.c_str(), m_strCurSearchKey.size(), m_sCurSearchOption, &m_CurRegexp);
		break;
	case SEARCH_MIGEMO: // MIGEMOインクリメンタルサーチ
		{
			//migemoで捜す
			std::wstring strMigemoWord = m_pcmigemo->migemo_query_w(m_strCurSearchKey.c_str());
			
			/* 検索パターンのコンパイル */
			const wchar_t* p = strMigemoWord.c_str();
			m_sSearchPattern.SetPattern(this->GetHwnd(), p, (int)strMigemoWord.size(), m_sCurSearchOption, &m_CurRegexp);

		}
		break;
	}
}

/*!	@brief ISearchメッセージ構築

	現在のサーチモード及び検索中文字列から
	メッセージエリアに表示する文字列を構築する
	
	@param msg [out] メッセージバッファ
	
	@author isearch
	@date 2004/10/13
	@date 2005.01.13 genta 文字列修正
*/
void CEditView::ISearchSetStatusMsg(CNativeT* msg) const
{

	switch ( m_nISearchMode){
	case SEARCH_NORMAL:
		msg->SetString(_T("I-Search") );
		break;
	case SEARCH_REGEXP:
		msg->SetString(_T("[RegExp] I-Search") );
		break;
	case SEARCH_MIGEMO:
		msg->SetString(_T("[Migemo] I-Search") );
		break;
	default:
		msg->SetString(_T(""));
		return;
	}
	if (m_nISearchDirection == SEARCH_BACKWARD){
		msg->AppendString(_T(" Backward: "));
	}
	else{
		msg->AppendString(_T(": "));
	}

	if(m_nISearchHistoryCount > 0)
		msg->AppendString(to_tchar(m_strCurSearchKey.c_str()));
}

/*!
	ISearch状態をツールバーに反映させる．
	
	@sa CEditWnd::IsFuncChecked()

	@param nCommand [in] 調べたいコマンドのID
	@return true:チェック有り / false: チェック無し
	
	@date 2005.01.10 genta 新規作成
*/
bool CEditView::IsISearchEnabled(int nCommand) const
{
	switch( nCommand )
	{
	case F_ISEARCH_NEXT:
		return (m_nISearchMode == SEARCH_NORMAL) && (m_nISearchDirection == SEARCH_FORWARD);
	case F_ISEARCH_PREV:
		return (m_nISearchMode == SEARCH_NORMAL) && (m_nISearchDirection == SEARCH_BACKWARD);
	case F_ISEARCH_REGEXP_NEXT:
		return (m_nISearchMode == SEARCH_REGEXP) && (m_nISearchDirection == SEARCH_FORWARD);
	case F_ISEARCH_REGEXP_PREV:
		return (m_nISearchMode == SEARCH_REGEXP) && (m_nISearchDirection == SEARCH_BACKWARD);
	case F_ISEARCH_MIGEMO_NEXT:
		return (m_nISearchMode == SEARCH_MIGEMO) && (m_nISearchDirection == SEARCH_FORWARD);
	case F_ISEARCH_MIGEMO_PREV:
		return (m_nISearchMode == SEARCH_MIGEMO) && (m_nISearchDirection == SEARCH_BACKWARD);
	}
	return false;
}

