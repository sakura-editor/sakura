#include "stdafx.h"
#include "file.h"
#include "CharPointer.h"
#include "util/module.h"
#include "CShareData.h"
#include <io.h>

/**	�t�@�C�����̐؂�o��

	�w�蕶���񂩂�t�@�C�����ƔF������镶��������o���A
	�擪Offset�y�ђ�����Ԃ��B
	
	@param pLine [in] �T���Ώە�����
	@param pnBgn [out] �擪offset�BpLine + *pnBgn���t�@�C�����擪�ւ̃|�C���^�B
	@param pnPathLen [out] �t�@�C�����̒���
	@param bFileOnly [in] true: �t�@�C���̂ݑΏ� / false: �f�B���N�g�����Ώ�
	
	@retval true �t�@�C��������
	@retval false �t�@�C�����͌�����Ȃ�����
	
	@date 2002.01.04 genta �f�B���N�g���������ΏۊO�ɂ���@�\��ǉ�
	@date 2003.01.15 matsumo gcc�̃G���[���b�Z�[�W(:��؂�)�ł��t�@�C�������o�\��
	@date 2004.05.29 genta C:\����t�@�C��C���؂�o�����̂�h�~
	@date 2004.11.13 genta/Moca �t�@�C�����擪��*?���l��
	@date 2005.01.10 genta �ϐ����ύX j -> cur_pos
	@date 2005.01.23 genta �x���}���̂��߁Cgoto��return�ɕύX
	
*/
bool IsFilePath(
	const wchar_t*	pLine,
	int*			pnBgn,
	int*			pnPathLen,
	bool			bFileOnly
)
{
	int		i;
	int		nLineLen;
	wchar_t	szJumpToFile[1024];
	wmemset( szJumpToFile, 0, _countof( szJumpToFile ) );

	nLineLen = wcslen( pLine );

	//�擪�̋󔒂�ǂݔ�΂�
	for( i = 0; i < nLineLen; ++i ){
		if( L' ' != pLine[i] &&
			L'\t' != pLine[i] &&
			L'\"' != pLine[i]
		){
			break;
		}
	}

	//	#include <�t�@�C����>�̍l��
	//	#�Ŏn�܂�Ƃ���"�܂���<�܂œǂݔ�΂�
	if( i < nLineLen &&
		L'#' == pLine[i]
	){
		for( ; i < nLineLen; ++i ){
			if( L'<'  == pLine[i] ||
				L'\"' == pLine[i]
			){
				++i;
				break;
			}
		}
	}

	//	���̎��_�Ŋ��ɍs���ɒB���Ă�����t�@�C�����͌�����Ȃ�
	if( i >= nLineLen ){
		return false;
	}
	*pnBgn = i;
	int cur_pos = 0;
	for( ; i <= nLineLen && cur_pos + 1 < _countof(szJumpToFile); ++i ){
		if( ( i == nLineLen    ||
			  pLine[i] == L' '  ||
			  pLine[i] == L'\t' ||	//@@@ 2002.01.08 YAZAKI �^�u�������B
			  pLine[i] == L'('  ||
			  pLine[i] == L'\r' ||
			  pLine[i] == L'\n' ||
			  pLine[i] == L'\0' ||
			  pLine[i] == L'>'  ||
			  // May 29, 2004 genta C:\��:�̓t�@�C����؂�ƌ��Ȃ��ė~�����Ȃ�
			  ( cur_pos > 1 && pLine[i] == L':' ) ||   //@@@ 2003/1/15/ matsumo (for gcc)
			  pLine[i] == L'"'
			) &&
			0 < wcslen( szJumpToFile )
		){
			//	Jan. 04, 2002 genta
			//	�t�@�C�����݊m�F���@�ύX
			//if( -1 != _access( szJumpToFile, 0 ) )
			if( IsFileExists(to_tchar(szJumpToFile), bFileOnly))
			{
				i--;
				break;
			}
		}
		if( pLine[i] == L'\r'  ||
			pLine[i] == L'\n' ){
			break;
		}
//	From Here Sept. 27, 2000 JEPRO �^�O�W�����v�ł��Ȃ��͈̂ȉ��̕�����1�o�C�g�R�[�h�Ō����Ƃ��݂̂Ƃ����B
//	(SJIS2�o�C�g�R�[�h��2�o�C�g�ڂɌ����ꍇ�̓p�X���g�p�֎~�����Ƃ͔F�����Ȃ��Ŗ�������悤�ɕύX)
//		if( /*pLine[i] == '/' ||*/
//			pLine[i] == '<' ||
//			pLine[i] == '>' ||
//			pLine[i] == '?' ||
//			pLine[i] == '"' ||
//			pLine[i] == '|' ||
//			pLine[i] == '*'
//		){
//			return false;
//		}
//
//		szJumpToFile[cur_pos] = pLine[i];
//		cur_pos++;
//	}
//  To Here comment out
//	From Here Sept. 27, 2000 JEPRO added
//			  Oct. 3, 2000 JEPRO corrected
		if( /*pLine[i] == '/' ||*/
			 pLine[i] == L'<' ||	//	0x3C
			 pLine[i] == L'>' ||	//	0x3E
			 pLine[i] == L'?' ||	//	0x3F
			 pLine[i] == L'"' ||	//	0x22
			 pLine[i] == L'|' ||	//	0x7C
			 pLine[i] == L'*'	//	0x2A
			 ){
			/*
			&&
			// ��̕�����SJIS2�o�C�g�R�[�h��2�o�C�g�ڂłȂ����Ƃ��A1�O�̕�����SJIS2�o�C�g�R�[�h��1�o�C�g�ڂłȂ����ƂŔ��f����
			//	Oct. 5, 2002 genta
			//	2004.11.13 Moca/genta �擪�ɏ�̕���������ꍇ�̍l����ǉ�
			( i == 0 || ( i > 0 && ! _IS_SJIS_1( (unsigned char)pLine[i - 1] ))) ){
			*/
			return false;
		}else{
			szJumpToFile[cur_pos] = pLine[i];
			cur_pos++;
		}
	}
//	To Here Sept. 27, 2000
//	if( i >= nLineLen ){
//		return FALSE;
//	}
	//	Jan. 04, 2002 genta
	//	�t�@�C�����݊m�F���@�ύX
	if( 0 < wcslen( szJumpToFile ) &&
		IsFileExists(to_tchar(szJumpToFile), bFileOnly))
	//	-1 != _access( szJumpToFile, 0 ) )
	{
		//	Jan. 04, 2002 genta
		//	���܂�ɕςȃR�[�f�B���O�Ȃ̂ňړ�
		*pnPathLen = wcslen( szJumpToFile );
		return true;
	}else{
	}

	return false;

}

