/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_TCHAR_CONVERT_2F41312D_27C8_4366_89F5_046BF7ED3B32_H_
#define SAKURA_TCHAR_CONVERT_2F41312D_27C8_4366_89F5_046BF7ED3B32_H_
#pragma once

#include "basis/primitive.h"

//WCHARに変換
const WCHAR* to_wchar(const ACHAR* src);
const WCHAR* to_wchar(const ACHAR* pSrcData, int nSrcLength);

//ACHARに変換
inline
const ACHAR* to_achar(const ACHAR* src){ return src; }
const ACHAR* to_achar(const WCHAR* src);
const ACHAR* to_achar(const WCHAR* pSrc, int nSrcLength);

#endif /* SAKURA_TCHAR_CONVERT_2F41312D_27C8_4366_89F5_046BF7ED3B32_H_ */
