/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CFILE_4C6F2C6F_75E4_470C_8AB0_0A8567BF032E9_H_
#define SAKURA_CFILE_4C6F2C6F_75E4_470C_8AB0_0A8567BF032E9_H_

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

#endif /* SAKURA_CFILE_4C6F2C6F_75E4_470C_8AB0_0A8567BF032E9_H_ */
/*[EOF]*/
