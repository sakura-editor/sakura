/*!	@file
	@brief 文字列共通定義

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, Stonee, jepro
	Copyright (C) 2002, KK
	Copyright (C) 2003, MIK
	Copyright (C) 2005, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/global.h"

#include "_main/CNormalProcess.h"
#include "basis/CErrorInfo.h"
#include "config/app_constants.h"
#include "window/CEditWnd.h"
#include "version.h"

#ifdef DEV_VERSION
#pragma message("-------------------------------------------------------------------------------------")
#pragma message("---  This is a Dev version and under development. Be careful to use this version. ---")
#pragma message("-------------------------------------------------------------------------------------")
#endif

/*!
	アプリ名を取得します。
	プロセスの生成前にアプリ名を取得することはできません。

	@date 2007/09/21 kobake 整理
 */
LPCWSTR GetAppName( void )
{
	const auto pcProcess = CProcess::getInstance();
	if( !pcProcess )
	{
		::_com_raise_error(E_FAIL, MakeMsgError(L"Any process has been instantiated."));
	}
	return pcProcess->GetAppName();
}

/*! 選択領域描画用パラメータ */
const COLORREF	SELECTEDAREA_RGB = RGB( 255, 255, 255 );
const int		SELECTEDAREA_ROP2 = R2_XORPEN;

HINSTANCE G_AppInstance()
{
	return CProcess::getInstance()->GetProcessInstance();
}

/*!
 * コンストラクタ
 */
SSearchOption::SSearchOption() noexcept
	: SSearchOption(false, false, false)
{
}

/*!
 * コンストラクタ(値指定)
 */
SSearchOption::SSearchOption(
	bool _bRegularExp,
	bool _bLoHiCase,
	bool _bWordOnly
) noexcept
	: bRegularExp(_bRegularExp)
	, bLoHiCase(_bLoHiCase)
	, bWordOnly(_bWordOnly)
{
}

//! リセットする(全部falseにする)
void SSearchOption::Reset()
{
	bRegularExp = false;
	bLoHiCase = false;
	bWordOnly = false;
}

/*!
 * 同型との等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しい
 * @retval false 等しくない
 */
bool SSearchOption::operator == (const SSearchOption& rhs) const noexcept
{
	if (this == &rhs) return true;
	return bRegularExp == rhs.bRegularExp
		&& bLoHiCase == rhs.bLoHiCase
		&& bWordOnly == rhs.bWordOnly;
}

/*!
 * 同型との否定の等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しくない
 * @retval false 等しい
 */
bool SSearchOption::operator != (const SSearchOption& rhs) const noexcept
{
	return !(*this == rhs);
}
