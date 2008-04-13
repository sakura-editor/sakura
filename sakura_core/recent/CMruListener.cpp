#include "stdafx.h"
#include "CMruListener.h"
#include "doc/CEditDoc.h"
#include "util/file.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �Z�[�u�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CMruListener::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	_HoldBookmarks_And_AddToMRU();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���[�h�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
ECallbackResult CMruListener::OnCheckLoad(SLoadInfo* pLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();
	bool bConfirmCodeChange = GetDllShareData().m_Common.m_sFile.m_bQueryIfCodeChange;

	/* MRU���X�g�ɑ��݂��邩���ׂ�  ���݂���Ȃ�΃t�@�C������Ԃ� */
	CMRU		cMRU;
	BOOL		bIsExistInMRU;
	EditInfo	fi;
	if ( cMRU.GetEditInfo( pLoadInfo->cFilePath, &fi ) ){
		bIsExistInMRU = TRUE;

		// �O��Ɏw�肳�ꂽ�����R�[�h��ʂɕύX����
		if( CODE_NONE == pcDoc->m_cDocFile.m_sFileInfo.eCharCode ){
			pcDoc->m_cDocFile.m_sFileInfo.eCharCode = fi.m_nCharCode;
		}
		
		// �����R�[�h�w��̍ăI�[�v���Ȃ�O��𖳎�
		if( !bConfirmCodeChange && CODE_AUTODETECT==pcDoc->m_cDocFile.m_sFileInfo.eCharCode ){
			pcDoc->m_cDocFile.m_sFileInfo.eCharCode = fi.m_nCharCode;
		}

		// ���݂��Ȃ��t�@�C���̕����R�[�h�w��Ȃ��Ȃ�O����p��
		if( !fexist(pLoadInfo->cFilePath) && CODE_AUTODETECT==pcDoc->m_cDocFile.m_sFileInfo.eCharCode ){
			pcDoc->m_cDocFile.m_sFileInfo.eCharCode = fi.m_nCharCode;
		}
	}
	else {
		bIsExistInMRU = FALSE;
	}

	/* �����R�[�h���قȂ�Ƃ��Ɋm�F���� */
	if( bConfirmCodeChange && bIsExistInMRU ){
		if (pcDoc->m_cDocFile.m_sFileInfo.eCharCode != fi.m_nCharCode ) {	// MRU �̕����R�[�h�Ɣ��ʂ��قȂ�
			const TCHAR* pszCodeName = NULL;
			const TCHAR* pszCodeNameNew = NULL;

			// gm_pszCodeNameArr_Normal ���g���悤�ɕύX Moca. 2002/05/26
			if(IsValidCodeType(fi.m_nCharCode)){
				pszCodeName = CCodeTypeName(fi.m_nCharCode).Normal();
			}
			if(IsValidCodeType(pcDoc->m_cDocFile.m_sFileInfo.eCharCode)){
				pszCodeNameNew = CCodeTypeName(pcDoc->m_cDocFile.m_sFileInfo.eCharCode).Normal();
			}
			if( pszCodeName != NULL ){
				::MessageBeep( MB_ICONQUESTION );
				int nRet = MYMESSAGEBOX(
					CEditWnd::Instance()->GetHwnd(),
					MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
					_T("�����R�[�h���"),
					_T("%ts\n\n���̃t�@�C���́A�O��͕ʂ̕����R�[�h %ts �ŊJ����Ă��܂��B\n")
					_T("�O��Ɠ��������R�[�h���g���܂����H\n")
					_T("\n")
					_T("�E[�͂�(Y)]  ��%ts\n")
					_T("�E[������(N)]��%ts\n")
					_T("�E[�L�����Z��]���J���܂���"),
					pcDoc->m_cDocFile.GetFilePath(),
					pszCodeName,
					pszCodeName,
					pszCodeNameNew
				);
				if( IDYES == nRet ){
					/* �O��Ɏw�肳�ꂽ�����R�[�h��ʂɕύX���� */
					pcDoc->m_cDocFile.m_sFileInfo.eCharCode = fi.m_nCharCode;
				}
				else if( IDCANCEL == nRet ){
					pcDoc->m_cDocFile.m_sFileInfo.eCharCode = CODE_DEFAULT;
					//	Sep. 10, 2002 genta
					pcDoc->SetFilePathAndIcon( _T("") );
					return CALLBACK_INTERRUPT;
				}
			}else{
				PleaseReportToAuthor(
					CEditWnd::Instance()->GetHwnd(),
					_T("�o�O���႟�������I�I�I")
				);
				//	Sep. 10, 2002 genta
				pcDoc->SetFilePathAndIcon( _T("") );
				return CALLBACK_INTERRUPT;
			}
		}
	}
	return CALLBACK_CONTINUE;
}

