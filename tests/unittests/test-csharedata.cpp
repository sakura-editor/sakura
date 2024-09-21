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
#include "pch.h"

#include "env/CShareData.h"

#include "_main/CProcessFactory.h"

#include "apiwrap/kernel/message_error.hpp"

#include "eval_outputs.hpp"

#define ASSERT_THROW_MESSAGE(statement, expected_exception, expected_message) \
	try { \
		statement; \
		FAIL() << "Expected " << #expected_exception << "."; \
	} \
	catch (const expected_exception& e) { \
		EXPECT_STREQ(expected_message, e.message()); \
	} \
	(void)0

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

std::wstring GetSharaDataName(std::optional<LPCWSTR> profileName);

// 言語選択（呼ぶだけ）
TEST(CShareData, ChangeLang)
{
	const auto process = CProcessFactory().CreateInstance( L"-NOWIN");

	// 共有メモリのインスタンスを取得する
	auto pShareData = CShareData::getInstance();
	EXPECT_NE(nullptr, pShareData);

	EXPECT_TRUE(pShareData->InitShareData());

	// 言語切り替えのテストを実施する
	std::vector<std::wstring> values;
	pShareData->ConvertLangValues(values, true);
	CSelectLang::ChangeLang(L"sakura_lang_en_US.dll");
	pShareData->ConvertLangValues(values, false);
	pShareData->RefreshString();
}

struct file_mapping_of_share_data : public file_mapping<DLLSHAREDATA>
{
	MOCK_CONST_METHOD6(CreateFileMappingW, HANDLE(
		_In_     HANDLE hFile,
		_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
		_In_     DWORD flProtect,
		_In_     DWORD dwMaximumSizeHigh,
		_In_     DWORD dwMaximumSizeLow,
		std::wstring_view name
	));

	MOCK_CONST_METHOD0(GetLastError, DWORD());
};

// CreateFileMappingW失敗
TEST(file_mapping, CreateMapping001)
{
	const auto mapping = std::make_shared<file_mapping_of_share_data>();
    EXPECT_CALL(*mapping, CreateFileMappingW(_, _, _, _, _, _)).WillOnce(Return(nullptr));
    EXPECT_CALL(*mapping, GetLastError()).Times(0);

	ASSERT_THROW_MESSAGE(mapping->CreateMapping(nullptr, nullptr, 0, 0, 0, L"test"), message_error, L"CreateFileMapping()に失敗しました");
}

struct MockCProcess : public CProcess
{
	constexpr static auto& MESSAGE = L"異なるバージョンのエディタを同時に起動することはできません。";

	using CProcess::CProcess;

	MOCK_METHOD0(InitializeProcess, bool());

	MOCK_METHOD0(InitProcess, void());
};

TEST(CProcess, Run_failWithMessage)
{
	auto process = std::make_unique<MockCProcess>(nullptr, std::make_unique<CCommandLine>(), SW_SHOWDEFAULT);
	EXPECT_CALL(*process, InitializeProcess()).WillOnce(Invoke([]() -> bool { throw message_error(MockCProcess::MESSAGE); }));
	EXPECT_ERROUT(process->Run(), MockCProcess::MESSAGE);
}

TEST(CShareData, InitShareData)
{
	file_mapping<DLLSHAREDATA> dummy;

	const bool isCreated = dummy.CreateMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE | SEC_COMMIT,
		0,
		sizeof(DLLSHAREDATA),
		GetSharaDataName(std::nullopt)
	);
	ASSERT_TRUE(isCreated);

	auto pShareData = dummy.m_pData.get();
	ASSERT_NE(nullptr, pShareData);

	pShareData->m_vStructureVersion = 1;
	pShareData->m_nSize = sizeof(DLLSHAREDATA) + 1;

	auto process = CProcessFactory().CreateInstance(L"-NOWIN");
	auto shareData = &process->GetCShareData();
	ASSERT_THROW_MESSAGE(shareData->InitShareData(), message_error, MockCProcess::MESSAGE);
}
