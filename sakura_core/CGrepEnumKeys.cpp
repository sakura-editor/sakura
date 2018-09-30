/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CGrepEnumKeys.h"


// ファイルフィルタのフィルタタイプを表す定数
enum KeyFilterType {
	FILTER_SEARCH,
	FILTER_EXCEPT_FILE,
	FILTER_EXCEPT_FOLDER,
};

//retval 1 * \file.exe などのフォルダ部分でのワイルドカードはエラー
class Error_PathFilterIncludesAmbiguousFolder : public std::runtime_error
{
	static constexpr char message[] = "A path filter can't include '*' in folder-part.";

public:
	Error_PathFilterIncludesAmbiguousFolder(void)
		: runtime_error(message) {}
};

//return 2; // 絶対パス指定は不可
class Error_PathFilterIncludesAbsPath : public std::runtime_error
{
	static constexpr char message[] = "A path filter can't include an absolute path.";

public:
	Error_PathFilterIncludesAbsPath(void)
		: runtime_error(message) {}
};

// フィルタの追加処理を種類別に切り替えるための基底クラス
class KeyFilter {
protected:
	VGrepEnumKeys& _keys;
	VGrepEnumKeys& _absKeys;

public:
	KeyFilter(VGrepEnumKeys& keys, VGrepEnumKeys& absKeys) noexcept
		: _keys(keys)
		, _absKeys(absKeys)
	{
	}

public:
	virtual void addFilter(_In_z_ LPCTSTR token) = 0;

	virtual void GetFilters(SFilePathLong& szDst, _In_opt_ TCHAR chDelimiter = _T(';')) const noexcept = 0;
	virtual void GetFilterKeys(SFilePathLong& szDst, _In_opt_ TCHAR chDelimiter = _T(';')) const noexcept = 0;

	void ClearFilters() noexcept {
		ClearEnumKeys(_keys);
		ClearEnumKeys(_absKeys);
	}

protected:
	static void ClearEnumKeys(VGrepEnumKeys& keys) {
		for (int i = 0; i < (int)keys.size(); i++) {
			delete[] keys[i];
		}
		keys.clear();
	}

protected:
	void push_back_unique( VGrepEnumKeys& keys, LPCTSTR addKey ){
		if( ! IsExist( keys, addKey) ){
			TCHAR* newKey = new TCHAR[ _tcslen( addKey ) + 1 ];
			_tcscpy( newKey, addKey );
			keys.push_back( newKey );
		}
	}

	BOOL IsExist( VGrepEnumKeys& keys, LPCTSTR addKey ){
		for( int i = 0; i < (int)keys.size(); i++ ){
			if( _tcscmp( keys[ i ], addKey ) == 0 ){
				return TRUE;
			}
		}
		return FALSE;
	}

	void preAddFilter(_In_z_ LPCTSTR key) {

#pragma region ReplaceQuoteInFilter

		// ここの処理は実質的に置換処理
		// javascriptで書くとこんな処理
		//   token = token.replace(/"/,'');
		LPTSTR &token = const_cast<LPTSTR &>(key);

		// "を取り除いて左に詰める
		TCHAR* p;
		TCHAR* q;
		p = q = token;
		while( *p ){
			if( *p != _T('"') ){
				if( p != q ){
					*q = *p;
				}
				q++;
			}
			p++;
		}
		//*q = _T('\0');

#pragma endregion ReplaceQuoteInFilter

		// 
		bool wildcard = false;
		for (int i = 0; key[i]; i++) {
			if (!wildcard && (key[i] == _T('*') || key[i] == _T('?'))) {
				wildcard = true;
			}
			else if (wildcard && (key[i] == _T('\\') || key[i] == _T('/'))) {
				// *\file.exe などのフォルダ部分でのワイルドカードはエラー
				throw Error_PathFilterIncludesAmbiguousFolder();
			}
		}
	}
};

