/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/design_template.h"

#include <Windows.h>
#include <commdlg.h>

//! Comdlg32.dll呼出をテスト可能にするDIっぽいもの
struct Comdlg32 : public TSakuraSingleton<Comdlg32> {
	~Comdlg32() override = default;

	virtual BOOL ChooseColorW(
		LPCHOOSECOLORW pCc
	) const;

	virtual BOOL ChooseFontW(
		LPCHOOSEFONTW pCf
	) const;

	virtual DWORD CommDlgExtendedError() const;

	virtual BOOL GetOpenFileNameW(
		LPOPENFILENAMEW pOfn
	) const;

	virtual BOOL GetSaveFileNameW(
		LPOPENFILENAMEW pOfn
	) const;

	virtual BOOL PrintDlgW(
		_Inout_ LPPRINTDLGW pPD
	) const;
};

