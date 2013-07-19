/*!	@file
	@brief テキストのレイアウト情報

	@author Norio Nakatani
	@date 1998/3/11 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CLayout;

#ifndef _CLAYOUT_H_
#define _CLAYOUT_H_



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
	CLayout(
	)
	{
		m_pPrev = NULL;
		m_pNext = NULL;
		m_pCDocLine = NULL;
		m_nOffset = 0;	// 対応する論理行の先頭からのオフセット
		m_nLinePhysical = 0;	// 対応する論理行番号
		m_nLength = 0;	// 対応する論理行のハイト数
		m_nTypePrev = COLORIDX_DEFAULT;// タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
		m_nIndent = 0;	// このレイアウト行のインデント数 @@@ 2002.09.23 YAZAKI */
		m_nLayoutWidth = 0;		// このレイアウト行の改行を含むレイアウト長（「折り返さない」選択時のみ）	// 2009.08.28 nasukoji
	}
	~CLayout();
	void DUMP( void );
	
	// m_nOffsetで補正したあとの文字列を得る
	char*	GetPtr() const {	return m_pCDocLine->m_cLine.GetStringPtr() + m_nOffset;	}
	int		GetLengthWithEOL() const {	return m_nLength;	}	//	ただしEOLは常に1文字とカウント？？
	int		GetLengthWithoutEOL() const {	return m_nLength - (m_cEol.GetLen() ? 1 : 0);	}
	int		GetLength() const {	return m_nLength;	}	//	CMemoryIterator用（EOL含む）
	int		GetIndent() const {	return m_nIndent;	}	//	CMemoryIterator用

public:
	CLayout*		m_pPrev;
	CLayout*		m_pNext;

	const CDocLine*	m_pCDocLine;		//!< 実データへの参照
	int				m_nOffset;			//!< 対応する改行単位の行頭からのオフセット
	int				m_nLinePhysical;	//!< 対応する改行単位の行の番号
	int				m_nLength;			//!< このレイアウト行の長さ(ハイト数)

	EColorIndexType	m_nTypePrev;		//!< タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
	int				m_nIndent;			//!< このレイアウト行のインデント数 @@@ 2002.09.23 YAZAKI
	CEol			m_cEol;
	int				m_nLayoutWidth;		//!< このレイアウト行の改行を含むレイアウト長（「折り返さない」選択時のみ） // 2009.08.28 nasukoji
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUT_H_ */


/*[EOF]*/
