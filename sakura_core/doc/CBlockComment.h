/*!	@file
	@brief ブロックコメントデリミタを管理する

	@author Yazaki
	@date 2002/09/17 新規作成
*/
/*
	Copyright (C) 2002, Yazaki
	Copyright (C) 2005, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CBLOCKCOMMENT_H_
#define _CBLOCKCOMMENT_H_

//	sakura
#include "global.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*! ブロックコメントデリミタを管理する

	@note CBlockCommentは、共有メモリTypesに含まれるので、メンバ変数は常に実体を持っていなければならない。
*/
#define BLOCKCOMMENT_NUM	2
#define BLOCKCOMMENT_BUFFERSIZE	16

class SAKURA_CORE_API CBlockComment
{
	wchar_t	m_szBlockCommentFrom[BLOCKCOMMENT_NUM][BLOCKCOMMENT_BUFFERSIZE]; //!< ブロックコメントデリミタ(From)
	wchar_t	m_szBlockCommentTo[BLOCKCOMMENT_NUM][BLOCKCOMMENT_BUFFERSIZE];   //!< ブロックコメントデリミタ(To)
	int		m_nBlockFromLen[BLOCKCOMMENT_NUM];
	int		m_nBlockToLen[BLOCKCOMMENT_NUM];

public:
	/*
	||  Constructors
	*/
	CBlockComment();

	void CopyTo( const int n, const wchar_t* pszFrom, const wchar_t* pszTo );	//	行コメントデリミタをコピーする
	bool Match_CommentFrom( int n, int nPos, int nLineLen, const wchar_t* pLine ) const;	//	行コメントに値するか確認する	
	int Match_CommentTo( int n, int nPos, int nLineLen, const wchar_t* pLine ) const;	//	行コメントに値するか確認する

	const wchar_t* getBlockCommentFrom( const int n ){
		return m_szBlockCommentFrom[n];
	};
	const wchar_t* getBlockCommentTo( const int n ){
		return m_szBlockCommentTo[n];
	};
	//	2005.11.10 Moca アクセス関数追加
	int getBlockFromLen( int n ) const {
		return m_nBlockFromLen[n];
	};
	int getBlockToLen( int n ) const {
		return m_nBlockToLen[n];
	};
};


///////////////////////////////////////////////////////////////////////
#endif /* _CBLOCKCOMMENT_H_ */



