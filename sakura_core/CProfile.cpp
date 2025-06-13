/*!	@file
	@brief INIファイル入出力

	@author D.S.Koba
	@date 2003-10-21 D.S.Koba メンバ関数の名前と引数をそのままにしてメンバ変数，関数の中身を書き直し
	@date 2004-01-10 D.S.Koba 返値をBOOLからboolへ変更。IOProfileDataを型別の関数に分け，引数を減らす
	@date 2006-02-11 D.S.Koba 読み込み/書き出しを引数でなく，メンバで判別
	@date 2006-02-12 D.S.Koba IOProfileDataの中身の読み込みと書き出しを関数に分ける
*/
/*
	Copyright (C) 2003, D.S.Koba
	Copyright (C) 2004, D.S.Koba, MIK, genta
	Copyright (C) 2006, D.S.Koba, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CProfile.h"

#include <algorithm>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "io/CTextStream.h"
#include "charset/CUtf8.h"		// Resource読み込みに使用
#include "CEol.h"
#include "util/file.h"
#include "apiwrap/StdApi.h"

void EnsureDirectoryExist( const std::wstring& strProfileName )
{
	const size_t cchLastYen = strProfileName.find_last_of( L'\\' );
	if( cchLastYen != std::wstring::npos && cchLastYen < strProfileName.length() && cchLastYen + 1 < _MAX_PATH ){
		// フォルダーのパスを取得する
		WCHAR szProfileFolder[_MAX_PATH]{ 0 };
		::wcsncpy_s( szProfileFolder, strProfileName.data(), cchLastYen + 1 );

		// フォルダーが存在しなければ作成する
		if( !IsDirectory( szProfileFolder ) ){
			MakeSureDirectoryPathExistsW( szProfileFolder );
		}
	}
}

/*! Profileを初期化
	
	@date 2003-10-21 D.S.Koba STLで書き直す
*/
void CProfile::Init( void )
{
	m_strProfileName.clear();
	m_ProfileData.clear();
	m_bRead = true;
	return;
}

/*!
	sakura.iniの1行を処理する．

	1行の読み込みが完了するごとに呼ばれる．
	
	@param line [in] 読み込んだ行
*/
void CProfile::ReadOneline(
	const wstring& line
)
{
	//	空行を読み飛ばす
	if( line.empty() )
		return;

	//コメント行を読みとばす
	if( 0 == line.compare( 0, 2, LTEXT("//") ))
		return;

	// セクション取得
	//	Jan. 29, 2004 genta compare使用
	if( line.compare( 0, 1, LTEXT("[") ) == 0 
			&& line.find( LTEXT('=') ) == line.npos
			&& line.find( LTEXT(']') ) == ( line.size() - 1 ) ) {
		Section Buffer;
		Buffer.strSectionName = line.substr( 1, line.size() - 1 - 1 );
		m_ProfileData.push_back( Buffer );
	}
	// エントリ取得
	else if( !m_ProfileData.empty() ) {	//最初のセクション以前の行のエントリは無視
		wstring::size_type idx = line.find( LTEXT('=') );
		if( line.npos != idx ) {
			m_ProfileData.back().mapEntries.emplace( line.substr(0,idx), line.substr(idx+1) );
		}
	}
}

/*! Profileをファイルから読み出す
	
	@param pszProfileName [in] ファイル名

	@retval true  成功
	@retval false 失敗

	@date 2003-10-21 D.S.Koba STLで書き直す
	@date 2003-10-26 D.S.Koba ReadProfile()から分離
	@date 2004-01-29 genta stream使用をやめてCライブラリ使用に．
	@date 2004-01-31 genta 行の解析の方を別関数にしてReadFileをReadProfileに
		
*/
bool CProfile::ReadProfile( const WCHAR* pszProfileName )
{
	m_strProfileName = pszProfileName;

	CTextInputStream in(m_strProfileName.c_str());
	if(!in){
		return false;
	}

	try{
		while( in ){
			//1行読込
			wstring line=in.ReadLineW();

			//解析
			ReadOneline(line);
		}
	}
	catch( ... ){
		return false;
	}

	return true;
}

