// $Id$
/*!	@file
	@brief CLayout��CDocLine�̃C�e���[�^

	@author Yazaki
	@date 2002/09/25 �V�K�쐬
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
�N���X�̐錾
-----------------------------------------------------------------------*/
/*! �u���b�N�R�����g�f���~�^���Ǘ�����

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

	/*! ���ʒu���s�̐擪�ɃZ�b�g  */
	void first(){
		m_nIndex = 0;
		m_nColumn = m_nIndent;
		m_nIndex_Delta = 0;
		m_nColumn_Delta = 0;
	};
	
	/*! �s�����ǂ���
		@return true: �s��, false: �s���ł͂Ȃ�
	 */
	bool end() const {
		return (m_nLineLen <= m_nIndex);
	};
	void scanNext(){	//	���̕������m�F���Ď��̕����Ƃ̍������߂�
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
	
	/*! �\�ߌv�Z�������������ʒu�ɉ�����D
		@sa scanNext()
	 */
	void addDelta(){
		m_nColumn += m_nColumn_Delta;
		m_nIndex += m_nIndex_Delta;
	};	//	�|�C���^�����炷
	
	int getIndex() const {	return m_nIndex;	};
	int getColumn() const {	return m_nColumn;	};
	int getIndexDelta() const {	return m_nIndex_Delta;	};
	int getColumnDelta() const {	return m_nColumn_Delta;	};

	//	2002.10.07 YAZAKI
	const char getCurrentChar(){	return m_pLine[m_nIndex];	};
	//	Jul. 20, 2003 genta �ǉ�
	//	memcpy������̂Ƀ|�C���^���Ƃ�Ȃ��Ɩʓ|
	const char* getCurrentPos(){	return m_pLine + m_nIndex;	};
};


///////////////////////////////////////////////////////////////////////
#endif /* _CBLOCKCOMMENT_H_ */


/*[EOF]*/
