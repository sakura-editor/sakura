/*!	@file
	@brief �L�[�{�[�h�}�N��

	@author Norio Nakatani

	@date 20011229 aroka �o�O�C���A�R�����g�ǉ�
	YAZAKI �g�ւ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka
	Copyright (C) 2002, YAZAKI, aroka, genta
	Copyright (C) 2004, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "CKeyMacroMgr.h"
#include "CMacro.h"
#include "macro/CSMacroMgr.h"// 2002/2/10 aroka
#include "debug/Debug.h"
#include "charset/charcode.h"
#include "mem/CMemory.h"
#include "CMacroFactory.h"
#include "io/CTextStream.h"

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
void CKeyMacroMgr::Append(
	EFunctionCode	nFuncID,
	LPARAM			lParam1,
	CEditView*		pcEditView
)
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
BOOL CKeyMacroMgr::SaveKeyMacro( HINSTANCE hInstance, const TCHAR* pszPath ) const
{
	CTextOutputStream out(pszPath);
	if(!out){
		return FALSE;
	}

	//�ŏ��̃R�����g
	out.WriteF(L"//�L�[�{�[�h�}�N���̃t�@�C��\n");

	//�}�N�����e
	CMacro* p = m_pTop;
	while (p){
		p->Save( hInstance, out );
		p = p->GetNext();
	}

	out.Close();
	return TRUE;
}



/** �L�[�{�[�h�}�N���̎��s
	CMacro�ɈϏ��B
	
	@date 2007.07.20 genta flags�ǉ��DCMacro::Exec()��
		FA_FROMMACRO���܂߂��l��n���D
*/
void CKeyMacroMgr::ExecKeyMacro( CEditView* pcEditView, int flags ) const
{
	CMacro* p = m_pTop;
	int macroflag = flags | FA_FROMMACRO;
	while (p){
		p->Exec(pcEditView, macroflag);
		p = p->GetNext();
	}
}

