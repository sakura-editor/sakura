/*!	@file
	@brief タグジャンプリストダイアログボックス

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 2003, MIK

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



class CDlgTagJumpList;

#ifndef	_CDLGTAGJUMPLIST_H_
#define	_CDLGTAGJUMPLIST_H_

#include "CDialog.h"

class SAKURA_CORE_API CDlgTagJumpList : public CDialog
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

	bool AddParam( char *s0, char *s1, int n2, char *s3, char *s4 );	//登録
	bool GetSelectedParam( char *s0, char *s1, int *n2, char *s3, char *s4 );	//取得
	void SetFileName( const char *pszFileName );

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	BOOL	OnBnClicked( int );
	BOOL	OnNotify( WPARAM wParam, LPARAM lParam );
	LPVOID	GetHelpIdTable( void );

	void	SetData( void );	/* ダイアログデータの設定 */
	int		GetData( void );	/* ダイアログデータの取得 */

	char	*GetNameByType( const char type, const char *name );	//タイプを名前に変換する。
	int		SearchBestTag( void );	//もっとも確率の高そうなインデックスを返す。

private:
	typedef struct {
		char	*s0;	//キーワード
		char	*s1;	//ファイル名
		int		n2;		//行番号
		char	*s3;	//タイプ
		char	*s4;	//備考
	} ParamTag;

#define	MAX_TAGJUMPLIST	100	//タグジャンプリストの最大管理数(これ以上あっても選べる？)
	int			m_nCount;
	ParamTag	m_uParam[MAX_TAGJUMPLIST];
	int			m_nIndex;
	bool		m_bOverflow;	//登録数が多すぎるか？
	char		*m_pszFileName;

};

#endif	//_CDLGTAGJUMPLIST_H_

