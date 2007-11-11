#pragma once

class CEditWnd;

class CMainToolBar{
public:
	CMainToolBar(CEditWnd* pOwner);

	//作成・破棄
	void CreateToolBar( void );		//!< ツールバー作成
	void DestroyToolBar( void );	//!< ツールバー破棄

	//メッセージ
	bool EatMessage(MSG* msg);		//!< メッセージ処理。なんか処理したなら true を返す。
	void ProcSearchBox( MSG* );		//!< 検索コンボボックスのメッセージ処理

	//イベント
	void OnToolbarTimer( void );	//!< タイマーの処理 20060128 aroka

	//描画
	LPARAM ToolBarOwnerDraw( LPNMCUSTOMDRAW pnmh );

	//共有データとの同期
	void AcceptSharedSearchKey();

	//取得
	HWND GetToolbarHwnd() const{ return m_hwndToolBar; }
	HWND GetRebarHwnd() const{ return m_hwndReBar; }
	HWND GetSearchHwnd() const{ return m_hwndSearchBox; }
	int GetSearchKey(wchar_t* pBuf, int nBufCount); //!< 検索キーを取得。戻り値は検索キーの文字数。

	//操作
	void SetFocusSearchBox( void ) const;		/* ツールバー検索ボックスへフォーカスを移動 */	// 2006.06.04 yukihane

private:
	CEditWnd*	m_pOwner;
    HWND		m_hwndToolBar;

	//子ウィンドウ
    HWND		m_hwndReBar;		//!< Rebar ウィンドウ	//@@@ 2006.06.17 ryoji
	HWND		m_hwndSearchBox;	//!< 検索コンボボックス

	//フォント
	HFONT		m_fontSearchBox;	//!< 検索コンボボックスのフォント
};
