#include "stdafx.h"
#include "CReadHelper.h"
#include "util/file.h"

bool IsDirectory(LPCTSTR pszPath)
{
	HANDLE			hFind;
	WIN32_FIND_DATA	w32fd;
	hFind = ::FindFirstFile( pszPath, &w32fd );
	if(hFind!=INVALID_HANDLE_VALUE){
		::FindClose( hFind );
		return (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	else{
		return false;
	}
}


ECallbackResult CReadHelper::OnCheckLoad(SLoadInfo* pLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//フォルダが指定された場合は「ファイルを開く」ダイアログを表示し、実際のファイル入力を促す
	if( IsDirectory(pLoadInfo->cFilePath) ){
		pLoadInfo->eCharCode = CODE_AUTODETECT;	// 文字コード自動判別
		pLoadInfo->bViewMode = false;			// ビューモードオフ
		bool bDlgResult = pcDoc->m_cDocFileOperation.OpenFileDialog(
			CEditWnd::Instance()->GetHwnd(),
			pLoadInfo->cFilePath.GetDirPath().c_str(),	//指定されたフォルダ
			pLoadInfo
		);
		if( !bDlgResult ){
			return CALLBACK_INTERRUPT; //キャンセルされた場合は中断
		}
	}
	return CALLBACK_CONTINUE;
}
