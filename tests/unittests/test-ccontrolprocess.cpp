/*! @file */
/*
	Copyright (C) 2024, Sakura Editor Organization

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

#include "TMockProcess.hpp"

#include "eval_outputs.hpp"

#include "_main/CControlProcess.h"

#include "config/system_constants.h"

#include "_main/CNormalProcess.h"

/*!
 * @brief コントロールプロセスの異常系テスト
 */
using CControlProcessTest = TProcessTest<CControlProcess>;

TEST_F(CControlProcessTest, getEditorProcess)
{
	EXPECT_TRUE(CProcess::getInstance());
	EXPECT_FALSE(getEditorProcess());
}

// 多重起動制御ミューテックスの作成失敗
TEST_F(CControlProcessTest, InitializeProcess001)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Return(nullptr));
    EXPECT_CALL(*process, GetLastError()).Times(0);
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).Times(0);
    EXPECT_CALL(*process, InitShareData()).Times(0);

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, LS(STR_ERR_CTRLMTX1));
}

// 多重起動制御ミューテックスの作成失敗(すでに存在している)
TEST_F(CControlProcessTest, InitializeProcess002)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Invoke(DefaultCreateMutexW));
    EXPECT_CALL(*process, GetLastError()).WillOnce(Return(ERROR_ALREADY_EXISTS));
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).Times(0);
    EXPECT_CALL(*process, InitShareData()).Times(0);

	EXPECT_FALSE(process->InitializeProcess());
}

// 初期化完了イベントの作成失敗
TEST_F(CControlProcessTest, InitializeProcess011)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Invoke(DefaultCreateMutexW));
    EXPECT_CALL(*process, GetLastError()).WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).WillOnce(Return(nullptr));
    EXPECT_CALL(*process, InitShareData()).Times(0);

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, LS(STR_ERR_CTRLMTX2));
}

// 初期化完了イベントがすでに存在している
TEST_F(CControlProcessTest, InitializeProcess012)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Invoke(DefaultCreateMutexW));
    EXPECT_CALL(*process, GetLastError())
        .WillOnce(Return(ERROR_SUCCESS))
        .WillOnce(Return(ERROR_ALREADY_EXISTS));
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).WillOnce(Invoke(DefaultCreateEventW));
    EXPECT_CALL(*process, InitShareData()).Times(0);

	EXPECT_FALSE(process->InitializeProcess());
}

// 共有メモリの初期化失敗
TEST_F(CControlProcessTest, InitializeProcess021)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Invoke(DefaultCreateMutexW));
    EXPECT_CALL(*process, GetLastError())
        .WillOnce(Return(ERROR_SUCCESS))
        .WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).WillOnce(Invoke(DefaultCreateEventW));
    EXPECT_CALL(*process, InitShareData()).WillOnce(Return(false));

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, LS(STR_ERR_DLGPROCESS1));
}

// 初期化処理で発生したエラーメッセージを表示する
TEST_F(CControlProcessTest, Run001)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Invoke(DefaultCreateMutexW));
    EXPECT_CALL(*process, GetLastError())
        .WillOnce(Return(ERROR_SUCCESS))
        .WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).WillOnce(Invoke(DefaultCreateEventW));
    EXPECT_CALL(*process, InitShareData()).WillOnce(Return(false));

	EXPECT_ERROUT(process->Run(), LS(STR_ERR_DLGPROCESS1));
}
