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

/* for TRACE() of MFC */
//#ifdef _DEBUG
//	#include <afx.h>
//#endif
//#ifndef _DEBUG
//	#include <windows.h>
//#endif


#include "stdafx.h"
#include "charcode.h"
#include "CDocLineMgr.h"
#include "debug.h"
#include "charcode.h"
// Oct 6, 2000 ao
#include <stdio.h>
#include <io.h>
//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "CBregexp.h"
#include <commctrl.h>
#include "global.h"
#include "CRunningTimer.h"

//	May 15, 2000 genta
#include "CEol.h"
#include "CDocLine.h"// 2002/2/10 aroka �w�b�_����
#include "CMemory.h"// 2002/2/10 aroka

#include "CFileWrite.h" //2002/05/22 Frozen
#include "CFileLoad.h" // 2002/08/30 Moca
#include "my_icmp.h" // Nov. 29, 2002 genta/moca
#include "CIoBridge.h"
#include "basis/SakuraBasis.h"
#include "parse/CWordParse.h"
#include "util/window.h"
#include "util/file.h"

CDocLineMgr::CDocLineMgr()
{
	Init();
}




CDocLineMgr::~CDocLineMgr()
{
	Empty();
}





void CDocLineMgr::Init()
{
	m_pDocLineTop = NULL;
	m_pDocLineBot = NULL;
	m_nLines = CLogicInt(0);
	m_nPrevReferLine = CLogicInt(0);
	m_pCodePrevRefer = NULL;
	m_bIsDiffUse = false;	/* DIFF�g�p�� */	//@@@ 2002.05.25 MIK
	return;
}

/*!
	�f�[�^�̃N���A

	�S�Ă̍s���폜����
*/
void CDocLineMgr::Empty()
{
	CDocLine* pDocLine;
	CDocLine* pDocLineNext;
	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine ){
		pDocLineNext = pDocLine->m_pNext;
		delete pDocLine;
		pDocLine = pDocLineNext;
	}
	return;
}





const wchar_t* CDocLineMgr::GetLineStr( CLogicInt nLine, CLogicInt* pnLineLen )
{
	CDocLine* pDocLine;
	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		*pnLineLen = CLogicInt(0);
		return NULL;
	}
	// 2002/2/10 aroka CMemory �̃����o�ϐ��ɒ��ڃA�N�Z�X���Ȃ�(inline������Ă���̂ő��x�I�Ȗ��͂Ȃ�)
	return pDocLine->m_cLine.GetStringPtr( pnLineLen );
}

/*!
	�w�肳�ꂽ�s�ԍ��̕�����Ɖ��s�R�[�h�������������擾
	
	@author Moca
	@date 2003.06.22
*/
const wchar_t* CDocLineMgr::GetLineStrWithoutEOL( CLogicInt nLine, int* pnLineLen )
{
	const CDocLine* pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		*pnLineLen = 0;
		return NULL;
	}
	*pnLineLen = pDocLine->GetLengthWithoutEOL();
	return pDocLine->m_cLine.GetStringPtr();
}

/*!
	�w�肳�ꂽ�ԍ��̍s�ւ̃|�C���^��Ԃ�

	@param nLine [in] �s�ԍ�
	@return �s�I�u�W�F�N�g�ւ̃|�C���^�B�Y���s���Ȃ��ꍇ��NULL�B
*/
CDocLine* CDocLineMgr::GetLineInfo( CLogicInt nLine )
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
			MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::GetLineInfo() 	m_pCodePrevRefer == NULL" );
		}

		if( nLine < (m_nLines / 2) ){
			nCounter = CLogicInt(0);
			pDocLine = m_pDocLineTop;
			while( pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->m_pNext;
					return pDocLine;
				}
				pDocLine = pDocLine->m_pNext;
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
					m_pDocLineCurrent = pDocLine->m_pNext;
					return pDocLine;
				}
				pDocLine = pDocLine->m_pPrev;
				nCounter--;
			}
		}

	}
	else{
		if( nLine == m_nPrevReferLine ){
			m_nPrevReferLine = nLine;
			m_pDocLineCurrent = m_pCodePrevRefer->m_pNext;
			return m_pCodePrevRefer;
		}
		else if( nLine > m_nPrevReferLine ){
			nCounter = m_nPrevReferLine + CLogicInt(1);
			pDocLine = m_pCodePrevRefer->m_pNext;
			while( NULL != pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->m_pNext;
					return pDocLine;
				}
				pDocLine = pDocLine->m_pNext;
				++nCounter;
			}
		}
		else{
			nCounter = m_nPrevReferLine - CLogicInt(1);
			pDocLine = m_pCodePrevRefer->m_pPrev;
			while( NULL != pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->m_pNext;
					return pDocLine;
				}
				pDocLine = pDocLine->m_pPrev;
				nCounter--;
			}
		}
	}
	return NULL;
}





/*! ���A�N�Z�X���[�h�F�擪�s�𓾂�

	@param pnLineLen [out] �s�̒������Ԃ�B
	@return 1�s�ڂ̐擪�ւ̃|�C���^�B
	�f�[�^��1�s���Ȃ��Ƃ��́A����0�A�|�C���^NULL���Ԃ�B

*/
const wchar_t* CDocLineMgr::GetFirstLinrStr( int* pnLineLen )
{
	const wchar_t* pszLine;
	if( CLogicInt(0) == m_nLines ){
		pszLine = NULL;
		*pnLineLen = 0;
	}else{
		pszLine = m_pDocLineTop->m_cLine.GetStringPtr( pnLineLen );

		m_pDocLineCurrent = m_pDocLineTop->m_pNext;
	}
	return pszLine;
}





/*!
	���A�N�Z�X���[�h�F���̍s�𓾂�

	@param pnLineLen [out] �s�̒������Ԃ�B
	@return ���s�̐擪�ւ̃|�C���^�B
	GetFirstLinrStr()���Ăяo����Ă��Ȃ���NULL���Ԃ�

*/
const wchar_t* CDocLineMgr::GetNextLinrStr( int* pnLineLen )
{
	const wchar_t* pszLine;
	if( NULL == m_pDocLineCurrent ){
		pszLine = NULL;
		*pnLineLen = 0;
	}else{
		pszLine = m_pDocLineCurrent->m_cLine.GetStringPtr( pnLineLen );

		m_pDocLineCurrent = m_pDocLineCurrent->m_pNext;
	}
	return pszLine;
}



/*!
	�����ɍs��ǉ�

	@version 1.5

	@param pData    [in] �ǉ����镶����ւ̃|�C���^
	@param nDataLen [in] ������̒����B�����P�ʁB
	@param cEol     [in] �s���R�[�h

*/


void CDocLineMgr::AddLineStrX( const wchar_t* pData, int nDataLen, CEOL cEol )
{
#ifdef _DEBUG
//	CRunningTimer cRunningTimer( "CDocLineMgr::AddLineStrX(const char*, int, int)" );
#endif

	//�`�F�[���K�p
	CDocLine* pDocLine = new CDocLine;
	_PushBottom(pDocLine);

	//�C���X�^���X�ݒ�
	pDocLine->m_cEol = cEol;	/* ���s�R�[�h�̎�� */
	pDocLine->m_cLine.SetString(pData, nDataLen);
}

