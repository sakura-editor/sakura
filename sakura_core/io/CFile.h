#pragma once

#include "basis/CMyString.h" //CFilePath
#include "util/file.h"

//!�t�@�C���̔r�����䃂�[�h  2007.10.11 kobake �쐬
enum EShareMode{
	SHAREMODE_NOT_EXCLUSIVE,	//!< �r�����䂵�Ȃ�
	SHAREMODE_DENY_WRITE,		//!< ���v���Z�X����̏㏑�����֎~
	SHAREMODE_DENY_READWRITE,	//!< ���v���Z�X����̓ǂݏ������֎~
};

class CFile{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CFile(LPCTSTR pszPath = NULL);
	virtual ~CFile();
	//�p�X
	const CFilePath& GetFilePathClass() const { return m_szFilePath; }
	LPCTSTR GetFilePath() const { return m_szFilePath; }
	//�ݒ�
	void SetFilePath(LPCTSTR pszPath){ m_szFilePath.Assign(pszPath); }
	//�e�픻��
	bool IsFileExist() const;
	bool HasWritablePermission() const;
	bool IsFileWritable() const;
	bool IsFileReadable() const;
	//���b�N
	bool FileLock(EShareMode eShareMode, bool bMsg);	//!< �t�@�C���̔r�����b�N
	void FileUnlock();						//!< �t�@�C���̔r�����b�N����
	bool IsFileLocking() const{ return m_hLockedFile!=INVALID_HANDLE_VALUE; }
	EShareMode GetShareMode() const{ return m_nFileShareModeOld; }
	void SetShareMode(EShareMode eShareMode) { m_nFileShareModeOld = eShareMode; }
private:
	CFilePath	m_szFilePath;				//!< �t�@�C���p�X
	HANDLE		m_hLockedFile;				//!< ���b�N���Ă���t�@�C���̃n���h��
	EShareMode	m_nFileShareModeOld;		//!< �t�@�C���̔r�����䃂�[�h
};


//!�ꎞ�t�@�C��
class CTmpFile{
public:
	CTmpFile(){ m_fp = tmpfile(); }
	~CTmpFile(){ fclose(m_fp); }
	FILE* GetFilePointer() const{ return m_fp; }
private:
	FILE* m_fp;
};
