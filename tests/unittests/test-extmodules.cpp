/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

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
#include <gtest/gtest.h>

#include "TExtModule.hpp"

#include "extmodule/CBregexpDll2.h"
#include "extmodule/CHtmlHelp.h"
#include "extmodule/CIcu4cI18n.h"
#include "extmodule/CMigemo.h"
#include "extmodule/CUchardet.h"
//#include "extmodule/CUxTheme.h"		//TSingletonなのでテストできない
#include "macro/CPPA.h"
//#include "plugin/CDllPlugin.h"		//継承不可なのでテストできない

/*!
	外部DLLの読み込みテスト
 */
template <typename T>
class LoadTest : public ::testing::Test {
};

//! パラメータテストであるとマークする
TYPED_TEST_CASE_P(LoadTest);

/*!
	読み込み失敗のテスト

	DLLが見つからなくて失敗するケース
 */
TYPED_TEST_P(LoadTest, FailedToLoadLibrary)
{
	// テスト対象のCDllImpl派生クラスをテストするための型を定義する
	using ExtModule = TUnresolvedExtModule<TypeParam>;

	// テストクラスをインスタンス化する
	ExtModule extModule;

	// DLLが見つからなくてIsAvailableはfalseになる
	EXPECT_FALSE(extModule.IsAvailable());
}

/*!
	読み込み失敗のテスト

	エクスポート関数が取得できなくて失敗するケース
 */
TYPED_TEST_P(LoadTest, FailedToGetProcAddress)
{
	// テスト対象のCDllImpl派生クラスをテストするための型を定義する
	using ExtModule = TUnsufficientExtModule<TypeParam>;

	// テストクラスをインスタンス化する
	ExtModule extModule;

	// エクスポート関数の一部が見つからなくてIsAvailableはfalseになる
	EXPECT_FALSE(extModule.IsAvailable());
}

// test suiteを登録する
REGISTER_TYPED_TEST_SUITE_P(
	LoadTest,
	FailedToLoadLibrary, FailedToGetProcAddress);

/*!
	テスト対象（外部DLLを読み込むクラス）

	CDllImp派生クラスである必要がある。
 */
using ExtModuleImplementations = ::testing::Types<
	CBregexpDll2,
	CHtmlHelp,
	CIcu4cI18n,
	CMigemo,
	CUchardet,
	CPPA>;

//! パラメータテストをインスタンス化する
INSTANTIATE_TYPED_TEST_SUITE_P(
	ExtModule,
	LoadTest,
	ExtModuleImplementations);
