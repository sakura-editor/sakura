/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

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
#include "typeprop/CImpExpManager.h"

#include "MockShareDataAccessor.hpp"

/*!
 * 構築するだけ。
 */
TEST(CImpExpType, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	int         nIdx = 1;
	STypeConfig types = {};
	const auto  hwndList= (HWND)nullptr;
	EXPECT_NO_THROW({ CImpExpType dlg(nIdx, types, hwndList, std::move(pShareDataAccessor)); });
}
