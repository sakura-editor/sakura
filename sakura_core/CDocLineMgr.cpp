//	$Id$
/*!	@file
	@brief �s�f�[�^�̊Ǘ�

	@author Norio Nakatani
	@date 1998/03/05  �V�K�쐬
	@date 2001/06/23 N.Nakatani �P��P�ʂŌ�������@�\������
	@date 2001/06/23 N.Nakatani WhereCurrentWord()�ύX WhereCurrentWord_2���R�[������悤�ɂ���
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, jepro, hor
	Copyright (C) 2002, hor, aroka, MIK, Moca

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
#include "etc_uty.h"
#include "CRunningTimer.h"

//	May 15, 2000 genta
#include "CEol.h"
#include "CDocLine.h"// 2002/2/10 aroka �w�b�_����
#include "CMemory.h"// 2002/2/10 aroka

#include "CFileWrite.h" //2002/05/22 Frozen
#include "CFileLoad.h" // 2002/08/30 Moca
#include "my_icmp.h" // Nov. 29, 2002 genta/moca

/* ������ގ��ʎq */
#define	CK_NULL			0	/*!< NULL 0x0<=c<=0x0 */
#define	CK_TAB			1	/*!< �^�u 0x9<=c<=0x9 */
#define	CK_CR			2	/*!< CR = 0x0d  */
#define	CK_LF			3	/*!< LF = 0x0a  */

#define	CK_SPACE		4	/*!< ���p�̃X�y�[�X 0x20<=c<=0x20 */
#define	CK_CSYM			5	/*!< ���p�̉p���A�A���_�[�X�R�A�A�����̂����ꂩ */
#define	CK_KATA			6	/*!< ���p�̃J�^�J�i 0xA1<=c<=0xFD */
#define	CK_ETC			7	/*!< ���p�̂��̑� */

#define	CK_MBC_SPACE	12	/*!< 2�o�C�g�̃X�y�[�X */
							/*!< 0x8140<=c<=0x8140 �S�p�X�y�[�X */
#define	CK_MBC_NOVASU	13	/*!< �L�΂��L�� 0x815B<=c<=0x815B '�[' */
#define	CK_MBC_CSYM		14	/*!< 2�o�C�g�̉p���A�A���_�[�X�R�A�A�����̂����ꂩ */
							/*!< 0x8151<=c<=0x8151 �S�p�A���_�[�X�R�A */
							/*!< 0x824F<=c<=0x8258 �S�p���� */
							/*!< 0x8260<=c<=0x8279 �S�p�p���啶�� */
							/*!< 0x8281<=c<=0x829a �S�p�p�������� */
#define	CK_MBC_KIGO		15	/*!< 2�o�C�g�̋L�� */
							/*!< 0x8141<=c<=0x81FD */
#define	CK_MBC_HIRA		16	/*!< 2�o�C�g�̂Ђ炪�� */
							/*!< 0x829F<=c<=0x82F1 �S�p�Ђ炪�� */
#define	CK_MBC_KATA		17	/*!< 2�o�C�g�̃J�^�J�i */
							/*!< 0x8340<=c<=0x8396 �S�p�J�^�J�i */
#define	CK_MBC_GIRI		18	/*!< 2�o�C�g�̃M���V������ */
							/*!< 0x839F<=c<=0x83D6 �S�p�M���V������ */
#define	CK_MBC_ROS		19	/*!< 2�o�C�g�̃��V�A����: */
							/*!< 0x8440<=c<=0x8460 �S�p���V�A�����啶�� */
							/*!< 0x8470<=c<=0x8491 �S�p���V�A���������� */
#define	CK_MBC_SKIGO	20	/*!< 2�o�C�g�̓���L�� */
							/*!< 0x849F<=c<=0x879C �S�p����L�� */
#define	CK_MBC_ETC		21	/*!< 2�o�C�g�̂��̑��i�����Ȃǁj */





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
	m_nLines = 0;
	m_nPrevReferLine = 0;
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





const char* CDocLineMgr::GetLineStr( int nLine, int* pnLineLen )
{
	CDocLine* pDocLine;
	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		*pnLineLen = 0;
		return NULL;
	}
	// 2002/2/10 aroka CMemory �̃����o�ϐ��ɒ��ڃA�N�Z�X���Ȃ�(inline������Ă���̂ő��x�I�Ȗ��͂Ȃ�)
	return pDocLine->m_pLine->GetPtr( pnLineLen );
//	*pnLineLen = pDocLine->m_pLine->m_nDataLen;
//	return pDocLine->m_pLine->m_pData;
}

