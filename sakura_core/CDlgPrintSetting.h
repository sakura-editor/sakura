//	$Id$
/*!	@file
	@brief 印刷設定ダイアログ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgPrintSetting;

#ifndef _CDLGPRINTSETTING_H_
#define _CDLGPRINTSETTING_H_

#include "CDialog.h"

/*!	印刷設定ダイアログ

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CDlgPrintSetting : public CDialog
{
public:
	/*
	||  Constructors
	*/

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, int*, PRINTSETTING* );	/* モーダルダイアログの表示 */

	int				m_nCurrentPrintSetting;
	PRINTSETTING	m_PrintSettingArr[MAX_PRINTSETTINGARR];


protected:
	/*
	||  実装ヘルパ関数
	*/
	void SetData( void );	/* ダイアログデータの設定 */
	int GetData( void );	/* ダイアログデータの取得 */
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnDestroy( void );
	BOOL OnNotify( WPARAM,  LPARAM );
	BOOL OnCbnSelChange( HWND, int );
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void OnChangeSettingType( BOOL );	/* 設定のタイプが変わった */
	void OnSpin( int , BOOL );	/* スピンコントロールの処理 */
	int DataCheckAndCrrect( int , int );	/* 入力値(数値)のエラーチェックをして正しい値を返す */
	BOOL OnTimer( WPARAM );	/* タイマー処理 */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPRINTSETTING_H_ */


/*[EOF]*/
