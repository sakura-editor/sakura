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
#include "CMemory.h"
#include "my_icmp.h" // 2002/11/30 Moca 追加

CBlockComment::CBlockComment()
{
	int i;
	for ( i=0; i<BLOCKCOMMENT_NUM; i++ ){
		m_szBlockCommentFrom[ i ][ 0 ] = '\0';
		m_szBlockCommentTo[ i ][ 0 ] = '\0';
		m_nBlockFromLen[ i ] = 0;
		m_nBlockToLen[ i ] = 0;
	}
}

/*!
	ブロックコメントデリミタをコピーする
*/
void CBlockComment::CopyTo(
	const int	n,			//!< [in] コピー対象のコメント番号
	const char*	pszFrom,	//!< [in] コメント開始文字列
	const char*	pszTo		//!< [in] コメント終了文字列
)
{
	int nStrLen = _tcslen( pszFrom );
	if( 0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE ){
		_tcscpy( m_szBlockCommentFrom[n], pszFrom );
		m_nBlockFromLen[ n ] = nStrLen;
	}
	else {
		m_szBlockCommentFrom[n][0] = '\0';
		m_nBlockFromLen[n] = 0;
	}
	nStrLen = _tcslen( pszTo );
	if( 0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE ){
		_tcscpy( m_szBlockCommentTo[n], pszTo );
		m_nBlockToLen[ n ] = nStrLen;
	}
	else {
		m_szBlockCommentTo[n][0] = '\0';
		m_nBlockToLen[n] = 0;
	}
}

/*!
	n番目のブロックコメントの、nPosからの文字列が開始文字列(From)に当てはまるか確認する。

	@retval true  一致した
	@retval false 一致しなかった
*/
bool CBlockComment::Match_CommentFrom(
	int			n,				//!< [in] 検査対象のコメント番号
	int			nPos,			//!< [in] 探索開始位置
	int			nLineLen,		//!< [in] pLineの長さ
	const char*	pLine			//!< [in] 探索行の先頭．探索開始位置のポインタではないことに注意
) const
{
	if (
		'\0' != m_szBlockCommentFrom[n][0] &&
		'\0' != m_szBlockCommentTo[n][0]  &&
		nPos <= nLineLen - m_nBlockFromLen[n] &&	/* ブロックコメントデリミタ(From) */
		0 == my_memicmp( &pLine[nPos], m_szBlockCommentFrom[n], m_nBlockFromLen[n] )
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
	int			n,			//!< [in] 検査対象のコメント番号
	int			nPos,		//!< [in] 探索開始位置
	int			nLineLen,	//!< [in] pLineの長さ
	const char*	pLine		//!< [in] 探索行の先頭．探索開始位置のポインタではないことに注意
) const
{
	int i;
	for( i = nPos; i <= nLineLen - m_nBlockToLen[n]; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		int nCharChars_2 = CMemory::GetSizeOfChar( (const char *)pLine, nLineLen, i );
		if( 0 == nCharChars_2 ){
			nCharChars_2 = 1;
		}
		if( 0 == my_memicmp( &pLine[i], m_szBlockCommentTo[n], m_nBlockToLen[n] ) ){
			return i + m_nBlockToLen[n];
		}
		if( 2 == nCharChars_2 ){
			++i;
		}
	}
	return nLineLen;
}
/*[EOF]*/
