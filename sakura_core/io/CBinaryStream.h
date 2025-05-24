/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