/*! �L�[�{�[�h�}�N���̓ǂݍ���
	�G���[���b�Z�[�W�͏o���܂���B�Ăяo�����ł悫�ɂ͂�����Ă��������B
*/
BOOL CKeyMacroMgr::LoadKeyMacro( HINSTANCE hInstance, const TCHAR* pszPath )
{
	/* �L�[�}�N���̃o�b�t�@���N���A���� */
	ClearAll();

	CTextInputStream in( pszPath );
	if(!in){
		m_nReady = false;
		return FALSE;
	}

	WCHAR	szFuncName[100];
	WCHAR	szFuncNameJapanese[256];
	EFunctionCode	nFuncID;
	int		i;
	int		nBgn, nEnd;
	CMacro* macro = NULL;

	//	Jun. 16, 2002 genta
	m_nReady = true;	//	�G���[�������false�ɂȂ�
	static const TCHAR MACRO_ERROR_TITLE[] = _T("Macro�ǂݍ��݃G���[");

	int line = 1;	//	�G���[���ɍs�ԍ���ʒm���邽�߁D1�n�܂�D
	for( ; in.Good() ; ++line ){
		std::wstring szLine = in.ReadLineW();
		using namespace WCODE;

		int nLineLen = auto_strlen( szLine.c_str() );
		// ��s����󔒂��X�L�b�v
		for( i = 0; i < nLineLen; ++i ){
			if( szLine[i] != SPACE && szLine[i] != TAB ){
				break;
			}
		}
		nBgn = i;
		// �R�����g�s�̌��o
		//# �p�t�H�[�}���X�F'/'�̂Ƃ������Q�����ڂ��e�X�g
		if( szLine[nBgn] == LTEXT('/') && nBgn + 1 < nLineLen && szLine[nBgn + 1] == LTEXT('/') ){
			continue;
		}
		//	Jun. 16, 2002 genta ��s�𖳎�����
		if( szLine[nBgn] == LTEXT('\0') ){
			continue;
		}

		// �֐����̎擾
		szFuncName[0]='\0';// ������
		for( ; i < nLineLen; ++i ){
			//# �o�b�t�@�I�[�o�[�����`�F�b�N
			if( szLine[i] == LTEXT('(') && (i - nBgn)< _countof(szFuncName) ){
				auto_memcpy( szFuncName, &szLine[nBgn], i - nBgn );
				szFuncName[i - nBgn] = L'\0';
				++i;
				nBgn = i;
				break;
			}
		}
		// �֐�����S_���t���Ă�����

		/* �֐������@�\ID�C�@�\�����{�� */
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		nFuncID = CSMacroMgr::GetFuncInfoByName( hInstance, szFuncName, szFuncNameJapanese );
		if( -1 != nFuncID ){
			macro = new CMacro( nFuncID );
			// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N�p�ɒǉ�
			int nArgs;
			const MacroFuncInfo* mInfo= CSMacroMgr::GetFuncInfoByID( nFuncID );
			//	Skip Space
			for(nArgs = 0; szLine[i] ; ++nArgs ) {
				// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
				if( nArgs >= _countof( mInfo->m_varArguments ) ){
					::MYMESSAGEBOX(
						NULL,
						MB_OK | MB_ICONSTOP | MB_TOPMOST,
						MACRO_ERROR_TITLE,
						_T("Line %d: Column %d: �������������܂�\n"),
						line,
						i + 1
					);
					m_nReady = false;
				}

				while( szLine[i] == LTEXT(' ') || szLine[i] == LTEXT('\t') )
					i++;

				//@@@ 2002.2.2 YAZAKI PPA.DLL�}�N���ɂ��킹�Ďd�l�ύX�B�������''�ň͂ށB
				//	Jun. 16, 2002 genta double quotation�����e����
				if( LTEXT('\'') == szLine[i] || LTEXT('\"') == szLine[i]  ){	//	'�Ŏn�܂����當���񂾂悫���ƁB
					// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
					// Jun. 27, 2002 genta �]���Ȉ����𖳎�����悤�CVT_EMPTY�����e����D
					if( mInfo->m_varArguments[nArgs] != VT_BSTR && 
						mInfo->m_varArguments[nArgs] != VT_EMPTY ){
						::MYMESSAGEBOX(
							NULL,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							MACRO_ERROR_TITLE,
							_T("Line %d: Column %d\r\n")
							_T("�֐�%ls��%d�Ԗڂ̈����ɕ�����͒u���܂���D"),
							line,
							i + 1,
							szFuncName,
							nArgs + 1
						);
						m_nReady = false;
						break;
					}
					WCHAR cQuote = szLine[i];
					++i;
					nBgn = i;	//	nBgn�͈����̐擪�̕���
					//	Jun. 16, 2002 genta
					//	�s���̌��o�̂��߁C���[�v�񐔂�1���₵��
					for( ; i <= nLineLen; ++i ){		//	�Ō�̕���+1�܂ŃX�L����
						if( szLine[i] == LTEXT('\\') ){	// �G�X�P�[�v�̃X�L�b�v
							++i;
							continue;
						}
						if( szLine[i] == cQuote ){	//	�n�܂�Ɠ���quotation�ŏI���B
							nEnd = i;	//	nEnd�͏I���̎��̕����i'�j
							break;
						}
						if( szLine[i] == LTEXT('\0') ){	//	�s���ɗ��Ă��܂���
							::MYMESSAGEBOX(
								NULL,
								MB_OK | MB_ICONSTOP | MB_TOPMOST,
								MACRO_ERROR_TITLE,
								_T("Line %d:\r\n�֐�%ls��%d�Ԗڂ̈����̏I���� %lc ������܂���D"),
								line,
								szFuncName,
								nArgs + 1,
								cQuote
							);
							m_nReady = false;
							nEnd = i - 1;	//	nEnd�͏I���̎��̕����i'�j
							break;
						}
					}
					//	Jun. 16, 2002 genta
					if( !m_nReady ){
						break;
					}

					CNativeW cmemWork;
					cmemWork.SetString( szLine.c_str() + nBgn, nEnd - nBgn );
					cmemWork.Replace( LTEXT("\\\'"), LTEXT("\'") );

					//	Jun. 16, 2002 genta double quotation���G�X�P�[�v����
					cmemWork.Replace( LTEXT("\\\""), LTEXT("\"") );
					cmemWork.Replace( LTEXT("\\\\"), LTEXT("\\") );
					macro->AddStringParam( cmemWork.GetStringPtr() );	//	�����𕶎���Ƃ��Ēǉ�
				}
				else if ( Is09(szLine[i]) ){	//	�����Ŏn�܂����琔���񂾁B
					// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
					// Jun. 27, 2002 genta �]���Ȉ����𖳎�����悤�CVT_EMPTY�����e����D
					if( mInfo->m_varArguments[nArgs] != VT_I4 && 
						mInfo->m_varArguments[nArgs] != VT_EMPTY){
						::MYMESSAGEBOX(
							NULL,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							MACRO_ERROR_TITLE,
							_T("Line %d: Column %d\r\n")
							_T("�֐�%ls��%d�Ԗڂ̈����ɐ��l�͒u���܂���D"),
							line,
							i + 1,
							szFuncName,
							nArgs + 1
						);
						m_nReady = false;
						break;
					}
					nBgn = i;	//	nBgn�͈����̐擪�̕���
					for( ; i < nLineLen; ++i ){		//	�Ō�̕����܂ŃX�L����
						if( Is09(szLine[i]) ){	// �܂����l
//							++i;
							continue;
						}
						else {
							nEnd = i;	//	�����̍Ō�̕���
							i--;
							break;
						}
					}

					CNativeW cmemWork;
					cmemWork.SetString( szLine.c_str() + nBgn, nEnd - nBgn );
					// Jun. 16, 2002 genta
					//	�����̒���quotation�͓����Ă��Ȃ���
					//cmemWork.Replace( L"\\\'", L"\'" );
					//cmemWork.Replace( L"\\\\", L"\\" );
					macro->AddStringParam( cmemWork.GetStringPtr() );	//	�����𕶎���Ƃ��Ēǉ�
				}
				//	Jun. 16, 2002 genta
				else if( szLine[i] == LTEXT(')') ){
					//	��������
					break;
				}
				else {
					//	Parse Error:���@�G���[���ۂ��B
					//	Jun. 16, 2002 genta
					nBgn = nEnd = i;
					::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
						_T("Line %d: Column %d: Syntax Error\n"), line, i );
					m_nReady = false;
					break;
				}

				for( ; i < nLineLen; ++i ){		//	�Ō�̕����܂ŃX�L����
					if( szLine[i] == LTEXT(')') || szLine[i] == LTEXT(',') ){	//	,��������)��ǂݔ�΂�
						i++;
						break;
					}
				}
				if (szLine[i-1] == LTEXT(')')){
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
				_T("Line %d: %ls�͑��݂��Ȃ��֐��ł��D\n"), line, szFuncName );
			//	Jun. 16, 2002 genta
			m_nReady = false;
			break;
		}
	}
	in.Close();

	//	Jun. 16, 2002 genta
	//	�}�N�����ɃG���[����������ُ�I���ł���悤�ɂ���D
	return m_nReady ? TRUE : FALSE;
}

