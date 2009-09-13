/*!	@file
	@brief テキストのレイアウト情報

	@author Norio Nakatani
	@date 1998/3/11 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CLayout.h"
#include "debug.h"



CLayout::CLayout()
{
	m_pPrev = NULL;
	m_pNext = NULL;
	m_nLinePhysical = 0;	/* 対応する論理行番号 */
	m_pCDocLine = NULL;
	m_nOffset = 0;	/* 対応する論理行の先頭からのオフセット */
	m_nLength = 0;	/* 対応する論理行のハイト数 */
	m_nIndent = 0;	/* このレイアウト行のインデント数 @@@ 2002.09.23 YAZAKI */
	m_nTypePrev = 0;/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
	m_nLayoutWidth = 0;		/* このレイアウト行の改行を含むレイアウト長（「折り返さない」選択時のみ） */	// 2009.08.28 nasukoji
	return;
}


CLayout::~CLayout()
{
	return;
}

void CLayout::DUMP( void )
{
#ifdef _DEBUG
	MYTRACE( "\n\n■CLayout::DUMP()======================\n" );
	MYTRACE( "m_nLinePhysical=%d\t\t対応する論理行番号\n", m_nLinePhysical );
	MYTRACE( "m_nOffset=%d\t\t対応する論理行の先頭からのオフセット\n", m_nOffset );
	MYTRACE( "m_nLength=%d\t\t対応する論理行のハイト数\n", m_nLength );
	MYTRACE( "m_nTypePrev=%d\t\tタイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 \n", m_nTypePrev );
	MYTRACE( "======================\n" );
#endif
	return;
}


/*[EOF]*/
