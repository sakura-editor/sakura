//	$Id$
/*!	@file
	@brief �s�f�[�^�̊Ǘ�

	@author Norio Nakatani
	@date 1999/12/17  �V�K�쐬
	@date 2001/12/03  hor Bookmark�@�\�ǉ�
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, hor, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "charcode.h"
#include "CDocLineMgr.h"
#include "debug.h"
#include "charcode.h"
#include <io.h>
#include <commctrl.h>
#include "global.h"
#include "etc_uty.h"
#include "CRunningTimer.h"
#include "CDlgCancel.h"
#include "CDocLine.h"// 2002/2/10 aroka
#include "CMemory.h"// 2002/2/10 aroka
#include "CBregexp.h"// 2002/2/10 aroka
#include "sakura_rc.h"// 2002/2/10 aroka


/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
  From�̈ʒu�փe�L�X�g��}������
*/
void CDocLineMgr::ReplaceData( DocLineReplaceArg* pArg )
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
// 2002/2/10 aroka ���g�p
//	char* pLine2;
//	int nLineLen2;
	int i;
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
	CDlgCancel*	pCDlgCancel = NULL;
	HWND		hwndCancel;
	HWND		hwndProgress;

	pArg->nNewLine = pArg->nDelLineFrom;
	pArg->nNewPos =  pArg->nDelPosFrom;

	/* ��ʂ̃f�[�^�𑀍삷��Ƃ� */
	if( 3000 < pArg->nDelLineTo - pArg->nDelLineFrom
	 || 1024000 < pArg->nInsDataLen
	){

		/* �i���_�C�A���O�̕\�� */
		pCDlgCancel = new CDlgCancel;
		if( NULL != ( hwndCancel = pCDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
			::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
 			::SendMessage( hwndProgress, PBM_SETPOS, 0, 0 );
		}
	}


	// �폜�f�[�^�̎擾�̃��[�v
	/* �O���珈�����Ă��� */
	/* ���ݍs�̏��𓾂� */
	pCDocLine = GetLineInfo( pArg->nDelLineFrom );
	for( i = pArg->nDelLineFrom; i <= pArg->nDelLineTo && NULL != pCDocLine; i++ ){
		pLine = pCDocLine->m_pLine->GetPtr(); // 2002/2/10 aroka CMemory�ύX
		nLineLen = pCDocLine->m_pLine->GetLength(); // 2002/2/10 aroka CMemory�ύX
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
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
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
			nWorkPos + nWorkLen > nLineLen - pCDocLine->m_cEol.GetLen() // 2002/2/10 aroka CMemory�ύX
		){
			/* �폜���钷���ɉ��s���܂߂� */
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}
		/* �폜���ꂽ�f�[�^��ۑ� */
		// 2002/2/10 aroka from here CMemory�ύX �O�̂��߁B
		if( pLine != pCDocLine->m_pLine->GetPtr() ){
			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, "��҂ɋ����ė~�����G���[",
				"CDocLineMgr::ReplaceData()\n\npLine != pCDocLine->m_pLine->GetPtr() =%d\ni=%d\npArg->nDelLineTo=%d", pLine, i, pArg->nDelLineTo
			);
		}
		// 2002/2/10 aroka to here CMemory�ύX
		if( NULL == pArg->pcmemDeleted->Append( &pLine[nWorkPos], nWorkLen ) ){
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
		pLine = pCDocLine->m_pLine->GetPtr(); // 2002/2/10 aroka CMemory�ύX
		nLineLen = pCDocLine->m_pLine->GetLength(); // 2002/2/10 aroka CMemory�ύX
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
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}

		if( 0 == nWorkLen ){
			/* �O�̍s�� */
			goto prev_line;
		}
		/* ���s���폜����񂩂��̂��E�E�E�H */
		if( EOL_NONE != pCDocLine->m_cEol &&
			nWorkPos + nWorkLen > nLineLen - pCDocLine->m_cEol.GetLen() // 2002/2/10 aroka CMemory�ύX
		){
			/* �폜���钷���ɉ��s���܂߂� */
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}


		/* �s�S�̂̍폜 */
		if( nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory�ύX
			/* �폜�����s�̑��� */
			++(pArg->nDeletedLineNum);
			/* �s�I�u�W�F�N�g�̍폜�A���X�g�ύX�A�s��-- */
			DeleteNode( pCDocLine );
			pCDocLine = NULL;
		}else
		/* ���̍s�ƘA������悤�ȍ폜 */
		if( nWorkPos + nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory�ύX

			/* �s���f�[�^�폜 */
			{// 20020119 aroka �u���b�N���� pWork ������߂�
				char* pWork = new char[nWorkPos + 1];
				memcpy( pWork, pLine, nWorkPos ); // 2002/2/10 aroka ���x�� GetPtr ���Ă΂Ȃ�
				pCDocLine->m_pLine->SetData( pWork, nWorkPos );
				delete [] pWork;
			}

			/* ���̍s������ */
			if( NULL != pCDocLineNext ){
				/* ���s�R�[�h�̏����X�V (���̍s������炤) */
				pCDocLine->m_cEol = pCDocLineNext->m_cEol;	/* ���s�R�[�h�̎�� */

				/* ���̍s�̃f�[�^���Ō�ɒǉ� */
				// 2002/2/10 aroka ���� CMemory �� Append
				//pLine2 = pCDocLineNext->m_pLine->GetPtr();
				//nLineLen2 = pCDocLineNext->m_pLine->GetLength();
				pCDocLine->m_pLine->Append( pCDocLineNext->m_pLine );
				/* ���̍s �s�I�u�W�F�N�g�̍폜 */
				DeleteNode( pCDocLineNext );
				pCDocLineNext = NULL;
				/* �폜�����s�̑��� */
				++(pArg->nDeletedLineNum);
			}else{
				/* ���s�R�[�h�̏����X�V */
				pCDocLine->m_cEol.SetType( EOL_NONE );
			}
			pCDocLine->SetModifyFlg(true);	/* �ύX�t���O */
		}else{
		/* �s�������̍폜 */
			{// 20020119 aroka �u���b�N���� pWork ������߂�
				// 2002/2/10 aroka CMemory�ύX ���x�� GetLength,GetPtr ����΂Ȃ��B
				int nLength = pCDocLine->m_pLine->GetLength();
				char* pWork = new char[nLength - nWorkLen + 1];
				memcpy( pWork, pLine, nWorkPos );

				memcpy( &pWork[nWorkPos], &pLine[nWorkPos + nWorkLen], nLength - ( nWorkPos + nWorkLen ) );

				pCDocLine->m_pLine->SetData( pWork, nLength - nWorkLen );
				delete [] pWork;
			}
			pCDocLine->SetModifyFlg(true);	/* �ύX�t���O */
		}

