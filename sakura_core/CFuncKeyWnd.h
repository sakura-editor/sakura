//	$Id$
/************************************************************************
*
*	CFuncKeyWnd.h
*	Copyright (C) 1998-2000, Norio Nakatani
*
************************************************************************/
class CFuncKeyWnd;

#ifndef _CFUNCKEYWND_H_
#define _CFUNCKEYWND_H_

#include "CWnd.h"
//#include <windows.h>
#include "CShareData.h"
#include "CEditDoc.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CFuncKeyWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CFuncKeyWnd();
	virtual ~CFuncKeyWnd();
	/*
	|| メンバ関数
	*/
	HWND Open( HINSTANCE, HWND, CEditDoc*, BOOL );	/* ウィンドウ　オープン */
	void Close( void );	/* ウィンドウ　クローズ */
	void SizeBox_ONOFF( BOOL );	/* サイズボックスの表示／非表示切り替え */
	/*
	|| メンバ変数
	*/
	const char*		m_pszClassName;	/* クラス名 */
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	CEditDoc*		m_pCEditDoc;
	HFONT			m_hFont;	/* 表示用フォント */
	int				m_nCurrentKeyState;
	char			m_szFuncNameArr[12][256];
	HWND			m_hwndButtonArr[12];
	int				m_nFuncCodeArr[12];
	int				m_nButtonGroupNum;
	BOOL			m_bSizeBox;
	HWND			m_hwndSizeBox;
	int				m_nTimerCount;
protected:
	/*
	|| 実装ヘルパ系
	*/
	void CreateButtons( void );	/* ボタンの生成 */
	int CalcButtonSize( void );	/* ボタンのサイズを計算 */

	/* 仮想関数 */
	
	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	virtual LRESULT OnTimer( HWND, UINT, WPARAM, LPARAM );	// WM_TIMERタイマーの処理
	virtual LRESULT OnCommand( HWND, UINT, WPARAM, LPARAM );	// WM_COMMAND処理
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );// WM_SIZE処理
	virtual LRESULT OnDestroy( HWND, UINT, WPARAM, LPARAM );// WM_DSESTROY処理
};


///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCKEYWND_H_ */

/*[EOF]*/
