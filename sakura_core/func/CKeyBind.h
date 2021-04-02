﻿/*!	@file
	@brief キー割り当てに関するクラス

	@author Norio Nakatani
	@date 1998/03/25 新規作成
	@date 1998/05/16 クラス内にデータを持たないように変更
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CKEYBIND_9B930CDD_DCA8_4096_B61C_37CED61C575A_H_
#define SAKURA_CKEYBIND_9B930CDD_DCA8_4096_B61C_37CED61C575A_H_
#pragma once

#include <Windows.h>
#include "Funccode_enum.h"

class CFuncLookup;
class CNativeW;

//! キー情報を保持する
struct KEYDATA {
	/*! キーコード	*/
	short			m_nKeyCode;
	
	/*!	キーの名前	*/
	WCHAR			m_szKeyName[30];
	
	/*!	対応する機能番号

		SHIFT, CTRL, ALTの３つのシフト状態のそれぞれに対して
		機能を割り当てるため、配列になっている。
	*/
	EFunctionCode	m_nFuncCodeArr[8];
};

/*! 仮想キーコード独自拡張 */
#define VKEX_DBL_CLICK		0x0100	// ダブルクリック
#define VKEX_R_CLICK		0x0101	// 右クリック
#define VKEX_MDL_CLICK		0x0102	// 中クリック
#define VKEX_LSD_CLICK		0x0103	// 左サイドクリック
#define VKEX_RSD_CLICK		0x0104	// 右サイドクリック

#define VKEX_TRI_CLICK		0x0105	// トリプルクリック
#define VKEX_QUA_CLICK		0x0106	// クアドラプルクリック

#define VKEX_WHEEL_UP		0x0107	// ホイールアップ
#define VKEX_WHEEL_DOWN		0x0108	// ホイールダウン
#define VKEX_WHEEL_LEFT		0x0109	// ホイール左
#define VKEX_WHEEL_RIGHT	0x010A	// ホイール右

extern const WCHAR* jpVKEXNames[];
extern const int jpVKEXNamesLen;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief キー割り当て関連ルーチン
	
	すべての関数はstaticで保持するデータはない。
*/
class CKeyBind
{
public:
	/*
	||  Constructors
	*/
	CKeyBind();
	~CKeyBind();

	/*
	||  参照系メンバ関数
	*/
	static HACCEL CreateAccerelator( int nKeyNameArrNum, KEYDATA* pKeyNameArr );
	static EFunctionCode GetFuncCode( WORD nAccelCmd, int nKeyNameArrNum, KEYDATA* pKeyNameArr, BOOL bGetDefFuncCode = TRUE );
	static EFunctionCode GetFuncCodeAt( KEYDATA& KeyData, int nState, BOOL bGetDefFuncCode = TRUE );	/* 特定のキー情報から機能コードを取得する */	// 2007.02.24 ryoji
	static EFunctionCode GetDefFuncCode( int nKeyCode, int nState );	/* キーのデフォルト機能を取得する */	// 2007.02.22 ryoji

	//! キー割り当て一覧を作成する
	static int CreateKeyBindList( HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, CNativeW& cMemList, CFuncLookup* pcFuncLookup, BOOL bGetDefFuncCode = TRUE );
	static int GetKeyStr( HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, CNativeW& cMemList, int nFuncId, BOOL bGetDefFuncCode = TRUE );	/* 機能に対応するキー名の取得 */
	static int GetKeyStrList( HINSTANCE	hInstance, int nKeyNameArrNum,KEYDATA* pKeyNameArr, CNativeW*** pppcMemList, int nFuncId, BOOL bGetDefFuncCode = TRUE );	/* 機能に対応するキー名の取得(複数) */
	static WCHAR* GetMenuLabel( HINSTANCE hInstance, int nKeyNameArrNum, KEYDATA* pKeyNameArr, int nFuncId, WCHAR* pszLabel, const WCHAR* pszKey, BOOL bKeyStr, int nLabelSize, BOOL bGetDefFuncCode = TRUE );	/* メニューラベルの作成 */	// add pszKey	2010/5/17 Uchi

	static WCHAR* MakeMenuLabel(const WCHAR* sName, const WCHAR* sKey);

protected:
	/*
	||  実装ヘルパ関数
	*/
	static bool GetKeyStrSub(int& nKeyNameArrBegin, int nKeyNameArrEnd, KEYDATA* pKeyNameArr,
			int nShiftState, CNativeW& cMemList, int nFuncId, BOOL bGetDefFuncCode );
};
#endif /* SAKURA_CKEYBIND_9B930CDD_DCA8_4096_B61C_37CED61C575A_H_ */
