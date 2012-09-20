#include "StdAfx.h"
#include <WindowsX.h>
#include "CMainToolBar.h"
#include "window/CEditWnd.h"
#include "util/os.h"
#include "CEditApp.h"
#include "util/tchar_receive.h"
#include "util/window.h"

CMainToolBar::CMainToolBar(CEditWnd* pOwner)
: m_pOwner(pOwner)
, m_fontSearchBox(NULL)
, m_hwndReBar(NULL)
, m_hwndToolBar(NULL)
, m_hwndSearchBox(NULL)
{
}

/*! 検索ボックスでの処理 */
void CMainToolBar::ProcSearchBox( MSG *msg )
{
	if( msg->message == WM_KEYDOWN /* && ::GetParent( msg->hwnd ) == m_hwndSearchBox */ )
	{
		if( msg->wParam == VK_RETURN )  //リターンキー
		{
			//検索キーワードを取得
			wchar_t	szText[_MAX_PATH];
			wmemset( szText, 0, _countof(szText) );
			::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
			if( szText[0] )	//キー文字列がある
			{
				//検索キーを登録
				CSearchKeywordManager().AddToSearchKeyArr( szText );

				//検索ボックスを更新	// 2010/6/6 Uchi
				AcceptSharedSearchKey();

				//::SetFocus( m_hWnd );	//先にフォーカスを移動しておかないとキャレットが消える
				m_pOwner->GetActiveView().SetFocus();

				// 検索開始時のカーソル位置登録条件を変更 02/07/28 ai start
				m_pOwner->GetActiveView().m_ptSrchStartPos_PHY = m_pOwner->GetActiveView().GetCaret().GetCaretLogicPos();
				// 02/07/28 ai end

				//次を検索
				m_pOwner->OnCommand( (WORD)0 /*メニュー*/, (WORD)F_SEARCH_NEXT, (HWND)0 );
			}
		}
		else if( msg->wParam == VK_TAB )	//タブキー
		{
			//フォーカスを移動
			//	2004.10.27 MIK IME表示位置のずれ修正
			::SetFocus( m_pOwner->GetHwnd()  );
		}
	}
}

/*! サブクラス化したツールバーのウィンドウプロシージャ
	@author ryoji
	@date 2006.09.06 ryoji
*/
static WNDPROC g_pOldToolBarWndProc;	// ツールバーの本来のウィンドウプロシージャ

static LRESULT CALLBACK ToolBarWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	// WinXP Visual Style のときにツールバー上でのマウス左右ボタン同時押しで無応答になる
	//（マウスをキャプチャーしたまま放さない） 問題を回避するために右ボタンを無視する
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		return 0L;				// 右ボタンの UP/DOWN は本来のウィンドウプロシージャに渡さない

	case WM_DESTROY:
		// サブクラス化解除
		::SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR)g_pOldToolBarWndProc );
		break;
	}
	return ::CallWindowProc( g_pOldToolBarWndProc, hWnd, msg, wParam, lParam );
}


