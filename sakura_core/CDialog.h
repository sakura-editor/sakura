//	$Id$
/*!	@file
	@brief Dialog Box基底クラスヘッダファイル

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDialog;

#ifndef _CDIALOG_H_
#define _CDIALOG_H_

#include <windows.h>
#include <dbt.h>
#include "sakura_rc.h"
//#include "debug.h"
#include "CShareData.h"
#include "etc_uty.h"
#include "debug.h"
#include "global.h"


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief ダイアログウィンドウを扱うクラス

	ダイアログボックスを作るときにはここから継承させる．
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
	virtual BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */
	int DoModal( HINSTANCE, HWND, int, LPARAM );	/* モーダルダイアログの表示 */
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
	virtual BOOL OnDbnDropDown( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnDbnCloseUp( HWND hwndCtl, int wID ){return FALSE;}
	virtual BOOL OnLbnDblclk( int wID ){return FALSE;}
	virtual BOOL OnKillFocus( WPARAM wParam, LPARAM lParam ){return FALSE;}
	virtual int OnVKeyToItem( WPARAM wParam, LPARAM lParam ){ return -1; }
	virtual LRESULT OnCharToItem( WPARAM wParam, LPARAM lParam ){ return -1; }
//	virtual BOOL OnNextDlgCtl( WPARAM wParam, LPARAM lParam ){ return 1; }
	virtual BOOL OnPopupHelp( WPARAM, LPARAM );	//@@@ 2002.01.18 add
	virtual BOOL OnContextMenu( WPARAM, LPARAM );	//@@@ 2002.01.18 add
	virtual LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add


//	virtual BOOL OnLbnSelChange( HWND, int );



	HINSTANCE		m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND			m_hWnd;			/* このダイアログのハンドル */
	HWND			m_hwndSizeBox;
	LPARAM			m_lParam;
	BOOL			m_bModal;		/* モーダル ダイアログか */
	int				m_nWidth;
	int				m_nHeight;
	int				m_xPos;
	int				m_yPos;
//	void*			m_pcEditView;
	char			m_szHelpFile[_MAX_PATH + 1];
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	BOOL			m_bInited;


protected:
	void CreateSizeBox( void );
	BOOL OnCommand( WPARAM, LPARAM );

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDIALOG_H_ */


/*[EOF]*/