/*!
	���[�J���h���C�u�̔���

	@param[in] pszDrive �h���C�u�����܂ރp�X��
	
	@retval true ���[�J���h���C�u
	@retval false �����[�o�u���h���C�u�D�l�b�g���[�N�h���C�u�D
	
	@author MIK
	@date 2001.03.29 MIK �V�K�쐬
	@date 2001.12.23 YAZAKI MRU�̕ʃN���X���ɔ����֐���
	@date 2002.01.28 genta �߂�l�̌^��BOOL����bool�ɕύX�D
	@date 2005.11.12 aroka �������蕔�ύX
	@date 2006.01.08 genta CMRU::IsRemovableDrive��CEditDoc::IsLocalDrive��
		�����I�ɓ������̂�����
*/
bool IsLocalDrive( const TCHAR* pszDrive )
{
	TCHAR	szDriveType[_MAX_DRIVE+1];	// "A:\ "�o�^�p
	long	lngRet;

	if( iswalpha(pszDrive[0]) ){
		auto_sprintf(szDriveType, _T("%tc:\\"), _totupper(pszDrive[0]));
		lngRet = GetDriveType( szDriveType );
		if( lngRet == DRIVE_REMOVABLE || lngRet == DRIVE_CDROM || lngRet == DRIVE_REMOTE )
		{
			return false;
		}
	}
	return true;
}



