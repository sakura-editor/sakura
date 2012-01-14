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
#ifdef USE_STREAM
#  include <fstream>
#else
#  include <stdio.h>
#  include <string.h>
#endif
//#include <sstream>
#include <algorithm>
#include "CProfile.h"
#include "Debug.h"


/*! Profileを初期化
	
	@date 2003-10-21 D.S.Koba STLで書き直す
*/
void CProfile::Init( void )
{
	m_strProfileName = _T("");
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
	const std::basic_string< TCHAR >& line
)
{
	//	空行を読み飛ばす
	if( line.empty() )
		return;

	//コメント行を読みとばす
	if( 0 == line.compare( 0, 2, _T("//") ))
		return;

	// セクション取得
	//	Jan. 29, 2004 genta compare使用
	if( line.compare( 0, 1, _T("[") ) == 0 
			&& line.find( _T("=") ) == line.npos
			&& line.find( _T("]") ) == ( line.size() - 1 ) ) {
		Section Buffer;
		Buffer.strSectionName = line.substr( 1, line.size() - 1 - 1 );
		m_ProfileData.push_back( Buffer );
	}
	// エントリ取得
	else if( !m_ProfileData.empty() ) {	//最初のセクション以前の行のエントリは無視
		std::basic_string< TCHAR >::size_type idx = line.find( _T("=") );
		if( line.npos != idx ) {
			m_ProfileData.back().mapEntries.insert( PAIR_STR_STR( line.substr(0,idx), line.substr(idx+1) ) );
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
	@date 2007-07-02 ryoji バイナリモードで読むように変更
		テキストモードではCtrl+Z(0x1A)は入力時にEOF文字として解釈されるので末尾に到達する前に読み終わってしまうことがある
		例）0x1Aを検索した後で再起動すると設定が初期状態に戻る障害が発生する
*/
bool CProfile::ReadProfile( const TCHAR* pszProfileName )
{
	m_strProfileName = pszProfileName;
#ifdef USE_STREAM
	std::basic_ifstream< TCHAR > ifs( m_strProfileName.c_str() );
	if(!ifs.is_open()) return false;
	
	std::basic_string< TCHAR > strWork;
	try {
		for(;;) {
			std::getline( ifs, strWork );
			ReadOneline( strWork );
			if(ifs.eof()) break;
		}
	} //try
	catch(...) {
		//ファイルの読み込み失敗
		ifs.close();
		return false;
	}
	ifs.close();
#else
	FILE* fp = _tfopen( m_strProfileName.c_str(), _T("rb"));	// 2009.07.02 ryoji "r"->"rb"
	if( fp == NULL ){
		return false;
	}

	const int CHUNK_SIZE = 2048;			// てきと～
	TCHAR* buf = new TCHAR[ CHUNK_SIZE ];	//	読み込み用
	std::basic_string< TCHAR > bstr;		//	作業用
	unsigned int offset = 0;				//	出力済み領域チェック用

	try {
		while( !feof( fp )){
			int length = fread( buf, sizeof( TCHAR ), CHUNK_SIZE, fp );
			
			//	エラーチェック
			if( ferror( fp )){
				delete [] buf;
				fclose( fp );
				return false;
			}
			
			bstr = bstr.substr( offset ) + std::basic_string< TCHAR >( buf, length );
			offset = 0;
			
			int pos;
			//	\nが見つかる間ループ
			while(( pos = bstr.find( _T("\r\n"), offset ) ) != bstr.npos ){	// 2009.07.02 ryoji '\n'->"\r\n"
				//	改行コードは渡さない
				ReadOneline( bstr.substr( offset, pos - offset ) );
				offset = pos + 2;	// 2009.07.02 ryoji 1->2
			}

			if( feof( fp )){
				if( offset < bstr.size() ){
					//	最終行は\nが無くてもとりあえず
					ReadOneline( bstr.substr( offset ) );
				}
				break;
			}
		}
	}
	catch( ... ){
		delete [] buf;
		fclose( fp );
		return false;
	}
	delete [] buf;
	fclose( fp );
#endif

#ifdef _DEBUG
	//DUMP();
#endif
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
	const TCHAR* pszProfileName,
	const TCHAR* pszComment
)
{
	if( NULL != pszProfileName ) {
		m_strProfileName = pszProfileName;
	}
    
	std::vector< std::basic_string< TCHAR > > vecLine;
	if( NULL != pszComment ) {
		vecLine.push_back( _T("//") + std::basic_string< TCHAR >( pszComment ) );
		vecLine.push_back( _T("") );
	}
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator iterEnd = m_ProfileData.end();
	MAP_STR_STR::iterator mapiter;
	MAP_STR_STR::iterator mapiterEnd;
	for( iter = m_ProfileData.begin(); iter != iterEnd; iter++ ) {
		//セクション名を書き込む
		vecLine.push_back( _T("[") + iter->strSectionName + _T("]") );
		mapiterEnd = iter->mapEntries.end();
		for( mapiter = iter->mapEntries.begin(); mapiter != mapiterEnd; mapiter++ ) {
			//エントリを書き込む
			vecLine.push_back( mapiter->first + _T("=") + mapiter->second );
		}
		vecLine.push_back( _T("") );
	}

	// 別ファイルに書き込んでから置き換える（プロセス強制終了などへの安全対策）
	TCHAR szMirrorFile[_MAX_PATH];
	szMirrorFile[0] = _T('\0');
	TCHAR szPath[_MAX_PATH];
	LPTSTR lpszName;
	DWORD nLen = ::GetFullPathName(m_strProfileName.c_str(), sizeof(szPath)/sizeof(TCHAR), szPath, &lpszName);
	if( 0 < nLen && nLen < sizeof(szPath)/sizeof(TCHAR)
		&& (lpszName - szPath + 11) < sizeof(szMirrorFile)/sizeof(TCHAR) )	// path\preuuuu.TMP
	{
		*lpszName = _T('\0');
		::GetTempFileName(szPath, _T("sak"), 0, szMirrorFile);
	}

	if( !WriteFile(szMirrorFile[0]? std::basic_string< TCHAR >(szMirrorFile): m_strProfileName, vecLine) )
		return false;

	if( szMirrorFile[0] ){
		BOOL (__stdcall *pfnReplaceFile)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPVOID, LPVOID);
		HMODULE hModule = ::GetModuleHandle(_T("KERNEL32"));
		pfnReplaceFile = (BOOL (__stdcall *)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPVOID, LPVOID))
#ifndef _UNICODE
			::GetProcAddress(hModule, "ReplaceFileA");
#else
			::GetProcAddress(hModule, "ReplaceFileW");
#endif
		if( !pfnReplaceFile || !pfnReplaceFile(m_strProfileName.c_str(), szMirrorFile, NULL, 0, NULL, NULL) ){
			::DeleteFile(m_strProfileName.c_str());
			::MoveFile(szMirrorFile, m_strProfileName.c_str());
		}
	}

	return true;
}

/*! ファイルへ書き込む
	
	@param strFilename [in] ファイル名
	@param vecLine [out] 文字列格納先

	@retval true  成功
	@retval false 失敗

	@date 2004-01-28 D.S.Koba WriteProfile()から分離
	@date 2004-01-29 genta stream使用をやめてCライブラリ使用に．
*/
bool CProfile::WriteFile(
	const std::basic_string< TCHAR >& strFilename,
	std::vector< std::basic_string< TCHAR > >& vecLine
)
{
#ifdef USE_STREAM
	std::basic_ofstream< TCHAR > ofs( strFilename.c_str() );
	if(!ofs.is_open()) return false;

	std::vector< std::basic_string< TCHAR > >::iterator iter;
	std::vector< std::basic_string< TCHAR > >::iterator iterEnd = vecLine.end();

	try {
		for( iter = vecLine.begin(); iter != iterEnd; iter++ ) {
			ofs << iter->c_str() << _T("\n");
		}
	}
	catch(...) {
		ofs.close();
		return false;
	}
	ofs.close();

#else
	//	Jan. 29, 2004 genta standard i/o version
	FILE* fp = _tfopen( strFilename.c_str(), _T( "w" ));
	if( fp == NULL ){
		return false;
	}

	std::vector< std::basic_string< TCHAR > >::iterator iter;
	std::vector< std::basic_string< TCHAR > >::iterator iterEnd = vecLine.end();

	for( iter = vecLine.begin(); iter != iterEnd; iter++ ) {
		//	文字列に\0を含む場合を考慮してバイナリ出力
		if( fwrite( iter->data(), sizeof( TCHAR ), iter->size(), fp ) != iter->size() ){
			fclose( fp );
			return false;
		}
		if( _fputtc( _T('\n'), fp ) == _TEOF ){
			fclose( fp );
			return false;
		}
	}
	fclose( fp );
#endif
	return true;
}

bool CProfile::IOProfileData(
	const TCHAR* pszSectionName, const TCHAR* pszEntryKey, bool& bEntryValue)
{
	if(m_bRead)	return GetProfileData( pszSectionName, pszEntryKey, bEntryValue );
	else		return SetProfileData( pszSectionName, pszEntryKey, bEntryValue );
}

bool CProfile::IOProfileData(
	const TCHAR* pszSectionName, const TCHAR* pszEntryKey, int& nEntryValue)
{
	if(m_bRead)	return GetProfileData( pszSectionName, pszEntryKey, nEntryValue );
	else		return SetProfileData( pszSectionName, pszEntryKey, nEntryValue );
}

bool CProfile::IOProfileData(
	const TCHAR* pszSectionName, const TCHAR* pszEntryKey, WORD& wEntryValue)
{
	if(m_bRead)	return GetProfileData( pszSectionName, pszEntryKey, wEntryValue );
	else		return SetProfileData( pszSectionName, pszEntryKey, wEntryValue );
}

bool CProfile::IOProfileData(
	const TCHAR* pszSectionName, const TCHAR* pszEntryKey, TCHAR& chEntryValue)
{
	if(m_bRead)	return GetProfileData( pszSectionName, pszEntryKey, chEntryValue );
	else		return SetProfileData( pszSectionName, pszEntryKey, chEntryValue );
}

bool CProfile::IOProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	TCHAR*			pEntryValue,
	const int&		nEntryValueSize
)
{
	if(m_bRead)	return GetProfileData( pszSectionName, pszEntryKey, pEntryValue, nEntryValueSize );
	else		return SetProfileData( pszSectionName, pszEntryKey, pEntryValue, nEntryValueSize );
}