/*!
	�w�肳�ꂽ�ԍ��̍s�ւ̃|�C���^��Ԃ�

	@param nLine [in] �s�ԍ�
	@return �s�I�u�W�F�N�g�ւ̃|�C���^�B�Y���s���Ȃ��ꍇ��NULL�B
*/
CDocLine* CDocLineMgr::GetLineInfo( int nLine )
{
	int nCounter;
	CDocLine* pDocLine;
	if( 0 == m_nLines ){
		return NULL;
	}
	if( nLine >= m_nLines ){
		return NULL;
	}
	if( m_pCodePrevRefer == NULL ){
		MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::GetLineInfo() 	m_pCodePrevRefer == NULL" );





#if 0 /////////	1999.12.22
		nCounter = 0;
		pDocLine = m_pDocLineTop;
		do{
			if( nLine == nCounter ){
				m_nPrevReferLine = nLine;
				m_pCodePrevRefer = pDocLine;
				m_pDocLineCurrent = pDocLine->m_pNext;
				return pDocLine;
			}
			pDocLine = pDocLine->m_pNext;
			++nCounter;
		}while( NULL != pDocLine );
#endif ///////////////
		if( nLine < (m_nLines / 2) ){
			nCounter = 0;
			pDocLine = m_pDocLineTop;
			while( NULL != pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->m_pNext;
					return pDocLine;
				}
				pDocLine = pDocLine->m_pNext;
				nCounter++;
			}
		}else{
			nCounter = m_nLines - 1;
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

	}else{
		if( nLine == m_nPrevReferLine ){
			m_nPrevReferLine = nLine;
			m_pDocLineCurrent = m_pCodePrevRefer->m_pNext;
			return m_pCodePrevRefer;
		}else
		if( nLine > m_nPrevReferLine ){
			nCounter = m_nPrevReferLine + 1;
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
		}else{
			nCounter = m_nPrevReferLine - 1;
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
const char* CDocLineMgr::GetFirstLinrStr( int* pnLineLen )
{
	char* pszLine;
	if( 0 == m_nLines ){
		pszLine = NULL;
		*pnLineLen = 0;
	}else{
		pszLine = m_pDocLineTop->m_pLine->GetPtr( pnLineLen );

		m_pDocLineCurrent = m_pDocLineTop->m_pNext;
	}
	return (const char*)pszLine;
}





/*!
	���A�N�Z�X���[�h�F���̍s�𓾂�

	@param pnLineLen [out] �s�̒������Ԃ�B
	@return ���s�̐擪�ւ̃|�C���^�B
	GetFirstLinrStr()���Ăяo����Ă��Ȃ���NULL���Ԃ�

*/
const char* CDocLineMgr::GetNextLinrStr( int* pnLineLen )
{
	char* pszLine;
	if( NULL == m_pDocLineCurrent ){
		pszLine = NULL;
		*pnLineLen = 0;
	}else{
		pszLine = m_pDocLineCurrent->m_pLine->GetPtr( pnLineLen );

		m_pDocLineCurrent = m_pDocLineCurrent->m_pNext;
	}
	return (const char*)pszLine;
}




#if 0
	/* �����ɍs��ǉ� Ver0 */
	void CDocLineMgr::AddLineStrSz( const char* pszStr )
	{
	#ifdef _DEBUG
		CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::AddLineStrSz" );
	#endif
		CDocLine* pDocLine;
		if( 0 == m_nLines ){
			m_pDocLineBot = m_pDocLineTop = new CDocLine;
			m_pDocLineTop->m_pPrev = NULL;
			m_pDocLineTop->m_pNext = NULL;
			m_pDocLineTop->m_pLine = new CMemory( pszStr, lstrlen( pszStr ) );
		}else{
			pDocLine = new CDocLine;
			pDocLine->m_pPrev = m_pDocLineBot;
			pDocLine->m_pNext = NULL;
			pDocLine->m_pLine = new CMemory( pszStr, lstrlen( pszStr ) );
			m_pDocLineBot->m_pNext = pDocLine;
			m_pDocLineBot = pDocLine;
		}
		++m_nLines;
	}
	/* �����ɍs��ǉ� Ver1 */
	void CDocLineMgr::AddLineStr( const char* pData, int nDataLen )
	{
	#ifdef _DEBUG
		CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::AddLineStr(const char*, int)" );
	#endif
		CDocLine* pDocLine;
		if( 0 == m_nLines ){
			m_pDocLineBot = m_pDocLineTop = new CDocLine;
			m_pDocLineTop->m_pPrev = NULL;
			m_pDocLineTop->m_pNext = NULL;
			m_pDocLineTop->m_pLine = new CMemory( pData, nDataLen );
		}else{
			pDocLine = new CDocLine;
			pDocLine->m_pPrev = m_pDocLineBot;
			pDocLine->m_pNext = NULL;
			pDocLine->m_pLine = new CMemory( pData, nDataLen );
			m_pDocLineBot->m_pNext = pDocLine;
			m_pDocLineBot = pDocLine;
		}
		++m_nLines;
	}
	/* �����ɍs��ǉ� Ver2 */
	void CDocLineMgr::AddLineStr( CMemory& cmemData )
	{
	//#ifdef _DEBUG
	//	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::AddLineStr(CMemory&)" );
	//#endif
		char*	pData;
		int		nDataLen;
		pData = cmemData.GetPtr( &nDataLen );
	//	AddLineStr( pData, nDataLen );

		CDocLine* pDocLine;
		if( 0 == m_nLines ){
			m_pDocLineBot = m_pDocLineTop = new CDocLine;
			m_pDocLineTop->m_pPrev = NULL;
			m_pDocLineTop->m_pNext = NULL;
			m_pDocLineTop->m_pLine = new CMemory( pData, nDataLen );
		}else{
			pDocLine = new CDocLine;
			pDocLine->m_pPrev = m_pDocLineBot;
			pDocLine->m_pNext = NULL;
			pDocLine->m_pLine = new CMemory( pData, nDataLen );
			m_pDocLineBot->m_pNext = pDocLine;
			m_pDocLineBot = pDocLine;
		}
		++m_nLines;
		return;
	}
#endif

/*!
	�����ɍs��ǉ�

	@version 1.5

	@param pData [in] �ǉ����镶����ւ̃|�C���^
	@param nDataLen [in] ������̒���
	@param cEol [in] �s���R�[�h

*/
void CDocLineMgr::AddLineStrX( const char* pData, int nDataLen, CEOL cEol )
{
#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::AddLineStrX(const char*, int, int)" );
#endif
	CDocLine* pDocLine;
	if( 0 == m_nLines ){
		m_pDocLineBot = m_pDocLineTop = new CDocLine;
		m_pDocLineTop->m_pPrev = NULL;
		m_pDocLineTop->m_pNext = NULL;
//		if( bCRLF ){

//		if( EOL_NONE != nEOLType ){
//			m_pDocLineTop->m_pLine->m_pData[nDataLen] = '\r';
//			m_pDocLineTop->m_pLine->m_pData[nDataLen + 1] = '\0';
//			++m_pDocLineTop->m_pLine->m_nDataLen;
//		}
		m_pDocLineTop->m_cEol = cEol;	/* ���s�R�[�h�̎�� */
		m_pDocLineTop->m_pLine = new CMemory( pData, nDataLen/* - m_pDocLineTop->m_nEOLLen*/ );
	}else{
		pDocLine = new CDocLine;
		pDocLine->m_pPrev = m_pDocLineBot;
		pDocLine->m_pNext = NULL;
//		if( bCRLF ){

//		if( EOL_NONE != nEOLType ){
//			pDocLine->m_pLine->m_pData[nDataLen] = '\r';
//			pDocLine->m_pLine->m_pData[nDataLen + 1] = '\0';
//			++pDocLine->m_pLine->m_nDataLen;
//		}
		pDocLine->m_cEol = cEol;	/* ���s�R�[�h�̎�� */
		pDocLine->m_pLine = new CMemory( pData, nDataLen/* - pDocLine->m_nEOLLen*/ );
		m_pDocLineBot->m_pNext = pDocLine;
		m_pDocLineBot = pDocLine;
	}
	++m_nLines;
	return;
}

/*!
	�t�@�C����ǂݍ���Ŋi�[����i�����ǂݍ��݃e�X�g�Łj
	@version	2.0
	@note	Windows�p�ɃR�[�f�B���O���Ă���
	@param	nFlags
		bit 0: MIME Encode���ꂽ�w�b�_��decode���邩�ǂ���
	@retval	TRUE	����ǂݍ���
	@retval	FALSE	�G���[(�܂��̓��[�U�ɂ��L�����Z��?)
	@date	2002/08/30 Moca ��ReadFile�����ɍ쐬 �t�@�C���A�N�Z�X�Ɋւ��镔����CFileLoad�ōs��
*/
int CDocLineMgr::ReadFile( const char* pszPath, HWND hWndParent, HWND hwndProgress, int nCharCode, FILETIME* pFileTime, int nFlags )
{
#ifdef _DEBUG
	MYTRACE( "pszPath=[%s]\n", pszPath );
	MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::ReadFile" );
#endif
	int			nRetVal = TRUE;
	int			nLineNum = 0;
	//	May 15, 2000 genta
	CEOL cEol;
	CFileLoad cfl; 	//	2002/08/30 Moca Add
	const char*	pLine;
	int			nLineLen;

	/* �����f�[�^�̃N���A */
	Empty();
	Init();

	/* �������̃��[�U�[������\�ɂ��� */
	if( !::BlockingHook( NULL ) ){
		return FALSE;
	}

	try{
	// �t�@�C�����J��
	// �t�@�C�������ɂ�FileClose�����o���̓f�X�g���N�^�̂ǂ��炩�ŏ����ł��܂�
	cfl.FileOpen( pszPath, nCharCode, nFlags );

	/* �t�@�C�������̎擾 */
	FILETIME	FileTime;
	if( TRUE == cfl.GetFileTime( NULL, NULL, &FileTime ) ){
		*pFileTime = FileTime;
//		SYSTEMTIME	systimeL;
//		::FileTimeToLocalFileTime( &FileTime, &FileTime );
//		::FileTimeToSystemTime( &FileTime, &systimeL );
//		MYTRACE( "Last Update: %d/%d/%d %02d:%02d:%02d\n",
//			systimeL.wYear,
//			systimeL.wMonth,
//			systimeL.wDay,
//			systimeL.wHour,
//			systimeL.wMinute,
//			systimeL.wSecond
//		);
	}else{
//		MYTRACE( "GetFileTime() error.\n" );
	}

	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
	}

	// ReadLine�̓t�@�C������ �����R�[�h�ϊ����ꂽ1�s��ǂݏo���܂�
	// �G���[����throw CError_FileRead �𓊂��܂�
	while( NULL != ( pLine = cfl.ReadLine( &nLineLen, &cEol ) ) ){
		++nLineNum;
		AddLineStrX( pLine, nLineLen, cEol );
		if( NULL != hwndProgress && 0 == ( nLineNum % 512 ) ){
			::PostMessage( hwndProgress, PBM_SETPOS, cfl.GetPercent(), 0 );
			/* �������̃��[�U�[������\�ɂ��� */
			if( !::BlockingHook( NULL ) ){
				return FALSE;
			}
		}
	}

	// �t�@�C�����N���[�Y����
	cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		nRetVal = FALSE;
		if( -1 == _access( pszPath, 0 )){
			// �t�@�C�����Ȃ�
			::MYMESSAGEBOX(
				hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("%s\n�Ƃ����t�@�C�����J���܂���B\n�t�@�C�������݂��܂���B"),	//Mar. 24, 2001 jepro �኱�C��
				pszPath
			 );
		}
		else if( -1 == _access( pszPath, 4 )){
			// �ǂݍ��݃A�N�Z�X�����Ȃ�
			::MYMESSAGEBOX(
				hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("\'%s\'\n�Ƃ����t�@�C�����J���܂���B\n�ǂݍ��݃A�N�Z�X��������܂���B"),
				pszPath
			 );
		}
//		else if( �t�@�C���T�C�Y > 2GB ){
//			nRetVal = FALSE;
//			::MYMESSAGEBOX(
//				hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//				_("\'%s\'\n�Ƃ����t�@�C�����J���܂���B\n2GB�ȏ�̃t�@�C���T�C�Y�͊J���܂���B"),
//				pszPath
//			 );
//		}
		else{
			::MYMESSAGEBOX(
				hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("\'%s\'\n�Ƃ����t�@�C�����J���܂���B\n���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B"),
				pszPath
			 );
		}
	}
	catch( CError_FileRead ){
		nRetVal = FALSE;
		::MYMESSAGEBOX(
			hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			_T("\'%s\'�Ƃ����t�@�C���̓ǂݍ��ݒ��ɃG���[���������܂����B\n�t�@�C���̓ǂݍ��݂𒆎~���܂��B"),
			pszPath
		 );
		/* �����f�[�^�̃N���A */
		Empty();
		Init();
		nRetVal = FALSE;
	} // ��O�����I���

	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
	}
	/* �������̃��[�U�[������\�ɂ��� */
	if( !::BlockingHook( NULL ) ){
		return FALSE;
	}

	/* �s�ύX��Ԃ����ׂă��Z�b�g */
	ResetAllModifyFlag();
	return nRetVal;
}




/* �o�b�t�@���e���t�@�C���ɏ����o�� (�e�X�g�p) */
/* (����) Windows�p�ɃR�[�f�B���O���Ă��� */
int CDocLineMgr::WriteFile( const char* pszPath, HWND hWndParent, HWND hwndProgress, int nCharCode, FILETIME* pFileTime, CEOL cEol )
{
	const char*		pLine;
//	char*			pLineUnicode;
	int				nLineLen;
	int				nRetVal;
	int				nLineNumber;
//	int				i;
	int				nWriteLen;
//	int				bCRLF;
//	int				nLineUnicodeLen;
//	char*			pszCRLF_UNICODE = "\x0d\x0\x0a\x0";
//	char*			pszCRLF_LF = "\x0a";
	CMemory			cmemBuf;
	CDocLine*		pCDocLine;

	/*
	|| �o�b�t�@�T�C�Y�̒���
	*/
	cmemBuf.AllocBuffer( 32000 );

//#ifdef _DEBUG
//		if( IDYES != MYMESSAGEBOX(
//			hWndParent,
//			MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
//			"_DEBUG �e�X�g�@�\",
//			"���s�R�[�h�� LFCR �ɂ��ĕۑ����܂��B\n��낵���ł����H"
//		) ){
//			return FALSE;
//		}
//#endif
//	int				nCharCode = CODE_SJIS;
//	int				nCharCode = CODE_EUC;
//	int				nCharCode = CODE_JIS;
//	int				nCharCode = CODE_UNICODE;


	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
	}

	nRetVal = TRUE;

// 2002/05/22 Frozen�@��������폜�iCFileWrite_C�̃R���X�g���N�^�ֈړ��j----
// CFileWrite�N���X�ő�p
//
//	//<< 2002/04/13 Azumaiya
//	//  WriteFile �𒼂Ɏg���Ă��܂��ƁA�������ݑ��x�����I�ɒx���Ȃ�̂ŁA��͂�A
//	// WriteFile �����b�s���O���Ă���W���֐��ɖ߂��B
//	// �t�@�C���������擾����B
//	DWORD dwFileAttribute;
//	dwFileAttribute = ::GetFileAttributes(pszPath);
//	if ( dwFileAttribute != (DWORD)-1 )
//	{
//		// �ǎ��p���������c��(�m�[�}���������t���Ă����炻����c��)�B
//		BOOL bRes = ::SetFileAttributes(pszPath, dwFileAttribute & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_NORMAL));
//	}
//	else
//	{
//		dwFileAttribute = FILE_ATTRIBUTE_NORMAL;	//@@@ 2002.04.15 MIK
//	}
//	//>> 2002/04/13 Azumaiya
//
//
//
//	///* �t�@�C�����������ݗp�ɃI�[�v������ */
//// Oct 6, 2000 ao
///* �t�@�C���o�͂�stream ���g���悤�ɂ��� */
//	// ���s�R�[�h������ɐ��䂳��Ȃ��l�A�o�C�i�����[�h�ŊJ��
//	FILE *sFile= fopen(pszPath,"wb"); /*add*/
//
//	2002/05/22 Frozen �����܂ō폜-----------------------------

	try
	{
	{//���̊��ʂɑΉ���������ʂ�file�̃f�X�g���N�^���Ăяo����A�t�@�C���������܂��B 
		CFileWrite file(pszPath);// 2002/05/22 Frozen

	///* �t�@�C�����������ݗp�ɃI�[�v������ */
//-	hFile = _lopen( pszPath, OF_WRITE );
//-	if( HFILE_ERROR == hFile ){
//-	}else{
//-		_lclose( hFile );
//-	}
//-	hFile = _lcreat(pszPath, 0);
//-	if( HFILE_ERROR == hFile ){



//	2002/05/22 Frozen �t�@�C���̃I�[�v���Ɏ��s�����ꍇ�͗�O���N����悤�ɂ������߂�������폜----
//	����ɂ����ɂ�����Ă��镶�̈ꕔ��catch�߂ֈړ�
//
//	if( !sFile ){ /*add*/
////		MYTRACE( "file create error %s\n", pszPath );
//		::MYMESSAGEBOX(
//			hWndParent,
//			MB_OK | MB_ICONSTOP,
//			GSTR_APPNAME,
//			"\'%s\'\n�t�@�C����ۑ��ł��܂���B\n�p�X�����݂��Ȃ����A���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B",
//			pszPath
//		 );
//		nRetVal = FALSE;
//		//<< 2002/04/13 Azumaiya
//		if ( dwFileAttribute != (DWORD)-1 )
//		{
//			// �t�@�C�����������ɖ߂��B
//			::SetFileAttributes(pszPath, dwFileAttribute);
//		}
//		//>> 2002/04/13 Azumaiya
//		goto _RETURN_;
//	}
//
//�@2002/05/22 Frozen �����܂ō폜

	switch( nCharCode ){
	case CODE_UNICODE:
		file.Write("\xff\xfe",sizeof(char)*2);// 2002/05/22 break�̎���}�܂ł����̈�s�Œu������
////-		if( HFILE_ERROR == _lwrite( hFile, "\xff\xfe", 2 ) ){
//		if( fwrite( "\xff\xfe", sizeof( char ), 2, sFile ) < 2 ){ /* add */
////			MYTRACE( "file write error %s\n", pszPath );
//			nRetVal = FALSE;
//			goto _CLOSEFILE_;
//		}
		break;
	case CODE_UNICODEBE:
		file.Write( "\xfe\xff", sizeof(char) * 2 );
		break;
//	case CODE_UTF8: // 2003.05.04 Moca BOM�̊ԈႢ�����
//		file.Write( "\xfe\xbb\xbf", sizeof(char) * 3 );
//		break;
	}

	nLineNumber = 0;
//	pLine = GetFirstLinrStr( &nLineLen );
	pCDocLine = m_pDocLineTop;

//	while( NULL != pLine ){
	while( NULL != pCDocLine ){
		++nLineNumber;
		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );


		if( NULL != hwndProgress && 0 < m_nLines && 0 == ( nLineNumber % 1024 ) ){
			::PostMessage( hwndProgress, PBM_SETPOS, nLineNumber * 100 / m_nLines , 0 );
			/* �������̃��[�U�[������\�ɂ��� */
			if( !::BlockingHook( NULL ) ){
				return FALSE;
			}
		}

		nWriteLen = nLineLen - pCDocLine->m_cEol.GetLen();
		cmemBuf.SetDataSz( "" );
		if( 0 < nWriteLen ){
			cmemBuf.SetData( pLine, nWriteLen );

			/* �������ݎ��̃R�[�h�ϊ� */
			switch( nCharCode ){
			case CODE_UNICODE:
				/* SJIS��Unicode�R�[�h�ϊ� */
				cmemBuf.SJISToUnicode();
				break;
			case CODE_UTF8:	/* UTF-8 */
				/* SJIS��UTF-8�R�[�h�ϊ� */
				cmemBuf.SJISToUTF8();
				break;
			case CODE_UTF7:	/* UTF-7 */
				/* SJIS��UTF-7�R�[�h�ϊ� */
				cmemBuf.SJISToUTF7();
				break;
			case CODE_EUC:
				/* SJIS��EUC�R�[�h�ϊ� */
				cmemBuf.SJISToEUC();
				break;
			case CODE_JIS:
				/* SJIS��JIS�R�[�h�ϊ� */
				cmemBuf.SJIStoJIS();
				break;
			case CODE_UNICODEBE:
				/* SJIS��UnicodeBE�R�[�h�ϊ� */
				cmemBuf.SJISToUnicodeBE();
				break;
			case CODE_SJIS:
			default:
				break;
			}
		}
//		if( bCRLF ){
		if( EOL_NONE != pCDocLine->m_cEol ){

// 2002/05/09 Frozen ��������
			if( nCharCode == CODE_UNICODE ){
				if( cEol==EOL_NONE )
					cmemBuf.Append( pCDocLine->m_cEol.GetUnicodeValue(), pCDocLine->m_cEol.GetLen()*sizeof(wchar_t));
				else
					cmemBuf.Append( cEol.GetUnicodeValue(), cEol.GetLen()*sizeof(wchar_t));
			}else if( nCharCode == CODE_UNICODEBE ){
				/* UnicodeBE �̉��s�R�[�h�ݒ� Moca, 2002/05/26 */
				if( cEol == EOL_NONE ) /*  */
					cmemBuf.Append( pCDocLine->m_cEol.GetUnicodeBEValue(), pCDocLine->m_cEol.GetLen()*sizeof(wchar_t) );
				else
					cmemBuf.Append( cEol.GetUnicodeBEValue(), cEol.GetLen()*sizeof(wchar_t) );
			}else{
				if( cEol == EOL_NONE )
					cmemBuf.Append(pCDocLine->m_cEol.GetValue(),pCDocLine->m_cEol.GetLen());
				else
					cmemBuf.Append(cEol.GetValue(),cEol.GetLen());
			}
// 2002/05/09 Frozen �����܂�

// 2002/05/09 Frozen ��������폜

//			/* �������ݎ��̉��s�R�[�h�ϊ� */
//			switch( nCharCode ){
//			case CODE_UNICODE:
//				/* ASCII+SJIS��Unicode�ϊ� */
//				//	May 15, 2000 genta
//				{
//					CEOL ue( EOL_CRLF_UNICODE );
//					cmemBuf.Append( ue.GetValue(), ue.GetLen() );
//				}
//				break;
//// 1999.12.20
////			case CODE_EUC:
////				cmemBuf.Append( gm_pszEolDataArr[EOL_LF], LEN_EOL_LF );
////				break;
//			default:
//				//	From Here Feb. 8, 2001 genta ���s�R�[�h�ϊ�������ǉ�
//				if( cEol == EOL_NONE ){
//// 1999.12.20
////				/* ���s�R�[�h��CRLF�ɕϊ� */
////				cmemBuf.Append( gm_pszEolDataArr[EOL_CRLF], LEN_EOL_CRLF );
//					/* ���s�R�[�h��ϊ����Ȃ� */
//					cmemBuf.Append( pCDocLine->m_cEol.GetValue(), pCDocLine->m_cEol.GetLen() );
//					break;
//				}
//				else {
//					/* ���s�R�[�h���w�肳�ꂽ���̂ɕϊ� */
//					cmemBuf.Append( cEol.GetValue(), cEol.GetLen() );
//				}
//				//	To Here Feb. 8, 2001 genta
//			}
// 2002/05/09 Frozen �����܂ō폜

		}
		if( 0 < cmemBuf.GetLength() )//{
			file.Write(cmemBuf.GetPtr(),sizeof(char)*cmemBuf.GetLength());//2002/05/22 Frozen goto�̎���}�܂ł����̈�s�Œu������
////-			if( HFILE_ERROR == _lwrite( hFile, cmemBuf.GetPtr(), cmemBuf.GetLength() ) ){
//			if( fwrite( cmemBuf.GetPtr(), sizeof( char ), cmemBuf. GetLength(), sFile ) /* add */
//					< (size_t)cmemBuf.GetLength() ){ /* add */
////				MYTRACE( "file write error %s\n", pszPath );
//				nRetVal = FALSE;
//				goto _CLOSEFILE_;
//			}
//		}


//		pLine = GetNextLinrStr( &nLineLen );
		pCDocLine = pCDocLine->m_pNext;

	}
	}//���̊��ʂ�CFileWrite�̃f�X�g���N�^���Ăяo����A�t�@�C���������܂��B

// 2002/05/22 Frozen ��������폜�i���l�̏�����CFileWrite�̃f�X�g���N�^�ő���ɍs���Ă��܂��j------
//
//_CLOSEFILE_:;
////-	_lclose( hFile );
//	fflush( sFile );/* add */
//	fclose( sFile );/* add */
//	//<< 2002/04/13 Azumaiya
//	// �t�@�C�����������ɖ߂��B
//	::SetFileAttributes(pszPath, dwFileAttribute);
//	//>> 2002/04/13 Azumaiya
//
// Oct 6, 2000 ao end
/* �t�@�C���o�͂Ɋւ���ύX�͂����܂ŁB
	���̌�ύX��̃t�@�C�������J�����߂Ƀt�@�C���A�N�Z�X���Ă��邪�A�����܂Ŗ����ɕύX����K�v�͂Ȃ��ł��傤�B*/

// 2002/05/22 Frozen�@�����܂ō폜---------------------------------------------------------


	/* �X�V��̃t�@�C�������̎擾
	 * CloseHandle�O�ł�FlushFileBuffers���Ă�ł��^�C���X�^���v���X�V
	 * ����Ȃ����Ƃ�����B���������Ȃ��̂ł�������N���[�Y���čăI�[�v
	 * �����Ď������擾����B
	 */
	DWORD dwFileAttribute = ::GetFileAttributes(pszPath);
	if ( dwFileAttribute == (DWORD)-1 )
	{
		dwFileAttribute = FILE_ATTRIBUTE_NORMAL;
	}
	HANDLE hFile;
	hFile = ::CreateFile(
						pszPath,			// �J���t�@�C����
						GENERIC_READ,		// �ǂݍ��݃��[�h�ŊJ���B
						0,					// ���L���Ȃ��B
						NULL,				// �n���h�����p�����Ȃ��B
						OPEN_EXISTING,		// ���݂���t�@�C�����J���B
						dwFileAttribute,	// �t�@�C�������B
						NULL				// �e���v���[�g�t�@�C�����g��Ȃ��B
						);
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		::GetFileTime( (HANDLE)hFile, NULL, NULL, pFileTime );
		::CloseHandle(hFile);
	}

	}
	catch(CError_FileOpen)
	{
		::MYMESSAGEBOX(
			hWndParent,
			MB_OK | MB_ICONSTOP,
			GSTR_APPNAME,
			"\'%s\'\n�t�@�C����ۑ��ł��܂���B\n�p�X�����݂��Ȃ����A���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B",
			pszPath);
		nRetVal = FALSE;
	}
	catch(CError_FileWrite)
	{
		nRetVal = FALSE;
	}