const TCHAR* GetFileTitlePointer(const TCHAR* tszPath)
{
	CharPointerT p;
	const TCHAR* pszName;
	p = pszName = tszPath;
	while( *p )
	{
		if( *p == _T('\\') ){
			pszName = p + 1;
			p++;
		}
		else{
			p++;
		}
	}
	return pszName;
}


/*! fname�����΃p�X�̏ꍇ�́A���s�t�@�C���̃p�X����̑��΃p�X�Ƃ��ĊJ��
	@author Moca
	@date 2003.06.23
	@date 2007.05.20 ryoji �֐����ύX�i���Ffopen_absexe�j�A�ėp�e�L�X�g�}�b�s���O��
*/
FILE* _tfopen_absexe(LPCTSTR fname, LPCTSTR mode)
{
	if( _IS_REL_PATH( fname ) ){
		TCHAR path[_MAX_PATH];
		GetExedir( path, fname );
		return _tfopen( path, mode );
	}
	return _tfopen( fname, mode );
}

/*! fname�����΃p�X�̏ꍇ�́AINI�t�@�C���̃p�X����̑��΃p�X�Ƃ��ĊJ��
	@author ryoji
	@date 2007.05.19 �V�K�쐬�i_tfopen_absexe�x�[�X�j
*/
FILE* _tfopen_absini(LPCTSTR fname, LPCTSTR mode, BOOL bOrExedir/*=TRUE*/ )
{
	if( _IS_REL_PATH( fname ) ){
		TCHAR path[_MAX_PATH];
		if( bOrExedir )
			GetInidirOrExedir( path, fname );
		else
			GetInidir( path, fname );
		return _tfopen( path, mode );
	}
	return _tfopen( fname, mode );
}



/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� "c:\\"���̃��[�g�͎�菜���Ȃ� */
void CutLastYenFromDirectoryPath( TCHAR* pszFolder )
{
	if( 3 == _tcslen( pszFolder )
	 && pszFolder[1] == _T(':')
	 && pszFolder[2] == _T('\\')
	){
		/* �h���C�u��:\ */
	}else{
		/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� */
		int	nFolderLen;
		int	nCharChars;
		nFolderLen = _tcslen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CNativeT::GetCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && _T('\\') == pszFolder[nFolderLen - 1] ){
				pszFolder[nFolderLen - 1] = _T('\0');
			}
		}
	}
	return;
}




/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
void AddLastYenFromDirectoryPath( TCHAR* pszFolder )
{
	if( 3 == _tcslen( pszFolder )
	 && pszFolder[1] == _T(':')
	 && pszFolder[2] == _T('\\')
	){
		/* �h���C�u��:\ */
	}else{
		/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
		int	nFolderLen;
		int	nCharChars;
		nFolderLen = _tcslen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CNativeT::GetCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && _T('\\') == pszFolder[nFolderLen - 1] ){
			}else{
				pszFolder[nFolderLen] = _T('\\');
				pszFolder[nFolderLen + 1] = _T('\0');
			}
		}
	}
	return;
}



/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
void SplitPath_FolderAndFile( const TCHAR* pszFilePath, TCHAR* pszFolder, TCHAR* pszFile )
{
	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	int		nFolderLen;
	int		nCharChars;
	_tsplitpath( pszFilePath, szDrive, szDir, szFname, szExt );
	if( NULL != pszFolder ){
		_tcscpy( pszFolder, szDrive );
		_tcscat( pszFolder, szDir );
		/* �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� */
		nFolderLen = _tcslen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CNativeT::GetCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && _T('\\') == pszFolder[nFolderLen - 1] ){
				pszFolder[nFolderLen - 1] = _T('\0');
			}
		}
	}
	if( NULL != pszFile ){
		_tcscpy( pszFile, szFname );
		_tcscat( pszFile, szExt );
	}
	return;
}



