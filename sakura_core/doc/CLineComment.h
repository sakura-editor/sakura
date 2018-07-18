/*!	@file
	@brief 行コメントデリミタを管理する

	@author Yazaki
	@date 2002/09/17 新規作成
*/
/*
	Copyright (C) 2002, Yazaki

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CLINECOMMENT_H_
#define _CLINECOMMENT_H_

//	sakura
#include "_main/global.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
#define COMMENT_DELIMITER_NUM	3
#define COMMENT_DELIMITER_BUFFERSIZE	16

/*! 行コメントデリミタを管理する

	@note CLineCommentは、共有メモリSTypeConfigに含まれるので、メンバ変数は常に実体を持っていなければならない。
*/
class CLineComment
{
public:
	/*
	||  Constructors：コンパイラ標準を使用。
	*/
	CLineComment();

	void CopyTo( const int n, const wchar_t* buffer, int nCommentPos );	//	行コメントデリミタをコピーする
	bool Match( int nPos, const CStringRef& cStr ) const;	//	行コメントに値するか確認する

	const wchar_t* getLineComment( const int n ){
		return m_pszLineComment[n];
	}
	int getLineCommentPos( const int n ) const {
		return m_nLineCommentPos[n];
	}

private:
	wchar_t	m_pszLineComment[COMMENT_DELIMITER_NUM][COMMENT_DELIMITER_BUFFERSIZE];	//!< 行コメントデリミタ
	int		m_nLineCommentPos[COMMENT_DELIMITER_NUM];	//!< 行コメントの開始位置(負数は指定無し)
	int		m_nLineCommentLen[COMMENT_DELIMITER_NUM];	//!< 行コメント文字列の長さ
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLINECOMMENT_H_ */



