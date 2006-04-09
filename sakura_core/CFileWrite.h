/*!	@file CFileWrite.h
	@brief �t�@�C���������ݗp�N���X

	@author Frozen
	@date 2000/05/22 �V�K�쐬
*/
/*
	Copyright (C) 2000, Frozen

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#ifndef FILE_WRITE_H
#define FILE_WRITE_H


//! �t�@�C���̃I�[�v���Ɏ��s�����Ƃ��ɑ��o�����O

#ifndef DEF_CERROR_FILEOPEN

#define DEF_CERROR_FILEOPEN
class CError_FileOpen
{
};

#endif

//! �t�@�C���̏������݂Ɏ��s�����Ƃ��ɑ��o�����O
class CError_FileWrite
{
};

//! �t�@�C���������݃N���X�iC�̃��C�u�����֐����g�p���Ď����j
class CFileWrite
{
public:
	CFileWrite(const TCHAR* pszPath);
	~CFileWrite();
	inline void Write(const void* buffer,size_t size)
	{
		if(::fwrite(buffer,1,size,m_hFile)!=size)
			throw CError_FileWrite();
	}
private:
	FILE* m_hFile;
	DWORD m_dwFileAttribute;
	TCHAR* m_pszPath;
	bool m_ChangeAttribute;	
};

#endif // FILE_WRITE_H
