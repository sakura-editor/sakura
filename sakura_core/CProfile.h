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

class CProfile;
#ifndef _CPROFILE_H_
#define _CPROFILE_H_

#pragma warning( disable : 4786 )

#include <windows.h>
#include <string>
#include <vector>
#include <map>

#define tstring string

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief INIファイル入出力
*/
class CProfile
{
	//文字列型
	typedef std::tstring tstring;

	typedef std::pair< tstring, tstring > PAIR_STR_STR;
	typedef std::map< tstring, tstring > MAP_STR_STR;
	struct Section
	{
		tstring     strSectionName;
		MAP_STR_STR mapEntries;
	};

public:
	CProfile() {}
	~CProfile() {};
	void Init( void );
	bool IsReadingMode( void ) { return m_bRead; }
	void SetReadingMode( void ) { m_bRead = true; }
	void SetWritingMode( void ) { m_bRead = false; }
	bool ReadProfile( const TCHAR* );
	bool WriteProfile( const TCHAR*, const TCHAR* pszComment);

	bool IOProfileData( const TCHAR*, const TCHAR*, bool& );
	bool IOProfileData( const TCHAR*, const TCHAR*, int& );
	bool IOProfileData( const TCHAR*, const TCHAR*, WORD& );
	bool IOProfileData( const TCHAR*, const TCHAR*, TCHAR& );
	bool IOProfileData( const TCHAR*, const TCHAR*, TCHAR*, const int& );
	bool IOProfileData( const tstring&, const tstring&, tstring& );

	void DUMP( void );

protected:
	void ReadOneline( const tstring& line );
	bool _WriteFile( const tstring& strFilename, std::vector< tstring >& vecLine);

	bool GetProfileData( const TCHAR*, const TCHAR*, bool& );
	bool GetProfileData( const TCHAR*, const TCHAR*, int& );
	bool GetProfileData( const TCHAR*, const TCHAR*, WORD& );
	bool GetProfileData( const TCHAR*, const TCHAR*, TCHAR& );
	bool GetProfileData( const TCHAR*, const TCHAR*, TCHAR*, const int& );
	bool GetProfileData( const tstring&, const tstring&, tstring& );
	bool SetProfileData( const TCHAR*, const TCHAR*, const bool& );
	bool SetProfileData( const TCHAR*, const TCHAR*, const int& );
	bool SetProfileData( const TCHAR*, const TCHAR*, const WORD& );
	bool SetProfileData( const TCHAR*, const TCHAR*, const TCHAR& );
	bool SetProfileData( const TCHAR*, const TCHAR*, const TCHAR*, const int& );
	bool SetProfileData( const tstring&, const tstring&, const tstring& );

	// メンバ変数
	tstring					m_strProfileName;	//!< 最後に読み書きしたファイル名
	std::vector< Section >	m_ProfileData;
	bool					m_bRead;			//!< モード(true=読み込み/false=書き出し)
};

///////////////////////////////////////////////////////////////////////
#endif /* _CPROFILE_H_ */

/*[EOF]*/
