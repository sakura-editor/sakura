/*!	@file
	@brief ZIP file操作

*/
/*
	Copyright (C) 2011, Uchi
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include <shellapi.h>
#include "CZipFile.h"
#include "basis/CMyString.h"

// コンストラクタ
CZipFile::CZipFile() {
	HRESULT		hr;

	hr = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, reinterpret_cast<void **>(&psd));
	if (FAILED(hr)) {
		psd = NULL;
	}
	pZipFile = NULL;
}

// デストラクタ
CZipFile::~CZipFile() {
	if (pZipFile != NULL) {
		pZipFile->Release();
		pZipFile = NULL;
	}
	psd = NULL;
}

// Zip File名 設定
bool CZipFile::SetZip(const std::wstring& sZipPath)
{
	HRESULT			hr;
	VARIANT			var;

	if (pZipFile != NULL) {
		pZipFile->Release();
		pZipFile = NULL;
	}

	// ZIP Folder設定
	VariantInit(&var);
	var.vt = VT_BSTR;
	var.bstrVal = SysAllocString(sZipPath.c_str());
	hr = psd->NameSpace(var, &pZipFile);
	if (hr != S_OK) {
		pZipFile = NULL;
		return false;
	}

	sZipName = sZipPath;

	return true;
}

// ZIP File 内 フォルダー名取得と定義ファイル検査(Plugin用)
bool CZipFile::ChkPluginDef(const std::wstring& sDefFile, std::wstring& sFolderName)
{
	HRESULT			hr;
	VARIANT			vari;
	FolderItems*	pZipFileItems;
	long			lCount;
	bool			bFoundDef = false;

	sFolderName.clear();

	// ZIP File List
	hr = pZipFile->Items(&pZipFileItems);
	if (hr != S_OK) {
		pZipFile->Release();
		return false;
	}

	// 検査
	hr = pZipFileItems->get_Count(&lCount);
	VariantInit(&vari);
	vari.vt = VT_I4;
	for (vari.lVal = 0; vari.lVal < lCount; vari.lVal++) {
		BSTR			bps;
		VARIANT_BOOL	vFolder;
		FolderItem*		pFileItem;

		hr = pZipFileItems->Item(vari, &pFileItem);
		if (hr != S_OK) { continue; }
		hr = pFileItem->get_Name(&bps);
		if (hr != S_OK) { continue; }
		hr = pFileItem->get_IsFolder(&vFolder);
		if (hr != S_OK) { continue; }
		if (vFolder) {
			long			lCount2;
			VARIANT			varj;
			FolderItems*	pFileItems2;
			Folder*			pFile;

			sFolderName = bps;	// Install Follder Name
			hr = pFileItem->get_GetFolder((IDispatch **)&pFile);
			if (hr != S_OK) { continue; }
			hr = pFile->Items(&pFileItems2);
			if (hr != S_OK) { continue; }
			hr = pFileItems2->get_Count(&lCount2);
			if (hr != S_OK) { continue; }
			varj.vt = VT_I4;
			for (varj.lVal = 0; varj.lVal < lCount2; varj.lVal++) {
				hr = pFileItems2->Item(varj, &pFileItem);
				if (hr != S_OK) { continue; }
				hr = pFileItem->get_IsFolder(&vFolder);
				if (hr != S_OK) { continue; }
				hr = pFileItem->get_Path(&bps);
				if (hr != S_OK) { continue; }

				// 定義ファイルか
				if (!vFolder && wcslen(bps) >= sDefFile.length()
					&& (wmemicmp(bps, ((sFolderName + L"/" + sDefFile).c_str())) == 0
					|| wmemicmp(bps, ((sFolderName + L"\\" + sDefFile).c_str())) == 0
					|| wmemicmp(bps, ((sZipName + L"\\" + sFolderName + L"\\" + sDefFile).c_str())) == 0)) {
					bFoundDef = true;
					break;
				}
			}
			VariantClear(&varj);
			if (bFoundDef) {
				break;
			}
		}
	}
	VariantClear(&vari);

	pZipFileItems->Release();

	return bFoundDef;
}

// ZIP File 解凍
bool CZipFile::Unzip(const std::wstring sOutPath)
{
	HRESULT			hr;
	VARIANT			var;
	VARIANT			varOpt;
	Folder*			pOutFolder;
	FolderItems*	pZipFileItems;

	// ZIP File List
	hr = pZipFile->Items(&pZipFileItems);
	if (hr != S_OK) {
		pZipFile->Release();
		return false;
	}

	// 出力Folder設定
	VariantInit(&var);
	var.vt = VT_BSTR;
	var.bstrVal = SysAllocString(sOutPath.c_str());
	hr = psd->NameSpace(var, &pOutFolder);
	VariantClear(&var);
	if (hr != S_OK) {
		pZipFileItems->Release();
		pZipFile->Release();
		return false;
	}

	// 展開の設定
	VariantInit(&var);
	var.vt = VT_DISPATCH;
	var.pdispVal = pZipFileItems;
	VariantInit(&varOpt);
	varOpt.vt = VT_I4;
	varOpt.lVal = FOF_SILENT | FOF_NOCONFIRMATION;

	// 展開
	hr = pOutFolder->CopyHere(var, varOpt);

	pOutFolder->Release();
	pZipFileItems->Release();

	return (hr == S_OK);
}
