﻿/*!	@file
	@brief Python Macro Manager
*/
/*
	Copyright (C) 2018-2022 Sakura Editor Organization

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
