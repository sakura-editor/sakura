﻿/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_CFORMATMANAGER_4161FE80_FFA1_4619_BD0A_74FF4F59BDDA_H_
#define SAKURA_CFORMATMANAGER_4161FE80_FFA1_4619_BD0A_74FF4F59BDDA_H_
#pragma once

struct DLLSHAREDATA;
DLLSHAREDATA& GetDllShareData();

//!書式管理
class CFormatManager{
public:
	CFormatManager()
	{
		m_pShareData = &GetDllShareData();
	}
	//書式 //@@@ 2002.2.9 YAZAKI
	// 共有DLLSHAREDATA依存
	const WCHAR* MyGetDateFormat( const SYSTEMTIME& systime, WCHAR* pszDest, int nDestLen );
	const WCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, WCHAR* pszDest, int nDestLen );

	// 共有DLLSHAREDATA非依存
	const WCHAR* MyGetDateFormat( const SYSTEMTIME& systime, WCHAR* pszDest, int nDestLen, int nDateFormatType, const WCHAR* szDateFormat );
	const WCHAR* MyGetTimeFormat( const SYSTEMTIME& systime, WCHAR* pszDest, int nDestLen, int nTimeFormatType, const WCHAR* szTimeFormat );
private:
	DLLSHAREDATA* m_pShareData;
};
#endif /* SAKURA_CFORMATMANAGER_4161FE80_FFA1_4619_BD0A_74FF4F59BDDA_H_ */
