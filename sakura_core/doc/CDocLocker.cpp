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

	// 読み取り専用ファイルの場合は、以下の処理は行わない
	if( !pcDoc->m_cDocFile.HasWritablePermission() ){
		m_bIsDocWritable = false;
		return;
	}

	//書き込めるか検査
	CDocFile& cDocFile = pcDoc->m_cDocFile;
	m_bIsDocWritable = cDocFile.IsFileWritable();
	if(!m_bIsDocWritable){
		TopWarningMessage(
			CEditWnd::Instance()->GetHwnd(),
			_T("%ts\nは現在他のプロセスによって書込みが禁止されています。"),
			cDocFile.GetFilePathClass().IsValidPath() ? cDocFile.GetFilePath() : _T("（無題）")
		);
	}

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
	if( pcDoc->m_cDocFileOperation._ToDoLock() ){
		// ファイルの排他ロック
		pcDoc->m_cDocFile.DoFileLock(GetDllShareData().m_Common.m_sFile.m_nFileShareMode);
	}
}


