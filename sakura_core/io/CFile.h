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
public:
	//コンストラクタ・デストラクタ
	CFile(LPCTSTR pszPath = NULL);
	virtual ~CFile();
	//パス
	const CFilePath& GetFilePathClass() const { return m_szFilePath; }
	LPCTSTR GetFilePath() const { return m_szFilePath; }
	//設定
	void SetFilePath(LPCTSTR pszPath){ m_szFilePath.Assign(pszPath); }
	//各種判定
	bool IsFileExist() const;
	bool HasWritablePermission() const;
	bool IsFileWritable() const;
	bool IsFileReadable() const;
	//ロック
	bool FileLock(EShareMode eShareMode);	//!< ファイルの排他ロック
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
public:
	CTmpFile(){ m_fp = tmpfile(); }
	~CTmpFile(){ fclose(m_fp); }
	FILE* GetFilePointer() const{ return m_fp; }
private:
	FILE* m_fp;
};
