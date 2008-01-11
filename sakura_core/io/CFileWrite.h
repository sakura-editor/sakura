/*!	@file CFileWrite.h
	@brief ファイル書き込み用クラス

	@author Frozen
	@date 2000/05/22 新規作成
*/
/*
	Copyright (C) 2000, Frozen

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#ifndef FILE_WRITE_H
#define FILE_WRITE_H


//! ファイルのオープンに失敗したときに送出する例外

#ifndef DEF_CERROR_FILEOPEN

#define DEF_CERROR_FILEOPEN
class CError_FileOpen
{
};

#endif

//! ファイルの書き込みに失敗したときに送出する例外
class CError_FileWrite
{
};

//! ファイル書き込みクラス（Cのライブラリ関数を使用して実装）
// 2007.08.23 kobake デストラクタと同じ処理をClose関数からも呼べるようにしました。Close後のWriteは禁止です。
class CFileWrite
{
public:
	CFileWrite(const TCHAR* pszPath);
	~CFileWrite();
	void Close();
	inline void Write(const void* buffer,size_t size)
	{
		assert(m_hFile);
		if(::fwrite(buffer,1,size,m_hFile)!=size)
			throw CError_FileWrite();
	}
	//※終端'\0'は出力しない
	inline void WriteSz(const char* szData)
	{
		Write(szData,strlen(szData));
	}
private:
	FILE* m_hFile;
	DWORD m_dwFileAttribute;
	TCHAR* m_pszPath;
	bool m_ChangeAttribute;	
};

#endif // FILE_WRITE_H
