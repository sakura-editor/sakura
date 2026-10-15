/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"

#include <span>
#include <string_view>
#include <type_traits>

namespace cxx {

/*!
 * @brief ファイルマッピングのビューを取得する
 *
 * @param[in] hFileMappingObject ファイルマッピングオブジェクトのハンドル
 * @param[in] dwDesiredAccess 要求するアクセス権
 * @param[in] fileOffset ファイルマッピングのビューの開始位置
 * @param[in] dwNumberOfBytesToMap マップするバイト数
 * @return マッピングされたデータ
 */
std::span<std::byte> MapViewOfFile(
	_In_ HANDLE hFileMappingObject,
	_In_ DWORD dwDesiredAccess,
	const ULARGE_INTEGER& fileOffset,
	_In_ DWORD dwNumberOfBytesToMap
);

/*!
 * @brief マッピングされたデータをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 */
template <typename T>
class MappedFileView : public cxx::ResourceHolder<&::UnmapViewOfFile, T>
{
private:
	using Base = cxx::ResourceHolder<&::UnmapViewOfFile, T>;
	using Me = MappedFileView;

public:
	/*!
	 * @brief マッピングされたデータを取得する
	 *
	 * @param[in] hFileMappingObject ファイルマッピングオブジェクトのハンドル
	 * @param[in] dwDesiredAccess 要求するアクセス権
	 * @param[in] maximumSize 最大サイズ
	 */
	static Me MapViewOfFile(
		_In_ HANDLE hFileMappingObject,
		_In_ DWORD dwDesiredAccess,
		const ULARGE_INTEGER& fileOffset = ULARGE_INTEGER{ 0 }
	)
	{
		// マッピングされたデータを返却する
		auto viewOfFile = cxx::MapViewOfFile(
			hFileMappingObject,
			dwDesiredAccess,
			fileOffset,
			sizeof(std::remove_pointer_t<T>)
		);

		return Me{ std::bit_cast<T>(viewOfFile.data()) };
	}

	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;
};

} // namespace cxx
