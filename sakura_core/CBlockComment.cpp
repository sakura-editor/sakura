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
	m_szBlockCommentFrom[ 0 ] = '\0';
	m_szBlockCommentTo[ 0 ] = '\0';
	m_nBlockFromLen = 0;
	m_nBlockToLen = 0;
}

/*!
	ブロックコメントデリミタをコピーする
*/
void CBlockComment::SetBlockCommentRule(
	const TCHAR*	pszFrom,	//!< [in] コメント開始文字列
	const TCHAR*	pszTo		//!< [in] コメント終了文字列
)
{
	int nStrLen = _tcslen( pszFrom );
	if( 0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE ){
		_tcscpy( m_szBlockCommentFrom, pszFrom );
		m_nBlockFromLen = nStrLen;
	}
	else {
		m_szBlockCommentFrom[0] = '\0';
		m_nBlockFromLen = 0;
	}
	nStrLen = _tcslen( pszTo );
	if( 0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE ){
		_tcscpy( m_szBlockCommentTo, pszTo );
		m_nBlockToLen = nStrLen;
	}
	else {
		m_szBlockCommentTo[0] = '\0';
		m_nBlockToLen = 0;
	}
}

/*!
	n番目のブロックコメントの、nPosからの文字列が開始文字列(From)に当てはまるか確認する。

	@retval true  一致した
	@retval false 一致しなかった
*/
bool CBlockComment::Match_CommentFrom(
	int				nPos,		//!< [in] 探索開始位置
	int				nLineLen,	//!< [in] pLineの長さ
	const TCHAR*	pLine		//!< [in] 探索行の先頭．探索開始位置のポインタではないことに注意
) const
{
	if (
		_T('\0') != m_szBlockCommentFrom[0] &&
		_T('\0') != m_szBlockCommentTo[0]  &&
		nPos <= nLineLen - m_nBlockFromLen &&	/* ブロックコメントデリミタ(From) */
		0 == my_memicmp( &pLine[nPos], m_szBlockCommentFrom, m_nBlockFromLen )
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
	int				nPos,		//!< [in] 探索開始位置
	int				nLineLen,	//!< [in] pLineの長さ
	const TCHAR*	pLine		//!< [in] 探索行の先頭．探索開始位置のポインタではないことに注意
) const
{
	int i;
	for( i = nPos; i <= nLineLen - m_nBlockToLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		int nCharChars_2 = CMemory::GetSizeOfChar( (const char *)pLine, nLineLen, i );
		if( 0 == nCharChars_2 ){
			nCharChars_2 = 1;
		}
		if( 0 == my_memicmp( &pLine[i], m_szBlockCommentTo, m_nBlockToLen ) ){
			return i + m_nBlockToLen;
		}
		if( 2 == nCharChars_2 ){
			++i;
		}
	}
	return nLineLen;
}
/*[EOF]*/