/* ツールバー作成
	@date @@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	@date 2005.08.29 aroka ツールバーの折り返し
	@date 2006.06.17 ryoji ビジュアルスタイルが有効の場合はツールバーを Rebar に入れてサイズ変更時のちらつきを無くす
*/
void CMainToolBar::CreateToolBar( void )
{
	if( m_hwndToolBar )return;

	REBARINFO		rbi;
	REBARBANDINFO	rbBand;
	int				nFlag;
	TBBUTTON		tbb;
	int				i;
	int				nIdx;
	UINT			uToolType;
	nFlag = 0;

	// 2006.06.17 ryoji
	// Rebar ウィンドウの作成
	if( IsVisualStyle() ){	// ビジュアルスタイル有効
		m_hwndReBar = ::CreateWindowEx(
			WS_EX_TOOLWINDOW,
			REBARCLASSNAME, //レバーコントロール
			NULL,
			WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |	// 2007.03.08 ryoji WS_VISIBLE 除去
			RBS_BANDBORDERS | CCS_NODIVIDER,
			0, 0, 0, 0,
			m_pOwner->GetHwnd(),
			NULL,
			CEditApp::getInstance()->GetAppInstance(),
			NULL
		);

		if( NULL == m_hwndReBar ){
			TopWarningMessage( m_pOwner->GetHwnd(), _T("Rebar の作成に失敗しました。") );
			return;
		}

		if( GetDllShareData().m_Common.m_sToolBar.m_bToolBarIsFlat ){	/* フラットツールバーにする／しない */
			PreventVisualStyle( m_hwndReBar );	// ビジュアルスタイル非適用のフラットな Rebar にする
		}

		::ZeroMemory(&rbi, sizeof(rbi));
		rbi.cbSize = sizeof(rbi);
		Rebar_SetbarInfo(m_hwndReBar, &rbi);

		nFlag = CCS_NORESIZE | CCS_NODIVIDER | CCS_NOPARENTALIGN | TBSTYLE_FLAT;	// ツールバーへの追加スタイル
	}

	/* ツールバーウィンドウの作成 */
	m_hwndToolBar = ::CreateWindowEx(
		0,
		TOOLBARCLASSNAME,
		NULL,
		WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPCHILDREN | /*WS_BORDER | */	// 2006.06.17 ryoji WS_CLIPCHILDREN 追加	// 2007.03.08 ryoji WS_VISIBLE 除去
/*		WS_EX_WINDOWEDGE| */
		TBSTYLE_TOOLTIPS |
//		TBSTYLE_WRAPABLE |
//		TBSTYLE_ALTDRAG |
//		CCS_ADJUSTABLE |
		nFlag,
		0, 0,
		0, 0,
		m_pOwner->GetHwnd(),
		(HMENU)ID_TOOLBAR,
		CEditApp::getInstance()->GetAppInstance(),
		NULL
	);
	if( NULL == m_hwndToolBar ){
		if( GetDllShareData().m_Common.m_sToolBar.m_bToolBarIsFlat ){	/* フラットツールバーにする／しない */
			GetDllShareData().m_Common.m_sToolBar.m_bToolBarIsFlat = FALSE;
		}
		TopWarningMessage( m_pOwner->GetHwnd(), _T("ツールバーの作成に失敗しました。") );
		DestroyToolBar();	// 2006.06.17 ryoji
	}
	else{
		// 2006.09.06 ryoji ツールバーをサブクラス化する
		g_pOldToolBarWndProc = (WNDPROC)::SetWindowLongPtr(
			m_hwndToolBar,
			GWLP_WNDPROC,
			(LONG_PTR)ToolBarWndProc
		);

		Toolbar_SetButtonSize( m_hwndToolBar, DpiScaleX(22), DpiScaleY(22) );	// 2009.10.01 ryoji 高DPI対応スケーリング
		Toolbar_ButtonStructSize( m_hwndToolBar, sizeof(TBBUTTON) );
		//	Oct. 12, 2000 genta
		//	既に用意されているImage Listをアイコンとして登録
		CEditApp::getInstance()->GetIcons().SetToolBarImages( m_hwndToolBar );
		/* ツールバーにボタンを追加 */
		int count = 0;	//@@@ 2002.06.15 MIK
		int nToolBarButtonNum = 0;// 2005/8/29 aroka
		//	From Here 2005.08.29 aroka
		// はじめにツールバー構造体の配列を作っておく
		TBBUTTON *pTbbArr = new TBBUTTON[GetDllShareData().m_Common.m_sToolBar.m_nToolBarButtonNum];
		for( i = 0; i < GetDllShareData().m_Common.m_sToolBar.m_nToolBarButtonNum; ++i ){
			nIdx = GetDllShareData().m_Common.m_sToolBar.m_nToolBarButtonIdxArr[i];
			pTbbArr[nToolBarButtonNum] = m_pOwner->GetMenuDrawer().getButton(nIdx);
			// セパレータが続くときはひとつにまとめる
			// 折り返しボタンもTBSTYLE_SEP属性を持っているので
			// 折り返しの前のセパレータは全て削除される．
			if( (pTbbArr[nToolBarButtonNum].fsStyle & TBSTYLE_SEP) && (nToolBarButtonNum!=0)){
				if( (pTbbArr[nToolBarButtonNum-1].fsStyle & TBSTYLE_SEP) ){
					pTbbArr[nToolBarButtonNum-1] = pTbbArr[nToolBarButtonNum];
					nToolBarButtonNum--;
				}
			}
			// 仮想折返しボタンがきたら直前のボタンに折返し属性を付ける
			if( pTbbArr[nToolBarButtonNum].fsState & TBSTATE_WRAP ){
				if( nToolBarButtonNum!=0 ){
					pTbbArr[nToolBarButtonNum-1].fsState |= TBSTATE_WRAP;
				}
				continue;
			}
			nToolBarButtonNum++;
		}
		//	To Here 2005.08.29 aroka

		for( i = 0; i < nToolBarButtonNum; ++i ){
			tbb = pTbbArr[i];

			//@@@ 2002.06.15 MIK start
			switch( tbb.fsStyle )
			{
			case TBSTYLE_DROPDOWN:	//ドロップダウン
				//拡張スタイルに設定
				Toolbar_SetExtendedStyle( m_hwndToolBar, TBSTYLE_EX_DRAWDDARROWS );
				Toolbar_AddButtons( m_hwndToolBar, 1, &tbb );
				count++;
				break;

			case TBSTYLE_COMBOBOX:	//コンボボックス
				{
					RECT			rc;
					TBBUTTONINFO	tbi;
					TBBUTTON		my_tbb;
					LOGFONT		lf;

					switch( tbb.idCommand )
					{
					case F_SEARCH_BOX:
						if( m_hwndSearchBox )
						{
							break;
						}
						
						//セパレータ作る
						memset_raw( &my_tbb, 0, sizeof(my_tbb) );
						my_tbb.fsStyle   = TBSTYLE_BUTTON;  //ボタンにしないと描画が乱れる 2005/8/29 aroka
						my_tbb.idCommand = tbb.idCommand;	//同じIDにしておく
						if( tbb.fsState & TBSTATE_WRAP ){   //折り返し 2005/8/29 aroka
							my_tbb.fsState |=  TBSTATE_WRAP;
						}
						Toolbar_AddButtons( m_hwndToolBar, 1, &my_tbb );
						count++;

						//サイズを設定する
						tbi.cbSize = sizeof(tbi);
						tbi.dwMask = TBIF_SIZE;
						tbi.cx     = DpiScaleX(160);	//ボックスの幅	// 2009.10.01 ryoji 高DPI対応スケーリング
						Toolbar_SetButtonInfo( m_hwndToolBar, tbb.idCommand, &tbi );

						//位置とサイズを取得する
						rc.right = rc.left = rc.top = rc.bottom = 0;
						Toolbar_GetItemRect( m_hwndToolBar, count-1, &rc );

						//コンボボックスを作る
						//	Mar. 8, 2003 genta 検索ボックスを1ドット下にずらした
						m_hwndSearchBox = CreateWindow( _T("COMBOBOX"), _T("Combo"),
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN
								/*| CBS_SORT*/ | CBS_AUTOHSCROLL /*| CBS_DISABLENOSCROLL*/,
								rc.left, rc.top + 1, rc.right - rc.left, (rc.bottom - rc.top) * 10,
								m_hwndToolBar, (HMENU)(INT_PTR)tbb.idCommand, CEditApp::getInstance()->GetAppInstance(), NULL );
						if( m_hwndSearchBox )
						{
							m_pOwner->SetCurrentFocus(0);

							memset_raw( &lf, 0, sizeof(lf) );
							lf.lfHeight			= DpiPointsToPixels(-9); // Jan. 14, 2003 genta ダイアログにあわせてちょっと小さく	// 2009.10.01 ryoji 高DPI対応（ポイント数から算出）
							lf.lfWidth			= 0;
							lf.lfEscapement		= 0;
							lf.lfOrientation	= 0;
							lf.lfWeight			= FW_NORMAL;
							lf.lfItalic			= FALSE;
							lf.lfUnderline		= FALSE;
							lf.lfStrikeOut		= FALSE;
							lf.lfCharSet		= SHIFTJIS_CHARSET;
							lf.lfOutPrecision	= OUT_DEFAULT_PRECIS;
							lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
							lf.lfQuality		= DEFAULT_QUALITY;
							lf.lfPitchAndFamily	= FF_MODERN | DEFAULT_PITCH;
							_tcscpy( lf.lfFaceName, _T("ＭＳ Ｐゴシック") );
							m_fontSearchBox = ::CreateFontIndirect( &lf );
							if( m_fontSearchBox )
							{
								::SendMessage( m_hwndSearchBox, WM_SETFONT, (WPARAM)m_fontSearchBox, MAKELONG (TRUE, 0) );
							}

							//入力長制限
							ComboBox_LimitText( m_hwndSearchBox, (WPARAM)_MAX_PATH - 1 );

							//検索ボックスを更新	// 関数化 2010/6/6 Uchi
							AcceptSharedSearchKey();
						}
						break;

					default:
						break;
					}
				}
				break;

			case TBSTYLE_BUTTON:	//ボタン
			case TBSTYLE_SEP:		//セパレータ
			default:
				Toolbar_AddButtons( m_hwndToolBar, 1, &tbb );
				count++;
				break;
			}
			//@@@ 2002.06.15 MIK end
		}
		if( GetDllShareData().m_Common.m_sToolBar.m_bToolBarIsFlat ){	/* フラットツールバーにする／しない */
			uToolType = (UINT)::GetWindowLong(m_hwndToolBar, GWL_STYLE);
			uToolType |= (TBSTYLE_FLAT);
			::SetWindowLong(m_hwndToolBar, GWL_STYLE, uToolType);
			::InvalidateRect(m_hwndToolBar, NULL, TRUE);
		}
		delete []pTbbArr;// 2005/8/29 aroka
	}

	// 2006.06.17 ryoji
	// ツールバーを Rebar に入れる
	if( m_hwndReBar && m_hwndToolBar ){
		// ツールバーの高さを取得する
		DWORD dwBtnSize = Toolbar_GetButtonSize( m_hwndToolBar );
		DWORD dwRows = Toolbar_GetRows( m_hwndToolBar );

		// バンド情報を設定する
		// 以前のプラットフォームに _WIN32_WINNT >= 0x0600 で定義される構造体のフルサイズを渡すと失敗する	// 2007.12.21 ryoji
		rbBand.cbSize = CCSIZEOF_STRUCT( REBARBANDINFO, wID );
		rbBand.fMask  = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
		rbBand.fStyle = RBBS_CHILDEDGE;
		rbBand.hwndChild  = m_hwndToolBar;	// ツールバー
		rbBand.cxMinChild = 0;
		rbBand.cyMinChild = HIWORD(dwBtnSize) * dwRows;
		rbBand.cx         = 250;

		// バンドを追加する
		Rebar_InsertBand( m_hwndReBar, -1, &rbBand );
		::ShowWindow( m_hwndToolBar, SW_SHOW );
	}

	return;
}

