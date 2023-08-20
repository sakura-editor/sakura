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
#include "apiwrap/apiwrap.hpp"

#include "MockUser32Dll.hpp"

using ::testing::_;
using ::testing::Gt;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::StrEq;

TEST(apiwrap, MockedSetWindowTextW)
{
	const auto hWnd = (HWND)0x1234;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, SetWindowTextW(hWnd, StrEq(L"test")))
		.WillOnce(Return(true));

	EXPECT_TRUE(apiwrap::SetWindowTextW(hWnd, L"test", std::move(pMock)));
}

TEST(apiwrap, MockedSetWindowTextW_with_trunc)
{
	const auto hWnd = (HWND)0x1234;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, SetWindowTextW(hWnd, StrEq(L"test")))
		.WillOnce(Return(true));

	EXPECT_TRUE(apiwrap::SetWindowTextW(hWnd, std::wstring_view(L"test1", 4), std::move(pMock)));
}

TEST(apiwrap, MockedSetWindowTextW_with_null)
{
	const auto hWnd = (HWND)0x1234;
	const auto text = (LPCWSTR)nullptr;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, SetWindowTextW(hWnd, text))
		.WillOnce(Return(true));

	EXPECT_TRUE(apiwrap::SetWindowTextW(hWnd, text, std::move(pMock)));
}

TEST(apiwrap, MockedGetWindowTextW)
{
	const auto hWnd   = (HWND)0x1234;

	auto pMock = std::make_shared<MockUser32Dll>();
	::testing::Sequence sequence;
	EXPECT_CALL(*pMock, GetWindowTextLengthW(hWnd))
		.WillOnce(Return(1024));
	EXPECT_CALL(*pMock, GetWindowTextW(hWnd, _, Gt(1024)))
		.WillOnce(Invoke([](HWND, LPWSTR lpString, int nMaxCount)
			{
				wcscpy_s(lpString, nMaxCount, L"test");
				return 4;
			}));

	auto buffer = apiwrap::GetWindowTextW(hWnd, std::move(pMock));
}

TEST(apiwrap, MockedSetDlgItemTextW)
{
	const auto hWnd       = (HWND)0x1234;
	const auto nIDDlgItem = 109;
	const auto hItem      = (HWND)0x5678;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, GetDlgItem(hWnd, nIDDlgItem))
		.WillOnce(Return(hItem));
	EXPECT_CALL(*pMock, SetWindowTextW(hItem, StrEq(L"test")))
		.WillOnce(Return(true));

	EXPECT_TRUE(apiwrap::SetDlgItemTextW(hWnd, nIDDlgItem, L"test", std::move(pMock)));
}

TEST(apiwrap, MockedSetDlgItemTextW_fail)
{
	const auto hWnd       = (HWND)0x1234;
	const auto nIDDlgItem = 109;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, GetDlgItem(hWnd, nIDDlgItem))
		.WillOnce(Return(nullptr));

	EXPECT_FALSE(apiwrap::SetDlgItemTextW(hWnd, nIDDlgItem, L"test", std::move(pMock)));
}

TEST(apiwrap, MockedSetDlgItemTextW_with_trunc)
{
	const auto hWnd       = (HWND)0x1234;
	const auto nIDDlgItem = 109;
	const auto hItem      = (HWND)0x5678;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, GetDlgItem(hWnd, nIDDlgItem))
		.WillOnce(Return(hItem));
	EXPECT_CALL(*pMock, SetWindowTextW(hItem, StrEq(L"test")))
		.WillOnce(Return(true));

	EXPECT_TRUE(apiwrap::SetDlgItemTextW(hWnd, nIDDlgItem, std::wstring_view(L"test1", 4), std::move(pMock)));
}

TEST(apiwrap, MockedSetDlgItemTextW_with_trunc_fail)
{
	const auto hWnd       = (HWND)0x1234;
	const auto nIDDlgItem = 109;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, GetDlgItem(hWnd, nIDDlgItem))
		.WillOnce(Return(nullptr));

	EXPECT_FALSE(apiwrap::SetDlgItemTextW(hWnd, nIDDlgItem, std::wstring_view(L"test1", 4), std::move(pMock)));
}

