//	$Id$
/*!	@file
	@brief INIファイル入出力

	@author D.S.Koba
	$Revision$
	@date 2003-10-21 D.S.Koba メンバ関数の名前と引数をそのままにしてメンバ変数，関数の中身を書き直し
	@date 2004-01-10 D.S.Koba 返値をBOOLからboolへ変更。IOProfileDataを型別の関数に分け，引数を減らす
*/
/*
	Copyright (C) 2003, D.S.Koba

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

#include "CProfile.h"
#include "Debug.h"
#include <fstream>
#include <sstream>
#include <algorithm>


/*! Profileを初期化
	
	@date 2003-10-21 D.S.Koba STLで書き直す
*/
void CProfile::Init( void )
{
	m_strProfileName = _T("");
	m_ProfileData.clear();
	return;
}

/*! Profileをファイルから読み出す
	
	@param pszProfileName [in] ファイル名

	@retval true  成功
	@retval false 失敗

	@date 2003-10-21 D.S.Koba STLで書き直す
*/
bool CProfile::ReadProfile( const TCHAR* pszProfileName )
{
	m_strProfileName = pszProfileName;
	m_ProfileData.reserve( 34 );//高速化のためメモリ確保

	// ファイル読み込み
	std::vector< std::basic_string< TCHAR > > vecLine;
	if( false == ReadFile( m_strProfileName, vecLine ) ) return false;

	unsigned int idx;
	VEC_STR_ITER iter;
	VEC_STR_ITER enditer = vecLine.end();

	// コメント削除
	for( iter=vecLine.begin(); iter!=enditer; iter++ )
	{
		idx = iter->find( _T("//") );
		if( iter->npos != idx )
		{
			iter->erase( idx );
		}
	}

	try
	{
		for( iter=vecLine.begin(); iter!=enditer; iter++ )
		{
			// セクション取得
			if( 0 == iter->find( _T("[") ) && iter->npos == iter->find( _T("=") ) )
			{
				// データを直接変更するので，以降セクション名とは分からなくなることに注意
				iter->erase( iter->begin() );
				iter->erase( --(iter->end()) );
				Section Buffer;
				Buffer.strSectionName = iter->data();
				m_ProfileData.push_back( Buffer );
			}
			// エントリ取得
			else if( 0 != m_ProfileData.size() )	//最初のセクション以前の行のエントリは無視
			{
				idx = iter->find( _T("=") );
				if( iter->npos != idx )
				{
					m_ProfileData.back().Data.insert( PAIR_STR_STR( iter->substr(0,idx), iter->substr(idx+1) ) );
				}
			}
		}
	} //try
	catch(...)
	{
		//iniファイルから情報読み取り失敗
		return false;
	}
    return true;
}

/*! Profileの特定のセクションをファイルから読み出す
	
	@param pszProfileName [in] ファイル名
	@param pszSectionName [in] セクション名

	@retval true  成功
	@retval false 失敗

	@date 2003-10-26 D.S.Koba ReadProfile()を元に作成
*/
bool CProfile::ReadProfileSection(
	const TCHAR* pszProfileName,
	const TCHAR* pszSectionName
)
{
	m_strProfileName = pszProfileName;

	// ファイル読み込み
	std::vector< std::basic_string< TCHAR > > vecLine;
	if( false == ReadFile( m_strProfileName, vecLine ) ) return false;

	unsigned int idx;
	VEC_STR_ITER iter;
	VEC_STR_ITER enditer = vecLine.end();

	// コメント削除
	for( iter=vecLine.begin(); iter!=enditer; iter++ )
	{
		idx = iter->find( _T("//") );
		if( iter->npos != idx )
		{
			iter->erase( idx );
		}
	}

	std::basic_string< TCHAR > strSectionLine = _T("[");
	strSectionLine += pszSectionName;
	strSectionLine += _T("]");

	//目的のセクションを取得
	iter = std::find( vecLine.begin(), vecLine.end(), strSectionLine );
	if( vecLine.end() == iter ) return false;
	
	Section Buffer;
	Buffer.strSectionName = pszSectionName;
	m_ProfileData.push_back( Buffer );

	try
	{
		//エントリを取得
		for( ++iter; iter!=enditer; iter++ )
		{
			//次のセクションに入ったらエントリ読み込み終了
			if( 0 == iter->find( _T("[") ) && iter->npos == iter->find( _T("=") ) ) break;
			idx = iter->find( _T("=") );
			if( iter->npos != idx )
			{
				m_ProfileData.back().Data.insert( PAIR_STR_STR( iter->substr(0,idx), iter->substr(idx+1) ) );
			}
		}
	}
	catch(...)
	{
		//iniファイルから情報読み取り失敗
		return false;
	}
	return true;
}