void CMruListener::OnBeforeLoad(const SLoadInfo& sLoadInfo)
{
	// Mar. 30, 2003 genta �u�b�N�}�[�N�ۑ��̂���MRU�֓o�^
	_HoldBookmarks_And_AddToMRU();
}

void CMruListener::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	CMRU		cMRU;

	EditInfo	eiOld;
	bool bIsExistInMRU = cMRU.GetEditInfo(pcDoc->m_cDocFile.GetFilePath(),&eiOld);

	//�L�����b�g�ʒu�̕���
	if( bIsExistInMRU && GetDllShareData().m_Common.m_sFile.GetRestoreCurPosition() ){
		//�L�����b�g�ʒu�擾
		CLayoutPoint ptCaretPos;
		pcDoc->m_cLayoutMgr.LogicToLayout(eiOld.m_ptCursor, &ptCaretPos);

		//�r���[�擾
		CEditView& cView = pcDoc->m_pcEditWnd->GetActiveView();

		if( ptCaretPos.GetY2() >= pcDoc->m_cLayoutMgr.GetLineCount() ){
			//�t�@�C���̍Ō�Ɉړ�
			cView.GetCommander().HandleCommand( F_GOFILEEND, 0, 0, 0, 0, 0 );
		}
		else{
			cView.GetTextArea().SetViewTopLine( eiOld.m_nViewTopLine ); // 2001/10/20 novice
			cView.GetTextArea().SetViewLeftCol( eiOld.m_nViewLeftCol ); // 2001/10/20 novice
			// From Here Mar. 28, 2003 MIK
			// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤�ɁB
			const CDocLine *pTmpDocLine = pcDoc->m_cDocLineMgr.GetLine( eiOld.m_ptCursor.GetY2() );	// 2008.08.22 ryoji ���s�P�ʂ̍s�ԍ���n���悤�ɏC��
			if( pTmpDocLine ){
				if( pTmpDocLine->GetLengthWithoutEOL() < eiOld.m_ptCursor.x ) ptCaretPos.x--;
			}
			// To Here Mar. 28, 2003 MIK
			cView.GetCaret().MoveCursor( ptCaretPos, TRUE );
			cView.GetCaret().m_nCaretPosX_Prev = cView.GetCaret().GetCaretLayoutPos().GetX2();
		}
	}

	// �u�b�N�}�[�N����  // 2002.01.16 hor
	if( bIsExistInMRU ){
		if( GetDllShareData().m_Common.m_sFile.GetRestoreBookmarks() ){
			CBookmarkManager(&pcDoc->m_cDocLineMgr).SetBookMarks(eiOld.m_szMarkLines);
		}
	}
	else{
		wcscpy(eiOld.m_szMarkLines,L"");
	}

	// MRU���X�g�ւ̓o�^
	EditInfo	eiNew;
	pcDoc->GetEditInfo(&eiNew);
	cMRU.Add( &eiNew );
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �N���[�Y�O��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

ECallbackResult CMruListener::OnBeforeClose()
{
	//	Mar. 30, 2003 genta �T�u���[�`���ɂ܂Ƃ߂�
	_HoldBookmarks_And_AddToMRU();

	return CALLBACK_CONTINUE;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �w���p                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�J�����g�t�@�C����MRU�ɓo�^����B
	�u�b�N�}�[�N���ꏏ�ɓo�^����B

	@date 2003.03.30 genta �쐬

*/
void CMruListener::_HoldBookmarks_And_AddToMRU()
{
	//EditInfo�擾
	CEditDoc* pcDoc = GetListeningDoc();
	EditInfo	fi;
	pcDoc->GetEditInfo( &fi );

	//�u�b�N�}�[�N���̕ۑ�
	wcscpy( fi.m_szMarkLines, CBookmarkManager(&pcDoc->m_cDocLineMgr).GetBookMarks() );

	//MRU���X�g�ɓo�^
	CMRU	cMRU;
	cMRU.Add( &fi );
}

