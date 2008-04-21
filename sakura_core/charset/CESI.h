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

enum EJisESCSeqType;

/*
	文字コード判定情報 構造体群
*/
struct MBCODE_INFO{
	ECodeType eCodeID;		// 文字コード識別番号
	int nSpecBytes;			// 特有バイト数
	int nDiff;				// ポイント数 := 特有バイト数 － 不正バイト数
};
struct WCCODE_INFO{
	ECodeType eCodeID;	// 文字コード識別番号
	int nCRorLF;			// ワイド文字の改行の個数
	int nLostBytes;			// 不正バイト数
};
struct UNICODE_INFO{
	WCCODE_INFO Uni;		// 文字コード判定情報 for UNICODE
	WCCODE_INFO UniBe;		// 文字コード判定情報 for UNICODE BE
	int nCRorLF_ascii;		// マルチバイト文字の改行の個数
};


/*
	Encoding and Scan and Information class.

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

	//デバッグ
public:
	static CNativeT GetDebugInfo(const char* pBuf, int nBufLen);

private:
	MBCODE_INFO		m_pEI[NUM_OF_MBCODE];	//!< マルチバイト系コード情報
	UNICODE_INFO	m_WEI;					//!< ユニコード系コード情報


	// -- -- -- -- static -- -- -- -- //
	/*
	|| 共有実装ヘルパ関数
	*/
public:
	// --- ユニコード BOM 検出器
	static ECodeType DetectUnicodeBom( const char*, int );
protected:
	// --- 文字長予測．
	static int _GuessCharLen_utf8( const uchar_t*, const int nStrLen = 4 );
	static int _GuessCharLen_sjis( const uchar_t*, const int nStrLen = 2 );
	static int _GuessCharLen_eucjp( const uchar_t*, const int nStrLen = 3 );
	// --- 一文字チェック
	static int _CheckSJisChar( const uchar_t*, const int );
	static int _CheckSJisCharR( const uchar_t*, const int );  // CheckCharLenAsSJis の逆方向モード
	static int _CheckEucJpChar( const uchar_t*, const int );
	static int _CheckUtf8Char( const uchar_t*, const int );
	static int _imp_CheckUtf16Char( const uchar_t*, const int, bool );
	static int _CheckUtf16Char( const uchar_t*, const int );
	static int _CheckUtf16BeChar( const uchar_t*, const int );
	// --- UTF-7 正当性チェック
	static int _CheckUtf7SetDPart( const uchar_t*, const int, uchar_t*& ref_pNextC );
	static int _CheckUtf7SetBPart( const uchar_t*, const int, uchar_t*& ref_pNextC );
	// --- JIS エスケープシーケンス検出
	static int _DetectJisESCSeq( const uchar_t* pS, const int nLen, EJisESCSeqType* pnEscType );

	/*
	|| 文字列の文字コード情報を得る．
	*/
protected:
	// --- 文字列チェック
	static void _GetEncdInf_SJis( const char*, const int, MBCODE_INFO* );
	static void _GetEncdInf_Jis( const char*, const int, MBCODE_INFO* );
	static void _GetEncdInf_EucJp( const char*, const int, MBCODE_INFO* );
	static void _GetEncdInf_Utf8( const char*, const int, MBCODE_INFO* );
	static void _GetEncdInf_Utf7( const char*, const int, MBCODE_INFO* );
	static void _GetEncdInf_Uni( const char*, const int, UNICODE_INFO* );
};



#endif /*_CESI_H_*/

