/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif // if defined(_MSC_VER) && defined(_DEBUG)

#include "cxx/ResourceHolder.hpp"

namespace testing {

/*!
 * Cランタイムのメモリアロケーションをフックする
 */
template<class Ch, int _AllocType, size_t _Count = 1>
class CrtAllocHook
{
public:
	CrtAllocHook() = default;

private:

#if defined(_MSC_VER) && defined(_DEBUG)

	static int HookFunc(
		int nAllocType,
		void* pvData [[maybe_unused]],
		size_t nSize,
		int nBlockUse,
		long lRequest [[maybe_unused]],
		const unsigned char *szFileName [[maybe_unused]],
		int nLine [[maybe_unused]]
	)
	{
		if (nBlockUse == _CRT_BLOCK) {
			return TRUE;
		}

		if (nAllocType != _AllocType) {
			return TRUE;
		}

		if (nSize != sizeof(Ch) * _Count) {
			return TRUE;
		}

		return FALSE;
	}

	using allocHookHolder = cxx::ResourceHolder<&::_CrtSetAllocHook>;

	allocHookHolder _DefaultAllocHook{ ::_CrtSetAllocHook(&HookFunc) };

#endif // if defined(_MSC_VER) && defined(_DEBUG)
};

} // end of namespace testing
