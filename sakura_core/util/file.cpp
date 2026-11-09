/*! @file */
/*
	Copyright (C) 2002, SUI
	Copyright (C) 2003, MIK
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "file.h"

#include "_main/CProcessFactory.h"

#include <Shlwapi.h>

#include <regex>
#include <string_view>

#include "charset/codechecker.h"
#include "util/file/FileHandle.hpp"
#include "util/module.h"
#include "util/os.h"
#include "util/window.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CFileNameManager.h"

//! ファイル名に使えない文字(ADSを使えるように':'は除外する)
static constexpr auto& g_InvalidChars = LTEXT(R"(*?"<>|)");

bool fexist(LPCWSTR pszPath)
{
	return _waccess(pszPath,0)!=-1;
}

bool fexist(const std::filesystem::path& path) noexcept
{
	std::error_code ec;
	return std::filesystem::exists(path, ec) && 0 == _waccess_s(path.c_str(), 0);
}

/*!
 * パスがファイル名に使えない文字を含んでいるかチェックする
 *
 * @param[in] szPath チェック対象のパス文字列
 * @retval true  パスはファイル名に使えない文字を含んでいる
 * retuval false パスはファイル名に使えない文字を含んでいない
 */
bool IsInvalidFilenameChars(
	sakura::wstring_view szPath
)
{
	// 引数からパスオブジェクトを作成する
	std::filesystem::path path{ szPath };

	// 文字列中のファイル名を抽出する
	if (!path.has_filename()) {
		return false;
	}
	
	// ファイル名を抽出する
	if (path.has_parent_path()) {
		// ファイル名のみのパスオブジェクトにする
		path = path.filename();
	}

	// パスオブジェクトから文字列を取り出す
	const auto& filename = path.native();

	// ファイル名に使えない文字が含まれる場合、trueを返す
	return sakura::wstring::npos != filename.find_first_of(g_InvalidChars);
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
	const WCHAR*	pLine,		//!< [in]  探査対象文字列
	size_t*			pnBgn,		//!< [out] 先頭offset。pLine + *pnBgnがファイル名先頭へのポインタ。
	size_t*			pnPathLen,	//!< [out] ファイル名の長さ
	bool			bFileOnly	//!< [in]  true: ファイルのみ対象 / false: ディレクトリも対象
)
{
	WCHAR	szJumpToFile[_MAX_PATH];
	wmemset( szJumpToFile, 0, int(std::size(szJumpToFile)) );

	size_t	nLineLen = wcslen( pLine );

	//先頭の空白を読み飛ばす
	size_t	i;
	for( i = 0; i < nLineLen; ++i ){
		WCHAR c = pLine[i];
		if( LCHAR(' ')!=c && LCHAR('\t')!=c && LCHAR('\"')!=c ){
			break;
		}
	}

	//	#include <ファイル名>の考慮
	//	#で始まるときは"または<まで読み飛ばす
	if( i < nLineLen && LCHAR('#') == pLine[i] ){
		for( ; i < nLineLen; ++i ){
			if( LCHAR('<')  == pLine[i] || LCHAR('\"') == pLine[i]){
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
	for( ; i <= nLineLen && cur_pos + 1 < int(std::size(szJumpToFile)); ++i ){
		//ファイル名終端を検知する
		if( WCODE::IsLineDelimiterExt(pLine[i]) || pLine[i] == LCHAR('\0') ){
			break;
		}

		//ファイル名終端を検知する
		if( ( i == nLineLen    ||
			  // 2002.01.08 YAZAKI タブ文字も。
			  // 2013.05.27 Moca 文字種追加
			  wcschr(LTEXT(" \t(\")'`[]{};#!@&%$"), pLine[i]) != nullptr
			) &&
			szJumpToFile[0] != LCHAR('\0')
		){
			//	ファイル存在確認
			if( IsFileExists(szJumpToFile, bFileOnly)){
				tmp_end = cur_pos;
			}
		}

		// May 29, 2004 genta C:\の:はファイル区切りと見なして欲しくない
		if( cur_pos > 1 && pLine[i] == LCHAR(':') ){   //@@@ 2003/1/15/ matsumo (for gcc)
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
	if( szJumpToFile[0] != LCHAR('\0') && IsFileExists(szJumpToFile, bFileOnly)){
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
bool IsLocalDrive( const WCHAR* pszDrive )
{
	WCHAR	szDriveType[_MAX_DRIVE+1];	// "A:\ "登録用
	long	lngRet;

	if( iswalpha(pszDrive[0]) ){
		auto_sprintf(szDriveType, LTEXT("%c:\\"), towupper(pszDrive[0]));
		lngRet = GetDriveType( szDriveType );
		if( lngRet == DRIVE_REMOVABLE || lngRet == DRIVE_CDROM || lngRet == DRIVE_REMOTE )
		{
			return false;
		}
	}
	else if (pszDrive[0] == LCHAR('\\') && pszDrive[1] == LCHAR('\\')) {
		// ネットワークパス	2010/5/27 Uchi
		return false;
	}
	return true;
}

/*! フルパスからファイル名を返す

	@date 2006.04.10 fon 新規作成
	@date 2006.09.14 genta ディレクトリがない場合に最初の1文字が切れないように
 */
const WCHAR* GetFileTitlePointer(const WCHAR* pszPath)
{
	sakura::wstring_view name(pszPath);
	if (const size_t sep = name.find_last_of(LCHAR('\\')); sep != sakura::wstring_view::npos) {
		if (sep + 1 == name.length()) {
			return pszPath + name.length();
		}
		return pszPath + sep + 1;
	}
	return pszPath;
}

/*! fnameが相対パスの場合は、実行ファイルのパスからの相対パスとして開く
	@author Moca
	@date 2003.06.23
	@date 2007.05.20 ryoji 関数名変更（旧：fopen_absexe）、汎用テキストマッピング化
*/
FILE* _wfopen_absexe(LPCWSTR fname, LPCWSTR mode)
{
	if( _IS_REL_PATH( fname ) ){
		WCHAR path[_MAX_PATH];
		GetExedir( path, fname );
		return _wfopen( path, mode );
	}
	return _wfopen( fname, mode );
}

/*! fnameが相対パスの場合は、INIファイルのパスからの相対パスとして開く
	@author ryoji
	@date 2007.05.19 新規作成（_wfopen_absexeベース）
*/
FILE* _wfopen_absini(LPCWSTR fname, LPCWSTR mode, BOOL bOrExedir/*=TRUE*/ )
{
	if( _IS_REL_PATH( fname ) ){
		WCHAR path[_MAX_PATH];
		if( bOrExedir )
			GetInidirOrExedir( path, fname );
		else
			GetInidir( path, fname );
		return _wfopen( path, mode );
	}
	return _wfopen( fname, mode );
}

/* フォルダーの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない */
void CutLastYenFromDirectoryPath( WCHAR* pszFolder )
{
	if( 3 == wcslen( pszFolder )
	 && pszFolder[1] == LCHAR(':')
	 && pszFolder[2] == LCHAR('\\')
	){
		/* ドライブ名:\ */
	}else{
		/* フォルダーの最後が半角かつ'\\'の場合は、取り除く */
		int	nFolderLen;
		int	nCharChars;
		nFolderLen = (int)wcslen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = int(&pszFolder[nFolderLen] - CNativeW::GetCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] ));
			if( 1 == nCharChars && LCHAR('\\') == pszFolder[nFolderLen - 1] ){
				pszFolder[nFolderLen - 1] = LCHAR('\0');
			}
		}
	}
	return;
}

namespace cxx {

void AddLastYenFromDirectoryPath(std::span<WCHAR> folder)
{
	const auto cchFolder = std::size(folder);

	auto pszFolder = std::data(folder);

	const auto nFolderLen = static_cast<int>(::wcsnlen(pszFolder, cchFolder));

	if (0 == nFolderLen) {
		return;
	}

	/* フォルダー文字列の末尾がフォルダー区切り文字になっていない場合は付加する */
	if (const auto lastChar = pszFolder[nFolderLen - 1];
		LCHAR('\\') != lastChar && LCHAR('/') != lastChar)
	{
		::wcscat_s(pszFolder, cchFolder, LTEXT("\\"));
	}

	return;
}

} // namespace cxx

//! パスらしき文字列の末尾に'\\'または'/'がなかったら'\\'を付加する
sakura::wstring AddLastYenPath(sakura::wstring_view path)
{
	sakura::wstring ret{ path };
	if (0 == ret.size()) {
		return ret;
	}
	if (auto c = ret.back(); !(c == LCHAR('\\') || c == LCHAR('/'))) {
		ret.append(LTEXT("\\"));
	}
	return ret;
}

/*!
	パスの区切りの / を \ に置き換える

	ファイルダイアログの「ファイル名」に / 区切りのパスを入力したときに使う(Issue #2727)。
	URL(:// を含むもの)は変えない。
*/
sakura::wstring ReplaceSlashToYen(sakura::wstring_view path)
{
	sakura::wstring ret{ path };
	if (ret.find(LTEXT("://")) != sakura::wstring::npos) {
		return ret;
	}
	std::ranges::replace(ret, LCHAR('/'), LCHAR('\\'));
	return ret;
}

/* ファイルのフルパスを、フォルダーとファイル名に分割 */
/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
void SplitPath_FolderAndFile( const WCHAR* pszFilePath, WCHAR* pszFolder, WCHAR* pszFile )
{
	WCHAR	szDrive[_MAX_DRIVE];
	WCHAR	szDir[_MAX_DIR];
	WCHAR	szFname[_MAX_FNAME];
	WCHAR	szExt[_MAX_EXT];
	int		nFolderLen;
	int		nCharChars;
	_wsplitpath_s( pszFilePath, szDrive, szDir, szFname, szExt );
	if( nullptr != pszFolder ){
		wcscpy( pszFolder, szDrive );
		wcscat( pszFolder, szDir );
		/* フォルダーの最後が半角かつ'\\'の場合は、取り除く */
		nFolderLen = (int)wcslen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = int(&pszFolder[nFolderLen] - CNativeW::GetCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] ));
			if( 1 == nCharChars && LCHAR('\\') == pszFolder[nFolderLen - 1] ){
				pszFolder[nFolderLen - 1] = LCHAR('\0');
			}
		}
	}
	if( nullptr != pszFile ){
		wcscpy( pszFile, szFname );
		wcscat( pszFile, szExt );
	}
	return;
}

/* フォルダー、ファイル名から、結合したパスを作成
 * [c:\work\test] + [aaa.txt] → [c:\work\test\aaa.txt]
 * フォルダー末尾に円記号があってもなくても良い。
 */
void Concat_FolderAndFile( const WCHAR* pszDir, const WCHAR* pszTitle, WCHAR* pszPath )
{
	WCHAR* out=pszPath;
	const WCHAR* in;

	//フォルダーをコピー
	for( in=pszDir ; *in != '\0'; ){
		*out++ = *in++;
	}
	//円記号を付加
	if( *(out-1) != '\\' ){ *out++ = '\\'; }
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
BOOL GetLongFileName( const WCHAR* pszFilePathSrc, WCHAR* pszFilePathDes )
{
	WCHAR* name;
	WCHAR szBuf[_MAX_PATH + 1];
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
		wcscpy( pszFilePathDes, szBuf );
	}
	return TRUE;
}

/* 拡張子を調べる */
BOOL CheckEXT( const WCHAR* pszPath, const WCHAR* pszExt )
{
	WCHAR	szExt[_MAX_EXT];
	WCHAR*	pszWork;
	_wsplitpath_s( pszPath, nullptr, 0, nullptr, 0, nullptr, 0, szExt, std::size(szExt) );
	pszWork = szExt;
	if( pszWork[0] == LCHAR('.') ){
		pszWork++;
	}
	if( 0 == _wcsicmp( pszExt, pszWork ) ){
		return TRUE;
	}else{
		return FALSE;
	}
}

/*! 相対パスか判定する
	@author Moca
	@date 2003.06.23
*/
bool _IS_REL_PATH(const WCHAR* path)
{
	bool ret = true;
	if( (((LCHAR('A') <= path[0] && path[0] <= LCHAR('Z')) || (LCHAR('a') <= path[0] && path[0] <= LCHAR('z')))
		&& path[1] == LCHAR(':') && path[2] == LCHAR('\\'))
		|| (path[0] == LCHAR('\\') && path[1] == LCHAR('\\'))
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
	const WCHAR* path	//!< [in] 深さを調べたいファイル/ディレクトリのフルパス
)
{
	int depth = 0;

	//	とりあえず\の数を数える
	sakura::wstring_view p(path);
	sakura::wstring_view::size_type pos = 0;
	do {
		pos = p.find_first_of(LCHAR('\\'), pos);
		if (pos == sakura::wstring_view::npos) break;
		++depth;
		++pos;

		//	フルパスには入っていないはずだが念のため
		//	.\はカレントディレクトリなので，深さに関係ない．
		while (pos + 2 <= p.length()
			&& p[pos + 0] == LCHAR('.')
			&& p[pos + 1] == LCHAR('\\')) {
			pos += 2;
		}
	} while (pos < p.length());
 
	//	補正
	//	ドライブ名はパスの深さに数えない
	if (std::regex_search(path, std::wregex(LTEXT(R"(^[A-Z]:\\)"), std::wregex::icase))) {
		//フルパス
		--depth; // C:\ の \ はルートの記号なので階層深さではない
	}
	else if( path[0] == LCHAR('\\') ){
		if( path[1] == LCHAR('\\') ){
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
	@brief exeファイルパスを取得する
 */
std::filesystem::path GetExeFileName()
{
	// sakura.exe のパスを取得して返却
	const auto path = cxx::GetModuleFileNameW(nullptr);
	if (const auto cchMaxPath = decltype(DLLSHAREDATA::m_szIniFile)::size();
		cchMaxPath <= path.length())
	{
		// EXEパスが長過ぎる場合、例外を投げる
		throw std::system_error(
			std::make_error_code(std::errc::filename_too_long),
			std::format("exe path is too long. (length: at least {}, allowed: {})", path.length(), cchMaxPath - 1)
		);
	}

	return path;
}

/*!
	@brief exeファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す．
	
	@author genta
	@date 2002.12.02 genta
	@date 2007.05.20 ryoji 関数名変更（旧：GetExecutableDir）、汎用テキストマッピング化
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
void GetExedir(
	LPWSTR	pDir,	//!< [out] EXEファイルのあるディレクトリを返す場所．予め_MAX_PATHのバッファを用意しておくこと．
	LPCWSTR	szFile	//!< [in]  ディレクトリ名に結合するファイル名．
)
{
	if( pDir == nullptr )
		return;

	sakura::wstring partialPath;
	if (szFile != nullptr) {
		partialPath = szFile;
	}
	if (partialPath.empty() || partialPath[0] != LCHAR('\\')) {
		partialPath.insert(partialPath.cbegin(), LCHAR('\\'));
	}

	// exeフォルダーのフルパス、またはexe基準のファイルパスを取得
	auto path = GetExeFileName().parent_path().concat(partialPath);
	::wcsncpy_s(pDir, decltype(DLLSHAREDATA::m_szIniFile)::BUFFER_COUNT, path.c_str(), _TRUNCATE);
}

/*!
	@brief iniファイルパスを取得する
 */
std::filesystem::path GetIniFileName()
{
	if (const auto pProcess = CProcess::getInstance(); pProcess != nullptr) {
		return pProcess->GetIniFileName();
	}
	return GetExeFileName().replace_extension(LTEXT(".ini"));
}

/*!
	@brief INIファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す．
	
	@author ryoji
	@date 2007.05.19 新規作成（GetExedirベース）
*/
void GetInidir(
	LPWSTR	pDir,				//!< [out] INIファイルのあるディレクトリを返す場所．予め_MAX_PATHのバッファを用意しておくこと．
	LPCWSTR szFile	/*=NULL*/	//!< [in] ディレクトリ名に結合するファイル名．
)
{
	if( pDir == nullptr )
		return;
	
	sakura::wstring partialPath;
	if (szFile != nullptr) {
		partialPath = szFile;
	}
	if (partialPath.empty() || partialPath[0] != LCHAR('\\')) {
		partialPath.insert(partialPath.cbegin(), LCHAR('\\'));
	}

	// 設定フォルダーのフルパス、またはini基準のファイルパスを取得
	auto path = GetIniFileName().parent_path().concat(partialPath);
	::wcsncpy_s(pDir, decltype(DLLSHAREDATA::m_szPrivateIniFile)::BUFFER_COUNT, path.c_str(), _TRUNCATE);
}

/*!
	@brief INIファイルまたはEXEファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す（INIを優先）．
	
	@author ryoji
	@date 2007.05.22 新規作成
*/
void GetInidirOrExedir(
	LPWSTR	pDir,								//!< [out] INIファイルまたはEXEファイルのあるディレクトリを返す場所．
												//         予め_MAX_PATHのバッファを用意しておくこと．
	LPCWSTR	szFile					/*=NULL*/,	//!< [in] ディレクトリ名に結合するファイル名．
	BOOL	bRetExedirIfFileEmpty	/*=FALSE*/	//!< [in] ファイル名の指定が空の場合はEXEファイルのフルパスを返す．
)
{
	WCHAR	szInidir[_MAX_PATH];
	WCHAR	szExedir[_MAX_PATH];

	// ファイル名の指定が空の場合はEXEファイルのフルパスを返す（オプション）
	if( bRetExedirIfFileEmpty && (szFile == nullptr || szFile[0] == LCHAR('\0')) ){
		GetExedir( pDir );
		return;
	}

	// INI基準のフルパスが実在すればそのパスを返す
	GetInidir( szInidir, szFile );
	if( fexist(szInidir) ){
		::lstrcpy( pDir, szInidir );
		return;
	}

	// EXE基準のフルパスが実在すればそのパスを返す
	if( GetExedir( szExedir, szFile ); fexist(szExedir) ){
		::wcsncpy_s( pDir, _MAX_PATH - 1, szExedir, _TRUNCATE );
		return;
	}

	// どちらにも実在しなければINI基準のフルパスを返す
	::lstrcpy( pDir, szInidir );
}

/*!
	@brief INIファイルまたはEXEファイルのあるディレクトリの相対パスを返す（INIを優先）．
	@param pszPath [in] 対象パス
	@date 2013.06.26 novice 新規作成
*/
LPCWSTR GetRelPath( LPCWSTR pszPath )
{
	WCHAR szPath[_MAX_PATH + 1];
	LPCWSTR pszFileName = pszPath;

	GetInidir( szPath, LTEXT("") );
	auto nLen = int(wcslen(szPath));
	if( 0 == wmemicmp( szPath, pszPath, nLen ) ){
		pszFileName = pszPath + nLen;
	}else{
		GetExedir( szPath, LTEXT("") );
		nLen = (int)wcslen( szPath );
		if( 0 == wmemicmp( szPath, pszPath, nLen ) ){
			pszFileName = pszPath + nLen;
		}
	}

	return pszFileName;
}

/*!
 * 新しいテンポラリファイルパスを生成する
 * Windows APIが生成したパスを「開いて閉じる」ため、呼ぶとファイルが生成される
 *
 * 生成されるパスの形式は以下の通り。
 * C:\Users\berryzplus\AppData\Local\Temp\tesC85A.tmp
 *
 * @param [in] prefix ファイル名の前に付ける3文字の接頭辞。
 * @param [in, opt] optTempDir 一時フォルダーのパス。指定しない場合はシステムの一時フォルダーを使う。
 */
// TODO: いつか廃止する
std::filesystem::path GetTempFilePath(
	sakura::wstring_view prefix,
	const std::optional<std::filesystem::path>& optTempDir
)
{
	// 一時フォルダーのパスを取得する
	std::filesystem::path tempDir;
	if (optTempDir.has_value()) {
		tempDir = optTempDir.value();
	} else {
		SFilePath szTempDir;
		::GetTempPathW(std::size(szTempDir), szTempDir);
		tempDir = std::filesystem::path{ szTempDir };
	}

	// パス生成に必要なバッファを確保する
	// （一時フォルダーのパス＋接頭辞(3文字)＋4桁の16進数＋拡張子＋NUL終端）
	sakura::wstring buf(std::size(tempDir.native()) + 3 + 4 + 4 + 1, LCHAR('\0'));

	// Windows API関数を呼び出す。
	// （オーバーフローしないので、エラーチェック省略）
	constexpr uint16_t uUnique = 0;
	::GetTempFileNameW(tempDir.c_str(), std::data(prefix), uUnique, std::data(buf));

	return buf.c_str();
}

//! パスに使えない文字が含まれていないかチェックする
// ファイル名、フォルダー名には「<>*|"?」が使えない
// しかし「\\?\C:\Program files\」の形式では?が3文字目にあるので除外
// ストリーム名とのセパレータにはfilename.ext:streamの形式でコロンが使われる
// コロンは除外文字に入っていない
bool IsValidPathAvailableChar(const std::filesystem::path& path) noexcept
{
	if (path.empty()) {
		// 空文字列セーフ
		return true;
	}
	constexpr auto& dos_device_path = LTEXT(R"(\\?\)");
	sakura::wstring_view pathStr{ path.native() };
	size_t pos = 0;
	if (pathStr.starts_with(dos_device_path)) {
		pos = std::size(dos_device_path) - 1;
	}
	// ファイル名に使えない文字が含まれない場合、trueを返す
	return pathStr.find_first_of(g_InvalidChars, pos) == sakura::wstring::npos;
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
bool IsFileExists(const WCHAR* path, bool bFileOnly)
{
	if (!IsValidPathAvailableChar(path)) {
		// ワイルドカード指定を除外
		return false;
	}
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
bool IsDirectory(LPCWSTR pszPath)
{
	if (!IsValidPathAvailableChar(pszPath)) {
		// ワイルドカード指定を除外
		return false;
	}
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile( pszPath, &fd );
	if(hFind!=INVALID_HANDLE_VALUE){
		::FindClose( hFind );
		return (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	return false;
}

/*!
 * @brief ファイルの更新日時を取得
 *
 * @param[in] filename ファイルのパス
 * @returns ファイルの更新日時
 */
FILETIME GetLastWriteTime(
	sakura::wstring_view	fileName	//!< [in]  ファイルのパス
)
{
	using namespace cxx;

	// dwDesiredAccess に 0 を指定する事で読み取りアクセスが拒否されてしまうような場合でも、
	// ファイル自体にはアクセスせずにメタデータの読み取りが可能
	// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew
	// If this parameter is zero, the application can query certain metadata such as file, directory, or device attributes without accessing that file or device, even if GENERIC_READ access would have been denied.
	const auto hFile = FileHandle::OpenExisting(
		fileName,
		0,
		0
	);

	FILETIME ftLastWrite{};
	hFile.GetFileTime(
		nullptr,
		nullptr,
		&ftLastWrite
	);

	return ftLastWrite;
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
	const char *pf = nullptr;
	while( *pt != '\0' ){	/* 文字列の終端まで調べる。 */
		if( ( *pt == ch1 ) || ( *pt == ch2 ) )	pf = pt;	/* pf = 検索文字の位置 */
		if( _IS_SJIS_1(*pt) )	pt++;	/* Shift_JIS の1文字目なら、次の1文字をスキップ */
		if( *pt != '\0' )		pt++;	/* 次の文字へ */
	}
	return	(char *)pf;
}
WCHAR* wcsrchr2( const WCHAR *pt , const WCHAR ch1 , const WCHAR ch2 ){
	const WCHAR *pf = nullptr;
	while( *pt != LCHAR('\0') ){	/* 文字列の終端まで調べる。 */
		if( ( *pt == ch1 ) || ( *pt == ch2 ) )	pf = pt;	/* pf = 検索文字の位置 */
		if( *pt != '\0' )		pt++;	/* 次の文字へ */
	}
	return	(WCHAR *)pf;
}

#define		GetExistPath_NO_DriveLetter	0	/* ドライブレターが無い */
#define		GetExistPath_IV_Drive		1	/* ドライブが無効 */
#define		GetExistPath_AV_Drive		2	/* ドライブが有効 */

void GetExistPathW( WCHAR *po , const WCHAR *pi )
{
	WCHAR	*pw,*ps;
	int		cnt;
	int		dl;		/* ドライブの状態 */

	/* pi の内容を
	/ ・ " を削除しつつ
	/ ・ / を \ に変換しつつ(Win32API では / も \ と同等に扱われるから)
	/ ・最大 ( _MAX_PATH-1 ) 文字まで
	/ po にコピーする。 */
	for( pw=po,cnt=0 ; ( *pi != LCHAR('\0') ) && ( cnt < _MAX_PATH-1 ) ; pi++ ){
		/* /," 共に Shift_JIS の漢字コード中には含まれないので Shift_JIS 判定は不要。 */
		if( *pi == LCHAR('\"') )	continue;		/*  " なら何もしない。次の文字へ */
		if( *pi == LCHAR('/') )	*pw++ = LCHAR('\\');	/*  / なら \ に変換してコピー    */
		else				*pw++ = *pi;	/* その他の文字はそのままコピー  */
		cnt++;	/* コピーした文字数 ++ */
	}
	*pw = LCHAR('\0');		/* 文字列終端 */

	dl = GetExistPath_NO_DriveLetter;	/*「ドライブレターが無い」にしておく*/
	if( *(po+1)==LCHAR(':') && WCODE::IsAZ(*po) ){	/* 先頭にドライブレターがある。そのドライブが有効かどうか判定する */
		WCHAR	drv[4] = LTEXT("_:\\");
		drv[0] = *po;
		if( _waccess(drv,0) == 0 )	dl = GetExistPath_AV_Drive;		/* 有効 */
		else						dl = GetExistPath_IV_Drive;		/* 無効 */
	}

	if( dl == GetExistPath_IV_Drive ){	/* ドライブ自体が無効 */
		/* フロッピーディスク中のファイルが指定されていて、
		　 そのドライブにフロッピーディスクが入っていない、とか */
		*po = LCHAR('\0');	/* 返値文字列 = "";(空文字列) */
		return;		/* これ以上何もしない */
	}

	/* ps = 検索開始位置 */
	ps = po;	/* ↓文字列の先頭が \\ なら、\ 検索処理の対象から外す */
	if( ( *po == LCHAR('\\') )&&( *(po+1) == LCHAR('\\') ) )	ps +=2;

	if( *ps == LCHAR('\0') ){	/* 検索対象が空文字列なら */
		*po = LCHAR('\0');		/* 返値文字列 = "";(空文字列) */
		return;			/*これ以上何もしない */
	}

	for(;;){
		if( _waccess(po,0) == 0 )	break;	/* 有効なパス文字列が見つかった */
		/* ↓文字列最後尾の \ または ' ' を探し出し、そこを文字列終端にする。*/

		pw = wcsrchr2(ps,'\\',' ');	/* 最末尾の \ か ' ' を探す。 */
		if ( pw == nullptr ){	/* 文字列中に '\\' も ' ' も無かった */
			/* 例えば "C:testdir" という文字列が来た時に、"C:testdir" が実在
			　 しなくとも C:ドライブが有効なら "C:" という文字列だけでも返し
			　 たい。以下↓は、そのための処理。 */
			if( dl == GetExistPath_AV_Drive ){
				/* 先頭に有効なドライブのドライブレターがある。 */
				*(po+2) = LCHAR('\0');		/* ドライブレター部の文字列のみ返す */
			}
			else{	/* 有効なパス部分が全く見つからなかった */
				*po = LCHAR('\0');	/* 返値文字列 = "";(空文字列) */
			}
			break;		/* ループを抜ける */
		}
		/* ↓ルートディレクトリを引っかけるための処理 */
		if( ( *pw == LCHAR('\\') )&&( ps < pw )&&( *(pw-1) == LCHAR(':') ) ){	/* C:\ とかの \ っぽい */
			* (pw+1) = LCHAR('\0');		/* \ の後ろの位置を文字列の終端にする。 */
			if( _waccess(po,0) == 0 )	break;	/* 有効なパス文字列が見つかった */
		}
		*pw = LCHAR('\0');		/* \ か ' ' の位置を文字列の終端にする。 */
		/* ↓末尾がスペースなら、スペースを全て削除する */
		while( ( pw != ps ) && ( *(pw-1) == LCHAR(' ') ) )	* --pw = LCHAR('\0');
	}

	return;
}

/* 与えられたコマンドライン文字列の先頭部分から実在するファイル・ディレクトリ
　 のパス文字列を抽出し、そのパスを分解して drv dir fnm ext に書き込む。
　 先頭部分に有効なパス名が存在しない場合、全てに空文字列が返る。 */
void my_splitpath_w (
	const WCHAR *comln,
	WCHAR *drv,
	WCHAR *dir,
	WCHAR *fnm,
	WCHAR *ext
)
{
	WCHAR	ppp[_MAX_PATH];		/* パス格納（作業用） */
	WCHAR	*pd;
	WCHAR	*pf;
	WCHAR	*pe;
	WCHAR	ch;
	DWORD	attr;
	int		a_dir;

	if( drv != nullptr )	*drv = LCHAR('\0');
	if( dir != nullptr )	*dir = LCHAR('\0');
	if( fnm != nullptr )	*fnm = LCHAR('\0');
	if( ext != nullptr )	*ext = LCHAR('\0');
	if( *comln == LCHAR('\0') )	return;

	/* コマンドライン先頭部分の実在するパス名を ppp に書き出す。 */
	GetExistPathW( ppp , comln );

	if( *ppp != LCHAR('\0') ) {	/* ファイル・ディレクトリが存在する場合 */
		/* 先頭文字がドライブレターかどうか判定し、
		　 pd = ディレクトリ名の先頭位置に設定する。 */
		pd = ppp;
		if(*(pd+1)==LCHAR(':') && WCODE::IsAZ(*pd)){	/* 先頭にドライブレターがある。 */
			pd += 2;	/* pd = ドライブレター部の後ろ         */
		}				/*      ( = ディレクトリ名の先頭位置 ) */
		/* ここまでで、pd = ディレクトリ名の先頭位置 */

		attr =  GetFileAttributesW(ppp);
		a_dir = ( attr & FILE_ATTRIBUTE_DIRECTORY ) ?  1 : 0;

		if( ! a_dir ){	/* 見つけた物がファイルだった場合。 */
			pf = wcsrchr(ppp,LCHAR('\\'));	/* 最末尾の \ を探す。 */
			if(pf != nullptr)	pf++;		/* 見つかった→  pf=\の次の文字の位置*/
			else			pf = pd;	/* 見つからない→pf=パス名の先頭位置 */
			/* ここまでで pf = ファイル名の先頭位置 */
			pe = wcsrchr(pf,LCHAR('.'));		/* 最末尾の '.' を探す。 */
			if( pe != nullptr ){					/* 見つかった(pe = L'.'の位置)*/
				if( ext != nullptr ){	/* 拡張子を返値として書き込む。 */
					wcsncpy(ext,pe,_MAX_EXT-1);
					ext[_MAX_EXT -1] = LCHAR('\0');
				}
				*pe = LCHAR('\0');	/* 区切り位置を文字列終端にする。pe = 拡張子名の先頭位置。 */
			}
			if( fnm != nullptr ){	/* ファイル名を返値として書き込む。 */
				wcsncpy(fnm,pf,_MAX_FNAME-1);
				fnm[_MAX_FNAME -1] = LCHAR('\0');
			}
			*pf = LCHAR('\0');	/* ファイル名の先頭位置を文字列終端にする。 */
		}
		/* ここまでで文字列 ppp はドライブレター＋ディレクトリ名のみになっている */
		if( dir != nullptr ){
			/* ディレクトリ名の最後の文字が \ ではない場合、\ にする。 */

			/* ↓最後の文字を ch に得る。(ディレクトリ文字列が空の場合 ch=L'\\' となる) */
			for( ch = LCHAR('\\') , pf = pd ; *pf != LCHAR('\0') ; pf++ ){
				ch = *pf;
			}
			/* 文字列が空でなく、かつ、最後の文字が \ でなかったならば \ を追加。 */
			if( ( ch != LCHAR('\\') ) && ( wcslen(ppp) < _MAX_PATH -1 ) ){
				*pf++ = LCHAR('\\');	*pf = LCHAR('\0');
			}

			/* ディレクトリ名を返値として書き込む。 */
			wcsncpy(dir,pd,_MAX_DIR -1);
			dir[_MAX_DIR -1] = LCHAR('\0');
		}
		*pd = LCHAR('\0');		/* ディレクトリ名の先頭位置を文字列終端にする。 */
		if( drv != nullptr ){	/* ドライブレターを返値として書き込む。 */
			wcsncpy(drv,ppp,_MAX_DRIVE -1);
			drv[_MAX_DRIVE -1] = LCHAR('\0');
		}
	}
	return;
}

//
//
//
//
//
// -----------------------------------------------------------------------------
int FileMatchScore( const WCHAR *file1, const WCHAR *file2 );

// フルパスからファイル名と拡張子（ファイル名の.以降）を分離する
// @date 2014/06/15 moca_skr フォルダー名に.が含まれた場合、フォルダーが分離されたのを修正した対応で新規作成
static void FileNameSepExt(const std::filesystem::path& path, sakura::wstring& outFile, sakura::wstring& outExt)
{
	if (path.has_filename()) {
		if (path.has_extension()) {
			outExt = path.extension();
			outFile = path.filename();
			outFile = outFile.substr(0, outFile.length() - outExt.length());
		} else {
			outExt.clear();
			outFile = path.filename();
		}
	} else {
		outFile.clear();
		outExt.clear();
	}
}

int FileMatchScoreSepExt( sakura::wstring_view file1, sakura::wstring_view file2 )
{
	sakura::wstring szFile1;
	sakura::wstring szFile2;
	sakura::wstring szFileExt1;
	sakura::wstring szFileExt2;
	FileNameSepExt(file1, szFile1, szFileExt1);
	FileNameSepExt(file2, szFile2, szFileExt2);
	int score = FileMatchScore(szFile1.c_str(), szFile2.c_str());
	score += FileMatchScore(szFileExt1.c_str(), szFileExt2.c_str());
	return score;
}

/*!	2つのファイル名の最長一致部分の長さを返す
*/
int FileMatchScore( const WCHAR *file1, const WCHAR *file2 )
{
	int score = 0;
	auto len1 = int(wcslen(file1));
	auto len2 = int(wcslen(file2));
	if( len1 < len2 ){
		const WCHAR * tmp = file1;
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
				int chars1 = (Int)CNativeW::GetSizeOfChar(file1, len1, pos1);
				int chars2 = (Int)CNativeW::GetSizeOfChar(file2, len2, m);
				if( chars1 == chars2 ){
					if( chars1 == 1 ){
						if( skr_towlower(file1[pos1]) == skr_towlower(file2[m]) ){
							tmpScore += chars1;
						}else{
							break;
						}
					}else{
						if( 0 == wmemicmp(&file1[pos1], &file2[m], chars1) ){
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
			k += t_max(1, (int)(Int)CNativeW::GetSizeOfChar(file2, len2, k));
		}
		i += t_max(1, (int)(Int)CNativeW::GetSizeOfChar(file1, len1, i));
	}
	return score;
}

/*! 指定幅までに文字列を省略
	@date 2014.06.12 新規作成 Moca
*/
void GetStrTrancateWidth( WCHAR* dest, int nSize, const WCHAR* path, HDC hDC, int nPxWidth )
{
	// できるだけ左側から表示
	// \\server\dir...
	const auto nPathLen = int(wcslen(path));
	CTextWidthCalc calc(hDC);
	if( calc.GetTextWidth(path) <= nPxWidth ){
		wcsncpy_s(dest, nSize, path, _TRUNCATE);
		return;
	}
	sakura::wstring strTemp;
	sakura::wstring strTempOld;
	int nPos = 0;
	while( path[nPos] != LCHAR('\0') ){
		strTemp.assign(path, nPos);
		sakura::wstring strTemp2 = strTemp;
		strTemp2 += LTEXT("...");
		if( nPxWidth < calc.GetTextWidth(strTemp2.c_str()) ){
			// 入りきらなかったので1文字前までをコピー
			wcsncpy_s(dest, t_max(0, nSize - 3), strTempOld.c_str(), _TRUNCATE);
			wcscat_s(dest, nSize, LTEXT("..."));
			return;
		}
		strTempOld = strTemp;
		nPos += t_max(1, (int)(Int)CNativeW::GetSizeOfChar(path, nPathLen, nPos));
	}
	// 全部表示(ここには来ないはず)
	wcsncpy_s(dest, nSize, path, _TRUNCATE);
}

/*! パスの省略表示
	in  C:\sub1\sub2\sub3\file.ext
	out C:\...\sub3\file.ext
	@date 2014.06.12 新規作成 Moca
*/
void GetShortViewPath( WCHAR* dest, int nSize, const WCHAR* path, HDC hDC, int nPxWidth, bool bFitMode )
{
	int nLeft = 0; // 左側固定表示部分
	int nSkipLevel = 1;
	const auto nPathLen = int(wcslen(path));
	CTextWidthCalc calc(hDC);
	if( calc.GetTextWidth(path) <= nPxWidth ){
		// 全部表示可能
		wcsncpy_s(dest, nSize, path, _TRUNCATE);
		return;
	}
	if( path[0] == LCHAR('\\') && path[1] == LCHAR('\\') ){
		if( path[2] == LCHAR('?') && path[4] == LCHAR('\\') ){
			// [\\?\A:\]
			nLeft = 4;
		}else{
			nSkipLevel = 2; // [\\server\dir\] の2階層飛ばす
			nLeft = 2;
		}
	}else{
		// http://server/ とか ftp://server/ とかを保持
		int nTop = 0;
		while( path[nTop] != LCHAR('\0') && path[nTop] != LCHAR('/') ){
			nTop += t_max(1, (int)(Int)CNativeW::GetSizeOfChar(path, nPathLen, nTop));
		}
		if( 0 < nTop && path[nTop - 1] == ':' ){
			// 「ほにゃらら:/」だった /が続いてる間飛ばす
			while( path[nTop] == LCHAR('/') ){
				nTop += t_max(1, (int)(Int)CNativeW::GetSizeOfChar(path, nPathLen, nTop));
			}
			nLeft = nTop;
		}
	}
	for( int i = 0; i < nSkipLevel; i++ ){
		while( path[nLeft] != LCHAR('\0') && path[nLeft] != LCHAR('\\') && path[nLeft] != LCHAR('/') ){
			nLeft += t_max(1, (int)(Int)CNativeW::GetSizeOfChar(path, nPathLen, nLeft));
		}
		if( path[nLeft] != LCHAR('\0') ){
			if( i + 1 < nSkipLevel ){
				nLeft++;
			}
		}else{
			if( bFitMode ){
				GetStrTrancateWidth(dest, nSize, path, hDC, nPxWidth);
				return;
			}
			// ここで終端なら全部表示
			wcsncpy_s(dest, nSize, path, _TRUNCATE);
			return;
		}
	}
	int nRight = nLeft; // 右側の表示開始位置(nRightは\を指している)
	while( path[nRight] != LCHAR('\0') ){
		int nNext = nRight;
		nNext++;
		while( path[nNext] != LCHAR('\0') && path[nNext] != LCHAR('\\') && path[nNext] != LCHAR('/') ){
			nNext += t_max(1, (int)(Int)CNativeW::GetSizeOfChar(path, nPathLen, nNext));
		}
		if( path[nNext] != LCHAR('\0') ){
			// サブフォルダー省略
			// C:\...\dir\file.ext
			sakura::wstring strTemp(path, nLeft + 1);
			if( nLeft + 1 < nRight ){
				strTemp += LTEXT("...");
			}
			strTemp += &path[nRight];
			if( calc.GetTextWidth(strTemp.c_str()) <= nPxWidth ){
				wcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
				return;
			}
			// C:\...\dir\   フォルダーパスだった。最後のフォルダーを表示
			if( path[nNext+1] == LCHAR('\0') ){
				if( bFitMode ){
					GetStrTrancateWidth(dest, nSize, strTemp.c_str(), hDC, nPxWidth);
					return;
				}
				wcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
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
	sakura::wstring strTemp(path, nLeftLen);
	if( nLeft != nRight ){
		strTemp += LTEXT("...");
	}
	strTemp += &path[nRight];
	if( bFitMode ){
		if( calc.GetTextWidth(strTemp.c_str()) <= nPxWidth ){
			wcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
			return;
		}
		// ファイル名(か左側固定部)が長すぎてはいらない
		int nExtPos = -1;
		{
			// 拡張子の.を探す
			int nExt = nRight;
			while( path[nExt] != LCHAR('\0') ){
				if( path[nExt] == LCHAR('.') ){
					nExtPos = nExt;
				}
				nExt += t_max(1, (int)(Int)CNativeW::GetSizeOfChar(path, nPathLen, nExt));
			}
		}
		if( nExtPos != -1 ){
			sakura::wstring strLeftFile(path, nLeftLen); // [C:\]  
			if( nLeft != nRight ){
				strLeftFile += LTEXT("..."); // C:\...
			}
			int nExtWidth = calc.GetTextWidth(&path[nExtPos]);
			int nLeftWidth = calc.GetTextWidth(strLeftFile.c_str());
			int nFileNameWidth = nPxWidth - nLeftWidth - nExtWidth;
			if( 0 < nFileNameWidth ){
				// 拡張子は省略しない(ファイルタイトルを省略)
				sakura::wstring strFile(&path[nRight], nExtPos - nRight); // \longfilename
				strLeftFile += strFile; // C:\...\longfilename
				int nExtLen = nPathLen - nExtPos;
				GetStrTrancateWidth(dest, t_max(0, nSize - nExtLen), strLeftFile.c_str(), hDC, nPxWidth - nExtWidth);
				wcscat_s(dest, nSize, &path[nExtPos+1]); // 拡張子連結 C:\...\longf...ext
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
	wcsncpy_s(dest, nSize, strTemp.c_str(), _TRUNCATE);
}
