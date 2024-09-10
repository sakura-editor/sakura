/*!	@file
	@brief コマンドラインパーサ ヘッダーファイル

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
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CCOMMANDLINE_DF7E2E03_76E1_458C_82AC_7C485EECF677_H_
#define SAKURA_CCOMMANDLINE_DF7E2E03_76E1_458C_82AC_7C485EECF677_H_
#pragma once

#include "_main/global.h"
#include "mem/CNativeW.h"
#include "EditInfo.h"
#include "GrepInfo.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

/*!
 * @brief コマンドラインパーサ クラス
 */
class CCommandLine {
public:
	[[nodiscard]] static CCommandLine* getInstance();

	CCommandLine() noexcept;

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
	bool IsGrepMode() const noexcept { return m_bGrepMode; }
	bool IsGrepDlg() const noexcept { return m_bGrepDlg; }
	bool IsDebugMode() const noexcept { return m_bDebugMode; }
	bool IsViewMode() const noexcept { return m_bViewMode; }
	bool GetEditInfo(EditInfo* fi) const noexcept { *fi = m_fi; return true; }
	bool GetGrepInfo(GrepInfo* gi) const noexcept { *gi = m_gi; return true; }
	int GetGroupId() const noexcept { return m_nGroup; }	// 2007.06.26 ryoji
	std::optional<LPCWSTR> GetMacro() const noexcept { return m_Macro.size() ? std::optional<LPCWSTR>(m_Macro.c_str()) : std::nullopt; }
	std::optional<LPCWSTR> GetMacroType() const noexcept {
		constexpr auto defaultMacroType = L"file"sv;
		return 0 != _wcsnicmp(m_MacroType.c_str(), defaultMacroType.data(), defaultMacroType.size()) && m_MacroType.size() ? std::optional<LPCWSTR>(m_MacroType.c_str()) : std::nullopt;
	}
	bool IsSetProfile() const noexcept { return m_bSetProfile; }
	std::optional<LPCWSTR> GetProfileOpt() const noexcept { return m_bSetProfile ? std::optional<LPCWSTR>(m_ProfileName.c_str()) : std::nullopt; }
	LPCWSTR GetProfileName() const noexcept { return GetProfileOpt().value_or(nullptr); }
	bool IsProfileMgr() const noexcept { return m_bProfileMgr; }
	const CLogicPoint& GetCaretLocation() const noexcept { return m_fi.m_ptCursor; }
	CLayoutPoint GetViewLocation() const noexcept { return { m_fi.m_nViewLeftCol,  m_fi.m_nViewTopLine }; }
	tagSIZE GetWindowSize() const noexcept { return { m_fi.m_nWindowSizeX, m_fi.m_nWindowSizeY }; }
	tagPOINT GetWindowOrigin() const noexcept { return { m_fi.m_nWindowOriginX, m_fi.m_nWindowOriginY }; }
	LPCWSTR GetOpenFile() const noexcept { return m_fi.m_szPath; }
	int GetFileNum(void) const noexcept { return static_cast<int>(m_vFiles.size()); }
	const WCHAR* GetFileName(int i) const noexcept { return i < GetFileNum() ? m_vFiles[i].c_str() : NULL; }
	void ClearFile(void) noexcept { m_vFiles.clear(); }
	LPCWSTR GetDocType() const noexcept { return m_fi.m_szDocType; }
	ECodeType GetDocCode() const noexcept { return m_fi.m_nCharCode; }

	std::wstring ToCommandArgs() const;

	void ParseKanjiCodeFromFileName( LPWSTR pszExeFileName, int cchExeFileName );
	void ParseCommandLine( LPCWSTR pszCmdLineSrc, bool bResponse = true );

	void SetNoWindow(bool noWindow) { m_bNoWindow = noWindow; }
	void SetMacro(std::wstring_view macro) { m_Macro = macro; }
	void SetMacroType(std::wstring_view macroType) { m_MacroType = macroType; }
	void SetProfileName(std::wstring_view profileName) {
		m_ProfileName = profileName;
		m_bSetProfile = true;
	}

// member valiables
private:
	bool		m_bGrepMode;		//! [out] TRUE: Grep Mode
	bool		m_bGrepDlg;			//  Grepダイアログ
	bool		m_bDebugMode;		
	bool		m_bNoWindow = false;		//! [out] TRUE: 編集Windowを開かない
	bool		m_bProfileMgr;
	EditInfo	m_fi;				//!
	GrepInfo	m_gi;				//!
	bool		m_bViewMode;		//! [out] TRUE: Read Only
	int			m_nGroup;			//! グループID	// 2007.06.26 ryoji

	std::wstring	m_Macro;                //!< マクロファイル名／マクロ文
	std::wstring	m_MacroType;            //!< マクロ種別

	bool			m_bSetProfile = false;  //!< プロファイル名が指定された
	std::wstring	m_ProfileName;          //!< プロファイル名

	std::vector<std::wstring> m_vFiles;	//!< ファイル名(複数)
};

#endif /* SAKURA_CCOMMANDLINE_DF7E2E03_76E1_458C_82AC_7C485EECF677_H_ */
