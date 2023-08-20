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
#include "window/CCustomWnd.hpp"

#include "MockUser32Dll.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

/*!
 * LPWNDCLASSEXWのメンバーをチェックするためのマッチャー定義
 *
 * lpfnWndProcについては、想定値(CCustomWnd::WndProc)の取得が面倒なのではチェックしない
 */
MATCHER_P7(WndClassEx,
	className, hCursor, hbrBackground, uStyles, hIcon, hIconSm, cbWndExtra,
	"LPWNDCLASSEXW may be invalid.")
{
	return arg
		&& arg->cbSize        == sizeof(WNDCLASSEXW)
		&& arg->style         == uStyles
		&& arg->lpfnWndProc
		&& !arg->cbClsExtra
		&& arg->cbWndExtra    == cbWndExtra
		&& !arg->hInstance
		&& arg->hIcon         == hIcon
		&& arg->hCursor       == hCursor
		&& arg->hbrBackground == hbrBackground
		&& !arg->lpszMenuName
		&& arg->lpszClassName == className
		&& arg->hIconSm       == hIconSm;
}

/*!
 * 構築するだけ。
 */
TEST(CCustomWnd, Construct)
{
	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_NO_THROW({ CCustomWnd wnd(std::move(pUser32Dll)); });
}

TEST(CCustomWnd, RegisterWnd_with_bad_classname)
{
	const auto hWndParent = (HWND)103;

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, GetClassInfoExW(_, _, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, RegisterClassExW(_)).Times(0);
	EXPECT_CALL(*pUser32Dll, CreateWindowExW(_, _, _, _, _, _, _, _, _, _, _, _)).Times(0);

	CCustomWnd wnd(std::move(pUser32Dll));

	EXPECT_FALSE(wnd.RegisterWnd(L"", static_cast<HCURSOR>(nullptr), MakeHBrush(COLOR_WINDOW)));

	EXPECT_FALSE(wnd.CreateWnd(hWndParent));
}

TEST(CCustomWnd, RegisterClass_fail)
{
	const auto        hInstance     = static_cast<HINSTANCE>(nullptr);
	std::wstring_view className     = L"TestWnd";
	const auto        hbrBackground = MakeHBrush(COLOR_WINDOW);
	const auto        hCursor       = LoadSysCursor(IDC_ARROW);
	UINT              uStyles       = CS_HREDRAW | CS_VREDRAW;
	const auto        hIcon         = static_cast<HICON>(nullptr);
	const auto        hIconSm       = static_cast<HICON>(nullptr);
	const auto        cbWndExtra    = sizeof(LPARAM);

	const auto        hWndParent = (HWND)103;

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, GetClassInfoExW(hInstance, StrEq(className.data()), _)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, RegisterClassExW(WndClassEx(className, hCursor, hbrBackground, uStyles, hIcon, hIconSm, cbWndExtra))).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, CreateWindowExW(_, _, _, _, _, _, _, _, _, _, _, _)).Times(0);

	CCustomWnd wnd(std::move(pUser32Dll));

	EXPECT_FALSE(wnd.RegisterWnd(className, hCursor, hbrBackground, uStyles, hIcon, hIconSm, cbWndExtra));

	EXPECT_FALSE(wnd.CreateWnd(hWndParent));
}

TEST(CCustomWnd, RegisterWnd02)
{
	const auto        hInstance     = static_cast<HINSTANCE>(nullptr);
	std::wstring_view className     = L"TestWnd";
	const auto        hbrBackground = MakeHBrush(COLOR_WINDOW);
	const auto        hCursor       = LoadSysCursor(IDC_ARROW);
	UINT              uStyles       = CS_HREDRAW | CS_VREDRAW;
	const auto        hIcon         = static_cast<HICON>(nullptr);
	const auto        hIconSm       = static_cast<HICON>(nullptr);
	const auto        cbWndExtra    = sizeof(LPARAM);

	DWORD             dwExStyle   = 101;
	std::wstring_view windowTitle = L"WindowTitle";
	DWORD             dwStyle     = 102;
	LONG              x           = CW_USEDEFAULT;
	LONG              y           = CW_USEDEFAULT;
	LONG              cx          = CW_USEDEFAULT;
	LONG              cy          = CW_USEDEFAULT;
	const auto        hWndParent  = std::bit_cast<HWND>(static_cast<size_t>(103));
	const auto        hMenu       = std::bit_cast<HMENU>(static_cast<size_t>(104));

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, GetClassInfoExW(hInstance, StrEq(className.data()), _)).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, RegisterClassExW(_)).Times(0);
	EXPECT_CALL(*pUser32Dll, CreateWindowExW(dwExStyle, StrEq(className.data()), StrEq(windowTitle.data()), dwStyle, x, y, cx, cy, hWndParent, hMenu, hInstance, _)).WillOnce(Return(nullptr));

	CCustomWnd wnd(std::move(pUser32Dll));

	ASSERT_TRUE(wnd.RegisterWnd(className, hCursor, hbrBackground, uStyles, hIcon, hIconSm, cbWndExtra));

	EXPECT_FALSE(wnd.CreateWnd(hWndParent, windowTitle, dwStyle, dwExStyle, static_cast<UINT>(reinterpret_cast<size_t>(hMenu)), nullptr));
}

