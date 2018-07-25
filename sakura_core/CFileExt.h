/*!	@file
	@brief オープンダイアログ用ファイル拡張子管理

	@author MIK
	@date 2003.5.12
*/
/*
	Copyright (C) 2003, MIK

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

#ifndef SAKURA_CFILEEXT_H_
#define SAKURA_CFILEEXT_H_

#include "_main/global.h"
#include "config/maxdata.h"
#include "util/design_template.h"

class CFileExt
{
public:
	CFileExt();
	~CFileExt();

	bool AppendExt( const TCHAR *pszName, const TCHAR *pszExt );
	bool AppendExtRaw( const TCHAR *pszName, const TCHAR *pszExt );
	const TCHAR *GetName( int nIndex );
	const TCHAR *GetExt( int nIndex );

	//ダイアログに渡す拡張子フィルタを取得する。(lpstrFilterに直接指定可能)
	//2回呼び出すと古いバッファが無効になることがあるのに注意
	const TCHAR *GetExtFilter( void );

	int GetCount( void ) { return m_nCount; }

protected:
	// 2014.10.30 syat ConvertTypesExtToDlgExtをCDocTypeManagerに移動
	//bool ConvertTypesExtToDlgExt( const TCHAR *pszSrcExt, TCHAR *pszDstExt );

private:

	typedef struct {
		TCHAR	m_szName[64];		//名前(64文字以下のはず→m_szTypeName)
		TCHAR	m_szExt[MAX_TYPES_EXTS*3+1];	//拡張子(64文字以下のはず→m_szTypeExts) なお "*." を追加するのでそれなりに必要
	} FileExtInfoTag;

	int				m_nCount;
	FileExtInfoTag	*m_puFileExtInfo;
	std::vector<TCHAR>	m_vstrFilter;

private:
	DISALLOW_COPY_AND_ASSIGN(CFileExt);
};

#endif	//SAKURA_CFILEEXT_H_

