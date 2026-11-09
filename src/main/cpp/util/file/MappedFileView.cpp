/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/file/MappedFileView.hpp"

#include "util/os/Kernel32.hpp"
#include "util/os/make_system_error.hpp"

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
)
{
	const auto pView = Kernel32::getInstance()->MapViewOfFile(
		hFileMappingObject,
		dwDesiredAccess,
		fileOffset.HighPart,
		fileOffset.LowPart,
		dwNumberOfBytesToMap
	);

	// 戻り値が NULL だった場合
	if (!pView) {
		// システムエラー例外を発生させる
		throw cxx::make_system_error("MapViewOfFile() failed");
	}

	return std::span{ std::bit_cast<std::byte*>(pView), dwNumberOfBytesToMap };
}

}	// namespace cxx