/*!
	�t�@�C����ǂݍ���Ŋi�[����i�����ǂݍ��݃e�X�g�Łj
	@version	2.0
	@note	Windows�p�ɃR�[�f�B���O���Ă���
	@retval	TRUE	����ǂݍ���
	@retval	FALSE	�G���[(�܂��̓��[�U�ɂ��L�����Z��?)
	@date	2002/08/30 Moca ��ReadFile�����ɍ쐬 �t�@�C���A�N�Z�X�Ɋւ��镔����CFileLoad�ōs��
	@date	2003/07/26 ryoji BOM�̏�Ԃ̎擾��ǉ�
*/
int CDocLineMgr::ReadFile(
	const TCHAR*		pszPath,		//!<
	HWND				hWndParent,		//!< [in] �e�E�B���h�E�̃n���h��
	HWND				hwndProgress,	//!< [in] Progress bar�̃E�B���h�E�n���h��
	ECodeType			nCharCode,		//!<
	FILETIME*			pFileTime,		//!<
	int					nFlags,			//!< [in] bit 0: MIME Encode���ꂽ�w�b�_��decode���邩�ǂ���
	BOOL*				pbBomExist		//!<
)
{
#ifdef _DEBUG
	MYTRACE_A( "pszPath=[%hs]\n", pszPath );
	MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::ReadFile" );
#endif
	/* �����f�[�^�̃N���A */
	Empty();
	Init();

	/* �������̃��[�U�[������\�ɂ��� */
	if( !::BlockingHook( NULL ) ){
		return FALSE;
	}

	int nRetVal = TRUE;

	try{
		CFileLoad cfl;

		// �t�@�C�����J��
		// �t�@�C�������ɂ�FileClose�����o���̓f�X�g���N�^�̂ǂ��炩�ŏ����ł��܂�
		//	Jul. 28, 2003 ryoji BOM�p�����[�^�ǉ�
		cfl.FileOpen( pszPath, nCharCode, nFlags, pbBomExist );

		/* �t�@�C�������̎擾 */
		FILETIME	FileTime;
		if( cfl.GetFileTime( NULL, NULL, &FileTime ) ){
			*pFileTime = FileTime;
		}

		//�v���O���X�o�[������
		if( NULL != hwndProgress ){
			::PostMessageAny( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
			::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
		}

		// ReadLine�̓t�@�C������ �����R�[�h�ϊ����ꂽ1�s��ǂݏo���܂�
		// �G���[����throw CError_FileRead �𓊂��܂�
		const wchar_t*	pLine;
		int				nLineLen;
		int				nLineNum = 0;
		CEOL			cEol;
		CNativeW2		cUnicodeBuffer;
		while( NULL != ( pLine = cfl.ReadLine( &cUnicodeBuffer, &nLineLen, &cEol ) ) ){
			++nLineNum;
			AddLineStrX( pLine, nLineLen, cEol );
			//512�s���Ƀv���O���X�o�[���X�V
			if( NULL != hwndProgress && 0 == ( nLineNum % 512 ) ){
				::PostMessageAny( hwndProgress, PBM_SETPOS, cfl.GetPercent(), 0 );
				/* �������̃��[�U�[������\�ɂ��� */
				if( !::BlockingHook( NULL ) ){
					return FALSE;
				}
			}
		}

		// �t�@�C�����N���[�Y����
		cfl.FileClose();
	}
	catch( CError_FileOpen ){
		nRetVal = FALSE;
		if( -1 == _taccess( pszPath, 0 )){
			// �t�@�C�����Ȃ�
			::MYMESSAGEBOX(
				hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("%ls\n�Ƃ����t�@�C�����J���܂���B\n�t�@�C�������݂��܂���B"),	//Mar. 24, 2001 jepro �኱�C��
				pszPath
			 );
		}
		else if( -1 == _taccess( pszPath, 4 )){
			// �ǂݍ��݃A�N�Z�X�����Ȃ�
			::MYMESSAGEBOX(
				hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("\'%ts\'\n�Ƃ����t�@�C�����J���܂���B\n�ǂݍ��݃A�N�Z�X��������܂���B"),
				pszPath
			 );
		}
		else{
			::MYMESSAGEBOX(
				hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("\'%ts\'\n�Ƃ����t�@�C�����J���܂���B\n���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B"),
				pszPath
			 );
		}
	}
	catch( CError_FileRead ){
		nRetVal = FALSE;
		::MYMESSAGEBOX(
			hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			_T("\'%ts\'�Ƃ����t�@�C���̓ǂݍ��ݒ��ɃG���[���������܂����B\n�t�@�C���̓ǂݍ��݂𒆎~���܂��B"),
			pszPath
		 );
		/* �����f�[�^�̃N���A */
		Empty();
		Init();
		nRetVal = FALSE;
	} // ��O�����I���

	if( NULL != hwndProgress ){
		::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
	}
	/* �������̃��[�U�[������\�ɂ��� */
	if( !::BlockingHook( NULL ) ){
		return FALSE;
	}

	/* �s�ύX��Ԃ����ׂă��Z�b�g */
	ResetAllModifyFlag();
	return nRetVal;
}




/*! �o�b�t�@���e���t�@�C���ɏ����o�� (�e�X�g�p)

	@param cEol [in]		�g�p������s�R�[�h
	@param bBomExist [in]	�t�@�C���擪��BOM��t���邩

	@note Windows�p�ɃR�[�f�B���O���Ă���
	@date 2003.07.26 ryoji BOM�����ǉ�
*/
EConvertResult CDocLineMgr::WriteFile(
	const TCHAR*	pszPath,
	HWND			hWndParent,
	HWND			hwndProgress,
	ECodeType		nCharCode,
	FILETIME*		pFileTime,
	CEOL			cEol,
	BOOL			bBomExist
)
{
	int				nLineNumber;
	int				nWriteLen;
	CDocLine*		pCDocLine;


	if( NULL != hwndProgress ){
		::PostMessageAny( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
	}

	EConvertResult		nRetVal = RESULT_COMPLETE;

	try
	{
		CFileWrite file(pszPath);// 2002/05/22 Frozen

		//	Jul. 26, 2003 ryoji bBomExit�ɂ����BOM��t���邩�ǂ��������߂�
		if (bBomExist) {
			switch( nCharCode ){
			case CODE_UNICODE:
				file.WriteSz( "\xff\xfe" );
				break;
			case CODE_UNICODEBE:
				file.WriteSz( "\xfe\xff" );
				break;
			case CODE_UTF8: // 2003.05.04 Moca BOM�̊ԈႢ�����
				file.WriteSz( "\xef\xbb\xbf" );
				break;
			default:
				//	genta �����ɗ���̂̓o�O��
				;
			}
		}

		nLineNumber = 0;
		pCDocLine = m_pDocLineTop;

		//�t�@�C�������p�o�b�t�@(���̒��ŕ����R�[�h�ϊ����s��)

		while( NULL != pCDocLine ){
			++nLineNumber;
			int				nLineLen;
			const wchar_t*	pLine = pCDocLine->m_cLine.GetStringPtr( &nLineLen );

			//1024�s�������ނ��ƂɃv���O���X�o�[���X�V����
			if( NULL != hwndProgress && CLogicInt(0) < m_nLines && 0 == ( nLineNumber % 1024 ) ){
				::PostMessageAny( hwndProgress, PBM_SETPOS, nLineNumber * 100 / m_nLines , 0 );
				/* �������̃��[�U�[������\�ɂ��� */
				if( !::BlockingHook( NULL ) ){
					return RESULT_FAILURE;
				}
			}

			//1�s�o�� -> cFileCodeBuffer
			nWriteLen = nLineLen - pCDocLine->m_cEol.GetLen();
			CMemory cFileCodeBuffer;
			if( 0 < nWriteLen ){
				CNativeW2 cmemBuf( pLine, nWriteLen );

				// �������ݎ��̃R�[�h�ϊ� cmemBuf -> cFileCodeBuffer
				EConvertResult e = CIoBridge::ImplToFile(&cmemBuf,&cFileCodeBuffer,nCharCode);
				if(e==RESULT_LOSESOME){
					if(nRetVal==RESULT_COMPLETE)nRetVal=RESULT_LOSESOME;
				}

				cmemBuf.SetString(L"",0);
			}

			//���s�o�� -> cFileCodeBuffer
			if( EOL_NONE != pCDocLine->m_cEol ){

// 2002/05/09 Frozen ��������
				if( nCharCode == CODE_UNICODE ){
					if( cEol==EOL_NONE )
						cFileCodeBuffer.AppendRawData( pCDocLine->m_cEol.GetUnicodeValue(), (Int)pCDocLine->m_cEol.GetLen()*sizeof(wchar_t));
					else
						cFileCodeBuffer.AppendRawData( cEol.GetUnicodeValue(), (Int)cEol.GetLen()*sizeof(wchar_t));
				}else if( nCharCode == CODE_UNICODEBE ){
					/* UnicodeBE �̉��s�R�[�h�ݒ� Moca, 2002/05/26 */
					if( cEol == EOL_NONE ) /*  */
						cFileCodeBuffer.AppendRawData( pCDocLine->m_cEol.GetUnicodeBEValue(), (Int)pCDocLine->m_cEol.GetLen()*sizeof(wchar_t) );
					else
						cFileCodeBuffer.AppendRawData( cEol.GetUnicodeBEValue(), (Int)cEol.GetLen()*sizeof(wchar_t) );
				}else{
					if( cEol == EOL_NONE )
						cFileCodeBuffer.AppendRawData(pCDocLine->m_cEol.GetValue(), (Int)pCDocLine->m_cEol.GetLen());
					else
						cFileCodeBuffer.AppendRawData(cEol.GetValue(), (Int)cEol.GetLen());
				}
// 2002/05/09 Frozen �����܂�

			}

			//cFileCodeBuffer�����ۂɏo�� (�����ŏ��߂ăt�@�C����������������)
			if( 0 < cFileCodeBuffer.GetRawLength() )//{
				file.Write(cFileCodeBuffer.GetRawPtr(),cFileCodeBuffer.GetRawLength());//2002/05/22 Frozen goto�̎���}�܂ł����̈�s�Œu������

			//���̍s��
			pCDocLine = pCDocLine->m_pNext;

		}

		file.Close();


		/* �X�V��̃t�@�C�������̎擾
		 * CloseHandle�O�ł�FlushFileBuffers���Ă�ł��^�C���X�^���v���X�V
		 * ����Ȃ����Ƃ�����B
		 */

		// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX�i�t�@�C�������b�N����Ă��Ă��^�C���X�^���v�擾�\�j
		FILETIME ftime;
		if( GetLastWriteTimestamp( pszPath, ftime )){
			*pFileTime = ftime;
		}

	}
	catch(CError_FileOpen)
	{
		::MYMESSAGEBOX_A(
			hWndParent,
			MB_OK | MB_ICONSTOP,
			GSTR_APPNAME_A,
			"\'%ts\'\n"
			"�t�@�C����ۑ��ł��܂���B\n"
			"�p�X�����݂��Ȃ����A���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B",
			pszPath
		);
		nRetVal = RESULT_FAILURE;
	}
	catch(CError_FileWrite)
	{
		nRetVal = RESULT_FAILURE;
	}

//_RETURN_:; 2002/05/22 Frozen �폜�i��O�������g�p����̂�goto�p�̃��x���͎g�p���Ȃ��j
	if( NULL != hwndProgress ){
		::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( NULL ) ){
			return RESULT_FAILURE;
		}
	}

	return nRetVal;
}





/* �f�[�^�̍폜 */
/*
|| �w��s���̕��������폜�ł��܂���
|| �f�[�^�ύX�ɂ���ĉe���̂������A�ύX�O�ƕύX��̍s�͈̔͂�Ԃ��܂�
|| ���̏������ƂɁA���C�A�E�g���Ȃǂ��X�V���Ă��������B
||
	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CDocLineMgr::DeleteData_CDocLineMgr(
	CLogicInt	nLine,
	CLogicInt	nDelPos,
	CLogicInt	nDelLen,
	CLogicInt*	pnModLineOldFrom,	//!< �e���̂������ύX�O�̍s(from)
	CLogicInt*	pnModLineOldTo,		//!< �e���̂������ύX�O�̍s(to)
	CLogicInt*	pnDelLineOldFrom,	//!< �폜���ꂽ�ύX�O�_���s(from)
	CLogicInt*	pnDelLineOldNum,	//!< �폜���ꂽ�s��
	CNativeW2*	cmemDeleted			//!< [out] �폜���ꂽ�f�[�^
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CDocLineMgr::DeleteData" );
#endif
	CDocLine*	pDocLine;
	CDocLine*	pDocLine2;
	wchar_t*	pData;
	CLogicInt	nDeleteLength;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	const wchar_t*	pLine2;
	CLogicInt		nLineLen2;

	*pnModLineOldFrom = nLine;	/* �e���̂������ύX�O�̍s(from) */
	*pnModLineOldTo = nLine;	/* �e���̂������ύX�O�̍s(to) */
	*pnDelLineOldFrom = CLogicInt(0);		/* �폜���ꂽ�ύX�O�_���s(from) */
	*pnDelLineOldNum = CLogicInt(0);		/* �폜���ꂽ�s�� */
//	cmemDeleted.SetData( "", lstrlen( "" ) );
	cmemDeleted->Clear();

	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		return;
	}

	pDocLine->SetModifyFlg(true);		/* �ύX�t���O */

	pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );

	if( nDelPos >= nLineLen ){
		return;
	}
	/* �u���s�v���폜����ꍇ�́A���̍s�ƘA������ */
