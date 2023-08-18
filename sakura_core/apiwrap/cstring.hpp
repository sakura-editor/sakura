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
#include "StdAfx.h"
#include "apiwrap/apiwrap.hpp"

namespace apiwrap {

/*!
 * c++の文字列参照とCのNUL終端文字配列の橋渡しをするクラス
 */
struct cstring
{
	std::wstring_view _View;
	std::wstring      _Buffer;

	explicit cstring(_In_opt_ LPCWSTR pszValue)
	{
		if (pszValue && *pszValue)
		{
			_View = pszValue;
		}
	}

	explicit cstring(std::wstring_view value)
		: _View(value)
	{
		if (0 < _View.length() && *(_View.data() + _View.length()))
		{
			_Buffer = _View;
			_View   = _Buffer;
		}
	}

	virtual ~cstring() = default;

	explicit virtual operator std::wstring_view() const noexcept
	{
		return _View;
	}
};

LPCWSTR get_psz_or_null(const cstring& str) noexcept;

} //end of namespace apiwrap
