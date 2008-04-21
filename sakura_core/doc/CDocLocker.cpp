#include "stdafx.h"
#include "CDocLocker.h"



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
	CheckWritable(true);

	// ファイルの排他ロック
	if(pcDoc->m_cDocFileOperation._ToDoLock())
		pcDoc->m_cDocFile.DoFileLock(GetDllShareData().m_Common.m_sFile.m_nFileShareMode);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        セーブ前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CDocLocker::OnBeforeSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// ファイルの排他ロック解除
	pcDoc->m_cDocFile.DoFileUnLock();
}

void CDocLocker::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// 書き込めるか検査
	m_bIsDocWritable = true;

	// ファイルの排他ロック
	if( pcDoc->m_cDocFileOperation._ToDoLock() ){
		pcDoc->m_cDocFile.DoFileLock(GetDllShareData().m_Common.m_sFile.m_nFileShareMode);
	}
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
		TopWarningMessage(
			CEditWnd::Instance()->GetHwnd(),
			_T("%ts\nは現在他のプロセスによって書込みが禁止されています。"),
			cDocFile.GetFilePathClass().IsValidPath() ? cDocFile.GetFilePath() : _T("（無題）")
		);
	}
}
