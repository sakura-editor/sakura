/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/ole/OleInit.hpp"

namespace cxx {

/*!
 * @brief COMライブラリのクリーンアップ
 */
/* static */ void OleInit::_CleanUp(
	const OleInit* pThis
) noexcept
{
	// 引数がNULLなら何もしない
	if (!pThis) return;

	// OLE初期化に失敗していたら何もしない
	if (!pThis->IsInitialized()) return;

	// OLEをシャットダウンする
	::OleUninitialize();
}

}	// namespace cxx

