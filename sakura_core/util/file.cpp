/*
	Copyright (C) 2002, SUI
	Copyright (C) 2003, MIK
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include <io.h>
#include "file.h"
#include "charset/CharPointer.h"
#include "util/module.h"
#include "util/window.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CFileNameManager.h"
#include "_main/CCommandLine.h"

bool fexist(LPCTSTR pszPath)
{
	return _taccess(pszPath,0)!=-1;
}

/*!	ファイル名の切り出し

	指定文字列からファイル名と認識される文字列を取り出し、
	先頭Offset及び長さを返す。
	
	@retval true ファイル名発見
	@retval false ファイル名は見つからなかった
	
	@date 2002.01.04 genta ファイル存在確認方法変更
	@date 2002.01.04 genta ディレクトリを検査対象外にする機能を追加
	@date 2003.01.15 matsumo gccのエラーメッセージ(:区切り)でもファイルを検出可能に
	@date 2004.05.29 genta C:\からファイルCが切り出されるのを防止
	@date 2004.11.13 genta/Moca ファイル名先頭の*?を考慮
	@date 2005.01.10 genta 変数名変更 j -> cur_pos
	@date 2005.01.23 genta 警告抑制のため，gotoをreturnに変更
	@date 2013.05.27 Moca 最長一致に変更
*/
bool IsFilePath(
	const wchar_t*	pLine,		//!< [in]  探査対象文字列
	size_t*			pnBgn,		//!< [out] 先頭offset。pLine + *pnBgnがファイル名先頭へのポインタ。
	size_t*			pnPathLen,	//!< [out] ファイル名の長さ
	bool			bFileOnly	//!< [in]  true: ファイルのみ対象 / false: ディレクトリも対象
)
{
	wchar_t	szJumpToFile[_MAX_PATH];
	wmemset( szJumpToFile, 0, _countof( szJumpToFile ) );

	size_t	nLineLen = wcslen( pLine );

	//先頭の空白を読み飛ばす
	size_t	i;
	for( i = 0; i < nLineLen; ++i ){
		wchar_t c = pLine[i];
		if( L' '!=c && L'\t'!=c && L'\"'!=c ){
			break;
		}
	}

	//	#include <ファイル名>の考慮
	//	#で始まるときは"または<まで読み飛ばす
	if( i < nLineLen && L'#' == pLine[i] ){
		for( ; i < nLineLen; ++i ){
			if( L'<'  == pLine[i] || L'\"' == pLine[i]){
				++i;
				break;
			}
		}
	}

	//	この時点で既に行末に達していたらファイル名は見つからない
	if( i >= nLineLen ){
		return false;
	}

	*pnBgn = i;
	size_t cur_pos = 0;
	size_t tmp_end = 0;
	for( ; i <= nLineLen && cur_pos + 1 < _countof(szJumpToFile); ++i ){
		//ファイル名終端を検知する
		if( WCODE::IsLineDelimiterExt(pLine[i]) || pLine[i] == L'\0' ){
			break;
		}

		//ファイル名終端を検知する
		if( ( i == nLineLen    ||
			  // 2002.01.08 YAZAKI タブ文字も。
			  // 2013.05.27 Moca 文字種追加
			  wcschr(L" \t(\")'`[]{};#!@&%$", pLine[i]) != NULL
			) &&
			szJumpToFile[0] != L'\0'
		){
			//	ファイル存在確認
			if( IsFileExists(to_tchar(szJumpToFile), bFileOnly)){
				tmp_end = cur_pos;
			}
		}

		// May 29, 2004 genta C:\の:はファイル区切りと見なして欲しくない
		if( cur_pos > 1 && pLine[i] == L':' ){   //@@@ 2003/1/15/ matsumo (for gcc)
			break;
		}
		//ファイル名に使えない文字が含まれていたら、即ループ終了
		if( !WCODE::IsValidFilenameChar(pLine[i]) ){
			break;
		}

		szJumpToFile[cur_pos] = pLine[i];
		cur_pos++;
	}

	//	Jan. 04, 2002 genta
	//	ファイル存在確認方法変更
	if( szJumpToFile[0] != L'\0' && IsFileExists(to_tchar(szJumpToFile), bFileOnly)){
		tmp_end = cur_pos;
	}
	if( tmp_end != 0 ){
		*pnPathLen = tmp_end;
		return true;
	}

	return false;

}

