/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK

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
#include "global.h"
#include "CShareData.h"
#include "CRecentEditNode.h"
#include <string.h>
#include "my_icmp.h"

CRecentCmd::CRecentCmd()
{
	Create(
		(char*)m_pShareData->m_sHistory.m_szCmdArr,
		&m_pShareData->m_sHistory.m_nCmdArrNum,
		NULL,
		MAX_CMDARR,
		NULL,
		sizeof( m_pShareData->m_sHistory.m_szCmdArr[0] ),	//MAX_CMDLEN
		0,
		0	//AppendItemのデータは文字列
	);
}


/*
	アイテムを検索する。
*/
int CRecentCmd::FindItem( const char *pszItemData ) const
{
	int	i;
	const char	*p, *q;

	if( ! IsAvailable() ) return -1;
	if( NULL == pszItemData ) return -1;

	p = pszItemData;

	for( i = 0; i < *m_pnUserItemCount; i++ )
	{
		q = GetArrayOffset( i ) + m_nOffset;

		if( strcmp( p, q ) == 0 ) return i;
	}

	return -1;
}
