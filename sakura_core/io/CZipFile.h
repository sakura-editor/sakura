/*!	@file
	@brief ZIP file操作

*/
/*
	Copyright (C) 2011, Uchi
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CZIPFILE_EA7F9762_A67F_449D_B346_EAB3075A9E2C_H_
#define SAKURA_CZIPFILE_EA7F9762_A67F_449D_B346_EAB3075A9E2C_H_
#pragma once

#include <ShlDisp.h>

class CZipFile {
private:
	IShellDispatch*	psd;
	Folder*			pZipFile;
	std::wstring	sZipName;

	using Me = CZipFile;

public:
	CZipFile();		// コンストラクタ
	CZipFile(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CZipFile(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CZipFile();	// デストラクタ

public:
	bool	IsOk() { return (psd != NULL); }			// Zip Folderが使用できるか?
	bool	SetZip(const std::wstring& sZipPath);		// Zip File名 設定
	bool	ChkPluginDef(const std::wstring& sDefFile, std::wstring& sFolderName);	// ZIP File 内 フォルダー名取得と定義ファイル検査(Plugin用)
	bool	Unzip(const std::wstring sOutPath);			// Zip File 解凍
};
#endif /* SAKURA_CZIPFILE_EA7F9762_A67F_449D_B346_EAB3075A9E2C_H_ */
