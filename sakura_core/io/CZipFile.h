/*!	@file
	@brief ZIP file操作

*/
/*
	Copyright (C) 2011, Uchi
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CZIPFILE_EA7F9762_A67F_449D_B346_EAB3075A9E2C_H_
#define SAKURA_CZIPFILE_EA7F9762_A67F_449D_B346_EAB3075A9E2C_H_
#pragma once

#include "cxx/com_pointer.hpp"

class CZipFile {
private:
	using IShellDispatchPtr = cxx::com_pointer<IShellDispatch>;
	using FolderPtr = cxx::com_pointer<Folder>;

	using Me = CZipFile;

public:
	CZipFile();		// コンストラクタ
	CZipFile(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CZipFile(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CZipFile();	// デストラクタ

	bool	IsOk() const noexcept { return (m_pShellDispatch != nullptr); }			// Zip Folderが使用できるか?
	bool	SetZip(const std::filesystem::path& zipPath);		// Zip File名 設定
	bool	Unzip(const std::filesystem::path& outDir);			// Zip File 解凍
	bool	ChkPluginDef(std::wstring_view defFileName, std::wstring& sFolderName);	// ZIP File 内 フォルダー名取得と定義ファイル検査(Plugin用)

private:
	IShellDispatchPtr	m_pShellDispatch = nullptr;
	FolderPtr			m_pZipFolder = nullptr;
	std::wstring		m_ZipPath;
};

#endif /* SAKURA_CZIPFILE_EA7F9762_A67F_449D_B346_EAB3075A9E2C_H_ */
