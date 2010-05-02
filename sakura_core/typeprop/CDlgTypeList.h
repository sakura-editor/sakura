/*!	@file
	@brief ファイルタイプ一覧ダイアログ

	@author Norio Nakatani
	@date 1998/12/23 新規作成
	@date 1999/12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgTypeList;

#ifndef _CDLGTYPELIST_H_
#define _CDLGTYPELIST_H_

#include "dlg/CDialog.h"
using std::wstring;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief ファイルタイプ一覧ダイアログ
*/
class CDlgTypeList : public CDialog
{
public:
	// 型
	struct SResult{
		CTypeConfig	cDocumentType;	//!< 文書種類
		bool			bTempChange;	//!< 旧PROP_TEMPCHANGE_FLAG
	};

public:
	// インターフェース
	int DoModal( HINSTANCE, HWND, SResult* );	/* モーダルダイアログの表示 */

protected:
	// 実装ヘルパ関数
	BOOL OnLbnDblclk( int );
	BOOL OnBnClicked( int );
	void SetData();	/* ダイアログデータの設定 */
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	bool Import( void );			// 2010/4/12 Uchi
	bool Export( void );			// 2010/4/12 Uchi
	bool InitializeType( void );	// 2010/4/12 Uchi

private:
	CTypeConfig				m_nSettingType;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGTYPELIST_H_ */



