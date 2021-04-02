﻿/*!	@file
	@brief アウトライン解析  データ要素

	@author Norio Nakatani
	@date	1998/06/23 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, Moca
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CFUNCINFO_9A6DFB84_EFEE_4E20_8E60_9F6F4D83B93C_H_
#define SAKURA_CFUNCINFO_9A6DFB84_EFEE_4E20_8E60_9F6F4D83B93C_H_
#pragma once

class CFuncInfo;

#include "mem/CMemory.h"
#include "mem/CNativeW.h"

// CDlgFuncList::SetTree()用 m_Info
#define FUNCINFO_INFOMASK	0xFFFF
//	2003.06.27 Moca
#define FUNCINFO_NOCLIPTEXT 0x10000

//! アウトライン解析  データ要素
//@date 2002.04.01 YAZAKI 深さ導入
class CFuncInfo {
	public:
		CFuncInfo( CLogicInt nFuncLineCRLF, CLogicInt nFuncColCRLF,
				   CLayoutInt nFuncLineLAYOUT, CLayoutInt nFuncColLAYOUT,
				   const WCHAR* pszFuncName, const WCHAR* pszFileName,
				   int nInfo );	/* CFuncInfoクラス構築 */
		~CFuncInfo();	/* CFuncInfoクラス消滅 */

		//! クリップボードに追加する要素か？
		//	2003.06.27 Moca
		inline bool IsAddClipText( void ) const{
			return ( FUNCINFO_NOCLIPTEXT != ( m_nInfo & FUNCINFO_NOCLIPTEXT ) );
		}

//	private:
		CLogicInt	m_nFuncLineCRLF;	/*!< 関数のある行(CRLF単位) */
		CLayoutInt	m_nFuncLineLAYOUT;	/*!< 関数のある行(折り返し単位) */
		CLogicInt	m_nFuncColCRLF;		/*!< 関数のある桁(CRLF単位) */
		CLayoutInt	m_nFuncColLAYOUT;	/*!< 関数のある桁(折り返し単位) */
		CNativeW	m_cmemFuncName;	/*!< 関数名 */
		CNativeW	m_cmemFileName;	/*!< ファイル名 */
		int			m_nInfo;		/*!< 付加情報 */
		int			m_nDepth;		/*!< 深さ */
};
#endif /* SAKURA_CFUNCINFO_9A6DFB84_EFEE_4E20_8E60_9F6F4D83B93C_H_ */