bool CProfile::IOProfileData(
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	std::basic_string< TCHAR >&			strEntryValue
)
{
	if(m_bRead)	return GetProfileData( strSectionName, strEntryKey, strEntryValue );
	else		return SetProfileData( strSectionName, strEntryKey, strEntryValue );
}

/*! エントリ値(bool型)をProfileから読み込む
	
	@param[in] pszSectionName セクション名
	@param[in] pszEntryKey エントリ名
	@param[out] bEntryValue エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::GetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	bool&			bEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
	if( strWork != _T("0") )	bEntryValue = true;
	else						bEntryValue = false;
	return true;
}

/*! エントリ値(bool型)をProfileへ書き込む
	
	@param[in] pszSectionName セクション名
	@param[in] pszEntryKey エントリ名
	@param[in] bEntryValue エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::SetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	const bool&		bEntryValue
)
{
	std::basic_string< TCHAR > strNewEntryValue;
	if( bEntryValue == true )	strNewEntryValue = _T("1");
	else						strNewEntryValue = _T("0");
	return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
}

/*! エントリ値(int型)をProfileから読み込む
	
	@param[in] pszSectionName セクション名
	@param[in] pszEntryKey エントリ名
	@param[out] nEntryValue エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
	@date 2004-02-14 MIK _tstoi→_ttoi
*/
bool CProfile::GetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	int&			nEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
	nEntryValue = _ttoi( strWork.c_str() );
	//std::basic_stringstream< TCHAR > stream;
	//stream << strWork;
	//stream >> EntryValue;
	return true;
}


