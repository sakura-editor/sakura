//	$Id$
/************************************************************************
	CDocLineMgr_New.cpp
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1999/12/17  �V�K�쐬
************************************************************************/
#include "charcode.h"
#include "CDocLineMgr.h"
#include "debug.h"
#include "charcode.h"
//#include <stdio.h>
#include <io.h>
//#include <string.h>
//#include <memory.h>
#include "CJre.h"
#include <commctrl.h>
#include "global.h"
#include "etc_uty.h"
#include "CRunningTimer.h"
#include "CDlgCancel.h"


/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
  From�̈ʒu�փe�L�X�g��}������
*/
void CDocLineMgr::ReplaceData(
		DocLineReplaceArg* pArg
#if 0
	int			nDelLineFrom,			/* �폜�͈͍s  From ���s�P�ʂ̍s�ԍ� 0�J�n) */
	int			nDelPosFrom,			/* �폜�͈͈ʒuFrom ���s�P�ʂ̍s������̃o�C�g�ʒu�@0�J�n) */
	int			nDelLineTo,			/* �폜�͈͍s�@To   ���s�P�ʂ̍s�ԍ� 0�J�n) */
	int			nDelPosTo,				/* �폜�͈͈ʒuTo   ���s�P�ʂ̍s������̃o�C�g�ʒu�@0�J�n) */
	CMemory*	pcmemDeleted,		/* �폜���ꂽ�f�[�^��ۑ� */
	int*		pnDeletedLineNum,	/* �폜�����s�̑��� */
	const char*	pInsData,			/* �}������f�[�^ */      
	int			nInsDataLen,		/* �}������f�[�^�̒��� */
	int*		pnInsLineNum,		/* �}���ɂ���đ������s�̐� */
	int*		pnNewLine,			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int*		pnNewPos			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
#endif
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::ReplaceData()" );
#endif
	/* �}���ɂ���đ������s�̐� */
	pArg->nInsLineNum = 0;
	/* �폜�����s�̑��� */
	pArg->nDeletedLineNum = 0;
	/* �폜���ꂽ�f�[�^ */
	pArg->pcmemDeleted->SetDataSz( "" );

	CDocLine* pCDocLine;
	CDocLine* pCDocLinePrev;
	CDocLine* pCDocLineNext;
	CDocLine* pCDocLineNew;
	int nWorkPos;
	int nWorkLen;
	char* pLine;
	int nLineLen;
	char* pLine2;
	int nLineLen2;
	int i;
	char* pWork;
	int			nBgn;
	int			nPos;
	int			nAllLinesOld;
	int			nCount;
	int			nProgress;
	CMemory		cmemPrevLine;
	CMemory		cmemCurLine;
	CMemory		cmemNextLine;
	//	May 15, 2000
	CEOL cEOLType;
	CEOL cEOLTypeNext;
	// enumEOLType nEOLType;
	// enumEOLType nEOLTypeNext;
	CDlgCancel*	pCDlgCancel = NULL;
	HWND		hwndCancel;
	HWND		hwndProgress;
	

	pArg->nNewLine = pArg->nDelLineFrom;
	pArg->nNewPos =  pArg->nDelPosFrom;

	/* ��ʂ̃f�[�^�𑀍삷��Ƃ� */
	if( 3000 < pArg->nDelLineTo - pArg->nDelLineFrom 
	 || 1024000 < pArg->nInsDataLen
	){
//		/* �o�b�t�@�T�C�Y�̒��� */
//		cmemWork.AllocBuffer( 1024000 );
		
		/* �i���_�C�A���O�̕\�� */
		pCDlgCancel = new CDlgCancel;
		if( NULL != ( hwndCancel = pCDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
			::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
 			::SendMessage( hwndProgress, PBM_SETPOS, 0, 0 );

//			hwndStatic = ::GetDlgItem( hwndCancel, IDC_STATIC_KENSUU );
// 			::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)"" );
		}
	}
//	pcmemDeleted->cmemWork( 1024000 );
	

	
	
	
	

	// �폜�f�[�^�̎擾�̃��[�v
	/* �O���珈�����Ă��� */
	/* ���ݍs�̏��𓾂� */
	pCDocLine = GetLineInfo( pArg->nDelLineFrom );
	for( i = pArg->nDelLineFrom; i <= pArg->nDelLineTo && NULL != pCDocLine; i++ ){
		pLine = pCDocLine->m_pLine->m_pData;
		nLineLen = pCDocLine->m_pLine->m_nDataLen;
		pCDocLinePrev = pCDocLine->m_pPrev;
		pCDocLineNext = pCDocLine->m_pNext;
		/* ���ݍs�̍폜�J�n�ʒu�𒲂ׂ� */
		if( i == pArg->nDelLineFrom ){
			nWorkPos = pArg->nDelPosFrom;
		}else{
			nWorkPos = 0;
		}
		/* ���ݍs�̍폜�f�[�^���𒲂ׂ� */
		if( i == pArg->nDelLineTo ){
			nWorkLen = pArg->nDelPosTo - nWorkPos;
		}else{
			nWorkLen = pCDocLine->m_pLine->m_nDataLen - nWorkPos;
		}
		if( 0 == nWorkLen ){
			/* �O�̍s�� */
			goto next_line;
		}
		if( 0 > nWorkLen ){
			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, "��҂ɋ����ė~�����G���[",
				"CDocLineMgr::ReplaceData()\n\n0 > nWorkLen\nnWorkLen=%d\ni=%d\npArg->nDelLineTo=%d", nWorkLen, i, pArg->nDelLineTo
			);
		}

		/* ���s���폜����񂩂��̂��E�E�E�H */
		if( EOL_NONE != pCDocLine->m_cEol &&
			nWorkPos + nWorkLen > pCDocLine->m_pLine->m_nDataLen - pCDocLine->m_cEol.GetLen() 
		){
			/* �폜���钷���ɉ��s���܂߂� */
			nWorkLen = pCDocLine->m_pLine->m_nDataLen - nWorkPos;

		}
		/* �폜���ꂽ�f�[�^��ۑ� */
		if( NULL == pArg->pcmemDeleted->Append( &pCDocLine->m_pLine->m_pData[nWorkPos], nWorkLen ) ){
//		if( NULL == pArg->pcmemDeleted->m_pData ){
			/* �������m�ۂɎ��s���� */
			pArg->pcmemDeleted->SetDataSz( "" );
			break;
		}
next_line:;		
		/* ���̍s�̃I�u�W�F�N�g�̃|�C���^ */
		pCDocLine = pCDocLineNext;
		/* �ŋߎQ�Ƃ����s�ԍ��ƍs�f�[�^ */
		++m_nPrevReferLine;
		m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (i - pArg->nDelLineFrom) && ( 0 == ((i - pArg->nDelLineFrom) % 32)) ){
				nProgress = (i - pArg->nDelLineFrom) * 100 / (pArg->nDelLineTo - pArg->nDelLineFrom) / 2;
				::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );

//				wsprintf( szWork, "%d/%d", (i - pArg->nDelLineFrom), (pArg->nDelLineTo - pArg->nDelLineFrom) );
//				::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szWork );
//				/* �������̃��[�U�[������\�ɂ��� */
//				if( !::BlockingHook() ){
//					return;
//				}
//				/* ���f�{�^�������`�F�b�N */
//				if( pCDlgCancel->IsCanceled() ){
//					break;
//				}
			}
		}
	} // �폜�f�[�^�̎擾�̃��[�v

	
	
	
	/* ���ݍs�̏��𓾂� */
	pCDocLine = GetLineInfo( pArg->nDelLineTo );
	i = pArg->nDelLineTo;
	if( 0 < pArg->nDelLineTo && NULL == pCDocLine ){
		pCDocLine = GetLineInfo( pArg->nDelLineTo - 1 );
		i--;
	}
	/* ��납�珈�����Ă��� */
	for( ; i >= pArg->nDelLineFrom && NULL != pCDocLine; i-- ){
		pLine = pCDocLine->m_pLine->m_pData;
		nLineLen = pCDocLine->m_pLine->m_nDataLen;
		pCDocLinePrev = pCDocLine->m_pPrev;
		pCDocLineNext = pCDocLine->m_pNext;
		/* ���ݍs�̍폜�J�n�ʒu�𒲂ׂ� */
		if( i == pArg->nDelLineFrom ){
			nWorkPos = pArg->nDelPosFrom;
		}else{
			nWorkPos = 0;
		}
		/* ���ݍs�̍폜�f�[�^���𒲂ׂ� */
		if( i == pArg->nDelLineTo ){
//			/* �폜�J�n�ʒu�����s�̈ʒu�Ȃ� */
//			if( EOL_NONE != pCDocLine->m_cEol &&
	//				nWorkPos >= pCDocLine->m_pLine->m_nDataLen - pCDocLine->m_cEol.GetLen()
//			){
//				nWorkLen = pCDocLine->m_pLine->m_nDataLen - nWorkPos;
//			}else{
				nWorkLen = pArg->nDelPosTo - nWorkPos;
//			}
		}else{
			nWorkLen = pCDocLine->m_pLine->m_nDataLen - nWorkPos;
		}

		if( 0 == nWorkLen ){
			/* �O�̍s�� */
			goto prev_line;
		}
		/* ���s���폜����񂩂��̂��E�E�E�H */
		if( EOL_NONE != pCDocLine->m_cEol &&
			nWorkPos + nWorkLen > pCDocLine->m_pLine->m_nDataLen - pCDocLine->m_cEol.GetLen() 
		){
			/* �폜���钷���ɉ��s���܂߂� */
			nWorkLen = pCDocLine->m_pLine->m_nDataLen - nWorkPos;

		}

		/* �폜���ꂽ�f�[�^��ۑ� */
// 1999.12.23 �䖝�ł��Ȃ�����
//t		/* �x�����ǉ䖝���� */
//t		cmemWork = (*(pArg->pcmemDeleted));
//t		(*(pArg->pcmemDeleted)).SetData( &pCDocLine->m_pLine->m_pData[nWorkPos], nWorkLen );
//t		(*(pArg->pcmemDeleted)) += cmemWork;

//		/* �o�b�t�@�̐擪�Ƀf�[�^��}������ */
//		pArg->pcmemDeleted->InsertTop( &pCDocLine->m_pLine->m_pData[nWorkPos], nWorkLen );


		/* �s�S�̂̍폜 */
		if( nWorkLen >= pCDocLine->m_pLine->m_nDataLen ){
			/* �폜�����s�̑��� */
			++(pArg->nDeletedLineNum);
			/* �s�I�u�W�F�N�g�̍폜�A���X�g�ύX�A�s��-- */
			DeleteNode( pCDocLine );
			pCDocLine = NULL;
		}else
		/* ���̍s�ƘA������悤�ȍ폜 */
		if( nWorkPos + nWorkLen >= pCDocLine->m_pLine->m_nDataLen ){

			/* �s���f�[�^�폜 */
			pWork = new char[nWorkPos + 1];
			memcpy( pWork, pLine, nWorkPos );
			pCDocLine->m_pLine->SetData( pWork, nWorkPos );
			delete [] pWork;
			pWork = NULL;
			
			/* ���̍s������ */
			if( NULL != pCDocLineNext ){
				/* ���s�R�[�h�̏����X�V�@(���̍s������炤) */
				pCDocLine->m_cEol = pCDocLineNext->m_cEol;	/* ���s�R�[�h�̎�� */

				/* ���̍s�̃f�[�^���Ō�ɒǉ� */
				pLine2 = pCDocLineNext->m_pLine->m_pData;
				nLineLen2 = pCDocLineNext->m_pLine->m_nDataLen;
				pCDocLine->m_pLine->Append( pLine2, nLineLen2 );
				/* ���̍s �s�I�u�W�F�N�g�̍폜 */
				DeleteNode( pCDocLineNext );
				pCDocLineNext = NULL;
				/* �폜�����s�̑��� */
				++(pArg->nDeletedLineNum);
			}else{
				/* ���s�R�[�h�̏����X�V */
				pCDocLine->m_cEol.SetType( EOL_NONE );
			}
			pCDocLine->m_bModify = TRUE;	/* �ύX�t���O */
		}else{
		/* �s�������̍폜 */
			pWork = new char[pCDocLine->m_pLine->m_nDataLen - nWorkLen + 1];
			memcpy( pWork, pLine, nWorkPos );
//			pWork[nWorkPos] = '\0';
//			MYTRACE( "pWork=[%s]\n", pWork );

			memcpy( &pWork[nWorkPos], &pLine[nWorkPos + nWorkLen], pCDocLine->m_pLine->m_nDataLen - ( nWorkPos + nWorkLen ) );
//			pWork[pCDocLine->m_pLine->m_nDataLen - nWorkLen] = '\0';
//			MYTRACE( "pWork=[%s]\n", pWork );

			pCDocLine->m_pLine->SetData( pWork, pCDocLine->m_pLine->m_nDataLen - nWorkLen );
			delete [] pWork;
			pWork = NULL;
			pCDocLine->m_bModify = TRUE;	/* �ύX�t���O */
		}

prev_line:;		
		/* ���O�̍s�̃I�u�W�F�N�g�̃|�C���^ */
		pCDocLine = pCDocLinePrev;
		/* �ŋߎQ�Ƃ����s�ԍ��ƍs�f�[�^ */
		--m_nPrevReferLine;
		m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (pArg->nDelLineTo - i) && ( 0 == ((pArg->nDelLineTo - i) % 32)) ){
				nProgress = (pArg->nDelLineTo - i) * 100 / (pArg->nDelLineTo - pArg->nDelLineFrom) / 2 + 50;
				::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );

