/*!	@file
	@brief Cookieマネージャ

*/
/*
	Copyright (C) 2012, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CCOOKIEMANAGER_69A96E71_61BD_417E_BD29_10B3D27AA11D_H_
#define SAKURA_CCOOKIEMANAGER_69A96E71_61BD_417E_BD29_10B3D27AA11D_H_
#pragma once

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
#endif /* SAKURA_CCOOKIEMANAGER_69A96E71_61BD_417E_BD29_10B3D27AA11D_H_ */
