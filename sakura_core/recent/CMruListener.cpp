#include "StdAfx.h"
#include "CMruListener.h"
#include "recent/CMRUFile.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "view/CEditView.h"
#include "charset/CCodeMediator.h"
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
void CMruListener::OnBeforeLoad(SLoadInfo* pLoadInfo)
{
	// �ă��[�h�p�Ɍ��݃t�@�C����MRU�o�^���Ă���
	// Mar. 30, 2003 genta �u�b�N�}�[�N�ۑ��̂���MRU�֓o�^
	_HoldBookmarks_And_AddToMRU();	// �� �V�K�I�[�v���i�t�@�C�������ݒ�j�ł͉������Ȃ�

	CEditDoc* pcDoc = GetListeningDoc();

	// �����R�[�h�w��͖����I�ł��邩
	bool bSpecified = IsValidCodeType(pLoadInfo->eCharCode);

	// �O��̃R�[�h -> ePrevCode
	EditInfo	fi;
	ECodeType ePrevCode = CODE_NONE;
	CTypeConfig nPrevType = CTypeConfig(-1);
	if(CMRUFile().GetEditInfo( pLoadInfo->cFilePath, &fi )){
		ePrevCode = fi.m_nCharCode;
		nPrevType = fi.m_nType;
	}

	// �^�C�v�ʐݒ�
	if( !pLoadInfo->nType.IsValid() ){
		if( !nPrevType.IsValid() ){
			pLoadInfo->nType = CDocTypeManager().GetDocumentTypeOfPath( pLoadInfo->cFilePath );
		}else{
			pLoadInfo->nType = nPrevType;
		}
	}


	// �w��̃R�[�h -> pLoadInfo->eCharCode
	if( CODE_AUTODETECT == pLoadInfo->eCharCode ){
		if( fexist(pLoadInfo->cFilePath) ){
			// �f�t�H���g�����R�[�h�F���̂��߂Ɉꎞ�I�ɓǂݍ��ݑΏۃt�@�C���̃t�@�C���^�C�v��K�p����
			const STypeConfig& type = CDocTypeManager().GetTypeSetting(pLoadInfo->nType);
			CCodeMediator cmediator( type.m_encoding );
			pLoadInfo->eCharCode = cmediator.CheckKanjiCodeOfFile( pLoadInfo->cFilePath );
		}
		else{
			pLoadInfo->eCharCode = ePrevCode;
		}
	}
	else if( CODE_NONE == pLoadInfo->eCharCode ){
		pLoadInfo->eCharCode = ePrevCode;
	}
	if(CODE_NONE==pLoadInfo->eCharCode)
		pLoadInfo->eCharCode = pLoadInfo->nType.GetTypeConfig()->m_encoding.m_eDefaultCodetype;	//�����l�̉��	// 2011.01.24 ryoji CODE_DEFAULT -> m_eDefaultCodetype

	//�H���Ⴄ�ꍇ
	if(IsValidCodeType(ePrevCode) && pLoadInfo->eCharCode!=ePrevCode){
		//�I�v�V�����F�O��ƕ����R�[�h���قȂ�Ƃ��ɖ₢���킹���s��
		if( GetDllShareData().m_Common.m_sFile.m_bQueryIfCodeChange && !pLoadInfo->bRequestReload ){
			const TCHAR* pszCodeNameOld = CCodeTypeName(ePrevCode).Normal();
			const TCHAR* pszCodeNameNew = CCodeTypeName(pLoadInfo->eCharCode).Normal();
			ConfirmBeep();
			int nRet = MYMESSAGEBOX(
				CEditWnd::getInstance()->GetHwnd(),
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				_T("�����R�[�h���"),
				_T("%ts\n")
				_T("\n")
				_T("���̃t�@�C���𕶎��R�[�h %ts �ŊJ�����Ƃ��Ă��܂����A�O��͕ʂ̕����R�[�h %ts �ŊJ����Ă��܂��B\n")
				_T("�O��Ɠ��������R�[�h���g���܂����H\n")
				_T("\n")
				_T("�E[�͂�(Y)]  ��%ts\n")
				_T("�E[������(N)]��%ts"),
				pLoadInfo->cFilePath.c_str(),
				pszCodeNameNew,
				pszCodeNameOld,
				pszCodeNameOld,
				pszCodeNameNew
			);
			if( IDYES == nRet ){
				// �O��̕����R�[�h���̗p����
				pLoadInfo->eCharCode = ePrevCode;
			}
			else{
				// ���X�g�����Ƃ��Ă��������R�[�h���̗p����
				pLoadInfo->eCharCode = pLoadInfo->eCharCode;
			}
		}
		//�H������Ă��₢���킹���s��Ȃ��ꍇ
		else{
			//�f�t�H���g�̉�
			//  �������ʂ̏ꍇ�F�O��̕����R�[�h���̗p
			//  �����w��̏ꍇ�F�����w��̕����R�[�h���̗p
			if(!bSpecified){ //��������
				pLoadInfo->eCharCode = ePrevCode;
			}
			else{ //�����w��
				pLoadInfo->eCharCode = pLoadInfo->eCharCode;
			}
		}
	}
}


void CMruListener::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	CMRUFile		cMRU;

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
			cView.GetCommander().HandleCommand( F_GOFILEEND, false, 0, 0, 0, 0 );
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
			cView.GetCaret().MoveCursor( ptCaretPos, true );
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
	CMRUFile	cMRU;
	cMRU.Add( &fi );
}

