/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "basis/CMyString.h" //CFilePath
#include "basis/primitive.h"
#include "cxx/ResourceHolder.hpp"
#include "util/file/FileHandle.hpp"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

#include <Windows.h>

namespace cxx {

class NamedFilePointer;

/*!
 * @brief Cストリームをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 * fopen() で開いた Cストリーム(FILE*) を RAII っぽく扱えるようにする。
 */
class FilePointer : public cxx::ResourceHolder<&::fclose>
{
private:
	using Base = cxx::ResourceHolder<&::fclose>;
	using Me = FilePointer;

public:
	/*!
	 * @brief 新しいファイルを作って Cストリーム を開く
	 *
	 * @param[in] path 作成するファイルのパス
	 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
	 */
	static NamedFilePointer CreateFilePath(
		std::wstring_view path
	);

	/*!
	 * @brief 新しいファイルを作って Cストリーム を開く
	 *
	 * @param[in] path 作成するファイルのパス
	 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	static NamedFilePointer CreateFilePath(
		const A& path
	);

	/*!
	 * @brief 一時ファイルを作成する
	 *
	 * @param[in, opt] optPrefix ファイル名の前に付ける3文字の接頭辞。
	 * @param[in, opt] optTempDir 一時フォルダーのパス。指定しない場合はシステムの一時フォルダーを使う。
	 * @param[in, opt] dwFlagsAndAttributes ファイル属性と作成フラグ。指定しない場合は FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE を使う。
	 * @param[in, opt] optExt 一時ファイルの拡張子。指定しない場合は "tmp" を使う。
	 */
	static NamedFilePointer CreateTempFile(
		const std::optional<std::wstring>& optPrefix = std::nullopt,
		const std::optional<std::wstring>& optTempDir = std::nullopt,
		DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
		const std::optional<std::wstring>& optExt = std::nullopt
	);

	/*!
	 * @brief OSのファイルハンドルから Cストリーム を開く
	 *
	 * @param[in] hFile OSのファイルハンドル
	 * @param[in] mode _wfdopen() に渡すモード文字列。
	 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
	 */
	static Me OpenFileHandle(
		FileHandle&& hFile,
		std::wstring_view mode
	);

	/*!
	 * @briefパスを指定して Cストリーム を開く
	 *
	 * @param[in] path ファイルパス
	 * @param[in] mode fopen() のモード文字列
	 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
	 */
	static NamedFilePointer OpenFilePath(
		std::wstring_view path,
		std::wstring_view mode
	);

	/*!
	 * @briefパスを指定して Cストリーム を開く
	 *
	 * @param[in] path ファイルパス
	 * @param[in] mode fopen() のモード文字列
	 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	static NamedFilePointer OpenFilePath(
		const A& path,
		std::wstring_view mode
	);

	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;

	FilePointer(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	FilePointer(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

	virtual ~FilePointer() = default;

	/*!
	 * @brief ファイルに書き込めるかどうかをチェックする
	 *
	 * @throw std::system_error ファイルサイズが大きすぎるとき
	 * @throw std::system_error 書き込みテストに失敗したとき
	 */
	void check_write_permissions() const;

	/*!
	 * @brief ストリームからデータを読み込む
	 *
	 * @param[in, out] buffer 読み込むバッファー
	 */
	void read(
		std::string& buffer
	) const;

	/*!
	 * @brief 指定した位置にファイルポインターを移動する
	 *
	 * @param[in] offset 移動先の位置
	 */
	void seek(
		long offset
	) const;

	/*!
	 * @brief ストリームにデータを書き込む
	 *
	 * @param[in] data 書き込むデータ
	 */
	void write(
		std::string_view data
	) const;
};

} // namespace cxx
