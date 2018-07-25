/*!	@file
	@brief キーワード補完

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2004, genta, Moca, novice
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CHokanMgr.h"
#include "env/CShareData.h"
#include "view/CEditView.h"
#include "plugin/CJackManager.h"
#include "plugin/CComplementIfObj.h"
#include "util/input.h"
#include "util/os.h"
#include "util/other_util.h"
#include "sakura_rc.h"

WNDPROC			gm_wpHokanListProc;


LRESULT APIENTRY HokanList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// Modified by KEITA for WIN64 2003.9.6
	CDialog* pCDialog = ( CDialog* )::GetWindowLongPtr( ::GetParent( hwnd ), DWLP_USER );
	CHokanMgr* pCHokanMgr = (CHokanMgr*)::GetWindowLongPtr( ::GetParent( hwnd ), DWLP_USER );

	switch( uMsg ){
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
			// アクティブ化を防止するために自前でリスト選択処理を実施する
			LRESULT lResult = ::SendMessageAny( hwnd, LB_ITEMFROMPOINT, 0, lParam );
			if( HIWORD(lResult) == 0 ){	// クライアントエリア内
				if( uMsg == WM_LBUTTONDOWN ){
					List_SetCurSel( hwnd, LOWORD(lResult) );
					pCHokanMgr->OnLbnSelChange( hwnd, IDC_LIST_WORDS );
				}
				else if( uMsg == WM_LBUTTONDBLCLK ){
					pCHokanMgr->DoHokan(0);
				}
			}
		}
		return 0;	// 本来のウィンドウプロシージャは呼ばない（アクティブ化しない）
	}
	return CallWindowProc( gm_wpHokanListProc, hwnd, uMsg, wParam, lParam);
}



CHokanMgr::CHokanMgr()
{
	m_cmemCurWord.SetString(L"");

	m_nCurKouhoIdx = -1;
	m_bTimerFlag = TRUE;
}

CHokanMgr::~CHokanMgr()
{
}

/* モードレスダイアログの表示 */
HWND CHokanMgr::DoModeless( HINSTANCE hInstance , HWND hwndParent, LPARAM lParam )
{
	HWND hwndWork;
	hwndWork = CDialog::DoModeless( hInstance, hwndParent, IDD_HOKAN, lParam, SW_HIDE );
	::SetFocus( ((CEditView*)m_lParam)->GetHwnd() );	//エディタにフォーカスを戻す
	OnSize( 0, 0 );
	/* リストをフック */
	// Modified by KEITA for WIN64 2003.9.6
	::gm_wpHokanListProc = (WNDPROC) ::SetWindowLongPtr( ::GetDlgItem( GetHwnd(), IDC_LIST_WORDS ), GWLP_WNDPROC, (LONG_PTR)HokanList_SubclassProc  );

	return hwndWork;
}

/* モードレス時：対象となるビューの変更 */
void CHokanMgr::ChangeView( LPARAM pcEditView )
{
	m_lParam = pcEditView;
	return;
}

void CHokanMgr::Hide( void )
{

	::ShowWindow( GetHwnd(), SW_HIDE );
	m_nCurKouhoIdx = -1;
	/* 入力フォーカスを受け取ったときの処理 */
	CEditView* pcEditView = reinterpret_cast<CEditView*>(m_lParam);
	pcEditView->OnSetFocus();
	return;

}

