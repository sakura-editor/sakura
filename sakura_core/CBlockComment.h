//	$Id$
/*!	@file
	@brief �u���b�N�R�����g�f���~�^���Ǘ�����

	@author Yazaki
	@date 2002/09/17 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 2002, Yazaki

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CBLOCKCOMMENT_H_
#define _CBLOCKCOMMENT_H_

//	sakura
#include "global.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*! �u���b�N�R�����g�f���~�^���Ǘ�����

	@note CBlockComment�́A���L������Types�Ɋ܂܂��̂ŁA�����o�ϐ��͏�Ɏ��̂������Ă��Ȃ���΂Ȃ�Ȃ��B
*/
#define BLOCKCOMMENT_NUM	2
#define BLOCKCOMMENT_BUFFERSIZE	16

class SAKURA_CORE_API CBlockComment
{
	char	m_szBlockCommentFrom[BLOCKCOMMENT_NUM][BLOCKCOMMENT_BUFFERSIZE];		/*!< �u���b�N�R�����g�f���~�^(From) */
	char	m_szBlockCommentTo[BLOCKCOMMENT_NUM][BLOCKCOMMENT_BUFFERSIZE];			/*!< �u���b�N�R�����g�f���~�^(To) */	int		m_nBlockFromLen[BLOCKCOMMENT_NUM];
	int		m_nBlockToLen[BLOCKCOMMENT_NUM];

	friend class CShareData;
public:
	/*
	||  Constructors�F�R���p�C���W�����g�p�B
	*/
	CBlockComment();

	void CopyTo( const int n, const char* pszFrom, const char* pszTo );	//	�s�R�����g�f���~�^���R�s�[����
	bool Match_CommentFrom( int n, int nPos, int nLineLen, const char* pLine ) const;	//	�s�R�����g�ɒl���邩�m�F����	
	int Match_CommentTo( int n, int nPos, int nLineLen, const char* pLine ) const;	//	�s�R�����g�ɒl���邩�m�F����

	const char* getBlockCommentFrom( const int n ){
		return m_szBlockCommentFrom[n];
	};
	const char* getBlockCommentTo( const int n ){
		return m_szBlockCommentTo[n];
	};
};


///////////////////////////////////////////////////////////////////////
#endif /* _CBLOCKCOMMENT_H_ */


/*[EOF]*/
