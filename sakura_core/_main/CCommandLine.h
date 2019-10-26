/*!	@file
	@brief コマンドラインパーサ ヘッダファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CControlTrayより分離
	Copyright (C) 2002, genta
	Copyright (C) 2005, D.S.Koba
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CCOMMANDLINE_H_
#define _CCOMMANDLINE_H_

#include <vector>

#include "global.h"
#include "charset/charcode.h"
#include "mem/CNativeW.h"
#include "EditInfo.h"
#include "util/design_template.h"

/*!	検索オプション
	20020118 aroka
*/
struct GrepInfo {
	CNativeW		cmGrepKey;			//!< 検索キー
	CNativeW		cmGrepRep;			//!< 置換キー
	CNativeW		cmGrepFile;			//!< 検索対象ファイル
	CNativeW		cmGrepFolder;		//!< 検索対象フォルダ
	CNativeW		cmExcludeFile;		//!< 除外対象ファイル
	CNativeW		cmExcludeFolder;	//!< 除外対象フォルダ
	SSearchOption	sGrepSearchOption;	//!< 検索オプション
	bool			bGrepCurFolder;		//!< カレントディレクトリを維持
	bool			bGrepStdout;		//!< 標準出力モード
	bool			bGrepHeader;		//!< ヘッダ情報表示
	bool			bGrepSubFolder;		//!< サブフォルダを検索する
	ECodeType		nGrepCharSet;		//!< 文字コードセット
	int				nGrepOutputStyle;	//!< 結果出力形式
	int				nGrepOutputLineType;	//!< 結果出力：行を出力/該当部分/否マッチ行
	bool			bGrepOutputFileOnly;	//!< ファイル毎最初のみ検索
	bool			bGrepOutputBaseFolder;	//!< ベースフォルダ表示
	bool			bGrepSeparateFolder;	//!< フォルダ毎に表示
	bool			bGrepReplace;		//!< Grep置換
	bool			bGrepPaste;			//!< クリップボードから貼り付け
	bool			bGrepBackup;		//!< 置換でバックアップを保存

	GrepInfo() noexcept
		: cmGrepKey()
		, cmGrepRep()
		, cmGrepFile()
		, cmGrepFolder()
		, cmExcludeFile()
		, cmExcludeFolder()
		, sGrepSearchOption()
		, bGrepCurFolder(false)
		, bGrepStdout(false)
		, bGrepHeader(true)
		, bGrepSubFolder(false)
		, nGrepCharSet(CODE_SJIS)
		, nGrepOutputStyle(1)
		, nGrepOutputLineType(0)
		, bGrepOutputFileOnly(false)
		, bGrepOutputBaseFolder(false)
		, bGrepSeparateFolder(false)
		, bGrepReplace(false)
		, bGrepPaste(false)
		, bGrepBackup(false)
	{
	}

	bool operator == (const GrepInfo& rhs) const noexcept {
		if (this == &rhs) return true;
		return cmGrepKey == rhs.cmGrepKey
			&& cmGrepRep == rhs.cmGrepRep
			&& cmGrepFile == rhs.cmGrepFile
			&& cmGrepFolder == rhs.cmGrepFolder
			&& cmExcludeFile == rhs.cmExcludeFile
			&& cmExcludeFolder == rhs.cmExcludeFolder
			&& sGrepSearchOption == rhs.sGrepSearchOption
			&& bGrepCurFolder == rhs.bGrepCurFolder
			&& bGrepStdout == rhs.bGrepStdout
			&& bGrepHeader == rhs.bGrepHeader
			&& bGrepSubFolder == rhs.bGrepSubFolder
			&& nGrepCharSet == rhs.nGrepCharSet
			&& nGrepOutputStyle == rhs.nGrepOutputStyle
			&& nGrepOutputLineType == rhs.nGrepOutputLineType
			&& bGrepOutputFileOnly == rhs.bGrepOutputFileOnly
			&& bGrepOutputBaseFolder == rhs.bGrepOutputBaseFolder
			&& bGrepSeparateFolder == rhs.bGrepSeparateFolder
			&& bGrepReplace == rhs.bGrepReplace
			&& bGrepPaste == rhs.bGrepPaste
			&& bGrepBackup == rhs.bGrepBackup;
	}
	bool operator != (const GrepInfo& rhs) const noexcept { return !(*this == rhs); }
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

/*!
	@brief コマンドラインパーサ クラス
*/
class CCommandLine  : public TSingleton<CCommandLine> {
	friend class TSingleton<CCommandLine>;
public:
	CCommandLine();

private:
	static int CheckCommandLine(
		LPWSTR	str,		//!< [in] 検証する文字列（先頭の-は含まない）
		WCHAR**	arg,		//!< [out] 引数がある場合はその先頭へのポインタ
		int*	arglen		//!< [out] 引数の長さ
	);

