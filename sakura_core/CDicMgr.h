//	$Id$
/************************************************************************

	CDicMgr.h


    UPDATE:
    CREATE: 1998/11/05  新規作成
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CDicMgr;

#ifndef _CDICMGR_H_
#define _CDICMGR_H_

#include <windows.h>
#include "CMemory.h"
#include "debug.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDicMgr
{
public:
	/*
	||  Constructors
	*/
	CDicMgr();
	~CDicMgr();

	/*
	||  Attributes & Operations
	*/
//	BOOL Open( char* );
	static BOOL Search( const char*, CMemory**, const char* );
	static int HokanSearch( const char* , BOOL, CMemory** , int, const char* );
//	BOOL Close( char* );


protected:
	/*
	||  実装ヘルパ関数
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDICMGR_H_ */

/*[EOF]*/

