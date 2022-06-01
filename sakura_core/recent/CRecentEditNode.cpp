﻿/*! @file */
/*
	Copyright (C) 2008, kobake
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

#include "StdAfx.h"
#include "CRecentEditNode.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentEditNode::CRecentEditNode()
{
	Create(
		GetShareData()->m_sNodes.m_pEditArr,
		0,
		&GetShareData()->m_sNodes.m_nEditArrNum,
		NULL,
		MAX_EDITWINDOWS,
		NULL
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザー管理の構造体にキャストして参照してください。
*/
const WCHAR* CRecentEditNode::GetItemText( int nIndex ) const
{
	return L"WIN"; //※テキスト情報は無い (GetWindowTextしてあげても良いけど、この関数は実行されないので、意味は無い)
}

bool CRecentEditNode::DataToReceiveType( const EditNode** dst, const EditNode* src ) const
{
	*dst = src;
	return true;
}

bool CRecentEditNode::TextToDataType( EditNode* dst, LPCWSTR pszText ) const
{
	return false;
}

int CRecentEditNode::CompareItem( const EditNode* p1, const EditNode* p2 ) const
{
	return p1->m_hWnd - p2->m_hWnd;
}

void CRecentEditNode::CopyItem( EditNode* dst, const EditNode* src ) const
{
	*dst = *src;
}

bool CRecentEditNode::ValidateReceiveType( const EditNode* ) const
{
	return true;
}

size_t CRecentEditNode::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   固有インターフェース                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int CRecentEditNode::FindItemByHwnd(HWND hwnd) const
{
	int n = GetItemCount();
	for(int i=0;i<n;i++){
		if(GetItem(i)->m_hWnd == hwnd)return i;
	}
	return -1;
}

void CRecentEditNode::DeleteItemByHwnd(HWND hwnd)
{
	int n = FindItemByHwnd(hwnd);
	if(n!=-1){
		DeleteItem(n);
	}
	else{
		DEBUG_TRACE( L"DeleteItemByHwnd失敗\n" );
	}
}
