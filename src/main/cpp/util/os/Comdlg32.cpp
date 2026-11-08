/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "util/os/Comdlg32.hpp"

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>

BOOL Comdlg32::ChooseColorW(
	LPCHOOSECOLORW pCc
) const
{
	return ::ChooseColorW(pCc);
}

BOOL Comdlg32::ChooseFontW(
	LPCHOOSEFONTW pCf
) const
{
	return ::ChooseFontW(
		pCf
	);
}

DWORD Comdlg32::CommDlgExtendedError() const
{
	return ::CommDlgExtendedError();
}

BOOL Comdlg32::GetOpenFileNameW(
	LPOPENFILENAMEW pOfn
) const
{
	return ::GetOpenFileNameW(pOfn);
}

BOOL Comdlg32::GetSaveFileNameW(
	LPOPENFILENAMEW pOfn
) const
{
	return ::GetSaveFileNameW(pOfn);
}

BOOL Comdlg32::PrintDlgW(
	_Inout_ LPPRINTDLGW pPD
) const
{
	return ::PrintDlgW(pPD);
}

