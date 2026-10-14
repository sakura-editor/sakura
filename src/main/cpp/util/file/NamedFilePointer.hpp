/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "basis/CMyString.h" //SFilePath
#include "basis/primitive.h"
#include "cxx/ResourceHolder.hpp"
#include "util/file/FilePointer.hpp"

#include <span>
#include <string>
#include <string_view>
#include <type_traits>

namespace cxx {

/*!
 * @brief パス付き Cストリーム をラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 * fopen() で開いた Cストリーム(FILE*) を RAII っぽく扱えるようにする。
 * Cストリームとファイルパスを紐付ける拡張を施したもの。
 */
class NamedFilePointer : public cxx::FilePointer
{
private:
	using Base = cxx::FilePointer;
	using Me = NamedFilePointer;

public:
	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;

	explicit NamedFilePointer(
		FilePointer&& fp,
		std::wstring_view path
	);

	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	explicit NamedFilePointer(
		FilePointer&& fp,
		const A& path
	)
		: NamedFilePointer(
			std::move(fp),
			cxx::NullTerminatedString<WCHAR>{ path }.str()
		)
	{
	}

	NamedFilePointer(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	NamedFilePointer(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

	std::wstring GetPath() const { return std::wstring{ m_Path.str() }; }

private:
	SFilePath	m_Path;
};

template <basis::NullTerminatedStringConstructible<WCHAR> A>
	requires (!std::is_same_v<A, std::wstring_view>)
NamedFilePointer FilePointer::CreateFilePath(
	const A& path
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> fileName{ path };

	// 文字列を渡すバージョンを呼び出す
	return CreateFilePath(
		fileName.str()
	);
}

template <basis::NullTerminatedStringConstructible<WCHAR> A>
	requires (!std::is_same_v<A, std::wstring_view>)
NamedFilePointer FilePointer::OpenFilePath(
	const A& path,
	std::wstring_view mode
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> fileName{ path };

	// 文字列を渡すバージョンを呼び出す
	return OpenFilePath(
		fileName.str(),
		mode
	);
}

} // namespace cxx