//_RETURN_:; 2002/05/22 Frozen �폜�i��O�������g�p����̂�goto�p�̃��x���͎g�p���Ȃ��j
	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( NULL ) ){
			return FALSE;
		}
	}

	/* �s�ύX��Ԃ����ׂă��Z�b�g */
//	if ( nRetVal ) ResetAllModifyFlag();	// �����łނ��傤����Ƀ��Z�b�g���Ă͂����܂���B2002/04/28 YAZAKI

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
			int			nLine,
			int			nDelPos,
			int			nDelLen,
			int*		pnModLineOldFrom,	/* �e���̂������ύX�O�̍s(from) */
			int*		pnModLineOldTo,		/* �e���̂������ύX�O�̍s(to) */
			int*		pnDelLineOldFrom,	/* �폜���ꂽ�ύX�O�_���s(from) */
			int*		pnDelLineOldNum,	/* �폜���ꂽ�s�� */
			CMemory&	cmemDeleted			/* �폜���ꂽ�f�[�^ */
//			int			bUndo				/* Undo���삩�ǂ��� */
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::DeleteData" );
#endif
	CDocLine*	pDocLine;
	CDocLine*	pDocLine2;
	char*		pData;
	int			nDeleteLength;
	char*		pLine;
	int			nLineLen;
	char*		pLine2;
	int			nLineLen2;
	*pnModLineOldFrom = nLine;	/* �e���̂������ύX�O�̍s(from) */
	*pnModLineOldTo = nLine;	/* �e���̂������ύX�O�̍s(to) */
	*pnDelLineOldFrom = 0;		/* �폜���ꂽ�ύX�O�_���s(from) */
	*pnDelLineOldNum = 0;		/* �폜���ꂽ�s�� */