// フィルタの追加処理を実装するクラス
template<KeyFilterType _Type, TCHAR _PrefixChar = _T('\0')>
class KeyFilterImpl : public KeyFilter
{
public:
	static const TCHAR PREFIX_CHAR = _PrefixChar;
	static const TCHAR PREFIX_STR[];
	KeyFilterImpl(VGrepEnumKeys& keys, VGrepEnumKeys& absKeys) noexcept
		: KeyFilter(keys, absKeys)
	{
	}

public:
	void addFilter(_In_z_ LPCTSTR token) override {
		//if (keyType == FILTER_EXCEPT_FILE || FILTER_EXCEPT_FOLDER) {
		token++;

		preAddFilter(token);

		bool bRelPath = _IS_REL_PATH(token);
		if (bRelPath) {
			push_back_unique(_keys, token);
		}
		else {
			push_back_unique(_absKeys, token);
		}
	};

	void GetFilters(SFilePathLong& szDst, _In_opt_ TCHAR chDelimiter = _T(';')) const noexcept override {
		static TCHAR szDelimiter[] = { chDelimiter, _T('\0') };
		szDst[0] = _T('\0');
		for (auto it = _keys.cbegin(); it != _keys.cend(); ++it) {
			//_tcscat_s(szDst, szDst.BUFFER_COUNT, PREFIX_STR);
			_tcscat_s(szDst, szDst.BUFFER_COUNT, *it);
			_tcscat_s(szDst, szDst.BUFFER_COUNT, szDelimiter);
		}
		for (auto it = _absKeys.cbegin(); it != _absKeys.cend(); ++it) {
			//_tcscat_s(szDst, szDst.BUFFER_COUNT, PREFIX_STR);
			_tcscat_s(szDst, szDst.BUFFER_COUNT, *it);
			_tcscat_s(szDst, szDst.BUFFER_COUNT, szDelimiter);
		}
	}
	void GetFilterKeys(SFilePathLong& szDst, _In_opt_ TCHAR chDelimiter = _T(';')) const noexcept override {
		static TCHAR szDelimiter[] = { chDelimiter, _T('\0') };
		szDst[0] = _T('\0');
		for (auto it = _keys.cbegin(); it != _keys.cend(); ++it) {
			_tcscat_s(szDst, szDst.BUFFER_COUNT, PREFIX_STR);
			_tcscat_s(szDst, szDst.BUFFER_COUNT, *it);
			_tcscat_s(szDst, szDst.BUFFER_COUNT, szDelimiter);
		}
		for (auto it = _absKeys.cbegin(); it != _absKeys.cend(); ++it) {
			_tcscat_s(szDst, szDst.BUFFER_COUNT, PREFIX_STR);
			_tcscat_s(szDst, szDst.BUFFER_COUNT, *it);
			_tcscat_s(szDst, szDst.BUFFER_COUNT, szDelimiter);
		}
	}
};

// テンプレートクラスの static 変数を実体化しておく
template<KeyFilterType _Type, TCHAR _PrefixChar>
const TCHAR KeyFilterImpl<_Type, _PrefixChar>::PREFIX_STR[] = { _PrefixChar ,_T('\0') };

// フィルタの追加処理を実装するクラス:FILTER_SEARCH向けの特殊化
template<>
class KeyFilterImpl<FILTER_SEARCH> : public KeyFilter
{
public:
	KeyFilterImpl(VGrepEnumKeys& keys, VGrepEnumKeys& absKeys) noexcept
		: KeyFilter(keys, absKeys)
	{
	}

public:
	void addFilter(_In_z_ LPCTSTR token) override {
		preAddFilter(token);

		bool bRelPath = _IS_REL_PATH(token);
		if (bRelPath) {
			push_back_unique(_keys, token);
		}
		else {
			//return 2; // 絶対パス指定は不可
			throw Error_PathFilterIncludesAbsPath();
		}
	};
	void GetFilters(SFilePathLong& szFile, _In_opt_ TCHAR chDelimiter = _T(';')) const noexcept override {
		static TCHAR szDelimiter[] = { chDelimiter, _T('\0') };
		szFile[0] = _T('\0');
		for (auto it = _keys.cbegin(); it != _keys.cend(); ++it) {
			_tcscat_s(szFile, szFile.BUFFER_COUNT, *it);
			_tcscat_s(szFile, szFile.BUFFER_COUNT, szDelimiter);
		}
	}
	void GetFilterKeys(SFilePathLong& szFile, _In_opt_ TCHAR chDelimiter = _T(';')) const noexcept override {
		static TCHAR szDelimiter[] = { chDelimiter, _T('\0') };
		szFile[0] = _T('\0');
		for (auto it = _keys.cbegin(); it != _keys.cend(); ++it) {
			_tcscat_s(szFile, szFile.BUFFER_COUNT, *it);
			_tcscat_s(szFile, szFile.BUFFER_COUNT, szDelimiter);
		}
	}
};