prev_line:;
		/* ���O�̍s�̃I�u�W�F�N�g�̃|�C���^ */
		pCDocLine = pCDocLinePrev;
		/* �ŋߎQ�Ƃ����s�ԍ��ƍs�f�[�^ */
		--m_nPrevReferLine;
		m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (pArg->nDelLineTo - i) && ( 0 == ((pArg->nDelLineTo - i) % 32) ) ){
				nProgress = (pArg->nDelLineTo - i) * 100 / (pArg->nDelLineTo - pArg->nDelLineFrom) / 2 + 50;
				::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );
			}
		}
	}


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
		pCDocLine->SetModifyFlg(true);	/* �ύX�t���O */

		// 2002/2/10 aroka ���x�� GetPtr ���Ă΂Ȃ�
		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );
		cmemPrevLine.SetData( pLine, pArg->nDelPosFrom );
		cmemNextLine.SetData( &pLine[pArg->nDelPosFrom], nLineLen - pArg->nDelPosFrom );

		cEOLTypeNext = pCDocLine->m_cEol;
	}
	nBgn = 0;
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

					++m_nLines;
				}
			}

			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			++nCount;
			++(pArg->nNewLine);	/* �}�����ꂽ�����̎��̈ʒu�̍s */
			if( NULL != hwndCancel ){
				if( 0 != pArg->nInsDataLen && ( 0 == (nPos % 1024) ) ){
					nProgress = nPos * 100 / pArg->nInsDataLen;
					::SendMessage( hwndProgress, PBM_SETPOS, nProgress, 0 );
				}
			}

		}else{
			++nPos;
		}
	}
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

			}else{
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pCDocLineNew;
				}
				pCDocLineNew->m_pPrev = m_pDocLineBot;
				m_pDocLineBot = pCDocLineNew;
				pCDocLineNew->m_pNext = NULL;
				pCDocLineNew->m_pLine->SetData( &cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */
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

				++m_nLines;
				pArg->nNewPos = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}
		}
	}
	pArg->nInsLineNum = m_nLines - nAllLinesOld;
end_of_func:;
	if( NULL != pCDlgCancel ){
		delete pCDlgCancel;
	}
	return;
}


//!�u�b�N�}�[�N�̑S����
/*
	@date 2001.12.03 hor
*/
void CDocLineMgr::ResetAllBookMark( void )
{
	CDocLine* pDocLine;
	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine ){
		pDocLine->SetBookMark(false);
		pDocLine = pDocLine->m_pNext;
	}
	return;
}


//! �u�b�N�}�[�N����
/*
	@date 2001.12.03 hor
*/
int CDocLineMgr::SearchBookMark(
	int			nLineNum,		/* �����J�n�s */
	int			bPrevOrNext,	/* 0==�O������ 1==������� */
	int*		pnLineNum 		/* �}�b�`�s */
)
{
	CDocLine*	pDocLine;
	int			nLinePos=nLineNum;

	/* 0==�O������ 1==������� */
	if( 0 == bPrevOrNext ){
		nLinePos--;
		pDocLine = GetLineInfo( nLinePos );
		while( NULL != pDocLine ){
			if(pDocLine->IsBookMarked()){
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return TRUE;
			}
			nLinePos--;
			pDocLine = pDocLine->m_pPrev;
		}
	}else{
		nLinePos++;
		pDocLine = GetLineInfo( nLinePos );
		while( NULL != pDocLine ){
			if(pDocLine->IsBookMarked()){
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return TRUE;
			}
			nLinePos++;
			pDocLine = pDocLine->m_pNext;
		}
	}
	return FALSE;
}


