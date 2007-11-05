#include "stdafx.h"
#include "CBinaryStream.h"


CBinaryInputStream::CBinaryInputStream(LPCTSTR tszFilePath)
: CStream(tszFilePath,_T("rb"))
{
}

//! ストリームの「残り」サイズを取得
int CBinaryInputStream::GetLength()
{
	long nCur = ftell(m_fp);
	fseek(m_fp, 0, SEEK_END);
	long nDataLen = ftell(m_fp);
	fseek(m_fp, nCur, SEEK_SET);
	return nDataLen;
}

//! データを無変換で読み込む。戻り値は読み込んだバイト数。
int CBinaryInputStream::Read(void* pBuffer, int nSizeInBytes)
{
	return fread(pBuffer,1,nSizeInBytes,m_fp);
}



CBinaryOutputStream::CBinaryOutputStream(LPCTSTR tszFilePath)
: CStream(tszFilePath,_T("wb"))
{
}

//! データを無変換で書き込む。戻り値は書き込んだバイト数。
int CBinaryOutputStream::Write(const void* pBuffer, int nSizeInBytes)
{
	return fwrite(pBuffer,1,nSizeInBytes,m_fp);
}

