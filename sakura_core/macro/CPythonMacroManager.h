/*!	@file
	@brief Python Macro Manager
*/
/*
	Copyright (C) 2018-2022 Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#pragma once

#include <Windows.h>
#include <string>
#include "macro/CMacroManagerBase.h"
class CEditView;

class CPythonMacroManager final : public CMacroManagerBase {
public:
	CPythonMacroManager();
	virtual ~CPythonMacroManager() = default;

	bool ExecKeyMacro(CEditView *EditView, int flags) const override;
	BOOL LoadKeyMacro(HINSTANCE hInstance, const WCHAR* pszPath) override;
	BOOL LoadKeyMacroStr(HINSTANCE hInstance, const WCHAR* pszCode) override;

	static CMacroManagerBase* Creator(const WCHAR* FileExt);
	static void declare();

protected:
	std::string m_strPath;
	std::wstring m_wstrPath;
	std::string m_strMacro;
};
