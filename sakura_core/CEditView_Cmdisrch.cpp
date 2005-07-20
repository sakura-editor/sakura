//	$Id$
/*!	@file
	@brief CEditViewクラスのインクリメンタルサーチ関連コマンド処理系関数群

	@date	2005/01/10 作成
	$Revision$
*/
/*
	Copyright (C) 2004, isearch
	Copyright (C) 2005, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "sakura_rc.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "CDocLine.h"
#include "CMigemo.h"
#include "etc_uty.h"

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
	int&	nCommand,
	BOOL&	bRedraw,
	LPARAM&	lparam1,
	LPARAM&	lparam2,
	LPARAM&	lparam3,
	LPARAM&	lparam4
)
{
	if (m_nISearchMode <= 0 )
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
		case F_CHAR:
		case F_IME_CHAR:
			nCommand = F_ISEARCH_ADD_CHAR;
			break;
		case F_INSTEXT:
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
	BOOL	bRedraw,
	LPARAM	lparam1,
	LPARAM	lparam2,
	LPARAM	lparam3,
	LPARAM	lparam4
)
{
	switch( nCommand ){
		//	検索文字列の変更操作
		case F_ISEARCH_ADD_CHAR:
			ISearchExec((WORD)lparam1);
			return true;
		
		case F_ISEARCH_DEL_BACK:
			ISearchBack();
			return true;

		case F_ISEARCH_ADD_STR:
			ISearchExec((const char*)lparam1);
			return true;

		//	検索モードへの移行
		case F_ISEARCH_NEXT:
			ISearchEnter(1,1);	//前方インクリメンタルサーチ //2004.10.13 isearch
			return true;
		case F_ISEARCH_PREV:
			ISearchEnter(1,0);	//後方インクリメンタルサーチ //2004.10.13 isearch
			return true;
		case F_ISEARCH_REGEXP_NEXT:
			ISearchEnter(2,1);	//前方正規表現インクリメンタルサーチ  //2004.10.13 isearch
			return true;
		case F_ISEARCH_REGEXP_PREV:
			ISearchEnter(2,0);	//後方正規表現インクリメンタルサーチ  //2004.10.13 isearch
			return true;
		case F_ISEARCH_MIGEMO_NEXT:
			ISearchEnter(3,1);	//前方MIGEMOインクリメンタルサーチ    //2004.10.13 isearch
			return true;
		case F_ISEARCH_MIGEMO_PREV:
			ISearchEnter(3,0);	//後方MIGEMOインクリメンタルサーチ    //2004.10.13 isearch
			return true;
	}
	return false;
}

/*!
	インクリメンタルサーチモードに入る

	@param mode [in] 検索方法 1:通常, 2:正規表現, 3:MIGEMO
	@param direction [in] 検索方向 0:後方(上方), 1:前方(下方)

	@author isearch
*/
void CEditView::ISearchEnter( int mode  ,int direction)
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
		if(IsTextSelected())	
			DisableSelectArea( TRUE );
		
		if(m_pcmigemo==NULL){
			m_pcmigemo = CMigemo::getInstance();
			m_pcmigemo->Init();
		}
		switch( mode ) {
			case 1: // 通常インクリメンタルサーチ
				m_bCurSrchRegularExp = FALSE;
				m_pShareData->m_Common.m_bRegularExp = FALSE;
				//SendStatusMessage("I-Search: ");
				break;
			case 2: // 正規表現インクリメンタルサーチ
				if (!m_CurRegexp.IsAvailable()){
					MessageBeep(MB_ICONEXCLAMATION);
					SendStatusMessage("BREGREP.DLLが使用できません。");
					return;
				}
				m_bCurSrchRegularExp = TRUE;
				m_pShareData->m_Common.m_bRegularExp = TRUE;
				//SendStatusMessage("[RegExp] I-Search: ");
				break;
			case 3: // MIGEMOインクリメンタルサーチ
				if (!m_CurRegexp.IsAvailable()){
					MessageBeep(MB_ICONEXCLAMATION);
					SendStatusMessage("BREGREP.DLLが使用できません。");
					return;
				}
				//migemo dll チェック
				//	Jan. 10, 2005 genta 設定変更で使えるようになっている
				//	可能性があるので，使用可能でなければ一応初期化を試みる
				if ( ! m_pcmigemo->IsAvailable() && ! m_pcmigemo->Init() ){
					MessageBeep(MB_ICONEXCLAMATION);
					SendStatusMessage("MIGEMO.DLLが使用できません。");
					return;
				}
				m_pcmigemo->migemo_load_all();
				if (m_pcmigemo->migemo_is_enable()) {
					m_bCurSrchRegularExp = TRUE;
					m_pShareData->m_Common.m_bRegularExp = TRUE;
					//SendStatusMessage("[MIGEMO] I-Search: ");
				}else{
					MessageBeep(MB_ICONEXCLAMATION);
					SendStatusMessage("MIGEMOは使用できません。 ");
					return;
				}
				break;
		}
		
		//	Feb. 04, 2005 genta	検索開始位置を記録
		//	インクリメンタルサーチ間でモードを切り替える場合には開始と見なさない
		if( m_nISearchMode == 0 ){
			m_nSrchStartPosX_PHY = m_nCaretPosX_PHY;
			m_nSrchStartPosY_PHY = m_nCaretPosY_PHY;
		}
		
		//m_szCurSrchKey[0] = '\0';
		m_bCurSrchKeyMark = FALSE;
		m_nISearchDirection = direction;
		m_nISearchMode = mode;
		
		m_nISearchHistoryCount = 0;
		m_nISearchX1History[m_nISearchHistoryCount] = m_nCaretPosX;
		m_nISearchY1History[m_nISearchHistoryCount] = m_nCaretPosY;
		m_nISearchX2History[m_nISearchHistoryCount] = m_nCaretPosX;
		m_nISearchY2History[m_nISearchHistoryCount] = m_nCaretPosY;

		Redraw();
		
		CMemory msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetPtr());
		
		m_bISearchWrap = false;
		m_bISearchFirst = true;
	}

	//マウスカーソル変更
	if (direction == 1){
		::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
	}else{
		::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
	}
}

