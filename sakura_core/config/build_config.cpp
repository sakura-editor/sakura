/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "build_config.h"
#include <stdlib.h> //malloc,free

//デバッグ検証用：newされた領域をわざと汚す。2007.11.27 kobake
#ifdef FILL_STRANGE_IN_NEW_MEMORY

template <size_t srcCount>
inline void _fill_new_memory(void* ptr, size_t size, const char(&src)[srcCount])
{
	const size_t srcLength = srcCount - 1;
	size_t rest = size;
	for (auto dst = reinterpret_cast<char*>(ptr); rest;)
	{
		auto unit = std::min(srcLength, rest);
		memcpy_s(dst, rest, src, unit);
		dst += unit;
		rest -= unit;
	}
}

void* operator new(
	size_t const size,
	int const    block_use,
	char const*  file_name,
	int const    line_number
	)
{
	auto p = _malloc_dbg(size, block_use, file_name, line_number);
	_fill_new_memory(p, size, "n_e_w_!_"); //確保されたばかりのメモリ状態は「n_e_w_!_....」となります
	return p;
}

void* operator new[](size_t const size,
	int const    block_use,
	char const*  file_name,
	int const    line_number
	)
{
	auto p = operator new(size, block_use, file_name, line_number);
	_fill_new_memory(p, size, "N_E_W_!_"); //確保されたばかりのメモリ状態は「N_E_W_!_N_E_W_!_N_E_W_!_....」となります
	return p;
}
#endif
