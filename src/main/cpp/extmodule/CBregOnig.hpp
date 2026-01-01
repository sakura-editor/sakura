/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CBREGEXPDLL2_033C910A_6B78_47CB_9993_675C48A2AB64_H_
#define SAKURA_CBREGEXPDLL2_033C910A_6B78_47CB_9993_675C48A2AB64_H_
#pragma once

#include "extmodule/CDllHandler.h"

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
class CBregOnig : public CDllImp
{
public:
	CBregOnig();
	~CBregOnig() override;

protected:
	// CDllImpインターフェース
	LPCWSTR	GetDllNameImp(int nIndex) override;

	bool	InitDllImp() override;

protected:
	// DLL関数の型
	typedef int            (__cdecl *BREGEXP_BMatchW2)        (const wchar_t* str, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);
	typedef int            (__cdecl *BREGEXP_BSubstW2)        (const wchar_t* str, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);
	typedef void           (__cdecl *BREGEXP_BRegfreeW2)      (BREGEXP_W* rx);
	typedef const wchar_t* (__cdecl *BREGEXP_BRegexpVersionW2)(void);
	typedef int            (__cdecl *BREGEXP_BMatchExW2)      (const wchar_t* str, const wchar_t* targetbeg, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);
	typedef int            (__cdecl *BREGEXP_BSubstExW2)      (const wchar_t* str, const wchar_t* targetbeg, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);

public:
	// UNICODEインターフェースを提供する
	int BMatch(const wchar_t* str, const wchar_t* target,const wchar_t* targetendp,BREGEXP_W** rxp,wchar_t* msg)
	{
		return m_BMatch(str,target,targetendp,rxp,msg);
	}
	int BSubst(const wchar_t* str, const wchar_t* target,const wchar_t* targetendp,BREGEXP_W** rxp,wchar_t* msg)
	{
		return m_BSubst(str,target,targetendp,rxp,msg);
	}
	void BRegfree(BREGEXP_W* rx)
	{
		return m_BRegfree(rx);
	}
	const wchar_t* BRegexpVersion(void)
	{
		return m_BRegexpVersion();
	}
	int BMatchEx(const wchar_t* str, const wchar_t* targetbeg, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg)
	{
		return m_BMatchEx(str,targetbeg,target,targetendp,rxp,msg);
	}
	int BSubstEx(const wchar_t* str, const wchar_t* targetbeg, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg)
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
	BREGEXP_BRegfreeW2       m_BRegfree;
	BREGEXP_BRegexpVersionW2 m_BRegexpVersion;
	BREGEXP_BMatchExW2       m_BMatchEx;
	BREGEXP_BSubstExW2       m_BSubstEx;
};

#endif /* SAKURA_CBREGEXPDLL2_033C910A_6B78_47CB_9993_675C48A2AB64_H_ */
