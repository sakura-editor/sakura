/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "StdAfx.h"
#include "CBinaryStream.h"

CBinaryInputStream::CBinaryInputStream(LPCWSTR pszFilePath)
: CStream(pszFilePath,L"rb")
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
int CBinaryInputStream::Read(void* pBuffer, int nSizeInBytes)
{
	return fread(pBuffer,1,nSizeInBytes,GetFp());
}

CBinaryOutputStream::CBinaryOutputStream(LPCWSTR pszFilePath, bool bExceptionMode)
: COutputStream(pszFilePath,L"wb",bExceptionMode)
{
}
