/*!	@file
	@brief Python Macro Manager
*/
/*
	Copyright (C) 2018-2026 Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#pragma once

#include "cxx/ResourceHolder.hpp"
#include "macro/CMacroManagerBase.h"

class CPythonMacroManager final : public CMacroManagerBase {
private:
	using DllHolder = cxx::ResourceHolder<&::FreeLibrary>;

public:
	static CMacroManagerBase* Creator(const WCHAR* FileExt);
	static void declare();

	static inline DllHolder s_hModule = nullptr;

	CPythonMacroManager();
	~CPythonMacroManager() override = default;

	bool ExecKeyMacro(CEditView *EditView, int flags) const override;
	BOOL LoadKeyMacro(HINSTANCE hInstance, const WCHAR* pszPath) override;
	BOOL LoadKeyMacroStr(HINSTANCE hInstance, const WCHAR* pszCode) override;

private:
	std::string m_strPath;
	std::wstring m_wstrPath;
	std::string m_strMacro;
};
