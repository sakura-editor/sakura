//	$Id$
/************************************************************************

	COpe.cpp

	編集操作要素
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/9  新規作成

************************************************************************/
#include "COpe.h"
#include "debug.h"


/* COpeクラス構築 */
COpe::COpe()
{
	m_nOpe = 0;					/* 操作種別 */
//	m_nCaretPosX_Before = 0;	/* 操作前のキャレット位置Ｘ */
//	m_nCaretPosY_Before = 0;	/* 操作前のキャレット位置Ｙ */
//	m_nCaretPosX_To = 0;		/* 操作前のキャレット位置Ｘ To */
//	m_nCaretPosY_To = 0;		/* 操作前のキャレット位置Ｙ To */
//	m_nCaretPosX_After = 0; 	/* 操作後のキャレット位置Ｘ */
//	m_nCaretPosY_After = 0; 	/* 操作後のキャレット位置Ｙ */

	m_nCaretPosX_PHY_To = 0;	/* 操作前のキャレット位置Ｘ To 改行単位行の行番号（０開始）*/
	m_nCaretPosY_PHY_To = 0;	/* 操作前のキャレット位置Ｙ To 改行単位行先頭からのバイト数（０開始）*/
	m_nCaretPosX_PHY_Before = -1;	/* カーソル位置　改行単位行先頭からのバイト数（０開始） */
	m_nCaretPosY_PHY_Before = -1;	/* カーソル位置　改行単位行の行番号（０開始） */
	m_nCaretPosX_PHY_After = -1;		/* カーソル位置　改行単位行先頭からのバイト数（０開始） */
	m_nCaretPosY_PHY_After = -1;		/* カーソル位置　改行単位行の行番号（０開始） */
//	m_nOpePosX = 0;				/* 操作位置Ｘ */
//	m_nOpePosY = 0;				/* 操作位置Ｙ */
	m_nDataLen = 0;				/* 操作に関連するデータのサイズ */
	m_pcmemData = NULL;			/* 操作に関連するデータ */
	return;
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
	MYTRACE( "\t\tm_nOpe              = [%d]\n", m_nOpe               );
	MYTRACE( "\t\tm_nCaretPosX_PHY_Before  = [%d]\n", m_nCaretPosX_PHY_Before   );
	MYTRACE( "\t\tm_nCaretPosY_PHY_Before  = [%d]\n", m_nCaretPosY_PHY_Before   );
	MYTRACE( "\t\tm_nCaretPosX_PHY_After;  = [%d]\n", m_nCaretPosX_PHY_After   );
	MYTRACE( "\t\tm_nCaretPosY_PHY_After;  = [%d]\n", m_nCaretPosY_PHY_After   );
	MYTRACE( "\t\tm_nDataLen          = [%d]\n", m_nDataLen           );
	if( NULL == m_pcmemData ){
		MYTRACE( "\t\tm_pcmemData         = [NULL]\n" );
	}else{
		MYTRACE( "\t\tm_pcmemData         = [%s]\n", m_pcmemData->GetPtr( NULL ) );
	}
#endif
	return;
}




/*[EOF]*/
