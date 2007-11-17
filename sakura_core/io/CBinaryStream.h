#pragma once

#include "CStream.h"

class CBinaryInputStream : public CStream{
public:
	CBinaryInputStream(LPCTSTR tszFilePath);

public:
	//! ストリームの「残り」サイズを取得
	int GetLength();

	//! データを無変換で読み込む。戻り値は読み込んだバイト数。
	int Read(void* pBuffer, int nSizeInBytes);
};

class CBinaryOutputStream : public CStream{
public:
	CBinaryOutputStream(LPCTSTR tszFilePath);

public:
	//! データを無変換で書き込む。戻り値は書き込んだバイト数。
	int Write(const void* pBuffer, int nSizeInBytes);
};

