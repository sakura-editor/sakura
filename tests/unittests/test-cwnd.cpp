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
#include <window/CWnd.h>

#include "MockUser32Dll.hpp"

using ::testing::_;
using ::testing::Return;

class CWndForOnCreateTest : public CWnd
{
public:
	CWndForOnCreateTest() = default;

	using CWnd::OnCreate;
};

TEST(CWnd, OnCreate)
{
	const auto   hWnd         = std::bit_cast<HWND>(static_cast<size_t>(101));
	const auto   hInstance    = std::bit_cast<HINSTANCE>(static_cast<size_t>(102));
	const auto   hMenu        = std::bit_cast<HMENU>(static_cast<size_t>(103));
	const auto   hWndParent   = std::bit_cast<HWND>(static_cast<size_t>(104));
	CREATESTRUCT createStruct = { nullptr, hInstance, hMenu, hWndParent, };

	CWndForOnCreateTest wnd;
	EXPECT_FALSE(wnd.OnCreate(nullptr, nullptr));
	EXPECT_FALSE(wnd.OnCreate(hWnd, nullptr));
	EXPECT_TRUE(wnd.OnCreate(hWnd, &createStruct));

	EXPECT_EQ(hInstance,  wnd.GetAppInstance());
	EXPECT_EQ(hWndParent, wnd.GetParentHwnd());
}

class CWndForCallDefWndProc : public CWnd
{
public:
	explicit CWndForCallDefWndProc(std::shared_ptr<User32Dll> User32Dll_)
		: CWnd(L"test", std::move(User32Dll_))
	{
	}

	using CWnd::CallDefWndProc;
};

TEST(CWnd, CallDefWndProc)
{
	const auto hWnd    = static_cast<HWND>(nullptr);
	const auto uMsg    = static_cast<UINT>(1);
	const auto wParam  = static_cast<WPARAM>(2);
	const auto lParam  = static_cast<LPARAM>(3);
	const auto lResult = static_cast<LRESULT>(4);

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, DefWindowProcW(hWnd, uMsg, wParam, lParam)).WillOnce(Return(lResult));

	CWndForCallDefWndProc wnd(std::move(pUser32Dll));
	EXPECT_EQ(lResult, wnd.CallDefWndProc(hWnd, uMsg, wParam, lParam));
}
