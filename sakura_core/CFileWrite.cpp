/*! 
 * @file CFileWrite.cpp
 * @brief ファイル書き込み用クラス
 *
 * @author Frozen
 * @date 2000/05/22
 *
 */
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "CFileWrite.h"

CFileWrite::CFileWrite(const TCHAR* pszPath)
{
	m_ChangeAttribute=false;
	m_pszPath=NULL;
	m_dwFileAttribute = ::GetFileAttributes(pszPath);
	if ( m_dwFileAttribute != (DWORD)-1 )
	{//ファイルが存在した場合
		if(m_dwFileAttribute & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
		{//隠しorシステムファイルはCの関数で読み書きできないので属性を変更する
			// 読取専用属性だけ残す(ノーマル属性が付いていたらそれも残す)。
			::SetFileAttributes(pszPath, m_dwFileAttribute & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_NORMAL));
			m_ChangeAttribute=true;
			m_pszPath=::_tcsdup(pszPath);
		}
	}
	m_hFile=::_tfopen(pszPath,_T("wb"));
	if(m_hFile==NULL){
		// Mar. 30, 2003 genta
		// コンストラクタでthrowするとデストラクタのSetFileAttributeとfreeが
		// 呼ばれないので、ここで対処
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
//  ネットワーク上のファイルを扱っている場合、
//	書き込み後にFlushを行うとデットロックが発生することがあるので、
//	ここで::fflushを呼び出してはいけません。
//  詳細：http://www.microsoft.com/japan/support/faq/KBArticles2.asp?URL=/japan/support/kb/articles/jp288/7/94.asp

	::fclose(m_hFile);
	if(m_ChangeAttribute)
		::SetFileAttributes(m_pszPath,m_dwFileAttribute);
	::free(m_pszPath);
}
