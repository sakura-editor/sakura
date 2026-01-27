/*!	@file
	@brief ZIP file操作

*/
/*
	Copyright (C) 2011, Uchi
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "io/CZipFile.h"

// コンストラクタ
CZipFile::CZipFile() {
	if (const auto hr = m_pShellDispatch.CreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER); FAILED(hr)) {
		m_pShellDispatch = nullptr;
	}
}

// デストラクタ
CZipFile::~CZipFile() = default;

// Zip File名 設定
bool CZipFile::SetZip(const std::filesystem::path& zipPath)
{
	m_pZipFolder = nullptr;

	// ZIP Folder設定
	_variant_t var(zipPath.c_str());
	if (const auto hr = m_pShellDispatch->NameSpace(var, &m_pZipFolder); hr != S_OK) {
		m_pZipFolder = nullptr;
		return false;
	}

	m_ZipPath = zipPath;

	return true;
}

// ZIP File 内 フォルダー名取得と定義ファイル検査(Plugin用)
bool CZipFile::ChkPluginDef(std::wstring_view sDefFile, std::wstring& sFolderName)
{
	sFolderName.clear();

	if (!m_pZipFolder) {
		return false;
	}

	// ZIP File List
	cxx::com_pointer<FolderItems> pZipFileItems = nullptr;
	if (const auto hr = m_pZipFolder->Items(&pZipFileItems); FAILED(hr)) {
		m_pZipFolder = nullptr;
		return false;
	}

	// 検査
	long lCount = 0;
	if (const auto hr = pZipFileItems->get_Count(&lCount); hr != S_OK) {
		return false;
	}

	for (_variant_t vari(long(0), VT_I4); vari.lVal <= lCount; ++vari.lVal) {
		cxx::com_pointer<FolderItem> pFileItem = nullptr;
		_bstr_t buffer;
		VARIANT_BOOL isFolder;
		cxx::com_pointer<FolderItems> pFileItems2 = nullptr;
		cxx::com_pointer<Folder> pFile = nullptr;
		long lCount2 = 0;

		if (FAILED(pZipFileItems->Item(vari, &pFileItem)) ||
			FAILED(pFileItem->get_Name(buffer.GetAddress())) ||
			FAILED(pFileItem->get_IsFolder(&isFolder)) ||
			!isFolder ||
			FAILED(pFileItem->get_GetFolder((IDispatch**)&pFile)) ||
			FAILED(pFile->Items(&pFileItems2)) ||
			FAILED(pFileItems2->get_Count(&lCount2)))
		{
			continue;
		}

		sFolderName = buffer;

		for (_variant_t varj(long(0), VT_I4); varj.lVal < lCount2; ++varj.lVal) {
			if (FAILED(pFileItems2->Item(varj, &pFileItem)) ||
				FAILED(pFileItem->get_IsFolder(&isFolder)) ||
				isFolder ||
				FAILED(pFileItem->get_Name(buffer.GetAddress())))
			{
				continue;
			}

			// 定義ファイルか
			if (0 == wmemicmp(buffer, std::data(sDefFile))) {
				return true;
			}
		}
	}

	return false;
}

// ZIP File 解凍
bool CZipFile::Unzip(const std::filesystem::path& outDir)
{
	if (!m_pZipFolder) {
		return false;
	}

	// ZIP File List
	cxx::com_pointer<FolderItems> pZipFileItems = nullptr;
	if (const auto hr = m_pZipFolder->Items(&pZipFileItems); FAILED(hr)) {
		m_pZipFolder = nullptr;
		return false;
	}

	// 出力Folder設定
	cxx::com_pointer<Folder> pOutFolder = nullptr;
	_variant_t var(outDir.c_str());
	if (const auto hr = m_pShellDispatch->NameSpace(var, &pOutFolder); hr != S_OK) {
		return false;
	}

	// 展開の設定
	var = _variant_t(LPDISPATCH(pZipFileItems), true);
	_variant_t varOpt(long(FOF_SILENT | FOF_NOCONFIRMATION), VT_I4);

	// 展開
	return SUCCEEDED(pOutFolder->CopyHere(var, varOpt));
}
