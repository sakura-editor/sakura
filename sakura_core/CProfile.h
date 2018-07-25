/*!	@file
	@brief INIファイル入出力

	@author D.S.Koba
	@date 2003-10-21 D.S.Koba メンバ関数の名前と引数をそのままにしてメンバ変数，関数の中身を書き直し
	@date 2004-01-10 D.S.Koba 返値をBOOLからboolへ変更。IOProfileDataを型別の関数に分け，引数を減らす
	@date 2006-02-11 D.S.Koba 読み込み/書き出しを引数でなく，メンバで判別
	@date 2006-02-12 D.S.Koba IOProfileDataの中身の読み込みと書き出しを関数に分ける
*/
/*
	Copyright (C) 2003-2006, D.S.Koba

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

#ifndef _CPROFILE_H_
#define _CPROFILE_H_

#include <Windows.h>
#include <string>
#include <vector>
#include <map>

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief INIファイル入出力
*/
class CProfile
{
	//文字列型
	typedef std::wstring wstring;
	typedef std::string string;

	typedef std::pair< wstring, wstring > PAIR_STR_STR;
	typedef std::map< wstring, wstring > MAP_STR_STR;
	struct Section
	{
		wstring     strSectionName;
		MAP_STR_STR mapEntries;
	};

public:
	CProfile() {}
	~CProfile() {}
	void Init( void );
	bool IsReadingMode( void ) { return m_bRead; }
	void SetReadingMode( void ) { m_bRead = true; }
	void SetWritingMode( void ) { m_bRead = false; }
	bool ReadProfile( const TCHAR* );
	bool ReadProfileRes( const TCHAR*, const TCHAR*, std::vector<std::wstring>* = NULL );				// 200/5/19 Uchi
	bool WriteProfile( const TCHAR*, const WCHAR* pszComment);



	void DUMP( void );

protected:
	void ReadOneline( const wstring& line );
	bool _WriteFile( const tstring& strFilename, const std::vector< wstring >& vecLine);


	bool GetProfileDataImp( const wstring& strSectionName, const wstring& strEntryKey, wstring& strEntryValue);

	bool SetProfileDataImp( const wstring& strSectionName, const wstring& strEntryKey, const wstring& strEntryValue );

protected:
	// メンバ変数
	tstring					m_strProfileName;	//!< 最後に読み書きしたファイル名
	std::vector< Section >	m_ProfileData;
	bool					m_bRead;			//!< モード(true=読み込み/false=書き出し)
};

#define _INI_T LTEXT




///////////////////////////////////////////////////////////////////////
#endif /* _CPROFILE_H_ */


