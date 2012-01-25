#include "StdAfx.h"
#include "CViewCommander.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "CControlTray.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "charset/charcode.h"
#include "window/CEditWnd.h"
#include "CAppMode.h"
#include "sakura_rc.h"

/*! GREP�_�C�A���O�̕\��

	@date 2005.01.10 genta CEditView_Command���ړ�
	@author Yazaki
*/
void CViewCommander::Command_GREP_DIALOG( void )
{
	CNativeW	cmemCurText;

	/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	/* �L�[���Ȃ��Ȃ�A��������Ƃ��Ă��� */
	if( 0 == cmemCurText.GetStringLength() ){
//		cmemCurText.SetData( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0], lstrlen( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0] ) );
		cmemCurText.SetString( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0] );
	}
	wcscpy( GetEditWindow()->m_cDlgGrep.m_szText, cmemCurText.GetStringPtr() );

	/* Grep�_�C�A���O�̕\�� */
	int nRet = GetEditWindow()->m_cDlgGrep.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath() );
//	MYTRACE_A( "nRet=%d\n", nRet );
	if( !nRet ){
		return;
	}
	HandleCommand(F_GREP, TRUE, 0, 0, 0, 0);	//	GREP�R�}���h�̔��s
}

/*! GREP���s

	@date 2005.01.10 genta CEditView_Command���ړ�
*/
void CViewCommander::Command_GREP( void )
{
	CNativeW		cmWork1;
	CNativeT		cmWork2;
	CNativeT		cmWork3;

	/* �ҏW�E�B���h�E�̏���`�F�b�N */
	if( GetDllShareData().m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
		TCHAR szMsg[128];
		auto_sprintf( szMsg, _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );
		::MessageBox( m_pCommanderView->GetHwnd(), szMsg, GSTR_APPNAME, MB_OK );
		return;
	}
	cmWork1.SetString( GetEditWindow()->m_cDlgGrep.m_szText );
	cmWork2.SetString( GetEditWindow()->m_cDlgGrep.m_szFile );
	cmWork3.SetString( GetEditWindow()->m_cDlgGrep.m_szFolder );

	/*	����EditView��Grep���ʂ�\������B
		Grep���[�h�̂Ƃ��A�܂��͖��ҏW�Ŗ��肩�A�E�g�v�b�g�łȂ��ꍇ�B
		���E�B���h�E��Grep���s�����A(�ُ�I������̂�)�ʃE�B���h�E�ɂ���
	*/
	if( (  CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode &&
		  !CEditApp::Instance()->m_pcGrepAgent->m_bGrepRunning ) ||
		( !GetDocument()->m_cDocEditor.IsModified() &&
		  !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() &&		/* ���ݕҏW���̃t�@�C���̃p�X */
		  !CAppMode::Instance()->IsDebugMode()
		)
	){
		CEditApp::Instance()->m_pcGrepAgent->DoGrep(
			m_pCommanderView,
			&cmWork1,
			&cmWork2,
			&cmWork3,
			GetEditWindow()->m_cDlgGrep.m_bSubFolder,
			GetEditWindow()->m_cDlgGrep.m_sSearchOption,
			GetEditWindow()->m_cDlgGrep.m_nGrepCharSet,
			GetEditWindow()->m_cDlgGrep.m_bGrepOutputLine,
			GetEditWindow()->m_cDlgGrep.m_nGrepOutputStyle
		);
	}
	else{
		/*======= Grep�̎��s =============*/
		/* Grep���ʃE�B���h�E�̕\�� */
		cmWork1.Replace( L"\"", L"\"\"" );
		cmWork2.Replace( _T("\""), _T("\"\"") );
		cmWork3.Replace( _T("\""), _T("\"\"") );

		// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		CCommandLineString cCmdLine;
		cCmdLine.AppendF(
			_T("-GREPMODE -GKEY=\"%ls\" -GFILE=\"%ts\" -GFOLDER=\"%ts\" -GCODE=%d"),
			cmWork1.GetStringPtr(),
			cmWork2.GetStringPtr(),
			cmWork3.GetStringPtr(),
			GetEditWindow()->m_cDlgGrep.m_nGrepCharSet
		);

		//GOPT�I�v�V����
		TCHAR	pOpt[64];
		pOpt[0] = _T('\0');
		if( GetEditWindow()->m_cDlgGrep.m_bSubFolder				)_tcscat( pOpt, _T("S") );	// �T�u�t�H���_�������������
		if( GetEditWindow()->m_cDlgGrep.m_sSearchOption.bWordOnly	)_tcscat( pOpt, _T("W") );	// �P��P�ʂŒT��
		if( GetEditWindow()->m_cDlgGrep.m_sSearchOption.bLoHiCase	)_tcscat( pOpt, _T("L") );	// �p�啶���Ɖp����������ʂ���
		if( GetEditWindow()->m_cDlgGrep.m_sSearchOption.bRegularExp	)_tcscat( pOpt, _T("R") );	// ���K�\��
		if( GetEditWindow()->m_cDlgGrep.m_bGrepOutputLine			)_tcscat( pOpt, _T("P") );	// �s���o�͂��邩�Y�����������o�͂��邩
		if( 1 == GetEditWindow()->m_cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("1") );	// Grep: �o�͌`��
		if( 2 == GetEditWindow()->m_cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("2") );	// Grep: �o�͌`��
		if( 0 < _tcslen( pOpt ) ){
			cCmdLine.AppendF( _T(" -GOPT=%ts"), pOpt );
		}

//		MYTRACE_A( "pCmdLine=[%ls]\n", pCmdLine );
		/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = _T("");
		sLoadInfo.eCharCode = CODE_NONE;
		sLoadInfo.bViewMode = false;
		CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo, cCmdLine.c_str() );
		/*======= Grep�̎��s =============*/
		/* Grep���ʃE�B���h�E�̕\�� */
	}
	return;
}