//	if( ( nDelPos == nLineLen -1 && ( pLine[nDelPos] == CR || pLine[nDelPos] == LF ) )
//	 || nDelPos + nDelLen >= nLineLen
	if( ( EOL_NONE != pDocLine->m_cEol && nDelPos == nLineLen - pDocLine->m_cEol.GetLen() )
	 || ( EOL_NONE != pDocLine->m_cEol && nDelPos + nDelLen >  nLineLen - pDocLine->m_cEol.GetLen() )
	 || ( EOL_NONE == pDocLine->m_cEol && nDelPos + nDelLen >= nLineLen - pDocLine->m_cEol.GetLen() )
	){
		/* ���ۂɍ폜����o�C�g�� */
		nDeleteLength = nLineLen - nDelPos;

		/* �폜�����f�[�^ */
		cmemDeleted->SetString( &pLine[nDelPos], nDeleteLength );

		/* ���̍s�̏�� */
		if( NULL == ( pDocLine2 = pDocLine->m_pNext ) ){
			pData = new wchar_t[nLineLen + 1];
			if( nDelPos > 0 ){
				wmemcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				wmemcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			pData[ nLineLen - nDeleteLength ] = L'\0';
			/* ���s�R�[�h�̏����X�V */
			pDocLine->m_cEol.SetType( EOL_NONE );

			if( 0 < nLineLen - nDeleteLength ){
				pDocLine->m_cLine.SetString( pData, nLineLen - nDeleteLength );
			}else{
				// �s�̍폜
				// 2004.03.18 Moca �֐����g��
				DeleteNode( pDocLine );
				pDocLine = NULL;
				*pnDelLineOldFrom = nLine;	/* �폜���ꂽ�ύX�O�_���s(from) */
				*pnDelLineOldNum = CLogicInt(1);		/* �폜���ꂽ�s�� */
			}
			delete [] pData;
		}
		else{
			*pnModLineOldTo = nLine + CLogicInt(1);	/* �e���̂������ύX�O�̍s(to) */
			pLine2 = pDocLine2->m_cLine.GetStringPtr( &nLineLen2 );
			pData = new wchar_t[nLineLen + nLineLen2 + 1];
			if( nDelPos > 0 ){
				wmemcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				wmemcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			/* ���̍s�̃f�[�^��A�� */
			wmemcpy( pData + (nLineLen - nDeleteLength), pLine2, nLineLen2 );
			pData[ nLineLen - nDeleteLength + nLineLen2 ] = L'\0';
			pDocLine->m_cLine.SetString( pData, nLineLen - nDeleteLength + nLineLen2 );
			/* ���s�R�[�h�̏����X�V */
			pDocLine->m_cEol = pDocLine2->m_cEol;

			/* ���̍s���폜 && �����s�Ƃ̃��X�g�̘A��*/
			// 2004.03.18 Moca DeleteNode ���g��
			DeleteNode( pDocLine2 );
			pDocLine2 = NULL;
			*pnDelLineOldFrom = nLine + CLogicInt(1);	/* �폜���ꂽ�ύX�O�_���s(from) */
			*pnDelLineOldNum = CLogicInt(1);			/* �폜���ꂽ�s�� */
			delete [] pData;
		}
	}
	else{
		/* ���ۂɍ폜����o�C�g�� */
		nDeleteLength = nDelLen;

		/* �폜�����f�[�^ */
		cmemDeleted->SetString( &pLine[nDelPos], nDeleteLength );

		pData = new wchar_t[nLineLen + 1];
		if( nDelPos > 0 ){
			wmemcpy( pData, pLine, nDelPos );
		}
		if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
			wmemcpy(
				pData + nDelPos,
				pLine + nDelPos + nDeleteLength,
				nLineLen - ( nDelPos + nDeleteLength )
			);
		}
		pData[ nLineLen - nDeleteLength ] = L'\0';
		if( 0 < nLineLen - nDeleteLength ){
			pDocLine->m_cLine.SetString( pData, nLineLen - nDeleteLength );
		}else{
		}
		delete [] pData;
	}
//	DUMP();
	return;
}





