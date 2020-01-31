/*! @file */
/*
	Copyright (C) 2018-2020 Sakura Editor Organization

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

#include <string_view>

#include "extmodule/CIcu4cI18n.h"

/*!
 * @brief 文字コード検出クラス
 */
class CharsetDetector final
{
    CIcu4cI18n _icuin;
    UCharsetDetector *_csd;

  public:
    CharsetDetector() noexcept;
    ~CharsetDetector() noexcept;

    bool IsAvailable() const noexcept
    {
        return _icuin.IsAvailable();
    }

    ECodeType Detect(const std::string_view &bytes);
};
