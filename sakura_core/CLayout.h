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

//#include "CDocLine.h"
#include "CEol.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/4/21 YAZAKI
#include "CMemory.h"// 2002/4/21 YAZAKI
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
	
	/* m_nOffsetで補正したあとの文字列を得る */
	char*	GetPtr() const {	return m_pCDocLine->m_pLine->GetPtr() + m_nOffset;	};
	int		GetLengthWithEOL() const {	return m_nLength;	};	//	ただしEOLは常に1文字とカウント？？
	int		GetLengthWithoutEOL() const {	return m_nLength - (m_cEol.GetLen() ? 1 : 0);	};

public:
	CLayout*		m_pPrev;
	CLayout*		m_pNext;
	int				m_nLinePhysical;		/*!< 対応する改行単位の行の番号 */
	const CDocLine*	m_pCDocLine;
//	const char*		m_pLine;
	int				m_nOffset;		/*!< 対応する改行単位の行頭からのオフセット */
	int				m_nLength;		/*!< このレイアウト行の長さ(ハイト数) */
	int				m_nTypePrev;	/*!< タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
	int				m_nTypeNext;	/*!< タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
//	enumEOLType		m_enumEOLType;	/*!< 改行コードの種類 */
//	int				m_nEOLLen;		/*!< 改行コードの長さ */
	CEOL			m_cEol;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUT_H_ */


/*[EOF]*/