/*!	�f�[�^�̑}��

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CDocLineMgr::InsertData_CDocLineMgr(
	CLogicInt		nLine,
	CLogicInt		nInsPos,
	const wchar_t*	pInsData,
	CLogicInt		nInsDataLen,
	CLogicInt*		pnInsLineNum,	// �}���ɂ���đ������s�̐�
	CLogicPoint*	pptNewPos		// �}�����ꂽ�����̎��̈ʒu
	/*
	CLogicInt*		pnNewLine,		// �}�����ꂽ�����̎��̈ʒu�̍s
	CLogicInt*		pnNewPos		// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
	*/
)
{
	CDocLine*	pDocLine;
	CNativeW2	cmemPrevLine;
	CNativeW2	cmemCurLine;
	CNativeW2	cmemNextLine;
	CLogicInt	nAllLinesOld = m_nLines;
	CEOL 		cEOLTypeNext;

	bool		bBookMarkNext;	// 2001.12.03 hor �}���ɂ��}�[�N�s�̐���

	pptNewPos->y = nLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */

	//	Jan. 25, 2004 genta
	//	�}�������񒷂�0�̏ꍇ�ɍŌ�܂�pnNewPos���ݒ肳��Ȃ��̂�
	//	�����l�Ƃ���0�ł͂Ȃ��J�n�ʒu�Ɠ����l�����Ă����D
	pptNewPos->x  = nInsPos;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */

	/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
	*pnInsLineNum = CLogicInt(0);
	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		/* ������NULL���A���Ă���Ƃ������Ƃ́A*/
		/* �S�e�L�X�g�̍Ō�̎��̍s��ǉ����悤�Ƃ��Ă��邱�Ƃ����� */
		cmemPrevLine.SetString(L"");
		cmemNextLine.SetString(L"");
		cEOLTypeNext.SetType( EOL_NONE );
		bBookMarkNext=false;	// 2001.12.03 hor
	}
	else{
		pDocLine->SetModifyFlg(true);		/* �ύX�t���O */

		CLogicInt		nLineLen;
		const wchar_t*	pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
		cmemPrevLine.SetString( pLine, nInsPos );
		cmemNextLine.SetString( &pLine[nInsPos], nLineLen - nInsPos );

		cEOLTypeNext = pDocLine->m_cEol;
		bBookMarkNext= pDocLine->IsBookMarked();	// 2001.12.03 hor
	}

	int			nCount = 0;
	CLogicInt	nBgn   = CLogicInt(0);
	CLogicInt	nPos   = CLogicInt(0);
	CEOL 		cEOLType;
	for( nPos = CLogicInt(0); nPos < nInsDataLen; ){
		if( pInsData[nPos] == L'\n' || pInsData[nPos] == L'\r' ){
			/* �s�I�[�q�̎�ނ𒲂ׂ� */
			cEOLType.GetTypeFromString( &pInsData[nPos], nInsDataLen - nPos );
			/* �s�I�[�q���܂߂ăe�L�X�g���o�b�t�@�Ɋi�[ */
			cmemCurLine.SetString( &pInsData[nBgn], nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + CLogicInt(cEOLType.GetLen());
			nPos = nBgn;
			if( NULL == pDocLine ){
				CDocLine* pDocLineNew = new CDocLine;

				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					this->_PushBottom(pDocLineNew);

					pDocLineNew->m_cLine.SetNativeData( cmemPrevLine );
					pDocLineNew->m_cLine += cmemCurLine;

					pDocLineNew->m_cEol = cEOLType;							/* ���s�R�[�h�̎�� */
				}
				else{
					this->_PushBottom(pDocLineNew);

					pDocLineNew->m_cLine.SetNativeData( cmemCurLine );

					pDocLineNew->m_cEol = cEOLType;							/* ���s�R�[�h�̎�� */
				}
				pDocLine = NULL;
			}else{
				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					pDocLine->m_cLine.SetNativeData( cmemPrevLine );
					pDocLine->m_cLine += cmemCurLine;

					pDocLine->m_cEol = cEOLType;						/* ���s�R�[�h�̎�� */

					// 2001.12.13 hor
					// �s���ŉ��s�����猳�̍s�̃}�[�N��V�����s�Ɉړ�����
					// ����ȊO�Ȃ猳�̍s�̃}�[�N���ێ����ĐV�����s�ɂ̓}�[�N��t���Ȃ�
					if(nInsPos==CLogicInt(0)){
						pDocLine->SetBookMark(false);
					}
					else{
						bBookMarkNext=false;
					}

					pDocLine = pDocLine->m_pNext;
				}else{
					CDocLine* pDocLineNew = new CDocLine;

					_Insert(pDocLineNew, pDocLine); //pDocLine�̒��O�ɑ}��
					
					pDocLineNew->m_cLine.SetNativeData( cmemCurLine );

					pDocLineNew->m_cEol = cEOLType;							/* ���s�R�[�h�̎�� */
				}
			}

			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			++nCount;
			++pptNewPos->y;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
		}else{
			++nPos;
		}
	}
