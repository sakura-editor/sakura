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
#ifndef SAKURA_CAPPNODEMANAGER_CAE7A323_DEA3_47E4_91DE_C99A88C32683_H_
#define SAKURA_CAPPNODEMANAGER_CAE7A323_DEA3_47E4_91DE_C99A88C32683_H_
#pragma once

#include "basis/CMyString.h"
#include "config/maxdata.h"
#include "util/design_template.h"

class CAppNodeGroupHandle;

//! 編集ウィンドウノード
struct EditNode {
	int				m_nIndex;
	int				m_nGroup;					//!< グループID								//@@@ 2007.06.20 ryoji
	HWND			m_hWnd;
	int				m_nId;						//!< 無題Id
	WIN_CHAR		m_szTabCaption[_MAX_PATH];	//!< タブウインドウ用：キャプション名		//@@@ 2003.05.31 MIK
	SFilePath		m_szFilePath;				//!< タブウインドウ用：ファイル名			//@@@ 2006.01.28 ryoji
	bool			m_bIsGrep;					//!< Grepのウィンドウか						//@@@ 2006.01.28 ryoji
	UINT			m_showCmdRestore;			//!< 元のサイズに戻すときのサイズ種別		//@@@ 2007.06.20 ryoji
	BOOL			m_bClosing;					//!< 終了中か（「最後のファイルを閉じても(無題)を残す」用）	//@@@ 2007.06.20 ryoji

	HWND GetHwnd() const{ if(this)return m_hWnd; else return NULL; } // TODO: Remove "this" check
	static HWND GetSafeHwnd(const EditNode* node) { return node ? node->m_hWnd : NULL; }
	int GetId() const{ if(this)return m_nId; else return 0; } // TODO: Remove "this" check
	static int GetSafeId(const EditNode* node) { return node ? node->m_nId : 0; }
	CAppNodeGroupHandle GetGroup() const;
	static CAppNodeGroupHandle GetGroup_Safe(const EditNode* node);
	bool IsTopInGroup() const;
	static bool IsTopInGroup_Safe(const EditNode* node);
};

//! 拡張構造体
struct EditNodeEx{
	EditNode*	p;			//!< 編集ウィンドウ配列要素へのポインタ
	int			nGroupMru;	//!< グループ単位のMRU番号
};

//! 共有メモリ内構造体
struct SShare_Nodes{
	int					m_nEditArrNum;	//short->intに修正	//@@@ 2003.05.31 MIK
	EditNode			m_pEditArr[MAX_EDITWINDOWS];	//最大値修正	@@@ 2003.05.31 MIK
	LONG				m_nSequences;	/* ウィンドウ連番 */
	LONG				m_nNonameSequences;	/* 無題連番 */
	LONG				m_nGroupSequences;	// タブグループ連番	// 2007.06.20 ryoji
};

//! ノードアクセサ
class CAppNodeHandle{
public:
	CAppNodeHandle(HWND hwnd);
	EditNode* operator->(){ return m_pNodeRef; }
private:
	EditNode* m_pNodeRef;
};

//! グループアクセサ
class CAppNodeGroupHandle{
public:
	CAppNodeGroupHandle(int nGroupId) : m_nGroup(nGroupId) { }
	CAppNodeGroupHandle(HWND hwnd){ m_nGroup = CAppNodeHandle(hwnd)->GetGroup(); }

	EditNode* GetTopEditNode(){ return GetEditNodeAt(0); }	//
	EditNode* GetEditNodeAt( int nIndex );					//!< 指定位置の編集ウィンドウ情報を取得する
	BOOL AddEditWndList(HWND hWnd);							//!< 編集ウィンドウの登録	// 2007.06.26 ryoji nGroup引数追加
	void DeleteEditWndList(HWND hWnd);							//!< 編集ウィンドウリストからの削除
	BOOL RequestCloseEditor( EditNode* pWndArr, int nArrCnt, BOOL bExit, BOOL bCheckConfirm, HWND hWndFrom );
															//!< いくつかのウィンドウへ終了要求を出す	// 2007.02.13 ryoji 「編集の全終了」を示す引数(bExit)を追加	// 2007.06.20 ryoji nGroup引数追加

	int GetEditorWindowsNum( bool bExcludeClosing = true );				/* 現在の編集ウィンドウの数を調べる */	// 2007.06.20 ryoji nGroup引数追加	// 2008.04.19 ryoji bExcludeClosing引数追加

	//全ウィンドウ一括操作
	BOOL PostMessageToAllEditors( UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast );	/* 全編集ウィンドウへメッセージをポストする */	// 2007.06.20 ryoji nGroup引数追加
	BOOL SendMessageToAllEditors( UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast );	/* 全編集ウィンドウへメッセージを送るする */	// 2007.06.20 ryoji nGroup引数追加

public:
	bool operator==(const CAppNodeGroupHandle& rhs) const{ return m_nGroup==rhs.m_nGroup; }
	bool IsValidGroup() const{ return m_nGroup>=0; }
	operator int() const{ return m_nGroup; }

private:
	int m_nGroup;
};

class CAppNodeManager : public TSingleton<CAppNodeManager>{
	friend class TSingleton<CAppNodeManager>;
	CAppNodeManager(){}

public:
	//グループ
	void ResetGroupId();									/* グループをIDリセットする */

	//ウィンドウハンドル → ノード　変換
	EditNode* GetEditNode( HWND hWnd );							/* 編集ウィンドウ情報を取得する */
	int GetNoNameNumber(HWND hWnd);

	//タブ
	bool ReorderTab( HWND hSrcTab, HWND hDstTab );				/* タブ移動に伴うウィンドウの並び替え 2007.07.07 genta */
	HWND SeparateGroup( HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[] );/* タブ分離に伴うウィンドウ処理 2007.07.07 genta */

	//総合情報
	int GetOpenedWindowArr(EditNode** ppEditNode, BOOL bSort, BOOL bGSort = FALSE );				/* 現在開いている編集ウィンドウの配列を返す */

protected:
	int _GetOpenedWindowArrCore(EditNode** ppEditNode, BOOL bSort, BOOL bGSort = FALSE );			/* 現在開いている編集ウィンドウの配列を返す（コア処理部） */

public:
	static bool IsSameGroup( HWND hWnd1, HWND hWnd2 );					/* 同一グループかどうかを調べる */
	int GetFreeGroupId( void );											/* 空いているグループ番号を取得する */
	HWND GetNextTab(HWND hWndCur);										// Close した時の次のWindowを取得する(タブまとめ表示の場合)	2013/4/10 Uchi
};

inline CAppNodeGroupHandle EditNode::GetGroup() const{ if(this)return m_nGroup; else return 0; } // TODO: Remove "this" check
inline CAppNodeGroupHandle EditNode::GetGroup_Safe(const EditNode* node)
{
	if (node)
		return node->m_nGroup;
	return 0;
}

inline bool EditNode::IsTopInGroup() const{ return this && (CAppNodeGroupHandle(m_nGroup).GetEditNodeAt(0) == this); } // TODO: Remove "this" check
inline bool EditNode::IsTopInGroup_Safe(const EditNode* node)
{
	if (node)
		return node->IsTopInGroup();
	return false;
}

inline CAppNodeHandle::CAppNodeHandle(HWND hwnd)
{
	m_pNodeRef = CAppNodeManager::getInstance()->GetEditNode(hwnd);
}
#endif /* SAKURA_CAPPNODEMANAGER_CAE7A323_DEA3_47E4_91DE_C99A88C32683_H_ */
