//	$Id$
/*!	@file
	@brief アウトライン解析  データ要素

	@author Norio Nakatani
	@date	1998/06/23 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CFuncInfo;

#ifndef _CFUNCINFO_H_
#define _CFUNCINFO_H_

#include "CMemory.h"

//! アウトライン解析  データ要素
//@date 2002.04.01 YAZAKI 深さ導入
class CFuncInfo {
	public:
		CFuncInfo( int, int, char*, int );	/* CFuncInfoクラス構築 */
		~CFuncInfo();	/* CFuncInfoクラス消滅 */

//	private:
		int			m_nFuncLineCRLF;	/*!< 関数のある行(CRLF単位) */
		int			m_nFuncLineLAYOUT;	/*!< 関数のある行(折り返し単位) */
		CMemory		m_cmemFuncName;	/*!< 関数名 */
		int			m_nInfo;		/*!< 付加情報 */
		int			m_nDepth;		/*!< 深さ */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFO_H_ */


/*[EOF]*/