//	nEOLType = EOL_NONE;
	if( CLogicInt(0) < nPos - nBgn || 0 < cmemNextLine.GetStringLength() ){
		cmemCurLine.SetString( &pInsData[nBgn], nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pDocLine ){
			CDocLine* pDocLineNew = new CDocLine;
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				_PushBottom(pDocLineNew);

				pDocLineNew->m_cLine.SetNativeData( cmemPrevLine );
				pDocLineNew->m_cLine += cmemCurLine;

				pDocLineNew->m_cEol = cEOLTypeNext;							/* ���s�R�[�h�̎�� */

			}else{
				_PushBottom(pDocLineNew);

				pDocLineNew->m_cLine.SetNativeData( cmemCurLine );

				pDocLineNew->m_cEol = cEOLTypeNext;							/* ���s�R�[�h�̎�� */

			}
			pDocLine = NULL;
			pptNewPos->x = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
		}else{
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				pDocLine->m_cLine.SetNativeData( cmemPrevLine );
				pDocLine->m_cLine += cmemCurLine;

				pDocLine->m_cEol = cEOLTypeNext;						/* ���s�R�[�h�̎�� */
				// pDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];	/* ���s�R�[�h�̒��� */

				pDocLine = pDocLine->m_pNext;
				pptNewPos->x = CLogicInt(cmemPrevLine.GetStringLength()) + nPos - nBgn;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}
			else{
				CDocLine* pDocLineNew = new CDocLine;

				_Insert(pDocLineNew, pDocLine); //pDocLine�̒��O�ɑ}��

				pDocLineNew->m_cLine.SetNativeData( cmemCurLine );

				pDocLineNew->m_cEol = cEOLTypeNext;							/* ���s�R�[�h�̎�� */
				pDocLineNew->SetBookMark(bBookMarkNext);	// 2001.12.03 hor �u�b�N�}�[�N�𕜌�

				pptNewPos->x = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}
		}
	}
	*pnInsLineNum = m_nLines - nAllLinesOld;
	return;
}

//nEOLType nEOLType nEOLTypeNext



/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
// 2001/06/23 N.Nakatani WhereCurrentWord()�ύX WhereCurrentWord_2���R�[������悤�ɂ���
bool CDocLineMgr::WhereCurrentWord(
	CLogicInt	nLineNum,
	CLogicInt	nIdx,
	CLogicInt*	pnIdxFrom,
	CLogicInt*	pnIdxTo,
	CNativeW2*	pcmcmWord,
	CNativeW2*	pcmcmWordLeft
)
{
	*pnIdxFrom = nIdx;
	*pnIdxTo = nIdx;

	CDocLine*	pDocLine = GetLineInfo( nLineNum );
	if( NULL == pDocLine ){
		return false;
	}

	CLogicInt		nLineLen;
	const wchar_t*	pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );

	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	return CWordParse::WhereCurrentWord_2( pLine, nLineLen, nIdx, pnIdxFrom, pnIdxTo, pcmcmWord, pcmcmWordLeft );
}




// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
bool CDocLineMgr::PrevOrNextWord(
	CLogicInt	nLineNum,		//	�s��
	CLogicInt	nIdx,			//	����
	CLogicInt*	pnColmNew,		//	���������ʒu
	BOOL		bLEFT,			//	TRUE:�O���i���j�֌������BFALSE:����i�E�j�֌������B
	BOOL		bStopsBothEnds	//	�P��̗��[�Ŏ~�܂�
)
{
	using namespace WCODE;

	CDocLine*	pDocLine = GetLineInfo( nLineNum );
	if( NULL == pDocLine ){
		return false;
	}

	CLogicInt		nLineLen;
	const wchar_t*	pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );

	// ABC D[EOF]�ƂȂ��Ă����Ƃ��ɁAD�̌��ɃJ�[�\�������킹�A�P��̍��[�Ɉړ�����ƁAA�ɃJ�[�\���������o�O�C���BYAZAKI
	if( nIdx >= nLineLen ){
		if (bLEFT && nIdx == nLineLen){
		}
		else {
			nIdx = nLineLen - CLogicInt(1);
		}
	}
	/* ���݈ʒu�̕����̎�ނɂ���Ă͑I��s�\ */
	if( !bLEFT && ( pLine[nIdx] == CR || pLine[nIdx] == LF ) ){
		return false;
	}
	/* �O�̒P�ꂩ�H���̒P�ꂩ�H */
	if( bLEFT ){
		/* ���݈ʒu�̕����̎�ނ𒲂ׂ� */
		ECharKind	nCharKind = CWordParse::WhatKindOfChar( pLine, nLineLen, nIdx );
		if( nIdx == 0 ){
			return false;
		}

		/* ������ނ��ς��܂őO���փT�[�` */
		/* �󔒂ƃ^�u�͖������� */
		int		nCount = 0;
		CLogicInt	nIdxNext = nIdx;
		CLogicInt	nCharChars = CLogicInt(&pLine[nIdxNext] - CNativeW2::GetCharPrev( pLine, nLineLen, &pLine[nIdxNext] ));
		while( nCharChars > 0 ){
			CLogicInt		nIdxNextPrev = nIdxNext;
			nIdxNext -= nCharChars;
			ECharKind nCharKindNext = CWordParse::WhatKindOfChar( pLine, nLineLen, nIdxNext );
			/* �󔒂ƃ^�u�͖������� */
			if( nCharKind == CK_ZEN_NOBASU ){
				if( nCharKindNext == CK_HIRA || nCharKindNext == CK_ZEN_KATA ){
					nCharKind = nCharKindNext;
				}
			}
			else if( nCharKind == CK_HIRA || nCharKind == CK_ZEN_KATA ){
				if( nCharKindNext == CK_ZEN_NOBASU ){
					nCharKindNext = nCharKind;
				}
			}
			if( nCharKind != nCharKindNext ){
				/* �T�[�`�J�n�ʒu�̕������󔒂܂��̓^�u�̏ꍇ */
				if( nCharKind == CK_TAB	|| nCharKind == CK_SPACE ){
					if ( bStopsBothEnds && nCount ){
						nIdxNext = nIdxNextPrev;
						break;
					}
					nCharKind = nCharKindNext;
				}else{
					if( nCount == 0){
						nCharKind = nCharKindNext;
					}else{
						nIdxNext = nIdxNextPrev;
						break;
					}
				}
			}
			nCharChars = CLogicInt(&pLine[nIdxNext] - CNativeW2::GetCharPrev( pLine, nLineLen, &pLine[nIdxNext] ));
			++nCount;
		}
		*pnColmNew = nIdxNext;
	}else{
		CWordParse::SearchNextWordPosition(pLine, nLineLen, nIdx, pnColmNew, bStopsBothEnds);
	}
	return true;
}





