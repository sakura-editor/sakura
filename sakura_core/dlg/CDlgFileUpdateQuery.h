﻿/*! @file
	@brief 更新通知及び確認ダイアログ

	ファイルの更新通知と動作の確認を行うダイアログボックス

	@author genta
	@date 2002.12.04
*/
/*
	Copyright (C) 2002, genta
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
#ifndef SAKURA_CDLGFILEUPDATEQUERY_1A03B33B_FB78_44D9_BB96_96506DAD0C2E_H_
#define SAKURA_CDLGFILEUPDATEQUERY_1A03B33B_FB78_44D9_BB96_96506DAD0C2E_H_
#pragma once

#include "dlg/CDialog.h"

enum EFileUpdateQuery {
	EFUQ_CLOSE			= 0,	//!< 閉じる
	EFUQ_RELOAD			= 1,	//!< 再読込
	EFUQ_NOTIFYONLY		= 2,	//!< 以後通知メッセージのみ
	EFUQ_NOSUPERVISION	= 3,	//!< 以後更新を監視しない
	EFUQ_AUTOLOAD		= 4		//!< 以後未編集で再ロード
};

class CDlgFileUpdateQuery final : public CDialog {
public:
	CDlgFileUpdateQuery(const WCHAR* filename, bool IsModified)
	: m_pFilename( filename )
	, m_bModified( IsModified )
	{
	}
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam ) override;
	BOOL OnBnClicked( int id ) override;

private:
	const WCHAR* m_pFilename;
	bool m_bModified;
};
#endif /* SAKURA_CDLGFILEUPDATEQUERY_1A03B33B_FB78_44D9_BB96_96506DAD0C2E_H_ */
