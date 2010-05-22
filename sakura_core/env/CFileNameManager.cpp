/*
	2008.05.18 kobake CShareData ���番��
*/

#include "stdafx.h"
#include "DLLSHAREDATA.h"

#include "CFileNameManager.h"
#include <shlobj.h> //CSIDL_PROFILE��
#include "util/module.h"
#include "util/os.h"
#include "util/shell.h"
#include "CCommandLine.h"
#include "COsVersionInfo.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �t�@�C�����Ǘ�                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*!	���L�f�[�^�̐ݒ�ɏ]���ăp�X���k���\�L�ɕϊ�����
	@param pszSrc   [in]  �t�@�C����
	@param pszDest  [out] �ϊ���̃t�@�C�����̊i�[��
	@param nDestLen [in]  �I�[��NULL���܂�pszDest��TCHAR�P�ʂ̒��� _MAX_PATH �܂�
	@date 2003.01.27 Moca �V�K�쐬
	@note �A�����ČĂяo���ꍇ�̂��߁A�W�J�ς݃��^��������L���b�V�����č��������Ă���B
*/
LPTSTR CFileNameManager::GetTransformFileNameFast( LPCTSTR pszSrc, LPTSTR pszDest, int nDestLen )
{
	int i;
	TCHAR szBuf[_MAX_PATH + 1];

	if( -1 == m_nTransformFileNameCount ){
		TransformFileName_MakeCache();
	}

	if( 0 < m_nTransformFileNameCount ){
		GetFilePathFormat( pszSrc, pszDest, nDestLen,
			m_szTransformFileNameFromExp[0],
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[m_nTransformFileNameOrgId[0]]
		);
		for( i = 1; i < m_nTransformFileNameCount; i++ ){
			_tcscpy( szBuf, pszDest );
			GetFilePathFormat( szBuf, pszDest, nDestLen,
				m_szTransformFileNameFromExp[i],
				m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[m_nTransformFileNameOrgId[i]] );
		}
	}else{
		// �ϊ�����K�v���Ȃ� �R�s�[��������
		_tcsncpy( pszDest, pszSrc, nDestLen - 1 );
		pszDest[nDestLen - 1] = '\0';
	}
	return pszDest;
}

/*!	�W�J�ς݃��^������̃L���b�V�����쐬�E�X�V����
	@retval �L���ȓW�J�ςݒu���O������̐�
	@date 2003.01.27 Moca �V�K�쐬
	@date 2003.06.23 Moca �֐����ύX
*/
int CFileNameManager::TransformFileName_MakeCache( void ){
	int i;
	int nCount = 0;
	for( i = 0; i < m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum; i++ ){
		if( L'\0' != m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i][0] ){
			if( ExpandMetaToFolder( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i],
			 m_szTransformFileNameFromExp[nCount], _MAX_PATH ) ){
				// m_szTransformFileNameTo��m_szTransformFileNameFromExp�̔ԍ�������邱�Ƃ�����̂ŋL�^���Ă���
				m_nTransformFileNameOrgId[nCount] = i;
				nCount++;
			}
		}
	}
	m_nTransformFileNameCount = nCount;
	return nCount;
}


/*!	�t�@�C���E�t�H���_����u�����āA�ȈՕ\�������擾����
	@date 2002.11.27 Moca �V�K�쐬
	@note �召��������ʂ��Ȃ��BnDestLen�ɒB�����Ƃ��͌���؂�̂Ă���
*/
LPCTSTR CFileNameManager::GetFilePathFormat( LPCTSTR pszSrc, LPTSTR pszDest, int nDestLen, LPCTSTR pszFrom, LPCTSTR pszTo )
{
	int i, j;
	int nSrcLen;
	int nFromLen, nToLen;
	int nCopy;

	nSrcLen  = _tcslen( pszSrc );
	nFromLen = _tcslen( pszFrom );
	nToLen   = _tcslen( pszTo );

	nDestLen--;

	for( i = 0, j = 0; i < nSrcLen && j < nDestLen; i++ ){
#if defined(_MBCS)
		if( 0 == strnicmp( &pszSrc[i], pszFrom, nFromLen ) )
#else
		if( 0 == _tcsncicmp( &pszSrc[i], pszFrom, nFromLen ) )
#endif
		{
			nCopy = __min( nToLen, nDestLen - j );
			memcpy( &pszDest[j], pszTo, nCopy * sizeof( TCHAR ) );
			j += nCopy;
			i += nFromLen - 1;
		}else{
#if defined(_MBCS)
// SJIS ��p����
			if( _IS_SJIS_1( (unsigned char)pszSrc[i] ) && i + 1 < nSrcLen && _IS_SJIS_2( (unsigned char)pszSrc[i + 1] ) ){
				if( j + 1 < nDestLen ){
					pszDest[j] = pszSrc[i];
					j++;
					i++;
				}else{
					// SJIS�̐�s�o�C�g�����R�s�[�����̂�h��
					break;// goto end_of_func;
				}
			}
#endif
			pszDest[j] = pszSrc[i];
			j++;
		}
	}
// end_of_func:;
	pszDest[j] = '\0';
	return pszDest;
}