//!	インクリメンタルサーチモードから抜ける
void CEditView::ISearchExit()
{
	CShareData::getInstance()->AddToSearchKeyArr( m_szCurSrchKey );
	m_nISearchDirection = 0;
	m_nISearchMode = 0;
	
	if (m_nISearchHistoryCount == 0){
		m_szCurSrchKey[0] = '\0';
	}

	//マウスカーソルを元に戻す
	POINT point1;
	GetCursorPos(&point1);
	OnMOUSEMOVE(0,point1.x,point1.y);

	//ステータス表示エリアをクリア
	SendStatusMessage("");

}

/*!
	@brief インクリメンタルサーチの実行(1文字追加)
	
	@param wChar [in] 追加する文字 (1byte or 2byte)
*/
void CEditView::ISearchExec(WORD wChar)
{
	//特殊文字は処理しない
	switch ( wChar){
		case '\r':
		case '\n':
			ISearchExit();
			return;
		//case '\t':
		//	break;
	}
	
	int l;
	if (m_bISearchFirst){
		m_bISearchFirst = false;
		l = 0 ;
	}else	
		l = strlen(m_szCurSrchKey) ;

	if (wChar <= 255 ) {
		if ( l < sizeof(m_szCurSrchKey) - 1) {
			m_szCurSrchKey[l] =(char)wChar;
			m_szCurSrchKey[l+1] = '\0';				
		}
	}else{
		if ( l < sizeof(m_szCurSrchKey) - 2) {
			m_szCurSrchKey[l]   =(char)(wChar>>8);
			m_szCurSrchKey[l+1] =(char)wChar;
			m_szCurSrchKey[l+2] = '\0';				
		}
	}

	ISearchExec(false);
	return ;
}

