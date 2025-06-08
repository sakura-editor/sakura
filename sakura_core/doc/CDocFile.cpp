/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2013, Uchi
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CDocFile.h"

/*
	保存時のファイルのパス（マクロ用）の取得

	2017/5/17 CFile.hから移動
*/
const WCHAR* CDocFile::GetSaveFilePath(void) const
{
	if (m_szSaveFilePath.IsValidPath()) {
		return m_szSaveFilePath;
	}
	else {
		return GetFilePath();
	}
}
