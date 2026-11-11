/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "basis/primitive.h"
#include "cxx/ResourceHolder.hpp"

#include <span>
#include <string_view>
#include <type_traits>

#include <Windows.h>

namespace cxx {

/*!
 * @brief OSのファイルハンドルをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 */
class FileHandle : public cxx::ResourceHolder<&::CloseHandle>
{
private:
	using Base = cxx::ResourceHolder<&::CloseHandle>;
	using Me = FileHandle;

public:
	/*!
	 * @brief ファイルハンドルを作成する
	 *
	 * @param[in] fileName ファイル名
	 * @param[in] dwDesiredAccess アクセス権
	 * @param[in] dwShareMode 共有モード
	 * @param[in] lpSecurityAttributes セキュリティ属性
	 * @param[in] dwCreationDisposition 作成方法
	 * @param[in] dwFlagsAndAttributes ファイル属性と作成フラグ
	 * @param[in] hTemplateFile テンプレートファイルのハンドル
	 */
	static Me CreateFileW(
		std::wstring_view fileName,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile
	);

	/*!
	 * @brief ファイルハンドルを作成する
	 *
	 * @param[in] path ファイル名
	 * @param[in] dwDesiredAccess アクセス権
	 * @param[in] dwShareMode 共有モード
	 * @param[in] lpSecurityAttributes セキュリティ属性
	 * @param[in] dwCreationDisposition 作成方法
	 * @param[in] dwFlagsAndAttributes ファイル属性と作成フラグ
	 * @param[in] hTemplateFile テンプレートファイルのハンドル
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	static Me CreateFileW(
		const A& path,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile
	)
	{
		// 引数はNUL終端文字列として扱う
		cxx::NullTerminatedString<WCHAR> _Path{ path };

		// 文字列を渡すバージョンを呼び出す
		return CreateFileW(
			_Path.str(),
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile
		);
	}

	/*!
	 * @brief 新しいファイルを作る
	 *
	 * @param[in] path ファイル名
	 * @param[in, opt] dwFlagsAndAttributes ファイル属性と作成フラグ
	 * @return 新しいファイルのハンドル。作成できなかった場合は空のハンドル
	 * @throw std::system_error Windowsがエラーを返したとき
	 */
	static Me CreateNew(
		std::wstring_view fileName,
		_In_opt_ DWORD dwFlagsAndAttributes = 0
	);

	/*!
	 * @brief 新しいファイルを作る
	 *
	 * @param[in] path ファイル名
	 * @param[in, opt] dwFlagsAndAttributes ファイル属性と作成フラグ
	 * @return 新しいファイルのハンドル。作成できなかった場合は空のハンドル
	 * @throw std::system_error Windowsがエラーを返したとき
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	static Me CreateNew(
		const A& path,
		_In_opt_ DWORD dwFlagsAndAttributes = 0
	)
	{
		// 引数はNUL終端文字列として扱う
		cxx::NullTerminatedString<WCHAR> _Path{ path };

		// 文字列を渡すバージョンを呼び出す
		return CreateNew(
			_Path.str(),
			dwFlagsAndAttributes
		);
	}

	/*!
	 * @brief 存在しているファイルを開く
	 *
	 * @param[in] path ファイル名
	 * @param[in] dwDesiredAccess アクセス権
	 * @param[in] dwShareMode 共有モード
	 * @return 開いたファイルのハンドル。開けなかった場合は空のハンドル
	 * @throw std::system_error Windowsがエラーを返したとき
	 */
	static Me OpenExisting(
		std::wstring_view path,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode
	);

	/*!
	 * @brief 存在しているファイルを開く
	 *
	 * @param[in] path ファイル名
	 * @param[in] dwDesiredAccess アクセス権
	 * @param[in] dwShareMode 共有モード
	 * @return 開いたファイルのハンドル。開けなかった場合は空のハンドル
	 * @throw std::system_error Windowsがエラーを返したとき
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	static Me OpenExisting(
		const A& path,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode
	)
	{
		// 引数はNUL終端文字列として扱う
		cxx::NullTerminatedString<WCHAR> _Path{ path };

		// 文字列を渡すバージョンを呼び出す
		return OpenExisting(
			_Path.str(),
			dwDesiredAccess,
			dwShareMode
		);
	}

	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;

	void GetFileTime(
		_Out_opt_ LPFILETIME lpCreationTime,
		_Out_opt_ LPFILETIME lpLastAccessTime,
		_Out_opt_ LPFILETIME lpLastWriteTime
	) const;

	void SetFileTime(
		_In_opt_ CONST FILETIME* lpCreationTime,
		_In_opt_ CONST FILETIME* lpLastAccessTime,
		_In_opt_ CONST FILETIME* lpLastWriteTime
	) const;
};

} // namespace cxx
