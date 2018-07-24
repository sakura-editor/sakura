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
				LS( STR_ERR_DLGEDITDOC21 ),	//"%ts\nは現在他のプロセスによって書込みが禁止されています。"
				cDocFile.GetFilePathClass().IsValidPath() ? cDocFile.GetFilePath() : LS(STR_NO_TITLE1)	//"(無題)"
			);
		}
	}
}
