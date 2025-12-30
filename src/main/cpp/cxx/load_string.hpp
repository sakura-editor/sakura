/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

namespace cxx {

std::wstring_view load_string(UINT id, const std::optional<HMODULE>& optModule = std::nullopt);

std::string load_string_as_acp(UINT id, const std::optional<HMODULE>& optModule = std::nullopt);

} // namespace cxx
