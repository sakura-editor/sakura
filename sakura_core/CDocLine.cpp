//	$Id$
/*!	@file
	@brief 文書データ1行

	@author Norio Nakatani
	
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor, genta
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CDocLine.h"
#include "CMemory.h"

CDocLine::CDocLine()
	: m_pPrev( NULL ), m_pNext( NULL ), m_pLine( NULL )
{
//	m_bMark.m_bAllMark  = 0;
	m_bMark.m_bMarkArray.m_bModify   = 1;	//true
	m_bMark.m_bMarkArray.m_bBookMark = 0;	//false
	m_bMark.m_bMarkArray.m_bDiffMark = 0;
}

CDocLine::~CDocLine()
{
	//	deleteはNULLを単純に無視するのでNULL判定は不要
	delete m_pLine;
}

/* 空行（スペース、タブ、改行記号のみの行）かどうかを取得する
	true：空行だ。
	false：空行じゃないぞ。

	2002/04/26 YAZAKI
*/
bool CDocLine::IsEmptyLine( void )
{
	char* pLine = GetPtr();
	int nLineLen = GetLengthWithoutEOL();
	int i;
	for ( i = 0; i < nLineLen; i++ ){
		if (pLine[i] != ' ' && pLine[i] != '\t'){
			return false;	//	スペースでもタブでもない文字があったらfalse。
		}
	}
	return true;	//	すべてスペースかタブだけだったらtrue。
}

/*[EOF]*/
