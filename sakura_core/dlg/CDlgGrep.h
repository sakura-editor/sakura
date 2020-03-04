/*!	@file
	@brief GREPダイアログボックス

	@author Norio Nakatani
	@date 1998.09/07  新規作成
	@date 1999.12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#pragma once

class CDlgGrep;

#include "dlg/CDialog.h"
#include "recent/CRecent.h"
#include "util/window.h"

#define DEFAULT_EXCLUDE_FILE_PATTERN    L"*.msi;*.exe;*.obj;*.pdb;*.ilk;*.res;*.pch;*.iobj;*.ipdb"
#define DEFAULT_EXCLUDE_FOLDER_PATTERN  L".git;.svn;.vs"

//! GREPダイアログボックス
class CDlgGrep : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgGrep();
	/*
	||  Attributes & Operations
	*/
	BOOL OnCbnDropDown( HWND hwndCtl, int wID ) override;
	int DoModal( HINSTANCE, HWND, const WCHAR* );	/* モーダルダイアログの表示 */
//	HWND DoModeless( HINSTANCE, HWND, const char* );	/* モードレスダイアログの表示 */

	BOOL		m_bSubFolder;/*!< サブフォルダからも検索する */
	BOOL		m_bFromThisText;/*!< この編集中のテキストから検索する */
	BOOL		m_bUseRipgrep;//!<ripgrepを使う

	SSearchOption	m_sSearchOption;	//!< 検索オプション

	ECodeType	m_nGrepCharSet;			/*!< 文字コードセット */
	int			m_nGrepOutputStyle;		/*!< Grep: 出力形式 */
	int			m_nGrepOutputLineType;		//!< 結果出力：行を出力/該当部分/否マッチ行
	bool		m_bGrepOutputFileOnly;		/*!< ファイル毎最初のみ検索 */
	bool		m_bGrepOutputBaseFolder;	/*!< ベースフォルダ表示 */
	bool		m_bGrepSeparateFolder;		/*!< フォルダ毎に表示 */

	std::wstring	m_strText;				/*!< 検索文字列 */
	bool			m_bSetText;				//!< 検索文字列を設定したか
	SFilePathLong	m_szFile;				//!< 検索ファイル
	SFilePathLong	m_szFolder;				//!< 検索フォルダ
	SFilePathLong	m_szExcludeFile;		//!< 除外ファイル
	SFilePathLong	m_szExcludeFolder;		//!< 除外フォルダ
	SFilePath	m_szCurrentFilePath;
protected:
	SComboBoxItemDeleter	m_comboDelText;
	CRecentSearch			m_cRecentSearch;

	SComboBoxItemDeleter	m_comboDelFile;
	CRecentGrepFile			m_cRecentGrepFile;

	SComboBoxItemDeleter	m_comboDelFolder;
	CRecentGrepFolder		m_cRecentGrepFolder;

	SComboBoxItemDeleter	m_comboDelExcludeFile;
	CRecentExcludeFile		m_cRecentExcludeFile;

	SComboBoxItemDeleter	m_comboDelExcludeFolder;
	CRecentExcludeFolder	m_cRecentExcludeFolder;

	CFontAutoDeleter		m_cFontText;

	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL OnDestroy() override;
	BOOL OnBnClicked(int wID) override;
	LPVOID GetHelpIdTable(void) override;	//@@@ 2002.01.18 add

	void SetData( void ) override;	/* ダイアログデータの設定 */
	int GetData( void ) override;	/* ダイアログデータの取得 */
	void SetDataFromThisText(bool bChecked);	/* 現在編集中ファイルから検索チェックでの設定 */
	void SetUseripgrep(bool bChecked); /* ripgrepを使うチェックでの設定 */
};
