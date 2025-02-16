/*!	@file
	@brief Cookieマネージャ

*/
/*
	Copyright (C) 2012, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
	auto keyVal = cookies->find(key);
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
	auto it = cookies->begin();
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
		if( (L'0' <= cookieName[i] && cookieName[i] <= L'9') ||
			(L'a' <= cookieName[i] && cookieName[i] <= L'z') ||
			(L'A' <= cookieName[i] && cookieName[i] <= L'Z') ||
			L'_' <= cookieName[i] ){
		}else{
			return false;
		}
	}
	return true;
}
