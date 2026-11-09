/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/file/NamedFilePointer.hpp"

#include <string_view>

namespace cxx {

//! Cストリームとパスを紐付ける
NamedFilePointer::NamedFilePointer(
	FilePointer&& file,
	std::wstring_view path
)
	: Base(std::move(file))
	, m_Path{ path }
{
}

} // namespace cxx
