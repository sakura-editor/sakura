/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