TEST(apiwrap, MockedSetDlgItemTextW_with_null)
{
	const auto hWnd       = (HWND)0x1234;
	const auto nIDDlgItem = 109;
	const auto hItem      = (HWND)0x5678;
	const auto text       = (LPCWSTR)nullptr;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, GetDlgItem(hWnd, nIDDlgItem))
		.WillOnce(Return(hItem));
	EXPECT_CALL(*pMock, SetWindowTextW(hItem, text))
		.WillOnce(Return(true));

	EXPECT_TRUE(apiwrap::SetDlgItemTextW(hWnd, nIDDlgItem, text, std::move(pMock)));
}

TEST(apiwrap, MockedSetDlgItemTextW_with_null_fail)
{
	const auto hWnd       = (HWND)0x1234;
	const auto nIDDlgItem = 109;
	const auto text       = (LPCWSTR)nullptr;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, GetDlgItem(hWnd, nIDDlgItem))
		.WillOnce(Return(nullptr));

	EXPECT_FALSE(apiwrap::SetDlgItemTextW(hWnd, nIDDlgItem, text, std::move(pMock)));
}

TEST(apiwrap, MockedGetDlgItemTextW)
{
	const auto hWnd       = (HWND)0x1234;
	const auto nIDDlgItem = 109;
	const auto hItem      = (HWND)0x5678;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, GetDlgItem(hWnd, nIDDlgItem))
		.WillOnce(Return(hItem));
	EXPECT_CALL(*pMock, GetWindowTextLengthW(hItem))
		.WillOnce(Return(1024));
	EXPECT_CALL(*pMock, GetWindowTextW(hItem, _, Gt(1024)))
		.WillOnce(Invoke([](HWND, LPWSTR lpString, int nMaxCount)
			{
				wcscpy_s(lpString, nMaxCount, L"test");
				return 4;
			}));

	auto buffer = apiwrap::GetDlgItemTextW(hWnd, nIDDlgItem, std::move(pMock));
}

TEST(apiwrap, MockedGetDlgItemTextW_fail)
{
	const auto hWnd       = (HWND)0x1234;
	const auto nIDDlgItem = 109;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, GetDlgItem(hWnd, nIDDlgItem))
		.WillOnce(Return(nullptr));

	auto buffer = std::wstring(L"default string");
	buffer = apiwrap::GetDlgItemTextW(hWnd, nIDDlgItem, std::move(buffer), std::move(pMock));

	EXPECT_EQ(0, buffer.length());
}

TEST(apiwrap, MockedSendEmLimitTextW)
{
	const auto hWnd     = (HWND)0x1234;
	const auto cchLimit = 1024;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, SendMessageW(hWnd, EM_LIMITTEXT, cchLimit, 0L))
		.WillOnce(Return(0L));

	apiwrap::SendEmLimitTextW(hWnd, cchLimit, std::move(pMock));
}

TEST(apiwrap, MockedSendEmLimitTextW_dlgItem)
{
	const auto hWnd       = (HWND)0x1234;
	const auto nIDDlgItem = 109;
	const auto hItem      = (HWND)0x5678;
	const auto cchLimit   = 1024;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, GetDlgItem(hWnd, nIDDlgItem))
		.WillOnce(Return(hItem));
	EXPECT_CALL(*pMock, SendMessageW(hItem, EM_LIMITTEXT, cchLimit, 0L))
		.WillOnce(Return(0L));

	apiwrap::SendEmLimitTextW(hWnd, nIDDlgItem, cchLimit, std::move(pMock));
}

TEST(apiwrap, MockedSendEmLimitTextW_fail)
{
	const auto hWnd       = (HWND)0x1234;
	const auto nIDDlgItem = 109;
	const auto hItem      = (HWND)0x5678;
	const auto cchLimit   = 1024;

	auto pMock = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pMock, GetDlgItem(hWnd, nIDDlgItem))
		.WillOnce(Return(nullptr));
	EXPECT_CALL(*pMock, SendMessageW(hItem, EM_LIMITTEXT, cchLimit, 0L))
		.Times(0);

	apiwrap::SendEmLimitTextW(hWnd, nIDDlgItem, cchLimit, std::move(pMock));
}

TEST(IsWndClassRegistered, bad_arg)
{
	EXPECT_FALSE(apiwrap::IsWndClassRegistered(L""));
}
