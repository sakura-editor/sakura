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
#include "env/CProfile.h"

#include <algorithm>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "io/CTextStream.h"
#include "charset/CUtf8.h"		// Resource読み込みに使用
#include "basis/CEol.h"
#include "util/file.h"
#include "apiwrap/StdApi.h"

/*!
	sakura.iniの1行を処理する．

	1行の読み込みが完了するごとに呼ばれる．
	
	@param line [in] 読み込んだ行
*/
void CProfile::_ReadOneline(
	const std::wstring& line
)
{
	//	空行を読み飛ばす
	if( line.empty() )
		return;

	//コメント行を読みとばす
	if (';' == line.front() || line.starts_with(L"//")) {
		return;
	}

	// セクション取得
	if (std::wsmatch m; std::regex_match(line, m, std::wregex(LR"(^\[([^=]+)\]$)"))) {
		m_ProfileData.emplace_back(static_cast<std::wstring>(m[1]));
		return;
	}

	// エントリ取得
	// ※最初のセクション以前の行のエントリは無視
	if (std::wsmatch m; !m_ProfileData.empty() && std::regex_match(line, m, std::wregex(LR"(^([^=]+)=(.*)$)"))) {
		m_ProfileData.back().m_Entries.try_emplace(m[1], m[2]);
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
bool CProfile::ReadProfile(
	const std::optional<std::filesystem::path>& optProfilePath
) noexcept
{
	if (optProfilePath.has_value()) {
		m_ProfilePath = optProfilePath.value();
	}

	CTextInputStream in(m_ProfilePath.c_str());
	if(!in){
		return false;
	}

	try{
		while( in ){
			//1行読込
			std::wstring line=in.ReadLineW();

			//解析
			_ReadOneline(line);
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
	std::wstring line;
	CMemory cmLine;
	CNativeW cmLineW;
	//m_strProfileName = L"-Res-";

	if (( hRsrc = ::FindResource( nullptr, pName, pType )) != nullptr
	 && ( hGlobal = ::LoadResource( nullptr, hRsrc )) != nullptr
	 && ( psMMres = (char *)::LockResource(hGlobal)) != nullptr
	 && ( nSize = (size_t)::SizeofResource( nullptr, hRsrc )) != 0) {
		p    = psMMres;
		if (nSize >= sizeof(UTF8_BOM) && memcmp( p, UTF8_BOM, sizeof(UTF8_BOM) )==0) {
			// Skip BOM
			p += sizeof(UTF8_BOM);
		}
		for (; p < psMMres + nSize ; p = pn) {
			// 1行切り取り（長すぎた場合切捨て）
			pn = strpbrk(p, "\n");
			if (pn == nullptr) {
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
				_ReadOneline(line);
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
	const std::optional<std::filesystem::path>& optProfilePath,
	const std::optional<std::wstring>& optComment
)
{
	if (optProfilePath.has_value()) {
		m_ProfilePath = optProfilePath.value();
	}

	if (m_ProfilePath.empty()) {
		return false;
	}

	if (std::error_code ec; !std::filesystem::exists( m_ProfilePath.parent_path(), ec)) {
		std::filesystem::create_directories(m_ProfilePath.parent_path());
	}
    
	std::vector<std::wstring> lines;
	if (optComment.has_value()) {
		lines.emplace_back(L";" + optComment.value());		// //->;	2008/5/24 Uchi
		lines.emplace_back();
	}
	for(const auto& section : m_ProfileData) {
		//セクション名を書き込む
		lines.emplace_back( L"[" + section.m_Name + L"]" );
		for(const auto& [key, val] : section.m_Entries) {
			//エントリを書き込む
			lines.emplace_back(key + L"=" + val);
		}
		lines.emplace_back();
	}

	// 別ファイルに書き込んでから置き換える（プロセス強制終了などへの安全対策）
	WCHAR szMirrorFile[_MAX_PATH];
	szMirrorFile[0] = L'\0';
	WCHAR szPath[_MAX_PATH];
	LPWSTR lpszName;
	DWORD nLen = ::GetFullPathName(m_ProfilePath.c_str(), int(std::size(szPath)), szPath, &lpszName);
	if( 0 < nLen && nLen < int(std::size(szPath))
		&& (lpszName - szPath + 11) < int(std::size(szMirrorFile)) )	// path\preuuuu.TMP
	{
		*lpszName = L'\0';
		::GetTempFileName(szPath, L"sak", 0, szMirrorFile);
	}

	if( !_WriteFile(szMirrorFile[0]? szMirrorFile: m_ProfilePath, lines) )
		return false;

	if( szMirrorFile[0] ){
		if (!::ReplaceFile(m_ProfilePath.c_str(), szMirrorFile, nullptr, 0, nullptr, nullptr)) {
			if (fexist(m_ProfilePath.c_str())) {
				if (!::DeleteFile(m_ProfilePath.c_str())) {
					return false;
				}
			}
			if (!::MoveFile(szMirrorFile, m_ProfilePath.c_str())) {
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
	const std::filesystem::path&	path,	//!< [in]  ファイル名
	std::span<const std::wstring>	lines			//!< [out] 文字列格納先
)
{
	CTextOutputStream out(path.c_str());
	if(!out){
		return false;
	}

	for(const auto &line : lines){
		// 出力
		out.WriteString(line.c_str());
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
	const std::wstring&	sectionName,	//!< [in] セクション名
	const std::wstring&	entryKey,		//!< [in] エントリ名
	std::wstring&		strEntryValue	//!< [out] エントリ値
) const
{
	// セクション名が一致するセクションを探す
	const auto foundSection = std::ranges::find_if(m_ProfileData, [&sectionName](const auto& section) { return section.m_Name == sectionName; });
	if (foundSection == m_ProfileData.cend()) {
		return false;
	}

	// キーが一致するエントリを探す
	auto& sectionEntries = foundSection->m_Entries;
	const auto foundEntries = sectionEntries.find(entryKey);
	if (foundEntries == sectionEntries.cend()) {
		return false;
	}

	// エントリの値をコピーする
	strEntryValue = foundEntries->second;

	return true;
}

/*! エントリをProfileへ書き込む
	
	@date 2003-10-21 D.S.Koba 作成
*/
void CProfile::SetProfileData(
	const std::wstring&	sectionName,	//!< [in] セクション名
	const std::wstring&	entryKey,		//!< [in] エントリ名
	std::wstring_view	entryValue		//!< [in] エントリ値
)
{
	// セクション名が一致するセクションがない場合、空のセクションを追加する
	if (const auto iter = std::ranges::find_if(m_ProfileData, [&sectionName](const auto& section) { return section.m_Name == sectionName; }); iter == m_ProfileData.cend()) {
		m_ProfileData.emplace_back(sectionName);
	}

	// セクション名が一致するセクションを探す
	auto foundSection = std::ranges::find_if(m_ProfileData, [&sectionName](const auto& section) { return section.m_Name == sectionName; });
	if (foundSection == m_ProfileData.end()) {
		return;
	}

	// エントリに指定された値を書き込む
	foundSection->m_Entries[entryKey] = entryValue;
}

void CProfile::DUMP( void )
{
#ifdef _DEBUG
	//	2006.02.20 ryoji: MAP_STR_STR_ITER削除時の修正漏れによるコンパイルエラー修正
	MYTRACE( L"\n\nCProfile::DUMP()======================" );
	for (const auto& section : m_ProfileData) {
		MYTRACE( L"\n■strSectionName=%ls", section.m_Name.c_str() );
		for (const auto& [key, val] : section.m_Entries) {
			MYTRACE( L"\"%ls\" = \"%ls\"\n", key.c_str(), val.c_str() );
		}
	}
	MYTRACE( L"========================================\n" );
#endif
	return;
}
