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

#include "StdAfx.h"
#include <stdlib.h>
#include "outline/CFuncInfoArr.h"
#include "outline/CFuncInfo.h"


/* CFuncInfoArr�N���X�\�z */
CFuncInfoArr::CFuncInfoArr()
{
	m_nFuncInfoArrNum = 0;	/* �z��v�f�� */
	m_ppcFuncInfoArr = NULL;	/* �z�� */
	m_nAppendTextLenMax = 0;
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
	m_AppendTextArr.clear();
	m_nAppendTextLenMax = 0;
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
	CLogicInt		nFuncColCRLF,		//!< �֐��̂��錅(CRLF�P��)
	CLayoutInt		nFuncLineLAYOUT,	//!< �֐��̂���s(�܂�Ԃ��P��)
	CLayoutInt		nFuncColLAYOUT,		//!< �֐��̂��錅(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		//!< �֐���
	int				nInfo,				//!< �t�����
	int				nDepth				//!< �[��
)
{
	CFuncInfo* pcFuncInfo = new CFuncInfo( nFuncLineCRLF, nFuncColCRLF, nFuncLineLAYOUT, nFuncColLAYOUT, pszFuncName, nInfo );
	pcFuncInfo->m_nDepth = nDepth;
	AppendData( pcFuncInfo );
	return;
}

void CFuncInfoArr::AppendData(
	CLogicInt			nFuncLineCRLF,		//!< �֐��̂���s(CRLF�P��)
	CLogicInt			nFuncColCRLF,		//!< �֐��̂��錅(CRLF�P��)
	CLayoutInt			nFuncLineLAYOUT,	//!< �֐��̂���s(�܂�Ԃ��P��)
	CLayoutInt			nFuncColLAYOUT,		//!< �֐��̂��錅(�܂�Ԃ��P��)
	const NOT_TCHAR*	pszFuncName,		//!< �֐���
	int					nInfo,				//!< �t�����
	int					nDepth				//!< �[��
)
{
	AppendData(nFuncLineCRLF,nFuncColCRLF,nFuncLineLAYOUT,nFuncColLAYOUT,to_tchar(pszFuncName),nInfo,nDepth);
}

void CFuncInfoArr::AppendData(
	CLogicInt		nFuncLineCRLF,		//!< �֐��̂���s(CRLF�P��)
	CLayoutInt		nFuncLineLAYOUT,	//!< �֐��̂���s(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		//!< �֐���
	int				nInfo,				//!< �t�����
	int				nDepth				//!< �[��
)
{
	AppendData(nFuncLineCRLF,CLogicInt(1),nFuncLineLAYOUT,CLayoutInt(1),pszFuncName,nInfo,nDepth);
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
	MYTRACE( _T("=============================\n") );
	for( i = 0; i < m_nFuncInfoArrNum; i++ ){
		MYTRACE( _T("[%d]------------------\n"), i );
		MYTRACE( _T("m_nFuncLineCRLF	=%d\n"), m_ppcFuncInfoArr[i]->m_nFuncLineCRLF );
		MYTRACE( _T("m_nFuncLineLAYOUT	=%d\n"), m_ppcFuncInfoArr[i]->m_nFuncLineLAYOUT );
		MYTRACE( _T("m_cmemFuncName	=[%ts]\n"), m_ppcFuncInfoArr[i]->m_cmemFuncName.GetStringPtr() );
		MYTRACE( _T("m_nInfo			=%d\n"), m_ppcFuncInfoArr[i]->m_nInfo );
	}
	MYTRACE( _T("=============================\n") );
#endif
}

void CFuncInfoArr::SetAppendText( int info, std::wstring s, bool overwrite )
{
	if( m_AppendTextArr.find( info ) == m_AppendTextArr.end() ){
		// �L�[�����݂��Ȃ��ꍇ�A�ǉ�����
		std::pair<int, std::wstring> pair(info, s);
		m_AppendTextArr.insert( pair );
		if( m_nAppendTextLenMax < (int)s.length() ){
			m_nAppendTextLenMax = s.length();
		}
#ifndef	_UNICODE
		std::tstring t = to_tchar(s.c_str());
		if( m_nAppendTextLenMax < (int)t.length() ){
			m_nAppendTextLenMax = t.length();
		}
#endif
	}else{
		// �L�[�����݂���ꍇ�A�l������������
		if( overwrite ){
			m_AppendTextArr[ info ] = s;
		}
	}
}

std::wstring CFuncInfoArr::GetAppendText( int info )
{
	if( m_AppendTextArr.find( info ) == m_AppendTextArr.end() ){
		// �L�[�����݂��Ȃ��ꍇ�A�󕶎����Ԃ�
		return std::wstring();
	}else{
		// �L�[�����݂���ꍇ�A�l��Ԃ�
		return m_AppendTextArr[ info ];
	}
}

