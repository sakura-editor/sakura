//	$Id$
/*!	@file
	強調キーワード管理

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class CKeyWordSetMgr;

#ifndef _CKEYWORDSETMGR_H_
#define _CKEYWORDSETMGR_H_

#include <windows.h>
#include "CMemory.h"

#define		MAX_SETNUM		20	//Jul. 12, 2001 jepro notes: 強調キーワードのセット数の最大値
#define		MAX_SETNAMELEN	32
#define		MAX_KEYWORDNUM	1000
#define		MAX_KEYWORDLEN	100

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CKeyWordSetMgr
{
public:
	/*
	||  Constructors
	*/
	CKeyWordSetMgr();
	~CKeyWordSetMgr();
	BOOL AddKeyWordSet( const char*, BOOL );	/* セットの追加 */
	BOOL DelKeyWordSet( int  );	/* ｎ番目のセットを削除 */
	char* GetTypeName( int );	/* ｎ番目のセット名を返す */
	int GetKeyWordNum( int );	/* ｎ番目のセットのキーワードの数を返す */
	char* GetKeyWord( int , int );	/* ｎ番目のセットのｍ番目のキーワードを返す */
	char* UpdateKeyWord( int , int , const char* );	/* ｎ番目のセットのｍ番目のキーワードを編集 */
	BOOL AddKeyWord( int, const char* );	/* ｎ番目のセットにキーワードを追加 */
	BOOL DelKeyWord( int , int );			/* ｎ番目のセットのｍ番目のキーワードを削除 */
	int SearchKeyWord( int , const char*, int );	/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
	BOOL IsModify( CKeyWordSetMgr&, BOOL* pnModifyFlagArr );	/* 変更状況を調査 */
	void SortKeyWord( int );						/* ｎ番目のセットのキーワードをソートする */  //MIK
	int SearchKeyWord2( int , const char*, int );	/* ｎ番目のセットから指定キーワードをバイナリサーチ 無いときは-1を返す */	//MIK
	void SetKeyWordCase( int, int );				/* ｎ番目のセットの大文字小文字判断をセットする */	//MIK
	int GetKeyWordCase( int );						/* ｎ番目のセットの大文字小文字判断を取得する */			//MIK
	/*
	|| 演算子
	*/
	const CKeyWordSetMgr& operator=( CKeyWordSetMgr& );
	/*
	||  Attributes & Operations
	*/
	int		m_nCurrentKeyWordSetIdx;
	int		m_nKeyWordSetNum;
	char	m_szSetNameArr[MAX_SETNUM][MAX_SETNAMELEN + 1];
	int		m_nKEYWORDCASEArr[MAX_SETNUM];	/* キーワードの英大文字小文字区別 */
	int		m_nKeyWordNumArr[MAX_SETNUM];
	char	m_szKeyWordArr[MAX_SETNUM][MAX_KEYWORDNUM][MAX_KEYWORDLEN + 1];
	char	m_IsSorted[MAX_SETNUM];	/* ソートしたかどうかのフラグ */  //MIK
protected:
	/*
	||  実装ヘルパ関数
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CKEYWORDSETMGR_H_ */


/*[EOF]*/
