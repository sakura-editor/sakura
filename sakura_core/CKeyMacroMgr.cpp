//	$Id$
/*!	@file
	@brief �L�[�{�[�h�}�N��

	@author Norio Nakatani

	@date 20011229 aroka �o�O�C���A�R�����g�ǉ�
	YAZAKI �g�ւ�
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
//	#include <stdio.h>
//	#include <stdlib.h>
//	#include <malloc.h>
#include "CKeyMacroMgr.h"
#include "CMacro.h"
//	#include "debug.h"
#include "charcode.h"
//	#include "etc_uty.h"
//	#include "global.h"
//	#include "CEditView.h"

CKeyMacroMgr::CKeyMacroMgr()
{
	m_pTop = NULL;
	m_pBot = NULL;
	m_nKeyMacroDataArrNum = 0;
	m_nReady = FALSE;
	return;
}

CKeyMacroMgr::~CKeyMacroMgr()
{
	/* �L�[�}�N���̃o�b�t�@���N���A���� */
	ClearAll();
	return;
}


/*! �L�[�}�N���̃o�b�t�@���N���A���� */
void CKeyMacroMgr::ClearAll( void )
{
	CMacro* p = m_pTop;
	CMacro* del_p;
	while (p){
		del_p = p;
		p = p->GetNext();
		delete del_p;
	}
	m_nKeyMacroDataArrNum = 0;
	m_pTop = NULL;
	m_pBot = NULL;
	return;

}

/*! �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�
	�@�\�ԍ��ƁA�����ЂƂ�ǉ��ŁB
*/
void CKeyMacroMgr::Append( int nFuncID, LPARAM lParam1 )
{
	CMacro* macro = new CMacro( nFuncID );
	macro->AddLParam( lParam1 );
	Append(macro);
}

/*! �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�
	CMacro���w�肵�Ēǉ������
*/
void CKeyMacroMgr::Append( CMacro* macro )
{
	if (m_pTop){
		m_pBot->SetNext(macro);
		m_pBot = macro;
	}
	else {
		m_pTop = macro;
		m_pBot = m_pTop;
	}
	m_nKeyMacroDataArrNum++;
	return;
}



/*! �L�[�{�[�h�}�N���̕ۑ�
	�G���[���b�Z�[�W�͏o���܂���B�Ăяo�����ł悫�ɂ͂�����Ă��������B
*/
BOOL CKeyMacroMgr::SaveKeyMacro( HINSTANCE hInstance, const char* pszPath ) const
{
	HFILE		hFile;
	char		szLine[1024];
	CMemory		cmemWork;
	hFile = _lcreat( pszPath, 0 );
	if( HFILE_ERROR == hFile ){
		return FALSE;
	}
	strcpy( szLine, "//�L�[�{�[�h�}�N���̃t�@�C��\r\n" );
	_lwrite( hFile, szLine, strlen( szLine ) );
	CMacro* p = m_pTop;

	while (p){
		p->Save( hInstance, hFile );
		p = p->GetNext();
	}
	_lclose( hFile );
	return TRUE;
}



/*! �L�[�{�[�h�}�N���̎��s
	CMacro�ɈϏ��B
*/
void CKeyMacroMgr::ExecKeyMacro( CEditView* pcEditView ) const
{
	CMacro* p = m_pTop;
	while (p){
		p->Exec(pcEditView);
		p = p->GetNext();
	}
}