/*! �����O�t�@�C�������擾���� 

	@param[in] pszFilePathSrc �ϊ����p�X��
	@param[out] pszFilePathDes ���ʏ������ݐ� (����MAX_PATH�̗̈悪�K�v)

	@date Oct. 2, 2005 genta GetFilePath API���g���ď�������
	@date Oct. 4, 2005 genta ���΃p�X����΃p�X�ɒ�����Ȃ�����
	@date Oct. 5, 2005 Moca  ���΃p�X���΃p�X�ɕϊ�����悤��
*/
BOOL GetLongFileName( const TCHAR* pszFilePathSrc, TCHAR* pszFilePathDes )
{
	TCHAR* name;
	TCHAR szBuf[_MAX_PATH + 1];
	int len = ::GetFullPathName( pszFilePathSrc, _MAX_PATH, szBuf, &name );
	if( len <= 0 || _MAX_PATH <= len ){
		len = ::GetLongPathName( pszFilePathSrc, pszFilePathDes, _MAX_PATH );
		if( len <= 0 || _MAX_PATH < len ){
			return FALSE;
		}
		return TRUE;
	}
	len = ::GetLongPathName( szBuf, pszFilePathDes, _MAX_PATH );
	if( len <= 0 || _MAX_PATH < len ){
		_tcscpy( pszFilePathDes, szBuf );
	}
	return TRUE;
}


/* �g���q�𒲂ׂ� */
BOOL CheckEXT( const TCHAR* pszPath, const TCHAR* pszExt )
{
	TCHAR	szExt[_MAX_EXT];
	TCHAR*	pszWork;
	_tsplitpath( pszPath, NULL, NULL, NULL, szExt );
	pszWork = szExt;
	if( pszWork[0] == _T('.') ){
		pszWork++;
	}
	if( 0 == _tcsicmp( pszExt, pszWork ) ){
		return TRUE;
	}else{
		return FALSE;
	}
}

/*! ���΃p�X�����肷��
	@author Moca
	@date 2003.06.23
*/
bool _IS_REL_PATH(const TCHAR* path)
{
	bool ret = true;
	if( ( _T('A') <= path[0] && path[0] <= _T('Z') || _T('a') <= path[0] && path[0] <= _T('z') )
		&& path[1] == _T(':') && path[2] == _T('\\')
		|| path[0] == _T('\\') && path[1] == _T('\\')
		 ){
		ret = false;
	}
	return ret;
}






/*! @brief �f�B���N�g���̐[�����v�Z����

	�^����ꂽ�p�X������f�B���N�g���̐[�����v�Z����D
	�p�X�̋�؂��\�D���[�g�f�B���N�g�����[��0�ŁC�T�u�f�B���N�g������
	�[����1���オ���Ă����D
 
	@date 2003.04.30 genta �V�K�쐬
*/
int CalcDirectoryDepth(
	const TCHAR* path	//!< [in] �[���𒲂ׂ����t�@�C��/�f�B���N�g���̃t���p�X
)
{
	int depth = 0;
 
	//	�Ƃ肠����\�̐��𐔂���
	for( CharPointerT p = path; *p != _T('\0'); ++p ){
		if( *p == _T('\\') ){
			++depth;
			//	�t���p�X�ɂ͓����Ă��Ȃ��͂������O�̂���
			//	.\�̓J�����g�f�B���N�g���Ȃ̂ŁC�[���Ɋ֌W�Ȃ��D
			while( p[1] == _T('.') && p[2] == _T('\\') ){
				p += 2;
			}
		}
	}
 
	//	�␳
	//	�h���C�u���̓p�X�̐[���ɐ����Ȃ�
	if( _T('A') <= path[0] && path[0] <= _T('Z') && path[1] == _T(':') && path[2] == _T('\\') ){
		//�t���p�X
		--depth; // C:\ �� \ �̓��[�g�̋L���Ȃ̂ŊK�w�[���ł͂Ȃ�
	}
	else if( path[0] == _T('\\') ){
		if( path[1] == _T('\\') ){
			//	�l�b�g���[�N�p�X
			//	�擪��2�̓l�b�g���[�N��\���C���̎��̓z�X�g���Ȃ̂�
			//	�f�B���N�g���K�w�Ƃ͖��֌W
			depth -= 3;
		}
		else {
			//	�h���C�u�������̃t���p�X
			//	�擪��\�͑ΏۊO
			--depth;
		}
	}
	return depth;
}