void CMainToolBar::DestroyToolBar( void )
{
	if( m_hwndToolBar )
	{
		if( m_hwndSearchBox )
		{
			if( m_fontSearchBox )
			{
				::DeleteObject( m_fontSearchBox );
				m_fontSearchBox = NULL;
			}

			::DestroyWindow( m_hwndSearchBox );
			m_hwndSearchBox = NULL;

			m_pOwner->SetCurrentFocus(0);
		}

		::DestroyWindow( m_hwndToolBar );
		m_hwndToolBar = NULL;

		//if( m_cTabWnd.m_pOwner->GetHwnd() ) ::UpdateWindow( m_cTabWnd.m_pOwner->GetHwnd() );
		//if( m_CFuncKeyWnd.m_pOwner->GetHwnd() ) ::UpdateWindow( m_CFuncKeyWnd.m_pOwner->GetHwnd() );
	}

	// 2006.06.17 ryoji Rebar を破棄する
	if( m_hwndReBar )
	{
		::DestroyWindow( m_hwndReBar );
		m_hwndReBar = NULL;
	}

	return;
}

//! メッセージ処理。なんか処理したなら true を返す。
bool CMainToolBar::EatMessage(MSG* msg)
{
	if( m_hwndSearchBox && ::IsDialogMessage( m_hwndSearchBox, msg ) ){	//検索コンボボックス
		ProcSearchBox( msg );
		return true;
	}
	return false;
}


