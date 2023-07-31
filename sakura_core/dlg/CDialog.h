/*!	@file
	@brief Dialog Box基底クラスヘッダーファイル

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2011, nasukoji
	Copyright (C) 2012, Uchi
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CDIALOG_17C8C15C_881C_4C1F_B953_CB11FCC8B70B_H_
#define SAKURA_CDIALOG_17C8C15C_881C_4C1F_B953_CB11FCC8B70B_H_
#pragma once

#include "apimodule/User32Dll.hpp"

#include "env/ShareDataAccessor.hpp"
#include "env/DLLSHAREDATA.h"

#include <windowsx.h>

#include "sakura_rc.h"

class CDialog;
class CRecent;

enum EAnchorStyle
{
	ANCHOR_NONE              = 0,
	ANCHOR_LEFT              = 1,
	ANCHOR_RIGHT             = 2,
	ANCHOR_LEFT_RIGHT        = 3,
	ANCHOR_TOP               = 4,
	ANCHOR_TOP_LEFT          = 5,
	ANCHOR_TOP_RIGHT         = 6,
	ANCHOR_TOP_LEFT_RIGHT    = 7,
	ANCHOR_BOTTOM            = 8,
	ANCHOR_BOTTOM_LEFT       = 9,
	ANCHOR_BOTTOM_RIGHT      = 10,
	ANCHOR_BOTTOM_LEFT_RIGHT = 11,
	ANCHOR_TOP_BOTTOM        = 12,
	ANCHOR_TOP_BOTTOM_LEFT   = 13,
	ANCHOR_TOP_BOTTOM_RIGHT  = 14,
	ANCHOR_ALL               = 15
};

struct SAnchorList
{
	int id;
	EAnchorStyle anchor;
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief ダイアログウィンドウを扱うクラス

	ダイアログボックスを作るときにはここから継承させる．

*/
class CDialog : public User32DllClient
{
	WORD _idDialog;

