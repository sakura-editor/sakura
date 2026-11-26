/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/os/Ole32.hpp"

HRESULT Ole32::DoDragDrop(
	IN LPDATAOBJECT pDataObj,
	IN LPDROPSOURCE pDropSource,
	IN DWORD dwOKEffects,
	OUT LPDWORD pdwEffect
) const
{
	return ::DoDragDrop(
		pDataObj,
		pDropSource,
		dwOKEffects,
		pdwEffect
	);
}
