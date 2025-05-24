/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDOCFILEOPERATION_EE1C0546_8985_4FB1_941F_3BCC29BB3997_H_
#define SAKURA_CDOCFILEOPERATION_EE1C0546_8985_4FB1_941F_3BCC29BB3997_H_
#pragma once

#include "doc/CDocListener.h" // SLoadInfo
#include "CEol.h"

class CEditDoc;

class CDocFileOperation{
public:
	CDocFileOperation(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) { }

	//ロック
	bool _ToDoLock() const;
	void DoFileLock(bool bMsg = true);
	void DoFileUnlock();
	
	//ロードUI
	bool OpenFileDialog(
		HWND				hwndParent,
		const WCHAR*		pszOpenFolder,	//!< [in]  NULL以外を指定すると初期フォルダーを指定できる
		SLoadInfo*			pLoadInfo,		//!< [in,out] ロード情報
		std::vector<std::wstring>&	files
	);

	//ロードフロー
	bool DoLoadFlow(SLoadInfo* pLoadInfo);
	bool FileLoad(
		SLoadInfo*	pLoadInfo			//!< [in,out]
	);
	bool FileLoadWithoutAutoMacro(
		SLoadInfo*	pLoadInfo			//!< [in,out]
	);
	void ReloadCurrentFile(				//!< 同一ファイルの再オープン Jul. 26, 2003 ryoji BOMオプション追加
		ECodeType	nCharCode			//!< [in] 文字コード種別
	);

	//セーブUI
	bool SaveFileDialog(SSaveInfo* pSaveInfo);	//!<「ファイル名を付けて保存」ダイアログ
	bool SaveFileDialog(LPWSTR szPath);			//!<「ファイル名を付けて保存」ダイアログ

	//セーブフロー
	bool DoSaveFlow(SSaveInfo* pSaveInfo);
	bool FileSaveAs( const WCHAR* filename = NULL,ECodeType eCodeType = CODE_NONE, EEolType eEolType = EEolType::none, bool bDialog = true);	//!< ダイアログでファイル名を入力させ、保存。	// 2006.12.30 ryoji
	bool FileSave();			//!< 上書き保存。ファイル名が指定されていなかったらダイアログで入力を促す。	// 2006.12.30 ryoji

	//クローズ
	bool FileClose();			//!< 閉じて(無題)	// 2006.12.30 ryoji

	//その他
	void FileCloseOpen(				//!< 閉じて開く	// 2006.12.30 ryoji
		const SLoadInfo& sLoadInfo = SLoadInfo(L"", CODE_AUTODETECT, false)
	);

private:
	CEditDoc* m_pcDocRef;
};
#endif /* SAKURA_CDOCFILEOPERATION_EE1C0546_8985_4FB1_941F_3BCC29BB3997_H_ */
