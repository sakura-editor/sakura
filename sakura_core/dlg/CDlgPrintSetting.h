/*!	@file
	@brief 印刷設定ダイアログ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDLGPRINTSETTING_CA277796_0DDF_4D2D_92A6_67ACAC460E10_H_
#define SAKURA_CDLGPRINTSETTING_CA277796_0DDF_4D2D_92A6_67ACAC460E10_H_
#pragma once

#include "dlg/CDialog.h"
#include "config/maxdata.h" // MAX_PRINTSETTINGARR
#include "print/CPrint.h" //PRINTSETTING

/*!	印刷設定ダイアログ

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CDlgPrintSetting final : public CDialog
{
public:
	/*
	||  Constructors
	*/

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE hInstance, HWND hwndParent, int* pnCurrentPrintSetting,
				 PRINTSETTING* pPrintSettingArr, int nLineNumberColumns );	/* モーダルダイアログの表示 */

private:
	int				m_nCurrentPrintSetting;
	PRINTSETTING	m_PrintSettingArr[MAX_PRINTSETTINGARR];
	int				m_nLineNumberColumns;					// 行番号表示する場合の桁数
	bool			m_bPrintableLinesAndColumnInvalid;

protected:
	/*
	||  実装ヘルパ関数
	*/
	void SetData( void ) override;	/* ダイアログデータの設定 */
	int GetData( void ) override;	/* ダイアログデータの取得 */
	BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL OnDestroy( void ) override;
	BOOL OnNotify(NMHDR* pNMHDR) override;
	BOOL OnCbnSelChange(HWND hwndCtl, int wID) override;
	BOOL OnBnClicked(int wID) override;
	BOOL OnStnClicked(int wID) override;
	BOOL OnEnChange( HWND hwndCtl, int wID ) override;
	BOOL OnEnSetFocus(HWND hwndCtl, int wID) override;
	BOOL OnEnKillFocus( HWND hwndCtl, int wID ) override;
	LPVOID GetHelpIdTable(void) override;	//@@@ 2002.01.18 add

	void OnChangeSettingType(BOOL bGetData);	/* 設定のタイプが変わった */
	void OnSpin(int nCtrlId, BOOL bDown);	/* スピンコントロールの処理 */
	int DataCheckAndCorrect(int nCtrlId, int nData);	/* 入力値(数値)のエラーチェックをして正しい値を返す */
	BOOL CalcPrintableLineAndColumn();	/* 行数と桁数を計算 */
	void UpdatePrintableLineAndColumn();	/* 行数と桁数の計算要求 */
	void SetFontName( int idTxt, int idUse, LOGFONT& lf, int nPointSize );	// フォント名/使用ボタンの設定
};
#endif /* SAKURA_CDLGPRINTSETTING_CA277796_0DDF_4D2D_92A6_67ACAC460E10_H_ */