	using Me = CDialog;

public:
	/*
	||  Constructors
	*/
	explicit CDialog(WORD idDialog_ = 0, std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>());
	CDialog(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CDialog(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CDialog() override;

	/*
	||  Attributes & Operations
	*/
	virtual INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */
	INT_PTR DoModal(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam);	/* モーダルダイアログの表示 */
	virtual INT_PTR DoModal(HWND hWndParent = NULL, LPARAM lParam = 0L);
	HWND    DoModeless(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam, int nCmdShow);	/* モードレスダイアログの表示 */
	HWND    Show(HWND hWndParent = NULL, int nCmdShow = SW_SHOW, LPARAM lParam = 0L);
	HWND DoModeless(HINSTANCE hInstance, HWND hwndParent, LPCDLGTEMPLATE lpTemplate, LPARAM lParam, int nCmdShow);	/* モードレスダイアログの表示 */
	void CloseDialog(INT_PTR nModalRetVal);

private:
	static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	virtual void    SetDlgData(HWND hDlg) const;
	virtual INT_PTR GetDlgData(HWND hDlg);

	virtual INT_PTR DispatchDlgEvent(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual BOOL    OnDlgInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
	virtual BOOL    OnDlgDestroy(HWND hDlg);
	virtual BOOL    OnDlgMove(HWND hDlg, int x, int y);
	virtual BOOL    OnDlgActivate(HWND hDlg, UINT state, HWND hWndActDeact, BOOL fMinimized);
	virtual BOOL    OnDlgKillFocus(HWND hDlg, HWND hWndNewFocus);
	virtual BOOL    OnDlgNotify(HWND hDlg, int idFrom, LPNMHDR pNMHDR);
	virtual BOOL    OnDlgKey(HWND hDlg, UINT vk, BOOL fDown, int cRepeat, UINT flags);
	virtual BOOL    OnDlgCommand(HWND hDlg, int id, HWND hWndCtl, UINT codeNotify);
	virtual BOOL    OnDlgTimer(HWND hDlg, UINT id);
	virtual BOOL    OnDlgHelp(HWND hDlg, LPHELPINFO pHelpInfo);
	virtual BOOL    OnDlgContextMenu(HWND hDlg, HWND hWndContext, UINT xPos, UINT yPos);

public:
	virtual BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	virtual void SetDialogPosSize();
	virtual BOOL OnDestroy( void );
	virtual BOOL OnNotify(NMHDR* pNMHDR){return FALSE;}
	BOOL OnSize();
	virtual BOOL OnSize( WPARAM wParam, LPARAM lParam );
	virtual BOOL OnMove( WPARAM wParam, LPARAM lParam ){return TRUE;}
	virtual BOOL OnDrawItem( WPARAM wParam, LPARAM lParam ){return TRUE;}
	virtual BOOL OnTimer( WPARAM wParam ){return TRUE;}
	virtual BOOL OnKeyDown( WPARAM wParam, LPARAM lParam ){return TRUE;}
	virtual int GetData( void ){return 1;}/* ダイアログデータの取得 */
	virtual void SetData( void ){return;}/* ダイアログデータの設定 */
	virtual BOOL OnBnClicked(int wID);
	virtual BOOL OnStnClicked( int ){return FALSE;}
	virtual BOOL OnEnChange( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnEnSetFocus( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnEnKillFocus( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnLbnSelChange( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnLbnDblclk( int wID ){return FALSE;}
	virtual BOOL OnCbnSelChange( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnCbnEditChange( HWND hwndCtl, int wID ){return FALSE;} // @@2005.03.31 MIK タグジャンプDialog
	virtual BOOL OnCbnDropDown( HWND hwndCtl, int wID );
	static BOOL OnCbnDropDown( HWND hwndCtl, bool scrollBar );
//	virtual BOOL OnCbnCloseUp( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnCbnSelEndOk( HWND hwndCtl, int wID );

	virtual BOOL OnKillFocus( WPARAM wParam, LPARAM lParam ){return FALSE;}
	virtual BOOL OnActivate( WPARAM wParam, LPARAM lParam ){return FALSE;}	//@@@ 2003.04.08 MIK
	virtual BOOL OnPopupHelp(WPARAM wPara, LPARAM lParam);	//@@@ 2002.01.18 add
	virtual BOOL OnContextMenu(WPARAM wPara, LPARAM lParam);	//@@@ 2002.01.18 add
	virtual LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	virtual INT_PTR GetHelpIds(void) const noexcept;

	void ResizeItem( HWND hTarget, const POINT& ptDlgDefalut, const POINT& ptDlgNew, const RECT& rcItemDefault, EAnchorStyle anchor, bool bUpdate = true);
	void GetItemClientRect( int wID, RECT& rc );

	//! @brief コンボボックスに履歴削除・単語削除の機能を追加する
	//!
	//! @param hwndCtl コンボボックスのハンドル。CBS_DROPDOWNLISTスタイルのコンボボックスには対応していません。
	//! @param pRecent 紐づけるCRecentへのポインタ。nullptrは指定できません。
	static void SetComboBoxDeleter( HWND hwndCtl, CRecent* pRecent );
public:

	static bool DirectoryUp(WCHAR* szDir);

public:
	HWND GetHwnd() const{ return m_hWnd; }
	//特殊インターフェース (使用は好ましくない)
	void _SetHwnd(HWND hwnd){ m_hWnd = hwnd; }

public:
	HINSTANCE		m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */
private:
	HWND			m_hWnd;			/* このダイアログのハンドル */
	HFONT			m_hFontDialog;	// ダイアログに設定されているフォント(破棄禁止)
public:
	HWND			m_hwndSizeBox;
	LPARAM			m_lParam;
	BOOL			m_bModal;		/* モーダル ダイアログか */
	bool			m_bSizable;		// 可変ダイアログかどうか
	int				m_nShowCmd;		//	最大化/最小化
//	void*			m_pcEditView;
	BOOL			m_bInited;
	HINSTANCE		m_hLangRsrcInstance;		// メッセージリソースDLLのインスタンスハンドル	// 2011.04.10 nasukoji

protected:
	int				m_nWidth;
	int				m_nHeight;
	int				m_xPos;
	int				m_yPos;
	void CreateSizeBox( void );
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	HWND GetItemHwnd(int nID){ return ::GetDlgItem( GetHwnd(), nID ); }

	// このダイアログに設定されているフォントを取得
	HFONT GetDialogFont() { return m_hFontDialog; }

	virtual LONG_PTR SetWindowLongPtrW(_In_ HWND hWnd, int nIndex, LONG_PTR dwNewLong) const
	{
		return GetUser32Dll()->SetWindowLongPtrW(hWnd, nIndex, dwNewLong);
	}

	virtual bool ShowWindow(
		_In_ HWND hWnd,
		_In_ int nCmdShow) const
	{
		return GetUser32Dll()->ShowWindow(hWnd, nCmdShow);
	}
};

/*!
 * 拡張版ダイアログの基底クラス
 *
 * 共有メモリにアクセスする機能を付加する。
 * 
 * @date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
 */
class CSakuraDialog : public CDialog, public ShareDataAccessorClient
{
public:
	DLLSHAREDATA* m_pShareData;

	explicit CSakuraDialog(WORD idDialog_, std::shared_ptr<ShareDataAccessor> ShareDataAccessor_, std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>())
		: CDialog(idDialog_, std::move(User32Dll_))
		, ShareDataAccessorClient(std::move(ShareDataAccessor_))
	{
		// 共有メモリのアドレスをメンバ変数に取得する
		m_pShareData = &GetDllShareData();
	}
	~CSakuraDialog() override = default;

protected:
	// コントロールに画面のフォントを設定	2012/11/27 Uchi
	HFONT SetMainFont(HWND hTarget);
};

/*!
 * 可変ダイアログの基底クラス
 *
 * 表示位置とサイズを復元する機能を付加する。
 */
class CSizeRestorableDialog : public CSakuraDialog
{
public:
	explicit CSizeRestorableDialog(WORD idDialog_, std::shared_ptr<ShareDataAccessor> ShareDataAccessor_, std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>())
		: CSakuraDialog(idDialog_, std::move(ShareDataAccessor_), std::move(User32Dll_))
	{
	}
	~CSizeRestorableDialog() override = default;
};

#endif /* SAKURA_CDIALOG_17C8C15C_881C_4C1F_B953_CB11FCC8B70B_H_ */