TEST(CCustomWnd, RegisterWnd03)
{
	const auto        hInstance     = static_cast<HINSTANCE>(nullptr);
	std::wstring_view className     = L"TestWnd";
	const auto        hbrBackground = MakeHBrush(COLOR_WINDOW);
	const auto        hCursor       = LoadSysCursor(IDC_ARROW);
	UINT              uStyles       = CS_HREDRAW | CS_VREDRAW;
	const auto        hIcon         = static_cast<HICON>(nullptr);
	const auto        hIconSm       = static_cast<HICON>(nullptr);
	const auto        cbWndExtra    = sizeof(LPARAM);

	DWORD             dwExStyle   = 101;
	std::wstring_view windowTitle = L"WindowTitle";
	DWORD             dwStyle     = 102;
	LONG              x           = CW_USEDEFAULT;
	LONG              y           = CW_USEDEFAULT;
	LONG              cx          = CW_USEDEFAULT;
	LONG              cy          = CW_USEDEFAULT;
	const auto        hWndParent  = std::bit_cast<HWND>(static_cast<size_t>(103));
	const auto        hMenu       = std::bit_cast<HMENU>(static_cast<size_t>(104));
	const auto        hWnd        = std::bit_cast<HWND>(static_cast<size_t>(105));

	RECT rcWin = { x, y, cx, cy };

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, GetClassInfoExW(hInstance, StrEq(className.data()), _)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, RegisterClassExW(WndClassEx(className, hCursor, hbrBackground, uStyles, hIcon, hIconSm, cbWndExtra))).WillOnce(Return(TRUE));
	EXPECT_CALL(*pUser32Dll, CreateWindowExW(dwExStyle, StrEq(className.data()), StrEq(windowTitle.data()), dwStyle, x, y, cx, cy, hWndParent, hMenu, hInstance, _)).WillOnce(Return(hWnd));

	CCustomWnd wnd(std::move(pUser32Dll));

	ASSERT_TRUE(wnd.RegisterWnd(className, hCursor, hbrBackground, uStyles, hIcon, hIconSm, cbWndExtra));

	EXPECT_EQ(hWnd, wnd.CreateWnd(hWndParent, windowTitle, dwStyle, dwExStyle, static_cast<UINT>(reinterpret_cast<size_t>(hMenu)), &rcWin));
}

class CCreationFailedSample : public CCustomWnd
{
public:
	CCreationFailedSample() = default;

	bool OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) override
	{
		return WndProc(nullptr, WM_NULL, 0, 0);
	}
};

TEST(CCustomWnd, CreationFail)
{
	std::wstring_view className     = L"TestWnd";
	const auto        hbrBackground = MakeHBrush(COLOR_WINDOW);
	const auto        hCursor       = LoadSysCursor(IDC_ARROW);
	const auto        hWndParent    = static_cast<HWND>(nullptr);

	CCreationFailedSample wnd;

	ASSERT_TRUE(wnd.RegisterWnd(className, hCursor, hbrBackground));

	const auto hWnd = wnd.CreateWnd(hWndParent);
	EXPECT_FALSE(hWnd);
}

TEST(CCustomWnd, CreateWnd)
{
	std::wstring_view className     = L"test-popup";
	const auto        hbrBackground = MakeHBrush(COLOR_WINDOW);
	const auto        hCursor       = LoadSysCursor(IDC_ARROW);
	const auto        hWndParent    = static_cast<HWND>(nullptr);

	CCustomWnd wnd;

	ASSERT_TRUE(wnd.RegisterWnd(className, hCursor, hbrBackground));

	const auto hWnd = wnd.CreateWnd(hWndParent);
	EXPECT_TRUE(hWnd);

	if (hWnd) {
		DestroyWindow(hWnd);
	}
}

class CCustomWndForOnCreateTest : public CCustomWnd
{
public:
	CCustomWndForOnCreateTest() = default;

	using CCustomWnd::OnCreate;
};

TEST(CCustomWnd, OnCreate)
{
	const auto   hWnd         = std::bit_cast<HWND>(static_cast<size_t>(101));
	CREATESTRUCT createStruct = {};

	CCustomWndForOnCreateTest wnd;

	EXPECT_FALSE(wnd.OnCreate(nullptr, nullptr));
	EXPECT_FALSE(wnd.OnCreate(hWnd, nullptr));
	EXPECT_TRUE(wnd.OnCreate(hWnd, &createStruct));
}
