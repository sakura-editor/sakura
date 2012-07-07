/*!	@file
	@brief 編集操作要素

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "COpe.h"
#include "Debug.h"
#include "CMemory.h"// 2002/2/10 aroka


// COpeクラス構築
COpe::COpe()
{
	m_nOpe = OPE_UNKNOWN;					// 操作種別

	m_nCaretPosX_PHY_To = 0;		// 操作前のキャレット位置Ｘ To 改行単位行の行番号（０開始）
	m_nCaretPosY_PHY_To = 0;		// 操作前のキャレット位置Ｙ To 改行単位行先頭からのバイト数（０開始）
	m_nCaretPosX_PHY_Before = -1;	// カーソル位置 改行単位行先頭からのバイト数（０開始）
	m_nCaretPosY_PHY_Before = -1;	// カーソル位置 改行単位行の行番号（０開始）
	m_nCaretPosX_PHY_After = -1;	// カーソル位置 改行単位行先頭からのバイト数（０開始）
	m_nCaretPosY_PHY_After = -1;	// カーソル位置 改行単位行の行番号（０開始）
	m_nDataLen = 0;					// 操作に関連するデータのサイズ 
	m_pcmemData = NULL;				// 操作に関連するデータ 
}




/* COpeクラス消滅 */
COpe::~COpe()
{
	if( NULL != m_pcmemData ){	/* 操作に関連するデータ */
		delete m_pcmemData;
		m_pcmemData = NULL;
	}
	return;
}

/* 編集操作要素のダンプ */
void COpe::DUMP( void )
{
#ifdef _DEBUG
	MYTRACE_A( "\t\tm_nOpe              = [%d]\n", m_nOpe               );
	MYTRACE_A( "\t\tm_nCaretPosX_PHY_Before = [%d]\n", m_nCaretPosX_PHY_Before   );
	MYTRACE_A( "\t\tm_nCaretPosY_PHY_Before = [%d]\n", m_nCaretPosY_PHY_Before   );
	MYTRACE_A( "\t\tm_nCaretPosX_PHY_After; = [%d]\n", m_nCaretPosX_PHY_After   );
	MYTRACE_A( "\t\tm_nCaretPosY_PHY_After; = [%d]\n", m_nCaretPosY_PHY_After   );
	MYTRACE_A( "\t\tm_nDataLen              = [%d]\n", m_nDataLen           );
	if( NULL == m_pcmemData ){
		MYTRACE_A( "\t\tm_pcmemData         = [NULL]\n" );
	}else{
		MYTRACE_A( "\t\tm_pcmemData         = [%s]\n", m_pcmemData->GetStringPtr() );
	}
#endif
	return;
}


/*[EOF]*/