/*! �P�ꌟ��

	@date 2003.05.22 ����� �s�������Ȃǌ�����
	@date 2005.11.26 ����� \r��.��\r\n�Ƀq�b�g���Ȃ��悤��
*/
/* ������Ȃ��ꍇ�͂O��Ԃ� */
int CDocLineMgr::SearchWord(
	CLogicPoint				ptSerachBegin,	//!< �����J�n�ʒu
	const wchar_t*			pszPattern,		//!< ��������
	ESearchDirection		eDirection,		//!< ��������
	const SSearchOption&	sSearchOption,	//!< �����I�v�V����
	CLogicRange*			pMatchRange,	//!< [out] �}�b�`�͈́B���W�b�N�P�ʁB
	CBregexp*				pRegexp			//!< [in]  ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������ */
)
{
	CDocLine*	pDocLine;
	CLogicInt	nLinePos;
	CLogicInt	nIdxPos;
	CLogicInt	nIdxPosOld;
	const wchar_t*	pLine;
	int			nLineLen;
	const wchar_t*	pszRes;
	int			nHitTo;
	int			nHitPos;
	int			nHitPosOld;
	int			nRetVal = 0;
	//	Jun. 10, 2003 Moca
	//	lstrlen�𖈉�Ă΂���nPatternLen���g���悤�ɂ���
	const int	nPatternLen = wcslen( pszPattern );	//2001/06/23 N.Nakatani

	// ���������̏�� -> pnKey_CharCharsArr
	int* pnKey_CharCharsArr = NULL;
	CDocLineMgr::CreateCharCharsArr(
		pszPattern,
		nPatternLen,
		&pnKey_CharCharsArr
	);

	//���K�\��
	if( sSearchOption.bRegularExp ){
		nLinePos = ptSerachBegin.GetY2();		// �����s�������J�n�s
		pDocLine = GetLineInfo( nLinePos );
		//�O������
		if( eDirection == SEARCH_BACKWARD ){
			//
			// �O��(��)����(���K�\��)
			//
			nHitTo = ptSerachBegin.x;				// �����J�n�ʒu
			nIdxPos = 0;
			while( NULL != pDocLine ){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				nHitPos		= -1;	// -1:���̍s�Ń}�b�`�ʒu�Ȃ�
				while( 1 ){
					nHitPosOld = nHitPos;
					nIdxPosOld = nIdxPos;
					if (	nIdxPos <= pDocLine->GetLengthWithoutEOL() 
						&&	pRegexp->Match( pLine, nLineLen, nIdxPos ) ){
						// �����Ƀ}�b�`�����I
						nHitPos = pRegexp->GetIndex();
						nIdxPos = pRegexp->GetLastIndex();
						// �����O�Ń}�b�`�����̂ŁA���̈ʒu�ōēx�}�b�`���Ȃ��悤�ɁA�P�����i�߂�
						if (nIdxPos == nHitPos) {
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nIdxPos += (CNativeW2::GetSizeOfChar( pLine, nLineLen, nIdxPos ) == 2 ? 2 : 1);
						}
						if( nHitPos >= nHitTo ){
							// �}�b�`�����̂́A�J�[�\���ʒu�ȍ~������
							// ���łɃ}�b�`�����ʒu������΁A�����Ԃ��A�Ȃ���ΑO�̍s��
							break;
						}
					} else {
						// �}�b�`���Ȃ�����
						// ���łɃ}�b�`�����ʒu������΁A�����Ԃ��A�Ȃ���ΑO�̍s��
						break;
					}
				}

				if ( -1 != nHitPosOld ) {
					// ���̍s�Ń}�b�`�����ʒu�����݂���̂ŁA���̍s�Ō����I��
					pMatchRange->SetFromX( nHitPosOld );	// �}�b�`�ʒufrom
					pMatchRange->SetToX  ( nIdxPosOld );	// �}�b�`�ʒuto
					break;
				} else {
					// ���̍s�Ń}�b�`�����ʒu�����݂��Ȃ��̂ŁA�O�̍s��������
					nLinePos--;
					pDocLine = pDocLine->m_pPrev;
					nIdxPos = 0;
					if( NULL != pDocLine ){
						nHitTo = pDocLine->m_cLine.GetStringLength() + 1;		// �O�̍s��NULL����(\0)�ɂ��}�b�`�����邽�߂�+1 2003.05.16 ����� 
					}
				}
			}
		}
		//�������
		else {
			//
			// �������(���K�\��)
			//
			nIdxPos = ptSerachBegin.x;
			while( NULL != pDocLine ){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				if(		nIdxPos <= pDocLine->GetLengthWithoutEOL() 
					&&	pRegexp->Match( pLine, nLineLen, nIdxPos ) ){
					// �}�b�`����
					pMatchRange->SetFromX( pRegexp->GetIndex()     );	// �}�b�`�ʒufrom
					pMatchRange->SetToX  ( pRegexp->GetLastIndex() );	// �}�b�`�ʒuto
					break;
				}
				++nLinePos;
				pDocLine = pDocLine->m_pNext;
				nIdxPos = 0;
			}
		}
		//
		// ���K�\�������̌㏈��
		if ( pDocLine != NULL ) {
			// �}�b�`�����s������
			pMatchRange->SetFromY(nLinePos); // �}�b�`�s
			pMatchRange->SetToY  (nLinePos); // �}�b�`�s
			nRetVal = 1;
			// ���C�A�E�g�s�ł͉��s�������̈ʒu��\���ł��Ȃ����߁A�}�b�`�J�n�ʒu��␳
			if (pMatchRange->GetFrom().x > pDocLine->GetLengthWithoutEOL()) {
				// \r\n���s����\n�Ƀ}�b�`����ƒu���ł��Ȃ��s��ƂȂ邽��
				// ���s�������Ń}�b�`�����ꍇ�A���s�����̎n�߂���}�b�`�������Ƃɂ���
				pMatchRange->SetFromX( pDocLine->GetLengthWithoutEOL() );
			}
		}
	}
	//�P��̂݌���
	else if( sSearchOption.bWordOnly ){
		/*
			2001/06/23 Norio Nakatani
			�P��P�ʂ̌����������I�Ɏ����B�P���WhereCurrentWord()�Ŕ��ʂ��Ă܂��̂ŁA
			�p�P���C/C++���ʎq�Ȃǂ̌��������Ȃ�q�b�g���܂��B
		*/

		// �O������
		if( eDirection == SEARCH_BACKWARD ){
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = GetLineInfo( nLinePos );
			CLogicInt nNextWordFrom;
			CLogicInt nNextWordFrom2;
			CLogicInt nNextWordTo2;
			CLogicInt nWork;
			nNextWordFrom = ptSerachBegin.GetX2();
			while( NULL != pDocLine ){
				if( PrevOrNextWord( nLinePos, nNextWordFrom, &nWork, TRUE, FALSE ) ){
					nNextWordFrom = nWork;
					if( WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL ) ){
						if( nPatternLen == nNextWordTo2 - nNextWordFrom2 ){
							const wchar_t* pData = pDocLine->m_cLine.GetStringPtr();	// 2002/2/10 aroka CMemory�ύX
							/* 1==�啶���������̋�� */
							if( (!sSearchOption.bLoHiCase && 0 == _memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) ) ||
								(sSearchOption.bLoHiCase && 0 ==	 auto_memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) )
							){
								pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
								pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
								pMatchRange->SetFromX(nNextWordFrom2);						// �}�b�`�ʒufrom
								pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);// �}�b�`�ʒuto
								nRetVal = 1;
								goto end_of_func;
							}
						}
						continue;
					}
				}
				/* �O�̍s�����ɍs�� */
				nLinePos--;
				pDocLine = pDocLine->m_pPrev;
				if( NULL != pDocLine ){
					nNextWordFrom = pDocLine->m_cLine.GetStringLength() - pDocLine->m_cEol.GetLen();
					if( 0 > nNextWordFrom ){
						nNextWordFrom = CLogicInt(0);
					}
				}
			}
		}
		// �������
		else{
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = GetLineInfo( nLinePos );
			CLogicInt nNextWordFrom;

			CLogicInt nNextWordFrom2;
			CLogicInt nNextWordTo2;
			nNextWordFrom = ptSerachBegin.GetX2();
			while( NULL != pDocLine ){
				if( WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL ) ){
					if( nPatternLen == nNextWordTo2 - nNextWordFrom2 ){
						const wchar_t* pData = pDocLine->m_cLine.GetStringPtr();	// 2002/2/10 aroka CMemory�ύX
						/* 1==�啶���������̋�� */
						if( (!sSearchOption.bLoHiCase && 0 ==  _memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) ) ||
							(sSearchOption.bLoHiCase && 0 == auto_memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) )
						){
							pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
							pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
							pMatchRange->SetFromX(nNextWordFrom2);						// �}�b�`�ʒufrom
							pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);// �}�b�`�ʒuto
							nRetVal = 1;
							goto end_of_func;
						}
					}
					/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
					if( PrevOrNextWord( nLinePos, nNextWordFrom, &nNextWordFrom, FALSE, FALSE ) ){
						continue;
					}
				}
				/* ���̍s�����ɍs�� */
				nLinePos++;
				pDocLine = pDocLine->m_pNext;
				nNextWordFrom = CLogicInt(0);
			}
		}

		nRetVal = 0;
		goto end_of_func;
	}
	//���ʂ̌��� (���K�\���ł��P��P�ʂł��Ȃ�)
	else{
		//�O������
		if( eDirection == SEARCH_BACKWARD ){
			nLinePos = ptSerachBegin.GetY2();
			nHitTo = ptSerachBegin.x;

			nIdxPos = 0;
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				nHitPos = -1;
				while( 1 ){
					nHitPosOld = nHitPos;
					nIdxPosOld = nIdxPos;
					pszRes = SearchString(
						pLine,
						nLineLen,
						nIdxPos,
						pszPattern,
						nPatternLen,
						pnKey_CharCharsArr,
						sSearchOption.bLoHiCase
					);
					if( NULL != pszRes ){
						nHitPos = pszRes - pLine;
						nIdxPos = nHitPos + nPatternLen;	// �}�b�`�����񒷐i�߂�悤�ɕύX 2005.10.28 Karoto
						if( nHitPos >= nHitTo ){
							if( -1 != nHitPosOld ){
								pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
								pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
								pMatchRange->SetFromX(nHitPosOld);	// �}�b�`�ʒufrom
 								pMatchRange->SetToX  (nIdxPosOld);	// �}�b�`�ʒuto
								nRetVal = 1;
								goto end_of_func;
							}else{
								break;
							}
						}
					}else{
						if( -1 != nHitPosOld ){
							pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
							pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
							pMatchRange->SetFromX(nHitPosOld);	// �}�b�`�ʒufrom
							pMatchRange->SetToX  (nIdxPosOld);	// �}�b�`�ʒuto
							nRetVal = 1;
							goto end_of_func;
						}else{
							break;
						}
					}
				}
				nLinePos--;
				pDocLine = pDocLine->m_pPrev;
				nIdxPos = 0;
				if( NULL != pDocLine ){
					nHitTo = pDocLine->m_cLine.GetStringLength();
				}
			}
			nRetVal = 0;
			goto end_of_func;
		}
		//�������
		else{
			nIdxPos = ptSerachBegin.x;
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				pszRes = SearchString(
					pLine,
					nLineLen,
					nIdxPos,
					pszPattern,
					nPatternLen,
					pnKey_CharCharsArr,
					sSearchOption.bLoHiCase
				);
				if( NULL != pszRes ){
					pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
					pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
					pMatchRange->SetFromX(pszRes - pLine);							// �}�b�`�ʒufrom (�����P��)
					pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);	// �}�b�`�ʒuto   (�����P��)
					nRetVal = 1;
					goto end_of_func;
				}
				++nLinePos;
				pDocLine = pDocLine->m_pNext;
				nIdxPos = 0;
			}
			nRetVal = 0;
			goto end_of_func;
		}
	}
