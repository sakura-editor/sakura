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
	const std::map<sakura::wstring, sakura::wstring>* cookies = SelectCookieType(scope);
	if( cookies == nullptr ){
		return SysString(LTEXT(""), 0);
	}
	sakura::wstring key = cookieName;
	auto keyVal = cookies->find(key);
	if( keyVal == cookies->end() ){
		return SysString(LTEXT(""), 0);
	}
	return SysString(keyVal->second.c_str(), keyVal->second.length());
}

SysString CCookieManager::GetCookieDefault(LPCWSTR scope, LPCWSTR cookieName, LPCWSTR defVal, int len) const
{
	std::map<sakura::wstring, sakura::wstring>* cookies = SelectCookieType(scope);
	if( cookies == nullptr ){
		return SysString(LTEXT(""), 0);
	}
	sakura::wstring key = cookieName;
	const auto& keyVal = cookies->find(key);
	if( keyVal == cookies->end() ){
		return SysString(defVal, len);
	}
	return SysString(keyVal->second.c_str(), keyVal->second.length());
}

int CCookieManager::SetCookie(LPCWSTR scope, LPCWSTR cookieName, LPCWSTR val, int len)
{
	std::map<sakura::wstring, sakura::wstring>* cookies = SelectCookieType(scope);
	if( cookies == nullptr ){
		return 1;
	}
	if( !ValidateCookieName(cookieName) ){
		return 2;
	}
	(*cookies)[cookieName] = sakura::wstring(val, len);
	return 0;
}

int CCookieManager::DeleteCookie(LPCWSTR scope, LPCWSTR cookieName)
{
	std::map<sakura::wstring, sakura::wstring>* cookies = SelectCookieType(scope);
	if( cookies == nullptr ){
		return 1;
	}
	if( !ValidateCookieName(cookieName) ){
		return 2;
	}
	sakura::wstring key = cookieName;
	const auto& keyVal = cookies->find(key);
	if( keyVal == cookies->end() ){
		return 5;
	}
	cookies->erase(keyVal);
	return 0;
}

SysString CCookieManager::GetCookieNames(LPCWSTR scope) const
{
	const std::map<sakura::wstring, sakura::wstring>* cookies = SelectCookieType(scope);
	if( cookies == nullptr ){
		return SysString(LTEXT(""), 0);
	}
	auto it = cookies->begin();
	sakura::wstring keyNames;
	if( it != cookies->end() ){
		keyNames += it->first;
		++it;
	}
	for(; it != cookies->end(); ++it){
		keyNames += LTEXT(",");
		keyNames += it->first;
	}
	return SysString(keyNames.c_str(), keyNames.length());
}

int CCookieManager::DeleteAll(LPCWSTR scope)
{
	std::map<sakura::wstring, sakura::wstring>* cookies = SelectCookieType(scope);
	if( cookies == nullptr ){
		return 1;
	}
	cookies->clear();
	return 0;
}

std::map<sakura::wstring, sakura::wstring>* CCookieManager::SelectCookieType(LPCWSTR scope) const
{
	if( 0 == wcscmp(scope, LTEXT("window")) ){
		return const_cast<std::map<sakura::wstring, sakura::wstring>*>(&m_cookieWindow);
	}else if( 0 == wcscmp(scope, LTEXT("document")) ){
		return const_cast<std::map<sakura::wstring, sakura::wstring>*>(&m_cookieDocument);
	}
	return nullptr;
}

bool CCookieManager::ValidateCookieName(LPCWSTR cookieName) const
{
	for(int i = 0; cookieName[i] != LCHAR('\0'); i++){
		if( (LCHAR('0') <= cookieName[i] && cookieName[i] <= LCHAR('9')) ||
			(LCHAR('a') <= cookieName[i] && cookieName[i] <= LCHAR('z')) ||
			(LCHAR('A') <= cookieName[i] && cookieName[i] <= LCHAR('Z')) ||
			LCHAR('_') <= cookieName[i] ){
		}else{
			return false;
		}
	}
	return true;
}
