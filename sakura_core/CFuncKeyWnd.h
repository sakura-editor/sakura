/*!	@file
	@brief ファンクションキーウィンドウ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2006, aroka
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
class CFuncKeyWnd;

#ifndef _CFUNCKEYWND_H_
#define _CFUNCKEYWND_H_

#include "CWnd.h"
#include "CShareData.h"
class CEditDoc; // 2002/2/10 aroka

//! ファンクションキーウィンドウ
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
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
	HWND Open( HINSTANCE, HWND, CEditDoc*, BOOL );	/* ウィンドウ オープン */
	void Close( void );	/* ウィンドウ クローズ */
	void SizeBox_ONOFF( BOOL );	/* サイズボックスの表示／非表示切り替え */
	void Timer_ONOFF( BOOL ); /* 更新の開始／停止 20060126 aroka */
	/*
	|| メンバ変数
	*/
private:
	// 20060126 aroka すべてPrivateにして、初期化順序に合わせて並べ替え
	const TCHAR*	m_pszClassName;	/*!< クラス名 */
	CEditDoc*		m_pCEditDoc;
	DLLSHAREDATA*	m_pShareData;
	int				m_nCurrentKeyState;
	WCHAR			m_szFuncNameArr[12][256];
	HWND			m_hwndButtonArr[12];
	HFONT			m_hFont;	/*!< 表示用フォント */
	BOOL			m_bSizeBox;
	HWND			m_hwndSizeBox;
	int				m_nTimerCount;
	int				m_nButtonGroupNum; // Openで初期化
	int				m_nFuncCodeArr[12]; // Open->CreateButtonsで初期化
protected:
	/*
	|| 実装ヘルパ系
	*/
	void CreateButtons( void );	/* ボタンの生成 */
	int CalcButtonSize( void );	/* ボタンのサイズを計算 */

	/* 仮想関数 */
	virtual void AfterCreateWindow( void ){}	// ウィンドウ作成後の処理	// 2007.03.13 ryoji 可視化しない

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	virtual LRESULT OnTimer( HWND, UINT, WPARAM, LPARAM );	// WM_TIMERタイマーの処理
	virtual LRESULT OnCommand( HWND, UINT, WPARAM, LPARAM );	// WM_COMMAND処理
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );// WM_SIZE処理
	virtual LRESULT OnDestroy( HWND, UINT, WPARAM, LPARAM );// WM_DESTROY処理
};


///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCKEYWND_H_ */


/*[EOF]*/
