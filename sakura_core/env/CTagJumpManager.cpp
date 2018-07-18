/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake

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
#include "env/DLLSHAREDATA.h"

#include "CTagJumpManager.h"


/*!
	@brief タグジャンプ情報の保存

	タグジャンプするときに、タグジャンプ先の情報を保存する。

	@param[in] pTagJump 保存するタグジャンプ情報
	@retval true	保存成功
	@retval false	保存失敗

	@date 2004/06/21 新規作成
	@date 2004/06/22 Moca 一杯になったら一番古い情報を削除しそこに新しい情報を入れる
*/
void CTagJumpManager::PushTagJump(const TagJump *pTagJump)
{
	int i = m_pShareData->m_sTagJump.m_TagJumpTop + 1;
	if( MAX_TAGJUMPNUM <= i ){
		i = 0;
	}
	if( m_pShareData->m_sTagJump.m_TagJumpNum < MAX_TAGJUMPNUM ){
		m_pShareData->m_sTagJump.m_TagJumpNum++;
	}
	m_pShareData->m_sTagJump.m_TagJump[i] = *pTagJump;
	m_pShareData->m_sTagJump.m_TagJumpTop = i;
}


/*!
	@brief タグジャンプ情報の参照

	タグジャンプバックするときに、タグジャンプ元の情報を参照する。

	@param[out] pTagJump 参照するタグジャンプ情報
	@retval true	参照成功
	@retval false	参照失敗

	@date 2004/06/21 新規作成
	@date 2004/06/22 Moca SetTagJump変更による修正
*/
bool CTagJumpManager::PopTagJump(TagJump *pTagJump)
{
	if( 0 < m_pShareData->m_sTagJump.m_TagJumpNum ){
		*pTagJump = m_pShareData->m_sTagJump.m_TagJump[m_pShareData->m_sTagJump.m_TagJumpTop--];
		if( m_pShareData->m_sTagJump.m_TagJumpTop < 0 ){
			m_pShareData->m_sTagJump.m_TagJumpTop = MAX_TAGJUMPNUM - 1;
		}
		m_pShareData->m_sTagJump.m_TagJumpNum--;
		return true;
	}
	return false;
}
