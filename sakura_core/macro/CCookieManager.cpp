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

#include "StdAfx.h"
#include "CCookieManager.h"


SysString CCookieManager::GetCookie(LPCWSTR scope, LPCWSTR cookieName) const
{
	const std::map<wstring, wstring>* cookies = SelectCookieType(scope);
	if( cookies == NULL ){
		return SysString(L"", 0);
	}
	wstring key = cookieName;
	const auto& keyVal = cookies->find(key);
	if( keyVal == cookies->end() ){
		return SysString(L"", 0);
	}
	return SysString(keyVal->second.c_str(), keyVal->second.length());
}

SysString CCookieManager::GetCookieDefault(LPCWSTR scope, LPCWSTR cookieName, LPCWSTR defVal, int len) const
{
	std::map<wstring, wstring>* cookies = SelectCookieType(scope);
	if( cookies == NULL ){
		return SysString(L"", 0);
	}
	wstring key = cookieName;
	const auto& keyVal = cookies->find(key);
	if( keyVal == cookies->end() ){
		return SysString(defVal, len);
	}
	return SysString(keyVal->second.c_str(), keyVal->second.length());
}

int CCookieManager::SetCookie(LPCWSTR scope, LPCWSTR cookieName, LPCWSTR val, int len)
{
	std::map<wstring, wstring>* cookies = SelectCookieType(scope);
	if( cookies == NULL ){
		return 1;
	}
	if( !ValidateCookieName(cookieName) ){
		return 2;
	}
	(*cookies)[cookieName] = wstring(val, len);
	return 0;
}

int CCookieManager::DeleteCookie(LPCWSTR scope, LPCWSTR cookieName)
{
	std::map<wstring, wstring>* cookies = SelectCookieType(scope);
	if( cookies == NULL ){
		return 1;
	}
	if( !ValidateCookieName(cookieName) ){
		return 2;
	}
	wstring key = cookieName;
	const auto& keyVal = cookies->find(key);
	if( keyVal == cookies->end() ){
		return 5;
	}
	cookies->erase(keyVal);
	return 0;
}

SysString CCookieManager::GetCookieNames(LPCWSTR scope) const
{
	const std::map<wstring, wstring>* cookies = SelectCookieType(scope);
	if( cookies == NULL ){
		return SysString(L"", 0);
	}
	auto& it = cookies->begin();
	wstring keyNames;
	if( it != cookies->end() ){
		keyNames += it->first;
		++it;
	}
	for(; it != cookies->end(); ++it){
		keyNames += L",";
		keyNames += it->first;
	}
	return SysString(keyNames.c_str(), keyNames.length());
}

int CCookieManager::DeleteAll(LPCWSTR scope)
{
	std::map<wstring, wstring>* cookies = SelectCookieType(scope);
	if( cookies == NULL ){
		return 1;
	}
	cookies->clear();
	return 0;
}

std::map<std::wstring, std::wstring>* CCookieManager::SelectCookieType(LPCWSTR scope) const
{
	if( 0 == wcscmp(scope, L"window") ){
		return const_cast<std::map<std::wstring, std::wstring>*>(&m_cookieWindow);
	}else if( 0 == wcscmp(scope, L"document") ){
		return const_cast<std::map<std::wstring, std::wstring>*>(&m_cookieDocument);
	}
	return NULL;
}

bool CCookieManager::ValidateCookieName(LPCWSTR cookieName) const
{
	for(int i = 0; cookieName[i] != L'\0'; i++){
		if( L'0' <= cookieName[i] && cookieName[i] <= L'9' ||
			L'a' <= cookieName[i] && cookieName[i] <= L'z' ||
			L'A' <= cookieName[i] && cookieName[i] <= L'Z' ||
			L'_' <= cookieName[i] ){
		}else{
			return false;
		}
	}
	return true;
}
