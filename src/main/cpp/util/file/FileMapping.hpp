/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"
#include "util/file/MappedFileView.hpp"

#include <span>
#include <string_view>

#include <Windows.h>

namespace cxx {

/*!
 * @brief OSのファイルマッピングオブジェクトをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 */
class FileMapping : public cxx::ResourceHolder<&::CloseHandle>
{
private:
	using Base = cxx::ResourceHolder<&::CloseHandle>;
	using Me = FileMapping;

public:
	/*!
	 * @brief ファイルマッピングを作る
	 *
	 * @param[in] hFile ファイルハンドル
	 * @param[in] lpFileMappingAttributes セキュリティ属性
	 * @param[in] flProtect 保護属性
	 * @param[in] maximumSize 最大サイズ
	 * @param[in] name ファイルマッピングの名前
	 */
	static Me CreateFileMappingW(
		_In_ HANDLE hFile,
		_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
		_In_ DWORD flProtect,
		const ULARGE_INTEGER& maximumSize,
		std::wstring_view name
	);

	/*!
	 * @brief ファイルマッピングを開く
	 *
	 * @param[in] dwDesiredAccess 要求するアクセス権
	 * @param[in] bInheritHandle ハンドルを継承するかどうか
	 * @param[in] name ファイルマッピングの名前
	 */
	static Me OpenFileMappingW(
		_In_ DWORD dwDesiredAccess,
		_In_ BOOL bInheritHandle,
		std::wstring_view name
	);

	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;

	/*!
	 * @brief マッピングされたデータを取得する
	 *
	 * @param[in] dwDesiredAccess 要求するアクセス権
	 * @param[in] fileOffset マップするデータのファイル内オフセット
	 */
	template <typename T>
	MappedFileView<T> MapViewOfFile(
		_In_ DWORD dwDesiredAccess,
		const ULARGE_INTEGER& fileOffset = ULARGE_INTEGER{ 0 }
	) const
	{
		return MappedFileView<T>::MapViewOfFile(
			get(),
			dwDesiredAccess,
			fileOffset
		);
	}
};

} // namespace cxx
