﻿/*!	@file
	@brief タグジャンプリストダイアログボックス

	@author MIK
	@date 2003.4.13
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
	Copyright (C) 2010, Moca
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

#ifndef SAKURA_CDLGTAGJUMPLIST_D44C3C98_9BF7_4B99_923B_9005CD54872F_H_
#define SAKURA_CDLGTAGJUMPLIST_D44C3C98_9BF7_4B99_923B_9005CD54872F_H_
#pragma once

#include "dlg/CDialog.h"
#include "recent/CRecentTagjumpKeyword.h"
#include "mem/CNativeW.h"

//タグファイル名	//	@@ 2005.03.31 MIK 定数化
#define TAG_FILENAME_T        L"tags"

// 2010.07.22 いくつかcppへ移動

class CSortedTagJumpList;

/*!	@brief ダイレクトタグジャンプ候補一覧ダイアログ

	ダイレクトタグジャンプで複数の候補がある場合及び
	キーワード指定タグジャンプのためのダイアログボックス制御
*/
class CDlgTagJumpList final : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgTagJumpList(bool bDirectTagJump);
	~CDlgTagJumpList();

	/*
	||  Attributes & Operations
	*/
	int DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam);	/* モーダルダイアログの表示 */

	//	@@ 2005.03.31 MIK 階層パラメータを追加
//	bool AddParamA( const ACHAR*, const ACHAR*, int, const ACHAR*, const ACHAR*, int depth, int baseDirId );	//登録
	void SetFileName( const WCHAR *pszFileName );
	void SetKeyword( const wchar_t *pszKeyword );	//	@@ 2005.03.31 MIK
	int  FindDirectTagJump();

	bool GetSelectedFullPathAndLine( WCHAR* fullPath, int count, int* lineNum, int* depth );

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL	OnBnClicked(int wID) override;
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam ) override;
	BOOL	OnSize( WPARAM wParam, LPARAM lParam ) override;
	BOOL	OnMove( WPARAM wParam, LPARAM lParam ) override;
	BOOL	OnMinMaxInfo( LPARAM lParam );
	BOOL	OnNotify(NMHDR* pNMHDR) override;
	//	@@ 2005.03.31 MIK キーワード入力エリアのイベント処理
	BOOL	OnCbnSelChange( HWND hwndCtl, int wID ) override;
	BOOL	OnCbnEditChange( HWND hwndCtl, int wID ) override;
	//BOOL	OnEnChange( HWND hwndCtl, int wID ) override;
	BOOL	OnTimer( WPARAM wParam ) override;
	LPVOID	GetHelpIdTable( void ) override;

private:
	struct STagFindState {
		int   m_nDepth;
		int   m_nMatchAll;
		int   m_nNextMode;
		int   m_nLoop;
		bool  m_bJumpPath;
		WCHAR m_szCurPath[1024];
	};

	struct STagSearchRule {
		bool bTagJumpExactMatch;
		bool bTagJumpPartialMatch;
		bool bTagJumpICase;
		int baseDirId;
		int nTop;
	};

	void	StopTimer( void );
	void	StartTimer(int nDelay);

	void	SetData( void ) override;	/* ダイアログデータの設定 */
	int		GetData( void ) override;	/* ダイアログデータの取得 */
	void	UpdateData(bool bInit);	//	@@ 2005.03.31 MIK

	WCHAR	*GetNameByType( const WCHAR type, const WCHAR *name );	//タイプを名前に変換する。
	int		SearchBestTag( void );	//もっとも確率の高そうなインデックスを返す。
	//	@@ 2005.03.31 MIK
	const WCHAR *GetFileName( void );
	const WCHAR *GetFilePath( void ){ return m_pszFileName != NULL ? m_pszFileName : L""; }
	void Empty( void );
	void SetTextDir();
	void FindNext(bool bNewFind);
	void find_key( const wchar_t* keyword );
	int find_key_core(int  nTop, const wchar_t* keyword, bool bTagJumpPartialMatch, bool bTagJumpExactMatch, bool bTagJumpICase, bool bTagJumpICaseByTags, int  nDefaultNextMode);
	bool parseTagsLine(ACHAR s[][1024], ACHAR* szLineData, int* n2, int nTagFormat);
	bool ReadTagsParameter(FILE* fp, bool bTagJumpICaseByTags, STagFindState* state, CSortedTagJumpList& cList, int* nTagFormat, bool* bSorted, bool* bFoldcase, bool* bTagJumpICase, PTCHAR szNextPath, int* baseDirId);
	void find_key_for_BinarySearch( FILE* fp, const ACHAR* paszKeyword, int nTagFormat, STagFindState* state, const STagSearchRule* rule );
	void find_key_for_LinearSearch( FILE* fp, const ACHAR* paszKeyword, int nTagFormat, STagFindState* state, const STagSearchRule* rule, bool bSorted, bool bFoldcase, int length );

	bool IsDirectTagJump();

	void ClearPrevFindInfo();
	bool GetFullPathAndLine( int index, WCHAR *fullPath, int count, int *lineNum, int *depth );

	//! depthから完全パス名(相対パス/絶対パス)を作成する
	static WCHAR* GetFullPathFromDepth(WCHAR* pszOutput, int count, WCHAR* basePath, const WCHAR* fileName, int depth);
	static WCHAR* CopyDirDir( WCHAR* dest, const WCHAR* target, const WCHAR* base );
public:
	static int CalcMaxUpDirectory(const WCHAR* p);
	static WCHAR* DirUp( WCHAR* dir );

private:
	bool	m_bDirectTagJump;

	int		m_nIndex;		//!< 選択された要素番号
	WCHAR	*m_pszFileName;	//!< 編集中のファイル名
	wchar_t	*m_pszKeyword;	//!< キーワード(DoModalのlParam!=0を指定した場合に指定できる)
	int		m_nLoop;		//!< さかのぼれる階層数
	CSortedTagJumpList*	m_pcList;	//!< タグジャンプ情報
	UINT_PTR	m_nTimerId;		//!< タイマ番号
	BOOL	m_bTagJumpICase;	//!< 大文字小文字を同一視
	BOOL	m_bTagJumpPartialMatch;	//!< 文字列の途中にマッチ
	BOOL	m_bTagJumpExactMatch; //!< 完全一致(画面無し)

	int 	m_nTop;			//!< ページめくりの表示の先頭(0開始)
	bool	m_bNextItem;	//!< まだ次にヒットするものがある

	// 絞り込み検索用
	STagFindState* m_psFindPrev; //!< 前回の最後に検索した状態
	STagFindState* m_psFind0Match; //!< 前回の1つもHitしなかった最後のtags

	CNativeW	m_strOldKeyword;	//!< 前回のキーワード
	BOOL	m_bOldTagJumpICase;	//!< 前回の大文字小文字を同一視
	BOOL	m_bOldTagJumpPartialMatch;	//!< 前回の文字列の途中にマッチ

	CRecentTagjumpKeyword	m_cRecentKeyword;

	POINT	m_ptDefaultSize;
	RECT	m_rcItems[11];

	DISALLOW_COPY_AND_ASSIGN(CDlgTagJumpList);
};
#endif /* SAKURA_CDLGTAGJUMPLIST_D44C3C98_9BF7_4B99_923B_9005CD54872F_H_ */
