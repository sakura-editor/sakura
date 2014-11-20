/*!	@file
@brief CViewCommander�N���X�̃R�}���h(Grep)�֐��Q

*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "_main/CControlTray.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "plugin/CPlugin.h"
#include "plugin/CJackManager.h"

/*! GREP�_�C�A���O�̕\��

	@date 2005.01.10 genta CEditView_Command���ړ�
	@author Yazaki
*/
void CViewCommander::Command_GREP_DIALOG( void )
{
	CNativeW	cmemCurText;
	// 2014.07.01 ����Grep�E�B���h�E���g�������Ă���ꍇ�Ȃǂɉe�����Ȃ��悤�ɁA���ݒ�̂Ƃ�����History������
	bool bGetHistory = GetEditWindow()->m_cDlgGrep.m_bSetText == false;

	/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	bool bSet = m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText, bGetHistory );	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	if( bSet ){
		GetEditWindow()->m_cDlgGrep.m_strText = cmemCurText.GetStringPtr();
		GetEditWindow()->m_cDlgGrep.m_bSetText = true;
	}

	/* Grep�_�C�A���O�̕\�� */
	int nRet = GetEditWindow()->m_cDlgGrep.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath() );
//	MYTRACE( _T("nRet=%d\n"), nRet );
	if( !nRet ){
		return;
	}
	HandleCommand(F_GREP, true, 0, 0, 0, 0);	//	GREP�R�}���h�̔��s
}

/*! GREP���s

	@date 2005.01.10 genta CEditView_Command���ړ�
*/
void CViewCommander::Command_GREP( void )
{
	CNativeW		cmWork1;
	CNativeT		cmWork2;
	CNativeT		cmWork3;
	CNativeW		cmWork4;

	cmWork1.SetString( GetEditWindow()->m_cDlgGrep.m_strText.c_str() );
	cmWork2.SetString( GetEditWindow()->m_cDlgGrep.m_szFile );
	cmWork3.SetString( GetEditWindow()->m_cDlgGrep.m_szFolder );

	/*	����EditView��Grep���ʂ�\������B
		Grep���[�h�̂Ƃ��A�܂��͖��ҏW�Ŗ��肩�A�E�g�v�b�g�łȂ��ꍇ�B
		���E�B���h�E��Grep���s�����A(�ُ�I������̂�)�ʃE�B���h�E�ɂ���
	*/
	if( (  CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode &&
		  !CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning ) ||
		( !GetDocument()->m_cDocEditor.IsModified() &&
		  !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() &&		/* ���ݕҏW���̃t�@�C���̃p�X */
		  !CAppMode::getInstance()->IsDebugMode()
		)
	){
		// 2011.01.23 Grep�^�C�v�ʓK�p
		if( !GetDocument()->m_cDocEditor.IsModified() && GetDocument()->m_cDocLineMgr.GetLineCount() == 0 ){
			CTypeConfig cTypeGrep = CDocTypeManager().GetDocumentTypeOfExt( _T("grepout") );
			const STypeConfigMini* pConfig;
			CDocTypeManager().GetTypeConfigMini( cTypeGrep, &pConfig );
			GetDocument()->m_cDocType.SetDocumentTypeIdx( pConfig->m_id );
			GetDocument()->m_cDocType.LockDocumentType();
			GetDocument()->OnChangeType();
		}
		
		CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
			m_pCommanderView,
			false,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			false,
			GetEditWindow()->m_cDlgGrep.m_bSubFolder,
			false,
			true, // Header
			GetEditWindow()->m_cDlgGrep.m_sSearchOption,
			GetEditWindow()->m_cDlgGrep.m_nGrepCharSet,
			GetEditWindow()->m_cDlgGrep.m_bGrepOutputLine,
			GetEditWindow()->m_cDlgGrep.m_nGrepOutputStyle,
			GetEditWindow()->m_cDlgGrep.m_bGrepOutputFileOnly,
			GetEditWindow()->m_cDlgGrep.m_bGrepOutputBaseFolder,
			GetEditWindow()->m_cDlgGrep.m_bGrepSeparateFolder,
			false,
			false
		);

		//�v���O�C���FDocumentOpen�C�x���g���s
		CPlug::Array plugs;
		CWSHIfObj::List params;
		CJackManager::getInstance()->GetUsablePlug( PP_DOCUMENT_OPEN, 0, &plugs );
		for( CPlug::ArrayIter it = plugs.begin(); it != plugs.end(); it++ ){
			(*it)->Invoke(&GetEditWindow()->GetActiveView(), params);
		}
	}
	else{
		// �ҏW�E�B���h�E�̏���`�F�b�N
		if( GetDllShareData().m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
			OkMessage( m_pCommanderView->GetHwnd(), LS(STR_MAXWINDOW), MAX_EDITWINDOWS );
			return;
		}

		/*======= Grep�̎��s =============*/
		/* Grep���ʃE�B���h�E�̕\�� */
		CControlTray::DoGrepCreateWindow(G_AppInstance(), m_pCommanderView->GetHwnd(), GetEditWindow()->m_cDlgGrep);
	}
	return;
}

