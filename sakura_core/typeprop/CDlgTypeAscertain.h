﻿/*!	@file
	@brief タイプ別設定インポート確認ダイアログ

	@author Uchi
	@date 2010/4/17 新規作成
*/
/*
	Copyright (C) 2010, Uchi
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

#ifndef SAKURA_CDLGTYPEASCERTAIN_7000A035_D26C_4FB2_AE75_6A63F3F806B9_H_
#define SAKURA_CDLGTYPEASCERTAIN_7000A035_D26C_4FB2_AE75_6A63F3F806B9_H_
#pragma once

class CDlgTypeAscertain;

using std::wstring;

#include "dlg/CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief ファイルタイプ一覧ダイアログ
*/
class CDlgTypeAscertain final : public CDialog
{
public:
	// 型
	struct SAscertainInfo {
		wstring	sImportFile;	//!< in インポートファイル名
		wstring	sTypeNameTo;	//!< in タイプ名（インポート先）
		wstring	sTypeNameFile;	//!< in タイプ名（ファイルから）
		int 	nColorType;		//!< out 文書種類(カラーコピー用)
		wstring	sColorFile;		//!< out 色設定ファイル名
		bool	bAddType;		//!< out タイプを追加する
	};

public:
	//  Constructors
	CDlgTypeAscertain();
	// モーダルダイアログの表示
	int DoModal( HINSTANCE hInstance, HWND hwndParent, SAscertainInfo* psAscertainInfo );	/* モーダルダイアログの表示 */

protected:
	// 実装ヘルパ関数
	BOOL OnBnClicked(int wID) override;
	void SetData() override;	/* ダイアログデータの設定 */
	LPVOID GetHelpIdTable(void) override;

private:
	SAscertainInfo* m_psi;			// インターフェース
};
#endif /* SAKURA_CDLGTYPEASCERTAIN_7000A035_D26C_4FB2_AE75_6A63F3F806B9_H_ */
