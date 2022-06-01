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
#ifndef SAKURA_CFILE_53DA3C63_95C0_49D0_9ED1_1C0131493912_H_
#define SAKURA_CFILE_53DA3C63_95C0_49D0_9ED1_1C0131493912_H_
#pragma once

#include "basis/CMyString.h" //CFilePath
#include "util/file.h"

//!ファイルの排他制御モード  2007.10.11 kobake 作成
enum EShareMode{
	SHAREMODE_NOT_EXCLUSIVE,	//!< 排他制御しない
	SHAREMODE_DENY_WRITE,		//!< 他プロセスからの上書きを禁止
	SHAREMODE_DENY_READWRITE,	//!< 他プロセスからの読み書きを禁止
};

class CFile{
	using Me = CFile;

public:
	//コンストラクタ・デストラクタ
	CFile(LPCWSTR pszPath = NULL);
	CFile(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CFile(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CFile();
	//パス
	const CFilePath& GetFilePathClass() const { return m_szFilePath; }
	LPCWSTR GetFilePath() const { return m_szFilePath; }
	//設定
	void SetFilePath(LPCWSTR pszPath){ m_szFilePath.Assign(pszPath); }
	//各種判定
	bool IsFileExist() const;
	bool HasWritablePermission() const;
	bool IsFileWritable() const;
	bool IsFileReadable() const;
	//ロック
	bool FileLock(EShareMode eShareMode, bool bMsg);	//!< ファイルの排他ロック
	void FileUnlock();						//!< ファイルの排他ロック解除
	bool IsFileLocking() const{ return m_hLockedFile!=INVALID_HANDLE_VALUE; }
	EShareMode GetShareMode() const{ return m_nFileShareModeOld; }
	void SetShareMode(EShareMode eShareMode) { m_nFileShareModeOld = eShareMode; }
private:
	CFilePath	m_szFilePath;				//!< ファイルパス
	HANDLE		m_hLockedFile;				//!< ロックしているファイルのハンドル
	EShareMode	m_nFileShareModeOld;		//!< ファイルの排他制御モード
};

//!一時ファイル
class CTmpFile{
	using Me = CTmpFile;

public:
	CTmpFile(){ m_fp = tmpfile(); }
	CTmpFile(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CTmpFile(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CTmpFile(){ fclose(m_fp); }
	FILE* GetFilePointer() const{ return m_fp; }
private:
	FILE* m_fp;
};
#endif /* SAKURA_CFILE_53DA3C63_95C0_49D0_9ED1_1C0131493912_H_ */