/*!	@brief ToolBarのOwnerDraw

	@param pnmh [in] Owner Draw情報

	@note Common Control V4.71以降はNMTBCUSTOMDRAWを送ってくるが，
	Common Control V4.70はLPNMCUSTOMDRAWしか送ってこないので
	安全のため小さい方に合わせて処理を行う．
	
	@author genta
	@date 2003.07.21 作成

*/
LPARAM CMainToolBar::ToolBarOwnerDraw( LPNMCUSTOMDRAW pnmh )
{
	switch( pnmh->dwDrawStage ){
	case CDDS_PREPAINT:
		//	描画開始前
		//	アイテムを自前で描画する旨を通知する
		return CDRF_NOTIFYITEMDRAW;
	
	case CDDS_ITEMPREPAINT:
		//	面倒くさいので，枠はToolbarに描いてもらう
		//	アイコンが登録されていないので中身は何も描かれない
		// 2010.07.15 Moca 検索(ボックス)なら枠を描かない
		if( pnmh->dwItemSpec == F_SEARCH_BOX ){
			return CDRF_SKIPDEFAULT;
		}
		return CDRF_NOTIFYPOSTPAINT;
	
	case CDDS_ITEMPOSTPAINT:
		{
			//	描画
			// コマンド番号（pnmh->dwItemSpec）からアイコン番号を取得する	// 2007.11.02 ryoji
			int nIconId = Toolbar_GetBitmap( pnmh->hdr.hwndFrom, (WPARAM)pnmh->dwItemSpec );

			int offset = ((pnmh->rc.bottom - pnmh->rc.top) - CEditApp::getInstance()->GetIcons().cy()) / 2;		// アイテム矩形からの画像のオフセット	// 2007.03.25 ryoji
			int shift = pnmh->uItemState & ( CDIS_SELECTED | CDIS_CHECKED ) ? 1 : 0;	//	Aug. 30, 2003 genta ボタンを押されたらちょっと画像をずらす
			int color = pnmh->uItemState & CDIS_CHECKED ? COLOR_3DHILIGHT : COLOR_3DFACE;

			//	Sep. 6, 2003 genta 押下時は右だけでなく下にもずらす
			CEditApp::getInstance()->GetIcons().Draw( nIconId, pnmh->hdc, pnmh->rc.left + offset + shift, pnmh->rc.top + offset + shift,
				(pnmh->uItemState & CDIS_DISABLED ) ? ILD_MASK : ILD_NORMAL
			);
		}
		break;
	default:
		break;
	}
	return CDRF_DODEFAULT;
}


