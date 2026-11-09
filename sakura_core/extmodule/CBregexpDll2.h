/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CBREGEXPDLL2_033C910A_6B78_47CB_9993_675C48A2AB64_H_
#define SAKURA_CBREGEXPDLL2_033C910A_6B78_47CB_9993_675C48A2AB64_H_
#pragma once

#include "CDllHandler.h"

typedef struct bregexp {
	const WCHAR *outp;		/* result string start ptr  */
	const WCHAR *outendp;	/* result string end ptr    */
	const int   splitctr;	/* split result counter     */
	const WCHAR **splitp;	/* split result pointer ptr     */
	int rsv1;					/* reserved for external use    */
	WCHAR *parap;				/* parameter start ptr ie. "s/xxxxx/yy/gi"  */
	WCHAR *paraendp;			/* parameter end ptr     */
	WCHAR *transtblp;			/* translate table ptr   */
	WCHAR **startp;				/* match string start ptr   */
	WCHAR **endp;				/* match string end ptr     */
	int nparens;				/* number of parentheses */
} BREGEXP_W;

//!BREGONIG.DLLをラップしたもの。
//2007.09.13 kobake 作成
class CBregexpDll2 : public CDllImp{
public:
	CBregexpDll2();
	virtual ~CBregexpDll2();

protected:
	// CDllImpインターフェース
	virtual LPCWSTR GetDllNameImp(int nIndex); // Jul. 5, 2001 genta インターフェース変更に伴う引数追加
	virtual bool InitDllImp();

protected:
	// DLL関数の型
	typedef int            (__cdecl *BREGEXP_BMatchW2)        (const WCHAR* str, const WCHAR* target, const WCHAR* targetendp, BREGEXP_W** rxp, WCHAR* msg);
	typedef int            (__cdecl *BREGEXP_BSubstW2)        (const WCHAR* str, const WCHAR* target, const WCHAR* targetendp, BREGEXP_W** rxp, WCHAR* msg);
	typedef int            (__cdecl *BREGEXP_BTransW2)        (const WCHAR* str, WCHAR* target, WCHAR* targetendp, BREGEXP_W** rxp, WCHAR* msg);
	typedef int            (__cdecl *BREGEXP_BSplitW2)        (const WCHAR* str, WCHAR* target, WCHAR* targetendp, int limit, BREGEXP_W** rxp, WCHAR* msg);
	typedef void           (__cdecl *BREGEXP_BRegfreeW2)      (BREGEXP_W* rx);
	typedef const WCHAR* (__cdecl *BREGEXP_BRegexpVersionW2)(void);
	typedef int            (__cdecl *BREGEXP_BMatchExW2)      (const WCHAR* str, const WCHAR* targetbeg, const WCHAR* target, const WCHAR* targetendp, BREGEXP_W** rxp, WCHAR* msg);
	typedef int            (__cdecl *BREGEXP_BSubstExW2)      (const WCHAR* str, const WCHAR* targetbeg, const WCHAR* target, const WCHAR* targetendp, BREGEXP_W** rxp, WCHAR* msg);

public:
	// UNICODEインターフェースを提供する
	int BMatch(const WCHAR* str, const WCHAR* target,const WCHAR* targetendp,BREGEXP_W** rxp,WCHAR* msg)
	{
		return m_BMatch(str,target,targetendp,rxp,msg);
	}
	int BSubst(const WCHAR* str, const WCHAR* target,const WCHAR* targetendp,BREGEXP_W** rxp,WCHAR* msg)
	{
		return m_BSubst(str,target,targetendp,rxp,msg);
	}
	int BTrans(const WCHAR* str, WCHAR* target,WCHAR* targetendp,BREGEXP_W** rxp,WCHAR* msg)
	{
		return m_BTrans(str,target,targetendp,rxp,msg);
	}
	int BSplit(const WCHAR* str, WCHAR* target,WCHAR* targetendp,int limit,BREGEXP_W** rxp,WCHAR* msg)
	{
		return m_BSplit(str,target,targetendp,limit,rxp,msg);
	}
	void BRegfree(BREGEXP_W* rx)
	{
		return m_BRegfree(rx);
	}
	const WCHAR* BRegexpVersion(void)
	{
		return m_BRegexpVersion();
	}
	int BMatchEx(const WCHAR* str, const WCHAR* targetbeg, const WCHAR* target, const WCHAR* targetendp, BREGEXP_W** rxp, WCHAR* msg)
	{
		return m_BMatchEx(str,targetbeg,target,targetendp,rxp,msg);
	}
	int BSubstEx(const WCHAR* str, const WCHAR* targetbeg, const WCHAR* target, const WCHAR* targetendp, BREGEXP_W** rxp, WCHAR* msg)
	{
		return m_BSubstEx(str,targetbeg,target,targetendp,rxp,msg);
	}

	// 関数があるかどうか
	bool ExistBMatchEx() const{ return m_BMatchEx!=nullptr; }
	bool ExistBSubstEx() const{ return m_BSubstEx!=nullptr; }

private:
	//DLL内関数ポインタ
	BREGEXP_BMatchW2         m_BMatch;
	BREGEXP_BSubstW2         m_BSubst;
	BREGEXP_BTransW2         m_BTrans;
	BREGEXP_BSplitW2         m_BSplit;
	BREGEXP_BRegfreeW2       m_BRegfree;
	BREGEXP_BRegexpVersionW2 m_BRegexpVersion;
	BREGEXP_BMatchExW2       m_BMatchEx;
	BREGEXP_BSubstExW2       m_BSubstEx;
};
#endif /* SAKURA_CBREGEXPDLL2_033C910A_6B78_47CB_9993_675C48A2AB64_H_ */
