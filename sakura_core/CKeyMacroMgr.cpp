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

#include <stdio.h>
//	#include <stdlib.h>
//	#include <malloc.h>
#include <string.h>
#include "CKeyMacroMgr.h"
#include "CMacro.h"
#include "CSMacroMgr.h"// 2002/2/10 aroka
#include "debug.h"
#include "charcode.h"
#include "etc_uty.h" // Oct. 5, 2002 genta
//	#include "global.h"
//	#include "CEditView.h"
#include "CMemory.h"
#include "CMacroFactory.h"

CKeyMacroMgr::CKeyMacroMgr()
{
	m_pTop = NULL;
	m_pBot = NULL;
//	m_nKeyMacroDataArrNum = 0;	2002.2.2 YAZAKI
	//	Apr. 29, 2002 genta
	//	m_nReady��CMacroManagerBase��
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
//	m_nKeyMacroDataArrNum = 0;	2002.2.2 YAZAKI
	m_pTop = NULL;
	m_pBot = NULL;
	return;

}

/*! �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�
	�@�\�ԍ��ƁA�����ЂƂ�ǉ��ŁB
	@date 2002.2.2 YAZAKI pcEditView���n���悤�ɂ����B
*/
void CKeyMacroMgr::Append( int nFuncID, LPARAM lParam1, CEditView* pcEditView )
{
	CMacro* macro = new CMacro( nFuncID );
	macro->AddLParam( lParam1, pcEditView );
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
//	m_nKeyMacroDataArrNum++;	2002.2.2 YAZAKI
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
		m_nReady = false;
		return FALSE;
	}

	char	szFuncName[100];
	char	szFuncNameJapanese[256];
	int		nFuncID;
	int		i;
	int		nBgn, nEnd;
	CMemory cmemWork;
	CMacro* macro = NULL;

	//	Jun. 16, 2002 genta
	m_nReady = true;	//	�G���[�������false�ɂȂ�
	static const char MACRO_ERROR_TITLE[] = "Macro�ǂݍ��݃G���[";

	// ��s���ǂ݂��݁A�R�����g�s��r��������ŁAmacro�R�}���h���쐬����B
	char	szLine[10240];
	
	int line = 1;	//	�G���[���ɍs�ԍ���ʒm���邽�߁D1�n�܂�D
	for( ; NULL != fgets( szLine, sizeof(szLine), hFile ) ; ++line ){
		int nLineLen = strlen( szLine );
		// ��s����󔒂��X�L�b�v
		for( i = 0; i < nLineLen; ++i ){
			//	Jun. 16, 2002 genta '\r' �ǉ�
			if( szLine[i] != SPACE && szLine[i] != TAB && szLine[i] != '\r' ){
				break;
			}
		}
		nBgn = i;
		// �R�����g�s�̌��o
		//# �p�t�H�[�}���X�F'/'�̂Ƃ������Q�����ڂ��e�X�g
		if( szLine[nBgn] == '/' && nBgn + 1 < nLineLen && szLine[nBgn + 1] == '/' ){
			continue;
		}
		//	Jun. 16, 2002 genta ��s�𖳎�����
		if( szLine[nBgn] == '\n' || szLine[nBgn] == '\0' ){
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
		// �֐�����S_���t���Ă�����

		/* �֐������@�\ID�C�@�\�����{�� */
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
//		nFuncID = CMacro::GetFuncInfoByName( hInstance, szFuncName, szFuncNameJapanese );
		nFuncID = CSMacroMgr::GetFuncInfoByName( hInstance, szFuncName, szFuncNameJapanese );
		if( -1 != nFuncID ){
			macro = new CMacro( nFuncID );
			// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N�p�ɒǉ�
			int nArgs;
			const MacroFuncInfo* mInfo= CSMacroMgr::GetFuncInfoByID( nFuncID );
			//	Skip Space
			for(nArgs = 0; szLine[i] ; ++nArgs ) {
				// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
				if( nArgs >= sizeof( mInfo->m_varArguments ) / sizeof( mInfo->m_varArguments[0] )){
					::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
						_T("Line %d: Column %d: �������������܂�\n" ), line, i + 1 );
					m_nReady = false;
				}

				while( szLine[i] == ' ' || szLine[i] == '\t' )
					i++;

				//@@@ 2002.2.2 YAZAKI PPA.DLL�}�N���ɂ��킹�Ďd�l�ύX�B�������''�ň͂ށB
				//	Jun. 16, 2002 genta double quotation�����e����
				if( '\'' == szLine[i] || '\"' == szLine[i]  ){	//	'�Ŏn�܂����當���񂾂悫���ƁB
					// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
					// Jun. 27, 2002 genta �]���Ȉ����𖳎�����悤�CVT_EMPTY�����e����D
					if( mInfo->m_varArguments[nArgs] != VT_BSTR && 
						mInfo->m_varArguments[nArgs] != VT_EMPTY ){
						::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
							_T("Line %d: Column %d\r\n"
							"�֐�%s��%d�Ԗڂ̈����ɕ�����͒u���܂���D" ), line, i + 1, szFuncName, nArgs + 1 );
						m_nReady = false;
						break;
					}
					int cQuote = szLine[i];
					++i;
					nBgn = i;	//	nBgn�͈����̐擪�̕���
					//	Jun. 16, 2002 genta
					//	�s���̌��o�̂��߁C���[�v�񐔂�1���₵��
					for( ; i <= nLineLen; ++i ){		//	�Ō�̕���+1�܂ŃX�L����
						if( _IS_SJIS_1( (unsigned char)szLine[i] ) ){
							++i;
							continue;
						}
						if( szLine[i] == '\\' ){	// �G�X�P�[�v�̃X�L�b�v
							++i;
							continue;
						}
						if( szLine[i] == cQuote ){	//	�n�܂�Ɠ���quotation�ŏI���B
							nEnd = i;	//	nEnd�͏I���̎��̕����i'�j
							break;
						}
						if( szLine[i] == '\0' ){	//	�s���ɗ��Ă��܂���
							::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
								_T("Line %d:\r\n�֐�%s��%d�Ԗڂ̈����̏I����%c������܂���D" ),
								line, szFuncName, nArgs + 1, cQuote);
							m_nReady = false;
							nEnd = i - 1;	//	nEnd�͏I���̎��̕����i'�j
							break;
						}
					}
					//	Jun. 16, 2002 genta
					if( !m_nReady ){
						break;
					}
					cmemWork.SetData( szLine + nBgn, nEnd - nBgn );
					cmemWork.Replace( "\\\'", "\'" );

					//	Jun. 16, 2002 genta double quotation���G�X�P�[�v����
					cmemWork.Replace( "\\\"", "\"" );
					cmemWork.Replace( "\\\\", "\\" );
					macro->AddParam( cmemWork.GetPtr() );	//	�����𕶎���Ƃ��Ēǉ�
				}
				else if ( '0' <= szLine[i] && szLine[i] <= '9' ){	//	�����Ŏn�܂����琔���񂾁B
					// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
					// Jun. 27, 2002 genta �]���Ȉ����𖳎�����悤�CVT_EMPTY�����e����D
					if( mInfo->m_varArguments[nArgs] != VT_I4 && 
						mInfo->m_varArguments[nArgs] != VT_EMPTY){
						::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
							_T("Line %d: Column %d\r\n"
							"�֐�%s��%d�Ԗڂ̈����ɐ��l�͒u���܂���D" ), line, i + 1, szFuncName, nArgs + 1);
						m_nReady = false;
						break;
					}
					nBgn = i;	//	nBgn�͈����̐擪�̕���
					for( ; i < nLineLen; ++i ){		//	�Ō�̕����܂ŃX�L����
						if( '0' <= szLine[i] && szLine[i] <= '9' ){	// �܂����l
//							++i;
							continue;
						}
						else {
							nEnd = i;	//	�����̍Ō�̕���
							i--;
							break;
						}
					}
					cmemWork.SetData( szLine + nBgn, nEnd - nBgn );
					// Jun. 16, 2002 genta
					//	�����̒���quotation�͓����Ă��Ȃ���
					//cmemWork.Replace( "\\\'", "\'" );
					//cmemWork.Replace( "\\\\", "\\" );
					macro->AddParam( cmemWork.GetPtr() );	//	�����𕶎���Ƃ��Ēǉ�
				}
				//	Jun. 16, 2002 genta
				else if( szLine[i] == ')' ){
					//	��������
					break;
				}
				else {
					//	Parse Error:���@�G���[���ۂ��B
					//	Jun. 16, 2002 genta
					nBgn = nEnd = i;
					::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
						_T("Line %d: Column %d: Syntax Error\n" ), line, i );
					m_nReady = false;
					break;
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
			//	Jun. 16, 2002 genta
			if( !m_nReady ){
				//	�ǂ����ŃG���[���������炵��
				delete macro;
				break;
			}
			/* �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ� */
			Append( macro );
		}
		else {
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
				_T("Line %d: %s�͑��݂��Ȃ��֐��ł��D\n" ), line, szFuncName );
			//	Jun. 16, 2002 genta
			m_nReady = false;
			break;
		}
	}
	fclose( hFile );

	//	Jun. 16, 2002 genta
	//	�}�N�����ɃG���[����������ُ�I���ł���悤�ɂ���D
	return m_nReady ? TRUE : FALSE;
}

//	From Here Apr. 29, 2002 genta
/*!
	Factory

	@param ext [in] �I�u�W�F�N�g�����̔���Ɏg���g���q(������)

	@date 2004-01-31 genta RegisterExt�̔p�~�̂���RegisterCreator�ɒu������
		���̂��߁C�߂����I�u�W�F�N�g�������s��Ȃ����߂Ɋg���q�`�F�b�N�͕K�{�D
*/
CMacroManagerBase* CKeyMacroMgr::Creator(const char* ext)
{
	if( strcmp( ext, "mac" ) == 0 ){
		return new CKeyMacroMgr;
	}
	return NULL;
}

/*!	CKeyMacroManager�̓o�^

	@date 2004.01.31 genta RegisterExt�̔p�~�̂���RegisterCreator�ɒu������
*/
void CKeyMacroMgr::declare (void)
{
	//	��Ɏ��s
	CMacroFactory::Instance()->RegisterCreator( Creator );
}
//	To Here Apr. 29, 2002 genta

/*[EOF]*/
