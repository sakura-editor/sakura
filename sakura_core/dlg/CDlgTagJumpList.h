/*!	@file
	@brief タグジャンプリストダイアログボックス

	@author MIK
	@date 2003.4.13
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
	Copyright (C) 2010, Moca

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

#ifndef	SAKURA_CDLGTAGJUMPLIST_H_
#define	SAKURA_CDLGTAGJUMPLIST_H_

#include "dlg/CDialog.h"
#include "recent/CRecentTagjumpKeyword.h"

//タグファイル名	//	@@ 2005.03.31 MIK 定数化
#define TAG_FILENAME_T        _T("tags")

// 2010.07.22 いくつかcppへ移動

class CSortedTagJumpList;

/*!	@brief ダイレクトタグジャンプ候補一覧ダイアログ

	ダイレクトタグジャンプで複数の候補がある場合及び
	キーワード指定タグジャンプのためのダイアログボックス制御
*/
class CDlgTagJumpList : public CDialog
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
	int DoModal( HINSTANCE, HWND, LPARAM );	/* モーダルダイアログの表示 */

	//	@@ 2005.03.31 MIK 階層パラメータを追加
//	bool AddParamA( const ACHAR*, const ACHAR*, int, const ACHAR*, const ACHAR*, int depth, int baseDirId );	//登録
	void SetFileName( const TCHAR *pszFileName );
	void SetKeyword( const wchar_t *pszKeyword );	//	@@ 2005.03.31 MIK
	int  FindDirectTagJump();

	bool GetSelectedFullPathAndLine( TCHAR* fullPath, int count, int* lineNum, int* depth );

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	BOOL	OnBnClicked( int );
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
	BOOL	OnSize( WPARAM wParam, LPARAM lParam );
	BOOL	OnMove( WPARAM wParam, LPARAM lParam );
	BOOL	OnMinMaxInfo( LPARAM lParam );
	BOOL	OnNotify( WPARAM wParam, LPARAM lParam );
	//	@@ 2005.03.31 MIK キーワード入力エリアのイベント処理
	BOOL	OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL	OnCbnEditChange( HWND hwndCtl, int wID );
	//BOOL	OnEnChange( HWND hwndCtl, int wID );
	BOOL	OnTimer( WPARAM wParam );
	LPVOID	GetHelpIdTable( void );

private:
	void	StopTimer( void );
	void	StartTimer( int );

	void	SetData( void );	/* ダイアログデータの設定 */
	int		GetData( void );	/* ダイアログデータの取得 */
	void	UpdateData( bool );	//	@@ 2005.03.31 MIK

	TCHAR	*GetNameByType( const TCHAR type, const TCHAR *name );	//タイプを名前に変換する。
	int		SearchBestTag( void );	//もっとも確率の高そうなインデックスを返す。
	//	@@ 2005.03.31 MIK
	const TCHAR *GetFileName( void );
	const TCHAR *GetFilePath( void ){ return m_pszFileName != NULL ? m_pszFileName : _T(""); }
	void Empty( void );
	void SetTextDir();
	void FindNext( bool );
	void find_key( const wchar_t* keyword );
	int find_key_core(int, const wchar_t*, bool, bool, bool, bool, int);
	
	bool IsDirectTagJump();
	
	void ClearPrevFindInfo();
	bool GetFullPathAndLine( int index, TCHAR *fullPath, int count, int *lineNum, int *depth );


	//! depthから完全パス名(相対パス/絶対パス)を作成する
	static TCHAR* GetFullPathFromDepth( TCHAR*, int, TCHAR*, const TCHAR*, int );
	static TCHAR* CopyDirDir( TCHAR* dest, const TCHAR* target, const TCHAR* base );
public:
	static int CalcMaxUpDirectory( const TCHAR* );
	static TCHAR* DirUp( TCHAR* dir );

private:

	struct STagFindState{
		int   m_nDepth;
		int   m_nMatchAll;
		int   m_nNextMode;
		int   m_nLoop;
		bool  m_bJumpPath;
		TCHAR m_szCurPath[1024];
	};
	
	bool	m_bDirectTagJump;

	int		m_nIndex;		//!< 選択された要素番号
	TCHAR	*m_pszFileName;	//!< 編集中のファイル名
	wchar_t	*m_pszKeyword;	//!< キーワード(DoModalのlParam!=0を指定した場合に指定できる)
	int		m_nLoop;		//!< さかのぼれる階層数
	CSortedTagJumpList*	m_pcList;	//!< タグジャンプ情報
	UINT_PTR	m_nTimerId;		//!< タイマ番号
	BOOL	m_bTagJumpICase;	//!< 大文字小文字を同一視
	BOOL	m_bTagJumpAnyWhere;	//!< 文字列の途中にマッチ
	BOOL	m_bTagJumpExactMatch; //!< 完全一致(画面無し)

	int 	m_nTop;			//!< ページめくりの表示の先頭(0開始)
	bool	m_bNextItem;	//!< まだ次にヒットするものがある

	// 絞り込み検索用
	STagFindState* m_psFindPrev; //!< 前回の最後に検索した状態
	STagFindState* m_psFind0Match; //!< 前回の1つもHitしなかった最後のtags

	CNativeW	m_strOldKeyword;	//!< 前回のキーワード
	BOOL	m_bOldTagJumpICase;	//!< 前回の大文字小文字を同一視
	BOOL	m_bOldTagJumpAnyWhere;	//!< 前回の文字列の途中にマッチ

	SComboBoxItemDeleter	m_comboDel;
	CRecentTagjumpKeyword	m_cRecentKeyword;
	
	POINT	m_ptDefaultSize;
	RECT	m_rcItems[11];

private:
	DISALLOW_COPY_AND_ASSIGN(CDlgTagJumpList);
};

#endif	//SAKURA_CDLGTAGJUMPLIST_H_

