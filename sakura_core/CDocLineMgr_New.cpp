/*!	@file
	@brief �s�f�[�^�̊Ǘ�

	@author Norio Nakatani
	@date 1999/12/17  �V�K�쐬
	@date 2001/12/03  hor Bookmark�@�\�ǉ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, hor, aroka, MIK, Moca, genta
	Copyright (C) 2003, Moca, genta
	Copyright (C) 2005, �����

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "charcode.h"
#include "CDocLineMgr.h"
#include "debug.h"
#include "charcode.h"
#include <commctrl.h>
#include "global.h"
#include "CRunningTimer.h"
#include "CDlgCancel.h"
#include "CDocLine.h"// 2002/2/10 aroka
#include "CMemory.h"// 2002/2/10 aroka
#include "CBregexp.h"// 2002/2/10 aroka
#include "sakura_rc.h"// 2002/2/10 aroka
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
  From�̈ʒu�փe�L�X�g��}������
*/
void CDocLineMgr::ReplaceData( DocLineReplaceArg* pArg )
{
	MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::ReplaceData()" );

	/* �}���ɂ���đ������s�̐� */
	pArg->nInsLineNum = CLogicInt(0);
	/* �폜�����s�̑��� */
	pArg->nDeletedLineNum = CLogicInt(0);
	/* �폜���ꂽ�f�[�^ */
	pArg->pcmemDeleted->SetString(L"");

	CDocLine* pCDocLine;
	CDocLine* pCDocLinePrev;
	CDocLine* pCDocLineNext;
	CDocLine* pCDocLineNew;
	int nWorkPos;
	int nWorkLen;
	const wchar_t* pLine;
	int nLineLen;
	int i;
	int			nBgn;
	int			nPos;
	CLogicInt	nAllLinesOld;
	int			nCount;
	int			nProgress;
	CNativeW	cmemPrevLine;
	CNativeW	cmemCurLine;
	CNativeW	cmemNextLine;
	//	May 15, 2000
	CEOL cEOLType;
	CEOL cEOLTypeNext;
	HWND		hwndCancel = NULL;	//	������
	HWND		hwndProgress = NULL;	//	������

	pArg->ptNewPos = pArg->sDelRange.GetFrom();

	/* ��ʂ̃f�[�^�𑀍삷��Ƃ� */
	CDlgCancel*	pCDlgCancel = NULL;
	if( 3000 < pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y
	 || 1024000 < pArg->nInsDataLen
	){

		/* �i���_�C�A���O�̕\�� */
		pCDlgCancel = new CDlgCancel;
		if( NULL != ( hwndCancel = pCDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
			hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
			::SendMessageAny( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
 			::SendMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
		}
	}


	// �폜�f�[�^�̎擾�̃��[�v
	/* �O���珈�����Ă��� */
	/* ���ݍs�̏��𓾂� */
	pCDocLine = GetLineInfo( pArg->sDelRange.GetFrom().GetY2() );
	for( i = pArg->sDelRange.GetFrom().y; i <= pArg->sDelRange.GetTo().y && NULL != pCDocLine; i++ ){
		pLine = pCDocLine->m_cLine.GetStringPtr(); // 2002/2/10 aroka CMemory�ύX
		nLineLen = pCDocLine->m_cLine.GetStringLength(); // 2002/2/10 aroka CMemory�ύX
		pCDocLinePrev = pCDocLine->m_pPrev;
		pCDocLineNext = pCDocLine->m_pNext;
		/* ���ݍs�̍폜�J�n�ʒu�𒲂ׂ� */
		if( i == pArg->sDelRange.GetFrom().y ){
			nWorkPos = pArg->sDelRange.GetFrom().x;
		}else{
			nWorkPos = 0;
		}
		/* ���ݍs�̍폜�f�[�^���𒲂ׂ� */
		if( i == pArg->sDelRange.GetTo().y ){
			nWorkLen = pArg->sDelRange.GetTo().x - nWorkPos;
		}else{
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}
		if( 0 == nWorkLen ){
			/* �O�̍s�� */
			goto next_line;
		}
		if( 0 > nWorkLen ){
			::MYMESSAGEBOX_A(	NULL, MB_OK | MB_ICONINFORMATION,
				"��҂ɋ����ė~�����G���[",
				"CDocLineMgr::ReplaceData()\n"
				"\n"
				"0 > nWorkLen\nnWorkLen=%d\n"
				"i=%d\n"
				"pArg->sDelRange.GetTo().y=%d", nWorkLen, i, pArg->sDelRange.GetTo().y
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
		if( pLine != pCDocLine->m_cLine.GetStringPtr() ){
			::MYMESSAGEBOX_A(	NULL, MB_OK | MB_ICONINFORMATION, "��҂ɋ����ė~�����G���[",
				"CDocLineMgr::ReplaceData()\n"
				"\n"
				"pLine != pCDocLine->m_cLine.GetPtr() =%d\n"
				"i=%d\n"
				"pArg->sDelRange.GetTo().y=%d",
				pLine, i, pArg->sDelRange.GetTo().y
			);
		}

		pArg->pcmemDeleted->AppendString( &pLine[nWorkPos], nWorkLen );

next_line:;
		/* ���̍s�̃I�u�W�F�N�g�̃|�C���^ */
		pCDocLine = pCDocLineNext;
		/* �ŋߎQ�Ƃ����s�ԍ��ƍs�f�[�^ */
		++m_nPrevReferLine;
		m_pCodePrevRefer = pCDocLine;

		if( NULL != hwndCancel){
			if( 0 != (i - pArg->sDelRange.GetFrom().y) && ( 0 == ((i - pArg->sDelRange.GetFrom().y) % 32)) ){
				nProgress = (i - pArg->sDelRange.GetFrom().y) * 100 / (pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y) / 2;
				::SendMessageAny( hwndProgress, PBM_SETPOS, nProgress, 0 );

			}
		}
	} // �폜�f�[�^�̎擾�̃��[�v


	/* ���ݍs�̏��𓾂� */
	pCDocLine = GetLineInfo( pArg->sDelRange.GetTo().GetY2() );
	i = pArg->sDelRange.GetTo().y;
	if( 0 < pArg->sDelRange.GetTo().y && NULL == pCDocLine ){
		pCDocLine = GetLineInfo( pArg->sDelRange.GetTo().GetY2() - CLogicInt(1) );
		i--;
	}
	/* ��납�珈�����Ă��� */
	for( ; i >= pArg->sDelRange.GetFrom().y && NULL != pCDocLine; i-- ){
		pLine = pCDocLine->m_cLine.GetStringPtr(); // 2002/2/10 aroka CMemory�ύX
		nLineLen = pCDocLine->m_cLine.GetStringLength(); // 2002/2/10 aroka CMemory�ύX
		pCDocLinePrev = pCDocLine->m_pPrev;
		pCDocLineNext = pCDocLine->m_pNext;
		/* ���ݍs�̍폜�J�n�ʒu�𒲂ׂ� */
		if( i == pArg->sDelRange.GetFrom().y ){
			nWorkPos = pArg->sDelRange.GetFrom().x;
		}else{
			nWorkPos = 0;
		}
		/* ���ݍs�̍폜�f�[�^���𒲂ׂ� */
		if( i == pArg->sDelRange.GetTo().y ){
			nWorkLen = pArg->sDelRange.GetTo().x - nWorkPos;
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
		}
		/* ���̍s�ƘA������悤�ȍ폜 */
		else if( nWorkPos + nWorkLen >= nLineLen ){ // 2002/2/10 aroka CMemory�ύX

			/* �s���f�[�^�폜 */
			{// 20020119 aroka �u���b�N���� pWork ������߂�
				wchar_t* pWork = new wchar_t[nWorkPos + 1];
				auto_memcpy( pWork, pLine, nWorkPos ); // 2002/2/10 aroka ���x�� GetPtr ���Ă΂Ȃ�
				pCDocLine->m_cLine.SetString( pWork, nWorkPos );
				delete [] pWork;
			}

			/* ���̍s������ */
			if( pCDocLineNext ){
				/* ���s�R�[�h�̏����X�V (���̍s������炤) */
				pCDocLine->m_cEol = pCDocLineNext->m_cEol;	/* ���s�R�[�h�̎�� */

				/* ���̍s�̃f�[�^���Ō�ɒǉ� */
				// 2002/2/10 aroka ���� CMemory �� Append
				//pLine2 = pCDocLineNext->m_cLine.GetPtr();
				//nLineLen2 = pCDocLineNext->m_cLine.GetLength();
				pCDocLine->m_cLine.AppendNativeData( pCDocLineNext->m_cLine );
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
				int nLength = pCDocLine->m_cLine.GetStringLength();
				wchar_t* pWork = new wchar_t[nLength - nWorkLen + 1];
				wmemcpy( pWork, pLine, nWorkPos );

				wmemcpy( &pWork[nWorkPos], &pLine[nWorkPos + nWorkLen], nLength - ( nWorkPos + nWorkLen ) );

				pCDocLine->m_cLine.SetString( pWork, nLength - nWorkLen );
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
			if( 0 != (pArg->sDelRange.GetTo().y - i) && ( 0 == ((pArg->sDelRange.GetTo().y - i) % 32) ) ){
				nProgress = (pArg->sDelRange.GetTo().y - i) * 100 / (pArg->sDelRange.GetTo().y - pArg->sDelRange.GetFrom().y) / 2 + 50;
				::SendMessageAny( hwndProgress, PBM_SETPOS, nProgress, 0 );
			}
		}
	}


	/* �f�[�^�}������ */
	if( 0 == pArg->nInsDataLen ){
		goto end_of_func;
	}
	nAllLinesOld = m_nLines;
	pArg->ptNewPos.y = pArg->sDelRange.GetFrom().y;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
	pArg->ptNewPos.x = 0;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */

	/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
	nCount = 0;
	pArg->nInsLineNum = CLogicInt(0);
	pCDocLine = GetLineInfo( pArg->sDelRange.GetFrom().GetY2() );



	if( NULL == pCDocLine ){
		/* ������NULL���A���Ă���Ƃ������Ƃ́A*/
		/* �S�e�L�X�g�̍Ō�̎��̍s��ǉ����悤�Ƃ��Ă��邱�Ƃ����� */
		cmemPrevLine.SetString(L"");
		cmemNextLine.SetString(L"");
		cEOLTypeNext.SetType( EOL_NONE );
		// ::MessageBoxA( NULL, "pDocLine==NULL","Warning",MB_OK);
	}else{
		pCDocLine->SetModifyFlg(true);	/* �ύX�t���O */

		// 2002/2/10 aroka ���x�� GetPtr ���Ă΂Ȃ�
		pLine = pCDocLine->m_cLine.GetStringPtr( &nLineLen );
		cmemPrevLine.SetString( pLine, pArg->sDelRange.GetFrom().x );
		cmemNextLine.SetString( &pLine[pArg->sDelRange.GetFrom().x], nLineLen - pArg->sDelRange.GetFrom().x );

		cEOLTypeNext = pCDocLine->m_cEol;
	}
	nBgn = 0;
	for( nPos = 0; nPos < pArg->nInsDataLen; ){
		if( pArg->pInsData[nPos] == '\n' || pArg->pInsData[nPos] == '\r' ){
			/* �s�I�[�q�̎�ނ𒲂ׂ� */
			cEOLType.GetTypeFromString( &(pArg->pInsData[nPos]), pArg->nInsDataLen - nPos );
			/* �s�I�[�q���܂߂ăe�L�X�g���o�b�t�@�Ɋi�[ */
			cmemCurLine.SetString( &(pArg->pInsData[nBgn]), nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + cEOLType.GetLen();
			nPos = nBgn;
			if( NULL == pCDocLine){
				pCDocLineNew = new CDocLine;

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
					pCDocLineNew->m_cLine.SetNativeData( cmemPrevLine );
					pCDocLineNew->m_cLine += cmemCurLine;

					pCDocLineNew->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */
				}else{
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pCDocLineNew;
					}
					pCDocLineNew->m_pPrev = m_pDocLineBot;
					m_pDocLineBot = pCDocLineNew;
					pCDocLineNew->m_pNext = NULL;
					pCDocLineNew->m_cLine.SetNativeData( cmemCurLine );

					pCDocLineNew->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */
				}
				pCDocLine = NULL;
				++m_nLines;
			}else{
				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					pCDocLine->m_cLine.SetNativeData( cmemPrevLine );
					pCDocLine->m_cLine += cmemCurLine;

					pCDocLine->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */
					pCDocLine = pCDocLine->m_pNext;
				}else{
					pCDocLineNew = new CDocLine;
					pCDocLineNew->m_pPrev = pCDocLine->m_pPrev;
					pCDocLineNew->m_pNext = pCDocLine;
					pCDocLine->m_pPrev->m_pNext = pCDocLineNew;
					pCDocLine->m_pPrev = pCDocLineNew;
					pCDocLineNew->m_cLine.SetNativeData( cmemCurLine );

					pCDocLineNew->m_cEol = cEOLType;	/* ���s�R�[�h�̎�� */

					++m_nLines;
				}
			}

			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			++nCount;
			++(pArg->ptNewPos.y);	/* �}�����ꂽ�����̎��̈ʒu�̍s */
			if( NULL != hwndCancel ){
				if( 0 != pArg->nInsDataLen && ( 0 == (nPos % 1024) ) ){
					nProgress = nPos * 100 / pArg->nInsDataLen;
					::SendMessageAny( hwndProgress, PBM_SETPOS, nProgress, 0 );
				}
			}

		}else{
			++nPos;
		}
	}
	if( 0 < nPos - nBgn || 0 < cmemNextLine.GetStringLength() ){
		cmemCurLine.SetString( &(pArg->pInsData[nBgn]), nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pCDocLine){
			pCDocLineNew = new CDocLine;
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
				pCDocLineNew->m_cLine.SetNativeData( cmemPrevLine );
				pCDocLineNew->m_cLine += cmemCurLine;

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */

			}else{
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pCDocLineNew;
				}
				pCDocLineNew->m_pPrev = m_pDocLineBot;
				m_pDocLineBot = pCDocLineNew;
				pCDocLineNew->m_pNext = NULL;
				pCDocLineNew->m_cLine.SetNativeData( cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */
			}
			pCDocLine = NULL;
			++m_nLines;
			pArg->ptNewPos.x = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
		}else{
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				pCDocLine->m_cLine.SetNativeData( cmemPrevLine );
				pCDocLine->m_cLine += cmemCurLine;

				pCDocLine->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */

				pCDocLine = pCDocLine->m_pNext;
				pArg->ptNewPos.x = cmemPrevLine.GetStringLength() + nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}else{
				pCDocLineNew = new CDocLine;
				pCDocLineNew->m_pPrev = pCDocLine->m_pPrev;
				pCDocLineNew->m_pNext = pCDocLine;
				pCDocLine->m_pPrev->m_pNext = pCDocLineNew;
				pCDocLine->m_pPrev = pCDocLineNew;
				pCDocLineNew->m_cLine.SetNativeData( cmemCurLine );

				pCDocLineNew->m_cEol = cEOLTypeNext;	/* ���s�R�[�h�̎�� */

				++m_nLines;
				pArg->ptNewPos.x = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
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
	CLogicInt			nLineNum,		/* �����J�n�s */
	ESearchDirection	bPrevOrNext,	/* 0==�O������ 1==������� */
	CLogicInt*			pnLineNum 		/* �}�b�`�s */
)
{
	CDocLine*	pDocLine;
	CLogicInt	nLinePos=nLineNum;

	//�O������
	if( bPrevOrNext == SEARCH_BACKWARD ){
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
	}
	//�������
	else{
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
	const wchar_t*	pszPattern,			//!< ��������
	const SSearchOption& sSearchOption,	//!< �����I�v�V����
	/*
	bool			bRegularExp,		// 1==���K�\��
	bool			bLoHiCase,			// 1==�p�啶���������̋��
	bool			bWordOnly,			// 1==�P��̂݌���
	*/
	CBregexp*		pRegexp				//!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
)
{
	CDocLine*	pDocLine;
	const wchar_t*	pLine;
	int			nLineLen;
	const wchar_t*	pszRes;
	int*		pnKey_CharCharsArr;
	//	Jun. 10, 2003 Moca
	//	wcslen�𖈉�Ă΂���nPatternLen���g���悤�ɂ���
	const int	nPatternLen = wcslen( pszPattern );

	/* 1==���K�\�� */
	if( sSearchOption.bRegularExp ){
		pDocLine = GetLineInfo( CLogicInt(0) );
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked()){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				// 2005.03.19 ����� �O����v�T�|�[�g�̂��߂̃��\�b�h�ύX
				if( pRegexp->Match( pLine, nLineLen, 0 ) ){
					pDocLine->SetBookMark(true);
				}
			}
			pDocLine = pDocLine->m_pNext;
		}
	}
	/* 1==�P��̂݌��� */
	else if( sSearchOption.bWordOnly ){
		pDocLine = GetLineInfo( CLogicInt(0) );
		CLogicInt nLinePos = CLogicInt(0);
		CLogicInt nNextWordFrom = CLogicInt(0);
		CLogicInt nNextWordFrom2;
		CLogicInt nNextWordTo2;
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked() &&
				WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL )) {
				const wchar_t* pData = pDocLine->m_cLine.GetStringPtr(); // 2002/2/10 aroka CMemory�ύX
				
				if(( nPatternLen == nNextWordTo2 - nNextWordFrom2 ) &&
				   (( !sSearchOption.bLoHiCase && 0 ==  _memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )) ||
					( sSearchOption.bLoHiCase && 0 == auto_memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen )))){
					pDocLine->SetBookMark(true);
				}
				else if( PrevOrNextWord( nLinePos, nNextWordFrom, &nNextWordFrom, FALSE, FALSE) ){
					continue;
				}
			}
			/* ���̍s�����ɍs�� */
			nLinePos++;
			pDocLine = pDocLine->m_pNext;
			nNextWordFrom = CLogicInt(0);
		}
	}
	else{
		/* ���������̏�� */
		pnKey_CharCharsArr = NULL;
		CDocLineMgr::CreateCharCharsArr(
			pszPattern,
			nPatternLen,
			&pnKey_CharCharsArr
		);
		pDocLine = GetLineInfo( CLogicInt(0) );
		while( NULL != pDocLine ){
			if(!pDocLine->IsBookMarked()){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				pszRes = SearchString(
					pLine,
					nLineLen,
					0,
					pszPattern,
					nPatternLen,
					pnKey_CharCharsArr,
					sSearchOption.bLoHiCase
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
void CDocLineMgr::SetBookMarks( wchar_t* pMarkLines )
{
	CDocLine*	pCDocLine;
	wchar_t *p;
	wchar_t delim[] = L", ";
	p = pMarkLines;
	while(wcstok(p, delim) != NULL) {
		while(wcschr(delim, *p) != NULL)p++;
		pCDocLine=GetLineInfo( CLogicInt(_wtol(p)) );
		if(NULL!=pCDocLine)pCDocLine->SetBookMark(true);
		p += wcslen(p) + 1;
	}
	return;
}


//! �s�}�[�N����Ă镨���s�ԍ��̃��X�g�����
/*
	@date 2002.01.16 hor
*/
wchar_t* CDocLineMgr::GetBookMarks( void )
{
	CDocLine*	pCDocLine;
	static wchar_t szText[MAX_MARKLINES_LEN + 1];	//2002.01.17 // Feb. 17, 2003 genta static��
	wchar_t szBuff[10];
	CLogicInt	nLinePos=CLogicInt(0);
	pCDocLine = GetLineInfo( nLinePos );
	wcscpy( szText, L"" );
	while( NULL != pCDocLine ){
		if(pCDocLine->IsBookMarked()){
			auto_sprintf( szBuff, L"%d,",nLinePos );
			if(wcslen(szBuff)+wcslen(szText)>MAX_MARKLINES_LEN)break;	//2002.01.17
			wcscat( szText, szBuff);
		}
		nLinePos++;
		pCDocLine = pCDocLine->m_pNext;
	}
	return szText; // Feb. 17, 2003 genta
}

/*!	�����\���̑S����
	@author	MIK
	@date	2002.05.25
*/
void CDocLineMgr::ResetAllDiffMark( void )
{
	CDocLine* pDocLine;

	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine )
	{
		pDocLine->SetDiffMark( 0 );
		pDocLine = pDocLine->m_pNext;
	}

	m_bIsDiffUse = false;

	return;
}

/*! ��������
	@author	MIK
	@date	2002.05.25
*/
int CDocLineMgr::SearchDiffMark(
	CLogicInt			nLineNum,		//!< �����J�n�s
	ESearchDirection	bPrevOrNext,	//!< 0==�O������ 1==�������
	CLogicInt*			pnLineNum 		//!< �}�b�`�s
)
{
	CDocLine*	pDocLine;
	CLogicInt	nLinePos = nLineNum;

	//�O������
	if( bPrevOrNext == SEARCH_BACKWARD )
	{
		nLinePos--;
		pDocLine = GetLineInfo( nLinePos );
		while( NULL != pDocLine )
		{
			if( pDocLine->IsDiffMarked() )
			{
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return TRUE;
			}
			nLinePos--;
			pDocLine = pDocLine->m_pPrev;
		}
	}
	//�������
	else
	{
		nLinePos++;
		pDocLine = GetLineInfo( nLinePos );
		while( NULL != pDocLine )
		{
			if( pDocLine->IsDiffMarked() )
			{
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return TRUE;
			}
			nLinePos++;
			pDocLine = pDocLine->m_pNext;
		}
	}
	return FALSE;
}

/*!	���������s�͈͎w��œo�^����B
	@author	MIK
	@date	2002/05/25
*/
void CDocLineMgr::SetDiffMarkRange( int nMode, CLogicInt nStartLine, CLogicInt nEndLine )
{
	CDocLine	*pCDocLine;

	m_bIsDiffUse = true;

	if( nStartLine < CLogicInt(0) ) nStartLine = CLogicInt(0);

	//�ŏI�s����ɍ폜�s����
	CLogicInt	nLines = GetLineCount();
	if( nLines <= nEndLine )
	{
		nEndLine = nLines - CLogicInt(1);
		pCDocLine = GetLineInfo( nEndLine );
		if( NULL != pCDocLine ) pCDocLine->SetDiffMark( MARK_DIFF_DEL_EX );
	}

	//�s�͈͂Ƀ}�[�N������
	for( CLogicInt i = nStartLine; i <= nEndLine; i++ )
	{
		pCDocLine = GetLineInfo( i );
		if( NULL != pCDocLine ) pCDocLine->SetDiffMark( nMode );
	}

	return;
}

/*[EOF]*/
