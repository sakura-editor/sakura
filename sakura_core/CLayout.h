/*!	@file
	@brief テキストのレイアウト情報

	@author Norio Nakatani
	@date 1998/3/11 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CLayout;
class CLayoutMgr;

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
	//2007.08.23 kobake コンストラクタでメンバ変数を初期化するようにした
	CLayout(
		const CDocLine*	pcDocLine,		//!< 実データへの参照
		CLogicPoint		ptLogicPos,		//!< 実データ参照位置
		CLogicInt		nLength,		//!< 実データ内データ長
		EColorIndexType	nTypePrev,
		CLayoutInt		nTypeIndent
	)
	{
		m_pPrev			= NULL;
		m_pNext			= NULL;
		m_pCDocLine		= pcDocLine;
		m_ptLogicPos	= ptLogicPos;	// 実データ参照位置
		m_nLength		= nLength;		// 実データ内データ長
		m_nTypePrev		= nTypePrev;	// タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
		m_nIndent		= nTypeIndent;	// このレイアウト行のインデント数 @@@ 2002.09.23 YAZAKI
	}
	~CLayout();
	void DUMP( void );
	
	// m_ptLogicPos.xで補正したあとの文字列を得る
	const wchar_t* GetPtr() const   { return m_pCDocLine->m_cLine.GetStringPtr() + m_ptLogicPos.x; };
	int GetLengthWithEOL() const    { return m_nLength;	};	//	ただしEOLは常に1文字とカウント？？
	int GetLengthWithoutEOL() const { return m_nLength - (m_cEol.GetLen() ? 1 : 0);	};
	CLogicInt GetLength() const {	return m_nLength;	};	//	CMemoryIterator用（EOL含む）
	CLayoutInt GetIndent() const {	return m_nIndent;	};	//!< このレイアウト行のインデントサイズを取得。単位は半角文字。	CMemoryIterator用

	//取得インターフェース
	CLogicInt GetLogicLineNo() const{ return m_ptLogicPos.GetY2(); }
	CLogicInt GetLogicOffset() const{ return m_ptLogicPos.GetX2(); }
	CLogicPoint GetLogicPos() const{ return m_ptLogicPos; }
	EColorIndexType GetColorTypePrev() const{ return m_nTypePrev; }

	//変更インターフェース
	void OffsetLogicLineNo(CLogicInt n){ m_ptLogicPos.y+=n; }
	void SetColorTypePrev(EColorIndexType n){ m_nTypePrev=n; }

	//!レイアウト幅を計算。インデントも改行も含まない。2007.10.11 kobake
	CLayoutInt CalcLayoutWidth(const CLayoutMgr& cLayoutMgr) const;

	//! オフセット値をレイアウト単位に変換して取得。2007.10.17 kobake
	CLayoutInt CalcLayoutOffset(const CLayoutMgr& cLayoutMgr) const;

	//! 文字列参照を取得
	CStringRef GetStringRef() const{ return CStringRef(GetPtr(), GetLength()); }
public:
	CLayout*		m_pPrev;
	CLayout*		m_pNext;
	const CDocLine*	m_pCDocLine;

private:
	CLogicPoint			m_ptLogicPos;		//!< 対応するロジック参照位置
	CLogicInt			m_nLength;			//!< このレイアウト行の長さ。文字単位。
	EColorIndexType		m_nTypePrev;		//!< タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
	CLayoutInt			m_nIndent;			//!< このレイアウト行のインデント数 @@@ 2002.09.23 YAZAKI

public:
	CEOL			m_cEol;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUT_H_ */


/*[EOF]*/
