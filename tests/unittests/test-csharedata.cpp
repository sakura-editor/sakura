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
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

#include "_main/CCommandLine.h"
#include "_main/CNormalProcess.h"

#include "doc/CEditDoc.h"
#include "view/CEditView.h"

#include "config/system_constants.h"

#include "MockKernel32Dll.hpp"
#include "MockShareDataAccessor.hpp"

using ::testing::_;
using ::testing::Return;

/*!
 * @brief CShareDataのテスト
 */
TEST( CShareData, test )
{
	// 共有メモリをインスタンス化するにはプロセスのインスタンスが必要。
	CNormalProcess cProcess(::GetModuleHandle(nullptr), L"");

	// 共有メモリのインスタンスを取得する
	auto pShareData = CShareData::getInstance();
	ASSERT_NE(nullptr, pShareData);

	// 共有メモリを初期化するにはコマンドラインのインスタンスが必要
	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(L"", false);

	// 共有メモリのインスタンスを初期化する
	ASSERT_TRUE(pShareData->InitShareData(cCommandLine.GetProfileName()));

	// 言語切り替えのテストを実施する
	std::vector<std::wstring> values;
	pShareData->ConvertLangValues(values, true);
	CSelectLang::ChangeLang(L"sakura_lang_en_US.dll");
	pShareData->ConvertLangValues(values, false);
	pShareData->RefreshString();
}

TEST(CShareData, InitShareData_CreateFileMapping_fail)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();

	auto pKernel32Dll = std::make_shared<MockKernel32Dll>();
	EXPECT_CALL(*pKernel32Dll, CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE | SEC_COMMIT, 0, sizeof(DLLSHAREDATA), _)).WillOnce(Return(nullptr));
	EXPECT_CALL(*pKernel32Dll, GetLastError()).Times(0);
	EXPECT_CALL(*pKernel32Dll, CloseHandle(_)).Times(0);

	EXPECT_CALL(*pShareDataAccessor, SetShareData(_)).Times(0);

	CShareData cShareData(pKernel32Dll, pShareDataAccessor);

	ASSERT_FALSE(cShareData.InitShareData(L"test"));
}

TEST(CShareData, InitShareData_MapViewOfFile_fail)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();

	const auto hFileMap = std::bit_cast<HANDLE>(static_cast<size_t>(0x69));

	auto pKernel32Dll = std::make_shared<MockKernel32Dll>();
	EXPECT_CALL(*pKernel32Dll, CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE | SEC_COMMIT, 0, sizeof(DLLSHAREDATA), _)).WillOnce(Return(hFileMap));
	EXPECT_CALL(*pKernel32Dll, GetLastError()).WillOnce(Return(ERROR_ALREADY_EXISTS));
	EXPECT_CALL(*pKernel32Dll, MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0)).WillOnce(Return(nullptr));
	EXPECT_CALL(*pKernel32Dll, UnmapViewOfFile(_)).Times(0);
	EXPECT_CALL(*pKernel32Dll, CloseHandle(hFileMap)).WillOnce(Return(TRUE));

	EXPECT_CALL(*pShareDataAccessor, SetShareData(_)).Times(0);

	CShareData cShareData(pKernel32Dll, pShareDataAccessor);

	ASSERT_FALSE(cShareData.InitShareData(L"test"));
}

TEST(CShareData, InitShareData_MapViewOfFile_returns_invalid)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	pDllShareData->m_nSize = sizeof(DLLSHAREDATA);
	pDllShareData->m_vStructureVersion = N_SHAREDATA_VERSION + 1;

	const auto hFileMap = std::bit_cast<HANDLE>(static_cast<size_t>(0x69));

	auto pKernel32Dll = std::make_shared<MockKernel32Dll>();
	EXPECT_CALL(*pKernel32Dll, CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE | SEC_COMMIT, 0, sizeof(DLLSHAREDATA), _)).WillOnce(Return(hFileMap));
	EXPECT_CALL(*pKernel32Dll, GetLastError()).WillOnce(Return(ERROR_ALREADY_EXISTS));
	EXPECT_CALL(*pKernel32Dll, MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0)).WillOnce(Return(pDllShareData.get()));
	EXPECT_CALL(*pKernel32Dll, UnmapViewOfFile(pDllShareData.get())).WillOnce(Return(TRUE));
	EXPECT_CALL(*pKernel32Dll, CloseHandle(hFileMap)).WillOnce(Return(TRUE));

	EXPECT_CALL(*pShareDataAccessor, SetShareData(_)).Times(0);

	CShareData cShareData(pKernel32Dll, pShareDataAccessor);

	ASSERT_FALSE(cShareData.InitShareData(L"test"));
}

TEST(CShareData, InitShareData_for_Editor)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	pDllShareData->m_nSize = sizeof(DLLSHAREDATA);
	pDllShareData->m_vStructureVersion = N_SHAREDATA_VERSION;

	const auto hFileMap = std::bit_cast<HANDLE>(static_cast<size_t>(0x69));

	auto pKernel32Dll = std::make_shared<MockKernel32Dll>();
	EXPECT_CALL(*pKernel32Dll, CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE | SEC_COMMIT, 0, sizeof(DLLSHAREDATA), _)).WillOnce(Return(hFileMap));
	EXPECT_CALL(*pKernel32Dll, GetLastError()).WillOnce(Return(ERROR_ALREADY_EXISTS));
	EXPECT_CALL(*pKernel32Dll, MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0)).WillOnce(Return(pDllShareData.get()));
	EXPECT_CALL(*pKernel32Dll, UnmapViewOfFile(pDllShareData.get())).WillOnce(Return(TRUE));
	EXPECT_CALL(*pKernel32Dll, CloseHandle(hFileMap)).WillOnce(Return(TRUE));

	::testing::Sequence sequence;
	EXPECT_CALL(*pShareDataAccessor, SetShareData(pDllShareData.get())).InSequence(sequence);
	EXPECT_CALL(*pShareDataAccessor, SetShareData(nullptr)).InSequence(sequence);

	CShareData cShareData(pKernel32Dll, pShareDataAccessor);

	ASSERT_TRUE(cShareData.InitShareData(L"test"));
}

TEST(CSearchKeywordManager, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	EXPECT_NO_THROW({ CSearchKeywordManager mgr(std::move(pShareDataAccessor)); });
}

TEST(CTagJumpManager, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	EXPECT_NO_THROW({ CTagJumpManager mgr(std::move(pShareDataAccessor)); });
}
