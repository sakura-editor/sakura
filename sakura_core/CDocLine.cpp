//	$Id$
/************************************************************************
	CDocLine.cpp
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/

#include "CDocLine.h"

CDocLine::CDocLine()
{
	m_pPrev = NULL;
	m_pNext = NULL;
	m_pLine = NULL;
//	m_nType = 0;	/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
	m_bModify = TRUE;	/* 変更フラグ */
//	m_enumEOLType = EOL_NONE;	/* 改行コードの種類 */
//	m_nEOLLen = gm_pnEolLenArr[EOL_NONE];
//	m_nModifyCount = 32000;	/* 変更回数 */
	return;
}


CDocLine::~CDocLine()
{
	if( NULL != m_pLine ){
		delete m_pLine;
		m_pLine = NULL;
	}
	return;
}


/*[EOF]*/
