﻿/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CStream.h"
#include <string>
#include "util/std_macro.h"

//	::fflush(m_hFile);
//  ネットワーク上のファイルを扱っている場合、
//	書き込み後にFlushを行うとデットロックが発生することがあるので、
//	Close時に::fflushを呼び出してはいけません。
//  詳細：http://www.microsoft.com/japan/support/faq/KBArticles2.asp?URL=/japan/support/kb/articles/jp288/7/94.asp

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  ファイル属性操作クラス                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CFileAttribute{
public:
	CFileAttribute(const WCHAR* pszPath)
	: m_strPath(pszPath)
	, m_bAttributeChanged(false)
	, m_dwAttribute(0)
	{
	}

	//! 指定属性を取り除く
	void PopAttribute(DWORD dwPopAttribute)
	{
		if(m_bAttributeChanged)return; //既に取り除き済み

		m_dwAttribute = ::GetFileAttributes(m_strPath.c_str());
		if( m_dwAttribute != (DWORD)-1 ){
			if(m_dwAttribute & dwPopAttribute){
				DWORD dwNewAttribute = m_dwAttribute & ~dwPopAttribute;
				::SetFileAttributes(m_strPath.c_str(), dwNewAttribute);
				m_bAttributeChanged=true;
			}
		}
	}
	
	//! 属性を元に戻す
	void RestoreAttribute()
	{
		if(m_bAttributeChanged)
			::SetFileAttributes(m_strPath.c_str(),m_dwAttribute);
		m_bAttributeChanged = false;
		m_dwAttribute = 0;
	}
private:
	std::wstring	m_strPath;
	bool			m_bAttributeChanged;
	DWORD			m_dwAttribute;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CStream::CStream(const WCHAR* pszPath, const WCHAR* pszMode, bool bExceptionMode)
{
	m_fp = nullptr;
	m_pcFileAttribute = nullptr;
	m_bExceptionMode = bExceptionMode;
	Open(pszPath,pszMode);
}

/*
CStream::CStream()
{
	m_fp = NULL;
	m_pcFileAttribute = NULL;
	m_bExceptionMode = false;
}
*/

CStream::~CStream()
{
	Close();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    オープン・クローズ                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
void CStream::Open(const WCHAR* pszPath, const WCHAR* pszMode)
{
	Close(); //既に開いていたら、一度閉じる

	//属性変更：隠しorシステムファイルはCの関数で読み書きできないので属性を変更する
	m_pcFileAttribute = new CFileAttribute(pszPath);
	m_pcFileAttribute->PopAttribute(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

	//オープン
	m_fp = _wfopen(pszPath,pszMode);
	if(!m_fp){
		Close(); //属性復元
	}

	//エラー処理
	if(!m_fp && IsExceptionMode()){
		throw CError_FileOpen();
	}
}

void CStream::Close()
{
	//クローズ
	if(m_fp){
		fclose(m_fp);
		m_fp=nullptr;
	}

	//属性復元
	if(m_pcFileAttribute){
		m_pcFileAttribute->RestoreAttribute();
		SAFE_DELETE(m_pcFileAttribute);
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           操作                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CStream::SeekSet(	//!< シーク
	long offset	//!< ストリーム先頭からのオフセット 
)
{
	fseek(m_fp,offset,SEEK_SET);
}

void CStream::SeekEnd(   //!< シーク
	long offset //!< ストリーム終端からのオフセット
)
{
	fseek(m_fp,offset,SEEK_END);
}
