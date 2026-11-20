/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"

#include <type_traits>

#include <Windows.h>

namespace cxx {

/*!
 * @brief OSの同期ハンドルをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 */
class HandleHolder : public cxx::ResourceHolder<&::CloseHandle>
{
private:
	using Base = cxx::ResourceHolder<&::CloseHandle>;
	using Me = HandleHolder;

public:
	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;

	virtual ~HandleHolder() = default;

	void lock()
	{
		Lock(INFINITE);	//無限に待つ
	}

	bool try_lock()
	{
		return Lock(0);	//ロック取得を試行
	}

	template<class Rep, class Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
	{
		const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(rel_time);
		return Lock(DWORD(milliseconds.count()));
	}

	bool unlock()
	{
		return Unlock();
	}

	virtual bool Lock(
		DWORD dwTimeout = INFINITE
	);

	virtual bool Unlock();

	Me& operator = (HANDLE t)
	{
		reset(t);
		return *this;
	}
};

DWORD WaitForSingleObject(
	_In_ HANDLE hHandle,
	_In_ DWORD dwMilliseconds
);

bool WaitForMultipleObjects(
	std::span<const HANDLE> handles,
	bool waitAll,
	DWORD timeout
);

template <class Rep, class Period>
bool WaitForMultipleObjects(
	std::span<const HANDLE> handles,
	bool waitAll,
	const std::chrono::duration<Rep, Period>& timeout
)
{
	const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timeout);

	return WaitForMultipleObjects(
		handles,
		waitAll,
		DWORD(milliseconds.count())
	);
}

DWORD MsgWaitForMultipleObjects(
	std::span<const HANDLE> handles,
	bool waitAll,
	DWORD timeout,
	DWORD wakeMask
);

} // namespace cxx