/*!
	@brief インクリメンタルサーチの実行(文字列追加)
	
	@param pszText [in] 追加する文字列
*/
void CEditView::ISearchExec(const char* pszText)
{
	//一文字ずつ分解して実行

	const char* p;
	WORD  c;
	p = pszText;
	
	while(*p!='\0'){
		if (IsDBCSLeadByte(*p)){
			c =( ((WORD)*p) * 256) | (unsigned char)*(p+1);
			p++;
		}else
			c =*p;
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
	int nLineFrom, nColmFrom, nColmTo, nLineTo;
	
	if ( (m_szCurSrchKey[0] == '\0') || (m_nISearchMode == 0)){
		//ステータスの表示
		CMemory msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetPtr());		
		return ;
	}
	
	ISearchWordMake();
	
	int nLine, nIdx,nIdx1;
	
	if ( bNext && m_bISearchWrap ) {
		switch (m_nISearchDirection)
		{
		case 1:
			nLine = 0;
			nIdx1 = 0;
			break;
		case 0:
			//最後から検索
			int nLineP, nIdxP;
			nLineP =  m_pcEditDoc->m_cDocLineMgr.GetLineCount() -1 ;
			CDocLine* pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( nLineP );
			nIdxP = pDocLine->GetLength() -1;
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(nIdxP,nLineP,&nIdx1,&nLine);
		}
	}else if (IsTextSelected()){
		switch( m_nISearchDirection * 2 + (bNext ? 1: 0)){
			case 2 : //前方検索で現在位置から検索のとき
			case 1 : //後方検索で次を検索のとき
				//選択範囲の先頭を検索開始位置に
				nLine = m_nSelectLineFrom;
				nIdx1 = m_nSelectColmFrom;
				break;
			case 0 : //前方検索で次を検索
			case 3 : //後方検索で現在位置から検索
				//選択範囲の後ろから
				nLine = m_nSelectLineTo;
				nIdx1 = m_nSelectColmTo;
				break;
		}
	}else{
		nLine = m_nCaretPosY;
		nIdx1  = m_nCaretPosX;
	}

	//桁位置からindexに変換
	CLayout* pCLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLine );
	nIdx = LineColmnToIndex( pCLayout, nIdx1 );

	m_nISearchHistoryCount ++ ;

	CMemory msg;
	ISearchSetStatusMsg(&msg);

	if (m_nISearchHistoryCount >= 256) {
		m_nISearchHistoryCount = 156;
		for(int i = 100 ; i<= 255 ; i++){
			m_bISearchFlagHistory[i-100] = m_bISearchFlagHistory[i];
			m_nISearchX1History[i-100] = m_nISearchX1History[i] ;
			m_nISearchY1History[i-100] = m_nISearchY1History[i] ;
			m_nISearchX2History[i-100] = m_nISearchX2History[i] ;
			m_nISearchY2History[i-100] = m_nISearchY2History[i] ;
		}
	}
	m_bISearchFlagHistory[m_nISearchHistoryCount] = bNext;

	if (m_pcEditDoc->m_cLayoutMgr.SearchWord(
		nLine, 									/* 検索開始行 */
		nIdx, 									/* 検索開始位置 */
		m_szCurSrchKey,							/* 検索条件 */
		m_nISearchDirection,					/* 0==前方検索 1==後方検索 */
		m_bCurSrchRegularExp,					/* 1==正規表現 */
		FALSE,									/* 1==英大文字小文字の区別 */
		FALSE,									/* 1==単語のみ検索 */
		&nLineFrom,								/* マッチレイアウト行from */
		&nColmFrom, 							/* マッチレイアウト位置from */
		&nLineTo, 								/* マッチレイアウト行to */
		&nColmTo, 								/* マッチレイアウト位置to */
		&m_CurRegexp	) == 0 )
	{
		/*検索結果がない*/
		msg.AppendSz(" (見つかりません)");
		SendStatusMessage(msg.GetPtr());
		
		if (bNext) 	m_bISearchWrap = true;
		if (IsTextSelected()){
			m_nISearchX1History[m_nISearchHistoryCount] = m_nSelectColmFrom;
			m_nISearchY1History[m_nISearchHistoryCount] = m_nSelectLineFrom;
			m_nISearchX2History[m_nISearchHistoryCount] = m_nSelectColmTo;
			m_nISearchY2History[m_nISearchHistoryCount] = m_nSelectLineTo;
		}else{
			m_nISearchX1History[m_nISearchHistoryCount] = m_nCaretPosX;
			m_nISearchY1History[m_nISearchHistoryCount] = m_nCaretPosY;
			m_nISearchX2History[m_nISearchHistoryCount] = m_nCaretPosX;
			m_nISearchY2History[m_nISearchHistoryCount] = m_nCaretPosY;
		}
	}else{
		//検索結果あり
		//キャレット移動
		MoveCursor( nColmFrom, nLineFrom , TRUE, _CARETMARGINRATE / 3 );
		//	2005.06.24 Moca
		SetSelectArea( nLineFrom, nColmFrom, nLineTo, nColmTo );

		m_bISearchWrap = false;
		m_nISearchX1History[m_nISearchHistoryCount] = nColmFrom;
		m_nISearchY1History[m_nISearchHistoryCount] = nLineFrom;
		m_nISearchX2History[m_nISearchHistoryCount] = nColmTo;
		m_nISearchY2History[m_nISearchHistoryCount] = nLineTo;

	}

	m_bCurSrchKeyMark = TRUE;

	Redraw();	
	SendStatusMessage(msg.GetPtr());
	return ;
}

