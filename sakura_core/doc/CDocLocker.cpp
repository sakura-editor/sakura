/*! @file */
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
#include "CDocLocker.h"
#include "CDocFile.h"
#include "window/CEditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CDocLocker::CDocLocker()
: m_bIsDocWritable(true)
{
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ロード前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CDocLocker::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//書き込めるか検査
	CheckWritable(!sLoadInfo.bViewMode && !sLoadInfo.bWritableNoMsg);
	if( !m_bIsDocWritable ){
		return;
	}

	// ファイルの排他ロック
	pcDoc->m_cDocFileOperation.DoFileLock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        セーブ前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CDocLocker::OnBeforeSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// ファイルの排他ロック解除
	pcDoc->m_cDocFileOperation.DoFileUnlock();
}

void CDocLocker::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// 書き込めるか検査
	m_bIsDocWritable = true;

	// ファイルの排他ロック
	pcDoc->m_cDocFileOperation.DoFileLock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         チェック                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 書き込めるか検査
void CDocLocker::CheckWritable(bool bMsg)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// ファイルが存在しない場合 (「開く」で新しくファイルを作成した扱い) は、以下の処理は行わない
	if( !fexist(pcDoc->m_cDocFile.GetFilePath()) ){
		m_bIsDocWritable = true;
		return;
	}

	// 読み取り専用ファイルの場合は、以下の処理は行わない
	if( !pcDoc->m_cDocFile.HasWritablePermission() ){
		m_bIsDocWritable = false;
		return;
	}

	// 書き込めるか検査
	CDocFile& cDocFile = pcDoc->m_cDocFile;
	m_bIsDocWritable = cDocFile.IsFileWritable();
	if(!m_bIsDocWritable && bMsg){
		// 排他されている場合だけメッセージを出す
		// その他の原因（ファイルシステムのセキュリティ設定など）では読み取り専用と同様にメッセージを出さない
		if( ::GetLastError() == ERROR_SHARING_VIOLATION ){
			TopWarningMessage(
				CEditWnd::getInstance()->GetHwnd(),
				LS( STR_ERR_DLGEDITDOC21 ),	//"%s\nは現在他のプロセスによって書込みが禁止されています。"
				cDocFile.GetFilePathClass().IsValidPath() ? cDocFile.GetFilePath() : LS(STR_NO_TITLE1)	//"(無題)"
			);
		}
	}
}
