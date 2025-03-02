/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

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
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "util/design_template.h"


/*!
 * TSingleInstanceの挙動を検証するためのクラス
 */
class CSingleInstance : public TSingleInstance<CSingleInstance>
{
public:
	CSingleInstance() = default;
	virtual ~CSingleInstance() noexcept = default;
};

/*!
 * @brief TSingleInstanceの挙動を検証するテスト
 */
TEST(CSingleInstance, CSingleInstance)
{
	// インスタンスが生成される前にgetInstanceするとNULLが返る
	ASSERT_TRUE(CSingleInstance::getInstance() == nullptr);

	{
		// 1つ目のインスタンスを確保する
		CSingleInstance instance;

		// インスタンスが生成された後にgetInstanceするとNULL以外が返る
		ASSERT_FALSE(CSingleInstance::getInstance() == nullptr);

		// 返却されるポインタは、ローカルで確保したインスタンスと等しい
		ASSERT_EQ(&instance, CSingleInstance::getInstance());

		// 2つ目のインスタンスを確保しようとすると例外が発生する
		ASSERT_THROW({ CSingleInstance instance; }, std::domain_error);
	}

	// インスタンスが破棄された後にgetInstanceするとNULLが返る
	ASSERT_TRUE(CSingleInstance::getInstance() == nullptr);
}

/*!
 * TInstanceHolderの挙動を検証するためのクラス
 */
class CInstanceHolder : public TInstanceHolder<CInstanceHolder>
{
public:
	CInstanceHolder() = default;
	virtual ~CInstanceHolder() noexcept = default;
};

/*!
 * @brief TInstanceHolderの挙動を検証するテスト
 */
TEST(CInstanceHolder, CInstanceHolder)
{
	// インスタンスが生成される前にgetInstanceするとNULLが返る
	ASSERT_TRUE(CSingleInstance::getInstance() == nullptr);

	// インスタンスが生成される前にGetInstance(0)するとNULLが返る
	ASSERT_TRUE(CInstanceHolder::GetInstance(0) == nullptr);

	// インスタンスの数は0で開始する
	ASSERT_EQ(0, CInstanceHolder::GetInstanceCount());

	{
		// 1つ目のインスタンスを確保する
		CInstanceHolder instance;

		// インスタンスが生成され後にGetInstance(0)するとNULL以外が返る
		ASSERT_FALSE(CInstanceHolder::GetInstance(0) == nullptr);

		// 返却されるポインタは、ローカルで確保したインスタンスと等しい
		ASSERT_EQ(&instance, CInstanceHolder::GetInstance(0));
		ASSERT_EQ(&instance, CInstanceHolder::getInstance());

		// インスタンスの数は1になる
		ASSERT_EQ(1, CInstanceHolder::GetInstanceCount());

		{
			// 2つ目のインスタンスを確保する
			CInstanceHolder instance2;

			// インスタンスが生成された後にGetInstance(1)するとNULL以外が返る
			ASSERT_FALSE(CInstanceHolder::GetInstance(1) == nullptr);

			// 返却されるポインタは、ローカルで確保したインスタンスと等しい
			ASSERT_EQ(&instance2, CInstanceHolder::GetInstance(1));

			// インスタンスの数は2になる
			ASSERT_EQ(2, CInstanceHolder::GetInstanceCount());
		}

		// インスタンスが破棄された後は元に戻る
		ASSERT_FALSE(CInstanceHolder::GetInstance(0) == nullptr);
		ASSERT_EQ(&instance, CInstanceHolder::GetInstance(0));
		ASSERT_EQ(1, CInstanceHolder::GetInstanceCount());
	}

	// インスタンスが破棄された後は元に戻る
	ASSERT_TRUE(CSingleInstance::getInstance() == nullptr);
	ASSERT_TRUE(CInstanceHolder::GetInstance(0) == nullptr);
	ASSERT_EQ(0, CInstanceHolder::GetInstanceCount());
}
