/*!	@file
	@brief 文字コード調査情報保持クラス

	@author Sakura-Editor collaborators
	@date 2006/12/10 新規作成
*/
/*
	Copyright (C) 2006, rastiv

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

#ifndef _CESI_H_
#define _CESI_H_

#include "global.h"
#include "charcode.h"

/*
	使用方法
	
	CESI( const char*, const int ) でオブジェクトを作成するか，
	ScanEncoding() を使ってデータを取得してから，
	Detect 系関数を使う.
*/


class CESI
{
public:
	
	// CODE_CODEMAX -2 := マルチバイト系文字コードの数
	enum enum_CESI_public_constants {
		NUM_OF_MBCODE = CODE_CODEMAX - 2,
	};
	
	CESI(){ }
	CESI( const char* pS, const int nLen ){ ScanEncoding( pS, nLen ); }
	bool ScanEncoding( const char*, const int );
	int DetectUnicode( WCCODE_INFO* );
	int DetectMultibyte( MBCODE_INFO* );
	
private:
	
	MBCODE_INFO m_pEI[NUM_OF_MBCODE];	//!< マルチバイト系コード情報
	UNICODE_INFO m_WEI;		//!< ユニコード系コード情報
};


#endif /*_CESI_H_*/

