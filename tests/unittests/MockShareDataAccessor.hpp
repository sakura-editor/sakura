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
#pragma once

#include "env/ShareDataAccessor.hpp"
#include "env/DLLSHAREDATA.h"

#include <gmock/gmock.h>
#include <tuple>

struct MockShareDataAccessor : public ShareDataAccessor
{
	~MockShareDataAccessor() override = default;

	MOCK_CONST_METHOD0(GetShareData, DLLSHAREDATA*());
	MOCK_CONST_METHOD1(SetShareData, void(DLLSHAREDATA*));
};

/*!
 * ダミーの共有メモリを作成する
 */
inline auto MakeDummyShareData()
{
	// アクセサのモックを生成する
	auto pShareDataAccessor = std::make_shared<MockShareDataAccessor>();

	// ダミー共有メモリをnewする
	auto pDllShareData = std::make_shared<DLLSHAREDATA>();

	// ダミー共有メモリをモックに設定する
	EXPECT_CALL(*pShareDataAccessor, GetShareData())
		.WillRepeatedly(::testing::Return(pDllShareData.get()));

	// ダミー共有メモリとアクセサをtupleとして返却する
	return std::make_tuple(std::move(pDllShareData), std::move(pShareDataAccessor));
}
