/*!	@file
	@brief ZIP file操作

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
#ifndef SAKURA_CZIPFILE_5D06C90F_5043_418e_BA31_FB599CF6FD03_H_
#define SAKURA_CZIPFILE_5D06C90F_5043_418e_BA31_FB599CF6FD03_H_

#include <ShlDisp.h>

class CZipFile {
private:
	IShellDispatch*	psd;
	Folder*			pZipFile;

public:
	// コンストラクタ
	CZipFile() {
		HRESULT		hr;

		hr = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, reinterpret_cast<void **>(&psd));
		if (FAILED(hr)) {
			psd = NULL;
		}
		pZipFile = NULL;
	}
	// デストラクタ
	~CZipFile() {
		if (pZipFile != NULL) {
			pZipFile->Release();
			pZipFile = NULL;
		}
		psd = NULL;
	}

public:
	bool	IsOk() { return (psd != NULL); }
	bool	SetZip(const std::tstring sZipPath);
	bool	ChkPluginDef(const std::tstring sDefFile, std::tstring& sFolderName);
	bool	Unzip(const std::tstring sOutPath);
};
#endif	// SAKURA_CZIPFILE_5D06C90F_5043_418e_BA31_FB599CF6FD03_H_