/*! �L�[�{�[�h�}�N���̓ǂݍ���
	�G���[���b�Z�[�W�͏o���܂���B�Ăяo�����ł悫�ɂ͂�����Ă��������B
*/
BOOL CKeyMacroMgr::LoadKeyMacro( HINSTANCE hInstance, const char* pszPath )
{
	/* �L�[�}�N���̃o�b�t�@���N���A���� */
	ClearAll();

	FILE* hFile = fopen( pszPath, "r" );
	if( NULL == hFile ){
		m_nReady = FALSE;
		return FALSE;
	}

	char	szFuncName[100];
	char	szFuncNameJapanese[256];
	int		nFuncID;
	int		i;
	int		nBgn, nEnd;
	CMemory cmemWork;
	CMacro* macro = NULL;

	// ��s���ǂ݂��݁A�R�����g�s��r��������ŁAmacro�R�}���h���쐬����B
	char	szLine[10240];
	while( NULL != fgets( szLine, sizeof(szLine), hFile ) ){
		int nLineLen = strlen( szLine );
		// ��s����󔒂��X�L�b�v
		for( i = 0; i < nLineLen; ++i ){
			if( szLine[i] != SPACE && szLine[i] != TAB ){
				break;
			}
		}
		nBgn = i;
		// �R�����g�s�̌��o
		//# �p�t�H�[�}���X�F'/'�̂Ƃ������Q�����ڂ��e�X�g
		if( szLine[nBgn] == '/' && nBgn + 1 < nLineLen && szLine[nBgn + 1] == '/' ){
			continue;
		}
		// �֐����̎擾
		szFuncName[0]='\0';// ������
		for( ; i < nLineLen; ++i ){
			//# �o�b�t�@�I�[�o�[�����`�F�b�N
			if( szLine[i] == '(' && (i - nBgn)< sizeof(szFuncName) ){
				memcpy( szFuncName, &szLine[nBgn], i - nBgn );
				szFuncName[i - nBgn] = '\0';
				++i;
				nBgn = i;
				break;
			}
		}

		/* �֐������@�\ID�C�@�\�����{�� */
		nFuncID = CMacro::GetFuncInfoByName( hInstance, szFuncName, szFuncNameJapanese );
		if( -1 != nFuncID ){
			macro = new CMacro( nFuncID );
			//	Skip Space
			while (szLine[i]) {
				while( szLine[i] == ' ' || szLine[i] == '\t' )
					i++;

				if( '\"' == szLine[i] ){	//	"�Ŏn�܂����當���񂾂悫���ƁB
					++i;
					nBgn = i;	//	nBgn�͈����̐擪�̕���
					for( ; i < nLineLen; ++i ){		//	�Ō�̕����܂ŃX�L����
						if( szLine[i] == '\\' ){	// �G�X�P�[�v�̃X�L�b�v
							++i;
							continue;
						}
						if( szLine[i] == '\"' ){	//	\"�ŏI���B
							nEnd = i;	//	nEnd�͏I���̎��̕����i"�j
							break;
						}
					}
					cmemWork.SetData( szLine + nBgn, nEnd - nBgn );
					cmemWork.Replace( "\\\"", "\"" );
					cmemWork.Replace( "\\\\", "\\" );
					macro->AddParam( cmemWork.GetPtr( NULL ) );	//	�����𕶎���Ƃ��Ēǉ�
				}
				else if ( '0' <= szLine[i] && szLine[i] <= '9' ){	//	�����Ŏn�܂����琔���񂾁B
					nBgn = i;	//	nBgn�͈����̐擪�̕���
					for( ; i < nLineLen; ++i ){		//	�Ō�̕����܂ŃX�L����
						if( '0' <= szLine[i] && szLine[i] <= '9' ){	// �G�X�P�[�v�̃X�L�b�v
							++i;
							continue;
						}
						else {
							nEnd = i;	//	�I���̎��̕����i��������Ȃ������j
							break;
						}
					}
					macro->AddParam( atoi(&szLine[nBgn]) );	//	�����𐔒l�Ƃ��Ēǉ�
				}
				else {
					//	Parse Error:���@�G���[���ۂ��B
					nBgn = nEnd = i;
				}

				for( ; i < nLineLen; ++i ){		//	�Ō�̕����܂ŃX�L����
					if( szLine[i] == ')' || szLine[i] == ',' ){	//	,��������)��ǂݔ�΂�
						i++;
						break;
					}
				}
				if (szLine[i-1] == ')'){
					break;
				}
			}
			/* �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ� */
			Append( macro );
		}
	}
	fclose( hFile );

	m_nReady = TRUE;
	return TRUE;
}


/*[EOF]*/