//	cmemDeleted.SetData( "", lstrlen( "" ) );
	cmemDeleted.SetDataSz( "" );

	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		return;
	}

	pDocLine->SetModifyFlg(true);		/* �ύX�t���O */

	pLine = pDocLine->m_pLine->GetPtr( &nLineLen );

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
		cmemDeleted.SetData( &pLine[nDelPos], nDeleteLength );

		/* ���̍s�̏�� */
		if( NULL == ( pDocLine2 = pDocLine->m_pNext ) ){
			pData = new char[nLineLen + 1];
			if( nDelPos > 0 ){
				memcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				memcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			pData[ nLineLen - nDeleteLength ] = '\0';
			/* ���s�R�[�h�̏����X�V */
			pDocLine->m_cEol.SetType( EOL_NONE );

			if( 0 < nLineLen - nDeleteLength ){
				pDocLine->m_pLine->SetData( pData, nLineLen - nDeleteLength );
			}else{
				/* �s�̍폜 */
				if( NULL != pDocLine->m_pPrev ){
					pDocLine->m_pPrev->m_pNext = NULL;
				}
				m_pDocLineBot = pDocLine->m_pPrev;
				*pnDelLineOldFrom = nLine;	/* �폜���ꂽ�ύX�O�_���s(from) */
				*pnDelLineOldNum = 1;		/* �폜���ꂽ�s�� */
				m_nLines--;					/* �S�s�� */
				if( 0 == m_nLines ){
					/* �f�[�^���Ȃ��Ȃ��� */
					Init();
				}
			}
			delete [] pData;
		}else{
			*pnModLineOldTo = nLine + 1;	/* �e���̂������ύX�O�̍s(to) */
			pLine2 = pDocLine2->m_pLine->GetPtr( &nLineLen2 );
			pData = new char[nLineLen + nLineLen2 + 1];
			if( nDelPos > 0 ){
				memcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				memcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			/* ���̍s�̃f�[�^��A�� */
			memcpy( pData + (nLineLen - nDeleteLength), pLine2, nLineLen2 );
			pData[ nLineLen - nDeleteLength + nLineLen2 ] = '\0';
			pDocLine->m_pLine->SetData( pData, nLineLen - nDeleteLength + nLineLen2 );
			/* ���s�R�[�h�̏����X�V */
			pDocLine->m_cEol = pDocLine2->m_cEol;

			/* ���̍s���폜 && �����s�Ƃ̃��X�g�̘A��*/
			pDocLine->m_pNext = pDocLine2->m_pNext;
			if( NULL != pDocLine->m_pNext ){
				pDocLine->m_pNext->m_pPrev = pDocLine;
			}else{
				m_pDocLineBot = pDocLine;
			}
			delete pDocLine2;
			*pnDelLineOldFrom = nLine + 1;	/* �폜���ꂽ�ύX�O�_���s(from) */
			*pnDelLineOldNum = 1;			/* �폜���ꂽ�s�� */
			m_nLines--;						/* �S�s�� */
			if( 0 == m_nLines ){
				/* �f�[�^���Ȃ��Ȃ��� */
				Init();
			}
			delete [] pData;
		}
	}else{
		/* ���ۂɍ폜����o�C�g�� */
		nDeleteLength = nDelLen;

		/* �폜�����f�[�^ */
		cmemDeleted.SetData( &pLine[nDelPos], nDeleteLength );

		pData = new char[nLineLen + 1];
		if( nDelPos > 0 ){
			memcpy( pData, pLine, nDelPos );
		}
		if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
			memcpy(
				pData + nDelPos,
				pLine + nDelPos + nDeleteLength,
				nLineLen - ( nDelPos + nDeleteLength )
			);
		}
		pData[ nLineLen - nDeleteLength ] = '\0';
		if( 0 < nLineLen - nDeleteLength ){
			pDocLine->m_pLine->SetData( pData, nLineLen - nDeleteLength );
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
			int			nLine,
			int			nInsPos,
			const char*	pInsData,
			int			nInsDataLen,
			int*		pnInsLineNum,	/* �}���ɂ���đ������s�̐� */
			int*		pnNewLine,		/* �}�����ꂽ�����̎��̈ʒu�̍s */
			int*		pnNewPos		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
//			int			bUndo			/* Undo���삩�ǂ��� */
)
{
	CDocLine*	pDocLine;
	CDocLine*	pDocLineNew;
	char*		pLine;
	int			nLineLen;
	int			nBgn;
	int			nPos;
	CMemory		cmemPrevLine;
	CMemory		cmemCurLine;
	CMemory		cmemNextLine;
	int			nAllLinesOld = m_nLines;
	int			nCount;

	//	May 15, 2000 genta
	CEOL 		cEOLType;
	CEOL 		cEOLTypeNext;
	//enumEOLType nEOLType;
	//enumEOLType nEOLTypeNext;

	bool		bBookMarkNext;	// 2001.12.03 hor �}���ɂ��}�[�N�s�̐���

	*pnNewLine = nLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
	*pnNewPos  = 0;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */

	/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
	nCount = 0;
	*pnInsLineNum = 0;
	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		/* ������NULL���A���Ă���Ƃ������Ƃ́A*/
		/* �S�e�L�X�g�̍Ō�̎��̍s��ǉ����悤�Ƃ��Ă��邱�Ƃ����� */
		cmemPrevLine.SetDataSz( "" );
		cmemNextLine.SetDataSz( "" );
		cEOLTypeNext.SetType( EOL_NONE );
		bBookMarkNext=false;	// 2001.12.03 hor
	}else{
		pDocLine->SetModifyFlg(true);		/* �ύX�t���O */

		pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
		cmemPrevLine.SetData( pLine, nInsPos );
		cmemNextLine.SetData( &pLine[nInsPos], nLineLen - nInsPos );

		cEOLTypeNext = pDocLine->m_cEol;
		bBookMarkNext= pDocLine->IsBookMarked();	// 2001.12.03 hor
	}
	nBgn = 0;
	nPos = 0;
	for( nPos = 0; nPos < nInsDataLen; ){
		if( pInsData[nPos] == '\n' || pInsData[nPos] == '\r' ){
			/* �s�I�[�q�̎�ނ𒲂ׂ� */
			cEOLType.GetTypeFromString( &pInsData[nPos], nInsDataLen - nPos );
			/* �s�I�[�q���܂߂ăe�L�X�g���o�b�t�@�Ɋi�[ */
			cmemCurLine.SetData( &pInsData[nBgn], nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + cEOLType.GetLen();
			nPos = nBgn;
			if( NULL == pDocLine ){
				pDocLineNew = new CDocLine;

				pDocLineNew->m_pLine = new CMemory;
				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					if( NULL == m_pDocLineTop ){
						m_pDocLineTop = pDocLineNew;
					}
					pDocLineNew->m_pPrev = m_pDocLineBot;
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pDocLineNew;
					}
					m_pDocLineBot = pDocLineNew;
					pDocLineNew->m_pNext = NULL;
					pDocLineNew->m_pLine->SetData( &cmemPrevLine );
					*(pDocLineNew->m_pLine) += cmemCurLine;

					pDocLineNew->m_cEol = cEOLType;							/* ���s�R�[�h�̎�� */
					// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLType];	/* ���s�R�[�h�̒��� */
				}else{
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pDocLineNew;
					}
					pDocLineNew->m_pPrev = m_pDocLineBot;
					m_pDocLineBot = pDocLineNew;
					pDocLineNew->m_pNext = NULL;
					pDocLineNew->m_pLine->SetData( &cmemCurLine );

					pDocLineNew->m_cEol = cEOLType;							/* ���s�R�[�h�̎�� */
					// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLType];	/* ���s�R�[�h�̒��� */
				}
				pDocLine = NULL;
				++m_nLines;
			}else{
				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					pDocLine->m_pLine->SetData( &cmemPrevLine );
					*(pDocLine->m_pLine) += cmemCurLine;

					pDocLine->m_cEol = cEOLType;						/* ���s�R�[�h�̎�� */
					// pDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLType];	/* ���s�R�[�h�̒��� */

					// 2001.12.13 hor
					// �s���ŉ��s�����猳�̍s�̃}�[�N��V�����s�Ɉړ�����
					// ����ȊO�Ȃ猳�̍s�̃}�[�N���ێ����ĐV�����s�ɂ̓}�[�N��t���Ȃ�
					if(nInsPos==0){
						pDocLine->SetBookMark(false);
					}else{
						bBookMarkNext=false;
					}

					pDocLine = pDocLine->m_pNext;
				}else{
					pDocLineNew = new CDocLine;
					pDocLineNew->m_pLine = new CMemory;
					pDocLineNew->m_pPrev = pDocLine->m_pPrev;
					pDocLineNew->m_pNext = pDocLine;
					pDocLine->m_pPrev->m_pNext = pDocLineNew;
					pDocLine->m_pPrev = pDocLineNew;
					pDocLineNew->m_pLine->SetData( &cmemCurLine );

					pDocLineNew->m_cEol = cEOLType;							/* ���s�R�[�h�̎�� */
					// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLType];	/* ���s�R�[�h�̒��� */

					++m_nLines;
				}
			}

			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			++nCount;
			++(*pnNewLine);	/* �}�����ꂽ�����̎��̈ʒu�̍s */
		}else{
			++nPos;
		}
	}