/*!	%MYDOC%�Ȃǂ̃p�����[�^�w������ۂ̃p�X���ɕϊ�����

	@param pszSrc  [in]  �ϊ��O������
	@param pszDes  [out] �ϊ��㕶����
	@param nDesLen [in]  pszDes��NULL���܂�TCHAR�P�ʂ̒���
	@retval true  ����ɕϊ��ł���
	@retval false �o�b�t�@������Ȃ������C�܂��̓G���[�BpszDes�͕s��
	@date 2002.11.27 Moca �쐬�J�n
*/
bool CFileNameManager::ExpandMetaToFolder( LPCTSTR pszSrc, LPTSTR pszDes, int nDesLen )
{
	LPCTSTR ps;
	LPTSTR  pd, pd_end;

#define _USE_META_ALIAS
#ifdef _USE_META_ALIAS
	struct MetaAlias{
		LPTSTR szAlias;
		int nLenth;
		LPTSTR szOrig;
	};
	static const MetaAlias AliasList[] = {
		{  _T("COMDESKTOP"), 10, _T("Common Desktop") },
		{  _T("COMMUSIC"), 8, _T("CommonMusic") },
		{  _T("COMVIDEO"), 8, _T("CommonVideo") },
		{  _T("MYMUSIC"),  7, _T("My Music") },
		{  _T("MYVIDEO"),  7, _T("Video") },
		{  _T("COMPICT"),  7, _T("CommonPictures") },
		{  _T("MYPICT"),   6, _T("My Pictures") },
		{  _T("COMDOC"),   6, _T("Common Documents") },
		{  _T("MYDOC"),    5, _T("Personal") },
		{ NULL, 0 , NULL }
	};
#endif

	pd_end = pszDes + ( nDesLen - 1 );
	for( ps = pszSrc, pd = pszDes; _T('\0') != *ps; ps++ ){
		if( pd_end <= pd ){
			if( pd_end == pd ){
				*pd = _T('\0');
			}
			return false;
		}

		if( _T('%') != *ps ){
			*pd = *ps;
			pd++;
			continue;
		}

		// %% �� %
		if( _T('%') == ps[1] ){
			*pd = _T('%');
			pd++;
			ps++;
			continue;
		}

		if( _T('\0') != ps[1] ){
			TCHAR szMeta[_MAX_PATH];
			TCHAR szPath[_MAX_PATH + 1];
			int   nMetaLen;
			int   nPathLen;
			bool  bFolderPath;
			LPCTSTR  pStr;
			ps++;
			// %SAKURA%
			if( 0 == auto_strnicmp( _T("SAKURA%"), ps, 7 ) ){
				// exe�̂���t�H���_
				GetExedir( szPath );
				nMetaLen = 6;
			}
			// %SAKURADATA%	// 2007.06.06 ryoji
			else if( 0 == auto_strnicmp( _T("SAKURADATA%"), ps, 11 ) ){
				// ini�̂���t�H���_
				GetInidir( szPath );
				nMetaLen = 10;
			}
			// ���^��������ۂ�
			else if( NULL != (pStr = _tcschr( ps, _T('%') ) )){
				nMetaLen = pStr - ps;
				if( nMetaLen < _MAX_PATH ){
					auto_memcpy( szMeta, ps, nMetaLen );
					szMeta[nMetaLen] = _T('\0');
				}
				else{
					*pd = _T('\0');
					return false;
				}
#ifdef _USE_META_ALIAS
				// ���^�����񂪃G�C���A�X���Ȃ珑��������
				const MetaAlias* pAlias;
				for( pAlias = &AliasList[0]; nMetaLen < pAlias->nLenth; pAlias++ )
					; // �ǂݔ�΂�
				for( ; nMetaLen == pAlias->nLenth; pAlias++ ){
					if( 0 == auto_stricmp( pAlias->szAlias, szMeta ) ){
						_tcscpy( szMeta, pAlias->szOrig );
						break;
					}
				}
#endif
				// ���ڃ��W�X�g���Œ��ׂ�
				szPath[0] = _T('\0');
				bFolderPath = ReadRegistry( HKEY_CURRENT_USER,
					_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
					szMeta, szPath, _countof( szPath ) );
				if( false == bFolderPath || _T('\0') == szPath[0] ){
					bFolderPath = ReadRegistry( HKEY_LOCAL_MACHINE,
						_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
						szMeta, szPath, _countof( szPath ) );
				}
				if( false == bFolderPath || _T('\0') == szPath[0] ){
					pStr = _tgetenv( szMeta );
					// ���ϐ�
					if( NULL != pStr ){
						nPathLen = _tcslen( pStr );
						if( nPathLen < _MAX_PATH ){
							_tcscpy( szPath, pStr );
						}else{
							*pd = _T('\0');
							return false;
						}
					}
					// ����`�̃��^������� ���͂��ꂽ%...%���C���̂܂ܕ����Ƃ��ď�������
					else if(  pd + ( nMetaLen + 2 ) < pd_end ){
						*pd = _T('%');
						auto_memcpy( &pd[1], ps, nMetaLen );
						pd[nMetaLen + 1] = _T('%');
						pd += nMetaLen + 2;
						ps += nMetaLen;
						continue;
					}else{
						*pd = _T('\0');
						return false;
					}
				}
			}else{
				// %...%�̏I����%���Ȃ� �Ƃ肠�����C%���R�s�[
				*pd = _T('%');
				pd++;
				ps--; // ���ps++���Ă��܂����̂Ŗ߂�
				continue;
			}

			// �����O�t�@�C�����ɂ���
			nPathLen = _tcslen( szPath );
			LPTSTR pStr2 = szPath;
			if( nPathLen < _MAX_PATH && 0 != nPathLen ){
				if( FALSE != GetLongFileName( szPath, szMeta ) ){
					pStr2 = szMeta;
				}
			}

			// �Ō�̃t�H���_��؂�L�����폜����
			// [A:\]�Ȃǂ̃��[�g�ł����Ă��폜
			for(nPathLen = 0; pStr2[nPathLen] != _T('\0'); nPathLen++ ){
#ifdef _MBCS
				if( _IS_SJIS_1( (unsigned char)pStr2[nPathLen] ) && _IS_SJIS_2( (unsigned char)pStr2[nPathLen + 1] ) ){
					// SJIS�ǂݔ�΂�
					nPathLen++; // 2003/01/17 sui
				}else
#endif
				if( _T('\\') == pStr2[nPathLen] && _T('\0') == pStr2[nPathLen + 1] ){
					pStr2[nPathLen] = _T('\0');
					break;
				}
			}

			if( pd + nPathLen < pd_end && 0 != nPathLen ){
				auto_memcpy( pd, pStr2, nPathLen );
				pd += nPathLen;
				ps += nMetaLen;
			}else{
				*pd = _T('\0');
				return false;
			}
		}else{
			// �Ō�̕�����%������
			*pd = *ps;
			pd++;
		}
	}
	*pd = _T('\0');
	return true;
}




