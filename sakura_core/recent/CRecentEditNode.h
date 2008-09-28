#pragma once

#include "CRecentImp.h"
#include "env/CShareData.h" //EditNode

//! EditNode(ウィンドウリスト)の履歴を管理 (RECENT_FOR_EDITNODE)
class CRecentEditNode : public CRecentImp<EditNode>{
public:
	//生成
	CRecentEditNode();

	//オーバーライド
	int				CompareItem( const EditNode* p1, const EditNode* p2 ) const;
	void			CopyItem( EditNode* dst, const EditNode* src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;

	//固有インターフェース
	int FindItemByHwnd(HWND hwnd) const;
	void DeleteItemByHwnd(HWND hwnd);
};