/*!	初期化
	pcmemHokanWord == NULLのとき、補完候補がひとつだったら、補完ウィンドウを表示しないで終了します。
	Search()呼び出し元で確定処理を進めてください。

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
int CHokanMgr::Search(
	POINT*			ppoWin,
	int				nWinHeight,
	int				nColumnWidth,
	const wchar_t*	pszCurWord,
	const TCHAR*	pszHokanFile,
	bool			bHokanLoHiCase,	// 入力補完機能：英大文字小文字を同一視する 2001/06/19 asa-o
	bool			bHokanByFile,	// 編集中データから候補を探す 2003.06.23 Moca
	int				nHokanType,
	bool			bHokanByKeyword,
	CNativeW*		pcmemHokanWord	// 2001/06/19 asa-o
)
{
	CEditView* pcEditView = reinterpret_cast<CEditView*>(m_lParam);

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData();

	/*
	||  補完キーワードの検索
	||
	||  ・見つかった候補をすべて返す(改行で区切って返す)
	||  ・指定された候補の最大数を超えると処理を中断する
	||  ・見つかった数を返す
	||
	*/
	m_vKouho.clear();
	CDicMgr::HokanSearch(
		pszCurWord,
		bHokanLoHiCase,								// 引数からに変更	2001/06/19 asa-o
		m_vKouho,
		0, //Max候補数
		pszHokanFile
	);

	// 2003.05.16 Moca 追加 編集中データ内から候補を探す
	if( bHokanByFile ){
		pcEditView->HokanSearchByFile(
			pszCurWord,
			bHokanLoHiCase,
			m_vKouho,
			1024 // 編集中データからなので数を制限しておく
		);
	}
	// 2012.10.13 Moca 強調キーワードから候補を探す
	if( bHokanByKeyword ){
		HokanSearchByKeyword(
			pszCurWord,
			bHokanLoHiCase,
			m_vKouho
		);
	}

	{
		int nOption = (
			  (bHokanLoHiCase ? 0x01 : 0)
			  | (bHokanByFile ? 0x02 : 0)
			);
		
		CPlug::Array plugs;
		CPlug::Array plugType;
		CJackManager::getInstance()->GetUsablePlug( PP_COMPLEMENTGLOBAL, 0, &plugs );
		if( nHokanType != 0 ){
			CJackManager::getInstance()->GetUsablePlug( PP_COMPLEMENT, nHokanType, &plugType );
			if( 0 < plugType.size() ){
				plugs.push_back( plugType[0] );
			}
		}

		for( auto it = plugs.begin(); it != plugs.end(); ++it ){
			//インタフェースオブジェクト準備
			CWSHIfObj::List params;
			std::wstring curWord = pszCurWord;
			CComplementIfObj* objComp = new CComplementIfObj( curWord , this, nOption );
			objComp->AddRef();
			params.push_back( objComp );
			//プラグイン呼び出し
			(*it)->Invoke( pcEditView, params );

			objComp->Release();
		}
	}

	if( 0 == m_vKouho.size() ){
		m_nCurKouhoIdx = -1;
		return 0;
	}

//	2001/06/19 asa-o 候補が１つの場合補完ウィンドウは表示しない(逐次補完の場合は除く)
	if( 1 == m_vKouho.size() ){
		if(pcmemHokanWord != NULL){
			m_nCurKouhoIdx = -1;
			pcmemHokanWord->SetString( m_vKouho[0].c_str() );
			return 1;
		}
	}



//	m_hFont = hFont;
	m_poWin.x = ppoWin->x;
	m_poWin.y = ppoWin->y;
	m_nWinHeight = nWinHeight;
	m_nColumnWidth = nColumnWidth;
//	m_cmemCurWord.SetData( pszCurWord, lstrlen( pszCurWord ) );
	m_cmemCurWord.SetString( pszCurWord );


	m_nCurKouhoIdx = 0;
//	SetCurKouhoStr();



//	::ShowWindow( GetHwnd(), SW_SHOWNA );







	HWND hwndList;
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_WORDS );
	List_ResetContent( hwndList );
	{
		size_t kouhoNum = m_vKouho.size();
		for( size_t i = 0; i < kouhoNum; ++i ){
			::List_AddString( hwndList, m_vKouho[i].c_str() );
		}
	}
	List_SetCurSel( hwndList, 0 );


//@@	::EnableWindow( ::GetParent( ::GetParent( m_hwndParent ) ), FALSE );


	int nX;
	int nY;
	int nCX;
	int nCY;
	RECT	rcDesktop;
	//	May 01, 2004 genta マルチモニタ対応
	::GetMonitorWorkRect( GetHwnd(), &rcDesktop );

	nX = m_poWin.x - m_nColumnWidth;
	nY = m_poWin.y + m_nWinHeight + 4;
	nCX = m_nWidth;
	nCY = m_nHeight;

	/* 下に入るなら */
	if( nY + nCY < rcDesktop.bottom ){
		/* 何もしない */
	}else
	/* 上に入るなら */
	if( rcDesktop.top < m_poWin.y - m_nHeight - 4 ){
		/* 上に出す */
		nY = m_poWin.y - m_nHeight - 4;
	}else
	/* 上に出すか下に出すか(広いほうに出す) */
	if(	rcDesktop.bottom - nY > m_poWin.y ){
		/* 下に出す */
//		m_nHeight = rcDesktop.bottom - nY;
		nCY = rcDesktop.bottom - nY;
	}else{
		/* 上に出す */
		nY = rcDesktop.top;
		nCY = m_poWin.y - 4 - rcDesktop.top;
	}