/*!
	ローカルドライブの判定

	@param[in] pszDrive ドライブ名を含むパス名
	
	@retval true ローカルドライブ
	@retval false リムーバブルドライブ．ネットワークドライブ．
	
	@author MIK
	@date 2001.03.29 MIK 新規作成
	@date 2001.12.23 YAZAKI MRUの別クラス化に伴う関数化
	@date 2002.01.28 genta 戻り値の型をBOOLからboolに変更．
	@date 2005.11.12 aroka 文字判定部変更
	@date 2006.01.08 genta CMRU::IsRemovableDriveとCEditDoc::IsLocalDriveが
		実質的に同じものだった
*/
bool IsLocalDrive( const TCHAR* pszDrive )
{
	TCHAR	szDriveType[_MAX_DRIVE+1];	// "A:\ "登録用
	long	lngRet;

	if( iswalpha(pszDrive[0]) ){
		auto_sprintf(szDriveType, _T("%tc:\\"), _totupper(pszDrive[0]));
		lngRet = GetDriveType( szDriveType );
		if( lngRet == DRIVE_REMOVABLE || lngRet == DRIVE_CDROM || lngRet == DRIVE_REMOTE )
		{
			return false;
		}
	}
	else if (pszDrive[0] == _T('\\') && pszDrive[1] == _T('\\')) {
		// ネットワークパス	2010/5/27 Uchi
		return false;
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


/*! fnameが相対パスの場合は、実行ファイルのパスからの相対パスとして開く
	@author Moca
	@date 2003.06.23
	@date 2007.05.20 ryoji 関数名変更（旧：fopen_absexe）、汎用テキストマッピング化
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

/*! fnameが相対パスの場合は、INIファイルのパスからの相対パスとして開く
	@author ryoji
	@date 2007.05.19 新規作成（_tfopen_absexeベース）
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



/* フォルダの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない */
void CutLastYenFromDirectoryPath( TCHAR* pszFolder )
{
	if( 3 == _tcslen( pszFolder )
	 && pszFolder[1] == _T(':')
	 && pszFolder[2] == _T('\\')
	){
		/* ドライブ名:\ */
	}else{
		/* フォルダの最後が半角かつ'\\'の場合は、取り除く */
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




/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
void AddLastYenFromDirectoryPath( CHAR* pszFolder )
{
	if( 3 == auto_strlen( pszFolder )
	 && pszFolder[1] == ':'
	 && pszFolder[2] == '\\'
	){
		/* ドライブ名:\ */
	}else{
		/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
		int	nFolderLen;
		int	nCharChars;
		nFolderLen = auto_strlen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CNativeA::GetCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && ('\\' == pszFolder[nFolderLen - 1] || '/' == pszFolder[nFolderLen - 1]) ){
			}else{
				pszFolder[nFolderLen] = '\\';
				pszFolder[nFolderLen + 1] = '\0';
			}
		}
	}
	return;
}

void AddLastYenFromDirectoryPath( WCHAR* pszFolder )
{
	if( 3 == auto_strlen( pszFolder )
	 && pszFolder[1] == L':'
	 && pszFolder[2] == L'\\'
	){
		/* ドライブ名:\ */
	}else{
		/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
		int	nFolderLen;
		nFolderLen = auto_strlen( pszFolder );
		if( 0 < nFolderLen ){
			if( L'\\' == pszFolder[nFolderLen - 1] || L'/' == pszFolder[nFolderLen - 1] ){
			}else{
				pszFolder[nFolderLen] = L'\\';
				pszFolder[nFolderLen + 1] = L'\0';
			}
		}
	}
	return;
}


/* ファイルのフルパスを、フォルダとファイル名に分割 */
/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
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
		/* フォルダの最後が半角かつ'\\'の場合は、取り除く */
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

/* フォルダ、ファイル名から、結合したパスを作成
 * [c:\work\test] + [aaa.txt] → [c:\work\test\aaa.txt]
 * フォルダ末尾に円記号があってもなくても良い。
 */
void Concat_FolderAndFile( const TCHAR* pszDir, const TCHAR* pszTitle, TCHAR* pszPath )
{
	TCHAR* out=pszPath;
	const TCHAR* in;

	//フォルダをコピー
	for( in=pszDir ; *in != '\0'; ){
		*out++ = *in++;
	}
	//円記号を付加
#if UNICODE
	if( *(out-1) != '\\' ){ *out++ = '\\'; }
#else
	if( *(out-1) != '\\' ||
		(1 == out - CNativeT::GetCharPrev( pszDir, out - pszDir, out )) ){
			*out++ = '\\';
	}
#endif
	//ファイル名をコピー
	for( in=pszTitle; *in != '\0'; ){
		*out++ = *in++;
	}
	*out = '\0';
	return;
}


/*! ロングファイル名を取得する 

	@param[in] pszFilePathSrc 変換元パス名
	@param[out] pszFilePathDes 結果書き込み先 (長さMAX_PATHの領域が必要)

	@date Oct. 2, 2005 genta GetFilePath APIを使って書き換え
	@date Oct. 4, 2005 genta 相対パスが絶対パスに直されなかった
	@date Oct. 5, 2005 Moca  相対パスを絶対パスに変換するように
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


/* 拡張子を調べる */
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

/*! 相対パスか判定する
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






/*! @brief ディレクトリの深さを計算する

	与えられたパス名からディレクトリの深さを計算する．
	パスの区切りは\．ルートディレクトリが深さ0で，サブディレクトリ毎に
	深さが1ずつ上がっていく．
 
	@date 2003.04.30 genta 新規作成
*/
int CalcDirectoryDepth(
	const TCHAR* path	//!< [in] 深さを調べたいファイル/ディレクトリのフルパス
)
{
	int depth = 0;
 
	//	とりあえず\の数を数える
	for( CharPointerT p = path; *p != _T('\0'); ++p ){
		if( *p == _T('\\') ){
			++depth;
			//	フルパスには入っていないはずだが念のため
			//	.\はカレントディレクトリなので，深さに関係ない．
			while( p[1] == _T('.') && p[2] == _T('\\') ){
				p += 2;
			}
		}
	}
 
	//	補正
	//	ドライブ名はパスの深さに数えない
	if( ((_T('A') <= path[0] && path[0] <= _T('Z')) || (_T('a') <= path[0] && path[0] <= _T('z')))
		&& path[1] == _T(':') && path[2] == _T('\\') ){
		//フルパス
		--depth; // C:\ の \ はルートの記号なので階層深さではない
	}
	else if( path[0] == _T('\\') ){
		if( path[1] == _T('\\') ){
			//	ネットワークパス
			//	先頭の2つはネットワークを表し，その次はホスト名なので
			//	ディレクトリ階層とは無関係
			depth -= 3;
		}
		else {
			//	ドライブ名無しのフルパス
			//	先頭の\は対象外
			--depth;
		}
	}
	return depth;
}


/*!
	@brief exeファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す．
	
	@author genta
	@date 2002.12.02 genta
	@date 2007.05.20 ryoji 関数名変更（旧：GetExecutableDir）、汎用テキストマッピング化
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
void GetExedir(
	LPTSTR	pDir,	//!< [out] EXEファイルのあるディレクトリを返す場所．予め_MAX_PATHのバッファを用意しておくこと．
	LPCTSTR	szFile	//!< [in]  ディレクトリ名に結合するファイル名．
)
{
	if( pDir == NULL )
		return;
	
	TCHAR	szPath[_MAX_PATH];
	// sakura.exe のパスを取得
	::GetModuleFileName( NULL, szPath, _countof(szPath) );
	if( szFile == NULL ){
		SplitPath_FolderAndFile( szPath, pDir, NULL );
	}
	else {
		TCHAR	szDir[_MAX_PATH];
		SplitPath_FolderAndFile( szPath, szDir, NULL );
		auto_snprintf_s( pDir, _MAX_PATH, _T("%ts\\%ts"), szDir, szFile );
	}
}

/*!
	@brief INIファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す．
	
	@author ryoji
	@date 2007.05.19 新規作成（GetExedirベース）
*/
void GetInidir(
	LPTSTR	pDir,				//!< [out] INIファイルのあるディレクトリを返す場所．予め_MAX_PATHのバッファを用意しておくこと．
	LPCTSTR szFile	/*=NULL*/	//!< [in] ディレクトリ名に結合するファイル名．
)
{
	if( pDir == NULL )
		return;
	
	std::tstring strProfileName = to_tchar(CCommandLine::getInstance()->GetProfileName());
	TCHAR	szPath[_MAX_PATH];

	// sakura.ini のパスを取得
	CFileNameManager::getInstance()->GetIniFileName( szPath, strProfileName.c_str() );
	if( szFile == NULL ){
		SplitPath_FolderAndFile( szPath, pDir, NULL );
	}
	else {
		TCHAR	szDir[_MAX_PATH];
		SplitPath_FolderAndFile( szPath, szDir, NULL );
		auto_snprintf_s( pDir, _MAX_PATH, _T("%ts\\%ts"), szDir, szFile );
	}
}


/*!
	@brief INIファイルまたはEXEファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す（INIを優先）．
	
	@author ryoji
	@date 2007.05.22 新規作成
*/
void GetInidirOrExedir(
	LPTSTR	pDir,								//!< [out] INIファイルまたはEXEファイルのあるディレクトリを返す場所．
												//         予め_MAX_PATHのバッファを用意しておくこと．
	LPCTSTR	szFile					/*=NULL*/,	//!< [in] ディレクトリ名に結合するファイル名．
	BOOL	bRetExedirIfFileEmpty	/*=FALSE*/	//!< [in] ファイル名の指定が空の場合はEXEファイルのフルパスを返す．
)
{
	TCHAR	szInidir[_MAX_PATH];
	TCHAR	szExedir[_MAX_PATH];

	// ファイル名の指定が空の場合はEXEファイルのフルパスを返す（オプション）
	if( bRetExedirIfFileEmpty && (szFile == NULL || szFile[0] == _T('\0')) ){
		GetExedir( szExedir, szFile );
		::lstrcpy( pDir, szExedir );
		return;
	}

	// INI基準のフルパスが実在すればそのパスを返す
	GetInidir( szInidir, szFile );
	if( fexist(szInidir) ){
		::lstrcpy( pDir, szInidir );
		return;
	}

	// EXE基準のフルパスが実在すればそのパスを返す
	if( CShareData::getInstance()->IsPrivateSettings() ){	// INIとEXEでパスが異なる場合
		GetExedir( szExedir, szFile );
		if( fexist(szExedir) ){
			::lstrcpy( pDir, szExedir );
			return;
		}
	}

	// どちらにも実在しなければINI基準のフルパスを返す
	::lstrcpy( pDir, szInidir );
}

/*!
	@brief INIファイルまたはEXEファイルのあるディレクトリの相対パスを返す（INIを優先）．
	@param pszPath [in] 対象パス
	@date 2013.06.26 novice 新規作成
*/
LPCTSTR GetRelPath( LPCTSTR pszPath )
{
	TCHAR szPath[_MAX_PATH + 1];
	LPCTSTR pszFileName = pszPath;

	GetInidir( szPath, _T("") );
	int nLen = auto_strlen( szPath );
	if( 0 == auto_strnicmp( szPath, pszPath, nLen ) ){
		pszFileName = pszPath + nLen;
	}else{
		GetExedir( szPath, _T("") );
		nLen = auto_strlen( szPath );
		if( 0 == auto_strnicmp( szPath, pszPath, nLen ) ){
			pszFileName = pszPath + nLen;
		}
	}

	return pszFileName;
}



/**	ファイルの存在チェック

	指定されたパスのファイルが存在するかどうかを確認する。
	
	@param path [in] 調べるパス名
	@param bFileOnly [in] true: ファイルのみ対象 / false: ディレクトリも対象
	
	@retval true  ファイルは存在する
	@retval false ファイルは存在しない
	
	@author genta
	@date 2002.01.04 新規作成
*/
bool IsFileExists(const TCHAR* path, bool bFileOnly)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile( path, &fd );
	if( hFind != INVALID_HANDLE_VALUE ){
		::FindClose( hFind );
		if( bFileOnly && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )return false;
		return true;
	}
	return false;
}

/**	ディレクトリチェック

	指定されたパスがディレクトリかどうかを確認する。

	@param pszPath [in] 調べるパス名

	@retval true  ディレクトリ
	@retval false ディレクトリではない
	
	@author ryoji
	@date 2009.08.20 新規作成
*/
bool IsDirectory(LPCTSTR pszPath)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile( pszPath, &fd );
	if(hFind!=INVALID_HANDLE_VALUE){
		::FindClose( hFind );
		return (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	return false;
}



/*!	ファイルの更新日時を取得

	@return true: 成功, false: FindFirstFile失敗

	@author genta by assitance with ryoji
	@date 2005.10.22 new

	@note 書き込み後にファイルを再オープンしてタイムスタンプを得ようとすると
	ファイルがまだロックされていることがあり，上書き禁止と誤認されることがある．
	FindFirstFileを使うことでファイルのロック状態に影響されずにタイムスタンプを
	取得できる．(ryoji)
*/
bool GetLastWriteTimestamp(
	const TCHAR*	pszFileName,	//!< [in]  ファイルのパス
	CFileTime*		pcFileTime		//!< [out] 更新日時を返す場所
)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA ffd;

	hFindFile = ::FindFirstFile( pszFileName, &ffd );
	if( INVALID_HANDLE_VALUE != hFindFile )
	{
		::FindClose( hFindFile );
		pcFileTime->SetFILETIME(ffd.ftLastWriteTime);
		return true;
	}
	else{
		//	ファイルが見つからなかった
		pcFileTime->ClearFILETIME();
		return false;
	}
}








// -----------------------------------------------------------------------------
//
//
//                   MY_SP.c by SUI
//
//



/* ============================================================================
my_splitpath( const char *CommandLine, char *drive, char *dir, char *fname, char *ext );

★ 概要
CommandLine に与えられたコマンドライン文字列の先頭から、実在するファイル・ディ
レクトリの部分のみを抽出し、その抽出部分に対して _splitpath() と同等の処理をお
こないます。
先頭部分に実在するファイル・ディレクトリ名が無い場合は空文字列が返ります。
文字列中の日本語(Shift_JISコードのみ)に対応しています。

★ プログラム記述内容について(言い訳あれこれ)
文字列の split 処理部だけにでも _splitpath() を使えばもっと短くなったのですが、
そうやらずに全て自前で処理している理由は、
・コンパイラによっては _splitpath() が日本語に対応していない可能性もある。
・_splitpath() って、コンパイラによって、詳細動作が微妙に異なるかもしれないから
　仕様をハッキリさせるためにもコンパイラに添付されている _splitpath() にあまり
　頼りたくなかった。
・というか、主に動作確認に使用していた LSI-C試食版にはそもそも _splitpath() が
　存在しないから、やらざるをえなかった。 :-(
という事によります。

※ "LFN library" -> http://webs.to/ken/

★ 詳細動作
my_splitpath( CommandLine, drive, dir, fname, ext );
CommandLine に文字列として D:\Test.ext が与えられた場合、
├・D:\Test.ext というファイルが存在する場合
│　drive = "D:"
│　dir   = "\"
│　fname = "Test"
│　ext   = ".ext"
├・D:\Test.ext というディレクトリが存在する場合
│　drive = "D:"
│　dir   = "\Test.ext\"
│　fname = ""
│　ext   = ""
└・D:\Test.ext というファイル・ディレクトリが存在しない場合、
　　├・D:ドライブは有効
　　│　drive = "D:"
　　│　dir   = "\"
　　│　fname = ""
　　│　ext   = ""
　　└・D:ドライブは無効
　　　　drive = ""
　　　　dir   = ""
　　　　fname = ""
　　　　ext   = ""
)=========================================================================== */

/* Shift_JIS 対応で検索対象文字を２個指定できる strrchr() みたいなもの。
/ 指定された２つの文字のうち、見つかった方(より後方の方)の位置を返す。
/ # strrchr( char *s , char c ) とは、文字列 s 中の最後尾の c を探し出す関数。
/ # 文字 c が見つかったら、その位置を返す。
/ # 文字 c が見つからない場合は NULL を返す。 */
char *sjis_strrchr2( const char *pt , const char ch1 , const char ch2 ){
	const char *pf = NULL;
	while( *pt != '\0' ){	/* 文字列の終端まで調べる。 */
		if( ( *pt == ch1 ) || ( *pt == ch2 ) )	pf = pt;	/* pf = 検索文字の位置 */
		if( _IS_SJIS_1(*pt) )	pt++;	/* Shift_JIS の1文字目なら、次の1文字をスキップ */
		if( *pt != '\0' )		pt++;	/* 次の文字へ */
	}
	return	(char *)pf;
}
wchar_t* wcsrchr2( const wchar_t *pt , const wchar_t ch1 , const wchar_t ch2 ){
	const wchar_t *pf = NULL;
	while( *pt != L'\0' ){	/* 文字列の終端まで調べる。 */
		if( ( *pt == ch1 ) || ( *pt == ch2 ) )	pf = pt;	/* pf = 検索文字の位置 */
		if( *pt != '\0' )		pt++;	/* 次の文字へ */
	}
	return	(wchar_t *)pf;
}

#define		GetExistPath_NO_DriveLetter	0	/* ドライブレターが無い */
#define		GetExistPath_IV_Drive		1	/* ドライブが無効 */
#define		GetExistPath_AV_Drive		2	/* ドライブが有効 */

void	GetExistPath( char *po , const char *pi )
{
	char	*pw,*ps;
	int		cnt;
	char	drv[4] = "_:\\";
	int		dl;		/* ドライブの状態 */

	/* pi の内容を
	/ ・ " を削除しつつ
	/ ・ / を \ に変換しつつ(Win32API では / も \ と同等に扱われるから)
	/ ・最大 ( _MAX_PATH -1 ) 文字まで
	/ po にコピーする。 */
	for( pw=po,cnt=0 ; ( *pi != '\0' ) && ( cnt < _MAX_PATH -1 ) ; pi++ ){
		/* /," 共に Shift_JIS の漢字コード中には含まれないので Shift_JIS 判定は不要。 */
		if( *pi == '\"' )	continue;		/*  " なら何もしない。次の文字へ */
		if( *pi == '/' )	*pw++ = '\\';	/*  / なら \ に変換してコピー    */
		else				*pw++ = *pi;	/* その他の文字はそのままコピー  */
		cnt++;	/* コピーした文字数 ++ */
	}
	*pw = '\0';		/* 文字列終端 */

	dl = GetExistPath_NO_DriveLetter;	/*「ドライブレターが無い」にしておく*/
	if(
		( *(po+1) == ':' )&&
		( ACODE::IsAZ(*po) )
	){	/* 先頭にドライブレターがある。そのドライブが有効かどうか判定する */
		drv[0] = *po;
		if( access(drv,0) == 0 )	dl = GetExistPath_AV_Drive;		/* 有効 */
		else						dl = GetExistPath_IV_Drive;		/* 無効 */
	}

	if( dl == GetExistPath_IV_Drive ){	/* ドライブ自体が無効 */
		/* フロッピーディスク中のファイルが指定されていて、
		　 そのドライブにフロッピーディスクが入っていない、とか */
		*po = '\0';	/* 返値文字列 = "";(空文字列) */
		return;		/* これ以上何もしない */
	}

	/* ps = 検索開始位置 */
	ps = po;	/* ↓文字列の先頭が \\ なら、\ 検索処理の対象から外す */
	if( ( *po == '\\' )&&( *(po+1) == '\\' ) )	ps +=2;

	if( *ps == '\0' ){	/* 検索対象が空文字列なら */
		*po = '\0';		/* 返値文字列 = "";(空文字列) */
		return;			/*これ以上何もしない */
	}

	for(;;){
		if( access(po,0) == 0 )	break;	/* 有効なパス文字列が見つかった */
		/* ↓文字列最後尾の \ または ' ' を探し出し、そこを文字列終端にする。*/

		pw = sjis_strrchr2(ps,'\\',' ');	/* 最末尾の \ か ' ' を探す。 */
		if ( pw == NULL ){	/* 文字列中に '\\' も ' ' も無かった */
			/* 例えば "C:testdir" という文字列が来た時に、"C:testdir" が実在
			　 しなくとも C:ドライブが有効なら "C:" という文字列だけでも返し
			　 たい。以下↓は、そのための処理。 */
			if( dl == GetExistPath_AV_Drive ){
				/* 先頭に有効なドライブのドライブレターがある。 */
				*(po+2) = '\0';		/* ドライブレター部の文字列のみ返す */
			}
			else{	/* 有効なパス部分が全く見つからなかった */
				*po = '\0';	/* 返値文字列 = "";(空文字列) */
			}
			break;		/* ループを抜ける */
		}
		/* ↓ルートディレクトリを引っかけるための処理 */
		if( ( *pw == '\\' )&&( *(pw-1) == ':' ) ){	/* C:\ とかの \ っぽい */
			* (pw+1) = '\0';		/* \ の後ろの位置を文字列の終端にする。 */
			if( access(po,0) == 0 )	break;	/* 有効なパス文字列が見つかった */
		}
		*pw = '\0';		/* \ か ' ' の位置を文字列の終端にする。 */
		/* ↓末尾がスペースなら、スペースを全て削除する */
		while( ( pw != ps ) && ( *(pw-1) == ' ' ) )	* --pw = '\0';
	}

	return;
}

void GetExistPathW( wchar_t *po , const wchar_t *pi )
{
	wchar_t	*pw,*ps;
	int		cnt;
	wchar_t	drv[4] = L"_:\\";
	int		dl;		/* ドライブの状態 */

	/* pi の内容を
	/ ・ " を削除しつつ
	/ ・ / を \ に変換しつつ(Win32API では / も \ と同等に扱われるから)
	/ ・最大 ( _MAX_PATH-1 ) 文字まで
	/ po にコピーする。 */
	for( pw=po,cnt=0 ; ( *pi != L'\0' ) && ( cnt < _MAX_PATH-1 ) ; pi++ ){
		/* /," 共に Shift_JIS の漢字コード中には含まれないので Shift_JIS 判定は不要。 */
		if( *pi == L'\"' )	continue;		/*  " なら何もしない。次の文字へ */
		if( *pi == L'/' )	*pw++ = L'\\';	/*  / なら \ に変換してコピー    */
		else				*pw++ = *pi;	/* その他の文字はそのままコピー  */
		cnt++;	/* コピーした文字数 ++ */
	}
	*pw = L'\0';		/* 文字列終端 */

	dl = GetExistPath_NO_DriveLetter;	/*「ドライブレターが無い」にしておく*/
	if( *(po+1)==L':' && WCODE::IsAZ(*po) ){	/* 先頭にドライブレターがある。そのドライブが有効かどうか判定する */
		drv[0] = *po;
		if( _waccess(drv,0) == 0 )	dl = GetExistPath_AV_Drive;		/* 有効 */
		else						dl = GetExistPath_IV_Drive;		/* 無効 */
	}

	if( dl == GetExistPath_IV_Drive ){	/* ドライブ自体が無効 */
		/* フロッピーディスク中のファイルが指定されていて、
		　 そのドライブにフロッピーディスクが入っていない、とか */
		*po = L'\0';	/* 返値文字列 = "";(空文字列) */
		return;		/* これ以上何もしない */
	}

	/* ps = 検索開始位置 */
	ps = po;	/* ↓文字列の先頭が \\ なら、\ 検索処理の対象から外す */
	if( ( *po == L'\\' )&&( *(po+1) == L'\\' ) )	ps +=2;

	if( *ps == L'\0' ){	/* 検索対象が空文字列なら */
		*po = L'\0';		/* 返値文字列 = "";(空文字列) */
		return;			/*これ以上何もしない */
	}

	for(;;){
		if( _waccess(po,0) == 0 )	break;	/* 有効なパス文字列が見つかった */
		/* ↓文字列最後尾の \ または ' ' を探し出し、そこを文字列終端にする。*/

		pw = wcsrchr2(ps,'\\',' ');	/* 最末尾の \ か ' ' を探す。 */
		if ( pw == NULL ){	/* 文字列中に '\\' も ' ' も無かった */
			/* 例えば "C:testdir" という文字列が来た時に、"C:testdir" が実在
			　 しなくとも C:ドライブが有効なら "C:" という文字列だけでも返し
			　 たい。以下↓は、そのための処理。 */
			if( dl == GetExistPath_AV_Drive ){
				/* 先頭に有効なドライブのドライブレターがある。 */
				*(po+2) = L'\0';		/* ドライブレター部の文字列のみ返す */
			}
			else{	/* 有効なパス部分が全く見つからなかった */
				*po = L'\0';	/* 返値文字列 = "";(空文字列) */
			}
			break;		/* ループを抜ける */
		}
		/* ↓ルートディレクトリを引っかけるための処理 */
		if( ( *pw == L'\\' )&&( *(pw-1) == L':' ) ){	/* C:\ とかの \ っぽい */
			* (pw+1) = L'\0';		/* \ の後ろの位置を文字列の終端にする。 */
			if( _waccess(po,0) == 0 )	break;	/* 有効なパス文字列が見つかった */
		}
		*pw = L'\0';		/* \ か ' ' の位置を文字列の終端にする。 */
		/* ↓末尾がスペースなら、スペースを全て削除する */
		while( ( pw != ps ) && ( *(pw-1) == L' ' ) )	* --pw = L'\0';
	}

	return;
}

#ifndef _UNICODE
/* 与えられたコマンドライン文字列の先頭部分から実在するファイル・ディレクトリ
　 のパス文字列を抽出し、そのパスを分解して drv dir fnm ext に書き込む。
　 先頭部分に有効なパス名が存在しない場合、全てに空文字列が返る。 */
void	my_splitpath ( const char *comln , char *drv,char *dir,char *fnm,char *ext )
{
	char	ppp[_MAX_PATH];		/* パス格納（作業用） */
	char	*pd;
	char	*pf;
	char	*pe;
	char	ch;
	DWORD	attr;
	int		a_dir;

	if( drv != NULL )	*drv = '\0';
	if( dir != NULL )	*dir = '\0';
	if( fnm != NULL )	*fnm = '\0';
	if( ext != NULL )	*ext = '\0';
	if( *comln == '\0' )	return;

	/* コマンドライン先頭部分の実在するパス名を ppp に書き出す。 */
	GetExistPath( ppp , comln );

	if( *ppp != '\0' ) {	/* ファイル・ディレクトリが存在する場合 */
		/* 先頭文字がドライブレターかどうか判定し、
		　 pd = ディレクトリ名の先頭位置に設定する。 */
		pd = ppp;
		if(
			( *(pd+1)==':' )&&
			( ACODE::IsAZ(*pd) )
		){	/* 先頭にドライブレターがある。 */
			pd += 2;	/* pd = ドライブレター部の後ろ         */
		}				/*      ( = ディレクトリ名の先頭位置 ) */
		/* ここまでで、pd = ディレクトリ名の先頭位置 */

		attr =  GetFileAttributesA(ppp);
		a_dir = ( attr & FILE_ATTRIBUTE_DIRECTORY ) ?  1 : 0;
		if( ! a_dir ){	/* 見つけた物がファイルだった場合。 */
			pf = sjis_strrchr2(ppp,'\\','\\');	/* 最末尾の \ を探す。 */
			if(pf != NULL)	pf++;		/* 見つかった→  pf=\の次の文字の位置*/
			else			pf = pd;	/* 見つからない→pf=パス名の先頭位置 */
			/* ここまでで pf = ファイル名の先頭位置 */
			pe = sjis_strrchr2(pf,'.','.');		/* 最末尾の '.' を探す。 */
			if( pe != NULL ){					/* 見つかった(pe = '.'の位置)*/
				if( ext != NULL ){	/* 拡張子を返値として書き込む。 */
					strncpy(ext,pe,_MAX_EXT -1);
					ext[_MAX_EXT -1] = '\0';
				}
				*pe = '\0';	/* 区切り位置を文字列終端にする。pe = 拡張子名の先頭位置。 */
			}
			if( fnm != NULL ){	/* ファイル名を返値として書き込む。 */
				strncpy(fnm,pf,_MAX_FNAME -1);
				fnm[_MAX_FNAME -1] = '\0';
			}
			*pf = '\0';	/* ファイル名の先頭位置を文字列終端にする。 */
		}
		/* ここまでで文字列 ppp はドライブレター＋ディレクトリ名のみになっている */
		if( dir != NULL ){
			/* ディレクトリ名の最後の文字が \ ではない場合、\ にする。 */

			/* ↓最後の文字を ch に得る。(ディレクトリ文字列が空の場合 ch='\\' となる) */
			for( ch = '\\' , pf = pd ; *pf != '\0' ; pf++ ){
				ch = *pf;
				if( _IS_SJIS_1(*pf) )	pf++;	/* Shift_JIS の1文字目なら次の1文字をスキップ */
			}
			/* 文字列が空でなく、かつ、最後の文字が \ でなかったならば \ を追加。 */
			if( ( ch != '\\' ) && ( strlen(ppp) < _MAX_PATH -1 ) ){
				*pf++ = '\\';	*pf = '\0';
			}

			/* ディレクトリ名を返値として書き込む。 */
			strncpy(dir,pd,_MAX_DIR -1);
			dir[_MAX_DIR -1] = '\0';
		}
		*pd = '\0';		/* ディレクトリ名の先頭位置を文字列終端にする。 */
		if( drv != NULL ){	/* ドライブレターを返値として書き込む。 */
			strncpy(drv,ppp,_MAX_DRIVE -1);
			drv[_MAX_DRIVE -1] = '\0';
		}
	}
	return;
}

#else

/* 与えられたコマンドライン文字列の先頭部分から実在するファイル・ディレクトリ
　 のパス文字列を抽出し、そのパスを分解して drv dir fnm ext に書き込む。
　 先頭部分に有効なパス名が存在しない場合、全てに空文字列が返る。 */
void my_splitpath_w (
	const wchar_t *comln,
	wchar_t *drv,
	wchar_t *dir,
	wchar_t *fnm,
	wchar_t *ext
)
{
	wchar_t	ppp[_MAX_PATH];		/* パス格納（作業用） */
	wchar_t	*pd;
	wchar_t	*pf;
	wchar_t	*pe;
	wchar_t	ch;
	DWORD	attr;
	int		a_dir;

	if( drv != NULL )	*drv = L'\0';
	if( dir != NULL )	*dir = L'\0';
	if( fnm != NULL )	*fnm = L'\0';
	if( ext != NULL )	*ext = L'\0';
	if( *comln == L'\0' )	return;

	/* コマンドライン先頭部分の実在するパス名を ppp に書き出す。 */
	GetExistPathW( ppp , comln );

	if( *ppp != L'\0' ) {	/* ファイル・ディレクトリが存在する場合 */
		/* 先頭文字がドライブレターかどうか判定し、
		　 pd = ディレクトリ名の先頭位置に設定する。 */
		pd = ppp;
		if(*(pd+1)==L':' && WCODE::IsAZ(*pd)){	/* 先頭にドライブレターがある。 */
			pd += 2;	/* pd = ドライブレター部の後ろ         */
		}				/*      ( = ディレクトリ名の先頭位置 ) */
		/* ここまでで、pd = ディレクトリ名の先頭位置 */

		attr =  GetFileAttributesW(ppp);
		a_dir = ( attr & FILE_ATTRIBUTE_DIRECTORY ) ?  1 : 0;

		if( ! a_dir ){	/* 見つけた物がファイルだった場合。 */
			pf = wcsrchr(ppp,L'\\');	/* 最末尾の \ を探す。 */
			if(pf != NULL)	pf++;		/* 見つかった→  pf=\の次の文字の位置*/
			else			pf = pd;	/* 見つからない→pf=パス名の先頭位置 */
			/* ここまでで pf = ファイル名の先頭位置 */
			pe = wcsrchr(pf,L'.');		/* 最末尾の '.' を探す。 */
			if( pe != NULL ){					/* 見つかった(pe = L'.'の位置)*/
				if( ext != NULL ){	/* 拡張子を返値として書き込む。 */
					wcsncpy(ext,pe,_MAX_EXT-1);
					ext[_MAX_EXT -1] = L'\0';
				}
				*pe = L'\0';	/* 区切り位置を文字列終端にする。pe = 拡張子名の先頭位置。 */
			}
			if( fnm != NULL ){	/* ファイル名を返値として書き込む。 */
				wcsncpy(fnm,pf,_MAX_FNAME-1);
				fnm[_MAX_FNAME -1] = L'\0';
			}
			*pf = L'\0';	/* ファイル名の先頭位置を文字列終端にする。 */
		}
		/* ここまでで文字列 ppp はドライブレター＋ディレクトリ名のみになっている */
		if( dir != NULL ){
			/* ディレクトリ名の最後の文字が \ ではない場合、\ にする。 */

			/* ↓最後の文字を ch に得る。(ディレクトリ文字列が空の場合 ch=L'\\' となる) */
			for( ch = L'\\' , pf = pd ; *pf != L'\0' ; pf++ ){
				ch = *pf;
			}
			/* 文字列が空でなく、かつ、最後の文字が \ でなかったならば \ を追加。 */
			if( ( ch != L'\\' ) && ( wcslen(ppp) < _MAX_PATH -1 ) ){
				*pf++ = L'\\';	*pf = L'\0';
			}

			/* ディレクトリ名を返値として書き込む。 */
			wcsncpy(dir,pd,_MAX_DIR -1);
			dir[_MAX_DIR -1] = L'\0';
		}
		*pd = L'\0';		/* ディレクトリ名の先頭位置を文字列終端にする。 */
		if( drv != NULL ){	/* ドライブレターを返値として書き込む。 */
			wcsncpy(drv,ppp,_MAX_DRIVE -1);
			drv[_MAX_DRIVE -1] = L'\0';
		}
	}
	return;
}
#endif

//
//
//
//
//
// -----------------------------------------------------------------------------
int FileMatchScore( const TCHAR *file1, const TCHAR *file2 );

// フルパスからファイル名の.以降を分離する
// 2014.06.15 フォルダ名に.が含まれた場合、フォルダが分離されたのを修正
static void FileNameSepExt( const TCHAR *file, TCHAR* pszFile, TCHAR* pszExt )
{
	const TCHAR* folderPos = file;
	const TCHAR* x = folderPos;
	while( x ){
		x = auto_strchr(folderPos, _T('\\'));
		if( x ){
			x++;
			folderPos = x;
		}
	}
	const TCHAR* p = auto_strchr(folderPos, _T('.'));
	if( p ){
		auto_memcpy(pszFile, file, p - file);
		pszFile[p - file] = _T('\0');
		auto_strcpy(pszExt, p);
	}else{
		auto_strcpy(pszFile, file);
		pszExt[0] = _T('\0');
	}
}

int FileMatchScoreSepExt( const TCHAR *file1, const TCHAR *file2 )
{
	TCHAR szFile1[_MAX_PATH];
	TCHAR szFile2[_MAX_PATH];
	TCHAR szFileExt1[_MAX_PATH];
	TCHAR szFileExt2[_MAX_PATH];
	FileNameSepExt(file1, szFile1, szFileExt1);
	FileNameSepExt(file2, szFile2, szFileExt2);
	int score = FileMatchScore(szFile1, szFile2);
	score += FileMatchScore(szFileExt1, szFileExt2);
	return score;
}

/*!	2つのファイル名の最長一致部分の長さを返す
*/
int FileMatchScore( const TCHAR *file1, const TCHAR *file2 )
{
	int score = 0;
	int len1 = auto_strlen(file1);
	int len2 = auto_strlen(file2);
	if( len1 < len2 ){
		const TCHAR * tmp = file1;
		file1 = file2;
		file2 = tmp;
		int tmpLen = len1;
		len1 = len2;
		len2 = tmpLen;
	}
	for( int i = 0; i < len1; ){
		for( int k = 0; k < len2 && score < (len2 - k); ){
			int tmpScore = 0;
			for( int m = k; m < len2; ){
				int pos1 = i + (m - k);
				int chars1 = (Int)CNativeT::GetSizeOfChar(file1, len1, pos1);
				int chars2 = (Int)CNativeT::GetSizeOfChar(file2, len2, m);
				if( chars1 == chars2 ){
					if( chars1 == 1 ){
						if( _tcs_tolower(file1[pos1]) == _tcs_tolower(file2[m]) ){
							tmpScore += chars1;
						}else{
							break;
						}
					}else{
						if( 0 == auto_strnicmp(&file1[pos1], &file2[m], chars1) ){
							tmpScore += chars1;
						}else{
							break;
						}
					}
				}else{
					break;
				}
				m += t_max(1, chars1);
			}
			if( score < tmpScore ){
				score = tmpScore;
			}
			k += t_max(1, (int)(Int)CNativeT::GetSizeOfChar(file2, len2, k));
		}
		i += t_max(1, (int)(Int)CNativeT::GetSizeOfChar(file1, len1, i));
	}
	return score;
}

/*! 指定幅までに文字列を省略
	@date 2014.06.12 新規作成 Moca
*/
void GetStrTrancateWidth( TCHAR* dest, int nSize, const TCHAR* path, HDC hDC, int nPxWidth )
{
	// できるだけ左側から表示
	// \\server\dir...
	const int nPathLen = auto_strlen(path);
	CTextWidthCalc calc(hDC);
	if( calc.GetTextWidth(path) <= nPxWidth ){
		_tcsncpy_s(dest, nSize, path, _TRUNCATE);
		return;
	}
	std::tstring strTemp;
	std::tstring strTempOld;
	int nPos = 0;
	while( path[nPos] != _T('\0') ){
		strTemp.assign(path, nPos);
		std::tstring strTemp2 = strTemp;
		strTemp2 += _T("...");
		if( nPxWidth < calc.GetTextWidth(strTemp2.c_str()) ){
			// 入りきらなかったので1文字前までをコピー
			_tcsncpy_s(dest, t_max(0, nSize - 3), strTempOld.c_str(), _TRUNCATE);
			_tcscat_s(dest, nSize, _T("..."));
			return;
		}
		strTempOld = strTemp;
		nPos += t_max(1, (int)(Int)CNativeT::GetSizeOfChar(path, nPathLen, nPos));
	}
	// 全部表示(ここには来ないはず)
	_tcsncpy_s(dest, nSize, path, _TRUNCATE);
}

/*! パスの省略表示
	in  C:\sub1\sub2\sub3\file.ext
	out C:\...\sub3\file.ext
	@date 2014.06.12 新規作成 Moca
*/
void GetShortViewPath( TCHAR* dest, int nSize, const TCHAR* path, HDC hDC, int nPxWidth, bool bFitMode )
{
	int nLeft = 0; // 左側固定表示部分
	int nSkipLevel = 1;
	const int nPathLen = auto_strlen(path);
	CTextWidthCalc calc(hDC);
	if( calc.GetTextWidth(path) <= nPxWidth ){
		// 全部表示可能
		_tcsncpy_s(dest, nSize, path, _TRUNCATE);
		return;
	}
	if( path[0] == _T('\\') && path[1] == _T('\\') ){
		if( path[2] == _T('?') && path[4] == _T('\\') ){
			// [\\?\A:\]
			nLeft = 4;
		}else{
			nSkipLevel = 2; // [\\server\dir\] の2階層飛ばす
			nLeft = 2;
		}
	}else{
		// http://server/ とか ftp://server/ とかを保持
		int nTop = 0;
		while( path[nTop] != _T('\0') && path[nTop] != _T('/') ){
			nTop += t_max(1, (int)(Int)CNativeT::GetSizeOfChar(path, nPathLen, nTop));
		}
		if( 0 < nTop && path[nTop - 1] == ':' ){
			// 「ほにゃらら:/」だった /が続いてる間飛ばす
			while( path[nTop] == _T('/') ){
				nTop += t_max(1, (int)(Int)CNativeT::GetSizeOfChar(path, nPathLen, nTop));
			}
			nLeft = nTop;
		}
	}
	for( int i = 0; i < nSkipLevel; i++ ){
		while( path[nLeft] != _T('\0') && path[nLeft] != _T('\\') && path[nLeft] != _T('/') ){
			nLeft += t_max(1, (int)(Int)CNativeT::GetSizeOfChar(path, nPathLen, nLeft));
		}
		if( path[nLeft] != _T('\0') ){
			if( i + 1 < nSkipLevel ){
				nLeft++;
			}
		}else{
			if( bFitMode ){
				GetStrTrancateWidth(dest, nSize, path, hDC, nPxWidth);
				return;
			}
			// ここで終端なら全部表示
			_tcsncpy_s(dest, nSize, path, _TRUNCATE);
			return;
		}
	}
	int nRight = nLeft; // 右側の表示開始位置(nRightは\を指している)
	while( path[nRight] != _T('\0') ){
		int nNext = nRight;
		nNext++;
		while( path[nNext] != _T('\0') && path[nNext] != _T('\\') && path[nNext] != _T('/') ){
			nNext += t_max(1, (int)(Int)CNativeT::GetSizeOfChar(path, nPathLen, nNext));
		}
		if( path[nNext] != _T('\0') ){
			// サブフォルダ省略
			// C:\...\dir\file.ext
			std::tstring strTemp(path, nLeft + 1);
			if( nLeft + 1 < nRight ){
				strTemp += _T("...");
			}
			strTemp += &path[nRight];
			if( calc.GetTextWidth(strTemp.c_str()) <= nPxWidth ){
				_tcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
				return;
			}
			// C:\...\dir\   フォルダパスだった。最後のフォルダを表示
			if( path[nNext+1] == _T('\0') ){
				if( bFitMode ){
					GetStrTrancateWidth(dest, nSize, strTemp.c_str(), hDC, nPxWidth);
					return;
				}
				_tcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
				return;
			}
			nRight = nNext;
		}else{
			break;
		}
	}
	// nRightより右に\が見つからなかった=ファイル名だったのでファイル名表示
	// C:\...\file.ext
	int nLeftLen = nLeft;
	if( nLeftLen && nLeftLen != nRight ){
		nLeftLen++;
	}
	std::tstring strTemp(path, nLeftLen);
	if( nLeft != nRight ){
		strTemp += _T("...");
	}
	strTemp += &path[nRight];
	if( bFitMode ){
		if( calc.GetTextWidth(strTemp.c_str()) <= nPxWidth ){
			_tcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
			return;
		}
		// ファイル名(か左側固定部)が長すぎてはいらない
		int nExtPos = -1;
		{
			// 拡張子の.を探す
			int nExt = nRight;
			while( path[nExt] != _T('\0') ){
				if( path[nExt] == _T('.') ){
					nExtPos = nExt;
				}
				nExt += t_max(1, (int)(Int)CNativeT::GetSizeOfChar(path, nPathLen, nExt));
			}
		}
		if( nExtPos != -1 ){
			std::tstring strLeftFile(path, nLeftLen); // [C:\]  
			if( nLeft != nRight ){
				strLeftFile += _T("..."); // C:\...
			}
			int nExtWidth = calc.GetTextWidth(&path[nExtPos]);
			int nLeftWidth = calc.GetTextWidth(strLeftFile.c_str());
			int nFileNameWidth = nPxWidth - nLeftWidth - nExtWidth;
			if( 0 < nFileNameWidth ){
				// 拡張子は省略しない(ファイルタイトルを省略)
				std::tstring strFile(&path[nRight], nExtPos - nRight); // \longfilename
				strLeftFile += strFile; // C:\...\longfilename
				int nExtLen = nPathLen - nExtPos;
				GetStrTrancateWidth(dest, t_max(0, nSize - nExtLen), strLeftFile.c_str(), hDC, nPxWidth - nExtWidth);
				_tcscat_s(dest, nSize, &path[nExtPos+1]); // 拡張子連結 C:\...\longf...ext
			}else{
				// ファイル名が置けないくらい拡張子か左側が長い。パスの左側を優先して残す
				GetStrTrancateWidth(dest, nSize, strTemp.c_str(), hDC, nPxWidth);
			}
		}else{
			// 拡張子はなかった。左側から残す
			GetStrTrancateWidth(dest, nSize, strTemp.c_str(), hDC, nPxWidth);
		}
		return;
	}
	_tcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
}
