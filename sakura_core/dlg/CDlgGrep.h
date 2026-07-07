/*!	@file
	@brief GREPダイアログボックス

	@author Norio Nakatani
	@date 1998.09/07  新規作成
	@date 1999.12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#ifndef SAKURA_CDLGGREP_01A0D5CB_326B_4C56_A527_C811F84FD8D8_H_
#define SAKURA_CDLGGREP_01A0D5CB_326B_4C56_A527_C811F84FD8D8_H_
#pragma once

#include "dlg/CDialog.h"
#include "util/window.h"
#include "recent/CRecentSearch.h"
#include "recent/CRecentGrepFile.h"
#include "recent/CRecentGrepFolder.h"
#include "recent/CRecentExcludeFile.h"
#include "recent/CRecentExcludeFolder.h"

// 正規表現ONのときの既定（フルパスに対する正規表現）
inline constexpr const wchar_t* DEFAULT_EXCLUDE_FILE_PATTERN_REGEX    = L".*\\.msi$;.*\\.exe$;.*\\.obj$;.*\\.pdb$;.*\\.ilk$;.*\\.res$;.*\\.pch$;.*\\.iobj$;.*\\.ipdb$";
// 正規表現OFFのときの既定（ワイルドカード）
inline constexpr const wchar_t* DEFAULT_EXCLUDE_FILE_PATTERN_WILDCARD = L"*.msi;*.exe;*.obj;*.pdb;*.ilk;*.res;*.pch;*.iobj;*.ipdb";
inline constexpr const wchar_t* DEFAULT_EXCLUDE_FOLDER_PATTERN       = L".git;.svn;.vs";

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
	CNativeW GetPackedGFileString() const;	//!< 除外ファイル、除外フォルダーの設定を "-GFILE=" の設定に pack する
	BOOL OnCbnDropDown( HWND hwndCtl, int wID ) override;
	int DoModal( HINSTANCE, HWND, const WCHAR* );	/* モーダルダイアログの表示 */
//	HWND DoModeless( HINSTANCE, HWND, const char* );	/* モードレスダイアログの表示 */

	//! 除外パターンの初期値を決定する
	// 履歴が空なら既定値を補い、次回以降に選べるよう履歴にも積む。
	void DetermineDefaultExcludePatterns();

	//! 検索ファイル/フォルダー/除外パターン/カレントファイルパスの既定値を履歴から補完する
	// CDlgGrep::DoModal / CDlgGrepReplace::DoModal 共通の初期化（2-E で重複解消）。
	void LoadDefaultsFromShareData(const WCHAR* pszCurrentFilePath);

	//! HWND ファイルトークンの接頭辞（":HWND:..." 形式の共通定義）
	static constexpr const wchar_t HWND_FILE_TOKEN_PREFIX[] = L":HWND:";

	//! HWND ファイルトークンの生成と判定（":HWND:..." 形式）
	// 「編集中のテキストから検索」時の擬似ファイル名として使う。
	static std::wstring BuildHwndFileToken(HWND hwnd);
	// 実ファイルパスと区別するため、接頭辞だけを見て判定する。
	static bool IsHwndFileToken(const wchar_t* s);

	bool		m_bEnableThisText;
	bool		m_bSelectOnceThisText;
	BOOL		m_bSubFolder;/*!< サブフォルダーからも検索する */
	BOOL		m_bFromThisText;/*!< この編集中のテキストから検索する */
	BOOL		m_bExcludeFileRegularExp = FALSE;	//!< 除外ファイルを正規表現として扱う

	SSearchOption	m_sSearchOption;	//!< 検索オプション

	ECodeType	m_nGrepCharSet;			/*!< 文字コードセット */
	int			m_nGrepOutputStyle;		/*!< Grep: 出力形式 */
	int			m_nGrepOutputLineType;		//!< 結果出力：行を出力/該当部分/否マッチ行
	bool		m_bGrepOutputFileOnly;		/*!< ファイル毎最初のみ検索 */
	bool		m_bGrepOutputBaseFolder;	/*!< ベースフォルダー表示 */
	bool		m_bGrepSeparateFolder;		/*!< フォルダー毎に表示 */

	std::wstring	m_strText;				/*!< 検索文字列 */
	bool			m_bSetText;				//!< 検索文字列を設定したか
	SFilePathLong	m_szFile;				//!< 検索ファイル
	SFilePathLong	m_szFolder;				//!< 検索フォルダー
	SFilePathLong	m_szExcludeFile;		//!< 除外ファイル
	SFilePathLong	m_szExcludeFolder;		//!< 除外フォルダー
	SFilePath	m_szCurrentFilePath;
protected:
	CRecentSearch			m_cRecentSearch;
	CRecentGrepFile			m_cRecentGrepFile;
	CRecentGrepFolder		m_cRecentGrepFolder;
	CRecentExcludeFile		m_cRecentExcludeFile;
	CRecentExcludeFolder	m_cRecentExcludeFolder;

	std::vector<CFontAutoDeleter>	m_cFontDeleters;

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
	// C-9: GetData 分解（Phase 3）
	void GetDataSimpleOptions( void );					//!< チェックボックス・ラジオ・コンボ等の単純フィールド取得
	BOOL GetDataFilePattern( bool bFromThisText );		//!< 検索ファイルパターンの取得・検証
	BOOL GetDataFolderPath( bool bFromThisText );		//!< 検索フォルダーの取得・複数パス解決・検証
	BOOL ValidateSearchText( void );					//!< 検索文字列の正規表現構文検証
	void CommitDataToShareData( bool bFromThisText );	//!< 検証通過後の共有設定・履歴への書き込み
	static LRESULT CALLBACK OnFolderProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
};
#endif /* SAKURA_CDLGGREP_01A0D5CB_326B_4C56_A527_C811F84FD8D8_H_ */
