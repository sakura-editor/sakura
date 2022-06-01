﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CBINARYSTREAM_856F71C1_27E0_4075_BA32_245D18E142BA_H_
#define SAKURA_CBINARYSTREAM_856F71C1_27E0_4075_BA32_245D18E142BA_H_
#pragma once

#include "CStream.h"

class CBinaryInputStream final : public CStream{
public:
	CBinaryInputStream(LPCWSTR pszFilePath);

public:
	//! ストリームの「残り」サイズを取得
	int GetLength();

	//! データを無変換で読み込む。戻り値は読み込んだバイト数。
	int Read(void* pBuffer, int nSizeInBytes);
};

class CBinaryOutputStream final : public COutputStream{
public:
	CBinaryOutputStream(LPCWSTR pszFilePath, bool bExceptionMode = false);
};
#endif /* SAKURA_CBINARYSTREAM_856F71C1_27E0_4075_BA32_245D18E142BA_H_ */
