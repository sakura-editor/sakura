/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/design_template.h"

#include <Windows.h>
#include <ole2.h>

 //! Ole32.dll呼出をテスト可能にするDIっぽいもの
struct Ole32 : public TSakuraSingleton<Ole32>
{
	using Me = Ole32;

	~Ole32() override = default;

	virtual HRESULT DoDragDrop(
		IN LPDATAOBJECT pDataObj,
		IN LPDROPSOURCE pDropSource,
		IN DWORD dwOKEffects,
		OUT LPDWORD pdwEffect
	) const;
};