/*! エントリ値(int型)をProfileへ書き込む
	
	@param[in] pszSectionName セクション名
	@param[in] pszEntryKey エントリ名
	@param[in] EntryValue エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
	@date 2004-02-14 MIK _tstoi→_ttoi
*/
bool CProfile::SetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	const int&		nEntryValue
)
{
	TCHAR szWork[32];
	_itot( nEntryValue, szWork, 10 );
	std::basic_string< TCHAR > strNewEntryValue( szWork );
	//std::basic_string< TCHAR > strNewEntryValue;
	//std::basic_stringstream< TCHAR > stream;
	//stream << EntryValue;
	//stream >> strNewEntryValue;
	return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
}

/*! エントリ値(WORD型)をProfileから読み込む
	
	@param[in] pszSectionName セクション名
	@param[in] pszEntryKey エントリ名
	@param[out] wEntryValue エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::GetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	WORD&			wEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
	wEntryValue = _ttoi( strWork.c_str() );
	//std::basic_stringstream< TCHAR > stream;
	//stream << strWork;
	//stream >> EntryValue;
	return true;
}

/*! エントリ値(WORD型)をProfileへ書き込む
	
	@param[in] pszSectionName セクション名
	@param[in] pszEntryKey エントリ名
	@param[in] EntryValue エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::SetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	const WORD&		wEntryValue
)
{
	TCHAR szWork[32];
	_itot( wEntryValue, szWork, 10 );
	std::basic_string< TCHAR > strNewEntryValue( szWork );
	//std::basic_string< TCHAR > strNewEntryValue;
	//std::basic_stringstream< TCHAR > stream;
	//stream << EntryValue;
	//stream >> strNewEntryValue;
	return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
}

/*! エントリ値(TCHAR型)をProfileから読み込む
	
	@param[in] pszSectionName セクション名
	@param[in] pszEntryKey エントリ名
	@param[out] EntryValue エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::GetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	TCHAR&			chEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
	if( 0 == strWork.length() )	chEntryValue = _T('\0');
	else						chEntryValue = strWork.at(0);
	return true;
}

/*! エントリ値(TCHAR型)をProfileへ書き込む
	
	@param[in] pszSectionName セクション名
	@param[in] pszEntryKey エントリ名
	@param[in] chEntryValue エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::SetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	const TCHAR&	chEntryValue
)
{
	std::basic_string< TCHAR > strNewEntryValue;
	if( _T('\0') == chEntryValue )	strNewEntryValue = _T("");
	else							strNewEntryValue = chEntryValue;
	return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
}

/*! エントリ値(NULL文字終端の文字列)をProfileから読み込む
	
	@param[in] pszSectionName セクション名
	@param[in] pszEntryKey エントリ名
	@param[out] pEntryValue エントリ値
	@param[in] nEntryValueSize pEntryValueの確保メモリサイズ(サイズが十分であることが明確なら 0 指定可)

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::GetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	TCHAR*			pEntryValue,
	const int&		nEntryValueSize
)
{
	std::basic_string< TCHAR > strWork;
	if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
	if( nEntryValueSize > static_cast<int>(strWork.length())
			|| nEntryValueSize ==0 ) {
		strWork.copy( pEntryValue, strWork.length() );
		//copy()はNULL文字終端しないのでNULL文字追加
		pEntryValue[strWork.length()] = _T('\0');
	}
	else {
		strWork.copy( pEntryValue, nEntryValueSize-1 );
		//copy()はNULL文字終端しないのでNULL文字追加
		pEntryValue[nEntryValueSize-1] = _T('\0');
	}
	return true;
}

/*! エントリ値(NULL文字終端の文字列)をProfileへ書き込む
	
	@param[in] pszSectionName セクション名
	@param[in] pszEntryKey エントリ名
	@param[in] pEntryValue エントリ値
	@param[in] nEntryValueSize pEntryValueの確保メモリサイズ(使用しない)

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::SetProfileData(
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	const TCHAR*	pEntryValue,
	const int&		nEntryValueSize
)
{
	return SetProfileData( pszSectionName, pszEntryKey, std::basic_string< TCHAR >(pEntryValue) );
}

/*! エントリ値をProfileから読み込む
	
	@param[in] strSectionName セクション名
	@param[in] strEntryKey エントリ名
	@param[out] strEntryValue エントリ値

	@retval true 成功
	@retval false 失敗

	@date 2003-10-22 D.S.Koba 作成
*/
bool CProfile::GetProfileData(
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	std::basic_string< TCHAR >&			strEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator iterEnd = m_ProfileData.end();
	MAP_STR_STR::iterator mapiter;
	for( iter = m_ProfileData.begin(); iter != iterEnd; iter++ ) {
		if( iter->strSectionName == strSectionName ) {
			mapiter = iter->mapEntries.find( strEntryKey );
			if( iter->mapEntries.end() != mapiter ) {
				strEntryValue = mapiter->second;
				return true;
			}
		}
	}
	return false;
}

