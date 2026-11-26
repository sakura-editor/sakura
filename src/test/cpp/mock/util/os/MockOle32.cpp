/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "util/os/MockOle32.hpp"

/* static */ HRESULT MockOle32::_DoDragDrop(
	IN LPDATAOBJECT pDataObj,
	IN LPDROPSOURCE pDropSource,
	IN DWORD dwOKEffects,
	OUT LPDWORD pdwEffect
)
{
	return ::DoDragDrop(
		pDataObj,
		pDropSource,
		dwOKEffects,
		pdwEffect
	);
}

MockOle32::MockOle32()
{
	// デフォルトの動作を設定する
	ON_CALL(*this, DoDragDrop(_, _, _, _)).WillByDefault(&_DoDragDrop);
}
