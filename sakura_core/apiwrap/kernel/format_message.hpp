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
#pragma once

template<typename TAction = std::function<void(LPCWSTR)>>
void format_message(DWORD errorCode, const TAction& action)
{
	if (HLOCAL pMsg = nullptr;
		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_FROM_SYSTEM,
			nullptr,
			errorCode,
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			LPWSTR(&pMsg),
			0,
			nullptr
		))
	{
		action(LPCWSTR(pMsg));

		LocalFree(pMsg);
	}
}
