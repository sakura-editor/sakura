/*! @file
	@brief 更新通知及び確認ダイアログ

	ファイルの更新通知と動作の確認を行うダイアログボックス

	@author genta
	@date 2002.12.04
*/
/*
	Copyright (C) 2002, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
