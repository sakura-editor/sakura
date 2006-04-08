//	$Id$
/*!	@file
	@brief CEditView�N���X��grep�֘A�R�}���h�����n�֐��Q

	@author genta
	@date	2005/01/10 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "stdafx.h"
#include "sakura_rc.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "CEditApp.h"
#include "charcode.h"

/*!
	�R�}���h�R�[�h�̕ϊ�(grep mode��)
*/
void CEditView::TranslateCommand_grep(
	int&	nCommand,
	BOOL&	bRedraw,
	LPARAM&	lparam1,
	LPARAM&	lparam2,
	LPARAM&	lparam3,
	LPARAM&	lparam4
)
{
	if( ! m_pcEditDoc->m_bGrepMode )
		return;

	if( nCommand == F_CHAR ){
		//	Jan. 23, 2005 genta ��������Y��
		if(( lparam1 == CR || lparam1 == LF ) &&
			m_pShareData->m_Common.m_bGTJW_RETURN ){
			nCommand = F_TAGJUMP;
			lparam1 = (((SHORT)0x8000 & ::GetKeyState( VK_CONTROL )) != 0 );
		}
	}
}

/*! GREP�_�C�A���O�̕\��

	@date 2005.01.10 genta CEditView_Command���ړ�
	@author Yazaki
*/
void CEditView::Command_GREP_DIALOG( void )
{
	CMemory		cmemCurText;

	/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	GetCurrentTextForSearch( cmemCurText );

	/* �L�[���Ȃ��Ȃ�A��������Ƃ��Ă��� */
	if( 0 == cmemCurText.GetLength() ){
//		cmemCurText.SetData( m_pShareData->m_szSEARCHKEYArr[0], lstrlen( m_pShareData->m_szSEARCHKEYArr[0] ) );
		cmemCurText.SetDataSz( m_pShareData->m_szSEARCHKEYArr[0] );
	}
	strcpy( m_pcEditDoc->m_cDlgGrep.m_szText, cmemCurText.GetPtr() );

	/* Grep�_�C�A���O�̕\�� */
	int nRet = m_pcEditDoc->m_cDlgGrep.DoModal( m_hInstance, m_hWnd, m_pcEditDoc->GetFilePath() );
//	MYTRACE( "nRet=%d\n", nRet );
	if( FALSE == nRet ){
		return;
	}
	HandleCommand(F_GREP, TRUE, 0, 0, 0, 0);	//	GREP�R�}���h�̔��s
}

/*! GREP���s

	@date 2005.01.10 genta CEditView_Command���ړ�
*/
void CEditView::Command_GREP( void )
{
	CMemory		cmWork1;
	CMemory		cmWork2;
	CMemory		cmWork3;
	CMemory		cmemCurText;

	/* �ҏW�E�B���h�E�̏���`�F�b�N */
	if( m_pShareData->m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
		char szMsg[512];
		wsprintf( szMsg, "�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B", MAX_EDITWINDOWS );
		::MessageBox( m_hWnd, szMsg, GSTR_APPNAME, MB_OK );
		return;
	}
	cmWork1.SetDataSz( m_pcEditDoc->m_cDlgGrep.m_szText );
	cmWork2.SetDataSz( m_pcEditDoc->m_cDlgGrep.m_szFile );
	cmWork3.SetDataSz( m_pcEditDoc->m_cDlgGrep.m_szFolder );

	/*	����EditView��Grep���ʂ�\������B
		Grep���[�h�̂Ƃ��B�܂��́A�ύX�t���O���I�t�ŁA�t�@�C����ǂݍ���ł��Ȃ��ꍇ�B
	*/
	if( m_pcEditDoc->m_bGrepMode ||
		( !m_pcEditDoc->IsModified() &&
		  !m_pcEditDoc->IsFilePathAvailable() )		/* ���ݕҏW���̃t�@�C���̃p�X */
	){
		DoGrep(
			&cmWork1,
			&cmWork2,
			&cmWork3,
			m_pcEditDoc->m_cDlgGrep.m_bSubFolder,
			m_pcEditDoc->m_cDlgGrep.m_bLoHiCase,
			m_pcEditDoc->m_cDlgGrep.m_bRegularExp,
			m_pcEditDoc->m_cDlgGrep.m_nGrepCharSet,
			m_pcEditDoc->m_cDlgGrep.m_bGrepOutputLine,
			m_pcEditDoc->m_cDlgGrep.m_bWordOnly,
			m_pcEditDoc->m_cDlgGrep.m_nGrepOutputStyle
		);
	}else{
		/*======= Grep�̎��s =============*/
		/* Grep���ʃE�B���h�E�̕\�� */
		char*	pCmdLine = new char[1024];
		char*	pOpt = new char[64];
//		int		nDataLen;
		cmWork1.Replace( "\"", "\"\"" );
		cmWork2.Replace( "\"", "\"\"" );
		cmWork3.Replace( "\"", "\"\"" );
		/*
		|| -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		*/
		wsprintf( pCmdLine, "-GREPMODE -GKEY=\"%s\" -GFILE=\"%s\" -GFOLDER=\"%s\" -GCODE=%d",
			cmWork1.GetPtr(),
			cmWork2.GetPtr(),
			cmWork3.GetPtr(),
			m_pcEditDoc->m_cDlgGrep.m_nGrepCharSet
		);
		pOpt[0] = '\0';
		if( m_pcEditDoc->m_cDlgGrep.m_bSubFolder ){	/* �T�u�t�H���_������������� */
			strcat( pOpt, "S" );
		}
	//	if( m_bFromThisText ){	/* ���̕ҏW���̃e�L�X�g���猟������ */
	//
	//	}
		if( m_pcEditDoc->m_cDlgGrep.m_bWordOnly ){	/* �P��P�ʂŒT�� */
			strcat( pOpt, "W" );
		}
		if( m_pcEditDoc->m_cDlgGrep.m_bLoHiCase ){	/* �p�啶���Ɖp����������ʂ��� */
			strcat( pOpt, "L" );
		}
		if( m_pcEditDoc->m_cDlgGrep.m_bRegularExp ){	/* ���K�\�� */
			strcat( pOpt, "R" );
		}
//	2002/09/20 Moca �����R�[�h�Z�b�g�I�v�V�����ɓ���
//		if( m_pcEditDoc->m_cDlgGrep.m_KanjiCode_AutoDetect ){	/* �����R�[�h�������� */
//			strcat( pOpt, "K" );
//		}
		if( m_pcEditDoc->m_cDlgGrep.m_bGrepOutputLine ){	/* �s���o�͂��邩�Y�����������o�͂��邩 */
			strcat( pOpt, "P" );
		}
		if( 1 == m_pcEditDoc->m_cDlgGrep.m_nGrepOutputStyle ){	/* Grep: �o�͌`�� */
			strcat( pOpt, "1" );
		}
		if( 2 == m_pcEditDoc->m_cDlgGrep.m_nGrepOutputStyle ){	/* Grep: �o�͌`�� */
			strcat( pOpt, "2" );
		}
		if( 0 < lstrlen( pOpt ) ){
			strcat( pCmdLine, " -GOPT=" );
			strcat( pCmdLine, pOpt );
		}
//		MYTRACE( "pCmdLine=[%s]\n", pCmdLine );
		/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
		CEditApp::OpenNewEditor( m_hInstance, m_pShareData->m_hwndTray, pCmdLine, 0, FALSE );
		delete [] pCmdLine;
		delete [] pOpt;
		/*======= Grep�̎��s =============*/
		/* Grep���ʃE�B���h�E�̕\�� */
	}
	return;
}
/*[EOF]*/
