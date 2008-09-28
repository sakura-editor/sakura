/*!	@file
	@brief �A�E�g���C����� �f�[�^�z��

	@author Norio Nakatani
	@date	1998/06/23 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <malloc.h>
#include "outline/CFuncInfoArr.h"
#include "debug/Debug.h"// 2002/2/10 aroka


/* CFuncInfoArr�N���X�\�z */
CFuncInfoArr::CFuncInfoArr()
{
	m_nFuncInfoArrNum = 0;	/* �z��v�f�� */
	m_ppcFuncInfoArr = NULL;	/* �z�� */
	return;
}




/* CFuncInfoArr�N���X���� */
CFuncInfoArr::~CFuncInfoArr()
{
	Empty();
	return;
}

void CFuncInfoArr::Empty( void )
{
	int i;
	if( m_nFuncInfoArrNum > 0 && NULL != m_ppcFuncInfoArr ){
		for( i = 0; i < m_nFuncInfoArrNum; ++i ){
			delete m_ppcFuncInfoArr[i];
			m_ppcFuncInfoArr[i] = NULL;
		}
		m_nFuncInfoArrNum = 0;
		free( m_ppcFuncInfoArr );
		m_ppcFuncInfoArr = NULL;
	}
	return;
}


/* 0<=�̎w��ԍ��̃f�[�^��Ԃ� */
/* �f�[�^���Ȃ��ꍇ��NULL��Ԃ� */
CFuncInfo* CFuncInfoArr::GetAt( int nIdx )
{
	if( nIdx >= m_nFuncInfoArrNum ){
		return NULL;
	}
	return m_ppcFuncInfoArr[nIdx];
}



/*! �z��̍Ō�Ƀf�[�^��ǉ����� */
void CFuncInfoArr::AppendData( CFuncInfo* pcFuncInfo )
{
	if( 0 == m_nFuncInfoArrNum){
		m_ppcFuncInfoArr = (CFuncInfo**)malloc( sizeof(CFuncInfo*) * (m_nFuncInfoArrNum + 1) );
	}else{
		m_ppcFuncInfoArr = (CFuncInfo**)realloc( m_ppcFuncInfoArr, sizeof(CFuncInfo*) * (m_nFuncInfoArrNum + 1) );
	}
	m_ppcFuncInfoArr[m_nFuncInfoArrNum] = pcFuncInfo;
	m_nFuncInfoArrNum++;
	return;
}



/*! �z��̍Ō�Ƀf�[�^��ǉ�����

	@date 2002.04.01 YAZAKI �[������
*/
void CFuncInfoArr::AppendData(
	CLogicInt		nFuncLineCRLF,		//!< �֐��̂���s(CRLF�P��)
	CLayoutInt		nFuncLineLAYOUT,	//!< �֐��̂���s(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		//!< �֐���
	int				nInfo,				//!< �t�����
	int				nDepth				//!< �[��
)
{
	CFuncInfo* pcFuncInfo = new CFuncInfo( nFuncLineCRLF, nFuncLineLAYOUT, pszFuncName, nInfo );
	pcFuncInfo->m_nDepth = nDepth;
	AppendData( pcFuncInfo );
	return;
}

void CFuncInfoArr::AppendData(
	CLogicInt			nFuncLineCRLF,		//!< �֐��̂���s(CRLF�P��)
	CLayoutInt			nFuncLineLAYOUT,	//!< �֐��̂���s(�܂�Ԃ��P��)
	const NOT_TCHAR*	pszFuncName,		//!< �֐���
	int					nInfo,				//!< �t�����
	int					nDepth				//!< �[��
)
{
	AppendData(nFuncLineCRLF,nFuncLineLAYOUT,to_tchar(pszFuncName),nInfo,nDepth);
}


void CFuncInfoArr::DUMP( void )
{
#ifdef _DEBUG
	int i;
	MYTRACE_A( "=============================\n" );
	for( i = 0; i < m_nFuncInfoArrNum; i++ ){
		MYTRACE_A( "[%d]------------------\n", i );
		MYTRACE_A( "m_nFuncLineCRLF	=%d\n", m_ppcFuncInfoArr[i]->m_nFuncLineCRLF );
		MYTRACE_A( "m_nFuncLineLAYOUT	=%d\n", m_ppcFuncInfoArr[i]->m_nFuncLineLAYOUT );
		MYTRACE_A( "m_cmemFuncName	=[%ls]\n", m_ppcFuncInfoArr[i]->m_cmemFuncName.GetStringPtr() );
		MYTRACE_A( "m_nInfo			=%d\n", m_ppcFuncInfoArr[i]->m_nInfo );
	}
	MYTRACE_A( "=============================\n" );
#endif
}



