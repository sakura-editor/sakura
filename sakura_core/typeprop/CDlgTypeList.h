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
	BOOL OnActivate( WPARAM wParam, LPARAM lParam );
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
	void SetData();	/* ダイアログデータの設定 */
	void SetData(int);	/* ダイアログデータの設定 */
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	bool Import( void );			// 2010/4/12 Uchi
	bool Export( void );			// 2010/4/12 Uchi
	bool InitializeType( void );	// 2010/4/12 Uchi
	bool CopyType();
	bool UpType();
	bool DownType();
	bool AddType();
	bool DelType();
	bool AlertFileAssociation();	// 2011/8/20 syat

private:
	CTypeConfig				m_nSettingType;
	// 関連付け状態
	bool m_bRegistryChecked[ MAX_TYPES ];	//レジストリ確認 未／済
	bool m_bExtRMenu[ MAX_TYPES ];			//右クリック登録 未／済
	bool m_bExtDblClick[ MAX_TYPES ];		//ダブルクリック 未／済
	bool m_bAlertFileAssociation;			//関連付け警告の表示フラグ
	bool m_bEnableTempChange;				//一時適用の有効化
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGTYPELIST_H_ */



