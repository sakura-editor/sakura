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

class CProfile;
#ifndef _CPROFILE_H_
#define _CPROFILE_H_

#include <windows.h>
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
	typedef std::pair< std::basic_string< TCHAR >, std::basic_string< TCHAR > > PAIR_STR_STR;
	typedef std::vector< std::basic_string< TCHAR > >::iterator VEC_STR_ITER;
	typedef std::map< std::basic_string< TCHAR >, std::basic_string< TCHAR > >::iterator MAP_STR_STR_ITER;
	struct Section
	{
		std::basic_string< TCHAR > strSectionName;
		std::map< std::basic_string< TCHAR >, std::basic_string< TCHAR > > Data;
	};

public:
	CProfile() { return; }
	~CProfile() { return; }

	void Init( void );
	bool ReadProfile( const TCHAR* );
	bool ReadProfileSection( const TCHAR*, const TCHAR* );
	bool WriteProfile( const TCHAR*, const TCHAR* );
	bool WriteProfileSection( const TCHAR*, const TCHAR*, const TCHAR* );

	bool IOProfileData( const bool&, const TCHAR*, const TCHAR*, bool& );
	bool IOProfileData( const bool&, const TCHAR*, const TCHAR*, int& );
	bool IOProfileData( const bool&, const TCHAR*, const TCHAR*, WORD& );
	bool IOProfileData( const bool&, const TCHAR*, const TCHAR*, TCHAR& );
	bool IOProfileData( const bool&, const TCHAR*, const TCHAR*, TCHAR*, const int& );
	bool IOProfileData( const bool&, const std::basic_string< TCHAR >&, const std::basic_string< TCHAR >&, std::basic_string< TCHAR >& );

	void DUMP( void );

protected:
	bool GetProfileData( const std::basic_string< TCHAR >&, const std::basic_string< TCHAR >&, std::basic_string< TCHAR >& );
	bool SetProfileData( const std::basic_string< TCHAR >&, const std::basic_string< TCHAR >&, const std::basic_string< TCHAR >& );
	
	bool ReadFile( const std::basic_string< TCHAR >&, std::vector< std::basic_string< TCHAR > >& );

	// メンバ変数
	std::basic_string< TCHAR > m_strProfileName;//!< 最後に読み書きしたファイル名
	std::vector< Section > m_ProfileData;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROFILE_H_ */


/*[EOF]*/
