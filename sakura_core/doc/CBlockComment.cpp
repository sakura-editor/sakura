/*!	@file
	@brief ブロックコメントデリミタを管理する

	@author Yazaki
	@date 2002/09/17 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, Moca
	Copyright (C) 2005, D.S.Koba

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CBlockComment.h"
#include "mem/CMemory.h"

CBlockComment::CBlockComment()
{
	m_szBlockCommentFrom[ 0 ] = '\0';
	m_szBlockCommentTo[ 0 ] = '\0';
	m_nBlockFromLen = 0;
	m_nBlockToLen = 0;
}

/*!
	ブロックコメントデリミタをコピーする
*/
void CBlockComment::SetBlockCommentRule(
	const wchar_t*	pszFrom,	//!< [in] コメント開始文字列
	const wchar_t*	pszTo		//!< [in] コメント終了文字列
)
{
	int nStrLen = wcslen( pszFrom );
	if( 0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE ){
		wcscpy( m_szBlockCommentFrom, pszFrom );
		m_nBlockFromLen = nStrLen;
	}
	else {
		m_szBlockCommentFrom[0] = L'\0';
		m_nBlockFromLen = 0;
	}
	nStrLen = wcslen( pszTo );
	if( 0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE ){
		wcscpy( m_szBlockCommentTo, pszTo );
		m_nBlockToLen = nStrLen;
	}
	else {
		m_szBlockCommentTo[0] = L'\0';
		m_nBlockToLen = 0;
	}
}

/*!
	n番目のブロックコメントの、nPosからの文字列が開始文字列(From)に当てはまるか確認する。

	@retval true  一致した
	@retval false 一致しなかった
*/
bool CBlockComment::Match_CommentFrom(
	int					nPos,		//!< [in] 探索開始位置
	const CStringRef&	cStr		//!< [in] 探索対象文字列 ※探索開始位置のポインタではないことに注意
	/*
	int				nLineLen,	//!< [in] pLineの長さ
	const wchar_t*	pLine		//!< [in] 探索行の先頭．
	*/
) const
{
	if (
		L'\0' != m_szBlockCommentFrom[0] &&
		L'\0' != m_szBlockCommentTo[0]  &&
		nPos <= cStr.GetLength() - m_nBlockFromLen &&	/* ブロックコメントデリミタ(From) */
		//0 == auto_memicmp( &cStr.GetPtr()[nPos], m_szBlockCommentFrom, m_nBlockFromLen )	//非ASCIIも大文字小文字を区別しない	//###locale 依存
		0 == wmemicmp_ascii( &cStr.GetPtr()[nPos], m_szBlockCommentFrom, m_nBlockFromLen )	//ASCIIのみ大文字小文字を区別しない（高速）
	){
		return true;
	}
	return false;
}

/*!
	n番目のブロックコメントの、後者(To)に当てはまる文字列をnPos以降から探す

	@return 当てはまった位置を返すが、当てはまらなかったときは、nLineLenをそのまま返す。
*/
int CBlockComment::Match_CommentTo(
	int					nPos,		//!< [in] 探索開始位置
	const CStringRef&	cStr		//!< [in] 探索対象文字列 ※探索開始位置のポインタではないことに注意
	/*
	int				nLineLen,	//!< [in] pLineの長さ
	const wchar_t*	pLine		//!< [in] 探索行の先頭．探索開始位置のポインタではないことに注意
	*/
) const
{
	for( int i = nPos; i <= cStr.GetLength() - m_nBlockToLen; ++i ){
		//if( 0 == auto_memicmp( &cStr.GetPtr()[i], m_szBlockCommentTo, m_nBlockToLen ) ){	//非ASCIIも大文字小文字を区別しない	//###locale 依存
		if( 0 == wmemicmp_ascii( &cStr.GetPtr()[i], m_szBlockCommentTo, m_nBlockToLen ) ){	//ASCIIのみ大文字小文字を区別しない（高速）
			return i + m_nBlockToLen;
		}
	}
	return cStr.GetLength();
}

