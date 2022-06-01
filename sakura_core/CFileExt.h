﻿/*!	@file
	@brief オープンダイアログ用ファイル拡張子管理

	@author MIK
	@date 2003.5.12
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2018-2022, Sakura Editor Organization

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

#ifndef SAKURA_CFILEEXT_DEFC4B1D_6DA5_4C83_AA7E_198C3E0BC50F_H_
#define SAKURA_CFILEEXT_DEFC4B1D_6DA5_4C83_AA7E_198C3E0BC50F_H_
#pragma once

#include "_main/global.h"
#include "config/maxdata.h"
#include "util/design_template.h"

class CFileExt
{
public:
	CFileExt() = default;

	bool AppendExt( const WCHAR *pszName, const WCHAR *pszExt );
	bool AppendExtRaw( const WCHAR *pszName, const WCHAR *pszExt );
	const WCHAR *GetName( int nIndex );
	const WCHAR *GetExt( int nIndex );

	//ダイアログに渡す拡張子フィルタを取得する。(lpstrFilterに直接指定可能)
	//2回呼び出すと古いバッファが無効になることがあるのに注意
	const WCHAR *GetExtFilter( void );

	[[nodiscard]] int GetCount() const { return static_cast<int>(m_vFileExtInfo.size()); }

protected:
	// 2014.10.30 syat ConvertTypesExtToDlgExtをCDocTypeManagerに移動
	//bool ConvertTypesExtToDlgExt( const WCHAR *pszSrcExt, WCHAR *pszDstExt );

private:
	void CreateExtFilter(std::vector<WCHAR>& output) const;

	struct SFileExtInfo {
		std::wstring	m_sTypeName;	//名前
		std::wstring	m_sExt;			//拡張子
	};

	std::vector<SFileExtInfo>	m_vFileExtInfo;
	std::vector<WCHAR>	m_vstrFilter;

	DISALLOW_COPY_AND_ASSIGN(CFileExt);
};
#endif /* SAKURA_CFILEEXT_DEFC4B1D_6DA5_4C83_AA7E_198C3E0BC50F_H_ */