/*! Profileをリソースから読み出す
	
	@param pName [in] リソース名
	@param pType [in] リソースタイプ

	@retval true  成功
	@retval false 失敗

	@date 2010/5/19 MainMenu用に作成

	1行300文字までに制限
*/
bool CProfile::ReadProfileRes( const WCHAR* pName, const WCHAR* pType, std::vector<std::wstring>* pData )
{
	static const BYTE UTF8_BOM[]={0xEF,0xBB,0xBF};
	HRSRC		hRsrc;
	HGLOBAL		hGlobal;
	size_t		nSize;
	char*		psMMres;
	char*		p;
	char		sLine[300+1];
	char*		pn;
	size_t		lnsz;
	wstring		line;
	CMemory cmLine;
	CNativeW cmLineW;
	m_strProfileName = L"-Res-";

	if (( hRsrc = ::FindResource( nullptr, pName, pType )) != nullptr
	 && ( hGlobal = ::LoadResource( nullptr, hRsrc )) != nullptr
	 && ( psMMres = (char *)::LockResource(hGlobal)) != NULL
	 && ( nSize = (size_t)::SizeofResource( nullptr, hRsrc )) != 0) {
		p    = psMMres;
		if (nSize >= sizeof(UTF8_BOM) && memcmp( p, UTF8_BOM, sizeof(UTF8_BOM) )==0) {
			// Skip BOM
			p += sizeof(UTF8_BOM);
		}
		for (; p < psMMres + nSize ; p = pn) {
			// 1行切り取り（長すぎた場合切捨て）
			pn = strpbrk(p, "\n");
			if (pn == NULL) {
				// 最終行
				pn = psMMres + nSize;
			}
			else {
				pn++;
			}
			lnsz = (pn-p)<=300 ? (pn-p) : 300;
			memcpy(sLine, p, lnsz);
			sLine[lnsz] = '\0';
			if (sLine[lnsz-1] == '\n')	sLine[--lnsz] = '\0';
			if (sLine[lnsz-1] == '\r')	sLine[--lnsz] = '\0';
			
			// UTF-8 -> UNICODE
			cmLine.SetRawDataHoldBuffer( sLine, lnsz );
			CUtf8::UTF8ToUnicode( cmLine, &cmLineW );
			line = cmLineW.GetStringPtr();

			if( pData ){
				pData->push_back(line);
			}else{
				//解析
				ReadOneline(line);
			}
		}
	}
	return true;
}

/*! Profileをファイルへ書き出す
	
	@param pszProfileName [in] ファイル名(NULL=最後に読み書きしたファイル)
	@param pszComment [in] コメント文(NULL=コメント省略)

	@retval true  成功
	@retval false 失敗

	@date 2003-10-21 D.S.Koba STLで書き直す
	@date 2004-01-28 D.S.Koba ファイル書き込み部を分離
	@date 2009.06.24 ryoji 別ファイルに書き込んでから置き換える処理を追加
*/
bool CProfile::WriteProfile(
	const WCHAR* pszProfileName,
	const WCHAR* pszComment
)
{
	if( pszProfileName!=NULL ) {
		m_strProfileName = pszProfileName;

		EnsureDirectoryExist( m_strProfileName );
	}
    
	std::vector< wstring > vecLine;
	if( NULL != pszComment ) {
		vecLine.emplace_back( LTEXT(";") + wstring( pszComment ) );		// //->;	2008/5/24 Uchi
		vecLine.push_back( LTEXT("") );
	}
	for(auto iter = m_ProfileData.cbegin(); iter != m_ProfileData.cend(); iter++ ) {
		//セクション名を書き込む
		vecLine.push_back( LTEXT("[") + iter->strSectionName + LTEXT("]") );
		for(auto mapiter = iter->mapEntries.cbegin(); mapiter != iter->mapEntries.cend(); mapiter++ ) {
			//エントリを書き込む
			vecLine.push_back( mapiter->first + LTEXT("=") + mapiter->second );
		}
		vecLine.push_back( LTEXT("") );
	}

	// 別ファイルに書き込んでから置き換える（プロセス強制終了などへの安全対策）
	WCHAR szMirrorFile[_MAX_PATH];
	szMirrorFile[0] = L'\0';
	WCHAR szPath[_MAX_PATH];
	LPWSTR lpszName;
	DWORD nLen = ::GetFullPathName(m_strProfileName.c_str(), _countof(szPath), szPath, &lpszName);
	if( 0 < nLen && nLen < _countof(szPath)
		&& (lpszName - szPath + 11) < _countof(szMirrorFile) )	// path\preuuuu.TMP
	{
		*lpszName = L'\0';
		::GetTempFileName(szPath, L"sak", 0, szMirrorFile);
	}

	if( !_WriteFile(szMirrorFile[0]? szMirrorFile: m_strProfileName, vecLine) )
		return false;

	if( szMirrorFile[0] ){
		if (!::ReplaceFile(m_strProfileName.c_str(), szMirrorFile, NULL, 0, NULL, NULL)) {
			if (fexist(m_strProfileName.c_str())) {
				if (!::DeleteFile(m_strProfileName.c_str())) {
					return false;
				}
			}
			if (!::MoveFile(szMirrorFile, m_strProfileName.c_str())) {
				return false;
			}
		}
	}

	return true;
}