/*! エントリをProfileへ書き込む
	
	@param[in] strSectionName セクション名
	@param[in] strEntryKey エントリ名
	@param[in] strEntryValue エントリ値

	@retval true  成功
	@retval false 失敗(処理を入れていないのでfalseは返らない)

	@date 2003-10-21 D.S.Koba 作成
*/
bool CProfile::SetProfileData(
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	const std::basic_string< TCHAR >&	strEntryValue
)
{
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator iterEnd = m_ProfileData.end();
	MAP_STR_STR::iterator mapiter;
	MAP_STR_STR::iterator mapiterEnd;
	for( iter = m_ProfileData.begin(); iter != iterEnd; iter++ ) {
		if( iter->strSectionName == strSectionName ) {
			//既存のセクションの場合
			mapiter = iter->mapEntries.find( strEntryKey );
			if( iter->mapEntries.end() != mapiter ) {
				//既存のエントリの場合は値を上書き
				mapiter->second = strEntryValue;
				break;
			}
			else {
				//既存のエントリが見つからない場合は追加
				iter->mapEntries.insert( PAIR_STR_STR( strEntryKey, strEntryValue ) );
				break;
			}
		}
	}
	//既存のセクションではない場合，セクション及びエントリを追加
	if( iterEnd == iter ) {
		Section Buffer;
		Buffer.strSectionName = strSectionName;
		Buffer.mapEntries.insert( PAIR_STR_STR( strEntryKey, strEntryValue ) );
		m_ProfileData.push_back( Buffer );
	}
	return true;
}



void CProfile::DUMP( void )
{
#ifdef _DEBUG
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator iterEnd = m_ProfileData.end();
	//	2006.02.20 ryoji: MAP_STR_STR_ITER削除時の修正漏れによるコンパイルエラー修正
	MAP_STR_STR::iterator mapiter;
	MAP_STR_STR::iterator mapiterEnd;
	MYTRACE( "\n\nCProfile::DUMP()======================" );
	for( iter = m_ProfileData.begin(); iter != iterEnd; iter++ ) {
		MYTRACE( "\n■strSectionName=%s", iter->strSectionName.c_str() );
		mapiterEnd = iter->mapEntries.end();
		for( mapiter = iter->mapEntries.begin(); mapiter != mapiterEnd; mapiter++ ) {
			MYTRACE( "\"%s\" = \"%s\"\n", mapiter->first.c_str(), mapiter->second.c_str() );
		}
	}
	MYTRACE( "========================================\n" );
#endif
	return;
}

/*[EOF]*/