end_of_func:;
	if( NULL != pnKey_CharCharsArr ){
		delete [] pnKey_CharCharsArr;
		pnKey_CharCharsArr = NULL;
	}

	return nRetVal;
}

/* ���������̏��(�L�[������̑S�p�����p���̔z��)�쐬 */
void CDocLineMgr::CreateCharCharsArr(
	const wchar_t*	pszPattern,
	int				nSrcLen,
	int**			ppnCharCharsArr
)
{
	int		i;
	int*	pnCharCharsArr;
	pnCharCharsArr = new int[nSrcLen];
	for( i = 0; i < nSrcLen; /*i++*/ ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		pnCharCharsArr[i] = CNativeW2::GetSizeOfChar( pszPattern, nSrcLen, i );
		if( 0 == pnCharCharsArr[i] ){
			pnCharCharsArr[i] = 1;
		}
		if( 2 == pnCharCharsArr[i] ){
			pnCharCharsArr[i + 1] = pnCharCharsArr[i];
		}
		i+= pnCharCharsArr[i];
	}
	*ppnCharCharsArr = pnCharCharsArr;
	return;
}


/*!
	�����񌟍�
	@return ���������ꏊ�̃|�C���^�B������Ȃ�������NULL�B
*/
const wchar_t* CDocLineMgr::SearchString(
	const wchar_t*	pLine,
	int				nLineLen,
	int				nIdxPos,
	const wchar_t*	pszPattern,
	int				nPatternLen,
	int*			pnCharCharsArr,
	bool			bLoHiCase
)
{
	if( nLineLen < nPatternLen ){
		return NULL;
	}
	if( 0 >= nPatternLen || 0 >= nLineLen){
		return NULL;
	}

	int	nPos;
	int	nCompareTo;

	// ���`�T�� (kobake��: ���CharPointerW�ɒu��������ƁA�T���Q�[�g�y�A�ɑΉ��ł��܂�)
	nCompareTo = nLineLen - nPatternLen;	//	Mar. 4, 2001 genta

	for( nPos = nIdxPos; nPos <= nCompareTo; nPos++ ){
		int	i;
		for( i = 0; i < nPatternLen; i++ ){
			if( !bLoHiCase ){	//	�p�召�����̓��ꎋ
				if( towupper( pLine[nPos + i] ) != towupper( pszPattern[i] ) )
					break;
			}
			else {
				if( pLine[nPos + i] != pszPattern[i] ){
					break;
				}
			}
		}
		if( i >= nPatternLen ){
			return &pLine[nPos];
		}
	}
	return NULL;
}




/*!	@brief CDocLineMgrDEBUG�p

	@date 2004.03.18 Moca
		m_pDocLineCurrent��m_pCodePrevRefer���f�[�^�`�F�[����
		�v�f���w���Ă��邩�̌��؋@�\��ǉ��D

*/
void CDocLineMgr::DUMP( void )
{
#ifdef _DEBUG
	MYTRACE_A( "------------------------\n" );

	CDocLine* pDocLine;
	CDocLine* pDocLineNext;
	CDocLine* pDocLineEnd = NULL;
	pDocLine = m_pDocLineTop;

	// �������𒲂ׂ�
	bool bIncludeCurrent = false;
	bool bIncludePrevRefer = false;
	CLogicInt nNum = CLogicInt(0);
	if( m_pDocLineTop->m_pPrev != NULL ){
		MYTRACE_A( "error: m_pDocLineTop->m_pPrev != NULL\n");
	}
	if( m_pDocLineBot->m_pNext != NULL ){
		MYTRACE_A( "error: m_pDocLineBot->m_pNext != NULL\n" );
	}
	while( NULL != pDocLine ){
		if( m_pDocLineCurrent == pDocLine ){
			bIncludeCurrent = true;
		}
		if( m_pCodePrevRefer == pDocLine ){
			bIncludePrevRefer = true;
		}
		if( NULL != pDocLine->m_pNext ){
			if( pDocLine->m_pNext == pDocLine ){
				MYTRACE_A( "error: pDocLine->m_pPrev Invalid value.\n" );
				break;
			}
			if( pDocLine->m_pNext->m_pPrev != pDocLine ){
				MYTRACE_A( "error: pDocLine->m_pNext->m_pPrev != pDocLine.\n" );
				break;
			}
		}else{
			pDocLineEnd = pDocLine;
		}
		pDocLine = pDocLine->m_pNext;
		nNum++;
	}
	
	if( pDocLineEnd != m_pDocLineBot ){
		MYTRACE_A( "error: pDocLineEnd != m_pDocLineBot" );
	}
	
	if( nNum != m_nLines ){
		MYTRACE_A( "error: nNum(%d) != m_nLines(%d)\n", nNum, m_nLines );
	}
	if( false == bIncludeCurrent && m_pDocLineCurrent != NULL ){
		MYTRACE_A( "error: m_pDocLineCurrent=%08lxh Invalid value.\n", m_pDocLineCurrent );
	}
	if( false == bIncludePrevRefer && m_pCodePrevRefer != NULL ){
		MYTRACE_A( "error: m_pCodePrevRefer =%08lxh Invalid value.\n", m_pCodePrevRefer );
	}

	// DUMP
	MYTRACE_A( "m_nLines=%d\n", m_nLines );
	MYTRACE_A( "m_pDocLineTop=%08lxh\n", m_pDocLineTop );
	MYTRACE_A( "m_pDocLineBot=%08lxh\n", m_pDocLineBot );
	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine ){
		pDocLineNext = pDocLine->m_pNext;
		MYTRACE_A( "\t-------\n" );
		MYTRACE_A( "\tthis=%08lxh\n", pDocLine );
		MYTRACE_A( "\tpPrev; =%08lxh\n", pDocLine->m_pPrev );
		MYTRACE_A( "\tpNext; =%08lxh\n", pDocLine->m_pNext );

		MYTRACE_A( "\tm_enumEOLType =%ls\n", pDocLine->m_cEol.GetName() );
		MYTRACE_A( "\tm_nEOLLen =%d\n", pDocLine->m_cEol.GetLen() );


//		MYTRACE_A( "\t[%ls]\n", *(pDocLine->m_pLine) );
		MYTRACE_A( "\tpDocLine->m_cLine.GetLength()=[%d]\n", pDocLine->m_cLine.GetStringLength() );
		MYTRACE_A( "\t[%ls]\n", pDocLine->m_cLine.GetStringPtr() );


		pDocLine = pDocLineNext;
	}
	MYTRACE_A( "------------------------\n" );