//	nEOLType = EOL_NONE;
	if( 0 < nPos - nBgn || 0 < cmemNextLine.GetLength() ){
		cmemCurLine.SetData( &pInsData[nBgn], nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pDocLine ){
			pDocLineNew = new CDocLine;
			pDocLineNew->m_pLine = new CMemory;
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				if( NULL == m_pDocLineTop ){
					m_pDocLineTop = pDocLineNew;
				}
				pDocLineNew->m_pPrev = m_pDocLineBot;
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pDocLineNew;
				}
				m_pDocLineBot = pDocLineNew;
				pDocLineNew->m_pNext = NULL;
				pDocLineNew->m_pLine->SetData( &cmemPrevLine );
				*(pDocLineNew->m_pLine) += cmemCurLine;

				pDocLineNew->m_cEol = cEOLTypeNext;							/* ���s�R�[�h�̎�� */
				// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];	/* ���s�R�[�h�̒��� */

			}else{
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pDocLineNew;
				}
				pDocLineNew->m_pPrev = m_pDocLineBot;
				m_pDocLineBot = pDocLineNew;
				pDocLineNew->m_pNext = NULL;
				pDocLineNew->m_pLine->SetData( &cmemCurLine );

				pDocLineNew->m_cEol = cEOLTypeNext;							/* ���s�R�[�h�̎�� */
				// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];	/* ���s�R�[�h�̒��� */

			}
			pDocLine = NULL;
			++m_nLines;
			*pnNewPos = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
		}else{
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				pDocLine->m_pLine->SetData( &cmemPrevLine );
				*(pDocLine->m_pLine) += cmemCurLine;

				pDocLine->m_cEol = cEOLTypeNext;						/* ���s�R�[�h�̎�� */
				// pDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];	/* ���s�R�[�h�̒��� */

				pDocLine = pDocLine->m_pNext;
				*pnNewPos = cmemPrevLine.GetLength() + nPos - nBgn;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}else{
				pDocLineNew = new CDocLine;
				pDocLineNew->m_pLine = new CMemory;
				pDocLineNew->m_pPrev = pDocLine->m_pPrev;
				pDocLineNew->m_pNext = pDocLine;
				pDocLine->m_pPrev->m_pNext = pDocLineNew;
				pDocLine->m_pPrev = pDocLineNew;
				pDocLineNew->m_pLine->SetData( &cmemCurLine );

				pDocLineNew->m_cEol = cEOLTypeNext;							/* ���s�R�[�h�̎�� */
				// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];	/* ���s�R�[�h�̒��� */
				pDocLineNew->SetBookMark(bBookMarkNext);	// 2001.12.03 hor �u�b�N�}�[�N�𕜌�


				++m_nLines;
				*pnNewPos = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}
		}
	}
	*pnInsLineNum = m_nLines - nAllLinesOld;
	return;
}

//nEOLType nEOLType nEOLTypeNext



/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
// 2001/06/23 N.Nakatani WhereCurrentWord()�ύX WhereCurrentWord_2���R�[������悤�ɂ���
int	CDocLineMgr::WhereCurrentWord(
	int			nLineNum,
	int			nIdx,
	int*		pnIdxFrom,
	int*		pnIdxTo,
	CMemory*	pcmcmWord,
	CMemory*	pcmcmWordLeft
)
{
	CDocLine*	pDocLine;
	char*		pLine;
	int			nLineLen;
	*pnIdxFrom = nIdx;
	*pnIdxTo = nIdx;
	pDocLine = GetLineInfo( nLineNum );
	if( NULL == pDocLine ){
		return FALSE;
	}
	pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	return CDocLineMgr::WhereCurrentWord_2( pLine, nLineLen, nIdx, pnIdxFrom, pnIdxTo, pcmcmWord, pcmcmWordLeft );

}



//@@@ 2001.06.23 N.Nakatani
/*!
	@brief ���݈ʒu�̒P��͈̔͂𒲂ׂ� static�����o

	@author N.Nakatani

	@param pLine [in] ���ׂ郁�����S�̂̐擪�A�h���X
	@param nLineLen [in] ���ׂ郁�����S�̗̂L����
	@param nIdx [out] �����J�n�n�_:pLine����̑��ΓI�Ȉʒu
	@param pnIdxFrom [out]		�P�ꂪ���������ꍇ�́A�P��̐擪�C���f�b�N�X��Ԃ��B
	@param pnIdxTo [out]		�P�ꂪ���������ꍇ�́A�P��̏I�[�̎��̃o�C�g�̐擪�C���f�b�N�X��Ԃ��B
	@param pcmcmWord [out]		�P�ꂪ���������ꍇ�́A���ݒP���؂�o���Ďw�肳�ꂽCMemory�I�u�W�F�N�g�Ɋi�[����B��񂪕s�v�ȏꍇ��NULL���w�肷��B
	@param pcmcmWordLeft [out]	�P�ꂪ���������ꍇ�́A���ݒP��̍��Ɉʒu����P���؂�o���Ďw�肳�ꂽCMemory�I�u�W�F�N�g�Ɋi�[����B��񂪕s�v�ȏꍇ��NULL���w�肷��B

	@retval true	���� ���݈ʒu�̃f�[�^�́u�P��v�ƔF������B
	@retval false	���s ���݈ʒu�̃f�[�^�́u�P��v�Ƃ͌�������Ȃ��C������B
*/
int	CDocLineMgr::WhereCurrentWord_2(
	const char* pLine,
	int			nLineLen,
	int			nIdx,
	int*		pnIdxFrom,
	int*		pnIdxTo,
	CMemory*	pcmcmWord,
	CMemory*	pcmcmWordLeft
)
{
	int			nCharKind;
	int			nCharKindNext;
	int			nIdxNext;
	int			nIdxNextPrev;
	int			nCharChars;
	*pnIdxFrom = nIdx;
	*pnIdxTo = nIdx;
//	pDocLine = GetLineInfo( nLineNum );
//	if( NULL == pDocLine ){
//		return FALSE;
//	}
//	pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
	if( NULL == pLine ){
		return FALSE;
	}
	if( nIdx >= nLineLen ){
		return FALSE;
	}
	/* ���݈ʒu�̕����̎�ނɂ���Ă͑I��s�\ */
	if( pLine[nIdx] == CR || pLine[nIdx] == LF ){
		return FALSE;
	}
	/* ���݈ʒu�̕����̎�ނ𒲂ׂ� */
	nCharKind = WhatKindOfChar( (char*)pLine, nLineLen, nIdx );
	/* ������ނ��ς��܂őO���փT�[�` */
	nIdxNext = nIdx;
	nCharChars = &pLine[nIdxNext] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxNext] );
	while( nCharChars > 0 ){
		nIdxNextPrev = nIdxNext;
		nIdxNext -= nCharChars;
		nCharKindNext = WhatKindOfChar( (char*)pLine, nLineLen, nIdxNext );
		if( nCharKind == CK_MBC_NOVASU ){
			if( nCharKindNext == CK_MBC_HIRA ||
				nCharKindNext == CK_MBC_KATA ){
				nCharKind = nCharKindNext;
			}
		}else
		if( nCharKind == CK_MBC_HIRA ||
			nCharKind == CK_MBC_KATA ){
			if( nCharKindNext == CK_MBC_NOVASU ){
				nCharKindNext = nCharKind;
			}
		}
		if( nCharKind != nCharKindNext ){
			nIdxNext = nIdxNextPrev;
			break;
		}
		nCharChars = &pLine[nIdxNext] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxNext] );
	}
	*pnIdxFrom = nIdxNext;

	if( NULL != pcmcmWordLeft ){
		pcmcmWordLeft->SetData( &pLine[*pnIdxFrom], nIdx - *pnIdxFrom );
	}

	/* ������ނ��ς��܂Ō���փT�[�` */
	nIdxNext = nIdx;
	nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxNext] ) - &pLine[nIdxNext];
	while( nCharChars > 0 ){
		nIdxNext += nCharChars;
		nCharKindNext = WhatKindOfChar( (char*)pLine, nLineLen, nIdxNext );
		if( nCharKind == CK_MBC_NOVASU ){
			if( nCharKindNext == CK_MBC_HIRA ||
				nCharKindNext == CK_MBC_KATA ){
				nCharKind = nCharKindNext;
			}
		}else
		if( nCharKind == CK_MBC_HIRA ||
			nCharKind == CK_MBC_KATA ){
			if( nCharKindNext == CK_MBC_NOVASU ){
				nCharKindNext = nCharKind;
			}
		}
		if( nCharKind != nCharKindNext ){
			break;
		}
		nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxNext] ) - &pLine[nIdxNext];
	}
	*pnIdxTo = nIdxNext;

	if( NULL != pcmcmWord ){
		pcmcmWord->SetData( &pLine[*pnIdxFrom], *pnIdxTo - *pnIdxFrom );
	}
	return TRUE;
}



/*!	���̒P��̐擪��T��
	pLine�i�����FnLineLen�j�̕����񂩂�P���T���B
	�T���n�߂�ʒu��nIdx�Ŏw��B�����͌���Ɍ���B�P��̗��[�Ŏ~�܂�Ȃ��i�֌W�Ȃ�����j
*/
int CDocLineMgr::SearchNextWordPosition(
	const char* pLine,
	int			nLineLen,
	int			nIdx,		//	����
	int*		pnColmNew,	//	���������ʒu
	BOOL		bStopsBothEnds	//	�P��̗��[�Ŏ~�܂�
)
{
	/* ������ނ��ς��܂Ō���փT�[�` */
	/* �󔒂ƃ^�u�͖������� */

	/* ���݈ʒu�̕����̎�ނ𒲂ׂ� */
	int nCharKind = WhatKindOfChar( pLine, nLineLen, nIdx );

	int nIdxNext = nIdx;
	int nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxNext] ) - &pLine[nIdxNext];
	while( nCharChars > 0 ){
		nIdxNext += nCharChars;
		int nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );
		/* �󔒂ƃ^�u�͖������� */
		if( nCharKindNext == CK_TAB || nCharKindNext == CK_SPACE ){
			if ( bStopsBothEnds && nCharKind != nCharKindNext ){
				*pnColmNew = nIdxNext;
				return TRUE;
			}
			nCharKind = nCharKindNext;
		}
		else {
			if( nCharKind == CK_MBC_NOVASU ){
				if( nCharKindNext == CK_MBC_HIRA ||
					nCharKindNext == CK_MBC_KATA ){
					nCharKind = nCharKindNext;
				}
			}else
			if( nCharKind == CK_MBC_HIRA ||
				nCharKind == CK_MBC_KATA ){
				if( nCharKindNext == CK_MBC_NOVASU ){
					nCharKindNext = nCharKind;
				}
			}
			if( nCharKind != nCharKindNext ){
				*pnColmNew = nIdxNext;
				return TRUE;
			}
		}
		nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxNext] ) - &pLine[nIdxNext];
	}
	return FALSE;
}