/*!
	@brief exe�t�@�C���̂���f�B���N�g���C�܂��͎w�肳�ꂽ�t�@�C�����̃t���p�X��Ԃ��D
	
	@author genta
	@date 2002.12.02 genta
	@date 2007.05.20 ryoji �֐����ύX�i���FGetExecutableDir�j�A�ėp�e�L�X�g�}�b�s���O��
*/
void GetExedir(
	LPTSTR	pDir,	//!< [out] EXE�t�@�C���̂���f�B���N�g����Ԃ��ꏊ�D�\��_MAX_PATH�̃o�b�t�@��p�ӂ��Ă������ƁD
	LPCTSTR	szFile	//!< [in]  �f�B���N�g�����Ɍ�������t�@�C�����D
)
{
	if( pDir == NULL )
		return;
	
	TCHAR	szPath[_MAX_PATH];
	// sakura.exe �̃p�X���擾
	::GetModuleFileName( ::GetModuleHandle(NULL), szPath, sizeof(szPath) );
	if( szFile == NULL ){
		SplitPath_FolderAndFile( szPath, pDir, NULL );
	}
	else {
		TCHAR	szDir[_MAX_PATH];
		SplitPath_FolderAndFile( szPath, szDir, NULL );
		auto_snprintf( pDir, _MAX_PATH, _T("%ts\\%ts"), szDir, szFile );
		pDir[_MAX_PATH - 1] = _T('\0');
	}
}

/*!
	@brief INI�t�@�C���̂���f�B���N�g���C�܂��͎w�肳�ꂽ�t�@�C�����̃t���p�X��Ԃ��D
	
	@author ryoji
	@date 2007.05.19 �V�K�쐬�iGetExedir�x�[�X�j
*/
void GetInidir(
	LPTSTR	pDir,				//!< [out] INI�t�@�C���̂���f�B���N�g����Ԃ��ꏊ�D�\��_MAX_PATH�̃o�b�t�@��p�ӂ��Ă������ƁD
	LPCTSTR szFile	/*=NULL*/	//!< [in] �f�B���N�g�����Ɍ�������t�@�C�����D
)
{
	if( pDir == NULL )
		return;
	
	TCHAR	szPath[_MAX_PATH];
	// sakura.ini �̃p�X���擾
	CShareData::getInstance()->GetIniFileName( szPath );
	if( szFile == NULL ){
		SplitPath_FolderAndFile( szPath, pDir, NULL );
	}
	else {
		TCHAR	szDir[_MAX_PATH];
		SplitPath_FolderAndFile( szPath, szDir, NULL );
		auto_snprintf( pDir, _MAX_PATH, _T("%ts\\%ts"), szDir, szFile );
		pDir[_MAX_PATH - 1] = _T('\0');
	}
}


