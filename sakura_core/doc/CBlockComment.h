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
#include "_main/global.h"

enum ECommentType{
	COMMENT_TYPE_0	= 0,
	COMMENT_TYPE_1	= 1,
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*! ブロックコメントデリミタを管理する

	@note CBlockCommentGroupは、共有メモリSTypeConfigに含まれるので、メンバ変数は常に実体を持っていなければならない。
*/
#define BLOCKCOMMENT_NUM	2
#define BLOCKCOMMENT_BUFFERSIZE	16

//	2005.11.10 Moca アクセス関数追加
class CBlockComment{
public:
	//生成と破棄
	CBlockComment();

	//設定
	void SetBlockCommentRule( const wchar_t* pszFrom, const wchar_t* pszTo );	//	行コメントデリミタをコピーする

	//判定
	bool Match_CommentFrom( int nPos, const CStringRef& cStr ) const;	//	行コメントに値するか確認する
	int Match_CommentTo( int nPos, const CStringRef& cStr ) const;	//	行コメントに値するか確認する

	//取得
	const wchar_t* getBlockCommentFrom() const{ return m_szBlockCommentFrom; }
	const wchar_t* getBlockCommentTo() const{ return m_szBlockCommentTo; }
	int getBlockFromLen() const { return m_nBlockFromLen; }
	int getBlockToLen() const { return m_nBlockToLen; }

private:
	wchar_t	m_szBlockCommentFrom[BLOCKCOMMENT_BUFFERSIZE]; //!< ブロックコメントデリミタ(From)
	wchar_t	m_szBlockCommentTo[BLOCKCOMMENT_BUFFERSIZE];   //!< ブロックコメントデリミタ(To)
	int		m_nBlockFromLen;
	int		m_nBlockToLen;
};

///////////////////////////////////////////////////////////////////////
#endif /* _CBLOCKCOMMENT_H_ */



