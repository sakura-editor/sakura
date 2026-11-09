/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

// NtStatusカテゴリ（std::system_errorと組み合わせて使う）
class NtStatusCategory final : public std::error_category {
public:
	static const NtStatusCategory category;

	const char* name() const noexcept override
	{
		return "ntstatus";
	}

	std::string message(int value) const override
	{
		return std::format(
			"NTSTATUS 0x{:08X}",
			static_cast<std::uint32_t>(value)
		);
	}
};

inline const std::error_category& ntstatus_category() noexcept
{
	return NtStatusCategory::category;
}
