﻿/*!	@file
	@brief ファンクションキーウィンドウ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2006, aroka
	Copyright (C) 2007, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#ifndef SAKURA_CFUNCKEYWND_2EB0FD88_ABBB_4280_BEEA_46E8468E4550_H_
#define SAKURA_CFUNCKEYWND_2EB0FD88_ABBB_4280_BEEA_46E8468E4550_H_
#pragma once

#include "window/CWnd.h"
#include "env/DLLSHAREDATA.h"

struct DLLSHAREDATA;
class CEditDoc; // 2002/2/10 aroka

//! ファンクションキーウィンドウ
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class CFuncKeyWnd final : public CWnd
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
	HWND Open( HINSTANCE, HWND, CEditDoc*, bool );	/* ウィンドウ オープン */
	void Close( void );	/* ウィンドウ クローズ */
	void SizeBox_ONOFF(bool bSizeBox);	/* サイズボックスの表示／非表示切り替え */
	void Timer_ONOFF(bool bStart); /* 更新の開始／停止 20060126 aroka */
	/*
	|| メンバ変数
	*/
private:
	// 20060126 aroka すべてPrivateにして、初期化順序に合わせて並べ替え
	CEditDoc*		m_pcEditDoc;
	DLLSHAREDATA*	m_pShareData;
	int				m_nCurrentKeyState;
	WCHAR			m_szFuncNameArr[12][256];
	HWND			m_hwndButtonArr[12];
	HFONT			m_hFont;	/*!< 表示用フォント */
	bool			m_bSizeBox;
	HWND			m_hwndSizeBox;
	int				m_nTimerCount;
	int				m_nButtonGroupNum; // Openで初期化
	EFunctionCode	m_nFuncCodeArr[12]; // Open->CreateButtonsで初期化
protected:
	/*
	|| 実装ヘルパ系
	*/
	void CreateButtons( void );	/* ボタンの生成 */
	int CalcButtonSize( void );	/* ボタンのサイズを計算 */

	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	LRESULT OnTimer(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;	// WM_TIMERタイマーの処理
	LRESULT OnCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;	// WM_COMMAND処理
	LRESULT OnSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;// WM_SIZE処理
	LRESULT OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;// WM_DESTROY処理
};
#endif /* SAKURA_CFUNCKEYWND_2EB0FD88_ABBB_4280_BEEA_46E8468E4550_H_ */