/*!
	@brief INI�t�@�C���܂���EXE�t�@�C���̂���f�B���N�g���C�܂��͎w�肳�ꂽ�t�@�C�����̃t���p�X��Ԃ��iINI��D��j�D
	
	@author ryoji
	@date 2007.05.22 �V�K�쐬
*/
void GetInidirOrExedir(
	LPTSTR	pDir,								//!< [out] INI�t�@�C���܂���EXE�t�@�C���̂���f�B���N�g����Ԃ��ꏊ�D
												//         �\��_MAX_PATH�̃o�b�t�@��p�ӂ��Ă������ƁD
	LPCTSTR	szFile					/*=NULL*/,	//!< [in] �f�B���N�g�����Ɍ�������t�@�C�����D
	BOOL	bRetExedirIfFileEmpty	/*=FALSE*/	//!< [in] �t�@�C�����̎w�肪��̏ꍇ��EXE�t�@�C���̃t���p�X��Ԃ��D
)
{
	TCHAR	szInidir[_MAX_PATH];
	TCHAR	szExedir[_MAX_PATH];

	// �t�@�C�����̎w�肪��̏ꍇ��EXE�t�@�C���̃t���p�X��Ԃ��i�I�v�V�����j
	if( bRetExedirIfFileEmpty && (szFile == NULL || szFile[0] == _T('\0')) ){
		GetExedir( szExedir, szFile );
		::lstrcpy( pDir, szExedir );
		return;
	}

	// INI��̃t���p�X�����݂���΂��̃p�X��Ԃ�
	GetInidir( szInidir, szFile );
	if( _taccess(szInidir, 0) != -1 ){
		::lstrcpy( pDir, szInidir );
		return;
	}

	// EXE��̃t���p�X�����݂���΂��̃p�X��Ԃ�
	if( CShareData::getInstance()->IsPrivateSettings() ){	// INI��EXE�Ńp�X���قȂ�ꍇ
		GetExedir( szExedir, szFile );
		if( _taccess(szExedir, 0) != -1 ){
			::lstrcpy( pDir, szExedir );
			return;
		}
	}

	// �ǂ���ɂ����݂��Ȃ����INI��̃t���p�X��Ԃ�
	::lstrcpy( pDir, szInidir );
}




/**	�t�@�C���̑��݃`�F�b�N

	�w�肳�ꂽ�p�X�̃t�@�C�������݂��邩�ǂ������m�F����B
	
	@param path [in] ���ׂ�p�X��
	@param bFileOnly [in] true: �t�@�C���̂ݑΏ� / false: �f�B���N�g�����Ώ�
	
	@retval true  �t�@�C���͑��݂���
	@retval false �t�@�C���͑��݂��Ȃ�
	
	@author genta
	@date 2002.01.04 �V�K�쐬
*/
bool IsFileExists(const TCHAR* path, bool bFileOnly)
{
	WIN32_FIND_DATA fd;
	::ZeroMemory( &fd, sizeof(fd));

	HANDLE hFind = ::FindFirstFile( path, &fd );
	if( hFind != INVALID_HANDLE_VALUE ){
		::FindClose( hFind );
		if( bFileOnly == false ||
			( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
			return true;
	}
	return false;
}



/*!	�t�@�C���̍X�V�������擾

	@param[in] filename �t�@�C���̃p�X
	@param[out] ftime �X�V������Ԃ��ꏊ
	@return true: ����, false: FindFirstFile���s

	@author genta by assitance with ryoji
	@date 2005.10.22 new

	@note �������݌�Ƀt�@�C�����ăI�[�v�����ă^�C���X�^���v�𓾂悤�Ƃ����
	�t�@�C�����܂����b�N����Ă��邱�Ƃ�����C�㏑���֎~�ƌ�F����邱�Ƃ�����D
	FindFirstFile���g�����ƂŃt�@�C���̃��b�N��Ԃɉe�����ꂸ�Ƀ^�C���X�^���v��
	�擾�ł���D(ryoji)
*/
bool GetLastWriteTimestamp( const TCHAR* filename, FILETIME& ftime )
{
	HANDLE hFindFile;
	WIN32_FIND_DATA ffd;

	hFindFile = ::FindFirstFile( filename, &ffd );
	if( INVALID_HANDLE_VALUE != hFindFile )
	{
		::FindClose( hFindFile );
		ftime = ffd.ftLastWriteTime;
		return true;
	}
	//	�t�@�C����������Ȃ�����
	return false;
}