/*! ファイルを読み込む
	
	@param strFilename [in] ファイル名
	@param vecLine [out] 文字列格納先

	@retval true  成功
	@retval false 失敗

	@date 2003-10-26 D.S.Koba ReadProfile()から分離
*/
inline bool CProfile::ReadFile(
	const std::basic_string< TCHAR >& strFilename,
	std::vector< std::basic_string< TCHAR > >& vecLine
)
{
	std::basic_ifstream< TCHAR > ifs( strFilename.c_str() );
	if(!ifs.is_open()) return false;
	
	std::basic_string< TCHAR > strBuffer;
	try
	{
		for(;;)
		{
			std::getline( ifs, strBuffer );
			vecLine.push_back( strBuffer );
			if(ifs.eof()) break;
		}
	} //try
	catch(...)
	{
		//ファイルの読み込み失敗
		ifs.close();
		return false;
	}
	ifs.close();
	return true;
}


/*! Profileをファイルへ書き出す
	
	@param pszProfileName [in] ファイル名(NULL=最後に読み書きしたファイル)
	@param pszComment [in] コメント文(NULL=コメント省略)

	@retval true  成功
	@retval false 失敗

	@date 2003.02.12 Mr.Nak fprintfを使うように
	@date 2003-10-21 D.S.Koba STLで書き直す
*/
bool CProfile::WriteProfile(
	const TCHAR* pszProfileName,
	const TCHAR* pszComment
)
{
	if( NULL != pszProfileName )
	{
		m_strProfileName = pszProfileName;
	}

	std::basic_ofstream< TCHAR > ofs( m_strProfileName.c_str() );
	if(!ofs.is_open()) return false;

	// コメントを書き込む
	if( NULL != pszComment )
	{
		ofs << "//" << pszComment << "\n" << std::endl;
	}
    
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator enditer = m_ProfileData.end();
	MAP_STR_STR_ITER mapiter;
	MAP_STR_STR_ITER mapenditer;
	for( iter = m_ProfileData.begin(); iter != enditer; iter++ )
	{
		//セクション名を書き込む
		ofs << "[" << iter->strSectionName << "]\n";
		mapenditer = iter->Data.end();
		for( mapiter = iter->Data.begin(); mapiter != mapenditer; mapiter++ )
		{
			//エントリを書き込む
			ofs << mapiter->first << "=" << mapiter->second << "\n";
		}
		ofs << std::endl;
	}
	ofs.close();
	return true;
}

/*! Profileの特定のセクションのファイルへの書き出し
	
	@param pszSectionName [in] セクション名
	@param pszProfileName [in] ファイル名
	@param pszComment [in] コメント文(NULL=コメント省略)

	@retval true  成功
	@retval false 失敗

	@date 2003-10-21 D.S.Koba WriteProfile()を元に作成
*/
bool CProfile::WriteProfileSection(
	const TCHAR* pszSectionName,
	const TCHAR* pszProfileName,
	const TCHAR* pszComment
)
{  
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator enditer = m_ProfileData.end();
	MAP_STR_STR_ITER mapiter;
	MAP_STR_STR_ITER mapenditer;
	for( iter = m_ProfileData.begin(); iter != enditer; iter++ )
	{
		//目的のセクションの場合
		if( iter->strSectionName == pszSectionName )
		{
			std::basic_ofstream< TCHAR > ofs( pszProfileName );
			if(!ofs.is_open()) return false;

			// コメントを書き込む
			if( NULL != pszComment )
			{
				ofs << "//" << pszComment << "\n" << std::endl;
			}
			//セクション名を書き込む
			ofs << "[" << iter->strSectionName << "]\n";
			mapenditer = iter->Data.end();
			for( mapiter = iter->Data.begin(); mapiter != mapenditer; mapiter++ )
			{
				//エントリを書き込む
				ofs << mapiter->first << "=" << mapiter->second << "\n";
			}
			ofs << std::endl;
			ofs.close();
			break;
		}
	}
	if( enditer == iter ) return false;
	return true;
}