//	2001/06/19 Start by asa-o: 表示位置補正

	// 右に入る
	if(nX + nCX < rcDesktop.right ){
		// そのまま
	}else
	// 左に入る
	if(rcDesktop.left < nX - nCX + 8){
		// 左に表示
		nX -= nCX - 8;
	}else{
		// サイズを調整して右に表示
		nCX = t_max((int)(rcDesktop.right - nX) , 100);	// 最低サイズを100くらいに
	}

//	2001/06/19 End

//	2001/06/18 Start by asa-o: 補正後の位置・サイズを保存
	m_poWin.x = nX;
	m_poWin.y = nY;
	m_nHeight = nCY;
	m_nWidth = nCX;
//	2001/06/18 End

	/* はみ出すなら小さくする */
//	if( rcDesktop.bottom < nY + nCY ){
//		/* 下にはみ出す */
//		if( m_poWin.y - 4 - nCY < 0 ){
//			/* 上にはみ出す */
//			/* →高さだけ調節 */
//			nCY = rcDesktop.bottom - nY - 4;
//		}else{
//
//		}
//
//	}
	::MoveWindow( GetHwnd(), nX, nY, nCX, nCY, TRUE );
	::ShowWindow( GetHwnd(), SW_SHOWNA );


//	2001/06/18 asa-o:
	ShowTip();	// 補完ウィンドウで選択中の単語にキーワードヘルプを表示

//	2003.06.25 Moca 他のメソッドで使っていないので、とりあえず削除しておく
	int kouhoNum = m_vKouho.size();
	m_vKouho.clear();
	return kouhoNum;
}

void CHokanMgr::HokanSearchByKeyword(
	const wchar_t*	pszCurWord,
	bool 			bHokanLoHiCase,
	vector_ex<std::wstring>& 	vKouho
){
	const CEditView* pcEditView = reinterpret_cast<const CEditView*>(m_lParam);
	const STypeConfig& type = pcEditView->GetDocument()->m_cDocType.GetDocumentAttribute();
	CKeyWordSetMgr& keywordMgr = m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	const int nKeyLen = wcslen(pszCurWord);
	for( int n = 0; n < MAX_KEYWORDSET_PER_TYPE; n++ ){
		int kwdset = type.m_nKeyWordSetIdx[n];
		if( kwdset == -1 ){
			continue;
		}
		const int keyCount = keywordMgr.GetKeyWordNum(kwdset);
		for(int i = 0; i < keyCount; i++){
			const wchar_t* word = keywordMgr.GetKeyWord(kwdset,i);
			int nRet;
			if( bHokanLoHiCase ){
				nRet = auto_memicmp(pszCurWord, word, nKeyLen );
			}else{
				nRet = auto_memcmp(pszCurWord, word, nKeyLen );
			}
			if( nRet != 0 ){
				continue;
			}
			std::wstring strWord = std::wstring(word);
			AddKouhoUnique(vKouho, strWord);
		}
	}
}