/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
int CDocLineMgr::PrevOrNextWord(
	int			nLineNum,	//	�s��
	int			nIdx,		//	����
	int*		pnColmNew,	//	���������ʒu
	BOOL		bLEFT,		//	TRUE:�O���i���j�֌������BFALSE:����i�E�j�֌������B
	BOOL		bStopsBothEnds	//	�P��̗��[�Ŏ~�܂�
)
{
	CDocLine*	pDocLine;
	char*		pLine;
	int			nLineLen;
	int			nCharKind;
	int			nCharKindNext;
	int			nIdxNext;
	int			nIdxNextPrev;
	int			nCharChars;
	int			nCount;
	pDocLine = GetLineInfo( nLineNum );
	if( NULL == pDocLine ){
		return FALSE;
	}
	pLine = pDocLine->m_pLine->GetPtr( &nLineLen );

	// ABC D[EOF]�ƂȂ��Ă����Ƃ��ɁAD�̌��ɃJ�[�\�������킹�A�P��̍��[�Ɉړ�����ƁAA�ɃJ�[�\���������o�O�C���BYAZAKI
	if( nIdx >= nLineLen ){
		if (bLEFT && nIdx == nLineLen){

		}
		else {
			nIdx = nLineLen - 1;
		}
	}
	/* ���݈ʒu�̕����̎�ނɂ���Ă͑I��s�\ */
	if( FALSE == bLEFT && ( pLine[nIdx] == CR || pLine[nIdx] == LF ) ){
		return FALSE;
	}
	/* �O�̒P�ꂩ�H���̒P�ꂩ�H */
	if( bLEFT ){
		/* ���݈ʒu�̕����̎�ނ𒲂ׂ� */
		nCharKind = WhatKindOfChar( pLine, nLineLen, nIdx );
		if( nIdx == 0 ){
			return FALSE;
		}
		/* ������ނ��ς��܂őO���փT�[�` */
		/* �󔒂ƃ^�u�͖������� */
		nCount = 0;
		nIdxNext = nIdx;
		nCharChars = &pLine[nIdxNext] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxNext] );
		while( nCharChars > 0 ){
			nIdxNextPrev = nIdxNext;
			nIdxNext -= nCharChars;
			nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );
			/* �󔒂ƃ^�u�͖������� */
			if( nCharKind == CK_MBC_NOVASU ){
				if( nCharKindNext == CK_MBC_HIRA ||
					nCharKindNext == CK_MBC_KATA ){
					nCharKind = nCharKindNext;
				}
			}else
			if( nCharKind == CK_MBC_HIRA ||
				nCharKind == CK_MBC_KATA ){
				if( nCharKindNext == CK_MBC_NOVASU ){
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
			nCharChars = &pLine[nIdxNext] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxNext] );
			++nCount;
		}
		*pnColmNew = nIdxNext;
	}else{
		CDocLineMgr::SearchNextWordPosition(pLine, nLineLen, nIdx, pnColmNew, bStopsBothEnds);
	}
	return TRUE;
}





/*! �P�ꌟ��

	@date 2003.05.22 ����� �s�������Ȃǌ�����
*/
/* ������Ȃ��ꍇ�͂O��Ԃ� */
int CDocLineMgr::SearchWord(
	int			nLineNum,		/* �����J�n�s */
	int			nIdx, 			/* �����J�n�ʒu */
	const char*	pszPattern,		/* �������� */
	int			bPrevOrNext,	/* 0==�O������ 1==������� */
	int			bRegularExp,	/* 1==���K�\�� */
	int			bLoHiCase,		/* 1==�p�啶���������̋�� */
	int			bWordOnly,		/* 1==�P��̂݌��� */
	int*		pnLineNum, 		/* �}�b�`�s */
	int*		pnIdxFrom, 		/* �}�b�`�ʒufrom */
	int*		pnIdxTo,  		/* �}�b�`�ʒuto */
	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	CBregexp*	pRegexp			/*!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������ */
)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::SearchWord" );
//#endif
	CDocLine*	pDocLine;
	int			nLinePos;
	int			nIdxPos;
	const char*	pLine;
	int			nLineLen;
	char*		pszRes;
	int			nHitTo;
	int			nHitPos;
	int			nHitPosOld;
	int			nHitLenOld;
	int			nRetVal;
	int*		pnKey_CharCharsArr;
	//	Jun. 10, 2003 Moca
	//	lstrlen�𖈉�Ă΂���nPatternLen���g���悤�ɂ���
	const int	nPatternLen = lstrlen( pszPattern );	//2001/06/23 N.Nakatani
	pnKey_CharCharsArr = NULL;
//	int*		pnKey_CharUsedArr;
//	pnKey_CharUsedArr = NULL;
	/* ���������̏�� */
	CDocLineMgr::CreateCharCharsArr(
		(const unsigned char *)pszPattern,
		nPatternLen,
		&pnKey_CharCharsArr
	);