/*! エントリ値(bool型)をProfileから読み込む，又はProfileへ書き込む
	
	@param bRead [in] モード(true=読み込み, false=書き込み)
	@param pszSectionName [in] セクション名
	@param pszEntryKey [in] エントリ名
	@param pEntryValue [i/o] エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::IOProfileData(
	const bool&		bRead,
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	bool&			pEntryValue
)
{
	// 「読み込み」か「書き込み」か
	if( bRead )
	{
		std::basic_string< TCHAR > strWork;
		if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
		if( strWork != _T("0") )	pEntryValue = true;
		else						pEntryValue = false;
		return true;
	}
	else
	{
		std::basic_string< TCHAR > strNewEntryValue;
		if( pEntryValue == true )	strNewEntryValue = _T("1");
		else						strNewEntryValue = _T("0");
		return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
	}
}

/*! エントリ値(int型)をProfileから読み込む，又はProfileへ書き込む
	
	@param bRead [in] モード(true=読み込み, false=書き込み)
	@param pszSectionName [in] セクション名
	@param pszEntryKey [in] エントリ名
	@param pEntryValue [i/o] エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::IOProfileData(
	const bool&		bRead,
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	int&			pEntryValue
)
{
	// 「読み込み」か「書き込み」か
	if( bRead )
	{
		std::basic_string< TCHAR > strWork;
		if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
		std::basic_stringstream< TCHAR > stream;
		stream << strWork;
		stream >> pEntryValue;
		return true;
	}
	else
	{
		std::basic_string< TCHAR > strNewEntryValue;
		std::basic_stringstream< TCHAR > stream;
		stream << pEntryValue;
		stream >> strNewEntryValue;
		//strNewEntryValue = stream.str();
		return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
	}
}

/*! エントリ値(WORD型)をProfileから読み込む，又はProfileへ書き込む
	
	@param bRead [in] モード(true=読み込み, false=書き込み)
	@param pszSectionName [in] セクション名
	@param pszEntryKey [in] エントリ名
	@param pEntryValue [i/o] エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::IOProfileData(
	const bool&		bRead,
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	WORD&			pEntryValue
)
{
	// 「読み込み」か「書き込み」か
	if( bRead )
	{
		std::basic_string< TCHAR > strWork;
		if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
		std::basic_stringstream< TCHAR > stream;
		stream << strWork;
		stream >> pEntryValue;
		return true;
	}
	else
	{
		std::basic_string< TCHAR > strNewEntryValue;
		std::basic_stringstream< TCHAR > stream;
		stream << pEntryValue;
		stream >> strNewEntryValue;
		//strNewEntryValue = stream.str();
		return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
	}
}

/*! エントリ値(TCHAR型)をProfileから読み込む，又はProfileへ書き込む
	
	@param bRead [in] モード(true=読み込み, false=書き込み)
	@param pszSectionName [in] セクション名
	@param pszEntryKey [in] エントリ名
	@param pEntryValue [i/o] エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::IOProfileData(
	const bool&		bRead,
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	TCHAR&			pEntryValue
)
{
	// 「読み込み」か「書き込み」か
	if( bRead )
	{
		std::basic_string< TCHAR > strWork;
		if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
		if( 0 == strWork.length() )	pEntryValue = _T('\0');
		else						pEntryValue = strWork.at(0);
		return true;
	}
	else
	{
		std::basic_string< TCHAR > strNewEntryValue;
		if( _T('\0') == pEntryValue )	strNewEntryValue = _T("");
		else							strNewEntryValue = pEntryValue;
		return SetProfileData( pszSectionName, pszEntryKey, strNewEntryValue );
	}
}

/*! エントリ値(NULL文字終端の文字列)をProfileから読み込む，又はProfileへ書き込む
	
	@param bRead [in] モード(true=読み込み, false=書き込み)
	@param pszSectionName [in] セクション名
	@param pszEntryKey [in] エントリ名
	@param pEntryValue [i/o] エントリ値
	@param nEntryValueSize [in] pEntryValueの確保メモリサイズ(読み込み時サイズが十分であることが明確なら 0 指定可。書き込み時は使用しない)

	@retval true  成功
	@retval false 失敗

	@date 2004-01-10 D.S.Koba エントリの型別に関数を分離
*/
bool CProfile::IOProfileData(
	const bool&		bRead,
	const TCHAR*	pszSectionName,
	const TCHAR*	pszEntryKey,
	TCHAR*			pEntryValue,
	const int&		nEntryValueSize
)
{
	// 「読み込み」か「書き込み」か
	if( bRead )
	{
		std::basic_string< TCHAR > strWork;
		if( false == GetProfileData( pszSectionName, pszEntryKey, strWork ) ) return false;
		if( nEntryValueSize > static_cast<int>(strWork.length())
			|| nEntryValueSize ==0 )
		{
			strWork.copy( pEntryValue, strWork.length() );
			//copy()はNULL文字終端しないのでNULL文字追加
			pEntryValue[strWork.length()] = _T('\0');
		}
		else
		{
			strWork.copy( pEntryValue, nEntryValueSize-1 );
			//copy()はNULL文字終端しないのでNULL文字追加
			pEntryValue[nEntryValueSize-1] = _T('\0');
		}
		return true;
	}
	else
	{
		return SetProfileData( pszSectionName, pszEntryKey, pEntryValue );
	}
}

