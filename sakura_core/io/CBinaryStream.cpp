﻿/*! @file */
#include "StdAfx.h"
#include "CBinaryStream.h"

CBinaryInputStream::CBinaryInputStream(LPCWSTR pszFilePath)
	: CStream(pszFilePath, L"rb")
{
}

//! ストリームの「残り」サイズを取得
int CBinaryInputStream::GetLength()
{
	long nCur = ftell(GetFp());
	fseek(GetFp(), 0, SEEK_END);
	long nDataLen = ftell(GetFp());
	fseek(GetFp(), nCur, SEEK_SET);
	return nDataLen;
}

//! データを無変換で読み込む。戻り値は読み込んだバイト数。
int CBinaryInputStream::Read(void *pBuffer, int nSizeInBytes) { return fread(pBuffer, 1, nSizeInBytes, GetFp()); }

CBinaryOutputStream::CBinaryOutputStream(LPCWSTR pszFilePath, bool bExceptionMode)
	: COutputStream(pszFilePath, L"wb", bExceptionMode)
{
}