/*! ファイルへ書き込む
	
	@retval true  成功
	@retval false 失敗

	@date 2004-01-28 D.S.Koba WriteProfile()から分離
	@date 2004-01-29 genta stream使用をやめてCライブラリ使用に．
*/
bool CProfile::_WriteFile(
	const wstring&			strFilename,	//!< [in]  ファイル名
	const std::vector<wstring>&	vecLine		//!< [out] 文字列格納先
)
{
	CTextOutputStream out(strFilename.c_str());
	if(!out){
		return false;
	}

	size_t nSize = vecLine.size();
	for(size_t i=0;i<nSize;i++){
		// 出力
		out.WriteString(vecLine[i].c_str());
		out.WriteString(L"\n");
	}

	out.Close();

	return true;
}

/*! エントリ値をProfileから読み込む
	
	@retval true 成功
	@retval false 失敗

	@date 2003-10-22 D.S.Koba 作成
*/
bool CProfile::GetProfileData(
	std::wstring_view	sectionName,	//!< [in] セクション名
	std::wstring_view	entryKey,		//!< [in] エントリ名
	std::wstring&		strEntryValue	//!< [out] エントリ値
) const
{
	// セクション名が一致するセクションを探す
	if (const auto iter = std::find_if(m_ProfileData.cbegin(), m_ProfileData.cend(), [&sectionName](const auto& section) {return section.strSectionName == sectionName; }); iter != m_ProfileData.cend()) {
		// キーが一致するエントリを探す
		if (const auto mapiter = iter->mapEntries.find(entryKey.data()); iter->mapEntries.cend() != mapiter) {
			// エントリの値をコピーする
			strEntryValue = mapiter->second;
			return true;
		}
	}
	return false;
}

/*! エントリをProfileへ書き込む
	
	@date 2003-10-21 D.S.Koba 作成
*/
void CProfile::SetProfileData(
	std::wstring_view	sectionName,	//!< [in] セクション名
	std::wstring_view	entryKey,		//!< [in] エントリ名
	std::wstring_view	entryValue		//!< [in] エントリ値
)
{
	// セクション名が一致するセクションがない場合、空のセクションを追加する
	if (const auto iter = std::find_if(m_ProfileData.cbegin(), m_ProfileData.cend(), [&sectionName](const auto& section) {return section.strSectionName == sectionName; }); iter == m_ProfileData.cend()) {
		m_ProfileData.emplace_back(Section{ sectionName.data() });
	}
	// セクション名が一致するセクションを探す
	if (auto iter = std::find_if(m_ProfileData.begin(), m_ProfileData.end(), [&sectionName](const auto& section) {return section.strSectionName == sectionName; }); iter != m_ProfileData.end()) {
		// エントリに指定された値を書き込む
		auto& sectionEntries = iter->mapEntries;
		sectionEntries[entryKey.data()] = entryValue;
	}
}

void CProfile::DUMP( void )
{
#ifdef _DEBUG
	//	2006.02.20 ryoji: MAP_STR_STR_ITER削除時の修正漏れによるコンパイルエラー修正
	MYTRACE( L"\n\nCProfile::DUMP()======================" );
	for(auto iter = m_ProfileData.cbegin(); iter != m_ProfileData.cend(); iter++ ) {
		MYTRACE( L"\n■strSectionName=%ls", iter->strSectionName.c_str() );
		for(auto mapiter = iter->mapEntries.cbegin(); mapiter != iter->mapEntries.cend(); mapiter++ ) {
			MYTRACE( L"\"%ls\" = \"%ls\"\n", mapiter->first.c_str(), mapiter->second.c_str() );
		}
	}
	MYTRACE( L"========================================\n" );
#endif
	return;
}
