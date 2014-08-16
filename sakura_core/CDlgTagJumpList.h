/*!	@file
	@brief タグジャンプリストダイアログボックス

	@author MIK
	@date 2003.4.13
*/
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

#ifndef	_CDLGTAGJUMPLIST_H_
#define	_CDLGTAGJUMPLIST_H_

#include "CDialog.h"
#include "CSortedTagJumpList.h"
#include "design_template.h"

//タグファイル名	//	@@ 2005.03.31 MIK 定数化
#define TAG_FILENAME        "tags"
//タグファイルのフォーマット	//	@@ 2005.03.31 MIK 定数化
//	@@ 2005.04.03 MIK キーワードに空白が含まれる場合の考慮
#define TAG_FORMAT          "%[^\t\r\n]\t%[^\t\r\n]\t%d;\"\t%s\t%s"

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
	CDlgTagJumpList();
	~CDlgTagJumpList();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM );	/* モーダルダイアログの表示 */

	//	@@ 2005.03.31 MIK 階層パラメータを追加
	bool AddParam( char *s0, char *s1, int n2, char *s3, char *s4, int depth );	//登録
	bool GetSelectedParam( char *s0, char *s1, int *n2, char *s3, char *s4, int *depth );	//取得
	void SetFileName( const char *pszFileName );
	void SetKeyword( const char *pszKeyword );	//	@@ 2005.03.31 MIK

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	BOOL	OnBnClicked( int );
	BOOL	OnNotify( WPARAM wParam, LPARAM lParam );
	//	@@ 2005.03.31 MIK キーワード入力エリアのイベント処理
	BOOL	OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL	OnCbnEditChange( HWND hwndCtl, int wID );
	//BOOL	OnEnChange( HWND hwndCtl, int wID );
	BOOL	OnTimer( WPARAM wParam );
	LPVOID	GetHelpIdTable( void );

	void	StopTimer( void );
	void	StartTimer( void );

	void	SetData( void );	/* ダイアログデータの設定 */
	int		GetData( void );	/* ダイアログデータの取得 */
	void	UpdateData( void );	//	@@ 2005.03.31 MIK

	TCHAR	*GetNameByType( const TCHAR type, const TCHAR *name );	//タイプを名前に変換する。
	int		SearchBestTag( void );	//もっとも確率の高そうなインデックスを返す。
	//	@@ 2005.03.31 MIK
	const TCHAR *GetFileName( void );
	const TCHAR *GetFilePath( void ){ return m_pszFileName != NULL ? m_pszFileName : _T(""); }
	void find_key( const char* keyword );
	void Empty( void );



private:

	int		m_nIndex;		//!< 選択された要素番号
	TCHAR	*m_pszFileName;	//!< 編集中のファイル名
	char	*m_pszKeyword;	//!< キーワード(DoModalのlParam!=0を指定した場合に指定できる)
	int		m_nLoop;		//!< さかのぼれる階層数
	CSortedTagJumpList	m_cList;	//!< タグジャンプ情報
	UINT	m_nTimerId;		//!< タイマ番号
	BOOL	m_bTagJumpICase;	//!< 大文字小文字を同一視
	BOOL	m_bTagJumpAnyWhere;	//!< 文字列の途中にマッチ

private:
	DISALLOW_COPY_AND_ASSIGN(CDlgTagJumpList);
};

#endif	//_CDLGTAGJUMPLIST_H_

