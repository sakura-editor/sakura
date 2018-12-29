/*!	@file
	@brief ファイルオープンダイアログボックス

	@author Norio Nakatani
	@date	1998/08/10 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, Stonee, genta
	Copyright (C) 2002, MIK, YAZAKI, genta
	Copyright (C) 2003, MIK, KEITA, Moca, ryoji
	Copyright (C) 2004, genta
	Copyright (C) 2005, novice, ryoji
	Copyright (C) 2006, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "dlg/CDlgOpenFile.h"

extern std::shared_ptr<IDlgOpenFile> New_CDlgOpenFile_CommonFileDialog();
extern std::shared_ptr<IDlgOpenFile> New_CDlgOpenFile_CommonItemDialog();

CDlgOpenFile::CDlgOpenFile()
{
}

void CDlgOpenFile::Create(
	HINSTANCE					hInstance,
	HWND						hwndParent,
	const TCHAR*				pszUserWildCard,
	const TCHAR*				pszDefaultPath,
	const std::vector<LPCTSTR>& vMRU,
	const std::vector<LPCTSTR>& vOPENFOLDER
) {
	if( GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog ){
		m_pImpl = New_CDlgOpenFile_CommonItemDialog();
	}
	else {
		m_pImpl = New_CDlgOpenFile_CommonFileDialog();
	}
	m_pImpl->Create(hInstance, hwndParent, pszUserWildCard, pszDefaultPath, vMRU, vOPENFOLDER);
}

inline bool CDlgOpenFile::DoModal_GetOpenFileName(
	TCHAR* pszPath,
	EFilter eAddFileter)
{
	return m_pImpl->DoModal_GetOpenFileName(pszPath, eAddFileter);
}

inline bool CDlgOpenFile::DoModal_GetSaveFileName( TCHAR* pszPath )
{
	return m_pImpl->DoModal_GetSaveFileName(pszPath);
}

inline bool CDlgOpenFile::DoModalOpenDlg(
	SLoadInfo* pLoadInfo,
	std::vector<std::tstring>* pFilenames,
	bool bOptions)
{
	return m_pImpl->DoModalOpenDlg(pLoadInfo, pFilenames, bOptions);
}

inline bool CDlgOpenFile::DoModalSaveDlg(
	SSaveInfo* pSaveInfo,
	bool bSimpleMode)
{
	return m_pImpl->DoModalSaveDlg(pSaveInfo, bSimpleMode);
}

/* static */
/*! ファイル選択
	@note 実行ファイルのパスor設定ファイルのパスが含まれる場合は相対パスに変換
*/
BOOL CDlgOpenFile::SelectFile(
	HWND parent,
	HWND hwndCtl,
	const TCHAR* filter,
	bool resolvePath,
	EFilter eAddFilter)
{
	CDlgOpenFile cDlgOpenFile;
	TCHAR			szFilePath[_MAX_PATH + 1];
	TCHAR			szPath[_MAX_PATH + 1];
	::GetWindowText( hwndCtl, szFilePath, _countof(szFilePath) );
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	if( resolvePath && _IS_REL_PATH( szFilePath ) ){
		GetInidirOrExedir(szPath, szFilePath);
	}else{
		auto_strcpy(szPath, szFilePath);
	}
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		::GetModuleHandle(NULL),
		parent,
		filter,
		szPath
	);
	if( cDlgOpenFile.DoModal_GetOpenFileName(szPath, eAddFilter) ){
		const TCHAR* fileName;
		if( resolvePath ){
			fileName = GetRelPath( szPath );
		}else{
			fileName = szPath;
		}
		::SetWindowText( hwndCtl, fileName );
		return TRUE;
	}
	return FALSE;
}