// テンプレートのインスタンス化
typedef KeyFilterImpl<FILTER_SEARCH> SearchKeyFilter;
typedef KeyFilterImpl<FILTER_EXCEPT_FILE, _T('!')> FileExcludeKeyFilter;
typedef KeyFilterImpl<FILTER_EXCEPT_FOLDER, _T('#')> FolderExcludeKeyFilter;


CGrepEnumKeys::~CGrepEnumKeys() noexcept
{
	ClearItems();
}

void CGrepEnumKeys::ClearItems() noexcept
{
	// フィルタ操作クラスのインスタンスを生成
	SearchKeyFilter filterSearchFile(m_vecSearchFileKeys, VGrepEnumKeys());
	SearchKeyFilter filterSearchFolder(m_vecSearchFolderKeys, VGrepEnumKeys());
	FileExcludeKeyFilter filterExcludeFile(m_vecExceptFileKeys, m_vecExceptAbsFileKeys);
	FolderExcludeKeyFilter filterExcludeFolder(m_vecExceptFolderKeys, m_vecExceptAbsFolderKeys);

	// フィルタをすべてクリアする
	filterSearchFile.ClearFilters();
	filterSearchFolder.ClearFilters();
	filterExcludeFile.ClearFilters();
	filterExcludeFolder.ClearFilters();
}

/*!
 * Grepダイアログのファイルパターンを解析して分解する
 *
 * @param lpKeys ファイルパターン
 * @retval -1 不明なエラー(std::bad_alloc)
 * @retval 1 *\file.exe などのフォルダ部分でのワイルドカードはエラー
 * @retval 2 絶対パス指定は不可
 */
int CGrepEnumKeys::SetFileKeys(_In_z_ LPCTSTR lpKeys) noexcept{
	// フィルタ操作クラスのインスタンスを生成
	SearchKeyFilter filterSearchFile(m_vecSearchFileKeys, VGrepEnumKeys());
	SearchKeyFilter filterSearchFolder(m_vecSearchFolderKeys, VGrepEnumKeys());
	FileExcludeKeyFilter filterExcludeFile(m_vecExceptFileKeys, m_vecExceptAbsFileKeys);
	FolderExcludeKeyFilter filterExcludeFolder(m_vecExceptFolderKeys, m_vecExceptAbsFolderKeys);

	// 指定したタイプのフィルタ操作クラスを取得するローカル関数
	auto GetKeyFilter = [&](_In_z_ LPCTSTR pszFilter) -> KeyFilter* {
		switch (pszFilter[0]) {
		default: return &filterSearchFile;
		case FileExcludeKeyFilter::PREFIX_CHAR: return &filterExcludeFile;
		case FolderExcludeKeyFilter::PREFIX_CHAR: return &filterExcludeFolder;
		}
	};

	try
	{
		// lpKeysを解析するためのバッファを作る
		std::tstring wildCard(lpKeys ? lpKeys : _T(""));

		// 前回の解析結果をクリアする
		ClearItems();

		// my_strtokに渡す文字列ポインタを判断する
		TCHAR szDummy[] = _T(""); //ダミーバッファ(非constでなければならない)
		TCHAR*	pWildCard = wildCard.empty() ? szDummy : &*wildCard.begin();

		// lpKeysを区切り文字でトークン分割し、トークンがなくなるまで繰り返す
		int nPos = 0;
		TCHAR*	token;
		while (token = my_strtok(pWildCard, wildCard.length(), &nPos, WILDCARD_DELIMITER)) {
			// フィルタ操作クラスを取得する
			auto pKeyFilter = GetKeyFilter(token);
			assert(pKeyFilter);

			// フィルタにトークンを追加する
			pKeyFilter->addFilter(token);
		}

		if (m_vecSearchFileKeys.empty()) {
			filterSearchFile.addFilter(WILDCARD_ANY);
		}
		if (m_vecSearchFolderKeys.empty()) {
			filterSearchFolder.addFilter(WILDCARD_ANY);
		}
	}
	catch (const std::bad_alloc&) {
		return -1; //不明なエラー
	}
	catch (const Error_PathFilterIncludesAmbiguousFolder&) {
		return 1; // *\file.exe などのフォルダ部分でのワイルドカードはエラー
	}
	catch (const Error_PathFilterIncludesAbsPath&) {
		return 2; // 絶対パス指定は不可
	}

	return 0;
}

