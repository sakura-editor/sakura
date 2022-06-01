﻿/*!	@file
	@brief ファイルプロパティダイアログ

	@author Norio Nakatani
	@date 1999/02/31 新規作成
	@date 1999/12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2022, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CDLGPROPERTY_FF915918_FBA0_4D89_9A72_5E1409D7F88A_H_
#define SAKURA_CDLGPROPERTY_FF915918_FBA0_4D89_9A72_5E1409D7F88A_H_
#pragma once

class CDlgProperty;

#include "dlg/CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgProperty final : public CDialog
{
public:
	int DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam);	/* モーダルダイアログの表示 */
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnBnClicked(int wID) override;
	void SetData( void ) override;	/* ダイアログデータの設定 */
	LPVOID GetHelpIdTable(void) override;	//@@@ 2002.01.18 add
};
#endif /* SAKURA_CDLGPROPERTY_FF915918_FBA0_4D89_9A72_5E1409D7F88A_H_ */