/*! ツールバー更新用タイマーの処理
	@date 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	@date 2003.08.29 wmlhq, ryoji nTimerCountの導入
	@date 2006.01.28 aroka OnTimerから分離
	@date 2007.04.03 ryoji パラメータ無しにした
	@date 2008.10.05 nasukoji ツールバー更新部分を外に出した
*/
void CMainToolBar::OnToolbarTimer( void )
{
	m_pOwner->IncrementTimerCount(10);

	UpdateToolbar();	// 2008.09.23 nasukoji	ツールバーの表示を更新する
}

/*!
	@brief ツールバーの表示を更新する
	
	@note 他から呼べるようにOnToolbarTimer()より切り出した
	
	@date 2008.10.05 nasukoji
*/
void CMainToolBar::UpdateToolbar( void )
{
	// 印刷プレビュー中なら、何もしない。
	if( m_pOwner->IsInPreviewMode() )return;
	
	// ツールバーの状態更新
	if( m_hwndToolBar )
	{
		for( int i = 0; i < GetDllShareData().m_Common.m_sToolBar.m_nToolBarButtonNum; ++i )
		{
			TBBUTTON tbb = m_pOwner->GetMenuDrawer().getButton(
				GetDllShareData().m_Common.m_sToolBar.m_nToolBarButtonIdxArr[i]
			);

			// 機能が利用可能か調べる
			Toolbar_EnableButton(
				m_hwndToolBar,
				tbb.idCommand,
				IsFuncEnable( &m_pOwner->GetDocument(), &GetDllShareData(), (EFunctionCode)tbb.idCommand )
			);

			// 機能がチェック状態か調べる
			Toolbar_CheckButton(
				m_hwndToolBar,
				tbb.idCommand,
				IsFuncChecked( &m_pOwner->GetDocument(), &GetDllShareData(), (EFunctionCode)tbb.idCommand )
			);
		}
	}
}

//検索ボックスを更新
void CMainToolBar::AcceptSharedSearchKey()
{
	if( m_hwndSearchBox )
	{
		int	i;
		ComboBox_ResetContent( m_hwndSearchBox );
		for( i = 0; i < GetDllShareData().m_sSearchKeywords.m_aSearchKeys.size(); i++ )
		{
			Combo_AddString( m_hwndSearchBox, GetDllShareData().m_sSearchKeywords.m_aSearchKeys[i] );
		}
		ComboBox_SetCurSel( m_hwndSearchBox, 0 );
	}
}

int CMainToolBar::GetSearchKey(wchar_t* pBuf, int nBufCount)
{
	pBuf[0]=L'\0';
	return ::GetWindowText( m_hwndSearchBox, TcharReceiver<wchar_t>(pBuf,nBufCount), nBufCount );
	/*
	wmemset( szText, 0, _countof(szText) );
	::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
	*/
}


/*!
ツールバーの検索ボックスにフォーカスを移動する.
	@date 2006.06.04 yukihane 新規作成
*/
void CMainToolBar::SetFocusSearchBox( void ) const
{
	if( m_hwndSearchBox ){
		::SetFocus(m_hwndSearchBox);
	}
}