//! ���������ɊY������s�Ƀu�b�N�}�[�N���Z�b�g����
/*
	@date 2002.01.16 hor
*/
void CDocLineMgr::MarkSearchWord(
	const char*	pszPattern,		/* �������� */
	int			bRegularExp,	/* 1==���K�\�� */
	int			bLoHiCase,		/* 1==�p�啶���������̋�� */
	int			bWordOnly,		/* 1==�P��̂݌��� */
	CBregexp*	pRegexp			/*!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������ */
)
{
	CDocLine*	pDocLine;
	const char*	pLine;
	int			nLineLen;
	char*		pszRes;
	int*		pnKey_CharCharsArr;
	int			nPatternLen = lstrlen( pszPattern );

	/* 1==���K�\�� */
	if( bRegularExp ){
		BREGEXP* pRegexpData;
		pDocLine = GetLineInfo( 0 );
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked()){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				if( pRegexp->GetMatchInfo( pLine, nLineLen, 0, &pRegexpData ) ){
					pDocLine->SetBookMark(true);
				}
			}
			pDocLine = pDocLine->m_pNext;
		}
	}else
	/* 1==�P��̂݌��� */
	if( bWordOnly ){
		pDocLine = GetLineInfo( 0 );
		int nLinePos = 0;
		int nNextWordFrom = 0;
		int nNextWordFrom2;
		int nNextWordTo2;
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked() &&
				WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL )) {
				const char* pData = pDocLine->m_pLine->GetPtr(); // 2002/2/10 aroka CMemory�ύX
				
				if(( nPatternLen == nNextWordTo2 - nNextWordFrom2 ) &&
				   (( FALSE == bLoHiCase && 0 == _memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )) ||
					( TRUE  == bLoHiCase && 0 ==   memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )))){
					pDocLine->SetBookMark(true);
				}else
				if( PrevOrNextWord( nLinePos, nNextWordFrom, &nNextWordFrom, FALSE, FALSE) ){
					continue;
				}
			}
			/* ���̍s�����ɍs�� */
			nLinePos++;
			pDocLine = pDocLine->m_pNext;
			nNextWordFrom = 0;
		}
	}else{
		/* ���������̏�� */
		pnKey_CharCharsArr = NULL;
		CDocLineMgr::CreateCharCharsArr(
			(const unsigned char *)pszPattern,
			lstrlen( pszPattern ),
			&pnKey_CharCharsArr
		);
		pDocLine = GetLineInfo( 0 );
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked()){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				pszRes = SearchString(
					(const unsigned char *)pLine,
					nLineLen,
					0,
					(const unsigned char *)pszPattern,
					lstrlen( pszPattern ),
					pnKey_CharCharsArr,
					bLoHiCase
				);
				if( NULL != pszRes ){
					pDocLine->SetBookMark(true);
				}
			}
			pDocLine = pDocLine->m_pNext;
		}
		if( NULL != pnKey_CharCharsArr ){
			delete [] pnKey_CharCharsArr;
			pnKey_CharCharsArr = NULL;
		}
	}
	return;

}

//! �����s�ԍ��̃��X�g����܂Ƃ߂čs�}�[�N
/*
	@date 2002.01.16 hor
*/
void CDocLineMgr::SetBookMarks( char* pMarkLines )
{
	CDocLine*	pCDocLine;
	char *p;
	char delim[] = ", ";
	p = pMarkLines;
	while(strtok(p, delim) != NULL) {
		while(strchr(delim, *p) != NULL)p++;
		pCDocLine=GetLineInfo( atol(p) );
		if(NULL!=pCDocLine)pCDocLine->SetBookMark(true);
		p += strlen(p) + 1;
	}
	return;
}


//! �s�}�[�N����Ă镨���s�ԍ��̃��X�g�����
/*
	@date 2002.01.16 hor
*/
char* CDocLineMgr::GetBookMarks( void )
{
	CDocLine*	pCDocLine;
	char szText[MAX_MARKLINES_LEN + 1];	//2002.01.17
	char szBuff[10];
	int	nLinePos=0;
	pCDocLine = GetLineInfo( nLinePos );
	strcpy( szText, "" );
	while( NULL != pCDocLine ){
		if(pCDocLine->IsBookMarked()){
			wsprintf( szBuff, "%d,",nLinePos );
			if(lstrlen(szBuff)+lstrlen(szText)>MAX_MARKLINES_LEN)break;	//2002.01.17
			strcat( szText, szBuff);
		}
		nLinePos++;
		pCDocLine = pCDocLine->m_pNext;
	}
	return ((char*)"%s",szText);
}

/*[EOF]*/
