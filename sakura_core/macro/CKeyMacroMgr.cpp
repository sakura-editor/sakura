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

#include "StdAfx.h"
#include <stdio.h>
#include <string.h>
#include "CKeyMacroMgr.h"
#include "CMacro.h"
#include "macro/CSMacroMgr.h"// 2002/2/10 aroka
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
	const LPARAM*	lParams,
	CEditView*		pcEditView
)
{
	CMacro* macro = new CMacro( nFuncID );
	macro->AddLParam( lParams, pcEditView );
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
	out.WriteF(LSW(STR_ERR_DLGKEYMACMGR1));

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
bool CKeyMacroMgr::ExecKeyMacro( CEditView* pcEditView, int flags ) const
{
	CMacro* p = m_pTop;
	int macroflag = flags | FA_FROMMACRO;
	bool bRet = true;
	while (p){
		if( !p->Exec(pcEditView, macroflag) ){
			bRet = false;
			break;
		}
		p = p->GetNext();
	}
	return bRet;
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
	std::tstring MACRO_ERROR_TITLE_string = LS(STR_ERR_DLGKEYMACMGR2);
	const TCHAR* MACRO_ERROR_TITLE = MACRO_ERROR_TITLE_string.c_str();

	int line = 1;	//	�G���[���ɍs�ԍ���ʒm���邽�߁D1�n�܂�D
	for( ; in.Good() ; ++line ){
		std::wstring strLine = in.ReadLineW();
		const WCHAR* szLine = strLine.c_str(); // '\0'�I�[��������擾
		using namespace WCODE;

		int nLineLen = strLine.length();
		// ��s����󔒂��X�L�b�v
		for( i = 0; i < nLineLen; ++i ){
			if( szLine[i] != SPACE && szLine[i] != TAB ){
				break;
			}
		}
		nBgn = i;
		//	Jun. 16, 2002 genta ��s�𖳎�����
		if( nBgn == nLineLen || szLine[nBgn] == LTEXT('\0') ){
			continue;
		}
		// �R�����g�s�̌��o
		//# �p�t�H�[�}���X�F'/'�̂Ƃ������Q�����ڂ��e�X�g
		if( szLine[nBgn] == LTEXT('/') && nBgn + 1 < nLineLen && szLine[nBgn + 1] == LTEXT('/') ){
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
			int nArgSizeMax = _countof( mInfo->m_varArguments );
			if( mInfo->m_pData  ){
				nArgSizeMax = mInfo->m_pData->m_nArgMaxSize;
			}
			for(nArgs = 0; szLine[i] ; ++nArgs ) {
				// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
				if( nArgs >= nArgSizeMax ){
					::MYMESSAGEBOX(
						NULL,
						MB_OK | MB_ICONSTOP | MB_TOPMOST,
						MACRO_ERROR_TITLE,
						LS(STR_ERR_DLGKEYMACMGR3),
						line,
						i + 1
					);
					m_nReady = false;
				}
				VARTYPE type = VT_EMPTY;
				if( nArgs < 4 ){
					type = mInfo->m_varArguments[nArgs];
				}else{
					if(  mInfo->m_pData && nArgs < mInfo->m_pData->m_nArgMinSize ){
						type = mInfo->m_pData->m_pVarArgEx[nArgs - 4];
					}
				}

				//	Skip Space
				while( szLine[i] == LTEXT(' ') || szLine[i] == LTEXT('\t') )
					i++;

				//@@@ 2002.2.2 YAZAKI PPA.DLL�}�N���ɂ��킹�Ďd�l�ύX�B�������''�ň͂ށB
				//	Jun. 16, 2002 genta double quotation�����e����
				if( LTEXT('\'') == szLine[i] || LTEXT('\"') == szLine[i]  ){	//	'�Ŏn�܂����當���񂾂悫���ƁB
					// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
					// Jun. 27, 2002 genta �]���Ȉ����𖳎�����悤�CVT_EMPTY�����e����D
					if( type != VT_BSTR && 
						type != VT_EMPTY ){
						::MYMESSAGEBOX(
							NULL,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							MACRO_ERROR_TITLE,
							LS(STR_ERR_DLGKEYMACMGR4),
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
					nBgn = nEnd = i;	//	nBgn�͈����̐擪�̕���
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
						if( i == nLineLen ){	//	�s���ɗ��Ă��܂���
							::MYMESSAGEBOX(
								NULL,
								MB_OK | MB_ICONSTOP | MB_TOPMOST,
								MACRO_ERROR_TITLE,
								LS(STR_ERR_DLGKEYMACMGR5),
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
					cmemWork.SetString( strLine.c_str() + nBgn, nEnd - nBgn );
					// 2014.01.28 �u"\\'"�v�̂悤�ȏꍇ�̕s����C��
					cmemWork.Replace( L"\\\\", L"\\\1" ); // �ꎞ�u��(�ŏ��ɕK�v)
					cmemWork.Replace( LTEXT("\\\'"), LTEXT("\'") );

					//	Jun. 16, 2002 genta double quotation���G�X�P�[�v����
					cmemWork.Replace( LTEXT("\\\""), LTEXT("\"") );
					cmemWork.Replace( L"\\r", L"\r" );
					cmemWork.Replace( L"\\n", L"\n" );
					cmemWork.Replace( L"\\t", L"\t" );
					{
						// \uXXXX �u��
						size_t nLen = cmemWork.GetStringLength();
						size_t nBegin = 0;
						const wchar_t* p = cmemWork.GetStringPtr();
						CNativeW cmemTemp;
						for( size_t n = 0; n < nLen; n++ ){
							if( n + 1 < nLen && p[n] == L'\\' && p[n+1] == L'u' ){
								size_t k;
								for( k = n + 2; k < nLen && k < n + 2 + 4
									&& (WCODE::Is09(p[k])
										|| (L'a' <= p[k] && p[k] <= L'f')
										|| (L'A' <= p[k] && p[k] <= L'F')); k++ ){
								}
								cmemTemp.AppendString( p + nBegin, n - nBegin );
								nBegin = k;
								if( 0 < k - n - 2 ){
									wchar_t hex[5];
									wcsncpy( hex, &p[n+2], k - n - 2 );
									hex[k - n - 2] = L'\0';
									wchar_t* pEnd = NULL;
									wchar_t c = static_cast<wchar_t>(wcstol(hex, &pEnd, 16));
									cmemTemp.AppendString( &c, 1 );
								}
								n = k - 1;
							}
						}
						if( nBegin != 0 ){
							if( 0 < nLen - nBegin ){
								cmemTemp.AppendString( p + nBegin, nLen - nBegin );
							}
							cmemWork.swap( cmemTemp );
						}
					}
					cmemWork.Replace( L"\\\1", L"\\" ); // �ꎞ�u����\�ɖ߂�(�Ō�łȂ��Ƃ����Ȃ�)
					macro->AddStringParam( cmemWork.GetStringPtr(), cmemWork.GetStringLength() );	//	�����𕶎���Ƃ��Ēǉ�
				}
 				else if ( Is09(szLine[i]) || szLine[i] == L'-' ){	//	�����Ŏn�܂����琔����(-�L�����܂�)�B
					// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
					// Jun. 27, 2002 genta �]���Ȉ����𖳎�����悤�CVT_EMPTY�����e����D
					if( type != VT_I4 &&
						type != VT_EMPTY){
						::MYMESSAGEBOX(
							NULL,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							MACRO_ERROR_TITLE,
							LS(STR_ERR_DLGKEYMACMGR6),
							line,
							i + 1,
							szFuncName,
							nArgs + 1
						);
						m_nReady = false;
						break;
					}
					nBgn = nEnd = i;	//	nBgn�͈����̐擪�̕���
					//	�s���̌��o�̂��߁C���[�v�񐔂�1���₵��
					for( i = nBgn + 1; i <= nLineLen; ++i ){		//	�Ō�̕���+1�܂ŃX�L����
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
					cmemWork.SetString( strLine.c_str() + nBgn, nEnd - nBgn );
					// Jun. 16, 2002 genta
					//	�����̒���quotation�͓����Ă��Ȃ���
					//cmemWork.Replace( L"\\\'", L"\'" );
					//cmemWork.Replace( L"\\\\", L"\\" );
					macro->AddIntParam( _wtoi(cmemWork.GetStringPtr()) );	//	�����𐔎��Ƃ��Ēǉ�
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
						LS(STR_ERR_DLGKEYMACMGR7), line, i + 1 );
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
				LS(STR_ERR_DLGKEYMACMGR8), line, szFuncName );
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
	CMacroFactory::getInstance()->RegisterCreator( Creator );
}
//	To Here Apr. 29, 2002 genta


