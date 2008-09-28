/*!	@file
	@brief Dialog Box基底クラスヘッダファイル

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDialog;

#ifndef _CDIALOG_H_
#define _CDIALOG_H_

#include <windows.h>
#include <dbt.h>
#include "env/CShareData.h"
#include "global.h"


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief ダイアログウィンドウを扱うクラス

	ダイアログボックスを作るときにはここから継承させる．

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class SAKURA_CORE_API CDialog
{
public:
	/*
	||  Constructors
	*/
	CDialog();
	virtual ~CDialog();
	/*
	||  Attributes & Operations
	*/
	virtual INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */
	INT_PTR DoModal( HINSTANCE, HWND, int, LPARAM );	/* モーダルダイアログの表示 */
	HWND DoModeless( HINSTANCE, HWND, int, LPARAM, int );	/* モードレスダイアログの表示 */
	void CloseDialog( int );

	virtual BOOL OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	virtual BOOL OnDestroy( void );
	virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam ){return FALSE;}
	virtual BOOL OnSize( WPARAM wParam, LPARAM lParam );
	virtual BOOL OnMove( WPARAM wParam, LPARAM lParam );
	virtual BOOL OnDrawItem( WPARAM wParam, LPARAM lParam ){return TRUE;}
	virtual BOOL OnTimer( WPARAM wParam ){return TRUE;}
	virtual BOOL OnKeyDown( WPARAM wParam, LPARAM lParam ){return TRUE;}
	virtual BOOL OnDeviceChange( WPARAM wParam, LPARAM lParam ){return TRUE;}
	virtual int GetData( void ){return 1;}/* ダイアログデータの取得 */
	virtual void SetData( void ){return;}/* ダイアログデータの設定 */
	virtual BOOL OnBnClicked( int );
	virtual BOOL OnCbnSelChange( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnCbnEditChange( HWND hwndCtl, int wID ){return FALSE;} // @@2005.03.31 MIK タグジャンプDialog
//	virtual BOOL OnLbnSelChange( HWND hwndCtl, int wID ){return FALSE;}
//	virtual BOOL OnDbnDropDown( HWND hwndCtl, int wID ){return FALSE;}
//	virtual BOOL OnDbnCloseUp( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnEditChange( HWND hwndCtl, int wID ){return FALSE;}

	virtual BOOL OnLbnDblclk( int wID ){return FALSE;}
	virtual BOOL OnKillFocus( WPARAM wParam, LPARAM lParam ){return FALSE;}
	virtual BOOL OnActivate( WPARAM wParam, LPARAM lParam ){return FALSE;}	//@@@ 2003.04.08 MIK
	virtual int OnVKeyToItem( WPARAM wParam, LPARAM lParam ){ return -1; }
	virtual LRESULT OnCharToItem( WPARAM wParam, LPARAM lParam ){ return -1; }
//	virtual BOOL OnNextDlgCtl( WPARAM wParam, LPARAM lParam ){ return 1; }
	virtual BOOL OnPopupHelp( WPARAM, LPARAM );	//@@@ 2002.01.18 add
	virtual BOOL OnContextMenu( WPARAM, LPARAM );	//@@@ 2002.01.18 add
	virtual LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add


//	virtual BOOL OnLbnSelChange( HWND, int );

public:
	HWND GetHwnd() const{ return m_hWnd; }
	//特殊インターフェース (使用は好ましくない)
	void _SetHwnd(HWND hwnd){ m_hWnd = hwnd; }

public:
	HINSTANCE		m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */
private:
	HWND			m_hWnd;			/* このダイアログのハンドル */
public:
	HWND			m_hwndSizeBox;
	LPARAM			m_lParam;
	BOOL			m_bModal;		/* モーダル ダイアログか */
	int				m_nShowCmd;		//	最大化/最小化
	int				m_nWidth;
	int				m_nHeight;
	int				m_xPos;
	int				m_yPos;
//	void*			m_pcEditView;
	SFilePath		m_szHelpFile;
	DLLSHAREDATA*	m_pShareData;
	BOOL			m_bInited;


protected:
	void CreateSizeBox( void );
	BOOL OnCommand( WPARAM, LPARAM );

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDIALOG_H_ */



