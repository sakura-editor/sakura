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

#include "apiwrap/kernel/to_multibyte.hpp"

/*!
 * メッセージエラー
 * 
 * ワイド文字列でインスタンス化できるエラー。
 * 変換できない文字は '?' に置き換えられる。
 * @sa to_multibyte
 */
class message_error : public std::runtime_error {
private:
	std::wstring _Message;

public:
	explicit message_error(std::wstring_view message)
		: std::runtime_error(to_multibyte(message).c_str())
		, _Message(message)
	{
	}

	LPCWSTR message() const noexcept {
		return _Message.c_str();
	}
};