/*! GREP�u���_�C�A���O�̕\��
*/
void CViewCommander::Command_GREP_REPLACE_DLG( void )
{
	CNativeW	cmemCurText;
	CDlgGrepReplace& cDlgGrepRep = GetEditWindow()->m_cDlgGrepReplace;

	// ����Grep�E�B���h�E���g�������Ă���ꍇ�Ȃǂɉe�����Ȃ��悤�ɁA���ݒ�̂Ƃ�����History������
	bool bGetHistory = cDlgGrepRep.m_bSetText == false;

	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText, bGetHistory );

	if( 0 < cmemCurText.GetStringLength() ){
		cDlgGrepRep.m_strText = cmemCurText.GetStringPtr();
		cDlgGrepRep.m_bSetText = true;
	}
	if( 0 < GetDllShareData().m_sSearchKeywords.m_aReplaceKeys.size() ){
		if( cDlgGrepRep.m_nReplaceKeySequence < GetDllShareData().m_Common.m_sSearch.m_nReplaceKeySequence ){
			cDlgGrepRep.m_strText2 = GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0];
		}
	}

	int nRet = cDlgGrepRep.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath(), (LPARAM)m_pCommanderView );
	if( !nRet ){
		return;
	}
	HandleCommand(F_GREP_REPLACE, TRUE, 0, 0, 0, 0);	//	GREP�R�}���h�̔��s
}