/*!
	標準以外のメッセージを捕捉する
*/
INT_PTR CHokanMgr::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	// 念のため IME 関連のメッセージが来るようならビューに処理させる
	// 何かの環境依存（常駐ソフト？）によるものかもしれないが、
	// フォーカスが無くても IME 関連メッセージがこっちに来るケースがあったので、その対策
	if(wMsg >= WM_IME_STARTCOMPOSITION && wMsg <= WM_IME_KEYLAST || wMsg >= WM_IME_SETCONTEXT && wMsg <= WM_IME_KEYUP){
		CEditView* pcEditView = (CEditView*)m_lParam;
		pcEditView->DispatchEvent( pcEditView->GetHwnd(), wMsg, wParam, lParam );
		return TRUE;
	}

	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_MOUSEACTIVATE:
		// アクティブにしないでおく
		::SetWindowLongPtr( GetHwnd(), DWLP_MSGRESULT, MA_NOACTIVATE );
		return TRUE;
	case WM_LBUTTONDOWN:
		return TRUE;	// クライアント領域はリストボックスで埋まっているのでここへは来ないはずだけど念のため
	case WM_NCLBUTTONDOWN:
		// ここでもアクティブ化防止の対策が必要
		// ::SetCapture() して自前でサイズ変更する
		{
			POINT ptStart;
			POINT pt;
			RECT rcStart;
			RECT rc;
			::GetCursorPos( &ptStart );
			::GetWindowRect( GetHwnd(), &rcStart );
			::SetCapture( GetHwnd() );
			while( ::GetCapture() == GetHwnd() )
			{
				MSG msg;
				if (!::GetMessage(&msg, NULL, 0, 0)){
					::PostQuitMessage( (int)msg.wParam );
					break;
				}

				switch (msg.message){
				case WM_MOUSEMOVE:
					rc = rcStart;
					::GetCursorPos( &pt );
					{
						switch( wParam ){
						case HTTOP:
							rc.top += pt.y - ptStart.y;
							break;
						case HTBOTTOM:
							rc.bottom += pt.y - ptStart.y;
							break;
						case HTLEFT:
							rc.left += pt.x - ptStart.x;
							break;
						case HTRIGHT:
							rc.right += pt.x - ptStart.x;
							break;
						case HTTOPLEFT:
							rc.top += pt.y - ptStart.y;
							rc.left += pt.x - ptStart.x;
							break;
						case HTTOPRIGHT:
							rc.top += pt.y - ptStart.y;
							rc.right += pt.x - ptStart.x;
							break;
						case HTBOTTOMLEFT:
							rc.bottom += pt.y - ptStart.y;
							rc.left += pt.x - ptStart.x;
							break;
						case HTBOTTOMRIGHT:
							rc.bottom += pt.y - ptStart.y;
							rc.right += pt.x - ptStart.x;
							break;
						}
						::MoveWindow( GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE );
					}
					break;
				case WM_LBUTTONUP:
				case WM_RBUTTONDOWN:
					::ReleaseCapture();
					break;
				case WM_KEYDOWN:
					if( msg.wParam == VK_ESCAPE ){
						// キャンセル
						::ReleaseCapture();
					}
					break;
				default:
					::DispatchMessage( &msg );
					break;
				}
			}
		}
		return TRUE;
	case WM_GETMINMAXINFO:
		// 最小サイズを制限する
		MINMAXINFO *pmmi;
		pmmi = (MINMAXINFO*)lParam;
		pmmi->ptMinTrackSize.x = ::GetSystemMetrics(SM_CXVSCROLL) * 4;
		pmmi->ptMinTrackSize.y = ::GetSystemMetrics(SM_CYHSCROLL) * 4;
		break;
	}
	return result;
}

BOOL CHokanMgr::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );
	/* 基底クラスメンバ */
//-	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );

}

BOOL CHokanMgr::OnDestroy( void )
{
	/* 基底クラスメンバ */
	CreateSizeBox();
	return CDialog::OnDestroy();


}


