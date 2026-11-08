/*! @file */
/*
	Copyright (C) 2002, SUI
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_FILE_FE33056B_6B48_4668_AE95_923EC960A607_H_
#define SAKURA_FILE_FE33056B_6B48_4668_AE95_923EC960A607_H_
#pragma once

#include "basis/primitive.h"

#include <mutex>
#include <string>
#include <string_view>
#include <filesystem>

bool fexist(LPCWSTR pszPath); //!< ファイルまたはディレクトリが存在すればtrue
bool fexist(const std::filesystem::path& path) noexcept; //!< ファイルまたはディレクトリが存在すればtrue

bool IsFilePath( const wchar_t* pLine, size_t* pnBgn, size_t* pnPathLen, bool bFileOnly = true );
bool IsFileExists(const WCHAR* path, bool bFileOnly = false);
bool IsDirectory(LPCWSTR pszPath);	// 2009.08.20 ryoji

/*!
 * パスがファイル名に使えない文字を含んでいるかチェックする
 *
 * @param[in] szPath チェック対象のパス文字列
 * @retval true  パスはファイル名に使えない文字を含んでいる
 * retuval false パスはファイル名に使えない文字を含んでいない
 */
bool IsInvalidFilenameChars(
	std::wstring_view szPath
);

/*!
 * パスがファイル名に使えない文字を含んでいるかチェックする
 *
 * @tparam[in] A 文字列型（NUL終端文字列として扱える型）
 * @param[in] szPath チェック対象のパス文字列
 * @retval true  パスはファイル名に使えない文字を含んでいる
 * retuval false パスはファイル名に使えない文字を含んでいない
 */
template <basis::NullTerminatedStringConstructible<WCHAR> A>
	requires (!std::is_same_v<A, std::wstring_view>)
bool IsInvalidFilenameChars(
	const A& szPath
)
{
	// 入力元をNUL終端文字列とみなす
	const auto path = cxx::NullTerminatedString<WCHAR>{ szPath };

	// パスがファイル名に使えない文字を含んでいるかチェックする
	return IsInvalidFilenameChars(path.str());
}

[[nodiscard]] bool IsValidPathAvailableChar(const std::filesystem::path& path) noexcept;

//	Apr. 30, 2003 genta
//	ディレクトリの深さを調べる
int CalcDirectoryDepth(const WCHAR* path);

// 2005.11.26 aroka
bool IsLocalDrive( const WCHAR* pszDrive );

//※サクラ依存
FILE *_wfopen_absexe(LPCWSTR fname, LPCWSTR mode); // 2003.06.23 Moca
FILE *_wfopen_absini(LPCWSTR fname, LPCWSTR mode, BOOL bOrExedir = TRUE); // 2007.05.19 ryoji

//パス文字列処理
void CutLastYenFromDirectoryPath( WCHAR* pszFolder );			/* フォルダーの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない*/

namespace cxx {

void AddLastYenFromDirectoryPath(std::span<WCHAR> folder);			/* フォルダーの最後が半角かつ'\\'でない場合は、付加する */

} // namespace cxx

template <basis::WritableBuffer<WCHAR> A>
void AddLastYenFromDirectoryPath(A& folder)
{
	// パスの末尾に円マークを付ける
	cxx::AddLastYenFromDirectoryPath(std::span<WCHAR>{ folder });
}

std::wstring AddLastYenPath(std::wstring_view path);
std::wstring ReplaceSlashToYen(std::wstring_view path);	//!< パスの区切りの / を \ に置き換える
void SplitPath_FolderAndFile( const WCHAR* pszFilePath, WCHAR* pszFolder, WCHAR* pszFile );	/* ファイルのフルパスを、フォルダーとファイル名に分割 */
void Concat_FolderAndFile( const WCHAR* pszDir, const WCHAR* pszTitle, WCHAR* pszPath );/* フォルダー、ファイル名から、結合したパスを作成 */
BOOL GetLongFileName( const WCHAR* pszFilePathSrc, WCHAR* pszFilePathDes );					/* ロングファイル名を取得する */
BOOL CheckEXT( const WCHAR* pszPath, const WCHAR* pszExt );					/* 拡張子を調べる */
const WCHAR* GetFileTitlePointer(const WCHAR* pszPath);							//!< ファイルフルパス内のファイル名を指すポインタを取得。2007.09.20 kobake 作成
bool _IS_REL_PATH(const WCHAR* path);											//!< 相対パスか判定する。2003.06.23 Moca

std::filesystem::path GetExeFileName();
std::filesystem::path GetIniFileName();

//※サクラ依存
void GetExedir( LPWSTR pDir, LPCWSTR szFile = nullptr );
void GetInidir( LPWSTR pDir, LPCWSTR szFile = nullptr ); // 2007.05.19 ryoji
void GetInidirOrExedir( LPWSTR pDir, LPCWSTR szFile = nullptr, BOOL bRetExedirIfFileEmpty = FALSE ); // 2007.05.22 ryoji

