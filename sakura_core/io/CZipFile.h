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
#pragma once

#include <ShlDisp.h>

class CZipFile
{
  private:
    IShellDispatch *psd;
    Folder *pZipFile;
    std::wstring sZipName;

  public:
    CZipFile(); // コンストラクタ
    ~CZipFile(); // デストラクタ

  public:
    bool IsOk()
    {
        return (psd != NULL);
    } // Zip Folderが使用できるか?
    bool SetZip(const std::wstring &sZipPath); // Zip File名 設定
    bool ChkPluginDef(const std::wstring &sDefFile, std::wstring &sFolderName); // ZIP File 内 フォルダ名取得と定義ファイル検査(Plugin用)
    bool Unzip(const std::wstring sOutPath); // Zip File 解凍
};
