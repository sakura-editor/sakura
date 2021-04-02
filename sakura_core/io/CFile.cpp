﻿/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "StdAfx.h"
#include "io/CFile.h"
#include "window/CEditWnd.h" // 変更予定
#include <io.h>
#include "CSelectLang.h"
#include "String_define.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CFile::CFile(LPCWSTR pszPath)
: m_hLockedFile( INVALID_HANDLE_VALUE )
, m_nFileShareModeOld( SHAREMODE_NOT_EXCLUSIVE )
{
	if(pszPath){
		SetFilePath(pszPath);
	}
}

CFile::~CFile()
{
	FileUnlock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         各種判定                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFile::IsFileExist() const
{
	return fexist(GetFilePath());
}

bool CFile::HasWritablePermission() const
{
	return -1 != _waccess( GetFilePath(), 2 );
}

bool CFile::IsFileWritable() const
{
	//書き込めるか検査
	// Note. 他のプロセスが明示的に書き込み禁止しているかどうか
	//       ⇒ GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE でチェックする
	//          実際のファイル保存もこれと等価な _wfopen の L"wb" を使用している
	HANDLE hFile = CreateFile(
		this->GetFilePath(),			//ファイル名
		GENERIC_WRITE,					//書きモード
		FILE_SHARE_READ | FILE_SHARE_WRITE,	//読み書き共有
		NULL,							//既定のセキュリティ記述子
		OPEN_EXISTING,					//ファイルが存在しなければ失敗
		FILE_ATTRIBUTE_NORMAL,			//特に属性は指定しない
		NULL							//テンプレート無し
	);
	if(hFile==INVALID_HANDLE_VALUE){
		return false;
	}
	CloseHandle(hFile);
	return true;
}

bool CFile::IsFileReadable() const
{
	HANDLE hTest = CreateFile(
		this->GetFilePath(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
	if(hTest==INVALID_HANDLE_VALUE){
		// 読み込みアクセス権がない
		return false;
	}
	CloseHandle( hTest );
	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ロック                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ファイルの排他ロック解除
void CFile::FileUnlock()
{
	//クローズ
	if( m_hLockedFile != INVALID_HANDLE_VALUE ){
		::CloseHandle( m_hLockedFile );
		m_hLockedFile = INVALID_HANDLE_VALUE;
	}
}

//! ファイルの排他ロック
bool CFile::FileLock( EShareMode eShareMode, bool bMsg )
{
	// ロック解除
	FileUnlock();

	// ファイルの存在チェック
	if( !this->IsFileExist() ){
		return false;
	}

	// モード設定
	if(eShareMode==SHAREMODE_NOT_EXCLUSIVE)return true;
	
	//フラグ
	DWORD dwShareMode=0;
	switch(eShareMode){
	case SHAREMODE_NOT_EXCLUSIVE:	return true;										break; //排他制御無し
	case SHAREMODE_DENY_READWRITE:	dwShareMode = 0;									break; //読み書き禁止→共有無し
	case SHAREMODE_DENY_WRITE:		dwShareMode = FILE_SHARE_READ;						break; //書き込み禁止→読み込みのみ認める
	default:						dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;	break; //禁止事項なし→読み書き共に認める
	}

	//オープン
	m_hLockedFile = CreateFile(
		this->GetFilePath(),			//ファイル名
		GENERIC_READ,					//読み書きタイプ
		dwShareMode,					//共有モード
		NULL,							//既定のセキュリティ記述子
		OPEN_EXISTING,					//ファイルが存在しなければ失敗
		FILE_ATTRIBUTE_NORMAL,			//特に属性は指定しない
		NULL							//テンプレート無し
	);

	//結果
	if( INVALID_HANDLE_VALUE == m_hLockedFile && bMsg ){
		const WCHAR*	pszMode;
		switch( eShareMode ){
		case SHAREMODE_DENY_READWRITE:	pszMode = LS(STR_EXCLU_DENY_READWRITE); break;
		case SHAREMODE_DENY_WRITE:		pszMode = LS(STR_EXCLU_DENY_WRITE); break;
		default:						pszMode = LS(STR_EXCLU_UNDEFINED); break;
		}
		TopWarningMessage(
			CEditWnd::getInstance()->GetHwnd(),
			LS(STR_FILE_LOCK_ERR),
			GetFilePathClass().IsValidPath() ? GetFilePath() : LS(STR_NO_TITLE1),
			pszMode
		);
		return false;
	}

	return true;
}
