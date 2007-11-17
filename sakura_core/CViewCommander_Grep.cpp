#include "stdafx.h"
#include "CViewCommander.h"
#include "sakura_rc.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "CControlTray.h"
#include "charcode.h"
#include "CEditWnd.h"

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
//		cmemCurText.SetData( GetShareData()->m_aSearchKeys[0], lstrlen( GetShareData()->m_aSearchKeys[0] ) );
		cmemCurText.SetString( GetShareData()->m_aSearchKeys[0] );
	}
	wcscpy( GetEditWindow()->m_cDlgGrep.m_szText, cmemCurText.GetStringPtr() );

	/* Grep�_�C�A���O�̕\�� */
	int nRet = GetEditWindow()->m_cDlgGrep.DoModal( GetInstance(), m_pCommanderView->m_hWnd, GetDocument()->GetFilePath() );
//	MYTRACE_A( "nRet=%d\n", nRet );
	if( FALSE == nRet ){
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
	if( GetShareData()->m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
		TCHAR szMsg[512];
		auto_sprintf( szMsg, _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );
		::MessageBox( m_pCommanderView->m_hWnd, szMsg, GSTR_APPNAME, MB_OK );
		return;
	}
	cmWork1.SetString( GetEditWindow()->m_cDlgGrep.m_szText );
	cmWork2.SetString( GetEditWindow()->m_cDlgGrep.m_szFile );
	cmWork3.SetString( GetEditWindow()->m_cDlgGrep.m_szFolder );

	/*	����EditView��Grep���ʂ�\������B
		Grep���[�h�̂Ƃ��B�܂��́A�ύX�t���O���I�t�ŁA�t�@�C����ǂݍ���ł��Ȃ��ꍇ�B
	*/
	if( GetDocument()->m_bGrepMode ||
		( !GetDocument()->IsModified() &&
		  !GetDocument()->IsFilePathAvailable() )		/* ���ݕҏW���̃t�@�C���̃p�X */
	){
		m_pCommanderView->DoGrep(
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
		TCHAR*	pCmdLine = new TCHAR[1024];
		TCHAR*	pOpt = new TCHAR[64];
//		int		nDataLen;
		cmWork1.Replace( L"\"", L"\"\"" );
		cmWork2.Replace( _T("\""), _T("\"\"") );
		cmWork3.Replace( _T("\""), _T("\"\"") );
		/*
		|| -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		*/
		auto_sprintf(
			pCmdLine, _T("-GREPMODE -GKEY=\"%ls\" -GFILE=\"%ls\" -GFOLDER=\"%ts\" -GCODE=%d"),
			cmWork1.GetStringPtr(),
			cmWork2.GetStringPtr(),
			cmWork3.GetStringPtr(),
			GetEditWindow()->m_cDlgGrep.m_nGrepCharSet
		);
		pOpt[0] = _T('\0');
		if( GetEditWindow()->m_cDlgGrep.m_bSubFolder ){	/* �T�u�t�H���_������������� */
			_tcscat( pOpt, _T("S") );
		}
	//	if( m_bFromThisText ){	/* ���̕ҏW���̃e�L�X�g���猟������ */
	//
	//	}
		if( GetEditWindow()->m_cDlgGrep.m_sSearchOption.bWordOnly ){	/* �P��P�ʂŒT�� */
			_tcscat( pOpt, _T("W") );
		}
		if( GetEditWindow()->m_cDlgGrep.m_sSearchOption.bLoHiCase ){	/* �p�啶���Ɖp����������ʂ��� */
			_tcscat( pOpt, _T("L") );
		}
		if( GetEditWindow()->m_cDlgGrep.m_sSearchOption.bRegularExp ){	/* ���K�\�� */
			_tcscat( pOpt, _T("R") );
		}
//	2002/09/20 Moca �����R�[�h�Z�b�g�I�v�V�����ɓ���
//		if( GetEditWindow()->m_cDlgGrep.m_KanjiCode_AutoDetect ){	/* �����R�[�h�������� */
//			_tcscat( pOpt, _T("K") );
//		}
		if( GetEditWindow()->m_cDlgGrep.m_bGrepOutputLine ){	/* �s���o�͂��邩�Y�����������o�͂��邩 */
			_tcscat( pOpt, _T("P") );
		}
		if( 1 == GetEditWindow()->m_cDlgGrep.m_nGrepOutputStyle ){	/* Grep: �o�͌`�� */
			_tcscat( pOpt, _T("1") );
		}
		if( 2 == GetEditWindow()->m_cDlgGrep.m_nGrepOutputStyle ){	/* Grep: �o�͌`�� */
			_tcscat( pOpt, _T("2") );
		}
		if( 0 < _tcslen( pOpt ) ){
			_tcscat( pCmdLine, _T(" -GOPT=") );
			_tcscat( pCmdLine, pOpt );
		}
//		MYTRACE_A( "pCmdLine=[%ls]\n", pCmdLine );
		/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
		CControlTray::OpenNewEditor( GetInstance(), m_pCommanderView->m_hWnd, pCmdLine, CODE_DEFAULT, FALSE );
		delete [] pCmdLine;
		delete [] pOpt;
		/*======= Grep�̎��s =============*/
		/* Grep���ʃE�B���h�E�̕\�� */
	}
	return;
}
/*[EOF]*/
