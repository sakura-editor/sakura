/*!	@file
	@brief ファイルオープンダイアログボックス

	@author Norio Nakatani
	@date	1998/08/10 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, Moca
	Copyright (C) 2003, ryoji
	Copyright (C) 2004, genta, MIK
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, Moca, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CDLGOPENFILE_8084B9DB_6463_4168_BA59_132EB2596AE7_H_
#define SAKURA_CDLGOPENFILE_8084B9DB_6463_4168_BA59_132EB2596AE7_H_
#pragma once

#include <memory>
#include <vector>
#include "util/design_template.h"

struct SLoadInfo;	// doc/CDocListener.h
struct SSaveInfo;	// doc/CDocListener.h

/*! フィルタ設定 */
enum EFilter {
	EFITER_NONE,		//!< なし
	EFITER_TEXT,		//!< テキスト
	EFITER_MACRO,		//!< マクロ
	EFITER_MAX,
};

class IDlgOpenFile
{
public:
	virtual void Create(
		HINSTANCE					hInstance,
		HWND						hwndParent,
		const WCHAR*				pszUserWildCard,
		const WCHAR*				pszDefaultPath,
		const std::vector<LPCWSTR>& vMRU			= std::vector<LPCWSTR>(),
		const std::vector<LPCWSTR>& vOPENFOLDER		= std::vector<LPCWSTR>()
	) = 0;

	// 操作

	/*! 開くダイアログ モーダルダイアログの表示
		@param[in,out] pszPath 初期ファイル名．選択されたファイル名の格納場所
		@param[in] eAddFiler フィルタ設定
		@retval true ユーザーがファイル名を選択してOKした
		@retval false ダイアログをユーザーがキャンセル等で閉じたかもしくは開くのに失敗したか
	*/
	virtual bool DoModal_GetOpenFileName(
		WCHAR* pszPath,
		EFilter eAddFilter
	) = 0;

	/*! 保存ダイアログ モーダルダイアログの表示
		@param pszPath [i/o] 初期ファイル名．選択されたファイル名の格納場所
	*/
	virtual bool DoModal_GetSaveFileName(
		WCHAR* pszPath
	) = 0;

	/* 開くダイアログ モーダルダイアログの表示 */
	virtual bool DoModalOpenDlg(
		SLoadInfo* pLoadInfo,
		std::vector<std::wstring>* pFilenames,
		bool bOptions
	) = 0;

	/* 保存ダイアログ モーダルダイアログの表示 */
	virtual bool DoModalSaveDlg(
		SSaveInfo*	pSaveInfo,
		bool bSimpleMode
	) = 0;
};


/*!	ファイルオープンダイアログボックス
*/
class CDlgOpenFile final : public IDlgOpenFile
{
public:
	//コンストラクタ・デストラクタ
	CDlgOpenFile();
	~CDlgOpenFile() = default;

	void Create(
		HINSTANCE					hInstance,
		HWND						hwndParent,
		const WCHAR*				pszUserWildCard,
		const WCHAR*				pszDefaultPath,
		const std::vector<LPCWSTR>& vMRU			= std::vector<LPCWSTR>(),
		const std::vector<LPCWSTR>& vOPENFOLDER		= std::vector<LPCWSTR>()
	) override;

	//操作
	bool DoModal_GetOpenFileName(WCHAR* pszPath, EFilter eAddFileter = EFITER_TEXT) override;
	bool DoModal_GetSaveFileName(WCHAR* pszPath) override;
	bool DoModalOpenDlg(SLoadInfo* pLoadInfo,
		std::vector<std::wstring>* pFilenames,
		bool bOptions = true) override;
	bool DoModalSaveDlg(SSaveInfo*	pSaveInfo, bool bSimpleMode) override;

	// 設定フォルダー相対ファイル選択(共有データ,ini位置依存)
	static BOOL SelectFile(HWND parent, HWND hwndCtl, const WCHAR* filter,
						   bool resolvePath, EFilter eAddFilter = EFITER_TEXT);

	DISALLOW_COPY_AND_ASSIGN(CDlgOpenFile);
private:
	std::shared_ptr<IDlgOpenFile> m_pImpl;
};
#endif /* SAKURA_CDLGOPENFILE_8084B9DB_6463_4168_BA59_132EB2596AE7_H_ */
