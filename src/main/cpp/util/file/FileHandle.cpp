/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/file/FileHandle.hpp"

#include "basis/CMyString.h"
#include "util/os/Kernel32.hpp"
#include "util/os/make_system_error.hpp"

#include <algorithm>
#include <array>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <system_error>

namespace cxx {

/*!
 * @brief ファイルハンドルを作成する
 *
 * ハンドルリソースを「作成する」からCreateFile。
 *
 * ファイルを開くのか、作成するのかはdwCreationDispositionで指定する。
 *
 * @param[in] fileName ファイル名
 * @param[in] dwDesiredAccess アクセス権
 * @param[in] dwShareMode 共有モード
 * @param[in] lpSecurityAttributes セキュリティ属性
 * @param[in] dwCreationDisposition 作成方法
 * @param[in] dwFlagsAndAttributes ファイル属性と作成フラグ
 * @param[in] hTemplateFile テンプレートファイルのハンドル
 */
/* static */ FileHandle FileHandle::CreateFileW(
	std::wstring_view fileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _FileName{ fileName };

	// NUL終端文字列を取り出す
	const auto lpFileName = _FileName.c_str();

	// パラメーターチェック
	if (!lpFileName || !lpFileName[0]) {
		// この例外が出る場合、使い方が誤っているので、呼出元を修正すること。
		throw std::invalid_argument("fileName is required.");
	}

	if (SFilePath::size() <= _FileName.length()) {
		// この例外が出る場合、使い方が誤っているので、呼出元を修正すること。
		throw std::system_error(
			std::make_error_code(std::errc::filename_too_long),
			"fileName is too long."
		);
	}

	// ファイルハンドルを作成する
	const auto hFile = Kernel32::getInstance()->CreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile
	);

	// 戻り値が INVALID_HANDLE_VALUE だった場合
	if (INVALID_HANDLE_VALUE == hFile) {
		// システムエラー例外を発生させる
		throw cxx::make_system_error("CreateFileW() failed");
	}

	return FileHandle{ hFile };
}

/*!
 * @brief 新しいファイルを作る
 *
 * 指定されたパスに新しいファイルを作成します。
 * 既に存在している場合、戻り値は空になります。
 *
 * @param[in] path ファイル名
 * @param[in, opt] dwFlagsAndAttributes ファイル属性と作成フラグ
 * @return 新しいファイルのハンドル。作成できなかった場合は空のハンドル
 * @throw std::system_error Windowsがエラーを返したとき
 */
/* static */ FileHandle FileHandle::CreateNew(
	std::wstring_view path,
	_In_opt_ DWORD dwFlagsAndAttributes
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Path{ path };

	// 新規作成なので書き込み権限を要求する
	DWORD dwDesiredAccess = GENERIC_WRITE;

	// 書き込み共有は許可する
	DWORD dwShareMode	  = FILE_SHARE_WRITE;

	// 一時ファイル属性が付いていたら読めるようにしておく
	if (dwFlagsAndAttributes & FILE_ATTRIBUTE_TEMPORARY) {
		dwDesiredAccess |= GENERIC_READ;
		dwShareMode		|= FILE_SHARE_READ;
	}

	// 「閉じたら削除」フラグが立っていたら削除できるようにしておく
	if (dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE) {
		dwDesiredAccess |= DELETE;
		dwShareMode		|= FILE_SHARE_DELETE;
	}

	// OSのファイルハンドルを返却する
	FileHandle hFile{};

	// ファイルを作成する
	try {
		hFile = CreateFileW(
			_Path.str(),
			dwDesiredAccess,
			dwShareMode,
			nullptr,
			CREATE_NEW,
			dwFlagsAndAttributes,
			nullptr
		);
	}
	// エラーが発生した場合
	catch (const std::system_error& e) {
		// エラーコードが「既に存在」を示す場合以外はそのまま上位に投げる
		if (ERROR_FILE_EXISTS != e.code().value() &&
			ERROR_ALREADY_EXISTS != e.code().value())
		{
			throw;
		}
	}

	return hFile;
}

/*!
 * @brief 存在しているファイルを開く
 *
 * 指定されたパスに存在するファイルを開きます。
 *
 * @param[in] path ファイル名
 * @param[in] dwDesiredAccess アクセス権
 * @param[in] dwShareMode 共有モード
 * @return 開いたファイルのハンドル。開けなかった場合は空のハンドル
 * @throw std::system_error Windowsがエラーを返したとき
 */
/* static */ FileHandle FileHandle::OpenExisting(
	std::wstring_view path,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Path{ path };

	// OSのファイルハンドルを返却する
	FileHandle hFile{};

	// 存在しているファイルを開く
	try {
		hFile = CreateFileW(
			_Path.str(),
			dwDesiredAccess,
			dwShareMode,
			nullptr,
			OPEN_EXISTING,
			0L,				// 属性は指定しない
			nullptr
		);
	}
	// エラーが発生した場合
	catch (const std::system_error& e) {
		// エラーコードがアクセス拒否以外のエラーはそのまま上位に投げる
		if (ERROR_SHARING_VIOLATION != e.code().value() &&
			ERROR_ACCESS_DENIED != e.code().value())
		{
			throw;
		}
	}

	return hFile;
}

void FileHandle::GetFileTime(
	_Out_opt_ LPFILETIME lpCreationTime,
	_Out_opt_ LPFILETIME lpLastAccessTime,
	_Out_opt_ LPFILETIME lpLastWriteTime
) const
{
	if (Kernel32::getInstance()->GetFileTime(get(), lpCreationTime, lpLastAccessTime, lpLastWriteTime))
	{
		return;	// 成功した
	}

	throw cxx::make_system_error("GetFileTime() failed");
}

void FileHandle::SetFileTime(
	_In_opt_ CONST FILETIME* lpCreationTime,
	_In_opt_ CONST FILETIME* lpLastAccessTime,
	_In_opt_ CONST FILETIME* lpLastWriteTime
) const
{
	if (Kernel32::getInstance()->SetFileTime(get(), lpCreationTime, lpLastAccessTime, lpLastWriteTime))
	{
		return;	// 成功した
	}

	throw cxx::make_system_error("SetFileTime() failed");
}

} // namespace cxx
