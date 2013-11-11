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

#include "CDialog.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief ファイルタイプ一覧ダイアログ
*/
class CDlgTypeList : public CDialog
{
public:
	struct SResult{
		int	cDocumentType;	//!< 文書種類
		bool			bTempChange;	//!< 旧PROP_TEMPCHANGE_FLAG
	};

	int DoModal( HINSTANCE, HWND, SResult* );	/* モーダルダイアログの表示 */

protected:
	//  実装ヘルパ関数
	BOOL OnLbnDblclk( int );
	BOOL OnBnClicked( int );
	void SetData();	/* ダイアログデータの設定 */
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

private:
	int				m_nSettingType;
	bool m_bEnableTempChange;				//一時適用の有効化
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGTYPELIST_H_ */


/*[EOF]*/