LPCWSTR GetRelPath( LPCWSTR pszPath );

std::filesystem::path GetTempFilePath(std::wstring_view prefix, const std::optional<std::filesystem::path>& optTempDir = std::nullopt);

//ファイル時刻
//TODO: いつか個別ファイルに移動したい
// 移動先は util/file/CFileTime.hpp あたり？
class CFileTime{
public:
	/*!
	 * @brief FILETIMEをローカル時刻に変換する
	 *
	 * @param[in] ftime ファイル時刻(UTCで記録されている)
	 * @param[out] lpLocalTime ローカル時刻を返す場所
	 * @retval true  成功
	 * @retval false 失敗
	 */
	static bool FileTimeToLocalTime(
		const FILETIME& ftime,
		SYSTEMTIME* lpLocalTime
	)
	{
		// この例外が出る場合、使い方が誤っているので、呼出元を修正すること。
		if (!lpLocalTime) throw std::invalid_argument("lpLocalTime is required.");

		SYSTEMTIME gmt{};
		return
			::FileTimeToSystemTime(&ftime, &gmt) &&
			::SystemTimeToTzSpecificLocalTime(nullptr, &gmt, lpLocalTime);
	}

	CFileTime() = default;

	//TODO: いつかexplicit付ける
	// explicitにしたいが、既存コードに利用箇所があるので一旦放置。
	CFileTime(const FILETIME& ftime){ SetFILETIME(ftime); }

	//設定
	void ClearFILETIME()
	{
		FILETIME ftime{};
		SetFILETIME(ftime);
	}

	void SetFILETIME(const FILETIME& ftime)
	{
		std::unique_lock lock{ m_Mutex };

		m_ftime = ftime;
		m_bModified = true;
	}

	//取得
	const FILETIME& GetFILETIME() const noexcept { return m_ftime; }

	const SYSTEMTIME& GetSYSTEMTIME() const
	{
		std::unique_lock lock{ m_Mutex };

		//キャッシュ更新 -> m_systime, m_bModified
		if(m_bModified){
			m_bModified = false;
			if (!FileTimeToLocalTime(m_ftime, &m_systime))
			{
				memset(&m_systime,0,sizeof(m_systime)); //失敗時ゼロクリア
			}
		}

		return m_systime;
	}

	const SYSTEMTIME* operator->() const{ return &GetSYSTEMTIME(); }

	//判定
	bool has_value() const noexcept
	{
		return
			0 != m_ftime.dwLowDateTime ||
			0 != m_ftime.dwHighDateTime;
	}

	explicit operator bool() const noexcept
	{
		return has_value();
	}

	bool IsZero() const noexcept
	{
		return !has_value();
	}

private:
	FILETIME m_ftime{};

	mutable std::mutex m_Mutex{};

	//キャッシュ
	mutable SYSTEMTIME	m_systime{};
	mutable bool		m_bModified = false;
};

/*!
 * @brief ファイルの更新日時を取得
 *
 * @param[in] filename ファイルのパス
 * @returns ファイルの更新日時
 */
FILETIME GetLastWriteTime(
	std::wstring_view filename
);

/*!
 * @brief ファイルの更新日時を取得
 *
 * @param[in] filename ファイルのパス
 * @param[out] pcFileTime 更新日時を返す場所
 * @retval true  成功
 * @retval false 失敗
 * @date 2005/10/22 genta
 */
template <basis::NullTerminatedStringConstructible<WCHAR> A>
bool GetLastWriteTimestamp(
	const A& filename,
	_Out_ CFileTime* pcFileTime
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Filename{ filename };

	bool ret = false;

	try {
		// この例外が出る場合、使い方が誤っているので、呼出元を修正すること。
		if (!pcFileTime) throw std::invalid_argument("pcFileTime is required.");

		// C++バージョンを呼び出す
		const auto ftLastWrite = GetLastWriteTime(
			_Filename.str()
		);

		// 取得した日時をCFileTimeに設定する
		pcFileTime->SetFILETIME(ftLastWrite);

		ret = true;
	}
	// エラーが発生した場合
	catch (const std::system_error&) {
		pcFileTime->ClearFILETIME();
	}

	return ret;
}

//文字列分割
void my_splitpath_w ( const wchar_t *comln , wchar_t *drv,wchar_t *dir,wchar_t *fnm,wchar_t *ext );
#define my_splitpath_t my_splitpath_w

int FileMatchScoreSepExt( std::wstring_view file1, std::wstring_view file2 );

void GetStrTrancateWidth( WCHAR* dest, int nSize, const WCHAR* path, HDC hDC, int nPxWidth );
void GetShortViewPath(WCHAR* dest, int nSize, const WCHAR* path, HDC hDC, int nPxWidth, bool bFitMode );

#endif /* SAKURA_FILE_FE33056B_6B48_4668_AE95_923EC960A607_H_ */