/**
	�\���ݒ�t�@�C������ini�t�@�C�������擾����

	sakura.exe.ini����sakura.ini�̊i�[�t�H���_���擾���A�t���p�X����Ԃ�

	@param[out] pszPrivateIniFile �}���`���[�U�p��ini�t�@�C���p�X
	@param[out] pszIniFile EXE���ini�t�@�C���p�X

	@author ryoji
	@date 2007.09.04 ryoji �V�K�쐬
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
void CFileNameManager::GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile )
{
	TCHAR szPath[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFname[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];

	::GetModuleFileName(
		NULL,
		szPath, _countof(szPath)
	);
	_tsplitpath( szPath, szDrive, szDir, szFname, szExt );

	auto_snprintf_s( pszIniFile, _MAX_PATH - 1, _T("%ts%ts%ts%ts"), szDrive, szDir, szFname, _T(".ini") );

	// �}���`���[�U�p��ini�t�@�C���p�X
	//		exe�Ɠ����t�H���_�ɒu���ꂽ�}���`���[�U�\���ݒ�t�@�C���isakura.exe.ini�j�̓��e
	//		�ɏ]���ă}���`���[�U�p��ini�t�@�C���p�X�����߂�
	pszPrivateIniFile[0] = _T('\0');
	if( COsVersionInfo().IsWin2000_or_later() ){
		auto_snprintf_s( szPath, _MAX_PATH - 1, _T("%ts%ts%ts%ts"), szDrive, szDir, szFname, _T(".exe.ini") );
		int nEnable = ::GetPrivateProfileInt(_T("Settings"), _T("MultiUser"), 0, szPath );
		if( nEnable ){
			int nFolder = ::GetPrivateProfileInt(_T("Settings"), _T("UserRootFolder"), 0, szPath );
			switch( nFolder ){
			case 1:
				nFolder = CSIDL_PROFILE;			// ���[�U�̃��[�g�t�H���_
				break;
			case 2:
				nFolder = CSIDL_PERSONAL;			// ���[�U�̃h�L�������g�t�H���_
				break;
			case 3:
				nFolder = CSIDL_DESKTOPDIRECTORY;	// ���[�U�̃f�X�N�g�b�v�t�H���_
				break;
			default:
				nFolder = CSIDL_APPDATA;			// ���[�U�̃A�v���P�[�V�����f�[�^�t�H���_
				break;
			}
			::GetPrivateProfileString(_T("Settings"), _T("UserSubFolder"), _T("sakura"), szDir, _MAX_DIR, szPath );
			if( szDir[0] == _T('\0') )
				::lstrcpy( szDir, _T("sakura") );
			if( GetSpecialFolderPath( nFolder, szPath ) ){
				auto_snprintf_s( pszPrivateIniFile, _MAX_PATH - 1, _T("%ts\\%ts\\%ts%ts"), szPath, szDir, szFname, _T(".ini") );
			}
		}
	}
	return;
}

/**
	ini�t�@�C�����̎擾

	���L�f�[�^����sakura.ini�̊i�[�t�H���_���擾���A�t���p�X����Ԃ�
	�i���L�f�[�^���ݒ�̂Ƃ��͋��L�f�[�^�ݒ���s���j

	@param[out] pszIniFileName ini�t�@�C�����i�t���p�X�j
	@param[in] bRead true: �ǂݍ��� / false: ��������

	@author ryoji
	@date 2007.05.19 ryoji �V�K�쐬
*/
void CFileNameManager::GetIniFileName( LPTSTR pszIniFileName, BOOL bRead/*=FALSE*/ )
{
	if( !m_pShareData->m_sFileNameManagement.m_IniFolder.m_bInit ){
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bInit = true;			// �������σt���O
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bReadPrivate = false;	// �}���`���[�U�pini����̓ǂݏo���t���O
		m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate = false;	// �}���`���[�U�pini�ւ̏������݃t���O

		GetIniFileNameDirect( m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile, m_pShareData->m_sFileNameManagement.m_IniFolder.m_szIniFile );
		if( m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile[0] != _T('\0') ){
			m_pShareData->m_sFileNameManagement.m_IniFolder.m_bReadPrivate = true;
			m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate = true;
			if( CCommandLine::Instance()->IsNoWindow() && CCommandLine::Instance()->IsWriteQuit() )
				m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate = false;

			// �}���`���[�U�p��ini�t�H���_���쐬���Ă���
			if( m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate ){
				TCHAR szPath[_MAX_PATH];
				TCHAR szDrive[_MAX_DRIVE];
				TCHAR szDir[_MAX_DIR];
				_tsplitpath( m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile, szDrive, szDir, NULL, NULL );
				auto_snprintf_s( szPath, _MAX_PATH - 1, _T("%ts\\%ts"), szDrive, szDir );
				MakeSureDirectoryPathExistsT( szPath );
			}
		}
	}

	bool bPrivate = bRead? m_pShareData->m_sFileNameManagement.m_IniFolder.m_bReadPrivate: m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate;
	::lstrcpy( pszIniFileName, bPrivate? m_pShareData->m_sFileNameManagement.m_IniFolder.m_szPrivateIniFile: m_pShareData->m_sFileNameManagement.m_IniFolder.m_szIniFile );
}