/*! �L�[�{�[�h�}�N���𕶎��񂩂�ǂݍ��� */
BOOL CKeyMacroMgr::LoadKeyMacroStr( HINSTANCE hInstance, const TCHAR* pszCode )
{
	// �ꎞ�t�@�C�������쐬
	TCHAR szTempDir[_MAX_PATH];
	TCHAR szTempFile[_MAX_PATH];
	if( 0 == ::GetTempPath( _MAX_PATH, szTempDir ) )return FALSE;
	if( 0 == ::GetTempFileName( szTempDir, _T("mac"), 0, szTempFile ) )return FALSE;
	// �ꎞ�t�@�C���ɏ�������
	CTextOutputStream out = CTextOutputStream( szTempFile );
	out.WriteString( to_wchar( pszCode ) );
	out.Close();

	// �}�N���ǂݍ���
	BOOL bRet = LoadKeyMacro( hInstance, szTempFile );

	::DeleteFile( szTempFile );			// �ꎞ�t�@�C���폜

	return bRet;
}

//	From Here Apr. 29, 2002 genta
/*!
	Factory

	@param ext [in] �I�u�W�F�N�g�����̔���Ɏg���g���q(������)

	@date 2004-01-31 genta RegisterExt�̔p�~�̂���RegisterCreator�ɒu������
		���̂��߁C�߂����I�u�W�F�N�g�������s��Ȃ����߂Ɋg���q�`�F�b�N�͕K�{�D
*/
CMacroManagerBase* CKeyMacroMgr::Creator(const TCHAR* ext)
{
	if( _tcscmp( ext, _T("mac") ) == 0 ){
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


