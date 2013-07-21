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
#include "StdAfx.h"
#include "CLayout.h"
#include "Debug.h"



CLayout::~CLayout()
{
	return;
}

void CLayout::DUMP( void )
{
	DEBUG_TRACE( _T("\n\n■CLayout::DUMP()======================\n") );
	DEBUG_TRACE( _T("m_ptLogicPos.y=%d\t\t対応する論理行番号\n"), m_ptLogicPos.y );
	DEBUG_TRACE( _T("m_ptLogicPos.x=%d\t\t対応する論理行の先頭からのオフセット\n"), m_ptLogicPos.x );
	DEBUG_TRACE( _T("m_nLength=%d\t\t対応する論理行のハイト数\n"), m_nLength );
	DEBUG_TRACE( _T("m_nTypePrev=%d\t\tタイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 \n"), m_nTypePrev );
	DEBUG_TRACE( _T("======================\n") );
	return;
}


/*[EOF]*/
