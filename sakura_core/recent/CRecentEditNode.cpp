#include "StdAfx.h"
#include "CRecentEditNode.h"
#include <string.h>
#include "env/DLLSHAREDATA.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentEditNode::CRecentEditNode()
{
	Create(
		GetShareData()->m_sNodes.m_pEditArr,
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

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* CRecentEditNode::GetItemText( int nIndex ) const
{
	return _T("WIN"); //※テキスト情報は無い (GetWindowTextしてあげても良いけど、この関数は実行されないので、意味は無い)
}

bool CRecentEditNode::DataToReceiveType( const EditNode** dst, const EditNode* src ) const
{
	*dst = src;
	return true;
}

bool CRecentEditNode::TextToDataType( EditNode* dst, LPCTSTR pszText ) const
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
		DBPRINT_A("DeleteItemByHwnd失敗\n");
	}
}
