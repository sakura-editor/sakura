//	$Id$
/************************************************************************

	CFuncInfo.h

	アウトライン解析　データ要素
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/23  新規作成

************************************************************************/

class CFuncInfo;

#ifndef _CFUNCINFO_H_
#define _CFUNCINFO_H_

#include "CMemory.h"




/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/* アンドゥ・リドゥバッファ */
class CFuncInfo {
	public:
		CFuncInfo( int, int, char*, int );	/* CFuncInfoクラス構築 */
		~CFuncInfo();	/* CFuncInfoクラス消滅 */

//	private:
		int			m_nFuncLineCRLF;	/* 関数のある行(CRLF単位) */
		int			m_nFuncLineLAYOUT;	/* 関数のある行(折り返し単位) */
		CMemory		m_cmemFuncName;	/* 関数名 */
		int			m_nInfo;		/* 付加情報 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFO_H_ */

/*[EOF]*/
