//	$Id$
/*!	@file
	@brief テキストのレイアウト情報

	@author Norio Nakatani
	@date 1998/3/11 新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CLayout.h"
#include "debug.h"



CLayout::CLayout()
{
	m_pPrev = NULL;
	m_pNext = NULL;
	m_nLinePhysical = 0;	/* 対応する論理行番号 */
	m_pCDocLine = NULL;
//	m_pLine = NULL;
	m_nOffset = 0;	/* 対応する論理行の先頭からのオフセット */
	m_nLength = 0;	/* 対応する論理行のハイト数 */
	m_nTypePrev = 0;/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
	m_nTypeNext = 0;/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
//	m_enumEOLType = EOL_NONE;	/* 改行コードの種類 */
//	m_nEOLLen = gm_pnEolLenArr[EOL_NONE];
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
	MYTRACE( "m_nTypeNext=%d\t\tタイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列\n", m_nTypeNext );
	MYTRACE( "======================\n" );
#endif
	return;
}


/*[EOF]*/