// 除外ファイルを追加
void CGrepEnumKeys::AddExcludeFiles(
	_Inout_ SFilePathLong& szFile,
	_In_z_ LPCTSTR pszExcludeFile,
	_In_opt_ TCHAR chDelimiter
)
{
	TCHAR szDelimiter[] = { chDelimiter, _T('\0') };

	// おかしな引数は処理しない
	if (pszExcludeFile == nullptr || *pszExcludeFile == _T('\0')) return;

	// 解析するためのバッファを作る
	std::tstring wildCard(pszExcludeFile);

	// my_strtokに渡す文字列ポインタを判断する
	TCHAR*	pWildCard = &*wildCard.begin();

	// 区切り文字でトークン分割し、トークンがなくなるまで繰り返す
	int nPos = 0;
	TCHAR*	token;
	while (token = my_strtok(pWildCard, wildCard.length(), &nPos, WILDCARD_DELIMITER)) {
		if (*token == '\0') continue; //空文字はskip
		_tcscat_s(szFile, szFile.BUFFER_COUNT, FileExcludeKeyFilter::PREFIX_STR);
		_tcscat_s(szFile, szFile.BUFFER_COUNT, token);
		_tcscat_s(szFile, szFile.BUFFER_COUNT, szDelimiter);
	}
}

// 除外フォルダを追加
void CGrepEnumKeys::AddExcludeFolders(
	_Inout_ SFilePathLong& szFile,
	_In_z_ LPCTSTR pszExcludeFolder,
	_In_opt_ TCHAR chDelimiter
)
{
	TCHAR szDelimiter[] = { chDelimiter, _T('\0') };

	// おかしな引数は処理しない
	if (pszExcludeFolder == nullptr || *pszExcludeFolder == _T('\0')) return;

	// 解析するためのバッファを作る
	std::tstring wildCard(pszExcludeFolder);

	// my_strtokに渡す文字列ポインタ
	TCHAR*	pWildCard = &*wildCard.begin();

	// 区切り文字でトークン分割し、トークンがなくなるまで繰り返す
	int nPos = 0;
	TCHAR*	token;
	while (token = my_strtok(pWildCard, wildCard.length(), &nPos, WILDCARD_DELIMITER)) {
		if (*token == '\0') continue; //空文字はskip
		_tcscat_s(szFile, szFile.BUFFER_COUNT, FolderExcludeKeyFilter::PREFIX_STR);
		_tcscat_s(szFile, szFile.BUFFER_COUNT, token);
		_tcscat_s(szFile, szFile.BUFFER_COUNT, szDelimiter);
	}
}


void CGrepEnumKeys::GetSearchFile(SFilePathLong& szFile)
{
	SearchKeyFilter filterSearchFile(m_vecSearchFileKeys, VGrepEnumKeys());
	filterSearchFile.GetFilters(szFile);
	if (szFile[0] != _T('\0')) szFile[::_tcslen(szFile) - 1] = _T('\0');
}

void CGrepEnumKeys::GetExcludeFile(SFilePathLong& szExcludeFile)
{
	FileExcludeKeyFilter filterExcludeFile(m_vecExceptFileKeys, m_vecExceptAbsFileKeys);
	filterExcludeFile.GetFilters(szExcludeFile);
	if (szExcludeFile[0] != _T('\0')) szExcludeFile[::_tcslen(szExcludeFile) - 1] = _T('\0');
}

void CGrepEnumKeys::GetExcludeFolder(SFilePathLong& szExcludeFolder)
{
	FolderExcludeKeyFilter filterExcludeFolder(m_vecExceptFolderKeys, m_vecExceptAbsFolderKeys);
	filterExcludeFolder.GetFilters(szExcludeFolder);
	if (szExcludeFolder[0] != _T('\0')) szExcludeFolder[::_tcslen(szExcludeFolder) - 1] = _T('\0');
}


