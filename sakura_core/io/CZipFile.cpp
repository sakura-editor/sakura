/*!	@file
	@brief ZIP file����

*/
/*
	Copyright (C) 2011, Uchi

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "CZipFile.h"

bool CZipFile::SetZip(const std::tstring sZipPath)
{
	HRESULT			hr;
	VARIANT			var;

	if (pZipFile != NULL) {
		pZipFile->Release();
		pZipFile = NULL;
	}

	// ZIP Folder�ݒ�
	VariantInit(&var);
	var.vt = VT_BSTR;
	var.bstrVal = SysAllocString(to_wchar(sZipPath.c_str()));
	hr = psd->NameSpace(var, &pZipFile);
	if (hr != S_OK) {
		pZipFile = NULL;
		return false;
	}
	return true;
}



// ZIP File �� �t�H���_���擾�ƒ�`�t�@�C������(Plugin�p)
bool CZipFile::ChkPluginDef(const std::tstring sDefFile, std::tstring& sFolderName)
{
	HRESULT			hr;
	VARIANT			vari;
	FolderItems*	pZipFileItems;
	long			lCount;
	bool			bFoundDef = false;

	sFolderName = _T("");

	// ZIP File List
	hr = pZipFile->Items(&pZipFileItems);
	if (hr != S_OK) {
		pZipFile->Release();
		return false;
	}

	// ����
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

			sFolderName = to_tchar(bps);	// Install Follder Name
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

				// ��`�t�@�C����
				if (!vFolder && auto_strlen(bps) >= sDefFile.length()
					&& (auto_stricmp(to_tchar(bps), to_tchar((sFolderName + _T("/") + sDefFile).c_str())) == 0
					|| auto_stricmp(to_tchar(bps), to_tchar((sFolderName + _T("\\") + sDefFile).c_str())) == 0)) {
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



// ZIP File ��
bool CZipFile::Unzip(const std::tstring sOutPath)
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

	// �o��Folder�ݒ�
	VariantInit(&var);
	var.vt = VT_BSTR;
	var.bstrVal = SysAllocString(to_wchar(sOutPath.c_str()));
	hr = psd->NameSpace(var, &pOutFolder);
	VariantClear(&var);
	if (hr != S_OK) {
		pZipFileItems->Release();
		pZipFile->Release();
		return false;
	}

	// �W�J�̐ݒ�
	VariantInit(&var);
	var.vt = VT_DISPATCH;
	var.pdispVal = pZipFileItems;
	VariantInit(&varOpt);
	varOpt.vt = VT_I4;
	varOpt.lVal = FOF_SILENT | FOF_NOCONFIRMATION;

	// �W�J
	hr = pOutFolder->CopyHere(var, varOpt);

	pOutFolder->Release();
	pZipFileItems->Release();

	return (hr == S_OK);
}
