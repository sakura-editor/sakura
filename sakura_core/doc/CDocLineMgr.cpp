/*!	@file
	@brief �s�f�[�^�̊Ǘ�

	@author Norio Nakatani
	@date 1998/03/05  �V�K�쐬
	@date 2001/06/23 N.Nakatani �P��P�ʂŌ�������@�\������
	@date 2001/06/23 N.Nakatani WhereCurrentWord()�ύX WhereCurrentWord_2���R�[������悤�ɂ���
	@date 2005/09/25 D.S.Koba GetSizeOfChar�ŏ�������
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, ao
	Copyright (C) 2001, genta, jepro, hor
	Copyright (C) 2002, hor, aroka, MIK, Moca, genta, frozen, Azumaiya, YAZAKI
	Copyright (C) 2003, Moca, ryoji, genta, �����
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, D.S.Koba, ryoji, �����

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
// Oct 6, 2000 ao
#include <stdio.h>
#include <io.h>
#include <list>
#include "doc/CDocLineMgr.h"
#include "doc/CDocLine.h"// 2002/2/10 aroka �w�b�_����
#include "charset/charcode.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"
#include "charset/CCodeMediator.h"
//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "extmodule/CBregexp.h"
#include "_main/global.h"

//	May 15, 2000 genta
#include "CEol.h"
#include "mem/CMemory.h"// 2002/2/10 aroka

#include "io/CFileLoad.h" // 2002/08/30 Moca
#include "io/CIoBridge.h"
#include "basis/SakuraBasis.h"
#include "parse/CWordParse.h"
#include "util/window.h"
#include "util/file.h"
#include "debug/CRunningTimer.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CDocLineMgr::CDocLineMgr()
{
	_Init();
}

CDocLineMgr::~CDocLineMgr()
{
	DeleteAllLine();
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �s�f�[�^�̊Ǘ�                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


//! pPos�̒��O�ɐV�����s��}��
CDocLine* CDocLineMgr::InsertNewLine(CDocLine* pPos)
{
	CDocLine* pcDocLineNew = new CDocLine;
	_InsertBeforePos(pcDocLineNew,pPos);
	return pcDocLineNew;
}

//! �ŉ����ɐV�����s��}��
CDocLine* CDocLineMgr::AddNewLine()
{
	CDocLine* pcDocLineNew = new CDocLine;
	_PushBottom(pcDocLineNew);
	return pcDocLineNew;
}

//! �S�Ă̍s���폜����
void CDocLineMgr::DeleteAllLine()
{
	CDocLine* pDocLine = m_pDocLineTop;
	while( pDocLine ){
		CDocLine* pDocLineNext = pDocLine->GetNextLine();
		delete pDocLine;
		pDocLine = pDocLineNext;
	}
	_Init();
}


//! �s�̍폜
void CDocLineMgr::DeleteLine( CDocLine* pcDocLineDel )
{
	//Prev�؂藣��
	if( pcDocLineDel->GetPrevLine() ){
		pcDocLineDel->GetPrevLine()->m_pNext = pcDocLineDel->GetNextLine();
	}
	else{
		m_pDocLineTop = pcDocLineDel->GetNextLine();
	}

	//Next�؂藣��
	if( pcDocLineDel->GetNextLine() ){
		pcDocLineDel->m_pNext->m_pPrev = pcDocLineDel->GetPrevLine();
	}
	else{
		m_pDocLineBot = pcDocLineDel->GetPrevLine();
	}
	
	//�Q�Ɛ؂藣��
	if( m_pCodePrevRefer == pcDocLineDel ){
		m_pCodePrevRefer = pcDocLineDel->GetNextLine();
	}

	//�f�[�^�폜
	delete pcDocLineDel;

	//�s�����Z
	m_nLines--;
	if( CLogicInt(0) == m_nLines ){
		// �f�[�^���Ȃ��Ȃ���
		_Init();
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   �s�f�[�^�ւ̃A�N�Z�X                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�w�肳�ꂽ�ԍ��̍s�ւ̃|�C���^��Ԃ�

	@param nLine [in] �s�ԍ�
	@return �s�I�u�W�F�N�g�ւ̃|�C���^�B�Y���s���Ȃ��ꍇ��NULL�B
*/
CDocLine* CDocLineMgr::GetLine( CLogicInt nLine ) const
{
	CLogicInt nCounter;
	CDocLine* pDocLine;
	if( CLogicInt(0) == m_nLines ){
		return NULL;
	}
	// 2004.03.28 Moca nLine�����̏ꍇ�̃`�F�b�N��ǉ�
	if( CLogicInt(0) > nLine || nLine >= m_nLines ){
		return NULL;
	}
	// 2004.03.28 Moca m_pCodePrevRefer���ATop,Bot�̂ق����߂��ꍇ�́A������𗘗p����
	CLogicInt nPrevToLineNumDiff = t_abs( m_nPrevReferLine - nLine );
	if( m_pCodePrevRefer == NULL
	  || nLine < nPrevToLineNumDiff
	  || m_nLines - nLine < nPrevToLineNumDiff
	){
		if( m_pCodePrevRefer == NULL ){
			MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::GetLine() 	m_pCodePrevRefer == NULL" );
		}

		if( nLine < (m_nLines / 2) ){
			nCounter = CLogicInt(0);
			pDocLine = m_pDocLineTop;
			while( pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->GetNextLine();
					return pDocLine;
				}
				pDocLine = pDocLine->GetNextLine();
				nCounter++;
			}
		}
		else{
			nCounter = m_nLines - CLogicInt(1);
			pDocLine = m_pDocLineBot;
			while( NULL != pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->GetNextLine();
					return pDocLine;
				}
				pDocLine = pDocLine->GetPrevLine();
				nCounter--;
			}
		}

	}
	else{
		if( nLine == m_nPrevReferLine ){
			m_nPrevReferLine = nLine;
			m_pDocLineCurrent = m_pCodePrevRefer->GetNextLine();
			return m_pCodePrevRefer;
		}
		else if( nLine > m_nPrevReferLine ){
			nCounter = m_nPrevReferLine + CLogicInt(1);
			pDocLine = m_pCodePrevRefer->GetNextLine();
			while( NULL != pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->GetNextLine();
					return pDocLine;
				}
				pDocLine = pDocLine->GetNextLine();
				++nCounter;
			}
		}
		else{
			nCounter = m_nPrevReferLine - CLogicInt(1);
			pDocLine = m_pCodePrevRefer->GetPrevLine();
			while( NULL != pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->GetNextLine();
					return pDocLine;
				}
				pDocLine = pDocLine->GetPrevLine();
				nCounter--;
			}
		}
	}
	return NULL;
}








// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CDocLineMgr::_Init()
{
	m_pDocLineTop = NULL;
	m_pDocLineBot = NULL;
	m_nLines = CLogicInt(0);
	m_nPrevReferLine = CLogicInt(0);
	m_pCodePrevRefer = NULL;
	CDiffManager::getInstance()->SetDiffUse(false);	/* DIFF�g�p�� */	//@@@ 2002.05.25 MIK     //##���CDocListener::OnClear (OnAfterClose) ���쐬���A�����Ɉړ�
}

// -- -- �`�F�[���֐� -- -- // 2007.10.11 kobake �쐬
//!�ŉ����ɑ}��
void CDocLineMgr::_PushBottom(CDocLine* pDocLineNew)
{
	if( !m_pDocLineTop ){
		m_pDocLineTop = pDocLineNew;
	}
	pDocLineNew->m_pPrev = m_pDocLineBot;

	if( m_pDocLineBot ){
		m_pDocLineBot->m_pNext = pDocLineNew;
	}
	m_pDocLineBot = pDocLineNew;
	pDocLineNew->m_pNext = NULL;

	++m_nLines;
}

//!pPos�̒��O�ɑ}���BpPos��NULL���w�肵���ꍇ�́A�ŉ����ɒǉ��B
void CDocLineMgr::_InsertBeforePos(CDocLine* pDocLineNew, CDocLine* pPos)
{
	//New.Next��ݒ�
	pDocLineNew->m_pNext = pPos;

	//New.Prev, Other.Prev��ݒ�
	if(pPos){
		pDocLineNew->m_pPrev = pPos->GetPrevLine();
		pPos->m_pPrev = pDocLineNew;
	}
	else{
		pDocLineNew->m_pPrev = m_pDocLineBot;
		m_pDocLineBot = pDocLineNew;
	}

	//Other.Next��ݒ�
	if( pDocLineNew->GetPrevLine() ){
		pDocLineNew->GetPrevLine()->m_pNext = pDocLineNew;
	}
	else{
		m_pDocLineTop = pDocLineNew;
	}

	//�s�������Z
	++m_nLines;
}

