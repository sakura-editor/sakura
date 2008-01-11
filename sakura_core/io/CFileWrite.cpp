/*!	@file
	@brief �t�@�C���������ݗp�N���X

	@author Frozen
	@date 2000/05/22 �V�K�쐬
*/
/*
	Copyright (C) 2000, Frozen
	Copyright (C) 2003, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "io/CFileWrite.h"

CFileWrite::CFileWrite(const TCHAR* pszPath)
{
	m_ChangeAttribute=false;
	m_pszPath=NULL;
	m_dwFileAttribute = ::GetFileAttributes(pszPath);
	if ( m_dwFileAttribute != (DWORD)-1 )
	{//�t�@�C�������݂����ꍇ
		if(m_dwFileAttribute & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
		{//�B��or�V�X�e���t�@�C����C�̊֐��œǂݏ����ł��Ȃ��̂ő�����ύX����
			// �ǎ��p���������c��(�m�[�}���������t���Ă����炻����c��)�B
			::SetFileAttributes(pszPath, m_dwFileAttribute & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_NORMAL));
			m_ChangeAttribute=true;
			m_pszPath=::_tcsdup(pszPath);
		}
	}
	m_hFile=::_tfopen(pszPath,_T("wb"));
	if(m_hFile==NULL){
		// Mar. 30, 2003 genta
		// �R���X�g���N�^��throw����ƃf�X�g���N�^��SetFileAttribute��free��
		// �Ă΂�Ȃ��̂ŁA�����őΏ�
		if(m_ChangeAttribute)
			::SetFileAttributes(m_pszPath,m_dwFileAttribute);
		if(m_pszPath)
			::free(m_pszPath);
		throw CError_FileOpen();
	}
}
CFileWrite::~CFileWrite()
{
//	::fflush(m_hFile);
//  �l�b�g���[�N��̃t�@�C���������Ă���ꍇ�A
//	�������݌��Flush���s���ƃf�b�g���b�N���������邱�Ƃ�����̂ŁA
//	������::fflush���Ăяo���Ă͂����܂���B
//  �ڍׁFhttp://www.microsoft.com/japan/support/faq/KBArticles2.asp?URL=/japan/support/kb/articles/jp288/7/94.asp

	if(m_hFile){
		Close();
	}
}

void CFileWrite::Close()
{
	if(m_hFile){
		::fclose(m_hFile);
		if(m_ChangeAttribute)
			::SetFileAttributes(m_pszPath,m_dwFileAttribute);
		::free(m_pszPath);
	}
	else{
		//2�x�ڂ�Close�֎~
		assert(0);
	}
}
