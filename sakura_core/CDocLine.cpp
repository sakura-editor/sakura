//	$Id$
/*!	@file
	@brief 文書データ1行

	@author Norio Nakatani
	
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CDocLine.h"
#include "CMemory.h"

CDocLine::CDocLine()
	: m_pPrev( NULL ), m_pNext( NULL ), m_pLine( NULL ),
	m_bModify(true),
	m_bBookMark(false)
{
}
CDocLine::~CDocLine()
{
	//	deleteはNULLを単純に無視するのでNULL判定は不要
	delete m_pLine;
}


/*[EOF]*/