//! pPos�̒���ɑ}���BpPos��NULL���w�肵���ꍇ�́A�擪�ɒǉ��B
void CDocLineMgr::_InsertAfterPos(CDocLine* pDocLineNew, CDocLine* pPos)
{
	//New.Prev��ݒ�
	pDocLineNew->m_pPrev = pPos;

	//New.Next, Other.Next��ݒ�
	if( pPos ){
		pDocLineNew->m_pNext = pPos->GetNextLine();
		pPos->m_pNext = pDocLineNew;
	}
	else{
		pDocLineNew->m_pNext = m_pDocLineTop;
		m_pDocLineTop = pDocLineNew;
	}

	//Other.Prev��ݒ�
	if( pDocLineNew->GetNextLine() ){
		pDocLineNew->m_pNext->m_pPrev = pDocLineNew;
	}
	else{
		m_pDocLineBot = pDocLineNew;
	}

	//�s�������Z
	m_nLines++;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �f�o�b�O                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief CDocLineMgrDEBUG�p

	@date 2004.03.18 Moca
		m_pDocLineCurrent��m_pCodePrevRefer���f�[�^�`�F�[����
		�v�f���w���Ă��邩�̌��؋@�\��ǉ��D

*/
void CDocLineMgr::DUMP()
{
#ifdef _DEBUG
	MYTRACE( _T("------------------------\n") );

	CDocLine* pDocLine;
	CDocLine* pDocLineNext;
	CDocLine* pDocLineEnd = NULL;
	pDocLine = m_pDocLineTop;

	// �������𒲂ׂ�
	bool bIncludeCurrent = false;
	bool bIncludePrevRefer = false;
	CLogicInt nNum = CLogicInt(0);
	if( m_pDocLineTop->m_pPrev != NULL ){
		MYTRACE( _T("error: m_pDocLineTop->m_pPrev != NULL\n"));
	}
	if( m_pDocLineBot->m_pNext != NULL ){
		MYTRACE( _T("error: m_pDocLineBot->m_pNext != NULL\n") );
	}
	while( NULL != pDocLine ){
		if( m_pDocLineCurrent == pDocLine ){
			bIncludeCurrent = true;
		}
		if( m_pCodePrevRefer == pDocLine ){
			bIncludePrevRefer = true;
		}
		if( NULL != pDocLine->GetNextLine() ){
			if( pDocLine->m_pNext == pDocLine ){
				MYTRACE( _T("error: pDocLine->m_pPrev Invalid value.\n") );
				break;
			}
			if( pDocLine->m_pNext->m_pPrev != pDocLine ){
				MYTRACE( _T("error: pDocLine->m_pNext->m_pPrev != pDocLine.\n") );
				break;
			}
		}else{
			pDocLineEnd = pDocLine;
		}
		pDocLine = pDocLine->GetNextLine();
		nNum++;
	}
	
	if( pDocLineEnd != m_pDocLineBot ){
		MYTRACE( _T("error: pDocLineEnd != m_pDocLineBot") );
	}
	
	if( nNum != m_nLines ){
		MYTRACE( _T("error: nNum(%d) != m_nLines(%d)\n"), nNum, m_nLines );
	}
	if( false == bIncludeCurrent && m_pDocLineCurrent != NULL ){
		MYTRACE( _T("error: m_pDocLineCurrent=%08lxh Invalid value.\n"), m_pDocLineCurrent );
	}
	if( false == bIncludePrevRefer && m_pCodePrevRefer != NULL ){
		MYTRACE( _T("error: m_pCodePrevRefer =%08lxh Invalid value.\n"), m_pCodePrevRefer );
	}

	// DUMP
	MYTRACE( _T("m_nLines=%d\n"), m_nLines );
	MYTRACE( _T("m_pDocLineTop=%08lxh\n"), m_pDocLineTop );
	MYTRACE( _T("m_pDocLineBot=%08lxh\n"), m_pDocLineBot );
	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine ){
		pDocLineNext = pDocLine->GetNextLine();
		MYTRACE( _T("\t-------\n") );
		MYTRACE( _T("\tthis=%08lxh\n"), pDocLine );
		MYTRACE( _T("\tpPrev; =%08lxh\n"), pDocLine->GetPrevLine() );
		MYTRACE( _T("\tpNext; =%08lxh\n"), pDocLine->GetNextLine() );

		MYTRACE( _T("\tm_enumEOLType =%ls\n"), pDocLine->GetEol().GetName() );
		MYTRACE( _T("\tm_nEOLLen =%d\n"), pDocLine->GetEol().GetLen() );


//		MYTRACE( _T("\t[%ls]\n"), *(pDocLine->m_pLine) );
		MYTRACE( _T("\tpDocLine->m_cLine.GetLength()=[%d]\n"), pDocLine->GetLengthWithEOL() );
		MYTRACE( _T("\t[%ls]\n"), pDocLine->GetPtr() );


		pDocLine = pDocLineNext;
	}
	MYTRACE( _T("------------------------\n") );
#endif // _DEBUG
	return;
}

