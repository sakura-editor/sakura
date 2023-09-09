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

#include "apimodule/Ole32Dll.hpp"

#include <gmock/gmock.h>

struct MockOle32Dll : public Ole32Dll
{
	MOCK_CONST_METHOD5(CoCreateInstance, HRESULT(
		_In_ REFCLSID rclsid,
		_In_opt_ LPUNKNOWN pUnkOuter,
		_In_ DWORD dwClsContext,
		_In_ REFIID riid,
		_COM_Outptr_ _At_(*ppv, _Post_readable_size_(_Inexpressible_(varies))) LPVOID  FAR* ppv));
};