#endif
	return;
}

/* �s�ύX��Ԃ����ׂă��Z�b�g */
/*

  �E�ύX�t���OCDocLine�I�u�W�F�N�g�쐬���ɂ�TRUE�ł���
  �E�ύX�񐔂�CDocLine�I�u�W�F�N�g�쐬���ɂ�1�ł���

  �t�@�C����ǂݍ��񂾂Ƃ��͕ύX�t���O�� FALSE�ɂ���
  �t�@�C����ǂݍ��񂾂Ƃ��͕ύX�񐔂� 0�ɂ���

  �t�@�C�����㏑���������͕ύX�t���O�� FALSE�ɂ���
  �t�@�C�����㏑���������͕ύX�񐔂͕ς��Ȃ�

  �ύX�񐔂�Undo�����Ƃ���-1�����
  �ύX�񐔂�0�ɂȂ����ꍇ�͕ύX�t���O��FALSE�ɂ���



*/
void CDocLineMgr::ResetAllModifyFlag( void )
//		BOOL bResetModifyCount /* �ύX�񐔂�0�ɂ��邩�ǂ��� */
//)
{
	CDocLine* pDocLine;
	CDocLine* pDocLineNext;
	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine ){
		pDocLineNext = pDocLine->m_pNext;
		pDocLine->SetModifyFlg(false);		/* �ύX�t���O */
//		if( bResetModifyCount ){			/* �ύX�񐔂�0�ɂ��邩�ǂ��� */
//			pDocLine->m_nModifyCount = 0;	/* �ύX�� */
//		}

		pDocLine = pDocLineNext;
	}
	return;
}


/* �S�s�f�[�^��Ԃ�
	���s�R�[�h�́ACFLF���ꂳ���B
	@retval �S�s�f�[�^�Bfree�ŊJ�����Ȃ���΂Ȃ�Ȃ��B
	@note   Debug�ł̃e�X�g�ɂ̂ݎg�p���Ă���B
*/
wchar_t* CDocLineMgr::GetAllData( int*	pnDataLen )
{
	int			nDataLen;
	wchar_t*	pLine;
	int			nLineLen;
	CDocLine* 	pDocLine;

	pDocLine = m_pDocLineTop;
	nDataLen = 0;
	while( NULL != pDocLine ){
		//	Oct. 7, 2002 YAZAKI
		nDataLen += pDocLine->GetLengthWithoutEOL() + 2;	//	\r\n��ǉ����ĕԂ�����+2����B
		pDocLine = pDocLine->m_pNext;
	}

	wchar_t* pData;
	pData = (wchar_t*)malloc( (nDataLen + 1) * sizeof(wchar_t) );
	if( NULL == pData ){
		::MYMESSAGEBOX_A(
			NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME_A,
			"CDocLineMgr::GetAllData()\n�������m�ۂɎ��s���܂����B\n%d�o�C�g",
			nDataLen + 1
		);
		return NULL;
	}
	pDocLine = m_pDocLineTop;

	nDataLen = 0;
	while( NULL != pDocLine ){
		//	Oct. 7, 2002 YAZAKI
		nLineLen = pDocLine->GetLengthWithoutEOL();
		if( 0 < nLineLen ){
			pLine = pDocLine->m_cLine.GetStringPtr();
			wmemcpy( &pData[nDataLen], pLine, nLineLen );
			nDataLen += nLineLen;
		}
		pData[nDataLen++] = L'\r';
		pData[nDataLen++] = L'\n';
		pDocLine = pDocLine->m_pNext;
	}
	pData[nDataLen] = L'\0';
	*pnDataLen = nDataLen;
	return pData;
}


/* �s�I�u�W�F�N�g�̍폜�A���X�g�ύX�A�s��-- */
void CDocLineMgr::DeleteNode( CDocLine* pCDocLine )
{
	m_nLines--;	/* �S�s�� */
	if( CLogicInt(0) == m_nLines ){
		/* �f�[�^���Ȃ��Ȃ��� */
		Init();
		return;
	}
	
	if( NULL == pCDocLine->m_pPrev ){
		m_pDocLineTop = pCDocLine->m_pNext;
	}
	else{
		pCDocLine->m_pPrev->m_pNext = pCDocLine->m_pNext;
	}

	if( NULL == pCDocLine->m_pNext ){
		m_pDocLineBot = pCDocLine->m_pPrev;
	}
	else{
		pCDocLine->m_pNext->m_pPrev = pCDocLine->m_pPrev;
	}
	
	if( m_pCodePrevRefer == pCDocLine ){
		m_pCodePrevRefer = pCDocLine->m_pNext;
	}
	delete pCDocLine;

	return;
}



/* �s�I�u�W�F�N�g�̑}���A���X�g�ύX�A�s��++ */
/* pCDocLinePrev�̎���pCDocLine��}������ */
/* NULL==pCDocLinePrev�̂Ƃ����X�g�̐擪�ɑ}�� */
void CDocLineMgr::InsertNode( CDocLine* pCDocLinePrev, CDocLine* pCDocLine )
{
	pCDocLine->m_pPrev = pCDocLinePrev;
	if( NULL != pCDocLinePrev ){
		pCDocLine->m_pNext = pCDocLinePrev->m_pNext;
		pCDocLinePrev->m_pNext = pCDocLine;
	}else{
		pCDocLine->m_pNext = m_pDocLineTop;
		m_pDocLineTop = pCDocLine;
	}
	if( NULL != pCDocLine->m_pNext ){
		pCDocLine->m_pNext->m_pPrev = pCDocLine;
	}else{
		m_pDocLineBot = pCDocLine;
	}
	m_pDocLineTop = pCDocLine;
	m_nLines++;	/* �S�s�� */
	return;

}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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

//!pPos�̒��O�ɑ}��
void CDocLineMgr::_Insert(CDocLine* pDocLineNew, CDocLine* pPos)
{
	pDocLineNew->m_pPrev = pPos->m_pPrev;
	pDocLineNew->m_pNext = pPos;
	pPos->m_pPrev->m_pNext = pDocLineNew;
	pPos->m_pPrev = pDocLineNew;

	++m_nLines;
}