BOOL CHokanMgr::OnSize( WPARAM wParam, LPARAM lParam )
{
	/* 基底クラスメンバ */
	CDialog::OnSize( wParam, lParam );

	int	Controls[] = {
		IDC_LIST_WORDS
	};
	int		nControls = _countof( Controls );
	int		nWidth;
	int		nHeight;
	int		i;
	RECT	rc;
	HWND	hwndCtrl;
	POINT	po;
	RECT	rcDlg;


	::GetClientRect( GetHwnd(), &rcDlg );
	nWidth = rcDlg.right - rcDlg.left;  // width of client area
	nHeight = rcDlg.bottom - rcDlg.top; // height of client area

//	2001/06/18 Start by asa-o: サイズ変更後の位置を保存
	m_poWin.x = rcDlg.left - 4;
	m_poWin.y = rcDlg.top - 3;
	::ClientToScreen(GetHwnd(),&m_poWin);
//	2001/06/18 End

	for ( i = 0; i < nControls; ++i ){
		hwndCtrl = ::GetDlgItem( GetHwnd(), Controls[i] );
		::GetWindowRect( hwndCtrl, &rc );
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient( GetHwnd(), &po );
		rc.left = po.x;
		rc.top  = po.y;
		po.x = rc.right;
		po.y = rc.bottom;
		::ScreenToClient( GetHwnd(), &po );
		rc.right = po.x;
		rc.bottom  = po.y;
		if( Controls[i] == IDC_LIST_WORDS ){
			::SetWindowPos(
				hwndCtrl,
				NULL,
				rc.left,
				rc.top,
				nWidth - rc.left * 2,
				nHeight - rc.top * 2/* - 20*/,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
	}

//	2001/06/18 asa-o:
	ShowTip();	// 補完ウィンドウで選択中の単語にキーワードヘルプを表示

	return TRUE;

}


BOOL CHokanMgr::OnLbnSelChange( HWND hwndCtl, int wID )
{
//	2001/06/18 asa-o:
	ShowTip();	// 補完ウィンドウで選択中の単語にキーワードヘルプを表示
	return TRUE;
}



/* 補完実行 */
BOOL CHokanMgr::DoHokan( int nVKey )
{
	DEBUG_TRACE( _T("CHokanMgr::DoHokan( nVKey==%xh )\n"), nVKey );

	/* 補完候補決定キー */
	if( VK_RETURN	== nVKey && !m_pShareData->m_Common.m_sHelper.m_bHokanKey_RETURN )	return FALSE;/* VK_RETURN 補完決定キーが有効/無効 */
	if( VK_TAB		== nVKey && !m_pShareData->m_Common.m_sHelper.m_bHokanKey_TAB ) 		return FALSE;/* VK_TAB    補完決定キーが有効/無効 */
	if( VK_RIGHT	== nVKey && !m_pShareData->m_Common.m_sHelper.m_bHokanKey_RIGHT )		return FALSE;/* VK_RIGHT  補完決定キーが有効/無効 */

	HWND hwndList;
	int nItem;
	CEditView* pcEditView;
	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_WORDS );
	nItem = List_GetCurSel( hwndList );
	if( LB_ERR == nItem ){
		return FALSE;
	}
	int nLabelLen = List_GetTextLen( hwndList, nItem );
	auto_array_ptr<WCHAR> wszLabel( new WCHAR [nLabelLen + 1] );
	List_GetText( hwndList, nItem, &wszLabel[0] );

 	/* テキストを貼り付け */
	pcEditView = reinterpret_cast<CEditView*>(m_lParam);
	//	Apr. 28, 2000 genta
	pcEditView->GetCommander().HandleCommand( F_WordDeleteToStart, false, 0, 0, 0, 0 );
	pcEditView->GetCommander().HandleCommand( F_INSTEXT_W, true, (LPARAM)&wszLabel[0], wcslen(&wszLabel[0]), TRUE, 0 );

	// Until here
//	pcEditView->GetCommander().HandleCommand( F_INSTEXT_W, true, (LPARAM)(wszLabel + m_cmemCurWord.GetLength()), TRUE, 0, 0 );
	Hide();

	return TRUE;
}

/*
戻り値が -2 の場合は、アプリケーションは項目の選択を完了し、
リスト ボックスでそれ以上の動作が必要でないことを示します。

戻り値が -1 の場合は、リスト ボックスがキーストロークに応じて
デフォルトの動作を実行することを示します。

 戻り値が 0 以上の場合は、その値はリスト ボックスの項目の 0 を
基準としたインデックスを意味し、リスト ボックスがその項目での
キーストロークに応じてデフォルトの動作を実行することを示します。

*/
//	int CHokanMgr::OnVKeyToItem( WPARAM wParam, LPARAM lParam )
//	{
//		return KeyProc( wParam, lParam );
//	}

/*
戻り値が -2 の場合は、アプリケーションは項目の選択を完了し、
リスト ボックスでそれ以上の動作が必要でないことを示します。

戻り値が -1 の場合は、リスト ボックスがキーストロークに応じて
デフォルトの動作を実行することを示します。

 戻り値が 0 以上の場合は、その値はリスト ボックスの項目の 0 を
基準としたインデックスを意味し、リスト ボックスがその項目での
キーストロークに応じてデフォルトの動作を実行することを示します。

*/
//	int CHokanMgr::OnCharToItem( WPARAM wParam, LPARAM lParam )
//	{
//		WORD vkey;
//		WORD nCaretPos;
//		LPARAM hwndLB;
//		vkey = LOWORD(wParam);		// virtual-key code
//		nCaretPos = HIWORD(wParam);	// caret position
//		hwndLB = lParam;			// handle to list box
//	//	switch( vkey ){
//	//	}
//
//		MYTRACE( _T("CHokanMgr::OnCharToItem vkey=%xh\n"), vkey );
//		return -1;
//	}

int CHokanMgr::KeyProc( WPARAM wParam, LPARAM lParam )
{
	WORD vkey;
	vkey = LOWORD(wParam);		// virtual-key code
//	MYTRACE( _T("CHokanMgr::OnVKeyToItem vkey=%xh\n"), vkey );
	switch( vkey ){
	case VK_HOME:
	case VK_END:
	case VK_UP:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
		/* リストボックスのデフォルトの動作をさせる */
		::CallWindowProc( (WNDPROC)gm_wpHokanListProc, ::GetDlgItem( GetHwnd(), IDC_LIST_WORDS ), WM_KEYDOWN, wParam, lParam );
		return -1;
	case VK_RETURN:
	case VK_TAB:
	case VK_RIGHT:
#if 0
	case VK_SPACE:
#endif
		/* 補完実行 */
		if( DoHokan( vkey ) ){
			return -1;
		}else{
			return -2;
		}
	case VK_ESCAPE:
	case VK_LEFT:
		return -2;
	}
	return -2;
}

//	2001/06/18 Start by asa-o: 補完ウィンドウで選択中の単語にキーワードヘルプを表示
void CHokanMgr::ShowTip()
{
	INT			nItem,
				nTopItem,
				nItemHeight;
	POINT		point;
	CEditView*	pcEditView;
	HWND		hwndCtrl;
	RECT		rcHokanWin;

	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_LIST_WORDS );

	nItem = List_GetCurSel( hwndCtrl );
	if( LB_ERR == nItem )	return ;

	int nLabelLen = List_GetTextLen( hwndCtrl, nItem );
	auto_array_ptr<WCHAR> szLabel( new WCHAR [nLabelLen + 1] );
	List_GetText( hwndCtrl, nItem, &szLabel[0] );	// 選択中の単語を取得

	pcEditView = reinterpret_cast<CEditView*>(m_lParam);

	// すでに辞書Tipが表示されていたら
	if( pcEditView->m_dwTipTimer == 0 )
	{
		// 辞書Tipを消す
		pcEditView -> m_cTipWnd.Hide();
		pcEditView -> m_dwTipTimer = ::GetTickCount();
	}

	// 表示する位置を決定
	nTopItem = List_GetTopIndex( hwndCtrl );
	nItemHeight = List_GetItemHeight( hwndCtrl, 0 );
	point.x = m_poWin.x + m_nWidth;
	point.y = m_poWin.y + 4 + (nItem - nTopItem) * nItemHeight;
	// 2001/06/19 asa-o 選択中の単語が補完ウィンドウに表示されているなら辞書Tipを表示
	if( point.y > m_poWin.y && point.y < m_poWin.y + m_nHeight )
	{
		::SetRect( &rcHokanWin , m_poWin.x, m_poWin.y, m_poWin.x + m_nWidth, m_poWin.y + m_nHeight );
		if( !pcEditView -> ShowKeywordHelp( point, &szLabel[0], &rcHokanWin ) )
			pcEditView -> m_dwTipTimer = ::GetTickCount();	// 表示するべきキーワードヘルプが無い
	}
}
//	2001/06/18 End

bool CHokanMgr::AddKouhoUnique(vector_ex<std::wstring>& kouhoList, const std::wstring& strWord)
{
	return kouhoList.push_back_unique(strWord);
}

//@@@ 2002.01.18 add start
const DWORD p_helpids[] = {
	0, 0
};

LPVOID CHokanMgr::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