/*! エントリ値(std::basic_string<TCHAR>型)をProfileから読み込む，又はProfileへ書き込む
	
	@param bRead [in] モード(true=読み込み, false=書き込み)
	@param strSectionName [in] セクション名
	@param strEntryKey [in] エントリ名
	@param strEntryValue [i/o] エントリ値

	@retval true  成功
	@retval false 失敗

	@date 2003-10-22 D.S.Koba 作成
*/
bool CProfile::IOProfileData(
	const bool&							bRead,
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	std::basic_string< TCHAR >&			strEntryValue
)
{
	// 「読み込み」か「書き込み」か
	if( bRead ) return GetProfileData( strSectionName, strEntryKey, strEntryValue );
	else        return SetProfileData( strSectionName, strEntryKey, strEntryValue );
}

/*! エントリ値をProfileから読み込む
	
	@param strSectionName [in] セクション名
	@param strEntryKey [in] エントリ名
	@param strEntryValue [out] エントリ値

	@retval true 成功
	@retval false 失敗

	@date 2003-10-22 D.S.Koba 作成
*/
inline bool CProfile::GetProfileData(
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	std::basic_string< TCHAR >&			strEntryValue
)
{
	std::basic_string< TCHAR > strWork;
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator enditer = m_ProfileData.end();
	MAP_STR_STR_ITER mapiter;
	for( iter = m_ProfileData.begin(); iter != enditer; iter++ )
	{
		if( iter->strSectionName == strSectionName )
		{
			mapiter = iter->Data.find( strEntryKey );
			if( iter->Data.end() != mapiter )
			{
				strEntryValue = mapiter->second;
				return true;
			}
		}
	}
	return false;
}

/*! エントリをProfileへ書き込む
	
	@param strSectionName [in] セクション名
	@param strEntryKey [in] エントリ名
	@param strEntryValue [in] エントリ値

	@retval true  成功
	@retval false 失敗(処理を入れていないのでfalseは返らない)

	@date 2003-10-21 D.S.Koba 作成
*/
inline bool CProfile::SetProfileData(
	const std::basic_string< TCHAR >&	strSectionName,
	const std::basic_string< TCHAR >&	strEntryKey,
	const std::basic_string< TCHAR >&	strEntryValue
)
{
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator enditer = m_ProfileData.end();
	MAP_STR_STR_ITER mapiter;
	MAP_STR_STR_ITER mapenditer;
	for( iter = m_ProfileData.begin(); iter != enditer; iter++ )
	{
		if( iter->strSectionName == strSectionName )
		{
			//既存のセクションの場合
			mapiter = iter->Data.find( strEntryKey );
			if( iter->Data.end() != mapiter )
			{
				//既存のエントリの場合は値を上書き
				mapiter->second = strEntryValue;
				break;
			}
			else
			{
				//既存のエントリが見つからない場合は追加
				iter->Data.insert( PAIR_STR_STR( strEntryKey, strEntryValue ) );
				break;
			}
		}
	}
	//既存のセクションではない場合，セクション及びエントリを追加
	if( enditer == iter )
	{
		Section Buffer;
		Buffer.strSectionName = strSectionName;
		Buffer.Data.insert( PAIR_STR_STR( strEntryKey, strEntryValue ) );
		m_ProfileData.push_back( Buffer );
	}
	return true;
}



void CProfile::DUMP( void )
{
#ifdef _DEBUG
	std::vector< Section >::iterator iter;
	std::vector< Section >::iterator enditer = m_ProfileData.end();
	MAP_STR_STR_ITER mapiter;
	MAP_STR_STR_ITER mapenditer;
	MYTRACE( "\n\nCProfile::DUMP()======================" );
	for( iter = m_ProfileData.begin(); iter != enditer; iter++ )
	{
		MYTRACE( "\n■strSectionName=%s", iter->strSectionName.c_str() );
		mapenditer = iter->Data.end();
		for( mapiter = iter->Data.begin(); mapiter != mapenditer; mapiter++ )
		{
			MYTRACE( "\"%s\" = \"%s\"", mapiter->first.c_str(), mapiter->second.c_str() );
		}
	}
	MYTRACE( "========================================\n" );
#endif
	return;
}

/*[EOF]*/
