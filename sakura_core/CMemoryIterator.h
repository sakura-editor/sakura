// $Id$
/*!	@file
	@brief CLayoutとCDocLineのイテレータ

	@author Yazaki
	@date 2002/09/25 新規作成
	$Revision$
*/
/*
	Copyright (C) 2002, Yazaki

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CMEMORYITERATOR_H_
#define _CMEMORYITERATOR_H_

//	sakura
#include "global.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*! ブロックコメントデリミタを管理する

*/
template <class T>
class SAKURA_CORE_API CMemoryIterator
{
	int m_nIndent;
	int m_nIndex;
	int m_nColumn;
	int m_nIndex_Delta;
	int m_nColumn_Delta;

	const char* m_pLine;
	const int m_nLineLen;
	const int m_nTabSpace;
public:
	CMemoryIterator( const T* pcT, int nTabSpace )
		: m_pLine( pcT ? pcT->GetPtr() : NULL ),
		  m_nLineLen( pcT ? pcT->GetLength() : 0 ),
		  m_nTabSpace( nTabSpace ),
		  m_nIndent( pcT ? pcT->GetIndent() : 0 )
	{
		first();
	};
	
	void first(){
		m_nIndex = 0;
		m_nColumn = m_nIndent;
		m_nIndex_Delta = 0;
		m_nColumn_Delta = 0;
	};
	bool end(){
		return (m_nLineLen <= m_nIndex);
	};
	void scanNext(){	//	次の文字を確認して次の文字との差を求める
		if (m_pLine[m_nIndex] == TAB){
			m_nIndex_Delta = 1;
			m_nColumn_Delta = m_nTabSpace - ( m_nColumn % m_nTabSpace );
		}
		else{
			m_nIndex_Delta = CMemory::MemCharNext( m_pLine, m_nLineLen, &m_pLine[m_nIndex] ) - &m_pLine[m_nIndex];
			if( 0 == m_nIndex_Delta ){
				m_nIndex_Delta = 1;
			}
			m_nColumn_Delta = m_nIndex_Delta;
		}
	};
	void addDelta(){
		m_nColumn += m_nColumn_Delta;
		m_nIndex += m_nIndex_Delta;
	};	//	ポインタをずらす
	
	int getIndex(){	return m_nIndex;	};
	int getColumn(){	return m_nColumn;	};
	int getIndexDelta(){	return m_nIndex_Delta;	};
	int getColumnDelta(){	return m_nColumn_Delta;	};
};


///////////////////////////////////////////////////////////////////////
#endif /* _CBLOCKCOMMENT_H_ */


/*[EOF]*/
