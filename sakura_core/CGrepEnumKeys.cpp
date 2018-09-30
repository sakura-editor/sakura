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
		*q = _T('\0');

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