//				wsprintf( szWork, "%d/%d", (pArg->nDelLineTo - i), (pArg->nDelLineTo - pArg->nDelLineFrom) );
 //				::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szWork );
//				/* �������̃��[�U�[������\�ɂ��� */
//				if( !::BlockingHook() ){
//					return;
//				}
//				/* ���f�{�^�������`�F�b�N */
//				if( pCDlgCancel->IsCanceled() ){
//					break;
//				}
			}
		}
	}
//	MYTRACE( "\n\n����������������������������\n" );
//	MYTRACE( "(pArg->nDeletedLineNum)=%d\n", (pArg->nDeletedLineNum) );

//	/* ����̂ł�߂Ƃ� */
//	m_nPrevReferLine = 0;
//	m_pCodePrevRefer = NULL;

	
	
	
	/* �f�[�^�}������ */
	if( 0 == pArg->nInsDataLen ){
		goto end_of_func;
	}
	nAllLinesOld = m_nLines;
	pArg->nNewLine = pArg->nDelLineFrom;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
	pArg->nNewPos  = 0;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */

	/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
	nCount = 0;
	pArg->nInsLineNum = 0;
	pCDocLine = GetLineInfo( pArg->nDelLineFrom );



	if( NULL == pCDocLine ){
		/* ������NULL���A���Ă���Ƃ������Ƃ́A*/
		/* �S�e�L�X�g�̍Ō�̎��̍s��ǉ����悤�Ƃ��Ă��邱�Ƃ����� */
		cmemPrevLine.SetDataSz( "" );
		cmemNextLine.SetDataSz( "" );
		cEOLTypeNext.SetType( EOL_NONE );
		// ::MessageBox( NULL, "pDocLine==NULL","Warning",MB_OK);
	}else{
//		/* Undo���삩�ǂ��� */
//		if( bUndo ){
//			pCDocLine->m_nModifyCount--;	/* �ύX�� */
//			if( 0 == pCDocLine->m_nModifyCount ){	/* �ύX�� */
//				pCDocLine->m_bModify = FALSE;	/* �ύX�t���O */
//			}
//			if( 0 > pCDocLine->m_nModifyCount ){	/* �ύX�� */
//				::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, "��҂ɋ����ė~�����G���[",
//					"CDocLineMgr::InsertData()�ŕύX�J�E���^��0�ȉ��ɂȂ�܂����B�o�O���႟����"
//				);
//			}
//		}else{
//			++pCDocLine->m_nModifyCount;	/* �ύX�� */
//			pCDocLine->m_bModify = TRUE;	/* �ύX�t���O */
//		}
		pCDocLine->m_bModify = TRUE;	/* �ύX�t���O */

		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );
		cmemPrevLine.SetData( pLine, pArg->nDelPosFrom );
		cmemNextLine.SetData( &pLine[pArg->nDelPosFrom], nLineLen - pArg->nDelPosFrom );

		cEOLTypeNext = pCDocLine->m_cEol;
	}
	nBgn = 0;
	nPos = 0;
	for( nPos = 0; nPos < pArg->nInsDataLen; ){
		if( pArg->pInsData[nPos] == '\n' || pArg->pInsData[nPos] == '\r' ){
			/* �s�I�[�q�̎�ނ𒲂ׂ� */
			cEOLType.GetTypeFromString( &(pArg->pInsData[nPos]), pArg->nInsDataLen - nPos );
			/* �s�I�[�q���܂߂ăe�L�X�g���o�b�t�@�Ɋi�[ */
			cmemCurLine.SetData( &(pArg->pInsData[nBgn]), nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + cEOLType.GetLen();
			nPos = nBgn;
			if( NULL == pCDocLine){
				pCDocLineNew = new CDocLine;

				pCDocLineNew->m_pLine = new CMemory;
				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					if( NULL == m_pDocLineTop ){
						m_pDocLineTop = pCDocLineNew;
					}
					pCDocLineNew->m_pPrev = m_pDocLineBot;
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pCDocLineNew;
					}
					m_pDocLineBot = pCDocLineNew;
					pCDocLineNew->m_pNext = NULL;
					pCDocLineNew->m_pLine->SetData( &cmemPrevLine );
					*(pCDocLineNew->m_pLine) += cmemCurLine;

					pCDocLineNew->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */
					// pCDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLType];		/* ���s�R�[�h�̒��� */ 		
				}else{
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pCDocLineNew;
					}
					pCDocLineNew->m_pPrev = m_pDocLineBot;
					m_pDocLineBot = pCDocLineNew;
					pCDocLineNew->m_pNext = NULL;
					pCDocLineNew->m_pLine->SetData( &cmemCurLine );

					pCDocLineNew->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */
				}
				pCDocLine = NULL;
				++m_nLines;
			}else{
				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					pCDocLine->m_pLine->SetData( &cmemPrevLine );
					*(pCDocLine->m_pLine) += cmemCurLine;

					pCDocLine->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */
					//pCDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLType];		/* ���s�R�[�h�̒��� */ 		
					
					pCDocLine = pCDocLine->m_pNext;
				}else{
					pCDocLineNew = new CDocLine;
					pCDocLineNew->m_pLine = new CMemory;
					pCDocLineNew->m_pPrev = pCDocLine->m_pPrev;
					pCDocLineNew->m_pNext = pCDocLine;
					pCDocLine->m_pPrev->m_pNext = pCDocLineNew;
					pCDocLine->m_pPrev = pCDocLineNew;
					pCDocLineNew->m_pLine->SetData( &cmemCurLine );

					pCDocLineNew->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */
					//pCDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLType];		/* ���s�R�[�h�̒��� */ 		

					++m_nLines;
				}
			}

			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			++nCount;
			++(pArg->nNewLine);	/* �}�����ꂽ�����̎��̈ʒu�̍s */
			if( NULL != hwndCancel){
				if( 0 != pArg->nInsDataLen && ( 0 == (nPos % 1024)) ){
					nProgress = nPos * 100 / pArg->nInsDataLen;
					::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );
				}
			}

		}else{
			++nPos;
		}
		// ::MessageBox( NULL, cEOLType.GetName(),"cEOLTypeNext",MB_OK);
	}
