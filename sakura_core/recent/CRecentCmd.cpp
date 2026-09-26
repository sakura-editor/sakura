/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentCmd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentCmd::CRecentCmd()
{
	Create(
		GetShareData()->m_sHistory.m_aCommands.dataPtr(),
		GetShareData()->m_sHistory.m_aCommands.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sHistory.m_aCommands._GetSizeRef(),
		nullptr,
		MAX_CMDARR,
		nullptr
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザー管理の構造体にキャストして参照してください。
*/
const WCHAR* CRecentCmd::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentCmd::DataToReceiveType( LPCWSTR* dst, const CCmdString* src ) const
{
	*dst = *src;
	return true;
}

int CRecentCmd::CompareItem( const CCmdString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

