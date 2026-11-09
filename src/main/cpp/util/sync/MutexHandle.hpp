/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "basis/primitive.h"
#include "cxx/ResourceHolder.hpp"

#include <string_view>
#include <type_traits>

#include <Windows.h>

namespace cxx {

/*!
 * @brief OSのミューテックスハンドルをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 */
class MutexHandle : public cxx::ResourceHolder<&::CloseHandle>
{
private:
	using Base = cxx::ResourceHolder<&::CloseHandle>;
	using Me = MutexHandle;

public:
	/*!
	 * @brief ミューテックスハンドルを作成する
	 *
	 * @param[in] lpMutexAttributes セキュリティ属性
	 * @param[in] bInitialOwner 初期所有者フラグ
	 * @param[in] name ミューテックス名
	 */
	static Me CreateMutexW(
		_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
		bool bInitialOwner,
		std::wstring_view name
	);

	/*!
	 * @brief ミューテックスハンドルを作成する
	 *
	 * @param[in] lpMutexAttributes セキュリティ属性
	 * @param[in] bInitialOwner 初期所有者フラグ
	 * @param[in] name ミューテックス名
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	static Me CreateMutexW(
		_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
		bool bInitialOwner,
		const A& name
	)
	{
		// 引数はNUL終端文字列として扱う
		cxx::NullTerminatedString<WCHAR> _Name{ name };

		// 文字列を渡すバージョンを呼び出す
		return CreateMutexW(
			lpMutexAttributes,
			bInitialOwner,
			_Name.str()
		);
	}

	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;
};

} // namespace cxx