//	/* ���������̏��(�L�[������̎g�p�����\)�쐬 */
//	CDocLineMgr::CreateCharUsedArr(
//		(const unsigned char *)pszPattern,
//		lstrlen( pszPattern ),
//		pnKey_CharCharsArr,
//		&pnKey_CharUsedArr
//	);

	/* 1==���K�\�� */
	if( bRegularExp ){
		BREGEXP* pRegexpData;
		/* 0==�O������ 1==������� */
		if( 0 == bPrevOrNext ){
			//
			// �O��(��)����(���K�\��)
			//
			nLinePos = nLineNum;		// �����s
			nHitTo = nIdx;				// �����J�n�ʒu
			nIdxPos = 0;
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				nHitPos		= -1;
				//	Jun. 27, 2001 genta	���K�\�����C�u�����̈Ⴂ���z�����邽�ߕϐ��ǉ�
				int nCurLen = 0;	//	�}�b�`��������
				while( 1 ){
					nHitPosOld = nHitPos;
					//	From Here Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
					nHitLenOld = nCurLen;
					if( nIdxPos <= pDocLine->GetLengthWithoutEOL() 
						&& pRegexp->GetMatchInfo( pLine, nLineLen, nIdxPos, &pRegexpData ) ){
						// �����Ƀ}�b�`�����I
						nHitPos = pRegexpData->startp[0] - pLine;
						//nIdxPos = pRegexpData->endp[0] - pLine + 1;�ł́A�P���������ł��Q���������������̂� NG 2003.05.03 �����
						nIdxPos = pRegexpData->endp[0] - pLine;
						nCurLen = pRegexpData->endp[0] - pRegexpData->startp[0];
						// �����O�Ń}�b�`�����̂ŁA���̈ʒu�ōēx�}�b�`���Ȃ��悤�ɁA�P�����i�߂�
						if (nCurLen == 0) {
							nIdxPos += (CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxPos] ) - &pLine[nIdxPos] == 2 ? 2 : 1);
						}
						//	From Here Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
						if( nHitPos >= nHitTo ){
							// �}�b�`�����̂́A�J�[�\���ʒu�ȍ~������
							// ���Ȃ킿�A���̍s�łP�O�Ƀ}�b�`�����ʒu���A���������������ʒu
							if( -1 != nHitPosOld ){
								// ���̍s�łP�O�Ƀ}�b�`�����ʒu�����݂���̂ŁA�����Ԃ�
								*pnLineNum = nLinePos;				/* �}�b�`�s */
								*pnIdxFrom = nHitPosOld;			/* �}�b�`�ʒufrom */
								*pnIdxTo = *pnIdxFrom + nHitLenOld;	/* �}�b�`�ʒuto */
								nRetVal = 1;
								goto end_of_func;
							}else{
								// ���̍s�łP�O�Ƀ}�b�`�����ʒu�����݂��Ȃ��̂ŁA�O�̍s��������
								break;		// �O�̍s��������
							}
						}
					// From Here 2001.12.03 hor /^/ or /$/ �Ŗ������[�v����̂����
						if( -1 != nHitPosOld && nHitPosOld==nHitPos ){
							*pnLineNum = nLinePos;				/* �}�b�`�s */
							*pnIdxFrom = nHitPosOld;			/* �}�b�`�ʒufrom */
							*pnIdxTo = *pnIdxFrom + nHitLenOld;	/* �}�b�`�ʒuto */
							nRetVal = 1;
							goto end_of_func;
						}
					// To Here 2001.12.03 hor
					}else{
						if( -1 != nHitPosOld ){
							*pnLineNum = nLinePos;				/* �}�b�`�s */
							*pnIdxFrom = nHitPosOld;			/* �}�b�`�ʒufrom */
							*pnIdxTo = *pnIdxFrom + nHitLenOld;	/* �}�b�`�ʒuto */
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
//					nHitTo = lstrlen( pDocLine->m_pLine->GetPtr() );
//					nHitTo = pDocLine->m_pLine->GetLength();
					nHitTo = pDocLine->m_pLine->GetLength() + 1;		// �O�̍s��NULL����(\0)�ɂ��}�b�`�����邽�߂�+1 2003.05.16 ����� 
				}
			}
			nRetVal = 0;
			goto end_of_func;
		}else{
			//
			// �������(���K�\��)
			//
			nIdxPos = nIdx;
			nLinePos = nLineNum;
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				//	From Here Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
				if( nIdxPos <= pDocLine->GetLengthWithoutEOL() && // 2002.02.08 hor $�̎������Ŏ��̍s�Ɉړ��ł��Ȃ��������
					pRegexp->GetMatchInfo( pLine, nLineLen, nIdxPos, &pRegexpData ) ){
// �s����������������� endp[0]-pLine�͂O�ɂȂ�̂ŁA���̏����ł͍s�������̌������ł��Ȃ��s��ƂȂ�
//					if(nIdxPos<(pRegexpData->endp[0]-pLine)){	// 2002.02.08 hor EOF���O�̕��������x���}�b�`���Ă��܂��������
// EOF�s�̒��O�Ƀ}�b�`������́AGetMatchInfo���őΉ� 2003.05.03 by �����
					*pnLineNum = nLinePos;								/* �}�b�`�s */
					*pnIdxFrom = pRegexpData->startp[0] - pLine;		/* �}�b�`�ʒufrom */
					*pnIdxTo = pRegexpData->endp[0] - pLine;			/* �}�b�`�ʒuto */
				//	To Here Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
					nRetVal = 1;
					goto end_of_func;
				}
// ���� 2003.05.03
//				}
				++nLinePos;
				pDocLine = pDocLine->m_pNext;
				nIdxPos = 0;
			}
			nRetVal = 0;
			goto end_of_func;
		}
	}else
	/* 1==�P��̂݌��� */
	if( bWordOnly ){
		/*
			2001/06/23 Norio Nakatani
			�P��P�ʂ̌����������I�Ɏ����B�P���WhereCurrentWord()�Ŕ��ʂ��Ă܂��̂ŁA
			�p�P���C/C++���ʎq�Ȃǂ̌��������Ȃ�q�b�g���܂��B
		*/

		/* 0==�O������ 1==������� */
		if( 0 == bPrevOrNext ){
			nLinePos = nLineNum;
			pDocLine = GetLineInfo( nLinePos );
			int nNextWordFrom;
			int nNextWordFrom2;
			int nNextWordTo2;
//			int bState;
			int nWork;
//			CMemory cmemTest;
			nNextWordFrom = nIdx;
			while( NULL != pDocLine ){
				if( TRUE == PrevOrNextWord( nLinePos, nNextWordFrom, &nWork, TRUE, FALSE ) ){
					nNextWordFrom = nWork;
					if( WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL ) ){
						if( nPatternLen == nNextWordTo2 - nNextWordFrom2 ){
							const char* pData = pDocLine->m_pLine->GetPtr();	// 2002/2/10 aroka CMemory�ύX
							/* 1==�啶���������̋�� */
							if( (FALSE == bLoHiCase && 0 == _memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) ) ||
								(TRUE  == bLoHiCase && 0 ==	 memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) )
							){
								*pnLineNum = nLinePos;	/* �}�b�`�s */
								*pnIdxFrom = nNextWordFrom2;	/* �}�b�`�ʒufrom */
								*pnIdxTo = *pnIdxFrom + nPatternLen;	/* �}�b�`�ʒuto */
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
					nNextWordFrom = pDocLine->m_pLine->GetLength() - pDocLine->m_cEol.GetLen();
					if( 0 > nNextWordFrom ){
						nNextWordFrom = 0;
					}
				}
			}
		}else{
			nLinePos = nLineNum;
			pDocLine = GetLineInfo( nLinePos );
			int nNextWordFrom;

			int nNextWordFrom2;
			int nNextWordTo2;
//			int bState;
			nNextWordFrom = nIdx;
			while( NULL != pDocLine ){
				if( TRUE ==
					WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL )
				){
					if( nPatternLen == nNextWordTo2 - nNextWordFrom2 ){
						const char* pData = pDocLine->m_pLine->GetPtr();	// 2002/2/10 aroka CMemory�ύX
						/* 1==�啶���������̋�� */
						if( (FALSE == bLoHiCase && 0 == _memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) ) ||
							(TRUE  == bLoHiCase && 0 ==	  memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) )
						){
							*pnLineNum = nLinePos;	/* �}�b�`�s */
							*pnIdxFrom = nNextWordFrom2;	/* �}�b�`�ʒufrom */
							*pnIdxTo = *pnIdxFrom + nPatternLen;	/* �}�b�`�ʒuto */
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
				nNextWordFrom = 0;
			}
		}

		nRetVal = 0;
		goto end_of_func;
	}else{
		/* 0==�O������ 1==������� */
		if( 0 == bPrevOrNext ){
			nLinePos = nLineNum;
			nHitTo = nIdx;

			nIdxPos = 0;
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				nHitPos = -1;
				while( 1 ){
					nHitPosOld = nHitPos;
					pszRes = SearchString(
						(const unsigned char *)pLine,
						nLineLen,
						nIdxPos,
						(const unsigned char *)pszPattern,
						nPatternLen,
						pnKey_CharCharsArr,
//						pnKey_CharUsedArr,
						bLoHiCase
					);
					if( NULL != pszRes ){
						nHitPos = pszRes - pLine;
						nIdxPos = CMemory::MemCharNext(pLine, nLineLen, pszRes) - pLine;	//	Azumaiya�O�������őI��͈͂����������Ȃ邱�Ƃ�����o�O�C��
						if( nHitPos >= nHitTo ){
							if( -1 != nHitPosOld ){
								*pnLineNum = nLinePos;							/* �}�b�`�s */
								*pnIdxFrom = nHitPosOld;						/* �}�b�`�ʒufrom */
								*pnIdxTo = *pnIdxFrom + nPatternLen;			/* �}�b�`�ʒuto */
								nRetVal = 1;
								goto end_of_func;
							}else{
								break;
							}
						}
					}else{
						if( -1 != nHitPosOld ){
							*pnLineNum = nLinePos;							/* �}�b�`�s */
							*pnIdxFrom = nHitPosOld;						/* �}�b�`�ʒufrom */
							*pnIdxTo = *pnIdxFrom + nPatternLen;			/* �}�b�`�ʒuto */
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
//					nHitTo = lstrlen( pDocLine->m_pLine->GetPtr() );
					nHitTo = pDocLine->m_pLine->GetLength();
				}
			}
			nRetVal = 0;
			goto end_of_func;
		}else{
			nIdxPos = nIdx;
			nLinePos = nLineNum;
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				pszRes = SearchString(
					(const unsigned char *)pLine,
					nLineLen,
					nIdxPos,
					(const unsigned char *)pszPattern,
					nPatternLen,
					pnKey_CharCharsArr,
//					pnKey_CharUsedArr,
					bLoHiCase
				);
				if( NULL != pszRes ){
					*pnLineNum = nLinePos;							/* �}�b�`�s */
					*pnIdxFrom = pszRes - pLine;					/* �}�b�`�ʒufrom */
					*pnIdxTo = *pnIdxFrom + nPatternLen;	/* �}�b�`�ʒuto */
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
//	if( NULL != pnKey_CharUsedArr ){
//		delete [] pnKey_CharUsedArr;
//		pnKey_CharUsedArr = NULL;
//	}
	return nRetVal;

}

/* ���������̏��(�L�[������̑S�p�����p���̔z��)�쐬 */
void CDocLineMgr::CreateCharCharsArr(
	const unsigned char*	pszPattern,
	int						nSrcLen,
	int**					ppnCharCharsArr
)
{
	int		i;
	int*	pnCharCharsArr;
	pnCharCharsArr = new int[nSrcLen];
	for( i = 0; i < nSrcLen; /*i++*/ ){
		pnCharCharsArr[i] = CMemory::MemCharNext( (const char *)pszPattern, nSrcLen, (const char *)&pszPattern[i] ) - (const char *)&pszPattern[i];
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


//	/* ���������̏��(�L�[������̎g�p�����\)�쐬 */
//	void CDocLineMgr::CreateCharUsedArr(
//		const unsigned char*	pszPattern,
//		int						nSrcLen,
//		const int*				pnCharCharsArr,
//		int**					ppnCharUsedArr
//	)
//	{
//		int		i;
//		int*	pnCharUsedArr;
//		int		nCharCode;
//		pnCharUsedArr = new int[0x0000ffff];
//
//		i = 0xffff;//TEST
//
//		for( i = 0; i < 0x0000ffff; ++i ){
//			pnCharUsedArr[i] = nSrcLen;
//		}
//		for( i = 0; i < nSrcLen; /*i++*/ ){
//			if( 2 == pnCharCharsArr[i] ){
//	//			nCharCode = *((WORD*)(&pszPattern[i]));
//				nCharCode = MAKEWORD( pszPattern[i + 1], pszPattern[i] );
//			}else{
//				nCharCode = (int)pszPattern[i];
//			}
//			if( nSrcLen == pnCharUsedArr[nCharCode] ){
//				pnCharUsedArr[nCharCode] = i;
//			}
//			i+= pnCharCharsArr[i];
//		}
//		*ppnCharUsedArr = pnCharUsedArr;
//		return;
//	}


/* �����񌟍� */
char* CDocLineMgr::SearchString(
		const unsigned char*	pLine,
		int						nDesLen,
		int						nIdxPos,
		const unsigned char*	pszPattern,
		int						nSrcLen,
		int*					pnCharCharsArr,
//		int*					pnCharUsedArr,
		int						bLoHiCase
)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::SearchString" );
//#endif
	if( nDesLen < nSrcLen ){
		return NULL;
	}
	if( 0 >= nSrcLen || 0 >= nDesLen){
		return NULL;
	}

	int	nPos;
	int	i;
	int	j;
	int	nWork;
	int	nCharChars;
	int	nCharChars1;
	int	nCharChars2;
	int	nCompareTo;

//	Mar. 4, 2001 genta
#if 0
	if( !bLoHiCase ){
		nCompareTo = nDesLen - nSrcLen;
		for( nPos = nIdxPos; nPos <= nCompareTo; /*nPos++*/ ){
			nCharChars = CMemory::MemCharNext( (const char *)pLine, nDesLen, (const char *)&pLine[nPos] ) - (const char *)&pLine[nPos];
			nCharChars1 = nCharChars;
			for( i = 0; i < nSrcLen; /*i++*/ ){
				if( NULL != pnCharCharsArr ){
					nCharChars2 = pnCharCharsArr[i];
				}else{
					nCharChars2 = CMemory::MemCharNext( (const char *)pszPattern, nSrcLen, (const char *)&pszPattern[i] ) - (const char *)&pszPattern[i];
				}
				if( nCharChars1 != nCharChars2 ){
					break;
				}
				if( 0 == nCharChars1 ){
					nWork =1;
				}else{
					nWork = nCharChars1;
				}

				for( j = 0; j < nWork; ++j ){
					if( islower( pLine[nPos + i + j] ) ){
						if( islower( pszPattern[i + j] ) ){
							if( toupper( pLine[nPos + i + j] ) != toupper( pszPattern[i + j] ) ){
								break;
							}
						}else{
							if( toupper( pLine[nPos + i + j] ) != pszPattern[i + j] ){
								break;
							}
						}
					}else{
						if( islower( pszPattern[i + j] ) ){
							if( pLine[nPos + i + j] != toupper( pszPattern[i + j] ) ){
								break;
							}
						}else{
							if( pLine[nPos + i + j] != pszPattern[i + j] ){
								break;
							}
						}
					}
				}
				if( j < nWork ){
					break;
				}
				if( 0 == nCharChars2 ){
					++i;
				}else{
					i+= nCharChars2;
				}
				nCharChars1 = CMemory::MemCharNext( (const char *)pLine, nDesLen, (const char *)&pLine[nPos + i] ) - (const char *)&pLine[nPos + i];
			}
			if( i >= nSrcLen ){
				return (char *)&pLine[nPos];
			}
			if( 0 == nCharChars ){
				++nPos;
			}else{
				nPos+= nCharChars;
			}
		}
		return NULL;
	}else{
//	Mar. 4, 2001 genta
#endif
		/* ���`�T�� */
		nCompareTo = nDesLen - nSrcLen;	//	Mar. 4, 2001 genta
		for( nPos = nIdxPos; nPos <= nCompareTo; /*nPos++*/ ){
			nCharChars = CMemory::MemCharNext( (const char *)pLine, nDesLen, (const char *)&pLine[nPos] ) - (const char *)&pLine[nPos];
			nCharChars1 = nCharChars;
			for( i = 0; i < nSrcLen; /*i++*/ ){
				if( NULL != pnCharCharsArr ){
					nCharChars2 = pnCharCharsArr[i];
				}else{
					nCharChars2 = CMemory::MemCharNext( (const char *)pszPattern, nSrcLen, (const char *)&pszPattern[i] ) - (const char *)&pszPattern[i];
				}
				if( nCharChars1 != nCharChars2 ){
					break;
				}
				if( 0 == nCharChars1 ){
					nWork =1;
				}else{
					nWork = nCharChars1;
				}
				//	From Here Mar. 4, 2001 genta
				if( !bLoHiCase && nWork == 1 ){	//	�p�召�����̓��ꎋ
					if( toupper( pLine[nPos + i] ) != toupper( pszPattern[i] ) )
						break;
				}
				else {
					for( j = 0; j < nWork; ++j ){
						if( pLine[nPos + i + j] != pszPattern[i + j] ){
							break;
						}
					}
					if( j < nWork ){
						break;
					}
				}
				//	To Here
				if( 0 == nCharChars2 ){
					++i;
				}else{
					i+= nCharChars2;
				}
				nCharChars1 = CMemory::MemCharNext( (const char *)pLine, nDesLen, (const char *)&pLine[nPos + i] ) - (const char *)&pLine[nPos + i];
			}
			if( i >= nSrcLen ){
				return (char *)&pLine[nPos];
			}
			if( 0 == nCharChars ){
				++nPos;
			}else{
				nPos+= nCharChars;
			}
		}
		return NULL;
	//	Mar. 4, 2001 genta	: comment out
	//}
}



/* ���݈ʒu�̕����̎�ނ𒲂ׂ� */
int	CDocLineMgr::WhatKindOfChar(
		const char*	pData,
		int		pDataLen,
		int		nIdx
)
{
	int		nCharChars;
	WORD	wChar;
	nCharChars = CMemory::MemCharNext( pData, pDataLen, &pData[nIdx] ) - &pData[nIdx];
	if( nCharChars == 0 ){
		return CK_NULL;	/* NULL 0x0<=c<=0x0 */
	}else
	if( nCharChars == 1 ){	/* ���p���� */
		if( pData[nIdx] == CR ){
			return CK_CR;	/* CR = 0x0d */
		}
		if( pData[nIdx] == LF ){
			return CK_LF;	/* LF = 0x0a */
		}
		if( pData[nIdx] == TAB ){
			return CK_TAB;	/* �^�u 0x9<=c<=0x9 */
		}
		if( pData[nIdx] == SPACE ){
			return CK_SPACE;	/* ���p�̃X�y�[�X 0x20<=c<=0x20 */
		}
		if( __iscsym( pData[nIdx] ) ){
			return CK_CSYM;	/* ���p�̉p���A�A���_�[�X�R�A�A�����̂����ꂩ */
		}
		if( pData[nIdx] >= (char)0xa1 && pData[nIdx] <= (char)0xdf ){ // Mar. 30, 2003 genta fd->df
			return CK_KATA;	/* ���p�̃J�^�J�i 0xA1<=c<=0xFD */
		}
		return CK_ETC;	/* ���p�̂��̑� */

	}else
	if( nCharChars == 2 ){	/* �S�p���� */
		//<< 2002/03/28 Azumaiya
		// ������������̔�r�񐔂����Ȃ����čœK���B
		wChar =  MAKEWORD(pData[nIdx + 1], pData[nIdx]);
//		MYTRACE( "wChar=%0xh\n", wChar );
		if( wChar == (WORD)0x8140 ){
			return CK_MBC_SPACE;	/* 2�o�C�g�̃X�y�[�X */
		}
		if( wChar == (WORD)0x815B ){
			return CK_MBC_NOVASU;	/* �L�΂��L�� 0x815B<=c<=0x815B '�[' */
		}
//		if( wChar == (WORD)0x8151 ||								/* 0x8151<=c<=0x8151 �S�p�A���_�[�X�R�A */
//			(wChar >= (WORD)0x824F && wChar <= (WORD)0x8258 ) ||	/* 0x824F<=c<=0x8258 �S�p���� */
//			(wChar >= (WORD)0x8260 && wChar <= (WORD)0x8279 ) ||	/* 0x8260<=c<=0x8279 �S�p�p�啶�� */
//			(wChar >= (WORD)0x8281 && wChar <= (WORD)0x829a )		/* 0x8281<=c<=0x829a �S�p�p������ */
//		){
		if (
			(WORD)wChar == 0x8151 ||			/* 0x8151<=c<=0x8151 �S�p�A���_�[�X�R�A */
			(WORD)(wChar - 0x824F) <= 0x09 ||	/* 0x824F<=c<=0x8258 �S�p���� */
			(WORD)(wChar - 0x8260) <= 0x19 ||	/* 0x8260<=c<=0x8279 �S�p�p�啶�� */
			(WORD)(wChar - 0x8281) <= 0x19 		/* 0x8281<=c<=0x829a �S�p�p������ */
		   ){
			return CK_MBC_CSYM;	/* 2�o�C�g�̉p���A�A���_�[�X�R�A�A�����̂����ꂩ */
		}
//		if( wChar >= (WORD)0x8140 && wChar <= (WORD)0x81FD ){
		if( (WORD)(wChar - 0x8140) <= 0xBD ){ /* 0x8140<=c<=0x81FD 2�o�C�g�̋L�� */
			return CK_MBC_KIGO;	/* 2�o�C�g�̋L�� */
		}
//		if( wChar >= (WORD)0x829F && wChar <= (WORD)0x82F1 ){
		if( (WORD)(wChar - 0x829F) <= 0x52 ){	/* 0x829F<=c<=0x82F1 2�o�C�g�̂Ђ炪�� */
			return CK_MBC_HIRA;	/* 2�o�C�g�̂Ђ炪�� */
		}
//		if( wChar >= (WORD)0x8340 && wChar <= (WORD)0x8396 ){
		if( (WORD)(wChar - 0x8340) <= 0x56 ){	/* 0x8340<=c<=0x8396 2�o�C�g�̃J�^�J�i */
			return CK_MBC_KATA;	/* 2�o�C�g�̃J�^�J�i */
		}
//		if( wChar >= (WORD)0x839F && wChar <= (WORD)0x83D6 ){
		if( (WORD)(wChar - 0x839F) <= 0x37 ){	/* 0x839F<=c<=0x83D6 2�o�C�g�̃M���V������ */
			return CK_MBC_GIRI;	/* 2�o�C�g�̃M���V������ */
		}
//		if( ( wChar >= (WORD)0x8440 && wChar <= (WORD)0x8460 ) ||	/* 0x8440<=c<=0x8460 �S�p���V�A�����啶�� */
//			( wChar >= (WORD)0x8470 && wChar <= (WORD)0x8491 ) ){	/* 0x8470<=c<=0x8491 �S�p���V�A���������� */
		if(
			(WORD)(wChar - 0x8440) <= 0x20 ||	/* 0x8440<=c<=0x8460 �S�p���V�A�����啶�� */
			(WORD)(wChar - 0x8470) <= 0x21		/* 0x8470<=c<=0x8491 �S�p���V�A���������� */
		   ){
			return CK_MBC_ROS;	/* 2�o�C�g�̃��V�A����: */
		}
//		if( wChar >= (WORD)0x849F && wChar <= (WORD)0x879C ){
		if( (WORD)(wChar - 0x849F) <= 0x02FD ){	/* 0x849F<=c<=0x879C 2�o�C�g�̓���L�� */
			return CK_MBC_SKIGO;	/* 2�o�C�g�̓���L�� */
		}
		return CK_MBC_ETC;	/* 2�o�C�g�̂��̑�(�����Ȃ�) */
		//>> 2002/03/28 Azumaiya
	}else{
		return CK_NULL;	/* NULL 0x0<=c<=0x0 */
	}
}





void CDocLineMgr::DUMP( void )
{
#ifdef _DEBUG
	MYTRACE( "------------------------\n" );
	MYTRACE( "m_nLines=%d\n", m_nLines );
	MYTRACE( "m_pDocLineTop=%08lxh\n", m_pDocLineTop );
	MYTRACE( "m_pDocLineBot=%08lxh\n", m_pDocLineBot );

	CDocLine* pDocLine;
	CDocLine* pDocLineNext;
	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine ){
		pDocLineNext = pDocLine->m_pNext;
		MYTRACE( "\t-------\n" );
		MYTRACE( "\tthis=%08lxh\n", pDocLine );
		MYTRACE( "\tpPrev; =%08lxh\n", pDocLine->m_pPrev );
		MYTRACE( "\tpNext; =%08lxh\n", pDocLine->m_pNext );

		MYTRACE( "\tm_enumEOLType =%s\n", pDocLine->m_cEol.GetName() );
		MYTRACE( "\tm_nEOLLen =%d\n", pDocLine->m_cEol.GetLen() );


//		MYTRACE( "\t[%s]\n", (char*)*(pDocLine->m_pLine) );
		MYTRACE( "\tpDocLine->m_pLine->GetLength()=[%d]\n", pDocLine->m_pLine->GetLength() );
		MYTRACE( "\t[%s]\n", pDocLine->m_pLine->GetPtr() );


		pDocLine = pDocLineNext;
	}
	MYTRACE( "------------------------\n" );
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


/* �S�s�f�[�^��Ԃ� */
char* CDocLineMgr::GetAllData( int*	pnDataLen )
{
	int			nDataLen;
	char*		pLine;
	int			nLineLen;
	CDocLine* 	pDocLine;

	pDocLine = m_pDocLineTop;
	nDataLen = 0;
	while( NULL != pDocLine ){   
		//	Oct. 7, 2002 YAZAKI
		nDataLen += pDocLine->GetLengthWithoutEOL() + 2;	//	\r\n��ǉ����ĕԂ�����+2����B
		pDocLine = pDocLine->m_pNext;
	}

	char*	pData;
	pData = (char*)malloc( nDataLen + 1 );
	if( NULL == pData ){
		::MYMESSAGEBOX(
			NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
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
		if( 0 <= nLineLen ){
			pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
			memcpy( &pData[nDataLen], pLine, nLineLen );
			nDataLen += nLineLen;
		}
		memcpy( &pData[nDataLen], "\r\n", 2 );
		nDataLen += 2;
		pDocLine = pDocLine->m_pNext;
	}
	pData[nDataLen] = '\0';
	*pnDataLen = nDataLen;
	return (char*)pData;
}


/* �s�I�u�W�F�N�g�̍폜�A���X�g�ύX�A�s��-- */
void CDocLineMgr::DeleteNode( CDocLine* pCDocLine )
{
	m_nLines--;	/* �S�s�� */
	if( 0 == m_nLines ){
		/* �f�[�^���Ȃ��Ȃ��� */
		Init();
		return;
	}
	if( NULL == pCDocLine->m_pPrev ){
		m_pDocLineTop = pCDocLine->m_pNext;
	}else{
		pCDocLine->m_pPrev->m_pNext = pCDocLine->m_pNext;
	}
	if( NULL == pCDocLine->m_pNext ){
		m_pDocLineBot = pCDocLine->m_pPrev;
	}else{
		pCDocLine->m_pNext->m_pPrev = pCDocLine->m_pPrev;
	}
	if( m_pCodePrevRefer == pCDocLine ){
		m_pCodePrevRefer = pCDocLine->m_pNext;
	}
	delete pCDocLine;
	pCDocLine = NULL;


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


/*[EOF]*/
