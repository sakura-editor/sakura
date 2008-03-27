#pragma once

class CEditDoc;

class CDocFileOperation{
public:
	CDocFileOperation(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) { }

	//ロック
	bool _ToDoLock() const;

	
	//ロードUI
	bool OpenFileDialog(
		HWND				hwndParent,
		const TCHAR*		pszOpenFolder,	//!< [in]  NULL以外を指定すると初期フォルダを指定できる
		SLoadInfo*			pLoadInfo		//!< [in/out] ロード情報
	);

	//ロードフロー
	bool DoLoadFlow(SLoadInfo* pLoadInfo);
	bool FileLoad(
		SLoadInfo*	pLoadInfo			//!< [in/out]
	);
	void ReloadCurrentFile(				//!< 同一ファイルの再オープン Jul. 26, 2003 ryoji BOMオプション追加
		ECodeType	nCharCode			//!< [in] 文字コード種別
	);

	
	//セーブUI
	bool SaveFileDialog(SSaveInfo* pSaveInfo);	//!<「ファイル名を付けて保存」ダイアログ
	bool SaveFileDialog(LPTSTR szPath);			//!<「ファイル名を付けて保存」ダイアログ

	//セーブフロー
	bool DoSaveFlow(const SSaveInfo& sSaveInfo);
	bool FileSaveAs();			//!< ダイアログでファイル名を入力させ、保存。	// 2006.12.30 ryoji
	bool FileSave();			//!< 上書き保存。ファイル名が指定されていなかったらダイアログで入力を促す。	// 2006.12.30 ryoji

	//クローズ
	bool FileClose();			//!< 閉じて(無題)	// 2006.12.30 ryoji

	//その他
	void FileCloseOpen(				//!< 閉じて開く	// 2006.12.30 ryoji
		const SLoadInfo& sLoadInfo = SLoadInfo(_T(""), CODE_AUTODETECT, false)
	);

private:
	CEditDoc* m_pcDocRef;
};