	/*!
		引用符で囲まれている数値を認識するようにする
		@date 2002.12.05 genta
	*/
	static int AtoiOptionInt(const WCHAR* arg){
		return ( arg[0] == L'"' || arg[0] == L'\'' ) ?
			_wtoi( arg + 1 ) : _wtoi( arg );
	}

// member accessor method
public:
	bool IsNoWindow() const noexcept { return m_bNoWindow; }
	bool IsWriteQuit() const noexcept { return m_bWriteQuit; }	// 2007.05.19 ryoji sakuext用に追加
	bool IsGrepMode() const noexcept { return m_bGrepMode; }
	bool IsGrepDlg() const noexcept { return m_bGrepDlg; }
	bool IsDebugMode() const noexcept { return m_bDebugMode; }
	bool IsViewMode() const noexcept { return m_bViewMode; }
	bool GetEditInfo(EditInfo* fi) const noexcept { *fi = m_fi; return true; }
	const EditInfo& GetEditInfoRef() const noexcept { return m_fi; }
	bool GetGrepInfo(GrepInfo* gi) const noexcept { *gi = m_gi; return true; }
	const GrepInfo& GetGrepInfoRef() const noexcept { return m_gi; }
	int GetGroupId() const noexcept { return m_nGroup; }	// 2007.06.26 ryoji
	LPCWSTR GetMacro() const noexcept { return m_cmMacro.GetStringPtr(); }
	LPCWSTR GetMacroType() const noexcept { return m_cmMacroType.GetStringPtr(); }
	LPCWSTR GetProfileName() const noexcept { return m_cmProfile.GetStringPtr(); }
	bool IsSetProfile() const noexcept { return m_bSetProfile; }
	void SetProfileName(LPCWSTR s){
		m_bSetProfile = true;
		m_cmProfile.SetString(s);
	}
	bool IsProfileMgr() const noexcept { return m_bProfileMgr; }
	const CLogicPoint& GetCaretLocation() const noexcept { return m_fi.m_ptCursor; }
	CLayoutPoint GetViewLocation() const noexcept { return { m_fi.m_nViewLeftCol,  m_fi.m_nViewTopLine }; }
	tagSIZE GetWindowSize() const noexcept { return { m_fi.m_nWindowSizeX, m_fi.m_nWindowSizeY }; }
	tagPOINT GetWindowOrigin() const noexcept { return { m_fi.m_nWindowOriginX, m_fi.m_nWindowOriginY }; }
	LPCWSTR GetOpenFile() const noexcept { return m_fi.m_szPath; }
	int GetFileNum(void) const noexcept { return m_vFiles.size(); }
	const WCHAR* GetFileName(int i) const noexcept { return i < GetFileNum() ? m_vFiles[i].c_str() : NULL; }
	void ClearFile(void) noexcept { m_vFiles.clear(); }
	LPCWSTR GetDocType() const noexcept { return m_fi.m_szDocType; }
	ECodeType GetDocCode() const noexcept { return m_fi.m_nCharCode; }
	void ParseCommandLine( LPCWSTR pszCmdLineSrc, bool bResponse = true );

// member valiables
private:
	bool		m_bGrepMode;		//! [out] TRUE: Grep Mode
	bool		m_bGrepDlg;			//  Grepダイアログ
	bool		m_bDebugMode;		
	bool		m_bNoWindow;		//! [out] TRUE: 編集Windowを開かない
	bool		m_bWriteQuit;		//! [out] TRUE: 設定を保存して終了	// 2007.05.19 ryoji sakuext用に追加
	bool		m_bProfileMgr;
	bool		m_bSetProfile;
	EditInfo	m_fi;				//!
	GrepInfo	m_gi;				//!
	bool		m_bViewMode;		//! [out] TRUE: Read Only
	int			m_nGroup;			//! グループID	// 2007.06.26 ryoji
	CNativeW	m_cmMacro;			//! [out] マクロファイル名／マクロ文
	CNativeW	m_cmMacroType;		//! [out] マクロ種別
	CNativeW	m_cmProfile;		//! プロファイル名
	std::vector<std::wstring> m_vFiles;	//!< ファイル名(複数)
};

///////////////////////////////////////////////////////////////////////
#endif /* _CCOMMANDLINE_H_ */

