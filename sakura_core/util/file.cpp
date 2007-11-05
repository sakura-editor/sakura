#include "stdafx.h"
#include "file.h"
#include "CharPointer.h"
#include "util/module.h"
#include "CShareData.h"
#include <io.h>

/**	ファイル名の切り出し

	指定文字列からファイル名と認識される文字列を取り出し、
	先頭Offset及び長さを返す。
	
	@param pLine [in] 探査対象文字列
	@param pnBgn [out] 先頭offset。pLine + *pnBgnがファイル名先頭へのポインタ。
	@param pnPathLen [out] ファイル名の長さ
	@param bFileOnly [in] true: ファイルのみ対象 / false: ディレクトリも対象
	
	@retval true ファイル名発見
	@retval false ファイル名は見つからなかった
	
	@date 2002.01.04 genta ディレクトリを検査対象外にする機能を追加
	@date 2003.01.15 matsumo gccのエラーメッセージ(:区切り)でもファイルを検出可能に
	@date 2004.05.29 genta C:\からファイルCが切り出されるのを防止
	@date 2004.11.13 genta/Moca ファイル名先頭の*?を考慮
	@date 2005.01.10 genta 変数名変更 j -> cur_pos
	@date 2005.01.23 genta 警告抑制のため，gotoをreturnに変更
	
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

	//先頭の空白を読み飛ばす
	for( i = 0; i < nLineLen; ++i ){
		if( L' ' != pLine[i] &&
			L'\t' != pLine[i] &&
			L'\"' != pLine[i]
		){
			break;
		}
	}

	//	#include <ファイル名>の考慮
	//	#で始まるときは"または<まで読み飛ばす
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

	//	この時点で既に行末に達していたらファイル名は見つからない
	if( i >= nLineLen ){
		return false;
	}
	*pnBgn = i;
	int cur_pos = 0;
	for( ; i <= nLineLen && cur_pos + 1 < _countof(szJumpToFile); ++i ){
		if( ( i == nLineLen    ||
			  pLine[i] == L' '  ||
			  pLine[i] == L'\t' ||	//@@@ 2002.01.08 YAZAKI タブ文字も。
			  pLine[i] == L'('  ||
			  pLine[i] == L'\r' ||
			  pLine[i] == L'\n' ||
			  pLine[i] == L'\0' ||
			  pLine[i] == L'>'  ||
			  // May 29, 2004 genta C:\の:はファイル区切りと見なして欲しくない
			  ( cur_pos > 1 && pLine[i] == L':' ) ||   //@@@ 2003/1/15/ matsumo (for gcc)
			  pLine[i] == L'"'
			) &&
			0 < wcslen( szJumpToFile )
		){
			//	Jan. 04, 2002 genta
			//	ファイル存在確認方法変更
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
//	From Here Sept. 27, 2000 JEPRO タグジャンプできないのは以下の文字が1バイトコードで現れるときのみとした。
//	(SJIS2バイトコードの2バイト目に現れる場合はパス名使用禁止文字とは認識しないで無視するように変更)
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
			// 上の文字がSJIS2バイトコードの2バイト目でないことを、1つ前の文字がSJIS2バイトコードの1バイト目でないことで判断する
			//	Oct. 5, 2002 genta
			//	2004.11.13 Moca/genta 先頭に上の文字がある場合の考慮を追加
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
	//	ファイル存在確認方法変更
	if( 0 < wcslen( szJumpToFile ) &&
		IsFileExists(to_tchar(szJumpToFile), bFileOnly))
	//	-1 != _access( szJumpToFile, 0 ) )
	{
		//	Jan. 04, 2002 genta
		//	あまりに変なコーディングなので移動
		*pnPathLen = wcslen( szJumpToFile );
		return true;
	}else{
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
void AddLastYenFromDirectoryPath( TCHAR* pszFolder )
{
	if( 3 == _tcslen( pszFolder )
	 && pszFolder[1] == _T(':')
	 && pszFolder[2] == _T('\\')
	){
		/* ドライブ名:\ */
	}else{
		/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
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
	if( _T('A') <= path[0] && path[0] <= _T('Z') && path[1] == _T(':') && path[2] == _T('\\') ){
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
	
	TCHAR	szPath[_MAX_PATH];
	// sakura.ini のパスを取得
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
	if( _taccess(szInidir, 0) != -1 ){
		::lstrcpy( pDir, szInidir );
		return;
	}

	// EXE基準のフルパスが実在すればそのパスを返す
	if( CShareData::getInstance()->IsPrivateSettings() ){	// INIとEXEでパスが異なる場合
		GetExedir( szExedir, szFile );
		if( _taccess(szExedir, 0) != -1 ){
			::lstrcpy( pDir, szExedir );
			return;
		}
	}

	// どちらにも実在しなければINI基準のフルパスを返す
	::lstrcpy( pDir, szInidir );
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



/*!	ファイルの更新日時を取得

	@param[in] filename ファイルのパス
	@param[out] ftime 更新日時を返す場所
	@return true: 成功, false: FindFirstFile失敗

	@author genta by assitance with ryoji
	@date 2005.10.22 new

	@note 書き込み後にファイルを再オープンしてタイムスタンプを得ようとすると
	ファイルがまだロックされていることがあり，上書き禁止と誤認されることがある．
	FindFirstFileを使うことでファイルのロック状態に影響されずにタイムスタンプを
	取得できる．(ryoji)
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
	//	ファイルが見つからなかった
	return false;
}
