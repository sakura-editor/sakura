/*!	@file
	@brief �u���b�N�R�����g�f���~�^���Ǘ�����

	@author Yazaki
	@date 2002/09/17 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, Moca
	Copyright (C) 2005, D.S.Koba

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CBlockComment.h"
#include "mem/CMemory.h"

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
	�u���b�N�R�����g�f���~�^���R�s�[����
	@param n [in]         �R�s�[�Ώۂ̃R�����g�ԍ�
	@param pszFrom [in]   �R�����g�J�n������
	@param pszTo [in]     �R�����g�I��������
*/
void CBlockComment::SetBlockCommentRule( const int n, const wchar_t* pszFrom, const wchar_t* pszTo )
{
	int nStrLen = wcslen( pszFrom );
	if( 0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE ){
		wcscpy( m_szBlockCommentFrom[n], pszFrom );
		m_nBlockFromLen[ n ] = nStrLen;
	}
	else {
		m_szBlockCommentFrom[n][0] = L'\0';
		m_nBlockFromLen[n] = 0;
	}
	nStrLen = wcslen( pszTo );
	if( 0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE ){
		wcscpy( m_szBlockCommentTo[n], pszTo );
		m_nBlockToLen[ n ] = nStrLen;
	}
	else {
		m_szBlockCommentTo[n][0] = L'\0';
		m_nBlockToLen[n] = 0;
	}
}

/*!
	n�Ԗڂ̃u���b�N�R�����g�́AnPos����̕����񂪊J�n������(From)�ɓ��Ă͂܂邩�m�F����B
	@param n        [in] �����Ώۂ̃R�����g�ԍ�
	@param nPos     [in] �T���J�n�ʒu
	@param nLineLen [in] pLine�̒���
	@param pLine    [in] �T���s�̐擪�D�T���J�n�ʒu�̃|�C���^�ł͂Ȃ����Ƃɒ���

	@retval true  ��v����
	@retval false ��v���Ȃ�����
*/
bool CBlockComment::Match_CommentFrom(
	int n,
	int nPos,
	int nLineLen,
	const wchar_t* pLine
) const
{
	if (
		L'\0' != m_szBlockCommentFrom[n][0] &&
		L'\0' != m_szBlockCommentTo[n][0]  &&
		nPos <= nLineLen - m_nBlockFromLen[n] &&	/* �u���b�N�R�����g�f���~�^(From) */
		0 == auto_memicmp( &pLine[nPos], m_szBlockCommentFrom[n], m_nBlockFromLen[n] )
	){
		return true;
	}
	return false;
}

/*!
	n�Ԗڂ̃u���b�N�R�����g�́A���(To)�ɓ��Ă͂܂镶�����nPos�ȍ~����T��


	@param n        [in] �����Ώۂ̃R�����g�ԍ�
	@param nPos     [in] �T���J�n�ʒu
	@param nLineLen [in] pLine�̒���
	@param pLine    [in] �T���s�̐擪�D�T���J�n�ʒu�̃|�C���^�ł͂Ȃ����Ƃɒ���

	@return ���Ă͂܂����ʒu��Ԃ����A���Ă͂܂�Ȃ������Ƃ��́AnLineLen�����̂܂ܕԂ��B
*/
int CBlockComment::Match_CommentTo( int n, int nPos, int nLineLen, const wchar_t* pLine ) const
{
	int i;
	for( i = nPos; i <= nLineLen - m_nBlockToLen[n]; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		int nCharChars_2 = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
		if( 0 == nCharChars_2 ){
			nCharChars_2 = 1;
		}
		if( 0 == auto_memicmp( &pLine[i], m_szBlockCommentTo[n], m_nBlockToLen[n] ) ){
			return i + m_nBlockToLen[n];
		}
		if( 2 == nCharChars_2 ){
			++i;
		}
	}
	return nLineLen;
}

