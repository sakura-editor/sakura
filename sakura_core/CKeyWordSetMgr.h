//	$Id$
/************************************************************************

	CKeyWordSetMgr.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CKeyWordSetMgr;

#ifndef _CKEYWORDSETMGR_H_
#define _CKEYWORDSETMGR_H_

#include <windows.h>
#include "CMemory.h"

#define		MAX_SETNUM		20
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
	BOOL DelKeyWord( int , int );	/* ｎ番目のセットのｍ番目のキーワードを削除 */
	int SearchKeyWord( int , const char*, int );	/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
	BOOL IsModify( CKeyWordSetMgr&, BOOL* pnModifyFlagArr );	/* 変更状況を調査 */
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
protected:
	/*
	||  実装ヘルパ関数
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CKEYWORDSETMGR_H_ */

/*[EOF]*/
