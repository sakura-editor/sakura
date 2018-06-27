/*!	@file
	@brief Cookieマネージャ

*/
/*
	Copyright (C) 2012, Moca

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

#ifndef SAKURA_CCOOKIE_MANAGER_H_
#define SAKURA_CCOOKIE_MANAGER_H_

#include <map>
#include <string>
#include "_os/OleTypes.h"

class CCookieManager
{
	typedef std::wstring wstring;


public:
	SysString GetCookie(LPCWSTR scope, LPCWSTR cookieName) const;
	SysString GetCookieDefault(LPCWSTR scope, LPCWSTR cookieName, LPCWSTR defVal, int len) const;
	int SetCookie(LPCWSTR scope, LPCWSTR cookieName, LPCWSTR val, int len);
	int DeleteCookie(LPCWSTR scope, LPCWSTR cookieName);
	SysString GetCookieNames(LPCWSTR scope) const;
	int DeleteAll(LPCWSTR scope);

private:
	std::map<wstring, wstring>* SelectCookieType(LPCWSTR scope) const;
	bool ValidateCookieName(LPCWSTR cookieName) const;

	std::map<wstring, wstring> m_cookieWindow;
	std::map<wstring, wstring> m_cookieDocument;
};

#endif
