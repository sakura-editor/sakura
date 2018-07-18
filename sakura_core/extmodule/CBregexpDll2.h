/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CBREGEXPDLL2_850005D4_6AA3_41D2_B541_1EE730935E6B_H_
#define SAKURA_CBREGEXPDLL2_850005D4_6AA3_41D2_B541_1EE730935E6B_H_

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
	// CDllImpインタフェース
	virtual LPCTSTR GetDllNameImp(int nIndex); // Jul. 5, 2001 genta インターフェース変更に伴う引数追加
	virtual bool InitDllImp();

protected:
	// DLL関数の型
	typedef int            (__cdecl *BREGEXP_BMatchW2)        (const wchar_t* str, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);
	typedef int            (__cdecl *BREGEXP_BSubstW2)        (const wchar_t* str, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);
	typedef int            (__cdecl *BREGEXP_BTransW2)        (const wchar_t* str, wchar_t* target, wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);
	typedef int            (__cdecl *BREGEXP_BSplitW2)        (const wchar_t* str, wchar_t* target, wchar_t* targetendp, int limit, BREGEXP_W** rxp, wchar_t* msg);
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
	int BTrans(const wchar_t* str, wchar_t* target,wchar_t* targetendp,BREGEXP_W** rxp,wchar_t* msg)
	{
		return m_BTrans(str,target,targetendp,rxp,msg);
	}
	int BSplit(const wchar_t* str, wchar_t* target,wchar_t* targetendp,int limit,BREGEXP_W** rxp,wchar_t* msg)
	{
		return m_BSplit(str,target,targetendp,limit,rxp,msg);
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
	bool ExistBMatchEx() const{ return m_BMatchEx!=NULL; }
	bool ExistBSubstEx() const{ return m_BSubstEx!=NULL; }

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

#endif /* SAKURA_CBREGEXPDLL2_850005D4_6AA3_41D2_B541_1EE730935E6B_H_ */
/*[EOF]*/