/*! GREP�u�����s
*/
void CViewCommander::Command_GREP_REPLACE( void )
{
	CNativeW		cmWork1;
	CNativeT		cmWork2;
	CNativeT		cmWork3;
	CNativeW		cmWork4;

	CDlgGrepReplace& cDlgGrepRep = GetEditWindow()->m_cDlgGrepReplace;
	cmWork1.SetString( cDlgGrepRep.m_strText.c_str() );
	cmWork2.SetString( cDlgGrepRep.m_szFile );
	cmWork3.SetString( cDlgGrepRep.m_szFolder );
	cmWork4.SetString( cDlgGrepRep.m_strText2.c_str() );

	/*	����EditView��Grep���ʂ�\������B
		Grep���[�h�̂Ƃ��A�܂��͖��ҏW�Ŗ��肩�A�E�g�v�b�g�łȂ��ꍇ�B
		���E�B���h�E��Grep���s�����A(�ُ�I������̂�)�ʃE�B���h�E�ɂ���
	*/
	if( (  CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode &&
		  !CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning ) ||
		( !GetDocument()->m_cDocEditor.IsModified() &&
		  !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() &&		/* ���ݕҏW���̃t�@�C���̃p�X */
		  !CAppMode::getInstance()->IsDebugMode()
		)
	){
		CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
			m_pCommanderView,
			true,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			false,
			cDlgGrepRep.m_bSubFolder,
			false, // Stdout
			true, // Header
			cDlgGrepRep.m_sSearchOption,
			cDlgGrepRep.m_nGrepCharSet,
			cDlgGrepRep.m_bGrepOutputLine,
			cDlgGrepRep.m_nGrepOutputStyle,
			cDlgGrepRep.m_bGrepOutputFileOnly,
			cDlgGrepRep.m_bGrepOutputBaseFolder,
			cDlgGrepRep.m_bGrepSeparateFolder,
			cDlgGrepRep.m_bPaste,
			cDlgGrepRep.m_bBackup
		);
	}
	else{
		// �ҏW�E�B���h�E�̏���`�F�b�N
		if( GetDllShareData().m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
			OkMessage( m_pCommanderView->GetHwnd(), _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );
			return;
		}
		/*======= Grep�̎��s =============*/
		/* Grep���ʃE�B���h�E�̕\�� */
		cmWork1.Replace( L"\"", L"\"\"" );
		cmWork2.Replace( _T("\""), _T("\"\"") );
		cmWork3.Replace( _T("\""), _T("\"\"") );
		cmWork4.Replace( L"\"", L"\"\"" );

		// -GREPMODE -GKEY="1" -GREPR="2" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		CNativeT cCmdLine;
		TCHAR szTemp[20];
		cCmdLine.AppendString(_T("-GREPMODE -GKEY=\""));
		cCmdLine.AppendStringW(cmWork1.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GREPR=\""));
		cCmdLine.AppendStringW(cmWork4.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GFILE=\""));
		cCmdLine.AppendString(cmWork2.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GFOLDER=\""));
		cCmdLine.AppendString(cmWork3.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GCODE="));
		auto_sprintf( szTemp, _T("%d"), cDlgGrepRep.m_nGrepCharSet );
		cCmdLine.AppendString(szTemp);

		//GOPT�I�v�V����
		TCHAR	pOpt[64];
		pOpt[0] = _T('\0');
		if( cDlgGrepRep.m_bSubFolder				)_tcscat( pOpt, _T("S") );	// �T�u�t�H���_�������������
		if( cDlgGrepRep.m_sSearchOption.bWordOnly	)_tcscat( pOpt, _T("W") );	// �P��P�ʂŒT��
		if( cDlgGrepRep.m_sSearchOption.bLoHiCase	)_tcscat( pOpt, _T("L") );	// �p�啶���Ɖp����������ʂ���
		if( cDlgGrepRep.m_sSearchOption.bRegularExp	)_tcscat( pOpt, _T("R") );	// ���K�\��
		if( cDlgGrepRep.m_bGrepOutputLine			)_tcscat( pOpt, _T("P") );	// �s���o�͂��邩�Y�����������o�͂��邩
		if( 1 == cDlgGrepRep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("1") );	// Grep: �o�͌`��
		if( 2 == cDlgGrepRep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("2") );	// Grep: �o�͌`��
		if( 3 == cDlgGrepRep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("3") );
		if( cDlgGrepRep.m_bGrepOutputFileOnly		)_tcscat( pOpt, _T("F") );
		if( cDlgGrepRep.m_bGrepOutputBaseFolder		)_tcscat( pOpt, _T("B") );
		if( cDlgGrepRep.m_bGrepSeparateFolder		)_tcscat( pOpt, _T("D") );
		if( cDlgGrepRep.m_bPaste					)_tcscat( pOpt, _T("C") );	// �N���b�v�{�[�h����\��t��
		if( cDlgGrepRep.m_bBackup					)_tcscat( pOpt, _T("O") );	// �o�b�N�A�b�v�쐬
		if( 0 < _tcslen( pOpt ) ){
			cCmdLine.AppendString( _T(" -GOPT=") );
			cCmdLine.AppendString( pOpt );
		}

		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = _T("");
		sLoadInfo.eCharCode = CODE_NONE;
		sLoadInfo.bViewMode = false;
		CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo, cCmdLine.GetStringPtr(),
			false, NULL, GetDllShareData().m_Common.m_sTabBar.m_bNewWindow? true : false );
	}
	return;
}

