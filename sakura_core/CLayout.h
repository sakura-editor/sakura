//	$Id$
/************************************************************************

	CLayout.h

    テキストのレイアウト情報
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/3/11  新規作成



************************************************************************/

class CLayout;

#ifndef _CLAYOUT_H_
#define _CLAYOUT_H_


#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL 0
#endif

#include "CDocLine.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CLayout
{
public:
	/*
	||  Constructors
	*/
	CLayout();
	~CLayout();
	void DUMP( void );
public:
	CLayout*		m_pPrev;
	CLayout*		m_pNext;
	int				m_nLinePhysical;		/* 対応する改行単位の行の番号 */
	const CDocLine*	m_pCDocLine;	
//	const char*		m_pLine;
	int				m_nOffset;		/* 対応する改行単位の行頭からのオフセット */
	int				m_nLength;		/* このレイアウト行の長さ(ハイト数) */
	int				m_nTypePrev;	/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
	int				m_nTypeNext;	/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
//	enumEOLType		m_enumEOLType;	/* 改行コードの種類 */
//	int				m_nEOLLen;		/* 改行コードの長さ */
	CEOL			m_cEol;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUT_H_ */

/*[EOF]*/