/*!
 * 共有メモリに保存するファイルパターン文字列を組み立てる
 *
 * @param chDelimiter 区切り文字
 * @retval CShareDataのファイルパターン履歴に入れる文字列
 */
void CGrepEnumKeys::GetFileKeys(SFilePathLong& szFile, _In_opt_ TCHAR chDelimiter)
{
	// フィルタ操作クラスのインスタンスを生成
	SearchKeyFilter filterSearchFile(m_vecSearchFileKeys, VGrepEnumKeys());
	//SearchKeyFilter filterSearchFolder(m_vecSearchFolderKeys, VGrepEnumKeys());
	FileExcludeKeyFilter filterExcludeFile(m_vecExceptFileKeys, m_vecExceptAbsFileKeys);
	FolderExcludeKeyFilter filterExcludeFolder(m_vecExceptFolderKeys, m_vecExceptAbsFolderKeys);

	// 出力バッファをクリアする
	szFile[0] = _T('\0');

	// 一時バッファを確保する
	SFilePathLong szTemp;

	filterSearchFile.GetFilters(szTemp, chDelimiter);
	if (szFile.BUFFER_COUNT <= _tcslen(szFile) + _tcslen(szTemp) + 1) return;
	_tcscat_s(szFile, szFile.BUFFER_COUNT, szTemp);

	filterExcludeFile.GetFilters(szTemp, chDelimiter);
	if (szFile.BUFFER_COUNT <= _tcslen(szFile) + _tcslen(szTemp) + 1) return;
	_tcscat_s(szFile, szFile.BUFFER_COUNT, szTemp);

	filterExcludeFolder.GetFilters(szTemp, chDelimiter);
	if (szFile.BUFFER_COUNT <= _tcslen(szFile) + _tcslen(szTemp) + 1) return;
	_tcscat_s(szFile, szFile.BUFFER_COUNT, szTemp);

	if (szFile[0] != _T('\0')) szFile[::_tcslen(szFile) - 1] = _T('\0');
}

/*!
 * 共有メモリに保存するファイルパターン文字列を組み立てる
 *
 * @param chDelimiter 区切り文字
 * @retval CShareDataのファイルパターン履歴に入れる文字列
 */
std::tstring CGrepEnumKeys::getFileKeys(_In_ TCHAR chDelimiter)
{
	assert(chDelimiter);
	assert(_tcschr(WILDCARD_DELIMITER, chDelimiter));

	std::basic_ostringstream<TCHAR> out;

	//VGrepEnumKeys m_vecSearchFileKeys;
	std::for_each(m_vecSearchFileKeys.cbegin(), m_vecSearchFileKeys.cend(),
		[&out, chDelimiter](const auto &key) { out << key << chDelimiter; });

	//VGrepEnumKeys m_vecSearchFolderKeys;
	// 対象外

	//VGrepEnumKeys m_vecExceptFileKeys;
	std::for_each(m_vecExceptFileKeys.cbegin(), m_vecExceptFileKeys.cend(),
		[&out, chDelimiter](const auto &key) { out << _T('!') << key << chDelimiter; });

	//VGrepEnumKeys m_vecExceptFolderKeys;
	std::for_each(m_vecExceptFolderKeys.cbegin(), m_vecExceptFolderKeys.cend(),
		[&out, chDelimiter](const auto &key) { out << _T('#') << key << chDelimiter; });

	//VGrepEnumKeys m_vecExceptAbsFileKeys;
	std::for_each(m_vecExceptAbsFileKeys.cbegin(), m_vecExceptAbsFileKeys.cend(),
		[&out, chDelimiter](const auto &key) { out << _T('!') << key << chDelimiter; });

	//VGrepEnumKeys m_vecExceptAbsFolderKeys;
	std::for_each(m_vecExceptAbsFolderKeys.cbegin(), m_vecExceptAbsFolderKeys.cend(),
		[&out, chDelimiter](const auto &key) { out << _T('#') << key << chDelimiter; });

	return std::move(out.str());
}
