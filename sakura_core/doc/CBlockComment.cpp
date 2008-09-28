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
	m_szBlockCommentFrom[ 0 ] = '\0';
	m_szBlockCommentTo[ 0 ] = '\0';
	m_nBlockFromLen = 0;
	m_nBlockToLen = 0;
}

/*!
	�u���b�N�R�����g�f���~�^���R�s�[����
*/
void CBlockComment::SetBlockCommentRule(
	const wchar_t*	pszFrom,	//!< [in] �R�����g�J�n������
	const wchar_t*	pszTo		//!< [in] �R�����g�I��������
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
	n�Ԗڂ̃u���b�N�R�����g�́AnPos����̕����񂪊J�n������(From)�ɓ��Ă͂܂邩�m�F����B

	@retval true  ��v����
	@retval false ��v���Ȃ�����
*/
bool CBlockComment::Match_CommentFrom(
	int					nPos,		//!< [in] �T���J�n�ʒu
	const CStringRef&	cStr		//!< [in] �T���Ώە����� ���T���J�n�ʒu�̃|�C���^�ł͂Ȃ����Ƃɒ���
	/*
	int				nLineLen,	//!< [in] pLine�̒���
	const wchar_t*	pLine		//!< [in] �T���s�̐擪�D
	*/
) const
{
	if (
		L'\0' != m_szBlockCommentFrom[0] &&
		L'\0' != m_szBlockCommentTo[0]  &&
		nPos <= cStr.GetLength() - m_nBlockFromLen &&	/* �u���b�N�R�����g�f���~�^(From) */
		0 == auto_memicmp( &cStr.GetPtr()[nPos], m_szBlockCommentFrom, m_nBlockFromLen )
	){
		return true;
	}
	return false;
}

/*!
	n�Ԗڂ̃u���b�N�R�����g�́A���(To)�ɓ��Ă͂܂镶�����nPos�ȍ~����T��

	@return ���Ă͂܂����ʒu��Ԃ����A���Ă͂܂�Ȃ������Ƃ��́AnLineLen�����̂܂ܕԂ��B
*/
int CBlockComment::Match_CommentTo(
	int					nPos,		//!< [in] �T���J�n�ʒu
	const CStringRef&	cStr		//!< [in] �T���Ώە����� ���T���J�n�ʒu�̃|�C���^�ł͂Ȃ����Ƃɒ���
	/*
	int				nLineLen,	//!< [in] pLine�̒���
	const wchar_t*	pLine		//!< [in] �T���s�̐擪�D�T���J�n�ʒu�̃|�C���^�ł͂Ȃ����Ƃɒ���
	*/
) const
{
	for( int i = nPos; i <= cStr.GetLength() - m_nBlockToLen; ++i ){
		if( 0 == auto_memicmp( cStr.GetPtr() + i, m_szBlockCommentTo, m_nBlockToLen ) ){
			return i + m_nBlockToLen;
		}
	}
	return cStr.GetLength();
}