//!	バックスペースを押されたときの処理
void CEditView::ISearchBack(void) {
	if(m_nISearchHistoryCount==0) return;
	
	if(m_nISearchHistoryCount==1){
		m_bCurSrchKeyMark = FALSE;
		m_bISearchFirst = true;
	}else if( m_bISearchFlagHistory[m_nISearchHistoryCount] == false){
		//検索文字をへらす
		long l = strlen(m_szCurSrchKey);
		if (l > 0 ){
			//最後の文字の一つ前
			char* p = CharPrev(m_szCurSrchKey,&m_szCurSrchKey[l]);
			*p = '\0';
			//m_szCurSrchKey[l-1] = '\0';

			if ( (p - m_szCurSrchKey) > 0 ) 
				ISearchWordMake();
			else
				m_bCurSrchKeyMark = FALSE;

		}else{
			MessageBeep(MB_ICONEXCLAMATION);
		}
	}
	m_nISearchHistoryCount --;
	int nLineFrom, nColmFrom, nColmTo, nLineTo;
	nColmFrom = m_nISearchX1History[m_nISearchHistoryCount];
	nLineFrom = m_nISearchY1History[m_nISearchHistoryCount];
	nColmTo   = m_nISearchX2History[m_nISearchHistoryCount];
	nLineTo   = m_nISearchY2History[m_nISearchHistoryCount];

	if(m_nISearchHistoryCount == 0){
		DisableSelectArea( TRUE );
		nColmTo = nColmFrom;
	}

	MoveCursor( nColmFrom , nLineFrom , TRUE, _CARETMARGINRATE / 3 );
	if(m_nISearchHistoryCount != 0){
		//	2005.06.24 Moca
		SetSelectArea( nLineFrom, nColmFrom, nLineTo, nColmTo );
	}

	Redraw();

	//ステータス表示
	CMemory msg;
	ISearchSetStatusMsg(&msg);
	SendStatusMessage(msg.GetPtr());
	
}

//!	入力文字から、検索文字を生成する。
void CEditView::ISearchWordMake(void) {
	int nFlag = 0x00;
	switch ( m_nISearchMode ) {
		case 1: // 通常インクリメンタルサーチ
			break;
		case 2: // 正規表現インクリメンタルサーチ
			if( !InitRegexp( m_hWnd, m_CurRegexp, true ) ){
				return ;
			}
			nFlag |= m_bCurSrchLoHiCase ? 0x01 : 0x00;
			/* 検索パターンのコンパイル */
			m_CurRegexp.Compile(m_szCurSrchKey , nFlag );
			break;
		case 3: // MIGEMOインクリメンタルサーチ
			if( !InitRegexp( m_hWnd, m_CurRegexp, true ) ){
				return ;
			}
			nFlag |= m_bCurSrchLoHiCase ? 0x01 : 0x00;

			//migemoで捜す
			m_pszMigemoWord = (char*)m_pcmigemo->migemo_query((unsigned char*)m_szCurSrchKey);
			
			/* 検索パターンのコンパイル */
			m_CurRegexp.Compile(m_pszMigemoWord , nFlag );

			m_pcmigemo->migemo_release((unsigned char*)m_pszMigemoWord);
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
void CEditView::ISearchSetStatusMsg(CMemory* msg) const
{

	switch ( m_nISearchMode){
		case 1 :
			msg->SetDataSz("I-Search" );
			break;
		case 2 :
			msg->SetDataSz("[RegExp] I-Search" );
			break;
		case 3 :
			msg->SetDataSz("[Migemo] I-Search" );
			break;
		default:
			msg->SetDataSz("");
			return;
	}
	if (m_nISearchDirection == 0){
		msg->AppendSz(" Backward: ");
	}else{
		msg->AppendSz(": ");
	}
	if(m_nISearchHistoryCount > 0)
		msg->AppendSz(m_szCurSrchKey);
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
		return m_nISearchMode == 1 && m_nISearchDirection == 1;
	case F_ISEARCH_PREV:
		return m_nISearchMode == 1 && m_nISearchDirection == 0;
	case F_ISEARCH_REGEXP_NEXT:
		return m_nISearchMode == 2 && m_nISearchDirection == 1;
	case F_ISEARCH_REGEXP_PREV:
		return m_nISearchMode == 2 && m_nISearchDirection == 0;
	case F_ISEARCH_MIGEMO_NEXT:
		return m_nISearchMode == 3 && m_nISearchDirection == 1;
	case F_ISEARCH_MIGEMO_PREV:
		return m_nISearchMode == 3 && m_nISearchDirection == 0;
	}
	return false;
}
/*[EOF]*/
