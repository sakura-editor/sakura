/*!	@file
	@brief プロファイルマネージャ

	@author Moca
	@date 2013.12.31
*/
/*
	Copyright (C) 2013, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDLGPROFILEMGR_E77A329C_4D06_436A_84E3_01B4D8F34A9A_H_
#define SAKURA_CDLGPROFILEMGR_E77A329C_4D06_436A_84E3_01B4D8F34A9A_H_
#pragma once

#include "basis/CMyString.h"
#include "dlg/CDialog.h"
#include "util/StaticType.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

class CCommandLine;

/*!
 * @brief プロファイル名を格納するバッファ型
 *
 * @note ディレクトリ名として使用するため、ディレクトリ名の最大長を使用する
 * @note ディレクトリ名として使用するため、大文字小文字を区別せず比較する
 */
using SProfileName = StaticString<_MAX_DIR, false>;

struct SProfileSettings
{
	SFilePath m_szDllLanguage{};
	int	m_nDefaultIndex = -1;
	std::vector<std::wstring> m_vProfList{};
	bool m_bDefaultSelect = false;
};

class CDlgProfileMgr final : public CDialog
{
private:
	using Base = CDialog;
	using Me = CDlgProfileMgr;

	// 親クラスのDoModalを隠す
	using Base::DoModal;

public:
	//! コマンドラインだけでプロファイルが確定するか調べる
	static bool TrySelectProfile( CCommandLine* pcCommandLine ) noexcept;

	/*
	||  Constructors
	*/
	CDlgProfileMgr();

	/*
	||  Attributes & Operations
	*/
	int DoModal(
		HWND hWndParent,
		SProfileName& profileName
	);

	BOOL	OnBnClicked(int wID) override;
	INT_PTR	DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam ) override;

	void	SetData() override;	/* ダイアログデータの設定 */
	void	SetData(int nSelIndex);	/* ダイアログデータの設定 */
	int		GetData() override;	/* ダイアログデータの取得 */
	int		GetData(bool bStart);	/* ダイアログデータの取得 */
	LPVOID	GetHelpIdTable(void) override;

	void	UpdateIni();
	void	CreateProf();
	void	DeleteProf();
	void	RenameProf();
	void	SetDefaultProf(int index);
	void	ClearDefaultProf();

	SProfileName m_ProfileName{};

	static bool ReadProfSettings(SProfileSettings& settings);
	static bool WriteProfSettings(SProfileSettings& settings);
};

std::filesystem::path GetProfileMgrFileName();
std::filesystem::path GetProfileDirectory(std::wstring_view name);

[[nodiscard]] std::wstring GetProfileMgrFileName(std::wstring_view name);

#endif /* SAKURA_CDLGPROFILEMGR_E77A329C_4D06_436A_84E3_01B4D8F34A9A_H_ */