//	nEOLType = EOL_NONE;
	if( 0 < nPos - nBgn || 0 < cmemNextLine.GetLength() ){
		cmemCurLine.SetData( &(pArg->pInsData[nBgn]), nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pCDocLine){
			pCDocLineNew = new CDocLine;
			pCDocLineNew->m_pLine = new CMemory;
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				if( NULL == m_pDocLineTop ){
					m_pDocLineTop = pCDocLineNew;
				}
				pCDocLineNew->m_pPrev = m_pDocLineBot;
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pCDocLineNew;
				}
				m_pDocLineBot = pCDocLineNew;
				pCDocLineNew->m_pNext = NULL;
				pCDocLineNew->m_pLine->SetData( &cmemPrevLine );
				*(pCDocLineNew->m_pLine) += cmemCurLine;

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */
				// pCDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];		/* ���s�R�[�h�̒��� */ 		

			}else{
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pCDocLineNew;
				}
				pCDocLineNew->m_pPrev = m_pDocLineBot;
				m_pDocLineBot = pCDocLineNew;
				pCDocLineNew->m_pNext = NULL;
				pCDocLineNew->m_pLine->SetData( &cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */
				//pCDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];		/* ���s�R�[�h�̒��� */ 		

			}
			pCDocLine = NULL;
			++m_nLines;
			pArg->nNewPos = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
		}else{
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				pCDocLine->m_pLine->SetData( &cmemPrevLine );
				*(pCDocLine->m_pLine) += cmemCurLine;

				pCDocLine->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */
				//pCDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];		/* ���s�R�[�h�̒��� */ 		

				pCDocLine = pCDocLine->m_pNext;
				pArg->nNewPos = cmemPrevLine.GetLength() + nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}else{
				pCDocLineNew = new CDocLine;
				pCDocLineNew->m_pLine = new CMemory;
				pCDocLineNew->m_pPrev = pCDocLine->m_pPrev;
				pCDocLineNew->m_pNext = pCDocLine;
				pCDocLine->m_pPrev->m_pNext = pCDocLineNew;
				pCDocLine->m_pPrev = pCDocLineNew;
				pCDocLineNew->m_pLine->SetData( &cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */
				//pCDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];		/* ���s�R�[�h�̒��� */ 		


				++m_nLines;
				pArg->nNewPos = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}
		}
		// ::MessageBox( NULL, pCDocLine->m_cEol.GetName(),"cEOLTypeNext",MB_OK);
		// ::MessageBox( NULL, pCDocLine->m_pNext->m_cEol.GetName(),"cEOLTypeNext",MB_OK);
		// ::MessageBox( NULL, pCDocLine->m_pPrev->m_cEol.GetName(),"cEOLTypeNext",MB_OK);
	}
	pArg->nInsLineNum = m_nLines - nAllLinesOld;
end_of_func:;
//	pCDlgCancel->CloseDialog( 0 );	 
	if( NULL != pCDlgCancel ){
		delete pCDlgCancel;		
	}
	return;
}



